/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "GraphicExportDialog.hxx"
#include "GraphicExportFilter.hxx"
#include <comphelper/servicedecl.hxx>

#define GRAPHIC_EXPORT_FILTER_SERVICE "com.sun.star.comp.GraphicExportFilter"
#define GRAPHIC_EXPORT_DIALOG_SERVICE "com.sun.star.comp.GraphicExportDialog"

comphelper::service_decl::class_<GraphicExportFilter> serviceGraphicExportFilter;
const comphelper::service_decl::ServiceDecl graphicExportFilter(
    serviceGraphicExportFilter,
    GRAPHIC_EXPORT_FILTER_SERVICE,
    "com.sun.star.document.ExportFilter" );

comphelper::service_decl::class_<GraphicExportDialog> serviceGraphicExportDialog;
const comphelper::service_decl::ServiceDecl graphicExportDialog(
    serviceGraphicExportDialog,
    GRAPHIC_EXPORT_DIALOG_SERVICE,
    "com.sun.star.ui.dialog.FilterOptionsDialog" );


extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL graphicfilter_component_getFactory(
    sal_Char const* pImplementationName,
    ::com::sun::star::lang::XMultiServiceFactory* pServiceManager,
    ::com::sun::star::registry::XRegistryKey* pRegistryKey )
{
    if ( rtl_str_compare (pImplementationName, GRAPHIC_EXPORT_FILTER_SERVICE) == 0 )
    {
        return component_getFactoryHelper( pImplementationName, pServiceManager, pRegistryKey, graphicExportFilter );
    }
    else if ( rtl_str_compare (pImplementationName, GRAPHIC_EXPORT_DIALOG_SERVICE) == 0 )
    {
        return component_getFactoryHelper( pImplementationName, pServiceManager, pRegistryKey, graphicExportDialog );
    }
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
