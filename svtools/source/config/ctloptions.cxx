/*************************************************************************
 *
 *  $RCSfile: ctloptions.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 10:23:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "languageoptions.hxx"
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include "ctloptions.hxx"
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

using namespace ::com::sun::star::uno;

#define ASCII_STR(s)    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(s) )
#define CFG_READONLY_DEFAULT    sal_False

// SvtCJKOptions_Impl ----------------------------------------------------------

class SvtCTLOptions_Impl : public utl::ConfigItem
{
private:
    static Sequence< rtl::OUString > m_aPropertyNames;

    sal_Bool                        m_bIsLoaded;
    sal_Bool                        m_bCTLFontEnabled;
    sal_Bool                        m_bCTLSequenceChecking;
    SvtCTLOptions::CursorMovement   m_eCTLCursorMovement;
    SvtCTLOptions::TextNumerals     m_eCTLTextNumerals;

    sal_Bool                        m_bROCTLFontEnabled;
    sal_Bool                        m_bROCTLSequenceChecking;
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

    void            SetCTLCursorMovement( SvtCTLOptions::CursorMovement _eMovement );
    SvtCTLOptions::CursorMovement
                    GetCTLCursorMovement() const { return m_eCTLCursorMovement; }

    void            SetCTLTextNumerals( SvtCTLOptions::TextNumerals _eNumerals );
    SvtCTLOptions::TextNumerals
                    GetCTLTextNumerals() const { return m_eCTLTextNumerals; }

    sal_Bool        IsReadOnly(SvtCTLOptions::EOption eOption) const;
};
//------------------------------------------------------------------------------
Sequence< rtl::OUString > SvtCTLOptions_Impl::m_aPropertyNames;
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
    }
    return bReadOnly;
}
//------------------------------------------------------------------------------
SvtCTLOptions_Impl::SvtCTLOptions_Impl() :

    utl::ConfigItem( ASCII_STR("Office.Common/I18N/CTL") ),

    m_bIsLoaded             ( sal_False ),
    m_bCTLFontEnabled       ( sal_False ),
    m_bCTLSequenceChecking  ( sal_False ),
    m_eCTLCursorMovement    ( SvtCTLOptions::MOVEMENT_LOGICAL ),
    m_eCTLTextNumerals      ( SvtCTLOptions::NUMERALS_ARABIC ),

    m_bROCTLFontEnabled     ( CFG_READONLY_DEFAULT ),
    m_bROCTLSequenceChecking( CFG_READONLY_DEFAULT ),
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
void SvtCTLOptions_Impl::Notify( const Sequence< rtl::OUString >& aPropertyNames )
{
    Load();
}
// -----------------------------------------------------------------------------
void SvtCTLOptions_Impl::Commit()
{
    rtl::OUString* pOrgNames = m_aPropertyNames.getArray();
    sal_Int32 nOrgCount = m_aPropertyNames.getLength();

    Sequence< rtl::OUString > aNames( nOrgCount );
    Sequence< Any > aValues( nOrgCount );

    rtl::OUString* pNames = aNames.getArray();
    Any* pValues = aValues.getArray();
    sal_Int32 nRealCount = 0;

    const Type& rType = ::getBooleanCppuType();

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
        }
    }
    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties( aNames, aValues );
}
// -----------------------------------------------------------------------------
void SvtCTLOptions_Impl::Load()
{
    if ( !m_aPropertyNames.getLength() )
    {
        m_aPropertyNames.realloc(4);
        rtl::OUString* pNames = m_aPropertyNames.getArray();
        pNames[0] = ASCII_STR("CTLFont");
        pNames[1] = ASCII_STR("CTLSequenceChecking");
        pNames[2] = ASCII_STR("CTLCursorMovement");
        pNames[3] = ASCII_STR("CTLTextNumerals");
        EnableNotification( m_aPropertyNames );
    }
    Sequence< Any > aValues = GetProperties( m_aPropertyNames );
    Sequence< sal_Bool > aROStates = GetReadOnlyStates( m_aPropertyNames );
    const Any* pValues = aValues.getConstArray();
    const sal_Bool* pROStates = aROStates.getConstArray();
    DBG_ASSERT( aValues.getLength() == m_aPropertyNames.getLength(), "GetProperties failed" );
    DBG_ASSERT( aROStates.getLength() == m_aPropertyNames.getLength(), "GetReadOnlyStates failed" );
    if ( aValues.getLength() == m_aPropertyNames.getLength() && aROStates.getLength() == m_aPropertyNames.getLength() )
    {
        sal_Bool bValue = sal_False;
        sal_Int32 nValue = 0;

        for ( int nProp = 0; nProp < m_aPropertyNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                if ( pValues[nProp] >>= bValue )
                {
                    switch ( nProp )
                    {
                        case 0: { m_bCTLFontEnabled = bValue; m_bROCTLFontEnabled = pROStates[nProp]; } break;
                        case 1: { m_bCTLSequenceChecking = bValue; m_bROCTLSequenceChecking = pROStates[nProp]; } break;
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
    if ( !m_bCTLFontEnabled && ( nType & SCRIPTTYPE_COMPLEX ) )
    {
        m_bCTLFontEnabled = sal_True;
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
    }
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLSequenceChecking( sal_Bool _bEnabled )
{
    if(!m_bROCTLSequenceChecking && m_bCTLSequenceChecking != _bEnabled)
    {
        SetModified();
        m_bCTLSequenceChecking = _bEnabled;
    }
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLCursorMovement( SvtCTLOptions::CursorMovement _eMovement )
{
    if (!m_bROCTLCursorMovement && m_eCTLCursorMovement != _eMovement )
    {
        SetModified();
        m_eCTLCursorMovement = _eMovement;
    }
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLTextNumerals( SvtCTLOptions::TextNumerals _eNumerals )
{
    if (!m_bROCTLTextNumerals && m_eCTLTextNumerals != _eNumerals )
    {
        SetModified();
        m_eCTLTextNumerals = _eNumerals;
    }
}
// global ----------------------------------------------------------------

static SvtCTLOptions_Impl*  pCTLOptions = NULL;
static sal_Int32            nCTLRefCount = 0;
static ::osl::Mutex         aCTLMutex;

// class SvtCTLOptions --------------------------------------------------

SvtCTLOptions::SvtCTLOptions( sal_Bool bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( aCTLMutex );
    if ( !pCTLOptions )
        pCTLOptions = new SvtCTLOptions_Impl;
    if( !bDontLoad && !pCTLOptions->IsLoaded() )
        pCTLOptions->Load();

    ++nCTLRefCount;
    m_pImp = pCTLOptions;
}

// -----------------------------------------------------------------------

SvtCTLOptions::~SvtCTLOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( aCTLMutex );

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

