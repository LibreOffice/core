/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "MasterPageContainerProviders.hxx"

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "PreviewRenderer.hxx"
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/image.hxx>
#include <vcl/pngread.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::toolpanel::controls;


namespace sd { namespace toolpanel { namespace controls {


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
            String::CreateFromAscii(""),
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

TemplatePreviewProvider::TemplatePreviewProvider (const ::rtl::OUString& rsURL)
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

    // Load the thumbnail from a template document.
    uno::Reference<io::XInputStream> xIStream;

    uno::Reference< lang::XMultiServiceFactory > xServiceManager (
        ::comphelper::getProcessServiceFactory());
    if (xServiceManager.is())
    {
        try
        {
            uno::Reference<lang::XSingleServiceFactory> xStorageFactory(
                xServiceManager->createInstance(
                    ::rtl::OUString::createFromAscii(
                        "com.sun.star.embed.StorageFactory")),
                uno::UNO_QUERY);

            if (xStorageFactory.is())
            {
                uno::Sequence<uno::Any> aArgs (2);
                aArgs[0] <<= msURL;
                aArgs[1] <<= embed::ElementModes::READ;
                uno::Reference<embed::XStorage> xDocStorage (
                    xStorageFactory->createInstanceWithArguments(aArgs),
                    uno::UNO_QUERY);

                try
                {
                    if (xDocStorage.is())
                    {
                        uno::Reference<embed::XStorage> xStorage (
                            xDocStorage->openStorageElement(
                                ::rtl::OUString::createFromAscii("Thumbnails"),
                                embed::ElementModes::READ));
                        if (xStorage.is())
                        {
                            uno::Reference<io::XStream> xThumbnailCopy (
                                xStorage->cloneStreamElement(
                                    ::rtl::OUString::createFromAscii(
                                        "thumbnail.png")));
                            if (xThumbnailCopy.is())
                                xIStream = xThumbnailCopy->getInputStream();
                        }
                    }
                }
                catch (uno::Exception& rException)
                {
                    OSL_TRACE (
                        "caught exception while trying to access Thumbnail/thumbnail.png of %s: %s",
                        ::rtl::OUStringToOString(msURL,
                            RTL_TEXTENCODING_UTF8).getStr(),
                        ::rtl::OUStringToOString(rException.Message,
                            RTL_TEXTENCODING_UTF8).getStr());
                }

                try
                {
                    // An (older) implementation had a bug - The storage
                    // name was "Thumbnail" instead of "Thumbnails".  The
                    // old name is still used as fallback but this code can
                    // be removed soon.
                    if ( ! xIStream.is())
                    {
                        uno::Reference<embed::XStorage> xStorage (
                            xDocStorage->openStorageElement(
                                ::rtl::OUString::createFromAscii("Thumbnail"),
                                embed::ElementModes::READ));
                        if (xStorage.is())
                        {
                            uno::Reference<io::XStream> xThumbnailCopy (
                                xStorage->cloneStreamElement(
                                    ::rtl::OUString::createFromAscii(
                                        "thumbnail.png")));
                            if (xThumbnailCopy.is())
                                xIStream = xThumbnailCopy->getInputStream();
                        }
                    }
                }
                catch (uno::Exception& rException)
                {
                    OSL_TRACE (
                        "caught exception while trying to access Thumbnails/thumbnail.png of %s: %s",
                        ::rtl::OUStringToOString(msURL,
                            RTL_TEXTENCODING_UTF8).getStr(),
                        ::rtl::OUStringToOString(rException.Message,
                            RTL_TEXTENCODING_UTF8).getStr());
                }
            }
        }
        catch (uno::Exception& rException)
        {
            OSL_TRACE (
                "caught exception while trying to access tuhmbnail of %s: %s",
                ::rtl::OUStringToOString(msURL,
                    RTL_TEXTENCODING_UTF8).getStr(),
                ::rtl::OUStringToOString(rException.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
        }
    }

    // Extract the image from the stream.
    BitmapEx aThumbnail;
    if (xIStream.is())
    {
        ::std::auto_ptr<SvStream> pStream (
            ::utl::UcbStreamHelper::CreateStream (xIStream));
        ::vcl::PNGReader aReader (*pStream);
        aThumbnail = aReader.Read ();
    }

    // Note that the preview is returned without scaling it to the desired
    // width.  This gives the caller the chance to take advantage of a
    // possibly larger resolution then was asked for.
    return aThumbnail;
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

TemplatePageObjectProvider::TemplatePageObjectProvider (const ::rtl::OUString& rsURL)
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
    catch (uno::RuntimeException)
    {
        DBG_UNHANDLED_EXCEPTION();
        pPage = NULL;
    }

    return pPage;
}




::sd::DrawDocShell* TemplatePageObjectProvider::LoadDocument (const ::rtl::OUString& sFileName)
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


} } } // end of namespace ::sd::toolpanel::controls
