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
