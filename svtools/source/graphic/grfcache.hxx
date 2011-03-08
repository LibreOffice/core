/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _GRFCACHE_HXX
#define _GRFCACHE_HXX

#include <vcl/graph.hxx>
#include <vcl/timer.hxx>
#include <svtools/grfmgr.hxx>

// -----------------------
// - GraphicManagerCache -
// -----------------------

class GraphicCacheEntry;

class GraphicCache
{
private:

    GraphicManager&             mrMgr;
    Timer                       maReleaseTimer;
    List                        maGraphicCache;
    List                        maDisplayCache;
    ULONG                       mnReleaseTimeoutSeconds;
    ULONG                       mnMaxDisplaySize;
    ULONG                       mnMaxObjDisplaySize;
    ULONG                       mnUsedDisplaySize;

    BOOL                        ImplFreeDisplayCacheSpace( ULONG nSizeToFree );
    GraphicCacheEntry*          ImplGetCacheEntry( const GraphicObject& rObj );


                                DECL_LINK( ReleaseTimeoutHdl, Timer* pTimer );

public:

                                GraphicCache( GraphicManager& rMgr,
                                              ULONG nDisplayCacheSize = 10000000UL,
                                              ULONG nMaxObjDisplayCacheSize = 2400000UL );
                                ~GraphicCache();

public:

    void                        AddGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute,
                                                  const ByteString* pID, const GraphicObject* pCopyObj );
    void                        ReleaseGraphicObject( const GraphicObject& rObj );

    void                        GraphicObjectWasSwappedOut( const GraphicObject& rObj );
    BOOL                        FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute );
    void                        GraphicObjectWasSwappedIn( const GraphicObject& rObj );

    ByteString                  GetUniqueID( const GraphicObject& rObj ) const;

public:

    void                        SetMaxDisplayCacheSize( ULONG nNewCacheSize );
    ULONG                       GetMaxDisplayCacheSize() const { return mnMaxDisplaySize; };

    void                        SetMaxObjDisplayCacheSize( ULONG nNewMaxObjSize, BOOL bDestroyGreaterCached = FALSE );
    ULONG                       GetMaxObjDisplayCacheSize() const { return mnMaxObjDisplaySize; }

    ULONG                       GetUsedDisplayCacheSize() const { return mnUsedDisplaySize; }
    ULONG                       GetFreeDisplayCacheSize() const { return( mnMaxDisplaySize - mnUsedDisplaySize ); }

    void                        SetCacheTimeout( ULONG nTimeoutSeconds );
    ULONG                       GetCacheTimeout() const { return mnReleaseTimeoutSeconds; }

    void                        ClearDisplayCache();
    BOOL                        IsDisplayCacheable( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                    const GraphicObject& rObj, const GraphicAttr& rAttr ) const;
    BOOL                        IsInDisplayCache( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                  const GraphicObject& rObj, const GraphicAttr& rAttr ) const;
    BOOL                        CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                       const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                       const BitmapEx& rBmpEx );
    BOOL                        CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                       const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                       const GDIMetaFile& rMtf );
    BOOL                        DrawDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                     const GraphicObject& rObj, const GraphicAttr& rAttr );
};

#endif // _GRFCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
