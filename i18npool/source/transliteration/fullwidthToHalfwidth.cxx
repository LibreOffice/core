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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <i18nutil/widthfolding.hxx>
#define TRANSLITERATION_fullwidthToHalfwidth
#define TRANSLITERATION_fullwidthKatakanaToHalfwidthKatakana
#define TRANSLITERATION_fullwidthToHalfwidthLikeASC
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

using ::rtl::OUString;

namespace com { namespace sun { namespace star { namespace i18n {

fullwidthToHalfwidth::fullwidthToHalfwidth()
{
    func = (TransFunc) 0;
    table = &widthfolding::getfull2halfTable();
    transliterationName = "fullwidthToHalfwidth";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTH_HALFWIDTH";
}

/**
 * Transliterate fullwidth to halfwidth.
 * The output is a reference of OUString. You MUST delete this object when you do not need to use it any more
 * The output string contains a transliterated string only, not whole string.
 */
OUString SAL_CALL
fullwidthToHalfwidth::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    useOffset = sal_False;
    const OUString &tmp = transliteration_OneToOne::transliterate( newStr, 0, newStr.getLength(), offset);
    useOffset = sal_True;
    return tmp;
}

sal_Unicode SAL_CALL
fullwidthToHalfwidth::transliterateChar2Char( sal_Unicode inChar)
  throw(RuntimeException, MultipleCharsOutputException)
{
    sal_Unicode newChar = widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

fullwidthKatakanaToHalfwidthKatakana::fullwidthKatakanaToHalfwidthKatakana()
{
    func = (TransFunc) 0;
    table = &widthfolding::getfullKana2halfKanaTable();
    transliterationName = "fullwidthKatakanaToHalfwidthKatakana";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTHKATAKANA_HALFWIDTHKATAKANA";
}

/**
 * Transliterate fullwidth katakana to halfwidth katakana.
 */
OUString SAL_CALL
fullwidthKatakanaToHalfwidthKatakana::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    useOffset = sal_False;
    const OUString &tmp = transliteration_OneToOne::transliterate( newStr, 0, newStr.getLength(), offset);
    useOffset = sal_True;
    return tmp;
}

sal_Unicode SAL_CALL
fullwidthKatakanaToHalfwidthKatakana::transliterateChar2Char( sal_Unicode inChar )
  throw(RuntimeException, MultipleCharsOutputException)
{
    sal_Unicode newChar = widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

fullwidthToHalfwidthLikeASC::fullwidthToHalfwidthLikeASC()
{
    func = (TransFunc) 0;
    table = &widthfolding::getfull2halfTableForASC();
    transliterationName = "fullwidthToHalfwidthLikeASC";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTH_HALFWIDTH_LIKE_ASC";
}

/**
 * Transliterate fullwidth to halfwidth like Excel's ASC function.
 */
OUString SAL_CALL
fullwidthToHalfwidthLikeASC::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    useOffset = sal_False;
    const OUString &tmp = transliteration_OneToOne::transliterate( newStr, 0, newStr.getLength(), offset);
    useOffset = sal_True;

    return tmp;
}

sal_Unicode SAL_CALL
fullwidthToHalfwidthLikeASC::transliterateChar2Char( sal_Unicode inChar )
  throw(RuntimeException, MultipleCharsOutputException)
{
    sal_Unicode newChar = widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
