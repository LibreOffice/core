/*************************************************************************
 *
 *  $RCSfile: class2.cxx,v $
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
            sal_uInt32 nType = ImplGetUI16();
        }
        break;
        case 0xf8 : ComOut( CGM_GDSF_ONLY, "set Background Style" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};


