/*************************************************************************
 *
 *  $RCSfile: ctloptions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2002-08-14 11:02:25 $
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

// SvtCJKOptions_Impl ----------------------------------------------------------

class SvtCTLOptions_Impl : public utl::ConfigItem
{
private:
    static Sequence< rtl::OUString > m_aPropertyNames;

    sal_Bool    m_bIsLoaded;
    sal_Bool    m_bCTLFontEnabled;
    sal_Bool    m_bCTLSequenceChecking;

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
};
//------------------------------------------------------------------------------
Sequence< rtl::OUString > SvtCTLOptions_Impl::m_aPropertyNames;
//------------------------------------------------------------------------------
SvtCTLOptions_Impl::SvtCTLOptions_Impl() :

    utl::ConfigItem( ASCII_STR("Office.Common/I18N/CTL") ),

    m_bIsLoaded         ( sal_False ),
    m_bCTLFontEnabled   ( sal_False ),
    m_bCTLSequenceChecking( sal_False )

{
}
//------------------------------------------------------------------------------
SvtCTLOptions_Impl::~SvtCTLOptions_Impl()
{
}
// -----------------------------------------------------------------------------
void SvtCTLOptions_Impl::Notify( const Sequence< rtl::OUString >& aPropertyNames )
{
    Load();
}
// -----------------------------------------------------------------------------
void SvtCTLOptions_Impl::Commit()
{
    rtl::OUString* pNames = m_aPropertyNames.getArray();
    Sequence< Any > aValues( m_aPropertyNames.getLength() );
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    BOOL bVal;
    for ( int nProp = 0; nProp < m_aPropertyNames.getLength(); nProp++ )
    {
        switch( nProp )
        {
            case 0: bVal = m_bCTLFontEnabled; break;
            case 1: bVal = m_bCTLSequenceChecking; break;
        }
        pValues[nProp].setValue( &bVal, getBooleanCppuType() );
    }
    PutProperties( m_aPropertyNames, aValues );
}
// -----------------------------------------------------------------------------
void SvtCTLOptions_Impl::Load()
{
    if ( !m_aPropertyNames.getLength() )
    {
        m_aPropertyNames.realloc(2);
        rtl::OUString* pNames = m_aPropertyNames.getArray();
        pNames[0] = ASCII_STR("CTLFont");
        pNames[1] = ASCII_STR("CTLSequenceChecking");
        EnableNotification( m_aPropertyNames );
    }
    Sequence< Any > aValues = GetProperties( m_aPropertyNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == m_aPropertyNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == m_aPropertyNames.getLength() )
    {
        sal_Bool bValue = sal_False;

        for ( int nProp = 0; nProp < m_aPropertyNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                if ( pValues[nProp] >>= bValue )
                {
                    switch ( nProp )
                    {
                        case 0: m_bCTLFontEnabled = bValue; break;
                        case 1: m_bCTLSequenceChecking = bValue;break;
                    }
                }
            }
        }
    }
    m_bIsLoaded = sal_True;
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLFontEnabled( sal_Bool _bEnabled )
{
    if(m_bCTLFontEnabled != _bEnabled)
    {
        m_bCTLFontEnabled = _bEnabled;
        SetModified();
    }
}
//------------------------------------------------------------------------------
void SvtCTLOptions_Impl::SetCTLSequenceChecking( sal_Bool _bEnabled )
{
    if(m_bCTLSequenceChecking != _bEnabled)
    {
        SetModified();
        m_bCTLSequenceChecking = _bEnabled;
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

