/*************************************************************************
 *
 *  $RCSfile: DataPointProperties.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-13 09:40:42 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "DataPointProperties.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_FILLBITMAP_HPP_
#include <drafts/com/sun/star/chart2/FillBitmap.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_TRANSPARENCYSTYLE_HPP_
#include <drafts/com/sun/star/chart2/TransparencyStyle.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_DATACAPTIONSTYLE_HPP_
#include <drafts/com/sun/star/chart2/DataCaptionStyle.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_NUMBERFORMAT_HPP_
#include <drafts/com/sun/star/chart2/NumberFormat.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_SYMBOLPROPERTIES_HPP_
#include <drafts/com/sun/star/chart2/SymbolProperties.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void DataPointProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties,
    bool bIncludeStyleProperties /* = false */ )
{
    if( bIncludeStyleProperties )
    {
        rOutProperties.push_back(
            Property( C2U( "Style" ),
                      PROP_DATAPOINT_STYLE,
                      ::getCppuType( reinterpret_cast< const uno::Reference< ::com::sun::star::style::XStyle > * >(0)),
                      beans::PropertyAttribute::BOUND
                      | beans::PropertyAttribute::MAYBEVOID ));
    }

    // DataPointProperties
    // ===================

    // Common
    // ------
    rOutProperties.push_back(
        Property( C2U( "Color" ),
                  PROP_DATAPOINT_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Transparency" ),
                  PROP_DATAPOINT_TRANSPARENCY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // Fill Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "FillStyle" ),
                  PROP_DATAPOINT_FILL_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::FillStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TransparencyStyle" ),
                  PROP_DATAPOINT_TRANSPARENCY_STYLE,
                  ::getCppuType( reinterpret_cast< const chart2::TransparencyStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "TransparencyGradient" ),
                  PROP_DATAPOINT_TRANSPARENCY_GRADIENT,
                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "Gradient" ),
                  PROP_DATAPOINT_GRADIENT,
                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "Hatch" ),
                  PROP_DATAPOINT_HATCH,
                  ::getCppuType( reinterpret_cast< const drawing::Hatch * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "Bitmap" ),
                  PROP_DATAPOINT_BITMAP,
                  ::getCppuType( reinterpret_cast< const chart2::FillBitmap * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID  ));

    // border for filled objects
    rOutProperties.push_back(
        Property( C2U( "BorderColor" ),
                  PROP_DATAPOINT_BORDER_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "BorderStyle" ),
                  PROP_DATAPOINT_BORDER_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::LineStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "BorderWidth" ),
                  PROP_DATAPOINT_BORDER_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "BorderDash" ),
                  PROP_DATAPOINT_BORDER_DASH,
                  ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "BorderTransparency" ),
                  PROP_DATAPOINT_BORDER_TRANSPARENCY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // Line Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "LineStyle" ),
                  PROP_DATAPOINT_LINE_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::LineStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineWidth" ),
                  PROP_DATAPOINT_LINE_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineDash" ),
                  PROP_DATAPOINT_LINE_DASH,
                  ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // others
    rOutProperties.push_back(
        Property( C2U( "SymbolProperties" ),
                  PROP_DATAPOINT_SYMBOL_PROP,
                  ::getCppuType( reinterpret_cast< const chart2::SymbolProperties * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "Offset" ),
                  PROP_DATAPOINT_OFFSET,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "DataCaption" ),
                  PROP_DATAPOINT_DATA_CAPTION,
                  ::getCppuType( reinterpret_cast< const chart2::DataCaptionStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberFormat" ),
                  PROP_DATAPOINT_NUMBER_FORMAT,
                  ::getCppuType( reinterpret_cast< const chart2::NumberFormat * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void DataPointProperties::AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap,
    bool bIncludeStyleProperties /* = false */ )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_COLOR ));
    rOutMap[ PROP_DATAPOINT_COLOR ] =
        uno::makeAny( sal_Int32( 0x0000b8ff ));  // blue 7

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_TRANSPARENCY ));
    rOutMap[ PROP_DATAPOINT_TRANSPARENCY ] =
        uno::makeAny( sal_Int16( 0 ) );

    //fill
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_FILL_STYLE ));
    rOutMap[ PROP_DATAPOINT_FILL_STYLE ] =
        uno::makeAny( drawing::FillStyle_SOLID );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_TRANSPARENCY_STYLE ));
    rOutMap[ PROP_DATAPOINT_TRANSPARENCY_STYLE ] =
        uno::makeAny( chart2::TransparencyStyle_NONE );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_TRANSPARENCY_GRADIENT ));
    rOutMap[ PROP_DATAPOINT_TRANSPARENCY_GRADIENT ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_GRADIENT ));
    rOutMap[ PROP_DATAPOINT_GRADIENT ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_HATCH ));
    rOutMap[ PROP_DATAPOINT_HATCH ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BITMAP ));
    rOutMap[ PROP_DATAPOINT_BITMAP ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue

    //border
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_COLOR ));
    rOutMap[ PROP_DATAPOINT_BORDER_COLOR ] =
        uno::makeAny( sal_Int32( 0x00000000 ));  // black
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_STYLE ));
    rOutMap[ PROP_DATAPOINT_BORDER_STYLE ] =
        uno::makeAny( drawing::LineStyle_SOLID );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_WIDTH ));
    rOutMap[ PROP_DATAPOINT_BORDER_WIDTH ] =
        uno::makeAny( sal_Int32( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_DASH ));
    rOutMap[ PROP_DATAPOINT_BORDER_DASH ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_TRANSPARENCY ));
    rOutMap[ PROP_DATAPOINT_BORDER_TRANSPARENCY ] =
        uno::makeAny( sal_Int16( 0 ) );

    //line
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_LINE_STYLE ));
    rOutMap[ PROP_DATAPOINT_LINE_STYLE ] =
        uno::makeAny( drawing::LineStyle_SOLID );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_LINE_WIDTH ));
    rOutMap[ PROP_DATAPOINT_LINE_WIDTH ] =
        uno::makeAny( sal_Int32( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_LINE_DASH ));
    rOutMap[ PROP_DATAPOINT_LINE_DASH ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue


    //others
    chart2::SymbolProperties aSymbProp;
    aSymbProp.aStyle = chart2::SymbolStyle_NONE;
    aSymbProp.nStandardSymbol = 0;
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_SYMBOL_PROP ));
    rOutMap[ PROP_DATAPOINT_SYMBOL_PROP ] =
        uno::makeAny( aSymbProp );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_OFFSET ));
    rOutMap[ PROP_DATAPOINT_OFFSET ] =
        uno::makeAny( double( 0.0 ) );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_DATA_CAPTION ));
    rOutMap[ PROP_DATAPOINT_DATA_CAPTION ] =
        uno::makeAny( chart2::DataCaptionStyle(
                          sal_False, // ShowNumber
                          sal_False, // ShowNumberInPercent
                          sal_False, // ShowCategoryName
                          sal_False // ShowLegendSymbol
                          ));

    chart2::NumberFormat aFormat(
        C2U( "Standard" ),
        lang::Locale( C2U( "DE" ), C2U( "de" ), ::rtl::OUString()));
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_NUMBER_FORMAT ));
    rOutMap[ PROP_DATAPOINT_NUMBER_FORMAT ] =
        uno::makeAny( aFormat );
}

} //  namespace chart
