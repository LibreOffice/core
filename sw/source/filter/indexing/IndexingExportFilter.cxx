/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <IndexingExportFilter.hxx>
#include <IndexingExport.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>

#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/servicehelper.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <svl/outstrm.hxx>

using namespace css;

namespace sw
{
sal_Bool IndexingExportFilter::filter(const uno::Sequence<beans::PropertyValue>& aDescriptor)
{
    bool bReturn = false;

    utl::MediaDescriptor aMediaDesc = aDescriptor;

    // Actually get the SwRootFrame to call dumpAsXml
    auto pXTextDocument = comphelper::getFromUnoTunnel<SwXTextDocument>(m_xSourceDocument);
    if (pXTextDocument)
    {
        uno::Reference<io::XOutputStream> xOutputStream = aMediaDesc.getUnpackedValueOrDefault(
            utl::MediaDescriptor::PROP_OUTPUTSTREAM, uno::Reference<io::XOutputStream>());

        std::unique_ptr<SvStream> pStream(new SvOutputStream(xOutputStream));
        SwDocShell* pShell = pXTextDocument->GetDocShell();
        SwDoc* pDoc = pShell->GetDoc();
        if (pDoc)
        {
            IndexingExport aIndexingExport(*pStream, pDoc);
            bReturn = aIndexingExport.runExport();
        }
    }

    return bReturn;
}

} // end namespace sw

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_IndexingExportFilter_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new sw::IndexingExportFilter());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
