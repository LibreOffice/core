/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedIgnoreProperty.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:06:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedIgnoreProperty.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_BITMAPMODE_HPP_
#include <com/sun/star/drawing/BitmapMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_RECTANGLEPOINT_HPP_
#include <com/sun/star/drawing/RectanglePoint.hpp>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

//.............................................................................
namespace chart
{
//.............................................................................


WrappedIgnoreProperty::WrappedIgnoreProperty( const OUString& rOuterName, const Any& rDefaultValue )
                         : WrappedProperty( rOuterName, OUString() )
                         , m_aDefaultValue( rDefaultValue )
{
}
WrappedIgnoreProperty::~WrappedIgnoreProperty()
{
}

void WrappedIgnoreProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
}

Any WrappedIgnoreProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return getPropertyDefault(0);
}

void WrappedIgnoreProperty::setPropertyToDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
}

Any WrappedIgnoreProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return m_aDefaultValue;
}

beans::PropertyState WrappedIgnoreProperty::getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    return beans::PropertyState_DEFAULT_VALUE;
}

//static
void WrappedIgnoreProperties::addIgnoreLineProperties( std::vector< WrappedProperty* >& rList )
{
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_SOLID ) ) );
//     rList.push_back( new WrappedIgnoreProperty( C2U( "LineDash" ), uno::makeAny( drawing::LineDash() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineDashName" ), uno::makeAny( rtl::OUString() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineColor" ), uno::makeAny( sal_Int32(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineTransparence" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineWidth" ), uno::makeAny( sal_Int32(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "LineJoint" ), uno::makeAny( drawing::LineJoint_NONE ) ) );
}

//static
void WrappedIgnoreProperties::addIgnoreFillProperties( std::vector< WrappedProperty* >& rList )
{
    addIgnoreFillProperties_without_BitmapProperties( rList );
    addIgnoreFillProperties_only_BitmapProperties( rList );
}

//static
void WrappedIgnoreProperties::addIgnoreFillProperties_without_BitmapProperties( ::std::vector< WrappedProperty* >& rList )
{
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillStyle" ), uno::makeAny( drawing::FillStyle_SOLID ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillColor" ), uno::makeAny( sal_Int32(-1) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillTransparence" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillTransparenceGradientName" ), uno::makeAny( ::rtl::OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillTransparenceGradient" ), uno::makeAny( awt::Gradient() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillGradientName" ), uno::makeAny( ::rtl::OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillGradient" ), uno::makeAny( awt::Gradient() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillHatchName" ), uno::makeAny( ::rtl::OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillHatch" ), uno::makeAny( drawing::Hatch() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBackground" ), uno::makeAny( sal_Bool(sal_False) ) ) );
}

//static
void WrappedIgnoreProperties::addIgnoreFillProperties_only_BitmapProperties( ::std::vector< WrappedProperty* >& rList )
{
//     rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapName" ), uno::makeAny( ::rtl::OUString() ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmap" ), uno::makeAny( uno::Reference< awt::XBitmap > (0) ) ) );
//    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapURL" ), uno::makeAny( ::rtl::OUString() ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapOffsetX" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapOffsetY" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapPositionOffsetX" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapPositionOffsetY" ), uno::makeAny( sal_Int16(0) ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapRectanglePoint" ), uno::makeAny( drawing::RectanglePoint_LEFT_TOP ) ) );
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapLogicalSize" ), uno::makeAny( sal_Bool(sal_False) ) ) );//todo correct default?
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapSizeX" ), uno::makeAny( sal_Int32(10) ) ) );//todo which default?
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapSizeY" ), uno::makeAny( sal_Int32(10) ) ) );//todo which default?
    rList.push_back( new WrappedIgnoreProperty( C2U( "FillBitmapMode" ), uno::makeAny( drawing::BitmapMode_REPEAT ) ) );
}

//.............................................................................
} //namespace chart
//.............................................................................
