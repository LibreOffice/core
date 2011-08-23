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

#ifndef _SVTOOLS_LINGUCFG_HXX_
#define _SVTOOLS_LINGUCFG_HXX_

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>

#include <tools/solar.h>
#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <osl/mutex.hxx>
#include <bf_svtools/options.hxx>

namespace binfilter
{

class SvtLinguConfigItem;

//////////////////////////////////////////////////////////////////////

struct  SvtLinguOptions
{
    ::com::sun::star::uno::Sequence< rtl::OUString >	aActiveDics;
    ::com::sun::star::uno::Sequence< rtl::OUString >    aActiveConvDics;

    BOOL                                                bROActiveDics;
    BOOL                                                bROActiveConvDics;

    // Hyphenator service specific options
    INT16	nHyphMinLeading,
            nHyphMinTrailing,
            nHyphMinWordLength;

    BOOL    bROHyphMinLeading,
            bROHyphMinTrailing,
            bROHyphMinWordLength;

    // misc options (non-service specific)
    INT16	nDefaultLanguage;
    INT16	nDefaultLanguage_CJK;
    INT16	nDefaultLanguage_CTL;

    BOOL    bRODefaultLanguage;
    BOOL    bRODefaultLanguage_CJK;
    BOOL    bRODefaultLanguage_CTL;

    // spelling options (non-service specific)
    BOOL	bIsSpellSpecial;
    BOOL	bIsSpellInAllLanguages;
    BOOL	bIsSpellAuto;
    BOOL	bIsSpellHideMarkings;
    BOOL	bIsSpellReverse;

    BOOL    bROIsSpellSpecial;
    BOOL    bROIsSpellInAllLanguages;
    BOOL    bROIsSpellAuto;
    BOOL    bROIsSpellHideMarkings;
    BOOL    bROIsSpellReverse;

    // hyphenation options (non-service specific)
    BOOL	bIsHyphSpecial;
    BOOL	bIsHyphAuto;

    BOOL    bROIsHyphSpecial;
    BOOL    bROIsHyphAuto;

    // common to SpellChecker, Hyphenator and Thesaurus service
    BOOL	bIsGermanPreReform;
    BOOL	bIsUseDictionaryList;
    BOOL	bIsIgnoreControlCharacters;

    BOOL    bROIsGermanPreReform;
    BOOL    bROIsUseDictionaryList;
    BOOL    bROIsIgnoreControlCharacters;

    // SpellChecker service specific options
    BOOL	bIsSpellWithDigits,
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

    SvtLinguOptions();
};

//////////////////////////////////////////////////////////////////////

class  SvtLinguConfig: public Options
{

    // disallow copy-constructor and assignment-operator for now
    SvtLinguConfig( const SvtLinguConfig & );
    SvtLinguConfig & operator = ( const SvtLinguConfig & );

    // returns static object
     SvtLinguConfigItem &   GetConfigItem();

    SvtLinguConfigItem &   GetConfigItem() const    { return const_cast< SvtLinguConfig * >( this )->GetConfigItem(); }

public:
    SvtLinguConfig();
    virtual ~SvtLinguConfig();

    //
    // borrowed from utl::ConfigItem
    //
    com::sun::star::uno::Sequence< rtl::OUString >
        GetNodeNames( const rtl::OUString &rNode );

    BOOL    GetOptions( SvtLinguOptions &rOptions ) const;
};

//////////////////////////////////////////////////////////////////////

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
