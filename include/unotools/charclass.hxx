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

#include <boost/noncopyable.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <tools/solar.h>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>
#include <com/sun/star/i18n/ParseResult.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <osl/mutex.hxx>
#include <rtl/character.hxx>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
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
    LanguageTag                 maLanguageTag;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification >    xCC;
    mutable ::osl::Mutex        aMutex;

public:
    /// Preferred ctor with service manager specified
    CharClass(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rxContext,
        const LanguageTag& rLanguageTag );

    /// Depricated ctor, tries to get a process service manager or to load the
    /// library directly.
    CharClass( const LanguageTag& rLanguageTag );

    ~CharClass();

    /// set a new Locale
    void setLanguageTag( const LanguageTag& rLanguageTag );

    /// get current Locale
    const LanguageTag& getLanguageTag() const;


    /// isdigit() on ascii values
    SAL_DEPRECATED("Use rtl::isAsciiDigit instead")
    static inline bool isAsciiDigit( sal_Unicode c )
    {
        return rtl::isAsciiDigit( c );
    }

    /// isalpha() on ascii values
    SAL_DEPRECATED("Use rtl::isAsciiAlpha instead")
    static inline bool isAsciiAlpha( sal_Unicode c )
    {
        return rtl::isAsciiAlpha( c );
    }

    /// isalnum() on ascii values
    SAL_DEPRECATED("Use rtl::isAsciiAlphanumeric instead")
    static inline bool isAsciiAlphaNumeric( sal_Unicode c )
    {
        return rtl::isAsciiAlphanumeric( c );
    }

    /// isdigit() on ascii values of entire string
    static bool isAsciiNumeric( const OUString& rStr );

    /// isalpha() on ascii values of entire string
    static bool isAsciiAlpha( const OUString& rStr );

    /// isalnum() on ascii values of entire string
    static bool isAsciiAlphaNumeric( const OUString& rStr );

    /// whether type is pure alpha or not, e.g. return of getStringType
    static inline bool isAlphaType( sal_Int32 nType )
    {
        return ((nType & nCharClassAlphaType) != 0) &&
            ((nType & ~(nCharClassAlphaTypeMask)) == 0);
    }

    /// whether type is pure numeric or not, e.g. return of getStringType
    static inline bool isNumericType( sal_Int32 nType )
    {
        return ((nType & nCharClassNumericType) != 0) &&
            ((nType & ~(nCharClassNumericTypeMask)) == 0);
    }

    /// whether type is pure alphanumeric or not, e.g. return of getStringType
    static inline bool isAlphaNumericType( sal_Int32 nType )
    {
        return ((nType & (nCharClassAlphaType |
            nCharClassNumericType)) != 0) &&
            ((nType & ~(nCharClassAlphaTypeMask |
            nCharClassNumericTypeMask)) == 0);
    }

    /// whether type is pure letter or not, e.g. return of getStringType
    static inline bool isLetterType( sal_Int32 nType )
    {
        return ((nType & nCharClassLetterType) != 0) &&
            ((nType & ~(nCharClassLetterTypeMask)) == 0);
    }

    /// whether type is pure letternumeric or not, e.g. return of getStringType
    static inline bool isLetterNumericType( sal_Int32 nType )
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

    sal_Int16 getType( const OUString& rStr, sal_Int32 nPos ) const;
    sal_Int16 getCharacterDirection( const OUString& rStr, sal_Int32 nPos ) const;
    sal_Int16 getScript( const OUString& rStr, sal_Int32 nPos ) const;
    sal_Int32 getCharacterType( const OUString& rStr, sal_Int32 nPos ) const;
    sal_Int32 getStringType( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const;

    ::com::sun::star::i18n::ParseResult parseAnyToken(
                                    const OUString& rStr,
                                    sal_Int32 nPos,
                                    sal_Int32 nStartCharFlags,
                                    const OUString& userDefinedCharactersStart,
                                    sal_Int32 nContCharFlags,
                                    const OUString& userDefinedCharactersCont ) const;

    ::com::sun::star::i18n::ParseResult parsePredefinedToken(
                                    sal_Int32 nTokenType,
                                    const OUString& rStr,
                                    sal_Int32 nPos,
                                    sal_Int32 nStartCharFlags,
                                    const OUString& userDefinedCharactersStart,
                                    sal_Int32 nContCharFlags,
                                    const OUString& userDefinedCharactersCont ) const;


    // Functionality of class International methods

    bool isAlpha( const OUString& rStr, sal_Int32 nPos ) const;
    bool isLetter( const OUString& rStr, sal_Int32 nPos ) const;
    bool isDigit( const OUString& rStr, sal_Int32 nPos ) const;
    bool isAlphaNumeric( const OUString& rStr, sal_Int32 nPos ) const;
    bool isLetterNumeric( const OUString& rStr, sal_Int32 nPos ) const;
    bool isAlpha( const OUString& rStr ) const;
    bool isLetter( const OUString& rStr ) const;
    bool isNumeric( const OUString& rStr ) const;
    bool isAlphaNumeric( const OUString& rStr ) const;
    bool isLetterNumeric( const OUString& rStr ) const;

private:

    const ::com::sun::star::lang::Locale &  getMyLocale() const;
};

#endif // _UNOTOOLS_CHARCLASS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
