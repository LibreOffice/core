/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_PROVIDERS_HXX
#define SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_PROVIDERS_HXX

#include <rtl/ustring.hxx>
#include <sfx2/objsh.hxx>

class Image;
class SdDrawDocument;
class SdPage;
namespace sd { class PreviewRenderer; }
namespace sd { class DrawDocShell; }


namespace sd { namespace toolpanel { namespace controls {


/** Interface for a provider of page objects.  It is used by the
    MasterPageDescriptor to create master page objects on demand.
*/
class PageObjectProvider
{
public:
    /** Return a master page either by returning an already existing one, by
        creating a new page, or by loading a document.
        @param pDocument
            The document of the MasterPageContainer.  It may be used to
            create new pages.
    */
    virtual SdPage* operator() (SdDrawDocument* pDocument) = 0;

    /** An abstract value for the expected cost of providing a master page
        object.
        @return
            A value of 0 represents for the lowest cost, i.e. an almost
            immediate return.  Positive values stand for higher costs.
            Negative values are not supported.
    */
    virtual int GetCostIndex (void) = 0;

    virtual bool operator== (const PageObjectProvider& rProvider) = 0;
};




class PreviewProvider
{
public:
    /** Create a preview image in the specified width.
        @param nWidth
            Requested width of the preview.  The calling method can cope
            with other sizes as well but the resulting image quality is
            better when the returned image has the requested size.
        @param pPage
            Page object for which a preview is requested.  This may be NULL
            when the page object is expensive to get and the PreviewProvider
            does not need this object (NeedsPageObject() returns false.)
        @param rRenderer
            This PreviewRenderer may be used by the PreviewProvider to
            create a preview image.
    */
    virtual Image operator() (int nWidth, SdPage* pPage, ::sd::PreviewRenderer& rRenderer) = 0;

    /** Return a value that indicates how expensive the creation of a
        preview image is.  The higher the returned value the more expensive
        is the preview creation.  Return 0 when the preview is already
        present and can be returned immediately.
    */
    virtual int GetCostIndex (void) = 0;

    /** Return whether the page object passed is necessary to create a
        preview.
    */
    virtual bool NeedsPageObject (void) = 0;
};




/** Provide previews of existing page objects by rendering them.
*/
class PagePreviewProvider : public PreviewProvider
{
public:
    PagePreviewProvider (void);
    virtual Image operator () (int nWidth, SdPage* pPage, ::sd::PreviewRenderer& rRenderer);
    virtual int GetCostIndex (void);
    virtual bool NeedsPageObject (void);
private:
};




/** Provide master page objects for template documents for which only the
    URL is given.
*/
class TemplatePageObjectProvider : public PageObjectProvider
{
public:
    TemplatePageObjectProvider (const ::rtl::OUString& rsURL);
    virtual ~TemplatePageObjectProvider (void) {};
    virtual SdPage* operator () (SdDrawDocument* pDocument);
    virtual int GetCostIndex (void);
    virtual bool operator== (const PageObjectProvider& rProvider);
private:
    ::rtl::OUString msURL;
    SfxObjectShellLock mxDocumentShell;
    ::sd::DrawDocShell* LoadDocument (const ::rtl::OUString& sFileName);
};




/** Provide previews for template documents by loading the thumbnails from
    the documents.
*/
class TemplatePreviewProvider : public PreviewProvider
{
public:
    TemplatePreviewProvider (const ::rtl::OUString& rsURL);
    virtual ~TemplatePreviewProvider (void) {};
    virtual Image operator() (int nWidth, SdPage* pPage, ::sd::PreviewRenderer& rRenderer);
    virtual int GetCostIndex (void);
    virtual bool NeedsPageObject (void);
private:
    ::rtl::OUString msURL;
};




/** Create an empty default master page.
*/
class DefaultPageObjectProvider : public PageObjectProvider
{
public:
    DefaultPageObjectProvider (void);
    virtual SdPage* operator () (SdDrawDocument* pDocument);
    virtual int GetCostIndex (void);
    virtual bool operator== (const PageObjectProvider& rProvider);
};



/** This implementation of the PageObjectProvider simply returns an already
    existing master page object.
*/
class ExistingPageProvider : public ::sd::toolpanel::controls::PageObjectProvider
{
public:
    ExistingPageProvider (SdPage* pPage);
    virtual SdPage* operator() (SdDrawDocument* pDocument);
    virtual int GetCostIndex (void);
    virtual bool operator== (const PageObjectProvider& rProvider);
private:
    SdPage* mpPage;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
