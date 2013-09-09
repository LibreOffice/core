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
#include "dociter.hxx"
#include "document.hxx"
#include "rangeutl.hxx"
#include "tabvwsh.hxx"

SearchResults::SearchResults(ScDocument *pDoc) :
    ModelessDialog(NULL, "SearchResultsDialog", "modules/scalc/ui/searchresults.ui")
    , mpDoc(pDoc)
{
    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("results");
    Size aControlSize(150, 120);
    aControlSize = pContainer->LogicToPixel(aControlSize, MAP_APPFONT);
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    mpList = new SvSimpleTable(*pContainer);
    long nTabs[] = {3, 0, 40, 60};
    mpList->SetTabs(&nTabs[0]);
    mpList->InsertHeaderEntry("Sheet\tCell\tContent");
    mpList->SetSelectHdl( LINK(this, SearchResults, ListSelectHdl) );
}

SearchResults::~SearchResults()
{
    delete mpList;
}

void SearchResults::Show(const ScRangeList &rMatchedRanges)
{
    mpList->Clear();
    for (size_t i = 0, n = rMatchedRanges.size(); i < n; ++i)
    {
        ScCellIterator aIter(mpDoc, *rMatchedRanges[i]);
        for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
        {
            ScAddress aAddress = aIter.GetPos();
            OUString sAddress;
            ScRangeStringConverter::GetStringFromAddress(sAddress, aAddress,
                    mpDoc, formula::FormulaGrammar::CONV_OOO);
            mpList->InsertEntry(sAddress.replace('.', '\t') + "\t" + mpDoc->GetString(aAddress));
        }
    }
    ModelessDialog::Show();
}

IMPL_LINK_NOARG( SearchResults, ListSelectHdl )
{
    SvTreeListEntry *pEntry = mpList->FirstSelected();
    ScAddress aAddress;
    sal_Int32 nOffset = 0;
    OUString sAddress = mpList->GetEntryText(pEntry).replaceFirst("\t", ".");
    ScRangeStringConverter::GetAddressFromString(aAddress, sAddress,
            mpDoc, formula::FormulaGrammar::CONV_OOO, nOffset, '\t');
    ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell();
    pScViewShell->SetTabNo(aAddress.Tab());
    pScViewShell->SetCursor(aAddress.Col(), aAddress.Row());
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
