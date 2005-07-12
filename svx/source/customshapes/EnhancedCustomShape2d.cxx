/*************************************************************************
 *
 *  $RCSfile: EnhancedCustomShape2d.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 13:37:41 $
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
#ifndef _ENHANCEDCUSTOMSHAPE2D_HXX
#include "EnhancedCustomShape2d.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPEGEOMETRY_HXX
#include "EnhancedCustomShapeGeometry.hxx"
#endif
#ifndef _ENHANCED_CUSTOMSHAPE_TYPE_NAMES_HXX
#include "EnhancedCustomShapeTypeNames.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX
#include "EnhancedCustomShapeFunctionParser.hxx"
#endif
#ifndef _SVDOASHP_HXX
#include "svdoashp.hxx"
#endif
#ifndef _SVDTRANS_HXX
#include "svdtrans.hxx"
#endif
#ifndef _SVDOCIRC_HXX
#include <svdocirc.hxx>
#endif
#ifndef _SVDOGRP_HXX
#include <svdogrp.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svdopath.hxx>
#endif
#ifndef _SVDOCAPT_HXX
#include "svdocapt.hxx"
#endif
#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <xflclit.hxx>
#endif
#ifndef _SDASAITM_HXX
#include <sdasaitm.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif
#ifndef _XOUTX_HXX
#include <xoutx.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif
#ifndef SVX_XFILLIT0_HXX
#include <xfillit0.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX
#include <xlnstit.hxx>
#endif
#ifndef _SVX_XLNEDIT_HXX
#include <xlnedit.hxx>
#endif
#ifndef _SVX_XLNSTWIT_HXX
#include <xlnstwit.hxx>
#endif
#ifndef _SVX_XLNEDWIT_HXX
#include <xlnedwit.hxx>
#endif
#ifndef _SVX_XLNSTCIT_HXX
#include <xlnstcit.hxx>
#endif
#ifndef _SVX_XLNEDCIT_HXX
#include <xlnedcit.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERTYPE_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#endif
#ifndef __COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENTCOMMAND_HPP__
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

//#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
//#include <basegfx/polygon/b2dpolypolygontools.hxx>
//#endif

//#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
//#include <basegfx/matrix/b2dhommatrix.hxx>
//#endif

//#ifndef _EEITEM_HXX
//#include <eeitem.hxx>
//#endif
//
//#define ITEMID_COLOR          0
//
//#ifndef _SVX_LCOLITEM_HXX
//#include <lcolitem.hxx>
//#endif
//
//#ifndef _SVX_XLNTRIT_HXX
//#include <xlntrit.hxx>
//#endif
//
//#ifndef _SVX_XFLTRIT_HXX
//#include <xfltrit.hxx>
//#endif

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
//          const rtl::OUString sMap                ( RTL_CONSTASCII_USTRINGPARAM( "Map" ) );
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
                sal_Bool bMirroredX;
                if ( rPropVal.Value >>= bMirroredX )
                {
                    if ( bMirroredX )
                        rDestinationHandle.nFlags |= HANDLE_FLAGS_MIRRORED_X;
                }
            }
            else if ( rPropVal.Name.equals( sMirroredY ) )
            {
                sal_Bool bMirroredY;
                if ( rPropVal.Value >>= bMirroredY )
                {
                    if ( bMirroredY )
                        rDestinationHandle.nFlags |= HANDLE_FLAGS_MIRRORED_Y;
                }
            }
            else if ( rPropVal.Name.equals( sSwitched ) )
            {
                sal_Bool bSwitched;
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
    if ( pDefData ) // now check if we have to default some adjustment values
    {
        // first check if there are adjustment values are to be appended
        sal_Int32 i, nAdjustmentValues = seqAdjustmentValues.getLength();
        sal_Int32 nAdjustmentDefaults = *pDefData++;
        if ( nAdjustmentDefaults > nAdjustmentValues )
        {
            seqAdjustmentValues.realloc( nAdjustmentDefaults );
            for ( i = nAdjustmentValues; i < nAdjustmentDefaults; i++ )
            {
                seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
                seqAdjustmentValues[ i ].State = com::sun::star::beans::PropertyState_DEFAULT_VALUE;
            }
        }
        // check if there are defaulted adjustment values that should be filled the hard coded defaults (pDefValue)
        sal_Int32 nCount = nAdjustmentValues > nAdjustmentDefaults ? nAdjustmentDefaults : nAdjustmentValues;
        for ( i = 0; i < nCount; i++ )
        {
            if ( seqAdjustmentValues[ i ].State != com::sun::star::beans::PropertyState_DIRECT_VALUE )
                seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
        }
    }

    ///////////////
    // Coordsize //
    ///////////////
    const rtl::OUString sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
    const Any* pViewBox = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sViewBox );
    com::sun::star::awt::Rectangle aViewBox;
    if ( pViewBox && (*pViewBox >>= aViewBox ) )
    {
        nCoordWidth = labs( aViewBox.Width );
        nCoordHeight= labs( aViewBox.Height);
    }
    else if ( pDefCustomShape )
    {
        nCoordWidth = pDefCustomShape->nCoordWidth;
        nCoordHeight = pDefCustomShape->nCoordHeight;
    }

    const rtl::OUString sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );

    //////////////////////
    // Path/Coordinates //
    //////////////////////
    const rtl::OUString sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
    if ( pAny )
        *pAny >>= seqCoordinates;
    else if ( pDefCustomShape && pDefCustomShape->nVertices && pDefCustomShape->pVertices )
    {
        sal_Int32 i, nCount = pDefCustomShape->nVertices;
        seqCoordinates.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            SetEnhancedCustomShapeParameter( seqCoordinates[ i ].First, pDefCustomShape->pVertices[ i ].nValA );
            SetEnhancedCustomShapeParameter( seqCoordinates[ i ].Second, pDefCustomShape->pVertices[ i ].nValB );
        }
    }

    /////////////////////
    // Path/GluePoints //
    /////////////////////
    const rtl::OUString sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sGluePoints );
    if ( pAny )
        *pAny >>= seqGluePoints;
    else if ( pDefCustomShape && pDefCustomShape->nGluePoints && pDefCustomShape->pGluePoints )
    {
        sal_Int32 i, nCount = pDefCustomShape->nGluePoints;
        seqGluePoints.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            SetEnhancedCustomShapeParameter( seqGluePoints[ i ].First, pDefCustomShape->pGluePoints[ i ].nValA );
            SetEnhancedCustomShapeParameter( seqGluePoints[ i ].Second, pDefCustomShape->pGluePoints[ i ].nValB );
        }
    }

    ///////////////////
    // Path/Segments //
    ///////////////////
    const rtl::OUString sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sSegments );
    if ( pAny )
        *pAny >>= seqSegments;
    else if ( pDefCustomShape && pDefCustomShape->nElements && pDefCustomShape->pElements )
    {
        sal_Int32 i, nCount = pDefCustomShape->nElements;
        seqSegments.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            EnhancedCustomShapeSegment& rSegInfo = seqSegments[ i ];
            sal_uInt16 nSDat = pDefCustomShape->pElements[ i ];
            switch( nSDat >> 8 )
            {
                case 0x00 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::LINETO;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0x20 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CURVETO;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0x40 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::MOVETO;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0x60 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0x80 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0xa1 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
                    rSegInfo.Count   = ( nSDat & 0xff ) / 3;
                }
                break;
                case 0xa2 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
                    rSegInfo.Count   = ( nSDat & 0xff ) / 3;
                }
                break;
                case 0xa3 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARCTO;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa4 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARC;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa5 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa6 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa7 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0xa8 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0xaa :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xab :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOSTROKE;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                default:
                case 0xf8 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::UNKNOWN;
                    rSegInfo.Count   = nSDat;
                }
                break;
            }
        }
    }

    ///////////////////
    // Path/StretchX //
    ///////////////////
    const rtl::OUString sStretchX( RTL_CONSTASCII_USTRINGPARAM ( "StretchX" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sStretchX );
    if ( pAny )
    {
        sal_Int32 nStretchX;
        if ( *pAny >>= nStretchX )
            nXRef = nStretchX;
    }
    else if ( pDefCustomShape )
        nXRef = pDefCustomShape->nXRef;

    ///////////////////
    // Path/StretchY //
    ///////////////////
    const rtl::OUString sStretchY( RTL_CONSTASCII_USTRINGPARAM ( "StretchY" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sStretchY );
    if ( pAny )
    {
        sal_Int32 nStretchY;
        if ( *pAny >>= nStretchY )
            nYRef = nStretchY;
    }
    else if ( pDefCustomShape )
        nYRef = pDefCustomShape->nYRef;

    /////////////////////
    // Path/TextFrames //
    /////////////////////
    const rtl::OUString sTextFrames( RTL_CONSTASCII_USTRINGPARAM ( "TextFrames" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sTextFrames );
    if ( pAny )
        *pAny >>= seqTextFrames;
    else if ( pDefCustomShape && pDefCustomShape->nTextRect && pDefCustomShape->pTextRect )
    {
        sal_Int32 i, nCount = pDefCustomShape->nTextRect;
        seqTextFrames.realloc( nCount );
        const SvxMSDffTextRectangles* pRectangles = pDefCustomShape->pTextRect;
        for ( i = 0; i < nCount; i++, pRectangles++ )
        {
            SetEnhancedCustomShapeParameter( seqTextFrames[ i ].TopLeft.First,    pRectangles->nPairA.nValA );
            SetEnhancedCustomShapeParameter( seqTextFrames[ i ].TopLeft.Second,   pRectangles->nPairA.nValB );
            SetEnhancedCustomShapeParameter( seqTextFrames[ i ].BottomRight.First,  pRectangles->nPairB.nValA );
            SetEnhancedCustomShapeParameter( seqTextFrames[ i ].BottomRight.Second, pRectangles->nPairB.nValB );
        }
    }

    ///////////////
    // Equations //
    ///////////////
    const rtl::OUString sEquations( RTL_CONSTASCII_USTRINGPARAM( "Equations" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sEquations );
    if ( pAny )
        *pAny >>= seqEquations;
    else if ( pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
    {
        sal_Int32 i, nCount = pDefCustomShape->nCalculation;
        seqEquations.realloc( nCount );

        const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
        for ( i = 0; i < nCount; i++, pData++ )
            seqEquations[ i ] = GetEquation( pData->nFlags, pData->nVal[ 0 ], pData->nVal[ 1 ], pData->nVal[ 2 ] );
    }

    /////////////
    // Handles //
    /////////////
    const rtl::OUString sHandles( RTL_CONSTASCII_USTRINGPARAM( "Handles" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sHandles );
    if ( pAny )
        *pAny >>= seqHandles;
    else if ( pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
    {
        sal_Int32 i, n, nCount = pDefCustomShape->nHandles;
        const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
        seqHandles.realloc( nCount );
        for ( i = 0; i < nCount; i++, pData++ )
        {
            sal_Int32 nPropertiesNeeded = 1;    // position is always needed
            sal_Int32 nFlags = pData->nFlags;
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
            {
                nPropertiesNeeded++;
                if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                {
                    if ( pData->nRangeXMin != 0x80000000 )
                        nPropertiesNeeded++;
                    if ( pData->nRangeXMax != 0x7fffffff )
                        nPropertiesNeeded++;
                }
            }
            else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
            {
                if ( pData->nRangeXMin != 0x80000000 )
                    nPropertiesNeeded++;
                if ( pData->nRangeXMax != 0x7fffffff )
                    nPropertiesNeeded++;
                if ( pData->nRangeYMin != 0x80000000 )
                    nPropertiesNeeded++;
                if ( pData->nRangeYMax != 0x7fffffff )
                    nPropertiesNeeded++;
            }

            n = 0;
            com::sun::star::beans::PropertyValues& rPropValues = seqHandles[ i ];
            rPropValues.realloc( nPropertiesNeeded );

            // POSITION
            {
                const rtl::OUString sPosition( RTL_CONSTASCII_USTRINGPARAM ( "Position" ) );
                ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
                SetEnhancedCustomShapeHandleParameter( aPosition.First, pData->nPositionX, sal_True, sal_True );
                SetEnhancedCustomShapeHandleParameter( aPosition.Second, pData->nPositionY, sal_True, sal_False );
                rPropValues[ n ].Name = sPosition;
                rPropValues[ n++ ].Value <<= aPosition;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
            {
                const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
                sal_Bool bMirroredX = sal_True;
                rPropValues[ n ].Name = sMirroredX;
                rPropValues[ n++ ].Value <<= bMirroredX;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
            {
                const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
                sal_Bool bMirroredY = sal_True;
                rPropValues[ n ].Name = sMirroredY;
                rPropValues[ n++ ].Value <<= bMirroredY;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
            {
                const rtl::OUString sSwitched( RTL_CONSTASCII_USTRINGPARAM ( "Switched" ) );
                sal_Bool bSwitched = sal_True;
                rPropValues[ n ].Name = sSwitched;
                rPropValues[ n++ ].Value <<= bSwitched;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
            {
                const rtl::OUString sPolar( RTL_CONSTASCII_USTRINGPARAM ( "Polar" ) );
                ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aCenter;
                SetEnhancedCustomShapeHandleParameter( aCenter.First,  pData->nCenterX,
                    ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_X_IS_SPECIAL ) != 0, sal_True  );
                SetEnhancedCustomShapeHandleParameter( aCenter.Second, pData->nCenterY,
                    ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_Y_IS_SPECIAL ) != 0, sal_False );
                rPropValues[ n ].Name = sPolar;
                rPropValues[ n++ ].Value <<= aCenter;
                if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                {
                    if ( pData->nRangeXMin != 0x80000000 )
                    {
                        const rtl::OUString sRadiusRangeMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMinimum" ) );
                        ::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                        SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, pData->nRangeXMin,
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, sal_True  );
                        rPropValues[ n ].Name = sRadiusRangeMinimum;
                        rPropValues[ n++ ].Value <<= aRadiusRangeMinimum;
                    }
                    if ( pData->nRangeXMax != 0x7fffffff )
                    {
                        const rtl::OUString sRadiusRangeMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMaximum" ) );
                        ::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                        SetEnhancedCustomShapeHandleParameter( aRadiusRangeMaximum, pData->nRangeXMax,
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, sal_False );
                        rPropValues[ n ].Name = sRadiusRangeMaximum;
                        rPropValues[ n++ ].Value <<= aRadiusRangeMaximum;
                    }
                }
            }
            else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
            {
                if ( pData->nRangeXMin != 0x80000000 )
                {
                    const rtl::OUString sRangeXMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMinimum" ) );
                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMinimum;
                    SetEnhancedCustomShapeHandleParameter( aRangeXMinimum, pData->nRangeXMin,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, sal_True  );
                    rPropValues[ n ].Name = sRangeXMinimum;
                    rPropValues[ n++ ].Value <<= aRangeXMinimum;
                }
                if ( pData->nRangeXMax != 0x7fffffff )
                {
                    const rtl::OUString sRangeXMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMaximum" ) );
                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum;
                    SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, pData->nRangeXMax,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, sal_False );
                    rPropValues[ n ].Name = sRangeXMaximum;
                    rPropValues[ n++ ].Value <<= aRangeXMaximum;
                }
                if ( pData->nRangeYMin != 0x80000000 )
                {
                    const rtl::OUString sRangeYMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMinimum" ) );
                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMinimum;
                    SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, pData->nRangeYMin,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, sal_True );
                    rPropValues[ n ].Name = sRangeYMinimum;
                    rPropValues[ n++ ].Value <<= aRangeYMinimum;
                }
                if ( pData->nRangeYMax != 0x7fffffff )
                {
                    const rtl::OUString sRangeYMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMaximum" ) );
                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMaximum;
                    SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, pData->nRangeYMax,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, sal_False );
                    rPropValues[ n ].Name = sRangeYMaximum;
                    rPropValues[ n++ ].Value <<= aRangeYMaximum;
                }
            }
        }
    }
    return pDefData;
}

EnhancedCustomShape2d::~EnhancedCustomShape2d()
{
}

EnhancedCustomShape2d::EnhancedCustomShape2d( SdrObject* pAObj ) :
    pCustomShapeObj     ( pAObj ),
    SfxItemSet          ( pAObj->GetMergedItemSet() ),
    eSpType             ( mso_sptNil ),
    nXRef               ( 0x80000000 ),
    nYRef               ( 0x80000000 ),
    nCoordWidth         ( 21600 ),
    nCoordHeight        ( 21600 ),
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
    if ( pAny )
        *pAny >>= sShapeType;
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

    const sal_Int32* pDefData = ApplyShapeAttributes( rGeometryItem );
    switch( eSpType )
    {
        case mso_sptCan :                       nColorData = 0x20200000; break;
        case mso_sptCube :                      nColorData = 0x302d0000; break;
        case mso_sptActionButtonBlank :         nColorData = 0x502ad400; break;
        case mso_sptActionButtonHome :          nColorData = 0x702ad4ad; break;
        case mso_sptActionButtonHelp :          nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonInformation :   nColorData = 0x702ad4a5; break;
        case mso_sptActionButtonBackPrevious :  nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonForwardNext :   nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonBeginning :     nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonEnd :           nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonReturn :        nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonDocument :      nColorData = 0x702ad4da; break;
        case mso_sptActionButtonSound :         nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonMovie :         nColorData = 0x602ad4a0; break;
        case mso_sptBevel :                     nColorData = 0x502ad400; break;
        case mso_sptFoldedCorner :              nColorData = 0x20d00000; break;
        case mso_sptSmileyFace :                nColorData = 0x20d00000; break;
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
        {
            if ( ( seqAdjustmentValues.getLength() > 2 ) && ( seqAdjustmentValues[ 2 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE ) )
            {
                double fValue;
                seqAdjustmentValues[ 2 ].Value >>= fValue;
                fValue = 21600 - fValue;
                seqAdjustmentValues[ 2 ].Value <<= fValue;
            }
            nFlags |= DFF_CUSTOMSHAPE_FLIP_H;
        }
        break;
        case mso_sptCurvedUpArrow :
        {
            if ( ( seqAdjustmentValues.getLength() > 2 ) && ( seqAdjustmentValues[ 2 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE ) )
            {
                double fValue;
                seqAdjustmentValues[ 2 ].Value >>= fValue;
                fValue = 21600 - fValue;
                seqAdjustmentValues[ 2 ].Value <<= fValue;
            }
            nFlags |= DFF_CUSTOMSHAPE_FLIP_V | DFF_CUSTOMSHAPE_EXCH;
        }
        break;
        case mso_sptCurvedDownArrow :           nFlags |= DFF_CUSTOMSHAPE_EXCH; break;
        case mso_sptRibbon2 :                   nColorData = 0x30dd0000; break;
        case mso_sptRibbon :
        {
            if ( ( seqAdjustmentValues.getLength() > 1 ) && ( seqAdjustmentValues[ 1 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE ) )
            {
                double fValue;
                seqAdjustmentValues[ 1 ].Value >>= fValue;
                fValue = 21600 - fValue;
                seqAdjustmentValues[ 1 ].Value <<= fValue;
            }
            nFlags |= DFF_CUSTOMSHAPE_FLIP_V;
            nColorData = 0x30dd0000;
        }
        break;
        case mso_sptEllipseRibbon2 :            nColorData = 0x30dd0000; break;
        case mso_sptEllipseRibbon :             // !!!!!!!!!!
        {
            if ( ( seqAdjustmentValues.getLength() > 1 ) && ( seqAdjustmentValues[ 1 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE ) )
            {
                double fValue;
                seqAdjustmentValues[ 1 ].Value >>= fValue;
                fValue = 21600 - fValue;
                seqAdjustmentValues[ 1 ].Value <<= fValue;
            }
            nFlags |= DFF_CUSTOMSHAPE_FLIP_V;
            nColorData = 0x30dd0000;
        }
        break;
        case mso_sptVerticalScroll :            nColorData = 0x30dd0000; break;
        case mso_sptHorizontalScroll :          nColorData = 0x30dd0000; break;
    }
    fXScale = (double)aLogicRect.GetWidth() / (double)nCoordWidth;
    fYScale = (double)aLogicRect.GetHeight() / (double)nCoordHeight;
    if ( nXRef != 0x80000000 )
    {
        fXRatio = (double)aLogicRect.GetWidth() / (double)aLogicRect.GetHeight();
        if ( fXRatio > 1 )
            fXScale /= fXRatio;
        else
            fXRatio = 1.0;
    }
    else
        fXRatio = 1.0;
    if ( nYRef != 0x80000000 )
    {
        fYRatio = (double)aLogicRect.GetHeight() / (double)aLogicRect.GetWidth();
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
                vNodesSharedPtr[ i ] = EnhancedCustomShapeFunctionParser::parseFunction( seqEquations[ i ], *this );
            }
            catch ( ParseError& )
            {
                sal_Bool bUps = sal_True;
            }
        }
    }
}
double EnhancedCustomShape2d::GetEnumFunc( const EnumFunc eFunc ) const
{
    double fRet;
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
            sal_Int32 nNumber;
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
        }
        catch ( ... )
        {
            sal_Bool bUps = sal_True;
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
            double fNumber;
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
            sal_Int32 nAdjustmentIndex;
            if ( rParameter.Value >>= nAdjustmentIndex )
            {
                rRetValue = GetAdjustValueAsDouble( nAdjustmentIndex );
                bRetValue = sal_True;
            }
        }
        break;
        case EnhancedCustomShapeParameterType::EQUATION :
        {
            sal_Int32 nEquationIndex;
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
                sal_Int32 nValue;
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
Color EnhancedCustomShape2d::GetColorData( const Color& rFillColor, sal_uInt32 nIndex )
{
    Color aRetColor;

    sal_uInt32 i, nColor, nTmp, nCount = nColorData >> 28;

    if ( nCount )
    {
        if ( nIndex >= nCount )
            nIndex = nCount - 1;

        sal_uInt32 nFillColor = (sal_uInt32)rFillColor.GetRed() |
                                    ((sal_uInt32)rFillColor.GetGreen() << 8 ) |
                                        ((sal_uInt32)rFillColor.GetBlue() << 16 );

        sal_Int32 nLumDat = nColorData << ( ( 1 + nIndex ) << 2 );
        sal_Int32 nLuminance = ( nLumDat >> 28 ) * 12;

        nTmp = nFillColor;
        nColor = 0;
        for ( i = 0; i < 3; i++ )
        {
            sal_Int32 nC = (sal_uInt8)nTmp;
            nTmp >>= 8;
            nC += ( ( nLuminance * nC ) >> 8 );
            if ( nC < 0 )
                nC = 0;
            else if ( nC &~ 0xff )
                nC = 0xff;
            nColor >>= 8;
            nColor |= nC << 16;
        }
        aRetColor = Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
    }
    return aRetColor;
}

Rectangle EnhancedCustomShape2d::GetTextRect() const
{
    sal_Int32 nIndex, nSize = seqTextFrames.getLength();
    if ( !nSize )
        return aLogicRect;
    nIndex = 0;
    if ( bTextFlow && ( nSize > 1 ) )
        nIndex++;
    Point aTopLeft( GetPoint( seqTextFrames[ nIndex ].TopLeft, sal_True, sal_True ) );
    Point aBottomRight( GetPoint( seqTextFrames[ nIndex ].BottomRight, sal_True, sal_True ) );
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
                rReturnPosition = Point( Round( fX + aReferencePoint.X() ), Round( ( fY * fYScale ) / fXScale + aReferencePoint.Y() ) );
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
            sal_Bool bAdjFirst = aHandle.aPosition.First.Type == EnhancedCustomShapeParameterType::ADJUSTMENT;
            sal_Bool bAdjSecond= aHandle.aPosition.Second.Type == EnhancedCustomShapeParameterType::ADJUSTMENT;
            if ( bAdjFirst || bAdjSecond )
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

                sal_Int32 nFirstAdjustmentValue, nSecondAdjustmentValue;
                aHandle.aPosition.First.Value >>= nFirstAdjustmentValue;
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
                    if ( bAdjFirst )
                        SetAdjustValueAsDouble( fRadius, nFirstAdjustmentValue );
                    if ( bAdjSecond )
                        SetAdjustValueAsDouble( fAngle,  nSecondAdjustmentValue );
                }
                else
                {
                    if ( bAdjFirst )
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
                    if ( bAdjSecond )
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
    }
    return bRetValue;
}

void EnhancedCustomShape2d::SwapStartAndEndArrow( SdrObject* pObj ) //#108274
{
    XLineStartItem       aLineStart;
    aLineStart.SetValue(((XLineStartItem&)pObj->GetMergedItem( XATTR_LINEEND )).GetValue());
    XLineStartWidthItem  aLineStartWidth(((XLineStartWidthItem&)pObj->GetMergedItem( XATTR_LINEENDWIDTH )).GetValue());
    XLineStartCenterItem aLineStartCenter(((XLineStartCenterItem&)pObj->GetMergedItem( XATTR_LINEENDCENTER )).GetValue());

    XLineEndItem         aLineEnd;
    aLineEnd.SetValue(((XLineEndItem&)pObj->GetMergedItem( XATTR_LINESTART )).GetValue());
    XLineEndWidthItem    aLineEndWidth(((XLineEndWidthItem&)pObj->GetMergedItem( XATTR_LINESTARTWIDTH )).GetValue());
    XLineEndCenterItem   aLineEndCenter(((XLineEndCenterItem&)pObj->GetMergedItem( XATTR_LINESTARTCENTER )).GetValue());

    pObj->SetMergedItem( aLineStart );
    pObj->SetMergedItem( aLineStartWidth );
    pObj->SetMergedItem( aLineStartCenter );
    pObj->SetMergedItem( aLineEnd );
    pObj->SetMergedItem( aLineEndWidth );
    pObj->SetMergedItem( aLineEndCenter );
}

void AppendArc( const Rectangle& rRect, const Point& rStart, const Point& rEnd, const sal_Bool bClockwise, XPolygon& rPoly )
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
    Polygon aTempPoly( aRect, aStart, aEnd, POLY_ARC );

    sal_uInt16 j, nDstPt = rPoly.GetPointCount();
    if ( bClockwise )
    {
        for ( j = aTempPoly.GetSize(); j--; )
            rPoly[ nDstPt++ ] = aTempPoly[ j ];
    }
    else
    {
        for ( j = 0; j < aTempPoly.GetSize(); j++ )
            rPoly[ nDstPt++ ] = aTempPoly[ j ];
    }
}

void EnhancedCustomShape2d::CreateSubPath( sal_uInt16& rSrcPt, sal_uInt16& rSegmentInd, std::vector< SdrPathObj* >& rObjectList,
                                                                                        const sal_Bool bLineGeometryNeededOnly,
                                                                                        const sal_Bool bSortFilledObjectsToBack )
{
    SdrObject* pRet = NULL;

    sal_Bool bNoFill = sal_False;
    sal_Bool bNoStroke = sal_False;

    XPolyPolygon    aPolyPoly;
    XPolygon        aPoly;
    XPolygon        aEmptyPoly;

    sal_Int32 nCoordSize = seqCoordinates.getLength();
    sal_Int32 nSegInfoSize = seqSegments.getLength();
    if ( !nSegInfoSize )
    {
        sal_Int32 nPtNum;
        aPoly = XPolygon( (sal_uInt16)( nCoordSize + 1 ) );
        const EnhancedCustomShapeParameterPair* pTmp = seqCoordinates.getArray();
        for ( nPtNum = 0; nPtNum < nCoordSize; nPtNum++ )
            aPoly[ (sal_uInt16)nPtNum ] = GetPoint( *pTmp++, sal_True, sal_True );
        if ( aPoly[ 0 ] != aPoly[ (sal_uInt16)( nPtNum - 1 ) ] )
            aPoly[ (sal_uInt16)nPtNum ] = aPoly[ 0 ];
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
                    if ( aPoly.GetPointCount() > 1 )
                        aPolyPoly.Insert( aPoly );
                    aPoly = aEmptyPoly;
                    if ( rSrcPt < nCoordSize )
                        aPoly[ 0 ] = GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True );
                }
                break;
                case ENDSUBPATH :
                break;
                case CLOSESUBPATH :
                {
                    sal_uInt16 nDstPt = aPoly.GetPointCount();
                    if ( nDstPt )
                    {
                        aPoly[ nDstPt ] = aPoly[ 0 ];
                        if ( aPoly.GetPointCount() > 1 )
                            aPolyPoly.Insert( aPoly );
                        aPoly = aEmptyPoly;
                    }
                }
                break;
                case CURVETO :
                {
                    sal_uInt16 nDstPt = aPoly.GetPointCount();
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 2 ) < nCoordSize ); i++ )
                    {
                        aPoly[ nDstPt ] = GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True );
                        aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                        aPoly[ nDstPt ] = GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True );
                        aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                        aPoly[ nDstPt++ ] = GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True );
                    }
                }
                break;

                case ANGLEELLIPSE :
                {
                    if ( aPoly.GetPointCount() > 1 )
                        aPolyPoly.Insert( aPoly );
                    aPoly = aEmptyPoly;
                }
                case ANGLEELLIPSETO :
                {
                    sal_uInt16 nDstPt = aPoly.GetPointCount();
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 2 ) < nCoordSize ); i++ )
                    {    // create a circle

                        Point aCenter( GetPoint( seqCoordinates[ rSrcPt ], sal_True, sal_True ) );
                        double fWidth, fHeight;
                        GetParameter( fWidth,  seqCoordinates[ rSrcPt + 1 ].First, sal_True, sal_False  );
                        GetParameter( fHeight,  seqCoordinates[ rSrcPt + 1 ].Second, sal_False, sal_True );
                        fWidth *= fXScale;
                        fHeight*= fYScale;
                        Point aP( (sal_Int32)( aCenter.X() - fWidth ), (sal_Int32)( aCenter.Y() - fHeight ) );
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
                                        fEndAngle = fStartAngle;
                                    }
                                }
                                double fCenterX = aRect.Center().X();
                                double fCenterY = aRect.Center().Y();
                                double fx1 = ( cos( fStartAngle * F_PI180 ) * 65536.0 * fXScale ) + fCenterX;
                                double fy1 = ( -sin( fStartAngle * F_PI180 ) * 65536.0 * fYScale ) + fCenterY;
                                double fx2 = ( cos( fEndAngle * F_PI180 ) * 65536.0 * fXScale ) + fCenterX;
                                double fy2 = ( -sin( fEndAngle * F_PI180 ) * 65536.0 * fYScale ) + fCenterY;
                                AppendArc( aRect, Point( (sal_Int32)fx1, (sal_Int32)fy1 ), Point( (sal_Int32)fx2, (sal_Int32)fy2 ), sal_False, aPoly );
                            }
                            else
                            {   /* SJ: TODO: this block should be replaced sometimes, because the current point
                                   is not set correct, it also does not use the correct moveto
                                   point if ANGLEELLIPSETO was used, but the method AppendArc
                                   is at the moment not able to draw full circles (if startangle is 0
                                   and endangle 360 nothing is painted :-( */
                                sal_Int32 nXControl = (sal_Int32)((double)aRect.GetWidth() * 0.2835 );
                                sal_Int32 nYControl = (sal_Int32)((double)aRect.GetHeight() * 0.2835 );
                                Point     aCenter( aRect.Center() );
                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Top() );
                                aPoly[ nDstPt ] = Point( aCenter.X() + nXControl, aRect.Top() );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt ] = Point( aRect.Right(), aCenter.Y() - nYControl );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt++ ] = Point( aRect.Right(), aCenter.Y() );
                                aPoly[ nDstPt ] = Point( aRect.Right(), aCenter.Y() + nYControl );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt ] = Point( aCenter.X() + nXControl, aRect.Bottom() );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Bottom() );
                                aPoly[ nDstPt ] = Point( aCenter.X() - nXControl, aRect.Bottom() );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt ] = Point( aRect.Left(), aCenter.Y() + nYControl );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt++ ] = Point( aRect.Left(), aCenter.Y() );
                                aPoly[ nDstPt ] = Point( aRect.Left(), aCenter.Y() - nYControl );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt ] = Point( aCenter.X() - nXControl, aRect.Top() );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Top() );
                            }
                        }
                        rSrcPt += 3;
                    }
                }
                break;

                case LINETO :
                {
                    sal_uInt16 nDstPt = aPoly.GetPointCount();
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                        aPoly[ nDstPt++ ] = GetPoint( seqCoordinates[ rSrcPt++ ], sal_True, sal_True );
                }
                break;

                case ARC :
                case CLOCKWISEARC :
                {
                    if ( aPoly.GetPointCount() > 1 )
                        aPolyPoly.Insert( aPoly );
                    aPoly = aEmptyPoly;
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
                            double fRatio = (double)aRect.GetHeight() / (double)aRect.GetWidth();
                            aStart.X() = (sal_Int32)( ( (double)( aStart.X() - aCenter.X() ) ) * fRatio ) + aCenter.X();
                            aStart.Y() = (sal_Int32)( ( (double)( aStart.Y() - aCenter.Y() ) ) ) + aCenter.Y();
                            aEnd.X() = (sal_Int32)( ( (double)( aEnd.X() - aCenter.X() ) ) * fRatio ) + aCenter.X();
                            aEnd.Y() = (sal_Int32)( ( (double)( aEnd.Y() - aCenter.Y() ) ) ) + aCenter.Y();
                            AppendArc( aRect, aStart, aEnd, bClockwise, aPoly );
                        }
                        rSrcPt += 4;
                    }
                }
                break;

                case ELLIPTICALQUADRANTX :
                case ELLIPTICALQUADRANTY :
                {
                    BOOL    bFirstDirection;
                    sal_uInt16  nDstPt = aPoly.GetPointCount();
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
                                    bFirstDirection = TRUE;
                                else if ( !bFirstDirection )
                                    nModT ^= 1;
                            }
                            else
                            {
                                if ( !i )
                                    bFirstDirection = FALSE;
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
                            aPoly[ nDstPt ] = aControl1;
                            aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                            nXVec = ( nX - aCurrent.X() ) >> 1;
                            nYVec = ( nY - aCurrent.Y() ) >> 1;
                            Point aControl2( aCurrent.X() + nXVec, aCurrent.Y() + nYVec );
                            aPoly[ nDstPt ] = aControl2;
                            aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                        }
                        aPoly[ nDstPt ] = aCurrent;
                        rSrcPt++;
                        nDstPt++;
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

    if ( aPoly.GetPointCount() > 1 )
        aPolyPoly.Insert( aPoly );
    aPoly = aEmptyPoly;
    if ( aPolyPoly.Count() )
    {
        // #i37011#
        bool bForceCreateTwoObjects(false);
        ::basegfx::B2DPolyPolygon aLocalPolyPolygon(aPolyPoly.getB2DPolyPolygon());

        if(!bSortFilledObjectsToBack && !aLocalPolyPolygon.isClosed() && !bNoStroke)
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
                ::basegfx::B2DPolyPolygon aClosedPolyPolygon(aLocalPolyPolygon);
                aClosedPolyPolygon.setClosed(true);
                SdrPathObj* pFill = new SdrPathObj(OBJ_POLY, XPolyPolygon(aClosedPolyPolygon));
                SfxItemSet aTempSet(*this);
                aTempSet.Put(SdrShadowItem(sal_False));
                aTempSet.Put(XLineStyleItem(XLINE_NONE));
                pFill->SetMergedItemSet(aTempSet);
                rObjectList.push_back(pFill);
            }
            if(!bNoStroke)
            {
                SdrPathObj* pStroke = new SdrPathObj(OBJ_PLIN, XPolyPolygon(aLocalPolyPolygon));
                SfxItemSet aTempSet(*this);
                aTempSet.Put(SdrShadowItem(sal_False));
                aTempSet.Put(XFillStyleItem(XFILL_NONE));
                pStroke->SetMergedItemSet(aTempSet);
                rObjectList.push_back(pStroke);
            }
        }
        else
        {
            SdrObjKind eObjKind(bNoFill ? OBJ_PLIN : OBJ_POLY);
            aLocalPolyPolygon.setClosed(true);
            SdrPathObj* pObj = new SdrPathObj(eObjKind, XPolyPolygon(aLocalPolyPolygon));
            SfxItemSet aTempSet(*this);
            aTempSet.Put(SdrShadowItem(sal_False));

            if(bNoFill)
            {
                aTempSet.Put(XFillStyleItem(XFILL_NONE));
            }

            if(bNoStroke)
            {
                aTempSet.Put(XLineStyleItem(XLINE_NONE));
            }

            pObj->SetMergedItemSet(aTempSet);
            rObjectList.push_back(pObj);
        }
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

    while( nSegmentInd <= seqSegments.getLength() )
    {
        CreateSubPath( nSrcPt, nSegmentInd, vObjectList, bLineGeometryNeededOnly, bSortFilledObjectsToBack );
    }

    SdrObject* pRet = NULL;
    sal_uInt32 i;

    if ( vObjectList.size() )
    {
        const SfxItemSet& rCustomShapeSet = pCustomShapeObj->GetMergedItemSet();
        const sal_Bool  bShadow(((SdrShadowItem&)rCustomShapeSet.Get( SDRATTR_SHADOW )).GetValue());
        Color           aBasicColor( COL_WHITE );
        Color           aFillColor;
        sal_uInt32      nColorCount = nColorData >> 28;
        sal_uInt32      nColorIndex = 0;

        if ( nColorCount )
        {
            aBasicColor = (((XFillColorItem&)rCustomShapeSet.Get( XATTR_FILLCOLOR )).GetValue());
        }

        // #i37011# remove invisible objects
        if(vObjectList.size())
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
            SdrPathObj* pObj(vObjectList[0L]);

            if(bShadow)
            {
                pObj->SetMergedItem(SdrShadowItem(sal_True));
            }

            if(!pObj->IsLine())
            {
                if ( nColorIndex < nColorCount )
                {
                    aFillColor = GetColorData( aBasicColor, nColorIndex++ );
                }

                if ( nColorCount )
                {
                    pObj->SetMergedItem( XFillColorItem( String(), aFillColor ) );
                }
            }
        }
        else
        {
            sal_Bool bContainsLines(sal_False);
            sal_Bool bContainsAreas(sal_False);

            // correct some values and collect content data
            for ( i = 0; i < vObjectList.size(); i++ )
            {
                SdrPathObj* pObj( vObjectList[ i ] );

                if(pObj->IsLine())
                {
                    bContainsLines = sal_True;
                }
                else
                {
                    bContainsAreas = sal_True;

                    if ( nColorIndex < nColorCount )
                    {
                        aFillColor = GetColorData( aBasicColor, nColorIndex++ );
                    }

                    if ( nColorCount )
                    {
                        pObj->SetMergedItem( XFillColorItem( String(), aFillColor ) );
                    }
                }
            }

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
    if(vObjectList.size())
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
// SJ: not setting model, so we save a lot of broadcasting and the model is not modified any longer
//      pRet->SetModel( pCustomShapeObj->GetModel() );
        pRet->SetMergedItemSet( *this );
    }
    else if ( eSpType == mso_sptEllipse )
    {
        pRet = new SdrCircObj( OBJ_CIRC, aLogicRect );
// SJ: not setting model, so we save a lot of broadcasting and the model is not modified any longer
//      pRet->SetModel( pCustomShapeObj->GetModel() );
        pRet->SetMergedItemSet( *this );
    }
/*
    else if ( eSpType == mso_sptArc )
    {   // the arc is something special, because sometimes the snaprect does not match
        Rectangle aPolyBoundRect;
        sal_Int32 nPtNum, nNumElemVert = seqCoordinates.getLength();
        if ( nNumElemVert )
        {
            XPolygon aXP( (sal_uInt16)nNumElemVert );
            const EnhancedCustomShapeParameterPair* pTmp = seqCoordinates.getArray();
            for ( nPtNum = 0; nPtNum < nNumElemVert; nPtNum++ )
                aXP[ (sal_uInt16)nPtNum ] = GetPoint( *pTmp++, sal_False, sal_False );
            aPolyBoundRect = Rectangle( aXP.GetBoundRect() );
        }
        else
            aPolyBoundRect = aLogicRect;
        sal_Int32   nEndAngle = NormAngle360( - GetAdjustValueAsInteger( 0 ) * 100 );
        sal_Int32   nStartAngle = NormAngle360( - GetAdjustValueAsInteger( 1 ) * 100 );

        if ( nStartAngle == nEndAngle )
            return NULL;

        if ( bFilled )      // ( filled ) ? we have to import an pie : we have to construct an arc
        {
            pRet = new SdrCircObj( OBJ_SECT, aPolyBoundRect, nStartAngle, nEndAngle );
            pRet->NbcSetSnapRect( aLogicRect );
// SJ: not setting model, so we save a lot of broadcasting and the model is not modified any longer
//          pRet->SetModel( pCustomShapeObj->GetModel() );
            pRet->SetMergedItemSet( *this );
        }
        else
        {
            Point aStart, aEnd, aCenter( aPolyBoundRect.Center() );
            aStart.X() = (sal_Int32)( ( cos( ( (double)nStartAngle * F_PI18000 ) ) * 1000.0 ) );
            aStart.Y() = - (sal_Int32)( ( sin( ( (double)nStartAngle * F_PI18000 ) ) * 1000.0 ) );
            aEnd.X() = (sal_Int32)( ( cos( ( (double)nEndAngle * F_PI18000 ) ) * 1000.0 ) );
            aEnd.Y() = - (sal_Int32)( ( sin( ( (double)nEndAngle * F_PI18000 ) ) * 1000.0 ) );
            aStart.X() += aCenter.X();
            aStart.Y() += aCenter.Y();
            aEnd.X() += aCenter.X();
            aEnd.Y() += aCenter.Y();

            Polygon aPolygon( aPolyBoundRect, aStart, aEnd, POLY_PIE );
            Rectangle aPolyPieRect( aPolygon.GetBoundRect() );

            USHORT nPt = aPolygon.GetSize();

            if ( nPt < 4 )
                return NULL;

            aPolygon[ 0 ] = aPolygon[ 1 ];                              // try to get the arc boundrect
            aPolygon[ nPt - 1 ] = aPolygon[ nPt - 2 ];
            Rectangle aPolyArcRect( aPolygon.GetBoundRect() );

            double  fYScale, fXScale;
            double  fYOfs, fXOfs;
            int     nCond;

            fYOfs = fXOfs = 0.0;
            if ( aPolyPieRect.GetWidth() != aPolyArcRect.GetWidth() )
            {
                nCond = ( (sal_uInt32)( nStartAngle - 9000 ) > 18000 ) && ( (sal_uInt32)( nEndAngle - 9000 ) > 18000 ) ? 1 : 0;
                nCond ^= bFlipH ? 1 : 0;
                if ( nCond )
                {
                    fXScale = (double)aLogicRect.GetWidth() / (double)aPolyPieRect.GetWidth();
                    fXOfs = ( (double)aPolyPieRect.GetWidth() - (double)aPolyArcRect.GetWidth() ) * fXScale;
                }
            }
            if ( aPolyPieRect.GetHeight() != aPolyArcRect.GetHeight() )
            {
                nCond = ( ( nStartAngle > 18000 ) && ( nEndAngle > 18000 ) ) ? 1 : 0;
                nCond ^= bFlipV ? 1 : 0;
                if ( nCond )
                {
                    fYScale = (double)aLogicRect.GetHeight() / (double)aPolyPieRect.GetHeight();
                    fYOfs = ( (double)aPolyPieRect.GetHeight() - (double)aPolyArcRect.GetHeight() ) * fYScale;
                }
            }
            fXScale = (double)aPolyArcRect.GetWidth() / (double)aPolyPieRect.GetWidth();
            fYScale = (double)aPolyArcRect.GetHeight() / (double)aPolyPieRect.GetHeight();

            aPolyArcRect = Rectangle( Point( aLogicRect.Left() + (sal_Int32)fXOfs, aLogicRect.Top() + (sal_Int32)fYOfs ),
                Size( (sal_Int32)( aLogicRect.GetWidth() * fXScale ), (sal_Int32)( aLogicRect.GetHeight() * fYScale ) ) );

            SdrCircObj* pObjCirc = new SdrCircObj( OBJ_CARC, aPolyBoundRect, nStartAngle, nEndAngle );
            pObjCirc->SetSnapRect( aPolyArcRect );
// SJ: not setting model, so we save a lot of broadcasting and the model is not modified any longer
//          pObjCirc->SetModel( pCustomShapeObj->GetModel() );
            pObjCirc->SetMergedItemSet( *this );

            int nSwap = bFlipH ? 1 : 0;
            nSwap ^= bFlipV ? 1 : 0;
            if ( nSwap )
                SwapStartAndEndArrow( pObjCirc );

            SdrRectObj* pRect = new SdrRectObj( aPolyArcRect );
            pRect->SetSnapRect( aPolyArcRect );
// SJ: not setting model, so we save a lot of broadcasting and the model is not modified any longer
//          pRect->SetModel( pCustomShapeObj->GetModel() );
            pRect->SetMergedItemSet( *this );
            pRect->SetMergedItem( XLineStyleItem( XLINE_NONE ) );
            pRect->SetMergedItem( XFillStyleItem( XFILL_NONE ) );

            pRet = new SdrObjGroup();
// SJ: not setting model, so we save a lot of broadcasting and the model is not modified any longer
//          pRet->SetModel( pCustomShapeObj->GetModel() );
            ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pRect );
            ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pObjCirc );
        }
    }
*/
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
            const Point& rPoint = GetPoint( seqGluePoints[ i ], sal_True, sal_True );
            double fXRel = rPoint.X();
            double fYRel = rPoint.Y();
            fXRel = fXRel / aLogicRect.GetWidth() * 10000;
            fYRel = fYRel / aLogicRect.GetHeight() * 10000;
            aGluePoint.SetPos( Point( (sal_Int32)fXRel, (sal_Int32)fYRel ) );
            aGluePoint.SetPercent( sal_True );
            aGluePoint.SetAlign( SDRVERTALIGN_TOP | SDRHORZALIGN_LEFT );
            aGluePoint.SetEscDir( SDRESC_SMART );
            SdrGluePointList* pList = pObj->ForceGluePointList();
            if( pList )
                sal_uInt16 nId = pList->Insert( aGluePoint );
        }
    }
}

SdrObject* EnhancedCustomShape2d::CreateLineGeometry()
{
    return CreateObject( sal_True );
}


