/*************************************************************************
 *
 *  $RCSfile: msashape.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2000-10-10 15:35:19 $
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
#include <msashape.hxx>
#endif
#ifndef _SVDOCIRC_HXX
#include <svdocirc.hxx>
#endif
#ifndef _SVDOGRP_HXX
#include <svdogrp.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svdopath.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <xflclit.hxx>
#endif
#ifndef _SDASAITM_HXX
#include <sdasaitm.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif

struct SvxMSDffCalculationData
{
    sal_uInt16  nFlags;
    sal_Int16   nVal[ 3 ];
};

struct mso_AutoShape
{
    sal_Int32*                  pVertices;
    sal_uInt32                  nVertices;
    sal_uInt16*                 pElements;
    sal_uInt32                  nElements;
    SvxMSDffCalculationData*    pCalculation;
    sal_uInt32                  nCalculation;
    sal_Int32*                  pDefData;
    sal_Int32*                  pTextRect;
    sal_Int32*                  pBoundRect;
    sal_Int32                   nXRef;
    sal_Int32                   nYRef;
};

/*
static const sal_Int32 Vert[] =
{
    0,  0,
};
static const sal_uInt16 Segm[] =
{
    0x4000,
    0x8000
};
static const SvxMSDffCalculationData Calc[] =
{
    { 0x0000, 0, 0, 0 }
};
static const sal_Int32 Default[] =
{
    0
};
static const sal_Int32 TextRect[] =
{
    0
};
static const sal_Int32 BoundRect[] =
{
    0, 0, 21600, 21600
};
static const mso_AutoShape mso =
{
    (sal_Int32*)mso_sptVert, sizeof( mso_sptVert ) >> 3,
    (sal_uInt16*)mso_sptSegm, sizeof( mso_sptSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptCalc, sizeof( mso_sptCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault,
    (sal_Int32*)mso_sptTextRect,
    (sal_Int32*)mso_sptBoundRect,
    0x80000000, 0x80000000
};
*/

#define MSO_I | (sal_Int32)0x80000000

static const sal_Int32 mso_sptDefault0[] =
{
    1, 0
};
static const sal_Int32 mso_sptDefault1400[] =
{
    1, 1400
};
static const sal_Int32 mso_sptDefault1800[] =
{
    1, 1800
};
static const sal_Int32 mso_sptDefault2700[] =
{
    1, 2700
};
static const sal_Int32 mso_sptDefault3600[] =
{
    1, 3600
};
static const sal_Int32 mso_sptDefault3700[] =
{
    1, 3700
};
static const sal_Int32 mso_sptDefault5400[] =
{
    1, 5400
};
static const sal_Int32 mso_sptDefault10800[] =
{
    1, 10800
};

static const mso_AutoShape msoRectangle =
{
    NULL, 0,
    NULL, 0,
    NULL, 0,
    (sal_Int32*)mso_sptDefault0,
    NULL,
    NULL,
    0x80000000, 0x80000000
};

static const SvxMSDffCalculationData mso_sptRoundRectangleCalc[] =  // adjustment1 : 0 - 10800
{
    { 0x2001, DFF_Prop_adjustValue, 1, 3 },
    { 0x8000, 21600, 0, 0x400 }
};
static const sal_Int32 mso_sptRoundRectangleTextRect[] =
{
    1, 0 MSO_I, 0 MSO_I, 1 MSO_I, 1 MSO_I
};
static const mso_AutoShape msoRoundRectangle =
{
    NULL, 0,
    NULL, 0,
    (SvxMSDffCalculationData*)mso_sptRoundRectangleCalc, sizeof( mso_sptRoundRectangleCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault3600,
    (sal_Int32*)mso_sptRoundRectangleTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptRightTriangleVert[] =
{
    0, 0, 21600, 21600, 0, 21600
};
static const sal_Int32 mso_sptRightTriangleTextRect[] =
{
    1, 1900, 12700, 12700, 19700
};
static const mso_AutoShape msoRightTriangle =
{
    (sal_Int32*)mso_sptRightTriangleVert, sizeof( mso_sptRightTriangleVert ) >> 3,
    NULL, 0,
    NULL, 0,
    NULL,
    (sal_Int32*)mso_sptRightTriangleTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptEllipseTextRect[] =
{
    1, 3200, 3200, 18400, 18400
};
static const mso_AutoShape msoEllipse =
{
    NULL, 0,
    NULL, 0,
    NULL, 0,
    NULL,
    (sal_Int32*)mso_sptEllipseTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptParallelogramVert[] = // adjustment1 : 0 - 21600
{
    0 MSO_I,    0,
    21600,      0,
    1 MSO_I,    21600,
    0,          21600
};
static const sal_uInt16 mso_sptParallelogramSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptParallelogramCalc[] =
{
    { 0x4000, 0, DFF_Prop_adjustValue, 0 },
    { 0x8000, 0, 21600, DFF_Prop_adjustValue },
    { 0x2001, DFF_Prop_adjustValue, 10, 24 },
    { 0x2000, 0x0402, 1750, 0 },
    { 0x8000, 21600, 0, 0x0403 }
};

static const sal_Int32 mso_sptParallelogramTextRect[] =
{
    1, 3 MSO_I, 3 MSO_I, 4 MSO_I, 4 MSO_I
};
static const mso_AutoShape msoParallelogram =
{
    (sal_Int32*)mso_sptParallelogramVert, sizeof( mso_sptParallelogramVert ) >> 3,
    (sal_uInt16*)mso_sptParallelogramSegm, sizeof( mso_sptParallelogramSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptParallelogramCalc, sizeof( mso_sptParallelogramCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault5400,
    (sal_Int32*)mso_sptParallelogramTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptDiamondVert[] =
{
    10800, 0,
    21600, 10800,
    10800, 21600,
    0,     10800
};
static const sal_Int32 mso_sptDiamondTextRect[] =
{
    1, 5400, 5400, 16200, 16200
};
static const mso_AutoShape msoDiamond =
{
    (sal_Int32*)mso_sptDiamondVert, sizeof( mso_sptDiamondVert ) >> 3,
    NULL, 0,
    NULL, 0,
    NULL,
    (sal_Int32*)mso_sptDiamondTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptTrapezoidVert[] =     // adjustment1 : 0 - 10800
{
    0,      0,
    21600,  0,
    0 MSO_I,21600,
    1 MSO_I,21600

};
static const sal_uInt16 mso_sptTrapezoidSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptTrapezoidCalc[] =
{
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2001, DFF_Prop_adjustValue, 10, 18 },
    { 0x2000, 0x0402, 1750, 0 },
    { 0x8000, 21600, 0, 0x403 }
};
static const sal_Int32 mso_sptTrapezoidTextRect[] =
{
    1, 3 MSO_I, 3 MSO_I, 4 MSO_I, 4 MSO_I
};
static const mso_AutoShape msoTrapezoid =
{
    (sal_Int32*)mso_sptTrapezoidVert, sizeof( mso_sptTrapezoidVert ) >> 3,
    (sal_uInt16*)mso_sptTrapezoidSegm, sizeof( mso_sptTrapezoidSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptTrapezoidCalc, sizeof( mso_sptTrapezoidCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault5400,
    (sal_Int32*)mso_sptTrapezoidTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptOctagonVert[] =       // adjustment1 : 0 - 10800
{
    0 MSO_I,    0,
    1 MSO_I,    0,
    21600,      0 MSO_I,
    21600,      2 MSO_I,
    1 MSO_I,    21600,
    0 MSO_I,    21600,
    0,          2 MSO_I,
    0,          0 MSO_I
};
static const sal_uInt16 mso_sptOctagonSegm[] =
{
    0x4000, 0x0007, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptOctagonCalc[] =
{
    { 0x4000, 0, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x2001, DFF_Prop_adjustValue, 1, 2 },
    { 0xa000, DFF_Prop_geoRight, 0, 0x403 },
    { 0xa000, DFF_Prop_geoBottom, 0, 0x403 }
};
static const sal_Int32 mso_sptOctagonDefault[] =
{
    1, 5000
};
static const sal_Int32 mso_sptOctagonTextRect[] =
{
    1, 3 MSO_I, 3 MSO_I, 4 MSO_I, 5 MSO_I
};
static const mso_AutoShape msoOctagon =
{
    (sal_Int32*)mso_sptOctagonVert, sizeof( mso_sptOctagonVert ) >> 3,
    (sal_uInt16*)mso_sptOctagonSegm, sizeof( mso_sptOctagonSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptOctagonCalc, sizeof( mso_sptOctagonCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptOctagonDefault,
    (sal_Int32*)mso_sptOctagonTextRect,
    NULL,
    10800, 10800
};

static const sal_Int32 mso_sptIsocelesTriangleVert[] =  // adjustment1 : 0 - 21600
{
    0 MSO_I,    0,
    21600,      21600,
    0,          21600
};
static const sal_uInt16 mso_sptIsocelesTriangleSegm[] =
{
    0x4000, 0x0002, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptIsocelesTriangleCalc[] =
{
    { 0x4000, 0, DFF_Prop_adjustValue, 0 },
    { 0x2001, DFF_Prop_adjustValue, 1, 2 },
    { 0x2000, 0x401, 10800, 0 },
    { 0x2001, DFF_Prop_adjustValue, 2, 3 },
    { 0x2000, 0x403, 7200, 0 }

};
static const sal_Int32 mso_sptIsocelesTriangleTextRect[] =
{
    2, 1 MSO_I, 10800, 2 MSO_I, 18000,
    3 MSO_I, 7200, 4 MSO_I, 21600
};
static const mso_AutoShape msoIsocelesTriangle =
{
    (sal_Int32*)mso_sptIsocelesTriangleVert, sizeof( mso_sptIsocelesTriangleVert ) >> 3,
    (sal_uInt16*)mso_sptIsocelesTriangleSegm, sizeof( mso_sptIsocelesTriangleSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptIsocelesTriangleCalc, sizeof( mso_sptIsocelesTriangleCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault10800,
    (sal_Int32*)mso_sptIsocelesTriangleTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptHexagonVert[] =               // adjustment1 : 0 - 10800
{
    0 MSO_I,    0,
    1 MSO_I,    0,
    21600,      10800,
    1 MSO_I,    21600,
    0 MSO_I,    21600,
    0,          10800
};
static const sal_uInt16 mso_sptHexagonSegm[] =
{
    0x4000, 0x0005, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptHexagonCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },
    { 0x2001, DFF_Prop_adjustValue, 100, 234 },
    { 0x2000, 0x402, 1700, 0 },
    { 0x8000, 21600, 0, 0x403 }
};
static const sal_Int32 mso_sptHexagonTextRect[] =
{
    1, 3 MSO_I, 3 MSO_I, 4 MSO_I, 4 MSO_I
};
static const mso_AutoShape msoHexagon =
{
    (sal_Int32*)mso_sptHexagonVert, sizeof( mso_sptHexagonVert ) >> 3,
    (sal_uInt16*)mso_sptHexagonSegm, sizeof( mso_sptHexagonSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptHexagonCalc, sizeof( mso_sptHexagonCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault5400,
    (sal_Int32*)mso_sptHexagonTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};


static const sal_Int32 mso_sptPentagonVert[] =
{
    0, 8260, 10800, 0, 21600, 8260, 17370, 21600, 4230, 21600
};
static const sal_Int32 mso_sptPentagonTextRect[] =
{
    1, 4230, 5080, 17370, 21600
};
static const mso_AutoShape msoPentagon =
{
    (sal_Int32*)mso_sptPentagonVert, sizeof( mso_sptPentagonVert ) >> 3,
    NULL, 0,
    NULL, 0,
    NULL,
    (sal_Int32*)mso_sptPentagonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptPlusVert[] =              // adjustment1 : 0 - 10800
{
    1 MSO_I,0,
    2 MSO_I,0,
    2 MSO_I,1 MSO_I,
    21600,  1 MSO_I,
    21600,  2 MSO_I,
    2 MSO_I,2 MSO_I,
    2 MSO_I,21600,
    1 MSO_I,21600,
    1 MSO_I,2 MSO_I,
    0,      2 MSO_I,
    0,      1 MSO_I,
    1 MSO_I,1 MSO_I
};
static const SvxMSDffCalculationData mso_sptPlusCalc[] =
{
    { 0x2001, DFF_Prop_adjustValue, 10799, 10800 },
    { 0x2000, 0x400, 0, 0 },
    { 0x8000, 21600, 0, 0x400 }
};
static const sal_Int32 mso_sptPlusTextRect[] =
{
    1, 1 MSO_I, 1 MSO_I, 2 MSO_I, 2 MSO_I
};
static const mso_AutoShape msoPlus =
{
    (sal_Int32*)mso_sptPlusVert, sizeof( mso_sptPlusVert ) >> 3,
    NULL, 0,
    (SvxMSDffCalculationData*)mso_sptPlusCalc, sizeof( mso_sptPlusCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault5400,
    (sal_Int32*)mso_sptPlusTextRect,
    (sal_Int32*)NULL,
    10800, 10800
};

static const sal_Int32 mso_sptCanVert[] =       // adjustment1 : 0 - 10800
{
    44,     0,                  // P0
    20,     0,                  // C1
    0,      2 MSO_I,            // C2
    0,      0 MSO_I,            // P3
    0,      3 MSO_I,            // P4
    0,      4 MSO_I,            // C5
    20,     10800,              // C6
    44,     10800,              // P7
    68,     10800,              // C8
    88,     4 MSO_I,            // C9
    88,     3 MSO_I,            // P10
    88,     0 MSO_I,            // P11
    88,     2 MSO_I,            // C12
    68,     0,                  // C13
    44,     0,                  // P14
    44,     0,                  // P0
    20,     0,                  // C1
    0,      2 MSO_I,            // C2
    0,      0 MSO_I,            // P3
    0,      5 MSO_I,            // C4
    20,     6 MSO_I,            // C5
    44,     6 MSO_I,            // P6
    68,     6 MSO_I,            // C7
    88,     5 MSO_I,            // C8
    88,     0 MSO_I,            // P9
    88,     2 MSO_I,            // C10
    68,     0,                  // C11
    44,     0                   // P12
};
static const sal_uInt16 mso_sptCanSegm[] =
{
    0x4000, 0x2001, 0x0001, 0x2002, 0x0001, 0x2001, 0x6001, 0x8000,
    0x4000, 0x2004, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptCanCalc[] =
{
    { 0x2001, DFF_Prop_adjustValue, 1, 4 },     // 1/4
    { 0x2001, 0x0400, 6, 11 },
    { 0xa000,   0x0400, 0, 0x0401 },
    { 0x8000, 10800, 0, 0x0400 },
    { 0x6000, 0x0403, 0x0401, 0 },
    { 0x6000, 0x0400, 0x0401, 0 },
    { 0x2001, DFF_Prop_adjustValue, 1, 2 }
};
static const sal_Int32 mso_sptCanTextRect[] =
{
    1, 0, 6 MSO_I, 88, 3 MSO_I
};
static const sal_Int32 mso_sptCanBoundRect[] =
{
    0, 0, 88, 10800
};
static const mso_AutoShape msoCan =
{
    (sal_Int32*)mso_sptCanVert, sizeof( mso_sptCanVert ) >> 3,
    (sal_uInt16*)mso_sptCanSegm, sizeof( mso_sptCanSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptCanCalc, sizeof( mso_sptCanCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault5400,
    (sal_Int32*)mso_sptCanTextRect,
    (sal_Int32*)mso_sptCanBoundRect,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptArrowVert[] =         // adjustment1: x 0 - 21600
{                                               // adjustment2: y 0 - 10800
    0,      0 MSO_I,
    1 MSO_I,0 MSO_I,
    1 MSO_I,0,
    21600,  10800,
    1 MSO_I,21600,
    1 MSO_I,2 MSO_I,
    0,      2 MSO_I
};
static const sal_uInt16 mso_sptArrowSegm[] =
{
    0x4000, 0x0006, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value }
};
static const sal_Int32 mso_sptArrowDefault[] =
{
    2, 16200, 5400
};
static const sal_Int32 mso_sptArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoArrow =
{
    (sal_Int32*)mso_sptArrowVert, sizeof( mso_sptArrowVert ) >> 3,
    (sal_uInt16*)mso_sptArrowSegm, sizeof( mso_sptArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptArrowCalc, sizeof( mso_sptArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptArrowDefault,
    (sal_Int32*)mso_sptArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptLeftArrowVert[] =     // adjustment1: x 0 - 21600
{                                               // adjustment2: y 0 - 10800
    21600,  0 MSO_I,
    1 MSO_I,0 MSO_I,
    1 MSO_I,0,
    0,      10800,
    1 MSO_I,21600,
    1 MSO_I,2 MSO_I,
    21600,  2 MSO_I
};
static const sal_uInt16 mso_sptLeftArrowSegm[] =
{
    0x4000, 0x0006, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptLeftArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value }
};
static const sal_Int32 mso_sptLeftArrowDefault[] =
{
    2, 5400, 5400
};
static const sal_Int32 mso_sptLeftArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoLeftArrow =
{
    (sal_Int32*)mso_sptLeftArrowVert, sizeof( mso_sptLeftArrowVert ) >> 3,
    (sal_uInt16*)mso_sptLeftArrowSegm, sizeof( mso_sptLeftArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptLeftArrowCalc, sizeof( mso_sptLeftArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptLeftArrowDefault,
    (sal_Int32*)mso_sptLeftArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptDownArrowVert[] =     // adjustment1: x 0 - 21600
{                                               // adjustment2: y 0 - 10800
    0 MSO_I,0,
    0 MSO_I,1 MSO_I,
    0,      1 MSO_I,
    10800,  21600,
    21600,  1 MSO_I,
    2 MSO_I,1 MSO_I,
    2 MSO_I,0
};
static const sal_uInt16 mso_sptDownArrowSegm[] =
{
    0x4000, 0x0006, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptDownArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value }
};
static const sal_Int32 mso_sptDownArrowDefault[] =
{
    2, 16200, 5400
};
static const sal_Int32 mso_sptDownArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoDownArrow =
{
    (sal_Int32*)mso_sptDownArrowVert, sizeof( mso_sptDownArrowVert ) >> 3,
    (sal_uInt16*)mso_sptDownArrowSegm, sizeof( mso_sptDownArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptDownArrowCalc, sizeof( mso_sptDownArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDownArrowDefault,
    (sal_Int32*)mso_sptDownArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptUpArrowVert[] =   // adjustment1: x 0 - 21600
{                                           // adjustment2: y 0 - 10800
    0 MSO_I,21600,
    0 MSO_I,1 MSO_I,
    0,      1 MSO_I,
    10800,  0,
    21600,  1 MSO_I,
    2 MSO_I,1 MSO_I,
    2 MSO_I,21600
};
static const sal_uInt16 mso_sptUpArrowSegm[] =
{
    0x4000, 0x0006, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptUpArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value }
};
static const sal_Int32 mso_sptUpArrowDefault[] =
{
    2, 5400, 5400
};
static const sal_Int32 mso_sptUpArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoUpArrow =
{
    (sal_Int32*)mso_sptUpArrowVert, sizeof( mso_sptUpArrowVert ) >> 3,
    (sal_uInt16*)mso_sptUpArrowSegm, sizeof( mso_sptUpArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptUpArrowCalc, sizeof( mso_sptUpArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptUpArrowDefault,
    (sal_Int32*)mso_sptUpArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptLeftRightArrowVert[] =    // adjustment1: x 0 - 10800
{                                                   // adjustment2: y 0 - 10800
    0,      10800,
    0 MSO_I,0,
    0 MSO_I,1 MSO_I,
    2 MSO_I,1 MSO_I,
    2 MSO_I,0,
    21600,10800,
    2 MSO_I,21600,
    2 MSO_I,3 MSO_I,
    0 MSO_I,3 MSO_I,
    0 MSO_I,21600
};
static const sal_uInt16 mso_sptLeftRightArrowSegm[] =
{
    0x4000, 0x0009, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptLeftRightArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value }
};
static const sal_Int32 mso_sptLeftRightArrowDefault[] =
{
    2, 4300, 5400
};
static const sal_Int32 mso_sptLeftRightArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoLeftRightArrow =
{
    (sal_Int32*)mso_sptLeftRightArrowVert, sizeof( mso_sptLeftRightArrowVert ) >> 3,
    (sal_uInt16*)mso_sptLeftRightArrowSegm, sizeof( mso_sptLeftRightArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptLeftRightArrowCalc, sizeof( mso_sptLeftRightArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptLeftRightArrowDefault,
    (sal_Int32*)mso_sptLeftRightArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptUpDownArrowVert[] =   // adjustment1: x 0 - 10800
{                                               // adjustment2: y 0 - 10800
    0,      0 MSO_I,
    10800,  0,
    21600,  0 MSO_I,
    2 MSO_I,0 MSO_I,
    2 MSO_I,3 MSO_I,
    21600,  3 MSO_I,
    10800,  21600,
    0,      3 MSO_I,
    1 MSO_I,3 MSO_I,
    1 MSO_I,0 MSO_I
};
static const sal_uInt16 mso_sptUpDownArrowSegm[] =
{
    0x4000, 0x0009, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptUpDownArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value }
};
static const sal_Int32 mso_sptUpDownArrowDefault[] =
{
    2, 5400, 4300
};
static const sal_Int32 mso_sptUpDownArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoUpDownArrow =
{
    (sal_Int32*)mso_sptUpDownArrowVert, sizeof( mso_sptUpDownArrowVert ) >> 3,
    (sal_uInt16*)mso_sptUpDownArrowSegm, sizeof( mso_sptUpDownArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptUpDownArrowCalc, sizeof( mso_sptUpDownArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptUpDownArrowDefault,
    (sal_Int32*)mso_sptUpDownArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptQuadArrowVert[] =     // adjustment1: x 0 - 10800
{                                               // adjustment2: x 0 - 10800
    0,      10800,                              // adjustment3: y 0 - 10800
    0 MSO_I,1 MSO_I,                // P1
    0 MSO_I,2 MSO_I,                // P2
    2 MSO_I,2 MSO_I,                // P3
    2 MSO_I,0 MSO_I,                // P4
    1 MSO_I,0 MSO_I,                // P5
    10800,  0,                      // P6
    3 MSO_I,0 MSO_I,                // P7
    4 MSO_I,0 MSO_I,                // P8
    4 MSO_I,2 MSO_I,                // P9
    5 MSO_I,2 MSO_I,                // P10
    5 MSO_I,1 MSO_I,                // P11
    21600,  10800,                  // P12
    5 MSO_I,3 MSO_I,                // P13
    5 MSO_I,4 MSO_I,                // P14
    4 MSO_I,4 MSO_I,                // P15
    4 MSO_I,5 MSO_I,                // P16
    3 MSO_I,5 MSO_I,                // P17
    10800,  21600,                  // P18
    1 MSO_I,5 MSO_I,                // P19
    2 MSO_I,5 MSO_I,                // P20
    2 MSO_I,4 MSO_I,                // P21
    0 MSO_I,4 MSO_I,                // P22
    0 MSO_I,3 MSO_I                 // P23
};
static const sal_uInt16 mso_sptQuadArrowSegm[] =
{
    0x4000, 0x0017, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptQuadArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value },
    { 0x8000, 21600, 0, DFF_Prop_adjust3Value }
};
static const sal_Int32 mso_sptQuadArrowDefault[] =
{
    3, 6500, 8600, 4300
};
static const sal_Int32 mso_sptQuadArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoQuadArrow =
{
    (sal_Int32*)mso_sptQuadArrowVert, sizeof( mso_sptQuadArrowVert ) >> 3,
    (sal_uInt16*)mso_sptQuadArrowSegm, sizeof( mso_sptQuadArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptQuadArrowCalc, sizeof( mso_sptQuadArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptQuadArrowDefault,
    (sal_Int32*)mso_sptQuadArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptLeftRightUpArrowVert[] =  // adjustment1: x 0 - 10800
{                                                   // adjustment2: x 0 - 10800
    10800,  0,                                      // adjustment3: y 0 - 21600
    3 MSO_I,2 MSO_I,                // P1
    4 MSO_I,2 MSO_I,                // P2
    4 MSO_I,1 MSO_I,                // P3
    5 MSO_I,1 MSO_I,                // P4
    5 MSO_I,0 MSO_I,                // P5
    21600,  10800,                  // P6
    5 MSO_I,3 MSO_I,                // P7
    5 MSO_I,4 MSO_I,                // P8
    2 MSO_I,4 MSO_I,                // P9
    2 MSO_I,3 MSO_I,                // P10
    0,      10800,                  // P11
    2 MSO_I,0 MSO_I,                // P12
    2 MSO_I,1 MSO_I,                // P13
    1 MSO_I,1 MSO_I,                // P14
    1 MSO_I,2 MSO_I,                // P15
    0 MSO_I,2 MSO_I                 // P16
};
static const sal_uInt16 mso_sptLeftRightUpArrowSegm[] =
{
    0x4000, 0x0010, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptLeftRightUpArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },             // 0
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },            // 1
    { 0x6001, 0x0403, DFF_Prop_adjust3Value, 21600 },   // 2
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },         // 3
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value },        // 4
    { 0x8000, 21600, 0, 0x0402 }                        // 5
};
static const sal_Int32 mso_sptLeftRightUpArrowDefault[] =
{
    3, 6500, 8600, 6200
};
static const sal_Int32 mso_sptLeftRightUpArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoLeftRightUpArrow =
{
    (sal_Int32*)mso_sptLeftRightUpArrowVert, sizeof( mso_sptLeftRightUpArrowVert ) >> 3,
    (sal_uInt16*)mso_sptLeftRightUpArrowSegm, sizeof( mso_sptLeftRightUpArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptLeftRightUpArrowCalc, sizeof( mso_sptLeftRightUpArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptLeftRightUpArrowDefault,
    (sal_Int32*)mso_sptLeftRightUpArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptBentArrowVert[] =     // adjustment1 : x 12427 - 21600
{                                               // adjustment2 : y 0 - 6079
    0,      21600,                              // P0
    0,      12160,                              // P1
    12427,  1 MSO_I,
    0 MSO_I,1 MSO_I,
    0 MSO_I,0,
    21600,  6079,
    0 MSO_I,12158,
    0 MSO_I,2 MSO_I,
    12427,  2 MSO_I,
    4 MSO_I,12160,
    4 MSO_I,21600
};
static const sal_uInt16 mso_sptBentArrowSegm[] =
{
    0x4000, 0x0001, 0xa801, 0x0006, 0xa701, 0x0001, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptBentArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x8000,   12158, 0, DFF_Prop_adjust2Value },
    { 0x8000, 6079, 0, DFF_Prop_adjust2Value },
    { 0x2001,   0x0403, 2, 1 }
};
static const sal_Int32 mso_sptBentArrowDefault[] =
{
    2, 15100, 2900
};
static const sal_Int32 mso_sptBentArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoBentArrow =
{
    (sal_Int32*)mso_sptBentArrowVert, sizeof( mso_sptBentArrowVert ) >> 3,
    (sal_uInt16*)mso_sptBentArrowSegm, sizeof( mso_sptBentArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptBentArrowCalc, sizeof( mso_sptBentArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptBentArrowDefault,
    (sal_Int32*)mso_sptBentArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptLeftUpArrowVert[] =   // adjustment1 : x 0 - 21600
{                                               // adjustment2 : x 0 - 21600
    0,      5 MSO_I,                            // adjustment3 : y 0 - 21600
    2 MSO_I,0 MSO_I,                // P1
    2 MSO_I,7 MSO_I,                // P2
    7 MSO_I,7 MSO_I,                // P3
    7 MSO_I,2 MSO_I,                // P4
    0 MSO_I,2 MSO_I,                // P5
    5 MSO_I,0,                      // P6
    21600,2 MSO_I,                  // P7
    1 MSO_I,2 MSO_I,                // P8
    1 MSO_I,1 MSO_I,                // P9
    2 MSO_I,1 MSO_I,                // P10
    2 MSO_I,21600                   // P11
};
static const sal_uInt16 mso_sptLeftUpArrowSegm[] =
{
    0x4000, 0x000b, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptLeftUpArrowCalc[] =
{
    { 0x2000,   DFF_Prop_adjustValue, 0, 0 },       // 0
    { 0x2000,   DFF_Prop_adjust2Value, 0, 0 },      // 1
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },        // 2
    { 0x8000,   21600, 0, DFF_Prop_adjustValue },   // 3
    { 0x2001,   0x0403, 1, 2 },                     // 4
    { 0x6000, DFF_Prop_adjustValue, 0x0404, 0 },    // 5
    { 0x8000,   21600, 0, DFF_Prop_adjust2Value },  // 6
    { 0x6000, DFF_Prop_adjustValue, 0x0406, 0 }     // 7
};
static const sal_Int32 mso_sptLeftUpArrowDefault[] =
{
    3, 9340, 18500, 6200
};
static const sal_Int32 mso_sptLeftUpArrowTextRect[] =
{
    2,  2 MSO_I, 7 MSO_I, 1 MSO_I, 1 MSO_I,
        7 MSO_I, 2 MSO_I, 1 MSO_I, 1 MSO_I
};
static const mso_AutoShape msoLeftUpArrow =
{
    (sal_Int32*)mso_sptLeftUpArrowVert, sizeof( mso_sptLeftUpArrowVert ) >> 3,
    (sal_uInt16*)mso_sptLeftUpArrowSegm, sizeof( mso_sptLeftUpArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptLeftUpArrowCalc, sizeof( mso_sptLeftUpArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptLeftUpArrowDefault,
    (sal_Int32*)mso_sptLeftUpArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptBentUpArrowVert[] =       // adjustment1 : x 0 - 21600
{                                           // adjustment2 : x 0 - 21600
    0,      8 MSO_I,                // adjustment3 : y 0 - 21600
    7 MSO_I,8 MSO_I,                // P1
    7 MSO_I,2 MSO_I,                // P2
    0 MSO_I,2 MSO_I,                // P3
    5 MSO_I,0,                      // P4
    21600,  2 MSO_I,                // P5
    1 MSO_I,2 MSO_I,                // P6
    1 MSO_I,21600,                  // P7
    0,      21600                   // P8
};
static const sal_uInt16 mso_sptBentUpArrowSegm[] =
{
    0x4000, 0x0008, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptBentUpArrowCalc[] =
{
    { 0x2000,   DFF_Prop_adjustValue, 0, 0 },       // 0
    { 0x2000,   DFF_Prop_adjust2Value, 0, 0 },      // 1
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },        // 2
    { 0x8000,   21600, 0, DFF_Prop_adjustValue },   // 3
    { 0x2001,   0x0403, 1, 2 },                     // 4
    { 0x6000, DFF_Prop_adjustValue, 0x0404, 0 },    // 5
    { 0x8000,   21600, 0, DFF_Prop_adjust2Value },  // 6
    { 0x6000, DFF_Prop_adjustValue, 0x0406, 0 },    // 7
    { 0x6000, 0x0407, 0x0406, 0 }                   // 8
};
static const sal_Int32 mso_sptBentUpArrowDefault[] =
{
    3, 9340, 18500, 7200
};
static const sal_Int32 mso_sptBentUpArrowTextRect[] =
{
    2,  2 MSO_I, 7 MSO_I, 1 MSO_I, 1 MSO_I,
        7 MSO_I, 2 MSO_I, 1 MSO_I, 1 MSO_I
};
static const mso_AutoShape msoBentUpArrow =
{
    (sal_Int32*)mso_sptBentUpArrowVert, sizeof( mso_sptBentUpArrowVert ) >> 3,
    (sal_uInt16*)mso_sptBentUpArrowSegm, sizeof( mso_sptBentUpArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptBentUpArrowCalc, sizeof( mso_sptBentUpArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptBentUpArrowDefault,
    (sal_Int32*)mso_sptBentUpArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptCurvedRightArrowVert[] =  // adjustment1 : y 10800 - 21600    !!!!!!!!
{                                               // adjustment2 : y 16424 - 21600
    21600,  0,                                  // adjustment3 : x 0 - 21600
    9675,   0,                                  // C1
    0,      10 MSO_I,                           // C2
    0,      9 MSO_I,                            // P3
    0,      11 MSO_I,                           // P4
    0,      14 MSO_I,                           // C5
    15 MSO_I,1 MSO_I,                           // C6
    2 MSO_I,1 MSO_I,                            // P7
    2 MSO_I,21600,                              // P8
    21600,  7 MSO_I,                            // P9
    2 MSO_I,0 MSO_I,                            // P10
    2 MSO_I,16 MSO_I,                           // P11
    2 MSO_I,16 MSO_I,                           // C12
    80,     8 MSO_I,                            // C13
    80,     8 MSO_I,                            // P14
    80,     8 MSO_I,                            // C16
    21600,  5 MSO_I,                            // C17
    21600,  0                                   // P18
};
static const sal_uInt16 mso_sptCurvedRightArrowSegm[] =
{
    0x4000, 0x2001, 0x0001, 0x2001, 0x0004, 0x2002, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptCurvedRightArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },                     // 0
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },                    // 1
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },                    // 2
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value },                // 3
    { 0xa000, DFF_Prop_adjust2Value, 0, DFF_Prop_adjustValue }, // 4
    { 0xa000, 0x0404, 0, 0x0403 },                              // 5
    { 0x2001, 0x0405, 1, 2 },                                   // 6
    { 0xa000, DFF_Prop_adjust2Value, 0, 0x0406 },               // 7
    { 0x2001, DFF_Prop_adjust2Value, 1, 2 },                    // 8
    { 0xa000, 0x0408, 0, 0x0406 },                              // 9
    { 0x2001, 0x0409, 10000, 22326 },                           // 10
    { 0x6000, 0x0409, 0x0405, 0 },                              // 11
    { 0xa000, DFF_Prop_adjust2Value, 0, 0x040b },               // 12
    { 0x2001, 0x040c, 10000, 23148 },                           // 13
    { 0x6000, 0x040d, 0x040b, 0 },                              // 14
    { 0x2001, DFF_Prop_adjust3Value, 10000, 25467 },            // 15
    { 0x6000, DFF_Prop_adjustValue, 0x0403, 0 }                 // 16
};
static const sal_Int32 mso_sptCurvedRightArrowDefault[] =
{
    3, 13000, 19400, 14400
};
static const sal_Int32 mso_sptCurvedRightArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoCurvedRightArrow =
{
    (sal_Int32*)mso_sptCurvedRightArrowVert, sizeof( mso_sptCurvedRightArrowVert ) >> 3,
    (sal_uInt16*)mso_sptCurvedRightArrowSegm, sizeof( mso_sptCurvedRightArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptCurvedRightArrowCalc, sizeof( mso_sptCurvedRightArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptCurvedRightArrowDefault,
    (sal_Int32*)mso_sptCurvedRightArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptStripedRightArrowVert[] =             // adjustment1 : x 3375 - 21600
{                                                           // adjustment2 : y 0 - 10800
    3375,   0 MSO_I,
    1 MSO_I,0 MSO_I,
    1 MSO_I,0,
    21600,  10800,
    1 MSO_I,21600,
    1 MSO_I,2 MSO_I,
    3375,   2 MSO_I,
    0,      0 MSO_I,
    675,    0 MSO_I,
    675,    2 MSO_I,
    0,      2 MSO_I,
    1350,   0 MSO_I,
    2700,   0 MSO_I,
    2700,   2 MSO_I,
    1350,   2 MSO_I
};
static const sal_uInt16 mso_sptStripedRightArrowSegm[] =
{
    0x4000, 0x0006, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptStripedRightArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjust2Value }
};
static const sal_Int32 mso_sptStripedRightArrowDefault[] =
{
    2, 16200, 5400
};
static const sal_Int32 mso_sptStripedRightArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoStripedRightArrow =
{
    (sal_Int32*)mso_sptStripedRightArrowVert, sizeof( mso_sptStripedRightArrowVert ) >> 3,
    (sal_uInt16*)mso_sptStripedRightArrowSegm, sizeof( mso_sptStripedRightArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptStripedRightArrowCalc, sizeof( mso_sptStripedRightArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptStripedRightArrowDefault,
    (sal_Int32*)mso_sptStripedRightArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptHomePlateVert[] =                 // adjustment1 : x 0 - 21600
{
    0,      0,
    0 MSO_I,0,
    21600,10800,
    0 MSO_I,21600,
    0,      21600
};
static const sal_uInt16 mso_sptHomePlateSegm[] =
{
    0x4000, 0x0004, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptHomePlateCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 }
};
static const sal_Int32 mso_sptHomePlateDefault[] =
{
    1, 16200
};
static const sal_Int32 mso_sptHomePlateTextRect[] =
{
    0
};
static const mso_AutoShape msoHomePlate =
{
    (sal_Int32*)mso_sptHomePlateVert, sizeof( mso_sptHomePlateVert ) >> 3,
    (sal_uInt16*)mso_sptHomePlateSegm, sizeof( mso_sptHomePlateSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptHomePlateCalc, sizeof( mso_sptHomePlateCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptHomePlateDefault,
    (sal_Int32*)mso_sptHomePlateTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptChevronVert[] =                       // adjustment1 : x 0 - 21600
{
    0,      0,
    0 MSO_I,0,
    21600,10800,
    0 MSO_I,21600,
    0,      21600,
    1 MSO_I,10800
};
static const sal_uInt16 mso_sptChevronSegm[] =
{
    0x4000, 0x0005, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptChevronCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, 0x0400 }
};
static const sal_Int32 mso_sptChevronDefault[] =
{
    1, 16200
};
static const sal_Int32 mso_sptChevronTextRect[] =
{
    0
};
static const mso_AutoShape msoChevron =
{
    (sal_Int32*)mso_sptChevronVert, sizeof( mso_sptChevronVert ) >> 3,
    (sal_uInt16*)mso_sptChevronSegm, sizeof( mso_sptChevronSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptChevronCalc, sizeof( mso_sptChevronCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptChevronDefault,
    (sal_Int32*)mso_sptChevronTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptRightArrowCalloutVert[] =     // adjustment1 : x 0 - 21000
{                                                       // adjustment2 : y 0 - 10800
    0,      0,
    0 MSO_I,0,
    0 MSO_I,3 MSO_I,
    2 MSO_I,3 MSO_I,
    2 MSO_I,1 MSO_I,
    21600,  10800,
    2 MSO_I,4 MSO_I,
    2 MSO_I,5 MSO_I,
    0 MSO_I,5 MSO_I,
    0 MSO_I,21600,
    0,      21600
};
static const sal_uInt16 mso_sptRightArrowCalloutSegm[] =
{
    0x4000, 0x000a, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptRightArrowCalloutCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust4Value, 0, 0 },
    { 0x8000, 21600, 0, 0x0401 },
    { 0x8000, 21600, 0, 0x0403 }
};
static const sal_Int32 mso_sptRightArrowCalloutDefault[] =
{
    4, 14400, 5400, 18000, 8100
};
static const sal_Int32 mso_sptRightArrowCalloutTextRect[] =
{
    1, 0, 0, 0 MSO_I, 21600
};
static const mso_AutoShape msoRightArrowCallout =
{
    (sal_Int32*)mso_sptRightArrowCalloutVert, sizeof( mso_sptRightArrowCalloutVert ) >> 3,
    (sal_uInt16*)mso_sptRightArrowCalloutSegm, sizeof( mso_sptRightArrowCalloutSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptRightArrowCalloutCalc, sizeof( mso_sptRightArrowCalloutCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptRightArrowCalloutDefault,
    (sal_Int32*)mso_sptRightArrowCalloutTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptLeftArrowCalloutVert[] =  // adjustment1 : x 0 - 21600
{                                                       // adjustment2 : y 0 - 10800
    0 MSO_I,0,                                          // adjustment3 : x 0 - 21600
    21600,  0,                                          // adjustment4 : y 0 - 10800
    21600,  21600,
    0 MSO_I,21600,
    0 MSO_I,5 MSO_I,
    2 MSO_I,5 MSO_I,
    2 MSO_I,4 MSO_I,
    0,      10800,
    2 MSO_I,1 MSO_I,
    2 MSO_I,3 MSO_I,
    0 MSO_I,3 MSO_I
};
static const sal_uInt16 mso_sptLeftArrowCalloutSegm[] =
{
    0x4000, 0x000a, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptLeftArrowCalloutCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust4Value, 0, 0 },
    { 0x8000, 21600, 0, 0x0401 },
    { 0x8000, 21600, 0, 0x0403 }
};
static const sal_Int32 mso_sptLeftArrowCalloutDefault[] =
{
    4, 7200, 5400, 3600, 8100
};
static const sal_Int32 mso_sptLeftArrowCalloutTextRect[] =
{
    1, 0 MSO_I, 0, 21600, 21600
};
static const mso_AutoShape msoLeftArrowCallout =
{
    (sal_Int32*)mso_sptLeftArrowCalloutVert, sizeof( mso_sptLeftArrowCalloutVert ) >> 3,
    (sal_uInt16*)mso_sptLeftArrowCalloutSegm, sizeof( mso_sptLeftArrowCalloutSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptLeftArrowCalloutCalc, sizeof( mso_sptLeftArrowCalloutCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptLeftArrowCalloutDefault,
    (sal_Int32*)mso_sptLeftArrowCalloutTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptUpArrowCalloutVert[] =
{
    21600,  0 MSO_I,
    21600,  21600,
    0,      21600,
    0,      0 MSO_I,
    3 MSO_I,0 MSO_I,
    3 MSO_I,2 MSO_I,
    1 MSO_I,2 MSO_I,
    10800,  0,
    4 MSO_I,2 MSO_I,
    5 MSO_I,2 MSO_I,
    5 MSO_I,0 MSO_I
};
static const sal_uInt16 mso_sptUpArrowCalloutSegm[] =
{
    0x4000, 0x000a, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptUpArrowCalloutCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust4Value, 0, 0 },
    { 0x8000, 21600, 0, 0x0401 },
    { 0x8000, 21600, 0, 0x0403 }
};
static const sal_Int32 mso_sptUpArrowCalloutDefault[] =
{
    4, 7200, 5400, 3600, 8100
};
static const sal_Int32 mso_sptUpArrowCalloutTextRect[] =
{
    1,  0, 0 MSO_I, 21600, 21600
};
static const mso_AutoShape msoUpArrowCallout =
{
    (sal_Int32*)mso_sptUpArrowCalloutVert, sizeof( mso_sptUpArrowCalloutVert ) >> 3,
    (sal_uInt16*)mso_sptUpArrowCalloutSegm, sizeof( mso_sptUpArrowCalloutSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptUpArrowCalloutCalc, sizeof( mso_sptUpArrowCalloutCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptUpArrowCalloutDefault,
    (sal_Int32*)mso_sptUpArrowCalloutTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptDownArrowCalloutVert[] =
{
    0,      0 MSO_I,
    0,      0,
    21600,  0,
    21600,  0 MSO_I,
    5 MSO_I,0 MSO_I,
    5 MSO_I,2 MSO_I,
    4 MSO_I,2 MSO_I,
    10800,  21600,
    1 MSO_I,2 MSO_I,
    3 MSO_I,2 MSO_I,
    3 MSO_I,0 MSO_I
};
static const sal_uInt16 mso_sptDownArrowCalloutSegm[] =
{
    0x4000, 0x000a, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptDownArrowCalloutCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust4Value, 0, 0 },
    { 0x8000, 21600, 0, 0x0401 },
    { 0x8000, 21600, 0, 0x0403 }
};
static const sal_Int32 mso_sptDownArrowCalloutDefault[] =
{
    4, 14400, 5400, 18000, 8100
};
static const sal_Int32 mso_sptDownArrowCalloutTextRect[] =
{
    1, 0, 0, 21600, 0 MSO_I
};
static const mso_AutoShape msoDownArrowCallout =
{
    (sal_Int32*)mso_sptDownArrowCalloutVert, sizeof( mso_sptDownArrowCalloutVert ) >> 3,
    (sal_uInt16*)mso_sptDownArrowCalloutSegm, sizeof( mso_sptDownArrowCalloutSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptDownArrowCalloutCalc, sizeof( mso_sptDownArrowCalloutCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDownArrowCalloutDefault,
    (sal_Int32*)mso_sptDownArrowCalloutTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptLeftRightArrowCalloutVert[] =
{
    0 MSO_I,0,
    4 MSO_I,0,
    4 MSO_I,3 MSO_I,
    6 MSO_I,3 MSO_I,
    6 MSO_I,1 MSO_I,
    21600,  10800,
    6 MSO_I,5 MSO_I,
    6 MSO_I,7 MSO_I,
    4 MSO_I,7 MSO_I,
    4 MSO_I,21600,
    0 MSO_I,21600,
    0 MSO_I,7 MSO_I,
    2 MSO_I,7 MSO_I,
    2 MSO_I,5 MSO_I,
    0,      10800,
    2 MSO_I,1 MSO_I,
    2 MSO_I,3 MSO_I,
    0 MSO_I,3 MSO_I
};
static const sal_uInt16 mso_sptLeftRightArrowCalloutSegm[] =
{
    0x4000, 0x0011, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptLeftRightArrowCalloutCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust4Value, 0, 0 },
    { 0x8000, 21600, 0, 0x0400 },
    { 0x8000, 21600, 0, 0x0401 },
    { 0x8000, 21600, 0, 0x0402 },
    { 0x8000, 21600, 0, 0x0403 }
};
static const sal_Int32 mso_sptLeftRightArrowCalloutDefault[] =
{
    4, 5400, 5500, 2700, 8100
};
static const sal_Int32 mso_sptLeftRightArrowCalloutTextRect[] =
{
    1, 0 MSO_I, 0, 4 MSO_I, 21600
};
static const mso_AutoShape msoLeftRightArrowCallout =
{
    (sal_Int32*)mso_sptLeftRightArrowCalloutVert, sizeof( mso_sptLeftRightArrowCalloutVert ) >> 3,
    (sal_uInt16*)mso_sptLeftRightArrowCalloutSegm, sizeof( mso_sptLeftRightArrowCalloutSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptLeftRightArrowCalloutCalc, sizeof( mso_sptLeftRightArrowCalloutCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptLeftRightArrowCalloutDefault,
    (sal_Int32*)mso_sptLeftRightArrowCalloutTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptUpDownArrowCalloutVert[] =
{
    0,      0 MSO_I,
    0,      4 MSO_I,
    3 MSO_I,4 MSO_I,
    3 MSO_I,6 MSO_I,
    1 MSO_I,6 MSO_I,
    10800,  21600,
    5 MSO_I,6 MSO_I,
    7 MSO_I,6 MSO_I,
    7 MSO_I,4 MSO_I,
    21600,  4 MSO_I,
    21600,  0 MSO_I,
    7 MSO_I,0 MSO_I,
    7 MSO_I,2 MSO_I,
    5 MSO_I,2 MSO_I,
    10800,  0,
    1 MSO_I,2 MSO_I,
    3 MSO_I,2 MSO_I,
    3 MSO_I,0 MSO_I
};
static const sal_uInt16 mso_sptUpDownArrowCalloutSegm[] =
{
    0x4000, 0x0011, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptUpDownArrowCalloutCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust4Value, 0, 0 },
    { 0x8000, 21600, 0, 0x0400 },
    { 0x8000, 21600, 0, 0x0401 },
    { 0x8000, 21600, 0, 0x0402 },
    { 0x8000, 21600, 0, 0x0403 }
};
static const sal_Int32 mso_sptUpDownArrowCalloutDefault[] =
{
    4, 5400, 5500, 2700, 8100
};
static const sal_Int32 mso_sptUpDownArrowCalloutTextRect[] =
{
    1, 0, 0 MSO_I, 21600, 4 MSO_I
};
static const mso_AutoShape msoUpDownArrowCallout =
{
    (sal_Int32*)mso_sptUpDownArrowCalloutVert, sizeof( mso_sptUpDownArrowCalloutVert ) >> 3,
    (sal_uInt16*)mso_sptUpDownArrowCalloutSegm, sizeof( mso_sptUpDownArrowCalloutSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptUpDownArrowCalloutCalc, sizeof( mso_sptUpDownArrowCalloutCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptUpDownArrowCalloutDefault,
    (sal_Int32*)mso_sptUpDownArrowCalloutTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptQuadArrowCalloutVert[] =
{
    0 MSO_I,0 MSO_I,                    // P0
    3 MSO_I,0 MSO_I,                    // P1
    3 MSO_I,2 MSO_I,                    // P2
    1 MSO_I,2 MSO_I,                    // P3
    10800,  0,                          // P4
    5 MSO_I,2 MSO_I,                    // P5
    7 MSO_I,2 MSO_I,                    // P6
    7 MSO_I,0 MSO_I,                    // P7
    4 MSO_I,0 MSO_I,                    // P8
    4 MSO_I,3 MSO_I,                    // P9
    6 MSO_I,3 MSO_I,                    // P10
    6 MSO_I,1 MSO_I,                    // P11
    21600,  10800,                      // P12
    6 MSO_I,5 MSO_I,                    // P13
    6 MSO_I,7 MSO_I,                    // P14
    4 MSO_I,7 MSO_I,                    // P15
    4 MSO_I,4 MSO_I,                    // P16
    7 MSO_I,4 MSO_I,                    // P17
    7 MSO_I,6 MSO_I,                    // P18
    5 MSO_I,6 MSO_I,                    // P19
    10800,  21600,                      // P20
    1 MSO_I,6 MSO_I,                    // P21
    3 MSO_I,6 MSO_I,                    // P22
    3 MSO_I,4 MSO_I,                    // P23
    0 MSO_I,4 MSO_I,                    // P24
    0 MSO_I,7 MSO_I,                    // P25
    2 MSO_I,7 MSO_I,                    // P26
    2 MSO_I,5 MSO_I,                    // P27
    0, 10800,                           // P28
    2 MSO_I,1 MSO_I,                    // P29
    2 MSO_I,3 MSO_I,                    // P30
    0 MSO_I,3 MSO_I                     // P31
};
static const sal_uInt16 mso_sptQuadArrowCalloutSegm[] =
{
    0x4000, 0x001f, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptQuadArrowCalloutCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },
    { 0x2000, DFF_Prop_adjust4Value, 0, 0 },
    { 0x8000, 21600, 0, 0x0400 },
    { 0x8000, 21600, 0, 0x0401 },
    { 0x8000, 21600, 0, 0x0402 },
    { 0x8000, 21600, 0, 0x0403 }
};
static const sal_Int32 mso_sptQuadArrowCalloutDefault[] =
{
    4, 5400, 8100, 2700, 9400
};
static const sal_Int32 mso_sptQuadArrowCalloutTextRect[] =
{
    1, 0 MSO_I, 0 MSO_I, 4 MSO_I, 4 MSO_I
};
static const mso_AutoShape msoQuadArrowCallout =
{
    (sal_Int32*)mso_sptQuadArrowCalloutVert, sizeof( mso_sptQuadArrowCalloutVert ) >> 3,
    (sal_uInt16*)mso_sptQuadArrowCalloutSegm, sizeof( mso_sptQuadArrowCalloutSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptQuadArrowCalloutCalc, sizeof( mso_sptQuadArrowCalloutCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptQuadArrowCalloutDefault,
    (sal_Int32*)mso_sptQuadArrowCalloutTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptCircularArrowVert[] =                 //!!!!!!!!!!!!!!!!!
{
    3 MSO_I, 3 MSO_I,
    20 MSO_I,20 MSO_I,
    19 MSO_I,18 MSO_I,
    17 MSO_I,16 MSO_I,
    0,      0,
    21600,  21600,
    9 MSO_I,8 MSO_I,
    11 MSO_I,10 MSO_I,
    24 MSO_I,23 MSO_I,
    40 MSO_I,39 MSO_I,
    29 MSO_I,28 MSO_I
};
static const sal_uInt16 mso_sptCircularArrowSegm[] =
{
    0xa404, 0xa504, 0x0003, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptCircularArrowCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },         // 0
    { 0x2000, DFF_Prop_adjust2Value, 0, 0 },        // 1
    { 0x2000, DFF_Prop_adjust3Value, 0, 0 },        // 2
    { 0x8000, 10800, 0,DFF_Prop_adjust3Value },     // 3
    { 0x4009, 10800, DFF_Prop_adjustValue, 0 },     // 4
    { 0x400a, 10800, DFF_Prop_adjustValue, 0 },     // 5
    { 0x4009, 10800, DFF_Prop_adjust2Value, 0 },    // 6
    { 0x400a, 10800, DFF_Prop_adjust2Value, 0 },    // 7
    { 0x2000, 0x0404, 10800, 0 },                   // 8
    { 0x2000, 0x0405, 10800, 0 },                   // 9
    { 0x2000, 0x0406, 10800, 0 },                   // 10
    { 0x2000, 0x0407, 10800, 0 },                   // 11
    { 0x6009, 0x0403, DFF_Prop_adjustValue, 0 },    // 12
    { 0x600a, 0x0403, DFF_Prop_adjustValue, 0 },    // 13
    { 0x6009, 0x0403, DFF_Prop_adjust2Value, 0 },   // 14
    { 0x600a, 0x0403, DFF_Prop_adjust2Value, 0 },   // 15
    { 0x2000, 0x040c, 10800, 0 },                   // 16
    { 0x2000, 0x040d, 10800, 0 },                   // 17
    { 0x2000, 0x040e, 10800, 0 },                   // 18
    { 0x2000, 0x040f, 10800, 0 },                   // 19
    { 0x8000, 21600, 0, 0x0403 },                   // 20
    { 0x4009, 12600, DFF_Prop_adjust2Value, 0 },    // 21
    { 0x400a, 12600, DFF_Prop_adjust2Value, 0 },    // 22
    { 0x2000, 0x0415, 10800, 0 },                   // 23
    { 0x2000, 0x0416, 10800, 0 },                   // 24
    { 0x2000, DFF_Prop_adjust3Value, 0, 1800 },     // 25
    { 0x6009, 0x0419, DFF_Prop_adjust2Value, 0 },   // 26
    { 0x600a, 0x0419, DFF_Prop_adjust2Value, 0 },   // 27
    { 0x2000, 0x041a, 10800, 0 },                   // 28
    { 0x2000, 0x041b, 10800, 0 },                   // 29
    { 0x2001, 0x0403, 1, 2 },                       // 30
    { 0x8000, 10800, 0, 0x041e },                   // 31
    { 0x4001, 35,   0x0403, 10800 },                // 32
    { 0x2000, 0x0420, 10, 0 },                      // 33
    { 0x2001, 0x0421, 256, 1 },                     // 34
    { 0x2001, 0x0422, 256, 1 },                     // 35
    { 0x6000, 0x0423, DFF_Prop_adjust2Value, 0 },   // 36
    { 0x6009, 0x041f, 0x0424, 0 },                  // 37
    { 0x600a, 0x041f, 0x0424, 0 },                  // 38
    { 0x2000, 0x0425, 10800, 0 },                   // 39
    { 0x2000, 0x0426, 10800, 0 }                    // 40
};
static const sal_Int32 mso_sptCircularArrowDefault[] =
{
    3, 180 << 16, 0, 5500
};
static const sal_Int32 mso_sptCircularArrowTextRect[] =
{
    0
};
static const mso_AutoShape msoCircularArrow =
{
    (sal_Int32*)mso_sptCircularArrowVert, sizeof( mso_sptCircularArrowVert ) >> 3,
    (sal_uInt16*)mso_sptCircularArrowSegm, sizeof( mso_sptCircularArrowSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptCircularArrowCalc, sizeof( mso_sptCircularArrowCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptCircularArrowDefault,
    (sal_Int32*)mso_sptCircularArrowTextRect,
    (sal_Int32*)NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptCubeVert[] =
{
    0,      21600,
    0,      1 MSO_I,
    2 MSO_I,0,
    21600,  0,
    21600,  3 MSO_I,
    4 MSO_I,21600,
    0,      1 MSO_I,
    2 MSO_I,0,
    21600,  0,
    4 MSO_I,1 MSO_I,
    4 MSO_I, 21600,
    4 MSO_I,1 MSO_I,
    21600,  0,
    21600,  3 MSO_I
};
static const sal_uInt16 mso_sptCubeSegm[] =
{
    0x4000, 0x0005, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptCubeCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoTop, 0x400, 0 },
    { 0x6000, DFF_Prop_geoLeft, 0x400, 0 },
    { 0xa000, DFF_Prop_geoBottom, 0, 0x400 },
    { 0xa000, DFF_Prop_geoRight, 0, 0x400 }
};
static const sal_Int32 mso_sptCubeTextRect[] =
{
    1, 0, 1 MSO_I, 4 MSO_I, 21600
};
static const mso_AutoShape msoCube =
{
    (sal_Int32*)mso_sptCubeVert, sizeof( mso_sptCubeVert ) >> 3,
    (sal_uInt16*)mso_sptCubeSegm, sizeof( mso_sptCubeSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptCubeCalc, sizeof( mso_sptCubeCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault5400,
    (sal_Int32*)mso_sptCubeTextRect,
    (sal_Int32*)NULL,
    10800, 10800
};

static const sal_Int32 mso_sptBevelVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      1 MSO_I, 0 MSO_I,   0 MSO_I, 0 MSO_I,
    21600,  0,      21600,  21600,  1 MSO_I, 1 MSO_I,   1 MSO_I, 0 MSO_I,
    21600,  21600,  0,      21600,  0 MSO_I,1 MSO_I,    1 MSO_I,1 MSO_I,
    0,      21600,  0,      0,      0 MSO_I,0 MSO_I,    0 MSO_I,1 MSO_I
};
static const sal_uInt16 mso_sptBevelSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptBevelCalc[] =
{
    { 0x2001, DFF_Prop_adjustValue, 21599, 21600 },
    { 0x8000, 21600, 0, 0x400 }
};
static const sal_Int32 mso_sptBevelTextRect[] =
{
    1, 0 MSO_I, 0 MSO_I, 1 MSO_I, 1 MSO_I
};
static const mso_AutoShape msoBevel =
{
    (sal_Int32*)mso_sptBevelVert, sizeof( mso_sptBevelVert ) >> 3,
    (sal_uInt16*)mso_sptBevelSegm, sizeof( mso_sptBevelSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptBevelCalc, sizeof( mso_sptBevelCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault2700,
    (sal_Int32*)mso_sptBevelTextRect,
    NULL,
    10800, 10800
};

static const sal_Int32 mso_sptFoldedCornerVert[] =  // adjustment1 : x 10800 - 21600
{
    0,      0,
    21600,  0,
    21600,  0 MSO_I,
    0 MSO_I,21600,
    0,      21600,
    0 MSO_I,21600,
    3 MSO_I,0 MSO_I,
    8 MSO_I,9 MSO_I,
    10 MSO_I, 11 MSO_I,
    21600,0 MSO_I
};
static const sal_uInt16 mso_sptFoldedCornerSegm[] =
{
    0x4000, 0x0004, 0x6001, 0x8000,
    0x4000, 0x0001, 0x2001, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptFoldedCornerCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, 0x400 },
    { 0x2001, 0x0401, 8000, 10800 },
    { 0x8000, 21600, 0, 0x0402 },
    { 0x2001, 0x0401, 1, 2 },
    { 0x2001, 0x0401, 1, 4 },
    { 0x2001, 0x0401, 1, 7 },
    { 0x2001, 0x0401, 1, 16 },
    { 0x6000, 0x0403, 0x405, 0 },
    { 0x6000, 0x0400, 0x406, 0 },
    { 0x8000, 21600, 0, 0x404 },
    { 0x6000, 0x400, 0x407, 0 }
};
static const sal_Int32 mso_sptFoldedCornerDefault[] =
{
    1, 18900
};
static const sal_Int32 mso_sptFoldedCornerTextRect[] =
{
    1, 0, 0, 21600, 11 MSO_I
};
static const mso_AutoShape msoFoldedCorner =
{
    (sal_Int32*)mso_sptFoldedCornerVert, sizeof( mso_sptFoldedCornerVert ) >> 3,
    (sal_uInt16*)mso_sptFoldedCornerSegm, sizeof( mso_sptFoldedCornerSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptFoldedCornerCalc, sizeof( mso_sptFoldedCornerCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptFoldedCornerDefault,
    (sal_Int32*)mso_sptFoldedCornerTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonBlankVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      1 MSO_I, 0 MSO_I,   0 MSO_I, 0 MSO_I,
    21600,  0,      21600,  21600,  1 MSO_I, 1 MSO_I,   1 MSO_I, 0 MSO_I,
    21600,  21600,  0,      21600,  0 MSO_I,1 MSO_I,    1 MSO_I,1 MSO_I,
    0,      21600,  0,      0,      0 MSO_I,0 MSO_I,    0 MSO_I,1 MSO_I
};
static const sal_uInt16 mso_sptActionButtonBlankSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonBlankCalc[] =
{
    { 0x2001, DFF_Prop_adjustValue, 21599, 21600 },
    { 0x8000, 21600, 0, 0x400 }
};
static const sal_Int32 mso_sptActionButtonBlankTextRect[] =
{
    1, 0 MSO_I, 0 MSO_I, 1 MSO_I, 1 MSO_I
};
static const mso_AutoShape msoActionButtonBlank =
{
    (sal_Int32*)mso_sptActionButtonBlankVert, sizeof( mso_sptActionButtonBlankVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonBlankSegm, sizeof( mso_sptActionButtonBlankSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonBlankCalc, sizeof( mso_sptActionButtonBlankCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonBlankTextRect,
    NULL,
    10800, 10800
};

static const sal_Int32 mso_sptActionButtonTextRect[] =
{
    1, 1 MSO_I, 2 MSO_I, 3 MSO_I, 4 MSO_I
};
static const sal_Int32 mso_sptActionButtonHomeVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    10800,      0xa MSO_I,
    0xc MSO_I,  0xe MSO_I,
    0xc MSO_I,  0x10 MSO_I,
    0x12 MSO_I, 0x10 MSO_I,
    0x12 MSO_I, 0x14 MSO_I,
    0x16 MSO_I, 10800,
    0x18 MSO_I, 10800,
    0x18 MSO_I, 0x1a MSO_I,
    0x1c MSO_I, 0x1a MSO_I,
    0x1c MSO_I, 10800,
    0x1e MSO_I, 10800,

    0xc MSO_I,  0xe MSO_I,
    0xc MSO_I,  0x10 MSO_I,
    0x12 MSO_I, 0x10 MSO_I,
    0x12 MSO_I, 0x14 MSO_I,

    0x20 MSO_I, 0x24 MSO_I,
    0x22 MSO_I, 0x24 MSO_I,
    0x22 MSO_I, 0x1a MSO_I,
    0x18 MSO_I, 0x1a MSO_I,
    0x18 MSO_I, 10800,
    0x1c MSO_I, 10800,
    0x1c MSO_I, 0x1a MSO_I,
    0x20 MSO_I, 0x1a MSO_I

};
static const sal_uInt16 mso_sptActionButtonHomeSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x000a, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0007, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonHomeCalc[] =    // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -8000, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0408, 0 },  // a
    { 0x4001, 2960, 0x0406, 1 },    // b
    { 0x6000, 0x040b, 0x0407, 0 },  // c
    { 0x4001, -5000, 0x0406, 1 },   // d
    { 0x6000, 0x040d, 0x0408, 0 },  // e
    { 0x4001, -7000, 0x0406, 1 },   // f
    { 0x6000, 0x040f, 0x0408, 0 },  // 10
    { 0x4001, 5000, 0x0406, 1 },    // 11
    { 0x6000, 0x0411, 0x0407, 0 },  // 12
    { 0x4001, -2960, 0x0406, 1 },   // 13
    { 0x6000, 0x0413, 0x0408, 0 },  // 14
    { 0x4001, 8000, 0x0406, 1 },    // 15
    { 0x6000, 0x0415,0x0407, 0 },   // 16
    { 0x4001, 6100, 0x0406, 1 },    // 17
    { 0x6000, 0x0417,0x0407, 0 },   // 18
    { 0x4001, 8260, 0x0406, 1 },    // 19
    { 0x6000, 0x0419, 0x0408, 0 },  // 1a
    { 0x4001, -6100, 0x0406, 1 },   // 1b
    { 0x6000, 0x041b, 0x0407, 0 },  // 1c
    { 0x4001, -8000, 0x0406, 1 },   // 1d
    { 0x6000, 0x041d, 0x0407, 0 },  // 1e
    { 0x4001, -1060, 0x0406, 1 },   // 1f
    { 0x6000, 0x041f, 0x0407, 0 },  // 20
    { 0x4001, 1060, 0x0406, 1 },    // 21
    { 0x6000, 0x0421, 0x0407, 0 },  // 22
    { 0x4001, 4020, 0x0406, 1 },    // 23
    { 0x6000, 0x0423, 0x0408, 0 }   // 24

};
static const mso_AutoShape msoActionButtonHome =
{
    (sal_Int32*)mso_sptActionButtonHomeVert, sizeof( mso_sptActionButtonHomeVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonHomeSegm, sizeof( mso_sptActionButtonHomeSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonHomeCalc, sizeof( mso_sptActionButtonHomeCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonHelpVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 0xc MSO_I, 0xe MSO_I, 0x10 MSO_I,

    0x12 MSO_I, 0x14 MSO_I, // p0
    0x12 MSO_I, 0x16 MSO_I, // p1
    0x12 MSO_I, 0x18 MSO_I, // c2
    0x1a MSO_I, 10800,      // c3
    0x1c MSO_I, 10800,      // p4
    0x1e MSO_I, 10800,      // c5
    0x20 MSO_I, 0x22 MSO_I, // c6
    0x20 MSO_I, 0x24 MSO_I, // p7
    0x20 MSO_I, 0x26 MSO_I, // c8
    0x28 MSO_I, 0x2a MSO_I, // c9
    10800,      0x2a MSO_I, // p10
    0x2c MSO_I, 0x2a MSO_I, // c11
    0x2e MSO_I, 0x26 MSO_I, // c12
    0x2e MSO_I, 0x24 MSO_I, // p13
    0x30 MSO_I, 0x24 MSO_I, // p14
    0x30 MSO_I, 0x32 MSO_I, // c15
    0x34 MSO_I, 0x36 MSO_I, // c16
    10800,      0x36 MSO_I, // p17
    0x12 MSO_I, 0x36 MSO_I, // c18
    0x1c MSO_I, 0x32 MSO_I, // c19
    0x1c MSO_I, 0x24 MSO_I, // p20
    0x1c MSO_I, 0x38 MSO_I, // c21
    0x3a MSO_I, 0x3c MSO_I, // c22
    0x12 MSO_I, 0x3c MSO_I, // p23
    10800,      0x3c MSO_I, // c24
    0x34 MSO_I, 10800,      // c25
    0x34 MSO_I, 0x16 MSO_I, // p26
    0x34 MSO_I, 0x14 MSO_I  // p27
};
static const sal_uInt16 mso_sptActionButtonHelpSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0xa302, 0x6000, 0x8000,
    0x4000, 0x0001, 0x2004, 0x0001, 0x2004, 0x0001, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonHelpCalc[] =    // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -1690, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0407, 0 },  // a
    { 0x4001, 4600, 0x0406, 1 },    // b
    { 0x6000, 0x040b, 0x0408, 0 },  // c
    { 0x4001, 1690, 0x0406, 1 },    // d
    { 0x6000, 0x040d, 0x0407, 0 },  // e
    { 0x4001, 7980, 0x0406, 1 },    // f
    { 0x6000, 0x040f, 0x0408, 0 },  // 10
    { 0x4001, 1270, 0x0406, 1 },    // 11
    { 0x6000, 0x0411, 0x0407, 0 },  // 12
    { 0x4001, 4000, 0x0406, 1 },    // 13
    { 0x6000, 0x0413, 0x0408, 0 },  // 14
    { 0x4001, 1750, 0x0406, 1 },    // 15
    { 0x6000, 0x0415, 0x0408, 0 },  // 16
    { 0x4001, 800, 0x0406, 1 },     // 17
    { 0x6000, 0x0417, 0x0408, 0 },  // 18
    { 0x4001, 1650, 0x0406, 1 },    // 19
    { 0x6000, 0x0419, 0x0407, 0 },  // 1a
    { 0x4001, 2340, 0x0406, 1 },    // 1b
    { 0x6000, 0x041b, 0x0407, 0 },  // 1c
    { 0x4001, 3640, 0x0406, 1 },    // 1d
    { 0x6000, 0x041d, 0x0407, 0 },  // 1e
    { 0x4001, 4670, 0x0406, 1 },    // 1f
    { 0x6000, 0x041f, 0x0407, 0 },  // 20
    { 0x4001, -1570, 0x0406, 1 },   // 21
    { 0x6000, 0x0421, 0x0408, 0 },  // 22
    { 0x4001, -3390, 0x0406, 1 },   // 23
    { 0x6000, 0x0423, 0x0408, 0 },  // 24
    { 0x4001, -6050, 0x0406, 1 },   // 25
    { 0x6000, 0x0425, 0x0408, 0 },  // 26
    { 0x4001, 2540, 0x0406, 1 },    // 27
    { 0x6000, 0x0427, 0x0407, 0 },  // 28
    { 0x4001, -8050, 0x0406, 1 },   // 29
    { 0x6000, 0x0429, 0x0408, 0 },  // 2a
    { 0x4001, -2540, 0x0406, 1 },   // 2b
    { 0x6000, 0x042b, 0x0407, 0 },  // 2c
    { 0x4001, -4460, 0x0406, 1 },   // 2d
    { 0x6000, 0x042d, 0x0407, 0 },  // 2e
    { 0x4001, -2330, 0x0406, 1 },   // 2f
    { 0x6000, 0x042f, 0x0407, 0 },  // 30
    { 0x4001, -4700, 0x0406, 1 },   // 31
    { 0x6000, 0x0431, 0x0408, 0 },  // 32
    { 0x4001, -1270, 0x0406, 1 },   // 33
    { 0x6000, 0x0433, 0x0407, 0 },  // 34
    { 0x4001, -5720, 0x0406, 1 },   // 35
    { 0x6000, 0x0435, 0x0408, 0 },  // 36
    { 0x4001, -2540, 0x0406, 1 },   // 37
    { 0x6000, 0x0437, 0x0408, 0 },  // 38
    { 0x4001, 1800, 0x0406, 1 },    // 39
    { 0x6000, 0x0439, 0x0407, 0 },  // 3a
    { 0x4001, -1700, 0x0406, 1 },   // 3b
    { 0x6000, 0x043b, 0x0408, 0 }   // 3c
};
static const mso_AutoShape msoActionButtonHelp =
{
    (sal_Int32*)mso_sptActionButtonHelpVert, sizeof( mso_sptActionButtonHelpVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonHelpSegm, sizeof( mso_sptActionButtonHelpSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonHelpCalc, sizeof( mso_sptActionButtonHelpCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonInformationVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 0xc MSO_I, 0xe MSO_I, 0x10 MSO_I,

    0x12 MSO_I, 0x14 MSO_I, 0x16 MSO_I, 0x18 MSO_I,

    0x1a MSO_I, 0x1c MSO_I,
    0x1e MSO_I, 0x1c MSO_I,
    0x1e MSO_I, 0x20 MSO_I,
    0x22 MSO_I, 0x20 MSO_I,
    0x22 MSO_I, 0x24 MSO_I,
    0x1a MSO_I, 0x24 MSO_I,
    0x1a MSO_I, 0x20 MSO_I,
    0x26 MSO_I, 0x20 MSO_I,
    0x26 MSO_I, 0x28 MSO_I,
    0x1a MSO_I, 0x28 MSO_I
};
static const sal_uInt16 mso_sptActionButtonInformationSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0xa302, 0x6000, 0x8000,
    0xa302, 0x6000, 0x8000,
    0x4000, 0x0009, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonInformationCalc[] = // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -8050, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0407, 0 },  // a
    { 0x4001, -8050, 0x0406, 1 },   // b
    { 0x6000, 0x040b, 0x0408, 0 },  // c
    { 0x4001, 8050, 0x0406, 1 },    // d
    { 0x6000, 0x040d, 0x0407, 0 },  // e
    { 0x4001, 8050, 0x0406, 1 },    // f
    { 0x6000, 0x040f, 0x0408, 0 },  // 10

    { 0x4001, -2060, 0x0406, 1 },   // 11
    { 0x6000, 0x0411, 0x0407, 0 },  // 12
    { 0x4001, -7620, 0x0406, 1 },   // 13
    { 0x6000, 0x0413, 0x0408, 0 },  // 14
    { 0x4001, 2060, 0x0406, 1 },    // 15
    { 0x6000, 0x0415, 0x0407, 0 },  // 16
    { 0x4001, -3500, 0x0406, 1 },   // 17
    { 0x6000, 0x0417, 0x0408, 0 },  // 18

    { 0x4001, -2960, 0x0406, 1 },   // 19
    { 0x6000, 0x0419, 0x0407, 0 },  // 1a
    { 0x4001, -2960, 0x0406, 1 },   // 1b
    { 0x6000, 0x041b, 0x0408, 0 },  // 1c
    { 0x4001, 1480, 0x0406, 1 },    // 1d
    { 0x6000, 0x041d, 0x0407, 0 },  // 1e
    { 0x4001, 5080, 0x0406, 1 },    // 1f
    { 0x6000, 0x041f, 0x0408, 0 },  // 20
    { 0x4001, 2960, 0x0406, 1 },    // 21
    { 0x6000, 0x0421, 0x0407, 0 },  // 22
    { 0x4001, 6140, 0x0406, 1 },    // 23
    { 0x6000, 0x0423, 0x0408, 0 },  // 24
    { 0x4001, -1480, 0x0406, 1 },   // 25
    { 0x6000, 0x0425, 0x0407, 0 },  // 26
    { 0x4001, -1920, 0x0406, 1 },   // 27
    { 0x6000, 0x0427, 0x0408, 0 }   // 28
};
static const mso_AutoShape msoActionButtonInformation =
{
    (sal_Int32*)mso_sptActionButtonInformationVert, sizeof( mso_sptActionButtonInformationVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonInformationSegm, sizeof( mso_sptActionButtonInformationSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonInformationCalc, sizeof( mso_sptActionButtonInformationCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonBackPreviousVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 10800,
    0xe MSO_I, 0xc MSO_I,
    0xe MSO_I, 0x10 MSO_I
};
static const sal_uInt16 mso_sptActionButtonForwardBackSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonForwardBackCalc[] = // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -8050, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0407, 0 },  // a
    { 0x4001, -8050, 0x0406, 1 },   // b
    { 0x6000, 0x040b, 0x0408, 0 },  // c
    { 0x4001, 8050, 0x0406, 1 },    // d
    { 0x6000, 0x040d, 0x0407, 0 },  // e
    { 0x4001, 8050, 0x0406, 1 },    // f
    { 0x6000, 0x040f, 0x0408, 0 }   // 10
};
static const mso_AutoShape msoActionButtonBackPrevious =
{
    (sal_Int32*)mso_sptActionButtonBackPreviousVert, sizeof( mso_sptActionButtonBackPreviousVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonForwardBackSegm, sizeof( mso_sptActionButtonForwardBackSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonForwardBackCalc, sizeof( mso_sptActionButtonForwardBackCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonForwardNextVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 0xc MSO_I,
    0xe MSO_I, 10800,
    0xa MSO_I, 0x10 MSO_I
};
static const mso_AutoShape msoActionButtonForwardNext =
{
    (sal_Int32*)mso_sptActionButtonForwardNextVert, sizeof( mso_sptActionButtonForwardNextVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonForwardBackSegm, sizeof( mso_sptActionButtonForwardBackSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonForwardBackCalc, sizeof( mso_sptActionButtonForwardBackCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonBeginningVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 10800,
    0xe MSO_I, 0xc MSO_I,
    0xe MSO_I, 0x10 MSO_I,

    0x12 MSO_I, 0xc MSO_I,
    0x14 MSO_I, 0xc MSO_I,
    0x14 MSO_I, 0x10 MSO_I,
    0x12 MSO_I, 0x10 MSO_I
};
static const sal_uInt16 mso_sptActionButtonBeginningEndSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,

    0x4000, 0x0002, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonBeginningEndCalc[] =    // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -4020, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0407, 0 },  // a
    { 0x4001, -8050, 0x0406, 1 },   // b
    { 0x6000, 0x040b, 0x0408, 0 },  // c
    { 0x4001, 8050, 0x0406, 1 },    // d
    { 0x6000, 0x040d, 0x0407, 0 },  // e
    { 0x4001, 8050, 0x0406, 1 },    // f
    { 0x6000, 0x040f, 0x0408, 0 },  // 10

    { 0x4001, -8050, 0x0406, 1 },   // 11
    { 0x6000, 0x0411, 0x0407, 0 },  // 12
    { 0x4001, -6140, 0x0406, 1 },   // 13
    { 0x6000, 0x0413, 0x0407, 0 },  // 14


    { 0x4001, 4020, 0x0406, 1 },    // 15
    { 0x6000, 0x0415, 0x0407, 0 },  // 16
    { 0x4001, 6140, 0x0406, 1 },    // 17
    { 0x6000, 0x0417, 0x0407, 0 }   // 18
};
static const mso_AutoShape msoActionButtonBeginning =
{
    (sal_Int32*)mso_sptActionButtonBeginningVert, sizeof( mso_sptActionButtonBeginningVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonBeginningEndSegm, sizeof( mso_sptActionButtonBeginningEndSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonBeginningEndCalc, sizeof( mso_sptActionButtonBeginningEndCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonEndVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0x16 MSO_I, 10800,
    0x12 MSO_I, 0x10 MSO_I,
    0x12 MSO_I, 0xc MSO_I,

    0x18 MSO_I, 0xc MSO_I,
    0x18 MSO_I, 0x10 MSO_I,
    0xe MSO_I, 0x10 MSO_I,
    0xe MSO_I, 0xc MSO_I
};
static const mso_AutoShape msoActionButtonEnd =
{
    (sal_Int32*)mso_sptActionButtonEndVert, sizeof( mso_sptActionButtonEndVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonBeginningEndSegm, sizeof( mso_sptActionButtonBeginningEndSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonBeginningEndCalc, sizeof( mso_sptActionButtonBeginningEndCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonReturnVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 0xc MSO_I,   // p0
    0xe MSO_I, 0xc MSO_I,   // p1
    0xe MSO_I, 0x10 MSO_I,  // p2
    0xe MSO_I, 0x12 MSO_I,  // c3
    0x14 MSO_I, 0x16 MSO_I, // c4
    0x18 MSO_I, 0x16 MSO_I, // p5
    10800,      0x16 MSO_I, // p6
    0x1a MSO_I, 0x16 MSO_I, // c7
    0x1c MSO_I, 0x12 MSO_I, // c8
    0x1c MSO_I, 0x10 MSO_I, // p9
    0x1c MSO_I, 0xc MSO_I,  // p10
    10800,      0xc MSO_I,  // p11
    0x1e MSO_I, 0x20 MSO_I, // p12
    0x22 MSO_I, 0xc MSO_I,  // p13
    0x24 MSO_I, 0xc MSO_I,  // p14
    0x24 MSO_I, 0x10 MSO_I, // p15
    0x24 MSO_I, 0x26 MSO_I, // c16
    0x28 MSO_I, 0x2a MSO_I, // c17
    10800,      0x2a MSO_I, // p18
    0x18 MSO_I, 0x2a MSO_I, // p19
    0x2c MSO_I, 0x2a MSO_I, // c20
    0xa MSO_I,  0x26 MSO_I, // c21
    0xa MSO_I,  0x10 MSO_I  // p22
};
static const sal_uInt16 mso_sptActionButtonReturnSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0002, 0x2001, 0x0001, 0x2001, 0x0006,0x2001, 0x0001, 0x2001, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonReturnCalc[] =  // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -8050, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0407, 0 },  // a
    { 0x4001, -3800, 0x0406, 1 },   // b
    { 0x6000, 0x040b, 0x0408, 0 },  // c
    { 0x4001, -4020, 0x0406, 1 },   // d
    { 0x6000, 0x040d, 0x0407, 0 },  // e
    { 0x4001, 2330, 0x0406, 1 },    // f
    { 0x6000, 0x040f, 0x0408, 0 },  // 10
    { 0x4001, 3390, 0x0406, 1 },    // 11
    { 0x6000, 0x0411, 0x0408, 0 },  // 12
    { 0x4001, -3100, 0x0406, 1 },   // 13
    { 0x6000, 0x0413, 0x0407, 0 },  // 14
    { 0x4001, 4230, 0x0406, 1 },    // 15
    { 0x6000, 0x0415, 0x0408, 0 },  // 16
    { 0x4001, -1910, 0x0406, 1 },   // 17
    { 0x6000, 0x0417, 0x0407, 0 },  // 18
    { 0x4001, 1190, 0x0406, 1 },    // 19
    { 0x6000, 0x0419, 0x0407, 0 },  // 1a
    { 0x4001, 2110, 0x0406, 1 },    // 1b
    { 0x6000, 0x041b, 0x0407, 0 },  // 1c
    { 0x4001, 4030, 0x0406, 1 },    // 1d
    { 0x6000, 0x041d, 0x0407, 0 },  // 1e
    { 0x4001, -7830, 0x0406, 1 },   // 1f
    { 0x6000, 0x041f, 0x0408, 0 },  // 20
    { 0x4001, 8250, 0x0406, 1 },    // 21
    { 0x6000, 0x0421, 0x0407, 0 },  // 22
    { 0x4001, 6140, 0x0406, 1 },    // 23
    { 0x6000, 0x0423, 0x0407, 0 },  // 24
    { 0x4001, 5510, 0x0406, 1 },    // 25
    { 0x6000, 0x0425, 0x0408, 0 },  // 26
    { 0x4001, 3180, 0x0406, 1 },    // 27
    { 0x6000, 0x0427, 0x0407, 0 },  // 28
    { 0x4001, 8450, 0x0406, 1 },    // 29
    { 0x6000, 0x0429, 0x0408, 0 },  // 2a
    { 0x4001, -5090, 0x0406, 1 },   // 2b
    { 0x6000, 0x042b, 0x0407, 0 }   // 2c
};
static const mso_AutoShape msoActionButtonReturn =
{
    (sal_Int32*)mso_sptActionButtonReturnVert, sizeof( mso_sptActionButtonReturnVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonReturnSegm, sizeof( mso_sptActionButtonReturnSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonReturnCalc, sizeof( mso_sptActionButtonReturnCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonDocumentVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 0xc MSO_I,
    0xe MSO_I, 0xc MSO_I,
    0x10 MSO_I, 0x12 MSO_I,
    0x10 MSO_I, 0x14 MSO_I,
    0xa MSO_I, 0x14 MSO_I,

    0xe MSO_I, 0xc MSO_I,
    0x10 MSO_I, 0x12 MSO_I,
    0xe MSO_I, 0x12 MSO_I
};
static const sal_uInt16 mso_sptActionButtonDocumentSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,

    0x4000, 0x0004, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonDocumentCalc[] =    // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -6350, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0407, 0 },  // a
    { 0x4001, -7830, 0x0406, 1 },   // b
    { 0x6000, 0x040b, 0x0408, 0 },  // c
    { 0x4001, 1690, 0x0406, 1 },    // d
    { 0x6000, 0x040d, 0x0407, 0 },  // e
    { 0x4001, 6350, 0x0406, 1 },    // f
    { 0x6000, 0x040f, 0x0407, 0 },  // 10
    { 0x4001, -3810, 0x0406, 1 },   // 11
    { 0x6000, 0x0411, 0x0408, 0 },  // 12
    { 0x4001, 7830, 0x0406, 1 },    // 13
    { 0x6000, 0x0413, 0x0408, 0 }   // 14
};
static const mso_AutoShape msoActionButtonDocument =
{
    (sal_Int32*)mso_sptActionButtonDocumentVert, sizeof( mso_sptActionButtonDocumentVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonDocumentSegm, sizeof( mso_sptActionButtonDocumentSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonDocumentCalc, sizeof( mso_sptActionButtonDocumentCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonSoundVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 0xc MSO_I,
    0xe MSO_I, 0xc MSO_I,
    0x10 MSO_I, 0x12 MSO_I,
    0x10 MSO_I, 0x14 MSO_I,
    0xe MSO_I, 0x16 MSO_I,
    0xa MSO_I, 0x16 MSO_I,

    0x18 MSO_I, 10800,
    0x1a MSO_I, 10800,

    0x18 MSO_I, 0xc MSO_I,
    0x1a MSO_I, 0x1c MSO_I,

    0x18 MSO_I, 0x16 MSO_I,
    0x1a MSO_I, 0x1e MSO_I
};
static const sal_uInt16 mso_sptActionButtonSoundSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,

    0x4000, 0x0005, 0x6001, 0x8000,
    0x4000, 0x0001, 0x8000,
    0x4000, 0x0001, 0x8000,
    0x4000, 0x0001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonSoundCalc[] =   // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -8050, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0407, 0 },  // a
    { 0x4001, -2750, 0x0406, 1 },   // b
    { 0x6000, 0x040b, 0x0408, 0 },  // c
    { 0x4001, -2960, 0x0406, 1 },   // d
    { 0x6000, 0x040d, 0x0407, 0 },  // e
    { 0x4001, 2120, 0x0406, 1 },    // f
    { 0x6000, 0x040f, 0x0407, 0 },  // 10
    { 0x4001, -8050, 0x0406, 1 },   // 11
    { 0x6000, 0x0411, 0x0408, 0 },  // 12
    { 0x4001, 8050, 0x0406, 1 },    // 13
    { 0x6000, 0x0413, 0x0408, 0 },  // 14
    { 0x4001, 2750, 0x0406, 1 },    // 15
    { 0x6000, 0x0415, 0x0408, 0 },  // 16
    { 0x4001, 4020, 0x0406, 1 },    // 17
    { 0x6000, 0x0417, 0x0407, 0 },  // 18
    { 0x4001, 8050, 0x0406, 1 },    // 19
    { 0x6000, 0x0419, 0x0407, 0 },  // 1a
    { 0x4001, -5930, 0x0406, 1 },   // 1b
    { 0x6000, 0x041b, 0x0408, 0 },  // 1c
    { 0x4001, 5930, 0x0406, 1 },    // 1d
    { 0x6000, 0x041d, 0x0408, 0 }   // 1e
};
static const mso_AutoShape msoActionButtonSound =
{
    (sal_Int32*)mso_sptActionButtonSoundVert, sizeof( mso_sptActionButtonSoundVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonSoundSegm, sizeof( mso_sptActionButtonSoundSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonSoundCalc, sizeof( mso_sptActionButtonSoundCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptActionButtonMovieVert[] =
{
    0,      0,      21600,  0,      21600,  21600,      0,      21600,
    0,      0,      21600,  0,      3 MSO_I, 2 MSO_I,   1 MSO_I, 2 MSO_I,
    21600,  0,      21600,  21600,  3 MSO_I, 4 MSO_I,   3 MSO_I, 2 MSO_I,
    21600,  21600,  0,      21600,  1 MSO_I,4 MSO_I,    3 MSO_I,4 MSO_I,
    0,      21600,  0,      0,      1 MSO_I,2 MSO_I,    1 MSO_I,4 MSO_I,

    0xa MSO_I, 0xc MSO_I,   // p0
    0xe MSO_I, 0xc MSO_I,   // p1
    0x10 MSO_I, 0x12 MSO_I, // p2
    0x14 MSO_I, 0x12 MSO_I, // p3
    0x16 MSO_I, 0x18 MSO_I, // p4
    0x16 MSO_I, 0x1a MSO_I, // p5
    0x1c MSO_I, 0x1a MSO_I, // p6
    0x1e MSO_I, 0x18 MSO_I, // p7
    0x20 MSO_I, 0x18 MSO_I, // p8
    0x20 MSO_I, 0x22 MSO_I, // p9
    0x1e MSO_I, 0x22 MSO_I, // p10
    0x1c MSO_I, 0x24 MSO_I, // p11
    0x16 MSO_I, 0x24 MSO_I, // p12
    0x16 MSO_I, 0x26 MSO_I, // p13
    0x2a MSO_I, 0x26 MSO_I, // p14
    0x2a MSO_I, 0x28 MSO_I, // p15
    0x10 MSO_I, 0x28 MSO_I, // p16
    0xe MSO_I, 0x2c MSO_I,  // p17,
    0xa MSO_I, 0x2c MSO_I   // p18
};
static const sal_uInt16 mso_sptActionButtonMovieSegm[] =
{
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0003, 0x6001, 0x8000,
    0x4000, 0x0012, 0x6001, 0x8000
};
static const SvxMSDffCalculationData mso_sptActionButtonMovieCalc[] =   // adj value 0 - 5400
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x0405, 1, 10800 },                           // scaling   6
    { 0x6000, DFF_Prop_geoRight, DFF_Prop_geoLeft, 10800 }, // lr center 7
    { 0x6000, DFF_Prop_geoBottom, DFF_Prop_geoTop, 10800 }, // ul center 8

    { 0x4001, -8050, 0x0406, 1 },   // 9
    { 0x6000, 0x0409, 0x0407, 0 },  // a
    { 0x4001, -4020, 0x0406, 1 },   // b
    { 0x6000, 0x040b, 0x0408, 0 },  // c
    { 0x4001, -7000, 0x0406, 1 },   // d
    { 0x6000, 0x040d, 0x0407, 0 },  // e
    { 0x4001, -6560, 0x0406, 1 },   // f
    { 0x6000, 0x040f, 0x0407, 0 },  // 10
    { 0x4001, -3600, 0x0406, 1 },   // 11
    { 0x6000, 0x0411, 0x0408, 0 },  // 12
    { 0x4001, 4020, 0x0406, 1 },    // 13
    { 0x6000, 0x0413, 0x0407, 0 },  // 14
    { 0x4001, 4660, 0x0406, 1 },    // 15
    { 0x6000, 0x0415, 0x0407, 0 },  // 16
    { 0x4001, -2960, 0x0406, 1 },   // 17
    { 0x6000, 0x0417, 0x0408, 0 },  // 18
    { 0x4001, -2330, 0x0406, 1 },   // 19
    { 0x6000, 0x0419, 0x0408, 0 },  // 1a
    { 0x4001, 6780, 0x0406, 1 },    // 1b
    { 0x6000, 0x041b, 0x0407, 0 },  // 1c
    { 0x4001, 7200, 0x0406, 1 },    // 1d
    { 0x6000, 0x041d, 0x0407, 0 },  // 1e
    { 0x4001, 8050, 0x0406, 1 },    // 1f
    { 0x6000, 0x041f, 0x0407, 0 },  // 20
    { 0x4001, 2960, 0x0406, 1 },    // 21
    { 0x6000, 0x0421, 0x0408, 0 },  // 22
    { 0x4001, 2330, 0x0406, 1 },    // 23
    { 0x6000, 0x0423, 0x0408, 0 },  // 24
    { 0x4001, 3800, 0x0406, 1 },    // 25
    { 0x6000, 0x0425, 0x0408, 0 },  // 26
    { 0x4001, -1060, 0x0406, 1 },   // 27
    { 0x6000, 0x0427, 0x0408, 0 },  // 28
    { 0x4001, -6350, 0x0406, 1 },   // 29
    { 0x6000, 0x0429, 0x0407, 0 },  // 2a
    { 0x4001, -640, 0x0406, 1 },    // 2b
    { 0x6000, 0x042b, 0x0408, 0 }   // 2c
};
static const mso_AutoShape msoActionButtonMovie =
{
    (sal_Int32*)mso_sptActionButtonMovieVert, sizeof( mso_sptActionButtonMovieVert ) >> 3,
    (sal_uInt16*)mso_sptActionButtonMovieSegm, sizeof( mso_sptActionButtonMovieSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptActionButtonMovieCalc, sizeof( mso_sptActionButtonMovieCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1400,
    (sal_Int32*)mso_sptActionButtonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptSmileyFaceVert[] =                        // adj value 15510 - 17520
{
    0,      0,      21600,  21600,
    6140,   6350,   8470,   8680,
    13130,  6350,   15460,  8680,
    4870,   1 MSO_I,8680,   2 MSO_I,
    12920,  2 MSO_I,16730,  1 MSO_I

};
static const sal_uInt16 mso_sptSmileyFaceSegm[] =
{
    0xa302, 0x6000, 0x8000,
    0xa302, 0x6000, 0x8000,
    0xa302, 0x6000, 0x8000,
    0x4000, 0x2001, 0x8000
};
static const SvxMSDffCalculationData mso_sptSmileyFaceCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 15510 },
    { 0x8000, 17520, 0, 0x400 },
    { 0x4000, 15510, 0x400, 0 }
};

static const sal_Int32 mso_sptSmileyFaceTextRect[] =
{
    1, 0, 1 MSO_I, 4 MSO_I, 21600
};
static const sal_Int32 mso_sptSmileyFaceDefault[] =
{
    1, 17520
};
static const mso_AutoShape msoSmileyFace =
{
    (sal_Int32*)mso_sptSmileyFaceVert, sizeof( mso_sptSmileyFaceVert ) >> 3,
    (sal_uInt16*)mso_sptSmileyFaceSegm, sizeof( mso_sptSmileyFaceSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptSmileyFaceCalc, sizeof( mso_sptSmileyFaceCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptSmileyFaceDefault,
    (sal_Int32*)mso_sptEllipseTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptDonutVert[] =                     // adj value 0 - 10800
{
    0,      0,      21600,  21600,
    0 MSO_I,0 MSO_I,1 MSO_I,1 MSO_I
};
static const sal_uInt16 mso_sptDonutSegm[] =
{
    0xa302, 0xa302, 0x8000
};
static const SvxMSDffCalculationData mso_sptDonutCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjustValue }
};
static const mso_AutoShape msoDonut =
{
    (sal_Int32*)mso_sptDonutVert, sizeof( mso_sptDonutVert ) >> 3,
    (sal_uInt16*)mso_sptDonutSegm, sizeof( mso_sptDonutSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptDonutCalc, sizeof( mso_sptDonutCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault5400,
    (sal_Int32*)mso_sptEllipseTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptNoSmokingVert[] =                     // adj value 0 - 7200
{
    0,      0,      21600,  21600,
    0 MSO_I,0 MSO_I,1 MSO_I,1 MSO_I,
    9 MSO_I,0xa MSO_I, 0xb MSO_I, 0xc MSO_I,
    0 MSO_I,0 MSO_I,1 MSO_I,1 MSO_I,
    0xd MSO_I, 0xe MSO_I, 0xf MSO_I, 0x10 MSO_I
};
static const sal_uInt16 mso_sptNoSmokingSegm[] =
{
    0xa302, 0xa404, 0xa404, 0x6000, 0x8000
};
static const SvxMSDffCalculationData mso_sptNoSmokingCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },         // 0
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },     // 1
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },     // 2
    { 0x2001, DFF_Prop_adjustValue, 1, 2 },         // 3
    { 0xa080, 0x403, 0, 0x402 },                    // 4
    { 0x8000, 10800, 0, 0x403 },                    // 5 x1
    { 0x4000, 10800, 0x403, 0 },                    // 6 x2
    { 0x8000, 10800, 0, 0x404 },                    // 7 y1
    { 0x4000, 10800, 0x404, 0 },                    // 8 y2
    { 0x6081, 0x405, 0x407, 450 },                  // 9
    { 0x6082, 0x405, 0x407, 450 },                  // a
    { 0x6081, 0x405, 0x408, 450 },                  // b
    { 0x6082, 0x405, 0x408, 450 },                  // c
    { 0x6081, 0x406, 0x408, 450 },                  // d
    { 0x6082, 0x406, 0x408, 450 },                  // e
    { 0x6081, 0x406, 0x407, 450 },                  // f
    { 0x6082, 0x406, 0x407, 450 }                   // 10
};
static const mso_AutoShape msoNoSmoking =
{
    (sal_Int32*)mso_sptNoSmokingVert, sizeof( mso_sptNoSmokingVert ) >> 3,
    (sal_uInt16*)mso_sptNoSmokingSegm, sizeof( mso_sptNoSmokingSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptNoSmokingCalc, sizeof( mso_sptNoSmokingCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault2700,
    (sal_Int32*)mso_sptEllipseTextRect,
    NULL,
    0x80000000, 0x80000000
};

// aware : control points are always part of the bounding box
static const sal_Int32 mso_sptHeartVert[] =
{
    10800,  21599, 321,     6886,   70,     6036,   // ppp
    -9,     5766,   -1,     5474,   2,      5192,   // ccp
    6,      4918,   43,     4641,   101,    4370,   // ccp
    159,    4103,   245,    3837,   353,    3582,   // ccp
    460,    3326,   591,    3077,   741,    2839,   // ccp
    892,    2598,   1066,   2369,   1253,   2155,   // ccp
    1443,   1938,   1651,   1732,   1874,   1543,   // ccp
    2097,   1351,   2337,   1174,   2587,   1014,   // ccp
    2839,   854,    3106,   708,    3380,   584,    // ccp
    3656,   459,    3945,   350,    4237,   264,    // ccp
    4533,   176,    4838,   108,    5144,   66,     // ccp
    5454,   22,     5771,   1,      6086,   3,      // ccp
    6407,   7,      6731,   35,     7048,   89,     // ccp
    7374,   144,    7700,   226,    8015,   335,    // ccp
    8344,   447,    8667,   590,    8972,   756,    // ccp
    9297,   932,    9613,   1135,   9907,   1363,   // ccp
    10224,  1609,   10504,  1900,   10802,  2169,   // ccp
    11697,  1363,                                   // p
    11971,  1116,   12304,  934,    12630,  756,    // ccp
    12935,  590,    13528,  450,    13589,  335,    // ccp
    13901,  226,    14227,  144,    14556,  89,     // ccp
    14872,  35,     15195,  7,      15517,  3,      // ccp
    15830,  0,      16147,  22,     16458,  66,     // ccp
    16764,  109,    17068,  177,    17365,  264,    // ccp
    17658,  349,    17946,  458,    18222,  584,    // ccp
    18496,  708,    18762,  854,    19015,  1014,   // ccp
    19264,  1172,   19504,  1349,   19730,  1543,   // ccp
    19950,  1731,   20158,  1937,   20350,  2155,   // ccp
    20536,  2369,   20710,  2598,   20861,  2839,   // ccp
    21010,  3074,   21143,  3323,   21251,  3582,   // ccp
    21357,  3835,   21443,  4099,   21502,  4370,   // ccp
    21561,  4639,   21595,  4916,   21600,  5192,   // ccp
    21606,  5474,   21584,  5760,   21532,  6036,   // ccp
    21478,  6326,   21366,  6603,   21282,  6887,   // ccp
    10802,  21602                                   // p
};
static const sal_uInt16 mso_sptHeartSegm[] =
{
    0x4000, 0x0002, 0x2010, 0x0001, 0x2010, 0x0001, 0x6001, 0x8000
};
static const sal_Int32 mso_sptHeartTextRect[] =
{
    1,  5080,   2540,   16520,  13550
};
static const sal_Int32 mso_sptHeartBoundRect[] =
{
    -9, 0, 21606, 21602
};
static const mso_AutoShape msoHeart =
{
    (sal_Int32*)mso_sptHeartVert, sizeof( mso_sptHeartVert ) >> 3,
    (sal_uInt16*)mso_sptHeartSegm, sizeof( mso_sptHeartSegm ) >> 1,
    NULL, 0,
    NULL,
    (sal_Int32*)mso_sptHeartTextRect,
    (sal_Int32*)mso_sptHeartBoundRect,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptLightningBoldVert[] =
{
    8458,   0,      0,      3923,
    7564,   8416,   4993,   9720,
    12197,  13904,  9987,   14934,
    21600,  21600,  14768,  12911,
    16558,  12016,  11030,  6840,
    12831,  6120,   8458,   0
};
static const sal_Int32 mso_sptLightningBoldTextRect[] =
{
    1,  8680,   7410,   13970,  14190
};
static const mso_AutoShape msoLightningBold =
{
    (sal_Int32*)mso_sptLightningBoldVert, sizeof( mso_sptLightningBoldVert ) >> 3,
    NULL, 0,
    NULL, 0,
    NULL,
    (sal_Int32*)mso_sptLightningBoldTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptSunVert[] =       // adj value 2700 -> 10125
{
    0, 10800, 4 MSO_I, 8 MSO_I, 4 MSO_I, 9 MSO_I,
    0x0a MSO_I, 0x0b MSO_I, 0x0c MSO_I, 0x0d MSO_I, 0x0e MSO_I, 0x0f MSO_I,
    0x10 MSO_I, 0x11 MSO_I, 0x12 MSO_I, 0x13 MSO_I, 0x14 MSO_I, 0x15 MSO_I,
    0x16 MSO_I, 0x17 MSO_I, 0x18 MSO_I, 0x19 MSO_I, 0x1a MSO_I, 0x1b MSO_I,
    0x1c MSO_I, 0x1d MSO_I, 0x1e MSO_I, 0x1f MSO_I, 0x20 MSO_I, 0x21 MSO_I,
    0x22 MSO_I, 0x23 MSO_I, 0x24 MSO_I, 0x25 MSO_I, 0x26 MSO_I, 0x27 MSO_I,
    0x28 MSO_I, 0x29 MSO_I, 0x2a MSO_I, 0x2b MSO_I, 0x2c MSO_I, 0x2d MSO_I,
    0x2e MSO_I, 0x2f MSO_I, 0x30 MSO_I, 0x31 MSO_I, 0x32 MSO_I, 0x33 MSO_I,
    0 MSO_I,0 MSO_I,1 MSO_I,1 MSO_I
};
static const sal_uInt16 mso_sptSunSegm[] =
{
    0x4000, 0x0002, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000,
    0x4000, 0x0002, 0x6001, 0x8000,
    0xa302, 0x6000, 0x8000
};
static const SvxMSDffCalculationData mso_sptSunCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },
    { 0x2000, DFF_Prop_adjustValue, 0, 2700 },
    { 0x2001, 0x402, 5080, 7425 },
    { 0x2000, 0x403, 2540, 0 },
    { 0x8000, 10125, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x405, 2120, 7425 },
    { 0x2000, 0x406, 210, 0 },
    { 0x4000, 10800, 0x407, 0 },    // y1 (0x8)
    { 0x8000, 10800, 0, 0x407 },    // y2 (0x9)
    { 0x0081, 0, 10800, 450 },      // 0xa
    { 0x0082, 0, 10800, 450 },      // 0xb
    { 0x6081, 0x404, 0x408, 450 },  // 0xc
    { 0x6082, 0x404, 0x408, 450 },  // 0xd
    { 0x6081, 0x404, 0x409, 450 },  // 0xe
    { 0x6082, 0x404, 0x409, 450 },  // 0xf
    { 0x0081, 0, 10800, 900 },      // 0x10
    { 0x0082, 0, 10800, 900 },      // 0x11
    { 0x6081, 0x404, 0x408, 900 },  // 0x12
    { 0x6082, 0x404, 0x408, 900 },  // 0x13
    { 0x6081, 0x404, 0x409, 900 },  // 0x14
    { 0x6082, 0x404, 0x409, 900 },  // 0x15
    { 0x0081, 0, 10800, 1350 },     // 0x16
    { 0x0082, 0, 10800, 1350 },     // 0x17
    { 0x6081, 0x404, 0x408, 1350 }, // 0x18
    { 0x6082, 0x404, 0x408, 1350 }, // 0x19
    { 0x6081, 0x404, 0x409, 1350 }, // 0x1a
    { 0x6082, 0x404, 0x409, 1350 }, // 0x1b
    { 0x0081, 0, 10800, 1800 },     // 0x1c
    { 0x0082, 0, 10800, 1800 },     // 0x1d
    { 0x6081, 0x404, 0x408, 1800 }, // 0x1e
    { 0x6082, 0x404, 0x408, 1800 }, // 0x1f
    { 0x6081, 0x404, 0x409, 1800 }, // 0x20
    { 0x6082, 0x404, 0x409, 1800 }, // 0x21
    { 0x0081, 0, 10800, 2250 },     // 0x22
    { 0x0082, 0, 10800, 2250 },     // 0x23
    { 0x6081, 0x404, 0x408, 2250 }, // 0x24
    { 0x6082, 0x404, 0x408, 2250 }, // 0x25
    { 0x6081, 0x404, 0x409, 2250 }, // 0x26
    { 0x6082, 0x404, 0x409, 2250 }, // 0x27
    { 0x0081, 0, 10800, 2700 },     // 0x28
    { 0x0082, 0, 10800, 2700 },     // 0x29
    { 0x6081, 0x404, 0x408, 2700 }, // 0x2a
    { 0x6082, 0x404, 0x408, 2700 }, // 0x2b
    { 0x6081, 0x404, 0x409, 2700 }, // 0x2c
    { 0x6082, 0x404, 0x409, 2700 }, // 0x2d
    { 0x0081, 0, 10800, 3150 },     // 0x2e
    { 0x0082, 0, 10800, 3150 },     // 0x2f
    { 0x6081, 0x404, 0x408, 3150 }, // 0x30
    { 0x6082, 0x404, 0x408, 3150 }, // 0x31
    { 0x6081, 0x404, 0x409, 3150 }, // 0x32
    { 0x6082, 0x404, 0x409, 3150 }, // 0x33
    { 0x2081, DFF_Prop_adjustValue, 10800, 450 },   // 0x34 ( textbox )
    { 0x2081, DFF_Prop_adjustValue, 10800, 2250 }   // 0x35

};
static const sal_Int32 mso_sptSunTextRect[] =
{
    1,  0x34 MSO_I, 0x34 MSO_I, 0x35 MSO_I, 0x35 MSO_I
};
static const mso_AutoShape msoSun =
{
    (sal_Int32*)mso_sptSunVert, sizeof( mso_sptSunVert ) >> 3,
    (sal_uInt16*)mso_sptSunSegm, sizeof( mso_sptSunSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptSunCalc, sizeof( mso_sptSunCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault5400,
    (sal_Int32*)mso_sptSunTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptMoonVert[] =      // adj value 0 -> 18900
{
    21600, 0,
    3 MSO_I, 4 MSO_I, 0 MSO_I, 5080, 0 MSO_I, 10800,    // ccp
    0 MSO_I, 16520, 3 MSO_I, 5 MSO_I, 21600, 21600,     // ccp
    9740, 21600, 0, 16730, 0, 10800,                    // ccp
    0, 4870, 9740, 0, 21600, 0                          // ccp
};
static const sal_uInt16 mso_sptMoonSegm[] =
{
    0x4000, 0x2004, 0x6000, 0x8000
};
static const SvxMSDffCalculationData mso_sptMoonCalc[] =
{
    { 0x2000, DFF_Prop_adjustValue, 0, 0 },
    { 0x8000, 21600, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x401, 1, 2 },
    { 0x6000, 0x402, DFF_Prop_adjustValue, 0 },
    { 0x2001, DFF_Prop_adjustValue, 1794, 10000 },
    { 0x8000, 21600, 0, 0x0404 },
    { 0x2001, DFF_Prop_adjustValue, 4000, 18900 },
    { 0x8081, 0, 10800, 0x406 },
    { 0x8082, 0, 10800, 0x406 },
    { 0x6000, 0x407, 0x407, 0 },
    { 0x8000, 21600, 0, 0x408 }
};
static const sal_Int32 mso_sptMoonTextRect[] =
{
    1,  9 MSO_I, 8 MSO_I, 0 MSO_I,  0xa MSO_I
};
static const mso_AutoShape msoMoon =
{
    (sal_Int32*)mso_sptMoonVert, sizeof( mso_sptMoonVert ) >> 3,
    (sal_uInt16*)mso_sptMoonSegm, sizeof( mso_sptMoonSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptMoonCalc, sizeof( mso_sptMoonCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault10800,
    (sal_Int32*)mso_sptMoonTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptBracketPairVert[] =       // adj value 0 -> 10800
{
    0 MSO_I,0,      0,      1 MSO_I,    // left top alignment
    0,      2 MSO_I,0 MSO_I,21600,      // left  bottom "
    3 MSO_I,21600,  21600,  2 MSO_I,    // right bottom "
    21600, 1 MSO_I, 3 MSO_I,0           // right top    "
};
static const sal_uInt16 mso_sptBracketPairSegm[] =
{
    0x4000, 0xa701, 0x0001, 0xa801, 0x8000,
    0x4000, 0xa701, 0x0001, 0xa801, 0x8000
};
static const SvxMSDffCalculationData mso_sptBracketPairCalc[] =
{
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0x2082, DFF_Prop_adjustValue, 0, 450 },
    { 0x2000, 0x404, 0, 10800 },
    { 0x8000, 0, 0, DFF_Prop_adjustValue },
    { 0xa000, 0x406, 0, 0x405 },
    { 0xa000, DFF_Prop_geoLeft, 0, 0x407 },
    { 0xa000, DFF_Prop_geoTop, 0, 0x407 },
    { 0x6000, DFF_Prop_geoRight, 0x407, 0 },
    { 0x6000, DFF_Prop_geoBottom, 0x407, 0 },
    { 0xa000, DFF_Prop_geoLeft, 0, 0x405 },
    { 0xa000, DFF_Prop_geoTop, 0, 0x405 },
    { 0x6000, DFF_Prop_geoRight, 0x405, 0 },
    { 0x6000, DFF_Prop_geoBottom, 0x405, 0 }
};
static const sal_Int32 mso_sptBracketPairTextRect[] =
{
    1, 8 MSO_I, 9 MSO_I, 0xa MSO_I, 0xb MSO_I
};
static const mso_AutoShape msoBracketPair =
{
    (sal_Int32*)mso_sptBracketPairVert, sizeof( mso_sptBracketPairVert ) >> 3,
    (sal_uInt16*)mso_sptBracketPairSegm, sizeof( mso_sptBracketPairSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptBracketPairCalc, sizeof( mso_sptBracketPairCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault3700,
    (sal_Int32*)mso_sptBracketPairTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_uInt16 mso_sptPlaqueSegm[] =
{
    0x4000, 0xa801, 0x0001, 0xa701, 0x0001, 0xa801, 0x0001, 0xa701, 0x6000, 0x8000
};
static const sal_Int32 mso_sptPlaqueTextRect[] =
{
    1, 0xc MSO_I, 0xd MSO_I, 0xe MSO_I, 0xf MSO_I
};
static const mso_AutoShape msoPlaque =
{
    (sal_Int32*)mso_sptBracketPairVert, sizeof( mso_sptBracketPairVert ) >> 3,
    (sal_uInt16*)mso_sptPlaqueSegm, sizeof( mso_sptPlaqueSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptBracketPairCalc, sizeof( mso_sptBracketPairCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault3600,
    (sal_Int32*)mso_sptPlaqueTextRect,
    NULL,
    0x80000000, 0x80000000
};

static const sal_Int32 mso_sptBracePairVert[] =     // adj value 0 -> 5400
{
    4 MSO_I,0,      0 MSO_I,1 MSO_I,            // left bracket
    0 MSO_I,6 MSO_I,0       ,10800,
    0 MSO_I,7 MSO_I,0 MSO_I,2 MSO_I,
    4 MSO_I,21600,
    8 MSO_I,21600,  3 MSO_I, 2 MSO_I,           // right bracket
    3 MSO_I,7 MSO_I,21600,  10800,
    3 MSO_I,6 MSO_I,3 MSO_I, 1 MSO_I,
    8 MSO_I,0
};
static const sal_uInt16 mso_sptBracePairSegm[] =
{
    0x4000, 0xa701, 0x0001, 0xa801, 0xa701, 0x0001, 0xa801, 0x8000,
    0x4000, 0xa701, 0x0001, 0xa801, 0xa701, 0x0001, 0xa801, 0x8000
};
static const SvxMSDffCalculationData mso_sptBracePairCalc[] =
{
    { 0x6000, DFF_Prop_geoLeft, DFF_Prop_adjustValue, 0 },
    { 0x6000, DFF_Prop_geoTop, DFF_Prop_adjustValue, 0 },
    { 0xa000, DFF_Prop_geoBottom, 0, DFF_Prop_adjustValue },
    { 0xa000, DFF_Prop_geoRight, 0, DFF_Prop_adjustValue },
    { 0x2001, 0x400, 2, 1 },                                    //  4
    { 0x2001, DFF_Prop_adjustValue, 2, 1 },                     //  5
    { 0x8000, 10800, 0, DFF_Prop_adjustValue },                 //  6
    { 0x8000, 21600, 0, 0x406 },                                //  7
    { 0xa000, DFF_Prop_geoRight, 0, 0x405 },                    //  8
    { 0x2001, DFF_Prop_adjustValue, 1, 3 },                     //  9
    { 0x6000, 0x409, DFF_Prop_adjustValue, 0 },                 // xa
    { 0x6000, DFF_Prop_geoLeft, 0x40a, 0 },                     // xb
    { 0x6000, DFF_Prop_geoTop, 0x409, 0 },                      // xc
    { 0xa000, DFF_Prop_geoRight, 0, 0x40a },                    // xd
    { 0xa000, DFF_Prop_geoBottom, 0, 0x409 }                    // xe

};
static const sal_Int32 mso_sptBracePairTextRect[] =
{
    1, 0xb MSO_I, 0xc MSO_I, 0xd MSO_I, 0xe MSO_I
};
static const mso_AutoShape msoBracePair =
{
    (sal_Int32*)mso_sptBracePairVert, sizeof( mso_sptBracePairVert ) >> 3,
    (sal_uInt16*)mso_sptBracePairSegm, sizeof( mso_sptBracePairSegm ) >> 1,
    (SvxMSDffCalculationData*)mso_sptBracePairCalc, sizeof( mso_sptBracePairCalc ) / sizeof( SvxMSDffCalculationData ),
    (sal_Int32*)mso_sptDefault1800,
    (sal_Int32*)mso_sptBracePairTextRect,
    NULL,
    0x80000000, 0x80000000
};

class SvxMSDffAdjustmentHandle
{
    sal_Int32   nAdjustValue;
    sal_Int32   nDefaultValue;
    sal_Bool    bIsDefault;

    public :
                SvxMSDffAdjustmentHandle() : nAdjustValue( 0 ), nDefaultValue( 0 ), bIsDefault( TRUE ) {};

    void        SetAdjustValue( sal_Int32 nVal ) { nAdjustValue = nVal; bIsDefault = FALSE; };
    sal_Int32   GetAdjustValue() const { return bIsDefault ? nDefaultValue : nAdjustValue; };
    void        SetDefaultValue( sal_Int32 nVal ) { nDefaultValue = nVal; };
    sal_Bool    IsDefault() const { return bIsDefault; };
};

SvxMSDffAutoShape::~SvxMSDffAutoShape()
{
    if ( bVertAlloc )
        delete pVertData;
    if ( bSegAlloc )
        delete pSegData;
    if ( bTextRectAlloc )
        delete pTextRectData;
    if ( bCalcAlloc )
        delete pCalculationData;
    delete[] pAdjustmentHandles;
}

SvxMSDffAutoShape::SvxMSDffAutoShape( const DffPropertyReader& rPropReader, SvStream& rSt,
                                        DffObjData& rData, Rectangle& rGeo, sal_Int32 nAngle ) :
    eSpType             ( rData.eShapeType ),
    aSnapRect           ( rGeo ),
    nFix16Angle         ( nAngle ),
    nXRef               ( 0x80000000 ),
    nYRef               ( 0x80000000 ),
    nColorData          ( 0 ),
    nCalculationData    ( 0 ),
    nAdjustmentHandles  ( 0 ),
    pAdjustmentHandles  ( NULL ),
    pVertData           ( NULL ),
    pSegData            ( NULL ),
    pCalculationData    ( NULL ),
    pTextRectData       ( NULL ),
    bIsEmpty            ( TRUE ),
    bVertAlloc          ( FALSE ),
    bSegAlloc           ( FALSE ),
    bCalcAlloc          ( FALSE ),
    bTextRectAlloc      ( FALSE ),
    bTextFlow           ( ( (MSO_TextFlow)rPropReader.GetPropertyValue( DFF_Prop_txflTextFlow ) ) == mso_txflTtoBA ),
    bFilled             ( ( rPropReader.GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 ) != 0 ),    // pie <-> arc
    bFlipH              ( ( rData.nSpFlags & SP_FFLIPH ) != 0 ),
    bFlipV              ( ( rData.nSpFlags & SP_FFLIPV ) != 0 )
{
    const sal_Int32*        pBoundRect = NULL;
    const sal_Int32*        pDefData = NULL;
    const mso_AutoShape*    pDefAutoShape = NULL;

    switch( eSpType )
    {
        case mso_sptRectangle :             pDefAutoShape = &msoRectangle; break;
        case mso_sptParallelogram :         pDefAutoShape = &msoParallelogram; break;
        case mso_sptTrapezoid :             pDefAutoShape = &msoTrapezoid; break;
        case mso_sptDiamond :               pDefAutoShape = &msoDiamond; break;
        case mso_sptRoundRectangle :        pDefAutoShape = &msoRoundRectangle; break;
        case mso_sptOctagon :               pDefAutoShape = &msoOctagon; break;
        case mso_sptIsocelesTriangle :      pDefAutoShape = &msoIsocelesTriangle; break;
        case mso_sptRightTriangle :         pDefAutoShape = &msoRightTriangle; break;
        case mso_sptEllipse :               pDefAutoShape = &msoEllipse; break;
        case mso_sptHexagon :               pDefAutoShape = &msoHexagon; break;
        case mso_sptPlus :                  pDefAutoShape = &msoPlus; break;
        case mso_sptPentagon :              pDefAutoShape = &msoPentagon; break;
        case mso_sptCan :
        {
            pDefAutoShape = &msoCan;
            nColorData = 0x20200000;
        }
        break;
        case mso_sptCube :
        {
            pDefAutoShape = &msoCube;
            nColorData = 0x302d0000;
        }
        break;

        case mso_sptActionButtonBlank :
        {
            pDefAutoShape = &msoActionButtonBlank;
            nColorData = 0x502ad400;
        }
        break;
        case mso_sptActionButtonHome :
        {
            pDefAutoShape = &msoActionButtonHome;
            nColorData = 0x702ad4ad;
        }
        break;
        case mso_sptActionButtonHelp :
        {
            pDefAutoShape = &msoActionButtonHelp;
            nColorData = 0x602ad4a0;
        }
        break;
        case mso_sptActionButtonInformation :
        {
            pDefAutoShape = &msoActionButtonInformation;
            nColorData = 0x702ad4a5;
        }
        break;
        case mso_sptActionButtonBackPrevious :
        {
            pDefAutoShape = &msoActionButtonBackPrevious;
            nColorData = 0x602ad4a0;
        }
        break;
        case mso_sptActionButtonForwardNext :
        {
            pDefAutoShape = &msoActionButtonForwardNext;
            nColorData = 0x602ad4a0;
        }
        break;
        case mso_sptActionButtonBeginning :
        {
            pDefAutoShape = &msoActionButtonBeginning;
            nColorData = 0x602ad4a0;
        }
        break;
        case mso_sptActionButtonEnd :
        {
            pDefAutoShape = &msoActionButtonEnd;
            nColorData = 0x602ad4a0;
        }
        break;
        case mso_sptActionButtonReturn :
        {
            pDefAutoShape = &msoActionButtonReturn;
            nColorData = 0x602ad4a0;
        }
        break;
        case mso_sptActionButtonDocument :
        {
            pDefAutoShape =&msoActionButtonDocument;
            nColorData = 0x702ad4da;
        }
        break;
        case mso_sptActionButtonSound :
        {
            pDefAutoShape = &msoActionButtonSound;
            nColorData = 0x602ad4a0;
        }
        break;
        case mso_sptActionButtonMovie :
        {
            pDefAutoShape = &msoActionButtonMovie;
            nColorData = 0x602ad4a0;
        }
        break;
        case mso_sptBevel :
        {
            pDefAutoShape = &msoBevel;
            nColorData = 0x502ad400;
        }
        break;

        case mso_sptFoldedCorner :
        {
            pDefAutoShape = &msoFoldedCorner;
            nColorData = 0x20d00000;
        }
        break;
        case mso_sptSmileyFace :
        {
            pDefAutoShape = &msoSmileyFace;
            nColorData = 0x20d00000;
        }
        break;
        case mso_sptDonut :                 pDefAutoShape = &msoDonut; break;
        case mso_sptNoSmoking :             pDefAutoShape = &msoNoSmoking; break;
//      case mso_sptBlockArc :              break;
        case mso_sptHeart :                 pDefAutoShape = &msoHeart; break;
        case mso_sptLightningBolt :         pDefAutoShape = &msoLightningBold; break;
        case mso_sptSun :                   pDefAutoShape = &msoSun; break;
        case mso_sptMoon :                  pDefAutoShape = &msoMoon; break;
        case mso_sptBracketPair :           pDefAutoShape = &msoBracketPair; break;
        case mso_sptBracePair :             pDefAutoShape = &msoBracePair; break;
        case mso_sptPlaque :                pDefAutoShape = &msoPlaque; break;
        case mso_sptArrow :                 pDefAutoShape = &msoArrow; break;
        case mso_sptUpArrow :               pDefAutoShape = &msoUpArrow; break;
        case mso_sptDownArrow :             pDefAutoShape = &msoDownArrow; break;
        case mso_sptLeftArrow :             pDefAutoShape = &msoLeftArrow; break;
        case mso_sptLeftRightArrow :        pDefAutoShape = &msoLeftRightArrow; break;
        case mso_sptUpDownArrow :           pDefAutoShape = &msoUpDownArrow; break;
        case mso_sptQuadArrow :             pDefAutoShape = &msoQuadArrow; break;
        case mso_sptLeftRightUpArrow :      pDefAutoShape = &msoLeftRightUpArrow; break;
        case mso_sptBentArrow :             pDefAutoShape = &msoBentArrow; break;
        case mso_sptLeftUpArrow :           pDefAutoShape = &msoLeftUpArrow; break;
        case mso_sptBentUpArrow :           pDefAutoShape = &msoBentUpArrow; break;
//      case mso_sptCurvedRightArrow :      pDefAutoShape = &msoCurvedRightArrow; break;    // !!!!!
        case mso_sptStripedRightArrow :     pDefAutoShape = &msoStripedRightArrow; break;
        case mso_sptHomePlate :             pDefAutoShape = &msoHomePlate; break;
        case mso_sptChevron :               pDefAutoShape = &msoChevron; break;
        case mso_sptRightArrowCallout :     pDefAutoShape = &msoRightArrowCallout; break;
        case mso_sptLeftArrowCallout :      pDefAutoShape = &msoLeftArrowCallout; break;
        case mso_sptUpArrowCallout :        pDefAutoShape = &msoUpArrowCallout; break;
        case mso_sptDownArrowCallout :      pDefAutoShape = &msoDownArrowCallout; break;
        case mso_sptLeftRightArrowCallout : pDefAutoShape = &msoLeftRightArrowCallout; break;
        case mso_sptUpDownArrowCallout :    pDefAutoShape = &msoUpDownArrowCallout; break;
        case mso_sptQuadArrowCallout :      pDefAutoShape = &msoQuadArrowCallout; break;
        case mso_sptCircularArrow :         pDefAutoShape = &msoCircularArrow; break;
        default :
        break;
    }
    if ( pDefAutoShape )
    {
        nNumElemVert = pDefAutoShape->nVertices;
        pVertData = pDefAutoShape->pVertices;
        nNumElemSeg = pDefAutoShape->nElements;
        pSegData = pDefAutoShape->pElements;
        nCalculationData = pDefAutoShape->nCalculation;
        pCalculationData = pDefAutoShape->pCalculation;
        pDefData = pDefAutoShape->pDefData;
        pTextRectData = pDefAutoShape->pTextRect;
        pBoundRect = pDefAutoShape->pBoundRect;
        nXRef = pDefAutoShape->nXRef;
        nYRef = pDefAutoShape->nYRef;
    }
    if ( rPropReader.SeekToContent( DFF_Prop_pVertices, rSt ) )
    {
        sal_uInt16 nTmp16, nNumElemMemVert, nElemSizeVert;
        rSt >> nTmp16 >> nNumElemMemVert >> nElemSizeVert;
        if ( nTmp16 )
        {
            nNumElemVert = nTmp16;
            sal_uInt32 i = nNumElemVert << 1;
            bVertAlloc = TRUE;
            pVertData = new sal_Int32[ i ];
            sal_Int32 *pTmp = pVertData;
            if ( nElemSizeVert == 8 )
            {
                while( i-- )
                {
                    rSt >> *pTmp++;
                }
            }
            else
            {
                sal_Int16 nTmp;
                while ( i-- )
                {
                    rSt >> nTmp;
                    *pTmp++ = nTmp;
                }
            }
        }
    }
    if ( pVertData || pDefAutoShape )
    {
        bIsEmpty = FALSE;
        if ( rPropReader.SeekToContent( DFF_Prop_pSegmentInfo, rSt ) )
        {
            sal_uInt16 nTmp16, nNumElemMemSeg, nElemSizeSeg;
            rSt >> nTmp16 >> nNumElemMemSeg >> nElemSizeSeg;
            if ( nTmp16 )
            {
                nNumElemSeg = nTmp16;
                bSegAlloc = TRUE;
                pSegData = new sal_uInt16[ nNumElemSeg ];
#ifdef __BIGENDIAN
                sal_uInt32 i = nNumElemSeg;
                sal_uInt16* pTmp = pSegData;
                while( i-- )

                {
                    rSt >> *pTmp++;
                }
#else
                rSt.Read( pSegData, nNumElemSeg << 1 );
#endif
            }
        }
        if ( rPropReader.SeekToContent( 342, rSt ) )
        {
            sal_uInt16 nTmp16, nNumElemMem, nElemSize;
            rSt >> nTmp16 >> nNumElemMem >> nElemSize;
            if ( nTmp16 && ( nElemSize == 8 ) )
            {
                nCalculationData = nTmp16;
                pCalculationData = new SvxMSDffCalculationData[ nCalculationData ];
                bCalcAlloc = TRUE;
                sal_uInt32 i;
                sal_uInt16 nVal0, nVal1, nVal2;
                for ( i = 0; i < nCalculationData; i++ )
                {
                    SvxMSDffCalculationData& rDat = pCalculationData[ i ];
                    rSt >> rDat.nFlags
                        >> nVal0 >> nVal1 >> nVal2;
                    rDat.nVal[ 0 ] = nVal0;
                    rDat.nVal[ 1 ] = nVal1;
                    rDat.nVal[ 2 ] = nVal2;
                }
            }
        }

        if ( rPropReader.SeekToContent( 343, rSt ) )
        {
            sal_uInt16 nTmp16, nNumElemMem, nElemSize;
            rSt >> nTmp16 >> nNumElemMem >> nElemSize;
            if ( nTmp16 && ( nElemSize == 16 ) )
            {
                sal_Int32 nNumElem = nTmp16;
                if ( nNumElem > 1 )
                    nNumElem = 2;
                bTextRectAlloc = TRUE;
                pTextRectData = new sal_Int32[ ( nNumElemMem << 2 ) + 1 ];
                sal_Int32* pTmp = pTextRectData;
                *pTmp++ = nNumElem;
#ifdef __BIGENDIAN
                sal_uInt32 i = nNumElem << 2;
                while( i-- )
                {
                    rSt >> *pTmp++;
                }
#else
                rSt.Read( pTmp, nNumElem << 4 );
#endif
            }
        }

        const sal_Int32* pTmp = pDefData;
        sal_Int32 nDefaults = 0;
        if ( pTmp )
            nDefaults = nAdjustmentHandles = *pTmp++;
        sal_Int32 i = 10;
        sal_Int32 nProperty = DFF_Prop_adjust10Value;
        do
        {
            if ( rPropReader.IsProperty( nProperty-- ) )
                break;
        }
        while ( --i );
        if ( i > nAdjustmentHandles )
            nAdjustmentHandles = i;
        if ( nAdjustmentHandles )
        {
            pAdjustmentHandles = new SvxMSDffAdjustmentHandle[ nAdjustmentHandles ];
            for ( i = 0; i < nAdjustmentHandles; i++ )
            {
                if ( i < nDefaults )
                    pAdjustmentHandles[ i ].SetDefaultValue( *pTmp++ );
                if ( rPropReader.IsProperty( i + DFF_Prop_adjustValue ) )
                    pAdjustmentHandles[ i ].SetAdjustValue( rPropReader.GetPropertyValue( i + DFF_Prop_adjustValue ) );
            }
        }
        if ( pBoundRect )
            aBoundRect = Rectangle( Point( pBoundRect[ 0 ], pBoundRect[ 1 ] ), Point( pBoundRect[ 2 ], pBoundRect[ 3 ] ) );
        else
        {
            if ( pCalculationData )
                aBoundRect = Rectangle( Point(), Size( 21600, 21600 ) );
            else
                aBoundRect = Rectangle( Point(), aSnapRect.GetSize() );
        }
        fXScale = (double)aSnapRect.GetWidth() / (double)aBoundRect.GetWidth();
        fYScale = (double)aSnapRect.GetHeight() / (double)aBoundRect.GetHeight();

        if ( rPropReader.IsProperty( 339 ) )
            nXRef = rPropReader.GetPropertyValue( 339 );
        if ( rPropReader.IsProperty( 340 ) )
            nYRef = rPropReader.GetPropertyValue( 340 );
    }
}

sal_Int32 SvxMSDffAutoShape::Fix16ToAngle( sal_Int32 nAngle ) const
{
    if ( nAngle )
    {
        nAngle = ( (sal_Int16)( nAngle >> 16) * 100L ) + ( ( ( nAngle & 0x0000ffff) * 100L ) >> 16 );
        nAngle = NormAngle360( -nAngle );
    }
    return nAngle;
}

sal_Int32 SvxMSDffAutoShape::GetAdjustValue( sal_Int32 nIndex ) const
{
    return ( nIndex < nAdjustmentHandles ) ? pAdjustmentHandles[ nIndex ].GetAdjustValue() : 0;
}

sal_Int32 SvxMSDffAutoShape::GetAdjustValue( sal_Int32 nIndex, sal_Int32 nDefault ) const
{
    if ( ( nIndex >= nAdjustmentHandles ) || pAdjustmentHandles[ nIndex ].IsDefault() )
        return nDefault;
    else
        return pAdjustmentHandles[ nIndex ].GetAdjustValue();
}

sal_Int32 SvxMSDffAutoShape::GetValue( sal_uInt32 nDat, sal_Bool bScale, sal_Bool bScaleWidth ) const
{
    double fVal;
    sal_uInt32  nGeometryFlags = 0;
    if ( ( nDat >> 16 ) == 0x8000 )
        fVal = ImplGetValue( (sal_uInt16)nDat, nGeometryFlags );
    else
        fVal = (sal_Int32)nDat;

    if ( bScale )
    {
        if ( nGeometryFlags & 0xc )
        {

        }
        if ( bScaleWidth )
        {
            if ( ( aSnapRect.GetWidth() > aSnapRect.GetHeight() ) && ( ( nXRef != 0x80000000 ) || nGeometryFlags ) )
            {
                sal_Bool bGeo = ( ( ( nGeometryFlags & 1 ) == 0 ) && ( fVal > nXRef ) )
                                    || ( ( nGeometryFlags & 4 ) != 0 );
                if ( ( nGeometryFlags & 5 ) == 5 )
                {
                    fVal -= (double)aBoundRect.GetWidth() * 0.5;
                    fVal *= fYScale;
                    fVal += (double)aSnapRect.GetWidth() * 0.5;
                }
                else
                {
                    fVal *= fYScale;
                    if ( bGeo )
                        fVal += (double)aBoundRect.GetWidth() * fXScale - (double)aBoundRect.GetWidth() * fYScale;
                }
            }
            else
            {
                fVal -= aBoundRect.Left();
                fVal *= fXScale;
            }
        }
        else
        {
            if ( ( aSnapRect.GetHeight() > aSnapRect.GetWidth() ) && ( ( nYRef != 0x80000000 ) || nGeometryFlags ) )
            {
                sal_Bool bGeo = ( ( ( nGeometryFlags & 2 ) == 0 ) && ( fVal > nYRef ) )
                                    || ( ( nGeometryFlags & 8 ) != 0 );
                if ( ( nGeometryFlags & 10 ) == 10 )
                {
                    fVal -= (double)aBoundRect.GetHeight() * 0.5;
                    fVal *= fXScale;
                    fVal += (double)aSnapRect.GetHeight() * 0.5;
                }
                else
                {
                    fVal *= fXScale;
                    if ( bGeo )
                        fVal += (double)aBoundRect.GetHeight() * fYScale - (double)aBoundRect.GetHeight() * fXScale;
                }
            }
            else
            {
                fVal -= aBoundRect.Top();
                fVal *= fYScale;
            }
        }
    }
    return fVal;
}

double SvxMSDffAutoShape::ImplGetValue( sal_Int16 nIndex, sal_uInt32& nGeometryFlags ) const
{
    if ( !pCalculationData )
        return 0;
    if ( nCalculationData <= nIndex )
        return 0;

    double fVal[ 3 ];
    sal_Int16 i, nFlags = pCalculationData[ nIndex ].nFlags;
    for ( i = 0; i < 3; i++ )
    {
        if ( nFlags & ( 0x2000 << i ) )
        {
            sal_Int16 nVal = pCalculationData[ nIndex ].nVal[ i ];
            if ( nVal & 0x400 )
                fVal[ i ] = ImplGetValue( nVal & 0xff, nGeometryFlags );
            else
            {
                switch ( nVal )
                {
                    case DFF_Prop_adjustValue :
                    case DFF_Prop_adjust2Value :
                    case DFF_Prop_adjust3Value :
                    case DFF_Prop_adjust4Value :
                    case DFF_Prop_adjust5Value :
                    case DFF_Prop_adjust6Value :
                    case DFF_Prop_adjust7Value :
                    case DFF_Prop_adjust8Value :
                    case DFF_Prop_adjust9Value :
                    case DFF_Prop_adjust10Value :
                        fVal[ i ] = GetAdjustValue( nVal - DFF_Prop_adjustValue );
                    break;
                    case DFF_Prop_geoLeft :
                    {
                        nGeometryFlags |= 1;
                        fVal[ i ]  = 0.0;
                    }
                    break;
                    case DFF_Prop_geoTop :
                    {
                        nGeometryFlags |= 2;
                        fVal[ i ]  = 0.0;
                    }
                    break;
                    case DFF_Prop_geoRight :
                    {
                        nGeometryFlags |= 4;
                        fVal[ i ] = aBoundRect.GetWidth();
                    }
                    break;
                    case DFF_Prop_geoBottom :
                    {
                        nGeometryFlags |= 8;
                        fVal[ i ] = aBoundRect.GetHeight();
                    }
                    break;
                    default:
                        fVal[ i ]  = 0.0;
                    break;
                }
            }
        }
        else
            fVal[ i ] = pCalculationData[ nIndex ].nVal[ i ];
    }
    switch ( nFlags & 0xff )
    {
        case 0 :
        {
            fVal[ 0 ] += fVal[ 1 ];
            fVal[ 0 ] -= fVal[ 2 ];
        }
        break;

        case 1 :
        {
            if ( fVal[ 1 ] != 0.0 )
                fVal[ 0 ] *= fVal[ 1 ];
            if ( fVal[ 2 ] != 0.0 )
                fVal[ 0 ] /= fVal[ 2 ];
        }
        break;

        case 3 :
        {

        }
        break;

        case 6 :
        {
            return fVal[ 0 ];
        }
        break;

        case 9 :    // in this special case the second parameter is a fixed fload
        {           // which has to be divided by 0x10000
            fVal[ 0 ] *= sin( fVal[ 1 ] / 65536 * F_PI180 );
        }
        break;

        case 0xa :  // in this special case the second parameter is a fixed fload
        {           // which has to be divided by 0x10000
            fVal[ 0 ] *= cos( fVal[ 1 ] / 65536 * F_PI180 );
        }
        break;

        case 0xe :
        {
            return fVal[ 0 ];
        }
        break;

        case 0x80 :
        {
            // fVal[0]^2 + fVal[1]^2 = fVal[2]^2
            if ( fVal[ 2 ] == 0.0 )
                fVal[ 0 ] = sqrt( fVal[ 0 ] * fVal[ 0 ] + fVal[ 1 ] * fVal[ 1 ] );
            else
            {
                double fA = fVal[ 0 ] != 0.0 ? fVal[ 0 ] : fVal[ 1 ];
                fVal[ 0 ] = sqrt( fVal[ 2 ] * fVal[ 2 ] - fA * fA );
            }
        }
        break;
        case 0x81 :
        {
            double fAngle = F_PI1800 * fVal[ 2 ];
            fVal[ 0 ] = ( cos( fAngle ) * ( fVal[ 0 ] - 10800 )
                            + sin( fAngle ) * ( fVal[ 1 ] - 10800 ) ) + 10800;
        }
        break;
        case 0x82 :
        {
            double fAngle = F_PI1800 * fVal[ 2 ];
            fVal[ 0 ] = - ( sin( fAngle ) * ( fVal[ 0 ] - 10800 )
                            - cos( fAngle ) * ( fVal[ 1 ] - 10800 ) ) + 10800;
        }
        break;
        default :
        {

        }
        break;
    }
    return fVal[ 0 ];
}

// nLumDat 28-31 = number of luminance entries in nLumDat
// nLumDat 27-24 = nLumDatEntry 0
// nLumDat 23-20 = nLumDatEntry 1 ...
// each 4bit entry is to be interpreted as a 10 percent signed luminance changing
Color SvxMSDffAutoShape::ImplGetColorData( const Color& rFillColor, sal_uInt32 nIndex )
{
    Color aRetColor;

    sal_uInt32 i, nColor, nTmp, nCount = nColorData >> 28;

    if ( nCount )
    {
        if ( nIndex >= nCount )
            nIndex = nCount - 1;

        sal_uInt32 nFillColor = (sal_uInt32)rFillColor.GetRed() |
                                    ((sal_uInt32)rFillColor.GetGreen() << 8 ) |
                                        ((sal_uInt32)rFillColor.GetBlue() << 16 );

        sal_Int32 nLumDat = nColorData << ( ( 1 + nIndex ) << 2 );
        sal_Int32 nLuminance = ( nLumDat >> 28 ) * 12;

        nTmp = nFillColor;
        nColor = 0;
        for ( i = 0; i < 3; i++ )
        {
            sal_Int32 nC = (sal_uInt8)nTmp;
            nTmp >>= 8;
            nC += ( ( nLuminance * nC ) >> 8 );
            if ( nC < 0 )
                nC = 0;
            else if ( nC &~ 0xff )
                nC = 0xff;
            nColor >>= 8;
            nColor |= nC << 16;
        }
        aRetColor = Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
    }
    return aRetColor;
}

sal_Bool SvxMSDffAutoShape::IsEmpty() const
{
    return bIsEmpty;
}

Rectangle SvxMSDffAutoShape::GetTextRect() const
{
    sal_Int32   nTextRectCount = 0;
    sal_Int32*  pTmp = pTextRectData;
    if ( pTmp )
        nTextRectCount = *pTmp++;

    if ( !nTextRectCount || ( aBoundRect.GetWidth() == 0 ) || ( aBoundRect.GetHeight() == 0 ) )
        return aSnapRect;
    sal_Int32 aAry[ 4 ], nPtNum;
    if ( bTextFlow && ( nTextRectCount > 1 ) )
        pTmp += 4;
    for ( nPtNum = 0; nPtNum < 4; nPtNum++ )
        aAry[ nPtNum ] = GetValue( *pTmp++, TRUE, ( nPtNum & 1 ) != 1 );
    Point aTopLeft( aAry[ 0 ], aAry[ 1 ] );
    Point aBottomRight( aAry[ 2 ], aAry[ 3 ] );
    Rectangle aRect( aTopLeft, aBottomRight );
    aRect.Move( aSnapRect.Left(), aSnapRect.Top() );
    return aRect;
}

SdrObject* SvxMSDffAutoShape::GetObject( SdrModel* pSdrModel, SfxItemSet& rSet, sal_Bool bSetAutoShapeAdjustItem )
{
    SdrObject* pRet = NULL;

    if ( !IsEmpty() )
    {
        if ( eSpType == mso_sptRectangle )
            pRet = new SdrRectObj( aSnapRect );
        else if ( eSpType == mso_sptRoundRectangle )
        {
            sal_Int32 nW = aSnapRect.Right() - aSnapRect.Left();
            sal_Int32 nH = aSnapRect.Bottom() - aSnapRect.Top();
            if ( nH < nW )
                nW = nH;
            double fAdjust = (double)GetAdjustValue( 0, 3600 ) / 21600.0;
            nW = (double)nW * fAdjust;
            rSet.Put( SdrEckenradiusItem( nW ) );
            pRet = new SdrRectObj( aSnapRect );
        }
        else if ( eSpType == mso_sptEllipse )
            pRet = new SdrCircObj( OBJ_CIRC, aSnapRect );

        if ( pRet )
        {
            pRet->SetModel( pSdrModel );
            pRet->NbcSetAttributes( rSet, FALSE );
        }
        else
        {
            // Header auswerten
            XPolygon aXP( (sal_uInt16)nNumElemVert );
            const sal_Int32* pTmp = pVertData;
            sal_uInt32 nVal32, nPtNum, nPtCount = nNumElemVert << 1;

            sal_Bool bScale = ( eSpType != mso_sptArc );

            for ( nPtNum = 0; nPtNum < nPtCount; nPtNum++ )
            {
                nVal32 = GetValue( *pTmp++, bScale, ( nPtNum & 1 ) != 1 );

                if ( nPtNum & 1 )
                    aXP[ (sal_uInt16)( nPtNum >> 1 ) ].Y() = nVal32;
                else
                    aXP[ (sal_uInt16)( nPtNum >> 1 ) ].X() = nVal32;
            }
            Rectangle   aPolyBoundRect( aXP.GetBoundRect() );

            // the arc is something special, because sometimes the snaprect does not match
            if ( eSpType == mso_sptArc )
            {
                // Groesse des Polygons mit allen Punkten korrigieren

                sal_Int32   nEndAngle = Fix16ToAngle( GetAdjustValue( 0, 270 << 16 ) );
                sal_Int32   nStartAngle = Fix16ToAngle( GetAdjustValue( 1, 0 ) );

                if ( nStartAngle == nEndAngle )
                    return NULL;

                if ( bFilled )      // ( filled ) ? we have to import an pie : we have to construct an arc
                    pRet = new SdrCircObj( OBJ_SECT, aPolyBoundRect, nStartAngle, nEndAngle );
                else
                {
                    Point aStart, aEnd, aCenter( aPolyBoundRect.Center() );
                    aStart.X() = (sal_Int32)( ( cos( ( (double)nStartAngle * F_PI18000 ) ) * 1000.0 ) );
                    aStart.Y() = - (sal_Int32)( ( sin( ( (double)nStartAngle * F_PI18000 ) ) * 1000.0 ) );
                    aEnd.X() = (sal_Int32)( ( cos( ( (double)nEndAngle * F_PI18000 ) ) * 1000.0 ) );
                    aEnd.Y() = - (sal_Int32)( ( sin( ( (double)nEndAngle * F_PI18000 ) ) * 1000.0 ) );
                    aStart.X() += aCenter.X();
                    aStart.Y() += aCenter.Y();
                    aEnd.X() += aCenter.X();
                    aEnd.Y() += aCenter.Y();

                    Polygon aPolygon( aPolyBoundRect, aStart, aEnd, POLY_PIE );
                    Rectangle aPolyPieRect( aPolygon.GetBoundRect() );

                    USHORT nPt = aPolygon.GetSize();

                    if ( nPt < 4 )
                        return NULL;

                    aPolygon[ 0 ] = aPolygon[ 1 ];                              // try to get the arc boundrect
                    aPolygon[ nPt - 1 ] = aPolygon[ nPt - 2 ];
                    Rectangle aPolyArcRect( aPolygon.GetBoundRect() );

                    if ( aPolyArcRect != aPolyPieRect )
                    {
                        double  fYScale, fXScale;
                        double  fYOfs, fXOfs;
                        int     nCond;

                        fYOfs = fXOfs = 0.0;
                        if ( aPolyPieRect.GetWidth() != aPolyArcRect.GetWidth() )
                        {
                            nCond = ( (sal_uInt32)( nStartAngle - 9000 ) > 18000 ) && ( (sal_uInt32)( nEndAngle - 9000 ) > 18000 ) ? 1 : 0;
                            nCond ^= bFlipH ? 1 : 0;
                            if ( nCond )
                            {
                                fXScale = (double)aSnapRect.GetWidth() / (double)aPolyPieRect.GetWidth();
                                fXOfs = ( (double)aPolyPieRect.GetWidth() - (double)aPolyArcRect.GetWidth() ) * fXScale;
                            }
                        }
                        if ( aPolyPieRect.GetHeight() != aPolyArcRect.GetHeight() )
                        {
                            nCond = ( ( nStartAngle > 18000 ) && ( nEndAngle > 18000 ) ) ? 1 : 0;
                            nCond ^= bFlipV ? 1 : 0;
                            if ( nCond )
                            {
                                fYScale = (double)aSnapRect.GetHeight() / (double)aPolyPieRect.GetHeight();
                                fYOfs = ( (double)aPolyPieRect.GetHeight() - (double)aPolyArcRect.GetHeight() ) * fYScale;
                            }
                        }
                        fXScale = (double)aPolyArcRect.GetWidth() / (double)aPolyPieRect.GetWidth();
                        fYScale = (double)aPolyArcRect.GetHeight() / (double)aPolyPieRect.GetHeight();

                        aSnapRect = Rectangle( Point( aSnapRect.Left() + fXOfs, aSnapRect.Top() + fYOfs ),
                            Size( aSnapRect.GetWidth() * fXScale, aSnapRect.GetHeight() * fYScale ) );

                    }
                    pRet = new SdrCircObj( OBJ_CARC, aPolyBoundRect, nStartAngle, nEndAngle );

                }
                pRet->NbcSetSnapRect( aSnapRect );
                pRet->SetModel( pSdrModel );
                pRet->NbcSetAttributes( rSet, FALSE );
            }
            if ( !pRet )
            {
                if ( aPolyBoundRect.GetSize() != aSnapRect.GetSize() )
                {
                    double      fXScale = (double)aSnapRect.GetWidth() / (double)aPolyBoundRect.GetWidth();
                    double      fYScale = (double)aSnapRect.GetHeight() / (double)aPolyBoundRect.GetHeight();
                    aXP.Scale( fXScale, fYScale );
                }
                if ( !pSegData )
                {
                    FASTBOOL bClosed = aXP[ 0 ] == aXP[ (sal_uInt16)( aXP.GetPointCount() - 1 ) ];
                    pRet = new SdrPathObj( bClosed ? OBJ_POLY : OBJ_PLIN, aXP );
                    pRet->NbcSetSnapRect( aSnapRect );
                    pRet->SetModel( pSdrModel );
                    pRet->NbcSetAttributes( rSet, FALSE );
                }
                else
                {
                    SdrObjGroup*    pGrp = NULL;
                    SdrPathObj*     pSdrPathObj = NULL;

                    XPolyPolygon    aPolyPoly;
                    XPolygon        aPoly;

                    XPolyPolygon    aEmptyPolyPoly;
                    XPolygon        aEmptyPoly;

                    BOOL            bClosed;
                    sal_uInt16      nPolyFlags;

                    Color           aFillColor( COL_WHITE );
                    sal_uInt32      nColorCount = nColorData >> 28;
                    sal_uInt32      nColorIndex = 0;
                    sal_uInt16      nSrcPt = 0;

                    Rectangle       aUnion;

                    const sal_uInt16* pTmp = pSegData;

                    if ( nColorCount )
                    {
                        const SfxPoolItem* pPoolItem = NULL;
                        SfxItemState eState = rSet.GetItemState( XATTR_FILLCOLOR, FALSE, &pPoolItem );
                        if( SFX_ITEM_SET == eState )
                        {
                            if ( pPoolItem )
                                aFillColor = ((XFillColorItem*)pPoolItem)->GetValue();
                        }
                    }
                    for ( sal_uInt16 i = 0; i < nNumElemSeg; i++ )
                    {
                        nPolyFlags = *pTmp++;
                        switch ( nPolyFlags >> 12 )
                        {
                            case 0x4 :
                            {
                                if ( aPoly.GetPointCount() > 1 )
                                {
                                    if ( bClosed )
                                        aPoly[ aPoly.GetPointCount() ] = aPoly[ 0 ];
                                    aPolyPoly.Insert( aPoly );
                                }
                                bClosed = FALSE;
                                aPoly = aEmptyPoly;
                                aPoly[ 0 ] = aXP[ nSrcPt++ ];
                            }
                            break;
                            case 0x8 :
                            {
                                if ( aPoly.GetPointCount() > 1 )
                                {
                                    if ( bClosed )
                                        aPoly[ aPoly.GetPointCount() ] = aPoly[ 0 ];
                                    aPolyPoly.Insert( aPoly );
                                }
                                aPoly = aEmptyPoly;
                                if ( aPolyPoly.Count() )
                                {
                                    if ( pSdrPathObj )
                                    {
                                        pGrp = new SdrObjGroup();
                                        pGrp->SetModel( pSdrModel );
                                        pGrp->NbcSetLogicRect( aSnapRect );
                                        pGrp->GetSubList()->NbcInsertObject( pSdrPathObj );
                                    }
                                    aUnion.Union( aPolyPoly.GetBoundRect() );
                                    pSdrPathObj = new SdrPathObj( bClosed ? OBJ_POLY : OBJ_PLIN, aPolyPoly );
                                    pSdrPathObj->SetModel( pSdrModel );
                                    if ( !bClosed )
                                        rSet.Put( SdrShadowItem( FALSE ) );
                                    else
                                    {
                                        if ( nColorIndex < nColorCount )
                                        {
                                            Color aColor( ImplGetColorData( aFillColor, nColorIndex++ ) );
                                            rSet.Put( XFillColorItem( String(), aColor ) );
                                        }
                                    }
                                    pSdrPathObj->NbcSetAttributes( rSet, FALSE );
                                    if ( pGrp )
                                    {
                                        if ( pSdrPathObj )
                                        {
                                            pGrp->GetSubList()->NbcInsertObject( pSdrPathObj );
                                            pSdrPathObj = NULL;
                                        }
                                    }
                                    aPolyPoly = aEmptyPolyPoly;
                                }
                            }
                            break;
                            case 0x6 :
                            {
                                bClosed = TRUE;
                            }
                            break;
                            case 0x2 :
                            {
                                sal_uInt16 nDstPt = aPoly.GetPointCount();
                                for ( sal_uInt16 i = 0; i < ( nPolyFlags & 0xfff ); i++ )
                                {
                                    aPoly[ nDstPt ] = aXP[ nSrcPt++ ];
                                    aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                    aPoly[ nDstPt ] = aXP[ nSrcPt++ ];
                                    aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                    aPoly[ nDstPt++ ] = aXP[ nSrcPt++ ];
                                }
                            }
                            break;
                            case 0xa :
                            case 0xb :
                            {
                                sal_uInt16 nPntCount = (BYTE)nPolyFlags;
                                if ( nPntCount )
                                {
                                    sal_uInt32 nMod = ( nPolyFlags >> 8 ) & 0xf;
                                    switch ( nMod )
                                    {
                                        case 3 :
                                        case 4 :
                                        case 5 :
                                        {
                                            sal_uInt16 nDstPt = aPoly.GetPointCount();
                                            if ( nDstPt > 1 )
                                            {
                                                if ( bClosed )
                                                    aPoly[ aPoly.GetPointCount() ] = aPoly[ 0 ];
                                                aPolyPoly.Insert( aPoly );
                                                aPoly = aEmptyPoly;
                                                nDstPt = 0;
                                            }
                                            if ( nPntCount == 2 )
                                            {   // create a circle
                                                Rectangle aRect( aXP[ nSrcPt ], aXP[ nSrcPt + 1 ] );
                                                sal_Int32 nXControl = (sal_Int32)((double)aRect.GetWidth() * 0.2835 );
                                                sal_Int32 nYControl = (sal_Int32)((double)aRect.GetHeight() * 0.2835 );
                                                Point     aCenter( aRect.Center() );
                                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Top() );
                                                aPoly[ nDstPt ] = Point( aCenter.X() + nXControl, aRect.Top() );
                                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                aPoly[ nDstPt ] = Point( aRect.Right(), aCenter.Y() - nYControl );
                                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                aPoly[ nDstPt++ ] = Point( aRect.Right(), aCenter.Y() );
                                                aPoly[ nDstPt ] = Point( aRect.Right(), aCenter.Y() + nYControl );
                                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                aPoly[ nDstPt ] = Point( aCenter.X() + nXControl, aRect.Bottom() );
                                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Bottom() );
                                                aPoly[ nDstPt ] = Point( aCenter.X() - nXControl, aRect.Bottom() );
                                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                aPoly[ nDstPt ] = Point( aRect.Left(), aCenter.Y() + nYControl );
                                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                aPoly[ nDstPt++ ] = Point( aRect.Left(), aCenter.Y() );
                                                aPoly[ nDstPt ] = Point( aRect.Left(), aCenter.Y() - nYControl );
                                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                aPoly[ nDstPt ] = Point( aCenter.X() - nXControl, aRect.Top() );
                                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Top() );
                                                nSrcPt += 2;
                                            }
                                            else
                                            {
                                                sal_uInt32 nXor = ( nMod == 5 ) ? 3 : 2;
                                                for ( sal_uInt32 i = 0; i < ( nPntCount >> 2 ); i++ )
                                                {
                                                    PolyStyle ePolyStyle = POLY_ARC;
                                                    Rectangle aRect( aXP[ nSrcPt ], aXP[ nSrcPt + 1 ] );
                                                    Point aCenter( aRect.Center() );
                                                    Point aStart( aXP[ nSrcPt + nXor ] );
                                                    Point aEnd( aXP[ nSrcPt + ( nXor ^ 1 ) ] );
                                                    aStart.X() = ( (double)( aStart.X() - aCenter.X() ) / fXScale ) + aCenter.X();
                                                    aStart.Y() = ( (double)( aStart.Y() - aCenter.Y() ) / fYScale ) + aCenter.Y();
                                                    aEnd.X() = ( (double)( aEnd.X() - aCenter.X() ) / fXScale ) + aCenter.X();
                                                    aEnd.Y() = ( (double)( aEnd.Y() - aCenter.Y() ) / fYScale ) + aCenter.Y();

                                                    Polygon aTempPoly( aRect, aStart, aEnd, ePolyStyle );
                                                    if ( nMod == 5 )
                                                    {
                                                        for ( sal_uInt16 j = aTempPoly.GetSize(); j--; )
                                                            aPoly[ nDstPt++ ] = aTempPoly[ j ];
                                                    }
                                                    else
                                                    {
                                                        for ( sal_uInt16 j = 0; j < aTempPoly.GetSize(); j++ )
                                                            aPoly[ nDstPt++ ] = aTempPoly[ j ];
                                                    }
                                                    nSrcPt += 4;
                                                }
                                            }
                                        }
                                        break;
                                        case 0 :
                                        case 1 :
                                        case 2 :
                                        case 6 :
                                        case 9 :
                                        case 0xa :
                                        case 0xb :
                                        case 0xc :
                                        case 0xd :
                                        case 0xe :
                                        case 0xf :

                                        case 7 :
                                        case 8 :
                                        {
                                            BOOL    bFirstDirection;
                                            sal_uInt16  nDstPt = aPoly.GetPointCount();
                                            for ( sal_uInt16 i = 0; i < ( nPolyFlags & 0xff ); i++ )
                                            {
                                                sal_uInt32 nModT = ( nMod == 7 ) ? 1 : 0;
                                                Point aCurrent( aXP[ nSrcPt ] );
                                                if ( nSrcPt )   // we need a previous point
                                                {
                                                    Point aPrev( aXP[ nSrcPt - 1 ] );
                                                    sal_Int32 nX, nY;
                                                    nX = aCurrent.X() - aPrev.X();
                                                    nY = aCurrent.Y() - aPrev.Y();
                                                    if ( ( nY ^ nX ) & 0x80000000 )
                                                    {
                                                        if ( !i )
                                                            bFirstDirection = TRUE;
                                                        else if ( !bFirstDirection )
                                                            nModT ^= 1;
                                                    }
                                                    else
                                                    {
                                                        if ( !i )
                                                            bFirstDirection = FALSE;
                                                        else if ( bFirstDirection )
                                                            nModT ^= 1;
                                                    }
                                                    if ( nModT )            // get the right corner
                                                    {
                                                        nX = aCurrent.X();
                                                        nY = aPrev.Y();
                                                    }
                                                    else
                                                    {
                                                        nX = aPrev.X();
                                                        nY = aCurrent.Y();
                                                    }
                                                    sal_Int32 nXVec = ( nX - aPrev.X() ) >> 1;
                                                    sal_Int32 nYVec = ( nY - aPrev.Y() ) >> 1;
                                                    Point aControl1( aPrev.X() + nXVec, aPrev.Y() + nYVec );
                                                    aPoly[ nDstPt ] = aControl1;
                                                    aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                    nXVec = ( nX - aCurrent.X() ) >> 1;
                                                    nYVec = ( nY - aCurrent.Y() ) >> 1;
                                                    Point aControl2( aCurrent.X() + nXVec, aCurrent.Y() + nYVec );
                                                    aPoly[ nDstPt ] = aControl2;
                                                    aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                                }
                                                aPoly[ nDstPt ] = aCurrent;
                                                nSrcPt++;
                                                nDstPt++;
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                            break;
                            case 0x0 :
                            {
                                sal_uInt16 nDstPt = aPoly.GetPointCount();
                                for ( sal_uInt16 i = 0; i < ( nPolyFlags & 0xfff ); i++ )
                                    aPoly[ nDstPt++ ] = aXP[ nSrcPt++ ];
                            }
                            break;
#ifdef DBG_AUTOSHAPE
                            default :
                            {
                                ByteString aString( "autoshapes::unknown PolyFlagValue :" );
                                aString.Append( ByteString::CreateFromInt32( nPolyFlags ) );
                                DBG_ERROR( aString.GetBuffer() );
                            }
                            break;
#endif
                        }
                    }
                    if ( pGrp )
                        pRet = pGrp;
                    else
                        pRet = pSdrPathObj;
                    if ( pRet )
                    {
                        pRet->NbcSetSnapRect( Rectangle( Point( aSnapRect.Left() + aUnion.Left(),
                                                                 aSnapRect.Top() + aUnion.Top() ),
                                                                    aUnion.GetSize() ) );
                    }
                }
            }
        }
    }
    if ( pRet )
    {
        if ( nFix16Angle )
        {
            double a = nFix16Angle * nPi180;
            pRet->NbcRotate( aSnapRect.Center(), nFix16Angle, sin( a ), cos( a ) );
        }
        if ( bSetAutoShapeAdjustItem )
        {
            if ( pRet->ISA( SdrObjGroup ) || pRet->ISA( SdrPathObj ) )
            {
                sal_uInt32  i, nCount = 0;
                SdrObject* pFirstObject = pRet;
                SdrObjList* pObjectList = NULL;
                if ( pRet->ISA( SdrPathObj ) )
                    nCount = 1;
                else
                {
                    pObjectList = ((SdrObjGroup*)pRet)->GetSubList();
                    if ( pObjectList )
                        nCount = pObjectList->GetObjCount();
                    if ( nCount )
                        pFirstObject = pObjectList->GetObj( 0 );
                }
                if ( nCount )
                {
                    SdrAutoShapeAdjustmentItem aAdjustItem;
                    SdrAutoShapeAdjustmentValue aAdjustValue;
                    if ( nAdjustmentHandles )
                    {
                        for ( i = 0; i < nAdjustmentHandles; i++ )
                        {
                            aAdjustValue.SetValue( pAdjustmentHandles[ i ].GetAdjustValue() );
                            aAdjustItem.SetValue( i, aAdjustValue );
                        }
                    }
                    if ( bVertAlloc == FALSE )
                    {
                        sal_uInt32 nMagicNumber = 0x80001234;           // this magic number identifies ms objects
                        sal_uInt32 nType = (sal_uInt32)eSpType << 16    // hiword is the shapetype
                                                        | nCount;       // loword the number of Path objects
                        sal_uInt32 nChecksum = 0;                       // checksum is used later to be sure that
                                                                        // the object is not changed

                        // here we are calculating the checksum
                        Rectangle aTRect( pRet->GetSnapRect() );
                        sal_Int32 nLeft = aTRect.Left();
                        sal_Int32 nTop = aTRect.Top();

                        SdrObject* pNext = pFirstObject;
                        for ( i = 0; i < nCount; i++ )
                        {
                            if ( i )
                                pNext = pObjectList->GetObj( i );

                            if ( !pNext->ISA( SdrPathObj ) )            // just only SdrPathObjs are valid as Autoshapes
                            {
                                nChecksum = 0;
                                break;
                            }
                            const XPolyPolygon& rPolyPolygon = ((SdrPathObj*)pNext)->GetPathPoly();
                            sal_uInt16 j, k = rPolyPolygon.Count();
                            sal_Int32 aVal[ 3 ];
                            while( k )
                            {
                                const XPolygon& rPoly = rPolyPolygon[ --k ];
                                j = rPoly.GetPointCount();
                                while( j )
                                {
                                    const Point& rPoint = rPoly[ --j ];
#ifdef __LITTLEENDIAN
                                    aVal[ 0 ] = SWAPLONG( rPoint.X() - nLeft );
                                    aVal[ 1 ] = SWAPLONG( rPoint.Y() - nTop );
                                    aVal[ 2 ] = SWAPLONG( rPoly.GetFlags( j ) );
#else
                                    aVal[ 0 ] = rPoint.X() - nLeft;
                                    aVal[ 1 ] = rPoint.Y() - nTop;
                                    aVal[ 2 ] = rPoly.GetFlags( j );
#endif
                                    nChecksum = rtl_crc32( nChecksum, &aVal[ 0 ], 12 );
                                }
                            }
                        }

                        if ( nChecksum )
                        {
                            i = aAdjustItem.GetCount();
                            aAdjustValue.SetValue( nChecksum );         // the last 3 entries in the adjustment item must be in this
                            aAdjustItem.SetValue( i++, aAdjustValue );  // order, otherwise it will be not possible to recreate the original
                            aAdjustValue.SetValue( nType );             // autoshape when exporting into ms formats
                            aAdjustItem.SetValue( i++, aAdjustValue );
                            aAdjustValue.SetValue( nMagicNumber );
                            aAdjustItem.SetValue( i++, aAdjustValue );
                        }
                    }
                    SfxItemSet aSet( pSdrModel->GetItemPool() );
                    aSet.Put( aAdjustItem );
                    pFirstObject->NbcSetAttributes( aSet, FALSE );
                }
            }
        }
    }
    return pRet;
}
