/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cpo/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>

namespace com::sun::star {
    namespace drawing {
        class XShapes;
        class XShape;
    }
}

class GraphicExportFilter :
    public cppu::WeakImplHelper < css::document::XFilter, css::document::XExporter, css::lang::XInitialization, css::lang::XServiceInfo >
{
    css::uno::Reference< cpo::uno::XComponentContext >  mxContext;
    css::uno::Reference< css::lang::XComponent >        mxDocument;
    css::uno::Reference< css::io::XOutputStream >       mxOutputStream;

    cpo::uno::Sequence< css::beans::PropertyValue >     maFilterDataSequence;

    OUString    maFilterExtension;
    OUString    maFilterOptions;
    sal_Int32   mnTargetWidth;
    sal_Int32   mnTargetHeight;
    bool        mbSelectionOnly;

    void gatherProperties( const cpo::uno::Sequence< css::beans::PropertyValue > & rDescriptor );
    bool filterRenderDocument() const;
    bool filterExportShape(
            const cpo::uno::Sequence< css::beans::PropertyValue > & rDescriptor,
            const css::uno::Reference< css::drawing::XShapes > & rxShapes,
            const css::uno::Reference< css::drawing::XShape > & rxShape ) const;

public:
    explicit GraphicExportFilter( css::uno::Reference< cpo::uno::XComponentContext > xContext );
    virtual ~GraphicExportFilter() override;

    //  XServiceInfo
    virtual bool supportsService(const OUString& sServiceName) override;
    virtual OUString getImplementationName() override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XFilter
    virtual bool filter( const cpo::uno::Sequence< css::beans::PropertyValue > & rDescriptor ) override;
    virtual void cancel( ) override;

    // XExporter
    virtual void setSourceDocument( const css::uno::Reference< css::lang::XComponent > & xDocument ) override;

    // XInitialization
    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any > & rArguments ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
