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

#ifndef SD_SPRITE_HXX
#define SD_SPRITE_HXX

#include <vcl/virdev.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/image.hxx>
#include <vcl/bitmapex.hxx>

// ----------
// - Sprite -
// ----------


namespace sd {

class Marker;
class MetaFile;

class Sprite
{
public:
    Sprite( List* pListOfBmpEx );
    ~Sprite();

    sal_Bool            StartMoving( OutputDevice* pOut,
                                 OutputDevice* pBottomLayer = NULL,
                                 BitmapEx* pTopLayer = NULL,
                                 MetaFile** ppTopMtf = NULL,
                                 Marker* pObjStartMarker = NULL,
                                 Marker* pObjEndMarker = NULL );
    void            MoveTo( OutputDevice* pOut, const Point& rPt, const Size* pSz = NULL );
    void            MoveTo( OutputDevice* pOut, const Point& rPt, const double& rScaleX, const double& rScaleY );
    void            EndMoving( OutputDevice* pOut );

protected:
    MapMode         aOldMap;
    Region          aOldClip;
    Rectangle       aPaintRect;
    Point           aPt;
    Size            aSz;
    Point           aLayerOffsetPix;
    VirtualDevice*  pPaintDev;
    VirtualDevice*  pBottomLayer;
    BitmapEx*       pActBmpEx;
    BitmapEx*       pTopLayer;
    MetaFile**  ppTopMtf;
    Marker*         pObjStartMarker;
    Marker*         pObjEndMarker;
    List*           pListOfBmpEx;
    sal_uLong           nLastTime;
    sal_Bool            bClipRegion;

    sal_Bool            ImplPrepareMoveTo();
    void            ImplDrawSprite( OutputDevice* pOut, const Point& rPt, const Size& rSz );

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
