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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_RTFEXPORTFILTER_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_RTFEXPORTFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <shellio.hxx>

/// Dummy Writer implementation to be able to use the string format methods of the base class
class RtfWriter : public Writer
{
protected:
    sal_uLong WriteStream() override
    {
        return 0;
    }
};

/// The physical access to the RTF document (for writing).
class RtfExportFilter : public cppu::WeakImplHelper
    <
    css::document::XFilter,
    css::document::XExporter
    >
{
protected:
    css::uno::Reference<css::uno::XComponentContext> m_xCtx;
    css::uno::Reference<css::lang::XComponent> m_xSrcDoc;
public:
    explicit RtfExportFilter(const css::uno::Reference<css::uno::XComponentContext>& xCtx);
    virtual ~RtfExportFilter();

    // XFilter
    virtual sal_Bool SAL_CALL filter(const css::uno::Sequence<css::beans::PropertyValue>& aDescriptor) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel() throw(css::uno::RuntimeException, std::exception) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument(const css::uno::Reference<css::lang::XComponent>& xDoc)
    throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    RtfWriter m_aWriter;
};

OUString RtfExport_getImplementationName();
css::uno::Sequence<OUString> SAL_CALL RtfExport_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL RtfExport_createInstance(const css::uno::Reference<css::uno::XComponentContext>& xCtx) throw(css::uno::Exception);

#define IMPL_NAME_RTFEXPORT "com.sun.star.comp.Writer.RtfExport"

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFEXPORTFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
