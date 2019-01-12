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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLREGRESSIONCURVEOBJECTCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLREGRESSIONCURVEOBJECTCONTEXT_HXX

#include <SchXMLImport.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/shapeimport.hxx>
#include <list>

#include "transporttypes.hxx"

class SchXMLRegressionCurveObjectContext : public SvXMLImportContext
{
public:
    SchXMLRegressionCurveObjectContext(
        SchXMLImportHelper& rImportHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        std::vector< RegressionStyle >& rRegressionStyleVector,
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        const css::awt::Size & rChartSize );

    virtual ~SchXMLRegressionCurveObjectContext() override;

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

private:

    SchXMLImportHelper&                mrImportHelper;
    css::uno::Reference<css::chart2::XDataSeries > mxSeries;
    css::awt::Size const               maChartSize;
    std::vector< RegressionStyle >&    mrRegressionStyleVector;
};

class SchXMLEquationContext : public SvXMLImportContext
{
public:
    SchXMLEquationContext(
        SchXMLImportHelper& rImportHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::awt::Size & rChartSize,
        RegressionStyle & rRegressionStyle );

    virtual ~SchXMLEquationContext() override;

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

private:
    SchXMLImportHelper&                           mrImportHelper;
    RegressionStyle&                              mrRegressionStyle;
    css::awt::Size const                          maChartSize;
};

#endif // INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLREGRESSIONCURVEOBJECTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
