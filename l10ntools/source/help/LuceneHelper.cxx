#include "LuceneHelper.hxx"

std::vector<TCHAR> OUStringToTCHARVec(rtl::OUString const &rStr)
{
    //UTF-16
    if (sizeof(TCHAR) == sizeof(sal_Unicode))
        return std::vector<TCHAR>(rStr.getStr(), rStr.getStr() + rStr.getLength() + 1);

    //UTF-32
    std::vector<TCHAR> aRet;
    for (sal_Int32 nStrIndex = 0; nStrIndex < rStr.getLength(); )
    {
        const sal_uInt32 nCode = rStr.iterateCodePoints(&nStrIndex);
        aRet.push_back(nCode);
    }
    aRet.push_back(0);
    return aRet;
}

rtl::OUString TCHARArrayToOUString(TCHAR const *str)
{
    // UTF-16
    if (sizeof(TCHAR) == sizeof(sal_Unicode))
        return rtl::OUString((const sal_Unicode*)(str));

    // UTF-32
    return ::OUString((const sal_uInt32*)str, wcslen(str));
}
