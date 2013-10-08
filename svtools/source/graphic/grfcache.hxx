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

#ifndef _GRFCACHE_HXX
#define _GRFCACHE_HXX

#include <rtl/ref.hxx>
#include <vcl/graph.hxx>
#include <vcl/timer.hxx>
#include <list>

// -----------------------
// - GraphicManagerCache -
// -----------------------

class GraphicAttr;
class GraphicCacheEntry;
class GraphicDisplayCacheEntry;
class GraphicObject;

class GraphicCache
{
private:
    typedef std::list< GraphicCacheEntry* > GraphicCacheEntryList;
    typedef std::list< GraphicDisplayCacheEntry* > GraphicDisplayCacheEntryList;

    Timer                maReleaseTimer;
    GraphicCacheEntryList maGraphicCache;
    GraphicDisplayCacheEntryList maDisplayCache;
    int                  mnReleaseTimeoutSeconds;
    size_t               mnMaxDisplaySize;
    size_t               mnMaxObjDisplaySize;
    size_t               mnUsedDisplaySize;

    bool                 ImplFreeDisplayCacheSpace( sal_uLong nSizeToFree );
    GraphicCacheEntry*   ImplGetCacheEntry( const rtl::Reference<GraphicObject>& rObj );
    GraphicCacheEntry*   ImplGetCacheEntry( const GraphicObject* pObj );


                         DECL_LINK( ReleaseTimeoutHdl, Timer* pTimer );

public:
                         GraphicCache( size_t nDisplayCacheSize = 10000000UL,
                                       size_t nMaxObjDisplayCacheSize = 2400000UL );
                        ~GraphicCache();

public:
    void                 AddGraphicObject( const GraphicObject* pObj,
                                           Graphic& rSubstitute,
                                           const OString* pID,
                                           const GraphicObject* pCopyObj );

    void                 ReleaseGraphicObject( const GraphicObject* pObj );

    void                 GraphicObjectWasSwappedOut( const rtl::Reference<GraphicObject>& rObj );
    bool                 FillSwappedGraphicObject( const rtl::Reference<GraphicObject>& rObj, Graphic& rSubstitute );
    void                 GraphicObjectWasSwappedIn( const rtl::Reference<GraphicObject>& rObj );

    OString              GetUniqueID( const rtl::Reference<GraphicObject>& rObj ) const;
    OString              GetUniqueID( const GraphicObject* pObj ) const;

public:

    void                 SetMaxDisplayCacheSize( sal_uLong nNewCacheSize );
    size_t               GetMaxDisplayCacheSize() const { return mnMaxDisplaySize; };

    void                 SetMaxObjDisplayCacheSize( size_t nNewMaxObjSize,
                                                    bool bDestroyGreaterCached = false );

    size_t               GetMaxObjDisplayCacheSize() const { return mnMaxObjDisplaySize; }

    size_t               GetUsedDisplayCacheSize() const { return mnUsedDisplaySize; }
    size_t               GetFreeDisplayCacheSize() const { return( mnMaxDisplaySize - mnUsedDisplaySize ); }

    void                 SetCacheTimeout( int nTimeoutSeconds );
    int                  GetCacheTimeout() const { return mnReleaseTimeoutSeconds; }

    bool                IsDisplayCacheable( OutputDevice* pOut,
                                            const Point& rPoint,
                                            const Size& rSize,
                                            const rtl::Reference<GraphicObject>& rObj,
                                            const GraphicAttr& rAttr ) const;

    bool                IsInDisplayCache( OutputDevice* pOut,
                                          const Point& rPoint,
                                          const Size& rSize,
                                          const rtl::Reference<GraphicObject>& rObj,
                                          const GraphicAttr& rAttr ) const;
    bool                IsInDisplayCache( OutputDevice* pOut,
                                          const Point& rPoint,
                                          const Size& rSize,
                                          const GraphicObject* pObj,
                                          const GraphicAttr& rAttr ) const;

    bool                CreateDisplayCacheObj( OutputDevice* pOut,
                                               const Point& rPoint,
                                               const Size& rSize,
                                               const rtl::Reference<GraphicObject>& rObj,
                                               const GraphicAttr& rAttr,
                                               const BitmapEx& rBmpEx );

    bool                CreateDisplayCacheObj( OutputDevice* pOut,
                                               const Point& rPoint,
                                               const Size& rSize,
                                               const rtl::Reference<GraphicObject>& rObj,
                                               const GraphicAttr& rAttr,
                                               const GDIMetaFile& rMtf );

    bool                DrawDisplayCacheObj( OutputDevice* pOut,
                                             const Point& rPoint,
                                             const Size& rSize,
                                             const GraphicObject* pObj,
                                             const GraphicAttr& rAttr );
};

#endif // _GRFCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
