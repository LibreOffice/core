/*************************************************************************
 *
 *  $RCSfile: class5.cxx,v $
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
//#define VCL_NEED_BASETSD

#if defined( WNT ) || defined( WIN )
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#endif
#include <main.hxx>
#include <outact.hxx>


// ---------------------------------------------------------------

void CGM::ImplDoClass5()
{
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_LEVEL1, "Line Bundle Index" )
            pElement->pLineBundle = (LineBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aLineList, pElement->aLineBundle );
        break;
        case 0x02 : ComOut( CGM_LEVEL1, "Line Type" )
        {
            if ( pElement->nAspectSourceFlags & ASF_LINETYPE )
                pElement->pLineBundle->eLineType = (LineType)ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aLineBundle.eLineType = (LineType)ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x03 : ComOut( CGM_LEVEL1, "Line Width" )
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
        case 0x04 : ComOut( CGM_LEVEL1, "Line Color" )
        {
            if ( pElement->nAspectSourceFlags & ASF_LINECOLOR )
                pElement->pLineBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aLineBundle.SetColor( ImplGetBitmapColor() );
            if ( mnMode & CGM_IMPORT_IM )                                       // PATCH
                mnParaSize = mnElementSize;
        }
        break;
        case 0x05 : ComOut( CGM_LEVEL1, "Marker Bundle Index" )
            pElement->pMarkerBundle = (MarkerBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aMarkerList, pElement->aMarkerBundle );
        break;
        case 0x06 : ComOut( CGM_LEVEL1, "Marker Type" )
        {
            if ( pElement->nAspectSourceFlags & ASF_MARKERTYPE )
                pElement->pMarkerBundle->eMarkerType = (MarkerType)ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aMarkerBundle.eMarkerType = (MarkerType)ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x07 : ComOut( CGM_LEVEL1, "Marker Size" )
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
        case 0x08 : ComOut( CGM_LEVEL1, "Marker Color" )
        {
            if ( pElement->nAspectSourceFlags & ASF_MARKERCOLOR )
                pElement->pMarkerBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aMarkerBundle.SetColor( ImplGetBitmapColor() );
            if ( mnMode & CGM_IMPORT_IM )                                       // PATCH
                mnParaSize = mnElementSize;
        }
        break;
        case 0x09 : ComOut( CGM_LEVEL1, "Text Bundle Index" )
            pElement->pTextBundle = (TextBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aTextList, pElement->aTextBundle );
        break;
        case 0x0a : ComOut( CGM_LEVEL1, "Text Font Index" )
        {
            if ( pElement->nAspectSourceFlags & ASF_TEXTFONTINDEX )
                pElement->pTextBundle->nTextFontIndex = ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aTextBundle.nTextFontIndex = ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x0b : ComOut( CGM_LEVEL1, "Text Precision" )
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
        case 0x0c : ComOut( CGM_LEVEL1, "Character Expansion Factor" )
        {
            if ( pElement->nAspectSourceFlags & ASF_CHARACTEREXPANSION )
                pElement->pTextBundle->nCharacterExpansion = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            else
                pElement->aTextBundle.nCharacterExpansion = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
        }
        break;
        case 0x0d : ComOut( CGM_LEVEL1, "Character Spacing" )
        {
            if ( pElement->nAspectSourceFlags & ASF_CHARACTERSPACING )
                pElement->pTextBundle->nCharacterSpacing = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            else
                pElement->aTextBundle.nCharacterSpacing = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
        }
        break;
        case 0x0e : ComOut( CGM_LEVEL1, "Text Color" )
        {
            if ( pElement->nAspectSourceFlags & ASF_TEXTCOLOR )
                pElement->pTextBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aTextBundle.SetColor( ImplGetBitmapColor() );
            if ( mnMode & CGM_IMPORT_IM )                                       // PATCH
                mnParaSize = mnElementSize;
        }
        break;
        case 0x0f : ComOut( CGM_LEVEL1, "Character Height" )
        {
                if ( pElement->eVDCType == VDC_INTEGER )
                    pElement->nCharacterHeight = ImplGetI( pElement->nVDCIntegerPrecision );
                else // ->floating points
                    pElement->nCharacterHeight = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                ImplMapDouble( pElement->nCharacterHeight );
                pElement->nCharacterHeight /= 18.0;
        }
        break;
        case 0x10 : ComOut( CGM_LEVEL1, "Character Orientation" )
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
        case 0x11 : ComOut( CGM_LEVEL1, "Text Path" )
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
        case 0x12 : ComOut( CGM_LEVEL1, "Text Alignment" )
        {
            pElement->eTextAlignmentH = (TextAlignmentH)ImplGetUI16();
            pElement->eTextAlignmentV = (TextAlignmentV)ImplGetUI16( 8 );
            pElement->nTextAlignmentHCont = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            pElement->nTextAlignmentVCont = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
        }
        break;
        case 0x13 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Character Set Index" )
            pElement->nCharacterSetIndex = ImplGetI( pElement->nIndexPrecision );
        break;
        case 0x14 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Alternate Character Set Index" )
            pElement->nAlternateCharacterSetIndex = ImplGetI( pElement->nIndexPrecision );
        break;
        case 0x15 : ComOut( CGM_LEVEL1, "Fill Bundle Index" )
            pElement->pFillBundle = (FillBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aFillList, pElement->aFillBundle );
        break;
        case 0x16 : ComOut( CGM_LEVEL1, "Fill Interior Style" )
        {
            if ( pElement->nAspectSourceFlags & ASF_FILLINTERIORSTYLE )
                pElement->pFillBundle->eFillInteriorStyle = (FillInteriorStyle)ImplGetUI16();
            else
                pElement->aFillBundle.eFillInteriorStyle = (FillInteriorStyle)ImplGetUI16();
        }
        break;
        case 0x17 : ComOut( CGM_LEVEL1, "Fill Color" )
        {
            if ( pElement->nAspectSourceFlags & ASF_FILLCOLOR )
                pElement->pFillBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aFillBundle.SetColor( ImplGetBitmapColor() );
            if ( mnMode & CGM_IMPORT_IM )                                       // PATCH
                mnParaSize = mnElementSize;
        }
        break;
        case 0x18 : ComOut( CGM_LEVEL1, "Fill Hatch Index" )
        {
            if ( pElement->nAspectSourceFlags & ASF_HATCHINDEX )
                pElement->pFillBundle->nFillHatchIndex = ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aFillBundle.nFillHatchIndex = ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x19 : ComOut( CGM_LEVEL1, "Fill Pattern Index" )
        {
            if ( pElement->nAspectSourceFlags & ASF_PATTERNINDEX )
                pElement->pFillBundle->nFillPatternIndex = ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aFillBundle.nFillPatternIndex = ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x1a : ComOut( CGM_LEVEL1, "Edge Bundle Index" )
            pElement->pEdgeBundle = (EdgeBundle*)pElement->GetBundleIndex( ImplGetI( pElement->nIndexPrecision ), pElement->aEdgeList, pElement->aEdgeBundle );
        break;
        case 0x1b : ComOut( CGM_LEVEL1, "Edge Type" )
        {
            if ( pElement->nAspectSourceFlags & ASF_EDGETYPE )
                pElement->pEdgeBundle->eEdgeType = (EdgeType)ImplGetI( pElement->nIndexPrecision );
            else
                pElement->aEdgeBundle.eEdgeType = (EdgeType)ImplGetI( pElement->nIndexPrecision );
        }
        break;
        case 0x1c : ComOut( CGM_LEVEL1, "Edge Width" )
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
        case 0x1d : ComOut( CGM_LEVEL1, "Edge Color" )
        {
            if ( pElement->nAspectSourceFlags & ASF_EDGECOLOR )
                pElement->pEdgeBundle->SetColor( ImplGetBitmapColor() );
            else
                pElement->aEdgeBundle.SetColor( ImplGetBitmapColor() );
            if ( mnMode & CGM_IMPORT_IM )                                       // PATCH
                mnParaSize = mnElementSize;
        }
        break;
        case 0x1e : ComOut( CGM_LEVEL1, "Edge Visibility" )
        {
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eEdgeVisibility = EV_OFF; break;
                case 1 : pElement->eEdgeVisibility = EV_ON; break;
                default : mbStatus = sal_False;
            }
        }
        break;
        case 0x1f : ComOut( CGM_LEVEL1, "Fill Reference Point" )
            ImplGetPoint( pElement->aFillRefPoint );
        break;
        case 0x20 : ComOut( CGM_LEVEL1, "Pattern Table" ) break;
        case 0x21 : ComOut( CGM_LEVEL1, "Pattern Size" ) break;
        case 0x22 : ComOut( CGM_LEVEL1, "Color Table" )
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
        case 0x23 : ComOut( CGM_LEVEL1, "Aspect Source Flags" )
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
        case 0x24 : ComOut( CGM_LEVEL2, "Pick Identifier" ) break;
        case 0x25 : ComOut( CGM_LEVEL3, "Line Cap" )
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
        case 0x26 : ComOut( CGM_LEVEL3, "Line Join" )
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
        case 0x27 : ComOut( CGM_LEVEL3, "Line Type Continuation" ) break;           // NS
        case 0x28 : ComOut( CGM_LEVEL3, "Line Type Initial Offset" ) break;         // NS
        case 0x29 : ComOut( CGM_LEVEL3, "Text Score Type" ) break;
        case 0x2a : ComOut( CGM_LEVEL3, "Restricted Text Type" ) break;
        case 0x2b : ComOut( CGM_LEVEL3, "Interpolated interior" ) break;
        case 0x2c : ComOut( CGM_LEVEL3, "Edge Cap" ) break;                         // NS
        case 0x2d : ComOut( CGM_LEVEL3, "Edge Join" ) break;
        case 0x2e : ComOut( CGM_LEVEL3, "Edge Type Continuation" ) break;           // NS
        case 0x2f : ComOut( CGM_LEVEL3, "Edge Type Initial Offset" ) break;         // NS
        case 0x30 : ComOut( CGM_LEVEL3, "Symbol Library Index" ) break;             // NS
        case 0x31 : ComOut( CGM_LEVEL3, "Symbol Color" ) break;                     // NS
        case 0x32 : ComOut( CGM_LEVEL3, "Symbol Size" ) break;                      // NS
        case 0x33 : ComOut( CGM_LEVEL3, "Symbol Orientation" ) break;               // NS
        case 0x50 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Region Margins" ) break;
        case 0x51 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Region Expansion" ) break;
        case 0x52 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Region Anchor" ) break;
        case 0x53 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Horizontal Alignment" ) break;
        case 0x54 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Vertical Alignment" ) break;
        case 0x55 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Line Flow" ) break;
        case 0x60 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Spacing" ) break;
        case 0x61 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Indent" ) break;
        case 0x62 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Tabs" ) break;
        case 0x63 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Bullets" ) break;
        case 0x64 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Bullet Level" ) break;
        case 0x65 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Line Horizontal Alignment" ) break;
        case 0x66 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Line Vertical Alignment" ) break;
        case 0x67 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragragh Line Spacing" ) break;
        case 0x68 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Paragraph Word Wrap" ) break;
        case 0x70 : ComOut( CGM_UNKNOWN_LEVEL, "Block Text Forward Advance Distance" ) break;
        case 0x71 : ComOut( CGM_UNKNOWN_LEVEL, "Word Spacing" ) break;
        case 0x72 : ComOut( CGM_UNKNOWN_LEVEL, "External Leading" ) break;
        case 0x7a : ComOut( CGM_UNKNOWN_LEVEL, "set Gradient Offset" )
        {
            long nHorzOffset = ImplGetI( pElement->nIndexPrecision );
            long nVertOffset = ImplGetI( pElement->nIndexPrecision );
            sal_uInt32 nType = ImplGetUI16();
            mpOutAct->SetGradientOffset( nHorzOffset, nVertOffset, nType );
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0x7b : ComOut( CGM_UNKNOWN_LEVEL, "set Gradient Edge" )
        {
//          mpOutAct->SetGradientEdge( long nEdge );
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0x7c : ComOut( CGM_UNKNOWN_LEVEL, "set Gradient Angle" )
        {
            mpOutAct->SetGradientAngle( ImplGetI( pElement->nIndexPrecision ) );
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0x7d : ComOut( CGM_UNKNOWN_LEVEL, "set Gradient Description" )
        {
            long    nStepDesignator = ImplGetI( pElement->nIndexPrecision );
            sal_uInt32  nNumberOfStages = ImplGetI( pElement->nIndexPrecision );
            sal_uInt32  i, nColorFrom = 0;
            sal_uInt32  nColorTo = 0xffffff;

            for ( i = 0; i < nNumberOfStages; i++ )
            {
                double aFloat = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
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
        case 0x7e : ComOut( CGM_UNKNOWN_LEVEL, "set Gradient Style" )
        {
            sal_uInt32 nStyle = ImplGetUI16( 8 );
            double fRatio = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            mpOutAct->SetGradientStyle( nStyle, fRatio );
            mnAct4PostReset |= ACT4_GRADIENT_ACTION;
        }
        break;
        case 0xff : ComOut( CGM_GDSF_ONLY, "inquire Font metrics" ) break;
        case 0xfe : ComOut( CGM_GDSF_ONLY, "inquire character widths" ) break;
        case 0xfd : ComOut( CGM_GDSF_ONLY, "set Text Font" ) break;
        case 0xfc : ComOut( CGM_GDSF_ONLY, "set current position" ) break;
        case 0xfb : ComOut( CGM_GDSF_ONLY, "set current position mode" ) break;
        case 0xfa : ComOut( CGM_GDSF_ONLY, "set character height mode" ) break;
        case 0xf9 : ComOut( CGM_GDSF_ONLY, "set Transform matrix 2D" ) break;
        case 0xf8 : ComOut( CGM_GDSF_ONLY, "set Transform matrix 3D" ) break;
        case 0xf7 : ComOut( CGM_GDSF_ONLY, "pop transformation state" ) break;
        case 0xf6 : ComOut( CGM_GDSF_ONLY, "clear transformation state" ) break;
        case 0xf5 : ComOut( CGM_GDSF_ONLY, "set character widths" ) break;
        case 0xf4 : ComOut( CGM_GDSF_ONLY, "set color name - for Pantone support" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};


