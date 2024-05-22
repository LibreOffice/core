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
#include "docxexport.hxx"

#include <docsh.hxx>
#include <pam.hxx>
#include <PostItMgr.hxx>
#include <unotxdoc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <viewsh.hxx>

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

    OUString aFilterName;
    auto& rMediaDescriptor = getMediaDescriptor();
    rMediaDescriptor[utl::MediaDescriptor::PROP_FILTERNAME] >>= aFilterName;
    bool bDocm = aFilterName.endsWith("VBA");

    if (!bDocm)
    {
        // Check whether application is in headless mode
        if (!Application::IsHeadlessModeEnabled())
        {
            uno::Reference<document::XStorageBasedDocument> xStorageBasedDocument(
                pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
            if (xStorageBasedDocument.is())
            {
                uno::Reference<embed::XStorage> xDocumentStorage =
                    xStorageBasedDocument->getDocumentStorage();
                if (xDocumentStorage.is() && xDocumentStorage->hasByName(u"_MS_VBA_Macros"_ustr))
                {
                    // Let user know that macros won't be saved in this format
                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                            nullptr,
                            VclMessageType::Warning, VclButtonsType::OkCancel,
                            SwResId(STR_CANT_SAVE_MACROS))
                    );
                    if (xBox->run() == RET_CANCEL)
                        return false;
                }
            }
        }
    }

    // get SwPaM*
    // FIXME so far we get SwPaM for the entire document; probably we should
    // be able to output just the selection as well - though no idea how to
    // get the correct SwPaM* then...
    SwPaM aPam( pDoc->GetNodes().GetEndOfContent() );
    aPam.SetMark();
    aPam.Move( fnMoveBackward, GoInDoc );

    std::shared_ptr<SwUnoCursor> pCurPam(pDoc->CreateUnoCursor(*aPam.End(), false));
    pCurPam->SetMark();
    *pCurPam->GetPoint() = *aPam.Start();

    // export the document
    // (in a separate block so that it's destructed before the commit)
    {
        DocxExport aExport(*this, *pDoc, pCurPam, aPam, bDocm, isExportTemplate());
        aExport.ExportDocument( true ); // FIXME support exporting selection only
    }

    commitStorage();

    // delete the pCurPam
    while ( pCurPam->GetNext() != pCurPam.get() )
        delete pCurPam->GetNext();

    return true;
}

// UNO stuff so that the filter is registered

OUString DocxExportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Writer.DocxExport"_ustr;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Writer_DocxExport_get_implementation(uno::XComponentContext* pCtx,
                                                       uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new DocxExportFilter(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
