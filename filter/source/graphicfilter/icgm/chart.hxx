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

#ifndef CGM_CHART_HXX_
#define CGM_CHART_HXX_

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
        sal_Int8                mnCurrentFileType;
        ::std::vector< TextEntry* > maTextEntryList;
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

        void                    ResetAnnotation();
        sal_Bool                    IsAnnotation();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
