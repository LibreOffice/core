/*************************************************************************
 *
 *  $RCSfile: chart.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
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

#ifndef CGM_CHART_HXX_
#define CGM_CHART_HXX_

#include <tools/list.hxx>

/* FILE TYPE CONSTANTS: */
#define NOCHART         0       /* Undefined chart.         */
#define XYCHART         1       /* Standard XY chart.       */
#define PIECHART        21      /* Standard pie chart file. */
#define ORGCHART        26      /* Standard org chart file. */
#define TTLCHART        31      /* Title chart file.        */
#define BULCHART        32      /* Bullet chart file.       */
#define TABCHART        33      /* Table chart file.        */
#define DRWCHART        41      /* Chart with drawings only.*/
#define MLTCHART        42      /* Multiple chart file.     */
#define LASTCHART       45      /* The largest chart type.  */
#define SHWFILE         46      /* Slide show file.         */
#define SYMFILE         47      /* Symbol file.             */
/* the following were added although SPC doesn't have a #define */
/* for them...                                                  */
#define AUTOTTLCHT      95      /* Autobuild TTL CHT        */
#define AUTOBULCHT      96      /* Autobuild BUL CHT        */
#define AUTOTABCHT      97      /* Autobuild TAB CHT        */

/* FNC 10/11/93: for the chart stream, ALLCHART was added.  */
/* It is used specifically by PPT in its Template to let    */
/* us know that the template applies to all charts, not to  */
/* one specific chart type.                                 */
#define ALLCHART       127      /* Applies to all chart types */
#define ALLCHART_TPL   255      /* Applies to all chart types */

#define IOC_CHTTITLE        1       /* Title for any chart.                  */
#define IOC_CHTFOOTNOTE     2       /* ::com::sun::star::text::Footnote for any chart.               */
#define IOC_XYAXIS          3       /* Axis title for XY charts.             */
#define IOC_XYSERIESLEGEND  4       /* Series legend titles for XY charts.   */
#define IOC_PIETITLE        5       /* Title for pie charts.                 */
#define IOC_TABLEBODY       6       /* Table chart text element.             */
#define IOC_TITLEBODY       7       /* Title chart text element.             */
#define IOC_BULLETBODY      8       /* Bullet chart text element.            */
#define IOC_XYLEGENDTITLE   9       /* Legend title for XY charts.           */
#define IOC_PIELEGENDTITLE  10      /* Legend title for pie charts.          */
#define IOC_TABLEGENDTITLE  11      /* Legend title for table charts.        */

typedef struct TextAttribute
{
    sal_uInt16          nTextAttribCount;
    sal_Int8            nTextColorIndex;
    sal_Int8            nTextColorRed;
    sal_Int8            nTextColorGreen;
    sal_Int8            nTextColorBlue;
    sal_Int8            nShadowColorIndex;
    sal_Int8            nShadowColorRed;
    sal_Int8            nShadowColorGreen;
    sal_Int8            nShadowColorBlue;
    float           nTextAttribSize;
    sal_uInt16          nTextAttribBits;
    sal_Int8            nTextFontType;      // font identifiers
    sal_Int8            nTextCharPage;
    sal_uInt16          nTextFontFamily;
    sal_Int8            nTextFontMemberID;
    sal_Int8            nTextFontVendorID;
    TextAttribute*  pNextAttribute;     // zero or pointer to next TextAttribute
} TextAttribute;

typedef struct TextEntry
{
    sal_uInt16          nTypeOfText;
    sal_uInt16          nRowOrLineNum;
    sal_uInt16          nColumnNum;
    sal_uInt16          nZoneSize;          // textzone attributes
    sal_uInt16          nLineType;
    sal_uInt16          nAttributes;
    char*           pText;              // null terminated text
    TextAttribute*  pAttribute;
} TextEntry;

typedef struct ZoneOption
{
    char            nOverTitle;
    char            nOverBody;
    char            nOverFoot;
    char            nFStyle_Title;
    char            nFStyle_Body;
    char            nFStyle_Foot;
    char            nFOutc_Title;
    char            nFOutc_Body;
    char            nFOutc_Foot;
    char            nFFillc_Title;
    char            nFFillc_Body;
    char            nFFillc_Foot;
} ZoneOption;

typedef struct BulletOption
{
    char            nBType;
    char            nBSize;
    char            nBColor;
    sal_Int16           nBStart;
    double          nTMargin;
    double          nBSpace;
    char            nCPlace;
} BulletOption;

typedef struct BulDef
{
    char            btype;
    char            bsize;
    char            bcolor;
    char            bnumber;
} BulDef;

typedef struct BulletLines
{
    BulDef          nBulDef[ 48 ];
} BulletLines;

typedef struct IntSettings
{
    sal_uInt16          nCountry;
    sal_uInt16          nDateFormat;
    sal_uInt16          nDateSep;
    sal_uInt16          nTimeFormat;
    sal_uInt16          nTimeSep;
    sal_uInt16          nNumSeps;
    sal_uInt16          nCurrencyFormat;
    char            nCurrencySymbol[ 5 ];
} IntSettings;

typedef struct PageOrientDim
{
    char            nOrientation;
    char            nDimension;
    float           nPageX;
    float           nPageY;
} PageOrientDim;

typedef struct DataNode
{
    sal_Int16           nBoxX1;
    sal_Int16           nBoxY1;
    sal_Int16           nBoxX2;
    sal_Int16           nBoxY2;
    sal_Int8            nZoneEnum;
} DataNode;

typedef struct ChartZone
{
    sal_Int16           nMinX;
    sal_Int16           nMinY;
    sal_Int16           nMaxX;
    sal_Int16           nMaxY;
    char            nUserDef;
    char            nPad1;
} ChartZone;

class CGM;
class CGMImpressOutAct;
class CGMChart
{
    friend class CGM;
    friend class CGMImpressOutAct;

    protected:
        CGM*                    mpCGM;
        sal_Int8                    mnCurrentFileType;
        List                    maTextEntryList;
        DataNode                mDataNode[ 7 ];
        ChartZone               mChartZone;
        PageOrientDim           mPageOrientDim;
        BulletOption            mBulletOption;
        BulletLines             mBulletLines;
        ZoneOption              mZoneOption;
        IntSettings             mIntSettings;

    public:
                                CGMChart( CGM& rCGM );
                                ~CGMChart();

        void                    DeleteTextEntry( TextEntry* );
        void                    InsertTextEntry( TextEntry* );
        TextEntry*              GetTextEntry( sal_uInt32 nLine, sal_uInt32 nColumn );
        TextEntry*              GetFirstTextEntry();

        void                    ResetAnnotation();
        sal_Bool                    IsAnnotation();
};

#endif

