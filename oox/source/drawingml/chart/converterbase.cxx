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

#include <drawingml/chart/converterbase.hxx>

#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/diagnose.h>
#include <basegfx/numeric/ftools.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>


namespace oox::drawingml::chart {

namespace cssc = ::com::sun::star::chart;

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::oox::core::XmlFilterBase;

namespace {

struct TitleKey : public ::std::pair< ObjectType, ::std::pair< sal_Int32, sal_Int32 > >
{
    explicit     TitleKey( ObjectType eObjType, sal_Int32 nMainIdx = -1, sal_Int32 nSubIdx = -1 )
                            { first = eObjType; second.first = nMainIdx; second.second = nSubIdx; }
};

/** A helper structure to store all data related to title objects. Needed for
    the conversion of manual title positions that needs the old Chart1 API.
 */
struct TitleLayoutInfo
{
    typedef Reference< XShape > (*GetShapeFunc)( const Reference< cssc::XChartDocument >& );

    Reference< XTitle > mxTitle;        /// The API title object.
    ModelRef< LayoutModel > mxLayout;   /// The layout model, if existing.
    GetShapeFunc        mpGetShape;     /// Helper function to receive the title shape.

    explicit     TitleLayoutInfo() : mpGetShape( nullptr ) {}

    void                convertTitlePos(
                            ConverterRoot const & rRoot,
                            const Reference< cssc::XChartDocument >& rxChart1Doc );
};

void TitleLayoutInfo::convertTitlePos( ConverterRoot const & rRoot, const Reference< cssc::XChartDocument >& rxChart1Doc )
{
    if( !(mxTitle.is() && mpGetShape) )
        return;

    try
    {
        // try to get the title shape
        Reference< XShape > xTitleShape = mpGetShape( rxChart1Doc );
        // get title rotation angle, needed for correction of position of top-left edge
        double fAngle = 0.0;
        PropertySet aTitleProp( mxTitle );
        aTitleProp.getProperty( fAngle, PROP_TextRotation );
        // convert the position
        LayoutModel& rLayout = mxLayout.getOrCreate();
        LayoutConverter aLayoutConv( rRoot, rLayout );
        aLayoutConv.convertFromModel( xTitleShape, fAngle );
    }
    catch( Exception& )
    {
    }
}

/*  The following local functions implement getting the XShape interface of all
    supported title objects (chart and axes). This needs some effort due to the
    design of the old Chart1 API used to access these objects. */

/** A code fragment that returns a shape object from the passed shape supplier
    using the specified interface function. Checks a boolean property first. */
#define OOX_FRAGMENT_GETTITLESHAPE( shape_supplier, supplier_func, property_name ) \
    PropertySet aPropSet( shape_supplier ); \
    if( shape_supplier.is() && aPropSet.getBoolProperty( PROP_##property_name ) ) \
        return shape_supplier->supplier_func(); \
    return Reference< XShape >(); \

/** Implements a function returning the drawing shape of an axis title, if
    existing, using the specified API interface and its function. */
#define OOX_DEFINEFUNC_GETAXISTITLESHAPE( func_name, interface_type, supplier_func, property_name ) \
Reference< XShape > func_name( const Reference< cssc::XChartDocument >& rxChart1Doc ) \
{ \
    Reference< cssc::interface_type > xAxisSupp( rxChart1Doc->getDiagram(), UNO_QUERY ); \
    OOX_FRAGMENT_GETTITLESHAPE( xAxisSupp, supplier_func, property_name ) \
}

/** Returns the drawing shape of the main title, if existing. */
Reference< XShape > lclGetMainTitleShape( const Reference< cssc::XChartDocument >& rxChart1Doc )
{
    OOX_FRAGMENT_GETTITLESHAPE( rxChart1Doc, getTitle, HasMainTitle )
}

OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetXAxisTitleShape, XAxisXSupplier, getXAxisTitle, HasXAxisTitle )
OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetYAxisTitleShape, XAxisYSupplier, getYAxisTitle, HasYAxisTitle )
OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetZAxisTitleShape, XAxisZSupplier, getZAxisTitle, HasZAxisTitle )
OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetSecXAxisTitleShape, XSecondAxisTitleSupplier, getSecondXAxisTitle, HasSecondaryXAxisTitle )
OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetSecYAxisTitleShape, XSecondAxisTitleSupplier, getSecondYAxisTitle, HasSecondaryYAxisTitle )

#undef OOX_DEFINEFUNC_GETAXISTITLESHAPE
#undef OOX_IMPLEMENT_GETTITLESHAPE

} // namespace

struct ConverterData
{
    typedef ::std::map< TitleKey, TitleLayoutInfo > TitleMap;

    ObjectFormatter     maFormatter;
    TitleMap            maTitles;
    XmlFilterBase&      mrFilter;
    ChartConverter&     mrConverter;
    Reference< XChartDocument > mxDoc;
    awt::Size                maSize;

    explicit            ConverterData(
                            XmlFilterBase& rFilter,
                            ChartConverter& rChartConverter,
                            const ChartSpaceModel& rChartModel,
                            const Reference< XChartDocument >& rxChartDoc,
                            const awt::Size& rChartSize );
                        ~ConverterData();
};

ConverterData::ConverterData(
        XmlFilterBase& rFilter,
        ChartConverter& rChartConverter,
        const ChartSpaceModel& rChartModel,
        const Reference< XChartDocument >& rxChartDoc,
        const awt::Size& rChartSize ) :
    maFormatter( rFilter, rxChartDoc, rChartModel ),
    mrFilter( rFilter ),
    mrConverter( rChartConverter ),
    mxDoc( rxChartDoc ),
    maSize( rChartSize )
{
    OSL_ENSURE( mxDoc.is(), "ConverterData::ConverterData - missing chart document" );
    // lock the model to suppress internal updates during conversion
    try
    {
        mxDoc->lockControllers();
    }
    catch( Exception& )
    {
    }

    // prepare conversion of title positions
    maTitles[ TitleKey( OBJECTTYPE_CHARTTITLE ) ].mpGetShape = lclGetMainTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_PRIM_AXESSET, API_X_AXIS ) ].mpGetShape = lclGetXAxisTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_PRIM_AXESSET, API_Y_AXIS ) ].mpGetShape = lclGetYAxisTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_PRIM_AXESSET, API_Z_AXIS ) ].mpGetShape = lclGetZAxisTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_SECN_AXESSET, API_X_AXIS ) ].mpGetShape = lclGetSecXAxisTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_SECN_AXESSET, API_Y_AXIS ) ].mpGetShape = lclGetSecYAxisTitleShape;
}

ConverterData::~ConverterData()
{
    // unlock the model
    try
    {
        mxDoc->unlockControllers();
    }
    catch( Exception& )
    {
    }
}

ConverterRoot::ConverterRoot(
        XmlFilterBase& rFilter,
        ChartConverter& rChartConverter,
        const ChartSpaceModel& rChartModel,
        const Reference< XChartDocument >& rxChartDoc,
        const awt::Size& rChartSize ) :
    mxData( std::make_shared<ConverterData>( rFilter, rChartConverter, rChartModel, rxChartDoc, rChartSize ) )
{
}

ConverterRoot::~ConverterRoot()
{
}

Reference< XInterface > ConverterRoot::createInstance( const OUString& rServiceName ) const
{
    Reference< XInterface > xInt;
    try
    {
        Reference<XMultiServiceFactory> xMSF(getComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW);

        xInt = xMSF->createInstance( rServiceName );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xInt.is(), "ConverterRoot::createInstance - cannot create instance" );
    return xInt;
}

Reference< XComponentContext > const & ConverterRoot::getComponentContext() const
{
    return mxData->mrFilter.getComponentContext();
}

XmlFilterBase& ConverterRoot::getFilter() const
{
    return mxData->mrFilter;
}

ChartConverter& ConverterRoot::getChartConverter() const
{
    return mxData->mrConverter;
}

Reference< XChartDocument > const & ConverterRoot::getChartDocument() const
{
    return mxData->mxDoc;
}

const awt::Size& ConverterRoot::getChartSize() const
{
    return mxData->maSize;
}

ObjectFormatter& ConverterRoot::getFormatter() const
{
    return mxData->maFormatter;
}

void ConverterRoot::registerTitleLayout( const Reference< XTitle >& rxTitle,
        const ModelRef< LayoutModel >& rxLayout, ObjectType eObjType, sal_Int32 nMainIdx, sal_Int32 nSubIdx )
{
    OSL_ENSURE( rxTitle.is(), "ConverterRoot::registerTitleLayout - missing title object" );
    TitleLayoutInfo& rTitleInfo = mxData->maTitles[ TitleKey( eObjType, nMainIdx, nSubIdx ) ];
    OSL_ENSURE( rTitleInfo.mpGetShape, "ConverterRoot::registerTitleLayout - invalid title key" );
    rTitleInfo.mxTitle = rxTitle;
    rTitleInfo.mxLayout = rxLayout;
}

void ConverterRoot::convertTitlePositions()
{
    try
    {
        Reference< cssc::XChartDocument > xChart1Doc( mxData->mxDoc, UNO_QUERY_THROW );
        for (auto & title : mxData->maTitles)
            title.second.convertTitlePos( *this, xChart1Doc );
    }
    catch( Exception& )
    {
    }
}

namespace {

/** Returns a position value in the chart area in 1/100 mm. */
sal_Int32 lclCalcPosition( sal_Int32 nChartSize, double fPos, sal_Int32 nPosMode )
{
    switch( nPosMode )
    {
        case XML_edge:      // absolute start position as factor of chart size
            return getLimitedValue< sal_Int32, double >( nChartSize * fPos + 0.5, 0, nChartSize );
        case XML_factor:    // position relative to object default position
            OSL_FAIL( "lclCalcPosition - relative positioning not supported" );
            return -1;
    };

    OSL_FAIL( "lclCalcPosition - unknown positioning mode" );
    return -1;
}

/** Returns a size value in the chart area in 1/100 mm. */
sal_Int32 lclCalcSize( sal_Int32 nPos, sal_Int32 nChartSize, double fSize, sal_Int32 nSizeMode )
{
    sal_Int32 nValue = getLimitedValue< sal_Int32, double >( nChartSize * fSize + 0.5, 0, nChartSize );
    switch( nSizeMode )
    {
        case XML_factor:    // passed value is width/height
            return nValue;
        case XML_edge:      // passed value is right/bottom position
            return nValue - nPos + 1;
    };

    OSL_FAIL( "lclCalcSize - unknown size mode" );
    return -1;
}

/** Returns a relative size value in the chart area. */
double lclCalcRelSize( double fPos, double fSize, sal_Int32 nSizeMode )
{
    switch( nSizeMode )
    {
        case XML_factor:    // passed value is width/height
        break;
        case XML_edge:      // passed value is right/bottom position
            fSize -= fPos;
        break;
        default:
            OSL_ENSURE( false, "lclCalcRelSize - unknown size mode" );
            fSize = 0.0;
    };
    return getLimitedValue< double, double >( fSize, 0.0, 1.0 - fPos );
}

} // namespace

LayoutConverter::LayoutConverter( const ConverterRoot& rParent, LayoutModel& rModel ) :
    ConverterBase< LayoutModel >( rParent, rModel )
{
}

LayoutConverter::~LayoutConverter()
{
}

bool LayoutConverter::calcAbsRectangle( awt::Rectangle& orRect ) const
{
    if( !mrModel.mbAutoLayout )
    {
        awt::Size aChartSize = getChartSize();
        if( aChartSize.Width <= 0 || aChartSize.Height <= 0 )
        {
            aChartSize = getDefaultPageSize();
        }
        orRect.X = lclCalcPosition( aChartSize.Width,  mrModel.mfX, mrModel.mnXMode );
        orRect.Y = lclCalcPosition( aChartSize.Height, mrModel.mfY, mrModel.mnYMode );
        if( (orRect.X >= 0) && (orRect.Y >= 0) )
        {
            orRect.Width  = lclCalcSize( orRect.X, aChartSize.Width,  mrModel.mfW, mrModel.mnWMode );
            orRect.Height = lclCalcSize( orRect.Y, aChartSize.Height, mrModel.mfH, mrModel.mnHMode );
            return (orRect.Width > 0) && (orRect.Height > 0);
        }
    }
    return false;
}

bool LayoutConverter::convertFromModel( PropertySet& rPropSet )
{
    if( !mrModel.mbAutoLayout &&
        (mrModel.mnXMode == XML_edge) && (mrModel.mfX >= 0.0) &&
        (mrModel.mnYMode == XML_edge) && (mrModel.mfY >= 0.0) )
    {
        RelativePosition aPos(
            getLimitedValue< double, double >( mrModel.mfX, 0.0, 1.0 ),
            getLimitedValue< double, double >( mrModel.mfY, 0.0, 1.0 ),
            Alignment_TOP_LEFT );
        rPropSet.setProperty( PROP_RelativePosition, aPos );

        RelativeSize aSize(
            lclCalcRelSize( aPos.Primary, mrModel.mfW, mrModel.mnWMode ),
            lclCalcRelSize( aPos.Secondary, mrModel.mfH, mrModel.mnHMode ) );
        if( (aSize.Primary > 0.0) && (aSize.Secondary > 0.0) )
        {
            rPropSet.setProperty( PROP_RelativeSize, aSize );
            return true;
        }
    }
    return false;
}

void LayoutConverter::convertFromModel( const Reference< XShape >& rxShape, double fRotationAngle )
{
    if( mrModel.mbAutoLayout )
        return;

    awt::Size aChartSize = getChartSize();
    if( aChartSize.Width <= 0 || aChartSize.Height <= 0 )
    {
        aChartSize = getDefaultPageSize();
    }
    awt::Point aShapePos(
        lclCalcPosition( aChartSize.Width,  mrModel.mfX, mrModel.mnXMode ),
        lclCalcPosition( aChartSize.Height, mrModel.mfY, mrModel.mnYMode ) );
    if( (aShapePos.X < 0) || (aShapePos.Y < 0) )
        return;

    bool bPropSet = false;
    // the call to XShape.getSize() may recalc the chart view
    awt::Size aShapeSize = rxShape->getSize();
    // rotated shapes need special handling...
    if( aShapeSize.Height > 0 || aShapeSize.Width > 0 )
    {
        double fSin = fabs(sin(basegfx::deg2rad(fRotationAngle)));
        // add part of height to X direction, if title is rotated down
        if( fRotationAngle > 180.0 )
            aShapePos.X += static_cast<sal_Int32>(fSin * aShapeSize.Height + 0.5);
        // add part of width to Y direction, if title is rotated up
        else if( fRotationAngle > 0.0 )
            aShapePos.Y += static_cast<sal_Int32>(fSin * aShapeSize.Width + 0.5);
    }
    else if( fRotationAngle == 90.0 || fRotationAngle == 270.0 )
    {
        PropertySet aShapeProp( rxShape );
        RelativePosition aPos(
            getLimitedValue< double, double >(mrModel.mfX, 0.0, 1.0),
            getLimitedValue< double, double >(mrModel.mfY, 0.0, 1.0),
            fRotationAngle == 90.0 ? Alignment_TOP_RIGHT : Alignment_BOTTOM_LEFT );
        // set the resulting position at the shape
        if( aShapeProp.setProperty(PROP_RelativePosition, aPos) )
            bPropSet = true;
    }
    // set the resulting position at the shape
    if( !bPropSet )
        rxShape->setPosition( aShapePos );
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
