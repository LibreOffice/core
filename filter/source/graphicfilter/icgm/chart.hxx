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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_CHART_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_CHART_HXX

/* FILE TYPE CONSTANTS: */
#define BULCHART        32      /* Bullet chart file.       */
/* the following were added although SPC doesn't have a #define */
/* for them...                                                  */
#define AUTOTTLCHT      95      /* Autobuild TTL CHT        */
#define AUTOBULCHT      96      /* Autobuild BUL CHT        */
#define AUTOTABCHT      97      /* Autobuild TAB CHT        */

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

struct ZoneOption
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
    ZoneOption()
        : nOverTitle(0)
        , nOverBody(0)
        , nOverFoot(0)
        , nFStyle_Title(0)
        , nFStyle_Body(0)
        , nFStyle_Foot(0)
        , nFOutc_Title(0)
        , nFOutc_Body(0)
        , nFOutc_Foot(0)
        , nFFillc_Title(0)
        , nFFillc_Body(0)
        , nFFillc_Foot(0)
    {
    }
};

struct BulletOption
{
    char            nBType;
    char            nBSize;
    char            nBColor;
    sal_Int16       nBStart;
    double          nTMargin;
    double          nBSpace;
    char            nCPlace;
    BulletOption()
        : nBType(0)
        , nBSize(0)
        , nBColor(0)
        , nBStart(0)
        , nTMargin(0)
        , nBSpace(0)
        , nCPlace(0)
    {
    }
};

struct IntSettings
{
    sal_uInt16          nCountry;
    sal_uInt16          nDateFormat;
    sal_uInt16          nDateSep;
    sal_uInt16          nTimeFormat;
    sal_uInt16          nTimeSep;
    sal_uInt16          nNumSeps;
    sal_uInt16          nCurrencyFormat;
    char            nCurrencySymbol[ 5 ];
    IntSettings()
        : nCountry(0)
        , nDateFormat(0)
        , nDateSep(0)
        , nTimeFormat(0)
        , nTimeSep(0)
        , nNumSeps(0)
        , nCurrencyFormat(0)
    {
        memset (nCurrencySymbol, 0, sizeof(nCurrencySymbol));
    }
};

struct PageOrientDim
{
    char            nOrientation;
    char            nDimension;
    float           nPageX;
    float           nPageY;
    PageOrientDim()
        : nOrientation(0)
        , nDimension(0)
        , nPageX(0.0)
        , nPageY(0.0)
    {
    }
};

struct DataNode
{
    sal_Int16           nBoxX1;
    sal_Int16           nBoxY1;
    sal_Int16           nBoxX2;
    sal_Int16           nBoxY2;
    sal_Int8            nZoneEnum;
    DataNode()
        : nBoxX1(0)
        , nBoxY1(0)
        , nBoxX2(0)
        , nBoxY2(0)
        , nZoneEnum(0)
    {
    }
};

struct ChartZone
{
    sal_Int16           nMinX;
    sal_Int16           nMinY;
    sal_Int16           nMaxX;
    sal_Int16           nMaxY;
    char            nUserDef;
    char            nPad1;
    ChartZone()
        : nMinX(0)
        , nMinY(0)
        , nMaxX(0)
        , nMaxY(0)
        , nUserDef(0)
        , nPad1(0)
    {
    }
};

class CGM;
class CGMImpressOutAct;
class CGMChart
{
    friend class CGM;
    friend class CGMImpressOutAct;

    protected:
        sal_Int8                mnCurrentFileType;
        ::std::vector< TextEntry* > maTextEntryList;
        DataNode                mDataNode[ 7 ];
        ChartZone               mChartZone;
        PageOrientDim           mPageOrientDim;
        BulletOption            mBulletOption;
        ZoneOption              mZoneOption;
        IntSettings             mIntSettings;

    public:
                                CGMChart();
                                ~CGMChart();

        void                    DeleteTextEntry( TextEntry* );
        void                    InsertTextEntry( TextEntry* );

        void                    ResetAnnotation();
        bool                    IsAnnotation();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
