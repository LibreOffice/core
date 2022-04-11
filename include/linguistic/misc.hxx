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

#ifndef INCLUDED_LINGUISTIC_MISC_HXX
#define INCLUDED_LINGUISTIC_MISC_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <cppuhelper/implbase.hxx>
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <linguistic/lngdllapi.h>

#include <vector>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::frame { class XDesktop2; }
namespace com::sun::star::linguistic2 { class XDictionary; }
namespace com::sun::star::linguistic2 { class XDictionaryEntry; }
namespace com::sun::star::linguistic2 { class XHyphenatedWord; }
namespace com::sun::star::linguistic2 { class XLinguProperties; }
namespace com::sun::star::linguistic2 { class XSearchableDictionaryList; }
namespace osl { class Mutex; }

class CharClass;
class LocaleDataWrapper;


inline constexpr OUStringLiteral SN_GRAMMARCHECKER = u"com.sun.star.linguistic2.Proofreader";
inline constexpr OUStringLiteral SN_SPELLCHECKER = u"com.sun.star.linguistic2.SpellChecker";
inline constexpr OUStringLiteral SN_HYPHENATOR = u"com.sun.star.linguistic2.Hyphenator";
inline constexpr OUStringLiteral SN_THESAURUS = u"com.sun.star.linguistic2.Thesaurus";


namespace linguistic
{


// AddEntryToDic return values
enum class DictionaryError
{
    NONE, FULL, READONLY, UNKNOWN, NOT_EXISTS
};

// values assigned to capitalization types
enum class CapType
{
    UNKNOWN,
    NOCAP,
    INITCAP,
    ALLCAP,
    MIXED
};

LNG_DLLPUBLIC ::osl::Mutex& GetLinguMutex();

const LocaleDataWrapper & GetLocaleDataWrapper( LanguageType nLang );

sal_Int32 LevDistance( std::u16string_view rTxt1, std::u16string_view rTxt2 );

/** Convert Locale to LanguageType for legacy handling.
    Linguistic specific handling of an empty locale denoting LANGUAGE_NONE.
    Does not resolve empty locale as system locale.
 */
LNG_DLLPUBLIC LanguageType LinguLocaleToLanguage( const css::lang::Locale& rLocale );

/** Convert LanguageType to Locale for legacy handling.
    Linguistic specific handling of LANGUAGE_NONE resulting in an empty locale.
    Avoid use!
 */
LNG_DLLPUBLIC css::lang::Locale LinguLanguageToLocale( LanguageType nLanguage );

/** Checks if a LanguageType is one of the values that denote absence of
    language or undetermined language or multiple languages, in short all
    values used in linguistic context that do not denote a specific language.
 */
LNG_DLLPUBLIC bool LinguIsUnspecified( LanguageType nLanguage );

/** The same as LinguIsUnspecified(LanguageType) but taking a BCP 47 language
    tag string instead. */
LNG_DLLPUBLIC bool LinguIsUnspecified( std::u16string_view rBcp47 );

std::vector< LanguageType >
    LocaleSeqToLangVec( css::uno::Sequence< css::lang::Locale > const &rLocaleSeq );
css::uno::Sequence<sal_Int16>
    LocaleSeqToLangSeq( css::uno::Sequence< css::lang::Locale > const &rLocaleSeq );

// checks if file pointed to by rURL is readonly
// and may also check return if such a file exists or not
bool    IsReadOnly( const OUString &rURL, bool *pbExist = nullptr );

// checks if a file with the given URL exists
bool    FileExists( const OUString &rURL );


OUString     GetDictionaryWriteablePath();
std::vector< OUString > GetDictionaryPaths();

/// @returns a URL for a new and writable dictionary rDicName.
///     The URL will point to the path given by 'GetDictionaryWriteablePath'
LNG_DLLPUBLIC OUString  GetWritableDictionaryURL( std::u16string_view rDicName );

LNG_DLLPUBLIC sal_Int32 GetPosInWordToCheck( std::u16string_view rTxt, sal_Int32 nPos );

css::uno::Reference< css::linguistic2::XHyphenatedWord >
            RebuildHyphensAndControlChars(
                const OUString &rOrigWord,
                css::uno::Reference< css::linguistic2::XHyphenatedWord > const &rxHyphWord );


LNG_DLLPUBLIC bool        IsUpper( const OUString &rText, sal_Int32 nPos, sal_Int32 nLen, LanguageType nLanguage );

inline bool        IsUpper( const OUString &rText, LanguageType nLanguage )     { return IsUpper( rText, 0, rText.getLength(), nLanguage ); }
LNG_DLLPUBLIC CapType capitalType(const OUString&, CharClass const *);

LNG_DLLPUBLIC bool      HasDigits( const OUString &rText );
LNG_DLLPUBLIC bool      IsNumeric( std::u16string_view rText );


LNG_DLLPUBLIC css::uno::Reference< css::linguistic2::XLinguProperties > GetLinguProperties();
css::uno::Reference< css::linguistic2::XSearchableDictionaryList > GetDictionaryList();
css::uno::Reference< css::linguistic2::XDictionary > GetIgnoreAllList();


bool IsUseDicList( const css::beans::PropertyValues &rProperties,
        const css::uno::Reference< css::beans::XPropertySet > &rxPropSet );

bool IsIgnoreControlChars( const css::beans::PropertyValues &rProperties,
        const css::uno::Reference< css::beans::XPropertySet > &rxPropSet );

css::uno::Reference<
    css::linguistic2::XDictionaryEntry >
        SearchDicList(
            const css::uno::Reference< css::linguistic2::XSearchableDictionaryList >& rDicList,
            const OUString& rWord, LanguageType nLanguage,
            bool bSearchPosDics, bool bSearchSpellEntry );

LNG_DLLPUBLIC DictionaryError AddEntryToDic(
    css::uno::Reference< css::linguistic2::XDictionary > const &rxDic,
    const OUString &rWord, bool bIsNeg,
    const OUString &rRplcTxt,
    bool bStripDot = true );

LNG_DLLPUBLIC bool SaveDictionaries( const css::uno::Reference< css::linguistic2::XSearchableDictionaryList > &xDicList );

// AppExitLstnr:
// virtual base class that calls it AtExit function when the application
// (ie the Desktop) is about to terminate

class AppExitListener :
    public cppu::WeakImplHelper< css::frame::XTerminateListener >
{
    css::uno::Reference< css::frame::XDesktop2 >     xDesktop;

public:
    AppExitListener();
    virtual ~AppExitListener() override;

    virtual void    AtExit() = 0;

    void            Activate();
    void            Deactivate();

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const css::lang::EventObject& aEvent ) override;
    virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent ) override;
};

}   // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
