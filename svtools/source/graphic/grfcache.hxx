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

#ifndef INCLUDED_SVTOOLS_SOURCE_GRAPHIC_GRFCACHE_HXX
#define INCLUDED_SVTOOLS_SOURCE_GRAPHIC_GRFCACHE_HXX

#include <vcl/graph.hxx>
#include <vcl/idle.hxx>
#include <list>


// - GraphicManagerCache -


class GraphicAttr;
class GraphicCacheEntry;
class GraphicDisplayCacheEntry;
class GraphicObject;

class GraphicCache
{
private:
    typedef std::list< GraphicCacheEntry* > GraphicCacheEntryList;
    typedef std::list< GraphicDisplayCacheEntry* > GraphicDisplayCacheEntryList;

    Idle                    maReleaseIdle;
    GraphicCacheEntryList   maGraphicCache;
    GraphicDisplayCacheEntryList maDisplayCache;
    sal_uLong               mnReleaseTimeoutSeconds;
    sal_uLong               mnMaxDisplaySize;
    sal_uLong               mnMaxObjDisplaySize;
    sal_uLong               mnUsedDisplaySize;

    bool                ImplFreeDisplayCacheSpace( sal_uLong nSizeToFree );
    GraphicCacheEntry*      ImplGetCacheEntry( const GraphicObject& rObj );


                            DECL_LINK_TYPED( ReleaseTimeoutHdl, Idle* pTimer, void );

    sal_uLong               mnReleaseLimit; // tdf#95614 - avoid dead lock

public:

                            GraphicCache(
                                sal_uLong nDisplayCacheSize = 10000000UL,
                                sal_uLong nMaxObjDisplayCacheSize = 2400000UL
                            );

                            ~GraphicCache();

public:

    void                    AddGraphicObject(
                                const GraphicObject& rObj,
                                Graphic& rSubstitute,
                                const OString* pID,
                                const GraphicObject* pCopyObj
                            );

    void                    ReleaseGraphicObject( const GraphicObject& rObj );

    void                    GraphicObjectWasSwappedOut( const GraphicObject& rObj );
    void                    GraphicObjectWasSwappedIn( const GraphicObject& rObj );

    OString            GetUniqueID( const GraphicObject& rObj ) const;

public:

    void                    SetMaxDisplayCacheSize( sal_uLong nNewCacheSize );
    sal_uLong               GetMaxDisplayCacheSize() const { return mnMaxDisplaySize; };

    void                    SetMaxObjDisplayCacheSize(
                                sal_uLong nNewMaxObjSize,
                                bool bDestroyGreaterCached = false
                            );

    sal_uLong               GetMaxObjDisplayCacheSize() const { return mnMaxObjDisplaySize; }

    sal_uLong               GetUsedDisplayCacheSize() const { return mnUsedDisplaySize; }
    sal_uLong               GetFreeDisplayCacheSize() const { return( mnMaxDisplaySize - mnUsedDisplaySize ); }

    void                    SetCacheTimeout( sal_uLong nTimeoutSeconds );
    sal_uLong               GetCacheTimeout() const { return mnReleaseTimeoutSeconds; }

    bool                IsDisplayCacheable(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr
                            ) const;

    bool                IsInDisplayCache(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr
                            ) const;

    bool                CreateDisplayCacheObj(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr,
                                const BitmapEx& rBmpEx
                            );

    bool                CreateDisplayCacheObj(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr,
                                const GDIMetaFile& rMtf
                            );

    bool                DrawDisplayCacheObj(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr
                            );
};

#endif // INCLUDED_SVTOOLS_SOURCE_GRAPHIC_GRFCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
