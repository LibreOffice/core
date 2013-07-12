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
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#define _SVTOOLS_LANGUAGEOPTIONS_HXX

#include "svl/svldllapi.h"
#include <sal/types.h>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <unotools/configitem.hxx>
#include <unotools/options.hxx>
#include <i18nlangtag/lang.h>

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
    void        SetAll( sal_Bool _bSet );
    sal_Bool    IsAnyEnabled() const;

    // CTL options
    void        SetCTLFontEnabled( sal_Bool _bEnabled );
    sal_Bool    IsCTLFontEnabled() const;

    void        SetCTLSequenceChecking( sal_Bool _bEnabled );

    void        SetCTLSequenceCheckingRestricted( sal_Bool _bEnable );

    void        SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnable );

    sal_Bool    IsReadOnly(EOption eOption) const;

    // returns for a language the scripttype
    static sal_uInt16 GetScriptTypeOfLanguage( sal_uInt16 nLang );
};
/** #i42730# Gives access to the Windows 16bit system locale
 */
class SVL_DLLPUBLIC SvtSystemLanguageOptions : public utl::ConfigItem
{
private:
    OUString m_sWin16SystemLocale;

    bool isKeyboardLayoutTypeInstalled(sal_Int16 scriptType) const;

public:
    SvtSystemLanguageOptions();
    ~SvtSystemLanguageOptions();

    virtual void    Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence< OUString >& rPropertyNames );

    LanguageType GetWin16SystemLanguage() const;

    bool isCTLKeyboardLayoutInstalled() const;
    bool isCJKKeyboardLayoutInstalled() const;
};

#endif // _SVTOOLS_LANGUAGEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
