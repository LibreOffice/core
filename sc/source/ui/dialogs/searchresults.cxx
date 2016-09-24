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
#include "scresid.hxx"

namespace sc {

SearchResultsDlg::SearchResultsDlg( SfxBindings* _pBindings, vcl::Window* pParent, sal_uInt16 /* nId */ ) :
    ModelessDialog(pParent, "SearchResultsDialog", "modules/scalc/ui/searchresults.ui"),
    mpBindings(_pBindings), mpDoc(nullptr)
{
    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("results");
    Size aControlSize(150, 120);
    aControlSize = pContainer->LogicToPixel(aControlSize, MAP_APPFONT);
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    mpList = VclPtr<SvSimpleTable>::Create(*pContainer);
    long nTabs[] = {3, 0, 40, 60};
    mpList->SetTabs(&nTabs[0]);
    mpList->InsertHeaderEntry(SC_RESSTR(STR_SHEET) + "\t" + SC_RESSTR(STR_CELL) + "\t" + SC_RESSTR(STR_CONTENT));
    mpList->SetSelectHdl( LINK(this, SearchResultsDlg, ListSelectHdl) );
}

SearchResultsDlg::~SearchResultsDlg()
{
    disposeOnce();
}

void SearchResultsDlg::dispose()
{
    mpList.disposeAndClear();
    ModelessDialog::dispose();
}

void SearchResultsDlg::FillResults( ScDocument* pDoc, const ScRangeList &rMatchedRanges, bool bCellNotes )
{
    mpList->Clear();
    mpList->SetUpdateMode(false);
    std::vector<OUString> aTabNames = pDoc->GetAllTableNames();
    SCTAB nTabCount = aTabNames.size();
    if (bCellNotes)
    {
        for (size_t i = 0, n = rMatchedRanges.size(); i < n; ++i)
        {
            /* TODO: a CellNotes iterator would come handy and migt speed
             * things up a little, though we only loop through the
             * search/replace result positions here. */
            ScRange aRange( *rMatchedRanges[i] );
            // Bear in mind that mostly the range is one address position
            // or a column or a row joined.
            ScAddress aPos( aRange.aStart );
            for ( ; aPos.Tab() <= aRange.aEnd.Tab(); aPos.IncTab())
            {
                if (aPos.Tab() >= nTabCount)
                    break;  // can this even happen? we just searched on existing sheets ...
                for (aPos.SetCol( aRange.aStart.Col()); aPos.Col() <= aRange.aEnd.Col(); aPos.IncCol())
                {
                    for (aPos.SetRow( aRange.aStart.Row()); aPos.Row() <= aRange.aEnd.Row(); aPos.IncRow())
                    {
                        const ScPostIt* pNote = pDoc->GetNote( aPos);
                        if (pNote)
                        {
                            OUString aPosStr = aPos.Format(ScRefFlags::ADDR_ABS, nullptr, pDoc->GetAddressConvention());
                            mpList->InsertEntry(aTabNames[aPos.Tab()] + "\t" + aPosStr + "\t" + pNote->GetText());
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (size_t i = 0, n = rMatchedRanges.size(); i < n; ++i)
        {
            ScCellIterator aIter(pDoc, *rMatchedRanges[i]);
            for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
            {
                ScAddress aPos = aIter.GetPos();
                if (aPos.Tab() >= nTabCount)
                    // Out-of-bound sheet index.
                    continue;

                OUString aPosStr = aPos.Format(ScRefFlags::ADDR_ABS, nullptr, pDoc->GetAddressConvention());
                mpList->InsertEntry(aTabNames[aPos.Tab()] + "\t" + aPosStr + "\t" + pDoc->GetString(aPos));
            }
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
        {
            pDispacher->ExecuteList(SID_SEARCH_RESULTS_DIALOG,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });
        }
    }

    return ModelessDialog::Close();
}

IMPL_LINK_NOARG_TYPED( SearchResultsDlg, ListSelectHdl, SvTreeListBox*, void )
{
    if (!mpDoc)
        return;

    SvTreeListEntry *pEntry = mpList->FirstSelected();
    OUString aTabStr = SvTabListBox::GetEntryText(pEntry, 0);
    OUString aPosStr = SvTabListBox::GetEntryText(pEntry, 1);

    SCTAB nTab = -1;
    if (!mpDoc->GetTable(aTabStr, nTab))
        // No sheet with specified name.
        return;

    ScAddress aPos;
    ScRefFlags nRes = aPos.Parse(aPosStr, mpDoc, mpDoc->GetAddressConvention());
    if (!(nRes & ScRefFlags::VALID))
        // Invalid address string.
        return;

    // Jump to the cell.
    ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell();
    pScViewShell->SetTabNo(nTab);
    pScViewShell->SetCursor(aPos.Col(), aPos.Row());
    pScViewShell->AlignToCursor(aPos.Col(), aPos.Row(), SC_FOLLOW_JUMP);
}

SearchResultsDlgWrapper::SearchResultsDlgWrapper(
    vcl::Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* /*pInfo*/ ) :
    SfxChildWindow(_pParent, nId)
{
    SetWindow( VclPtr<SearchResultsDlg>::Create(pBindings, _pParent, nId) );
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
