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

#include "svx/EnhancedCustomShape2d.hxx"
#include "svx/EnhancedCustomShapeGeometry.hxx"
#include "svx/EnhancedCustomShapeTypeNames.hxx"
#include <svx/svdoashp.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpage.hxx>
#include <svx/xflclit.hxx>
#include <svx/sdasaitm.hxx>
#include <svx/svdmodel.hxx>
#include <rtl/crc.h>
#include <rtl/math.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xhatch.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <rtl/strbuf.hxx>
#include <math.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand;

void EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( EnhancedCustomShapeParameter& rParameter, const sal_Int32 nValue )
{
    sal_uInt32 nDat = (sal_uInt32)nValue;
    sal_Int32  nNewValue = nValue;

    // check if this is a special point
    if ( ( nDat >> 16 ) == 0x8000 )
    {
        nNewValue = (sal_uInt16)nDat;
        rParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
    }
    else
        rParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
    rParameter.Value <<= nNewValue;
}

OUString EnhancedCustomShape2d::GetEquation( const sal_uInt16 nFlags, sal_Int32 nP1, sal_Int32 nP2, sal_Int32 nP3 )
{
    OUString aEquation;
    bool b1Special = ( nFlags & 0x2000 ) != 0;
    bool b2Special = ( nFlags & 0x4000 ) != 0;
    bool b3Special = ( nFlags & 0x8000 ) != 0;
    switch( nFlags & 0xff )
    {
        case 0 :
        case 14 :
        {
            sal_Int32 nOptimize = 0;
            if ( nP1 )
                nOptimize |= 1;
            if ( nP2 )
                nOptimize |= 2;
            if ( b1Special )
                nOptimize |= 4;
            if ( b2Special )
                nOptimize |= 8;
            switch( nOptimize )
            {
                case 0 :
                break;
                case 1 :
                case 4 :
                case 5 :
                    EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
                break;
                case 2 :
                case 8 :
                case 10:
                    EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
                break;
                default :
                {
                    EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
                    aEquation += "+";
                    EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
                }
                break;
            }
            if ( b3Special || nP3 )
            {
                aEquation += "-";
                EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            }
        }
        break;
        case 1 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            if ( b2Special || ( nP2 != 1 ) )
            {
                aEquation += "*";
                EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            }
            if ( b3Special || ( ( nP3 != 1 ) && ( nP3 != 0 ) ) )
            {
                aEquation += "/";
                EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            }
        }
        break;
        case 2 :
        {
            aEquation += "(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "+";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += ")/2";
        }
        break;
        case 3 :
        {
            aEquation += "abs(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += ")";
        }
        break;
        case 4 :
        {
            aEquation += "min(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += ",";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += ")";
        }
        break;
        case 5 :
        {
            aEquation += "max(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += ",";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += ")";
        }
        break;
        case 6 :
        {
            aEquation += "if(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += ",";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += ",";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += ")";
        }
        break;
        case 7 :
        {
            aEquation += "sqrt(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "*";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "+";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "*";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "+";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += "*";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += ")";
        }
        break;
        case 8 :
        {
            aEquation += "atan2(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += ",";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += ")/(pi/180)";
        }
        break;
        case 9 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "*sin(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "*(pi/180))";
        }
        break;
        case 10 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "*cos(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "*(pi/180))";
        }
        break;
        case 11 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "*cos(atan2(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += ",";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "))";
        }
        break;
        case 12 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "*sin(atan2(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += ",";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "))";
        }
        break;
        case 13 :
        {
            aEquation += "sqrt(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += ")";
        }
        break;
        case 15 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += "*sqrt(1-(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "/";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += ")";
            aEquation += "*(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "/";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "))";
        }
        break;
        case 16 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "*tan(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += ")";
        }
        break;
        case 0x80 :
        {
            aEquation += "sqrt(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += "*";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += "-";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "*";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += ")";
        }
        break;
        case 0x81 :
        {
            aEquation += "(cos(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += "*(pi/180))*(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "-10800)+sin(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += "*(pi/180))*(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "-10800))+10800";
        }
        break;
        case 0x82 :
        {
            aEquation += "-(sin(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += "*(pi/180))*(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += "-10800)-cos(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += "*(pi/180))*(";
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += "-10800))+10800";
        }
        break;
    }
    return aEquation;
}

void EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( OUString& rParameter, const sal_Int32 nPara, const bool bIsSpecialValue )
{
    if ( bIsSpecialValue )
    {
        if ( nPara & 0x400 )
        {
            rParameter += "?";
            rParameter += OUString::number( ( nPara & 0xff ) );
            rParameter += " ";
        }
        else
        {
            switch( nPara )
            {
                case DFF_Prop_adjustValue :
                case DFF_Prop_adjust2Value :
                case DFF_Prop_adjust3Value :
                case DFF_Prop_adjust4Value :
                case DFF_Prop_adjust5Value :
                case DFF_Prop_adjust6Value :
                case DFF_Prop_adjust7Value :
                case DFF_Prop_adjust8Value :
                case DFF_Prop_adjust9Value :
                case DFF_Prop_adjust10Value :
                {
                    rParameter += "$";
                    rParameter += OUString::number( ( nPara - DFF_Prop_adjustValue ) );
                    rParameter += " ";
                }
                break;
                case DFF_Prop_geoLeft :
                {
                    rParameter += "left";
                }
                break;
                case DFF_Prop_geoTop :
                {
                    rParameter += "top";
                }
                break;
                case DFF_Prop_geoRight :
                {
                    rParameter += "right";
                }
                break;
                case DFF_Prop_geoBottom :
                {
                    rParameter += "bottom";
                }
                break;
            }
        }
    }
    else
    {
        rParameter += OUString::number( ( nPara ) );
    }
}

void EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( EnhancedCustomShapeParameter& rParameter, const sal_Int32 nPara, const bool bIsSpecialValue, bool bHorz )
{
    sal_Int32 nValue = 0;
    if ( bIsSpecialValue )
    {
        if ( ( nPara >= 0x100 ) && ( nPara <= 0x107 ) )
        {
            nValue = nPara & 0xff;
            rParameter.Type = EnhancedCustomShapeParameterType::ADJUSTMENT;
        }
        else if ( ( nPara >= 3 ) && ( nPara <= 0x82 ) )
        {
            nValue = nPara - 3;
            rParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
        }
        else if ( nPara == 0 )
        {
            nValue = 0;
            if ( bHorz )
                rParameter.Type = EnhancedCustomShapeParameterType::LEFT;
            else
                rParameter.Type = EnhancedCustomShapeParameterType::TOP;
        }
        else if ( nPara == 1 )
        {
            nValue = 0;
            if ( bHorz )
                rParameter.Type = EnhancedCustomShapeParameterType::RIGHT;
            else
                rParameter.Type = EnhancedCustomShapeParameterType::BOTTOM;
        }
        else if ( nPara == 2 )  // means to be centered, but should not be
        {                       // used in our implementation
            nValue = 5600;
            rParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
        }
        else
        {
            nValue = nPara;
            rParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
        }
    }
    else
    {
        nValue = nPara;
        rParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
    }
    rParameter.Value <<= nValue;
}

bool EnhancedCustomShape2d::ConvertSequenceToEnhancedCustomShape2dHandle(
    const css::beans::PropertyValues& rHandleProperties,
        EnhancedCustomShape2d::Handle& rDestinationHandle )
{
    bool bRetValue = false;
    sal_uInt32 i, nProperties = rHandleProperties.getLength();
    if ( nProperties )
    {
        rDestinationHandle.nFlags = HandleFlags::NONE;
        for ( i = 0; i < nProperties; i++ )
        {
            const css::beans::PropertyValue& rPropVal = rHandleProperties[ i ];

            if ( rPropVal.Name == "Position" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aPosition )
                    bRetValue = true;
            }
            else if ( rPropVal.Name == "MirroredX" )
            {
                bool bMirroredX;
                if ( rPropVal.Value >>= bMirroredX )
                {
                    if ( bMirroredX )
                        rDestinationHandle.nFlags |= HandleFlags::MIRRORED_X;
                }
            }
            else if ( rPropVal.Name == "MirroredY" )
            {
                bool bMirroredY;
                if ( rPropVal.Value >>= bMirroredY )
                {
                    if ( bMirroredY )
                        rDestinationHandle.nFlags |= HandleFlags::MIRRORED_Y;
                }
            }
            else if ( rPropVal.Name == "Switched" )
            {
                bool bSwitched;
                if ( rPropVal.Value >>= bSwitched )
                {
                    if ( bSwitched )
                        rDestinationHandle.nFlags |= HandleFlags::SWITCHED;
                }
            }
            else if ( rPropVal.Name == "Polar" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aPolar )
                    rDestinationHandle.nFlags |= HandleFlags::POLAR;
            }
            else if ( rPropVal.Name == "RefX" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.nRefX )
                    rDestinationHandle.nFlags |= HandleFlags::REFX;
            }
            else if ( rPropVal.Name == "RefY" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.nRefY )
                    rDestinationHandle.nFlags |= HandleFlags::REFY;
            }
            else if ( rPropVal.Name == "RefAngle" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.nRefAngle )
                    rDestinationHandle.nFlags |= HandleFlags::REFANGLE;
            }
            else if ( rPropVal.Name == "RefR" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.nRefR )
                    rDestinationHandle.nFlags |= HandleFlags::REFR;
            }
            else if ( rPropVal.Name == "RadiusRangeMinimum" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aRadiusRangeMinimum )
                    rDestinationHandle.nFlags |= HandleFlags::RADIUS_RANGE_MINIMUM;
            }
            else if ( rPropVal.Name == "RadiusRangeMaximum" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aRadiusRangeMaximum )
                    rDestinationHandle.nFlags |= HandleFlags::RADIUS_RANGE_MAXIMUM;
            }
            else if ( rPropVal.Name == "RangeXMinimum" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aXRangeMinimum )
                    rDestinationHandle.nFlags |= HandleFlags::RANGE_X_MINIMUM;
            }
            else if ( rPropVal.Name == "RangeXMaximum" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aXRangeMaximum )
                    rDestinationHandle.nFlags |= HandleFlags::RANGE_X_MAXIMUM;
            }
            else if ( rPropVal.Name == "RangeYMinimum" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aYRangeMinimum )
                    rDestinationHandle.nFlags |= HandleFlags::RANGE_Y_MINIMUM;
            }
            else if ( rPropVal.Name == "RangeYMaximum" )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aYRangeMaximum )
                    rDestinationHandle.nFlags |= HandleFlags::RANGE_Y_MAXIMUM;
            }
        }
    }
    return bRetValue;
}

const sal_Int32* EnhancedCustomShape2d::ApplyShapeAttributes( const SdrCustomShapeGeometryItem& rGeometryItem )
{
    const sal_Int32* pDefData = nullptr;
    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
    if ( pDefCustomShape )
        pDefData = pDefCustomShape->pDefData;


    // AdjustmentValues
    const Any* pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( "AdjustmentValues" );
    if ( pAny )
        *pAny >>= seqAdjustmentValues;


    // Coordsize
    const Any* pViewBox = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( "ViewBox" );
    css::awt::Rectangle aViewBox;
    if ( pViewBox && (*pViewBox >>= aViewBox ) )
    {
        nCoordLeft    = aViewBox.X;
        nCoordTop     = aViewBox.Y;
        nCoordWidthG  = labs( aViewBox.Width );
        nCoordHeightG = labs( aViewBox.Height);
    }
    const OUString sPath( "Path" );


    // Path/Coordinates
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, "Coordinates" );
    if ( pAny )
        *pAny >>= seqCoordinates;


    // Path/GluePoints
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, "GluePoints" );
    if ( pAny )
        *pAny >>= seqGluePoints;


    // Path/Segments
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, "Segments" );
    if ( pAny )
        *pAny >>= seqSegments;


    // Path/SubViewSize
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, "SubViewSize" );
    if ( pAny )
        *pAny >>= seqSubViewSize;


    // Path/StretchX
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, "StretchX" );
    if ( pAny )
    {
        sal_Int32 nStretchX = 0;
        if ( *pAny >>= nStretchX )
            nXRef = nStretchX;
    }


    // Path/StretchY
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, "StretchY" );
    if ( pAny )
    {
        sal_Int32 nStretchY = 0;
        if ( *pAny >>= nStretchY )
            nYRef = nStretchY;
    }


    // Path/TextFrames
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, "TextFrames" );
    if ( pAny )
        *pAny >>= seqTextFrames;


    // Equations
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( "Equations" );
    if ( pAny )
        *pAny >>= seqEquations;


    // Handles
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( "Handles" );
    if ( pAny )
        *pAny >>= seqHandles;

    return pDefData;
}

EnhancedCustomShape2d::~EnhancedCustomShape2d()
{
}

void EnhancedCustomShape2d::SetPathSize( sal_Int32 nIndex )
{
    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;

    if ( seqSubViewSize.getLength() && nIndex < seqSubViewSize.getLength() ) {
        nWidth = seqSubViewSize[ nIndex ].Width;
        nHeight = seqSubViewSize[ nIndex ].Height;
        SAL_INFO(
            "svx",
            "set subpath " << nIndex << " size: " << nWidth << " x "
                << nHeight);
    }

    if ( nWidth && nHeight ) {
        nCoordWidth = nWidth;
        nCoordHeight = nHeight;
    } else {
        nCoordWidth = nCoordWidthG;
        nCoordHeight = nCoordHeightG;
    }

    fXScale = nCoordWidth == 0 ? 0.0 : (double)aLogicRect.GetWidth() / (double)nCoordWidth;
    fYScale = nCoordHeight == 0 ? 0.0 : (double)aLogicRect.GetHeight() / (double)nCoordHeight;
    if ( bOOXMLShape )
    {
        SAL_INFO(
            "svx",
            "ooxml shape, path width: " << nCoordWidth << " height: "
                << nCoordHeight);
        if ( nCoordWidth == 0 )
            fXScale = 1.0;
        if ( nCoordHeight == 0 )
            fYScale = 1.0;
    }
    if ( (sal_uInt32)nXRef != 0x80000000 && aLogicRect.GetHeight() )
    {
        fXRatio = (double)aLogicRect.GetWidth() / (double)aLogicRect.GetHeight();
        if ( fXRatio > 1 )
            fXScale /= fXRatio;
        else
            fXRatio = 1.0;
    }
    else
        fXRatio = 1.0;
    if ( (sal_uInt32)nYRef != 0x80000000 && aLogicRect.GetWidth() )
    {
        fYRatio = (double)aLogicRect.GetHeight() / (double)aLogicRect.GetWidth();
        if ( fYRatio > 1 )
            fYScale /= fYRatio;
        else
            fYRatio = 1.0;
    }
    else
        fYRatio = 1.0;
}

EnhancedCustomShape2d::EnhancedCustomShape2d( SdrObject* pAObj ) :
    SfxItemSet          ( pAObj->GetMergedItemSet() ),
    pCustomShapeObj     ( pAObj ),
    eSpType             ( mso_sptNil ),
    nCoordLeft          ( 0 ),
    nCoordTop           ( 0 ),
    nCoordWidthG        ( 21600 ),
    nCoordHeightG       ( 21600 ),
    bOOXMLShape         ( false ),
    nXRef               ( 0x80000000 ),
    nYRef               ( 0x80000000 ),
    nColorData          ( 0 ),
    bTextFlow           ( false ),
    bFilled             ( static_cast<const XFillStyleItem&>(pAObj->GetMergedItem( XATTR_FILLSTYLE )).GetValue() != drawing::FillStyle_NONE ),
    bStroked            ( static_cast<const XLineStyleItem&>(pAObj->GetMergedItem( XATTR_LINESTYLE )).GetValue() != drawing::LineStyle_NONE ),
    bFlipH              ( false ),
    bFlipV              ( false )
{
    // bTextFlow needs to be set before clearing the TextDirection Item

    ClearItem( SDRATTR_TEXTDIRECTION ); //SJ: vertical writing is not required, by removing this item no outliner is created

    // #i105323# For 2D AutoShapes, the shadow attribute does not need to be applied to any
    // of the constructed helper SdrObjects. This would lead to problems since the shadow
    // of one helper object would fall on one helper object behind it (e.g. with the
    // eyes of the smiley shape). This is not wanted; instead a single shadow 'behind'
    // the AutoShape visualisation is wanted. This is done with primitive functionailty
    // now in SdrCustomShapePrimitive2D::create2DDecomposition, but only for 2D objects
    // (see there and in EnhancedCustomShape3d::Create3DObject to read more).
    // This exception may be removed later when AutoShapes will create primitives directly.
    // So, currently remove the ShadowAttribute from the ItemSet to not apply it to any
    // 2D helper shape.
    ClearItem(SDRATTR_SHADOW);

    Point aP( pCustomShapeObj->GetSnapRect().Center() );
    Size aS( pCustomShapeObj->GetLogicRect().GetSize() );
    aP.X() -= aS.Width() / 2;
    aP.Y() -= aS.Height() / 2;
    aLogicRect = Rectangle( aP, aS );

    OUString sShapeType;
    const SdrCustomShapeGeometryItem& rGeometryItem = static_cast<const SdrCustomShapeGeometryItem&>(pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
    const Any* pAny = rGeometryItem.GetPropertyValueByName( "Type" );
    if ( pAny ) {
        *pAny >>= sShapeType;
        bOOXMLShape = ( sShapeType.startsWith("ooxml-") );
        OSL_TRACE("shape type: %s %d", OUStringToOString( sShapeType, RTL_TEXTENCODING_ASCII_US ).getStr(), bOOXMLShape);
    }
    eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

    pAny = rGeometryItem.GetPropertyValueByName( "MirroredX" );
    if ( pAny )
        *pAny >>= bFlipH;
    pAny = rGeometryItem.GetPropertyValueByName( "MirroredY" );
    if ( pAny )
        *pAny >>= bFlipV;

    if ( dynamic_cast<const SdrObjCustomShape*>( pCustomShapeObj) !=  nullptr )    // should always be a SdrObjCustomShape, but you don't know
        nRotateAngle = (sal_Int32)(static_cast<SdrObjCustomShape*>(pCustomShapeObj)->GetObjectRotation() * 100.0);
    else
         nRotateAngle = pCustomShapeObj->GetRotateAngle();

    /*const sal_Int32* pDefData =*/ ApplyShapeAttributes( rGeometryItem );
    SetPathSize();

    switch( eSpType )
    {
        case mso_sptCan :                       nColorData = 0x20400000; break;
        case mso_sptCube :                      nColorData = 0x302e0000; break;
        case mso_sptActionButtonBlank :         nColorData = 0x502ce400; break;
        case mso_sptActionButtonHome :          nColorData = 0x702ce4ce; break;
        case mso_sptActionButtonHelp :          nColorData = 0x602ce4c0; break;
        case mso_sptActionButtonInformation :   nColorData = 0x702ce4c5; break;
        case mso_sptActionButtonBackPrevious :  nColorData = 0x602ce4c0; break;
        case mso_sptActionButtonForwardNext :   nColorData = 0x602ce4c0; break;
        case mso_sptActionButtonBeginning :     nColorData = 0x602ce4c0; break;
        case mso_sptActionButtonEnd :           nColorData = 0x602ce4c0; break;
        case mso_sptActionButtonReturn :        nColorData = 0x602ce4c0; break;
        case mso_sptActionButtonDocument :      nColorData = 0x702ce4ec; break;
        case mso_sptActionButtonSound :         nColorData = 0x602ce4c0; break;
        case mso_sptActionButtonMovie :         nColorData = 0x602ce4c0; break;
        case mso_sptBevel :                     nColorData = 0x502ce400; break;
        case mso_sptFoldedCorner :              nColorData = 0x20e00000; break;
        case mso_sptSmileyFace :                nColorData = 0x20e00000; break;
        case mso_sptNil :
        {
            if( sShapeType.getLength() > 4 &&
                sShapeType.match( "col-" ))
            {
                nColorData = sShapeType.copy( 4 ).toUInt32( 16 );
            }
        }
        break;
        case mso_sptCurvedLeftArrow :
        case mso_sptCurvedRightArrow :
        case mso_sptCurvedUpArrow :
        case mso_sptCurvedDownArrow :           nColorData = 0x20d00000; break;
        case mso_sptRibbon2 :                   nColorData = 0x30ee0000; break;
        case mso_sptRibbon :                    nColorData = 0x30ee0000; break;

        case mso_sptEllipseRibbon2 :            nColorData = 0x30ee0000; break;
        case mso_sptEllipseRibbon :             nColorData = 0x30ee0000; break;

        case mso_sptVerticalScroll :            nColorData = 0x30ee0000; break;
        case mso_sptHorizontalScroll :          nColorData = 0x30ee0000; break;
        default:
            break;
    }

    sal_Int32 i, nLength = seqEquations.getLength();

    if ( nLength )
    {
        vNodesSharedPtr.resize( nLength );
        vEquationResults.resize( nLength );
        for ( i = 0; i < seqEquations.getLength(); i++ )
        {
            vEquationResults[ i ].bReady = false;
            try
            {
                vNodesSharedPtr[ i ] = EnhancedCustomShape::FunctionParser::parseFunction( seqEquations[ i ], *this );
            }
            catch ( EnhancedCustomShape::ParseError& )
            {
                SAL_INFO(
                    "svx",
                    "error: equation number: " << i << ", parser failed ("
                        << seqEquations[i] << ")");
            }
        }
    }
}
double EnhancedCustomShape2d::GetEnumFunc( const EnumFunc eFunc ) const
{
    double fRet = 0.0;
    switch( eFunc )
    {
        case ENUM_FUNC_PI :         fRet = F_PI; break;
        case ENUM_FUNC_LEFT :       fRet = 0.0; break;
        case ENUM_FUNC_TOP :        fRet = 0.0; break;
        case ENUM_FUNC_RIGHT :      fRet = (double)nCoordWidth * fXRatio;   break;
        case ENUM_FUNC_BOTTOM :     fRet = (double)nCoordHeight * fYRatio; break;
        case ENUM_FUNC_XSTRETCH :   fRet = nXRef; break;
        case ENUM_FUNC_YSTRETCH :   fRet = nYRef; break;
        case ENUM_FUNC_HASSTROKE :  fRet = bStroked ? 1.0 : 0.0; break;
        case ENUM_FUNC_HASFILL :    fRet = bFilled ? 1.0 : 0.0; break;
        case ENUM_FUNC_WIDTH :      fRet = nCoordWidth; break;
        case ENUM_FUNC_HEIGHT :     fRet = nCoordHeight; break;
        case ENUM_FUNC_LOGWIDTH :   fRet = aLogicRect.GetWidth(); break;
        case ENUM_FUNC_LOGHEIGHT :  fRet = aLogicRect.GetHeight(); break;
    }
    return fRet;
}
double EnhancedCustomShape2d::GetAdjustValueAsDouble( const sal_Int32 nIndex ) const
{
    double fNumber = 0.0;
    if ( nIndex < seqAdjustmentValues.getLength() )
    {
        if ( seqAdjustmentValues[ nIndex ].Value.getValueTypeClass() == TypeClass_DOUBLE )
            seqAdjustmentValues[ nIndex ].Value >>= fNumber;
        else
        {
            sal_Int32 nNumber = 0;
            seqAdjustmentValues[ nIndex ].Value >>= nNumber;
            fNumber = (double)nNumber;
        }
    }
    return fNumber;
}
double EnhancedCustomShape2d::GetEquationValueAsDouble( const sal_Int32 nIndex ) const
{
    double fNumber = 0.0;
#if OSL_DEBUG_LEVEL > 1
    static sal_uInt32 nLevel = 0;
#endif
    if ( nIndex < (sal_Int32)vNodesSharedPtr.size() )
    {
        if ( vNodesSharedPtr[ nIndex ].get() ) {
#if OSL_DEBUG_LEVEL > 1
            nLevel ++;
#endif
            try
            {
                if ( vEquationResults[ nIndex ].bReady )
                    fNumber = vEquationResults[ nIndex ].fValue;
                else {
                    // cast to non const, so that we can optimize by caching
                    // equation results, without changing all the const in the stack
                    struct EquationResult &aResult = const_cast<EnhancedCustomShape2d*>(this)->vEquationResults[ nIndex ];

                    fNumber = aResult.fValue = (*vNodesSharedPtr[ nIndex ])();
                    aResult.bReady = true;

                    if ( !rtl::math::isFinite( fNumber ) )
                        fNumber = 0.0;
#if OSL_DEBUG_LEVEL > 1
                    OSL_TRACE("equation %d (level: %d): %s --> %f (angle: %f)", nIndex,
                              nLevel, OUStringToOString( seqEquations[ nIndex ],
                                                         RTL_TEXTENCODING_ASCII_US ).getStr(), fNumber, 180.0*fNumber/10800000.0);
#endif
                }
            }
            catch ( ... )
            {
                OSL_TRACE("error: EnhancedCustomShape2d::GetEquationValueAsDouble failed");
            }
#if OSL_DEBUG_LEVEL > 1
        nLevel --;
#endif
        }
        SAL_INFO(
            "svx",
            "?" << nIndex << " --> " << fNumber << " (angle: "
                << 180.0*fNumber/10800000.0 << ")");
    }

    return fNumber;
}

bool EnhancedCustomShape2d::SetAdjustValueAsDouble( const double& rValue, const sal_Int32 nIndex )
{
    bool bRetValue = false;
    if ( nIndex < seqAdjustmentValues.getLength() )
    {
        // updating our local adjustment sequence
        seqAdjustmentValues[ nIndex ].Value <<= rValue;
        seqAdjustmentValues[ nIndex ].State = css::beans::PropertyState_DIRECT_VALUE;
        bRetValue = true;
    }
    return bRetValue;
}

Point EnhancedCustomShape2d::GetPoint( const css::drawing::EnhancedCustomShapeParameterPair& rPair,
                                        const bool bScale, const bool bReplaceGeoSize ) const
{
    Point       aRetValue;
    sal_uInt32  nPass = 0;
    do
    {
        sal_uInt32  nIndex = nPass;

        double      fVal;
        const EnhancedCustomShapeParameter& rParameter = nIndex ? rPair.Second : rPair.First;
        if ( nPass )    // height
        {
            GetParameter( fVal, rParameter, false, bReplaceGeoSize );
            fVal -= nCoordTop;
            if ( bScale )
            {
                fVal *= fYScale;
            }
            aRetValue.Y() = (sal_Int32)fVal;
        }
        else            // width
        {
            GetParameter( fVal, rParameter, bReplaceGeoSize, false );
            fVal -= nCoordLeft;
            if ( bScale )
            {
                fVal *= fXScale;
            }
            aRetValue.X() = static_cast<long>(fVal);
        }
    }
    while ( ++nPass < 2 );
    return aRetValue;
}

bool EnhancedCustomShape2d::GetParameter( double& rRetValue, const EnhancedCustomShapeParameter& rParameter,
                                              const bool bReplaceGeoWidth, const bool bReplaceGeoHeight ) const
{
    rRetValue = 0.0;
    bool bRetValue = false;
    switch ( rParameter.Type )
    {
        case EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            sal_Int32 nAdjustmentIndex = 0;
            if ( rParameter.Value >>= nAdjustmentIndex )
            {
                rRetValue = GetAdjustValueAsDouble( nAdjustmentIndex );
                bRetValue = true;
            }
        }
        break;
        case EnhancedCustomShapeParameterType::EQUATION :
        {
            sal_Int32 nEquationIndex = 0;
            if ( rParameter.Value >>= nEquationIndex )
            {
                rRetValue = GetEquationValueAsDouble( nEquationIndex );
                bRetValue = true;
            }
        }
        break;
        case EnhancedCustomShapeParameterType::NORMAL :
        {
            if ( rParameter.Value.getValueTypeClass() == TypeClass_DOUBLE )
            {
                double fValue(0.0);
                if ( rParameter.Value >>= fValue )
                {
                    rRetValue = fValue;
                    bRetValue = true;
                }
            }
            else
            {
                sal_Int32 nValue = 0;
                if ( rParameter.Value >>= nValue )
                {
                    rRetValue = nValue;
                    bRetValue = true;
                    if ( bReplaceGeoWidth && ( nValue == nCoordWidth ) )
                        rRetValue *= fXRatio;
                    else if ( bReplaceGeoHeight && ( nValue == nCoordHeight ) )
                        rRetValue *= fYRatio;
                }
            }
        }
        break;
        case EnhancedCustomShapeParameterType::LEFT :
        {
            rRetValue  = 0.0;
            bRetValue = true;
        }
        break;
        case EnhancedCustomShapeParameterType::TOP :
        {
            rRetValue  = 0.0;
            bRetValue = true;
        }
        break;
        case EnhancedCustomShapeParameterType::RIGHT :
        {
            rRetValue = nCoordWidth;
            bRetValue = true;
        }
        break;
        case EnhancedCustomShapeParameterType::BOTTOM :
        {
            rRetValue = nCoordHeight;
            bRetValue = true;
        }
        break;
    }
    return bRetValue;
}

// nLumDat 28-31 = number of luminance entries in nLumDat
// nLumDat 27-24 = nLumDatEntry 0
// nLumDat 23-20 = nLumDatEntry 1 ...
// each 4bit entry is to be interpreted as a 10 percent signed luminance changing
sal_Int32 EnhancedCustomShape2d::GetLuminanceChange( sal_uInt32 nIndex ) const
{
    const sal_uInt32 nCount = nColorData >> 28;
    if ( !nCount )
        return 0;

    if ( nIndex >= nCount )
        nIndex = nCount - 1;

    const sal_Int32 nLumDat = nColorData << ( ( 1 + nIndex ) << 2 );
    return ( nLumDat >> 28 ) * 10;
}

Color EnhancedCustomShape2d::GetColorData( const Color& rFillColor, sal_uInt32 nIndex, double dBrightness ) const
{
    const sal_Int32 nLuminance = GetLuminanceChange(nIndex);
    if( !nLuminance && dBrightness == 1.0 )
        return rFillColor;

    basegfx::BColor aHSVColor=
        basegfx::tools::rgb2hsv(
            basegfx::BColor(rFillColor.GetRed()/255.0,
                            rFillColor.GetGreen()/255.0,
                            rFillColor.GetBlue()/255.0));
    if (nLuminance ) {
        if( nLuminance > 0 )
        {
            aHSVColor.setGreen(
                aHSVColor.getGreen() * (1.0-nLuminance/100.0));
            aHSVColor.setBlue(
                nLuminance/100.0 +
                (1.0-nLuminance/100.0)*aHSVColor.getBlue());
        }
        else if( nLuminance < 0 )
        {
            aHSVColor.setBlue(
                (1.0+nLuminance/100.0)*aHSVColor.getBlue());
        }
    }

    aHSVColor = basegfx::tools::hsv2rgb(aHSVColor);
    return Color( (sal_uInt8)static_cast< sal_Int32 >( basegfx::clamp(dBrightness*aHSVColor.getRed(),0.0,1.0) * 255.0 + 0.5 ),
                  (sal_uInt8)static_cast< sal_Int32 >( basegfx::clamp(dBrightness*aHSVColor.getGreen(),0.0,1.0) * 255.0 + 0.5 ),
                  (sal_uInt8)static_cast< sal_Int32 >( basegfx::clamp(dBrightness*aHSVColor.getBlue(),0.0,1.0) * 255.0 + 0.5 ) );
}

Rectangle EnhancedCustomShape2d::GetTextRect() const
{
    sal_Int32 nIndex, nSize = seqTextFrames.getLength();
    if ( !nSize )
        return aLogicRect;
    nIndex = 0;
    if ( bTextFlow && ( nSize > 1 ) )
        nIndex++;
    Point aTopLeft( GetPoint( seqTextFrames[ nIndex ].TopLeft, !bOOXMLShape, true ) );
    Point aBottomRight( GetPoint( seqTextFrames[ nIndex ].BottomRight, !bOOXMLShape, true ) );
    if ( bFlipH )
    {
        aTopLeft.X() = aLogicRect.GetWidth() - aTopLeft.X();
        aBottomRight.X() = aLogicRect.GetWidth() - aBottomRight.X();
    }
    if ( bFlipV )
    {
        aTopLeft.Y() = aLogicRect.GetHeight() - aTopLeft.Y();
        aBottomRight.Y() = aLogicRect.GetHeight() - aBottomRight.Y();
    }
    Rectangle aRect( aTopLeft, aBottomRight );
    SAL_INFO("svx", aRect.GetWidth() << " x " << aRect.GetHeight());
    if( aRect.GetWidth() <= 1 || aRect.GetHeight() <= 1 )
        return aLogicRect;
    aRect.Move( aLogicRect.Left(), aLogicRect.Top() );
    aRect.Justify();
    return aRect;
}

sal_uInt32 EnhancedCustomShape2d::GetHdlCount() const
{
    return seqHandles.getLength();
}

bool EnhancedCustomShape2d::GetHandlePosition( const sal_uInt32 nIndex, Point& rReturnPosition ) const
{
    bool bRetValue = false;
    if ( nIndex < GetHdlCount() )
    {
        Handle aHandle;
        if ( ConvertSequenceToEnhancedCustomShape2dHandle( seqHandles[ nIndex ], aHandle ) )
        {
            if ( aHandle.nFlags & HandleFlags::POLAR )
            {
                Point aReferencePoint( GetPoint( aHandle.aPolar ) );

                double      fAngle;
                double      fRadius;
                GetParameter( fRadius, aHandle.aPosition.First, false, false );
                GetParameter( fAngle,  aHandle.aPosition.Second, false, false );

                double a = ( 360.0 - fAngle ) * F_PI180;
                double dx = fRadius * fXScale;
                double fX = dx * cos( a );
                double fY =-dx * sin( a );
                rReturnPosition =
                    Point(
                        svx::Round( fX + aReferencePoint.X() ),
                        basegfx::fTools::equalZero(fXScale) ? aReferencePoint.Y() :
                        svx::Round( ( fY * fYScale ) / fXScale + aReferencePoint.Y() ) );
            }
            else
            {
                if ( aHandle.nFlags & HandleFlags::SWITCHED )
                {
                    if ( aLogicRect.GetHeight() > aLogicRect.GetWidth() )
                    {
                        css::drawing::EnhancedCustomShapeParameter aFirst = aHandle.aPosition.First;
                        css::drawing::EnhancedCustomShapeParameter aSecond = aHandle.aPosition.Second;
                        aHandle.aPosition.First = aSecond;
                        aHandle.aPosition.Second = aFirst;
                    }
                }
                rReturnPosition = GetPoint( aHandle.aPosition );
            }
            const GeoStat aGeoStat( static_cast<SdrObjCustomShape*>(pCustomShapeObj)->GetGeoStat() );
            if ( aGeoStat.nShearAngle )
            {
                double nTan = aGeoStat.nTan;
                if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
                    nTan = -nTan;
                ShearPoint( rReturnPosition, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), nTan );
            }
            if ( nRotateAngle )
            {
                double a = nRotateAngle * F_PI18000;
                RotatePoint( rReturnPosition, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), sin( a ), cos( a ) );
            }
            if ( bFlipH )
                rReturnPosition.X() = aLogicRect.GetWidth() - rReturnPosition.X();
            if ( bFlipV )
                rReturnPosition.Y() = aLogicRect.GetHeight() - rReturnPosition.Y();
            rReturnPosition.Move( aLogicRect.Left(), aLogicRect.Top() );
            bRetValue = true;
        }
    }
    return bRetValue;
}

bool EnhancedCustomShape2d::SetHandleControllerPosition( const sal_uInt32 nIndex, const css::awt::Point& rPosition )
{
    bool bRetValue = false;
    if ( nIndex < GetHdlCount() )
    {
        Handle aHandle;
        if ( ConvertSequenceToEnhancedCustomShape2dHandle( seqHandles[ nIndex ], aHandle ) )
        {
            Point aP( rPosition.X, rPosition.Y );
            // apply the negative object rotation to the controller position

            aP.Move( -aLogicRect.Left(), -aLogicRect.Top() );
            if ( bFlipH )
                aP.X() = aLogicRect.GetWidth() - aP.X();
            if ( bFlipV )
                aP.Y() = aLogicRect.GetHeight() - aP.Y();
            if ( nRotateAngle )
            {
                double a = -nRotateAngle * F_PI18000;
                RotatePoint( aP, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), sin( a ), cos( a ) );
            }
            const GeoStat aGeoStat( static_cast<SdrObjCustomShape*>(pCustomShapeObj)->GetGeoStat() );
            if ( aGeoStat.nShearAngle )
            {
                double nTan = -aGeoStat.nTan;
                if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
                    nTan = -nTan;
                ShearPoint( aP, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), nTan );
            }

            double fPos1 = aP.X();  //( bFlipH ) ? aLogicRect.GetWidth() - aP.X() : aP.X();
            double fPos2 = aP.Y();  //( bFlipV ) ? aLogicRect.GetHeight() -aP.Y() : aP.Y();
            fPos1 /= fXScale;
            fPos2 /= fYScale;

            if ( aHandle.nFlags & HandleFlags::SWITCHED )
            {
                if ( aLogicRect.GetHeight() > aLogicRect.GetWidth() )
                {
                    double fX = fPos1;
                    double fY = fPos2;
                    fPos1 = fY;
                    fPos2 = fX;
                }
            }

            sal_Int32 nFirstAdjustmentValue = -1, nSecondAdjustmentValue = -1;

            if ( aHandle.aPosition.First.Type == EnhancedCustomShapeParameterType::ADJUSTMENT )
                aHandle.aPosition.First.Value >>= nFirstAdjustmentValue;
            if ( aHandle.aPosition.Second.Type == EnhancedCustomShapeParameterType::ADJUSTMENT )
                aHandle.aPosition.Second.Value>>= nSecondAdjustmentValue;

            if ( aHandle.nFlags & HandleFlags::POLAR )
            {
                double fXRef, fYRef, fAngle;
                GetParameter( fXRef, aHandle.aPolar.First, false, false );
                GetParameter( fYRef, aHandle.aPolar.Second, false, false );
                const double fDX = fPos1 - fXRef;
                fAngle = -( atan2( -fPos2 + fYRef, ( ( fDX == 0.0L ) ? 0.000000001 : fDX ) ) / F_PI180 );
                double fX = ( fPos1 - fXRef );
                double fY = ( fPos2 - fYRef );
                double fRadius = sqrt( fX * fX + fY * fY );
                if ( aHandle.nFlags & HandleFlags::RADIUS_RANGE_MINIMUM )
                {
                    double fMin;
                    GetParameter( fMin,  aHandle.aRadiusRangeMinimum, false, false );
                    if ( fRadius < fMin )
                        fRadius = fMin;
                }
                if ( aHandle.nFlags & HandleFlags::RADIUS_RANGE_MAXIMUM )
                {
                    double fMax;
                    GetParameter( fMax, aHandle.aRadiusRangeMaximum, false, false );
                    if ( fRadius > fMax )
                        fRadius = fMax;
                }
                if ( nFirstAdjustmentValue >= 0 )
                    SetAdjustValueAsDouble( fRadius, nFirstAdjustmentValue );
                if ( nSecondAdjustmentValue >= 0 )
                    SetAdjustValueAsDouble( fAngle,  nSecondAdjustmentValue );
            }
            else
            {
                if ( aHandle.nFlags & HandleFlags::REFX )
                {
                    nFirstAdjustmentValue = aHandle.nRefX;
                    fPos1 *= 100000.0;
                    fPos1 /= nCoordWidth;
                }
                if ( aHandle.nFlags & HandleFlags::REFY )
                {
                    nSecondAdjustmentValue = aHandle.nRefY;
                    fPos2 *= 100000.0;
                    fPos2 /= nCoordHeight;
                }
                if ( nFirstAdjustmentValue >= 0 )
                {
                    if ( aHandle.nFlags & HandleFlags::RANGE_X_MINIMUM )        // check if horizontal handle needs to be within a range
                    {
                        double fXMin;
                        GetParameter( fXMin, aHandle.aXRangeMinimum, false, false );
                        if ( fPos1 < fXMin )
                            fPos1 = fXMin;
                    }
                    if ( aHandle.nFlags & HandleFlags::RANGE_X_MAXIMUM )        // check if horizontal handle needs to be within a range
                    {
                        double fXMax;
                        GetParameter( fXMax, aHandle.aXRangeMaximum, false, false );
                        if ( fPos1 > fXMax )
                            fPos1 = fXMax;
                    }
                    SetAdjustValueAsDouble( fPos1, nFirstAdjustmentValue );
                }
                if ( nSecondAdjustmentValue >= 0 )
                {
                    if ( aHandle.nFlags & HandleFlags::RANGE_Y_MINIMUM )        // check if vertical handle needs to be within a range
                    {
                        double fYMin;
                        GetParameter( fYMin, aHandle.aYRangeMinimum, false, false );
                        if ( fPos2 < fYMin )
                            fPos2 = fYMin;
                    }
                    if ( aHandle.nFlags & HandleFlags::RANGE_Y_MAXIMUM )        // check if vertical handle needs to be within a range
                    {
                        double fYMax;
                        GetParameter( fYMax, aHandle.aYRangeMaximum, false, false );
                        if ( fPos2 > fYMax )
                            fPos2 = fYMax;
                    }
                    SetAdjustValueAsDouble( fPos2, nSecondAdjustmentValue );
                }
            }
            // and writing them back into the GeometryItem
            SdrCustomShapeGeometryItem aGeometryItem(
                static_cast<const SdrCustomShapeGeometryItem&>(pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )));
            css::beans::PropertyValue aPropVal;
            aPropVal.Name = "AdjustmentValues";
            aPropVal.Value <<= seqAdjustmentValues;
            aGeometryItem.SetPropertyValue( aPropVal );
            pCustomShapeObj->SetMergedItem( aGeometryItem );
            bRetValue = true;
        }
    }
    return bRetValue;
}

void EnhancedCustomShape2d::SwapStartAndEndArrow( SdrObject* pObj ) //#108274
{
    XLineStartItem       aLineStart;
    aLineStart.SetLineStartValue(static_cast<const XLineStartItem&>(pObj->GetMergedItem( XATTR_LINEEND )).GetLineStartValue());
    XLineStartWidthItem  aLineStartWidth(static_cast<const XLineStartWidthItem&>(pObj->GetMergedItem( XATTR_LINEENDWIDTH )).GetValue());
    XLineStartCenterItem aLineStartCenter(static_cast<const XLineStartCenterItem&>(pObj->GetMergedItem( XATTR_LINEENDCENTER )).GetValue());

    XLineEndItem         aLineEnd;
    aLineEnd.SetLineEndValue(static_cast<const XLineEndItem&>(pObj->GetMergedItem( XATTR_LINESTART )).GetLineEndValue());
    XLineEndWidthItem    aLineEndWidth(static_cast<const XLineEndWidthItem&>(pObj->GetMergedItem( XATTR_LINESTARTWIDTH )).GetValue());
    XLineEndCenterItem   aLineEndCenter(static_cast<const XLineEndCenterItem&>(pObj->GetMergedItem( XATTR_LINESTARTCENTER )).GetValue());

    pObj->SetMergedItem( aLineStart );
    pObj->SetMergedItem( aLineStartWidth );
    pObj->SetMergedItem( aLineStartCenter );
    pObj->SetMergedItem( aLineEnd );
    pObj->SetMergedItem( aLineEndWidth );
    pObj->SetMergedItem( aLineEndCenter );
}

static basegfx::B2DPolygon CreateArc( const Rectangle& rRect, const Point& rStart, const Point& rEnd, const bool bClockwise, bool bFullCircle = false )
{
    Rectangle aRect( rRect );
    Point aStart( rStart );
    Point aEnd( rEnd );

    sal_Int32 bSwapStartEndAngle = 0;

    if ( aRect.Left() > aRect.Right() )
        bSwapStartEndAngle ^= 0x01;
    if ( aRect.Top() > aRect.Bottom() )
        bSwapStartEndAngle ^= 0x11;
    if ( bSwapStartEndAngle )
    {
        aRect.Justify();
        if ( bSwapStartEndAngle & 1 )
        {
            Point aTmp( aStart );
            aStart = aEnd;
            aEnd = aTmp;
        }
    }

    tools::Polygon aTempPoly( aRect, aStart, aEnd, POLY_ARC, bFullCircle );
    basegfx::B2DPolygon aRetval;

    if ( bClockwise )
    {
        for ( sal_uInt16 j = aTempPoly.GetSize(); j--; )
        {
            aRetval.append(basegfx::B2DPoint(aTempPoly[ j ].X(), aTempPoly[ j ].Y()));
        }
    }
    else
    {
        for ( sal_uInt16 j = 0; j < aTempPoly.GetSize(); j++ )
        {
            aRetval.append(basegfx::B2DPoint(aTempPoly[ j ].X(), aTempPoly[ j ].Y()));
        }
    }

    return aRetval;
}

void EnhancedCustomShape2d::CreateSubPath( sal_uInt16& rSrcPt, sal_uInt16& rSegmentInd, std::vector< SdrPathObj* >& rObjectList,
                                           const bool bLineGeometryNeededOnly,
                                           const bool bSortFilledObjectsToBack,
                                           sal_Int32 nIndex )
{
    bool bNoFill = false;
    bool bNoStroke = false;
    double dBrightness = 1.0;

    basegfx::B2DPolyPolygon aNewB2DPolyPolygon;
    basegfx::B2DPolygon aNewB2DPolygon;

    SetPathSize( nIndex );

    sal_Int32 nCoordSize = seqCoordinates.getLength();
    sal_Int32 nSegInfoSize = seqSegments.getLength();
    if ( !nSegInfoSize )
    {
        const EnhancedCustomShapeParameterPair* pTmp = seqCoordinates.getArray();

        for ( sal_Int32 nPtNum(0L); nPtNum < nCoordSize; nPtNum++ )
        {
            const Point aTempPoint(GetPoint( *pTmp++, true, true ));
            aNewB2DPolygon.append(basegfx::B2DPoint(aTempPoint.X(), aTempPoint.Y()));
        }

        aNewB2DPolygon.setClosed(true);
    }
    else
    {
        for ( ;rSegmentInd < nSegInfoSize; )
        {
            sal_Int16 nCommand = seqSegments[ rSegmentInd ].Command;
            sal_Int16 nPntCount= seqSegments[ rSegmentInd++ ].Count;

            switch ( nCommand )
            {
                case NOFILL :
                    bNoFill = true;
                break;
                case NOSTROKE :
                    bNoStroke = true;
                break;
                case DARKEN :
                    dBrightness = 0.66666666;
                    break;
                case DARKENLESS :
                    dBrightness = 0.83333333;
                    break;
                case LIGHTEN :
                    dBrightness = 1.16666666;
                    break;
                case LIGHTENLESS :
                    dBrightness = 1.33333333;
                    break;
                case MOVETO :
                {
                    if(aNewB2DPolygon.count() > 1L)
                    {
                        // #i76201# Add conversion to closed polygon when first and last points are equal
                        basegfx::tools::checkClosed(aNewB2DPolygon);
                        aNewB2DPolyPolygon.append(aNewB2DPolygon);
                    }

                    aNewB2DPolygon.clear();

                    if ( rSrcPt < nCoordSize )
                    {
                        const Point aTempPoint(GetPoint( seqCoordinates[ rSrcPt++ ], true, true ));
                        SAL_INFO(
                            "svx",
                            "moveTo: " << aTempPoint.X() << ","
                                << aTempPoint.Y());
                        aNewB2DPolygon.append(basegfx::B2DPoint(aTempPoint.X(), aTempPoint.Y()));
                    }
                }
                break;
                case ENDSUBPATH :
                break;
                case CLOSESUBPATH :
                {
                    if(aNewB2DPolygon.count())
                    {
                        if(aNewB2DPolygon.count() > 1L)
                        {
                            aNewB2DPolygon.setClosed(true);
                            aNewB2DPolyPolygon.append(aNewB2DPolygon);
                        }

                        aNewB2DPolygon.clear();
                    }
                }
                break;
                case CURVETO :
                {
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 2 ) < nCoordSize ); i++ )
                    {
                        const Point aControlA(GetPoint( seqCoordinates[ rSrcPt++ ], true, true ));
                        const Point aControlB(GetPoint( seqCoordinates[ rSrcPt++ ], true, true ));
                        const Point aEnd(GetPoint( seqCoordinates[ rSrcPt++ ], true, true ));

                        DBG_ASSERT(aNewB2DPolygon.count(), "EnhancedCustomShape2d::CreateSubPath: Error in adding control point (!)");
                        aNewB2DPolygon.appendBezierSegment(
                            basegfx::B2DPoint(aControlA.X(), aControlA.Y()),
                            basegfx::B2DPoint(aControlB.X(), aControlB.Y()),
                            basegfx::B2DPoint(aEnd.X(), aEnd.Y()));
                    }
                }
                break;

                case ANGLEELLIPSE :
                {
                    if ( nPntCount )
                    {
                        if(aNewB2DPolygon.count() > 1L)
                        {
                            // #i76201# Add conversion to closed polygon when first and last points are equal
                            basegfx::tools::checkClosed(aNewB2DPolygon);
                            aNewB2DPolyPolygon.append(aNewB2DPolygon);
                        }
                        aNewB2DPolygon.clear();
                    }
                }
                case ANGLEELLIPSETO :
                {
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 2 ) < nCoordSize ); i++ )
                    {
                        // create a circle
                        Point _aCenter;
                        double fWidth, fHeight;
                        const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( mso_sptEllipse  );
                        bool bIsDefaultViewBox = false;
                        bool bIsDefaultPath = false;
                        bool bIsMSEllipse = false;

                        if( ( nCoordWidth == pDefCustomShape->nCoordWidth )
                            && ( nCoordHeight == pDefCustomShape->nCoordHeight ) )
                            bIsDefaultViewBox = true;
                        sal_Int32 j, nCount = pDefCustomShape->nVertices;//==3
                        css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair> seqCoordinates1, seqCoordinates2;

                        seqCoordinates1.realloc( nCount );
                        for ( j = 0; j < nCount; j++ )
                        {
                            seqCoordinates1[j] = seqCoordinates[ rSrcPt + j];
                        }

                        seqCoordinates2.realloc( nCount );
                        for ( j = 0; j < nCount; j++ )
                        {
                            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates2[ j ].First, pDefCustomShape->pVertices[ j ].nValA );
                            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates2[ j ].Second, pDefCustomShape->pVertices[ j ].nValB );
                        }
                        if(seqCoordinates1 == seqCoordinates2)
                            bIsDefaultPath = true;

                        OUString sShpType;
                        SdrCustomShapeGeometryItem& rGeometryItem = const_cast<SdrCustomShapeGeometryItem&>(static_cast<const SdrCustomShapeGeometryItem&>(pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )));
                        Any* pAny = rGeometryItem.GetPropertyValueByName( "Type" );
                        if ( pAny )
                            *pAny >>= sShpType;
                        if( sShpType.getLength() > 3 &&
                            sShpType.startsWith( "mso" )){
                                bIsMSEllipse = true;
                        }
                        if( (! bIsDefaultPath   && ! bIsDefaultViewBox) || (bIsDefaultViewBox && bIsMSEllipse) /*&& (nGeneratorVersion == SfxObjectShell::Sym_L2)*/ )
                        {
                            _aCenter = GetPoint( seqCoordinates[ rSrcPt ], true, true );
                            GetParameter( fWidth,  seqCoordinates[ rSrcPt + 1 ].First, true, false );
                            GetParameter( fHeight,  seqCoordinates[ rSrcPt + 1 ].Second, false, true );
                            fWidth /= 2;
                            fHeight /= 2;
                        }else if( bIsDefaultPath && !bIsDefaultViewBox /*&& (nGeneratorVersion == SfxObjectShell::Sym_L2)*/ )
                        {
                            _aCenter.X() = nCoordWidth/2 * fXScale;
                            _aCenter.Y() = nCoordHeight/2 * fYScale;
                            fWidth = nCoordWidth/2;
                            fHeight = nCoordHeight/2;
                            const Any* pViewBox = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( "ViewBox" );
                            css::awt::Rectangle aViewBox;
                            if ( pViewBox && (*pViewBox >>= aViewBox ) )
                            {
                                aViewBox.Width = pDefCustomShape->nCoordWidth;
                                aViewBox.Height = pDefCustomShape->nCoordHeight;
                            }
                            css::beans::PropertyValue aPropVal;
                            aPropVal.Name = "ViewBox";
                            aPropVal.Value <<= aViewBox;
                            rGeometryItem.SetPropertyValue( aPropVal );
                            pCustomShapeObj->SetMergedItem( rGeometryItem );
                        }else{
                            _aCenter = GetPoint( seqCoordinates[ rSrcPt ], true, true );
                            GetParameter( fWidth,  seqCoordinates[ rSrcPt + 1 ].First, true, false);
                            GetParameter( fHeight,  seqCoordinates[ rSrcPt + 1 ].Second, false, true );
                        }

                        fWidth *= fXScale;
                        fHeight*= fYScale;
                        Point aP( (sal_Int32)( _aCenter.X() - fWidth ), (sal_Int32)( _aCenter.Y() - fHeight ) );
                        Size  aS( (sal_Int32)( fWidth * 2.0 ), (sal_Int32)( fHeight * 2.0 ) );
                        Rectangle aRect( aP, aS );
                        if ( aRect.GetWidth() && aRect.GetHeight() )
                        {
                            double fStartAngle, fEndAngle;
                            GetParameter( fStartAngle, seqCoordinates[ rSrcPt + 2 ].First,  false, false );
                            GetParameter( fEndAngle  , seqCoordinates[ rSrcPt + 2 ].Second, false, false );

                            if ( ((sal_Int32)fStartAngle % 360) != ((sal_Int32)fEndAngle % 360) )
                            {
                                if ( (sal_Int32)fStartAngle & 0x7fff0000 )  // SJ: if the angle was imported from our escher import, then the
                                    fStartAngle /= 65536.0;                 // value is shifted by 16. TODO: already change the fixed float to a
                                if ( (sal_Int32)fEndAngle & 0x7fff0000 )    // double in the import filter
                                {
                                    fEndAngle /= 65536.0;
                                    fEndAngle = fEndAngle + fStartAngle;
                                    if ( fEndAngle < 0 )
                                    {   // in the binary filter the endangle is the amount
                                        double fTemp = fStartAngle;
                                        fStartAngle = fEndAngle;
                                        fEndAngle = fTemp;
                                    }
                                }
                                double fCenterX = aRect.Center().X();
                                double fCenterY = aRect.Center().Y();
                                double fx1 = ( cos( fStartAngle * F_PI180 ) * 65536.0 * fXScale ) + fCenterX;
                                double fy1 = ( -sin( fStartAngle * F_PI180 ) * 65536.0 * fYScale ) + fCenterY;
                                double fx2 = ( cos( fEndAngle * F_PI180 ) * 65536.0 * fXScale ) + fCenterX;
                                double fy2 = ( -sin( fEndAngle * F_PI180 ) * 65536.0 * fYScale ) + fCenterY;
                                aNewB2DPolygon.append(CreateArc( aRect, Point( (sal_Int32)fx1, (sal_Int32)fy1 ), Point( (sal_Int32)fx2, (sal_Int32)fy2 ), false));
                            }
                            else
                            {   /* SJ: TODO: this block should be replaced sometimes, because the current point
                                   is not set correct, it also does not use the correct moveto
                                   point if ANGLEELLIPSETO was used, but the method CreateArc
                                   is at the moment not able to draw full circles (if startangle is 0
                                   and endangle 360 nothing is painted :-( */
                                sal_Int32 nXControl = (sal_Int32)((double)aRect.GetWidth() * 0.2835 );
                                sal_Int32 nYControl = (sal_Int32)((double)aRect.GetHeight() * 0.2835 );
                                Point aCenter( aRect.Center() );

                                // append start point
                                aNewB2DPolygon.append(basegfx::B2DPoint(aCenter.X(), aRect.Top()));

                                // append four bezier segments
                                aNewB2DPolygon.appendBezierSegment(
                                    basegfx::B2DPoint(aCenter.X() + nXControl, aRect.Top()),
                                    basegfx::B2DPoint(aRect.Right(), aCenter.Y() - nYControl),
                                    basegfx::B2DPoint(aRect.Right(), aCenter.Y()));

                                aNewB2DPolygon.appendBezierSegment(
                                    basegfx::B2DPoint(aRect.Right(), aCenter.Y() + nYControl),
                                    basegfx::B2DPoint(aCenter.X() + nXControl, aRect.Bottom()),
                                    basegfx::B2DPoint(aCenter.X(), aRect.Bottom()));

                                aNewB2DPolygon.appendBezierSegment(
                                    basegfx::B2DPoint(aCenter.X() - nXControl, aRect.Bottom()),
                                    basegfx::B2DPoint(aRect.Left(), aCenter.Y() + nYControl),
                                    basegfx::B2DPoint(aRect.Left(), aCenter.Y()));

                                aNewB2DPolygon.appendBezierSegment(
                                    basegfx::B2DPoint(aRect.Left(), aCenter.Y() - nYControl),
                                    basegfx::B2DPoint(aCenter.X() - nXControl, aRect.Top()),
                                    basegfx::B2DPoint(aCenter.X(), aRect.Top()));

                                // close, rescue last controlpoint, remove double last point
                                basegfx::tools::closeWithGeometryChange(aNewB2DPolygon);
                            }
                        }
                        rSrcPt += 3;
                    }
                }
                break;

                case QUADRATICCURVETO :
                {
                    for ( sal_Int32 i(0L); ( i < nPntCount ) && ( rSrcPt + 1 < nCoordSize ); i++ )
                    {
                        if ( rSrcPt )
                        {
                            const Point aPreviousEndPoint(GetPoint( seqCoordinates[ rSrcPt - 1 ], true, true));
                            const Point aControlQ(GetPoint( seqCoordinates[ rSrcPt++ ], true, true ));
                            const Point aEnd(GetPoint( seqCoordinates[ rSrcPt++ ], true, true ));
                            const Point aControlA((aPreviousEndPoint + (aControlQ * 2)) / 3);
                            const Point aControlB(((aControlQ * 2) + aEnd) / 3);

                            DBG_ASSERT(aNewB2DPolygon.count(), "EnhancedCustomShape2d::CreateSubPath: Error in adding Q control point (!)");
                            aNewB2DPolygon.appendBezierSegment(
                                basegfx::B2DPoint(aControlA.X(), aControlA.Y()),
                                basegfx::B2DPoint(aControlB.X(), aControlB.Y()),
                                basegfx::B2DPoint(aEnd.X(), aEnd.Y()));
                        }
                        else // no previous point , do a moveto
                        {
                            rSrcPt++; // skip control point
                            const Point aEnd(GetPoint( seqCoordinates[ rSrcPt++ ], true, true ));

                            DBG_ASSERT(aNewB2DPolygon.count(), "EnhancedCustomShape2d::CreateSubPath: Error in adding Q control point (!)");
                            aNewB2DPolygon.append(basegfx::B2DPoint(aEnd.X(), aEnd.Y()));
                        }
                    }
                }
                break;

                case LINETO :
                {
                    for ( sal_Int32 i(0L); ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                    {
                        const Point aTempPoint(GetPoint( seqCoordinates[ rSrcPt++ ], true, true ));
                        SAL_INFO(
                            "svx",
                            "lineTo: " << aTempPoint.X() << ","
                                << aTempPoint.Y());
                        aNewB2DPolygon.append(basegfx::B2DPoint(aTempPoint.X(), aTempPoint.Y()));
                    }
                }
                break;

                case ARC :
                case CLOCKWISEARC :
                {
                    if(aNewB2DPolygon.count() > 1L)
                    {
                        // #i76201# Add conversion to closed polygon when first and last points are equal
                        basegfx::tools::checkClosed(aNewB2DPolygon);
                        aNewB2DPolyPolygon.append(aNewB2DPolygon);
                    }

                    aNewB2DPolygon.clear();
                }
                case ARCTO :
                case CLOCKWISEARCTO :
                {
                    bool bClockwise = ( nCommand == CLOCKWISEARC ) || ( nCommand == CLOCKWISEARCTO );
                    sal_uInt32 nXor = bClockwise ? 3 : 2;
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 3 ) < nCoordSize ); i++ )
                    {
                        Rectangle aRect( GetPoint( seqCoordinates[ rSrcPt ], true, true ), GetPoint( seqCoordinates[ rSrcPt + 1 ], true, true ) );
                        if ( aRect.GetWidth() && aRect.GetHeight() )
                        {
                            Point aCenter( aRect.Center() );
                            Point aStart( GetPoint( seqCoordinates[ (sal_uInt16)( rSrcPt + nXor ) ], true, true ) );
                            Point aEnd( GetPoint( seqCoordinates[ (sal_uInt16)( rSrcPt + ( nXor ^ 1 ) ) ], true, true ) );
                            aStart.X() = (sal_Int32)( ( (double)( aStart.X() - aCenter.X() ) ) ) + aCenter.X();
                            aStart.Y() = (sal_Int32)( ( (double)( aStart.Y() - aCenter.Y() ) ) ) + aCenter.Y();
                            aEnd.X() = (sal_Int32)( ( (double)( aEnd.X() - aCenter.X() ) ) ) + aCenter.X();
                            aEnd.Y() = (sal_Int32)( ( (double)( aEnd.Y() - aCenter.Y() ) ) ) + aCenter.Y();
                            aNewB2DPolygon.append(CreateArc( aRect, aStart, aEnd, bClockwise));
                        }
                        rSrcPt += 4;
                    }
                }
                break;

                case ARCANGLETO :
                {
                    double fWR, fHR, fStartAngle, fSwingAngle;

                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( rSrcPt + 1 < nCoordSize ); i++ )
                    {
                        GetParameter ( fWR, seqCoordinates[ (sal_uInt16)( rSrcPt ) ].First, true, false );
                        GetParameter ( fHR, seqCoordinates[ (sal_uInt16)( rSrcPt ) ].Second, false, true );

                        GetParameter ( fStartAngle, seqCoordinates[ (sal_uInt16)( rSrcPt + 1) ].First, false, false );
                        GetParameter ( fSwingAngle, seqCoordinates[ (sal_uInt16)( rSrcPt + 1 ) ].Second, false, false );

                        // Convert angles to radians, but don't do any scaling / translation yet.

                        fStartAngle *= F_PI180;
                        fSwingAngle *= F_PI180;

                        OSL_TRACE("ARCANGLETO scale: %f x %f angles: %f, %f", fWR, fHR, fStartAngle, fSwingAngle);

                        bool bClockwise = fSwingAngle >= 0.0;

                        if (aNewB2DPolygon.count() > 0)
                        {
                            basegfx::B2DPoint aStartPointB2D( aNewB2DPolygon.getB2DPoint(aNewB2DPolygon.count() - 1 ) );
                            Point aStartPoint( 0, 0 );

                            double fT = atan2((fWR*sin(fStartAngle)), (fHR*cos(fStartAngle)));
                            double fTE = atan2((fWR*sin(fStartAngle + fSwingAngle)), fHR*cos(fStartAngle + fSwingAngle));

                            OSL_TRACE("ARCANGLETO angles: %f, %f --> parameters: %f, %f", fStartAngle, fSwingAngle, fT, fTE );

                            fWR *= fXScale;
                            fHR *= fYScale;

                            Rectangle aRect ( Point ( aStartPoint.getX() - fWR*cos(fT) - fWR, aStartPoint.getY() - fHR*sin(fT) - fHR ),
                                              Point ( aStartPoint.getX() - fWR*cos(fT) + fWR, aStartPoint.getY() - fHR*sin(fT) + fHR) );

                            Point aEndPoint ( aStartPoint.getX() - fWR*(cos(fT) - cos(fTE)), aStartPoint.getY() - fHR*(sin(fT) - sin(fTE)) );

                            SAL_INFO(
                                "svx",
                                "ARCANGLETO rect: " << aRect.Left() << ", "
                                    << aRect.Top() << "   x   " << aRect.Right()
                                    << ", " << aRect.Bottom() << "   start: "
                                    << aStartPoint.X() << ", "
                                    << aStartPoint.Y() << " end: "
                                    << aEndPoint.X() << ", " << aEndPoint.Y()
                                    << " clockwise: " << int(bClockwise));
                            basegfx::B2DPolygon aArc = CreateArc( aRect, bClockwise ? aEndPoint : aStartPoint, bClockwise ? aStartPoint : aEndPoint, bClockwise, aStartPoint == aEndPoint && fSwingAngle > F_PI);
                            // Now that we have the arc, move it to aStartPointB2D.
                            basegfx::B2DHomMatrix aMatrix = basegfx::tools::createTranslateB2DHomMatrix(aStartPointB2D.getX(), aStartPointB2D.getY());
                            aArc.transform(aMatrix);
                            aNewB2DPolygon.append(aArc);
                        }

                        rSrcPt += 2;
                    }
                }
                break;

                case ELLIPTICALQUADRANTX :
                case ELLIPTICALQUADRANTY :
                {
                    bool bFirstDirection(true);
                    basegfx::B2DPoint aControlPointA;
                    basegfx::B2DPoint aControlPointB;

                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                    {
                        sal_uInt32 nModT = ( nCommand == ELLIPTICALQUADRANTX ) ? 1 : 0;
                        Point aCurrent( GetPoint( seqCoordinates[ rSrcPt ], true, true ) );

                        if ( rSrcPt )   // we need a previous point
                        {
                            Point aPrev( GetPoint( seqCoordinates[ rSrcPt - 1 ], true, true ) );
                            sal_Int32 nX, nY;
                            nX = aCurrent.X() - aPrev.X();
                            nY = aCurrent.Y() - aPrev.Y();
                            if ( ( nY ^ nX ) & 0x80000000 )
                            {
                                if ( !i )
                                    bFirstDirection = true;
                                else if ( !bFirstDirection )
                                    nModT ^= 1;
                            }
                            else
                            {
                                if ( !i )
                                    bFirstDirection = false;
                                else if ( bFirstDirection )
                                    nModT ^= 1;
                            }
                            if ( nModT )            // get the right corner
                            {
                                nX = aCurrent.X();
                                nY = aPrev.Y();
                            }
                            else
                            {
                                nX = aPrev.X();
                                nY = aCurrent.Y();
                            }
                            sal_Int32 nXVec = ( nX - aPrev.X() ) >> 1;
                            sal_Int32 nYVec = ( nY - aPrev.Y() ) >> 1;
                            Point aControl1( aPrev.X() + nXVec, aPrev.Y() + nYVec );

                            aControlPointA = basegfx::B2DPoint(aControl1.X(), aControl1.Y());

                            nXVec = ( nX - aCurrent.X() ) >> 1;
                            nYVec = ( nY - aCurrent.Y() ) >> 1;
                            Point aControl2( aCurrent.X() + nXVec, aCurrent.Y() + nYVec );

                            aControlPointB = basegfx::B2DPoint(aControl2.X(), aControl2.Y());

                            aNewB2DPolygon.appendBezierSegment(
                                aControlPointA,
                                aControlPointB,
                                basegfx::B2DPoint(aCurrent.X(), aCurrent.Y()));
                        }
                        else
                        {
                            aNewB2DPolygon.append(basegfx::B2DPoint(aCurrent.X(), aCurrent.Y()));
                        }

                        rSrcPt++;
                    }
                }
                break;

#ifdef DBG_CUSTOMSHAPE
                case UNKNOWN :
                default :
                {
                    OStringBuffer aString("CustomShapes::unknown PolyFlagValue :");
                    aString.append(static_cast<sal_Int32>(nCommand));
                    OSL_FAIL(aString.getStr());
                }
                break;
#endif
            }
            if ( nCommand == ENDSUBPATH )
                break;
        }
    }
    if ( rSegmentInd == nSegInfoSize )
        rSegmentInd++;

    if(aNewB2DPolygon.count() > 1L)
    {
        // #i76201# Add conversion to closed polygon when first and last points are equal
        basegfx::tools::checkClosed(aNewB2DPolygon);
        aNewB2DPolyPolygon.append(aNewB2DPolygon);
    }

    if(aNewB2DPolyPolygon.count())
    {
        if( !bLineGeometryNeededOnly )
        {
            // hack aNewB2DPolyPolygon to fill logic rect - this is
            // needed to produce gradient fills that look like mso
            aNewB2DPolygon.clear();
            aNewB2DPolygon.append(basegfx::B2DPoint(0,0));
            aNewB2DPolygon.setClosed(true);
            aNewB2DPolyPolygon.append(aNewB2DPolygon);

            aNewB2DPolygon.clear();
            aNewB2DPolygon.append(basegfx::B2DPoint(aLogicRect.GetWidth(),
                                                    aLogicRect.GetHeight()));
            aNewB2DPolygon.setClosed(true);
            aNewB2DPolyPolygon.append(aNewB2DPolygon);
        }

        // #i37011#
        bool bForceCreateTwoObjects(false);

        if(!bSortFilledObjectsToBack && !aNewB2DPolyPolygon.isClosed() && !bNoStroke)
        {
            bForceCreateTwoObjects = true;
        }

        if(bLineGeometryNeededOnly)
        {
            bForceCreateTwoObjects = true;
            bNoFill = true;
            bNoStroke = false;
        }

        if(bForceCreateTwoObjects || bSortFilledObjectsToBack)
        {
            if(bFilled && !bNoFill)
            {
                basegfx::B2DPolyPolygon aClosedPolyPolygon(aNewB2DPolyPolygon);
                aClosedPolyPolygon.setClosed(true);
                SdrPathObj* pFill = new SdrPathObj(OBJ_POLY, aClosedPolyPolygon, dBrightness);
                SfxItemSet aTempSet(*this);
                aTempSet.Put(makeSdrShadowItem(false));
                aTempSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
                pFill->SetMergedItemSet(aTempSet);
                rObjectList.push_back(pFill);
            }

            if(!bNoStroke)
            {
                // there is no reason to use OBJ_PLIN here when the polygon is actually closed,
                // the non-fill is defined by XFILL_NONE. Since SdrPathObj::ImpForceKind() needs
                // to correct the polygon (here: open it) using the type, the last edge may get lost.
                // Thus, use a type that fits the polygon
                SdrPathObj* pStroke = new SdrPathObj(
                    aNewB2DPolyPolygon.isClosed() ? OBJ_POLY : OBJ_PLIN,
                    aNewB2DPolyPolygon, dBrightness);
                SfxItemSet aTempSet(*this);
                aTempSet.Put(makeSdrShadowItem(false));
                aTempSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
                pStroke->SetMergedItemSet(aTempSet);
                rObjectList.push_back(pStroke);
            }
        }
        else
        {
            SdrPathObj* pObj = nullptr;
            SfxItemSet aTempSet(*this);
            aTempSet.Put(makeSdrShadowItem(false));

            if(bNoFill)
            {
                // see comment above about OBJ_PLIN
                pObj = new SdrPathObj(
                    aNewB2DPolyPolygon.isClosed() ? OBJ_POLY : OBJ_PLIN,
                    aNewB2DPolyPolygon, dBrightness);
                aTempSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
            }
            else
            {
                aNewB2DPolyPolygon.setClosed(true);
                pObj = new SdrPathObj(OBJ_POLY, aNewB2DPolyPolygon, dBrightness);
            }

            if(bNoStroke)
            {
                aTempSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
            }

            if(pObj)
            {
                pObj->SetMergedItemSet(aTempSet);
                rObjectList.push_back(pObj);
            }
        }
    }
}

void CorrectCalloutArrows( MSO_SPT eSpType, sal_uInt32 nLineObjectCount, std::vector< SdrPathObj* >& vObjectList )
{
    bool bAccent = false;
    switch( eSpType )
    {
        case mso_sptCallout1 :
        case mso_sptBorderCallout1 :
        case mso_sptCallout90 :
        case mso_sptBorderCallout90 :
        default:
        break;

        case mso_sptAccentCallout1 :
        case mso_sptAccentBorderCallout1 :
        case mso_sptAccentCallout90 :
        case mso_sptAccentBorderCallout90 :
        {
            sal_uInt32 i, nLine = 0;
            for ( i = 0; i < vObjectList.size(); i++ )
            {
                SdrPathObj* pObj( vObjectList[ i ] );
                if(pObj->IsLine())
                {
                    nLine++;
                    if ( nLine == nLineObjectCount )
                    {
                        pObj->ClearMergedItem( XATTR_LINESTART );
                        pObj->ClearMergedItem( XATTR_LINEEND );
                    }
                }
            }
        }
        break;

        // switch start & end
        case mso_sptAccentCallout2 :
        case mso_sptAccentBorderCallout2 :
            bAccent = true;
            SAL_FALLTHROUGH;
        case mso_sptCallout2 :
        case mso_sptBorderCallout2 :
        {
            sal_uInt32 i, nLine = 0;
            for ( i = 0; i < vObjectList.size(); i++ )
            {
                SdrPathObj* pObj( vObjectList[ i ] );
                if(pObj->IsLine())
                {
                    nLine++;
                    if ( nLine == 1 )
                        pObj->ClearMergedItem( XATTR_LINEEND );
                    else if ( ( bAccent && ( nLine == nLineObjectCount - 1 ) ) || ( !bAccent && ( nLine == nLineObjectCount ) ) )
                        pObj->ClearMergedItem( XATTR_LINESTART );
                    else
                    {
                        pObj->ClearMergedItem( XATTR_LINESTART );
                        pObj->ClearMergedItem( XATTR_LINEEND );
                    }
                }
            }
        }
        break;

        case mso_sptAccentCallout3 :
        case mso_sptAccentBorderCallout3 :
        case mso_sptCallout3 :
        case mso_sptBorderCallout3 :
        {
            sal_uInt32 i, nLine = 0;
            for ( i = 0; i < vObjectList.size(); i++ )
            {
                SdrPathObj* pObj( vObjectList[ i ] );
                if(pObj->IsLine())
                {
                    if ( nLine )
                    {
                        pObj->ClearMergedItem( XATTR_LINESTART );
                        pObj->ClearMergedItem( XATTR_LINEEND );
                    }
                    else
                        EnhancedCustomShape2d::SwapStartAndEndArrow( pObj );

                    nLine++;
                }
            }
        }
        break;
    }
}

void EnhancedCustomShape2d::AdaptObjColor(SdrPathObj& rObj, const SfxItemSet& rCustomShapeSet,
                                          sal_uInt32& nColorIndex, sal_uInt32 nColorCount)
{
    if ( !rObj.IsLine() )
    {
        const drawing::FillStyle eFillStyle = static_cast<const XFillStyleItem&>(rObj.GetMergedItem(XATTR_FILLSTYLE)).GetValue();
        switch( eFillStyle )
        {
            default:
            case drawing::FillStyle_SOLID:
            {
                Color aFillColor;
                if ( nColorCount || rObj.GetBrightness() != 1.0 )
                {
                    aFillColor = GetColorData(
                        static_cast<const XFillColorItem&>(rCustomShapeSet.Get( XATTR_FILLCOLOR )).GetColorValue(),
                        std::min(nColorIndex, nColorCount-1), rObj.GetBrightness() );
                    rObj.SetMergedItem( XFillColorItem( "", aFillColor ) );
                }
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                XGradient aXGradient(static_cast<const XFillGradientItem&>(rObj.GetMergedItem(XATTR_FILLGRADIENT)).GetGradientValue());
                if ( nColorCount || rObj.GetBrightness() != 1.0 )
                {
                    aXGradient.SetStartColor(
                        GetColorData(
                            aXGradient.GetStartColor(),
                            std::min(nColorIndex, nColorCount-1), rObj.GetBrightness() ));
                    aXGradient.SetEndColor(
                        GetColorData(
                            aXGradient.GetEndColor(),
                            std::min(nColorIndex, nColorCount-1), rObj.GetBrightness() ));
                }

                rObj.SetMergedItem( XFillGradientItem( "", aXGradient ) );
                break;
            }
            case drawing::FillStyle_HATCH:
            {
                XHatch aXHatch(static_cast<const XFillHatchItem&>(rObj.GetMergedItem(XATTR_FILLHATCH)).GetHatchValue());
                if ( nColorCount || rObj.GetBrightness() != 1.0 )
                {
                    aXHatch.SetColor(
                        GetColorData(
                            aXHatch.GetColor(),
                            std::min(nColorIndex, nColorCount-1), rObj.GetBrightness() ));
                }

                rObj.SetMergedItem( XFillHatchItem( "", aXHatch ) );
                break;
            }
            case drawing::FillStyle_BITMAP:
            {
                if ( nColorCount || rObj.GetBrightness() != 1.0 )
                {
                    Bitmap aBitmap(static_cast<const XFillBitmapItem&>(rObj.GetMergedItem(XATTR_FILLBITMAP)).GetGraphicObject().GetGraphic().GetBitmapEx().GetBitmap());

                    aBitmap.Adjust(
                        static_cast< short > ( GetLuminanceChange(
                            std::min(nColorIndex, nColorCount-1))));

                    rObj.SetMergedItem(XFillBitmapItem(OUString(), Graphic(aBitmap)));
                }

                break;
            }
        }

        if ( nColorIndex < nColorCount )
            nColorIndex++;
    }
}

SdrObject* EnhancedCustomShape2d::CreatePathObj( bool bLineGeometryNeededOnly )
{
    sal_Int32 nCoordSize = seqCoordinates.getLength();
    if ( !nCoordSize )
        return nullptr;

    sal_uInt16 nSrcPt = 0;
    sal_uInt16 nSegmentInd = 0;

    std::vector< SdrPathObj* > vObjectList;
    bool bSortFilledObjectsToBack = SortFilledObjectsToBackByDefault( eSpType );

    sal_Int32 nSubPathIndex = 0;

    while( nSegmentInd <= seqSegments.getLength() )
    {
        CreateSubPath( nSrcPt, nSegmentInd, vObjectList, bLineGeometryNeededOnly, bSortFilledObjectsToBack, nSubPathIndex );
        nSubPathIndex ++;
    }

    SdrObject* pRet = nullptr;

    if ( !vObjectList.empty() )
    {
        const SfxItemSet& rCustomShapeSet = pCustomShapeObj->GetMergedItemSet();
        sal_uInt32      nColorCount = nColorData >> 28;
        sal_uInt32      nColorIndex = 0;

        // #i37011# remove invisible objects
        if(!vObjectList.empty())
        {
            std::vector< SdrPathObj* > vTempList;

            for(size_t i = 0; i < vObjectList.size(); ++i)
            {
                SdrPathObj* pObj(vObjectList[i]);
                const drawing::LineStyle eLineStyle =static_cast<const XLineStyleItem&>(pObj->GetMergedItem(XATTR_LINESTYLE)).GetValue();
                const drawing::FillStyle eFillStyle = static_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE)).GetValue();

                // #i40600# if bLineGeometryNeededOnly is set, linestyle does not matter
                if( !bLineGeometryNeededOnly && ( drawing::LineStyle_NONE == eLineStyle ) && ( drawing::FillStyle_NONE == eFillStyle ) )
                    delete pObj;
                else
                    vTempList.push_back(pObj);
            }

            vObjectList = vTempList;
        }

        if(1L == vObjectList.size())
        {
            // a single object, correct some values
            AdaptObjColor(*vObjectList[0L],rCustomShapeSet,nColorIndex,nColorCount);
        }
        else
        {
            sal_Int32 nLineObjectCount = 0;
            sal_Int32 nAreaObjectCount = 0;

            // correct some values and collect content data
            for ( size_t i = 0; i < vObjectList.size(); ++i )
            {
                SdrPathObj* pObj( vObjectList[ i ] );

                if(pObj->IsLine())
                {
                    nLineObjectCount++;
                }
                else
                {
                    nAreaObjectCount++;
                    AdaptObjColor(*pObj,rCustomShapeSet,nColorIndex,nColorCount);
                }
            }

            // #i88870# correct line arrows for callouts
            if ( nLineObjectCount )
                CorrectCalloutArrows( eSpType, nLineObjectCount, vObjectList );

            // sort objects so that filled ones are in front. Necessary
            // for some strange objects
            if ( bSortFilledObjectsToBack )
            {
                std::vector< SdrPathObj* > vTempList;
                vTempList.reserve(vObjectList.size());

                for ( size_t i = 0; i < vObjectList.size(); ++i )
                {
                    SdrPathObj* pObj( vObjectList[ i ] );

                    if ( !pObj->IsLine() )
                    {
                        vTempList.push_back(pObj);
                    }
                }

                for ( size_t i = 0; i < vObjectList.size(); ++i )
                {
                    SdrPathObj* pObj( vObjectList[ i ] );

                    if ( pObj->IsLine() )
                    {
                        vTempList.push_back(pObj);
                    }
                }

                vObjectList = vTempList;
            }
        }
    }

    // #i37011#
    if(!vObjectList.empty())
    {
        // copy remaining objects to pRet
        if(vObjectList.size() > 1)
        {
            pRet = new SdrObjGroup;

            for (size_t i = 0; i < vObjectList.size(); ++i)
            {
                SdrObject* pObj(vObjectList[i]);
                pRet->GetSubList()->NbcInsertObject(pObj);
            }
        }
        else if(1 == vObjectList.size())
        {
            pRet = vObjectList[0L];
        }

        if(pRet)
        {
            // move to target position
            Rectangle aCurRect(pRet->GetSnapRect());
            aCurRect.Move(aLogicRect.Left(), aLogicRect.Top());
            pRet->NbcSetSnapRect(aCurRect);
        }
    }

    return pRet;
}

SdrObject* EnhancedCustomShape2d::CreateObject( bool bLineGeometryNeededOnly )
{
    SdrObject* pRet = nullptr;

    if ( eSpType == mso_sptRectangle )
    {
        pRet = new SdrRectObj( aLogicRect );
        pRet->SetMergedItemSet( *this );
    }
    if ( !pRet )
        pRet = CreatePathObj( bLineGeometryNeededOnly );

    return pRet;
}

void EnhancedCustomShape2d::ApplyGluePoints( SdrObject* pObj )
{
    if ( pObj && seqGluePoints.getLength() )
    {
        sal_uInt32 i, nCount = seqGluePoints.getLength();
        for ( i = 0; i < nCount; i++ )
        {
            SdrGluePoint aGluePoint;

            aGluePoint.SetPos( GetPoint( seqGluePoints[ i ], true, true ) );
            aGluePoint.SetPercent( false );
            aGluePoint.SetAlign( SdrAlign::VERT_TOP | SdrAlign::HORZ_LEFT );
            aGluePoint.SetEscDir( SdrEscapeDirection::SMART );
            SdrGluePointList* pList = pObj->ForceGluePointList();
            if( pList )
                /* sal_uInt16 nId = */ pList->Insert( aGluePoint );
        }
    }
}

bool EnhancedCustomShape2d::IsPostRotate() const
{
    return dynamic_cast<const SdrObjCustomShape*>( pCustomShapeObj) != nullptr && static_cast<SdrObjCustomShape*>(pCustomShapeObj)->IsPostRotate();
}

SdrObject* EnhancedCustomShape2d::CreateLineGeometry()
{
    return CreateObject( true );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
