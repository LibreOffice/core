/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace sw
{
class IndexingExportFilter final
    : public cppu::WeakImplHelper<css::document::XFilter, css::document::XExporter,
                                  css::lang::XInitialization, css::lang::XServiceInfo>
{
private:
    css::uno::Reference<css::lang::XComponent> m_xSourceDocument;

public:
    IndexingExportFilter() {}

    // XFilter
    virtual sal_Bool SAL_CALL
    filter(const css::uno::Sequence<css::beans::PropertyValue>& aDescriptor) override;

    virtual void SAL_CALL cancel() override {}

    // XExporter
    virtual void SAL_CALL
    setSourceDocument(const css::uno::Reference<css::lang::XComponent>& xDocument) override
    {
        m_xSourceDocument = xDocument;
    }

    // XInitialization
    virtual void SAL_CALL
    initialize(const css::uno::Sequence<css::uno::Any>& /*aArguments*/) override
    {
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.Writer.IndexingExportFilter"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const& rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.document.ExportFilter"_ustr };
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
