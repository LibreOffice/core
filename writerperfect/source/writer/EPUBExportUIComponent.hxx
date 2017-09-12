/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EPUBEXPORTUICOMPONENT_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EPUBEXPORTUICOMPONENT_HXX

#include <vector>

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/implbase.hxx>

namespace writerperfect
{

/// EPUB export UI component implementation.
class EPUBExportUIComponent : public cppu::WeakImplHelper
    <
    css::beans::XPropertyAccess,
    css::lang::XServiceInfo,
    css::ui::dialogs::XExecutableDialog
    >
{
public:
    EPUBExportUIComponent(const css::uno::Reference<css::uno::XComponentContext> &xContext);

    // XPropertyAccess
    css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getPropertyValues() override;
    void SAL_CALL setPropertyValues(const css::uno::Sequence<css::beans::PropertyValue> &rProperties) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString &ServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XExecutableDialog
    void SAL_CALL setTitle(const OUString &rTitle) override;
    sal_Int16 SAL_CALL execute() override;

private:
    /// The full set of property values.
    comphelper::SequenceAsHashMap maMediaDescriptor;
    /// The filter data key.
    comphelper::SequenceAsHashMap maFilterData;
};

} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
