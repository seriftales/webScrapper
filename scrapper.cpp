#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib> // rand() ve srand() için
#include <ctime>   // time() için

// --- 1. Paylaşılan Kaynaklar ve Kilit ---
// Başarılı indirme sayısını tutan paylaşılan sayaç
int successful_downloads = 0;

// Sayaca erişimi senkronize etmek için Mutex (Kilit)
std::mutex counter_mutex;

/**
 * @brief Bir URL'den veri çekme (indirme) işlemini simüle eder.
 *
 * @param url: İndirilecek web adresi (string).
 */
void download_url(const std::string& url) {
    // Thread ID'sini konsola yazdırarak hangi thread'in çalıştığını görelim.
    std::cout << "Thread " << std::this_thread::get_id() << ": "
              << url << " adresinden veri cekiliyor..." << std::endl;

    // Gerçek bir ağ beklemesi yerine, rastgele bir süre (200ms - 1000ms) uyuyarak 
    // G/Ç beklemesini (I/O Block) simüle edelim. Bu, işlemcinin bu thread'i bırakıp
    // diğer thread'lere geçmesine olanak tanır.
    int sleep_ms = 200 + (rand() % 800);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms)); 
    // Diğer thread'ler bu süre içinde paralel olarak çalışmaya devam edecektir.

    // ----------------------------------------------------
    // KRİTİK BÖLÜM BAŞLANGICI: Paylaşılan kaynağa erişim
    // ----------------------------------------------------
    
    // std::lock_guard, Mutex'i kilitler. Kapsam (scope) dışına çıkıldığında 
    // (fonksiyon bitince), kilidi otomatik olarak serbest bırakır (RAII prensibi).
    std::lock_guard<std::mutex> lock(counter_mutex);
    
    successful_downloads++; // Paylaşılan sayacı güvenli bir şekilde artır.
    
    // Kilit bu satırdan sonra otomatik serbest bırakılır.
    // ----------------------------------------------------

    std::cout << "Thread " << std::this_thread::get_id() << ": "
              << url << " TAMAMLANDI (" << sleep_ms << "ms). Toplam Basarili: "
              << successful_downloads << std::endl;
}

int main() {
    // Rastgelelik için tohum ayarı
    std::srand(std::time(0)); 

    // 2. URL Listesi (Görevin Verisi)
    std::vector<std::string> urls = {
        "https://site-a.com/page1", "https://site-b.com/data", "https://site-c.com/info",
        "https://site-d.com/login", "https://site-e.com/product", "https://site-f.com/contact",
        "https://site-g.com/about", "https://site-h.com/blog", "https://site-i.com/media",
        "https://site-j.com/pricing", "https://site-k.com/services", "https://site-l.com/help"
    };

    // Thread nesnelerini tutacak bir vector
    std::vector<std::thread> workers;

    // Çalışma süresini ölçmeye başla
    auto start_time = std::chrono::high_resolution_clock::now();

    // 3. Thread'leri Oluşturma ve Başlatma
    for (const auto& url : urls) {
        // Her URL için yeni bir thread oluştur ve download_url fonksiyonunu parametre ile başlat.
        workers.emplace_back(download_url, url);
    }

    std::cout << "\nAna Thread: Tum isci thread'ler baslatildi (" 
              << workers.size() << " adet). Bitmeleri bekleniyor...\n" << std::endl;

    // 4. Thread'lerin Bitmesini Bekleme (join)
    // Ana thread, tüm işçi thread'ler bitene kadar bekler. Bu olmadan program erken sonlanabilir.
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    // Çalışma süresini hesapla
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 5. Sonuçları Görüntüleme
    std::cout << "\n--------------------------------------------------" << std::endl;
    std::cout << "TUM INDIRMELER TAMAMLANDI." << std::endl;
    std::cout << "Toplam Basarili Indirme: " << successful_downloads << " / " << urls.size() << std::endl;
    std::cout << "Gecen Toplam Sure (Paralel): **" << duration.count() << " ms**" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    

    return 0;
}