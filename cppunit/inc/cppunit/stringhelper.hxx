#ifndef STRINGHELPER_HXX
#define STRINGHELPER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

inline void operator <<= (rtl::OString& _rAsciiString, const rtl::OUString& _rUnicodeString)
{
    _rAsciiString = rtl::OUStringToOString(_rUnicodeString,RTL_TEXTENCODING_ASCII_US);
}

#endif
