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


#include <svl/languageoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/syslocale.hxx>

#ifdef WNT
#include <windows.h>
#endif

using namespace ::com::sun::star;
// global

namespace { struct ALMutex : public rtl::Static< ::osl::Mutex, ALMutex > {}; }

SvtLanguageOptions::SvtLanguageOptions( bool _bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ALMutex::get() );

    m_pCJKOptions = new SvtCJKOptions( _bDontLoad );
    m_pCTLOptions = new SvtCTLOptions( _bDontLoad );
    m_pCTLOptions->AddListener(this);
    m_pCJKOptions->AddListener(this);
}
SvtLanguageOptions::~SvtLanguageOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ALMutex::get() );

    m_pCTLOptions->RemoveListener(this);
    m_pCJKOptions->RemoveListener(this);

    delete m_pCJKOptions;
    delete m_pCTLOptions;
}
// CJK options
bool SvtLanguageOptions::IsCJKFontEnabled() const
{
    return m_pCJKOptions->IsCJKFontEnabled();
}
bool SvtLanguageOptions::IsVerticalTextEnabled() const
{
    return m_pCJKOptions->IsVerticalTextEnabled();
}
bool SvtLanguageOptions::IsAsianTypographyEnabled() const
{
    return m_pCJKOptions->IsAsianTypographyEnabled();
}
bool SvtLanguageOptions::IsJapaneseFindEnabled() const
{
    return m_pCJKOptions->IsJapaneseFindEnabled();
}
void SvtLanguageOptions::SetAll( bool _bSet )
{
    m_pCJKOptions->SetAll( _bSet );
}
bool SvtLanguageOptions::IsAnyEnabled() const
{
    return m_pCJKOptions->IsAnyEnabled();
}
// CTL options
void SvtLanguageOptions::SetCTLFontEnabled( bool _bEnabled )
{
    m_pCTLOptions->SetCTLFontEnabled( _bEnabled );
}
bool SvtLanguageOptions::IsCTLFontEnabled() const
{
    return m_pCTLOptions->IsCTLFontEnabled();
}
void SvtLanguageOptions::SetCTLSequenceChecking( bool _bEnabled )
{
    m_pCTLOptions->SetCTLSequenceChecking( _bEnabled );
}

void SvtLanguageOptions::SetCTLSequenceCheckingRestricted( bool _bEnable )
{
    m_pCTLOptions->SetCTLSequenceCheckingRestricted( _bEnable );
}

void SvtLanguageOptions::SetCTLSequenceCheckingTypeAndReplace( bool _bEnable )
{
    m_pCTLOptions->SetCTLSequenceCheckingTypeAndReplace( _bEnable );
}

bool SvtLanguageOptions::IsReadOnly(SvtLanguageOptions::EOption eOption) const
{
    bool bReadOnly = false;
    switch(eOption)
    {
        // cjk options
        case SvtLanguageOptions::E_CJKFONT          : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_CJKFONT        ); break;
        case SvtLanguageOptions::E_VERTICALTEXT     : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_VERTICALTEXT   ); break;
        case SvtLanguageOptions::E_ASIANTYPOGRAPHY  : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_ASIANTYPOGRAPHY); break;
        case SvtLanguageOptions::E_JAPANESEFIND     : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_JAPANESEFIND   ); break;
        case SvtLanguageOptions::E_RUBY             : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_RUBY           ); break;
        case SvtLanguageOptions::E_CHANGECASEMAP    : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_CHANGECASEMAP  ); break;
        case SvtLanguageOptions::E_DOUBLELINES      : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_DOUBLELINES    ); break;
        case SvtLanguageOptions::E_EMPHASISMARKS    : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_EMPHASISMARKS  ); break;
        case SvtLanguageOptions::E_VERTICALCALLOUT  : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_VERTICALCALLOUT); break;
        case SvtLanguageOptions::E_ALLCJK           : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_ALL            ); break;
        // ctl options
        case SvtLanguageOptions::E_CTLFONT              : bReadOnly = m_pCTLOptions->IsReadOnly(SvtCTLOptions::E_CTLFONT            ); break;
        case SvtLanguageOptions::E_CTLSEQUENCECHECKING  : bReadOnly = m_pCTLOptions->IsReadOnly(SvtCTLOptions::E_CTLSEQUENCECHECKING); break;
        case SvtLanguageOptions::E_CTLCURSORMOVEMENT    : bReadOnly = m_pCTLOptions->IsReadOnly(SvtCTLOptions::E_CTLCURSORMOVEMENT  ); break;
        case SvtLanguageOptions::E_CTLTEXTNUMERALS      : bReadOnly = m_pCTLOptions->IsReadOnly(SvtCTLOptions::E_CTLTEXTNUMERALS    ); break;
    }
    return bReadOnly;
}

// returns for a language the scripttype
SvtScriptType SvtLanguageOptions::GetScriptTypeOfLanguage( sal_uInt16 nLang )
{
    if( LANGUAGE_DONTKNOW == nLang )
        nLang = LANGUAGE_ENGLISH_US;
    else if( LANGUAGE_SYSTEM == nLang  )
        nLang = SvtSysLocale().GetLanguageTag().getLanguageType();

    sal_Int16 nScriptType = MsLangId::getScriptType( nLang );
    SvtScriptType nScript;
    switch (nScriptType)
    {
        case css::i18n::ScriptType::ASIAN:
            nScript = SvtScriptType::ASIAN;
            break;
        case css::i18n::ScriptType::COMPLEX:
            nScript = SvtScriptType::COMPLEX;
            break;
        default:
            nScript = SvtScriptType::LATIN;
    }
    return nScript;
}

SvtScriptType SvtLanguageOptions::FromI18NToSvtScriptType( sal_Int16 nI18NType )
{
    switch ( nI18NType )
    {
        case i18n::ScriptType::LATIN:   return SvtScriptType::LATIN;
        case i18n::ScriptType::ASIAN:   return SvtScriptType::ASIAN;
        case i18n::ScriptType::COMPLEX: return SvtScriptType::COMPLEX;
        case i18n::ScriptType::WEAK:    return SvtScriptType::NONE; // no mapping
        default: assert(false && nI18NType && "Unknown i18n::ScriptType"); break;
    }
    return SvtScriptType::NONE;
}

sal_Int16 SvtLanguageOptions::FromSvtScriptTypeToI18N( SvtScriptType nItemType )
{
    switch ( nItemType )
    {
        case SvtScriptType::NONE:       return 0;
        case SvtScriptType::LATIN:      return i18n::ScriptType::LATIN;
        case SvtScriptType::ASIAN:      return i18n::ScriptType::ASIAN;
        case SvtScriptType::COMPLEX:    return i18n::ScriptType::COMPLEX;
        case SvtScriptType::UNKNOWN:    return 0; // no mapping
        default: assert(false && static_cast<int>(nItemType) && "unknown SvtScriptType"); break;
    }
    return 0;
}

sal_Int16 SvtLanguageOptions::GetI18NScriptTypeOfLanguage( sal_uInt16 nLang )
{
    return FromSvtScriptTypeToI18N( GetScriptTypeOfLanguage( nLang ) );
}

SvtSystemLanguageOptions::SvtSystemLanguageOptions() :
    utl::ConfigItem( "System/L10N")
{
    uno::Sequence< OUString > aPropertyNames(1);
    OUString* pNames = aPropertyNames.getArray();
    pNames[0] = "SystemLocale";
    uno::Sequence< uno::Any > aValues = GetProperties( aPropertyNames );

    if ( aValues.getLength() )
    {
        aValues[0]>>= m_sWin16SystemLocale;
    }
}

SvtSystemLanguageOptions::~SvtSystemLanguageOptions()
{
}

void    SvtSystemLanguageOptions::ImplCommit()
{
    //does nothing
}

void    SvtSystemLanguageOptions::Notify( const css::uno::Sequence< OUString >& )
{
    // no listeners supported yet
}

LanguageType SvtSystemLanguageOptions::GetWin16SystemLanguage() const
{
    if( m_sWin16SystemLocale.isEmpty() )
        return LANGUAGE_NONE;
    return LanguageTag::convertToLanguageTypeWithFallback( m_sWin16SystemLocale );
}

bool SvtSystemLanguageOptions::isKeyboardLayoutTypeInstalled(sal_Int16 scriptType) const
{
    bool isInstalled = false;
#ifdef WNT
    int nLayouts = GetKeyboardLayoutList(0, NULL);
    if (nLayouts > 0)
    {
        HKL *lpList = (HKL*)LocalAlloc(LPTR, (nLayouts * sizeof(HKL)));
        if (lpList)
        {
            nLayouts = GetKeyboardLayoutList(nLayouts, lpList);

            for(int i = 0; i < nLayouts; ++i)
            {
                LCID lang = MAKELCID((WORD)((DWORD_PTR)lpList[i] & 0xffff), SORT_DEFAULT);
                if (MsLangId::getScriptType(lang) == scriptType)
                {
                    isInstalled = true;
                    break;
                }
            }

            LocalFree(lpList);
        }
    }
#else
    (void)scriptType;
#endif
    return isInstalled;
}


bool SvtSystemLanguageOptions::isCTLKeyboardLayoutInstalled() const
{
    return isKeyboardLayoutTypeInstalled(css::i18n::ScriptType::COMPLEX);
}


bool SvtSystemLanguageOptions::isCJKKeyboardLayoutInstalled() const
{
    return isKeyboardLayoutTypeInstalled(css::i18n::ScriptType::ASIAN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
