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
#ifndef _I18N_TRANSLITERATION_TEXTTOPRONOUNCE_ZH_HXX_
#define _I18N_TRANSLITERATION_TEXTTOPRONOUNCE_ZH_HXX_

#include <transliteration_Ignore.hxx>
#include <indexentrysupplier_asian.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class TextToPronounce_zh : public transliteration_Ignore
{
protected:
        oslModule hModule;
        sal_uInt16 **idx;
        const sal_Unicode* SAL_CALL getPronounce(const sal_Unicode ch);

public:
        TextToPronounce_zh(const sal_Char* func_name);
        ~TextToPronounce_zh();

        rtl::OUString SAL_CALL
        folding(const rtl::OUString & inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 > & offset)
        throw (com::sun::star::uno::RuntimeException);

        sal_Int16 SAL_CALL getType() throw(com::sun::star::uno::RuntimeException);

        sal_Bool SAL_CALL
        equals( const rtl::OUString & str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32 & nMatch1, const rtl::OUString & str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32 & nMatch2)
        throw (com::sun::star::uno::RuntimeException);

        rtl::OUString SAL_CALL
        transliterateChar2String( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException);

        sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException,
            com::sun::star::i18n::MultipleCharsOutputException);
};

#define TRANSLITERATION_TextToPronounce_zh( name ) \
class name : public TextToPronounce_zh \
{ \
public: \
        name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_TextToPronounce_zh( TextToPinyin_zh_CN)
TRANSLITERATION_TextToPronounce_zh( TextToChuyin_zh_TW)
#endif
#undef TRANSLITERATION_TextToPronounce_zh

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
