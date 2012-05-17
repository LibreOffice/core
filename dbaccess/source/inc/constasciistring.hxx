/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _DBASHARED_CONSTASCIISTRING_HXX_
#define _DBASHARED_CONSTASCIISTRING_HXX_

#ifndef CONSTASCII_INCLUDED_INDIRECT
#error "don't include this file directly! use stringconstants.hrc instead!"
#endif


// no namespaces. This file is included from several other files _within_ a namespace.

#define DECLARE_CONSTASCII_USTRING( name ) \
    extern const ConstAsciiString name

#define IMPLEMENT_CONSTASCII_USTRING( name, string ) \
    const ConstAsciiString name = {RTL_CONSTASCII_STRINGPARAM(string)}

#endif // _DBASHARED_CONSTASCIISTRING_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
