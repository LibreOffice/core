/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _serviceregistration_tools.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:16:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#include "LabeledDataSequence.hxx"
#include "CachedDataSequence.hxx"
#include "DataSource.hxx"
#include "ConfigColorScheme.hxx"
#include "Scaling.hxx"
#include "ErrorBar.hxx"
#include "RegressionCurveModel.hxx"
#include "RegressionEquation.hxx"

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
   ,{ 0, 0, 0, 0, 0, 0 }
};

// component exports
extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_writeInfoHelper(
                pServiceManager, pRegistryKey, g_entries_chart2_tools );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries_chart2_tools );
}
}
//=========================================================================
