#include <iostream>
#include "WW8OutputWithDepth.hxx"

namespace doctok
{

using namespace ::std;

WW8OutputWithDepth::WW8OutputWithDepth()
: OutputWithDepth<string>("<tablegroup>", "</tablegroup>")
{
}

void WW8OutputWithDepth::output(const string & str) const
{
    cout << str << endl;
}
}
