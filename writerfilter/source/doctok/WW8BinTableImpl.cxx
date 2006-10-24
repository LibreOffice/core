#include "WW8BinTableImpl.hxx"

namespace doctok
{
using namespace ::std;

void PageNumber::dump(OutputWithDepth<string> & /*output*/) const
{
}

sal_uInt32 WW8BinTableImpl::getPageNumber(const Fc & rFc) const
{
    sal_uInt32 nResult = 0;

    if (mPageMap.find(rFc) == mPageMap.end())
    {
#if 0
        sal_uInt32 n = getEntryCount();

        while (rFc < getFc(n))
        {
            --n;
        }

        nResult = getPageNumber(n);
        mPageMap[rFc] = nResult;
#else
        sal_uInt32 left = 0;
        sal_uInt32 right = getEntryCount();

        while (right - left > 1)
        {
            sal_uInt32 middle = (right + left) / 2;

            Fc aFc = getFc(middle);

            if (rFc < aFc)
                right = middle;
            else
                left = middle;

        }

        nResult = getPageNumber(left);
        mPageMap[rFc] = nResult;
#endif

    }
    else
        nResult = mPageMap[rFc];

    return nResult;
}

string WW8BinTableImpl::toString() const
{
    string aResult;
    char sBuffer[255];

    aResult += "(";

    for (sal_uInt32 n = 0; n < getEntryCount(); n++)
    {
        if (n > 0)
            aResult += ", ";

        snprintf(sBuffer, 255, "%lx", getFc(n).get());
        aResult += sBuffer;
        aResult += "->";
        snprintf(sBuffer, 255, "%lx", getPageNumber(n));
        aResult += sBuffer;
    }

    aResult += ")";

    return aResult;
}
}
