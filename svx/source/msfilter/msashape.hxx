/*************************************************************************
 *
 *  $RCSfile: msashape.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sj $ $Date: 2001-02-15 18:11:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#define DFF_AUTOSHAPE_FLIP_V        1
#define DFF_AUTOSHAPE_FLIP_H        2
#define DFF_AUTOSHAPE_EXCH          4

class SdrModel;
class SfxItemSet;
struct SvxMSDffVertPair;
struct SvxMSDffCalculationData;
struct SvxMSDffTextRectangles;
class SvxMSDffAdjustmentHandle;
class SvxMSDffAutoShape
{
        MSO_SPT                     eSpType;

        Rectangle                   aSnapRect;
        Rectangle                   aBoundRect;

        double                      fXScale;
        double                      fYScale;

        sal_Int32                   nFix16Angle;
        sal_Int32                   nXRef;
        sal_Int32                   nYRef;
        sal_uInt32                  nFlags;

        sal_uInt32                  nNumElemVert;
        sal_uInt32                  nNumElemSeg;

        SvxMSDffVertPair*           pVertData;
        sal_uInt16*                 pSegData;
        sal_uInt32                  nColorData;

        sal_uInt32                  nTextRectData;
        SvxMSDffTextRectangles*     pTextRectData;
        sal_uInt32                  nCalculationData;
        SvxMSDffCalculationData*    pCalculationData;
        sal_uInt32                  nAdjustmentHandles;
        SvxMSDffAdjustmentHandle*   pAdjustmentHandles;

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

    public :

        SdrObject*                  GetObject( SdrModel* pSdrModel, SfxItemSet& rSet, sal_Bool bSetAdjustmentItem );
        sal_Bool                    IsEmpty() const;

        Rectangle                   GetTextRect() const;

        SvxMSDffAutoShape( const DffPropertyReader& rPropReader, SvStream&,
                            DffObjData& rObjDat, Rectangle& rSnapRect, sal_Int32 nFix16Angle );
        ~SvxMSDffAutoShape();

};

#endif

