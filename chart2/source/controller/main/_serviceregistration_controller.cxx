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
#include "ChartController.hxx"
#include "ChartFrameloader.hxx"
#include "dlg_CreationWizard_UNO.hxx"
#include "dlg_ChartType_UNO.hxx"
#include "ChartDocumentWrapper.hxx"
#include "AccessibleChartView.hxx"
#include "ElementSelector.hxx"
#include "ShapeToolbarController.hxx"
#include <cppuhelper/implementationentry.hxx>

static struct ::cppu::ImplementationEntry g_entries_chart2_controller[] =
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
          ::chart::CreationWizardUnoDlg::create
        , ::chart::CreationWizardUnoDlg::getImplementationName_Static
        , ::chart::CreationWizardUnoDlg::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ChartTypeUnoDlg::Create
        , ::chart::ChartTypeUnoDlg::getImplementationName_Static
        , ::chart::ChartTypeUnoDlg::getSupportedServiceNames_Static
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
    ,{
          ::chart::ElementSelectorToolbarController::create
        , ::chart::ElementSelectorToolbarController::getImplementationName_Static
        , ::chart::ElementSelectorToolbarController::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ShapeToolbarController::create
        , ::chart::ShapeToolbarController::getImplementationName_Static
        , ::chart::ShapeToolbarController::getSupportedServiceNames_Static
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
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries_chart2_controller );
}
}
//=========================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
