#include <iostream>
#include <string>
#include <rtl/string.hxx>

using namespace std;

#define atest(name) \
   { \
   /*if (strlen(#name))*/ { \
       cout << "String len: " << strlen(#name) << " string: " << #name << endl; \
   }}


// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    cout << "test preprocessor: " << endl;
    atest(fuck);
    atest();
    atest("");
}
