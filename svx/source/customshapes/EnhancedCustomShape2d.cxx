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
#include "precompiled_svx.hxx"
#include "svx/EnhancedCustomShape2d.hxx"
#include "svx/EnhancedCustomShapeGeometry.hxx"
#include "svx/EnhancedCustomShapeTypeNames.hxx"
#include <svx/svdoashp.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#ifndef _SVDOCAPT_HXX
#include <svx/svdocapt.hxx>
#endif
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
#ifndef __COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENTCOMMAND_HPP__
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/svdlegacy.hxx>
#include <math.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand;

//void old_ShearPoint(basegfx::B2DPoint& rPnt, const basegfx::B2DPoint& rRef, double tn)
//{
//  if (!basegfx::fTools::equal(rPnt.getY(), rRef.getY()))
//  {
//      rPnt.setX(rPnt.getX() - ((rPnt.getY() - rRef.getY()) * tn));
//  }
//}

//void old_RotatePoint(basegfx::B2DPoint& rPnt, const basegfx::B2DPoint& rRef, double sn, double cs)
//{
//  const double fx(rPnt.getX() - rRef.getX());
//  const double fy(rPnt.getY() - rRef.getY());
//
//  rPnt.setX(rRef.getX() + fx * cs + fy * sn);
//  rPnt.setY(rRef.getY() + fy * cs - fx * sn);
//}

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

rtl::OUString EnhancedCustomShape2d::GetEquation( const sal_uInt16 nFlags, sal_Int32 nP1, sal_Int32 nP2, sal_Int32 nP3 )
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

void EnhancedCustomShape2d::AppendEnhancedCustomShapeEquationParameter( rtl::OUString& rParameter, const sal_Int32 nPara, const sal_Bool bIsSpecialValue )
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
//          const rtl::OUString sMap                ( RTL_CONSTASCII_USTRINGPARAM( "Map" ) );
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
/* seems not to be used.
            else if ( rPropVal.Name.equals( sMap ) )
            {
                com::sun::star::drawing::EnhancedCustomShapeParameterPair aMap;
                if ( rPropVal.Value >>= aMap )
                {
                    if ( GetValueForEnhancedCustomShapeHandleParameter( nXMap, aMap.First ) )
                        rDestinationHandle.Flags |= 0x800;
                    if ( GetValueForEnhancedCustomShapeHandleParameter( nYMap, aMap.Second ) )
                        rDestinationHandle.Flags |= 0x1000;
                    rDestinationHandle.Flags |= 0x10;
                }
            }
*/
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
    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( meSpType );
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
        nCoordLeft  = aViewBox.X;
        nCoordTop   = aViewBox.Y;
        nCoordWidth = labs( aViewBox.Width );
        nCoordHeight= labs( aViewBox.Height);
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

EnhancedCustomShape2d::EnhancedCustomShape2d( SdrObject* pAObj ) :
    SfxItemSet          ( pAObj->GetMergedItemSet() ),
    pCustomShapeObj     ( pAObj ),
    meSpType            ( mso_sptNil ),
    nCoordLeft          ( 0 ),
    nCoordTop           ( 0 ),
    nCoordWidth         ( 21600 ),
    nCoordHeight        ( 21600 ),
    nXRef               ( 0x80000000 ),
    nYRef               ( 0x80000000 ),
    nFlags              ( 0 ),
    nColorData          ( 0 ),
    bTextFlow           ( sal_False ),
    bFilled             ( ((const XFillStyleItem&)pAObj->GetMergedItem( XATTR_FILLSTYLE )).GetValue() != XFILL_NONE ),
    bStroked            ( ((const XLineStyleItem&)pAObj->GetMergedItem( XATTR_LINESTYLE )).GetValue() != XLINE_NONE )//,
//  bFlipH              ( sal_False ),
//  bFlipV              ( sal_False )
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

    // Do not use GetSnapRect here as in the original since we are inside
    // the geometry creation (createViewIndependentPrimitive2DSequence)
    maLogicScale = basegfx::absolute(pCustomShapeObj->getSdrObjectScale());

    const rtl::OUString sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
    // TTTT: MirroredX/Y removed
//  const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
//  const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );

    rtl::OUString sShapeType;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)(const SdrCustomShapeGeometryItem&)pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
    if ( pAny )
        *pAny >>= sShapeType;
    meSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

// TTTT:
//  pAny = rGeometryItem.GetPropertyValueByName( sMirroredX );
//  if ( pAny )
//      *pAny >>= bFlipH;
//  pAny = rGeometryItem.GetPropertyValueByName( sMirroredY );
//  if ( pAny )
//      *pAny >>= bFlipV;

//  SdrObjCustomShape* pSdrObjCustomShape = dynamic_cast< SdrObjCustomShape* >(pCustomShapeObj);
//
//  if ( pSdrObjCustomShape )   // should always be a SdrObjCustomShape, but you don't know
//      nRotateAngle = basegfx::fround(pSdrObjCustomShape->GetObjectRotation() * 100.0);
//  else
//      nRotateAngle = sdr::legacy::GetRotateAngle(*pCustomShapeObj);

    /*const sal_Int32* pDefData =*/ ApplyShapeAttributes( rGeometryItem );
    switch( meSpType )
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
     fXScale = nCoordWidth == 0 ? 0.0 : maLogicScale.getX() / (double)nCoordWidth;
     fYScale = nCoordHeight == 0 ? 0.0 : maLogicScale.getY() / (double)nCoordHeight;
     if ( (sal_uInt32)nXRef != 0x80000000 && !basegfx::fTools::equalZero(maLogicScale.getY()) )
    {
        fXRatio = maLogicScale.getX() / maLogicScale.getY();
        if ( fXRatio > 1 )
            fXScale /= fXRatio;
        else
            fXRatio = 1.0;
    }
    else
        fXRatio = 1.0;
    if ( (sal_uInt32)nYRef != 0x80000000 && !basegfx::fTools::equalZero(maLogicScale.getX()) )
    {
        fYRatio = maLogicScale.getY() / maLogicScale.getX();
        if ( fYRatio > 1 )
            fYScale /= fYRatio;
        else
            fYRatio = 1.0;
    }
    else
        fYRatio = 1.0;

    sal_Int32 i, nLength = seqEquations.getLength();


    if ( nLength )
    {
        vNodesSharedPtr.resize( nLength );
        for ( i = 0; i < seqEquations.getLength(); i++ )
        {
            try
            {
                vNodesSharedPtr[ i ] = EnhancedCustomShape::FunctionParser::parseFunction( seqEquations[ i ], *this );
            }
            catch ( EnhancedCustomShape::ParseError& )
            {
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
        case ENUM_FUNC_LOGWIDTH :   fRet = maLogicScale.getX(); break;
        case ENUM_FUNC_LOGHEIGHT :  fRet = maLogicScale.getY(); break;
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
    if ( nIndex < (sal_Int32)vNodesSharedPtr.size() )
    {
        if ( vNodesSharedPtr[ nIndex ].get() )
        try
        {
            fNumber = (*vNodesSharedPtr[ nIndex ])();
            if ( !rtl::math::isFinite( fNumber ) )
                fNumber = 0.0;
        }
        catch ( ... )
        {
            /* sal_Bool bUps = sal_True; */
        }
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

basegfx::B2DPoint EnhancedCustomShape2d::GetPoint(
    const com::sun::star::drawing::EnhancedCustomShapeParameterPair& rPair,
                                        const sal_Bool bScale, const sal_Bool bReplaceGeoSize ) const
{
    basegfx::B2DPoint aRetValue;
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
                    fVal = maLogicScale.getY() - fVal;
            }
            aRetValue.setY(fVal);
        }
        else            // width
        {
            GetParameter( fVal, rParameter, bReplaceGeoSize, sal_False );
            fVal -= nCoordLeft;
            if ( bScale )
            {
                fVal *= fXScale;

                if ( nFlags & DFF_CUSTOMSHAPE_FLIP_H )
                    fVal = maLogicScale.getX() - fVal;
            }
            aRetValue.setX(fVal);
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
                double fValue;
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

Color EnhancedCustomShape2d::GetColorData( const Color& rFillColor, sal_uInt32 nIndex ) const
{
    const sal_Int32 nLuminance = GetLuminanceChange(nIndex);
    if( !nLuminance )
        return rFillColor;

    basegfx::BColor aHSVColor=
        basegfx::tools::rgb2hsv(
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

    aHSVColor = basegfx::tools::hsv2rgb(aHSVColor);
    return Color( (sal_uInt8)static_cast< sal_Int32 >( basegfx::clamp(aHSVColor.getRed(),0.0,1.0) * 255.0 + 0.5 ),
                  (sal_uInt8)static_cast< sal_Int32 >( basegfx::clamp(aHSVColor.getGreen(),0.0,1.0) * 255.0 + 0.5 ),
                  (sal_uInt8)static_cast< sal_Int32 >( basegfx::clamp(aHSVColor.getBlue(),0.0,1.0) * 255.0 + 0.5 ) );
}

basegfx::B2DRange EnhancedCustomShape2d::GetTextRange() const
{
    sal_Int32 nIndex, nSize = seqTextFrames.getLength();
    basegfx::B2DRange aRetval(basegfx::B2DRange::getUnitB2DRange());
    basegfx::B2DHomMatrix aTransform;

    if ( nSize )
    {
        nIndex = 0;
        if ( bTextFlow && ( nSize > 1 ) )
            nIndex++;
        basegfx::B2DPoint aTopLeft( GetPoint( seqTextFrames[ nIndex ].TopLeft, sal_True, sal_True ) );
        basegfx::B2DPoint aBottomRight( GetPoint( seqTextFrames[ nIndex ].BottomRight, sal_True, sal_True ) );
//      if ( bFlipH )
//      {
//          aTopLeft.setX(maLogicRange.getWidth() - aTopLeft.getX());
//          aBottomRight.setX(maLogicRange.getWidth() - aBottomRight.getX());
//      }
//      if ( bFlipV )
//      {
//          aTopLeft.setY(maLogicRange.getHeight() - aTopLeft.getY());
//          aBottomRight.setY(maLogicRange.getHeight() - aBottomRight.getY());
//      }
        aRetval = basegfx::B2DRange(aTopLeft, aBottomRight);
        aTransform.scale(
            basegfx::fTools::equalZero(maLogicScale.getX()) ? 1.0 : 1.0 / maLogicScale.getX(),
            basegfx::fTools::equalZero(maLogicScale.getY()) ? 1.0 : 1.0 / maLogicScale.getY());
    }

    // To keep tight to the original, ignore rotate and shear. If this
    // is not wanted, just use getSdrObjectTransformation() instead
    aTransform.scale(basegfx::absolute(pCustomShapeObj->getSdrObjectScale()));
    aTransform.translate(pCustomShapeObj->getSdrObjectTranslate());

    aRetval *= aTransform;

    return aRetval;
}

sal_uInt32 EnhancedCustomShape2d::GetHdlCount() const
{
    return seqHandles.getLength();
}

sal_Bool EnhancedCustomShape2d::GetHandlePosition( const sal_uInt32 nIndex, basegfx::B2DPoint& rReturnPosition ) const
{
    sal_Bool bRetValue = sal_False;
    if ( nIndex < GetHdlCount() )
    {
        Handle aHandle;
        if ( ConvertSequenceToEnhancedCustomShape2dHandle( seqHandles[ nIndex ], aHandle ) )
        {
            if ( aHandle.nFlags & HANDLE_FLAGS_POLAR )
            {
                basegfx::B2DPoint aReferencePoint( GetPoint( aHandle.aPolar, sal_True, sal_False ) );

                double      fAngle;
                double      fRadius;
                GetParameter( fRadius, aHandle.aPosition.First, sal_False, sal_False );
                GetParameter( fAngle,  aHandle.aPosition.Second, sal_False, sal_False );

                double a = ( 360.0 - fAngle ) * F_PI180;
                double dx = fRadius * fXScale;
                double fX = dx * cos( a );
                double fY =-dx * sin( a );
                rReturnPosition =
                    basegfx::B2DPoint(
                        fX + aReferencePoint.getX(),
                        basegfx::fTools::equalZero(fXScale)
                            ? aReferencePoint.getY()
                            : ( fY * fYScale ) / fXScale + aReferencePoint.getY() );
            }
            else
            {
                if ( aHandle.nFlags & HANDLE_FLAGS_SWITCHED )
                {
                    if ( maLogicScale.getY() > maLogicScale.getX() )
                    {
                        com::sun::star::drawing::EnhancedCustomShapeParameter aFirst = aHandle.aPosition.First;
                        com::sun::star::drawing::EnhancedCustomShapeParameter aSecond = aHandle.aPosition.Second;
                        aHandle.aPosition.First = aSecond;
                        aHandle.aPosition.Second = aFirst;
                    }
                }
                rReturnPosition = GetPoint( aHandle.aPosition, sal_True, sal_False );
            }

            // transform to logic coordinates using object's transformation. rReturnPosition
            // is already scaled
            rReturnPosition /= basegfx::absolute(pCustomShapeObj->getSdrObjectScale());
            rReturnPosition *= pCustomShapeObj->getSdrObjectTransformation();

//          const long aOldShear(sdr::legacy::GetShearAngleX(*pCustomShapeObj));
//          if ( aOldShear )
//          {
//              double nTan = tan(aOldShear*nPi180);
////                if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
////                    nTan = -nTan;
//              old_ShearPoint( rReturnPosition, maLogicRange.getRange() * 0.5, nTan );
//          }
//
//          if ( nRotateAngle )
//          {
//              double a = nRotateAngle * F_PI18000;
//              old_RotatePoint( rReturnPosition, maLogicRange.getRange() * 0.5, sin( a ), cos( a ) );
//          }
//
////            if ( bFlipH )
////                rReturnPosition.setX(maLogicRange.getWidth() - rReturnPosition.getX());
////
////            if ( bFlipV )
////                rReturnPosition.setY(maLogicRange.getHeight() - rReturnPosition.getY());
//
//          rReturnPosition += maLogicRange.getMinimum();
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
            basegfx::B2DPoint aP( rPosition.X, rPosition.Y );

            // transform back to just scaled
            basegfx::B2DHomMatrix aInvSdrObj(pCustomShapeObj->getSdrObjectTransformation());
            aInvSdrObj.invert();
            aP *= aInvSdrObj;
            aP *= basegfx::absolute(pCustomShapeObj->getSdrObjectScale());

//          // apply the negative object rotation to the controller position
//
//          aP -= maLogicRange.getMinimum();
////            if ( bFlipH )
////                aP.setX(maLogicRange.getWidth() - aP.getX());
////            if ( bFlipV )
////                aP.setY(maLogicRange.getHeight() - aP.getY());
//          if ( nRotateAngle )
//          {
//              double a = -nRotateAngle * F_PI18000;
//              old_RotatePoint( aP, maLogicRange.getRange() * 0.5, sin( a ), cos( a ) );
//          }
//
//          const long aOldShear(sdr::legacy::GetShearAngleX(*pCustomShapeObj));
//          if ( aOldShear )
//          {
//              double nTan = -tan(aOldShear*nPi180);
////                if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
////                    nTan = -nTan;
//              old_ShearPoint( aP, maLogicRange.getRange() * 0.5, nTan );
//          }

            double fPos1 = aP.getX();
            double fPos2 = aP.getY();
            fPos1 /= fXScale;
            fPos2 /= fYScale;

            if ( aHandle.nFlags & HANDLE_FLAGS_SWITCHED )
            {
                if ( maLogicScale.getY() > maLogicScale.getX() )
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

basegfx::B2DPolygon CreateArc(
    const basegfx::B2DRange& rRange,
    const basegfx::B2DPoint& rStart,
    const basegfx::B2DPoint& rEnd,
    const sal_Bool bClockwise )
{
    const basegfx::B2DPoint aCenter(rRange.getCenter());
    const basegfx::B2DVector aHalfRange(rRange.getRange() * 0.5);
    double fStart(basegfx::snapToZeroRange(atan2(rStart.getY() - aCenter.getY(), rStart.getX() - aCenter.getX()), F_2PI));
    double fEnd(basegfx::snapToZeroRange(atan2(rEnd.getY() - aCenter.getY(), rEnd.getX() - aCenter.getX()), F_2PI));

    basegfx::B2DPolygon aRetval(
        basegfx::tools::createPolygonFromEllipseSegment(
            aCenter,
            aHalfRange.getX(),
            aHalfRange.getY(),
            fEnd,
            fStart));

    if(!bClockwise)
        {
        aRetval.flip();
    }

    return aRetval;
}

void EnhancedCustomShape2d::CreateSubPath( sal_uInt16& rSrcPt, sal_uInt16& rSegmentInd, std::vector< SdrPathObj* >& rObjectList,
                                                                                        const sal_Bool bLineGeometryNeededOnly,
                                                                                        const sal_Bool bSortFilledObjectsToBack )
{
    sal_Bool bNoFill = sal_False;
    sal_Bool bNoStroke = sal_False;

    basegfx::B2DPolyPolygon aNewB2DPolyPolygon;
    basegfx::B2DPolygon aNewB2DPolygon;

    sal_Int32 nCoordSize = seqCoordinates.getLength();
    sal_Int32 nSegInfoSize = seqSegments.getLength();
    if ( !nSegInfoSize )
    {
        const EnhancedCustomShapeParameterPair* pTmp = seqCoordinates.getArray();

        for ( sal_Int32 nPtNum(0L); nPtNum < nCoordSize; nPtNum++ )
        {
            aNewB2DPolygon.append(GetPoint( *pTmp++, sal_True, sal_True ));
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
                        aNewB2DPolygon.append(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
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
                        const basegfx::B2DPoint aControlA(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
                        const basegfx::B2DPoint aControlB(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
                        const basegfx::B2DPoint aEnd(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));

                        DBG_ASSERT(aNewB2DPolygon.count(), "EnhancedCustomShape2d::CreateSubPath: Error in adding control point (!)");
                        aNewB2DPolygon.appendBezierSegment(aControlA, aControlB, aEnd);
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
                        basegfx::B2DPoint _aCenter( GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True ) );
                        double fWidth, fHeight;
                        MSO_SPT eSpType = mso_sptEllipse;
                        const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
                        sal_Bool bIsDefaultViewBox = sal_False;
                        sal_Bool bIsDefaultPath = sal_False;
                        sal_Bool bIsMSEllipse = sal_False;

                        if( ( nCoordWidth == pDefCustomShape->nCoordWidth )
                            && ( nCoordHeight == pDefCustomShape->nCoordHeight ) )
                        {
                            bIsDefaultViewBox = sal_True;
                        }

                        sal_Int32 j, nCount = pDefCustomShape->nVertices;//==3
                        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqCoordinates1, seqCoordinates2;

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
                        {
                            bIsDefaultPath = sal_True;
                        }

                        const rtl::OUString sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
                        rtl::OUString sShpType;
                        SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)(const SdrCustomShapeGeometryItem&)pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
                        Any* pAny = rGeometryItem.GetPropertyValueByName( sType );

                        if ( pAny )
                        {
                            *pAny >>= sShpType;
                        }

                        if( sShpType.getLength() > 3 && sShpType.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "mso" )))
                        {
                                bIsMSEllipse = sal_True;
                        }

                        if( (! bIsDefaultPath   && ! bIsDefaultViewBox) || (bIsDefaultViewBox && bIsMSEllipse) /*&& (nGeneratorVersion == SfxObjectShell::Sym_L2)*/ )
                        {
                            _aCenter = GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True );
                            GetParameter( fWidth,  seqCoordinates[ rSrcPt + 1 ].First, sal_True, sal_False  );
                            GetParameter( fHeight,  seqCoordinates[ rSrcPt + 1 ].Second, sal_False, sal_True );
                            fWidth /= 2;
                            fHeight /= 2;
                        }
                        else if( bIsDefaultPath && !bIsDefaultViewBox /*&& (nGeneratorVersion == SfxObjectShell::Sym_L2)*/ )
                        {
                            _aCenter.setX(nCoordWidth/2 * fXScale);
                            _aCenter.setY(nCoordHeight/2 * fYScale);
                            fWidth = nCoordWidth/2;
                            fHeight = nCoordHeight/2;

                            const rtl::OUString sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
                            const Any* pViewBox = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sViewBox );
                            com::sun::star::awt::Rectangle aViewBox;
                            if ( pViewBox && (*pViewBox >>= aViewBox ) )
                            {
                                aViewBox.Width = pDefCustomShape->nCoordWidth;
                                aViewBox.Height = pDefCustomShape->nCoordHeight;
                            }
                            com::sun::star::beans::PropertyValue aPropVal;
                            aPropVal.Name = sViewBox;
                            aPropVal.Value <<= aViewBox;
                            rGeometryItem.SetPropertyValue( aPropVal );
                            pCustomShapeObj->SetMergedItem( rGeometryItem );
                        }
                        else
                        {
                            _aCenter = GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True );
                            GetParameter( fWidth,  seqCoordinates[ rSrcPt + 1 ].First, sal_True, sal_False  );
                            GetParameter( fHeight,  seqCoordinates[ rSrcPt + 1 ].Second, sal_False, sal_True );
                        }

                        fWidth *= fXScale;
                        fHeight*= fYScale;
                        basegfx::B2DRange aRange(
                            _aCenter.getX() - fWidth, _aCenter.getY() - fHeight,
                            _aCenter.getX() + fWidth, _aCenter.getY() + fHeight);

                        if ( !basegfx::fTools::equalZero(aRange.getWidth()) && !basegfx::fTools::equalZero(aRange.getHeight()) )
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
                                double fCenterX = aRange.getCenterX();
                                double fCenterY = aRange.getCenterY();
                                double fx1 = ( cos( fStartAngle * F_PI180 ) * 65536.0 * fXScale ) + fCenterX;
                                double fy1 = ( -sin( fStartAngle * F_PI180 ) * 65536.0 * fYScale ) + fCenterY;
                                double fx2 = ( cos( fEndAngle * F_PI180 ) * 65536.0 * fXScale ) + fCenterX;
                                double fy2 = ( -sin( fEndAngle * F_PI180 ) * 65536.0 * fYScale ) + fCenterY;

                                const bool bMirrorX(fWidth < 0.0);
                                const bool bMirrorY(fHeight < 0.0);
                                const bool bSwap((bMirrorX || bMirrorY) && (bMirrorX != bMirrorY));
                                const basegfx::B2DPoint aTopLeft(fx1, fy1);
                                const basegfx::B2DPoint aBottomRight(fx2, fy2);
                                aNewB2DPolygon.append(
                                    CreateArc( // TTTT: check CreateArc, completely changed from old tools polygon (!)
                                        aRange,
                                        bSwap ? aBottomRight : aTopLeft,
                                        bSwap ? aTopLeft : aBottomRight,
                                        sal_False));
                            }
                            else
                            {   /* SJ: TODO: this block should be replaced sometimes, because the current point
                                   is not set correct, it also does not use the correct moveto
                                   point if ANGLEELLIPSETO was used, but the method CreateArc
                                   is at the moment not able to draw full circles (if startangle is 0
                                   and endangle 360 nothing is painted :-( */
                                double fXControl(aRange.getWidth() * 0.2835);
                                double fYControl(aRange.getHeight() * 0.2835);
                                basegfx::B2DPoint aCenter( aRange.getCenter() );

                                // append start point
                                aNewB2DPolygon.append(basegfx::B2DPoint(aCenter.getX(), aRange.getMinY()));

                                // append four bezier segments
                                aNewB2DPolygon.appendBezierSegment(
                                    basegfx::B2DPoint(aCenter.getX() + fXControl, aRange.getMinY()),
                                    basegfx::B2DPoint(aRange.getMaxX(), aCenter.getY() - fYControl),
                                    basegfx::B2DPoint(aRange.getMaxX(), aCenter.getY()));

                                aNewB2DPolygon.appendBezierSegment(
                                    basegfx::B2DPoint(aRange.getMaxX(), aCenter.getY() + fYControl),
                                    basegfx::B2DPoint(aCenter.getX() + fXControl, aRange.getMaxY()),
                                    basegfx::B2DPoint(aCenter.getX(), aRange.getMaxY()));

                                aNewB2DPolygon.appendBezierSegment(
                                    basegfx::B2DPoint(aCenter.getX() - fXControl, aRange.getMaxY()),
                                    basegfx::B2DPoint(aRange.getMinX(), aCenter.getY() + fYControl),
                                    basegfx::B2DPoint(aRange.getMinX(), aCenter.getY()));

                                aNewB2DPolygon.appendBezierSegment(
                                    basegfx::B2DPoint(aRange.getMinX(), aCenter.getY() - fYControl),
                                    basegfx::B2DPoint(aCenter.getX() - fXControl, aRange.getMinY()),
                                    basegfx::B2DPoint(aCenter.getX(), aRange.getMinY()));

                                // close, rescue last controlpoint, remove double last point
                                basegfx::tools::closeWithGeometryChange(aNewB2DPolygon);
                            }
                        }
                        rSrcPt += 3;
                    }
                }
                break;

                case LINETO :
                {
                    for ( sal_Int32 i(0L); ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                    {
                        aNewB2DPolygon.append(GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True ));
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
                        const basegfx::B2DPoint aPointTL(GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True ));
                        const basegfx::B2DPoint aPointBR(GetPoint( seqCoordinates[ rSrcPt + 1 ], sal_True, sal_True ));
                        basegfx::B2DRange aRange( aPointTL, aPointBR );

                        if ( !basegfx::fTools::equalZero(aRange.getWidth()) && !basegfx::fTools::equalZero(aRange.getHeight()) )
                        {
                            basegfx::B2DPoint aCenter( aRange.getCenter() );
                            basegfx::B2DPoint aStart( GetPoint( seqCoordinates[ (sal_uInt16)( rSrcPt + nXor ) ], sal_True, sal_True ) );
                            basegfx::B2DPoint aEnd( GetPoint( seqCoordinates[ (sal_uInt16)( rSrcPt + ( nXor ^ 1 ) ) ], sal_True, sal_True ) );
                            double fRatio = aRange.getHeight() / aRange.getWidth();
                            aStart.setX((((aStart.getX() - aCenter.getX())) * fRatio) + aCenter.getX());
                            aStart.setY((((aStart.getY() - aCenter.getY()))) + aCenter.getY());
                            aEnd.setX((((aEnd.getX() - aCenter.getX())) * fRatio) + aCenter.getX());
                            aEnd.setY((((aEnd.getY() - aCenter.getY()))) + aCenter.getY());
                            const bool bMirrorX(aPointTL.getX() > aPointBR.getX());
                            const bool bMirrorY(aPointTL.getY() > aPointBR.getY());
                            const bool bSwap((bMirrorX || bMirrorY) && (bMirrorX != bMirrorY));
                            aNewB2DPolygon.append(
                                CreateArc( // TTTT: check CreateArc, completely changed from old tools polygon (!)
                                    aRange,
                                    bSwap ? aEnd : aStart,
                                    bSwap ? aStart : aEnd,
                                    bClockwise));
                        }
                        rSrcPt += 4;
                    }
                }
                break;

                case ELLIPTICALQUADRANTX :
                case ELLIPTICALQUADRANTY :
                {
                    bool bFirstDirection(true);

                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                    {
                        sal_uInt32 nModT = ( nCommand == ELLIPTICALQUADRANTX ) ? 1 : 0;
                        basegfx::B2DPoint aCurrent( GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True ) );

                        if ( rSrcPt )   // we need a previous point
                        {
                            basegfx::B2DPoint aPrev( GetPoint( seqCoordinates[ rSrcPt - 1 ], sal_True, sal_True ) );
                            double fX(aCurrent.getX() - aPrev.getX());
                            double fY(aCurrent.getY() - aPrev.getY());

                            if((fX < 0.0 && fY > 0.0) || (fX > 0.0 && fY < 0.0))
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
                                fX = aCurrent.getX();
                                fY = aPrev.getY();
                            }
                            else
                            {
                                fX = aPrev.getX();
                                fY = aCurrent.getY();
                            }

                            const basegfx::B2DPoint aControlPointA(
                                aPrev.getX() + ((fX - aPrev.getX()) * 0.5),
                                aPrev.getY() + ((fY - aPrev.getY()) * 0.5));

                            const basegfx::B2DPoint aControlPointB(
                                aCurrent.getX() + ((fX - aCurrent.getX()) * 0.5),
                                aCurrent.getY() + ((fY - aCurrent.getY()) * 0.5));

                            aNewB2DPolygon.appendBezierSegment(
                                aControlPointA,
                                aControlPointB,
                                aCurrent);
                        }
                        else
                        {
                            aNewB2DPolygon.append(aCurrent);
                        }

                        rSrcPt++;
                    }
                }
                break;

#ifdef DBG_CUSTOMSHAPE
                case UNKNOWN :
                default :
                {
                    ByteString aString( "CustomShapes::unknown PolyFlagValue :" );
                    aString.Append( ByteString::CreateFromInt32( nCommand ) );
                    DBG_ERROR( aString.GetBuffer() );
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
                    pCustomShapeObj->getSdrModelFromSdrObject(),
                    aClosedPolyPolygon);
                SfxItemSet aTempSet(*this);
                aTempSet.Put(SdrOnOffItem(SDRATTR_SHADOW, sal_False));
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
                    pCustomShapeObj->getSdrModelFromSdrObject(),
                    aNewB2DPolyPolygon);
                SfxItemSet aTempSet(*this);
                aTempSet.Put(SdrOnOffItem(SDRATTR_SHADOW, sal_False));
                aTempSet.Put(XFillStyleItem(XFILL_NONE));
                pStroke->SetMergedItemSet(aTempSet);
                rObjectList.push_back(pStroke);
            }
        }
        else
        {
            SdrPathObj* pObj = 0;
            SfxItemSet aTempSet(*this);
            aTempSet.Put(SdrOnOffItem(SDRATTR_SHADOW, sal_False));

            if(bNoFill)
            {
                // see comment above about OBJ_PLIN
                pObj = new SdrPathObj(
                    pCustomShapeObj->getSdrModelFromSdrObject(),
                    aNewB2DPolyPolygon);
                aTempSet.Put(XFillStyleItem(XFILL_NONE));
            }
            else
            {
                aNewB2DPolyPolygon.setClosed(true);
                pObj = new SdrPathObj(
                    pCustomShapeObj->getSdrModelFromSdrObject(),
                    aNewB2DPolyPolygon);
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
                if(pObj->isLine())
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
                if(pObj->isLine())
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
                if(pObj->isLine())
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
    if ( !rObj.isLine() )
    {
        const XFillStyle eFillStyle = ((const XFillStyleItem&)rObj.GetMergedItem(XATTR_FILLSTYLE)).GetValue();
        switch( eFillStyle )
        {
            default:
            case XFILL_SOLID:
            {
                Color aFillColor;
                if ( nColorCount )
                {
                    aFillColor = GetColorData(
                        ((XFillColorItem&)rCustomShapeSet.Get( XATTR_FILLCOLOR )).GetColorValue(),
                        std::min(nColorIndex, nColorCount-1) );
                    rObj.SetMergedItem( XFillColorItem( String(), aFillColor ) );
                }
                break;
            }
            case XFILL_GRADIENT:
            {
                XGradient aXGradient(((const XFillGradientItem&)rObj.GetMergedItem(XATTR_FILLGRADIENT)).GetGradientValue());
                if ( nColorCount )
                {
                    aXGradient.SetStartColor(
                        GetColorData(
                            aXGradient.GetStartColor(),
                            std::min(nColorIndex, nColorCount-1) ));
                    aXGradient.SetEndColor(
                        GetColorData(
                            aXGradient.GetEndColor(),
                            std::min(nColorIndex, nColorCount-1) ));
                }

                rObj.SetMergedItem( XFillGradientItem( String(), aXGradient ) );
                break;
            }
            case XFILL_HATCH:
            {
                XHatch aXHatch(((const XFillHatchItem&)rObj.GetMergedItem(XATTR_FILLHATCH)).GetHatchValue());
                if ( nColorCount )
                {
                    aXHatch.SetColor(
                        GetColorData(
                            aXHatch.GetColor(),
                            std::min(nColorIndex, nColorCount-1) ));
                }

                rObj.SetMergedItem( XFillHatchItem( String(), aXHatch ) );
                break;
            }
            case XFILL_BITMAP:
            {
                if ( nColorCount )
                {
                    Bitmap aBitmap(((const XFillBitmapItem&)rObj.GetMergedItem(XATTR_FILLBITMAP)).GetGraphicObject().GetGraphic().GetBitmapEx().GetBitmap());

                    aBitmap.Adjust(
                        static_cast< short > ( GetLuminanceChange(
                            std::min(nColorIndex, nColorCount-1))));

                    rObj.SetMergedItem(XFillBitmapItem(String(), Graphic(aBitmap)));
                }

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
    sal_Bool bSortFilledObjectsToBack = SortFilledObjectsToBackByDefault( meSpType );

    while( nSegmentInd <= seqSegments.getLength() )
    {
        CreateSubPath( nSrcPt, nSegmentInd, vObjectList, bLineGeometryNeededOnly, bSortFilledObjectsToBack );
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
                {
                    deleteSdrObjectSafeAndClearPointer(pObj);
                }
                else
                {
                    vTempList.push_back(pObj);
                }
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

                if(pObj->isLine())
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
                CorrectCalloutArrows( meSpType, nLineObjectCount, vObjectList );

            // sort objects so that filled ones are in front. Necessary
            // for some strange objects
            if ( bSortFilledObjectsToBack )
            {
                std::vector< SdrPathObj* > vTempList;

                for ( i = 0; i < vObjectList.size(); i++ )
                {
                    SdrPathObj* pObj( vObjectList[ i ] );

                    if ( !pObj->isLine() )
                    {
                        vTempList.push_back(pObj);
                    }
                }

                for ( i = 0; i < vObjectList.size(); i++ )
                {
                    SdrPathObj* pObj( vObjectList[ i ] );

                    if ( pObj->isLine() )
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
            SdrObjGroup* pNewGroup = new SdrObjGroup(pCustomShapeObj->getSdrModelFromSdrObject());
            pRet = pNewGroup;

            for (i = 0L; i < vObjectList.size(); i++)
            {
                SdrObject* pObj(vObjectList[i]);
                pNewGroup->InsertObjectToSdrObjList(*pObj);
            }
        }
        else if(1L == vObjectList.size())
        {
            pRet = vObjectList[0L];
        }
    }

    return pRet;
}

SdrObject* EnhancedCustomShape2d::CreateObject( sal_Bool bLineGeometryNeededOnly )
{
    SdrObject* pRet = NULL;

    if ( meSpType == mso_sptRectangle )
    {
        pRet = new SdrRectObj(
            pCustomShapeObj->getSdrModelFromSdrObject(),
            pCustomShapeObj->getSdrObjectTransformation());
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

SdrObject* EnhancedCustomShape2d::CreateLineGeometry()
{
    return CreateObject( sal_True );
}


