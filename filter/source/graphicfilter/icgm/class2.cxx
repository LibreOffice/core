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


#include <main.hxx>

// ---------------------------------------------------------------

void CGM::ImplDoClass2()
{
    sal_uInt32  nUInteger;
    switch ( mnElementID )
    {
        case 0x01 : /*Scaling Mode*/
        {
            if ( mnElementSize )    // HACK (NASA.CGM)
            {
                switch( ImplGetUI16() )
                {
                    case 0 : pElement->eScalingMode = SM_ABSTRACT; break;
                    case 1 : pElement->eScalingMode = SM_METRIC; break;
                    default : mbStatus = sal_False; break;
                }
                pElement->nScalingFactor = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
                ImplSetMapMode();
            }
        }
        break;
        case 0x02 : /*Color Selection Mode*/
        {
            nUInteger = ImplGetUI16();
            switch( nUInteger )
            {
                case 0 : pElement->eColorSelectionMode = CSM_INDEXED; break;
                case 1 : pElement->eColorSelectionMode = CSM_DIRECT; break;
                default : mbStatus = sal_False; break;
            }
        }
        break;
        case 0x03 : /*Line Width Specification Mode*/
        {
            nUInteger = ImplGetUI16();
            switch( nUInteger )
            {
                case 0 : pElement->eLineWidthSpecMode = SM_ABSOLUTE; break;
                case 1 : pElement->eLineWidthSpecMode = SM_SCALED; break;
                default : mbStatus = sal_False; break;
            }
        }
        break;
        case 0x04 : /*Marker Size Specification Mode*/
        {
            nUInteger = ImplGetUI16();
            switch( nUInteger )
            {
                case 0 : pElement->eMarkerSizeSpecMode = SM_ABSOLUTE; break;
                case 1 : pElement->eMarkerSizeSpecMode = SM_SCALED; break;
                default : mbStatus = sal_False; break;
            }
        }
        break;
        case 0x05 : /*Edge Width Specification Mode*/
        {
            nUInteger = ImplGetUI16();
            switch( nUInteger )
            {
                case 0 : pElement->eEdgeWidthSpecMode = SM_ABSOLUTE; break;
                case 1 : pElement->eEdgeWidthSpecMode = SM_SCALED; break;
                default : mbStatus = sal_False; break;
            }
        }
        break;
        case 0x06 : /*VDC Extent*/
        {
            ImplGetRectangleNS( pElement->aVDCExtent );
            ImplSetMapMode();
        }
        break;
        case 0x07 : /*Background Color*/
            pElement->nBackGroundColor = ImplGetBitmapColor( sal_True );
        break;
        case 0x08 : /*Device Viewport*/
        {
            if ( pElement->eVDCType == VDC_INTEGER )
                ImplGetRectangle( pElement->aDeviceViewPort );
                ImplSetMapMode();
        }
        break;
        case 0x09 : /*Device Viewport Specification Mode*/
        {
            nUInteger = ImplGetUI16( 8 );
            switch( nUInteger )
            {
                case 0 : pElement->eDeviceViewPortMode = DVPM_FRACTION; break;
                case 1 : pElement->eDeviceViewPortMode = DVPM_METRIC; break;
                case 2 : pElement->eDeviceViewPortMode = DVPM_DEVICE; break;
                default : mbStatus = sal_False; break;
            }
            pElement->nDeviceViewPortScale = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            ImplSetMapMode();
        }
        break;
        case 0x0a : /*Device Viewport Mapping*/
        {
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eDeviceViewPortMap = DVPM_NOT_FORCED; break;
                case 1 : pElement->eDeviceViewPortMap = DVPM_FORCED; break;
                default : mbStatus = sal_False; break;
            }
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eDeviceViewPortMapH = DVPMH_LEFT; break;
                case 1 : pElement->eDeviceViewPortMapH = DVPMH_CENTER; break;
                case 2 : pElement->eDeviceViewPortMapH = CVPMH_RIGHT; break;
                default : mbStatus = sal_False; break;
            }
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eDeviceViewPortMapV = DVPMV_BOTTOM; break;
                case 1 : pElement->eDeviceViewPortMapV = DVPMV_CENTER; break;
                case 2 : pElement->eDeviceViewPortMapV = DVPMV_TOP; break;
                default : mbStatus = sal_False; break;
            }
            ImplSetMapMode();
        }
        break;
        case 0x0b : /*Line Representation*/
        {
            LineBundle  aTempLineBundle;
            aTempLineBundle.SetIndex( ImplGetI( pElement->nIndexPrecision ) );
            aTempLineBundle.eLineType = (LineType)ImplGetI( pElement->nIndexPrecision );
            aTempLineBundle.nLineWidth = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            aTempLineBundle.SetColor( ImplGetBitmapColor() );
            pElement->InsertBundle( pElement->aLineList, aTempLineBundle );
        }
        break;
        case 0x0c : /*Marker Representation*/
        {
            MarkerBundle aTempMarkerBundle;
            aTempMarkerBundle.SetIndex( ImplGetI( pElement->nIndexPrecision ) );
            aTempMarkerBundle.eMarkerType = (MarkerType)ImplGetI( pElement->nIndexPrecision );
            aTempMarkerBundle.nMarkerSize = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            aTempMarkerBundle.SetColor( ImplGetBitmapColor() );
            pElement->InsertBundle( pElement->aMarkerList, aTempMarkerBundle );
        }
        break;
        case 0x0d : /*Text Representation*/
        {
            TextBundle aTempTextBundle;
            aTempTextBundle.SetIndex( ImplGetI( pElement->nIndexPrecision ) );
            aTempTextBundle.nTextFontIndex = ImplGetI( pElement->nIndexPrecision );
            aTempTextBundle.eTextPrecision = (TextPrecision)ImplGetI( pElement->nIndexPrecision );
            aTempTextBundle.nCharacterSpacing = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            aTempTextBundle.nCharacterExpansion = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            aTempTextBundle.SetColor( ImplGetBitmapColor() );
            pElement->InsertBundle( pElement->aTextList, aTempTextBundle );
        }
        break;
        case 0x0e : /*Fill Representation*/
        {
            FillBundle aTempFillBundle;
            aTempFillBundle.SetIndex( ImplGetI( pElement->nIndexPrecision ) );
            aTempFillBundle.eFillInteriorStyle = (FillInteriorStyle)ImplGetI( pElement->nIndexPrecision );
            aTempFillBundle.SetColor( ImplGetBitmapColor() );
            aTempFillBundle.nFillPatternIndex = ImplGetI( pElement->nIndexPrecision );
            aTempFillBundle.nFillHatchIndex = ImplGetI( pElement->nIndexPrecision );
            pElement->InsertBundle( pElement->aFillList, aTempFillBundle );
        }
        break;
        case 0x0f : /*Edge Representation*/
        {
            EdgeBundle aTempEdgeBundle;
            aTempEdgeBundle.SetIndex( ImplGetI( pElement->nIndexPrecision ) );
            aTempEdgeBundle.eEdgeType = (EdgeType)ImplGetI( pElement->nIndexPrecision );
            aTempEdgeBundle.nEdgeWidth = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            aTempEdgeBundle.SetColor( ImplGetBitmapColor() );
            pElement->InsertBundle( pElement->aEdgeList, aTempEdgeBundle );
        }
        break;
        case 0x10 : /*Interior Style Specification Mode */break;    // NS
        case 0x11 : /*Line and Edge Type Definition */break;
        case 0x12 : /*Hatch Style Definition */break;               // NS
        case 0x13 : /*Geometric Pattern Definition */break;         // NS
        case 0xff : /*inquire VDC EXTENT */break;
        case 0xfe : /*inquire Background Color */break;
        case 0xfd : /*inquire Device Viewport */break;
        case 0xfc : /*set Font Selection Mode */break;
        case 0xfb : /*inquire Color Selection Mode */break;
        case 0xfa : /*inquire Font Selection Mode */break;
        case 0xf9 : /*set Char Height Spec Mode*/
        {
            ImplGetUI16(); // -Wall is this really needed?
        }
        break;
        case 0xf8 : /*set Background Style */break;
        default: break;
    }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
