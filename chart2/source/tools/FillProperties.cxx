/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FillProperties.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:08:17 $
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
#include "FillProperties.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_BITMAPMODE_HPP_
#include <com/sun/star/drawing/BitmapMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_RECTANGLEPOINT_HPP_
#include <com/sun/star/drawing/RectanglePoint.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

namespace
{

void lcl_AddPropertiesToVector_without_BitmapProperties( ::std::vector< ::com::sun::star::beans::Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "FillStyle" ),
                  FillProperties::PROP_FILL_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::FillStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillColor" ),
                  FillProperties::PROP_FILL_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillTransparence" ),
                  FillProperties::PROP_FILL_TRANSPARENCE,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillTransparenceGradientName" ),
                  FillProperties::PROP_FILL_TRANSPARENCE_GRADIENT_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //optional
//    rOutProperties.push_back(
//        Property( C2U( "FillTransparenceGradient" ),
//                  FillProperties::PROP_FILL_TRANSPARENCE_GRADIENT,
//                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
//                  beans::PropertyAttribute::BOUND
//                  | beans::PropertyAttribute::MAYBEDEFAULT
//                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "FillGradientName" ),
                  FillProperties::PROP_FILL_GRADIENT_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        beans::Property( C2U( "FillGradientStepCount" ),
                  FillProperties::PROP_FILL_GRADIENT_STEPCOUNT,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //optional
//    rOutProperties.push_back(
//        Property( C2U( "FillGradient" ),
//                  FillProperties::PROP_FILL_GRADIENT,
//                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
//                  beans::PropertyAttribute::BOUND
//                  | beans::PropertyAttribute::MAYBEDEFAULT
//                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "FillHatchName" ),
                  FillProperties::PROP_FILL_HATCH_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //optional
//    rOutProperties.push_back(
//        Property( C2U( "FillHatch" ),
//                  FillProperties::PROP_FILL_HATCH,
//                  ::getCppuType( reinterpret_cast< const drawing::Hatch * >(0)),
//                  beans::PropertyAttribute::BOUND
//                  | beans::PropertyAttribute::MAYBEDEFAULT
//                  | beans::PropertyAttribute::MAYBEVOID ));

    //bitmap properties see lcl_AddPropertiesToVector_only_BitmapProperties()

    rOutProperties.push_back(
        Property( C2U( "FillBackground" ),
                  FillProperties::PROP_FILL_BACKGROUND,
                  ::getCppuType( reinterpret_cast< const sal_Bool * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

//static
void lcl_AddPropertiesToVector_only_BitmapProperties( ::std::vector< ::com::sun::star::beans::Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "FillBitmapName" ),
                  FillProperties::PROP_FILL_BITMAP_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //optional
//    rOutProperties.push_back(
//        Property( C2U( "FillBitmap" ),
//                  FillProperties::PROP_FILL_BITMAP,
//                  ::getCppuType( reinterpret_cast< const uno::Reference< awt::XBitmap > * >(0)),
//                  beans::PropertyAttribute::BOUND
//                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //optional
//    rOutProperties.push_back(
//        Property( C2U( "FillBitmapURL" ),
//                  FillProperties::PROP_FILL_BITMAP_URL,
//                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
//                  beans::PropertyAttribute::BOUND
//                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapOffsetX" ),
                  FillProperties::PROP_FILL_BITMAP_OFFSETX,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapOffsetY" ),
                  FillProperties::PROP_FILL_BITMAP_OFFSETY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapPositionOffsetX" ),
                  FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapPositionOffsetY" ),
                  FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));


    rOutProperties.push_back(
        Property( C2U( "FillBitmapRectanglePoint" ),
                  FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT,
                  ::getCppuType( reinterpret_cast< const drawing::RectanglePoint * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapLogicalSize" ),
                  FillProperties::PROP_FILL_BITMAP_LOGICALSIZE,
                  ::getCppuType( reinterpret_cast< const sal_Bool * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapSizeX" ),
                  FillProperties::PROP_FILL_BITMAP_SIZEX,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapSizeY" ),
                  FillProperties::PROP_FILL_BITMAP_SIZEY,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapMode" ),
                  FillProperties::PROP_FILL_BITMAP_MODE,
                  ::getCppuType( reinterpret_cast< const drawing::BitmapMode * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}


void lcl_AddDefaultsToMap_without_BitmapProperties(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_STYLE, drawing::FillStyle_SOLID );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, FillProperties::PROP_FILL_COLOR, 0xd9d9d9 ); // gray85
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_TRANSPARENCE, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_GRADIENT_STEPCOUNT, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BACKGROUND, false );
}

void lcl_AddDefaultsToMap_only_BitmapProperties(
    ::chart::tPropertyValueMap & rOutMap )
{
    uno::Any aSalInt16Zero = uno::makeAny( sal_Int16( 0 ));
    uno::Any aSalInt32SizeDefault = uno::makeAny( sal_Int32( 0 ));

    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_OFFSETX, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_OFFSETY, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY, 0 );

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT, drawing::RectanglePoint_MIDDLE_MIDDLE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_LOGICALSIZE, true );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, FillProperties::PROP_FILL_BITMAP_SIZEX, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, FillProperties::PROP_FILL_BITMAP_SIZEY, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_MODE, drawing::BitmapMode_REPEAT );
}

}//end anonymous namespace

void FillProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    // Fill Properties see service drawing::FillProperties
    // ---------------
    lcl_AddPropertiesToVector_without_BitmapProperties( rOutProperties );
    lcl_AddPropertiesToVector_only_BitmapProperties( rOutProperties );
}

void FillProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    lcl_AddDefaultsToMap_without_BitmapProperties( rOutMap );
    lcl_AddDefaultsToMap_only_BitmapProperties( rOutMap );
}

} //  namespace chart
