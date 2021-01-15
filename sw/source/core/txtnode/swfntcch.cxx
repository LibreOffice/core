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

#include <sal/config.h>

#include <shellio.hxx>
#include <viewsh.hxx>
#include <swfntcch.hxx>
#include <fmtcol.hxx>
#include <fntcache.hxx>
#include <swfont.hxx>

// from atrstck.cxx
extern const sal_uInt8 StackPos[];

// FontCache is created in txtinit.cxx TextInit_ and deleted in TextFinit
SwFontCache *pSwFontCache = nullptr;

SwFontObj::SwFontObj( const void *pOwn, SwViewShell *pSh ) :
    SwCacheObj( pOwn ),
    m_aSwFont( &static_cast<SwTextFormatColl const *>(pOwn)->GetAttrSet(), pSh ? &pSh->getIDocumentSettingAccess() : nullptr )
{
    m_aSwFont.AllocFontCacheId( pSh, m_aSwFont.GetActual() );
    const SwAttrSet& rAttrSet = static_cast<SwTextFormatColl const *>(pOwn)->GetAttrSet();
    for (sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++)
        m_pDefaultArray[ StackPos[ i ] ] = &rAttrSet.Get( i );
}

SwFontObj::~SwFontObj()
{
}

SwFontAccess::SwFontAccess( const void *pOwn, SwViewShell *pSh ) :
    SwCacheAccess( *pSwFontCache, pOwn,
            static_cast<const SwTextFormatColl*>(pOwn)->IsInSwFntCache() ),
    m_pShell( pSh )
{
}

SwFontObj *SwFontAccess::Get( )
{
    return static_cast<SwFontObj *>( SwCacheAccess::Get( ) );
}

SwCacheObj *SwFontAccess::NewObj( )
{
    const_cast<SwTextFormatColl*>(static_cast<const SwTextFormatColl*>(m_pOwner))->SetInSwFntCache( true );
    return new SwFontObj( m_pOwner, m_pShell );
}

SAL_DLLPUBLIC_EXPORT void FlushFontCache()
{
    if (pSwFontCache)
        pSwFontCache->Flush();
    if (pFntCache)
        pFntCache->Flush();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
