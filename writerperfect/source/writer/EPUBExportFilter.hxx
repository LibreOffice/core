/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace writerperfect
{
namespace exp
{
struct FixedLayoutPage;
}

/// EPUB export XFilter implementation.
class EPUBExportFilter
    : public cppu::WeakImplHelper<css::document::XFilter, css::document::XExporter,
                                  css::lang::XServiceInfo>
{
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::lang::XComponent> mxSourceDocument;

public:
    EPUBExportFilter(css::uno::Reference<css::uno::XComponentContext> xContext);

    // XFilter
    sal_Bool SAL_CALL
    filter(const css::uno::Sequence<css::beans::PropertyValue>& rDescriptor) override;
    void SAL_CALL cancel() override;

    // XExporter
    void SAL_CALL
    setSourceDocument(const css::uno::Reference<css::lang::XComponent>& xDocument) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    /// Gives the default EPUB version.
    static sal_Int32 GetDefaultVersion();
    /// Gives the default split method.
    static sal_Int32 GetDefaultSplitMethod();
    /// Gives the default layout method.
    static sal_Int32 GetDefaultLayoutMethod();

private:
    /// Create page metafiles in case of fixed layout.
    void CreateMetafiles(std::vector<exp::FixedLayoutPage>& rPageMetafiles);
};

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
