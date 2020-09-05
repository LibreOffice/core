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

#include <string.h>
#include "ddeimp.hxx"
#include <svl/svdde.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <osl/thread.h>
#include <sot/exchange.hxx>

DdeData::DdeData()
{
    xImp.reset(new DdeDataImp);
    xImp->hData = nullptr;
    xImp->nData = 0;
    xImp->pData = nullptr;
    xImp->nFmt = SotClipboardFormatId::STRING;
}

DdeData::DdeData(const void* p, long n, SotClipboardFormatId f)
{
    xImp.reset(new DdeDataImp);
    xImp->hData = nullptr;
    xImp->pData = p;
    xImp->nData = n;
    xImp->nFmt  = f;
}

DdeData::DdeData( const OUString& s )
{
    xImp.reset(new DdeDataImp);
    xImp->hData = nullptr;
    xImp->pData = s.getStr();
    xImp->nData = s.getLength()+1;
    xImp->nFmt = SotClipboardFormatId::STRING;
}

DdeData::DdeData(const DdeData& rData)
{
    xImp.reset(new DdeDataImp);
    xImp->hData = rData.xImp->hData;
    xImp->nData = rData.xImp->nData;
    xImp->pData = rData.xImp->pData;
    xImp->nFmt  = rData.xImp->nFmt;
    Lock();
}

DdeData::DdeData(DdeData&& rData) noexcept
    : xImp(std::move(rData.xImp))
{
}

DdeData::~DdeData()
{
    if (xImp && xImp->hData)
        DdeUnaccessData(xImp->hData);
}

void DdeData::Lock()
{
    if (xImp->hData)
        xImp->pData = DdeAccessData(xImp->hData, &xImp->nData);
}

SotClipboardFormatId DdeData::GetFormat() const
{
    return xImp->nFmt;
}

void DdeData::SetFormat(SotClipboardFormatId nFmt)
{
    xImp->nFmt = nFmt;
}

void const * DdeData::getData() const
{
    return xImp->pData;
}

long DdeData::getSize() const
{
    return xImp->nData;
}

DdeData& DdeData::operator=(const DdeData& rData)
{
    if ( &rData != this )
    {
        DdeData tmp(rData);
        xImp = std::move(tmp.xImp);
    }

    return *this;
}

DdeData& DdeData::operator=(DdeData&& rData) noexcept
{
    xImp = std::move(rData.xImp);
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
                return RegisterClipboardFormatW( o3tl::toW(aName.getStr()) );
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
            WCHAR szName[ 256 ];

            if(GetClipboardFormatNameW( nFmt, szName, SAL_N_ELEMENTS(szName) ))
                return SotExchange::RegisterFormatName( OUString(o3tl::toU(szName)) );
        }
#endif
        break;
    }
    return static_cast<SotClipboardFormatId>(nFmt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
