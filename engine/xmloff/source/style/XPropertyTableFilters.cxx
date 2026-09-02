/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <xmlxtexp.hxx>
#include <xmlxtimp.hxx>

using namespace ::com::sun::star;

namespace
{

enum class XPropertyTableFilterMode
{
    Import,
    Export
};

/** Read or write one of the property tables of a drawing, such as the colour table or the gradient
    table.

    The descriptor holds URL, either a palette file or a path inside a package, Storage, the package
    a relative URL is resolved against, PropertyTable, the name container holding the entries, and
    on the way in Referer, the document the table is read for.
 */
class XPropertyTableFilter final
    : public cppu::WeakImplHelper<document::XFilter, lang::XServiceInfo>
{
public:
    explicit XPropertyTableFilter(XPropertyTableFilterMode eMode)
        : meMode(eMode)
    {
    }

    virtual bool SAL_CALL filter(const cpo::uno::Sequence<beans::PropertyValue>& rDescriptor) override;
    virtual void SAL_CALL cancel() override {}

    virtual OUString SAL_CALL getImplementationName() override;
    virtual bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual cpo::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    XPropertyTableFilterMode meMode;
};

bool XPropertyTableFilter::filter(const cpo::uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    comphelper::NamedValueCollection aDescriptor(rDescriptor);

    uno::Reference<container::XNameContainer> xTable(aDescriptor.getOrDefault(
        u"PropertyTable"_ustr, uno::Reference<container::XNameContainer>()));
    if (!xTable.is())
        return false;

    const OUString aURL(aDescriptor.getOrDefault(u"URL"_ustr, OUString()));
    const uno::Reference<embed::XStorage> xStorage(
        aDescriptor.getOrDefault(u"Storage"_ustr, uno::Reference<embed::XStorage>()));

    if (meMode == XPropertyTableFilterMode::Import)
        return SvxXMLXTableImport::load(aURL,
                                        aDescriptor.getOrDefault(u"Referer"_ustr, OUString()),
                                        xStorage, xTable);

    return SvxXMLXTableExportComponent::save(aURL, xTable, xStorage);
}

OUString XPropertyTableFilter::getImplementationName()
{
    if (meMode == XPropertyTableFilterMode::Import)
        return u"com.sun.star.comp.Svx.XPropertyTableImporter"_ustr;
    return u"com.sun.star.comp.Svx.XPropertyTableExporter"_ustr;
}

bool XPropertyTableFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence<OUString> XPropertyTableFilter::getSupportedServiceNames()
{
    return { getImplementationName() };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Svx_XPropertyTableImporter_get_implementation(
    cpo::uno::XComponentContext*, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new XPropertyTableFilter(XPropertyTableFilterMode::Import));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Svx_XPropertyTableExporter_get_implementation(
    cpo::uno::XComponentContext*, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new XPropertyTableFilter(XPropertyTableFilterMode::Export));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
