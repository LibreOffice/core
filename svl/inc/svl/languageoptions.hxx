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
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#define _SVTOOLS_LANGUAGEOPTIONS_HXX

#include "svl/svldllapi.h"
#include <sal/types.h>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <unotools/configitem.hxx>
#include <unotools/options.hxx>
#include <i18npool/lang.h>

// class SvtLanguageOptions ----------------------------------------------------

// these defines can be ored
#define SCRIPTTYPE_LATIN        0x01
#define SCRIPTTYPE_ASIAN        0x02
#define SCRIPTTYPE_COMPLEX      0x04

class SvtCJKOptions;
class SvtCTLOptions;

class SVL_DLLPUBLIC SvtLanguageOptions : public ::utl::detail::Options
{
private:
    SvtCJKOptions*  m_pCJKOptions;
    SvtCTLOptions*  m_pCTLOptions;

public:
    enum EOption
    {
        // cjk options
        E_CJKFONT,
        E_VERTICALTEXT,
        E_ASIANTYPOGRAPHY,
        E_JAPANESEFIND,
        E_RUBY,
        E_CHANGECASEMAP,
        E_DOUBLELINES,
        E_EMPHASISMARKS,
        E_VERTICALCALLOUT,
        E_ALLCJK,
        // ctl options
        E_CTLFONT,
        E_CTLSEQUENCECHECKING,
        E_CTLCURSORMOVEMENT,
        E_CTLTEXTNUMERALS
    };

    // bDontLoad is for referencing purposes only
    SvtLanguageOptions( sal_Bool _bDontLoad = sal_False );
    ~SvtLanguageOptions();

    // CJK options
    sal_Bool    IsCJKFontEnabled() const;
    sal_Bool    IsVerticalTextEnabled() const;
    sal_Bool    IsAsianTypographyEnabled() const;
    sal_Bool    IsJapaneseFindEnabled() const;
    sal_Bool    IsRubyEnabled() const;
    sal_Bool    IsChangeCaseMapEnabled() const;
    sal_Bool    IsDoubleLinesEnabled() const;
    sal_Bool    IsEmphasisMarksEnabled() const;
    sal_Bool    IsVerticalCallOutEnabled() const;
    void        SetAll( sal_Bool _bSet );
    sal_Bool    IsAnyEnabled() const;

    // CTL options
    void        SetCTLFontEnabled( sal_Bool _bEnabled );
    sal_Bool    IsCTLFontEnabled() const;

    void        SetCTLSequenceChecking( sal_Bool _bEnabled );
    sal_Bool    IsCTLSequenceChecking() const;

    void        SetCTLSequenceCheckingRestricted( sal_Bool _bEnable );
    sal_Bool    IsCTLSequenceCheckingRestricted( void ) const;

    void        SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnable );
    sal_Bool    IsCTLSequenceCheckingTypeAndReplace() const;

    sal_Bool    IsReadOnly(EOption eOption) const;

    // returns for a language the scripttype
    static sal_uInt16 GetScriptTypeOfLanguage( sal_uInt16 nLang );
};
/** #i42730# Gives access to the Windows 16bit system locale
 */
class SVL_DLLPUBLIC SvtSystemLanguageOptions : public utl::ConfigItem
{
private:
    ::rtl::OUString m_sWin16SystemLocale;

public:
    SvtSystemLanguageOptions();
    ~SvtSystemLanguageOptions();

    virtual void    Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames );

    LanguageType GetWin16SystemLanguage();
};

#endif // _SVTOOLS_LANGUAGEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
