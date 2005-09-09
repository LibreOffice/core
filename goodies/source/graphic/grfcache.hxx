/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grfcache.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:04:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _GRFCACHE_HXX
#define _GRFCACHE_HXX

#include <tools/list.hxx>
#include <vcl/graph.hxx>
#include <vcl/timer.hxx>
#include "grfmgr.hxx"

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

    void                        AddGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute, const ByteString* pID );
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
