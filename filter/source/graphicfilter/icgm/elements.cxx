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


#include "elements.hxx"

// ---------------------------------------------------------------


CGMElements::CGMElements( CGM& rCGM ) :
    mpCGM ( &rCGM )
{
    Init();
};

// ---------------------------------------------------------------

CGMElements::~CGMElements()
{
    DeleteAllBundles( aLineList );
    DeleteAllBundles( aMarkerList );
    DeleteAllBundles( aEdgeList );
    DeleteAllBundles( aTextList );
    DeleteAllBundles( aFillList );
}

// ---------------------------------------------------------------

CGMElements& CGMElements::operator=( CGMElements& rSource )
{
    sal_uInt32 nIndex;

    nVDCIntegerPrecision = rSource.nVDCIntegerPrecision;
    nIntegerPrecision = rSource.nIntegerPrecision;
    eRealPrecision = rSource.eRealPrecision;
    nRealSize = rSource.nRealSize;
    nIndexPrecision = rSource.nIndexPrecision;
    nColorPrecision = rSource.nColorPrecision;
    nColorIndexPrecision = rSource.nColorIndexPrecision;

    nMetaFileVersion = rSource.nMetaFileVersion;
    eScalingMode = rSource.eScalingMode;
    nScalingFactor = rSource.nScalingFactor;
    eVDCType = rSource.eVDCType;
    eVDCRealPrecision = rSource.eVDCRealPrecision;
    nVDCRealSize = rSource.nVDCRealSize;
    aVDCExtent = rSource.aVDCExtent;
    aVDCExtentMaximum = rSource.aVDCExtentMaximum;
    eDeviceViewPortMode = rSource.eDeviceViewPortMode;
    nDeviceViewPortScale = rSource.nDeviceViewPortScale;
    eDeviceViewPortMap = rSource.eDeviceViewPortMap;
    eDeviceViewPortMapH = rSource.eDeviceViewPortMapH;
    eDeviceViewPortMapV = rSource.eDeviceViewPortMapV;
    aDeviceViewPort = rSource.aDeviceViewPort;
    nMitreLimit = rSource.nMitreLimit;
    eClipIndicator = rSource.eClipIndicator;
    aClipRect = rSource.aClipRect;
    eColorSelectionMode = rSource.eColorSelectionMode;
    eColorModel = rSource.eColorModel;
    nColorMaximumIndex = rSource.nColorMaximumIndex;
    nLatestColorMaximumIndex = rSource.nLatestColorMaximumIndex;

    for ( nIndex = 1; nIndex < 256; nIndex++ )      // do not overwrite the background color
    {
        aColorTableEntryIs[ nIndex ] = rSource.aColorTableEntryIs[ nIndex ];
        aColorTable[ nIndex ] = rSource.aColorTable[ nIndex ];
        aLatestColorTable[ nIndex ] = rSource.aColorTable[ nIndex ];
    }

    for ( nIndex = 0; nIndex < 8; nIndex++ )
    {
        nColorValueExtent[ nIndex ] = rSource.nColorValueExtent[ nIndex ];
    }
    nAspectSourceFlags = rSource.nAspectSourceFlags;

    CopyAllBundles( rSource.aLineList, aLineList );
    aLineBundle = rSource.aLineBundle;
    pLineBundle = (LineBundle*)GetBundle( aLineList, rSource.pLineBundle->GetIndex() );
    eLineWidthSpecMode = rSource.eLineWidthSpecMode;
    eLineCapType = rSource.eLineCapType;
    eLineJoinType = rSource.eLineJoinType;

    CopyAllBundles( rSource.aMarkerList, aMarkerList );
    aMarkerBundle = rSource.aMarkerBundle;
    pMarkerBundle = (MarkerBundle*)GetBundle( aMarkerList, rSource.pMarkerBundle->GetIndex() );
    eMarkerSizeSpecMode = rSource.eMarkerSizeSpecMode;

    CopyAllBundles( rSource.aEdgeList, aEdgeList );
    aEdgeBundle = rSource.aEdgeBundle;
    pEdgeBundle = (EdgeBundle*)GetBundle( aEdgeList, rSource.pEdgeBundle->GetIndex() );
    eEdgeVisibility = rSource.eEdgeVisibility;
    eEdgeWidthSpecMode = rSource.eEdgeWidthSpecMode;

    CopyAllBundles( rSource.aTextList, aTextList );
    aTextBundle = rSource.aTextBundle;
    pTextBundle = (TextBundle*)GetBundle( aTextList, rSource.pTextBundle->GetIndex() );
    nCharacterHeight = rSource.nCharacterHeight;
    nCharacterOrientation[ 0 ] = rSource.nCharacterOrientation[ 0 ];
    nCharacterOrientation[ 1 ] = rSource.nCharacterOrientation[ 1 ];
    nCharacterOrientation[ 2 ] = rSource.nCharacterOrientation[ 2 ];
    nCharacterOrientation[ 3 ] = rSource.nCharacterOrientation[ 3 ];
    eUnderlineMode = rSource.eUnderlineMode;
    nUnderlineColor = rSource.nUnderlineColor;
    eTextPath = rSource.eTextPath;
    eTextAlignmentH = rSource.eTextAlignmentH;
    eTextAlignmentV = rSource.eTextAlignmentV;
    nTextAlignmentHCont = rSource.nTextAlignmentHCont;
    nTextAlignmentVCont = rSource.nTextAlignmentVCont;
    nCharacterSetIndex = rSource.nCharacterSetIndex;
    nAlternateCharacterSetIndex = rSource.nAlternateCharacterSetIndex;
    aFontList = rSource.aFontList;
    eCharacterCodingA = rSource.eCharacterCodingA;

    CopyAllBundles( rSource.aFillList, aFillList );
    aFillBundle = rSource.aFillBundle;
    pFillBundle = (FillBundle*)GetBundle( aFillList, rSource.pFillBundle->GetIndex() );
    aFillRefPoint = rSource.aFillRefPoint;
    eTransparency = rSource.eTransparency;
    nAuxiliaryColor = rSource.nAuxiliaryColor;

    maHatchMap = rSource.maHatchMap;
    bSegmentCount = rSource.bSegmentCount;
    return (*this);
}

// ---------------------------------------------------------------

void CGMElements::Init()
{

    nIntegerPrecision = nIndexPrecision = 2;
    nRealSize = nVDCRealSize = 4;
    nColorIndexPrecision = 1;
    nColorPrecision = 1;
    nVDCIntegerPrecision = 2;
    eRealPrecision = eVDCRealPrecision = RP_FIXED;      //RP_FLOAT;

    nMetaFileVersion = 1;
    eScalingMode = SM_ABSTRACT;
    eVDCType = VDC_INTEGER;
    aVDCExtent.Left = aVDCExtent.Bottom = 0;
//  aVDCExtent.Right = aVDCExtent.Top = 32767;
    aVDCExtent.Right = aVDCExtent.Top = 1.0;
    aVDCExtentMaximum.Left = aVDCExtentMaximum.Bottom = 0;
//  aVDCExtentMaximum.Right = aVDCExtentMaximum.Top = 32767;
    aVDCExtentMaximum.Right = aVDCExtentMaximum.Top = 1.0;

    eDeviceViewPortMode = DVPM_FRACTION;
    nDeviceViewPortScale = 1;
    eDeviceViewPortMap = DVPM_FORCED;
    eDeviceViewPortMapH = DVPMH_LEFT;
    eDeviceViewPortMapV = DVPMV_BOTTOM;
    aDeviceViewPort.Left = 0;
    aDeviceViewPort.Top = 1;
    aDeviceViewPort.Right = 0;
    aDeviceViewPort.Bottom = 1;

    nMitreLimit = 32767;
    eClipIndicator = CI_ON;
    aClipRect = aVDCExtent;

    eColorSelectionMode = CSM_INDEXED;
    eColorModel = CM_RGB;
    nColorMaximumIndex = 63;
    int i;
    for ( i = 0; i < 256; aColorTableEntryIs[ i++ ] = 0 ) ;
    aColorTable[ 0 ] = 0;
    for ( i = 1; i < 256; aColorTable[ i++ ] = 0xffffff ) ;
    nLatestColorMaximumIndex = 63;
    aLatestColorTable[ 0 ] = 0;
    for ( i = 1; i < 256; aLatestColorTable[ i++ ] = 0xffffff ) ;
    nColorValueExtent[ 0 ] = nColorValueExtent[ 1 ] = nColorValueExtent[ 2 ] = 0;
    nColorValueExtent[ 3 ] = nColorValueExtent[ 4 ] = nColorValueExtent[ 5 ] = 255;

    nAspectSourceFlags = 0;     // all flags are individual

    eLineWidthSpecMode = SM_SCALED;         // line parameter
    eLineCapType = LCT_NONE;
    eLineJoinType = LJT_NONE;
    pLineBundle = &aLineBundle;                 // line bundle parameter
    aLineBundle.SetIndex( 1 );
    aLineBundle.eLineType = LT_SOLID;
    aLineBundle.nLineWidth = 1;
    aLineBundle.SetColor( 0xffffff );
    InsertBundle( aLineList, aLineBundle );

    eMarkerSizeSpecMode = SM_SCALED;            // marker parameter
    pMarkerBundle = &aMarkerBundle;             // marker bundle parameter
    aMarkerBundle.SetIndex( 1 );
    aMarkerBundle.eMarkerType = MT_STAR;
    aMarkerBundle.nMarkerSize = 1;
    aMarkerBundle.SetColor( 0xffffff );
    InsertBundle( aMarkerList, aMarkerBundle );

    eEdgeVisibility = EV_OFF;                   // edge parameter
    eEdgeWidthSpecMode = SM_SCALED;
    pEdgeBundle = &aEdgeBundle;                 // edge bundle parameter
    aEdgeBundle.SetIndex( 1 );
    aEdgeBundle.eEdgeType = ET_SOLID;
    aEdgeBundle.nEdgeWidth = 1;
    aEdgeBundle.SetColor( 0xffffff );
    InsertBundle( aEdgeList, aEdgeBundle );

    nCharacterHeight = 327;                     // text parameter
    nCharacterOrientation[0] = 0;
    nCharacterOrientation[1] = 1;
    nCharacterOrientation[2] = 1;
    nCharacterOrientation[3] = 0;
    eUnderlineMode = UM_OFF;
    nUnderlineColor = 0xffffff;
    eTextPath = TPR_RIGHT;
    eTextAlignmentH = TAH_NORMAL;
    eTextAlignmentV = TAV_NORMAL;
    nCharacterSetIndex = nAlternateCharacterSetIndex = 1;
    eCharacterCodingA = CCA_BASIC_7;
    pTextBundle = &aTextBundle;                 // text bundle parameter
    aTextBundle.SetIndex( 1 );
    aTextBundle.nTextFontIndex = 1;
    aTextBundle.eTextPrecision = TPR_STRING;
    aTextBundle.nCharacterExpansion = 1;
    aTextBundle.nCharacterSpacing = 0;
    aTextBundle.SetColor( 0xffffff );
    InsertBundle( aTextList, aTextBundle );

    pFillBundle = &aFillBundle;                 // fill bundle parameter
    aFillBundle.SetIndex( 1 );
    aFillBundle.eFillInteriorStyle = FIS_HOLLOW;
    aFillBundle.nFillHatchIndex = 1;
    aFillBundle.nFillPatternIndex = 1;
    aFillBundle.SetColor( 0xffffff );
    InsertBundle( aFillList, aFillBundle );

    ImplInsertHatch( 0, 0, 0, 0 );
    ImplInsertHatch( 1, 0, 125, 0 );
    ImplInsertHatch( 2, 0, 125, 900 );
    ImplInsertHatch( 3, 0, 125, 450 );
    ImplInsertHatch( 4, 0, 125, 1350 );
    ImplInsertHatch( 5, 1, 125, 0 );
    ImplInsertHatch( 6, 1, 125, 450 );
    ImplInsertHatch( -1, 0, 75, 0 );
    ImplInsertHatch( -2, 0, 75, 900 );
    ImplInsertHatch( -3, 0, 75, 450 );
    ImplInsertHatch( -4, 0, 75, 1350 );
    ImplInsertHatch( -5, 1, 75, 0 );
    ImplInsertHatch( -6, 1, 75, 450 );
    ImplInsertHatch( -7, 2, 125, 0 );
    ImplInsertHatch( -8, 2, 125, 900 );
    ImplInsertHatch( -9, 2, 125, 450 );
    ImplInsertHatch( -10, 2, 125, 1350 );
    ImplInsertHatch( -11, 0, 40, 0 );
    ImplInsertHatch( -12, 0, 40, 900 );
    ImplInsertHatch( -13, 0, 40, 450 );
    ImplInsertHatch( -14, 0, 40, 1350 );
    ImplInsertHatch( -15, 1, 40, 0 );
    ImplInsertHatch( -16, 1, 40, 900 );
    ImplInsertHatch( -21, 0, 250, 0 );
    ImplInsertHatch( -22, 0, 250, 900 );
    ImplInsertHatch( -23, 0, 250, 450 );
    ImplInsertHatch( -24, 0, 250, 1350 );
    ImplInsertHatch( -25, 1, 250, 0 );
    ImplInsertHatch( -26, 1, 250, 450 );

    eTransparency = T_ON;

    nBackGroundColor = nAuxiliaryColor = 0;

    bSegmentCount = sal_False;
}

// ---------------------------------------------------------------

void CGMElements::ImplInsertHatch( sal_Int32 nKey, int nStyle, long nDistance, long nAngle )
{
    HatchEntry& rEntry = maHatchMap[nKey];
    rEntry.HatchStyle = nStyle;
    rEntry.HatchDistance = nDistance;
    rEntry.HatchAngle = nAngle;
}

// ---------------------------------------------------------------

void CGMElements::DeleteAllBundles( BundleList& rList )
{
    for ( size_t i = 0, n = rList.size(); i < n; ++i ) {
        delete rList[ i ];
    }
    rList.clear();
};


// ---------------------------------------------------------------

void CGMElements::CopyAllBundles( BundleList& rSource, BundleList& rDest )
{
    DeleteAllBundles( rDest );

    for ( size_t i = 0, n = rSource.size(); i < n; ++i )
    {
        Bundle* pPtr = rSource[ i ];
        Bundle* pTempBundle = pPtr->Clone();
        rDest.push_back( pTempBundle );
    }
};

// ---------------------------------------------------------------

Bundle* CGMElements::GetBundleIndex( long nIndex, BundleList& rList, Bundle& rBundle )
{
    rBundle.SetIndex( nIndex );
    Bundle* pBundle = GetBundle( rList, nIndex );
    if ( !pBundle )
        pBundle = InsertBundle( rList, rBundle );
    return pBundle;
}

// ---------------------------------------------------------------

Bundle* CGMElements::GetBundle( BundleList& rList, long nIndex )
{
    for ( size_t i = 0, n = rList.size(); i < n; ++i ) {
        if ( rList[ i ]->GetIndex() == nIndex ) {
            return rList[ i ];
        }
    }
    return NULL;
}

// ---------------------------------------------------------------

Bundle* CGMElements::InsertBundle( BundleList& rList, Bundle& rBundle )
{
    Bundle* pBundle = GetBundle( rList, rBundle.GetIndex() );
    if ( pBundle )
    {
        for ( BundleList::iterator it = rList.begin(); it != rList.end(); ++it ) {
            if ( *it == pBundle ) {
                rList.erase( it );
                delete pBundle;
                break;
            }
        }
    }
    pBundle = rBundle.Clone();
    rList.push_back( pBundle );
    return pBundle;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
