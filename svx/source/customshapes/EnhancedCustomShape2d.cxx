/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <svx/xbitmap.hxx>
#include <svx/xhatch.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <boost/shared_ptr.hpp>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

// #i76201#
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <rtl/strbuf.hxx>
#include <math.h>

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

rtl::OUString EnhancedCustomShape2d::GetEquation( const sal_uInt16 nFlags, sal_Int16 nP1, sal_Int16 nP2, sal_Int16 nP3 )
{
    rtl::OUString aEquation;
    sal_Bool b1Special = ( nFlags & 0x2000 ) != 0;
    sal_Bool b2Special = ( nFlags & 0x4000 ) != 0;
    sal_Bool b3Special = ( nFlags & 0x8000 ) != 0;
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
                    aEquation += rtl::OUString( (sal_Unicode)'+' );
                    EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
                }
                break;
            }
            if ( b3Special || nP3 )
            {
                aEquation += rtl::OUString( (sal_Unicode)'-' );
                EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            }
        }
        break;
        case 1 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            if ( b2Special || ( nP2 != 1 ) )
            {
                aEquation += rtl::OUString( (sal_Unicode)'*' );
                EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            }
            if ( b3Special || ( ( nP3 != 1 ) && ( nP3 != 0 ) ) )
            {
                aEquation += rtl::OUString( (sal_Unicode)'/' );
                EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            }
        }
        break;
        case 2 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "+" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")/2" ) );
        }
        break;
        case 3 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "abs(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")" ) );
        }
        break;
        case 4 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "min(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "," ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")" ) );
        }
        break;
        case 5 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "max(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "," ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")" ) );
        }
        break;
        case 6 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "if(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( (sal_Unicode)',' );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( (sal_Unicode)',' );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( (sal_Unicode)')' );
        }
        break;
        case 7 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "sqrt(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "+" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "+" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( (sal_Unicode)')' );
        }
        break;
        case 8 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "atan2(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "," ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")/(pi/180)" ) );
        }
        break;
        case 9 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*sin(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*(pi/180))" ) );
        }
        break;
        case 10 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*cos(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*(pi/180))" ) );
        }
        break;
        case 11 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*" ) );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "cos(atan2(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "," ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "))" ) );
        }
        break;
        case 12 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*" ) );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "sin(atan2(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "," ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "))" ) );
        }
        break;
        case 13 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "sqrt(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")" ) );
        }
        break;
        case 15 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*sqrt(1-(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")" ) );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "))" ) );
        }
        break;
        case 16 :
        {
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*tan(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")" ) );
        }
        break;
        case 0x80 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "sqrt(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "-" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( (sal_Unicode)')' );
        }
        break;
        case 0x81 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "(cos(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*(pi/180))*(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "-10800)+sin(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*(pi/180))*(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "-10800))+10800" ) );
        }
        break;
        case 0x82 :
        {
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "-(sin(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*(pi/180))*(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP1, b1Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "-10800)-cos(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP3, b3Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "*(pi/180))*(" ) );
            EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( aEquation, nP2, b2Special );
            aEquation += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "-10800))+10800" ) );
        }
        break;
    }
    return aEquation;
}

void EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( rtl::OUString& rParameter, const sal_Int16 nPara, const sal_Bool bIsSpecialValue )
{
    if ( bIsSpecialValue )
    {
        if ( nPara & 0x400 )
        {
            rParameter += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "?" ) );
            rParameter += rtl::OUString::valueOf( (sal_Int32)( nPara & 0xff ) );
            rParameter += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( " " ) );
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
                    rParameter += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "$" ) );
                    rParameter += rtl::OUString::valueOf( (sal_Int32)( nPara - DFF_Prop_adjustValue ) );
                    rParameter += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( " " ) );
                }
                break;
                case DFF_Prop_geoLeft :
                {
                    rParameter += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "left" ) );
                }
                break;
                case DFF_Prop_geoTop :
                {
                    rParameter += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "top" ) );
                }
                break;
                case DFF_Prop_geoRight :
                {
                    rParameter += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "right" ) );
                }
                break;
                case DFF_Prop_geoBottom :
                {
                    rParameter += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "bottom" ) );
                }
                break;
            }
        }
    }
    else
    {
        rParameter += rtl::OUString::valueOf( (sal_Int32)( nPara ) );
    }
}

void EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( EnhancedCustomShapeParameter& rParameter, const sal_Int32 nPara, const sal_Bool bIsSpecialValue, sal_Bool bHorz )
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

sal_Bool EnhancedCustomShape2d::ConvertSequenceToEnhancedCustomShape2dHandle(
    const com::sun::star::beans::PropertyValues& rHandleProperties,
        EnhancedCustomShape2d::Handle& rDestinationHandle )
{
    sal_Bool bRetValue = sal_False;
    sal_uInt32 i, nProperties = rHandleProperties.getLength();
    if ( nProperties )
    {
        rDestinationHandle.nFlags = 0;
        for ( i = 0; i < nProperties; i++ )
        {
            const com::sun::star::beans::PropertyValue& rPropVal = rHandleProperties[ i ];

            const rtl::OUString sPosition           ( RTL_CONSTASCII_USTRINGPARAM( "Position" ) );
            const rtl::OUString sMirroredX          ( RTL_CONSTASCII_USTRINGPARAM( "MirroredX" ) );
            const rtl::OUString sMirroredY          ( RTL_CONSTASCII_USTRINGPARAM( "MirroredY" ) );
            const rtl::OUString sSwitched           ( RTL_CONSTASCII_USTRINGPARAM( "Switched" ) );
            const rtl::OUString sPolar              ( RTL_CONSTASCII_USTRINGPARAM( "Polar" ) );
            const rtl::OUString sRefX               ( RTL_CONSTASCII_USTRINGPARAM( "RefX" ) );
            const rtl::OUString sRefY               ( RTL_CONSTASCII_USTRINGPARAM( "RefY" ) );
            const rtl::OUString sRefAngle           ( RTL_CONSTASCII_USTRINGPARAM( "RefAngle" ) );
            const rtl::OUString sRefR               ( RTL_CONSTASCII_USTRINGPARAM( "RefR" ) );
            const rtl::OUString sRadiusRangeMinimum ( RTL_CONSTASCII_USTRINGPARAM( "RadiusRangeMinimum" ) );
            const rtl::OUString sRadiusRangeMaximum ( RTL_CONSTASCII_USTRINGPARAM( "RadiusRangeMaximum" ) );
            const rtl::OUString sRangeXMinimum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeXMinimum" ) );
            const rtl::OUString sRangeXMaximum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeXMaximum" ) );
            const rtl::OUString sRangeYMinimum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeYMinimum" ) );
            const rtl::OUString sRangeYMaximum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeYMaximum" ) );

            if ( rPropVal.Name.equals( sPosition ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aPosition )
                    bRetValue = sal_True;
            }
            else if ( rPropVal.Name.equals( sMirroredX ) )
            {
                sal_Bool bMirroredX = sal_Bool();
                if ( rPropVal.Value >>= bMirroredX )
                {
                    if ( bMirroredX )
                        rDestinationHandle.nFlags |= HANDLE_FLAGS_MIRRORED_X;
                }
            }
            else if ( rPropVal.Name.equals( sMirroredY ) )
            {
                sal_Bool bMirroredY = sal_Bool();
                if ( rPropVal.Value >>= bMirroredY )
                {
                    if ( bMirroredY )
                        rDestinationHandle.nFlags |= HANDLE_FLAGS_MIRRORED_Y;
                }
            }
            else if ( rPropVal.Name.equals( sSwitched ) )
            {
                sal_Bool bSwitched = sal_Bool();
                if ( rPropVal.Value >>= bSwitched )
                {
                    if ( bSwitched )
                        rDestinationHandle.nFlags |= HANDLE_FLAGS_SWITCHED;
                }
            }
            else if ( rPropVal.Name.equals( sPolar ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aPolar )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_POLAR;
            }
            else if ( rPropVal.Name.equals( sRefX ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.nRefX )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_REFX;
            }
            else if ( rPropVal.Name.equals( sRefY ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.nRefY )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_REFY;
            }
            else if ( rPropVal.Name.equals( sRefAngle ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.nRefAngle )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_REFANGLE;
            }
            else if ( rPropVal.Name.equals( sRefR ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.nRefR )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_REFR;
            }
            else if ( rPropVal.Name.equals( sRadiusRangeMinimum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aRadiusRangeMinimum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RADIUS_RANGE_MINIMUM;
            }
            else if ( rPropVal.Name.equals( sRadiusRangeMaximum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aRadiusRangeMaximum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RADIUS_RANGE_MAXIMUM;
            }
            else if ( rPropVal.Name.equals( sRangeXMinimum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aXRangeMinimum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RANGE_X_MINIMUM;
            }
            else if ( rPropVal.Name.equals( sRangeXMaximum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aXRangeMaximum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RANGE_X_MAXIMUM;
            }
            else if ( rPropVal.Name.equals( sRangeYMinimum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aYRangeMinimum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RANGE_Y_MINIMUM;
            }
            else if ( rPropVal.Name.equals( sRangeYMaximum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aYRangeMaximum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RANGE_Y_MAXIMUM;
            }
        }
    }
    return bRetValue;
}

const sal_Int32* EnhancedCustomShape2d::ApplyShapeAttributes( const SdrCustomShapeGeometryItem& rGeometryItem )
{
    const sal_Int32* pDefData = NULL;
    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
    if ( pDefCustomShape )
        pDefData = pDefCustomShape->pDefData;

    //////////////////////
    // AdjustmentValues //
    //////////////////////
    const rtl::OUString sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
    const Any* pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sAdjustmentValues );
    if ( pAny )
        *pAny >>= seqAdjustmentValues;

    ///////////////
    // Coordsize //
    ///////////////
    const rtl::OUString sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
    const Any* pViewBox = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sViewBox );
    com::sun::star::awt::Rectangle aViewBox;
    if ( pViewBox && (*pViewBox >>= aViewBox ) )
    {
        nCoordLeft    = aViewBox.X;
        nCoordTop     = aViewBox.Y;
        nCoordWidthG  = labs( aViewBox.Width );
        nCoordHeightG = labs( aViewBox.Height);
    }
    const rtl::OUString sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );

    //////////////////////
    // Path/Coordinates //
    //////////////////////
    const rtl::OUString sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
    if ( pAny )
        *pAny >>= seqCoordinates;

    /////////////////////
    // Path/GluePoints //
    /////////////////////
    const rtl::OUString sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sGluePoints );
    if ( pAny )
        *pAny >>= seqGluePoints;

    ///////////////////
    // Path/Segments //
    ///////////////////
    const rtl::OUString sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sSegments );
    if ( pAny )
        *pAny >>= seqSegments;

    //////////////////////
    // Path/SubViewSize //
    //////////////////////
    const rtl::OUString sSubViewSize( RTL_CONSTASCII_USTRINGPARAM ( "SubViewSize" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sSubViewSize );
    if ( pAny )
        *pAny >>= seqSubViewSize;

    ///////////////////
    // Path/StretchX //
    ///////////////////
    const rtl::OUString sStretchX( RTL_CONSTASCII_USTRINGPARAM ( "StretchX" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sStretchX );
    if ( pAny )
    {
        sal_Int32 nStretchX = 0;
        if ( *pAny >>= nStretchX )
            nXRef = nStretchX;
    }

    ///////////////////
    // Path/StretchY //
    ///////////////////
    const rtl::OUString sStretchY( RTL_CONSTASCII_USTRINGPARAM ( "StretchY" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sStretchY );
    if ( pAny )
    {
        sal_Int32 nStretchY = 0;
        if ( *pAny >>= nStretchY )
            nYRef = nStretchY;
    }

    /////////////////////
    // Path/TextFrames //
    /////////////////////
    const rtl::OUString sTextFrames( RTL_CONSTASCII_USTRINGPARAM ( "TextFrames" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sTextFrames );
    if ( pAny )
        *pAny >>= seqTextFrames;

    ///////////////
    // Equations //
    ///////////////
    const rtl::OUString sEquations( RTL_CONSTASCII_USTRINGPARAM( "Equations" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sEquations );
    if ( pAny )
        *pAny >>= seqEquations;

    /////////////
    // Handles //
    /////////////
    const rtl::OUString sHandles( RTL_CONSTASCII_USTRINGPARAM( "Handles" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sHandles );
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
        OSL_TRACE("set subpath %d size: %d x %d", nIndex, nWidth, nHeight);
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
        OSL_TRACE("ooxml shape, path width: %d height: %d", nCoordWidth, nCoordHeight);
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
    bOOXMLShape         ( sal_False ),
    nXRef               ( 0x80000000 ),
    nYRef               ( 0x80000000 ),
    nFlags              ( 0 ),
    nColorData          ( 0 ),
    bTextFlow           ( sal_False ),
    bFilled             ( ((const XFillStyleItem&)pAObj->GetMergedItem( XATTR_FILLSTYLE )).GetValue() != XFILL_NONE ),
    bStroked            ( ((const XLineStyleItem&)pAObj->GetMergedItem( XATTR_LINESTYLE )).GetValue() != XLINE_NONE ),
    bFlipH              ( sal_False ),
    bFlipV              ( sal_False )
{
    // bTextFlow needs to be set before clearing the TextDirection Item

    ClearItem( SDRATTR_TEXTDIRECTION ); //SJ: vertical writing is not required, by removing this item no outliner is created

    // #i105323# For 2D AtoShapes, the shadow attirbute does not need to be applied to any
    // of the constucted helper SdrObjects. This would lead to problems since the shadow
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

    const rtl::OUString sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
    const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
    const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );

    rtl::OUString sShapeType;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)(const SdrCustomShapeGeometryItem&)pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
    if ( pAny ) {
        *pAny >>= sShapeType;
        bOOXMLShape = ( sShapeType.compareToAscii( "ooxml-", 6 ) == 0 );
        OSL_TRACE("shape type: %s %d", OUStringToOString( sShapeType, RTL_TEXTENCODING_ASCII_US ).getStr(), bOOXMLShape);
    }
    eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

    pAny = rGeometryItem.GetPropertyValueByName( sMirroredX );
    if ( pAny )
        *pAny >>= bFlipH;
    pAny = rGeometryItem.GetPropertyValueByName( sMirroredY );
    if ( pAny )
        *pAny >>= bFlipV;

    if ( pCustomShapeObj->ISA( SdrObjCustomShape ) )    // should always be a SdrObjCustomShape, but you don't know
        nRotateAngle = (sal_Int32)(((SdrObjCustomShape*)pCustomShapeObj)->GetObjectRotation() * 100.0);
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
                sShapeType.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "col-" )))
            {
                nColorData = sShapeType.copy( 4 ).toInt32( 16 );
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
            vEquationResults[ i ].bReady = sal_False;
            try
            {
                vNodesSharedPtr[ i ] = EnhancedCustomShape::FunctionParser::parseFunction( seqEquations[ i ], *this );
            }
            catch ( EnhancedCustomShape::ParseError& )
            {
                OSL_TRACE("error: equation number: %d, parser failed ( %s )",
                          i, OUStringToOString( seqEquations[ i ], RTL_TEXTENCODING_ASCII_US ).getStr());
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
                    struct EquationResult &aResult = ((EnhancedCustomShape2d*)this)->vEquationResults[ nIndex ];

                    fNumber = aResult.fValue = (*vNodesSharedPtr[ nIndex ])();
                    aResult.bReady = sal_True;

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
                /* sal_Bool bUps = sal_True; */
                OSL_TRACE("error: EnhancedCustomShape2d::GetEquationValueAsDouble failed");
            }
#if OSL_DEBUG_LEVEL > 1
        nLevel --;
#endif
        }
        OSL_TRACE("  ?%d --> %f (angle: %f)", nIndex,
                  fNumber, 180.0*fNumber/10800000.0);
    }

    return fNumber;
}
sal_Int32 EnhancedCustomShape2d::GetAdjustValueAsInteger( const sal_Int32 nIndex, const sal_Int32 nDefault ) const
{
    sal_Int32 nNumber = nDefault;
    if ( nIndex < seqAdjustmentValues.getLength() )
    {
        if ( seqAdjustmentValues[ nIndex ].Value.getValueTypeClass() == TypeClass_DOUBLE )
        {
            double fNumber = 0;
            seqAdjustmentValues[ nIndex ].Value >>= fNumber;
            nNumber = (sal_Int32)fNumber;
        }
        else
            seqAdjustmentValues[ nIndex ].Value >>= nNumber;
    }
    return nNumber;
}
sal_Bool EnhancedCustomShape2d::SetAdjustValueAsDouble( const double& rValue, const sal_Int32 nIndex )
{
    sal_Bool bRetValue = sal_False;
    if ( nIndex < seqAdjustmentValues.getLength() )
    {
        // updating our local adjustment sequence
        seqAdjustmentValues[ nIndex ].Value <<= rValue;
        seqAdjustmentValues[ nIndex ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
        bRetValue = sal_True;
    }
    return bRetValue;
}

Point EnhancedCustomShape2d::GetPoint( const com::sun::star::drawing::EnhancedCustomShapeParameterPair& rPair,
                                        const sal_Bool bScale, const sal_Bool bReplaceGeoSize ) const
{
    Point       aRetValue;
    sal_Bool    bExchange = ( nFlags & DFF_CUSTOMSHAPE_EXCH ) != 0; // x <-> y
    sal_uInt32  nPass = 0;
    do
    {
        sal_uInt32  nIndex = nPass;

        if ( bExchange )
            nIndex ^= 1;

        double      fVal;
        const EnhancedCustomShapeParameter& rParameter = nIndex ? rPair.Second : rPair.First;
        if ( nPass )    // height
        {
            GetParameter( fVal, rParameter, sal_False, bReplaceGeoSize );
            fVal -= nCoordTop;
            if ( bScale )
            {
                fVal *= fYScale;

                if ( nFlags & DFF_CUSTOMSHAPE_FLIP_V )
                    fVal = aLogicRect.GetHeight() - fVal;
            }
            aRetValue.Y() = (sal_Int32)fVal;
        }
        else            // width
        {
            GetParameter( fVal, rParameter, bReplaceGeoSize, sal_False );
            fVal -= nCoordLeft;
            if ( bScale )
            {
                fVal *= fXScale;

                if ( nFlags & DFF_CUSTOMSHAPE_FLIP_H )
                    fVal = aLogicRect.GetWidth() - fVal;
            }
            aRetValue.X() = (sal_Int32)fVal;
        }
    }
    while ( ++nPass < 2 );
    return aRetValue;
}

sal_Bool EnhancedCustomShape2d::GetParameter( double& rRetValue, const EnhancedCustomShapeParameter& rParameter,
                                              const sal_Bool bReplaceGeoWidth, const sal_Bool bReplaceGeoHeight ) const
{
    rRetValue = 0.0;
    sal_Bool bRetValue = sal_False;
    switch ( rParameter.Type )
    {
        case EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            sal_Int32 nAdjustmentIndex = 0;
            if ( rParameter.Value >>= nAdjustmentIndex )
            {
                rRetValue = GetAdjustValueAsDouble( nAdjustmentIndex );
                bRetValue = sal_True;
            }
        }
        break;
        case EnhancedCustomShapeParameterType::EQUATION :
        {
            sal_Int32 nEquationIndex = 0;
            if ( rParameter.Value >>= nEquationIndex )
            {
                rRetValue = GetEquationValueAsDouble( nEquationIndex );
                bRetValue = sal_True;
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
                    bRetValue = sal_True;
                }
            }
            else
            {
                sal_Int32 nValue = 0;
                if ( rParameter.Value >>= nValue )
                {
                    rRetValue = nValue;
                    bRetValue = sal_True;
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
            bRetValue = sal_True;
        }
        break;
        case EnhancedCustomShapeParameterType::TOP :
        {
            rRetValue  = 0.0;
            bRetValue = sal_True;
        }
        break;
        case EnhancedCustomShapeParameterType::RIGHT :
        {
            rRetValue = nCoordWidth;
            bRetValue = sal_True;
        }
        break;
        case EnhancedCustomShapeParameterType::BOTTOM :
        {
            rRetValue = nCoordHeight;
            bRetValue = sal_True;
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
    Point aTopLeft( GetPoint( seqTextFrames[ nIndex ].TopLeft, !bOOXMLShape, sal_True ) );
    Point aBottomRight( GetPoint( seqTextFrames[ nIndex ].BottomRight, !bOOXMLShape, sal_True ) );
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
    OSL_TRACE("EnhancedCustomShape2d::GetTextRect: %d x %d", aRect.GetWidth(), aRect.GetHeight());
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

sal_Bool EnhancedCustomShape2d::GetHandlePosition( const sal_uInt32 nIndex, Point& rReturnPosition ) const
{
    sal_Bool bRetValue = sal_False;
    if ( nIndex < GetHdlCount() )
    {
        Handle aHandle;
        if ( ConvertSequenceToEnhancedCustomShape2dHandle( seqHandles[ nIndex ], aHandle ) )
        {
            if ( aHandle.nFlags & HANDLE_FLAGS_POLAR )
            {
                Point aReferencePoint( GetPoint( aHandle.aPolar, sal_True, sal_False ) );

                double      fAngle;
                double      fRadius;
                GetParameter( fRadius, aHandle.aPosition.First, sal_False, sal_False );
                GetParameter( fAngle,  aHandle.aPosition.Second, sal_False, sal_False );

                double a = ( 360.0 - fAngle ) * F_PI180;
                double dx = fRadius * fXScale;
                double fX = dx * cos( a );
                double fY =-dx * sin( a );
                rReturnPosition =
                    Point(
                        Round( fX + aReferencePoint.X() ),
                        basegfx::fTools::equalZero(fXScale) ? aReferencePoint.Y() :
                        Round( ( fY * fYScale ) / fXScale + aReferencePoint.Y() ) );
            }
            else
            {
                if ( aHandle.nFlags & HANDLE_FLAGS_SWITCHED )
                {
                    if ( aLogicRect.GetHeight() > aLogicRect.GetWidth() )
                    {
                        com::sun::star::drawing::EnhancedCustomShapeParameter aFirst = aHandle.aPosition.First;
                        com::sun::star::drawing::EnhancedCustomShapeParameter aSecond = aHandle.aPosition.Second;
                        aHandle.aPosition.First = aSecond;
                        aHandle.aPosition.Second = aFirst;
                    }
                }
                rReturnPosition = GetPoint( aHandle.aPosition, sal_True, sal_False );
            }
            const GeoStat aGeoStat( ((SdrObjCustomShape*)pCustomShapeObj)->GetGeoStat() );
            if ( aGeoStat.nShearWink )
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
            bRetValue = sal_True;
        }
    }
    return bRetValue;
}

sal_Bool EnhancedCustomShape2d::SetHandleControllerPosition( const sal_uInt32 nIndex, const com::sun::star::awt::Point& rPosition )
{
    sal_Bool bRetValue = sal_False;
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
            const GeoStat aGeoStat( ((SdrObjCustomShape*)pCustomShapeObj)->GetGeoStat() );
            if ( aGeoStat.nShearWink )
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

            if ( aHandle.nFlags & HANDLE_FLAGS_SWITCHED )
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

            if ( aHandle.nFlags & HANDLE_FLAGS_POLAR )
            {
                double fXRef, fYRef, fAngle;
                GetParameter( fXRef, aHandle.aPolar.First, sal_False, sal_False );
                GetParameter( fYRef, aHandle.aPolar.Second, sal_False, sal_False );
                const double fDX = fPos1 - fXRef;
                fAngle = -( atan2( -fPos2 + fYRef, ( ( fDX == 0.0L ) ? 0.000000001 : fDX ) ) / F_PI180 );
                double fX = ( fPos1 - fXRef );
                double fY = ( fPos2 - fYRef );
                double fRadius = sqrt( fX * fX + fY * fY );
                if ( aHandle.nFlags & HANDLE_FLAGS_RADIUS_RANGE_MINIMUM )
                {
                    double fMin;
                    GetParameter( fMin,  aHandle.aRadiusRangeMinimum, sal_False, sal_False );
                    if ( fRadius < fMin )
                        fRadius = fMin;
                }
                if ( aHandle.nFlags & HANDLE_FLAGS_RADIUS_RANGE_MAXIMUM )
                {
                    double fMax;
                    GetParameter( fMax, aHandle.aRadiusRangeMaximum, sal_False, sal_False );
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
                if ( aHandle.nFlags & HANDLE_FLAGS_REFX )
                {
                    nFirstAdjustmentValue = aHandle.nRefX;
                    fPos1 *= 100000.0;
                    fPos1 /= nCoordWidth;
                }
                if ( aHandle.nFlags & HANDLE_FLAGS_REFY )
                {
                    nSecondAdjustmentValue = aHandle.nRefY;
                    fPos2 *= 100000.0;
                    fPos2 /= nCoordHeight;
                }
                if ( nFirstAdjustmentValue >= 0 )
                {
                    if ( aHandle.nFlags & HANDLE_FLAGS_RANGE_X_MINIMUM )        // check if horizontal handle needs to be within a range
                    {
                        double fXMin;
                        GetParameter( fXMin, aHandle.aXRangeMinimum, sal_False, sal_False );
                        if ( fPos1 < fXMin )
                            fPos1 = fXMin;
                    }
                    if ( aHandle.nFlags & HANDLE_FLAGS_RANGE_X_MAXIMUM )        // check if horizontal handle needs to be within a range
                    {
                        double fXMax;
                        GetParameter( fXMax, aHandle.aXRangeMaximum, sal_False, sal_False );
                        if ( fPos1 > fXMax )
                            fPos1 = fXMax;
                    }
                    SetAdjustValueAsDouble( fPos1, nFirstAdjustmentValue );
                }
                if ( nSecondAdjustmentValue >= 0 )
                {
                    if ( aHandle.nFlags & HANDLE_FLAGS_RANGE_Y_MINIMUM )        // check if vertical handle needs to be within a range
                    {
                        double fYMin;
                        GetParameter( fYMin, aHandle.aYRangeMinimum, sal_False, sal_False );
                        if ( fPos2 < fYMin )
                            fPos2 = fYMin;
                    }
                    if ( aHandle.nFlags & HANDLE_FLAGS_RANGE_Y_MAXIMUM )        // check if vertical handle needs to be within a range
                    {
                        double fYMax;
                        GetParameter( fYMax, aHandle.aYRangeMaximum, sal_False, sal_False );
                        if ( fPos2 > fYMax )
                            fPos2 = fYMax;
                    }
                    SetAdjustValueAsDouble( fPos2, nSecondAdjustmentValue );
                }
            }
            // and writing them back into the GeometryItem
            SdrCustomShapeGeometryItem aGeometryItem((SdrCustomShapeGeometryItem&)
                (const SdrCustomShapeGeometryItem&)pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
            const rtl::OUString sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
            com::sun::star::beans::PropertyValue aPropVal;
            aPropVal.Name = sAdjustmentValues;
            aPropVal.Value <<= seqAdjustmentValues;
            aGeometryItem.SetPropertyValue( aPropVal );
            pCustomShapeObj->SetMergedItem( aGeometryItem );
            bRetValue = sal_True;
        }
    }
    return bRetValue;
}

void EnhancedCustomShape2d::SwapStartAndEndArrow( SdrObject* pObj ) //#108274
{
    XLineStartItem       aLineStart;
    aLineStart.SetLineStartValue(((XLineStartItem&)pObj->GetMergedItem( XATTR_LINEEND )).GetLineStartValue());
    XLineStartWidthItem  aLineStartWidth(((XLineStartWidthItem&)pObj->GetMergedItem( XATTR_LINEENDWIDTH )).GetValue());
    XLineStartCenterItem aLineStartCenter(((XLineStartCenterItem&)pObj->GetMergedItem( XATTR_LINEENDCENTER )).GetValue());

    XLineEndItem         aLineEnd;
    aLineEnd.SetLineEndValue(((XLineEndItem&)pObj->GetMergedItem( XATTR_LINESTART )).GetLineEndValue());
    XLineEndWidthItem    aLineEndWidth(((XLineEndWidthItem&)pObj->GetMergedItem( XATTR_LINESTARTWIDTH )).GetValue());
    XLineEndCenterItem   aLineEndCenter(((XLineEndCenterItem&)pObj->GetMergedItem( XATTR_LINESTARTCENTER )).GetValue());

    pObj->SetMergedItem( aLineStart );
    pObj->SetMergedItem( aLineStartWidth );
    pObj->SetMergedItem( aLineStartCenter );
    pObj->SetMergedItem( aLineEnd );
    pObj->SetMergedItem( aLineEndWidth );
    pObj->SetMergedItem( aLineEndCenter );
}

basegfx::B2DPolygon CreateArc( const Rectangle& rRect, const Point& rStart, const Point& rEnd, const sal_Bool bClockwise, sal_Bool bFullCircle = sal_False )
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

    Polygon aTempPoly( aRect, aStart, aEnd, POLY_ARC, bFullCircle );
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
                                           const sal_Bool bLineGeometryNeededOnly,
                                           const sal_Bool bSortFilledObjectsToBack,
                                           sal_Int32 nIndex )
{
    sal_Bool bNoFill = sal_False;
    sal_Bool bNoStroke = sal_False;
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
            const Point aTempPoint(GetPoint( *pTmp++, sal_True, sal_True ));
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
                    bNoFill = sal_True;
                break;
                case NOSTROKE :
                    bNoStroke = sal_True;
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
                        const Point aTempPoint(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
                        OSL_TRACE("moveTo: %d,%d", aTempPoint.X(), aTempPoint.Y());
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
                        const Point aControlA(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
                        const Point aControlB(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
                        const Point aEnd(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));

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
                    if(aNewB2DPolygon.count() > 1L)
                    {
                        // #i76201# Add conversion to closed polygon when first and last points are equal
                        basegfx::tools::checkClosed(aNewB2DPolygon);
                        aNewB2DPolyPolygon.append(aNewB2DPolygon);
                    }

                    aNewB2DPolygon.clear();
                }
                case ANGLEELLIPSETO :
                {
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 2 ) < nCoordSize ); i++ )
                    {
                        // create a circle
                        Point _aCenter( GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True ) );
                        double fWidth, fHeight;
                        GetParameter( fWidth,  seqCoordinates[ rSrcPt + 1 ].First, sal_True, sal_False );
                        GetParameter( fHeight,  seqCoordinates[ rSrcPt + 1 ].Second, sal_False, sal_True );
                        fWidth *= fXScale;
                        fHeight*= fYScale;
                        Point aP( (sal_Int32)( _aCenter.X() - fWidth ), (sal_Int32)( _aCenter.Y() - fHeight ) );
                        Size  aS( (sal_Int32)( fWidth * 2.0 ), (sal_Int32)( fHeight * 2.0 ) );
                        Rectangle aRect( aP, aS );
                        if ( aRect.GetWidth() && aRect.GetHeight() )
                        {
                            double fStartAngle, fEndAngle;
                            GetParameter( fStartAngle, seqCoordinates[ rSrcPt + 2 ].First,  sal_False, sal_False );
                            GetParameter( fEndAngle  , seqCoordinates[ rSrcPt + 2 ].Second, sal_False, sal_False );

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
                                aNewB2DPolygon.append(CreateArc( aRect, Point( (sal_Int32)fx1, (sal_Int32)fy1 ), Point( (sal_Int32)fx2, (sal_Int32)fy2 ), sal_False));
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
                    for ( sal_Int32 i(0L); ( i < nPntCount ) && ( rSrcPt + 1 < nCoordSize ); i++ )
                    {
                        const Point aControl(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
                        const Point aEnd(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));

                        DBG_ASSERT(aNewB2DPolygon.count(), "EnhancedCustomShape2d::CreateSubPath: Error in adding control point (!)");
                        basegfx::B2DPoint aStartPointB2D( aNewB2DPolygon.getB2DPoint(aNewB2DPolygon.count() - 1 ) );
                        Point aStartPoint( aStartPointB2D.getX(), aStartPointB2D.getY() );

                        aNewB2DPolygon.appendBezierSegment(
                            basegfx::B2DPoint(aStartPoint.X() + 2.0*(aControl.X() - aStartPoint.X())/3.0, aStartPoint.Y() + 2.0*(aControl.Y() - aStartPoint.Y())/3.0),
                            basegfx::B2DPoint(aEnd.X() + 2.0*(aControl.X() - aEnd.X())/3.0, aEnd.Y() + 2.0*(aControl.Y() - aEnd.Y())/3.0),
                            basegfx::B2DPoint(aEnd.X(), aEnd.Y()));
                    }
                    break;

                case LINETO :
                {
                    for ( sal_Int32 i(0L); ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                    {
                        const Point aTempPoint(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
                        OSL_TRACE("lineTo: %d,%d", aTempPoint.X(), aTempPoint.Y());
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
                    sal_Bool bClockwise = ( nCommand == CLOCKWISEARC ) || ( nCommand == CLOCKWISEARCTO );
                    sal_uInt32 nXor = bClockwise ? 3 : 2;
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 3 ) < nCoordSize ); i++ )
                    {
                        Rectangle aRect( GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True ), GetPoint( seqCoordinates[ rSrcPt + 1 ], sal_True, sal_True ) );
                        if ( aRect.GetWidth() && aRect.GetHeight() )
                        {
                            Point aCenter( aRect.Center() );
                            Point aStart( GetPoint( seqCoordinates[ (sal_uInt16)( rSrcPt + nXor ) ], sal_True, sal_True ) );
                            Point aEnd( GetPoint( seqCoordinates[ (sal_uInt16)( rSrcPt + ( nXor ^ 1 ) ) ], sal_True, sal_True ) );
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
                        GetParameter ( fWR, seqCoordinates[ (sal_uInt16)( rSrcPt ) ].First, sal_True, sal_False );
                        GetParameter ( fHR, seqCoordinates[ (sal_uInt16)( rSrcPt ) ].Second, sal_False, sal_True );

                        GetParameter ( fStartAngle, seqCoordinates[ (sal_uInt16)( rSrcPt + 1) ].First, sal_False, sal_False );
                        GetParameter ( fSwingAngle, seqCoordinates[ (sal_uInt16)( rSrcPt + 1 ) ].Second, sal_False, sal_False );

                        fWR *= fXScale;
                        fHR *= fYScale;

                        fStartAngle *= F_PI180;
                        fSwingAngle *= F_PI180;

                        OSL_TRACE("ARCANGLETO scale: %f x %f angles: %f, %f", fWR, fHR, fStartAngle, fSwingAngle);

                        sal_Bool bClockwise = fSwingAngle >= 0.0;

                        if (aNewB2DPolygon.count() > 0)
                        {
                            basegfx::B2DPoint aStartPointB2D( aNewB2DPolygon.getB2DPoint(aNewB2DPolygon.count() - 1 ) );
                            Point aStartPoint( aStartPointB2D.getX(), aStartPointB2D.getY() );

                            double fT = atan2((fWR*sin(fStartAngle)), (fHR*cos(fStartAngle)));
                            double fTE = atan2((fWR*sin(fStartAngle + fSwingAngle)), fHR*cos(fStartAngle + fSwingAngle));

                            OSL_TRACE("ARCANGLETO angles: %f, %f --> parameters: %f, %f", fStartAngle, fSwingAngle, fT, fTE );

                            Rectangle aRect ( Point ( aStartPoint.getX() - fWR*cos(fT) - fWR, aStartPoint.getY() - fHR*sin(fT) - fHR ),
                                              Point ( aStartPoint.getX() - fWR*cos(fT) + fWR, aStartPoint.getY() - fHR*sin(fT) + fHR) );

                            Point aEndPoint ( aStartPoint.getX() - fWR*(cos(fT) - cos(fTE)), aStartPoint.getY() - fHR*(sin(fT) - sin(fTE)) );

                            OSL_TRACE("ARCANGLETO rect: %d, %d   x   %d, %d   start: %d, %d end: %d, %d clockwise: %d",
                                      aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom(),
                                      aStartPoint.X(), aStartPoint.Y(), aEndPoint.X(), aEndPoint.Y(), bClockwise);
                            aNewB2DPolygon.append(CreateArc( aRect, bClockwise ? aEndPoint : aStartPoint, bClockwise ? aStartPoint : aEndPoint, bClockwise, aStartPoint == aEndPoint && fSwingAngle > F_PI));
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
                        Point aCurrent( GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True ) );

                        if ( rSrcPt )   // we need a previous point
                        {
                            Point aPrev( GetPoint( seqCoordinates[ rSrcPt - 1 ], sal_True, sal_True ) );
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
                    rtl::OStringBuffer aString(RTL_CONSTASCII_STRINGPARAM(
                        "CustomShapes::unknown PolyFlagValue :"));
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
                aTempSet.Put(SdrShadowItem(sal_False));
                aTempSet.Put(XLineStyleItem(XLINE_NONE));
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
                aTempSet.Put(SdrShadowItem(sal_False));
                aTempSet.Put(XFillStyleItem(XFILL_NONE));
                pStroke->SetMergedItemSet(aTempSet);
                rObjectList.push_back(pStroke);
            }
        }
        else
        {
            SdrPathObj* pObj = 0;
            SfxItemSet aTempSet(*this);
            aTempSet.Put(SdrShadowItem(sal_False));

            if(bNoFill)
            {
                // see comment above about OBJ_PLIN
                pObj = new SdrPathObj(
                    aNewB2DPolyPolygon.isClosed() ? OBJ_POLY : OBJ_PLIN,
                    aNewB2DPolyPolygon, dBrightness);
                aTempSet.Put(XFillStyleItem(XFILL_NONE));
            }
            else
            {
                aNewB2DPolyPolygon.setClosed(true);
                pObj = new SdrPathObj(OBJ_POLY, aNewB2DPolyPolygon, dBrightness);
            }

            if(bNoStroke)
            {
                aTempSet.Put(XLineStyleItem(XLINE_NONE));
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
    sal_Bool bAccent = sal_False;
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
            bAccent = sal_True;
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
            bAccent = sal_False;
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
        const XFillStyle eFillStyle = ((const XFillStyleItem&)rObj.GetMergedItem(XATTR_FILLSTYLE)).GetValue();
        switch( eFillStyle )
        {
            default:
            case XFILL_SOLID:
            {
                Color aFillColor;
                if ( nColorCount || rObj.GetBrightness() != 1.0 )
                {
                    aFillColor = GetColorData(
                        ((XFillColorItem&)rCustomShapeSet.Get( XATTR_FILLCOLOR )).GetColorValue(),
                        std::min(nColorIndex, nColorCount-1), rObj.GetBrightness() );
                    rObj.SetMergedItem( XFillColorItem( String(), aFillColor ) );
                }
                break;
            }
            case XFILL_GRADIENT:
            {
                XGradient aXGradient(((const XFillGradientItem&)rObj.GetMergedItem(XATTR_FILLGRADIENT)).GetGradientValue());
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

                rObj.SetMergedItem( XFillGradientItem( String(), aXGradient ) );
                break;
            }
            case XFILL_HATCH:
            {
                XHatch aXHatch(((const XFillHatchItem&)rObj.GetMergedItem(XATTR_FILLHATCH)).GetHatchValue());
                if ( nColorCount || rObj.GetBrightness() != 1.0 )
                {
                    aXHatch.SetColor(
                        GetColorData(
                            aXHatch.GetColor(),
                            std::min(nColorIndex, nColorCount-1), rObj.GetBrightness() ));
                }

                rObj.SetMergedItem( XFillHatchItem( String(), aXHatch ) );
                break;
            }
            case XFILL_BITMAP:
            {
                Bitmap aBitmap(((const XFillBitmapItem&)rObj.GetMergedItem(XATTR_FILLBITMAP)).GetBitmapValue().GetBitmap());
                if ( nColorCount || rObj.GetBrightness() != 1.0 )
                {
                    aBitmap.Adjust(
                        static_cast< short > ( GetLuminanceChange(
                            std::min(nColorIndex, nColorCount-1))));
                }

                rObj.SetMergedItem( XFillBitmapItem( String(), aBitmap ) );
                break;
            }
        }

        if ( nColorIndex < nColorCount )
            nColorIndex++;
    }
}

SdrObject* EnhancedCustomShape2d::CreatePathObj( sal_Bool bLineGeometryNeededOnly )
{
    sal_Int32 nCoordSize = seqCoordinates.getLength();
    if ( !nCoordSize )
        return NULL;

    sal_uInt16 nSrcPt = 0;
    sal_uInt16 nSegmentInd = 0;

    std::vector< SdrPathObj* > vObjectList;
    sal_Bool bSortFilledObjectsToBack = SortFilledObjectsToBackByDefault( eSpType );

    sal_Int32 nSubPathIndex = 0;

    while( nSegmentInd <= seqSegments.getLength() )
    {
        CreateSubPath( nSrcPt, nSegmentInd, vObjectList, bLineGeometryNeededOnly, bSortFilledObjectsToBack, nSubPathIndex );
        nSubPathIndex ++;
    }

    SdrObject* pRet = NULL;
    sal_uInt32 i;

    if ( !vObjectList.empty() )
    {
        const SfxItemSet& rCustomShapeSet = pCustomShapeObj->GetMergedItemSet();
        Color           aFillColor;
        sal_uInt32      nColorCount = nColorData >> 28;
        sal_uInt32      nColorIndex = 0;

        // #i37011# remove invisible objects
        if(!vObjectList.empty())
        {
            std::vector< SdrPathObj* > vTempList;

            for(i = 0L; i < vObjectList.size(); i++)
            {
                SdrPathObj* pObj(vObjectList[i]);
                const XLineStyle eLineStyle = ((const XLineStyleItem&)pObj->GetMergedItem(XATTR_LINESTYLE)).GetValue();
                const XFillStyle eFillStyle = ((const XFillStyleItem&)pObj->GetMergedItem(XATTR_FILLSTYLE)).GetValue();

                //SJ: #i40600# if bLineGeometryNeededOnly is set linystyle does not matter
                if( !bLineGeometryNeededOnly && ( XLINE_NONE == eLineStyle ) && ( XFILL_NONE == eFillStyle ) )
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
            for ( i = 0; i < vObjectList.size(); i++ )
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

                for ( i = 0; i < vObjectList.size(); i++ )
                {
                    SdrPathObj* pObj( vObjectList[ i ] );

                    if ( !pObj->IsLine() )
                    {
                        vTempList.push_back(pObj);
                    }
                }

                for ( i = 0; i < vObjectList.size(); i++ )
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
        if(vObjectList.size() > 1L)
        {
            pRet = new SdrObjGroup;

            for (i = 0L; i < vObjectList.size(); i++)
            {
                SdrObject* pObj(vObjectList[i]);
                pRet->GetSubList()->NbcInsertObject(pObj);
            }
        }
        else if(1L == vObjectList.size())
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

SdrObject* EnhancedCustomShape2d::CreateObject( sal_Bool bLineGeometryNeededOnly )
{
    SdrObject* pRet = NULL;

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

            aGluePoint.SetPos( GetPoint( seqGluePoints[ i ], sal_True, sal_True ) );
            aGluePoint.SetPercent( sal_False );
            aGluePoint.SetAlign( SDRVERTALIGN_TOP | SDRHORZALIGN_LEFT );
            aGluePoint.SetEscDir( SDRESC_SMART );
            SdrGluePointList* pList = pObj->ForceGluePointList();
            if( pList )
                /* sal_uInt16 nId = */ pList->Insert( aGluePoint );
        }
    }
}

bool EnhancedCustomShape2d::IsPostRotate() const
{
    return pCustomShapeObj->ISA( SdrObjCustomShape ) ? ((SdrObjCustomShape*)pCustomShapeObj)->IsPostRotate() : false;
}

SdrObject* EnhancedCustomShape2d::CreateLineGeometry()
{
    return CreateObject( sal_True );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
