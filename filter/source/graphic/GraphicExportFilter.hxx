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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHIC_GRAPHICEXPORTFILTER_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHIC_GRAPHICEXPORTFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::beans;
using namespace css::document;

class GraphicExportFilter :
    public cppu::WeakImplHelper < XFilter, XExporter, XInitialization >
{
    Reference<XComponent>               mxDocument;
    Reference<io::XOutputStream>        mxOutputStream;

    void gatherProperties( const Sequence<PropertyValue>& rDescriptor );

    OUString    mFilterExtension;

    Sequence<PropertyValue> mFilterDataSequence;

    sal_Int32 mTargetWidth;
    sal_Int32 mTargetHeight;

public:
    explicit GraphicExportFilter( const Reference<XComponentContext>& rxContext );
    virtual ~GraphicExportFilter() override;

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence<PropertyValue>& rDescriptor ) override;
    virtual void SAL_CALL cancel( ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDocument ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence<Any>& aArguments ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
