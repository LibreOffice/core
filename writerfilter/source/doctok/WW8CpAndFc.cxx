#include <WW8CpAndFc.hxx>

#include <iterator>
#include <algorithm>
#include <string>
#include <map>

namespace doctok
{
using namespace ::std;

bool operator < (const Cp & rA, const Cp & rB)
{
    return rA.nCp < rB.nCp;
}

bool operator == (const Cp & rA, const Cp & rB)
{
    return rA.nCp == rB.nCp;
}

string Cp::toString() const
{
    char sBuffer[256];

    snprintf(sBuffer, 255, "%lx", get());

    return string(sBuffer);
}

ostream & operator << (ostream & o, const Cp & rCp)
{
    return o << rCp.toString();
}

bool operator < (const Fc & rA, const Fc & rB)
{
    return rA.mnFc < rB.mnFc;
}

bool operator == (const Fc & rA, const Fc & rB)
{
    return rA.mnFc == rB.mnFc;
}

string Fc::toString() const
{
    char sBuffer[256];

    snprintf(sBuffer, 255, "(%lx, %s)", static_cast<sal_uInt32>(get()),
             isComplex() ? "true" : "false");

    return string(sBuffer);
}

ostream & operator << (ostream & o, const Fc & rFc)
{

    return o << rFc.toString();
}

bool operator < (const CpAndFc & rA, const CpAndFc & rB)
{
    bool bResult = false;

    if (rA.mCp < rB.mCp)
        bResult = true;
    else if (rA.mCp == rB.mCp && rA.mType < rB.mType)
        bResult = true;

    return bResult;
}

bool operator == (const CpAndFc & rA, const CpAndFc & rB)
{
    return rA.mCp == rB.mCp;
}

ostream & operator << (ostream & o, const CpAndFc & rCpAndFc)
{
    return o << rCpAndFc.toString();
}

ostream & operator << (ostream & o, const CpAndFcs & rCpAndFcs)
{
    copy(rCpAndFcs.begin(), rCpAndFcs.end(),
         ostream_iterator<CpAndFc>(o, ", "));

    char sBuffer[256];

    snprintf(sBuffer, 255, "%d", rCpAndFcs.size());
    o << sBuffer;

    return o;
}
}
