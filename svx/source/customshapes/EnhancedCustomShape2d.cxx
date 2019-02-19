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

#include <sal/config.h>

#include <svx/EnhancedCustomShape2d.hxx>
#include <svx/EnhancedCustomShapeGeometry.hxx>
#include <svx/EnhancedCustomShapeTypeNames.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpage.hxx>
#include <svx/xflclit.hxx>
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
#include <sal/log.hxx>

#include <algorithm>
#include <math.h>
#include <unordered_set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand;

void EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( EnhancedCustomShapeParameter& rParameter, const sal_Int32 nValue )
{
    sal_uInt32 nDat = static_cast<sal_uInt32>(nValue);
    sal_Int32  nNewValue = nValue;

    // check if this is a special point
    if ( ( nDat >> 16 ) == 0x8000 )
    {
        nNewValue = static_cast<sal_uInt16>(nDat);
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
        rParameter += OUString::number( nPara );
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

void EnhancedCustomShape2d::ApplyShapeAttributes( const SdrCustomShapeGeometryItem& rGeometryItem )
{
    // AdjustmentValues
    const Any* pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( "AdjustmentValues" );
    if ( pAny )
        *pAny >>= seqAdjustmentValues;


    // Coordsize
    const Any* pViewBox = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( "ViewBox" );
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
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( sPath, "Coordinates" );
    if ( pAny )
        *pAny >>= seqCoordinates;


    // Path/GluePoints
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( sPath, "GluePoints" );
    if ( pAny )
        *pAny >>= seqGluePoints;


    // Path/Segments
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( sPath, "Segments" );
    if ( pAny )
        *pAny >>= seqSegments;


    // Path/SubViewSize
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( sPath, "SubViewSize" );
    if ( pAny )
        *pAny >>= seqSubViewSize;


    // Path/StretchX
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( sPath, "StretchX" );
    if ( pAny )
    {
        sal_Int32 nStretchX = 0;
        if ( *pAny >>= nStretchX )
            nXRef = nStretchX;
    }


    // Path/StretchY
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( sPath, "StretchY" );
    if ( pAny )
    {
        sal_Int32 nStretchY = 0;
        if ( *pAny >>= nStretchY )
            nYRef = nStretchY;
    }


    // Path/TextFrames
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( sPath, "TextFrames" );
    if ( pAny )
        *pAny >>= seqTextFrames;


    // Equations
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( "Equations" );
    if ( pAny )
        *pAny >>= seqEquations;


    // Handles
    pAny = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( "Handles" );
    if ( pAny )
        *pAny >>= seqHandles;
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

    fXScale = nCoordWidth == 0 ? 0.0 : static_cast<double>(aLogicRect.GetWidth()) / static_cast<double>(nCoordWidth);
    fYScale = nCoordHeight == 0 ? 0.0 : static_cast<double>(aLogicRect.GetHeight()) / static_cast<double>(nCoordHeight);
    if ( bOOXMLShape )
    {
        SAL_INFO(
            "svx",
            "ooxml shape, path width: " << nCoordWidth << " height: "
                << nCoordHeight);

        // Try to set up scale separately, if given only width or height
        // This is possible case in OOXML when only width or height is non-zero
        if ( nCoordWidth == 0 )
        {
            if ( nWidth )
                fXScale = static_cast<double>(aLogicRect.GetWidth()) / static_cast<double>(nWidth);
            else
                fXScale = 1.0;
        }
        if ( nCoordHeight == 0 )
        {
            if ( nHeight )
                fYScale = static_cast<double>(aLogicRect.GetHeight()) / static_cast<double>(nHeight);
            else
                fYScale = 1.0;
        }
    }
    if ( static_cast<sal_uInt32>(nXRef) != 0x80000000 && aLogicRect.GetHeight() )
    {
        fXRatio = static_cast<double>(aLogicRect.GetWidth()) / static_cast<double>(aLogicRect.GetHeight());
        if ( fXRatio > 1 )
            fXScale /= fXRatio;
        else
            fXRatio = 1.0;
    }
    else
        fXRatio = 1.0;
    if ( static_cast<sal_uInt32>(nYRef) != 0x80000000 && aLogicRect.GetWidth() )
    {
        fYRatio = static_cast<double>(aLogicRect.GetHeight()) / static_cast<double>(aLogicRect.GetWidth());
        if ( fYRatio > 1 )
            fYScale /= fYRatio;
        else
            fYRatio = 1.0;
    }
    else
        fYRatio = 1.0;
}

EnhancedCustomShape2d::EnhancedCustomShape2d(SdrObjCustomShape& rSdrObjCustomShape)
:   SfxItemSet          ( rSdrObjCustomShape.GetMergedItemSet() ),
    mrSdrObjCustomShape ( rSdrObjCustomShape ),
    eSpType             ( mso_sptNil ),
    nCoordLeft          ( 0 ),
    nCoordTop           ( 0 ),
    nCoordWidthG        ( 21600 ),
    nCoordHeightG       ( 21600 ),
    bOOXMLShape         ( false ),
    nXRef               ( 0x80000000 ),
    nYRef               ( 0x80000000 ),
    nColorData          ( 0 ),
    bFilled             ( rSdrObjCustomShape.GetMergedItem( XATTR_FILLSTYLE ).GetValue() != drawing::FillStyle_NONE ),
    bStroked            ( rSdrObjCustomShape.GetMergedItem( XATTR_LINESTYLE ).GetValue() != drawing::LineStyle_NONE ),
    bFlipH              ( false ),
    bFlipV              ( false )
{
    // bTextFlow needs to be set before clearing the TextDirection Item

    ClearItem( SDRATTR_TEXTDIRECTION ); //SJ: vertical writing is not required, by removing this item no outliner is created

    // #i105323# For 2D AutoShapes, the shadow attribute does not need to be applied to any
    // of the constructed helper SdrObjects. This would lead to problems since the shadow
    // of one helper object would fall on one helper object behind it (e.g. with the
    // eyes of the smiley shape). This is not wanted; instead a single shadow 'behind'
    // the AutoShape visualisation is wanted. This is done with primitive functionality
    // now in SdrCustomShapePrimitive2D::create2DDecomposition, but only for 2D objects
    // (see there and in EnhancedCustomShape3d::Create3DObject to read more).
    // This exception may be removed later when AutoShapes will create primitives directly.
    // So, currently remove the ShadowAttribute from the ItemSet to not apply it to any
    // 2D helper shape.
    ClearItem(SDRATTR_SHADOW);

    Point aP( mrSdrObjCustomShape.GetSnapRect().Center() );
    Size aS( mrSdrObjCustomShape.GetLogicRect().GetSize() );
    aP.AdjustX( -(aS.Width() / 2) );
    aP.AdjustY( -(aS.Height() / 2) );
    aLogicRect = tools::Rectangle( aP, aS );

    OUString sShapeType;
    const SdrCustomShapeGeometryItem& rGeometryItem(mrSdrObjCustomShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
    const Any* pAny = rGeometryItem.GetPropertyValueByName( "Type" );
    if ( pAny ) {
        *pAny >>= sShapeType;
        bOOXMLShape = sShapeType.startsWith("ooxml-");
        SAL_INFO("svx", "shape type: " << sShapeType << " " << bOOXMLShape);
    }
    eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

    pAny = rGeometryItem.GetPropertyValueByName( "MirroredX" );
    if ( pAny )
        *pAny >>= bFlipH;
    pAny = rGeometryItem.GetPropertyValueByName( "MirroredY" );
    if ( pAny )
        *pAny >>= bFlipV;

    nRotateAngle = static_cast<sal_Int32>(mrSdrObjCustomShape.GetObjectRotation() * 100.0);

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

using EnhancedCustomShape::ExpressionFunct;

double EnhancedCustomShape2d::GetEnumFunc( const ExpressionFunct eFunc ) const
{
    double fRet = 0.0;
    switch( eFunc )
    {
        case ExpressionFunct::EnumPi :         fRet = F_PI; break;
        case ExpressionFunct::EnumLeft :       fRet = static_cast<double>(nCoordLeft); break;
        case ExpressionFunct::EnumTop :        fRet = static_cast<double>(nCoordTop); break;
        case ExpressionFunct::EnumRight :      fRet = static_cast<double>(nCoordLeft + nCoordWidth) * fXRatio; break;
        case ExpressionFunct::EnumBottom :     fRet = static_cast<double>(nCoordTop + nCoordHeight) * fYRatio; break;
        case ExpressionFunct::EnumXStretch :   fRet = nXRef; break;
        case ExpressionFunct::EnumYStretch :   fRet = nYRef; break;
        case ExpressionFunct::EnumHasStroke :  fRet = bStroked ? 1.0 : 0.0; break;
        case ExpressionFunct::EnumHasFill :    fRet = bFilled ? 1.0 : 0.0; break;
        case ExpressionFunct::EnumWidth :      fRet = nCoordWidth; break;
        case ExpressionFunct::EnumHeight :     fRet = nCoordHeight; break;
        case ExpressionFunct::EnumLogWidth :   fRet = aLogicRect.GetWidth(); break;
        case ExpressionFunct::EnumLogHeight :  fRet = aLogicRect.GetHeight(); break;
        default: break;
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
            fNumber = static_cast<double>(nNumber);
        }
    }
    return fNumber;
}
double EnhancedCustomShape2d::GetEquationValueAsDouble( const sal_Int32 nIndex ) const
{
    double fNumber = 0.0;
    static sal_uInt32 nLevel = 0;
    if ( nIndex < static_cast<sal_Int32>(vNodesSharedPtr.size()) )
    {
        if ( vNodesSharedPtr[ nIndex ].get() ) {
            nLevel ++;
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

                    SAL_INFO("svx", "equation " << nLevel << " (level: " << seqEquations[nIndex] << "): "
                             << fNumber << " --> " << 180.0*fNumber/10800000.0);
                }
                if ( !rtl::math::isFinite( fNumber ) )
                    fNumber = 0.0;
            }
            catch ( ... )
            {
                SAL_WARN("svx", "EnhancedCustomShape2d::GetEquationValueAsDouble failed");
            }
            nLevel --;
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

basegfx::B2DPoint EnhancedCustomShape2d::GetPointAsB2DPoint( const css::drawing::EnhancedCustomShapeParameterPair& rPair,
                                        const bool bScale, const bool bReplaceGeoSize ) const
{
    double fValX, fValY;
    // width
    GetParameter(fValX, rPair.First, bReplaceGeoSize, false);
    fValX -= nCoordLeft;
    if (bScale)
    {
        fValX *= fXScale;
    }
    // height
    GetParameter(fValY, rPair.Second, false, bReplaceGeoSize);
    fValY -= nCoordTop;
    if (bScale)
    {
        fValY *= fYScale;
    }
    return basegfx::B2DPoint(fValX,fValY);
}

Point EnhancedCustomShape2d::GetPoint( const css::drawing::EnhancedCustomShapeParameterPair& rPair,
                                        const bool bScale, const bool bReplaceGeoSize ) const
{
    basegfx::B2DPoint aPoint(GetPointAsB2DPoint(rPair, bScale, bReplaceGeoSize));
    return Point(static_cast<long>(aPoint.getX()), static_cast<long>(aPoint.getY()));
}

void EnhancedCustomShape2d::GetParameter( double& rRetValue, const EnhancedCustomShapeParameter& rParameter,
                                              const bool bReplaceGeoWidth, const bool bReplaceGeoHeight ) const
{
    rRetValue = 0.0;
    switch ( rParameter.Type )
    {
        case EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            sal_Int32 nAdjustmentIndex = 0;
            if ( rParameter.Value >>= nAdjustmentIndex )
            {
                rRetValue = GetAdjustValueAsDouble( nAdjustmentIndex );
            }
        }
        break;
        case EnhancedCustomShapeParameterType::EQUATION :
        {
            sal_Int32 nEquationIndex = 0;
            if ( rParameter.Value >>= nEquationIndex )
            {
                rRetValue = GetEquationValueAsDouble( nEquationIndex );
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
                }
            }
            else
            {
                sal_Int32 nValue = 0;
                if ( rParameter.Value >>= nValue )
                {
                    rRetValue = nValue;
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
        }
        break;
        case EnhancedCustomShapeParameterType::TOP :
        {
            rRetValue  = 0.0;
        }
        break;
        case EnhancedCustomShapeParameterType::RIGHT :
        {
            rRetValue = nCoordWidth;
        }
        break;
        case EnhancedCustomShapeParameterType::BOTTOM :
        {
            rRetValue = nCoordHeight;
        }
        break;
    }
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
    if ( bOOXMLShape || ( mso_sptMin == eSpType /* ODF "non-primitive" */ ) )
    { //do LibreOffice way, using dBrightness
        if ( dBrightness == 0.0)
        {
            return rFillColor;
        }
        else
        {
            if (dBrightness >=0.0)
            { //lighten, blending with white
                return Color( static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(rFillColor.GetRed() * (1.0-dBrightness) + dBrightness * 255.0, 0.0, 255.0)  )),
                              static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(rFillColor.GetGreen() * (1.0-dBrightness) + dBrightness * 255.0, 0.0, 255.0) )),
                              static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(rFillColor.GetBlue() * (1.0-dBrightness) + dBrightness * 255.0, 0.0, 255.0) ))  );
            }
            else
            { //darken (indicated by negative sign), blending with black
                return Color( static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(rFillColor.GetRed() * (1.0+dBrightness), 0.0, 255.0)  )),
                              static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(rFillColor.GetGreen() * (1.0+dBrightness), 0.0, 255.0) )),
                              static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(rFillColor.GetBlue() * (1.0+dBrightness), 0.0, 255.0) ))  );
            }
        }
    }
    else
    { //do OpenOffice way, using nColorData
        const sal_Int32 nLuminance = GetLuminanceChange(nIndex);
        if( !nLuminance )
            return rFillColor;

        basegfx::BColor aHSVColor=
                basegfx::utils::rgb2hsv(
                    basegfx::BColor(rFillColor.GetRed()/255.0,
                                    rFillColor.GetGreen()/255.0,
                                    rFillColor.GetBlue()/255.0));

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

        aHSVColor = basegfx::utils::hsv2rgb(aHSVColor);
        return Color( static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(aHSVColor.getRed(),0.0,1.0) * 255.0 + 0.5 )),
                    static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(aHSVColor.getGreen(),0.0,1.0) * 255.0 + 0.5 )),
                    static_cast<sal_uInt8>(static_cast< sal_Int32 >( std::clamp(aHSVColor.getBlue(),0.0,1.0) * 255.0 + 0.5 )) );
    }
}

tools::Rectangle EnhancedCustomShape2d::GetTextRect() const
{
    sal_Int32 nIndex, nSize = seqTextFrames.getLength();
    if ( !nSize )
        return aLogicRect;
    nIndex = 0;
    Point aTopLeft( GetPoint( seqTextFrames[ nIndex ].TopLeft, !bOOXMLShape, true ) );
    Point aBottomRight( GetPoint( seqTextFrames[ nIndex ].BottomRight, !bOOXMLShape, true ) );
    if ( bFlipH )
    {
        aTopLeft.setX( aLogicRect.GetWidth() - aTopLeft.X() );
        aBottomRight.setX( aLogicRect.GetWidth() - aBottomRight.X() );
    }
    if ( bFlipV )
    {
        aTopLeft.setY( aLogicRect.GetHeight() - aTopLeft.Y() );
        aBottomRight.setY( aLogicRect.GetHeight() - aBottomRight.Y() );
    }
    tools::Rectangle aRect( aTopLeft, aBottomRight );
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

                double a = basegfx::deg2rad(360.0 - fAngle);
                double dx = fRadius * fXScale;
                double fX = dx * cos( a );
                double fY =-dx * sin( a );
                rReturnPosition =
                    Point(
                        FRound( fX + aReferencePoint.X() ),
                        basegfx::fTools::equalZero(fXScale) ? aReferencePoint.Y() :
                        FRound( ( fY * fYScale ) / fXScale + aReferencePoint.Y() ) );
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
            const GeoStat aGeoStat(mrSdrObjCustomShape.GetGeoStat());
            if ( aGeoStat.nShearAngle )
            {
                double nTan = aGeoStat.nTan;
                if (bFlipV != bFlipH)
                    nTan = -nTan;
                ShearPoint( rReturnPosition, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), nTan );
            }
            if ( nRotateAngle )
            {
                double a = nRotateAngle * F_PI18000;
                RotatePoint( rReturnPosition, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), sin( a ), cos( a ) );
            }
            if ( bFlipH )
                rReturnPosition.setX( aLogicRect.GetWidth() - rReturnPosition.X() );
            if ( bFlipV )
                rReturnPosition.setY( aLogicRect.GetHeight() - rReturnPosition.Y() );
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
                aP.setX( aLogicRect.GetWidth() - aP.X() );
            if ( bFlipV )
                aP.setY( aLogicRect.GetHeight() - aP.Y() );
            if ( nRotateAngle )
            {
                double a = -nRotateAngle * F_PI18000;
                RotatePoint( aP, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), sin( a ), cos( a ) );
            }
            const GeoStat aGeoStat(mrSdrObjCustomShape.GetGeoStat());
            if ( aGeoStat.nShearAngle )
            {
                double nTan = -aGeoStat.nTan;
                if (bFlipV != bFlipH)
                    nTan = -nTan;
                ShearPoint( aP, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), nTan );
            }

            double fPos1 = aP.X();  //( bFlipH ) ? aLogicRect.GetWidth() - aP.X() : aP.X();
            double fPos2 = aP.Y();  //( bFlipV ) ? aLogicRect.GetHeight() -aP.Y() : aP.Y();
            fPos1 = !basegfx::fTools::equalZero(fXScale) ? (fPos1 / fXScale) : SAL_MAX_INT32;
            fPos2 = !basegfx::fTools::equalZero(fYScale) ? (fPos2 / fYScale) : SAL_MAX_INT32;

            // Used for scaling the adjustment values based on handle positions
            double fWidth;
            double fHeight;

            if ( nCoordWidth || nCoordHeight )
            {
                fWidth = nCoordWidth;
                fHeight = nCoordHeight;
            }
            else
            {
                fWidth = aLogicRect.GetWidth();
                fHeight = aLogicRect.GetHeight();
            }

            if ( aHandle.nFlags & HandleFlags::SWITCHED )
            {
                if ( aLogicRect.GetHeight() > aLogicRect.GetWidth() )
                {
                    double fX = fPos1;
                    double fY = fPos2;
                    double fTmp = fWidth;
                    fPos1 = fY;
                    fPos2 = fX;
                    fHeight = fWidth;
                    fWidth = fTmp;
                }
            }

            sal_Int32 nFirstAdjustmentValue = -1, nSecondAdjustmentValue = -1;

            if ( aHandle.aPosition.First.Type == EnhancedCustomShapeParameterType::ADJUSTMENT )
                aHandle.aPosition.First.Value >>= nFirstAdjustmentValue;
            if ( aHandle.aPosition.Second.Type == EnhancedCustomShapeParameterType::ADJUSTMENT )
                aHandle.aPosition.Second.Value>>= nSecondAdjustmentValue;


            // DrawingML polar handles set REFR or REFANGLE instead of POLAR
            if ( aHandle.nFlags & ( HandleFlags::POLAR | HandleFlags::REFR | HandleFlags::REFANGLE ) )
            {
                double fXRef, fYRef, fAngle;
                if ( aHandle.nFlags & HandleFlags::POLAR )
                {
                    GetParameter( fXRef, aHandle.aPolar.First, false, false );
                    GetParameter( fYRef, aHandle.aPolar.Second, false, false );
                }
                else
                {
                    // DrawingML polar handles don't have reference center.
                    fXRef = fWidth / 2;
                    fYRef = fHeight / 2;
                }
                const double fDX = fPos1 - fXRef;
                fAngle = -basegfx::rad2deg(atan2(-fPos2 + fYRef, (fDX == 0.0) ? 0.000000001 : fDX));
                double fX = fPos1 - fXRef;
                double fY = fPos2 - fYRef;
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
                if (aHandle.nFlags & HandleFlags::REFR)
                {
                    fRadius *= 100000.0;
                    fRadius /= sqrt( fWidth * fWidth + fHeight * fHeight );
                    nFirstAdjustmentValue = aHandle.nRefR;
                }
                if (aHandle.nFlags & HandleFlags::REFANGLE)
                {
                    if ( fAngle < 0 )
                        fAngle += 360.0;
                    // Adjustment value referred by nRefAngle needs to be in 60000th a degree
                    // from 0 to 21600000.
                    fAngle *= 60000.0;
                    nSecondAdjustmentValue = aHandle.nRefAngle;
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
                    fPos1 /= fWidth;
                }
                if ( aHandle.nFlags & HandleFlags::REFY )
                {
                    nSecondAdjustmentValue = aHandle.nRefY;
                    fPos2 *= 100000.0;
                    fPos2 /= fHeight;
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
            SdrCustomShapeGeometryItem aGeometryItem(mrSdrObjCustomShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
            css::beans::PropertyValue aPropVal;
            aPropVal.Name = "AdjustmentValues";
            aPropVal.Value <<= seqAdjustmentValues;
            aGeometryItem.SetPropertyValue( aPropVal );
            mrSdrObjCustomShape.SetMergedItem( aGeometryItem );
            bRetValue = true;
        }
    }
    return bRetValue;
}

void EnhancedCustomShape2d::SwapStartAndEndArrow( SdrObject* pObj ) //#108274
{
    XLineStartItem       aLineStart;
    aLineStart.SetLineStartValue(pObj->GetMergedItem( XATTR_LINEEND ).GetLineEndValue());
    XLineStartWidthItem  aLineStartWidth(pObj->GetMergedItem( XATTR_LINEENDWIDTH ).GetValue());
    XLineStartCenterItem aLineStartCenter(pObj->GetMergedItem( XATTR_LINEENDCENTER ).GetValue());

    XLineEndItem         aLineEnd;
    aLineEnd.SetLineEndValue(pObj->GetMergedItem( XATTR_LINESTART ).GetLineStartValue());
    XLineEndWidthItem    aLineEndWidth(pObj->GetMergedItem( XATTR_LINESTARTWIDTH ).GetValue());
    XLineEndCenterItem   aLineEndCenter(pObj->GetMergedItem( XATTR_LINESTARTCENTER ).GetValue());

    pObj->SetMergedItem( aLineStart );
    pObj->SetMergedItem( aLineStartWidth );
    pObj->SetMergedItem( aLineStartCenter );
    pObj->SetMergedItem( aLineEnd );
    pObj->SetMergedItem( aLineEndWidth );
    pObj->SetMergedItem( aLineEndCenter );
}

static basegfx::B2DPolygon CreateArc( const tools::Rectangle& rRect, const Point& rStart, const Point& rEnd, const bool bClockwise, bool bFullCircle = false )
{
    tools::Rectangle aRect( rRect );
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

    tools::Polygon aTempPoly( aRect, aStart, aEnd, PolyStyle::Arc, bFullCircle );
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

static double lcl_getNormalizedCircleAngleRad(const double fWR, const double fHR, const double fEllipseAngleDeg)
{
    double fRet(0.0);
    double fEAngleDeg(fmod(fEllipseAngleDeg, 360.0));
    if (fEAngleDeg < 0.0)
        fEAngleDeg += 360.0;
    const double fX(fHR * cos(basegfx::deg2rad(fEAngleDeg)));
    const double fY(fWR * sin(basegfx::deg2rad(fEAngleDeg)));
    if (fX != 0.0 || fY != 0.0)
    {
        fRet = atan2(fY, fX);
        if (fRet < 0.0)
            fRet += F_2PI;
    }
    return fRet;
}

static double lcl_getNormalizedAngleRad(const double fCircleAngleDeg)
{
    double fRet(fmod(fCircleAngleDeg, 360.0));
    if (fRet < 0.0)
        fRet += 360.0;
    return basegfx::deg2rad(fRet);
}

void EnhancedCustomShape2d::CreateSubPath(
    sal_Int32& rSrcPt,
    sal_Int32& rSegmentInd,
    std::vector< std::pair< SdrPathObj*, double> >& rObjectList,
    const bool bLineGeometryNeededOnly,
    const bool bSortFilledObjectsToBack,
    sal_Int32 nIndex)
{
    bool bNoFill = false;
    bool bNoStroke = false;
    double dBrightness = 0.0; //no blending

    basegfx::B2DPolyPolygon aNewB2DPolyPolygon;
    basegfx::B2DPolygon aNewB2DPolygon;

    SetPathSize( nIndex );

    sal_Int32 nCoordSize = seqCoordinates.getLength();
    sal_Int32 nSegInfoSize = seqSegments.getLength();
    if ( !nSegInfoSize )
    {
        const EnhancedCustomShapeParameterPair* pTmp = seqCoordinates.getArray();

        for ( sal_Int32 nPtNum(0); nPtNum < nCoordSize; nPtNum++ )
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
                    dBrightness = -0.4; //use sign to distinguish DARKEN from LIGHTEN
                    break;
                case DARKENLESS :
                    dBrightness = -0.2;
                    break;
                case LIGHTEN :
                    dBrightness = 0.4;
                    break;
                case LIGHTENLESS :
                    dBrightness = 0.2;
                    break;
                case MOVETO :
                {
                    if(aNewB2DPolygon.count() > 1)
                    {
                        // #i76201# Add conversion to closed polygon when first and last points are equal
                        basegfx::utils::checkClosed(aNewB2DPolygon);
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
                        if(aNewB2DPolygon.count() > 1)
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

                case ANGLEELLIPSE: // command U
                case ANGLEELLIPSETO: // command T
                {
                    // Some shapes will need special handling, decide on property 'Type'.
                    OUString sShpType;
                    SdrCustomShapeGeometryItem& rGeometryItem = const_cast<SdrCustomShapeGeometryItem&>(mrSdrObjCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
                    Any* pAny = rGeometryItem.GetPropertyValueByName("Type");
                    if (pAny)
                        *pAny >>= sShpType;
                    // User defined shapes in MS binary format, which contain command U or T after import
                    // in LibreOffice, starts with "mso".
                    const bool bIsFromBinaryImport(sShpType.startsWith("mso"));
                    // The only own or imported preset shapes with U command are those listed below.
                    // Command T is not used in preset shapes.
                    const std::unordered_set<OUString> aPresetShapesWithU =
                        { "ellipse",  "ring", "smiley", "sun", "forbidden", "flowchart-connector",
                          "flowchart-summing-junction", "flowchart-or", "cloud-callout"};
                    std::unordered_set<OUString>::const_iterator aIter = aPresetShapesWithU.find(sShpType);
                    const bool bIsPresetShapeWithU(aIter != aPresetShapesWithU.end());

                    for (sal_uInt16 i = 0; (i < nPntCount) && ((rSrcPt + 2) < nCoordSize); i++)
                    {
                        // ANGLEELLIPSE is the same as ANGLEELLIPSETO, only that it
                        // makes an implicit MOVETO. That ends the previous subpath.
                        if (ANGLEELLIPSE == nCommand)
                        {
                            if (aNewB2DPolygon.count() > 1)
                            {
                                // #i76201# Add conversion to closed polygon when first and last points are equal
                                basegfx::utils::checkClosed(aNewB2DPolygon);
                                aNewB2DPolyPolygon.append(aNewB2DPolygon);
                            }
                            aNewB2DPolygon.clear();
                        }

                        // Read all parameters, but do not finally handle them.
                        basegfx::B2DPoint aCenter(GetPointAsB2DPoint(seqCoordinates[ rSrcPt ], true, true));
                        double fWR; // horizontal ellipse radius
                        double fHR; // vertical ellipse radius
                        GetParameter(fWR, seqCoordinates[rSrcPt + 1].First, true, false);
                        GetParameter(fHR, seqCoordinates[rSrcPt + 1].Second, false, true);
                        double fStartAngle;
                        GetParameter(fStartAngle, seqCoordinates[rSrcPt + 2].First, false, false);
                        double fEndAngle;
                        GetParameter(fEndAngle, seqCoordinates[rSrcPt + 2].Second, false, false);
                        // Increasing here allows flat case differentiation tree by using 'continue'.
                        rSrcPt += 3;

                        double fScaledWR(fWR * fXScale);
                        double fScaledHR(fHR * fYScale);
                        if (fScaledWR == 0.0 && fScaledHR == 0.0)
                        {
                            // degenerated ellipse, add center point
                            aNewB2DPolygon.append(aCenter);
                            continue;
                        }

                        if (bIsFromBinaryImport)
                        {
                            // If a shape comes from MS binary ('escher') import, the angles are in degrees*2^16
                            // and the second angle is not an end angle, but a swing angle.
                            // MS Word shows this behavior: 0deg right, 90deg top, 180deg left and 270deg
                            // bottom. Third and forth parameter are horizontal and vertical radius, not width
                            // and height as noted in VML spec. A positive swing angle goes counter-clock
                            // wise (in user view). The swing angle might go several times around in case
                            // abs(swing angle) >= 360deg. Stroke accumulates, so that e.g. dash-dot might fill the
                            // gaps of previous turn. Fill does not accumulate but uses even-odd rule, semi-transparent
                            // fill does not become darker. The start and end points of the arc are calculated by
                            // using the angles on a circle and then scaling the circle to the ellipse. Caution, that
                            // is different from angle handling in ARCANGLETO and ODF.
                            // The following implementation generates such rendering. It is only for rendering legacy
                            // MS shapes and independent of the meaning of commands U and T in ODF specification.

                            // Convert from fixedfloat to double
                            double fSwingAngle;
                            fStartAngle /= 65536.0;
                            fSwingAngle = fEndAngle / 65536.0;
                            // Convert orientation
                            fStartAngle = -fStartAngle;
                            fSwingAngle = -fSwingAngle;

                            fEndAngle = fStartAngle + fSwingAngle;
                            if (fSwingAngle < 0.0)
                                std::swap(fStartAngle, fEndAngle);
                            double fFrom(fStartAngle);
                            double fTo(fFrom + 180.0);
                            basegfx::B2DPolygon aTempB2DPolygon;
                            double fS; // fFrom in radians in [0..2Pi[
                            double fE; // fTo or fEndAngle in radians in [0..2PI[
                            while (fTo < fEndAngle)
                            {
                                fS = lcl_getNormalizedAngleRad(fFrom);
                                fE = lcl_getNormalizedAngleRad(fTo);
                                aTempB2DPolygon.append(basegfx::utils::createPolygonFromEllipseSegment(aCenter, fScaledWR, fScaledHR, fS,fE));
                                fFrom = fTo;
                                fTo += 180.0;
                            }
                            fS = lcl_getNormalizedAngleRad(fFrom);
                            fE = lcl_getNormalizedAngleRad(fEndAngle);
                            aTempB2DPolygon.append(basegfx::utils::createPolygonFromEllipseSegment(aCenter, fScaledWR, fScaledHR,fS, fE));
                            if (fSwingAngle < 0)
                                aTempB2DPolygon.flip();
                            aNewB2DPolygon.append(aTempB2DPolygon);
                            continue;
                        }

                        // The not yet handled shapes are own preset shapes, or preset shapes from MS binary import, or user
                        // defined shapes, or foreign shapes. Shapes from OOXML import do not use ANGLEELLIPSE or
                        // ANGLEELLIPSETO, but use ARCANGLETO.
                        if (bIsPresetShapeWithU)
                        {
                            // Besides "cloud-callout" all preset shapes have angle values '0 360'.
                            // The imported "cloud-callout" has angle values '0 360' too, only our own "cloud-callout"
                            // has values '0 23592960'. But that is fixedfloat and means 360*2^16. Thus all these shapes
                            // have a full ellipse with start at 0deg.
                            aNewB2DPolygon.append(basegfx::utils::createPolygonFromEllipse(aCenter, fScaledWR, fScaledHR));
                            continue;
                        }

                        // In all other cases, full ODF conform handling is necessary. ODF rules:
                        // Third and forth parameter are horizontal and vertical radius.
                        // An angle determines the start or end point of the segment by intersection of the second angle
                        // leg with the ellipse. The first angle leg is always the positive x-axis. For the position
                        // of the intersection points the angle is used modulo 360deg in range [0deg..360deg[.
                        // The position of range [0deg..360deg[ is the same as in command ARCANGLETO, with 0deg right,
                        // 90deg bottom, 180deg left and 270deg top. Only if abs(end angle - start angle) == 360 deg,
                        // a full ellipse is drawn. The segment is always drawn clock wise (in user view) from start
                        // point to end point. The end point of the segment becomes the new "current" point.

                        if (fabs(fabs(fEndAngle - fStartAngle) - 360.0) < 1.0E-15)
                        {
                            // draw full ellipse
                            // Because createPolygonFromEllipseSegment cannot create full ellipse and
                            // createPolygonFromEllipse has no variing starts, we use two half ellipses.
                            const double fS(lcl_getNormalizedCircleAngleRad(fWR, fHR, fStartAngle));
                            const double fH(lcl_getNormalizedCircleAngleRad(fWR, fHR, fStartAngle + 180.0));
                            const double fE(lcl_getNormalizedCircleAngleRad(fWR, fHR, fEndAngle));
                            aNewB2DPolygon.append(basegfx::utils::createPolygonFromEllipseSegment(aCenter, fScaledWR, fScaledHR, fS, fH));
                            aNewB2DPolygon.append(basegfx::utils::createPolygonFromEllipseSegment(aCenter, fScaledWR, fScaledHR, fH, fE));
                            continue;
                        }

                        // remaining cases with central segment angle < 360
                        double fS(lcl_getNormalizedCircleAngleRad(fWR, fHR, fStartAngle));
                        double fE(lcl_getNormalizedCircleAngleRad(fWR, fHR, fEndAngle));
                        aNewB2DPolygon.append(basegfx::utils::createPolygonFromEllipseSegment(aCenter, fScaledWR, fScaledHR, fS, fE));
                    } // end for
                } // end case
                break;

                case QUADRATICCURVETO :
                {
                    for ( sal_Int32 i(0); ( i < nPntCount ) && ( rSrcPt + 1 < nCoordSize ); i++ )
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
                    for ( sal_Int32 i(0); ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
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
                    if(aNewB2DPolygon.count() > 1)
                    {
                        // #i76201# Add conversion to closed polygon when first and last points are equal
                        basegfx::utils::checkClosed(aNewB2DPolygon);
                        aNewB2DPolyPolygon.append(aNewB2DPolygon);
                    }

                    aNewB2DPolygon.clear();

                    [[fallthrough]];
                }
                case ARCTO :
                case CLOCKWISEARCTO :
                {
                    bool bClockwise = ( nCommand == CLOCKWISEARC ) || ( nCommand == CLOCKWISEARCTO );
                    sal_uInt32 nXor = bClockwise ? 3 : 2;
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 3 ) < nCoordSize ); i++ )
                    {
                        tools::Rectangle aRect( GetPoint( seqCoordinates[ rSrcPt ], true, true ), GetPoint( seqCoordinates[ rSrcPt + 1 ], true, true ) );
                        if ( aRect.GetWidth() && aRect.GetHeight() )
                        {
                            Point aStart( GetPoint( seqCoordinates[ static_cast<sal_uInt16>( rSrcPt + nXor ) ], true, true ) );
                            Point aEnd( GetPoint( seqCoordinates[ static_cast<sal_uInt16>( rSrcPt + ( nXor ^ 1 ) ) ], true, true ) );
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
                        GetParameter ( fWR, seqCoordinates[ static_cast<sal_uInt16>(rSrcPt) ].First, true, false );
                        GetParameter ( fHR, seqCoordinates[ static_cast<sal_uInt16>(rSrcPt) ].Second, false, true );

                        GetParameter ( fStartAngle, seqCoordinates[ static_cast<sal_uInt16>( rSrcPt + 1) ].First, false, false );
                        GetParameter ( fSwingAngle, seqCoordinates[ static_cast<sal_uInt16>( rSrcPt + 1 ) ].Second, false, false );

                        // Convert angles to radians, but don't do any scaling / translation yet.

                        fStartAngle = basegfx::deg2rad(fStartAngle);
                        fSwingAngle = basegfx::deg2rad(fSwingAngle);

                        SAL_INFO("svx", "ARCANGLETO scale: " << fWR << "x" << fHR << " angles: " << fStartAngle << "," << fSwingAngle);

                        bool bClockwise = fSwingAngle >= 0.0;

                        if (aNewB2DPolygon.count() > 0)
                        {
                            basegfx::B2DPoint aStartPointB2D( aNewB2DPolygon.getB2DPoint(aNewB2DPolygon.count() - 1 ) );
                            Point aStartPoint( 0, 0 );

                            double fT = atan2((fWR*sin(fStartAngle)), (fHR*cos(fStartAngle)));
                            double fTE = atan2((fWR*sin(fStartAngle + fSwingAngle)), fHR*cos(fStartAngle + fSwingAngle));

                            SAL_INFO("svx", "ARCANGLETO angles: " << fStartAngle << ", " << fSwingAngle
                                             << " --> parameters: " << fT <<", " << fTE );

                            fWR *= fXScale;
                            fHR *= fYScale;

                            tools::Rectangle aRect ( Point ( aStartPoint.getX() - fWR*cos(fT) - fWR, aStartPoint.getY() - fHR*sin(fT) - fHR ),
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
                            basegfx::B2DPolygon aArc = CreateArc( aRect, bClockwise ? aEndPoint : aStartPoint, bClockwise ? aStartPoint : aEndPoint, bClockwise, aStartPoint == aEndPoint && ((bClockwise && fSwingAngle > F_PI) || (!bClockwise && fSwingAngle < -F_PI)));
                            // Now that we have the arc, move it to aStartPointB2D.
                            basegfx::B2DHomMatrix aMatrix = basegfx::utils::createTranslateB2DHomMatrix(aStartPointB2D.getX(), aStartPointB2D.getY());
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
                    if (nPntCount && (rSrcPt < nCoordSize))
                    {
                        // The arc starts at the previous point and ends at the point given in the parameter.
                        basegfx::B2DPoint aStart;
                        basegfx::B2DPoint aEnd;
                        sal_uInt16 i = 0;
                        if (rSrcPt)
                        {
                            aStart = GetPointAsB2DPoint(seqCoordinates[rSrcPt - 1], true, true);
                        }
                        else
                        {   // no previous point, path is ill-structured. But we want to show as much as possible.
                            // Thus make a moveTo to the point given as parameter and continue from there.
                            aStart = GetPointAsB2DPoint(seqCoordinates[static_cast<sal_uInt16>(rSrcPt)], true, true);
                            aNewB2DPolygon.append(aStart);
                            rSrcPt++;
                            i++;
                        }
                        // If there are several points, then the direction changes with every point.
                        bool bIsXDirection(nCommand == ELLIPTICALQUADRANTX);
                        basegfx::B2DPolygon aArc;
                        for ( ; ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                        {
                            aEnd = GetPointAsB2DPoint(seqCoordinates[rSrcPt], true, true);
                            basegfx::B2DPoint aCenter;
                            double fRadiusX = fabs(aEnd.getX() - aStart.getX());
                            double fRadiusY = fabs(aEnd.getY() - aStart.getY());
                            if (bIsXDirection)
                            {
                                aCenter = basegfx::B2DPoint(aStart.getX(),aEnd.getY());
                                if (aEnd.getX()<aStart.getX())
                                {
                                    if (aEnd.getY()<aStart.getY()) // left, up
                                    {
                                        aArc = basegfx::utils::createPolygonFromEllipseSegment(aCenter, fRadiusX, fRadiusY, F_PI2, F_PI);
                                    }
                                    else // left, down
                                    {
                                        aArc = basegfx::utils::createPolygonFromEllipseSegment(aCenter, fRadiusX, fRadiusY, F_PI, 1.5*F_PI);
                                        aArc.flip();
                                    }
                                }
                                else // aEnd.getX()>=aStart.getX()
                                {
                                    if (aEnd.getY()<aStart.getY()) // right, up
                                    {
                                        aArc = basegfx::utils::createPolygonFromEllipseSegment(aCenter, fRadiusX, fRadiusY, 0.0, F_PI2);
                                        aArc.flip();
                                    }
                                    else // right, down
                                    {
                                        aArc = basegfx::utils::createPolygonFromEllipseSegment(aCenter, fRadiusX, fRadiusY, 1.5*F_PI, F_2PI);
                                    }
                                }
                            }
                            else // y-direction
                            {
                                aCenter = basegfx::B2DPoint(aEnd.getX(),aStart.getY());
                                if (aEnd.getX()<aStart.getX())
                                {
                                    if (aEnd.getY()<aStart.getY()) // up, left
                                    {
                                        aArc = basegfx::utils::createPolygonFromEllipseSegment(aCenter, fRadiusX, fRadiusY, 1.5*F_PI, F_2PI);
                                        aArc.flip();
                                    }
                                    else // down, left
                                    {
                                        aArc = basegfx::utils::createPolygonFromEllipseSegment(aCenter, fRadiusX, fRadiusY, 0.0, F_PI2);
                                    }
                                }
                                else // aEnd.getX()>=aStart.getX()
                                {
                                    if (aEnd.getY()<aStart.getY()) // up, right
                                    {
                                        aArc = basegfx::utils::createPolygonFromEllipseSegment(aCenter, fRadiusX, fRadiusY, F_PI, 1.5*F_PI);
                                    }
                                    else // down, right
                                    {
                                        aArc = basegfx::utils::createPolygonFromEllipseSegment(aCenter, fRadiusX, fRadiusY, F_PI2, F_PI);
                                        aArc.flip();
                                    }
                                }
                            }
                            aNewB2DPolygon.append(aArc);
                            rSrcPt++;
                            bIsXDirection = !bIsXDirection;
                            aStart = aEnd;
                        }
                    }
                    // else error in path syntax, do nothing
                }
                break;

#ifdef DBG_CUSTOMSHAPE
                case UNKNOWN :
                default :
                {
                    SAL_WARN( "svx", "CustomShapes::unknown PolyFlagValue :" << nCommand );
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

    if(aNewB2DPolygon.count() > 1)
    {
        // #i76201# Add conversion to closed polygon when first and last points are equal
        basegfx::utils::checkClosed(aNewB2DPolygon);
        aNewB2DPolyPolygon.append(aNewB2DPolygon);
    }

    if(aNewB2DPolyPolygon.count())
    {
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
                SdrPathObj* pFill = new SdrPathObj(
                    mrSdrObjCustomShape.getSdrModelFromSdrObject(),
                    OBJ_POLY,
                    aClosedPolyPolygon);
                SfxItemSet aTempSet(*this);
                aTempSet.Put(makeSdrShadowItem(false));
                aTempSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
                pFill->SetMergedItemSet(aTempSet);
                rObjectList.push_back(std::pair< SdrPathObj*, double >(pFill, dBrightness));
            }

            if(!bNoStroke)
            {
                // there is no reason to use OBJ_PLIN here when the polygon is actually closed,
                // the non-fill is defined by XFILL_NONE. Since SdrPathObj::ImpForceKind() needs
                // to correct the polygon (here: open it) using the type, the last edge may get lost.
                // Thus, use a type that fits the polygon
                SdrPathObj* pStroke = new SdrPathObj(
                    mrSdrObjCustomShape.getSdrModelFromSdrObject(),
                    aNewB2DPolyPolygon.isClosed() ? OBJ_POLY : OBJ_PLIN,
                    aNewB2DPolyPolygon);
                SfxItemSet aTempSet(*this);
                aTempSet.Put(makeSdrShadowItem(false));
                aTempSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
                pStroke->SetMergedItemSet(aTempSet);
                rObjectList.push_back(std::pair< SdrPathObj*, double >(pStroke, dBrightness));
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
                    mrSdrObjCustomShape.getSdrModelFromSdrObject(),
                    aNewB2DPolyPolygon.isClosed() ? OBJ_POLY : OBJ_PLIN,
                    aNewB2DPolyPolygon);
                aTempSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
            }
            else
            {
                aNewB2DPolyPolygon.setClosed(true);
                pObj = new SdrPathObj(
                    mrSdrObjCustomShape.getSdrModelFromSdrObject(),
                    OBJ_POLY,
                    aNewB2DPolyPolygon);
            }

            if(bNoStroke)
            {
                aTempSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
            }

            pObj->SetMergedItemSet(aTempSet);
            rObjectList.push_back(std::pair< SdrPathObj*, double >(pObj, dBrightness));
        }
    }
}

static void CorrectCalloutArrows(
    MSO_SPT eSpType,
    sal_uInt32 nLineObjectCount,
    std::vector< std::pair< SdrPathObj*, double> >& vObjectList )
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
            sal_uInt32 nLine = 0;

            for ( std::pair< SdrPathObj*, double >& rCandidate : vObjectList )
            {
                SdrPathObj* pObj(rCandidate.first);

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
            [[fallthrough]];
        case mso_sptCallout2 :
        case mso_sptBorderCallout2 :
        {
            sal_uInt32 nLine = 0;

            for ( std::pair< SdrPathObj*, double >& rCandidate : vObjectList )
            {
                SdrPathObj* pObj(rCandidate.first);

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
            sal_uInt32 nLine = 0;

            for ( std::pair< SdrPathObj*, double >& rCandidate : vObjectList )
            {
                SdrPathObj* pObj(rCandidate.first);

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

void EnhancedCustomShape2d::AdaptObjColor(
    SdrPathObj& rObj,
    double dBrightness,
    const SfxItemSet& rCustomShapeSet,
    sal_uInt32& nColorIndex,
    sal_uInt32 nColorCount)
{
    if ( !rObj.IsLine() )
    {
        const drawing::FillStyle eFillStyle = rObj.GetMergedItem(XATTR_FILLSTYLE).GetValue();
        switch( eFillStyle )
        {
            default:
            case drawing::FillStyle_SOLID:
            {
                Color aFillColor;

                if ( nColorCount || 0.0 != dBrightness )
                {
                    aFillColor = GetColorData(
                        rCustomShapeSet.Get( XATTR_FILLCOLOR ).GetColorValue(),
                        std::min(nColorIndex, nColorCount-1),
                        dBrightness );
                    rObj.SetMergedItem( XFillColorItem( "", aFillColor ) );
                }
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                XGradient aXGradient(rObj.GetMergedItem(XATTR_FILLGRADIENT).GetGradientValue());

                if ( nColorCount || 0.0 != dBrightness )
                {
                    aXGradient.SetStartColor(
                        GetColorData(
                            aXGradient.GetStartColor(),
                            std::min(nColorIndex, nColorCount-1),
                            dBrightness ));
                    aXGradient.SetEndColor(
                        GetColorData(
                            aXGradient.GetEndColor(),
                            std::min(nColorIndex, nColorCount-1),
                            dBrightness ));
                }

                rObj.SetMergedItem( XFillGradientItem( "", aXGradient ) );
                break;
            }
            case drawing::FillStyle_HATCH:
            {
                XHatch aXHatch(rObj.GetMergedItem(XATTR_FILLHATCH).GetHatchValue());

                if ( nColorCount || 0.0 != dBrightness )
                {
                    aXHatch.SetColor(
                        GetColorData(
                            aXHatch.GetColor(),
                            std::min(nColorIndex, nColorCount-1),
                            dBrightness ));
                }

                rObj.SetMergedItem( XFillHatchItem( "", aXHatch ) );
                break;
            }
            case drawing::FillStyle_BITMAP:
            {
                if ( nColorCount || 0.0 != dBrightness )
                {
                    Bitmap aBitmap(rObj.GetMergedItem(XATTR_FILLBITMAP).GetGraphicObject().GetGraphic().GetBitmapEx().GetBitmap());

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
    const sal_Int32 nCoordSize(seqCoordinates.getLength());

    if ( !nCoordSize )
    {
        return nullptr;
    }

    std::vector< std::pair< SdrPathObj*, double > > vObjectList;
    const bool bSortFilledObjectsToBack(SortFilledObjectsToBackByDefault(eSpType));
    sal_Int32 nSubPathIndex(0);
    sal_Int32 nSrcPt(0);
    sal_Int32 nSegmentInd(0);
    SdrObject* pRet(nullptr);

    while( nSegmentInd <= seqSegments.getLength() )
    {
        CreateSubPath(
            nSrcPt,
            nSegmentInd,
            vObjectList,
            bLineGeometryNeededOnly,
            bSortFilledObjectsToBack,
            nSubPathIndex);
        nSubPathIndex++;
    }

    if ( !vObjectList.empty() )
    {
        const SfxItemSet& rCustomShapeSet(mrSdrObjCustomShape.GetMergedItemSet());
        const sal_uInt32 nColorCount(nColorData >> 28);
        sal_uInt32 nColorIndex(0);

        // #i37011# remove invisible objects
        std::vector< std::pair< SdrPathObj*, double> > vNewList;

        for ( std::pair< SdrPathObj*, double >& rCandidate : vObjectList )
        {
            SdrPathObj* pObj(rCandidate.first);
            const drawing::LineStyle eLineStyle(pObj->GetMergedItem(XATTR_LINESTYLE).GetValue());
            const drawing::FillStyle eFillStyle(pObj->GetMergedItem(XATTR_FILLSTYLE).GetValue());

            // #i40600# if bLineGeometryNeededOnly is set, linestyle does not matter
            if(!bLineGeometryNeededOnly && (drawing::LineStyle_NONE == eLineStyle) && (drawing::FillStyle_NONE == eFillStyle))
            {
                // always use SdrObject::Free(...) for SdrObjects (!)
                SdrObject* pTemp(pObj);
                SdrObject::Free(pTemp);
            }
            else
            {
                vNewList.push_back(rCandidate);
            }
        }

        vObjectList = vNewList;

        if(1 == vObjectList.size())
        {
            // a single object, correct some values
            AdaptObjColor(
                *vObjectList.begin()->first,
                vObjectList.begin()->second,
                rCustomShapeSet,
                nColorIndex,
                nColorCount);
        }
        else
        {
            sal_Int32 nLineObjectCount(0);

            // correct some values and collect content data
            for ( std::pair< SdrPathObj*, double >& rCandidate : vObjectList )
            {
                SdrPathObj* pObj(rCandidate.first);

                if(pObj->IsLine())
                {
                    nLineObjectCount++;
                }
                else
                {
                    AdaptObjColor(
                        *pObj,
                        rCandidate.second,
                        rCustomShapeSet,
                        nColorIndex,
                        nColorCount);

                    // OperationSmiley: when we have access to the SdrObjCustomShape and the
                    // CustomShape is built with more than a single filled Geometry, use it
                    // to define that all helper geometries defined here (SdrObjects currently)
                    // will use the same FillGeometryDefinition (from the referenced SdrObjCustomShape).
                    // This will all same-filled objects look like filled smoothly with the same style.
                    pObj->setFillGeometryDefiningShape(&mrSdrObjCustomShape);
                }
            }

            // #i88870# correct line arrows for callouts
            if ( nLineObjectCount )
            {
                CorrectCalloutArrows(
                    eSpType,
                    nLineObjectCount,
                    vObjectList);
            }

            // sort objects so that filled ones are in front. Necessary
            // for some strange objects
            if(bSortFilledObjectsToBack)
            {
                std::vector< std::pair< SdrPathObj*, double> > vTempList;
                vTempList.reserve(vObjectList.size());

                for ( std::pair< SdrPathObj*, double >& rCandidate : vObjectList )
                {
                    SdrPathObj* pObj(rCandidate.first);

                    if ( !pObj->IsLine() )
                    {
                        vTempList.push_back(rCandidate);
                    }
                }

                for ( std::pair< SdrPathObj*, double >& rCandidate : vObjectList )
                {
                    SdrPathObj* pObj(rCandidate.first);

                    if ( pObj->IsLine() )
                    {
                        vTempList.push_back(rCandidate);
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
            pRet = new SdrObjGroup(mrSdrObjCustomShape.getSdrModelFromSdrObject());

            for ( std::pair< SdrPathObj*, double >& rCandidate : vObjectList )
            {
                SdrPathObj* pObj(rCandidate.first);

                pRet->GetSubList()->NbcInsertObject(pObj);
            }
        }
        else if(1 == vObjectList.size())
        {
            pRet = vObjectList.begin()->first;
        }

        if(pRet)
        {
            // move to target position
            tools::Rectangle aCurRect(pRet->GetSnapRect());
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
        pRet = new SdrRectObj(mrSdrObjCustomShape.getSdrModelFromSdrObject(), aLogicRect);
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

SdrObject* EnhancedCustomShape2d::CreateLineGeometry()
{
    return CreateObject( true );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
