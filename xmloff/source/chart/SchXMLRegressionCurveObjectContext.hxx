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
#pragma once

#include <com/sun/star/awt/Size.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/SchXMLImportHelper.hxx>

#include "transporttypes.hxx"

class SchXMLRegressionCurveObjectContext : public SvXMLImportContext
{
public:
    SchXMLRegressionCurveObjectContext(
        SchXMLImportHelper& rImportHelper,
        SvXMLImport& rImport,
        std::vector< RegressionStyle >& rRegressionStyleVector,
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        const css::awt::Size & rChartSize );

    virtual ~SchXMLRegressionCurveObjectContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

private:

    SchXMLImportHelper&                mrImportHelper;
    css::uno::Reference<css::chart2::XDataSeries > mxSeries;
    css::awt::Size                     maChartSize;
    std::vector< RegressionStyle >&    mrRegressionStyleVector;
};

class SchXMLEquationContext : public SvXMLImportContext
{
public:
    SchXMLEquationContext(
        SchXMLImportHelper& rImportHelper,
        SvXMLImport& rImport,
        const css::awt::Size & rChartSize,
        RegressionStyle & rRegressionStyle );

    virtual ~SchXMLEquationContext() override;

    virtual void SAL_CALL startFastElement (sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs) override;

private:
    SchXMLImportHelper&                           mrImportHelper;
    RegressionStyle&                              mrRegressionStyle;
    css::awt::Size                                maChartSize;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
