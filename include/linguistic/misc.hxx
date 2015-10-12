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
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/linguistic2/XDictionaryEntry.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>

#include <cppuhelper/implbase.hxx>
#include <unotools/pathoptions.hxx>
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <unotools/charclass.hxx>
#include <osl/thread.h>
#include <osl/mutex.hxx>
#include <linguistic/lngdllapi.h>

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertySet;
    class XFastPropertySet;
}}}}

namespace com { namespace sun { namespace star { namespace frame {
    class XDesktop2;
}}}}

class LocaleDataWrapper;


#define SN_GRAMMARCHECKER           "com.sun.star.linguistic2.Proofreader"
#define SN_SPELLCHECKER             "com.sun.star.linguistic2.SpellChecker"
#define SN_HYPHENATOR               "com.sun.star.linguistic2.Hyphenator"
#define SN_THESAURUS                "com.sun.star.linguistic2.Thesaurus"


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

LocaleDataWrapper & GetLocaleDataWrapper( sal_Int16 nLang );

sal_Int32 LevDistance( const OUString &rTxt1, const OUString &rTxt2 );

/** Convert Locale to LanguageType for legacy handling.
    Linguistic specific handling of an empty locale denoting LANGUAGE_NONE.
    Does not resolve empty locale as system locale.
 */
LNG_DLLPUBLIC LanguageType LinguLocaleToLanguage( const ::com::sun::star::lang::Locale& rLocale );

/** Convert LanguageType to Locale for legacy handling.
    Linguistic specific handling of LANGUAGE_NONE resulting in an empty locale.
    Avoid use!
 */
LNG_DLLPUBLIC ::com::sun::star::lang::Locale LinguLanguageToLocale( LanguageType nLanguage );

/** Checks if a LanguageType is one of the values that denote absence of
    language or undetermined language or multiple languages, in short all
    values used in linguistic context that do not denote a specific language.
 */
LNG_DLLPUBLIC bool LinguIsUnspecified( LanguageType nLanguage );

/** The same as LinguIsUnspecified(LanguageType) but taking a BCP 47 language
    tag string instead. */
LNG_DLLPUBLIC bool LinguIsUnspecified( const OUString & rBcp47 );

::com::sun::star::uno::Sequence< sal_Int16 >
    LocaleSeqToLangSeq( ::com::sun::star::uno::Sequence<
        ::com::sun::star::lang::Locale > &rLocaleSeq );

// checks if file pointed to by rURL is readonly
// and may also check return if such a file exists or not
bool    IsReadOnly( const OUString &rURL, bool *pbExist = 0 );

// checks if a file with the given URL exists
bool    FileExists( const OUString &rURL );


OUString     GetDictionaryWriteablePath();
::com::sun::star::uno::Sequence< OUString > GetDictionaryPaths();

/// @returns an URL for a new and writable dictionary rDicName.
///     The URL will point to the path given by 'GetDictionaryWriteablePath'
LNG_DLLPUBLIC OUString  GetWritableDictionaryURL( const OUString &rDicName );

LNG_DLLPUBLIC sal_Int32 GetPosInWordToCheck( const OUString &rTxt, sal_Int32 nPos );

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XHyphenatedWord >
            RebuildHyphensAndControlChars( const OUString &rOrigWord,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XHyphenatedWord > &rxHyphWord );


LNG_DLLPUBLIC bool        IsUpper( const OUString &rText, sal_Int32 nPos, sal_Int32 nLen, sal_Int16 nLanguage );

inline bool        IsUpper( const OUString &rText, sal_Int16 nLanguage )     { return IsUpper( rText, 0, rText.getLength(), nLanguage ); }
LNG_DLLPUBLIC CapType SAL_CALL capitalType(const OUString&, CharClass *);

OUString      ToLower( const OUString &rText, sal_Int16 nLanguage );
LNG_DLLPUBLIC bool      HasDigits( const OUString &rText );
LNG_DLLPUBLIC bool      IsNumeric( const OUString &rText );


LNG_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLinguProperties > GetLinguProperties();
::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSearchableDictionaryList > GetDictionaryList();
::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > GetIgnoreAllList();


bool IsUseDicList( const ::com::sun::star::beans::PropertyValues &rProperties,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > &rxPropSet );

bool IsIgnoreControlChars( const ::com::sun::star::beans::PropertyValues &rProperties,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > &rxPropSet );

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionaryEntry >
        SearchDicList(
            const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSearchableDictionaryList >& rDicList,
            const OUString& rWord, sal_Int16 nLanguage,
            bool bSearchPosDics, bool bSearchSpellEntry );

LNG_DLLPUBLIC DictionaryError AddEntryToDic(
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary >  &rxDic,
    const OUString &rWord, bool bIsNeg,
    const OUString &rRplcTxt, sal_Int16 nRplcLang,
    bool bStripDot = true );

LNG_DLLPUBLIC bool SaveDictionaries( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSearchableDictionaryList > &xDicList );

// AppExitLstnr:
// virtual base class that calls it AtExit function when the application
// (ie the Desktop) is about to terminate

class AppExitListener :
    public cppu::WeakImplHelper
    <
        ::com::sun::star::frame::XTerminateListener
    >
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDesktop2 >     xDesktop;

public:
    AppExitListener();
    virtual ~AppExitListener();

    virtual void    AtExit() = 0;

    void            Activate();
    void            Deactivate();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

}   // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
