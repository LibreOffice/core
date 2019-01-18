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

#include <VLegendSymbolFactory.hxx>
#include <PropertyMapper.hxx>
#include <ShapeFactory.hxx>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace
{

void getPropNamesAndValues( const Reference< beans::XPropertySet >& xProp,
        ::chart::tNameSequence& rNames,
        ::chart::tAnySequence& rValues,
        ::chart::VLegendSymbolFactory::PropertyType ePropertyType,
        const awt::Size& aMaxSymbolExtent)
{
    const ::chart::tPropertyNameMap & aFilledSeriesNameMap( ::chart::PropertyMapper::getPropertyNameMapForFilledSeriesProperties());
    const ::chart::tPropertyNameMap & aLineSeriesNameMap( ::chart::PropertyMapper::getPropertyNameMapForLineSeriesProperties());
    const ::chart::tPropertyNameMap & aLineNameMap( ::chart::PropertyMapper::getPropertyNameMapForLineProperties());

    ::chart::tPropertyNameValueMap aValueMap;
    switch( ePropertyType )
    {
        case ::chart::VLegendSymbolFactory::PropertyType::FilledSeries:
            ::chart::PropertyMapper::getValueMap( aValueMap, aFilledSeriesNameMap, xProp );
            break;
        case ::chart::VLegendSymbolFactory::PropertyType::LineSeries:
            ::chart::PropertyMapper::getValueMap( aValueMap, aLineSeriesNameMap, xProp );
            break;
        case ::chart::VLegendSymbolFactory::PropertyType::Line:
            ::chart::PropertyMapper::getValueMap( aValueMap, aLineNameMap, xProp );
            break;
    }

    ::chart::PropertyMapper::getMultiPropertyListsFromValueMap( rNames, rValues, aValueMap );

    uno::Any* pLineWidthAny = ::chart::PropertyMapper::getValuePointer(rValues,rNames,"LineWidth");
    sal_Int32 nLineWidth = 0;
    if( pLineWidthAny && (*pLineWidthAny>>=nLineWidth) )
    {
        // use legend entry height as upper limit for line width
        sal_Int32 nMaxLineWidthForLegend = aMaxSymbolExtent.Height;
        if( nLineWidth>nMaxLineWidthForLegend )
            *pLineWidthAny <<= nMaxLineWidthForLegend;
    }
}

void lcl_setPropertiesToShape(
    const Reference< beans::XPropertySet > & xProp,
    const Reference< drawing::XShape > & xShape,
    ::chart::VLegendSymbolFactory::PropertyType ePropertyType,
    const awt::Size& aMaxSymbolExtent)
{
    ::chart::tNameSequence aPropNames;
    ::chart::tAnySequence aPropValues;
    getPropNamesAndValues( xProp, aPropNames, aPropValues,
            ePropertyType, aMaxSymbolExtent );

    Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
    ::chart::PropertyMapper::setMultiProperties( aPropNames, aPropValues, xShapeProp );
}

} // anonymous namespace

namespace chart
{

Reference< drawing::XShape > VLegendSymbolFactory::createSymbol(
    const awt::Size& rEntryKeyAspectRatio,
    const Reference< drawing::XShapes >& rSymbolContainer,
    LegendSymbolStyle eStyle,
    const Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const Reference< beans::XPropertySet > & xLegendEntryProperties,
    PropertyType ePropertyType, const uno::Any& rExplicitSymbol )
{
    Reference< drawing::XShape > xResult;

    if( ! (rSymbolContainer.is() && xShapeFactory.is()))
        return xResult;

    ShapeFactory* pShapeFactory = ShapeFactory::getOrCreateShapeFactory(xShapeFactory);
    xResult.set( pShapeFactory->createGroup2D( rSymbolContainer ), uno::UNO_QUERY );

    Reference< drawing::XShapes > xResultGroup( xResult, uno::UNO_QUERY );
    if( ! xResultGroup.is())
        return xResult;

    // add an invisible square box to maintain aspect ratio
    Reference< drawing::XShape > xBound( pShapeFactory->createInvisibleRectangle(
                xResultGroup, rEntryKeyAspectRatio  ));

    // create symbol
    try
    {
        if( eStyle == LegendSymbolStyle::Line )
        {
            Reference< drawing::XShape > xLine =
                pShapeFactory->createLine( xResultGroup, awt::Size( rEntryKeyAspectRatio.Width, 0 ),
                        awt::Point( 0, rEntryKeyAspectRatio.Height/2 ));
            if( xLine.is())
            {
                lcl_setPropertiesToShape( xLegendEntryProperties, xLine, ePropertyType, rEntryKeyAspectRatio );
            }

            Reference< drawing::XShape > xSymbol;
            const sal_Int32 nSize = std::min(rEntryKeyAspectRatio.Width,rEntryKeyAspectRatio.Height);
            chart2::Symbol aSymbol;
            if( rExplicitSymbol >>= aSymbol )
            {
                drawing::Direction3D aSymbolSize( nSize, nSize, 0 );
                drawing::Position3D aPos( rEntryKeyAspectRatio.Width/2.0, rEntryKeyAspectRatio.Height/2.0, 0 );
                ShapeFactory* pFactory = ShapeFactory::getOrCreateShapeFactory( xShapeFactory );
                if( aSymbol.Style == chart2::SymbolStyle_STANDARD )
                {
                    // take series color as fill color
                    xLegendEntryProperties->getPropertyValue( "Color") >>= aSymbol.FillColor;
                    // border of symbols always same as fill color
                    aSymbol.BorderColor = aSymbol.FillColor;

                    xSymbol.set( pFactory->createSymbol2D(
                                     xResultGroup,
                                     aPos,
                                     aSymbolSize,
                                     aSymbol.StandardSymbol,
                                     aSymbol.BorderColor,
                                     aSymbol.FillColor ));
                }
                else if( aSymbol.Style == chart2::SymbolStyle_GRAPHIC )
                {
                    xSymbol.set( pFactory->createGraphic2D(
                                     xResultGroup,
                                     aPos,
                                     aSymbolSize,
                                     aSymbol.Graphic ));
                }
                else if( aSymbol.Style == chart2::SymbolStyle_AUTO )
                {
                    SAL_WARN("chart2", "the given parameter is not allowed to contain an automatic symbol style");
                }
            }
        }
        else if( eStyle == LegendSymbolStyle::Circle )
        {
            sal_Int32 nSize = std::min( rEntryKeyAspectRatio.Width, rEntryKeyAspectRatio.Height );
            Reference< drawing::XShape > xShape =
                pShapeFactory->createCircle( xResultGroup, awt::Size( nSize, nSize ),
                        awt::Point( rEntryKeyAspectRatio.Width/2-nSize/2, rEntryKeyAspectRatio.Height/2-nSize/2 ));
            if( xShape.is() )
            {
                lcl_setPropertiesToShape( xLegendEntryProperties, xShape, ePropertyType, awt::Size(0,0) ); // PropertyType::FilledSeries );
            }
        }
        else // eStyle == LegendSymbolStyle::Box
        {
            tNameSequence aPropNames;
            tAnySequence aPropValues;

            getPropNamesAndValues( xLegendEntryProperties, aPropNames, aPropValues,
                    ePropertyType, awt::Size(0,0) );// PropertyType::FilledSeries

            pShapeFactory->createRectangle( xResultGroup,
                        rEntryKeyAspectRatio, awt::Point( 0, 0 ),
                        aPropNames, aPropValues );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
