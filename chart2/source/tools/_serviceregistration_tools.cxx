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

static struct ::cppu::ImplementationEntry g_entries_chart2_tools[] =
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
//==================================================================================================
OOO_DLLPUBLIC_CHARTTOOLS void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
OOO_DLLPUBLIC_CHARTTOOLS void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries_chart2_tools );
}
}
//=========================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
