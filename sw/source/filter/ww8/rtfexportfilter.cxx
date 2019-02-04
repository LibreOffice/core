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

#include "rtfexportfilter.hxx"
#include "rtfexport.hxx"

#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <unotxdoc.hxx>
#include <viewsh.hxx>

#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

RtfExportFilter::RtfExportFilter(uno::Reference<uno::XComponentContext> xCtx)
    : m_xCtx(std::move(xCtx))
{
}

RtfExportFilter::~RtfExportFilter() = default;

sal_Bool RtfExportFilter::filter(const uno::Sequence<beans::PropertyValue>& aDescriptor)
{
    utl::MediaDescriptor aMediaDesc = aDescriptor;
    uno::Reference<io::XStream> xStream = aMediaDesc.getUnpackedValueOrDefault(
        utl::MediaDescriptor::PROP_STREAMFOROUTPUT(), uno::Reference<io::XStream>());
    std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(xStream, true);
    m_aWriter.SetStream(pStream.get());

    // get SwDoc*
    uno::Reference<uno::XInterface> xIfc(m_xSrcDoc, uno::UNO_QUERY);
    auto pTextDoc = dynamic_cast<SwXTextDocument*>(xIfc.get());
    if (!pTextDoc)
    {
        return false;
    }

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    if (!pDoc)
    {
        return false;
    }

    // fdo#37161 - update layout (if present), for SwWriteTable
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if (pViewShell != nullptr)
        pViewShell->CalcLayout();

    // get SwPaM*
    // we get SwPaM for the entire document; copy&paste is handled internally, not via UNO
    SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
    aPam.SetMark();
    aPam.Move(fnMoveBackward, GoInDoc);

    auto pCurPam = std::make_unique<SwPaM>(*aPam.End(), *aPam.Start());

    // export the document
    // (in a separate block so that it's destructed before the commit)
    {
        RtfExport aExport(this, pDoc, pCurPam.get(), &aPam, nullptr);
        aExport.ExportDocument(true);
    }

    // delete the pCurPam
    while (pCurPam->GetNext() != pCurPam.get())
        delete pCurPam->GetNext();

    return true;
}

void RtfExportFilter::cancel() {}

void RtfExportFilter::setSourceDocument(const uno::Reference<lang::XComponent>& xDoc)
{
    m_xSrcDoc = xDoc;
}

// UNO helpers

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Writer_RtfExport_get_implementation(uno::XComponentContext* pCtx,
                                                      uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new RtfExportFilter(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
