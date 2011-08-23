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

#include "mapprov.hxx"
#include "schattr.hxx"

#include "app.hrc"			// for SID_TEXTBREAK
#include <bf_svx/svxids.hrc>	// for SID_ATTR_NUMBERFORMAT_SOURCE

// for SdrObject
// for SdrObjList
// for SdrObjListIter
// header for E3dExtrudeObj
// for SVX_UNOEDIT_CHAR_PROPERTIES, SVX_UNOEDIT_FONT_PROPERTIES
#include <bf_svx/unoshprp.hxx>

#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#include <com/sun/star/chart/ChartRegressionCurveType.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartSeriesAddress.hpp>

#include <com/sun/star/drawing/XShapes.hpp>
#include "unonames.hxx"
namespace binfilter {

using namespace ::com::sun::star;

// ----------------------------------
// create one global instance for use
// from all uno implemetations
// ----------------------------------
SchUnoPropertyMapProvider aSchMapProvider;


// ----------------------
// shared property ranges
// ----------------------

#define SCH_DATA_DESCR_PROPERTIES \
    { MAP_CHAR_LEN( UNONAME_DATACAPTION ),			SCHATTR_DATADESCR_DESCR,		&::getCppuType( (sal_Int32*)0 ),	0,	0 }

// attention: x and y grids are interchanged
#define SCH_SHOW_HIDE_PROPERTIES \
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_2X_AXIS ),	CHATTR_AXISGRID_SHOW_2X_AXIS,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_2X_DESCR ),	CHATTR_AXISGRID_SHOW_2X_DESCR,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_2Y_AXIS ),	CHATTR_AXISGRID_SHOW_2Y_AXIS,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_2Y_DESCR ),	CHATTR_AXISGRID_SHOW_2Y_DESCR,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_X_AXIS ),		CHATTR_AXISGRID_SHOW_X_AXIS,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_X_DESCR ),	CHATTR_AXISGRID_SHOW_X_DESCR,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_X_HELP ),		CHATTR_AXISGRID_SHOW_Y_HELP,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_X_MAIN ),		CHATTR_AXISGRID_SHOW_Y_MAIN,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_Y_AXIS ),		CHATTR_AXISGRID_SHOW_Y_AXIS,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_Y_DESCR ),	CHATTR_AXISGRID_SHOW_Y_DESCR,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_Y_HELP ),		CHATTR_AXISGRID_SHOW_X_HELP,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_Y_MAIN ),		CHATTR_AXISGRID_SHOW_X_MAIN,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_Z_AXIS ),		CHATTR_AXISGRID_SHOW_Z_AXIS,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_Z_DESCR ),	CHATTR_AXISGRID_SHOW_Z_DESCR,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_Z_HELP ),		CHATTR_AXISGRID_SHOW_Z_HELP,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_AXISGRID_SHOW_Z_MAIN ),		CHATTR_AXISGRID_SHOW_Z_MAIN,	&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_TITLE_SHOW_X_AXIS ),		CHATTR_TITLE_SHOW_X_AXIS,		&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_TITLE_SHOW_Y_AXIS ),		CHATTR_TITLE_SHOW_Y_AXIS,		&::getBooleanCppuType(),	0,	0 },\
    { MAP_CHAR_LEN( UNONAME_TITLE_SHOW_Z_AXIS ),		CHATTR_TITLE_SHOW_Z_AXIS,		&::getBooleanCppuType(),	0,	0 }

#define SCH_STAT_PROPERTIES \
    { MAP_CHAR_LEN( UNONAME_STAT_AVERAGE ),				SCHATTR_STAT_AVERAGE,			&::getBooleanCppuType(),			0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STAT_BIGERROR ),			SCHATTR_STAT_BIGERROR,			&::getCppuType((const double*)0),	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STAT_CONSTMINUS ),			SCHATTR_STAT_CONSTMINUS,		&::getCppuType((const double*)0),	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STAT_CONSTPLUS ),			SCHATTR_STAT_CONSTPLUS,			&::getCppuType((const double*)0),	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STAT_INDICATE ),			SCHATTR_STAT_INDICATE,			&::getCppuType((const chart::ChartErrorIndicatorType*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STAT_KIND_ERROR ),			SCHATTR_STAT_KIND_ERROR,		&::getCppuType((const chart::ChartErrorCategory*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STAT_PERCENT ),				SCHATTR_STAT_PERCENT,			&::getCppuType((const double*)0),	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STAT_REGRESSTYPE ),			SCHATTR_STAT_REGRESSTYPE,		&::getCppuType((const chart::ChartRegressionCurveType*)0), 0, 0 }

#define SCH_SUBTYPE_PROPERTIES \
    { MAP_CHAR_LEN( UNONAME_NUM_OF_LINES_FOR_BAR ),		CHATTR_NUM_OF_LINES_FOR_BAR,	&::getCppuType((const sal_Int32*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STOCK_UPDOWN ),				SCHATTR_STOCK_UPDOWN,			&::getBooleanCppuType(),	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STOCK_VOLUME ),				SCHATTR_STOCK_VOLUME,			&::getBooleanCppuType(),	0, 0 },\
      { MAP_CHAR_LEN( UNONAME_STYLE_3D ),					SCHATTR_STYLE_3D,				&::getBooleanCppuType(),  	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_DEEP ),				SCHATTR_STYLE_DEEP,				&::getBooleanCppuType() ,	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_LINES ),				SCHATTR_STYLE_LINES,			&::getBooleanCppuType(),  	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_PERCENT ),			SCHATTR_STYLE_PERCENT,			&::getBooleanCppuType() ,	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_SHAPE ),				SCHATTR_STYLE_SHAPE,			&::getCppuType((const sal_Int32*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_SPLINES ),			SCHATTR_STYLE_SPLINES,			&::getCppuType((const sal_Int32*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_STACKED ),			SCHATTR_STYLE_STACKED,			&::getBooleanCppuType()  ,	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_SYMBOL ),				SCHATTR_STYLE_SYMBOL,			&::getCppuType((const sal_Int32*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_SYMBOL_BITMAP_URL ),		SCHATTR_SYMBOL_BRUSH,			&::getCppuType((const ::rtl::OUString*)0),	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_SYMBOL_SIZE ),              SCHATTR_SYMBOL_SIZE,            &::getCppuType((const awt::Size*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_VERTICAL ),			SCHATTR_STYLE_VERTICAL,			&::getBooleanCppuType(),	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_SPLINE_ORDER ),				CHATTR_SPLINE_ORDER,			&::getCppuType((const sal_Int32*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_SPLINE_RESOLUTION ),		CHATTR_SPLINE_RESOLUTION,		&::getCppuType((const sal_Int32*)0), 0, 0 }


#define SCH_TEXT2_PROPERTIES \
    { MAP_CHAR_LEN( UNONAME_TEXT_DEGREES ),		SCHATTR_TEXT_DEGREES,	&::getCppuType((const sal_Int32*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_TEXT_OVERLAP ),		SCHATTR_TEXT_OVERLAP,	&::getBooleanCppuType(),	0, 0 }


#define	SCH_GFX_PROPERTIES \
    { MAP_CHAR_LEN( UNONAME_STYLE_SHAPE ),		SCHATTR_STYLE_SHAPE,	&::getCppuType((const sal_Int32*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_STYLE_SYMBOL ),		SCHATTR_STYLE_SYMBOL,	&::getCppuType((const sal_Int32*)0), 0, 0 },\
    { MAP_CHAR_LEN( UNONAME_SYMBOL_BITMAP_URL ),SCHATTR_SYMBOL_BRUSH,	&::getCppuType((const ::rtl::OUString*)0),	0, 0 },\
    { MAP_CHAR_LEN( UNONAME_SYMBOL_SIZE ),      SCHATTR_SYMBOL_SIZE,    &::getCppuType((const awt::Size*)0), 0, 0 }

#define SCH_ALIEN_PROPERTIES \
    { MAP_CHAR_LEN( UNONAME_USER_DEF_ATTR ), SCHATTR_USER_DEFINED_ATTR, &::getCppuType((uno::Reference< container::XNameContainer >*)0), 0, 0 }

#define SCH_MAP_END { 0,0,0,0,0 }

// ---------------
// map definitions
// ---------------

// see SchUnoPropertyMapProvider::GetMap()

// ------------------------------------------------------------
//
// Implementation for SchUnoPropertyMapProvider
//
// ------------------------------------------------------------

SchUnoPropertyMapProvider::SchUnoPropertyMapProvider()
{
    for( long i = 0; i < CHMAP_END; i++ )
        pMapArr[i] = 0;
}

SchUnoPropertyMapProvider::~SchUnoPropertyMapProvider()
{
    for( long i = 0; i < CHMAP_END; i++ )
        if( pMapArr[i] )
            delete pMapArr[i];
}

// -------------- copied from SwUnoPropertyMapProvider --------------
EXTERN_C
#ifdef WNT
int _cdecl
#else
int
#endif
Sch_CompareMap( const void* pSmaller, const void* pBigger )
{
    int nDiff = strcmp( ((const SfxItemPropertyMap*)pSmaller)->pName,
                        ((const SfxItemPropertyMap*)pBigger)->pName );
    return nDiff;
}

void SchUnoPropertyMapProvider::Sort(short nId)
{
    SfxItemPropertyMap* pTemp = pMapArr[ nId ];
    short i = 0;
    while( pTemp[i].pName )
        i++;

    qsort( pMapArr[ nId ], i, sizeof(SfxItemPropertyMap), Sch_CompareMap );
}

SfxItemPropertyMap* SchUnoPropertyMapProvider::CopyMap( const SfxItemPropertyMap* pMap1 )
{
    long nCnt = 0;

    while( pMap1[ nCnt ].pName )
        nCnt++;

    nCnt++;

    SfxItemPropertyMap* pNewMap = new SfxItemPropertyMap[ nCnt ];
    memcpy( pNewMap, pMap1, sizeof(SfxItemPropertyMap) * nCnt );
    return pNewMap;
}

SfxItemPropertyMap* SchUnoPropertyMapProvider::GetMap( short nPropertyId, ChartModel *pModel )
{
    // ---------------
    // map definitions
    // ---------------

    static SfxItemPropertyMap aEmptyMap[] = { SCH_MAP_END };

    // ChXChartDocument
    static SfxItemPropertyMap aChartDocumentPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN( UNONAME_HASLEGEND ),			CHATTR_DIAGRAM_START,	&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_TITLE_SHOW_MAIN ),		CHATTR_TITLE_SHOW_MAIN,	&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_TITLE_SHOW_SUB ),		CHATTR_TITLE_SHOW_SUB,	&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_CATEGORIES_RANGE_ADDRESS ), CHATTR_ADDR_CATEGORIES,	&::getCppuType((const ::rtl::OUString*)0),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_SERIES_ADDRESSES ),		CHATTR_ADDR_SERIES,		&::getCppuType(( uno::Sequence< chart::ChartSeriesAddress >*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_CHART_RANGE_ADDRESS ),	CHATTR_ADDR_CHART,      &::getCppuType((const ::rtl::OUString*)0),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_TABLE_NUMBER_LIST ),	CHATTR_TABLE_NUMBER_LIST, &::getCppuType((const ::rtl::OUString*)0),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_EXPORT_DATA ),          CHATTR_EXPORT_TABLE,    &::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_FIRST_COL_LABELS ),     CHATTR_FIRST_COL_LABELS,&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_FIRST_ROW_LABELS ),     CHATTR_FIRST_ROW_LABELS,&::getBooleanCppuType(),	0, 0 },
//  		{ MAP_CHAR_LEN( UNONAME_MAINTITLE_ADDRESS ), 	CHATTR_ADDR_MAIN_TITLE,	&::getCppuType((const ::rtl::OUString*)0),	0, 0 },
//  		{ MAP_CHAR_LEN( UNONAME_SUBTITLE_ADDRESS ),		CHATTR_ADDR_SUB_TITLE,	&::getCppuType((const ::rtl::OUString*)0),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_ADDITIONAL_SHAPES ),    CHATTR_ADDITIONAL_SHAPES,   &::getCppuType((const uno::Reference< drawing::XShapes >*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_HAS_TRANSLATED_COLS ),  CHATTR_HAS_TRANSLATED_COLS, &::getBooleanCppuType(),	beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN( UNONAME_HAS_TRANSLATED_ROWS ),  CHATTR_HAS_TRANSLATED_ROWS, &::getBooleanCppuType(),	beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN( UNONAME_TRANSLATED_COLS ),      CHATTR_TRANSLATED_COLS,     &::getCppuType(( uno::Sequence< sal_Int32 >*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_TRANSLATED_ROWS ),      CHATTR_TRANSLATED_ROWS,     &::getCppuType(( uno::Sequence< sal_Int32 >*)0), 0, 0 },
        SCH_ALIEN_PROPERTIES,
        SCH_MAP_END
    };

    static SfxItemPropertyMap aChartTitleShapePropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        { MAP_CHAR_LEN( UNONAME_TEXT_DEGREES ),		SCHATTR_TEXT_DEGREES,	&::getCppuType((const sal_Int32*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_TEXT_STRING ),		CHATTR_TITLE_MAIN,		&::getCppuType((const ::rtl::OUString*)0), 0, 0 },	// CHATTR_TEXT_TITLE_MAIN is just a 'placeholder' to select the uno property 'String' from the which id
        { MAP_CHAR_LEN( UNONAME_STACKED_TEXT ),     SCHATTR_TEXT_ORIENT,    &::getBooleanCppuType(),    0, 0 },
        SCH_ALIEN_PROPERTIES,
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SCH_MAP_END
    };

    static SfxItemPropertyMap aChartLegendPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        { MAP_CHAR_LEN( UNONAME_CHARTLEGENDALIGN ),	SCHATTR_LEGEND_POS,		&::getCppuType((const chart::ChartLegendPosition*)0), 0, 0 },
        SCH_ALIEN_PROPERTIES,
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SCH_MAP_END
    };

    // ChXDiagram
    static SfxItemPropertyMap aChartPropertyMap_Impl[] =
    {
        SCH_STAT_PROPERTIES,
        SCH_SHOW_HIDE_PROPERTIES,
        SCH_SUBTYPE_PROPERTIES,
        SCH_DATA_DESCR_PROPERTIES,
        { MAP_CHAR_LEN( UNONAME_DATAROWSOURCE ),		CHATTR_DATA_SWITCH,		&::getCppuType((const chart::ChartDataRowSource*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_STACKCONNECTORS ),		CHATTR_BARCONNECT,		&::getBooleanCppuType(),  0, 0 },
        { MAP_CHAR_LEN( UNONAME_ATTRIBUTED_DATA_POINTS ),CHATTR_ATTRIBUTED_DATA_POINTS, &::getCppuType(( uno::Sequence< uno::Sequence< sal_Int32 > >*)0), beans::PropertyAttribute::READONLY, 0},
        SCH_ALIEN_PROPERTIES,
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SPECIAL_3DSCENEOBJECT_PROPERTIES
        SCH_MAP_END
    };

    static SfxItemPropertyMap aChartAxisStylePropertyMap_Impl[] =
    {
        SCH_TEXT2_PROPERTIES,
        SVX_UNOEDIT_CHAR_PROPERTIES,
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        { MAP_CHAR_LEN( UNONAME_AXIS_AUTO_MAX ),			SCHATTR_AXIS_AUTO_MAX,			&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_AUTO_MIN ),			SCHATTR_AXIS_AUTO_MIN,			&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_AUTO_ORIGIN ),			SCHATTR_AXIS_AUTO_ORIGIN,		&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_AUTO_STEP_HELP ),		SCHATTR_AXIS_AUTO_STEP_HELP,	&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_AUTO_STEP_MAIN ),		SCHATTR_AXIS_AUTO_STEP_MAIN,	&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_HELPTICKS ),			SCHATTR_AXIS_HELPTICKS,			&::getCppuType((const sal_Int32*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_LOGARITHM ),			SCHATTR_AXIS_LOGARITHM,			&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_MAX ),					SCHATTR_AXIS_MAX,				&::getCppuType((const double*)0),	 0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_MIN ),					SCHATTR_AXIS_MIN,				&::getCppuType((const double*)0),	 0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_ORIGIN ),				SCHATTR_AXIS_ORIGIN,			&::getCppuType((const double*)0),	 0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_STEP_HELP ),			SCHATTR_AXIS_STEP_HELP,			&::getCppuType((const double*)0),	 0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_STEP_MAIN ),			SCHATTR_AXIS_STEP_MAIN,			&::getCppuType((const double*)0),	 0, 0 },
        { MAP_CHAR_LEN( UNONAME_AXIS_TICKS ),				SCHATTR_AXIS_TICKS,				&::getCppuType((const sal_Int32*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_DIAGRAM_GAPWIDTH ),			SCHATTR_BAR_GAPWIDTH,			&::getCppuType((const sal_Int32*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_DIAGRAM_OVERLAP ),			SCHATTR_BAR_OVERLAP,			&::getCppuType((const sal_Int32*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_NUMBERFORMAT ),				SCHATTR_AXIS_NUMFMT,			&::getCppuType((const sal_Int32*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_TEXTBREAK ),				SID_TEXTBREAK,					&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_TEXT_ORDER ),				SCHATTR_TEXT_ORDER,				&::getCppuType((const chart::ChartAxisArrangeOrderType*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_DISPLAY_LABELS ),			SCHATTR_AXIS_SHOWDESCR,			&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_VISIBLE ),					SCHATTR_AXIS_SHOWAXIS,			&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_LINK_NUMFMT ),              SID_ATTR_NUMBERFORMAT_SOURCE,	&::getBooleanCppuType(),	0, 0 },
        { MAP_CHAR_LEN( UNONAME_STACKED_TEXT ),             SCHATTR_TEXT_ORIENT,            &::getBooleanCppuType(),    0, 0 },
        SCH_ALIEN_PROPERTIES,
        SCH_MAP_END
    };

    static SfxItemPropertyMap aChartDataRowPropertyMap_Impl[] =
    {
        SCH_GFX_PROPERTIES,
        SCH_DATA_DESCR_PROPERTIES,
        SCH_STAT_PROPERTIES,
        SVX_UNOEDIT_CHAR_PROPERTIES,
        { MAP_CHAR_LEN( UNONAME_AXIS ),					SCHATTR_AXIS,					&::getCppuType((const sal_Int32*)0), 0, 0 },
        { MAP_CHAR_LEN( UNONAME_REGRESSION_PROPS ),		CHATTR_REGRESSION_PROPS,		&::getCppuType((const uno::Reference< beans::XPropertySet >*)0), beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN( UNONAME_ERROR_PROPS ),			CHATTR_ERROR_PROPS,				&::getCppuType((const uno::Reference< beans::XPropertySet >*)0), beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN( UNONAME_AVERAGE_PROPS ),		CHATTR_AVERAGE_PROPS,			&::getCppuType((const uno::Reference< beans::XPropertySet >*)0), beans::PropertyAttribute::READONLY, 0},
        SCH_ALIEN_PROPERTIES,
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        MISC_3D_OBJ_PROPERTIES
        SPECIAL_3DEXTRUDEOBJECT_PROPERTIES
        SPECIAL_3DLATHEOBJECT_PROPERTIES
        SPECIAL_3DPOLYGONOBJECT_PROPERTIES
        SPECIAL_3DBACKSCALE_PROPERTIES
        SCH_MAP_END
    };

    static SfxItemPropertyMap aChartDataPointPropertyMap_Impl[] =
    {
        SCH_GFX_PROPERTIES,
        SCH_DATA_DESCR_PROPERTIES,
        SVX_UNOEDIT_CHAR_PROPERTIES,
        { MAP_CHAR_LEN( UNONAME_PIE_SEGMENT_OFFSET ),	CHATTR_PIE_SEGMENT_OFFSET,		&::getCppuType((const sal_Int32*)0), 0, 0 },
        SCH_ALIEN_PROPERTIES,
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        MISC_3D_OBJ_PROPERTIES
        SPECIAL_3DEXTRUDEOBJECT_PROPERTIES
        SPECIAL_3DLATHEOBJECT_PROPERTIES
        SPECIAL_3DPOLYGONOBJECT_PROPERTIES
        SPECIAL_3DBACKSCALE_PROPERTIES
        SCH_MAP_END
    };

    static SfxItemPropertyMap aFilledShapePropertyMap_Impl[] =
    {
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SCH_ALIEN_PROPERTIES,
        SCH_MAP_END
    };

    static SfxItemPropertyMap aLineShapePropertyMap_Impl[] =
    {
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SCH_ALIEN_PROPERTIES,
        SCH_MAP_END
    };

    // ------------
    // program code
    // ------------

    if( nPropertyId >= CHMAP_END )
        DBG_ERROR2( "Invalid Id %d (>=%d)", nPropertyId, CHMAP_END );

    if( ! pMapArr[ nPropertyId ] )
    {
        SfxItemPropertyMap** pNewMap = &(pMapArr[ nPropertyId ]);
        switch( nPropertyId )
        {
            case CHMAP_CHART:
                *pNewMap = CopyMap( aChartPropertyMap_Impl );
                break;
            case  CHMAP_AREA:
                *pNewMap = CopyMap( aFilledShapePropertyMap_Impl );
                break;
            case  CHMAP_LINE:
                *pNewMap = CopyMap( aLineShapePropertyMap_Impl );
                break;
            case CHMAP_DOC:
                *pNewMap = CopyMap( aChartDocumentPropertyMap_Impl );
                break;
            case CHMAP_AXIS:
                *pNewMap = CopyMap( aChartAxisStylePropertyMap_Impl );
                break;
            case CHMAP_LEGEND:
                *pNewMap = CopyMap( aChartLegendPropertyMap_Impl );
                break;
            case CHMAP_TITLE:
                *pNewMap = CopyMap( aChartTitleShapePropertyMap_Impl );
                break;
            case CHMAP_DATAROW:
                *pNewMap = CopyMap( aChartDataRowPropertyMap_Impl );
                break;
            case CHMAP_DATAPOINT:
                *pNewMap = CopyMap( aChartDataPointPropertyMap_Impl );
                break;
            case CHMAP_NONE:
            default:
                *pNewMap = CopyMap( aEmptyMap );
                DBG_WARNING( "Using empty PropertyMap!" );
                break;
        }
        Sort( nPropertyId );
    }

    return pMapArr[ nPropertyId ];
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
