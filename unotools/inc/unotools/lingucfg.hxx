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

    sal_Bool                                                bROActiveDics;
    sal_Bool                                                bROActiveConvDics;

    // Hyphenator service specific options
    sal_Int16   nHyphMinLeading,
            nHyphMinTrailing,
            nHyphMinWordLength;

    sal_Bool    bROHyphMinLeading,
            bROHyphMinTrailing,
            bROHyphMinWordLength;

    // misc options (non-service specific)
    sal_Int16   nDefaultLanguage;
    sal_Int16   nDefaultLanguage_CJK;
    sal_Int16   nDefaultLanguage_CTL;

    sal_Bool    bRODefaultLanguage;
    sal_Bool    bRODefaultLanguage_CJK;
    sal_Bool    bRODefaultLanguage_CTL;

    // spelling options (non-service specific)
    sal_Bool    bIsSpellSpecial;
    sal_Bool    bIsSpellAuto;
    sal_Bool    bIsSpellReverse;

    sal_Bool    bROIsSpellSpecial;
    sal_Bool    bROIsSpellAuto;
    sal_Bool    bROIsSpellReverse;

    // hyphenation options (non-service specific)
    sal_Bool    bIsHyphSpecial;
    sal_Bool    bIsHyphAuto;

    sal_Bool    bROIsHyphSpecial;
    sal_Bool    bROIsHyphAuto;

    // common to SpellChecker, Hyphenator and Thesaurus service
    sal_Bool    bIsUseDictionaryList;
    sal_Bool    bIsIgnoreControlCharacters;

    sal_Bool    bROIsUseDictionaryList;
    sal_Bool    bROIsIgnoreControlCharacters;

    // SpellChecker service specific options
    sal_Bool    bIsSpellWithDigits,
            bIsSpellUpperCase,
            bIsSpellCapitalization;

    sal_Bool    bROIsSpellWithDigits,
            bROIsSpellUpperCase,
            bROIsSpellCapitalization;

    // text conversion specific options
    sal_Bool    bIsIgnorePostPositionalWord;
    sal_Bool    bIsAutoCloseDialog;
    sal_Bool    bIsShowEntriesRecentlyUsedFirst;
    sal_Bool    bIsAutoReplaceUniqueEntries;
    sal_Bool    bIsDirectionToSimplified;
    sal_Bool    bIsUseCharacterVariants;
    sal_Bool    bIsTranslateCommonTerms;
    sal_Bool    bIsReverseMapping;

    sal_Bool    bROIsIgnorePostPositionalWord;
    sal_Bool    bROIsAutoCloseDialog;
    sal_Bool    bROIsShowEntriesRecentlyUsedFirst;
    sal_Bool    bROIsAutoReplaceUniqueEntries;
    sal_Bool    bROIsDirectionToSimplified;
    sal_Bool    bROIsUseCharacterVariants;
    sal_Bool    bROIsTranslateCommonTerms;
    sal_Bool    bROIsReverseMapping;

    // check value need to determine if the configuration needs to updatet
    // or not (used for a quick check if data files have been changed/added
    // or deleted
    sal_Int32   nDataFilesChangedCheckValue;
    sal_Bool    bRODataFilesChangedCheckValue;

    sal_Bool    bIsGrammarAuto;
    sal_Bool    bIsGrammarInteractive;

    sal_Bool    bROIsGrammarAuto;
    sal_Bool    bROIsGrammarInteractive;

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
            GetProperty( sal_Int32 nPropertyHandle ) const;

    sal_Bool    SetProperty( const rtl::OUString &rPropertyName,
                         const com::sun::star::uno::Any &rValue );
    sal_Bool    SetProperty( sal_Int32 nPropertyHandle,
                         const com::sun::star::uno::Any &rValue );

    sal_Bool    GetOptions( SvtLinguOptions &rOptions ) const;
    sal_Bool    SetOptions( const SvtLinguOptions &rOptions );

    sal_Bool    IsReadOnly( const rtl::OUString &rPropertyName ) const;
    sal_Bool    IsReadOnly( sal_Int32 nPropertyHandle ) const;

    //!
    //! the following functions work on the 'ServiceManager' sub node of the
    //! linguistic configuration only
    //!
    sal_Bool GetElementNamesFor( const rtl::OUString &rNodeName, com::sun::star::uno::Sequence< rtl::OUString > &rElementNames ) const;
    //
    sal_Bool GetSupportedDictionaryFormatsFor( const rtl::OUString &rSetName, const rtl::OUString &rSetEntry, com::sun::star::uno::Sequence< rtl::OUString > &rFormatList ) const;
    void SetOrCreateSupportedDictionaryFormatsFor( const rtl::OUString &rSetName, const rtl::OUString &rSetEntry, const com::sun::star::uno::Sequence< rtl::OUString > &rFormatList  ) const;
    //
    sal_Bool GetDictionaryEntry( const rtl::OUString &rNodeName, SvtLinguConfigDictionaryEntry &rDicEntry ) const;
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

