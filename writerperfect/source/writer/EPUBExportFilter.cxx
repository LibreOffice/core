/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBExportFilter.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;

namespace writerperfect
{

EPUBExportFilter::EPUBExportFilter()
{
}

sal_Bool EPUBExportFilter::filter(const uno::Sequence<beans::PropertyValue> &/*rDescriptor*/)
{
    SAL_WARN("writerperfect", "EPUBExportFilter::filter: implement me");
    return true;
}

void EPUBExportFilter::cancel()
{
}

void EPUBExportFilter::setSourceDocument(const uno::Reference<lang::XComponent> &xDocument)
{
    mxSourceDocument = xDocument;
}

OUString EPUBExportFilter::getImplementationName()
{
    return OUString("com.sun.star.comp.Writer.EPUBExportFilter");
}

sal_Bool EPUBExportFilter::supportsService(const OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> EPUBExportFilter::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet =
    {
        OUString("com.sun.star.document.ExportFilter")
    };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface *SAL_CALL com_sun_star_comp_Writer_EPUBExportFilter_get_implementation(uno::XComponentContext *, uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new EPUBExportFilter);
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
