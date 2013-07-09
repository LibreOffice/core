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

#include "MasterPageContainerProviders.hxx"

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "PreviewRenderer.hxx"
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/thumbnailview.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/image.hxx>
#include <vcl/pngread.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace sidebar {


//===== PagePreviewProvider ===================================================

PagePreviewProvider::PagePreviewProvider (void)
{
}




Image PagePreviewProvider::operator () (
    int nWidth,
    SdPage* pPage,
    ::sd::PreviewRenderer& rRenderer)
{
    Image aPreview;

    if (pPage != NULL)
    {
        // Use the given renderer to create a preview of the given page
        // object.
        aPreview = rRenderer.RenderPage(
            pPage,
            nWidth,
            OUString(),
            false);
    }

    return aPreview;
}




int PagePreviewProvider::GetCostIndex (void)
{
    return 5;
}




bool PagePreviewProvider::NeedsPageObject (void)
{
    return true;
}




//===== TemplatePreviewProvider ===============================================

TemplatePreviewProvider::TemplatePreviewProvider (const OUString& rsURL)
    : msURL(rsURL)
{
}




Image TemplatePreviewProvider::operator() (
    int nWidth,
    SdPage* pPage,
    ::sd::PreviewRenderer& rRenderer)
{
    // Unused parameters.
    (void)nWidth;
    (void)pPage;
    (void)rRenderer;

    return ThumbnailView::readThumbnail(msURL);
}




int TemplatePreviewProvider::GetCostIndex (void)
{
    return 10;
}




bool TemplatePreviewProvider::NeedsPageObject (void)
{
    return false;
}




//===== TemplatePageObjectProvider =============================================

TemplatePageObjectProvider::TemplatePageObjectProvider (const OUString& rsURL)
    : msURL(rsURL),
      mxDocumentShell()
{
}




SdPage* TemplatePageObjectProvider::operator() (SdDrawDocument* pContainerDocument)
{
    // Unused parameters.
    (void)pContainerDocument;

    SdPage* pPage = NULL;

    mxDocumentShell = NULL;
    ::sd::DrawDocShell* pDocumentShell = NULL;
    try
    {
        // Load the template document and return its first page.
        pDocumentShell = LoadDocument (msURL);
        if (pDocumentShell != NULL)
        {
            SdDrawDocument* pDocument = pDocumentShell->GetDoc();
            if (pDocument != NULL)
            {
                pPage = pDocument->GetMasterSdPage(0, PK_STANDARD);
                // In order to make the newly loaded master page deletable
                // when copied into documents it is marked as no "precious".
                // When it is modified then it is marked as "precious".
                if (pPage != NULL)
                    pPage->SetPrecious(false);
            }
        }
    }
    catch (const uno::RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
        pPage = NULL;
    }

    return pPage;
}




::sd::DrawDocShell* TemplatePageObjectProvider::LoadDocument (const OUString& sFileName)
{
    SfxApplication* pSfxApp = SFX_APP();
    SfxItemSet* pSet = new SfxAllItemSet (pSfxApp->GetPool());
    pSet->Put (SfxBoolItem (SID_TEMPLATE, sal_True));
    pSet->Put (SfxBoolItem (SID_PREVIEW, sal_True));
    if (pSfxApp->LoadTemplate (mxDocumentShell, sFileName, sal_True, pSet))
    {
        mxDocumentShell = NULL;
    }
    SfxObjectShell* pShell = mxDocumentShell;
    return PTR_CAST(::sd::DrawDocShell,pShell);
}




int TemplatePageObjectProvider::GetCostIndex (void)
{
    return 20;
}




bool TemplatePageObjectProvider::operator== (const PageObjectProvider& rProvider)
{
    const TemplatePageObjectProvider* pTemplatePageObjectProvider
        = dynamic_cast<const TemplatePageObjectProvider*>(&rProvider);
    if (pTemplatePageObjectProvider != NULL)
        return (msURL == pTemplatePageObjectProvider->msURL);
    else
        return false;
}




//===== DefaultPageObjectProvider ==============================================

DefaultPageObjectProvider::DefaultPageObjectProvider (void)
{
}




SdPage* DefaultPageObjectProvider::operator () (SdDrawDocument* pContainerDocument)
{
    SdPage* pLocalMasterPage = NULL;
    if (pContainerDocument != NULL)
    {
        sal_Int32 nIndex (0);
        SdPage* pLocalSlide = pContainerDocument->GetSdPage((sal_uInt16)nIndex, PK_STANDARD);
        if (pLocalSlide!=NULL && pLocalSlide->TRG_HasMasterPage())
            pLocalMasterPage = dynamic_cast<SdPage*>(&pLocalSlide->TRG_GetMasterPage());
    }

    if (pLocalMasterPage == NULL)
    {
        DBG_ASSERT(false, "can not create master page for slide");
    }

    return pLocalMasterPage;
}




int DefaultPageObjectProvider::GetCostIndex (void)
{
    return 15;
}




bool DefaultPageObjectProvider::operator== (const PageObjectProvider& rProvider)
{
    return (dynamic_cast<const DefaultPageObjectProvider*>(&rProvider) != NULL);
}




//===== ExistingPageProvider ==================================================

ExistingPageProvider::ExistingPageProvider (SdPage* pPage)
    : mpPage(pPage)
{
}




SdPage* ExistingPageProvider::operator() (SdDrawDocument* pDocument)
{
    (void)pDocument; // Unused parameter.

    return mpPage;
}




int ExistingPageProvider::GetCostIndex (void)
{
    return 0;
}




bool ExistingPageProvider::operator== (const PageObjectProvider& rProvider)
{
    const ExistingPageProvider* pExistingPageProvider
        = dynamic_cast<const ExistingPageProvider*>(&rProvider);
    if (pExistingPageProvider != NULL)
        return (mpPage == pExistingPageProvider->mpPage);
    else
        return false;
}


} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
