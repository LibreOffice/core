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


#include <cclass_unicode.hxx>
#include <unicode/uchar.h>
#include <rtl/character.hxx>
#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/LocaleData2.hpp>
#include <com/sun/star/i18n/NativeNumberMode.hpp>
#include <com/sun/star/i18n/NativeNumberSupplier.hpp>

#include <string.h>
#include <string_view>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

#define TOKEN_DIGIT_FLAGS (ParserFlags::CHAR_VALUE | ParserFlags::VALUE | ParserFlags::VALUE_EXP | ParserFlags::VALUE_EXP_VALUE | ParserFlags::VALUE_DIGIT)

namespace i18npool {

// Default identifier/name specification is [A-Za-z_][A-Za-z0-9_]*

const sal_uInt8 cclass_Unicode::nDefCnt = 128;
const ParserFlags cclass_Unicode::pDefaultParserTable[ nDefCnt ] =
{
// (...) == Calc formula compiler specific, commented out and modified

    /* \0 */    ParserFlags::EXCLUDED,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
    /*  9 \t */ ParserFlags::CHAR_DONTCARE | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,     // (ParserFlags::ILLEGAL)
                ParserFlags::ILLEGAL,
    /* 11 \v */ ParserFlags::CHAR_DONTCARE | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,     // (ParserFlags::ILLEGAL)
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
                ParserFlags::ILLEGAL,
    /*  32   */ ParserFlags::CHAR_DONTCARE | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  33 ! */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  34 " */ ParserFlags::CHAR_STRING | ParserFlags::STRING_SEP,
    /*  35 # */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::WORD_SEP)
    /*  36 $ */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::CHAR_WORD | ParserFlags::WORD)
    /*  37 % */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::VALUE)
    /*  38 & */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  39 ' */ ParserFlags::NAME_SEP,
    /*  40 ( */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  41 ) */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  42 * */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  43 + */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP | ParserFlags::VALUE_EXP | ParserFlags::VALUE_SIGN,
    /*  44 , */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::CHAR_VALUE | ParserFlags::VALUE)
    /*  45 - */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP | ParserFlags::VALUE_EXP | ParserFlags::VALUE_SIGN,
    /*  46 . */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::WORD | ParserFlags::CHAR_VALUE | ParserFlags::VALUE)
    /*  47 / */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    //for ( i = 48; i < 58; i++ )
    /*  48 0 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  49 1 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  50 2 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  51 3 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  52 4 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  53 5 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  54 6 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  55 7 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  56 8 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  57 9 */ TOKEN_DIGIT_FLAGS | ParserFlags::WORD,
    /*  58 : */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::WORD)
    /*  59 ; */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  60 < */ ParserFlags::CHAR_BOOL | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  61 = */ ParserFlags::CHAR | ParserFlags::BOOL | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  62 > */ ParserFlags::CHAR_BOOL | ParserFlags::BOOL | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  63 ? */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::CHAR_WORD | ParserFlags::WORD)
    /*  64 @ */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    //for ( i = 65; i < 91; i++ )
    /*  65 A */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  66 B */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  67 C */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  68 D */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  69 E */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  70 F */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  71 G */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  72 H */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  73 I */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  74 J */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  75 K */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  76 L */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  77 M */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  78 N */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  79 O */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  80 P */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  81 Q */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  82 R */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  83 S */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  84 T */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  85 U */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  86 V */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  87 W */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  88 X */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  89 Y */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  90 Z */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  91 [ */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    /*  92 \ */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    /*  93 ] */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    /*  94 ^ */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,
    /*  95 _ */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  96 ` */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    //for ( i = 97; i < 123; i++ )
    /*  97 a */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  98 b */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /*  99 c */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 100 d */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 101 e */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 102 f */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 103 g */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 104 h */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 105 i */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 106 j */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 107 k */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 108 l */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 109 m */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 110 n */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 111 o */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 112 p */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 113 q */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 114 r */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 115 s */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 116 t */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 117 u */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 118 v */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 119 w */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 120 x */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 121 y */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 122 z */ ParserFlags::CHAR_WORD | ParserFlags::WORD,
    /* 123 { */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    /* 124 | */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    /* 125 } */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    /* 126 ~ */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP,  // (ParserFlags::ILLEGAL // UNUSED)
    /* 127   */ ParserFlags::CHAR | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP   // (ParserFlags::ILLEGAL // UNUSED)
};


const sal_Int32 cclass_Unicode::pParseTokensType[ nDefCnt ] =
{
    /* \0 */    KParseTokens::ASC_OTHER,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
    /*  9 \t */ KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
    /* 11 \v */ KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
                KParseTokens::ASC_CONTROL,
    /*  32   */ KParseTokens::ASC_OTHER,
    /*  33 ! */ KParseTokens::ASC_OTHER,
    /*  34 " */ KParseTokens::ASC_OTHER,
    /*  35 # */ KParseTokens::ASC_OTHER,
    /*  36 $ */ KParseTokens::ASC_DOLLAR,
    /*  37 % */ KParseTokens::ASC_OTHER,
    /*  38 & */ KParseTokens::ASC_OTHER,
    /*  39 ' */ KParseTokens::ASC_OTHER,
    /*  40 ( */ KParseTokens::ASC_OTHER,
    /*  41 ) */ KParseTokens::ASC_OTHER,
    /*  42 * */ KParseTokens::ASC_OTHER,
    /*  43 + */ KParseTokens::ASC_OTHER,
    /*  44 , */ KParseTokens::ASC_OTHER,
    /*  45 - */ KParseTokens::ASC_OTHER,
    /*  46 . */ KParseTokens::ASC_DOT,
    /*  47 / */ KParseTokens::ASC_OTHER,
    //for ( i = 48; i < 58; i++ )
    /*  48 0 */ KParseTokens::ASC_DIGIT,
    /*  49 1 */ KParseTokens::ASC_DIGIT,
    /*  50 2 */ KParseTokens::ASC_DIGIT,
    /*  51 3 */ KParseTokens::ASC_DIGIT,
    /*  52 4 */ KParseTokens::ASC_DIGIT,
    /*  53 5 */ KParseTokens::ASC_DIGIT,
    /*  54 6 */ KParseTokens::ASC_DIGIT,
    /*  55 7 */ KParseTokens::ASC_DIGIT,
    /*  56 8 */ KParseTokens::ASC_DIGIT,
    /*  57 9 */ KParseTokens::ASC_DIGIT,
    /*  58 : */ KParseTokens::ASC_COLON,
    /*  59 ; */ KParseTokens::ASC_OTHER,
    /*  60 < */ KParseTokens::ASC_OTHER,
    /*  61 = */ KParseTokens::ASC_OTHER,
    /*  62 > */ KParseTokens::ASC_OTHER,
    /*  63 ? */ KParseTokens::ASC_OTHER,
    /*  64 @ */ KParseTokens::ASC_OTHER,
    //for ( i = 65; i < 91; i++ )
    /*  65 A */ KParseTokens::ASC_UPALPHA,
    /*  66 B */ KParseTokens::ASC_UPALPHA,
    /*  67 C */ KParseTokens::ASC_UPALPHA,
    /*  68 D */ KParseTokens::ASC_UPALPHA,
    /*  69 E */ KParseTokens::ASC_UPALPHA,
    /*  70 F */ KParseTokens::ASC_UPALPHA,
    /*  71 G */ KParseTokens::ASC_UPALPHA,
    /*  72 H */ KParseTokens::ASC_UPALPHA,
    /*  73 I */ KParseTokens::ASC_UPALPHA,
    /*  74 J */ KParseTokens::ASC_UPALPHA,
    /*  75 K */ KParseTokens::ASC_UPALPHA,
    /*  76 L */ KParseTokens::ASC_UPALPHA,
    /*  77 M */ KParseTokens::ASC_UPALPHA,
    /*  78 N */ KParseTokens::ASC_UPALPHA,
    /*  79 O */ KParseTokens::ASC_UPALPHA,
    /*  80 P */ KParseTokens::ASC_UPALPHA,
    /*  81 Q */ KParseTokens::ASC_UPALPHA,
    /*  82 R */ KParseTokens::ASC_UPALPHA,
    /*  83 S */ KParseTokens::ASC_UPALPHA,
    /*  84 T */ KParseTokens::ASC_UPALPHA,
    /*  85 U */ KParseTokens::ASC_UPALPHA,
    /*  86 V */ KParseTokens::ASC_UPALPHA,
    /*  87 W */ KParseTokens::ASC_UPALPHA,
    /*  88 X */ KParseTokens::ASC_UPALPHA,
    /*  89 Y */ KParseTokens::ASC_UPALPHA,
    /*  90 Z */ KParseTokens::ASC_UPALPHA,
    /*  91 [ */ KParseTokens::ASC_OTHER,
    /*  92 \ */ KParseTokens::ASC_OTHER,
    /*  93 ] */ KParseTokens::ASC_OTHER,
    /*  94 ^ */ KParseTokens::ASC_OTHER,
    /*  95 _ */ KParseTokens::ASC_UNDERSCORE,
    /*  96 ` */ KParseTokens::ASC_OTHER,
    //for ( i = 97; i < 123; i++ )
    /*  97 a */ KParseTokens::ASC_LOALPHA,
    /*  98 b */ KParseTokens::ASC_LOALPHA,
    /*  99 c */ KParseTokens::ASC_LOALPHA,
    /* 100 d */ KParseTokens::ASC_LOALPHA,
    /* 101 e */ KParseTokens::ASC_LOALPHA,
    /* 102 f */ KParseTokens::ASC_LOALPHA,
    /* 103 g */ KParseTokens::ASC_LOALPHA,
    /* 104 h */ KParseTokens::ASC_LOALPHA,
    /* 105 i */ KParseTokens::ASC_LOALPHA,
    /* 106 j */ KParseTokens::ASC_LOALPHA,
    /* 107 k */ KParseTokens::ASC_LOALPHA,
    /* 108 l */ KParseTokens::ASC_LOALPHA,
    /* 109 m */ KParseTokens::ASC_LOALPHA,
    /* 110 n */ KParseTokens::ASC_LOALPHA,
    /* 111 o */ KParseTokens::ASC_LOALPHA,
    /* 112 p */ KParseTokens::ASC_LOALPHA,
    /* 113 q */ KParseTokens::ASC_LOALPHA,
    /* 114 r */ KParseTokens::ASC_LOALPHA,
    /* 115 s */ KParseTokens::ASC_LOALPHA,
    /* 116 t */ KParseTokens::ASC_LOALPHA,
    /* 117 u */ KParseTokens::ASC_LOALPHA,
    /* 118 v */ KParseTokens::ASC_LOALPHA,
    /* 119 w */ KParseTokens::ASC_LOALPHA,
    /* 120 x */ KParseTokens::ASC_LOALPHA,
    /* 121 y */ KParseTokens::ASC_LOALPHA,
    /* 122 z */ KParseTokens::ASC_LOALPHA,
    /* 123 { */ KParseTokens::ASC_OTHER,
    /* 124 | */ KParseTokens::ASC_OTHER,
    /* 125 } */ KParseTokens::ASC_OTHER,
    /* 126 ~ */ KParseTokens::ASC_OTHER,
    /* 127   */ KParseTokens::ASC_OTHER
};


// static
const sal_Unicode* cclass_Unicode::StrChr( const sal_Unicode* pStr, sal_uInt32 c )
{
    if ( !pStr )
        return nullptr;
    sal_Unicode cs[2];
    auto const n = rtl::splitSurrogates(c, cs);
    while ( *pStr )
    {
        if ( *pStr == cs[0] && (n == 1 || pStr[1] == cs[1]) )
            return pStr;
        pStr++;
    }
    return nullptr;
}


sal_Int32 cclass_Unicode::getParseTokensType(sal_uInt32 const c, bool const isFirst)
{
    if ( c < nDefCnt )
        return pParseTokensType[ sal_uInt8(c) ];
    else
    {

        //! all KParseTokens::UNI_... must be matched
        switch (u_charType(c))
        {
            case U_UPPERCASE_LETTER :
                return KParseTokens::UNI_UPALPHA;
            case U_LOWERCASE_LETTER :
                return KParseTokens::UNI_LOALPHA;
            case U_TITLECASE_LETTER :
                return KParseTokens::UNI_TITLE_ALPHA;
            case U_MODIFIER_LETTER :
                return KParseTokens::UNI_MODIFIER_LETTER;
            case U_OTHER_LETTER :
                // Non_Spacing_Mark could not be as leading character
                if (isFirst) break;
                [[fallthrough]]; // treat it as Other_Letter.
            case U_NON_SPACING_MARK :
                return KParseTokens::UNI_OTHER_LETTER;
            case U_DECIMAL_DIGIT_NUMBER :
                return KParseTokens::UNI_DIGIT;
            case U_LETTER_NUMBER :
                return KParseTokens::UNI_LETTER_NUMBER;
            case U_OTHER_NUMBER :
                return KParseTokens::UNI_OTHER_NUMBER;
        }

        return KParseTokens::UNI_OTHER;
    }
}

void cclass_Unicode::setupInternational( const Locale& rLocale )
{
    bool bChanged = (aParserLocale.Language != rLocale.Language
        || aParserLocale.Country != rLocale.Country
        || aParserLocale.Variant != rLocale.Variant);
    if ( bChanged )
    {
        aParserLocale.Language = rLocale.Language;
        aParserLocale.Country = rLocale.Country;
        aParserLocale.Variant = rLocale.Variant;
    }
    if ( !mxLocaleData.is() )
    {
        mxLocaleData.set( LocaleData2::create(m_xContext) );
    }
}


void cclass_Unicode::setupParserTable( const Locale& rLocale, sal_Int32 startCharTokenType,
            const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
            const OUString& userDefinedCharactersCont )
{
    bool bIntlEqual = (rLocale.Language == aParserLocale.Language &&
        rLocale.Country == aParserLocale.Country &&
        rLocale.Variant == aParserLocale.Variant);
    if ( !pTable || !bIntlEqual ||
            startCharTokenType != nStartTypes ||
            contCharTokenType != nContTypes ||
            userDefinedCharactersStart != aStartChars ||
            userDefinedCharactersCont != aContChars )
        initParserTable( rLocale, startCharTokenType, userDefinedCharactersStart,
            contCharTokenType, userDefinedCharactersCont );
}


void cclass_Unicode::initParserTable( const Locale& rLocale, sal_Int32 startCharTokenType,
            const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
            const OUString& userDefinedCharactersCont )
{
    // (Re)Init
    setupInternational( rLocale );
    // Memory of pTable is reused.
    if ( !pTable )
        pTable.reset(new ParserFlags[nDefCnt]);
    memcpy( pTable.get(), pDefaultParserTable, sizeof(ParserFlags) * nDefCnt );
    // Start and cont tables only need reallocation if different length.
    if ( pStart && userDefinedCharactersStart.getLength() != aStartChars.getLength() )
    {
        pStart.reset();
    }
    if ( pCont && userDefinedCharactersCont.getLength() != aContChars.getLength() )
    {
        pCont.reset();
    }
    nStartTypes = startCharTokenType;
    nContTypes = contCharTokenType;
    aStartChars = userDefinedCharactersStart;
    aContChars = userDefinedCharactersCont;

    // specials
    if( mxLocaleData.is() )
    {
        LocaleDataItem2 aItem =
            mxLocaleData->getLocaleItem2( aParserLocale );
//!TODO: theoretically separators may be a string, adjustment would have to be
//! done here and in parsing and in ::rtl::math::stringToDouble()
        cGroupSep = aItem.thousandSeparator[0];
        cDecimalSep = aItem.decimalSeparator[0];
        cDecimalSepAlt = aItem.decimalSeparatorAlternative.toChar();
    }

    if (nContTypes & KParseTokens::GROUP_SEPARATOR_IN_NUMBER)
    {
        if ( cGroupSep < nDefCnt )
            pTable[cGroupSep] |= ParserFlags::VALUE;
    }
    else
    {
        cGroupSep = 0;
    }
    if ( cDecimalSep < nDefCnt )
        pTable[cDecimalSep] |= ParserFlags::CHAR_VALUE | ParserFlags::VALUE;
    if ( cDecimalSepAlt && cDecimalSepAlt < nDefCnt )
        pTable[cDecimalSepAlt] |= ParserFlags::CHAR_VALUE | ParserFlags::VALUE;

    // Modify characters according to KParseTokens definitions.
    {
        using namespace KParseTokens;
        sal_uInt8 i;

        if ( !(nStartTypes & ASC_UPALPHA) )
            for ( i = 65; i < 91; i++ )
                pTable[i] &= ~ParserFlags::CHAR_WORD;  // not allowed as start character
        if ( !(nContTypes & ASC_UPALPHA) )
            for ( i = 65; i < 91; i++ )
                pTable[i] &= ~ParserFlags::WORD;       // not allowed as cont character

        if ( !(nStartTypes & ASC_LOALPHA) )
            for ( i = 97; i < 123; i++ )
                pTable[i] &= ~ParserFlags::CHAR_WORD;  // not allowed as start character
        if ( !(nContTypes & ASC_LOALPHA) )
            for ( i = 97; i < 123; i++ )
                pTable[i] &= ~ParserFlags::WORD;       // not allowed as cont character

        if ( nStartTypes & ASC_DIGIT )
            for ( i = 48; i < 58; i++ )
                pTable[i] |= ParserFlags::CHAR_WORD;   // allowed as start character
        if ( !(nContTypes & ASC_DIGIT) )
            for ( i = 48; i < 58; i++ )
                pTable[i] &= ~ParserFlags::WORD;       // not allowed as cont character

        if ( !(nStartTypes & ASC_UNDERSCORE) )
            pTable[95] &= ~ParserFlags::CHAR_WORD;     // not allowed as start character
        if ( !(nContTypes & ASC_UNDERSCORE) )
            pTable[95] &= ~ParserFlags::WORD;          // not allowed as cont character

        if ( nStartTypes & ASC_DOLLAR )
            pTable[36] |= ParserFlags::CHAR_WORD;      // allowed as start character
        if ( nContTypes & ASC_DOLLAR )
            pTable[36] |= ParserFlags::WORD;           // allowed as cont character

        if ( nStartTypes & ASC_DOT )
            pTable[46] |= ParserFlags::CHAR_WORD;      // allowed as start character
        if ( nContTypes & ASC_DOT )
            pTable[46] |= ParserFlags::WORD;           // allowed as cont character

        if ( nStartTypes & ASC_COLON )
            pTable[58] |= ParserFlags::CHAR_WORD;      // allowed as start character
        if ( nContTypes & ASC_COLON )
            pTable[58] |= ParserFlags::WORD;           // allowed as cont character

        if ( nStartTypes & ASC_CONTROL )
            for ( i = 1; i < 32; i++ )
                pTable[i] |= ParserFlags::CHAR_WORD;   // allowed as start character
        if ( nContTypes & ASC_CONTROL )
            for ( i = 1; i < 32; i++ )
                pTable[i] |= ParserFlags::WORD;        // allowed as cont character

        if ( nStartTypes & ASC_ANY_BUT_CONTROL )
            for ( i = 32; i < nDefCnt; i++ )
                pTable[i] |= ParserFlags::CHAR_WORD;   // allowed as start character
        if ( nContTypes & ASC_ANY_BUT_CONTROL )
            for ( i = 32; i < nDefCnt; i++ )
                pTable[i] |= ParserFlags::WORD;        // allowed as cont character

    }

    // Merge in (positively override with) user defined characters.
    // StartChars
    sal_Int32 nLen = aStartChars.getLength();
    if ( nLen )
    {
        if ( !pStart )
            pStart.reset(new ParserFlags[ nLen ]);
        const sal_Unicode* p = aStartChars.getStr();
        for ( sal_Int32 j=0; j<nLen; j++, p++ )
        {
            pStart[j] = ParserFlags::CHAR_WORD;
            if ( *p < nDefCnt )
                pTable[*p] |= ParserFlags::CHAR_WORD;
        }
    }
    // ContChars
    nLen = aContChars.getLength();
    if ( nLen )
    {
        if ( !pCont )
            pCont.reset(new ParserFlags[ nLen ]);
        const sal_Unicode* p = aContChars.getStr();
        for ( sal_Int32 j=0; j<nLen; j++ )
        {
            pCont[j] = ParserFlags::WORD;
            if ( *p < nDefCnt )
                pTable[*p] |= ParserFlags::WORD;
        }
    }
}


void cclass_Unicode::destroyParserTable()
{
    pCont.reset();
    pStart.reset();
    pTable.reset();
}


ParserFlags cclass_Unicode::getFlags(sal_uInt32 const c)
{
    ParserFlags nMask;
    if ( c < nDefCnt )
        nMask = pTable[ sal_uInt8(c) ];
    else
        nMask = getFlagsExtended(c);
    switch ( eState )
    {
        case ssGetChar :
        case ssRewindFromValue :
        case ssIgnoreLeadingInRewind :
        case ssGetWordFirstChar :
            if ( !(nMask & ParserFlags::CHAR_WORD) )
            {
                nMask |= getStartCharsFlags( c );
                if ( nMask & ParserFlags::CHAR_WORD )
                    nMask &= ~ParserFlags::EXCLUDED;
            }
        break;
        case ssGetValue :
        case ssGetWord :
            if ( !(nMask & ParserFlags::WORD) )
            {
                nMask |= getContCharsFlags( c );
                if ( nMask & ParserFlags::WORD )
                    nMask &= ~ParserFlags::EXCLUDED;
            }
        break;
        default:
            ;   // other cases aren't needed, no compiler warning
    }
    return nMask;
}


ParserFlags cclass_Unicode::getFlagsExtended(sal_uInt32 const c)
{
    if ( c == cGroupSep )
        return ParserFlags::VALUE;
    else if ( c == cDecimalSep )
        return ParserFlags::CHAR_VALUE | ParserFlags::VALUE;
    else if ( cDecimalSepAlt && c == cDecimalSepAlt )
        return ParserFlags::CHAR_VALUE | ParserFlags::VALUE;
    bool bStart = (eState == ssGetChar || eState == ssGetWordFirstChar ||
            eState == ssRewindFromValue || eState == ssIgnoreLeadingInRewind);
    sal_Int32 nTypes = (bStart ? nStartTypes : nContTypes);

    //! all KParseTokens::UNI_... must be matched
    switch (u_charType(c))
    {
        case U_UPPERCASE_LETTER :
            return (nTypes & KParseTokens::UNI_UPALPHA) ?
                (bStart ? ParserFlags::CHAR_WORD : ParserFlags::WORD) :
                ParserFlags::ILLEGAL;
        case U_LOWERCASE_LETTER :
            return (nTypes & KParseTokens::UNI_LOALPHA) ?
                (bStart ? ParserFlags::CHAR_WORD : ParserFlags::WORD) :
                ParserFlags::ILLEGAL;
        case U_TITLECASE_LETTER :
            return (nTypes & KParseTokens::UNI_TITLE_ALPHA) ?
                (bStart ? ParserFlags::CHAR_WORD : ParserFlags::WORD) :
                ParserFlags::ILLEGAL;
        case U_MODIFIER_LETTER :
            return (nTypes & KParseTokens::UNI_MODIFIER_LETTER) ?
                (bStart ? ParserFlags::CHAR_WORD : ParserFlags::WORD) :
                ParserFlags::ILLEGAL;
        case U_NON_SPACING_MARK :
        case U_COMBINING_SPACING_MARK :
            // Non_Spacing_Mark can't be a leading character,
            // nor can a spacing combining mark.
            if (bStart)
                return ParserFlags::ILLEGAL;
            [[fallthrough]]; // treat it as Other_Letter.
        case U_OTHER_LETTER :
            return (nTypes & KParseTokens::UNI_OTHER_LETTER) ?
                (bStart ? ParserFlags::CHAR_WORD : ParserFlags::WORD) :
                ParserFlags::ILLEGAL;
        case U_DECIMAL_DIGIT_NUMBER :
            return ((nTypes & KParseTokens::UNI_DIGIT) ?
                (bStart ? ParserFlags::CHAR_WORD : ParserFlags::WORD) :
                ParserFlags::ILLEGAL) | TOKEN_DIGIT_FLAGS;
        case U_LETTER_NUMBER :
            return ((nTypes & KParseTokens::UNI_LETTER_NUMBER) ?
                (bStart ? ParserFlags::CHAR_WORD : ParserFlags::WORD) :
                ParserFlags::ILLEGAL) | TOKEN_DIGIT_FLAGS;
        case U_OTHER_NUMBER :
            return ((nTypes & KParseTokens::UNI_OTHER_NUMBER) ?
                (bStart ? ParserFlags::CHAR_WORD : ParserFlags::WORD) :
                ParserFlags::ILLEGAL) | TOKEN_DIGIT_FLAGS;
        case U_SPACE_SEPARATOR :
            return ((nTypes & KParseTokens::IGNORE_LEADING_WS) ?
                ParserFlags::CHAR_DONTCARE : (bStart ? ParserFlags::CHAR_WORD : (ParserFlags::CHAR_DONTCARE | ParserFlags::WORD_SEP | ParserFlags::VALUE_SEP) ));
        case U_OTHER_PUNCTUATION:
            // fdo#61754 Lets see (if we not at the start) if this is midletter
            // punctuation and allow it in a word if it is similarly to
            // U_NON_SPACING_MARK, for example U+00B7 MIDDLE DOT.
            // tdf#123575 for U+30FB KATAKANA MIDDLE DOT property is not
            // U_WB_MIDLETTER but U_WB_KATAKANA instead, explicitly test that
            // and U+FF65 HALFWIDTH KATAKANA MIDDLE DOT.
            if (bStart || (U_WB_MIDLETTER != u_getIntPropertyValue(c, UCHAR_WORD_BREAK)
                        && c != 0x30FB && c != 0xFF65))
                return ParserFlags::ILLEGAL;
            else
            {
                //allowing it to continue the word
                return (nTypes & KParseTokens::UNI_OTHER_LETTER) ?
                    ParserFlags::WORD : ParserFlags::ILLEGAL;
            }
            break;
    }

    return ParserFlags::ILLEGAL;
}


ParserFlags cclass_Unicode::getStartCharsFlags( sal_uInt32 c )
{
    if ( pStart )
    {
        const sal_Unicode* pStr = aStartChars.getStr();
        const sal_Unicode* p = StrChr( pStr, c );
        if ( p )
            return pStart[ p - pStr ];
    }
    return ParserFlags::ILLEGAL;
}


ParserFlags cclass_Unicode::getContCharsFlags( sal_Unicode c )
{
    if ( pCont )
    {
        const sal_Unicode* pStr = aContChars.getStr();
        const sal_Unicode* p = StrChr( pStr, c );
        if ( p )
            return pCont[ p - pStr ];
    }
    return ParserFlags::ILLEGAL;
}


void cclass_Unicode::parseText( ParseResult& r, const OUString& rText, sal_Int32 nPos, sal_Int32 nTokenType )
{
    assert(r.LeadingWhiteSpace == 0);
    eState = ssGetChar;

    //! All the variables below (plus ParseResult) have to be resetted on ssRewindFromValue!
    OUStringBuffer aSymbol;
    bool isFirst(true);
    sal_Int32 index(nPos); // index of next code point after current
    sal_Int32 postSymbolIndex(index); // index of code point following last quote
    sal_uInt32 current((index < rText.getLength()) ? rText.iterateCodePoints(&index) : 0);
    sal_uInt32 cLast = 0;
    sal_Int32 nCodePoints(0);
    int nDecSeps = 0;
    bool bQuote = false;
    bool bMightBeWord = true;
    bool bMightBeWordLast = true;
    bool bDecSepAltUsed = false;
    //! All the variables above (plus ParseResult) have to be resetted on ssRewindFromValue!
    sal_Int32 nextCharIndex(nPos); // == index of nextChar

    while ((current != 0) && (eState != ssStop))
    {
        ++nCodePoints;
        ParserFlags nMask = getFlags(current);
        if ( nMask & ParserFlags::EXCLUDED )
            eState = ssBounce;
        if ( bMightBeWord )
        {   // only relevant for ssGetValue fall back
            if ( eState == ssGetChar || eState == ssRewindFromValue ||
                    eState == ssIgnoreLeadingInRewind )
                bMightBeWord = bool(nMask & ParserFlags::CHAR_WORD);
            else
                bMightBeWord = bool(nMask & ParserFlags::WORD);
        }
        sal_Int32 nParseTokensType = getParseTokensType(current, isFirst);
        isFirst = false;
        sal_Int32 const nextIndex(nextCharIndex); // == index of char following current
        nextCharIndex = index; // == index of nextChar
        sal_uInt32 nextChar((index < rText.getLength()) ? rText.iterateCodePoints(&index) : 0);
        switch (eState)
        {
            case ssGetChar :
            case ssRewindFromValue :
            case ssIgnoreLeadingInRewind :
            {
                if ( (nMask & ParserFlags::CHAR_VALUE) && eState != ssRewindFromValue
                        && eState != ssIgnoreLeadingInRewind )
                {   //! must be first, may fall back to ssGetWord via bMightBeWord
                    eState = ssGetValue;
                    if ( nMask & ParserFlags::VALUE_DIGIT )
                    {
                        if (128 <= current)
                            r.TokenType = KParseType::UNI_NUMBER;
                        else
                            r.TokenType = KParseType::ASC_NUMBER;
                    }
                    else if (current == cDecimalSep || (bDecSepAltUsed = (cDecimalSepAlt && current == cDecimalSepAlt)))
                    {
                        if (nextChar)
                            ++nDecSeps;
                        else
                            eState = ssRewindFromValue;
                            // retry for ONE_SINGLE_CHAR or others
                    }
                }
                else if ( nMask & ParserFlags::CHAR_WORD )
                {
                    eState = ssGetWord;
                    r.TokenType = KParseType::IDENTNAME;
                }
                else if ( nMask & ParserFlags::NAME_SEP )
                {
                    eState = ssGetWordFirstChar;
                    bQuote = true;
                    postSymbolIndex = nextCharIndex;
                    nParseTokensType = 0;   // will be taken of first real character
                    r.TokenType = KParseType::SINGLE_QUOTE_NAME;
                }
                else if ( nMask & ParserFlags::CHAR_STRING )
                {
                    eState = ssGetString;
                    postSymbolIndex = nextCharIndex;
                    nParseTokensType = 0;   // will be taken of first real character
                    r.TokenType = KParseType::DOUBLE_QUOTE_STRING;
                }
                else if ( nMask & ParserFlags::CHAR_DONTCARE )
                {
                    if ( nStartTypes & KParseTokens::IGNORE_LEADING_WS )
                    {
                        if (eState == ssRewindFromValue)
                            eState = ssIgnoreLeadingInRewind;
                        r.LeadingWhiteSpace = nextCharIndex - nPos;
                        nCodePoints--; // exclude leading whitespace
                        postSymbolIndex = nextCharIndex;
                        nParseTokensType = 0;   // wait until real character
                        bMightBeWord = true;
                    }
                    else
                        eState = ssBounce;
                }
                else if ( nMask & ParserFlags::CHAR_BOOL )
                {
                    eState = ssGetBool;
                    r.TokenType = KParseType::BOOLEAN;
                }
                else if ( nMask & ParserFlags::CHAR )
                {   //! must be last
                    eState = ssStop;
                    r.TokenType = KParseType::ONE_SINGLE_CHAR;
                }
                else
                    eState = ssBounce;      // not known
            }
            break;
            case ssGetValue :
            {
                if ( nMask & ParserFlags::VALUE_DIGIT )
                {
                    if (128 <= current)
                        r.TokenType = KParseType::UNI_NUMBER;
                    else if ( r.TokenType != KParseType::UNI_NUMBER )
                        r.TokenType = KParseType::ASC_NUMBER;
                }
                if ( nMask & ParserFlags::VALUE )
                {
                    if (current == cGroupSep)
                    {
                        if (getFlags(nextChar) & ParserFlags::VALUE_DIGIT)
                            nParseTokensType |= KParseTokens::GROUP_SEPARATOR_IN_NUMBER;
                        else
                        {
                            // Trailing group separator character is not a
                            // group separator.
                            eState = ssStopBack;
                        }
                    }
                    else if ((current == cDecimalSep ||
                                (bDecSepAltUsed = (cDecimalSepAlt && current == cDecimalSepAlt))) &&
                            ++nDecSeps > 1)
                    {
                        if (nCodePoints == 2)
                            eState = ssRewindFromValue;
                            // consecutive separators
                        else
                            eState = ssStopBack;
                    }
                    // else keep it going
                }
                else if (current == 'E' || current == 'e')
                {
                    ParserFlags nNext = getFlags(nextChar);
                    if ( nNext & ParserFlags::VALUE_EXP )
                        ;   // keep it going
                    else if (bMightBeWord && ((nNext & ParserFlags::WORD) || !nextChar))
                    {   // might be a numerical name (1.2efg)
                        eState = ssGetWord;
                        r.TokenType = KParseType::IDENTNAME;
                    }
                    else
                        eState = ssStopBack;
                }
                else if ( nMask & ParserFlags::VALUE_SIGN )
                {
                    if ( (cLast == 'E') || (cLast == 'e') )
                    {
                        ParserFlags nNext = getFlags(nextChar);
                        if ( nNext & ParserFlags::VALUE_EXP_VALUE )
                            ;   // keep it going
                        else if (bMightBeWord && ((nNext & ParserFlags::WORD) || !nextChar))
                        {   // might be a numerical name (1.2e+fg)
                            eState = ssGetWord;
                            r.TokenType = KParseType::IDENTNAME;
                        }
                        else
                            eState = ssStopBack;
                    }
                    else if ( bMightBeWord )
                    {   // might be a numerical name (1.2+fg)
                        eState = ssGetWord;
                        r.TokenType = KParseType::IDENTNAME;
                    }
                    else
                        eState = ssStopBack;
                }
                else if ( bMightBeWord && (nMask & ParserFlags::WORD) )
                {   // might be a numerical name (1995.A1)
                    eState = ssGetWord;
                    r.TokenType = KParseType::IDENTNAME;
                }
                else
                    eState = ssStopBack;
            }
            break;
            case ssGetWordFirstChar :
                eState = ssGetWord;
                [[fallthrough]];
            case ssGetWord :
            {
                if ( nMask & ParserFlags::WORD )
                    ;   // keep it going
                else if ( nMask & ParserFlags::NAME_SEP )
                {
                    if ( bQuote )
                    {
                        if ( cLast == '\\' )
                        {   // escaped
                            aSymbol.append(std::u16string_view(rText).substr(postSymbolIndex, nextCharIndex - postSymbolIndex - 2));
                            aSymbol.append(OUString(&current, 1));
                        }
                        else
                        {
                            eState = ssStop;
                            aSymbol.append(std::u16string_view(rText).substr(postSymbolIndex, nextCharIndex - postSymbolIndex - 1));
                        }
                        postSymbolIndex = nextCharIndex;
                    }
                    else
                        eState = ssStopBack;
                }
                else if ( bQuote )
                    ;   // keep it going
                else
                    eState = ssStopBack;
            }
            break;
            case ssGetString :
            {
                if ( nMask & ParserFlags::STRING_SEP )
                {
                    if ( cLast == '\\' )
                    {   // escaped
                        aSymbol.append(std::u16string_view(rText).substr(postSymbolIndex, nextCharIndex - postSymbolIndex - 2));
                        aSymbol.append(OUString(&current, 1));
                    }
                    else if (current == nextChar &&
                            !(nContTypes & KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING) )
                    {   // "" => literal " escaped
                        aSymbol.append(std::u16string_view(rText).substr(postSymbolIndex, nextCharIndex - postSymbolIndex));
                        nextCharIndex = index;
                        if (index < rText.getLength()) { ++nCodePoints; }
                        nextChar = (index < rText.getLength()) ? rText.iterateCodePoints(&index) : 0;
                    }
                    else
                    {
                        eState = ssStop;
                        aSymbol.append(std::u16string_view(rText).substr(postSymbolIndex, nextCharIndex - postSymbolIndex - 1));
                    }
                    postSymbolIndex = nextCharIndex;
                }
            }
            break;
            case ssGetBool :
            {
                if ( nMask & ParserFlags::BOOL )
                    eState = ssStop;    // maximum 2: <, >, <>, <=, >=
                else
                    eState = ssStopBack;
            }
            break;
            case ssStopBack :
            case ssBounce :
            case ssStop :
                ;   // nothing, no compiler warning
            break;
        }
        if ( eState == ssRewindFromValue )
        {
            r = ParseResult();
            index = nPos;
            postSymbolIndex = nPos;
            nextCharIndex = nPos;
            aSymbol.setLength(0);
            current = (index < rText.getLength()) ? rText.iterateCodePoints(&index) : 0;
            nCodePoints = (nPos < rText.getLength()) ? 1 : 0;
            isFirst = true;
            cLast = 0;
            nDecSeps = 0;
            bQuote = false;
            bMightBeWord = true;
            bMightBeWordLast = true;
            bDecSepAltUsed = false;
        }
        else
        {
            if ( !(r.TokenType & nTokenType) )
            {
                if ( (r.TokenType & (KParseType::ASC_NUMBER | KParseType::UNI_NUMBER))
                        && (nTokenType & KParseType::IDENTNAME) && bMightBeWord )
                    ;   // keep a number that might be a word
                else if (r.LeadingWhiteSpace == (nextCharIndex - nPos))
                    ;   // keep ignored white space
                else if ( !r.TokenType && eState == ssGetValue && (nMask & ParserFlags::VALUE_SEP) )
                    ;   // keep uncertain value
                else
                    eState = ssBounce;
            }
            if ( eState == ssBounce )
            {
                r.TokenType = 0;
                eState = ssStopBack;
            }
            if ( eState == ssStopBack )
            {   // put back
                nextChar = rText.iterateCodePoints(&index, -1);
                nextCharIndex = nextIndex;
                --nCodePoints;
                bMightBeWord = bMightBeWordLast;
                eState = ssStop;
            }
            if ( eState != ssStop )
            {
                if ( !r.StartFlags )
                    r.StartFlags |= nParseTokensType;
                else
                    r.ContFlags |= nParseTokensType;
            }
            bMightBeWordLast = bMightBeWord;
            cLast = current;
            current = nextChar;
        }
    }
    // r.CharLen is the length in characters (not code units) of the parsed
    // token not including any leading white space.
    r.CharLen = nCodePoints;
    r.EndPos = nextCharIndex;
    if ( r.TokenType & KParseType::ASC_NUMBER )
    {
        r.Value = rtl_math_uStringToDouble(rText.getStr() + nPos + r.LeadingWhiteSpace,
            rText.getStr() + r.EndPos, (bDecSepAltUsed ? cDecimalSepAlt : cDecimalSep), cGroupSep, nullptr, nullptr);
        if ( bMightBeWord )
            r.TokenType |= KParseType::IDENTNAME;
    }
    else if ( r.TokenType & KParseType::UNI_NUMBER )
    {
        if ( !xNatNumSup.is() )
        {
            if ( m_xContext.is() )
            {
                xNatNumSup = NativeNumberSupplier::create( m_xContext );
            }
        }
        OUString aTmp(rText.getStr() + nPos + r.LeadingWhiteSpace,
                r.EndPos - nPos - r.LeadingWhiteSpace);
        // transliterate to ASCII
        aTmp = xNatNumSup->getNativeNumberString( aTmp, aParserLocale,
                NativeNumberMode::NATNUM0 );
        r.Value = ::rtl::math::stringToDouble( aTmp, cDecimalSep, cGroupSep );
        if ( bMightBeWord )
            r.TokenType |= KParseType::IDENTNAME;
    }
    else if ( r.TokenType & (KParseType::SINGLE_QUOTE_NAME | KParseType::DOUBLE_QUOTE_STRING) )
    {
        if (postSymbolIndex < nextCharIndex)
        {   //! open quote
            aSymbol.append(std::u16string_view(rText).substr(postSymbolIndex, nextCharIndex - postSymbolIndex - 1));
            r.TokenType |= KParseType::MISSING_QUOTE;
        }
        r.DequotedNameOrString = aSymbol.toString();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
