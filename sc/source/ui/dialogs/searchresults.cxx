/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "searchresults.hxx"

#include <svtools/simptabl.hxx>
#include <svtools/treelistentry.hxx>
#include "tabvwsh.hxx"

SearchResults::SearchResults() :
    ModelessDialog(NULL, "SearchResultsDialog", "modules/scalc/ui/searchresults.ui")
{
    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("results");
    Size aControlSize(200, 100);
    aControlSize = pContainer->LogicToPixel(aControlSize, MAP_APPFONT);
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    mpList = new SvSimpleTable(*pContainer);
    long nTabs[] = {3, 60, 40, 100};
    mpList->SetTabs(&nTabs[0]);
    mpList->InsertHeaderEntry("Sheet\tCell\tContent");
    mpList->SetSelectHdl( LINK(this, SearchResults, ListSelectHdl) );
}

SearchResults::~SearchResults()
{
    delete mpList;
}

IMPL_LINK_NOARG( SearchResults, ListSelectHdl )
{
    SvTreeListEntry *pEntry = mpList->FirstSelected();
    ScAddress *pAddress = static_cast<ScAddress*>(pEntry->GetUserData());
    ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell();
    pScViewShell->SetTabNo(pAddress->Tab());
    pScViewShell->SetCursor(pAddress->Col(), pAddress->Row());
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
