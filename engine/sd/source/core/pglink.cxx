/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sfx2/linkmgr.hxx>

#include <SlideLink.hxx>
#include <pglink.hxx>
#include <sdpage.hxx>
#include <drawdoc.hxx>

/*************************************************************************
|*
|*      Ctor
|*
\************************************************************************/

SdPageLink::SdPageLink(SdPage* pPg, const OUString& rFileName,
                       const OUString& rBookmarkName) :
    ::sfx2::SvBaseLink( ::SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SIMPLE_FILE),
    pPage(pPg)
{
    pPage->SetFileName(rFileName);
    pPage->SetBookmarkName(rBookmarkName);
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SdPageLink::~SdPageLink()
{
}

/*************************************************************************
|*
|* Date have changed
|*
\************************************************************************/

::sfx2::SvBaseLink::UpdateResult SdPageLink::DataChanged(
    const OUString&, const cpo::uno::Any& )
{
    SdDrawDocument* pDoc = static_cast<SdDrawDocument*>( &pPage->getSdrModelFromSdrPage() );
    sfx2::LinkManager* pLinkManager = pDoc!=nullptr ? pDoc->GetLinkManager() : nullptr;

    if (pLinkManager)
    {
        /**********************************************************************
        * Only standard pages are allowed to be linked
        * The corresponding note pages are updated automatically
        **********************************************************************/
        OUString aFileName;
        OUString aBookmarkName;
        OUString aFilterName;
        sfx2::LinkManager::GetDisplayNames( this,nullptr, &aFileName, &aBookmarkName,
                                      &aFilterName);
        pPage->SetFileName(aFileName);
        pPage->SetBookmarkName(aBookmarkName);

        // A page is read from the file that holds the slides of its source document. A source that
        // nothing reads leaves the page the content it holds.
        const OUString aSourceFile = sd::SlideLink::GetSourceFile(*pDoc, aFileName);
        if (aSourceFile.isEmpty())
            return SUCCESS;

        const bool bStaged = aSourceFile != aFileName;
        SdDrawDocument* pBookmarkDoc = pDoc->OpenBookmarkDoc(aSourceFile, /*bNoDialogs=*/bStaged);

        if (pBookmarkDoc)
        {
            /******************************************************************
            * the linked page is replaced in the model
            ******************************************************************/
            const bool bNamedSource = !sd::SlideLink::GetSourceName(aFileName).isEmpty();
            if (bNamedSource)
            {
                const OUString aSourceSlide
                    = sd::SlideLink::GetSourceSlideName(*pBookmarkDoc, *pPage);
                if (aSourceSlide.isEmpty())
                {
                    if (!SdDrawDocument::s_pDocLockedInsertingLinks)
                        pDoc->CloseBookmarkDoc();
                    return SUCCESS;
                }

                aBookmarkName = aSourceSlide;
            }
            else if (aBookmarkName.isEmpty())
            {
                // no page name specified: we assume it is the first page
                aBookmarkName = pBookmarkDoc->GetSdPage(0, PageKind::Standard)->GetName();
                pPage->SetBookmarkName(aBookmarkName);
            }

            // The page read for this one records the slide it came from, as this page does: the
            // page read from a file records nothing of its own. The resolution below puts the page
            // read in place of this one, so what it records is taken while this page is still here.
            const OUString aKeptName = sd::SlideLink::GetRefreshedSlideName(*pPage, aBookmarkName);
            const OUString aRecordedGuid = pPage->GetSourcePageGuid();

            std::vector<OUString> aBookmarkList { aBookmarkName };
            sal_uInt16 nInsertPos = pPage->GetPageNum();
            // A file staged for one refresh was chosen for it, so reading it asks nothing.
            bool bNoDialogs = bStaged;
            bool bCopy = false;

            if (SdDrawDocument::s_pDocLockedInsertingLinks)
            {
                // resolving links while loading pDoc
                bNoDialogs = true;
                bCopy = true;
            }

            // The page keeps the source it records, so it stays linked to that source whichever
            // file its slides were read from.
            pDoc->ResolvePageLinks(aBookmarkList, nInsertPos, bNoDialogs, bCopy, aFileName);

            // The standard page of slide S sits at page position 2*S+1, and the slide read for
            // this one took its position.
            SdPage* pReadPage = pDoc->GetSdPage((nInsertPos - 1) >> 1, PageKind::Standard);
            if (bNamedSource && pReadPage)
            {
                pReadPage->SetBookmarkName(aKeptName);
                pReadPage->SetSourcePageGuid(aRecordedGuid);
            }

            if (!SdDrawDocument::s_pDocLockedInsertingLinks)
                pDoc->CloseBookmarkDoc();
        }
    }
    return SUCCESS;
}

/*************************************************************************
|*
|* Connect or disconnect link
|*
\************************************************************************/

void SdPageLink::Closed()
{
    // the connection is closed
    pPage->SetFileName(OUString());
    pPage->SetBookmarkName(OUString());

    SvBaseLink::Closed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
