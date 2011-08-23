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

#include <bf_svtools/undoopt.hxx>

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include "rtl/instance.hxx"
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <bf_svtools/smplhint.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#include <osl/mutex.hxx>
#include <rtl/logfile.hxx>
#include "itemholder2.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

namespace binfilter
{

static SvtUndoOptions_Impl* pOptions = NULL;
static sal_Int32           nRefCount = 0;

#define STEPS 0

class SvtUndoOptions_Impl : public utl::ConfigItem, public SfxBroadcaster
{
    sal_Int32               nUndoCount;
    Sequence< rtl::OUString > m_aPropertyNames;

public:
                            SvtUndoOptions_Impl();

    virtual void            Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            Commit();
    void                    Load();

    void                    SetUndoCount( sal_Int32 n ) { nUndoCount = n; SetModified();  }
    sal_Int32               GetUndoCount() const        { return nUndoCount; }
};

// -----------------------------------------------------------------------

SvtUndoOptions_Impl::SvtUndoOptions_Impl()
    : ConfigItem( OUString::createFromAscii("Office.Common/Undo") )
    , nUndoCount( 20 )
{
    Load();
}

void SvtUndoOptions_Impl::Commit()
{
    Sequence< Any > aValues( m_aPropertyNames.getLength() );
    Any* pValues = aValues.getArray();
    for ( int nProp = 0; nProp < m_aPropertyNames.getLength(); nProp++ )
    {
        switch ( nProp )
        {
            case STEPS :
                pValues[nProp] <<= nUndoCount;
                break;
            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }
    }

    PutProperties( m_aPropertyNames, aValues );
    //broadcast changes
    Broadcast(SfxSimpleHint(SFX_HINT_UNDO_OPTIONS_CHANGED));
}

// -----------------------------------------------------------------------
void SvtUndoOptions_Impl::Load()
{
    if(!m_aPropertyNames.getLength())
    {
        static const char* aPropNames[] =
        {
            "Steps",
        };

        const int nCount = sizeof( aPropNames ) / sizeof( const char* );
        m_aPropertyNames.realloc(nCount);
        OUString* pNames = m_aPropertyNames.getArray();
        for ( int i = 0; i < nCount; i++ )
            pNames[i] = OUString::createFromAscii( aPropNames[i] );
        EnableNotification( m_aPropertyNames );
    }

    Sequence< Any > aValues = GetProperties( m_aPropertyNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == m_aPropertyNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == m_aPropertyNames.getLength() )
    {
        for ( int nProp = 0; nProp < m_aPropertyNames.getLength(); nProp++ )
        {
            DBG_ASSERT( pValues[nProp].hasValue(), "property value missing" );
            if ( pValues[nProp].hasValue() )
            {
                switch ( nProp )
                {
                    case STEPS :
                    {
                        sal_Int32 nTemp = 0;
                        if ( pValues[nProp] >>= nTemp )
                            nUndoCount = nTemp;
                        else
                        {
                            DBG_ERROR( "Wrong Type!" );
                        }
                        break;
                    }

                    default:
                        DBG_ERROR( "Wrong Type!" );
                        break;
                }
            }
        }
    }
}
// -----------------------------------------------------------------------
void SvtUndoOptions_Impl::Notify( const Sequence<rtl::OUString>& )
{
    Load();
    //broadcast changes
    Broadcast(SfxSimpleHint(SFX_HINT_UNDO_OPTIONS_CHANGED));
}

// -----------------------------------------------------------------------
namespace
{
    class LocalSingleton : public rtl::Static< osl::Mutex, LocalSingleton >
    {
    };
}

// -----------------------------------------------------------------------
SvtUndoOptions::SvtUndoOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    if ( !pOptions )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtUndoOptions_Impl::ctor()");
        pOptions = new SvtUndoOptions_Impl;
         
        ItemHolder2::holdConfigItem(E_UNDOOPTIONS);
    }
    ++nRefCount;
    pImp = pOptions;
    StartListening(*pImp);
}

// -----------------------------------------------------------------------

SvtUndoOptions::~SvtUndoOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    EndListening(*pImp);
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        DELETEZ( pOptions );
    }
}

void SvtUndoOptions::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );
    Broadcast( rHint );
}

}
