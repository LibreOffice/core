/*************************************************************************
 *
 *  $RCSfile: elements.hxx,v $
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
        FontList            aFontList;

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

