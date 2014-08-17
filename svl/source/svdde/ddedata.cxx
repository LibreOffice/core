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


// ATTENTION: We assume StarView Clipboard format numbers and Windows
// Format numbers to be the same! If that's not the case, we need to
// adapt the code here. The implementation uses the conversions here.

#define UNICODE

#include <string.h>
#include "ddeimp.hxx"
#include <svl/svdde.hxx>

#include <osl/thread.h>

DdeData::DdeData()
{
    pImp = new DdeDataImp;
    pImp->hData = NULL;
    pImp->nData = 0;
    pImp->pData = NULL;
    pImp->nFmt  = CF_TEXT;
}

DdeData::DdeData( const void* p, long n, sal_uLong f )
{
    pImp = new DdeDataImp;
    pImp->hData = NULL;
    pImp->pData = (LPBYTE)p;
    pImp->nData = n;
    pImp->nFmt  = f;
}

DdeData::DdeData( const OUString& s )
{
    pImp = new DdeDataImp;
    pImp->hData = NULL;
    pImp->pData = (LPBYTE)s.getStr();
    pImp->nData = s.getLength()+1;
    pImp->nFmt  = CF_TEXT;
}

DdeData::DdeData( const DdeData& rData )
{
    pImp = new DdeDataImp;
    pImp->hData = rData.pImp->hData;
    pImp->nData = rData.pImp->nData;
    pImp->pData = rData.pImp->pData;
    pImp->nFmt  = rData.pImp->nFmt;
    Lock();
}

DdeData::~DdeData()
{
    if ( pImp && pImp->hData )
        DdeUnaccessData( pImp->hData );
    delete pImp;
}

void DdeData::Lock()
{
    if ( pImp->hData )
        pImp->pData = DdeAccessData( pImp->hData, (LPDWORD) &pImp->nData );
}

sal_uLong DdeData::GetFormat() const
{
    return pImp->nFmt;
}

void DdeData::SetFormat( sal_uLong nFmt )
{
    pImp->nFmt = nFmt;
}

DdeData::operator const void*() const
{
    return pImp->pData;
}

DdeData::operator long() const
{
    return pImp->nData;
}

DdeData& DdeData::operator = ( const DdeData& rData )
{
    if ( &rData != this )
    {
        DdeData tmp( rData );
        delete pImp;
        pImp = tmp.pImp;
        tmp.pImp = NULL;
    }

    return *this;
}

sal_uLong DdeData::GetExternalFormat( sal_uLong nFmt )
{
    switch( nFmt )
    {
    case FORMAT_STRING:
        nFmt = CF_TEXT;
        break;
    case FORMAT_BITMAP:
        nFmt = CF_BITMAP;
        break;
    case FORMAT_GDIMETAFILE:
        nFmt = CF_METAFILEPICT;
        break;

    default:
        {
#if defined(WNT)
            OUString aName( SotExchange::GetFormatName( nFmt ) );
            if( !aName.isEmpty() )
                nFmt = RegisterClipboardFormat( reinterpret_cast<LPCWSTR>(aName.getStr()) );
#endif
        }
    }
    return nFmt;
}

sal_uLong DdeData::GetInternalFormat( sal_uLong nFmt )
{
    switch( nFmt )
    {
    case CF_TEXT:
        nFmt = FORMAT_STRING;
        break;

    case CF_BITMAP:
        nFmt = FORMAT_BITMAP;
        break;

    case CF_METAFILEPICT:
        nFmt = FORMAT_GDIMETAFILE;
        break;

    default:
#if defined(WNT)
        if( nFmt >= CF_MAX )
        {
            TCHAR szName[ 256 ];

            if( GetClipboardFormatName( nFmt, szName, sizeof(szName) ) )
                nFmt = SotExchange::RegisterFormatName( OUString(reinterpret_cast<const sal_Unicode*>(szName)) );
        }
#endif
        break;
    }
    return nFmt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
