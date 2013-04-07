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

#include "VLegendSymbolFactory.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
#include "ShapeFactory.hxx"
#include "ObjectIdentifier.hxx"
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart2/Symbol.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
void lcl_setPropetiesToShape(
    const Reference< beans::XPropertySet > & xProp,
    const Reference< drawing::XShape > & xShape,
    ::chart::VLegendSymbolFactory::tPropertyType ePropertyType,
    const awt::Size& aMaxSymbolExtent = awt::Size(0,0))
{
    const ::chart::tPropertyNameMap & aFilledSeriesNameMap( ::chart::PropertyMapper::getPropertyNameMapForFilledSeriesProperties());
    const ::chart::tPropertyNameMap & aLineSeriesNameMap( ::chart::PropertyMapper::getPropertyNameMapForLineSeriesProperties());
    const ::chart::tPropertyNameMap & aLineNameMap( ::chart::PropertyMapper::getPropertyNameMapForLineProperties());
    const ::chart::tPropertyNameMap & aFillNameMap( ::chart::PropertyMapper::getPropertyNameMapForFillProperties());
    const ::chart::tPropertyNameMap & aFillLineNameMap( ::chart::PropertyMapper::getPropertyNameMapForFillAndLineProperties());

    Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
    if( xProp.is() && xShapeProp.is() )
    {
        ::chart::tPropertyNameValueMap aValueMap;
        switch( ePropertyType )
        {
            case ::chart::VLegendSymbolFactory::PROP_TYPE_FILLED_SERIES:
                ::chart::PropertyMapper::getValueMap( aValueMap, aFilledSeriesNameMap, xProp );
                break;
            case ::chart::VLegendSymbolFactory::PROP_TYPE_LINE_SERIES:
                ::chart::PropertyMapper::getValueMap( aValueMap, aLineSeriesNameMap, xProp );
                break;
            case ::chart::VLegendSymbolFactory::PROP_TYPE_LINE:
                ::chart::PropertyMapper::getValueMap( aValueMap, aLineNameMap, xProp );
                break;
            case ::chart::VLegendSymbolFactory::PROP_TYPE_FILL:
                ::chart::PropertyMapper::getValueMap( aValueMap, aFillNameMap, xProp );
                break;
            case ::chart::VLegendSymbolFactory::PROP_TYPE_FILL_AND_LINE:
                ::chart::PropertyMapper::getValueMap( aValueMap, aFillLineNameMap, xProp );
                break;
        }

        ::chart::tNameSequence aPropNames;
        ::chart::tAnySequence aPropValues;
        ::chart::PropertyMapper::getMultiPropertyListsFromValueMap( aPropNames, aPropValues, aValueMap );

        uno::Any* pLineWidthAny = ::chart::PropertyMapper::getValuePointer(aPropValues,aPropNames,"LineWidth");
        sal_Int32 nLineWidth = 0;
        if( pLineWidthAny && (*pLineWidthAny>>=nLineWidth) )
        {
            // use legend entry height as upper limit for line width
            sal_Int32 nMaxLineWidthForLegend = aMaxSymbolExtent.Height;
            if( nLineWidth>nMaxLineWidthForLegend )
                *pLineWidthAny = uno::makeAny( nMaxLineWidthForLegend );
        }

        ::chart::PropertyMapper::setMultiProperties( aPropNames, aPropValues, xShapeProp );
    }
}

} // anonymous namespace

namespace chart
{

Reference< drawing::XShape > VLegendSymbolFactory::createSymbol(
    const awt::Size& rEntryKeyAspectRatio,
    const Reference< drawing::XShapes > xSymbolContainer,
    LegendSymbolStyle eStyle,
    const Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const Reference< beans::XPropertySet > & xLegendEntryProperties,
    tPropertyType ePropertyType, const uno::Any& rExplicitSymbol )
{
    Reference< drawing::XShape > xResult;

    if( ! (xSymbolContainer.is() && xShapeFactory.is()))
        return xResult;

    xResult.set( xShapeFactory->createInstance(
                     "com.sun.star.drawing.GroupShape"), uno::UNO_QUERY );
    xSymbolContainer->add( xResult );
    Reference< drawing::XShapes > xResultGroup( xResult, uno::UNO_QUERY );
    if( ! xResultGroup.is())
        return xResult;

    // add an invisible square box to maintain aspect ratio
    Reference< drawing::XShape > xBound( ShapeFactory(xShapeFactory).createInvisibleRectangle(
                xResultGroup, rEntryKeyAspectRatio  ));

    // create symbol
    try
    {
        if( eStyle == LegendSymbolStyle_LINE )
        {
            Reference< drawing::XShape > xLine( xShapeFactory->createInstance(
                    "com.sun.star.drawing.LineShape"), uno::UNO_QUERY );
            if( xLine.is())
            {
                xResultGroup->add( xLine );
                xLine->setSize(  awt::Size( rEntryKeyAspectRatio.Width, 0 ));
                xLine->setPosition( awt::Point( 0, rEntryKeyAspectRatio.Height/2 ));

                lcl_setPropetiesToShape( xLegendEntryProperties, xLine, ePropertyType, rEntryKeyAspectRatio );
            }

            Reference< drawing::XShape > xSymbol;
            const sal_Int32 nSize = std::min(rEntryKeyAspectRatio.Width,rEntryKeyAspectRatio.Height);
            chart2::Symbol aSymbol;
            if( rExplicitSymbol >>= aSymbol )
            {
                drawing::Direction3D aSymbolSize( nSize, nSize, 0 );
                drawing::Position3D aPos( rEntryKeyAspectRatio.Width/2, rEntryKeyAspectRatio.Height/2, 0 );
                ShapeFactory aFactory( xShapeFactory );
                if( aSymbol.Style == chart2::SymbolStyle_STANDARD )
                {
                    // take series color as fill color
                    xLegendEntryProperties->getPropertyValue( "Color") >>= aSymbol.FillColor;
                    // border of symbols always same as fill color
                    aSymbol.BorderColor = aSymbol.FillColor;

                    xSymbol.set( aFactory.createSymbol2D(
                                     xResultGroup,
                                     aPos,
                                     aSymbolSize,
                                     aSymbol.StandardSymbol,
                                     aSymbol.BorderColor,
                                     aSymbol.FillColor ));
                }
                else if( aSymbol.Style == chart2::SymbolStyle_GRAPHIC )
                {
                    xSymbol.set( aFactory.createGraphic2D(
                                     xResultGroup,
                                     aPos,
                                     aSymbolSize,
                                     aSymbol.Graphic ));
                }
                else if( aSymbol.Style == chart2::SymbolStyle_AUTO )
                {
                    OSL_TRACE("the given parameter is not allowed to contain an automatic symbol style");
                }
            }
        }
        else if( eStyle == LegendSymbolStyle_CIRCLE )
        {
            Reference< drawing::XShape > xShape( xShapeFactory->createInstance(
                "com.sun.star.drawing.EllipseShape"), uno::UNO_QUERY );
            if( xShape.is() )
            {
                xResultGroup->add( xShape );
                sal_Int32 nSize = std::min( rEntryKeyAspectRatio.Width, rEntryKeyAspectRatio.Height );
                xShape->setSize( awt::Size( nSize, nSize ) );
                xShape->setPosition( awt::Point( rEntryKeyAspectRatio.Width/2-nSize/2, rEntryKeyAspectRatio.Height/2-nSize/2 ) );
                lcl_setPropetiesToShape( xLegendEntryProperties, xShape, ePropertyType ); // PROP_TYPE_FILLED_SERIES );
            }
        }
        else // eStyle == LegendSymbolStyle_BOX
        {
            Reference< drawing::XShape > xShape( xShapeFactory->createInstance(
                "com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY );
            if( xShape.is() )
            {
                xResultGroup->add( xShape );
                xShape->setSize( rEntryKeyAspectRatio );
                xShape->setPosition( awt::Point( 0, 0 ) );
                lcl_setPropetiesToShape( xLegendEntryProperties, xShape, ePropertyType ); // PROP_TYPE_FILLED_SERIES );
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
