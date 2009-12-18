/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: converterbase.cxx,v $
 *
 * $Revision: 1.4.6.1 $
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

#include "oox/drawingml/chart/converterbase.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <tools/solar.h>    // for F_PI180
#include "properties.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/theme.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::Size;
using ::com::sun::star::chart2::RelativePosition;
using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::drawing::XShape;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct ConverterData
{
    ObjectFormatter     maFormatter;
    XmlFilterBase&      mrFilter;
    ChartConverter&     mrConverter;
    Reference< XChartDocument > mxDoc;
    Size                maSize;

    explicit            ConverterData(
                            XmlFilterBase& rFilter,
                            ChartConverter& rChartConverter,
                            const ChartSpaceModel& rChartModel,
                            const Reference< XChartDocument >& rxChartDoc,
                            const Size& rChartSize );
                        ~ConverterData();
};

// ----------------------------------------------------------------------------

ConverterData::ConverterData(
        XmlFilterBase& rFilter,
        ChartConverter& rChartConverter,
        const ChartSpaceModel& rChartModel,
        const Reference< XChartDocument >& rxChartDoc,
        const Size& rChartSize ) :
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
        Reference< XModel > xModel( mxDoc, UNO_QUERY_THROW );
        xModel->lockControllers();
    }
    catch( Exception& )
    {
    }
}

ConverterData::~ConverterData()
{
    // unlock the model
    try
    {
        Reference< XModel > xModel( mxDoc, UNO_QUERY_THROW );
        xModel->unlockControllers();
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

ConverterRoot::ConverterRoot(
        XmlFilterBase& rFilter,
        ChartConverter& rChartConverter,
        const ChartSpaceModel& rChartModel,
        const Reference< XChartDocument >& rxChartDoc,
        const Size& rChartSize ) :
    mxData( new ConverterData( rFilter, rChartConverter, rChartModel, rxChartDoc, rChartSize ) )
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
        xInt = mxData->mrFilter.getGlobalFactory()->createInstance( rServiceName );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xInt.is(), "ConverterRoot::createInstance - cannot create instance" );
    return xInt;
}

XmlFilterBase& ConverterRoot::getFilter() const
{
    return mxData->mrFilter;
}

ChartConverter& ConverterRoot::getChartConverter() const
{
    return mxData->mrConverter;
}

Reference< XChartDocument > ConverterRoot::getChartDocument() const
{
    return mxData->mxDoc;
}

const Size& ConverterRoot::getChartSize() const
{
    return mxData->maSize;
}

ObjectFormatter& ConverterRoot::getFormatter() const
{
    return mxData->maFormatter;
}

// ============================================================================

namespace {

/** Returns a position value in the chart area in 1/100 mm. */
sal_Int32 lclCalcPosition( sal_Int32 nChartSize, double fPos, sal_Int32 nPosMode )
{
    switch( nPosMode )
    {
        case XML_edge:      // absolute start position as factor of chart size
            return getLimitedValue< sal_Int32, double >( nChartSize * fPos + 0.5, 0, nChartSize );
        case XML_factor:    // position relative to object default position
            OSL_ENSURE( false, "lclCalcPosition - relative positioning not supported" );
            return -1;
    };

    OSL_ENSURE( false, "lclCalcPosition - unknown positioning mode" );
    return -1;
}

/** Returns a size value in the chart area in 1/100 mm. */
sal_Int32 lclCalcSize( sal_Int32 nPos, sal_Int32 nChartSize, double fSize, sal_Int32 nSizeMode )
{
    sal_Int32 nValue = getLimitedValue< sal_Int32, double >( nChartSize * fSize + 0.5, 0, nChartSize );
    switch( nSizeMode )
    {
        case XML_factor:    // size as factor of chart size
            return nValue;
        case XML_edge:      // absolute end position as factor of chart size
            return nValue - nPos + 1;
    };

    OSL_ENSURE( false, "lclCalcSize - unknown size mode" );
    return -1;
}

} // namespace

// ----------------------------------------------------------------------------

LayoutConverter::LayoutConverter( const ConverterRoot& rParent, LayoutModel& rModel ) :
    ConverterBase< LayoutModel >( rParent, rModel )
{
}

LayoutConverter::~LayoutConverter()
{
}

bool LayoutConverter::calcAbsRectangle( Rectangle& orRect ) const
{
    if( !mrModel.mbAutoLayout )
    {
        const Size& rChartSize = getChartSize();
        orRect.X = lclCalcPosition( rChartSize.Width,  mrModel.mfX, mrModel.mnXMode );
        orRect.Y = lclCalcPosition( rChartSize.Height, mrModel.mfY, mrModel.mnYMode );
        if( (orRect.X >= 0) && (orRect.Y >= 0) )
        {
            orRect.Width  = lclCalcSize( orRect.X, rChartSize.Width,  mrModel.mfW, mrModel.mnWMode );
            orRect.Height = lclCalcSize( orRect.Y, rChartSize.Height, mrModel.mfH, mrModel.mnHMode );
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
        RelativePosition aPos;
        aPos.Primary = getLimitedValue< double, double >( mrModel.mfX, 0.0, 1.0 );
        aPos.Secondary = getLimitedValue< double, double >( mrModel.mfY, 0.0, 1.0 );
        aPos.Anchor = ::com::sun::star::drawing::Alignment_TOP_LEFT;
        rPropSet.setProperty( PROP_RelativePosition, aPos );
        return true;
    }
    return false;
}

bool LayoutConverter::convertFromModel( const Reference< XShape >& rxShape, double fRotationAngle )
{
    if( !mrModel.mbAutoLayout )
    {
        const Size& rChartSize = getChartSize();
        Point aShapePos(
            lclCalcPosition( rChartSize.Width,  mrModel.mfX, mrModel.mnXMode ),
            lclCalcPosition( rChartSize.Height, mrModel.mfY, mrModel.mnYMode ) );
        if( (aShapePos.X >= 0) && (aShapePos.Y >= 0) )
        {
            // the call to XShape.getSize() may recalc the chart view
            Size aShapeSize = rxShape->getSize();
            // rotated shapes need special handling...
            double fSin = fabs( sin( fRotationAngle * F_PI180 ) );
            // add part of height to X direction, if title is rotated down
            if( fRotationAngle > 180.0 )
                aShapePos.X += static_cast< sal_Int32 >( fSin * aShapeSize.Height + 0.5 );
            // add part of width to Y direction, if title is rotated up
            else if( fRotationAngle > 0.0 )
                aShapePos.Y += static_cast< sal_Int32 >( fSin * aShapeSize.Width + 0.5 );
            // set the resulting position at the shape
            rxShape->setPosition( aShapePos );
            return true;
        }
    }
    return false;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

