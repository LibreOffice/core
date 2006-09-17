/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _serviceregistration_controller.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:07:48 $
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
#ifndef _CHARTCONTROLLER_HXX
#include "ChartController.hxx"
#endif
#ifndef _CHARTFRAMELOADER_HXX
#include "ChartFrameloader.hxx"
#endif
#ifndef CHART_CHARTDOCUMENTWRAPPER_HXX
#include "ChartDocumentWrapper.hxx"
#endif

static struct ::cppu::ImplementationEntry g_entries_chart2_view[] =
{
    {
          ::chart::ChartController::create
        , ::chart::ChartController::getImplementationName_Static
        , ::chart::ChartController::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ChartFrameLoader::create
        , ::chart::ChartFrameLoader::getImplementationName_Static
        , ::chart::ChartFrameLoader::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::wrapper::ChartDocumentWrapper::create
        , ::chart::wrapper::ChartDocumentWrapper::getImplementationName_Static
        , ::chart::wrapper::ChartDocumentWrapper::getSupportedServiceNames_Static
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
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_writeInfoHelper(
        pServiceManager, pRegistryKey, g_entries_chart2_view );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries_chart2_view );
}
}
//=========================================================================
