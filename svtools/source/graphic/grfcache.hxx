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

#include <tools/list.hxx>
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
    sal_uLong                       mnReleaseTimeoutSeconds;
    sal_uLong                       mnMaxDisplaySize;
    sal_uLong                       mnMaxObjDisplaySize;
    sal_uLong                       mnUsedDisplaySize;

    sal_Bool                        ImplFreeDisplayCacheSpace( sal_uLong nSizeToFree );
    GraphicCacheEntry*          ImplGetCacheEntry( const GraphicObject& rObj );


                                DECL_LINK( ReleaseTimeoutHdl, Timer* pTimer );

public:

                                GraphicCache( GraphicManager& rMgr,
                                              sal_uLong nDisplayCacheSize = 10000000UL,
                                              sal_uLong nMaxObjDisplayCacheSize = 2400000UL );
                                ~GraphicCache();

public:

    void                        AddGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute,
                                                  const ByteString* pID, const GraphicObject* pCopyObj );
    void                        ReleaseGraphicObject( const GraphicObject& rObj );

    void                        GraphicObjectWasSwappedOut( const GraphicObject& rObj );
    sal_Bool                        FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute );
    void                        GraphicObjectWasSwappedIn( const GraphicObject& rObj );

    ByteString                  GetUniqueID( const GraphicObject& rObj ) const;

public:

    void                        SetMaxDisplayCacheSize( sal_uLong nNewCacheSize );
    sal_uLong                       GetMaxDisplayCacheSize() const { return mnMaxDisplaySize; };

    void                        SetMaxObjDisplayCacheSize( sal_uLong nNewMaxObjSize, sal_Bool bDestroyGreaterCached = sal_False );
    sal_uLong                       GetMaxObjDisplayCacheSize() const { return mnMaxObjDisplaySize; }

    sal_uLong                       GetUsedDisplayCacheSize() const { return mnUsedDisplaySize; }
    sal_uLong                       GetFreeDisplayCacheSize() const { return( mnMaxDisplaySize - mnUsedDisplaySize ); }

    void                        SetCacheTimeout( sal_uLong nTimeoutSeconds );
    sal_uLong                       GetCacheTimeout() const { return mnReleaseTimeoutSeconds; }

    void                        ClearDisplayCache();
    sal_Bool                        IsDisplayCacheable( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                    const GraphicObject& rObj, const GraphicAttr& rAttr ) const;
    sal_Bool                        IsInDisplayCache( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                  const GraphicObject& rObj, const GraphicAttr& rAttr ) const;
    sal_Bool                        CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                       const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                       const BitmapEx& rBmpEx );
    sal_Bool                        CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                       const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                       const GDIMetaFile& rMtf );
    sal_Bool                        DrawDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                     const GraphicObject& rObj, const GraphicAttr& rAttr );
};

#endif // _GRFCACHE_HXX
