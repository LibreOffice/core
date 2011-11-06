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


#ifndef CGM_ELEMENTS_HXX_
#define CGM_ELEMENTS_HXX_

#include "main.hxx"
#include <tools/table.hxx>

#define nBackGroundColor    aColorTable[ 0 ]

class CGMElements
{
        void                ImplInsertHatch( sal_Int32 Key, int Style, long Distance, long Angle );
    public:
        CGM*                mpCGM;
        long                nMetaFileVersion;

        sal_uInt32              nIntegerPrecision;  // maybe 1, 2, 4 Bytes
        sal_uInt32              nIndexPrecision;    //   "      "       "
        RealPrecision       eRealPrecision;
        sal_uInt32              nRealSize;          // maybe 4 or 8 bytes
        sal_uInt32              nColorPrecision;    //   "      "       "
        sal_uInt32              nColorIndexPrecision;//  "      "       "

        ScalingMode         eScalingMode;
        double              nScalingFactor;

        VDCType             eVDCType;           // Integer / Real
        sal_uInt32              nVDCIntegerPrecision;
        RealPrecision       eVDCRealPrecision;
        sal_uInt32              nVDCRealSize;
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
        sal_uInt32              nColorMaximumIndex;             // default 63
        sal_uInt32              nLatestColorMaximumIndex;       // default 63
        sal_Int8                aColorTableEntryIs[ 256 ];
        sal_uInt32              aColorTable[ 256 ];
        sal_uInt32              aLatestColorTable[ 256 ];
        sal_uInt32              nColorValueExtent[ 8 ]; // RGB, CMYK

        // ASPECT SOURCE FLAGS
        sal_uInt32              nAspectSourceFlags; // bit = 0 -> INDIVIDUAL
                                                //       1 -> BUNDLED

        LineBundle*         pLineBundle;        // Pointer to the current LineBundleIndex
        LineBundle          aLineBundle;
        List                aLineList;
        SpecMode            eLineWidthSpecMode;
        LineCapType         eLineCapType;
        LineJoinType        eLineJoinType;

        MarkerBundle*       pMarkerBundle;      // Pointer to the current MarkerBundleIndex
        MarkerBundle        aMarkerBundle;
        List                aMarkerList;
        SpecMode            eMarkerSizeSpecMode;

        EdgeBundle*         pEdgeBundle;        // Pointer to the current EdgeBundleIndex
        EdgeBundle          aEdgeBundle;
        List                aEdgeList;
        EdgeVisibility      eEdgeVisibility;
        SpecMode            eEdgeWidthSpecMode;

        TextBundle*         pTextBundle;        // Pointer to the current TextBundleIndex
        TextBundle          aTextBundle;
        List                aTextList;
        double              nCharacterHeight;
        double              nCharacterOrientation[ 4 ];
        UnderlineMode       eUnderlineMode;
        sal_uInt32              nUnderlineColor;
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
        List                aFillList;
        FloatPoint          aFillRefPoint;
        Table               aHatchTable;

        Transparency        eTransparency;

        sal_uInt32              nAuxiliaryColor;

        // Delimiter Counts -> which will be increased by each 'begin' operation
        //                     and decreased by each 'end' operation
        sal_Bool                bSegmentCount;
                            CGMElements( CGM& rCGM );
                            ~CGMElements();
        CGMElements&        operator=( CGMElements& );
        void                Init();
        void                DeleteTable( Table& );
        Bundle*             GetBundleIndex( sal_uInt32 nIndex, List&, Bundle& );
        Bundle*             GetBundle( List& rList, long nIndex );
        Bundle*             InsertBundle( List&, Bundle& );
        void                DeleteAllBundles( List& );
        void                CopyAllBundles( List& Source, List& Dest );
};

#endif

