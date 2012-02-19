#include "LuceneHelper.hxx"

std::vector<TCHAR> OUStringToTCHARVec(rtl::OUString const &rStr)
{
    //UTF-16
    if (sizeof(TCHAR) == sizeof(sal_Unicode))
        return std::vector<TCHAR>(rStr.getStr(), rStr.getStr() + rStr.getLength() + 1);

    //UTF-32
    std::vector<TCHAR> aRet;
    for (sal_Int32 nStrIndex = 0; nStrIndex < rStr.getLength() + 1; )
    {
        const sal_uInt32 nCode = rStr.iterateCodePoints(&nStrIndex);
        aRet.push_back(nCode);
    }
    return aRet;
}

inline unsigned tstrlen(TCHAR const *str) {
	unsigned i;
	for (i = 0; str[i] != 0; ++i) {}
	return i;
}

rtl::OUString TCHARArrayToOUString(TCHAR const *str)
{
	// UTF-16
	if (sizeof(TCHAR) == sizeof(sal_Unicode))
		return rtl::OUString((sal_Unicode*) str);

	// UTF-32
	return rtl::OUString((char*) str, tstrlen(str), RTL_TEXTENCODING_UCS4);
}
