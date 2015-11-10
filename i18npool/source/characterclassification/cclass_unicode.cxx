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
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <unicode/uchar.h>
#include <comphelper/string.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <breakiteratorImpl.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

//  class cclass_Unicode
//  ----------------------------------------------------;

cclass_Unicode::cclass_Unicode( const uno::Reference < XComponentContext >& rxContext ) : m_xContext( rxContext ),
        pTable( nullptr ),
        pStart( nullptr ),
        pCont( nullptr ),
        nStartTypes( 0 ),
        nContTypes( 0 ),
        eState( ssGetChar ),
        cGroupSep( ',' ),
        cDecimalSep( '.' )
{
    trans = new Transliteration_casemapping();
}

cclass_Unicode::~cclass_Unicode() {
    destroyParserTable();
    delete trans;
}


OUString SAL_CALL
cclass_Unicode::toUpper( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException, std::exception) {
    sal_Int32 len = Text.getLength();
    if (nPos >= len)
        return OUString();
    if (nCount + nPos > len)
        nCount = len - nPos;

    trans->setMappingType(MappingTypeToUpper, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

OUString SAL_CALL
cclass_Unicode::toLower( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException, std::exception) {
    sal_Int32 len = Text.getLength();
    if (nPos >= len)
        return OUString();
    if (nCount + nPos > len)
        nCount = len - nPos;

    trans->setMappingType(MappingTypeToLower, rLocale);
    return trans->transliterateString2String(Text, nPos, nCount);
}

OUString SAL_CALL
cclass_Unicode::toTitle( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException, std::exception) {
    try
    {
        sal_Int32 len = Text.getLength();
        if (nPos >= len)
            return OUString();
        if (nCount + nPos > len)
            nCount = len - nPos;

        trans->setMappingType(MappingTypeToTitle, rLocale);
        rtl_uString* pStr = rtl_uString_alloc(nCount);
        sal_Unicode* out = pStr->buffer;
        Reference< BreakIteratorImpl > xBrk(new BreakIteratorImpl(m_xContext));
        Boundary bdy = xBrk->getWordBoundary(Text, nPos, rLocale,
                    WordType::ANYWORD_IGNOREWHITESPACES, sal_True);
        for (sal_Int32 i = nPos; i < nCount + nPos; i++, out++) {
            if (i >= bdy.endPos)
                bdy = xBrk->nextWord(Text, bdy.endPos, rLocale,
                            WordType::ANYWORD_IGNOREWHITESPACES);
            *out = (i == bdy.startPos) ?
                trans->transliterateChar2Char(Text[i]) : Text[i];
        }
        *out = 0;
        return OUString( pStr, SAL_NO_ACQUIRE );
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception& e)
    {
        uno::Any a(cppu::getCaughtException());
        throw lang::WrappedTargetRuntimeException(
            "wrapped " + a.getValueTypeName() + ": " + e.Message,
            uno::Reference<uno::XInterface>(), a);
    }
}

sal_Int16 SAL_CALL
cclass_Unicode::getType( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException, std::exception) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;
    return (sal_Int16) u_charType(Text.iterateCodePoints(&nPos, 0));
}

sal_Int16 SAL_CALL
cclass_Unicode::getCharacterDirection( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException, std::exception) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;
    return (sal_Int16) u_charDirection(Text.iterateCodePoints(&nPos, 0));
}


sal_Int16 SAL_CALL
cclass_Unicode::getScript( const OUString& Text, sal_Int32 nPos ) throw(RuntimeException, std::exception) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;
    // ICU Unicode script type UBlockCode starts from 1 for Basic Latin,
    // while OO.o enum UnicideScript starts from 0.
    // To map ICU UBlockCode to OO.o UnicodeScript, it needs to shift 1.
    return (sal_Int16) ublock_getCode(Text.iterateCodePoints(&nPos, 0))-1;
}


sal_Int32 SAL_CALL
cclass_Unicode::getCharType( const OUString& Text, sal_Int32* nPos, sal_Int32 increment) {
    using namespace ::com::sun::star::i18n::KCharacterType;

    sal_uInt32 ch = Text.iterateCodePoints(nPos, increment);
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
cclass_Unicode::getCharacterType( const OUString& Text, sal_Int32 nPos, const Locale& /*rLocale*/ ) throw(RuntimeException, std::exception) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;
    return getCharType(Text, &nPos, 0);

}

sal_Int32 SAL_CALL
cclass_Unicode::getStringType( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount, const Locale& /*rLocale*/ ) throw(RuntimeException, std::exception) {
    if ( nPos < 0 || Text.getLength() <= nPos ) return 0;

    sal_Int32 result = 0;

    while (nCount > 0 && nPos < Text.getLength())
    {
        sal_Int32 nOrigPos = nPos;
        result |= getCharType(Text, &nPos, 1);
        sal_Int32 nUtf16Units = nPos - nOrigPos;
        nCount -= nUtf16Units;
    }

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
                throw(RuntimeException, std::exception)
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
                throw(RuntimeException, std::exception)
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

OUString SAL_CALL cclass_Unicode::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("com.sun.star.i18n.CharacterClassification_Unicode");
}

sal_Bool SAL_CALL cclass_Unicode::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL cclass_Unicode::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    aRet[0] = "com.sun.star.i18n.CharacterClassification_Unicode";
    return aRet;
}

} } } }

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_i18n_CharacterClassification_Unicode_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new css::i18n::cclass_Unicode(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
