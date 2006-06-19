/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msashape.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 16:19:09 $
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

#ifndef _MSASHAPE_HXX
#define _MSASHAPE_HXX

#ifndef _MSDFFDEF_HXX
#include <msdffdef.hxx>
#endif
#ifndef _MSDFFIMP_HXX
#include <msdffimp.hxx>
#endif

#define DFF_CUSTOMSHAPE_FLIP_V      1
#define DFF_CUSTOMSHAPE_FLIP_H      2
#define DFF_CUSTOMSHAPE_EXCH            4

class SdrModel;
class SfxItemSet;
struct SvxMSDffVertPair;
struct SvxMSDffCalculationData;
struct SvxMSDffTextRectangles;
class SvxMSDffAdjustmentHandle;
class SvxMSDffCustomShape
{
        MSFilterTracer*             pTracer;
        MSO_SPT                     eSpType;

        sal_Int32                   nCoordWidth;
        sal_Int32                   nCoordHeight;
        Rectangle                   aSnapRect;

        double                      fXScale;
        double                      fYScale;

        sal_Int32                   nFix16Angle;
        sal_Int32                   nXRef;
        sal_Int32                   nYRef;
        sal_uInt32                  nFlags;

        sal_uInt32                  nNumElemVert;
        sal_uInt32                  nNumElemSeg;

        sal_uInt32                  nColorData;

        SvxMSDffVertPair*           pVertData;
        sal_uInt16*                 pSegData;

        sal_uInt32                  nTextRectData;
        SvxMSDffTextRectangles*     pTextRectData;
        sal_uInt32                  nCalculationData;
        SvxMSDffCalculationData*    pCalculationData;
        sal_uInt32                  nAdjustmentHandles;
        SvxMSDffAdjustmentHandle*   pAdjustmentHandles;
        sal_uInt32                  nGluePoints;
        SvxMSDffVertPair*           pGluePoints;

        sal_Bool                    bIsEmpty        : 1;
        sal_Bool                    bVertAlloc      : 1;
        sal_Bool                    bSegAlloc       : 1;
        sal_Bool                    bCalcAlloc      : 1;
        sal_Bool                    bTextRectAlloc  : 1;

        sal_Bool                    bTextFlow       : 1;
        sal_Bool                    bFilled         : 1;
        sal_Bool                    bFlipH          : 1;
        sal_Bool                    bFlipV          : 1;

        sal_Int32                   GetAdjustValue( sal_uInt32 nIndex ) const;
        sal_Int32                   GetAdjustValue( sal_uInt32 nIndex, sal_Int32 nDefault ) const;
        sal_Int32                   Fix16ToAngle( sal_Int32 nAngle ) const;
        Color                       ImplGetColorData( const Color& rFillColor, sal_uInt32 nIndex );
        double                      ImplGetValue( sal_uInt16 nIndex, sal_uInt32& nGeometryFlags ) const;
        Point                       GetPoint( const SvxMSDffVertPair&, sal_Bool bScale = sal_True ) const;

        static void                 SwapStartAndEndArrow( SdrObject* pObj );

    public :

        SdrObject*                  GetObject( SdrModel* pSdrModel, SfxItemSet& rSet, sal_Bool bSetAdjustmentItem );
        sal_Bool                    IsEmpty() const;

        Rectangle                   GetTextRect() const;
        static sal_Bool             HasGluePointList( const MSO_SPT eShapeType );
        static MSO_SPT              GetShapeTypeFromSdrObject( const SdrObject* );

        SvxMSDffCustomShape( const DffPropertyReader& rPropReader, SvStream&,
                            DffObjData& rObjDat, Rectangle& rSnapRect, sal_Int32 nFix16Angle, MSFilterTracer* pTracer );
        ~SvxMSDffCustomShape();
};

#endif

