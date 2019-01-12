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

#include <sax/tools/converter.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlement.hxx>
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
                                        sal_uInt16 nPrefix,
                                        const OUString& rLocalName,
                                        std::vector< RegressionStyle >& rRegressionStyleVector,
                                        const css::uno::Reference<
                                                    css::chart2::XDataSeries >& xSeries,
                                        const awt::Size & rChartSize) :
    SvXMLImportContext( rImport, nPrefix, rLocalName ),
    mrImportHelper( rImpHelper ),
    mxSeries( xSeries ),
    maChartSize( rChartSize ),
    mrRegressionStyleVector( rRegressionStyleVector )
{
}

SchXMLRegressionCurveObjectContext::~SchXMLRegressionCurveObjectContext()
{
}

void SchXMLRegressionCurveObjectContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttributeList )
{
    sal_Int16 nAttributeCount = xAttributeList.is()? xAttributeList->getLength(): 0;
    OUString sAutoStyleName;

    for( sal_Int16 i = 0; i < nAttributeCount; i++ )
    {
        OUString sAttributeName = xAttributeList->getNameByIndex( i );
        OUString aLocalName;

        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttributeName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
            {
                sAutoStyleName = xAttributeList->getValueByIndex( i );
            }
        }
    }

    RegressionStyle aStyle( mxSeries, sAutoStyleName );
    mrRegressionStyleVector.push_back( aStyle );
}

SvXMLImportContextRef SchXMLRegressionCurveObjectContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    if( nPrefix == XML_NAMESPACE_CHART && IsXMLToken( rLocalName, XML_EQUATION ) )
    {
        xContext = new SchXMLEquationContext(
            mrImportHelper, GetImport(), nPrefix, rLocalName, maChartSize, mrRegressionStyleVector.back());
    }
    else
    {
        xContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
    }

    return xContext;
}

SchXMLEquationContext::SchXMLEquationContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const awt::Size& rChartSize,
    RegressionStyle& rRegressionStyle ) :

        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrRegressionStyle( rRegressionStyle ),
        maChartSize( rChartSize )
{}

SchXMLEquationContext::~SchXMLEquationContext()
{}

void SchXMLEquationContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    SchXMLImport& rImport = static_cast< SchXMLImport& >(GetImport());
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetRegEquationAttrTokenMap();
    OUString sAutoStyleName;

    bool bShowEquation = true;
    bool bShowRSquare = false;
    awt::Point aPosition;
    bool bHasXPos = false;
    bool bHasYPos = false;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_REGEQ_POS_X:
                rImport.GetMM100UnitConverter().convertMeasureToCore(
                        aPosition.X, aValue );
                bHasXPos = true;
                break;
            case XML_TOK_REGEQ_POS_Y:
                rImport.GetMM100UnitConverter().convertMeasureToCore(
                        aPosition.Y, aValue );
                bHasYPos = true;
                break;
            case XML_TOK_REGEQ_DISPLAY_EQUATION:
                (void)::sax::Converter::convertBool(bShowEquation, aValue);
                break;
            case XML_TOK_REGEQ_DISPLAY_R_SQUARE:
                (void)::sax::Converter::convertBool(bShowRSquare, aValue);
                break;
            case XML_TOK_REGEQ_STYLE_NAME:
                sAutoStyleName = aValue;
                break;
        }
    }

    if( !sAutoStyleName.isEmpty() || bShowEquation || bShowRSquare )
    {
        uno::Reference< beans::XPropertySet > xEqationProperties = chart2::RegressionEquation::create( comphelper::getProcessComponentContext() );

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
                    pPropStyleContext->FillPropertySet( xEqationProperties );
            }
        }
        xEqationProperties->setPropertyValue( "ShowEquation", uno::makeAny( bShowEquation ));
        xEqationProperties->setPropertyValue( "ShowCorrelationCoefficient", uno::makeAny( bShowRSquare ));

        if( bHasXPos && bHasYPos )
        {
            chart2::RelativePosition aRelPos;
            aRelPos.Primary = static_cast< double >( aPosition.X ) / static_cast< double >( maChartSize.Width );
            aRelPos.Secondary = static_cast< double >( aPosition.Y ) / static_cast< double >( maChartSize.Height );
            xEqationProperties->setPropertyValue( "RelativePosition", uno::makeAny( aRelPos ));
        }
        mrRegressionStyle.m_xEquationProperties.set( xEqationProperties );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
