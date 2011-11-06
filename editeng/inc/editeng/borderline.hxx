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

