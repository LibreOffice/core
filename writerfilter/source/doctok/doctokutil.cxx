#include <util.hxx>

using namespace ::std;

namespace doctok {
void util_assert(bool bTest)
{
    if (! bTest)
        clog << "ASSERT!\n" << endl;
}

void printBytes(ostream & o, const string & str)
{
    unsigned int nCount = str.size();
    for (unsigned int n = 0; n < nCount; ++n)
    {
        unsigned char c = static_cast<unsigned char>(str[n]);
        if (c < 128 && isprint(c))
            o << str[n];
        else
            o << ".";
    }
}

}

