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

//#define VCL_NEED_BASETSD

#include <main.hxx>
#include <outact.hxx>


// ---------------------------------------------------------------

void CGM::ImplDoClass5()
{
    switch ( mnElementID )
    {
        case 0x01 : /*Line Bundle Index*/
            pElement->pLineBundle = (LineBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aLineList, pElement->aLineBundle );
        break;
        case 0x02 : /*Line Type*/
        {
            if ( pElement->nAspectSourceFlags & ASF_LINETYPE )
                pElement->pLineBundle->eLineType = (LineType)ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aLineBundle.eLineType = (LineType)ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x03 : /*Line Width*/
        {
            double nWidth;
            if ( pElement->eLineWidthSpecMode == SM_ABSOLUTE )
            {
                if ( pElement->eVDCType == VDC_REAL )
                    nWidth = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                else
                    nWidth = (double)ImplGetI( pElement->nVDCIntegerPrecision );

                ImplMapDouble( nWidth );
            }
            else
                nWidth = (sal_uInt32)ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize ) * 25; // scaling in 1/4 mm

            ( pElement->nAspectSourceFlags & ASF_LINEWIDTH )
                ? pElement->aLineBundle.nLineWidth = nWidth
                    : pElement->aLineBundle.nLineWidth = nWidth;
        }
        break;
        case 0x04 : /*Line Color*/
        {
            if ( pElement->nAspectSourceFlags & ASF_LINECOLOR )
                pElement->pLineBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aLineBundle.SetColor( ImplGetBitmapColor() );
        }
        break;
        case 0x05 : /*Marker Bundle Index*/
            pElement->pMarkerBundle = (MarkerBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aMarkerList, pElement->aMarkerBundle );
        break;
        case 0x06 : /*Marker Type*/
        {
            if ( pElement->nAspectSourceFlags & ASF_MARKERTYPE )
                pElement->pMarkerBundle->eMarkerType = (MarkerType)ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aMarkerBundle.eMarkerType = (MarkerType)ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x07 : /*Marker Size*/
        {
            double nWidth;
            if ( pElement->eMarkerSizeSpecMode == SM_ABSOLUTE )
            {
                if ( pElement->eVDCType == VDC_REAL )
                    nWidth = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                else
                    nWidth = (double)ImplGetI( pElement->nVDCIntegerPrecision );
                ImplMapDouble( nWidth );
            }
            else
                nWidth = (sal_uInt32)ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize ) * 25;
            ( pElement->nAspectSourceFlags & ASF_MARKERSIZE )
                ? pElement->aMarkerBundle.nMarkerSize = nWidth
                    : pElement->aMarkerBundle.nMarkerSize = nWidth;
        }
        break;
        case 0x08 : /*Marker Color*/
        {
            if ( pElement->nAspectSourceFlags & ASF_MARKERCOLOR )
                pElement->pMarkerBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aMarkerBundle.SetColor( ImplGetBitmapColor() );
        }
        break;
        case 0x09 : /*Text Bundle Index*/
            pElement->pTextBundle = (TextBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aTextList, pElement->aTextBundle );
        break;
        case 0x0a : /*Text Font Index*/
        {
            if ( pElement->nAspectSourceFlags & ASF_TEXTFONTINDEX )
                pElement->pTextBundle->nTextFontIndex = ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aTextBundle.nTextFontIndex = ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x0b : /*Text Precision*/
        {
            TextBundle* pBundle;
            if ( pElement->nAspectSourceFlags & ASF_TEXTPRECISION )
                pBundle = pElement->pTextBundle;
            else
                pBundle = &pElement->aTextBundle;
            switch( ImplGetUI16() )
            {
                case 0 : pBundle->eTextPrecision = TPR_STRING; break;
                case 1 : pBundle->eTextPrecision = TPR_CHARACTER; break;
                case 2 : pBundle->eTextPrecision = TPR_STROKE; break;
                default : pBundle->eTextPrecision = TPR_UNDEFINED; break;
            }
        }
        break;
        case 0x0c : /*Character Expansion Factor*/
        {
            if ( pElement->nAspectSourceFlags & ASF_CHARACTEREXPANSION )
                pElement->pTextBundle->nCharacterExpansion = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            else
                pElement->aTextBundle.nCharacterExpansion = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
        }
        break;
        case 0x0d : /*Character Spacing*/
        {
            if ( pElement->nAspectSourceFlags & ASF_CHARACTERSPACING )
                pElement->pTextBundle->nCharacterSpacing = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            else
                pElement->aTextBundle.nCharacterSpacing = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
        }
        break;
        case 0x0e : /*Text Color*/
        {
            if ( pElement->nAspectSourceFlags & ASF_TEXTCOLOR )
                pElement->pTextBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aTextBundle.SetColor( ImplGetBitmapColor() );
        }
        break;
        case 0x0f : /*Character Height*/
        {
                if ( pElement->eVDCType == VDC_INTEGER )
                    pElement->nCharacterHeight = ImplGetI( pElement->nVDCIntegerPrecision );
                else // ->floating points
                    pElement->nCharacterHeight = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                ImplMapDouble( pElement->nCharacterHeight );
                pElement->nCharacterHeight /= 18.0;
        }
        break;
        case 0x10 : /*Character Orientation*/
        {
            if ( pElement->eVDCType == VDC_INTEGER )
            {
                pElement->nCharacterOrientation[0] = ImplGetI( pElement->nVDCIntegerPrecision );
                pElement->nCharacterOrientation[1] = ImplGetI( pElement->nVDCIntegerPrecision );
                pElement->nCharacterOrientation[2] = ImplGetI( pElement->nVDCIntegerPrecision );
                pElement->nCharacterOrientation[3] = ImplGetI( pElement->nVDCIntegerPrecision );
            }
            else // ->floating points
            {
                pElement->nCharacterOrientation[0] = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                pElement->nCharacterOrientation[1] = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                pElement->nCharacterOrientation[2] = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                pElement->nCharacterOrientation[3] = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
            }
        }
        break;
        case 0x11 : /*Text Path*/
        {
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eTextPath = TPR_RIGHT; break;
                case 1 : pElement->eTextPath = TPR_LEFT; break;
                case 2 : pElement->eTextPath = TPR_UP; break;
                case 3 : pElement->eTextPath = TPR_DOWN; break;
                default : mbStatus = sal_False; break;
            }
        }
        break;
        case 0x12 : /*Text Alignment*/
        {
            pElement->eTextAlignmentH = (TextAlignmentH)ImplGetUI16();
            pElement->eTextAlignmentV = (TextAlignmentV)ImplGetUI16( 8 );
            pElement->nTextAlignmentHCont = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            pElement->nTextAlignmentVCont = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
        }
        break;
        case 0x13 : /*Character Set Index*/
            pElement->nCharacterSetIndex = ImplGetI( pElement->nIndexPrecision );
        break;
        case 0x14 : /*Alternate Character Set Index*/
            pElement->nAlternateCharacterSetIndex = ImplGetI( pElement->nIndexPrecision );
        break;
        case 0x15 : /*Fill Bundle Index*/
            pElement->pFillBundle = (FillBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aFillList, pElement->aFillBundle );
        break;
        case 0x16 : /*Fill Interior Style*/
        {
            if ( pElement->nAspectSourceFlags & ASF_FILLINTERIORSTYLE )
                pElement->pFillBundle->eFillInteriorStyle = (FillInteriorStyle)ImplGetUI16();
            else
                pElement->aFillBundle.eFillInteriorStyle = (FillInteriorStyle)ImplGetUI16();
        }
        break;
        case 0x17 : /*Fill Color*/
        {
            if ( pElement->nAspectSourceFlags & ASF_FILLCOLOR )
                pElement->pFillBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aFillBundle.SetColor( ImplGetBitmapColor() );
        }
        break;
        case 0x18 : /*Fill Hatch Index*/
        {
            if ( pElement->nAspectSourceFlags & ASF_HATCHINDEX )
                pElement->pFillBundle->nFillHatchIndex = ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aFillBundle.nFillHatchIndex = ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x19 : /*Fill Pattern Index*/
        {
            if ( pElement->nAspectSourceFlags & ASF_PATTERNINDEX )
                pElement->pFillBundle->nFillPatternIndex = ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aFillBundle.nFillPatternIndex = ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x1a : /*Edge Bundle Index*/
            pElement->pEdgeBundle = (EdgeBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aEdgeList, pElement->aEdgeBundle );
        break;
        case 0x1b : /*Edge Type*/
        {
            if ( pElement->nAspectSourceFlags & ASF_EDGETYPE )
                pElement->pEdgeBundle->eEdgeType = (EdgeType)ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aEdgeBundle.eEdgeType = (EdgeType)ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x1c : /*Edge Width*/
        {
            double nWidth;
            if ( pElement->eEdgeWidthSpecMode == SM_ABSOLUTE )
            {
                if ( pElement->eVDCType == VDC_REAL )
                    nWidth = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                else
                    nWidth = (double)ImplGetI( pElement->nVDCIntegerPrecision );

                ImplMapDouble( nWidth );
            }
            else
                nWidth = (sal_uInt32)ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize ) * 25;
            ( pElement->nAspectSourceFlags & ASF_EDGEWIDTH )
                ? pElement->aEdgeBundle.nEdgeWidth = nWidth
                    : pElement->aEdgeBundle.nEdgeWidth = nWidth;
        }
        break;
        case 0x1d : /*Edge Color*/
        {
            if ( pElement->nAspectSourceFlags & ASF_EDGECOLOR )
                pElement->pEdgeBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aEdgeBundle.SetColor( ImplGetBitmapColor() );
        }
        break;
        case 0x1e : /*Edge Visibility*/
        {
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eEdgeVisibility = EV_OFF; break;
                case 1 : pElement->eEdgeVisibility = EV_ON; break;
                default : mbStatus = sal_False;
            }
        }
        break;
        case 0x1f : /*Fill Reference Point*/
            ImplGetPoint( pElement->aFillRefPoint );
        break;
        case 0x20 : /*Pattern Table" )*/ break;
        case 0x21 : /*Pattern Size" )*/ break;
        case 0x22 : /*Color Table*/
        {
            sal_uInt32 nColorStartIndex = ImplGetUI( pElement->nColorIndexPrecision );
            if ( ( nColorStartIndex > 255 ) ||
                ( ( ( mnElementSize - pElement->nColorIndexPrecision ) % ( pElement->nColorPrecision * 3 ) ) != 0 ) )
            {
                mbStatus = sal_False;
            }
            else
            {
                sal_uInt32 nColors = ( mnElementSize - pElement->nColorIndexPrecision ) / ( 3 * pElement->nColorPrecision );
                if ( nColors )
                {
                    sal_uInt32 nMaxColorIndex = nColorStartIndex + nColors - 1;
                    sal_uInt32 nIndex;
                    if ( nMaxColorIndex > 255 )
                    {
                        mbStatus = sal_False;
                    }
                    else
                    {
                        if ( pElement->nLatestColorMaximumIndex < nMaxColorIndex )
                            pElement->nLatestColorMaximumIndex = nMaxColorIndex;

                        for (  nIndex = nColorStartIndex; nIndex <= nMaxColorIndex; nIndex++ )
                        {
                            pElement->aLatestColorTable[ nIndex ] = ImplGetBitmapColor( sal_True );
                        }
                    }
                    pElement->nColorMaximumIndex = pElement->nLatestColorMaximumIndex;
                    for ( nIndex = nColorStartIndex; nIndex <= nMaxColorIndex; nIndex++ )
                    {
                        if ( !pElement->aColorTableEntryIs[ nIndex ] )
                        {
                            pElement->aColorTableEntryIs[ nIndex ] = 1;
                            pElement->aColorTable[ nIndex ] = pElement->aLatestColorTable[ nIndex ];
                        }
                    }
                }
            }
        }
        break;
        case 0x23 : /*Aspect Source Flags*/
        {
            int nFlags = mnElementSize >> 2;
            while ( nFlags-- > 0 )
            {
                sal_uInt32 nFlag = 0;
                switch( ImplGetUI16() )
                {
                    case 0 : nFlag = ASF_LINETYPE; break;
                    case 1 : nFlag = ASF_LINEWIDTH; break;
                    case 2 : nFlag = ASF_LINECOLOR; break;
                    case 3 : nFlag = ASF_MARKERTYPE; break;
                    case 4 : nFlag = ASF_MARKERSIZE; break;
                    case 5 : nFlag = ASF_MARKERCOLOR; break;
                    case 6 : nFlag = ASF_FILLINTERIORSTYLE; break;
                    case 7 : nFlag = ASF_HATCHINDEX; break;
                    case 8 : nFlag = ASF_PATTERNINDEX; break;
                    case 9 : nFlag = ASF_BITMAPINDEX; break;
                    case 10 : nFlag = ASF_FILLCOLOR; break;
                    case 11 : nFlag = ASF_EDGETYPE; break;
                    case 12 : nFlag = ASF_EDGEWIDTH; break;
                    case 13 : nFlag = ASF_EDGECOLOR; break;
                    case 14 : nFlag = ASF_TEXTFONTINDEX; break;
                    case 15 : nFlag = ASF_TEXTPRECISION; break;
                    case 16 : nFlag = ASF_CHARACTEREXPANSION; break;
                    case 17 : nFlag = ASF_CHARACTERSPACING; break;
                    case 18 : nFlag = ASF_TEXTCOLOR; break;
                    default : mbStatus = sal_False; break;
                }
                sal_uInt32  nASF = ImplGetUI16();
                switch ( nASF )
                {
                    case 0 : pElement->nAspectSourceFlags &= ~nFlag; break; // INDIVIDUAL
                    case 1 : pElement->nAspectSourceFlags |= nFlag; break;  // BUNDLED
                    default : mbStatus = sal_False; break;
                }
            }
        }
        break;
        case 0x24 : /*Pick Identifier" ) bre*/
        case 0x25 : /*Line Cap*/
        {
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eLineCapType = LCT_BUTT; break;
                case 1 : pElement->eLineCapType = LCT_ROUND; break;
                case 2 : pElement->eLineCapType = LCT_SQUARE; break;
                case 3 : pElement->eLineCapType = LCT_TRIANGLE; break;
                case 4 : pElement->eLineCapType = LCT_ARROW; break;
                default : pElement->eLineCapType = LCT_NONE; break;
            }
        }
        break;
        case 0x26 : /*Line Join*/
        {
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eLineJoinType = LJT_MITER; break;
                case 1 : pElement->eLineJoinType = LJT_ROUND; break;
                case 2 : pElement->eLineJoinType = LJT_BEVEL; break;
                default : pElement->eLineJoinType = LJT_NONE; break;
            }
        }
        break;
        case 0x27 : /*Line Type Continuation*/ break;           // NS
        case 0x28 : /*Line Type Initial Offset*/ break;         // NS
        case 0x29 : /*Text Score Type*/ break;
        case 0x2a : /*Restricted Text Type*/ break;
        case 0x2b : /*Interpolated interior*/ break;
        case 0x2c : /*Edge Cap*/ break;                         // NS
        case 0x2d : /*Edge Join*/ break;
        case 0x2e : /*Edge Type Continuation*/ break;           // NS
        case 0x2f : /*Edge Type Initial Offset*/ break;         // NS
        case 0x30 : /*Symbol Library Index*/ break;             // NS
        case 0x31 : /*Symbol Color*/ break;                     // NS
        case 0x32 : /*Symbol Size*/ break;                      // NS
        case 0x33 : /*Symbol Orientation*/ break;               // NS
        case 0x50 : /*Block Text Region Margins*/ break;
        case 0x51 : /*Block Text Region Expansion*/ break;
        case 0x52 : /*Block Text Region Anchor*/ break;
        case 0x53 : /*Block Text Paragraph Horizontal Alignment*/ break;
        case 0x54 : /*Block Text Paragraph Vertical Alignment*/ break;
        case 0x55 : /*Block Text Line Flow*/ break;
        case 0x60 : /*Block Text Paragraph Spacing*/ break;
        case 0x61 : /*Block Text Paragraph Indent*/ break;
        case 0x62 : /*Block Text Paragraph Tabs*/ break;
        case 0x63 : /*Block Text Paragraph Bullets*/ break;
        case 0x64 : /*Block Text Paragraph Bullet Level*/ break;
        case 0x65 : /*Block Text Paragraph Line Horizontal Alignment*/ break;
        case 0x66 : /*Block Text Paragraph Line Vertical Alignment*/ break;
        case 0x67 : /*Block Text Paragragh Line Spacing*/ break;
        case 0x68 : /*Block Text Paragraph Word Wrap*/ break;
        case 0x70 : /*Block Text Forward Advance Distance*/ break;
        case 0x71 : /*Word Spacing*/ break;
        case 0x72 : /*External Leading*/ break;
        case 0x7a : /*set Gradient Offset*/
        {
            long nHorzOffset = ImplGetI( pElement->nIndexPrecision );
            long nVertOffset = ImplGetI( pElement->nIndexPrecision );
            sal_uInt32 nType = ImplGetUI16();
            mpOutAct->SetGradientOffset( nHorzOffset, nVertOffset, nType );
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0x7b : /*set Gradient Edge*/
        {
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0x7c : /*set Gradient Angle*/
        {
            mpOutAct->SetGradientAngle( ImplGetI( pElement->nIndexPrecision ) );
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0x7d : /*set Gradient Description*/
        {
            ImplGetI( pElement->nIndexPrecision ); // -Wall is this needed?
            sal_uInt32  nNumberOfStages = ImplGetI( pElement->nIndexPrecision );
            sal_uInt32  i, nColorFrom = 0;
            sal_uInt32  nColorTo = 0xffffff;

            //FIXME,  does this loop actually do anything?
            for ( i = 0; i < nNumberOfStages; i++ )
            {
                ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize ); // -Wall is this needed
            }

            for ( i = 0; i <= nNumberOfStages; i++ )
            {
                sal_uInt32 nPara = mnParaSize + 24;
                if ( i == 0 )
                {
                    nColorTo = ImplGetBitmapColor();
                    nColorFrom = nColorTo ^ 0xffffff;
                }
                else if ( i == 1 )
                    nColorFrom = ImplGetBitmapColor();
                mnParaSize = nPara;
            }
            if ( nNumberOfStages > 1 )
                mpOutAct->SetGradientStyle( 0xff, 1 );

            mpOutAct->SetGradientDescriptor( nColorFrom, nColorTo );
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0x7e : /*set Gradient Style*/
        {
            sal_uInt32 nStyle = ImplGetUI16( 8 );
            double fRatio = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            mpOutAct->SetGradientStyle( nStyle, fRatio );
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0xff : /*inquire Font metrics*/ break;
        case 0xfe : /*inquire character widths*/ break;
        case 0xfd : /*set Text Font*/ break;
        case 0xfc : /*set current position*/ break;
        case 0xfb : /*set current position mode*/ break;
        case 0xfa : /*set character height mode*/ break;
        case 0xf9 : /*set Transform matrix 2D*/ break;
        case 0xf8 : /*set Transform matrix 3D*/ break;
        case 0xf7 : /*pop transformation state*/ break;
        case 0xf6 : /*clear transformation state*/ break;
        case 0xf5 : /*set character widths*/ break;
        case 0xf4 : /*set color name - for Pantone support*/ break;
        default: break;
    }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
