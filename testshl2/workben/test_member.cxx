#include <iostream>
#include <string>

using namespace std;

class Aint
{
    int n;

public:

    Aint():n(0)
        {
            cout << "ctor Aint()" << endl;
        }
    Aint(int _n):n(_n)
        {
            cout << "ctor Aint(int(" << _n << "))" << endl;
        }
    int getA() {return n;}
    ~Aint()
        {
            cout << "dtor Aint()" << endl;
        }

};

// -----------------------------------------------------------------------------

class B
{
    Aint a;
    Aint b;
    std::string aStr;

public:

    B()
            :a(2)
        {
            cout << "ctor B()" << endl;
        }
    B(int _a)
            :a(_a)
        {
            cout << "ctor B(int)" << endl;
        }

    void foo()
        {
            cout << "a.getA() := " << a.getA() << endl;
            cout << "b.getA() := " << b.getA() << endl;
        }

};


// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    B b(4);
    b.foo();
}
