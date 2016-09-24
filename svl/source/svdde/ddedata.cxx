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
    pImp->nFmt = SotClipboardFormatId::STRING;
}

DdeData::DdeData(const void* p, long n, SotClipboardFormatId f)
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
    pImp->nFmt = SotClipboardFormatId::STRING;
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

SotClipboardFormatId DdeData::GetFormat() const
{
    return pImp->nFmt;
}

void DdeData::SetFormat(SotClipboardFormatId nFmt)
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

sal_uLong DdeData::GetExternalFormat(SotClipboardFormatId nFmt)
{
    switch( nFmt )
    {
    case SotClipboardFormatId::STRING:
        return CF_TEXT;
    case SotClipboardFormatId::BITMAP:
        return CF_BITMAP;
    case SotClipboardFormatId::GDIMETAFILE:
        return CF_METAFILEPICT;
    default:
        {
#if defined(_WIN32)
            OUString aName( SotExchange::GetFormatName( nFmt ) );
            if( !aName.isEmpty() )
                return RegisterClipboardFormat( reinterpret_cast<LPCWSTR>(aName.getStr()) );
#endif
        }
    }
    return static_cast<sal_uLong>(nFmt);
}

SotClipboardFormatId DdeData::GetInternalFormat(sal_uLong nFmt)
{
    switch( nFmt )
    {
    case CF_TEXT:
        return SotClipboardFormatId::STRING;
    case CF_BITMAP:
        return SotClipboardFormatId::BITMAP;
    case CF_METAFILEPICT:
        return SotClipboardFormatId::GDIMETAFILE;
    default:
#if defined(_WIN32)
        if( nFmt >= CF_MAX )
        {
            TCHAR szName[ 256 ];

            if(GetClipboardFormatName( nFmt, szName, sizeof(szName) ))
                return SotExchange::RegisterFormatName( OUString(reinterpret_cast<const sal_Unicode*>(szName)) );
        }
#endif
        break;
    }
    return static_cast<SotClipboardFormatId>(nFmt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
