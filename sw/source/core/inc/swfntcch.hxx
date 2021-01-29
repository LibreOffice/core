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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWFNTCCH_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWFNTCCH_HXX

#define NUM_DEFAULT_VALUES 39

#include "swcache.hxx"
#include "swfont.hxx"

class SwViewShell;
class SfxPoolItem;

class SwFontCache : public SwCache
{
public:

    SwFontCache() : SwCache(50
#ifdef DBG_UTIL
    , "Global AttributSet/Font-Cache pSwFontCache"
#endif
    ) {}

};

// AttributSet/Font-Cache, globale Variable, in FontCache.Cxx angelegt
extern SwFontCache *pSwFontCache;

class SwFontObj final : public SwCacheObj
{
    friend class SwFontAccess;

private:
    SwFont m_aSwFont;
    const SfxPoolItem* m_pDefaultArray[ NUM_DEFAULT_VALUES ];

public:
    SwFontObj( const void* pOwner, SwViewShell *pSh );

    virtual ~SwFontObj() override;

    SwFont& GetFont()        { return m_aSwFont; }
    const SwFont& GetFont() const  { return m_aSwFont; }
    const SfxPoolItem** GetDefault() { return m_pDefaultArray; }
};

class SwFontAccess : public SwCacheAccess
{
    SwViewShell *m_pShell;
protected:
    virtual SwCacheObj *NewObj( ) override;

public:
    SwFontAccess( const void *pOwner, SwViewShell *pSh );
    SwFontObj *Get();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
