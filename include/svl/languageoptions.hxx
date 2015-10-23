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
#ifndef INCLUDED_SVL_LANGUAGEOPTIONS_HXX
#define INCLUDED_SVL_LANGUAGEOPTIONS_HXX

#include <svl/svldllapi.h>
#include <sal/types.h>
#include <unotools/configitem.hxx>
#include <unotools/options.hxx>
#include <i18nlangtag/lang.h>
#include <o3tl/typed_flags_set.hxx>

// class SvtLanguageOptions ----------------------------------------------------

// these defines can be ORed
// note these values DO NOT match the values in css::i18n::ScriptType
enum class SvtScriptType
{
    NONE         = 0x00,
    LATIN        = 0x01,
    ASIAN        = 0x02,
    COMPLEX      = 0x04,
    UNKNOWN      = 0x08 // (only used in SC) if type has not been determined yet
};
namespace o3tl
{
    template<> struct typed_flags<SvtScriptType> : is_typed_flags<SvtScriptType, 0x0f> {};
}

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
    SvtLanguageOptions( bool _bDontLoad = false );
    virtual ~SvtLanguageOptions();

    // CJK options
    bool    IsCJKFontEnabled() const;
    bool    IsVerticalTextEnabled() const;
    bool    IsAsianTypographyEnabled() const;
    bool    IsJapaneseFindEnabled() const;
    void    SetAll( bool _bSet );
    bool    IsAnyEnabled() const;

    // CTL options
    void    SetCTLFontEnabled( bool _bEnabled );
    bool    IsCTLFontEnabled() const;

    void    SetCTLSequenceChecking( bool _bEnabled );

    void    SetCTLSequenceCheckingRestricted( bool _bEnable );

    void    SetCTLSequenceCheckingTypeAndReplace( bool _bEnable );

    bool    IsReadOnly(EOption eOption) const;

    // returns for a language the scripttype
    static  SvtScriptType GetScriptTypeOfLanguage( sal_uInt16 nLang );

    // convert from css::i18n::ScriptType constants to SvtScriptType
    static SvtScriptType FromI18NToSvtScriptType( sal_Int16 nI18NType );

    static sal_Int16 FromSvtScriptTypeToI18N( SvtScriptType nI18NType );

    static sal_Int16 GetI18NScriptTypeOfLanguage( sal_uInt16 nLang );

};

/** #i42730# Gives access to the Windows 16bit system locale
 */
class SVL_DLLPUBLIC SvtSystemLanguageOptions : public utl::ConfigItem
{
private:
    OUString m_sWin16SystemLocale;

    bool            isKeyboardLayoutTypeInstalled(sal_Int16 scriptType) const;

    virtual void    ImplCommit() override;

public:
    SvtSystemLanguageOptions();
    virtual ~SvtSystemLanguageOptions();

    virtual void    Notify( const css::uno::Sequence< OUString >& rPropertyNames ) override;

    LanguageType    GetWin16SystemLanguage() const;

    bool            isCTLKeyboardLayoutInstalled() const;
    bool            isCJKKeyboardLayoutInstalled() const;
};

#endif // INCLUDED_SVL_LANGUAGEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
