#include src.hpp
#include <iostream>
int main(){
    init(64,4);
    int a1 = malloc(8);
    std::cout << a1 << n;
    int b1 = malloc(8);
    std::cout << b1 << n;
    free_at(b1,8);
    int c1 = malloc(8);
    std::cout << c1 << n;
    int d1 = malloc_at(32,16);
    std::cout << d1 << n;
    free_at(a1,8);
    free_at(c1,8);
    free_at(d1,16);
    return 0;
}
