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
#ifndef INCLUDED_I18NPOOL_INC_CCLASS_UNICODE_HXX
#define INCLUDED_I18NPOOL_INC_CCLASS_UNICODE_HXX

#include <com/sun/star/i18n/XNativeNumberSupplier.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/i18n/XLocaleData4.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <transliteration_body.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }


/// Flag values of table.
enum class ParserFlags : sal_uInt32  {
    ILLEGAL         = 0x00000000,
    CHAR            = 0x00000001,
    CHAR_BOOL       = 0x00000002,
    CHAR_WORD       = 0x00000004,
    CHAR_VALUE      = 0x00000008,
    CHAR_STRING     = 0x00000010,
    CHAR_DONTCARE   = 0x00000020,
    BOOL            = 0x00000040,
    WORD            = 0x00000080,
    WORD_SEP        = 0x00000100,
    VALUE           = 0x00000200,
    VALUE_SEP       = 0x00000400,
    VALUE_EXP       = 0x00000800,
    VALUE_SIGN      = 0x00001000,
    VALUE_EXP_VALUE = 0x00002000,
    VALUE_DIGIT     = 0x00004000,
    NAME_SEP        = 0x20000000,
    STRING_SEP      = 0x40000000,
    EXCLUDED        = 0x80000000,
};
namespace o3tl {
    template<> struct typed_flags<ParserFlags> : is_typed_flags<ParserFlags, 0xe0007fff> {};
}


namespace i18npool {

class cclass_Unicode : public cppu::WeakImplHelper < css::i18n::XCharacterClassification, css::lang::XServiceInfo >
{
public:
    cclass_Unicode(const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    virtual ~cclass_Unicode() override;

    virtual OUString SAL_CALL toUpper( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const css::lang::Locale& rLocale ) override;
    virtual OUString SAL_CALL toLower( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const css::lang::Locale& rLocale ) override;
    virtual OUString SAL_CALL toTitle( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const css::lang::Locale& rLocale ) override;
    virtual sal_Int16 SAL_CALL getType( const OUString& Text, sal_Int32 nPos ) override;
    virtual sal_Int16 SAL_CALL getCharacterDirection( const OUString& Text, sal_Int32 nPos ) override;
    virtual sal_Int16 SAL_CALL getScript( const OUString& Text, sal_Int32 nPos ) override;
    virtual sal_Int32 SAL_CALL getCharacterType( const OUString& text, sal_Int32 nPos,
        const css::lang::Locale& rLocale ) override;
    virtual sal_Int32 SAL_CALL getStringType( const OUString& text, sal_Int32 nPos, sal_Int32 nCount,
        const css::lang::Locale& rLocale ) override;
    virtual css::i18n::ParseResult SAL_CALL parseAnyToken( const OUString& Text, sal_Int32 nPos,
        const css::lang::Locale& rLocale, sal_Int32 nStartCharFlags, const OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags, const OUString& userDefinedCharactersCont ) override;
    virtual css::i18n::ParseResult SAL_CALL parsePredefinedToken( sal_Int32 nTokenType, const OUString& Text,
        sal_Int32 nPos, const css::lang::Locale& rLocale, sal_Int32 nStartCharFlags,
        const OUString& userDefinedCharactersStart, sal_Int32 nContCharFlags,
        const OUString& userDefinedCharactersCont ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    Transliteration_casemapping *trans;

// --- parser specific (implemented in cclass_unicode_parser.cxx) ---

    enum ScanState
    {
        ssGetChar, // initial state; -> ssBounce, ssGetValue, ssRewindFromValue, ssGetWord, ssGetWordFirstChar, ssGetString, ssGetBool, ssStop
        ssGetValue, // -> ssBounce, ssRewindFromValue, ssStopBack, ssGetWord
        ssGetWord, // -> ssBounce, ssStop, ssStopBack
        ssGetWordFirstChar, // -> ssBounce, ssGetWord, ssStop, ssStopBack
        ssGetString, // -> ssBounce, ssStop
        ssGetBool, // -> ssBounce, ssStop, ssStopBack
        ssRewindFromValue, // -> ssBounce, ssGetValue, ssGetWord, ssGetWordFirstChar, ssGetString, ssGetBool, ssStop, ssIgnoreLeadingInRewind
        ssIgnoreLeadingInRewind, // -> ssBounce, ssGetValue, ssRewindFromValue, ssGetWord, ssGetWordFirstChar, ssGetString, ssGetBool, ssStop
        ssStopBack, // -> ssStop
        ssBounce, // -> ssStopBack
        ssStop
    };

    static const sal_uInt8      nDefCnt;
    static const ParserFlags    pDefaultParserTable[];
    static const sal_Int32      pParseTokensType[];

    /// If and where c occurs in pStr
    static  const sal_Unicode*  StrChr( const sal_Unicode* pStr, sal_Unicode c );


    css::uno::Reference < css::uno::XComponentContext > m_xContext;

    /// used for parser only
    css::lang::Locale    aParserLocale;
    css::uno::Reference < css::i18n::XLocaleData4 > mxLocaleData;
    css::uno::Reference < css::i18n::XNativeNumberSupplier > xNatNumSup;
    OUString             aStartChars;
    OUString             aContChars;
    ParserFlags*         pTable;
    ParserFlags*         pStart;
    ParserFlags*         pCont;
    sal_Int32            nStartTypes;
    sal_Int32            nContTypes;
    ScanState            eState;
    sal_Unicode          cGroupSep;
    sal_Unicode          cDecimalSep;

    /// Get corresponding KParseTokens flag for a character
    static sal_Int32 getParseTokensType(sal_uInt32 c, bool isFirst);

    /// Access parser table flags.
    ParserFlags getFlags(sal_uInt32 c);

    /// Access parser flags via International and special definitions.
    ParserFlags getFlagsExtended(sal_uInt32 c);

    /// Access parser table flags for user defined start characters.
    ParserFlags getStartCharsFlags( sal_Unicode c );

    /// Access parser table flags for user defined continuation characters.
    ParserFlags getContCharsFlags( sal_Unicode c );

    /// Setup parser table. Calls initParserTable() only if needed.
    void setupParserTable( const css::lang::Locale& rLocale, sal_Int32 startCharTokenType,
        const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const OUString& userDefinedCharactersCont );

    /// Init parser table.
    void initParserTable( const css::lang::Locale& rLocale, sal_Int32 startCharTokenType,
        const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const OUString& userDefinedCharactersCont );

    /// Destroy parser table.
    void destroyParserTable();

    /// Parse a text.
    void parseText( css::i18n::ParseResult& r, const OUString& rText, sal_Int32 nPos,
        sal_Int32 nTokenType = 0xffffffff );

    /// Setup International class, new'ed only if different from existing.
    bool setupInternational( const css::lang::Locale& rLocale );

    /// Implementation of getCharacterType() for one single character
    static sal_Int32 SAL_CALL getCharType( const OUString& Text, sal_Int32 *nPos, sal_Int32 increment);

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
