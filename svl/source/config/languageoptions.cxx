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
// global ----------------------------------------------------------------------

namespace { struct ALMutex : public rtl::Static< ::osl::Mutex, ALMutex > {}; }

// class SvtLanguageOptions ----------------------------------------------------

SvtLanguageOptions::SvtLanguageOptions( sal_Bool _bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ALMutex::get() );

    m_pCJKOptions = new SvtCJKOptions( _bDontLoad );
    m_pCTLOptions = new SvtCTLOptions( _bDontLoad );
    m_pCTLOptions->AddListener(this);
    m_pCJKOptions->AddListener(this);
}
//------------------------------------------------------------------------------
SvtLanguageOptions::~SvtLanguageOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ALMutex::get() );

    m_pCTLOptions->RemoveListener(this);
    m_pCJKOptions->RemoveListener(this);

    delete m_pCJKOptions;
    delete m_pCTLOptions;
}
//------------------------------------------------------------------------------
// CJK options -----------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsCJKFontEnabled() const
{
    return m_pCJKOptions->IsCJKFontEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsVerticalTextEnabled() const
{
    return m_pCJKOptions->IsVerticalTextEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsAsianTypographyEnabled() const
{
    return m_pCJKOptions->IsAsianTypographyEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsJapaneseFindEnabled() const
{
    return m_pCJKOptions->IsJapaneseFindEnabled();
}
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetAll( sal_Bool _bSet )
{
    m_pCJKOptions->SetAll( _bSet );
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsAnyEnabled() const
{
    return m_pCJKOptions->IsAnyEnabled();
}
//------------------------------------------------------------------------------
// CTL options -----------------------------------------------------------------
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetCTLFontEnabled( sal_Bool _bEnabled )
{
    m_pCTLOptions->SetCTLFontEnabled( _bEnabled );
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsCTLFontEnabled() const
{
    return m_pCTLOptions->IsCTLFontEnabled();
}
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetCTLSequenceChecking( sal_Bool _bEnabled )
{
    m_pCTLOptions->SetCTLSequenceChecking( _bEnabled );
}

void SvtLanguageOptions::SetCTLSequenceCheckingRestricted( sal_Bool _bEnable )
{
    m_pCTLOptions->SetCTLSequenceCheckingRestricted( _bEnable );
}

void SvtLanguageOptions::SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnable )
{
    m_pCTLOptions->SetCTLSequenceCheckingTypeAndReplace( _bEnable );
}

//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsReadOnly(SvtLanguageOptions::EOption eOption) const
{
    sal_Bool bReadOnly = sal_False;
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
sal_uInt16 SvtLanguageOptions::GetScriptTypeOfLanguage( sal_uInt16 nLang )
{
    if( LANGUAGE_DONTKNOW == nLang )
        nLang = LANGUAGE_ENGLISH_US;
    else if( LANGUAGE_SYSTEM == nLang  )
        nLang = SvtSysLocale().GetLanguageTag().getLanguageType();

    sal_Int16 nScriptType = MsLangId::getScriptType( nLang );
    sal_uInt16 nScript;
    switch (nScriptType)
    {
        case ::com::sun::star::i18n::ScriptType::ASIAN:
            nScript = SCRIPTTYPE_ASIAN;
            break;
        case ::com::sun::star::i18n::ScriptType::COMPLEX:
            nScript = SCRIPTTYPE_COMPLEX;
            break;
        default:
            nScript = SCRIPTTYPE_LATIN;
    }
    return nScript;
}
// -----------------------------------------------------------------------------

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

void    SvtSystemLanguageOptions::Commit()
{
    //does nothing
}

void    SvtSystemLanguageOptions::Notify( const com::sun::star::uno::Sequence< OUString >& )
{
    // no listeners supported yet
}

LanguageType SvtSystemLanguageOptions::GetWin16SystemLanguage() const
{
    if( m_sWin16SystemLocale.isEmpty() )
        return LANGUAGE_NONE;
    return LanguageTag( m_sWin16SystemLocale ).getLanguageType();
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
                LCID lang = MAKELCID(((UINT)lpList[i] & 0xffffffff), SORT_DEFAULT);
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
    return isKeyboardLayoutTypeInstalled(::com::sun::star::i18n::ScriptType::COMPLEX);
}


bool SvtSystemLanguageOptions::isCJKKeyboardLayoutInstalled() const
{
    return isKeyboardLayoutTypeInstalled(::com::sun::star::i18n::ScriptType::ASIAN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
