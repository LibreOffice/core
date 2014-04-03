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

#include <string.h>
#include <rtl/ustrbuf.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>

Resource::Resource( const ResId& rResId )
    : m_pResMgr(NULL)
{
    GetRes( rResId.SetRT( RSC_RESOURCE ) );
}

void Resource::GetRes( const ResId& rResId )
{
    if( rResId.GetResMgr() )
        m_pResMgr = rResId.GetResMgr();
    assert(m_pResMgr);
    m_pResMgr->GetResource( rResId, this );
    IncrementRes( sizeof( RSHEADER_TYPE ) );
}

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
        // TODO: when we change the place that writes this binary resource format to match:
        // SetNanoSec( pResMgr->ReadLong() );
        // In the meantime:
        SetNanoSec( pResMgr->ReadShort() * ::Time::nanoPerCenti );
}

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

OUString ResId::toString() const
{
    SetRT( RSC_STRING );
    ResMgr* pResMgr = GetResMgr();

    if ( !pResMgr || !pResMgr->GetResource( *this ) )
    {
        OUString sRet;

#if OSL_DEBUG_LEVEL > 0
        sRet = OUStringBuffer().
            append("<resource id ").
            append(static_cast<sal_Int32>(GetId())).
            append(" not found>").
            makeStringAndClear();
#endif

        if( pResMgr )
            pResMgr->PopContext();

        return sRet;
    }

    // String loading
    RSHEADER_TYPE * pResHdr = (RSHEADER_TYPE*)pResMgr->GetClass();

    sal_Int32 nStringLen = rtl_str_getLength( (char*)(pResHdr+1) );
    OUString sRet((const char*)(pResHdr+1), nStringLen, RTL_TEXTENCODING_UTF8);

    sal_uInt32 nSize = sizeof( RSHEADER_TYPE )
        + sal::static_int_cast< sal_uInt32 >(nStringLen) + 1;
    nSize += nSize % 2;
    pResMgr->Increment( nSize );

    ResHookProc pImplResHookProc = ResMgr::GetReadStringHook();
    if ( pImplResHookProc )
        sRet = pImplResHookProc(sRet);
    return sRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
