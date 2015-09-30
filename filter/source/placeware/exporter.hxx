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
#ifndef INCLUDED_FILTER_SOURCE_PLACEWARE_EXPORTER_HXX
#define INCLUDED_FILTER_SOURCE_PLACEWARE_EXPORTER_HXX

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
    PlaceWareExporter( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    ~PlaceWareExporter();

    bool doExport( css::uno::Reference< css::lang::XComponent > xDoc,
                   css::uno::Reference < css::io::XOutputStream > xOutputStream,
                   const OUString& rURL,
                   css::uno::Reference < css::uno::XInterface > xHandler,
                   css::uno::Reference < css::task::XStatusIndicator >& rxStatusIndicator );

private:
    PageEntry* exportPage( css::uno::Reference< css::drawing::XDrawPage >&xDrawPage );

    css::uno::Reference< css::uno::XComponentContext >        mxContext;
    css::uno::Reference< css::drawing::XGraphicExportFilter > mxGraphicExporter;
    css::uno::Reference< css::task::XInteractionHandler >     mxInteractionHandler;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
