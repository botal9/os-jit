#include <cstring>
#include <iostream>
#include <sys/mman.h>

class function {
public:
    function() = default;

    function(const unsigned char function_code[], size_t size, size_t modification_offset)
        : size(size)
        , modification_offset(modification_offset)
    {
        code = mmap(nullptr, size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (code == MAP_FAILED) {
            code = nullptr;
            std::cout << "Unable to allocate memory: " << strerror(errno) << std::endl;
        }
        std::memcpy(code, function_code, size);
    }

    ~function() {
        if (munmap(code, size) == -1) {
            std::cout << "Unable to deallocate memory: " << strerror(errno) << std::endl;
        }
    }

    function(const function& other) = delete;

public:
    void modify(unsigned int value) {
        if (code == nullptr || mprotect(code, size, PROT_READ | PROT_WRITE) == -1) {
            throw std::runtime_error("Unable to modify function code: " + std::string(strerror(errno)));
        }
        char* from = static_cast<char*>(code) + modification_offset;
        for (char* i = from; i < from + sizeof(unsigned int); ++i) {
            *i = static_cast<unsigned char>(value);
            value >>= 8u;
        }
    }

    unsigned int execute() {
        if (code == nullptr || mprotect(code, size, PROT_READ | PROT_EXEC) == -1) {
            throw std::runtime_error("Unable to execute function code: " + std::string(strerror(errno)));
        }
        return ((unsigned int (*)())code)();
    }

private:
    const size_t size = 0;
    const size_t modification_offset = 0;
    void* code = nullptr;
};

/*
unsigned int count_ones() {
    unsigned int arg = 0xffff1111;
    unsigned int result = 0;
    while (arg) {
        result += (arg & 1);
        arg >>= 1;
    }
    return result;
}
 */
const unsigned char code[] = {0x55, 0x48, 0x89, 0xe5, 0xc7, 0x45, 0xf8, 0x11, 0x11, 0xff, 0xff, 0xc7, 0x45, 0xfc, 0x00,
                              0x00, 0x00, 0x00, 0x83, 0x7d, 0xf8, 0x00, 0x74, 0x0e, 0x8b, 0x45, 0xf8, 0x83, 0xe0, 0x01,
                              0x01, 0x45, 0xfc, 0xd1, 0x6d, 0xf8, 0xeb, 0xec, 0x8b, 0x45, 0xfc, 0x5d, 0xc3};

int main() {
    function count{code, sizeof(code), 7};

    std::string command;
    while (std::cin >> command) {
        if (std::cin.eof() || command == "exit") {
            break;
        } else if (command == "execute" || command == "e") {
            try {
                std::cout << "Executed. result: " << count.execute() << std::endl;
            } catch (std::runtime_error& e) {
                std::cout << e.what() << std::endl;
            }
        } else if (command == "modify"|| command == "m") {
            unsigned int arg;
            std::cin >> arg;
            try {
                count.modify(arg);
            } catch (std::runtime_error& e) {
                std::cout << e.what() << std::endl;
            }
            std::cout << "Modification success" << std::endl;
        } else {
            std::cout << "Wrong command!" << std::endl;
        }
    }
}
