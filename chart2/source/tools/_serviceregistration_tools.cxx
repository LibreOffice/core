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

#include <cppuhelper/implementationentry.hxx>
#include "LabeledDataSequence.hxx"
#include "CachedDataSequence.hxx"
#include "DataSource.hxx"
#include "ConfigColorScheme.hxx"
#include "Scaling.hxx"
#include "ErrorBar.hxx"
#include "RegressionCurveModel.hxx"
#include "RegressionEquation.hxx"
#include "InternalDataProvider.hxx"
#include "charttoolsdllapi.hxx"

static const struct ::cppu::ImplementationEntry g_entries_chart2_tools[] =
{
    {
          ::chart::LabeledDataSequence::create
        , ::chart::LabeledDataSequence::getImplementationName_Static
        , ::chart::LabeledDataSequence::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::CachedDataSequence::create
        , ::chart::CachedDataSequence::getImplementationName_Static
        , ::chart::CachedDataSequence::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::DataSource::create
        , ::chart::DataSource::getImplementationName_Static
        , ::chart::DataSource::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::ConfigColorScheme::create
        , ::chart::ConfigColorScheme::getImplementationName_Static
        , ::chart::ConfigColorScheme::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }

    ,{
          ::chart::LogarithmicScaling::create
        , ::chart::LogarithmicScaling::getImplementationName_Static
        , ::chart::LogarithmicScaling::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::ExponentialScaling::create
        , ::chart::ExponentialScaling::getImplementationName_Static
        , ::chart::ExponentialScaling::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::LinearScaling::create
        , ::chart::LinearScaling::getImplementationName_Static
        , ::chart::LinearScaling::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::PowerScaling::create
        , ::chart::PowerScaling::getImplementationName_Static
        , ::chart::PowerScaling::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::ErrorBar::create
        , ::chart::ErrorBar::getImplementationName_Static
        , ::chart::ErrorBar::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::MeanValueRegressionCurve::create
        , ::chart::MeanValueRegressionCurve::getImplementationName_Static
        , ::chart::MeanValueRegressionCurve::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::LinearRegressionCurve::create
        , ::chart::LinearRegressionCurve::getImplementationName_Static
        , ::chart::LinearRegressionCurve::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::LogarithmicRegressionCurve::create
        , ::chart::LogarithmicRegressionCurve::getImplementationName_Static
        , ::chart::LogarithmicRegressionCurve::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::ExponentialRegressionCurve::create
        , ::chart::ExponentialRegressionCurve::getImplementationName_Static
        , ::chart::ExponentialRegressionCurve::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::PotentialRegressionCurve::create
        , ::chart::PotentialRegressionCurve::getImplementationName_Static
        , ::chart::PotentialRegressionCurve::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::PolynomialRegressionCurve::create
        , ::chart::PolynomialRegressionCurve::getImplementationName_Static
        , ::chart::PolynomialRegressionCurve::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::MovingAverageRegressionCurve::create
        , ::chart::MovingAverageRegressionCurve::getImplementationName_Static
        , ::chart::MovingAverageRegressionCurve::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::RegressionEquation::create
        , ::chart::RegressionEquation::getImplementationName_Static
        , ::chart::RegressionEquation::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{
          ::chart::InternalDataProvider::create
        , ::chart::InternalDataProvider::getImplementationName_Static
        , ::chart::InternalDataProvider::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{ 0, 0, 0, 0, 0, 0 }
};

// component exports
extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL charttools_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries_chart2_tools );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
