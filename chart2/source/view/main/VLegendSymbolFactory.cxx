/*************************************************************************
 *
 *  $RCSfile: VLegendSymbolFactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:50:29 $
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
#include "VLegendSymbolFactory.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

// uncomment to disable line dashes at the border of boxes
// #define DISABLE_DASHES_AT_BORDER

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{
void lcl_setPropetiesToShape(
    const uno::Reference< beans::XPropertySet > & xProp,
    const uno::Reference< drawing::XShape > & xShape,
    bool bFilledSeries )
{
    static ::chart::tPropertyNameMap aFilledNameMap( ::chart::PropertyMapper::getPropertyNameMapForFilledSeriesProperties());
    static ::chart::tPropertyNameMap aLineNameMap( ::chart::PropertyMapper::getPropertyNameMapForLineSeriesProperties());

    uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
    if( xProp.is() && xShapeProp.is() )
    {
        ::chart::tPropertyNameValueMap aValueMap;
        if( bFilledSeries )
            ::chart::PropertyMapper::getValueMap( aValueMap, aFilledNameMap, xProp );
        else
            ::chart::PropertyMapper::getValueMap( aValueMap, aLineNameMap, xProp );

        ::chart::tNameSequence aPropNames;
        ::chart::tAnySequence aPropValues;
        ::chart::PropertyMapper::getMultiPropertyListsFromValueMap( aPropNames, aPropValues, aValueMap );
        ::chart::PropertyMapper::setMultiProperties( aPropNames, aPropValues, xShapeProp );
    }
}

} // anonymous namespace

namespace chart
{

// static
void VLegendSymbolFactory::createSymbol(
    const uno::Reference< drawing::XShapes > xSymbolGroup,
    chart2::LegendSymbolStyle eStyle,
    const uno::Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const uno::Reference< beans::XPropertySet > & xSeriesProperties )
{
    // aspect ratio of symbols is always 3:2
    awt::Size aBoundSize( 3000, 2000 );

    // add an invisible square box to maintain aspect ratio
    switch( eStyle )
    {
        case chart2::LegendSymbolStyle_BOX:
        case chart2::LegendSymbolStyle_LINE:
        case chart2::LegendSymbolStyle_DIAGONAL_LINE:
        case chart2::LegendSymbolStyle_LINE_WITH_BOX:
        case chart2::LegendSymbolStyle_LINE_WITH_SYMBOL:
        case chart2::LegendSymbolStyle_CIRCLE:
        {
            try
            {
                uno::Reference< drawing::XShape > xBound(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );
                if( xBound.is())
                {
                    xSymbolGroup->add( xBound );
                    xBound->setSize( aBoundSize );
                    uno::Reference< beans::XPropertySet > xBoundProp( xBound, uno::UNO_QUERY );
                    if( xBoundProp.is())
                    {
                        xBoundProp->setPropertyValue(
                            C2U("FillStyle"), uno::makeAny( drawing::FillStyle_NONE ));
                        xBoundProp->setPropertyValue(
                            C2U("LineStyle"), uno::makeAny( drawing::LineStyle_NONE ));
                    }
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
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
                uno::Reference< drawing::XShape > xShape;

                if( eStyle == chart2::LegendSymbolStyle_CIRCLE )
                    xShape.set( xShapeFactory->createInstance(
                                    C2U( "com.sun.star.drawing.EllipseShape" )), uno::UNO_QUERY );
                else
                    xShape.set( xShapeFactory->createInstance(
                                    C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );

                if( xShape.is())
                {
                    xSymbolGroup->add( xShape );
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

                lcl_setPropetiesToShape( xSeriesProperties, xShape, true /* bFilledSeries */ );

#ifdef DISABLE_DASHES_AT_BORDER
                // don't allow dashed border style
                uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
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

        case chart2::LegendSymbolStyle_LINE:
        {
            try
            {
                uno::Reference< drawing::XShape > xLine(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );
                if( xLine.is())
                {
                    xSymbolGroup->add( xLine );
                    xLine->setSize(  awt::Size( 3000, 0 ));
                    xLine->setPosition( awt::Point( 0, 1000 ));

                    lcl_setPropetiesToShape( xSeriesProperties, xLine, false /* bFilledSeries */ );
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
                uno::Reference< drawing::XShape > xLine(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );
                if( xLine.is())
                {
                    xSymbolGroup->add( xLine );
                    xLine->setSize(  awt::Size( 2000, 2000 ));
                    xLine->setPosition( awt::Point( 500, 0 ));

                    lcl_setPropetiesToShape( xSeriesProperties, xLine, false /* bFilledSeries */ );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            break;
        }

        case chart2::LegendSymbolStyle_LINE_WITH_BOX:
        {
            try
            {
                uno::Reference< drawing::XShape > xLine(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );
                if( xLine.is())
                {
                    xSymbolGroup->add( xLine );
                    xLine->setSize(  awt::Size( 3000, 0 ));
                    xLine->setPosition( awt::Point( 0, 1000 ));

                    lcl_setPropetiesToShape( xSeriesProperties, xLine, false /* bFilledSeries */ );
                }

                uno::Reference< drawing::XShape > xSymbol(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );
                if( xSymbol.is())
                {
                    xSymbolGroup->add( xSymbol );
                    sal_Int32 nSize = 1000;
                    xSymbol->setSize( awt::Size( nSize, nSize ));
                    xSymbol->setPosition( awt::Point( 1500 - nSize/2, 1000 - nSize/2 ));

                    lcl_setPropetiesToShape( xSeriesProperties, xSymbol, true /* bFilledSeries */ );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            break;
        }

        case chart2::LegendSymbolStyle_LINE_WITH_SYMBOL:
            break;

        case chart2::LegendSymbolStyle_USER_DEFINED:
            break;

        default:
            // just to remove warning (there is an auto-generated extra label)
            break;
    }
}

} //  namespace chart
