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
#ifndef CGM_ELEMENTS_HXX_
#define CGM_ELEMENTS_HXX_

#include "main.hxx"
#include "cgmtypes.hxx"
#include <vector>
#include <map>

#define nBackGroundColor    aColorTable[ 0 ]

typedef ::std::vector< Bundle* > BundleList;
typedef ::std::map<sal_uInt32, HatchEntry> HatchMap;

class CGMElements
{
        void                ImplInsertHatch( sal_Int32 Key, int Style, long Distance, long Angle );
    public:
        CGM*                mpCGM;
        long                nMetaFileVersion;

        sal_uInt32          nIntegerPrecision;  // maybe 1, 2, 4 Bytes
        sal_uInt32          nIndexPrecision;    //   "      "       "
        RealPrecision       eRealPrecision;
        sal_uInt32          nRealSize;          // maybe 4 or 8 bytes
        sal_uInt32          nColorPrecision;    //   "      "       "
        sal_uInt32          nColorIndexPrecision;//  "      "       "

        ScalingMode         eScalingMode;
        double              nScalingFactor;

        VDCType             eVDCType;           // Integer / Real
        sal_uInt32          nVDCIntegerPrecision;
        RealPrecision       eVDCRealPrecision;
        sal_uInt32          nVDCRealSize;
        FloatRect           aVDCExtent;
        FloatRect           aVDCExtentMaximum;

        DeviceViewPortMode  eDeviceViewPortMode;
        double              nDeviceViewPortScale;
        DeviceViewPortMap   eDeviceViewPortMap;
        DeviceViewPortMapH  eDeviceViewPortMapH;
        DeviceViewPortMapV  eDeviceViewPortMapV;
        FloatRect           aDeviceViewPort;

        double              nMitreLimit;

        ClipIndicator       eClipIndicator;
        FloatRect           aClipRect;

        ColorSelectionMode  eColorSelectionMode;
        ColorModel          eColorModel;
        sal_uInt32          nColorMaximumIndex;             // default 63
        sal_uInt32          nLatestColorMaximumIndex;       // default 63
        sal_Int8            aColorTableEntryIs[ 256 ];
        sal_uInt32          aColorTable[ 256 ];
        sal_uInt32          aLatestColorTable[ 256 ];
        sal_uInt32          nColorValueExtent[ 8 ]; // RGB, CMYK

        // ASPECT SOURCE FLAGS
        sal_uInt32          nAspectSourceFlags; // bit = 0 -> INDIVIDUAL
                                                //       1 -> BUNDLED

        LineBundle*         pLineBundle;        // Pointer to the current LineBundleIndex
        LineBundle          aLineBundle;
        BundleList          aLineList;
        SpecMode            eLineWidthSpecMode;
        LineCapType         eLineCapType;
        LineJoinType        eLineJoinType;

        MarkerBundle*       pMarkerBundle;      // Pointer to the current MarkerBundleIndex
        MarkerBundle        aMarkerBundle;
        BundleList          aMarkerList;
        SpecMode            eMarkerSizeSpecMode;

        EdgeBundle*         pEdgeBundle;        // Pointer to the current EdgeBundleIndex
        EdgeBundle          aEdgeBundle;
        BundleList          aEdgeList;
        EdgeVisibility      eEdgeVisibility;
        SpecMode            eEdgeWidthSpecMode;

        TextBundle*         pTextBundle;        // Pointer to the current TextBundleIndex
        TextBundle          aTextBundle;
        BundleList          aTextList;
        double              nCharacterHeight;
        double              nCharacterOrientation[ 4 ];
        UnderlineMode       eUnderlineMode;
        sal_uInt32          nUnderlineColor;
        TextPath            eTextPath;
        TextAlignmentH      eTextAlignmentH;
        TextAlignmentV      eTextAlignmentV;
        double              nTextAlignmentHCont;
        double              nTextAlignmentVCont;
        long                nCharacterSetIndex;
        long                nAlternateCharacterSetIndex;
        CharacterCodingA    eCharacterCodingA;
        CGMFList            aFontList;

        FillBundle*         pFillBundle;        // Pointer to the current EdgeBundleIndex
        FillBundle          aFillBundle;
        BundleList          aFillList;
        FloatPoint          aFillRefPoint;
        HatchMap            maHatchMap;

        Transparency        eTransparency;

        sal_uInt32          nAuxiliaryColor;

        // Delimiter Counts -> which will be increased by each 'begin' operation
        //                     and decreased by each 'end' operation
        sal_Bool            bSegmentCount;
                            CGMElements( CGM& rCGM );
                            ~CGMElements();
        CGMElements&        operator=( CGMElements& );
        void                Init();
        Bundle*             GetBundleIndex( long nIndex, BundleList&, Bundle& );
        Bundle*             GetBundle( BundleList& rList, long nIndex );
        Bundle*             InsertBundle( BundleList&, Bundle& );
        void                DeleteAllBundles( BundleList& );
        void                CopyAllBundles( BundleList& Source, BundleList& Dest );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
