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

UniString::UniString( const OUString& rStr )
    : mpData(NULL)
{
    OSL_ENSURE(rStr.pData->length < STRING_MAXLEN,
               "Overflowing OUString -> UniString cut to zero length");


    if (rStr.pData->length < STRING_MAXLEN)
    {
        mpData = reinterpret_cast< UniStringData * >(const_cast< OUString & >(rStr).pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
}

UniString& UniString::Assign( const OUString& rStr )
{
    OSL_ENSURE(rStr.pData->length < STRING_MAXLEN,
               "Overflowing OUString -> UniString cut to zero length");

    if (rStr.pData->length < STRING_MAXLEN)
    {
        if( mpData != NULL )
            STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = reinterpret_cast< UniStringData * >(const_cast< OUString & >(rStr).pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }

    return *this;
}

#include <rtl/ustrbuf.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>

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
