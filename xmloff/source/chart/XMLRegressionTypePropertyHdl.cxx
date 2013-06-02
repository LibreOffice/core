/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLRegressionTypePropertyHdl.hxx"
#include <xmloff/xmluconv.hxx>

using namespace css;
using namespace xmloff::token;

XMLRegressionTypePropertyHdl::XMLRegressionTypePropertyHdl()
{}

XMLRegressionTypePropertyHdl::~XMLRegressionTypePropertyHdl()
{}

sal_Bool XMLRegressionTypePropertyHdl::importXML( const OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    OUString aServiceName;

    if      (IsXMLToken( rStrImpValue, XML_LINEAR ))
    {
        aServiceName = "com.sun.star.chart2.LinearRegressionCurve";
    }
    else if (IsXMLToken( rStrImpValue, XML_LOGARITHMIC))
    {
        aServiceName = "com.sun.star.chart2.LogarithmicRegressionCurve";
    }
    else if (IsXMLToken( rStrImpValue, XML_EXPONENTIAL))
    {
        aServiceName = "com.sun.star.chart2.ExponentialRegressionCurve";
    }
    else if (IsXMLToken( rStrImpValue, XML_POWER))
    {
        aServiceName = "com.sun.star.chart2.PotentialRegressionCurve";
    }
    else if (IsXMLToken( rStrImpValue, XML_POLYNOMIAL))
    {
        aServiceName = "com.sun.star.chart2.PolynomialRegressionCurve";
    }
    else if (IsXMLToken( rStrImpValue, XML_MOVING_AVERAGE))
    {
        aServiceName = "com.sun.star.chart2.MovingAverageRegressionCurve";
    }
    rValue <<= aServiceName;

    return true;
}

sal_Bool XMLRegressionTypePropertyHdl::exportXML( OUString& /*rStrExpValue*/,
                                              const uno::Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
