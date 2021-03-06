/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace writerperfect
{
/// EPUB export UI component implementation.
class EPUBExportUIComponent
    : public cppu::WeakImplHelper<css::beans::XPropertyAccess, css::lang::XInitialization,
                                  css::lang::XServiceInfo, css::ui::dialogs::XExecutableDialog,
                                  css::document::XExporter>
{
public:
    EPUBExportUIComponent(css::uno::Reference<css::uno::XComponentContext> xContext);

    // XPropertyAccess
    css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getPropertyValues() override;
    void SAL_CALL
    setPropertyValues(const css::uno::Sequence<css::beans::PropertyValue>& rProperties) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XExecutableDialog
    void SAL_CALL setTitle(const OUString& rTitle) override;
    sal_Int16 SAL_CALL execute() override;

    // XExporter
    void SAL_CALL
    setSourceDocument(const css::uno::Reference<css::lang::XComponent>& xDocument) override;

    // XInitialization
    void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

private:
    /// The full set of property values.
    comphelper::SequenceAsHashMap maMediaDescriptor;
    /// The filter data key.
    comphelper::SequenceAsHashMap maFilterData;
    /// UNO context.
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::lang::XComponent> mxSourceDocument;
    css::uno::Reference<css::awt::XWindow> mxDialogParent;
};

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
