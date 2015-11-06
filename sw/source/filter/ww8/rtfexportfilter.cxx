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

#include <rtfexportfilter.hxx>
#include <rtfexport.hxx>

#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <editsh.hxx>
#include <unotxdoc.hxx>

#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

RtfExportFilter::RtfExportFilter(const uno::Reference< uno::XComponentContext >& xCtx)
    : m_xCtx(xCtx)
{
}

RtfExportFilter::~RtfExportFilter()
{
}

sal_Bool RtfExportFilter::filter(const uno::Sequence< beans::PropertyValue >& aDescriptor) throw(uno::RuntimeException, std::exception)
{
    utl::MediaDescriptor aMediaDesc = aDescriptor;
    uno::Reference<io::XStream> xStream = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_STREAMFOROUTPUT(), uno::Reference< io::XStream >());
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(xStream, true);
    m_aWriter.SetStream(pStream);

    // get SwDoc*
    uno::Reference< uno::XInterface > xIfc(m_xSrcDoc, uno::UNO_QUERY);
    SwXTextDocument* pTextDoc = dynamic_cast< SwXTextDocument* >(xIfc.get());
    if (!pTextDoc)
    {
        return sal_False;
    }

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    if (!pDoc)
    {
        return sal_False;
    }

    // fdo#37161 - update layout (if present), for SwWriteTable
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if (pViewShell != NULL)
        pViewShell->CalcLayout();

    // get SwPaM*
    // we get SwPaM for the entire document; copy&paste is handled internally, not via UNO
    SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
    aPam.SetMark();
    aPam.Move(fnMoveBackward, fnGoDoc);

    SwPaM* pCurPam = new SwPaM(*aPam.End(), *aPam.Start());

    // export the document
    // (in a separate block so that it's destructed before the commit)
    {
        RtfExport aExport(this, pDoc, pCurPam, &aPam, NULL);
        aExport.ExportDocument(true);
    }

    // delete the pCurPam
    while (pCurPam->GetNext() != pCurPam)
        delete pCurPam->GetNext();
    delete pCurPam;
    delete pStream;

    return sal_True;
}

void RtfExportFilter::cancel() throw(uno::RuntimeException, std::exception)
{
}

void RtfExportFilter::setSourceDocument(const uno::Reference< lang::XComponent >& xDoc) throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    m_xSrcDoc = xDoc;
}

// UNO helpers

OUString RtfExport_getImplementationName()
{
    return OUString(IMPL_NAME_RTFEXPORT);
}

uno::Sequence< OUString > SAL_CALL RtfExport_getSupportedServiceNames() throw()
{
    return uno::Sequence< OUString > { "com.sun.star.document.ExportFilter" };
}

uno::Reference< uno::XInterface > SAL_CALL RtfExport_createInstance(const uno::Reference< uno::XComponentContext >& xCtx) throw(uno::Exception)
{
    return static_cast<cppu::OWeakObject*>(new RtfExportFilter(xCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
