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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "VLegendSymbolFactory.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
#include "ShapeFactory.hxx"
#include "ObjectIdentifier.hxx"
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart2/Symbol.hpp>

// header for define DBG_ASSERT
#include <tools/debug.hxx>

// uncomment to disable line dashes at the border of boxes
// #define DISABLE_DASHES_AT_BORDER

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{
void lcl_setPropetiesToShape(
    const Reference< beans::XPropertySet > & xProp,
    const Reference< drawing::XShape > & xShape,
    ::chart::VLegendSymbolFactory::tPropertyType ePropertyType )
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

        uno::Any* pLineWidthAny = ::chart::PropertyMapper::getValuePointer(aPropValues,aPropNames,C2U("LineWidth"));
        sal_Int32 nLineWidth = 0;
        if( pLineWidthAny && (*pLineWidthAny>>=nLineWidth) )
        {
            const sal_Int32 nMaxLineWidthForLegend = 50;/*1/100 mm*///todo: make this dependent from legend entry height
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
    const Reference< drawing::XShapes > xSymbolContainer,
    chart2::LegendSymbolStyle eStyle,
    const Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const Reference< beans::XPropertySet > & xLegendEntryProperties,
    tPropertyType ePropertyType, const uno::Any& rExplicitSymbol )
{
    Reference< drawing::XShape > xResult;

    if( ! (xSymbolContainer.is() &&
           xShapeFactory.is()))
        return xResult;

    xResult.set( xShapeFactory->createInstance(
                     C2U( "com.sun.star.drawing.GroupShape" )), uno::UNO_QUERY );
    xSymbolContainer->add( xResult );
    Reference< drawing::XShapes > xResultGroup( xResult, uno::UNO_QUERY );
    if( ! xResultGroup.is())
        return xResult;

    // aspect ratio of symbols is always 3:2
    awt::Size aBoundSize( 3000, 2000 );
    bool bUseBox = false;

    // add an invisible square box to maintain aspect ratio
    switch( eStyle )
    {
        case chart2::LegendSymbolStyle_BOX:
        case chart2::LegendSymbolStyle_HORIZONTAL_LINE:
        case chart2::LegendSymbolStyle_VERTICAL_LINE:
        case chart2::LegendSymbolStyle_DIAGONAL_LINE:
        case chart2::LegendSymbolStyle_LINE_WITH_BOX:
        case chart2::LegendSymbolStyle_LINE_WITH_SYMBOL:
        case chart2::LegendSymbolStyle_CIRCLE:
        {
            Reference< drawing::XShape > xBound( ShapeFactory(xShapeFactory).createInvisibleRectangle(
                xResultGroup, aBoundSize  ));
            break;
        }

        case chart2::LegendSymbolStyle_BAR:
        case chart2::LegendSymbolStyle_RECTANGLE:
        case chart2::LegendSymbolStyle_STRETCHED_RECTANGLE:
        case chart2::LegendSymbolStyle_USER_DEFINED:
        default:
            break;
    }

    // create symbol
    switch( eStyle )
    {
        case chart2::LegendSymbolStyle_BOX:
        case chart2::LegendSymbolStyle_BAR:
        case chart2::LegendSymbolStyle_RECTANGLE:
        case chart2::LegendSymbolStyle_STRETCHED_RECTANGLE:
        case chart2::LegendSymbolStyle_CIRCLE:
        {
            try
            {
                Reference< drawing::XShape > xShape;

                if( eStyle == chart2::LegendSymbolStyle_CIRCLE )
                    xShape.set( xShapeFactory->createInstance(
                                    C2U( "com.sun.star.drawing.EllipseShape" )), uno::UNO_QUERY );
                else
                    xShape.set( xShapeFactory->createInstance(
                                    C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );

                if( xShape.is())
                {
                    xResultGroup->add( xShape );
                    if( eStyle == chart2::LegendSymbolStyle_BOX ||
                        eStyle == chart2::LegendSymbolStyle_CIRCLE )
                    {
                        xShape->setSize( awt::Size( 2000, 2000 ));
                        xShape->setPosition( awt::Point( 500, 0 ));
                    }
                    else
                    {
                        xShape->setSize( aBoundSize );
                    }
                }

                lcl_setPropetiesToShape( xLegendEntryProperties, xShape, ePropertyType ); // PROP_TYPE_FILLED_SERIES );

#ifdef DISABLE_DASHES_AT_BORDER
                // don't allow dashed border style
                Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
                if( xShapeProp.is())
                {
                    drawing::LineStyle aLineStyle;
                    if( ( xShapeProp->getPropertyValue( C2U("LineStyle")) >>= aLineStyle ) &&
                        aLineStyle == drawing::LineStyle_DASH )
                    {
                        aLineStyle = drawing::LineStyle_SOLID;
                        xShapeProp->setPropertyValue( C2U("LineStyle"), uno::makeAny( aLineStyle ));
                    }
                }
#endif
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            break;
        }

        case chart2::LegendSymbolStyle_HORIZONTAL_LINE:
        {
            try
            {
                Reference< drawing::XShape > xLine(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );
                if( xLine.is())
                {
                    xResultGroup->add( xLine );
                    xLine->setSize(  awt::Size( 3000, 0 ));
                    xLine->setPosition( awt::Point( 0, 1000 ));

                    lcl_setPropetiesToShape( xLegendEntryProperties, xLine, ePropertyType ); // PROP_TYPE_LINE_SERIES );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            break;
        }

        case chart2::LegendSymbolStyle_VERTICAL_LINE:
        {
            try
            {
                Reference< drawing::XShape > xLine(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );
                if( xLine.is())
                {
                    xResultGroup->add( xLine );
                    xLine->setSize(  awt::Size( 0, 2000 ));
                    xLine->setPosition( awt::Point( 1500, 0 ));

                    lcl_setPropetiesToShape( xLegendEntryProperties, xLine, ePropertyType ); // PROP_TYPE_LINE_SERIES );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            break;
        }

        case chart2::LegendSymbolStyle_DIAGONAL_LINE:
        {
            try
            {
                Reference< drawing::XShape > xLine(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );
                if( xLine.is())
                {
                    xResultGroup->add( xLine );
                    xLine->setSize(  awt::Size( 2000, 2000 ));
                    xLine->setPosition( awt::Point( 500, 0 ));

                    lcl_setPropetiesToShape( xLegendEntryProperties, xLine, ePropertyType ); // PROP_TYPE_LINE_SERIES );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            break;
        }

        case chart2::LegendSymbolStyle_LINE_WITH_BOX:
            bUseBox = true;
            // fall-through intended
        case chart2::LegendSymbolStyle_LINE_WITH_SYMBOL:
            try
            {
                Reference< drawing::XShape > xLine(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );
                if( xLine.is())
                {
                    xResultGroup->add( xLine );
                    xLine->setSize(  awt::Size( 3000, 0 ));
                    xLine->setPosition( awt::Point( 0, 1000 ));

                    lcl_setPropetiesToShape( xLegendEntryProperties, xLine, ePropertyType );
                }

                Reference< drawing::XShape > xSymbol;
                const sal_Int32 nSize = 1500;
                if( bUseBox )
                {
                    xSymbol.set( xShapeFactory->createInstance(
                                     C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );
                    xResultGroup->add( xSymbol );

                    if( xSymbol.is())
                    {
                        xSymbol->setSize( awt::Size( nSize, nSize ));
                        xSymbol->setPosition( awt::Point( 1500 - nSize/2, 1000 - nSize/2 ));

                        lcl_setPropetiesToShape( xLegendEntryProperties, xSymbol, ePropertyType );
                    }
                }
                else
                {
                    chart2::Symbol aSymbol;

                    if( rExplicitSymbol >>= aSymbol )
                    {
                        drawing::Direction3D aSymbolSize( nSize, nSize, 0 );
                        drawing::Position3D aPos( 1500, 1000, 0 );
                        ShapeFactory aFactory( xShapeFactory );
                        if( aSymbol.Style == chart2::SymbolStyle_STANDARD )
                        {
                            // take series color as fill color
                            xLegendEntryProperties->getPropertyValue( C2U("Color")) >>= aSymbol.FillColor;
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
                            OSL_FAIL("the given parameter is not allowed to contain an automatic symbol style");
                        }
                    }
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            break;

        case chart2::LegendSymbolStyle_USER_DEFINED:
            break;

        default:
            // just to remove warning (there is an auto-generated extra label)
            break;
    }

    return xResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
