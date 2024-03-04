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

#include "SchXMLRegressionCurveObjectContext.hxx"

#include <SchXMLImport.hxx>

#include <sax/tools/converter.hxx>
#include <sal/log.hxx>

#include <utility>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlstyle.hxx>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/chart2/RegressionEquation.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

SchXMLRegressionCurveObjectContext::SchXMLRegressionCurveObjectContext(
                                        SchXMLImportHelper& rImpHelper,
                                        SvXMLImport& rImport,
                                        std::vector< RegressionStyle >& rRegressionStyleVector,
                                        css::uno::Reference<
                                                    css::chart2::XDataSeries > xSeries,
                                        const awt::Size & rChartSize) :
    SvXMLImportContext( rImport ),
    mrImportHelper( rImpHelper ),
    mxSeries(std::move( xSeries )),
    maChartSize( rChartSize ),
    mrRegressionStyleVector( rRegressionStyleVector )
{
}

SchXMLRegressionCurveObjectContext::~SchXMLRegressionCurveObjectContext()
{
}

void SchXMLRegressionCurveObjectContext::startFastElement (sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    OUString sAutoStyleName;
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if (aIter.getToken() == XML_ELEMENT(CHART, XML_STYLE_NAME) )
            sAutoStyleName = aIter.toString();
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }

    RegressionStyle aStyle( mxSeries, sAutoStyleName );
    mrRegressionStyleVector.push_back( aStyle );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLRegressionCurveObjectContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if( nElement == XML_ELEMENT(CHART, XML_EQUATION) )
    {
        return new SchXMLEquationContext(
            mrImportHelper, GetImport(), maChartSize, mrRegressionStyleVector.back());
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

SchXMLEquationContext::SchXMLEquationContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const awt::Size& rChartSize,
    RegressionStyle& rRegressionStyle ) :
        SvXMLImportContext( rImport ),
        mrImportHelper( rImpHelper ),
        mrRegressionStyle( rRegressionStyle ),
        maChartSize( rChartSize )
{}

SchXMLEquationContext::~SchXMLEquationContext()
{}

void SchXMLEquationContext::startFastElement (sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // parse attributes
    SchXMLImport& rImport = static_cast< SchXMLImport& >(GetImport());
    OUString sAutoStyleName;
    bool bShowEquation = true;
    bool bShowRSquare = false;
    awt::Point aPosition;
    bool bHasXPos = false;
    bool bHasYPos = false;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(SVG, XML_X):
            case XML_ELEMENT(SVG_COMPAT, XML_X):
                rImport.GetMM100UnitConverter().convertMeasureToCore(
                        aPosition.X, aIter.toView() );
                bHasXPos = true;
                break;
            case XML_ELEMENT(SVG, XML_Y):
            case XML_ELEMENT(SVG_COMPAT, XML_Y):
                rImport.GetMM100UnitConverter().convertMeasureToCore(
                        aPosition.Y, aIter.toView() );
                bHasYPos = true;
                break;
            case XML_ELEMENT(CHART, XML_DISPLAY_EQUATION):
                (void)::sax::Converter::convertBool(bShowEquation, aIter.toView());
                break;
            case XML_ELEMENT(CHART, XML_DISPLAY_R_SQUARE):
                (void)::sax::Converter::convertBool(bShowRSquare, aIter.toView());
                break;
            case XML_ELEMENT(CHART, XML_STYLE_NAME):
                sAutoStyleName = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( sAutoStyleName.isEmpty() && !bShowEquation && !bShowRSquare )
        return;

    uno::Reference< beans::XPropertySet > xEquationProperties = chart2::RegressionEquation::create( comphelper::getProcessComponentContext() );

    if( !sAutoStyleName.isEmpty() )
    {
        const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
        if( pStylesCtxt )
        {
            const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                SchXMLImportHelper::GetChartFamilyID(), sAutoStyleName );

            XMLPropStyleContext* pPropStyleContext =
                const_cast< XMLPropStyleContext* >( dynamic_cast< const XMLPropStyleContext* >( pStyle ));

            if( pPropStyleContext )
                pPropStyleContext->FillPropertySet( xEquationProperties );
        }
    }
    xEquationProperties->setPropertyValue( "ShowEquation", uno::Any( bShowEquation ));
    xEquationProperties->setPropertyValue( "ShowCorrelationCoefficient", uno::Any( bShowRSquare ));

    if( bHasXPos && bHasYPos )
    {
        chart2::RelativePosition aRelPos;
        aRelPos.Primary = static_cast< double >( aPosition.X ) / static_cast< double >( maChartSize.Width );
        aRelPos.Secondary = static_cast< double >( aPosition.Y ) / static_cast< double >( maChartSize.Height );
        xEquationProperties->setPropertyValue( "RelativePosition", uno::Any( aRelPos ));
    }
    mrRegressionStyle.m_xEquationProperties.set( xEquationProperties );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
