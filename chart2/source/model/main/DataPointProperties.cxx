/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataPointProperties.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:33:47 $
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
#include "DataPointProperties.hxx"
#include "macros.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"

// #ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
// #include <com/sun/star/awt/Gradient.hpp>
// #endif
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
// #ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
// #include <com/sun/star/drawing/Hatch.hpp>
// #endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_BITMAPMODE_HPP_
#include <com/sun/star/drawing/BitmapMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_RECTANGLEPOINT_HPP_
#include <com/sun/star/drawing/RectanglePoint.hpp>
#endif

// #ifndef _COM_SUN_STAR_CHART2_FILLBITMAP_HPP_
// #include <com/sun/star/chart2/FillBitmap.hpp>
// #endif
#ifndef _COM_SUN_STAR_CHART2_DATAPOINTGEOMETRY3D_HPP_
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATAPOINTLABEL_HPP_
#include <com/sun/star/chart2/DataPointLabel.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void DataPointProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
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
        Property( C2U( "TransparencyGradientName" ),
                  PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "GradientName" ),
                  PROP_DATAPOINT_GRADIENT_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));


    rOutProperties.push_back(
        beans::Property( C2U( "GradientStepCount" ),
                  PROP_DATAPOINT_GRADIENT_STEPCOUNT,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "HatchName" ),
                  PROP_DATAPOINT_HATCH_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapName" ),
                  PROP_DATAPOINT_FILL_BITMAP_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID  ));
    rOutProperties.push_back(
        Property( C2U( "FillBackground" ),
                  PROP_DATAPOINT_FILL_BACKGROUND,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

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
        Property( C2U( "BorderDashName" ),
                  PROP_DATAPOINT_BORDER_DASH_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
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
                  LineProperties::PROP_LINE_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::LineStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineWidth" ),
                  LineProperties::PROP_LINE_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
         Property( C2U( "LineDash" ),
                   LineProperties::PROP_LINE_DASH,
                   ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                   beans::PropertyAttribute::BOUND
                   | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "LineDashName" ),
                  LineProperties::PROP_LINE_DASH_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // FillProperties
    // bitmap properties
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

    // others
    rOutProperties.push_back(
        Property( C2U( "Symbol" ),
                  PROP_DATAPOINT_SYMBOL_PROP,
                  ::getCppuType( reinterpret_cast< const chart2::Symbol * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Offset" ),
                  PROP_DATAPOINT_OFFSET,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Geometry3D" ),
                  PROP_DATAPOINT_GEOMETRY3D,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Label" ),
                  PROP_DATAPOINT_LABEL,
                  ::getCppuType( reinterpret_cast< const chart2::DataPointLabel * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberFormat" ),
                  PROP_DATAPOINT_NUMBER_FORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ReferenceDiagramSize" ),
                  PROP_DATAPOINT_REFERENCE_DIAGRAM_SIZE,
                  ::getCppuType( reinterpret_cast< const awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // statistics
    rOutProperties.push_back(
        Property( C2U( "ErrorBarX" ),
                  PROP_DATAPOINT_ERROR_BAR_X,
                  // XPropertySet supporting service ErrorBar
                  ::getCppuType( reinterpret_cast< const uno::Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "ErrorBarY" ),
                  PROP_DATAPOINT_ERROR_BAR_Y,
                  // XPropertySet supporting service ErrorBar
                  ::getCppuType( reinterpret_cast< const uno::Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "ShowErrorBox" ),
                  PROP_DATAPOINT_SHOW_ERROR_BOX,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "PercentDiagonal" ),
                  PROP_DATAPOINT_PERCENT_DIAGONAL,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void DataPointProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_COLOR ));
    rOutMap[ PROP_DATAPOINT_COLOR ] =
        uno::makeAny( sal_Int32( 0x0099ccff ));  // blue 8

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_TRANSPARENCY ));
    rOutMap[ PROP_DATAPOINT_TRANSPARENCY ] =
        uno::makeAny( sal_Int16( 0 ) );

    //fill
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_FILL_STYLE ));
    rOutMap[ PROP_DATAPOINT_FILL_STYLE ] =
        uno::makeAny( drawing::FillStyle_SOLID );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME ));
    rOutMap[ PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_GRADIENT_NAME ));
    rOutMap[ PROP_DATAPOINT_GRADIENT_NAME ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_GRADIENT_STEPCOUNT ));
    rOutMap[ PROP_DATAPOINT_GRADIENT_STEPCOUNT ] =
        uno::makeAny(sal_Int16(0));
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_HATCH_NAME ));
    rOutMap[ PROP_DATAPOINT_HATCH_NAME ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_FILL_BITMAP_NAME ));
    rOutMap[ PROP_DATAPOINT_FILL_BITMAP_NAME ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_FILL_BACKGROUND ));
    rOutMap[ PROP_DATAPOINT_FILL_BACKGROUND ] =
        uno::makeAny( false );

    //border
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_COLOR ));
    rOutMap[ PROP_DATAPOINT_BORDER_COLOR ] =
        uno::makeAny( sal_Int32( 0x00000000 ));  // black
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_STYLE ));
    rOutMap[ PROP_DATAPOINT_BORDER_STYLE ] =
        uno::makeAny( drawing::LineStyle_SOLID );
//         uno::makeAny( drawing::LineStyle_NONE );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_WIDTH ));
    rOutMap[ PROP_DATAPOINT_BORDER_WIDTH ] =
        uno::makeAny( sal_Int32( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_DASH_NAME ));
    rOutMap[ PROP_DATAPOINT_BORDER_DASH_NAME ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_BORDER_TRANSPARENCY ));
    rOutMap[ PROP_DATAPOINT_BORDER_TRANSPARENCY ] =
        uno::makeAny( sal_Int16( 0 ) );

    //line
    OSL_ASSERT( rOutMap.end() == rOutMap.find( LineProperties::PROP_LINE_STYLE ));
    rOutMap[ LineProperties::PROP_LINE_STYLE ] =
        uno::makeAny( drawing::LineStyle_SOLID );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( LineProperties::PROP_LINE_WIDTH ));
    rOutMap[ LineProperties::PROP_LINE_WIDTH ] =
        uno::makeAny( sal_Int32( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( LineProperties::PROP_LINE_DASH ));
    rOutMap[ LineProperties::PROP_LINE_DASH ] =
        uno::Any( drawing::LineDash() );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( LineProperties::PROP_LINE_DASH_NAME ));
    rOutMap[ LineProperties::PROP_LINE_DASH_NAME ] =
        uno::Any();//need this empty default value otherwise get a costly exception in DataSeries::GetDefaultValue

    //fill
    //bitmap
    uno::Any aSalInt16Zero = uno::makeAny( sal_Int16( 0 ));
    uno::Any aSalInt32SizeDefault = uno::makeAny( sal_Int32( 0 ));

    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_OFFSETX ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_OFFSETX ] = aSalInt16Zero;
    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_OFFSETY ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_OFFSETY ] = aSalInt16Zero;
    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX ] = aSalInt16Zero;
    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY ] = aSalInt16Zero;
    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT ] =
        uno::makeAny( drawing::RectanglePoint_MIDDLE_MIDDLE );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_LOGICALSIZE ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_LOGICALSIZE ] =
        uno::makeAny( true );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_SIZEX ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_SIZEX ] = aSalInt32SizeDefault;
    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_SIZEY ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_SIZEY ] = aSalInt32SizeDefault;
    OSL_ASSERT( rOutMap.end() == rOutMap.find( FillProperties::PROP_FILL_BITMAP_MODE ));
    rOutMap[ FillProperties::PROP_FILL_BITMAP_MODE ] =
        uno::makeAny( drawing::BitmapMode_REPEAT );

    //others
    chart2::Symbol aSymbProp;
    aSymbProp.Style = chart2::SymbolStyle_NONE;
    aSymbProp.StandardSymbol = 0;
    aSymbProp.Size = awt::Size( 250, 250 ); // ca. 7pt x 7pt (7pt=246.94)
    aSymbProp.BorderColor = 0x000000;       // Black
    aSymbProp.FillColor = 0xee4000;         // OrangeRed2
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_SYMBOL_PROP ));
    rOutMap[ PROP_DATAPOINT_SYMBOL_PROP ] =
        uno::makeAny( aSymbProp );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_OFFSET ));
    rOutMap[ PROP_DATAPOINT_OFFSET ] =
        uno::makeAny( double( 0.0 ) );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_GEOMETRY3D ));
    rOutMap[ PROP_DATAPOINT_GEOMETRY3D ] =
        uno::makeAny( chart2::DataPointGeometry3D::CUBOID );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_LABEL ));
    rOutMap[ PROP_DATAPOINT_LABEL ] =
        uno::makeAny( chart2::DataPointLabel(
                          sal_False, // ShowNumber
                          sal_False, // ShowNumberInPercent
                          sal_False, // ShowCategoryName
                          sal_False // ShowLegendSymbol
                          ));

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_NUMBER_FORMAT ));
    rOutMap[ PROP_DATAPOINT_NUMBER_FORMAT ] =
        uno::makeAny( sal_Int32(0) ); //todo maybe choose a different one here -> should be dynamically that of the attached axis

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_ERROR_BAR_X ));
    rOutMap[ PROP_DATAPOINT_ERROR_BAR_X ] =
        uno::makeAny( uno::Reference< beans::XPropertySet >(0) );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_ERROR_BAR_Y ));
    rOutMap[ PROP_DATAPOINT_ERROR_BAR_Y ] =
        uno::makeAny( uno::Reference< beans::XPropertySet >(0) );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATAPOINT_PERCENT_DIAGONAL ));
    rOutMap[ PROP_DATAPOINT_PERCENT_DIAGONAL ] =
        uno::makeAny( static_cast< sal_Int16 >(5) );
}

} //  namespace chart
