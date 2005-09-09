/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sprite.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:54:06 $
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

#ifndef SD_SPRITE_HXX
#define SD_SPRITE_HXX

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef _SV_REGION_HXX //autogen
#include <vcl/region.hxx>
#endif
#ifndef _SV_MAPMOD_HXX //autogen
#include <vcl/mapmod.hxx>
#endif
#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX //autogen
#include <vcl/bitmapex.hxx>
#endif

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

    BOOL            StartMoving( OutputDevice* pOut,
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
    ULONG           nLastTime;
    BOOL            bClipRegion;

    BOOL            ImplPrepareMoveTo();
    void            ImplDrawSprite( OutputDevice* pOut, const Point& rPt, const Size& rSz );

};

}

#endif
