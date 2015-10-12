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

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace com { namespace sun { namespace star { namespace i18n {

typedef sal_uInt32 UPT_FLAG_TYPE;

class cclass_Unicode : public cppu::WeakImplHelper < XCharacterClassification, css::lang::XServiceInfo >
{
public:
    cclass_Unicode(const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~cclass_Unicode();

    virtual OUString SAL_CALL toUpper( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL toLower( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL toTitle( const OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getType( const OUString& Text, sal_Int32 nPos )  throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getCharacterDirection( const OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getScript( const OUString& Text, sal_Int32 nPos ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getCharacterType( const OUString& text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getStringType( const OUString& text, sal_Int32 nPos, sal_Int32 nCount,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ParseResult SAL_CALL parseAnyToken( const OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int32 nStartCharFlags, const OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags, const OUString& userDefinedCharactersCont ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ParseResult SAL_CALL parsePredefinedToken( sal_Int32 nTokenType, const OUString& Text,
        sal_Int32 nPos, const com::sun::star::lang::Locale& rLocale, sal_Int32 nStartCharFlags,
        const OUString& userDefinedCharactersStart, sal_Int32 nContCharFlags,
        const OUString& userDefinedCharactersCont ) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException, std::exception ) override;

private:
    Transliteration_casemapping *trans;

// --- parser specific (implemented in cclass_unicode_parser.cxx) ---

    enum ScanState
    {
        ssGetChar,
        ssGetValue,
        ssGetWord,
        ssGetWordFirstChar,
        ssGetString,
        ssGetBool,
        ssRewindFromValue,
        ssIgnoreLeadingInRewind,
        ssStopBack,
        ssBounce,
        ssStop
    };

    static const sal_uInt8      nDefCnt;
    static const UPT_FLAG_TYPE  pDefaultParserTable[];
    static const sal_Int32      pParseTokensType[];

    /// Flag values of table.
    static const UPT_FLAG_TYPE  TOKEN_ILLEGAL;
    static const UPT_FLAG_TYPE  TOKEN_CHAR;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_BOOL;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_WORD;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_VALUE;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_STRING;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_DONTCARE;
    static const UPT_FLAG_TYPE  TOKEN_BOOL;
    static const UPT_FLAG_TYPE  TOKEN_WORD;
    static const UPT_FLAG_TYPE  TOKEN_WORD_SEP;
    static const UPT_FLAG_TYPE  TOKEN_VALUE;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_SEP;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_EXP;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_SIGN;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_EXP_VALUE;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_DIGIT;
    static const UPT_FLAG_TYPE  TOKEN_NAME_SEP;
    static const UPT_FLAG_TYPE  TOKEN_STRING_SEP;
    static const UPT_FLAG_TYPE  TOKEN_EXCLUDED;

    /// If and where c occurs in pStr
    static  const sal_Unicode*  StrChr( const sal_Unicode* pStr, sal_Unicode c );


    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_xContext;

    /// used for parser only
    com::sun::star::lang::Locale    aParserLocale;
    com::sun::star::uno::Reference < XLocaleData4 > mxLocaleData;
    com::sun::star::uno::Reference < com::sun::star::i18n::XNativeNumberSupplier > xNatNumSup;
    OUString             aStartChars;
    OUString             aContChars;
    UPT_FLAG_TYPE*              pTable;
    UPT_FLAG_TYPE*              pStart;
    UPT_FLAG_TYPE*              pCont;
    sal_Int32                   nStartTypes;
    sal_Int32                   nContTypes;
    ScanState                   eState;
    sal_Unicode                 cGroupSep;
    sal_Unicode                 cDecimalSep;

    /// Get corresponding KParseTokens flag for a character
    static sal_Int32 getParseTokensType( const sal_Unicode* aStr, sal_Int32 nPos );

    /// Access parser table flags.
    UPT_FLAG_TYPE getFlags( const sal_Unicode* aStr, sal_Int32 nPos );

    /// Access parser flags via International and special definitions.
    UPT_FLAG_TYPE getFlagsExtended( const sal_Unicode* aStr, sal_Int32 nPos );

    /// Access parser table flags for user defined start characters.
    UPT_FLAG_TYPE getStartCharsFlags( sal_Unicode c );

    /// Access parser table flags for user defined continuation characters.
    UPT_FLAG_TYPE getContCharsFlags( sal_Unicode c );

    /// Setup parser table. Calls initParserTable() only if needed.
    void setupParserTable( const com::sun::star::lang::Locale& rLocale, sal_Int32 startCharTokenType,
        const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const OUString& userDefinedCharactersCont );

    /// Init parser table.
    void initParserTable( const com::sun::star::lang::Locale& rLocale, sal_Int32 startCharTokenType,
        const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const OUString& userDefinedCharactersCont );

    /// Destroy parser table.
    void destroyParserTable();

    /// Parse a text.
    void parseText( ParseResult& r, const OUString& rText, sal_Int32 nPos,
        sal_Int32 nTokenType = 0xffffffff );

    /// Setup International class, new'ed only if different from existing.
    bool setupInternational( const com::sun::star::lang::Locale& rLocale );

    /// Implementation of getCharacterType() for one single character
    static sal_Int32 SAL_CALL getCharType( const OUString& Text, sal_Int32 *nPos, sal_Int32 increment);

};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
