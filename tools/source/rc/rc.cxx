/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rc.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-27 10:19:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#define _TOOLS_RC_CXX

#include <string.h>

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TIME_HXX
#include <tools/time.hxx>
#endif

#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif
#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif

// =======================================================================

Resource::Resource( const ResId& rResId )
{
    GetRes( rResId.SetRT( RSC_RESOURCE ) );
}

// -----------------------------------------------------------------------

void Resource::GetRes( const ResId& rResId )
{
    if( rResId.GetResMgr() )
        m_pResMgr = rResId.GetResMgr();
    m_pResMgr->GetResource( rResId, this );
    IncrementRes( sizeof( RSHEADER_TYPE ) );
}

// -----------------------------------------------------------------------

// =======================================================================

Time::Time( const ResId& rResId )
{
    nTime = 0;
    rResId.SetRT( RSC_TIME );
    ResMgr* pResMgr = NULL;

    ResMgr::GetResourceSkipHeader( rResId, &pResMgr );

    ULONG nObjMask = (USHORT)pResMgr->ReadLong();

    if ( 0x01 & nObjMask )
        SetHour( (USHORT)pResMgr->ReadShort() );
    if ( 0x02 & nObjMask )
        SetMin( (USHORT)pResMgr->ReadShort() );
    if ( 0x04 & nObjMask )
        SetSec( (USHORT)pResMgr->ReadShort() );
    if ( 0x08 & nObjMask )
        Set100Sec( (USHORT)pResMgr->ReadShort() );
}

// =======================================================================

Date::Date( const ResId& rResId ) : nDate(0)
{
    rResId.SetRT( RSC_DATE );
    ResMgr* pResMgr = NULL;

    ResMgr::GetResourceSkipHeader( rResId, &pResMgr );

    ULONG nObjMask = (USHORT)pResMgr->ReadLong();

    if ( 0x01 & nObjMask )
        SetYear( (USHORT)pResMgr->ReadShort() );
    if ( 0x02 & nObjMask )
        SetMonth( (USHORT)pResMgr->ReadShort() );
    if ( 0x04 & nObjMask )
        SetDay( (USHORT)pResMgr->ReadShort() );
}
