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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_MASTERPAGECONTAINERPROVIDERS_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_MASTERPAGECONTAINERPROVIDERS_HXX

#include <rtl/ustring.hxx>
#include <sfx2/objsh.hxx>

class Image;
class SdDrawDocument;
class SdPage;
namespace sd { class PreviewRenderer; }
namespace sd { class DrawDocShell; }

namespace sd { namespace sidebar {

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
    virtual int GetCostIndex() = 0;

    virtual bool operator== (const PageObjectProvider& rProvider) = 0;

protected:
    ~PageObjectProvider() {}
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
    virtual int GetCostIndex() = 0;

    /** Return whether the page object passed is necessary to create a
        preview.
    */
    virtual bool NeedsPageObject() = 0;

protected:
    ~PreviewProvider() {}
};

/** Provide previews of existing page objects by rendering them.
*/
class PagePreviewProvider : public PreviewProvider
{
public:
    PagePreviewProvider();
    virtual ~PagePreviewProvider() {}
    virtual Image operator () (int nWidth, SdPage* pPage, ::sd::PreviewRenderer& rRenderer) override;
    virtual int GetCostIndex() override;
    virtual bool NeedsPageObject() override;
private:
};

/** Provide master page objects for template documents for which only the
    URL is given.
*/
class TemplatePageObjectProvider : public PageObjectProvider
{
public:
    TemplatePageObjectProvider (const OUString& rsURL);
    virtual ~TemplatePageObjectProvider() {};
    virtual SdPage* operator () (SdDrawDocument* pDocument) override;
    virtual int GetCostIndex() override;
    virtual bool operator== (const PageObjectProvider& rProvider) override;
private:
    OUString msURL;
    SfxObjectShellLock mxDocumentShell;
    ::sd::DrawDocShell* LoadDocument (const OUString& sFileName);
};

/** Provide previews for template documents by loading the thumbnails from
    the documents.
*/
class TemplatePreviewProvider : public PreviewProvider
{
public:
    TemplatePreviewProvider (const OUString& rsURL);
    virtual ~TemplatePreviewProvider() {};
    virtual Image operator() (int nWidth, SdPage* pPage, ::sd::PreviewRenderer& rRenderer) override;
    virtual int GetCostIndex() override;
    virtual bool NeedsPageObject() override;
private:
    OUString msURL;
};

/** Create an empty default master page.
*/
class DefaultPageObjectProvider : public PageObjectProvider
{
public:
    DefaultPageObjectProvider();
    virtual ~DefaultPageObjectProvider() {}
    virtual SdPage* operator () (SdDrawDocument* pDocument) override;
    virtual int GetCostIndex() override;
    virtual bool operator== (const PageObjectProvider& rProvider) override;
};

/** This implementation of the PageObjectProvider simply returns an already
    existing master page object.
*/
class ExistingPageProvider : public PageObjectProvider
{
public:
    ExistingPageProvider (SdPage* pPage);
    virtual ~ExistingPageProvider() {}
    virtual SdPage* operator() (SdDrawDocument* pDocument) override;
    virtual int GetCostIndex() override;
    virtual bool operator== (const PageObjectProvider& rProvider) override;
private:
    SdPage* mpPage;
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
