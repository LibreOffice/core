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
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "dociter.hxx"
#include "document.hxx"
#include "rangeutl.hxx"
#include "tabvwsh.hxx"
#include <sc.hrc>

namespace sc {

SearchResultsDlg::SearchResultsDlg( SfxBindings* _pBindings, Window* pParent, sal_uInt16 /* nId */ ) :
    ModelessDialog(pParent, "SearchResultsDialog", "modules/scalc/ui/searchresults.ui"),
    mpBindings(_pBindings), mpDoc(NULL)
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
    mpList->SetSelectHdl( LINK(this, SearchResultsDlg, ListSelectHdl) );
}

SearchResultsDlg::~SearchResultsDlg()
{
    delete mpList;
}

void SearchResultsDlg::FillResults( ScDocument* pDoc, const ScRangeList &rMatchedRanges )
{
    mpList->Clear();
    mpList->SetUpdateMode(false);
    std::vector<OUString> aTabNames = pDoc->GetAllTableNames();
    SCTAB nTabCount = aTabNames.size();
    for (size_t i = 0, n = rMatchedRanges.size(); i < n; ++i)
    {
        ScCellIterator aIter(pDoc, *rMatchedRanges[i]);
        for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
        {
            ScAddress aPos = aIter.GetPos();
            if (aPos.Tab() >= nTabCount)
                // Out-of-bound sheet index.
                continue;

            OUString aPosStr = aPos.Format(SCA_ABS, NULL, pDoc->GetAddressConvention());
            mpList->InsertEntry(aTabNames[aPos.Tab()] + "\t" + aPosStr + "\t" + pDoc->GetString(aPos));
        }
    }
    mpList->SetUpdateMode(true);

    mpDoc = pDoc;
}

bool SearchResultsDlg::Close()
{
    if (mpBindings)
    {
        // Remove this dialog from the view frame after the dialog gets
        // dismissed, else it would keep popping up endlessly!
        SfxDispatcher* pDispacher = mpBindings ->GetDispatcher();
        SfxBoolItem aItem(SID_SEARCH_RESULTS_DIALOG, false);
        if (pDispacher)
            pDispacher->Execute(
                SID_SEARCH_RESULTS_DIALOG, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L);
    }

    return ModelessDialog::Close();
}

IMPL_LINK_NOARG( SearchResultsDlg, ListSelectHdl )
{
    if (!mpDoc)
        return 0;

    SvTreeListEntry *pEntry = mpList->FirstSelected();
    OUString aTabStr = mpList->GetEntryText(pEntry, 0);
    OUString aPosStr = mpList->GetEntryText(pEntry, 1);

    SCTAB nTab = -1;
    if (!mpDoc->GetTable(aTabStr, nTab))
        // No sheet with specified name.
        return 0;

    ScAddress aPos;
    sal_uInt16 nRes = aPos.Parse(aPosStr, mpDoc, mpDoc->GetAddressConvention());
    if (!(nRes & SCA_VALID))
        // Invalid address string.
        return 0;

    // Jump to the cell.
    ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell();
    pScViewShell->SetTabNo(nTab);
    pScViewShell->SetCursor(aPos.Col(), aPos.Row());
    pScViewShell->AlignToCursor(aPos.Col(), aPos.Row(), SC_FOLLOW_JUMP);

    return 0;
}

SearchResultsDlgWrapper::SearchResultsDlgWrapper(
    Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* /*pInfo*/ ) :
    SfxChildWindow(_pParent, nId)
{
    pWindow = new SearchResultsDlg(pBindings, _pParent, nId);
}

SearchResultsDlgWrapper::~SearchResultsDlgWrapper() {}

SfxChildWinInfo SearchResultsDlgWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    aInfo.bVisible = false;
    return aInfo;
}

SFX_IMPL_CHILDWINDOW_WITHID(SearchResultsDlgWrapper, SID_SEARCH_RESULTS_DIALOG);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
