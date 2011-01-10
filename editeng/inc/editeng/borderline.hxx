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

#ifndef SVX_BORDERLINE_HXX
#define SVX_BORDERLINE_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

// Line defaults in twips (former Writer defaults):

#define DEF_LINE_WIDTH_0        1
#define DEF_LINE_WIDTH_1        20
#define DEF_LINE_WIDTH_2        50
#define DEF_LINE_WIDTH_3        80
#define DEF_LINE_WIDTH_4        100
#define DEF_LINE_WIDTH_5        10

#define DEF_MAX_LINE_WIDHT      DEF_LINE_WIDTH_4
#define DEF_MAX_LINE_DIST       DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE0_OUT    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE0_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE0_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE1_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE1_IN     DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE1_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE2_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE2_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE2_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE3_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE3_IN     DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE3_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE4_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE4_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE4_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE5_OUT    DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE5_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE5_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE6_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE6_IN     DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE6_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE7_OUT    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE7_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE7_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE8_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE8_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE8_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE9_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE9_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE9_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE10_OUT   DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE10_IN    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE10_DIST  DEF_LINE_WIDTH_2

// ============================================================================

class EDITENG_DLLPUBLIC SvxBorderLine
{
protected:
    Color  aColor;
    sal_uInt16 nOutWidth;
    sal_uInt16 nInWidth;
    sal_uInt16 nDistance;

public:
    SvxBorderLine( const Color *pCol = 0, sal_uInt16 nOut = 0, sal_uInt16 nIn = 0, sal_uInt16 nDist = 0 );
    SvxBorderLine( const SvxBorderLine& r );

    SvxBorderLine& operator=( const SvxBorderLine& r );

    const Color&    GetColor() const { return aColor; }
    sal_uInt16          GetOutWidth() const { return nOutWidth; }
    sal_uInt16          GetInWidth() const { return nInWidth; }
    sal_uInt16          GetDistance() const { return nDistance; }

    void            SetColor( const Color &rColor ) { aColor = rColor; }
    void            SetOutWidth( sal_uInt16 nNew ) { nOutWidth = nNew; }
    void            SetInWidth( sal_uInt16 nNew ) { nInWidth = nNew;  }
    void            SetDistance( sal_uInt16 nNew ) { nDistance = nNew; }
    void            ScaleMetrics( long nMult, long nDiv );

    sal_Bool            operator==( const SvxBorderLine &rCmp ) const;

    String          GetValueString( SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit,
                                    const IntlWrapper* pIntl,
                                    sal_Bool bMetricStr = sal_False ) const;

    bool            HasPriority( const SvxBorderLine& rOtherLine ) const;

    bool isEmpty() const { return (0 == nOutWidth && 0 == nInWidth && 0 == nDistance); }
    bool isDouble() const { return (0 != nOutWidth && 0 != nInWidth); }
    sal_uInt16 getWidth() const { return nOutWidth + nInWidth + nDistance; }
};

// ============================================================================

#endif

