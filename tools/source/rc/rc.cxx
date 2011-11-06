/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
