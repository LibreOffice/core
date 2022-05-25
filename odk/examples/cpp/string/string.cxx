#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

#include <sal/config.h>
#include <sal/main.h>
#include <sal/types.h>

#include <cppuhelper/bootstrap.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <cppu/unotype.hxx>

using namespace css;
using namespace rtl;

SAL_IMPLEMENT_MAIN()
{
    // The text is: "واحِدْ إثٍنين ثلاثةٌ" which means "one two three"
    OUString aOneTwoThree(reinterpret_cast<const sal_Unicode*>(
        u"\u0648\u0627\u062d\u0650\u062f\u0652 \u0625\u062b\u064d\u0646\u064a\u0646"
        " \u062b\u0644\u0627\u062b\u0629\u064c"));
    std::cout << "" << aOneTwoThree << std::endl;

    OString oStr = OUStringToOString(aOneTwoThree, RTL_TEXTENCODING_UTF8);
    printf("Unicode OUString: %s\n", oStr.pData->buffer);

    OString oStr2("پ");
    std::cout << "Unicode OString: " << oStr2 << std::endl;

    // storing ASCII
    char c = 'a';
    printf("Non-Unicode C Character: %c\n", c);

    char s[] = "عربي";
    printf("8-bit encodded String: %s\n", s);

    setlocale(LC_ALL, "");
    wchar_t w_char = L'ب';
    printf("16-bit encodded String: %lc\n", w_char);

    sal_Unicode ouChar = u'ب';
    printf("Unicode Character: %lc\n", ouChar);

    rtl_String* rtl_str = nullptr;
    rtl_string_newFromStr(&rtl_str, "پ");
    printf("rtl_String: %s\n", rtl_str->buffer);
    rtl_string_release(rtl_str);

    rtl_uString* rtl_ustr = nullptr;
    rtl_uString_newFromStr(&rtl_ustr, (sal_Unicode*)u"الف");
    printf("%s", "rtl_uString: ");

//Windows specific
#ifdef _WIN32
    std::wstring w_string = L"الف";
    wprintf("std::wstring: %ls\n", w_string.c_str());
    printf("%ls\n", rtl_ustr->buffer);
#else
    for (int i = 0; i < rtl_ustr_getLength(rtl_ustr->buffer); ++i)
        printf("%lc", rtl_ustr->buffer[i]);
#endif
    rtl_uString_release(rtl_ustr);

    std::cout << std::endl;

    std::vector<char> vectorChar(11);
    strncpy(vectorChar.data(), "ABCDEFGHIJ", 11);
    printf("%s\n", vectorChar.data());

    return 0;
}
