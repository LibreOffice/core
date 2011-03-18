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
#include "precompiled_tools.hxx"

#define _TOOLS_RC_CXX

#include <string.h>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>

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

    sal_uIntPtr nObjMask = (sal_uInt16)pResMgr->ReadLong();

    if ( 0x01 & nObjMask )
        SetHour( (sal_uInt16)pResMgr->ReadShort() );
    if ( 0x02 & nObjMask )
        SetMin( (sal_uInt16)pResMgr->ReadShort() );
    if ( 0x04 & nObjMask )
        SetSec( (sal_uInt16)pResMgr->ReadShort() );
    if ( 0x08 & nObjMask )
        Set100Sec( (sal_uInt16)pResMgr->ReadShort() );
}

// =======================================================================

Date::Date( const ResId& rResId ) : nDate(0)
{
    rResId.SetRT( RSC_DATE );
    ResMgr* pResMgr = NULL;

    ResMgr::GetResourceSkipHeader( rResId, &pResMgr );

    sal_uIntPtr nObjMask = (sal_uInt16)pResMgr->ReadLong();

    if ( 0x01 & nObjMask )
        SetYear( (sal_uInt16)pResMgr->ReadShort() );
    if ( 0x02 & nObjMask )
        SetMonth( (sal_uInt16)pResMgr->ReadShort() );
    if ( 0x04 & nObjMask )
        SetDay( (sal_uInt16)pResMgr->ReadShort() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
