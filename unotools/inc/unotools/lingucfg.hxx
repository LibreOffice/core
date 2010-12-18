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

#ifndef _unotools_LINGUCFG_HXX_
#define _unotools_LINGUCFG_HXX_

#include "unotools/unotoolsdllapi.h"
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <tools/solar.h>
#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <osl/mutex.hxx>
#include <unotools/options.hxx>
#include <i18npool/lang.h>

#include <vector>

class SvtLinguConfigItem;

//////////////////////////////////////////////////////////////////////

struct UNOTOOLS_DLLPUBLIC SvtLinguOptions
{
    ::com::sun::star::uno::Sequence< rtl::OUString >    aActiveDics;
    ::com::sun::star::uno::Sequence< rtl::OUString >    aActiveConvDics;

    BOOL                                                bROActiveDics;
    BOOL                                                bROActiveConvDics;

    // Hyphenator service specific options
    INT16   nHyphMinLeading,
            nHyphMinTrailing,
            nHyphMinWordLength;

    BOOL    bROHyphMinLeading,
            bROHyphMinTrailing,
            bROHyphMinWordLength;

    // misc options (non-service specific)
    INT16   nDefaultLanguage;
    INT16   nDefaultLanguage_CJK;
    INT16   nDefaultLanguage_CTL;

    BOOL    bRODefaultLanguage;
    BOOL    bRODefaultLanguage_CJK;
    BOOL    bRODefaultLanguage_CTL;

    // spelling options (non-service specific)
    BOOL    bIsSpellSpecial;
    BOOL    bIsSpellAuto;
    BOOL    bIsSpellReverse;

    BOOL    bROIsSpellSpecial;
    BOOL    bROIsSpellAuto;
    BOOL    bROIsSpellReverse;

    // hyphenation options (non-service specific)
    BOOL    bIsHyphSpecial;
    BOOL    bIsHyphAuto;

    BOOL    bROIsHyphSpecial;
    BOOL    bROIsHyphAuto;

    // common to SpellChecker, Hyphenator and Thesaurus service
    BOOL    bIsUseDictionaryList;
    BOOL    bIsIgnoreControlCharacters;

    BOOL    bROIsUseDictionaryList;
    BOOL    bROIsIgnoreControlCharacters;

    // SpellChecker service specific options
    BOOL    bIsSpellWithDigits,
            bIsSpellUpperCase,
            bIsSpellCapitalization;

    BOOL    bROIsSpellWithDigits,
            bROIsSpellUpperCase,
            bROIsSpellCapitalization;

    // text conversion specific options
    BOOL    bIsIgnorePostPositionalWord;
    BOOL    bIsAutoCloseDialog;
    BOOL    bIsShowEntriesRecentlyUsedFirst;
    BOOL    bIsAutoReplaceUniqueEntries;
    BOOL    bIsDirectionToSimplified;
    BOOL    bIsUseCharacterVariants;
    BOOL    bIsTranslateCommonTerms;
    BOOL    bIsReverseMapping;

    BOOL    bROIsIgnorePostPositionalWord;
    BOOL    bROIsAutoCloseDialog;
    BOOL    bROIsShowEntriesRecentlyUsedFirst;
    BOOL    bROIsAutoReplaceUniqueEntries;
    BOOL    bROIsDirectionToSimplified;
    BOOL    bROIsUseCharacterVariants;
    BOOL    bROIsTranslateCommonTerms;
    BOOL    bROIsReverseMapping;

    // check value need to determine if the configuration needs to updatet
    // or not (used for a quick check if data files have been changed/added
    // or deleted
    INT32   nDataFilesChangedCheckValue;
    BOOL    bRODataFilesChangedCheckValue;

    BOOL    bIsGrammarAuto;
    BOOL    bIsGrammarInteractive;

    BOOL    bROIsGrammarAuto;
    BOOL    bROIsGrammarInteractive;

    SvtLinguOptions();
};

//////////////////////////////////////////////////////////////////////

struct UNOTOOLS_DLLPUBLIC SvtLinguConfigDictionaryEntry
{
    // the URL's pointing to the location of the files the dictionary consists of
    com::sun::star::uno::Sequence< rtl::OUString >  aLocations;
    // the name of the dictionary format implement
    rtl::OUString                                   aFormatName;
    // the list of languages (ISO names) the dictionary can be used for
    com::sun::star::uno::Sequence< rtl::OUString >  aLocaleNames;
};

//////////////////////////////////////////////////////////////////////

class UNOTOOLS_DLLPUBLIC SvtLinguConfig: public utl::detail::Options
{
    // returns static object
    UNOTOOLS_DLLPRIVATE SvtLinguConfigItem &   GetConfigItem();

    SvtLinguConfigItem &   GetConfigItem() const    { return const_cast< SvtLinguConfig * >( this )->GetConfigItem(); }

    // configuration update access for the 'Linguistic' main node
    mutable com::sun::star::uno::Reference< com::sun::star::util::XChangesBatch > m_xMainUpdateAccess;

    com::sun::star::uno::Reference< com::sun::star::util::XChangesBatch > GetMainUpdateAccess() const;

    com::sun::star::uno::Sequence< rtl::OUString > GetCurrentOrLastActiveDicts_Impl( const rtl::OUString &rPropName ) const;
    void SetCurrentOrLastActiveDicts_Impl( const rtl::OUString &rPropName, const com::sun::star::uno::Sequence< rtl::OUString > &rDictionaries ) const;

    rtl::OUString GetVendorImageUrl_Impl( const rtl::OUString &rServiceImplName, const rtl::OUString &rImageName ) const;

    // disallow copy-constructor and assignment-operator for now
    SvtLinguConfig( const SvtLinguConfig & );
    SvtLinguConfig & operator = ( const SvtLinguConfig & );

public:
    SvtLinguConfig();
    virtual ~SvtLinguConfig();

    //
    // borrowed from utl::ConfigItem
    //
    com::sun::star::uno::Sequence< rtl::OUString >
        GetNodeNames( const rtl::OUString &rNode );
    //
    com::sun::star::uno::Sequence< com::sun::star::uno::Any >
        GetProperties(
            const com::sun::star::uno::Sequence< rtl::OUString > &rNames );
    //
    sal_Bool
        ReplaceSetProperties(
            const rtl::OUString &rNode,
            com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues );

    com::sun::star::uno::Any
            GetProperty( const rtl::OUString &rPropertyName ) const;
    com::sun::star::uno::Any
            GetProperty( INT32 nPropertyHandle ) const;

    BOOL    SetProperty( const rtl::OUString &rPropertyName,
                         const com::sun::star::uno::Any &rValue );
    BOOL    SetProperty( INT32 nPropertyHandle,
                         const com::sun::star::uno::Any &rValue );

    BOOL    GetOptions( SvtLinguOptions &rOptions ) const;
    BOOL    SetOptions( const SvtLinguOptions &rOptions );

    BOOL    IsReadOnly( const rtl::OUString &rPropertyName ) const;
    BOOL    IsReadOnly( INT32 nPropertyHandle ) const;

    //!
    //! the following functions work on the 'ServiceManager' sub node of the
    //! linguistic configuration only
    //!
    BOOL GetElementNamesFor( const rtl::OUString &rNodeName, com::sun::star::uno::Sequence< rtl::OUString > &rElementNames ) const;
    //
    BOOL GetSupportedDictionaryFormatsFor( const rtl::OUString &rSetName, const rtl::OUString &rSetEntry, com::sun::star::uno::Sequence< rtl::OUString > &rFormatList ) const;
    void SetOrCreateSupportedDictionaryFormatsFor( const rtl::OUString &rSetName, const rtl::OUString &rSetEntry, const com::sun::star::uno::Sequence< rtl::OUString > &rFormatList  ) const;
    //
    BOOL GetDictionaryEntry( const rtl::OUString &rNodeName, SvtLinguConfigDictionaryEntry &rDicEntry ) const;
    void SetOrCreateDictionaryEntry( const rtl::OUString &rNodeName, const SvtLinguConfigDictionaryEntry &rDicEntry ) const;
    //
    com::sun::star::uno::Sequence< rtl::OUString > GetDisabledDictionaries() const;
    void SetDisabledDictionaries( const com::sun::star::uno::Sequence< rtl::OUString > &rDictionaries ) const;
    //
    std::vector< SvtLinguConfigDictionaryEntry > GetActiveDictionariesByFormat( const rtl::OUString &rFormatName );

    // functions returning file URLs to the respective images (if found) and empty string otherwise
    ::rtl::OUString     GetSpellAndGrammarDialogImage( const ::rtl::OUString &rServiceImplName, bool bHighContrast = false ) const;
    ::rtl::OUString     GetSpellAndGrammarContextSuggestionImage( const ::rtl::OUString &rServiceImplName, bool bHighContrast = false ) const;
    ::rtl::OUString     GetSpellAndGrammarContextDictionaryImage( const ::rtl::OUString &rServiceImplName, bool bHighContrast = false ) const;
    ::rtl::OUString     GetThesaurusDialogImage( const ::rtl::OUString &rServiceImplName, bool bHighContrast = false ) const;
    ::rtl::OUString     GetSynonymsContextImage( const ::rtl::OUString &rServiceImplName, bool bHighContrast = false ) const;

    bool                HasVendorImages( const char *pImageName ) const;
    bool                HasGrammarChecker() const;
};

//////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
