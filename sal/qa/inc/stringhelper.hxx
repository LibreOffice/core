#ifndef STRINGHELPER_HXX
#define STRINGHELPER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

inline void operator <<= (rtl::OString& _rAsciiString, rtl::OUString const & _rUnicodeString)
{
    _rAsciiString = rtl::OUStringToOString(_rUnicodeString,RTL_TEXTENCODING_ASCII_US);
}
inline void operator <<= (rtl::OUString& _rUnicodeString, rtl::OString const & _rAsciiString )
{
    _rUnicodeString = rtl::OStringToOUString(_rAsciiString, RTL_TEXTENCODING_ASCII_US);
}

#endif
