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

#include <cclass_unicode.hxx>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <unicode/uchar.h>
#include <i18nutil/x_rtl_ustring.h>
#include <breakiteratorImpl.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {
//  ----------------------------------------------------
//  class cclass_Unicode
//  ----------------------------------------------------;

cclass_Unicode::cclass_Unicode( uno::Reference < XMultiServiceFactory > xSMgr ) : xMSF( xSMgr ),
        pTable( NULL ),
        pStart( NULL ),
        pCont( NULL ),
        nStartTypes( 0 ),
        nContTypes( 0 ),
        eState( ssGetChar ),
        cGroupSep( ',' ),
        cDecimalSep( '.' )
{
    trans = new Transliteration_casemapping();
    cClass = "com.sun.star.i18n.CharacterClassification_Unicode";
}

cclass_Unicode::~cclass_Unicode() {
    destroyParserTable();
    delete trans;
}


OUString SAL_CALL
cclass_Unicode::toUpper( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    sal_Int32 len = Text.getLength();
    if (nPos >= len)
        return OUString();
    if (nCount + nPos > len)
        nCount = len - nPos;

    trans->setMappingType(MappingTypeToUpper, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

OUString SAL_CALL
cclass_Unicode::toLower( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    sal_Int32 len = Text.getLength();
    if (nPos >= len)
        return OUString();
    if (nCount + nPos > len)
        nCount = len - nPos;

    trans->setMappingType(MappingTypeToLower, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

OUString SAL_CALL
cclass_Unicode::toTitle( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException) {
    sal_Int32 len = Text.getLength();
    if (nPos >= len)
        return OUString();
    if (nCount + nPos > len)
        nCount = len - nPos;

    trans->setMappingType(MappingTypeToTitle, rLocale);
    rtl_uString* pStr = x_rtl_uString_new_WithLength( nCount, 1 );
    sal_Unicode* out = pStr->buffer;
    BreakIteratorImpl brk(xMSF);
    Boundary bdy = brk.getWordBoundary(Text, nPos, rLocale,
                WordType::ANYWORD_IGNOREWHITESPACES, sal_True);
    for (sal_Int32 i = nPos; i < nCount + nPos; i++, out++) {
        if (i >= bdy.endPos)
            bdy = brk.nextWord(Text, bdy.endPos, rLocale,
                        WordType::ANYWORD_IGNOREWHITESPACES);
        *out = (i == bdy.startPos) ?
            trans->transliterateChar2Char(Text[i]) : Text[i];
    }
    *out = 0;
    return OUString( pStr, SAL_NO_ACQUIRE );
}

sal_Int16 SAL_CALL
cclass_Unicode::getType( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;
    return (sal_Int16) u_charType(Text.iterateCodePoints(&nPos, 0));
}

sal_Int16 SAL_CALL
cclass_Unicode::getCharacterDirection( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;
    return (sal_Int16) u_charDirection(Text.iterateCodePoints(&nPos, 0));
}


sal_Int16 SAL_CALL
cclass_Unicode::getScript( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;
    // ICU Unicode script type UBlockCode starts from 1 for Basci Latin,
    // while OO.o enum UnicideScript starts from 0.
    // To map ICU UBlockCode to OO.o UnicodeScript, it needs to shift 1.
    return (sal_Int16) ublock_getCode(Text.iterateCodePoints(&nPos, 0))-1;
}


sal_Int32 SAL_CALL
cclass_Unicode::getCharType( const OUString& Text, sal_Int32* nPos, sal_Int32 increment) {
    using namespace ::com::sun::star::i18n::KCharacterType;

    sal_uInt32 ch = Text.iterateCodePoints(nPos, increment);
    if (increment > 0) ch = Text.iterateCodePoints(nPos, 0);
    switch ( u_charType(ch) ) {
    // Upper
    case U_UPPERCASE_LETTER :
        return UPPER|LETTER|PRINTABLE|BASE_FORM;

    // Lower
    case U_LOWERCASE_LETTER :
        return LOWER|LETTER|PRINTABLE|BASE_FORM;

    // Title
    case U_TITLECASE_LETTER :
        return TITLE_CASE|LETTER|PRINTABLE|BASE_FORM;

    // Letter
    case U_MODIFIER_LETTER :
    case U_OTHER_LETTER :
        return LETTER|PRINTABLE|BASE_FORM;

    // Digit
    case U_DECIMAL_DIGIT_NUMBER:
    case U_LETTER_NUMBER:
    case U_OTHER_NUMBER:
        return DIGIT|PRINTABLE|BASE_FORM;

    // Base
    case U_NON_SPACING_MARK:
    case U_ENCLOSING_MARK:
    case U_COMBINING_SPACING_MARK:
        return BASE_FORM|PRINTABLE;

    // Print
    case U_SPACE_SEPARATOR:

    case U_DASH_PUNCTUATION:
    case U_INITIAL_PUNCTUATION:
    case U_FINAL_PUNCTUATION:
    case U_CONNECTOR_PUNCTUATION:
    case U_OTHER_PUNCTUATION:

    case U_MATH_SYMBOL:
    case U_CURRENCY_SYMBOL:
    case U_MODIFIER_SYMBOL:
    case U_OTHER_SYMBOL:
        return PRINTABLE;

    // Control
    case U_CONTROL_CHAR:
    case U_FORMAT_CHAR:
        return CONTROL;

    case U_LINE_SEPARATOR:
    case U_PARAGRAPH_SEPARATOR:
        return CONTROL|PRINTABLE;

    // for all others
    default:
        return U_GENERAL_OTHER_TYPES;
    }
}

sal_Int32 SAL_CALL
cclass_Unicode::getCharacterType( const OUString& Text, sal_Int32 nPos, const Locale& /*rLocale*/ ) throw(RuntimeException) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;
    return getCharType(Text, &nPos, 0);

}

sal_Int32 SAL_CALL
cclass_Unicode::getStringType( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& /*rLocale*/ ) throw(RuntimeException) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;

    sal_Int32 result = getCharType(Text, &nPos, 0);
    for (sal_Int32 i = 1; i < nCount && nPos < Text.getLength(); i++)
        result |= getCharType(Text, &nPos, 1);
    return result;
}

ParseResult SAL_CALL cclass_Unicode::parseAnyToken(
            const OUString& Text,
            sal_Int32 nPos,
            const Locale& rLocale,
            sal_Int32 startCharTokenType,
            const OUString& userDefinedCharactersStart,
            sal_Int32 contCharTokenType,
            const OUString& userDefinedCharactersCont )
                throw(RuntimeException)
{
    ParseResult r;
    if ( Text.getLength() <= nPos )
        return r;

    setupParserTable( rLocale,
        startCharTokenType, userDefinedCharactersStart,
        contCharTokenType, userDefinedCharactersCont );
    parseText( r, Text, nPos );

    return r;
}


ParseResult SAL_CALL cclass_Unicode::parsePredefinedToken(
            sal_Int32 nTokenType,
            const OUString& Text,
            sal_Int32 nPos,
            const Locale& rLocale,
            sal_Int32 startCharTokenType,
            const OUString& userDefinedCharactersStart,
            sal_Int32 contCharTokenType,
            const OUString& userDefinedCharactersCont )
                throw(RuntimeException)
{
    ParseResult r;
    if ( Text.getLength() <= nPos )
        return r;

    setupParserTable( rLocale,
        startCharTokenType, userDefinedCharactersStart,
        contCharTokenType, userDefinedCharactersCont );
    parseText( r, Text, nPos, nTokenType );

    return r;
}

OUString SAL_CALL cclass_Unicode::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(cClass);
}


sal_Bool SAL_CALL cclass_Unicode::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cClass);
}

Sequence< OUString > SAL_CALL cclass_Unicode::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cClass);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
