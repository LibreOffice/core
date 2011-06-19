/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef STRINGHELPER_HXX
#define STRINGHELPER_HXX

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

inline void operator <<= (rtl::OString& _rAsciiString, rtl::OUString const & _rUnicodeString)
{
    _rAsciiString = rtl::OUStringToOString(_rUnicodeString,RTL_TEXTENCODING_ASCII_US);
}
inline void operator <<= (rtl::OUString& _rUnicodeString, rtl::OString const & _rAsciiString )
{
    _rUnicodeString = rtl::OStringToOUString(_rAsciiString, RTL_TEXTENCODING_ASCII_US);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
