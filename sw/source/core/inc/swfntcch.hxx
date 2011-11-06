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


#ifndef _SWFNTCCH_HXX
#define _SWFNTCCH_HXX

#include <tools/mempool.hxx>

#define NUM_DEFAULT_VALUES 36

#include "swcache.hxx"
#include "swfont.hxx"

class ViewShell;
class SfxPoolItem;

/*************************************************************************
 *                      class SwFontCache
 *************************************************************************/

class SwFontCache : public SwCache
{
public:

    inline SwFontCache() : SwCache(50,50
#ifdef DBG_UTIL
    , "Globaler AttributSet/Font-Cache pSwFontCache"
#endif
    ) {}

};

// AttributSet/Font-Cache, globale Variable, in FontCache.Cxx angelegt
extern SwFontCache *pSwFontCache;

/*************************************************************************
 *                      class SwFontObj
 *************************************************************************/

class SwFontObj : public SwCacheObj
{
    friend class SwFontAccess;

private:
    SwFont aSwFont;
    const SfxPoolItem* pDefaultArray[ NUM_DEFAULT_VALUES ];

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwFontObj)

    SwFontObj( const void* pOwner, ViewShell *pSh );

    virtual ~SwFontObj();

    inline       SwFont *GetFont()      { return &aSwFont; }
    inline const SwFont *GetFont() const  { return &aSwFont; }
    inline const SfxPoolItem** GetDefault() { return pDefaultArray; }
};

/*************************************************************************
 *                      class SwFontAccess
 *************************************************************************/


class SwFontAccess : public SwCacheAccess
{
    ViewShell *pShell;
protected:
    virtual SwCacheObj *NewObj( );

public:
    SwFontAccess( const void *pOwner, ViewShell *pSh );
    SwFontObj *Get();
};

#endif
