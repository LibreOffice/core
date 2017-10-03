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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ref.hxx>

#include <transliteration_Ignore.hxx>
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace i18npool {

OUString SAL_CALL
ignoreKana::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
{
    rtl::Reference< hiraganaToKatakana > t1(new hiraganaToKatakana);
    return t1->transliterate(inStr, startPos, nCount, offset);
}

Sequence< OUString > SAL_CALL
ignoreKana::transliterateRange( const OUString& str1, const OUString& str2 )
{
    rtl::Reference< hiraganaToKatakana > t1(new hiraganaToKatakana);
    rtl::Reference< katakanaToHiragana > t2(new katakanaToHiragana);

    return transliteration_Ignore::transliterateRange(str1, str2, *t1.get(), *t2.get());
}

sal_Unicode SAL_CALL
ignoreKana::transliterateChar2Char( sal_Unicode inChar)
{
    rtl::Reference< hiraganaToKatakana > t1(new hiraganaToKatakana);
    return t1->transliterateChar2Char(inChar);
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_i18n_Transliteration_IGNORE_KANA_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new i18npool::ignoreKana());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
