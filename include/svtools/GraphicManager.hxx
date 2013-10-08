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

#ifndef _GRAPHIC_MANAGER_HXX
#define _GRAPHIC_MANAGER_HXX

class Animation;
class Graphic;
class GraphicObject;
class GraphicAttr;
class GraphicObject;
class GraphicCache;

#include <vector>
#include <rtl/ref.hxx>
#include <vcl/virdev.hxx>
#include <svtools/svtdllapi.h>

#define GRAPHIC_MANAGER_CACHE_SIZE 15000000UL
#define GRAPHCI_MANAGER_MAX_OBJECT_CACHE_SIZE 3000000UL

typedef ::std::vector< const GraphicObject* > GraphicObjectList;

class SVT_DLLPUBLIC GraphicManager
{

  public:
          GraphicManager( size_t nCacheSize = GRAPHIC_MANAGER_CACHE_SIZE,
                          size_t nMaxObjCacheSize = GRAPHCI_MANAGER_MAX_OBJECT_CACHE_SIZE );
         ~GraphicManager();

    static void  Adjust( Animation& rAnimation, const GraphicAttr& rAttr,
                         sal_uInt32 nAdjustmentFlags );
    static void  Adjust( BitmapEx& rBmpEx, const GraphicAttr& rAttr,
                         sal_uInt32 nAdjustmentFlags );
    static void  Adjust( GDIMetaFile& rMtf, const GraphicAttr& rAttr,
                         sal_uInt32 nAdjustmentFlags );

    bool  Draw( OutputDevice* pOutDev, const Point& rPoint,
                const Size& rSize, GraphicObject* pGraphicObject,
                const GraphicAttr& rGraphicAttr, const sal_uInt32 nFlags,
                bool& rCached);

    bool  DrawObj( OutputDevice* pOut, const Point& rPt,
                   const Size& rSz, GraphicObject* pObj,
                   const GraphicAttr& rAttr, const sal_uInt32 nFlags,
                   bool& rCached);
    bool  FillSwappedGraphicObject( const rtl::Reference<GraphicObject>& rGraphicObject, Graphic& rSubstituteGraphicObject );
    OString GetUniqueID( const rtl::Reference<GraphicObject>& rGraphicObject ) const;
    OString GetUniqueID( const GraphicObject* pGraphicObject ) const;
    void  GraphicObjectWasSwappedIn( const rtl::Reference<GraphicObject>& rGraphicObjet );
    void  GraphicObjectWasSwappedOut( const rtl::Reference<GraphicObject>& rGRaphicObject );
    inline bool HasObjects() const { return !maObjectList.empty(); }
    bool  IsInCache( OutputDevice* pOut, const Point& rPt,
                     const Size& rSz, const rtl::Reference<GraphicObject>& rObj,
                     const GraphicAttr& rAttr ) const;
    bool  IsInCache( OutputDevice* pOut, const Point& rPt,
                     const Size& rSz, const GraphicObject* pObj,
                     const GraphicAttr& rAttr ) const;
    void  RegisterObject( const GraphicObject* rGraphicObject, Graphic& rSubstituteGraphicObject,
                          const OString* pID = NULL, const GraphicObject* pCopyObj = NULL );
    void  ReleaseFromCache( const GraphicObject& rObj );
    void  SetCacheTimeout( int nTimeoutSeconds );
    void  SetMaxCacheSize( size_t nNewCacheSize );
    void  SetMaxObjCacheSize( size_t nNewMaxObjSize,
                              bool bDestroyGreaterCached = false);
    void  UnregisterObject( const GraphicObject* pGraphicObject );

    static void  Draw( OutputDevice* pOutDev, const Point& rPoint,
                       const Size& rSize, GDIMetaFile& rMtf,
                       const GraphicAttr& rAttr );

  private:
    GraphicObjectList       maObjectList;
    GraphicCache*           mpCache;


    bool SVT_DLLPRIVATE ImplCreateOutput( OutputDevice* pOutDev, const Point& rPoint,
                                          const Size& rSize, const BitmapEx& rBmpEx,
                                          const GraphicAttr& rAttr, const sal_uInt32 nFlags,
                                          BitmapEx* pBmpEx = NULL);
    bool SVT_DLLPRIVATE ImplCreateOutput( OutputDevice* pOutDev, const Point& rPoint,
                                          const Size& rSize, const GDIMetaFile& rMtf,
                                          const GraphicAttr& rAttr, const sal_uInt32 nFlags,
                                          GDIMetaFile& rOutMtf, BitmapEx& rOutBmpEx);


    static GraphicManager*  mpGlobalManager;
    static void InitGlobal();

  public:
    static GraphicManager * GetGlobalManager()
        {
            /* FIXME: this is racy */
            if(!GraphicManager::mpGlobalManager)
            {
                GraphicManager::InitGlobal();
            };
            return GraphicManager::mpGlobalManager;
        }
};

#endif /* _GRAPHIC_MANAGER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

