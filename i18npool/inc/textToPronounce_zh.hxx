/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once
#if 1

#include <transliteration_Ignore.hxx>
#include <indexentrysupplier_asian.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class TextToPronounce_zh : public transliteration_Ignore
{
protected:
#ifndef DISABLE_DYNLOADING
        oslModule hModule;
#endif
        sal_uInt16 **idx;
        const sal_Unicode* SAL_CALL getPronounce(const sal_Unicode ch);

public:
#ifndef DISABLE_DYNLOADING
        TextToPronounce_zh(const sal_Char* func_name);
#else
        TextToPronounce_zh(sal_uInt16 ** (*function)());
#endif
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
