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

PagePreviewProvider::PagePreviewProvider()
{
}

Image PagePreviewProvider::operator () (
    int nWidth,
    SdPage* pPage,
    ::sd::PreviewRenderer& rRenderer)
{
    Image aPreview;

    if (pPage != nullptr)
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

int PagePreviewProvider::GetCostIndex()
{
    return 5;
}

bool PagePreviewProvider::NeedsPageObject()
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

    return Image(ThumbnailView::readThumbnail(msURL));
}

int TemplatePreviewProvider::GetCostIndex()
{
    return 10;
}

bool TemplatePreviewProvider::NeedsPageObject()
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

    SdPage* pPage = nullptr;

    mxDocumentShell = nullptr;
    ::sd::DrawDocShell* pDocumentShell = nullptr;
    try
    {
        // Load the template document and return its first page.
        pDocumentShell = LoadDocument (msURL);
        if (pDocumentShell != nullptr)
        {
            SdDrawDocument* pDocument = pDocumentShell->GetDoc();
            if (pDocument != nullptr)
            {
                pPage = pDocument->GetMasterSdPage(0, PK_STANDARD);
                // In order to make the newly loaded master page deletable
                // when copied into documents it is marked as no "precious".
                // When it is modified then it is marked as "precious".
                if (pPage != nullptr)
                    pPage->SetPrecious(false);
            }
        }
    }
    catch (const uno::RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
        pPage = nullptr;
    }

    return pPage;
}

::sd::DrawDocShell* TemplatePageObjectProvider::LoadDocument (const OUString& sFileName)
{
    SfxApplication* pSfxApp = SfxGetpApp();
    SfxItemSet* pSet = new SfxAllItemSet (pSfxApp->GetPool());
    pSet->Put (SfxBoolItem (SID_TEMPLATE, true));
    pSet->Put (SfxBoolItem (SID_PREVIEW, true));
    if (pSfxApp->LoadTemplate (mxDocumentShell, sFileName, true, pSet))
    {
        mxDocumentShell = nullptr;
    }
    SfxObjectShell* pShell = mxDocumentShell;
    return dynamic_cast< ::sd::DrawDocShell *>( pShell );
}

int TemplatePageObjectProvider::GetCostIndex()
{
    return 20;
}

bool TemplatePageObjectProvider::operator== (const PageObjectProvider& rProvider)
{
    const TemplatePageObjectProvider* pTemplatePageObjectProvider
        = dynamic_cast<const TemplatePageObjectProvider*>(&rProvider);
    if (pTemplatePageObjectProvider != nullptr)
        return (msURL == pTemplatePageObjectProvider->msURL);
    else
        return false;
}

//===== DefaultPageObjectProvider ==============================================

DefaultPageObjectProvider::DefaultPageObjectProvider()
{
}

SdPage* DefaultPageObjectProvider::operator () (SdDrawDocument* pContainerDocument)
{
    SdPage* pLocalMasterPage = nullptr;
    if (pContainerDocument != nullptr)
    {
        sal_Int32 nIndex (0);
        SdPage* pLocalSlide = pContainerDocument->GetSdPage((sal_uInt16)nIndex, PK_STANDARD);
        if (pLocalSlide!=nullptr && pLocalSlide->TRG_HasMasterPage())
            pLocalMasterPage = dynamic_cast<SdPage*>(&pLocalSlide->TRG_GetMasterPage());
    }

    if (pLocalMasterPage == nullptr)
    {
        SAL_WARN( "sd", "can not create master page for slide");
    }

    return pLocalMasterPage;
}

int DefaultPageObjectProvider::GetCostIndex()
{
    return 15;
}

bool DefaultPageObjectProvider::operator== (const PageObjectProvider& rProvider)
{
    return (dynamic_cast<const DefaultPageObjectProvider*>(&rProvider) != nullptr);
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

int ExistingPageProvider::GetCostIndex()
{
    return 0;
}

bool ExistingPageProvider::operator== (const PageObjectProvider& rProvider)
{
    const ExistingPageProvider* pExistingPageProvider
        = dynamic_cast<const ExistingPageProvider*>(&rProvider);
    if (pExistingPageProvider != nullptr)
        return (mpPage == pExistingPageProvider->mpPage);
    else
        return false;
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
