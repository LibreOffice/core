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

SdPageLink::SdPageLink(SdPage* pPg, const String& rFileName,
                       const String& rBookmarkName) :
    ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, FORMAT_FILE),
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
    const OUString&, const ::com::sun::star::uno::Any& )
{
    SdDrawDocument* pDoc = (SdDrawDocument*) pPage->GetModel();
    sfx2::LinkManager* pLinkManager = pDoc!=NULL ? pDoc->GetLinkManager() : NULL;

    if (pLinkManager)
    {
        /**********************************************************************
        * Only standard pages are allowed to be linked
        * The corresponding note pages are updated automaticly
        **********************************************************************/
        OUString aFileName;
        OUString aBookmarkName;
        OUString aFilterName;
        pLinkManager->GetDisplayNames( this,0, &aFileName, &aBookmarkName,
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
                aBookmarkName = pBookmarkDoc->GetSdPage(0, PK_STANDARD)->GetName();
                pPage->SetBookmarkName(aBookmarkName);
            }

            std::vector<OUString> aBookmarkList;
            aBookmarkList.push_back(aBookmarkName);
            sal_uInt16 nInsertPos = pPage->GetPageNum();
            sal_Bool bLink = sal_True;
            sal_Bool bReplace = sal_True;
            sal_Bool bNoDialogs = sal_False;
            sal_Bool bCopy = sal_False;

            if( pDoc->pDocLockedInsertingLinks )
            {
                // resolving links while loading pDoc
                bNoDialogs = sal_True;
                bCopy = sal_True;
            }

            pDoc->InsertBookmarkAsPage(aBookmarkList, NULL, bLink, bReplace,
                                       nInsertPos, bNoDialogs, NULL, bCopy, sal_True, sal_True);

            if( !pDoc->pDocLockedInsertingLinks )
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
    pPage->SetFileName(String());
    pPage->SetBookmarkName(String());

    SvBaseLink::Closed();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
