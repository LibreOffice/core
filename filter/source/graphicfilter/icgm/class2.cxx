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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <main.hxx>

// ---------------------------------------------------------------

void CGM::ImplDoClass2()
{
    sal_uInt32  nUInteger;
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Scaling Mode" )
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
        case 0x02 : ComOut( CGM_LEVEL1, "Color Selection Mode" )
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
        case 0x03 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Line Width Specification Mode" )
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
        case 0x04 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Marker Size Specification Mode" )
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
        case 0x05 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Edge Width Specification Mode" )
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
        case 0x06 : ComOut( CGM_LEVEL1, "VDC Extent" )
        {
            ImplGetRectangleNS( pElement->aVDCExtent );
            ImplSetMapMode();
        }
        break;
        case 0x07 : ComOut( CGM_LEVEL1, "Background Color" )
            pElement->nBackGroundColor = ImplGetBitmapColor( sal_True );
        break;
        case 0x08 : ComOut( CGM_LEVEL2, "Device Viewport" )
        {
            if ( pElement->eVDCType == VDC_INTEGER )
                ImplGetRectangle( pElement->aDeviceViewPort );
                ImplSetMapMode();
        }
        break;
        case 0x09 : ComOut( CGM_LEVEL2, "Device Viewport Specification Mode" )
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
        case 0x0a : ComOut( CGM_LEVEL2, "Device Viewport Mapping" )
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
        case 0x0b : ComOut( CGM_LEVEL2, "Line Representation" )
        {
            LineBundle  aTempLineBundle;
            aTempLineBundle.SetIndex( ImplGetI( pElement->nIndexPrecision ) );
            aTempLineBundle.eLineType = (LineType)ImplGetI( pElement->nIndexPrecision );
            aTempLineBundle.nLineWidth = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            aTempLineBundle.SetColor( ImplGetBitmapColor() );
            pElement->InsertBundle( pElement->aLineList, aTempLineBundle );
        }
        break;
        case 0x0c : ComOut( CGM_LEVEL2, "Marker Representation" )
        {
            MarkerBundle aTempMarkerBundle;
            aTempMarkerBundle.SetIndex( ImplGetI( pElement->nIndexPrecision ) );
            aTempMarkerBundle.eMarkerType = (MarkerType)ImplGetI( pElement->nIndexPrecision );
            aTempMarkerBundle.nMarkerSize = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            aTempMarkerBundle.SetColor( ImplGetBitmapColor() );
            pElement->InsertBundle( pElement->aMarkerList, aTempMarkerBundle );
        }
        break;
        case 0x0d : ComOut( CGM_LEVEL2, "Text Representation" )
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
        case 0x0e : ComOut( CGM_LEVEL2, "Fill Representation" )
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
        case 0x0f : ComOut( CGM_LEVEL2, "Edge Representation" )
        {
            EdgeBundle aTempEdgeBundle;
            aTempEdgeBundle.SetIndex( ImplGetI( pElement->nIndexPrecision ) );
            aTempEdgeBundle.eEdgeType = (EdgeType)ImplGetI( pElement->nIndexPrecision );
            aTempEdgeBundle.nEdgeWidth = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
            aTempEdgeBundle.SetColor( ImplGetBitmapColor() );
            pElement->InsertBundle( pElement->aEdgeList, aTempEdgeBundle );
        }
        break;
        case 0x10 : ComOut( CGM_LEVEL3, "Interior Style Specification Mode" ) break;    // NS
        case 0x11 : ComOut( CGM_LEVEL3, "Line and Edge Type Definition" ) break;
        case 0x12 : ComOut( CGM_LEVEL3, "Hatch Style Definition" ) break;               // NS
        case 0x13 : ComOut( CGM_LEVEL3, "Geometric Pattern Definition" ) break;         // NS
        case 0xff : ComOut( CGM_GDSF_ONLY, "inquire VDC EXTENT" ) break;
        case 0xfe : ComOut( CGM_GDSF_ONLY, "inquire Background Color" ) break;
        case 0xfd : ComOut( CGM_GDSF_ONLY, "inquire Device Viewport" ) break;
        case 0xfc : ComOut( CGM_GDSF_ONLY, "set Font Selection Mode" ) break;
        case 0xfb : ComOut( CGM_GDSF_ONLY, "inquire Color Selection Mode" ) break;
        case 0xfa : ComOut( CGM_GDSF_ONLY, "inquire Font Selection Mode" ) break;
        case 0xf9 : ComOut( CGM_GDSF_ONLY, "set Char Height Spec Mode" )
        {
            ImplGetUI16(); // -Wall is this really needed?
        }
        break;
        case 0xf8 : ComOut( CGM_GDSF_ONLY, "set Background Style" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};


