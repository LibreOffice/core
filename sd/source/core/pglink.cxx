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

#include <sfx2/linkmgr.hxx>

#include "pglink.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"

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
    const OUString&, const css::uno::Any& )
{
    SdDrawDocument* pDoc = static_cast<SdDrawDocument*>( pPage->GetModel() );
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

        SdDrawDocument* pBookmarkDoc = pDoc->OpenBookmarkDoc(aFileName);

        if (pBookmarkDoc)
        {
            /******************************************************************
            * the linked page is replaced in the model
            ******************************************************************/
            if (aBookmarkName.isEmpty())
            {
                // no page name specified: we assume it is the first page
                aBookmarkName = pBookmarkDoc->GetSdPage(0, PageKind::Standard)->GetName();
                pPage->SetBookmarkName(aBookmarkName);
            }

            std::vector<OUString> aBookmarkList;
            aBookmarkList.push_back(aBookmarkName);
            sal_uInt16 nInsertPos = pPage->GetPageNum();
            bool bNoDialogs = false;
            bool bCopy = false;

            if (SdDrawDocument::s_pDocLockedInsertingLinks)
            {
                // resolving links while loading pDoc
                bNoDialogs = true;
                bCopy = true;
            }

            pDoc->InsertBookmarkAsPage(aBookmarkList, nullptr, true/*bLink*/, true/*bReplace*/,
                                       nInsertPos, bNoDialogs, nullptr, bCopy, true, true);

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
