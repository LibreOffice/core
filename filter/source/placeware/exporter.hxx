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
#ifndef _PLACEWARE_EXPORTER_HXX
#define _PLACEWARE_EXPORTER_HXX

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XGraphicExportFilter.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class PageEntry;

class PlaceWareExporter
{
public:
    PlaceWareExporter( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    ~PlaceWareExporter();

    sal_Bool doExport( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xDoc,
                        ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xOutputStream,
                            const OUString& rURL,
                                ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xHandler,
                                ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator >& rxStatusIndicator );

private:
    PageEntry* exportPage( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >&xDrawPage );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XGraphicExportFilter > mxGraphicExporter;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > mxInteractionHandler;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
