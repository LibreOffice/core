/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include <svl/ctloptions.hxx>

#include <svl/languageoptions.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>
#include <cpo/uno/Any.h>
#include <cpo/uno/Sequence.hxx>
#include <cstdlib>
#include <cstring>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <osl/mutex.hxx>
#include "itemholder2.hxx"
#include <officecfg/System.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace cpo::uno;

#define CFG_READONLY_DEFAULT false

class SvtCTLOptions_Impl : public utl::ConfigItem
{
private:
    bool                        m_bIsLoaded;
    bool                        m_bCTLFontEnabled;
    bool m_bCTLVerticalText;
    bool                        m_bCTLSequenceChecking;
    bool                        m_bCTLRestricted;
    bool                        m_bCTLTypeAndReplace;
    SvtCTLOptions::CursorMovement   m_eCTLCursorMovement;
    SvtCTLOptions::TextNumerals     m_eCTLTextNumerals;

    bool                        m_bROCTLFontEnabled;
    bool m_bROCTLVerticalText;
    bool                        m_bROCTLSequenceChecking;
    bool                        m_bROCTLRestricted;
    bool                        m_bROCTLTypeAndReplace;
    bool                        m_bROCTLCursorMovement;
    bool                        m_bROCTLTextNumerals;

    virtual void    ImplCommit() override;

public:
    SvtCTLOptions_Impl();
    virtual ~SvtCTLOptions_Impl() override;

    virtual void    Notify( const Sequence< OUString >& _aPropertyNames ) override;
    void            Load();

    bool            IsLoaded() const { return m_bIsLoaded; }
};
namespace
{
    Sequence<OUString> & PropertyNames()
    {
        static Sequence<OUString> SINGLETON;
        return SINGLETON;
    }
}

SvtCTLOptions_Impl::SvtCTLOptions_Impl() :

    utl::ConfigItem(u"Office.Common/I18N/CTL"_ustr),

    m_bIsLoaded             ( false ),
    m_bCTLFontEnabled       ( true ),
    m_bCTLVerticalText      ( true ),
    m_bCTLSequenceChecking  ( false ),
    m_bCTLRestricted        ( false ),
    m_bCTLTypeAndReplace    ( false ),
    m_eCTLCursorMovement    ( SvtCTLOptions::MOVEMENT_LOGICAL ),
    m_eCTLTextNumerals      ( SvtCTLOptions::NUMERALS_ARABIC ),

    m_bROCTLFontEnabled     ( CFG_READONLY_DEFAULT ),
    m_bROCTLVerticalText    ( CFG_READONLY_DEFAULT ),
    m_bROCTLSequenceChecking( CFG_READONLY_DEFAULT ),
    m_bROCTLRestricted      ( CFG_READONLY_DEFAULT ),
    m_bROCTLTypeAndReplace  ( CFG_READONLY_DEFAULT ),
    m_bROCTLCursorMovement  ( CFG_READONLY_DEFAULT ),
    m_bROCTLTextNumerals    ( CFG_READONLY_DEFAULT )
{
}
SvtCTLOptions_Impl::~SvtCTLOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

void SvtCTLOptions_Impl::Notify( const Sequence< OUString >& )
{
    Load();
    NotifyListeners(ConfigurationHints::CtlSettingsChanged);
}

void SvtCTLOptions_Impl::ImplCommit()
{
    Sequence< OUString > &rPropertyNames = PropertyNames();
    OUString* pOrgNames = rPropertyNames.getArray();
    sal_Int32 nOrgCount = rPropertyNames.getLength();

    Sequence< OUString > aNames( nOrgCount );
    Sequence< Any > aValues( nOrgCount );

    OUString* pNames = aNames.getArray();
    Any* pValues = aValues.getArray();
    sal_Int32 nRealCount = 0;

    for ( int nProp = 0; nProp < nOrgCount; nProp++ )
    {
        switch ( nProp )
        {
            case  0:
            {
                if (!m_bROCTLFontEnabled)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount] <<= m_bCTLFontEnabled;
                    ++nRealCount;
                }
            }
            break;

            case  1:
            {
                if (!m_bROCTLSequenceChecking)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount] <<= m_bCTLSequenceChecking;
                    ++nRealCount;
                }
            }
            break;

            case  2:
            {
                if (!m_bROCTLCursorMovement)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount] <<= static_cast<sal_Int32>(m_eCTLCursorMovement);
                    ++nRealCount;
                }
            }
            break;

            case  3:
            {
                if (!m_bROCTLTextNumerals)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount] <<= static_cast<sal_Int32>(m_eCTLTextNumerals);
                    ++nRealCount;
                }
            }
            break;

            case  4:
            {
                if (!m_bROCTLRestricted)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount] <<= m_bCTLRestricted;
                    ++nRealCount;
                }
            }
            break;
            case 5:
            {
                if(!m_bROCTLTypeAndReplace)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount] <<= m_bCTLTypeAndReplace;
                    ++nRealCount;
                }
            }
            break;
            case 6:
            {
                if (!m_bROCTLVerticalText)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount] <<= m_bCTLVerticalText;
                    ++nRealCount;
                }
            }
            break;
        }
    }
    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties( aNames, aValues );
    //broadcast changes
    NotifyListeners(ConfigurationHints::CtlSettingsChanged);
}

void SvtCTLOptions_Impl::Load()
{
    Sequence< OUString >& rPropertyNames = PropertyNames();
    if ( !rPropertyNames.hasElements() )
    {
        rPropertyNames = {
            u"CTLFont"_ustr,
            u"CTLSequenceChecking"_ustr,
            u"CTLCursorMovement"_ustr,
            u"CTLTextNumerals"_ustr,
            u"CTLSequenceCheckingRestricted"_ustr,
            u"CTLSequenceCheckingTypeAndReplace"_ustr,
            u"CTLVerticalText"_ustr};
        EnableNotification( rPropertyNames );
    }
    Sequence< Any > aValues = GetProperties( rPropertyNames );
    Sequence< bool > aROStates = GetReadOnlyStates( rPropertyNames );
    const Any* pValues = aValues.getConstArray();
    const bool* pROStates = aROStates.getConstArray();
    assert(aValues.getLength() == rPropertyNames.getLength() && "GetProperties failed");
    assert(aROStates.getLength() == rPropertyNames.getLength() && "GetReadOnlyStates failed");
    if ( aValues.getLength() == rPropertyNames.getLength() && aROStates.getLength() == rPropertyNames.getLength() )
    {
        bool bValue = false;
        sal_Int32 nValue = 0;

        for ( int nProp = 0; nProp < rPropertyNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                if ( pValues[nProp] >>= bValue )
                {
                    switch ( nProp )
                    {
                        case 0: { m_bCTLFontEnabled = bValue; m_bROCTLFontEnabled = pROStates[nProp]; } break;
                        case 1: { m_bCTLSequenceChecking = bValue; m_bROCTLSequenceChecking = pROStates[nProp]; } break;
                        case 4: { m_bCTLRestricted = bValue; m_bROCTLRestricted = pROStates[nProp]; } break;
                        case 5: { m_bCTLTypeAndReplace = bValue; m_bROCTLTypeAndReplace = pROStates[nProp]; } break;
                        case 6: { m_bCTLVerticalText = bValue; m_bROCTLVerticalText = pROStates[nProp]; } break;
                    }
                }
                else if ( pValues[nProp] >>= nValue )
                {
                    switch ( nProp )
                    {
                        case 2: { m_eCTLCursorMovement = static_cast<SvtCTLOptions::CursorMovement>(nValue); m_bROCTLCursorMovement = pROStates[nProp]; } break;
                        case 3: { m_eCTLTextNumerals = static_cast<SvtCTLOptions::TextNumerals>(nValue); m_bROCTLTextNumerals = pROStates[nProp]; } break;
                    }
                }
            }
        }
    }

    if (!m_bCTLFontEnabled)
    {
        SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage(LANGUAGE_SYSTEM);
        //system locale is CTL
        bool bAutoEnableCTL = bool(nScriptType & SvtScriptType::COMPLEX);

        if (!bAutoEnableCTL)
        {
            //windows secondary system locale is CTL
            OUString sWin16SystemLocale = officecfg::System::L10N::SystemLocale::get();
            LanguageType eSystemLanguage = LANGUAGE_NONE;
            if (!sWin16SystemLocale.isEmpty())
            {
                eSystemLanguage
                    = LanguageTag::convertToLanguageTypeWithFallback(sWin16SystemLocale);
            }

            if (eSystemLanguage != LANGUAGE_SYSTEM)
            {
                SvtScriptType nWinScript
                    = SvtLanguageOptions::GetScriptTypeOfLanguage(eSystemLanguage);
                bAutoEnableCTL = bool(nWinScript & SvtScriptType::COMPLEX);
            }

            //CTL keyboard is installed
            if (!bAutoEnableCTL)
            {
                bAutoEnableCTL = SvtSystemLanguageOptions::isCTLKeyboardLayoutInstalled();
            }
        }

        if (bAutoEnableCTL)
        {
            m_bCTLFontEnabled = true;
            LanguageType nLanguage = SvtSysLocale().GetLanguageTag().getLanguageType();
            //enable sequence checking for the appropriate languages
            m_bCTLSequenceChecking = m_bCTLRestricted = m_bCTLTypeAndReplace
                = (MsLangId::needsSequenceChecking(nLanguage)
                   || MsLangId::needsSequenceChecking(LANGUAGE_SYSTEM));
            Commit();
        }
    }

    m_bIsLoaded = true;
}

namespace {

    // global
    std::weak_ptr<SvtCTLOptions_Impl> g_pCTLOptions;

    osl::Mutex& CTLMutex()
    {
        static osl::Mutex aMutex;
        return aMutex;
    }
}

SvtCTLOptions::SvtCTLOptions( bool bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( CTLMutex() );

    m_pImpl = g_pCTLOptions.lock();
    if ( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtCTLOptions_Impl>();
        g_pCTLOptions = m_pImpl;
        ItemHolder2::holdConfigItem(EItem::CTLOptions);
    }

    if( !bDontLoad && !m_pImpl->IsLoaded() )
        m_pImpl->Load();

    m_pImpl->AddListener(this);
}


SvtCTLOptions::~SvtCTLOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( CTLMutex() );

    m_pImpl->RemoveListener(this);
    m_pImpl.reset();
}

bool SvtCTLOptions::IsCTLFontEnabled()
{
    // tdf#168719: The CTL font can no longer be disabled
    return true;
}

bool SvtCTLOptions::IsCTLVerticalText()
{
    // tdf#168719: The CTL font can no longer be disabled
    return true;
}

bool SvtCTLOptions::IsCTLSequenceChecking()
{
    return officecfg::Office::Common::I18N::CTL::CTLSequenceChecking::get();
}

bool SvtCTLOptions::IsCTLSequenceCheckingRestricted()
{
    return officecfg::Office::Common::I18N::CTL::CTLSequenceCheckingRestricted::get();
}

bool SvtCTLOptions::IsCTLSequenceCheckingTypeAndReplace()
{
    return officecfg::Office::Common::I18N::CTL::CTLSequenceCheckingTypeAndReplace::get();
}

SvtCTLOptions::CursorMovement SvtCTLOptions::GetCTLCursorMovement()
{
    return static_cast<SvtCTLOptions::CursorMovement>(officecfg::Office::Common::I18N::CTL::CTLCursorMovement::get());
}

SvtCTLOptions::TextNumerals SvtCTLOptions::GetCTLTextNumerals()
{
    if (comphelper::IsFuzzing())
        return SvtCTLOptions::NUMERALS_ARABIC;

    // Collabora Online admins pick a server-wide default via coolwsd.xml's
    // per_document.ctl_numerals, forwarded here as an environment variable
    // because kit processes have no per-user profile to read a preference
    // from otherwise.
    static const char* pNumeralsOverride = getenv("COOL_CTL_NUMERALS");
    if (pNumeralsOverride)
    {
        if (strcmp(pNumeralsOverride, "hindi") == 0)
            return SvtCTLOptions::NUMERALS_HINDI;
        if (strcmp(pNumeralsOverride, "system") == 0)
            return SvtCTLOptions::NUMERALS_SYSTEM;
        if (strcmp(pNumeralsOverride, "context") == 0)
            return SvtCTLOptions::NUMERALS_CONTEXT;
    }

    return static_cast<SvtCTLOptions::TextNumerals>(officecfg::Office::Common::I18N::CTL::CTLTextNumerals::get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
