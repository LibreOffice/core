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

    bool            StartMoving( OutputDevice* pOut,
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
    bool            bClipRegion;

    bool            ImplPrepareMoveTo();
    void            ImplDrawSprite( OutputDevice* pOut, const Point& rPt, const Size& rSz );

};

}

#endif
