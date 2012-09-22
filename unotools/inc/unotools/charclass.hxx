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
#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_CHARCLASS_HXX
#define _UNOTOOLS_CHARCLASS_HXX

#include <ctype.h>      // isdigit(), isalpha()
#include <boost/noncopyable.hpp>
#include <tools/string.hxx>
#include <tools/solar.h>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>
#include <com/sun/star/i18n/ParseResult.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <osl/mutex.hxx>

class String;
namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

const sal_Int32 nCharClassAlphaType =
    ::com::sun::star::i18n::KCharacterType::UPPER |
    ::com::sun::star::i18n::KCharacterType::LOWER |
    ::com::sun::star::i18n::KCharacterType::TITLE_CASE;

const sal_Int32 nCharClassAlphaTypeMask =
    nCharClassAlphaType |
    ::com::sun::star::i18n::KCharacterType::PRINTABLE |
    ::com::sun::star::i18n::KCharacterType::BASE_FORM;

const sal_Int32 nCharClassLetterType =
    nCharClassAlphaType |
    ::com::sun::star::i18n::KCharacterType::LETTER;

const sal_Int32 nCharClassLetterTypeMask =
    nCharClassAlphaTypeMask |
    ::com::sun::star::i18n::KCharacterType::LETTER;

const sal_Int32 nCharClassNumericType =
    ::com::sun::star::i18n::KCharacterType::DIGIT;

const sal_Int32 nCharClassNumericTypeMask =
    nCharClassNumericType |
    ::com::sun::star::i18n::KCharacterType::PRINTABLE |
    ::com::sun::star::i18n::KCharacterType::BASE_FORM;


class UNOTOOLS_DLLPUBLIC CharClass : private boost::noncopyable
{
    ::com::sun::star::lang::Locale  aLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification >    xCC;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    mutable ::osl::Mutex        aMutex;

public:
    /// Preferred ctor with service manager specified
    CharClass(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
        const ::com::sun::star::lang::Locale& rLocale);

    /// Depricated ctor, tries to get a process service manager or to load the
    /// library directly.
    CharClass(const ::com::sun::star::lang::Locale& rLocale);

    ~CharClass();

    /// set a new Locale
    void setLocale( const ::com::sun::star::lang::Locale& rLocale );

    /// get current Locale
    const ::com::sun::star::lang::Locale& getLocale() const;


    /// isdigit() on ascii values
    static inline sal_Bool isAsciiDigit( sal_Unicode c )
    {
        return c < 128 ? sal_Bool(isdigit( (unsigned char) c ) != 0) : sal_False;
    }

    /// isalpha() on ascii values
    static inline sal_Bool isAsciiAlpha( sal_Unicode c )
    {
        return c < 128 ? sal_Bool(isalpha( (unsigned char) c ) != 0) : sal_False;
    }

    /// isalnum() on ascii values
    static inline sal_Bool isAsciiAlphaNumeric( sal_Unicode c )
    {
        return c < 128 ? sal_Bool(isalnum( (unsigned char) c ) != 0) : sal_False;
    }

    /// isdigit() on ascii values of entire string
    static sal_Bool isAsciiNumeric( const String& rStr );

    /// isalpha() on ascii values of entire string
    static sal_Bool isAsciiAlpha( const String& rStr );

    /// isalnum() on ascii values of entire string
    static sal_Bool isAsciiAlphaNumeric( const String& rStr );

    /// whether type is pure alpha or not, e.g. return of getStringType
    static inline sal_Bool isAlphaType( sal_Int32 nType )
    {
        return ((nType & nCharClassAlphaType) != 0) &&
            ((nType & ~(nCharClassAlphaTypeMask)) == 0);
    }

    /// whether type is pure numeric or not, e.g. return of getStringType
    static inline sal_Bool isNumericType( sal_Int32 nType )
    {
        return ((nType & nCharClassNumericType) != 0) &&
            ((nType & ~(nCharClassNumericTypeMask)) == 0);
    }

    /// whether type is pure alphanumeric or not, e.g. return of getStringType
    static inline sal_Bool isAlphaNumericType( sal_Int32 nType )
    {
        return ((nType & (nCharClassAlphaType |
            nCharClassNumericType)) != 0) &&
            ((nType & ~(nCharClassAlphaTypeMask |
            nCharClassNumericTypeMask)) == 0);
    }

    /// whether type is pure letter or not, e.g. return of getStringType
    static inline sal_Bool isLetterType( sal_Int32 nType )
    {
        return ((nType & nCharClassLetterType) != 0) &&
            ((nType & ~(nCharClassLetterTypeMask)) == 0);
    }

    /// whether type is pure letternumeric or not, e.g. return of getStringType
    static inline sal_Bool isLetterNumericType( sal_Int32 nType )
    {
        return ((nType & (nCharClassLetterType |
            nCharClassNumericType)) != 0) &&
            ((nType & ~(nCharClassLetterTypeMask |
            nCharClassNumericTypeMask)) == 0);
    }


    // Wrapper implementations of class CharacterClassification

    OUString uppercase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const;
    OUString lowercase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const;
    OUString titlecase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const;

    OUString uppercase( const OUString& _rStr ) const
    {
        return uppercase(_rStr, 0, _rStr.getLength());
    }
    OUString lowercase( const OUString& _rStr ) const
    {
        return lowercase(_rStr, 0, _rStr.getLength());
    }
    OUString titlecase( const OUString& _rStr ) const
    {
        return titlecase(_rStr, 0, _rStr.getLength());
    }

    sal_Int16 getType( const String& rStr, xub_StrLen nPos ) const;
    sal_Int16 getCharacterDirection( const String& rStr, xub_StrLen nPos ) const;
    sal_Int16 getScript( const String& rStr, xub_StrLen nPos ) const;
    sal_Int32 getCharacterType( const String& rStr, xub_StrLen nPos ) const;
    sal_Int32 getStringType( const String& rStr, xub_StrLen nPos, xub_StrLen nCount ) const;

    ::com::sun::star::i18n::ParseResult parseAnyToken(
                                    const String& rStr,
                                    sal_Int32 nPos,
                                    sal_Int32 nStartCharFlags,
                                    const String& userDefinedCharactersStart,
                                    sal_Int32 nContCharFlags,
                                    const String& userDefinedCharactersCont ) const;

    ::com::sun::star::i18n::ParseResult parsePredefinedToken(
                                    sal_Int32 nTokenType,
                                    const String& rStr,
                                    sal_Int32 nPos,
                                    sal_Int32 nStartCharFlags,
                                    const String& userDefinedCharactersStart,
                                    sal_Int32 nContCharFlags,
                                    const String& userDefinedCharactersCont ) const;


    // Functionality of class International methods

    sal_Bool isAlpha( const String& rStr, xub_StrLen nPos ) const;
    sal_Bool isLetter( const String& rStr, xub_StrLen nPos ) const;
    sal_Bool isDigit( const String& rStr, xub_StrLen nPos ) const;
    sal_Bool isAlphaNumeric( const String& rStr, xub_StrLen nPos ) const;
    sal_Bool isLetterNumeric( const String& rStr, xub_StrLen nPos ) const;
    sal_Bool isAlpha( const String& rStr ) const;
    sal_Bool isLetter( const String& rStr ) const;
    sal_Bool isNumeric( const String& rStr ) const;
    sal_Bool isAlphaNumeric( const String& rStr ) const;
    sal_Bool isLetterNumeric( const String& rStr ) const;
};

#endif // _UNOTOOLS_CHARCLASS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
