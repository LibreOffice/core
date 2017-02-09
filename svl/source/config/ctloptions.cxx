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


#include <svl/ctloptions.hxx>

#include <svl/languageoptions.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <unotools/syslocale.hxx>
#include <svl/hint.hxx>
#include "itemholder2.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define CFG_READONLY_DEFAULT false

class SvtCTLOptions_Impl : public utl::ConfigItem
{
private:
    bool                        m_bIsLoaded;
    bool                        m_bCTLFontEnabled;
    bool                        m_bCTLSequenceChecking;
    bool                        m_bCTLRestricted;
    bool                        m_bCTLTypeAndReplace;
    SvtCTLOptions::CursorMovement   m_eCTLCursorMovement;
    SvtCTLOptions::TextNumerals     m_eCTLTextNumerals;

    bool                        m_bROCTLFontEnabled;
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
    void            SetCTLFontEnabled( bool _bEnabled );
    bool            IsCTLFontEnabled() const { return m_bCTLFontEnabled; }

    void            SetCTLSequenceChecking( bool _bEnabled );
    bool            IsCTLSequenceChecking() const { return m_bCTLSequenceChecking;}

    void            SetCTLSequenceCheckingRestricted( bool _bEnable );
    bool            IsCTLSequenceCheckingRestricted() const   { return m_bCTLRestricted; }

    void            SetCTLSequenceCheckingTypeAndReplace( bool _bEnable );
    bool            IsCTLSequenceCheckingTypeAndReplace() const { return m_bCTLTypeAndReplace; }

    void            SetCTLCursorMovement( SvtCTLOptions::CursorMovement _eMovement );
    SvtCTLOptions::CursorMovement
                    GetCTLCursorMovement() const { return m_eCTLCursorMovement; }

    void            SetCTLTextNumerals( SvtCTLOptions::TextNumerals _eNumerals );
    SvtCTLOptions::TextNumerals
                    GetCTLTextNumerals() const { return m_eCTLTextNumerals; }

    bool            IsReadOnly(SvtCTLOptions::EOption eOption) const;
};
namespace
{
    struct PropertyNames
        : public rtl::Static< Sequence< OUString >, PropertyNames > {};
}
bool SvtCTLOptions_Impl::IsReadOnly(SvtCTLOptions::EOption eOption) const
{
    bool bReadOnly = CFG_READONLY_DEFAULT;
    switch(eOption)
    {
        case SvtCTLOptions::E_CTLFONT             : bReadOnly = m_bROCTLFontEnabled       ; break;
        case SvtCTLOptions::E_CTLSEQUENCECHECKING : bReadOnly = m_bROCTLSequenceChecking  ; break;
        case SvtCTLOptions::E_CTLCURSORMOVEMENT   : bReadOnly = m_bROCTLCursorMovement    ; break;
        case SvtCTLOptions::E_CTLTEXTNUMERALS     : bReadOnly = m_bROCTLTextNumerals      ; break;
        case SvtCTLOptions::E_CTLSEQUENCECHECKINGRESTRICTED: bReadOnly = m_bROCTLRestricted  ; break;
        case SvtCTLOptions::E_CTLSEQUENCECHECKINGTYPEANDREPLACE: bReadOnly = m_bROCTLTypeAndReplace; break;
        default: assert(false);
    }
    return bReadOnly;
}
SvtCTLOptions_Impl::SvtCTLOptions_Impl() :

    utl::ConfigItem("Office.Common/I18N/CTL"),

    m_bIsLoaded             ( false ),
    m_bCTLFontEnabled       ( true ),
    m_bCTLSequenceChecking  ( false ),
    m_bCTLRestricted        ( false ),
    m_bCTLTypeAndReplace    ( false ),
    m_eCTLCursorMovement    ( SvtCTLOptions::MOVEMENT_LOGICAL ),
    m_eCTLTextNumerals      ( SvtCTLOptions::NUMERALS_ARABIC ),

    m_bROCTLFontEnabled     ( CFG_READONLY_DEFAULT ),
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
    Sequence< OUString > &rPropertyNames = PropertyNames::get();
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
                    pValues[nRealCount] <<= (sal_Int32)m_eCTLCursorMovement;
                    ++nRealCount;
                }
            }
            break;

            case  3:
            {
                if (!m_bROCTLTextNumerals)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount] <<= (sal_Int32)m_eCTLTextNumerals;
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
    Sequence< OUString >& rPropertyNames = PropertyNames::get();
    if ( !rPropertyNames.getLength() )
    {
        rPropertyNames.realloc(6);
        OUString* pNames = rPropertyNames.getArray();
        pNames[0] = "CTLFont";
        pNames[1] = "CTLSequenceChecking";
        pNames[2] = "CTLCursorMovement";
        pNames[3] = "CTLTextNumerals";
        pNames[4] = "CTLSequenceCheckingRestricted";
        pNames[5] = "CTLSequenceCheckingTypeAndReplace";
        EnableNotification( rPropertyNames );
    }
    Sequence< Any > aValues = GetProperties( rPropertyNames );
    Sequence< sal_Bool > aROStates = GetReadOnlyStates( rPropertyNames );
    const Any* pValues = aValues.getConstArray();
    const sal_Bool* pROStates = aROStates.getConstArray();
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
                    }
                }
                else if ( pValues[nProp] >>= nValue )
                {
                    switch ( nProp )
                    {
                        case 2: { m_eCTLCursorMovement = (SvtCTLOptions::CursorMovement)nValue; m_bROCTLCursorMovement = pROStates[nProp]; } break;
                        case 3: { m_eCTLTextNumerals = (SvtCTLOptions::TextNumerals)nValue; m_bROCTLTextNumerals = pROStates[nProp]; } break;
                    }
                }
            }
        }
    }

    m_bIsLoaded = true;
}
void SvtCTLOptions_Impl::SetCTLFontEnabled( bool _bEnabled )
{
    if(!m_bROCTLFontEnabled && m_bCTLFontEnabled != _bEnabled)
    {
        m_bCTLFontEnabled = _bEnabled;
        SetModified();
        NotifyListeners(ConfigurationHints::NONE);
    }
}
void SvtCTLOptions_Impl::SetCTLSequenceChecking( bool _bEnabled )
{
    if(!m_bROCTLSequenceChecking && m_bCTLSequenceChecking != _bEnabled)
    {
        SetModified();
        m_bCTLSequenceChecking = _bEnabled;
        NotifyListeners(ConfigurationHints::NONE);
    }
}
void SvtCTLOptions_Impl::SetCTLSequenceCheckingRestricted( bool _bEnabled )
{
    if(!m_bROCTLRestricted && m_bCTLRestricted != _bEnabled)
    {
        SetModified();
        m_bCTLRestricted = _bEnabled;
        NotifyListeners(ConfigurationHints::NONE);
    }
}
void  SvtCTLOptions_Impl::SetCTLSequenceCheckingTypeAndReplace( bool _bEnabled )
{
    if(!m_bROCTLTypeAndReplace && m_bCTLTypeAndReplace != _bEnabled)
    {
        SetModified();
        m_bCTLTypeAndReplace = _bEnabled;
        NotifyListeners(ConfigurationHints::NONE);
    }
}
void SvtCTLOptions_Impl::SetCTLCursorMovement( SvtCTLOptions::CursorMovement _eMovement )
{
    if (!m_bROCTLCursorMovement && m_eCTLCursorMovement != _eMovement )
    {
        SetModified();
        m_eCTLCursorMovement = _eMovement;
        NotifyListeners(ConfigurationHints::NONE);
    }
}
void SvtCTLOptions_Impl::SetCTLTextNumerals( SvtCTLOptions::TextNumerals _eNumerals )
{
    if (!m_bROCTLTextNumerals && m_eCTLTextNumerals != _eNumerals )
    {
        SetModified();
        m_eCTLTextNumerals = _eNumerals;
        NotifyListeners(ConfigurationHints::NONE);
    }
}

namespace {

    // global
    std::weak_ptr<SvtCTLOptions_Impl> g_pCTLOptions;

    struct CTLMutex : public rtl::Static< osl::Mutex, CTLMutex > {};
}

SvtCTLOptions::SvtCTLOptions( bool bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( CTLMutex::get() );

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
    ::osl::MutexGuard aGuard( CTLMutex::get() );

    m_pImpl->RemoveListener(this);
    m_pImpl.reset();
}

void SvtCTLOptions::SetCTLFontEnabled( bool _bEnabled )
{
    assert(m_pImpl->IsLoaded());
    m_pImpl->SetCTLFontEnabled( _bEnabled );
}

bool SvtCTLOptions::IsCTLFontEnabled() const
{
    assert(m_pImpl->IsLoaded());
    return m_pImpl->IsCTLFontEnabled();
}

void SvtCTLOptions::SetCTLSequenceChecking( bool _bEnabled )
{
    assert(m_pImpl->IsLoaded());
    m_pImpl->SetCTLSequenceChecking(_bEnabled);
}

bool SvtCTLOptions::IsCTLSequenceChecking() const
{
    assert(m_pImpl->IsLoaded());
    return m_pImpl->IsCTLSequenceChecking();
}

void SvtCTLOptions::SetCTLSequenceCheckingRestricted( bool _bEnable )
{
    assert(m_pImpl->IsLoaded());
    m_pImpl->SetCTLSequenceCheckingRestricted(_bEnable);
}

bool SvtCTLOptions::IsCTLSequenceCheckingRestricted() const
{
    assert(m_pImpl->IsLoaded());
    return m_pImpl->IsCTLSequenceCheckingRestricted();
}

void SvtCTLOptions::SetCTLSequenceCheckingTypeAndReplace( bool _bEnable )
{
    assert(m_pImpl->IsLoaded());
    m_pImpl->SetCTLSequenceCheckingTypeAndReplace(_bEnable);
}

bool SvtCTLOptions::IsCTLSequenceCheckingTypeAndReplace() const
{
    assert(m_pImpl->IsLoaded());
    return m_pImpl->IsCTLSequenceCheckingTypeAndReplace();
}

void SvtCTLOptions::SetCTLCursorMovement( SvtCTLOptions::CursorMovement _eMovement )
{
    assert(m_pImpl->IsLoaded());
    m_pImpl->SetCTLCursorMovement( _eMovement );
}

SvtCTLOptions::CursorMovement SvtCTLOptions::GetCTLCursorMovement() const
{
    assert(m_pImpl->IsLoaded());
    return m_pImpl->GetCTLCursorMovement();
}

void SvtCTLOptions::SetCTLTextNumerals( SvtCTLOptions::TextNumerals _eNumerals )
{
    assert(m_pImpl->IsLoaded());
    m_pImpl->SetCTLTextNumerals( _eNumerals );
}

SvtCTLOptions::TextNumerals SvtCTLOptions::GetCTLTextNumerals() const
{
    assert(m_pImpl->IsLoaded());
    return m_pImpl->GetCTLTextNumerals();
}

bool SvtCTLOptions::IsReadOnly(EOption eOption) const
{
    assert(m_pImpl->IsLoaded());
    return m_pImpl->IsReadOnly(eOption);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
