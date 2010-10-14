/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CCLASS_CJK_H_
#define _CCLASS_CJK_H_

#include <cclass_unicode.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define MAXLEN  31      // Define max length for CJK number

#define ZERO        0x0030  // Define unicode zero
#define ONE     0x0031  // Define unicode one

class cclass_CJK : public cclass_Unicode {
public:
    cclass_CJK( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~cclass_CJK();
    i18n::ParseResult SAL_CALL parsePredefinedToken(
        sal_Int32 nTokenType,
        const rtl::OUString& Text,
        sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale,
        sal_Int32 nStartCharFlags,
        const rtl::OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags,
        const rtl::OUString& userDefinedCharactersCont )
        throw(com::sun::star::uno::RuntimeException);

protected:
    sal_Int32 size;
    rtl::OUString number;
    sal_Unicode *text;
    sal_Int32 textPos;

    sal_Unicode *Upper;
    sal_Unicode *Base;
    sal_Unicode *Plus;

    static sal_Int32 cclass_CJK::upperVal[];
    static sal_Int32 cclass_CJK::baseVal[];
    static sal_Int32 cclass_CJK::plusVal[];

private:
    sal_Int32 ToNum();
    sal_Int32 ToNum(sal_Unicode *n, sal_Int32 s);
    void Init(sal_Unicode *n, sal_Int32 s);
    void NumberCopy(sal_Unicode *s, sal_Unicode *t, sal_Int32 n);
    void NumberReverse(sal_Unicode *s, sal_Int32 n);
    sal_Int32 oneDigit(sal_Unicode s);
    sal_Int32 baseDigit(sal_Unicode s);
    sal_Int32 plusDigit(sal_Unicode s);
};

#define CCLASS_CJK( name ) \
class name : public cclass_CJK \
{ \
public: \
    name ( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF ); \
};

#ifdef CCLASS_ALL
CCLASS_CJK(cclass_zh)
CCLASS_CJK(cclass_zh_TW)
CCLASS_CJK(cclass_ko)
#endif
#undef CCLASS_CJK

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
