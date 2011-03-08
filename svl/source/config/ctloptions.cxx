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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <svl/ctloptions.hxx>

#include <svl/languageoptions.hxx>
#include <i18npool/mslangid.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <osl/mutex.hxx>
#include <svl/smplhint.hxx>
#include <rtl/instance.hxx>
#include <unotools/syslocale.hxx>
#include <itemholder2.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define ASCII_STR(s)    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(s) )
#define CFG_READONLY_DEFAULT    sal_False

// SvtCJKOptions_Impl ----------------------------------------------------------

class SvtCTLOptions_Impl : public utl::ConfigItem
{
private:
    sal_Bool                        m_bIsLoaded;
    sal_Bool                        m_bCTLFontEnabled;
    sal_Bool                        m_bCTLSequenceChecking;
    sal_Bool                        m_bCTLRestricted;
    sal_Bool                        m_bCTLTypeAndReplace;
    SvtCTLOptions::CursorMovement   m_eCTLCursorMovement;
    SvtCTLOptions::TextNumerals     m_eCTLTextNumerals;

    sal_Bool                        m_bROCTLFontEnabled;
    sal_Bool                        m_bROCTLSequenceChecking;
    sal_Bool                        m_bROCTLRestricted;
    sal_Bool                        m_bROCTLTypeAndReplace;
    sal_Bool                        m_bROCTLCursorMovement;
    sal_Bool                        m_bROCTLTextNumerals;

public:
    SvtCTLOptions_Impl();
    ~SvtCTLOptions_Impl();

    virtual void    Notify( const Sequence< rtl::OUString >& _aPropertyNames );
    virtual void    Commit();
    void            Load();

    sal_Bool        IsLoaded() { return m_bIsLoaded; }
    void            SetCTLFontEnabled( sal_Bool _bEnabled );
    sal_Bool        IsCTLFontEnabled() const { return m_bCTLFontEnabled; }

    void            SetCTLSequenceChecking( sal_Bool _bEnabled );
    sal_Bool        IsCTLSequenceChecking() const { return m_bCTLSequenceChecking;}

    void            SetCTLSequenceCheckingRestricted( sal_Bool _bEnable );
    sal_Bool        IsCTLSequenceCheckingRestricted( void ) const   { return m_bCTLRestricted; }

    void            SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnable );
    sal_Bool        IsCTLSequenceCheckingTypeAndReplace() const { return m_bCTLTypeAndReplace; }

    void            SetCTLCursorMovement( SvtCTLOptions::CursorMovement _eMovement );
    SvtCTLOptions::CursorMovement
                    GetCTLCursorMovement() const { return m_eCTLCursorMovement; }

    void            SetCTLTextNumerals( SvtCTLOptions::TextNumerals _eNumerals );
    SvtCTLOptions::TextNumerals
                    GetCTLTextNumerals() const { return m_eCTLTextNumerals; }

    sal_Bool        IsReadOnly(SvtCTLOptions::EOption eOption) const;
};
//------------------------------------------------------------------------------
namespace
{
    struct PropertyNames
        : public rtl::Static< Sequence< rtl::OUString >, PropertyNames > {};
}
//------------------------------------------------------------------------------
sal_Bool SvtCTLOptions_Impl::IsReadOnly(SvtCTLOptions::EOption eOption) const
{
    sal_Bool bReadOnly = CFG_READONLY_DEFAULT;
    switch(eOption)
    {
        case SvtCTLOptions::E_CTLFONT             : bReadOnly = m_bROCTLFontEnabled       ; break;
        case SvtCTLOptions::E_CTLSEQUENCECHECKING : bReadOnly = m_bROCTLSequenceChecking  ; break;
        case SvtCTLOptions::E_CTLCURSORMOVEMENT   : bReadOnly = m_bROCTLCursorMovement    ; break;
        case SvtCTLOptions::E_CTLTEXTNUMERALS     : bReadOnly = m_bROCTLTextNumerals      ; break;
        case SvtCTLOptions::E_CTLSEQUENCECHECKINGRESTRICTED: bReadOnly = m_bROCTLRestricted  ; break;
        case SvtCTLOptions::E_CTLSEQUENCECHECKINGTYPEANDREPLACE: bReadOnly = m_bROCTLTypeAndReplace; break;
        default: OSL_FAIL(  "SvtCTLOptions_Impl::IsReadOnly() - invalid option" );
    }
    return bReadOnly;
}
//------------------------------------------------------------------------------
SvtCTLOptions_Impl::SvtCTLOptions_Impl() :

    utl::ConfigItem( ASCII_STR("Office.Common/I18N/CTL") ),

    m_bIsLoaded             ( sal_False ),
    m_bCTLFontEnabled       ( sal_False ),
    m_bCTLSequenceChecking  ( sal_False ),
    m_bCTLRestricted        ( sal_False ),
    m_eCTLCursorMovement    ( SvtCTLOptions::MOVEMENT_LOGICAL ),
    m_eCTLTextNumerals      ( SvtCTLOptions::NUMERALS_ARABIC ),

    m_bROCTLFontEnabled     ( CFG_READONLY_DEFAULT ),
    m_bROCTLSequenceChecking( CFG_READONLY_DEFAULT ),
    m_bROCTLRestricted      ( CFG_READONLY_DEFAULT ),
    m_bROCTLCursorMovement  ( CFG_READONLY_DEFAULT ),
    m_bROCTLTextNumerals    ( CFG_READONLY_DEFAULT )
{
}
//------------------------------------------------------------------------------
SvtCTLOptions_Impl::~SvtCTLOptions_Impl()
{
    if ( IsModified() == sal_True )
        Commit();
}
// -----------------------------------------------------------------------------
void SvtCTLOptions_Impl::Notify( const Sequence< rtl::OUString >& )
{
    Load();
    NotifyListeners(SFX_HINT_CTL_SETTINGS_CHANGED);
}
// -----------------------------------------------------------------------------
void SvtCTLOptions_Impl::Commit()
{
    Sequence< rtl::OUString > &rPropertyNames = PropertyNames::get();
    rtl::OUString* pOrgNames = rPropertyNames.getArray();
    sal_Int32 nOrgCount = rPropertyNames.getLength();

    Sequence< rtl::OUString > aNames( nOrgCount );
    Sequence< Any > aValues( nOrgCount );

    rtl::OUString* pNames = aNames.getArray();
    Any* pValues = aValues.getArray();
    sal_Int32 nRealCount = 0;

    const uno::Type& rType = ::getBooleanCppuType();

    for ( int nProp = 0; nProp < nOrgCount; nProp++ )
    {
        switch ( nProp )
        {
            case  0:
            {
                if (!m_bROCTLFontEnabled)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount].setValue( &m_bCTLFontEnabled, rType );
                    ++nRealCount;
                }
            }
            break;

            case  1:
            {
                if (!m_bROCTLSequenceChecking)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount].setValue( &m_bCTLSequenceChecking, rType );
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
                    pValues[nRealCount].setValue( &m_bCTLRestricted, rType );
                    ++nRealCount;
                }
            }
            break;
            case 5:
            {
                if(!m_bROCTLTypeAndReplace)
                {
                    pNames[nRealCount] = pOrgNames[nProp];
                    pValues[nRealCount].setValue( &m_bCTLTypeAndReplace, rType );
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
    NotifyListeners(SFX_HINT_CTL_SETTINGS_CHANGED);
}
// -----------------------------------------------------------------------------
void SvtCTLOptions_Impl::Load()
{
    Sequence< rtl::OUString >& rPropertyNames = PropertyNames::get();
    if ( !rPropertyNames.getLength() )
    {
        rPropertyNames.realloc(6);
        rtl::OUString* pNames = rPropertyNames.getArray();
        pNames[0] = ASCII_STR("CTLFont");
        pNames[1] = ASCII_STR("CTLSequenceChecking");
        pNames[2] = ASCII_STR("CTLCursorMovement");
        pNames[3] = ASCII_STR("CTLTextNumerals");
        pNames[4] = ASCII_STR("CTLSequenceCheckingRestricted");
        pNames[5] = ASCII_STR("CTLSequenceCheckingTypeAndReplace");
        EnableNotification( rPropertyNames );
    }
    Sequence< Any > aValues = GetProperties( rPropertyNames );
    Sequence< sal_Bool > aROStates = GetReadOnlyStates( rPropertyNames );
    const Any* pValues = aValues.getConstArray();
    const sal_Bool* pROStates = aROStates.getConstArray();
    DBG_ASSERT( aValues.getLength() == rPropertyNames.getLength(), "GetProperties failed" );
    DBG_ASSERT( aROStates.getLength() == rPropertyNames.getLength(), "GetReadOnlyStates failed" );
    if ( aValues.getLength() == rPropertyNames.getLength() && aROStates.getLength() == rPropertyNames.getLength() )
    {
        sal_Bool bValue = sal_False;
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
    sal_uInt16 nType = SvtLanguageOptions::GetScriptTypeOfLanguage(LANGUAGE_SYSTEM);
    SvtSystemLanguageOptions aSystemLocaleSettings;
    LanguageType eSystemLanguage = aSystemLocaleSettings.GetWin16SystemLanguage();
    sal_uInt16 nWinScript = SvtLanguageOptions::GetScriptTypeOfLanguage( eSystemLanguage );
    if( !m_bCTLFontEnabled && (( nType & SCRIPTTYPE_COMPLEX ) ||
            ((eSystemLanguage != LANGUAGE_SYSTEM)  && ( nWinScript & SCRIPTTYPE_COMPLEX )))  )
    {
        m_bCTLFontEnabled = sal_True;
        sal_uInt16 nLanguage = SvtSysLocale().GetLanguage();
        //enable sequence checking for the appropriate languages
        m_bCTLSequenceChecking = m_bCTLRestricted = m_bCTLTypeAndReplace =
            (MsLangId::needsSequenceChecking( nLanguage) ||
             MsLangId::needsSequenceChecking( eSystemLanguage));
        Commit();
    }
    m_bIsLoaded = sal_True;
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLFontEnabled( sal_Bool _bEnabled )
{
    if(!m_bROCTLFontEnabled && m_bCTLFontEnabled != _bEnabled)
    {
        m_bCTLFontEnabled = _bEnabled;
        SetModified();
        NotifyListeners(0);
    }
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLSequenceChecking( sal_Bool _bEnabled )
{
    if(!m_bROCTLSequenceChecking && m_bCTLSequenceChecking != _bEnabled)
    {
        SetModified();
        m_bCTLSequenceChecking = _bEnabled;
        NotifyListeners(0);
    }
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLSequenceCheckingRestricted( sal_Bool _bEnabled )
{
    if(!m_bROCTLRestricted && m_bCTLRestricted != _bEnabled)
    {
        SetModified();
        m_bCTLRestricted = _bEnabled;
        NotifyListeners(0);
    }
}
//------------------------------------------------------------------------------
void  SvtCTLOptions_Impl::SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnabled )
{
    if(!m_bROCTLTypeAndReplace && m_bCTLTypeAndReplace != _bEnabled)
    {
        SetModified();
        m_bCTLTypeAndReplace = _bEnabled;
        NotifyListeners(0);
    }
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLCursorMovement( SvtCTLOptions::CursorMovement _eMovement )
{
    if (!m_bROCTLCursorMovement && m_eCTLCursorMovement != _eMovement )
    {
        SetModified();
        m_eCTLCursorMovement = _eMovement;
        NotifyListeners(0);
    }
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLTextNumerals( SvtCTLOptions::TextNumerals _eNumerals )
{
    if (!m_bROCTLTextNumerals && m_eCTLTextNumerals != _eNumerals )
    {
        SetModified();
        m_eCTLTextNumerals = _eNumerals;
        NotifyListeners(0);
    }
}
// global ----------------------------------------------------------------

static SvtCTLOptions_Impl*  pCTLOptions = NULL;
static sal_Int32            nCTLRefCount = 0;
namespace { struct CTLMutex : public rtl::Static< osl::Mutex, CTLMutex > {}; }

// class SvtCTLOptions --------------------------------------------------

SvtCTLOptions::SvtCTLOptions( sal_Bool bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( CTLMutex::get() );
    if ( !pCTLOptions )
    {
        pCTLOptions = new SvtCTLOptions_Impl;
        ItemHolder2::holdConfigItem(E_CTLOPTIONS);
    }
    if( !bDontLoad && !pCTLOptions->IsLoaded() )
        pCTLOptions->Load();

    ++nCTLRefCount;
    m_pImp = pCTLOptions;
    m_pImp->AddListener(this);
}

// -----------------------------------------------------------------------

SvtCTLOptions::~SvtCTLOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( CTLMutex::get() );

    m_pImp->RemoveListener(this);
    if ( !--nCTLRefCount )
        DELETEZ( pCTLOptions );
}
// -----------------------------------------------------------------------------
void SvtCTLOptions::SetCTLFontEnabled( sal_Bool _bEnabled )
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    pCTLOptions->SetCTLFontEnabled( _bEnabled );
}
// -----------------------------------------------------------------------------
sal_Bool SvtCTLOptions::IsCTLFontEnabled() const
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    return pCTLOptions->IsCTLFontEnabled();
}
// -----------------------------------------------------------------------------
void SvtCTLOptions::SetCTLSequenceChecking( sal_Bool _bEnabled )
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    pCTLOptions->SetCTLSequenceChecking(_bEnabled);
}
// -----------------------------------------------------------------------------
sal_Bool SvtCTLOptions::IsCTLSequenceChecking() const
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    return pCTLOptions->IsCTLSequenceChecking();
}
// -----------------------------------------------------------------------------
void SvtCTLOptions::SetCTLSequenceCheckingRestricted( sal_Bool _bEnable )
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    pCTLOptions->SetCTLSequenceCheckingRestricted(_bEnable);
}
// -----------------------------------------------------------------------------
sal_Bool SvtCTLOptions::IsCTLSequenceCheckingRestricted( void ) const
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    return pCTLOptions->IsCTLSequenceCheckingRestricted();
}
// -----------------------------------------------------------------------------
void SvtCTLOptions::SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnable )
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    pCTLOptions->SetCTLSequenceCheckingTypeAndReplace(_bEnable);
}
// -----------------------------------------------------------------------------
sal_Bool SvtCTLOptions::IsCTLSequenceCheckingTypeAndReplace() const
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    return pCTLOptions->IsCTLSequenceCheckingTypeAndReplace();
}
// -----------------------------------------------------------------------------
void SvtCTLOptions::SetCTLCursorMovement( SvtCTLOptions::CursorMovement _eMovement )
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    pCTLOptions->SetCTLCursorMovement( _eMovement );
}
// -----------------------------------------------------------------------------
SvtCTLOptions::CursorMovement SvtCTLOptions::GetCTLCursorMovement() const
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    return pCTLOptions->GetCTLCursorMovement();
}
// -----------------------------------------------------------------------------
void SvtCTLOptions::SetCTLTextNumerals( SvtCTLOptions::TextNumerals _eNumerals )
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    pCTLOptions->SetCTLTextNumerals( _eNumerals );
}
// -----------------------------------------------------------------------------
SvtCTLOptions::TextNumerals SvtCTLOptions::GetCTLTextNumerals() const
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    return pCTLOptions->GetCTLTextNumerals();
}
// -----------------------------------------------------------------------------
sal_Bool SvtCTLOptions::IsReadOnly(EOption eOption) const
{
    DBG_ASSERT( pCTLOptions->IsLoaded(), "CTL options not loaded" );
    return pCTLOptions->IsReadOnly(eOption);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
