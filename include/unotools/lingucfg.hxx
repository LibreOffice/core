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

#ifndef INCLUDED_UNOTOOLS_LINGUCFG_HXX
#define INCLUDED_UNOTOOLS_LINGUCFG_HXX

#include <unotools/unotoolsdllapi.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <osl/mutex.hxx>
#include <unotools/options.hxx>
#include <i18nlangtag/lang.h>
#include <vector>

class SvtLinguConfigItem;

struct UNOTOOLS_DLLPUBLIC SvtLinguOptions
{
    css::uno::Sequence< OUString >    aActiveDics;
    css::uno::Sequence< OUString >    aActiveConvDics;

    bool                              bROActiveDics;
    bool                              bROActiveConvDics;

    // Hyphenator service specific options
    sal_Int16   nHyphMinLeading,
            nHyphMinTrailing,
            nHyphMinWordLength;

    bool    bROHyphMinLeading,
            bROHyphMinTrailing,
            bROHyphMinWordLength;

    // misc options (non-service specific)
    sal_Int16   nDefaultLanguage;
    sal_Int16   nDefaultLanguage_CJK;
    sal_Int16   nDefaultLanguage_CTL;

    bool    bRODefaultLanguage;
    bool    bRODefaultLanguage_CJK;
    bool    bRODefaultLanguage_CTL;

    // spelling options (non-service specific)
    bool    bIsSpellSpecial;
    bool    bIsSpellAuto;
    bool    bIsSpellReverse;

    bool    bROIsSpellSpecial;
    bool    bROIsSpellAuto;
    bool    bROIsSpellReverse;

    // hyphenation options (non-service specific)
    bool    bIsHyphSpecial;
    bool    bIsHyphAuto;

    bool    bROIsHyphSpecial;
    bool    bROIsHyphAuto;

    // common to SpellChecker, Hyphenator and Thesaurus service
    bool    bIsUseDictionaryList;
    bool    bIsIgnoreControlCharacters;

    bool    bROIsUseDictionaryList;
    bool    bROIsIgnoreControlCharacters;

    // SpellChecker service specific options
    bool    bIsSpellWithDigits,
            bIsSpellUpperCase,
            bIsSpellCapitalization;

    bool    bROIsSpellWithDigits,
            bROIsSpellUpperCase,
            bROIsSpellCapitalization;

    // text conversion specific options
    bool    bIsIgnorePostPositionalWord;
    bool    bIsAutoCloseDialog;
    bool    bIsShowEntriesRecentlyUsedFirst;
    bool    bIsAutoReplaceUniqueEntries;
    bool    bIsDirectionToSimplified;
    bool    bIsUseCharacterVariants;
    bool    bIsTranslateCommonTerms;
    bool    bIsReverseMapping;

    bool    bROIsIgnorePostPositionalWord;
    bool    bROIsAutoCloseDialog;
    bool    bROIsShowEntriesRecentlyUsedFirst;
    bool    bROIsAutoReplaceUniqueEntries;
    bool    bROIsDirectionToSimplified;
    bool    bROIsUseCharacterVariants;
    bool    bROIsTranslateCommonTerms;
    bool    bROIsReverseMapping;

    // check value need to determine if the configuration needs to updatet
    // or not (used for a quick check if data files have been changed/added
    // or deleted
    sal_Int32   nDataFilesChangedCheckValue;
    bool    bRODataFilesChangedCheckValue;

    bool    bIsGrammarAuto;
    bool    bIsGrammarInteractive;

    bool    bROIsGrammarAuto;
    bool    bROIsGrammarInteractive;

    SvtLinguOptions();
};

struct UNOTOOLS_DLLPUBLIC SvtLinguConfigDictionaryEntry
{
    // the URL's pointing to the location of the files the dictionary consists of
    css::uno::Sequence< OUString >  aLocations;
    // the name of the dictionary format implement
    OUString                                   aFormatName;
    // the list of languages (ISO names) the dictionary can be used for
    css::uno::Sequence< OUString >  aLocaleNames;
};

class UNOTOOLS_DLLPUBLIC SvtLinguConfig: public utl::detail::Options
{
    // returns static object
    UNOTOOLS_DLLPRIVATE SvtLinguConfigItem &   GetConfigItem();

    SvtLinguConfigItem &   GetConfigItem() const    { return const_cast< SvtLinguConfig * >( this )->GetConfigItem(); }

    // configuration update access for the 'Linguistic' main node
    mutable css::uno::Reference< css::util::XChangesBatch > m_xMainUpdateAccess;

    css::uno::Reference< css::util::XChangesBatch > const & GetMainUpdateAccess() const;

    OUString GetVendorImageUrl_Impl( const OUString &rServiceImplName, const OUString &rImageName ) const;

    SvtLinguConfig( const SvtLinguConfig & ) = delete;
    SvtLinguConfig & operator = ( const SvtLinguConfig & ) = delete;

public:
    SvtLinguConfig();
    virtual ~SvtLinguConfig();

    // borrowed from utl::ConfigItem

    css::uno::Sequence< OUString >
        GetNodeNames( const OUString &rNode );

    css::uno::Sequence< css::uno::Any >
        GetProperties(
            const css::uno::Sequence< OUString > &rNames );

    bool
        ReplaceSetProperties(
            const OUString &rNode,
            const css::uno::Sequence< css::beans::PropertyValue >& rValues );

    css::uno::Any
            GetProperty( const OUString &rPropertyName ) const;
    css::uno::Any
            GetProperty( sal_Int32 nPropertyHandle ) const;

    bool    SetProperty( const OUString &rPropertyName,
                         const css::uno::Any &rValue );
    bool    SetProperty( sal_Int32 nPropertyHandle,
                         const css::uno::Any &rValue );

    bool    GetOptions( SvtLinguOptions &rOptions ) const;

    bool    IsReadOnly( const OUString &rPropertyName ) const;

    //!
    //! the following functions work on the 'ServiceManager' sub node of the
    //! linguistic configuration only
    //!
    bool GetElementNamesFor( const OUString &rNodeName, css::uno::Sequence< OUString > &rElementNames ) const;

    bool GetSupportedDictionaryFormatsFor( const OUString &rSetName, const OUString &rSetEntry, css::uno::Sequence< OUString > &rFormatList ) const;

    bool GetDictionaryEntry( const OUString &rNodeName, SvtLinguConfigDictionaryEntry &rDicEntry ) const;

    css::uno::Sequence< OUString > GetDisabledDictionaries() const;

    std::vector< SvtLinguConfigDictionaryEntry > GetActiveDictionariesByFormat( const OUString &rFormatName );

    // functions returning file URLs to the respective images (if found) and empty string otherwise
    OUString     GetSpellAndGrammarContextSuggestionImage( const OUString &rServiceImplName ) const;
    OUString     GetSpellAndGrammarContextDictionaryImage( const OUString &rServiceImplName ) const;
    OUString     GetSynonymsContextImage( const OUString &rServiceImplName ) const;

    bool                HasGrammarChecker() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
