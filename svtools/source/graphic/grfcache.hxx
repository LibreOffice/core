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
#include <vcl/timer.hxx>
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

    Timer                   maReleaseTimer;
    GraphicCacheEntryList   maGraphicCache;
    GraphicDisplayCacheEntryList maDisplayCache;
    sal_uLong               mnReleaseTimeoutSeconds;
    sal_uLong               mnMaxDisplaySize;
    sal_uLong               mnMaxObjDisplaySize;
    sal_uLong               mnUsedDisplaySize;

    sal_Bool                ImplFreeDisplayCacheSpace( sal_uLong nSizeToFree );
    GraphicCacheEntry*      ImplGetCacheEntry( const GraphicObject& rObj );


                            DECL_LINK( ReleaseTimeoutHdl, Timer* pTimer );

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
    sal_Bool                FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute );
    void                    GraphicObjectWasSwappedIn( const GraphicObject& rObj );

    OString            GetUniqueID( const GraphicObject& rObj ) const;

public:

    void                    SetMaxDisplayCacheSize( sal_uLong nNewCacheSize );
    sal_uLong               GetMaxDisplayCacheSize() const { return mnMaxDisplaySize; };

    void                    SetMaxObjDisplayCacheSize(
                                sal_uLong nNewMaxObjSize,
                                sal_Bool bDestroyGreaterCached = sal_False
                            );

    sal_uLong               GetMaxObjDisplayCacheSize() const { return mnMaxObjDisplaySize; }

    sal_uLong               GetUsedDisplayCacheSize() const { return mnUsedDisplaySize; }
    sal_uLong               GetFreeDisplayCacheSize() const { return( mnMaxDisplaySize - mnUsedDisplaySize ); }

    void                    SetCacheTimeout( sal_uLong nTimeoutSeconds );
    sal_uLong               GetCacheTimeout() const { return mnReleaseTimeoutSeconds; }

    sal_Bool                IsDisplayCacheable(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr
                            ) const;

    sal_Bool                IsInDisplayCache(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr
                            ) const;

    sal_Bool                CreateDisplayCacheObj(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr,
                                const BitmapEx& rBmpEx
                            );

    sal_Bool                CreateDisplayCacheObj(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr,
                                const GDIMetaFile& rMtf
                            );

    sal_Bool                DrawDisplayCacheObj(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicObject& rObj,
                                const GraphicAttr& rAttr
                            );
};

#endif // INCLUDED_SVTOOLS_SOURCE_GRAPHIC_GRFCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
