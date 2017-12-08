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

#include <memory>
#include "docxexportfilter.hxx"
#include "rtfexportfilter.hxx"
#include "docxexport.hxx"

#include <docsh.hxx>
#include <editsh.hxx>
#include <pam.hxx>
#include <PostItMgr.hxx>
#include <unotxdoc.hxx>
#include <IDocumentLayoutAccess.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <unotools/mediadescriptor.hxx>

using namespace ::comphelper;
using namespace ::com::sun::star;

DocxExportFilter::DocxExportFilter( const uno::Reference< uno::XComponentContext >& xContext )
    : oox::core::XmlFilterBase( xContext )
{
}

bool DocxExportFilter::exportDocument()
{
    // get SwDoc*
    uno::Reference< uno::XInterface > xIfc( getModel(), uno::UNO_QUERY );
    SwXTextDocument *pTextDoc = dynamic_cast< SwXTextDocument * >( xIfc.get() );
    if ( !pTextDoc )
        return false;

    SwDoc *pDoc = pTextDoc->GetDocShell()->GetDoc();
    if ( !pDoc )
        return false;

    // update layout (if present), for SwWriteTable
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if (pViewShell != nullptr)
        pViewShell->CalcLayout();

    // if we have an active postit window, update the document model
    if (pViewShell &&
        pViewShell->GetPostItMgr() &&
        pViewShell->GetPostItMgr()->HasActiveSidebarWin())
    {
        pViewShell->GetPostItMgr()->UpdateDataOnActiveSidebarWin();
    }

    // get SwPaM*
    // FIXME so far we get SwPaM for the entire document; probably we should
    // be able to output just the selection as well - though no idea how to
    // get the correct SwPaM* then...
    SwPaM aPam( pDoc->GetNodes().GetEndOfContent() );
    aPam.SetMark();
    aPam.Move( fnMoveBackward, GoInDoc );

    std::unique_ptr<SwPaM> pCurPam( new SwPaM( *aPam.End(), *aPam.Start() ) );

    OUString aFilterName;
    getMediaDescriptor()[utl::MediaDescriptor::PROP_FILTERNAME()] >>= aFilterName;
    bool bDocm = aFilterName.endsWith("VBA");

    // export the document
    // (in a separate block so that it's destructed before the commit)
    {
        DocxExport aExport( this, pDoc, pCurPam.get(), &aPam, bDocm );
        aExport.ExportDocument( true ); // FIXME support exporting selection only
    }

    commitStorage();

    // delete the pCurPam
    while ( pCurPam->GetNext() != pCurPam.get() )
        delete pCurPam->GetNext();

    return true;
}

// UNO stuff so that the filter is registered
#define IMPL_NAME "com.sun.star.comp.Writer.DocxExport"

OUString DocxExport_getImplementationName()
{
    return OUString( IMPL_NAME );
}

OUString DocxExportFilter::getImplementationName()
{
    return DocxExport_getImplementationName();
}

uno::Sequence< OUString > DocxExport_getSupportedServiceNames() throw()
{
    return uno::Sequence< OUString > { "com.sun.star.document.ExportFilter" };
}

/// @throws uno::Exception
uno::Reference< uno::XInterface > DocxExport_createInstance(const uno::Reference< uno::XComponentContext > & xCtx )
{
    return static_cast<cppu::OWeakObject*>(new DocxExportFilter( xCtx ));
}

extern "C"
{

::cppu::ImplementationEntry const entries [] =
{
    {
        DocxExport_createInstance, DocxExport_getImplementationName,
        DocxExport_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        nullptr, 0
    },
    {
        RtfExport_createInstance, RtfExport_getImplementationName,
        RtfExport_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

SAL_DLLPUBLIC_EXPORT void* msword_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, entries );
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
