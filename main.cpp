/*
 * Задание от Yandex
 * Вам дан заголовочный файл с описанием интерфейса,
 * текст которого приведен ниже.
 * Напишите библиотечный класс почтового ящика
 * для обмена данными между потоками:
 * один поток пишет, другой читает. В своей работе учтите условия:

 * обмен должен быть неблокирующий и потокобезопасный;
 * класс должен использовать статическую модель памяти;
 * при записи в заполненный ящик старые данные не должны быть повреждены,
 * а новые (записываемые) данные могут быть отброшены;
 * при чтении из пустого ящика по данным должно быть понятно,
 * что они невалидные.
 */

#include <atomic>
#include <iomanip>
#include <iostream>
#include <optional>
#include <thread>

enum class ErrorCode { OK, ERROR };

template <typename T>
class IMailBox {
 public:
  virtual bool IsEmpty() = 0;
  virtual bool IsFull() = 0;
  virtual ErrorCode SendData(const T &data) = 0;
  virtual ErrorCode GetData(std::optional<T> &data) = 0;
};

template <typename T>
class IMailBoxImpl : public IMailBox<T> {
 public:
  bool IsEmpty() override { return !IsFull(); }

  bool IsFull() override { return full_.load(std::memory_order_acquire); }

  ErrorCode SendData(const T &data) override {
    if (IsEmpty()) {
      data_ = data;
      full_.store(true, std::memory_order_release);
      return ErrorCode::OK;
    }
    return ErrorCode::ERROR;
  }

  ErrorCode GetData(std::optional<T> &data) override {
    if (IsFull()) {
      data = {data_};
      full_.store(false, std::memory_order_release);
      return ErrorCode::OK;
    }
    data.reset();
    return ErrorCode::ERROR;
  }

 private:
  T data_;
  std::atomic<bool> full_;
};

const int NUMBER_ITERATIONS{100};
const int TIME_TO_SLEEP_MILLSEC {5};


int main() {
  std::cout << "Yandex task" << std::endl;

  std::unique_ptr<IMailBox<int>> mail_box =
      std::make_unique<IMailBoxImpl<int>>();

  // thread - writer
  std::thread twriter([&]() {
    int i = 0;
    while (i < NUMBER_ITERATIONS) {
      if (ErrorCode::OK == mail_box->SendData(i)) {
        std::cout << "twriter: " << std::setw(4) << i << std::endl;
      } else {
        std::cout << "twriter: IMailBox is full" << std::endl;
      }
      i++;
      std::this_thread::sleep_for(
          std::chrono::milliseconds(TIME_TO_SLEEP_MILLSEC));  // for imitation of random writing
    }
  });

  // thread - reader
  std::thread treader([&]() {
    int i = 0;
    std::optional<int> data{};
    while (i < NUMBER_ITERATIONS) {
      if (mail_box->GetData(data) == ErrorCode::OK) {
        std::cout << "treader: " << std::setw(4) << data.value() << std::endl;
      } else {
        std::cout << "treader: invalid data" << std::endl;
      }
      i++;
      std::this_thread::sleep_for(
          std::chrono::milliseconds(TIME_TO_SLEEP_MILLSEC+1));  // for imitation of random reading
    }
  });

  if (twriter.joinable()) {
    twriter.join();
  }

  if (treader.joinable()) {
    treader.join();
  }

  return 0;
}
