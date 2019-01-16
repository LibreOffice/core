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
#include <cppuhelper/implbase.hxx>
#include <shellio.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{
class XComponentContext;
}
}
}
}

/// Dummy Writer implementation to be able to use the string format methods of the base class
class RtfWriter : public Writer
{
protected:
    ErrCode WriteStream() override { return ERRCODE_NONE; }
};

/// The physical access to the RTF document (for writing).
class RtfExportFilter final
    : public cppu::WeakImplHelper<css::document::XFilter, css::document::XExporter>
{
    css::uno::Reference<css::uno::XComponentContext> m_xCtx;
    css::uno::Reference<css::lang::XComponent> m_xSrcDoc;
    RtfWriter m_aWriter;

public:
    explicit RtfExportFilter(css::uno::Reference<css::uno::XComponentContext> xCtx);
    ~RtfExportFilter() override;

    // XFilter
    sal_Bool SAL_CALL
    filter(const css::uno::Sequence<css::beans::PropertyValue>& aDescriptor) override;
    void SAL_CALL cancel() override;

    // XExporter
    void SAL_CALL
    setSourceDocument(const css::uno::Reference<css::lang::XComponent>& xDoc) override;

    Writer& GetWriter() { return m_aWriter; }
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFEXPORTFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
