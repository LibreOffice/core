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
#include <strings.hrc>
#include "sc.hrc"
#include "scresid.hxx"

namespace sc {

SearchResultsDlg::SearchResultsDlg( SfxBindings* _pBindings, vcl::Window* pParent ) :
    ModelessDialog(pParent, "SearchResultsDialog", "modules/scalc/ui/searchresults.ui"),
    mpBindings(_pBindings), mpDoc(nullptr)
{
    get(mpLabel, "skipped");

    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("results");
    Size aControlSize(150, 120);
    aControlSize = pContainer->LogicToPixel(aControlSize, MapUnit::MapAppFont);
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    mpList = VclPtr<SvSimpleTable>::Create(*pContainer);
    long nTabs[] = {3, 0, 40, 60};
    mpList->SetTabs(&nTabs[0]);
    mpList->InsertHeaderEntry(ScResId(STR_SHEET) + "\t" + ScResId(STR_CELL) + "\t" + ScResId(STR_CONTENT));
    mpList->SetSelectHdl( LINK(this, SearchResultsDlg, ListSelectHdl) );
}

SearchResultsDlg::~SearchResultsDlg()
{
    disposeOnce();
}

void SearchResultsDlg::dispose()
{
    mpList.disposeAndClear();
    mpLabel.disposeAndClear();
    ModelessDialog::dispose();
}

namespace
{
    class ListWrapper {
        size_t mnCount;
        static const size_t mnMaximum = 1000;
        OUStringBuffer maName;
        VclPtr<FixedText> mpLabel;
        VclPtr<SvSimpleTable> mpList;
    public:
        ListWrapper(const VclPtr<SvSimpleTable> &pList,
                    const VclPtr<FixedText> &pLabel) :
            mnCount(0),
            mpLabel(pLabel),
            mpList(pList)
        {
            mpList->Clear();
            mpList->SetUpdateMode(false);
        }
        void Insert(const OUString &aTabName,
                    const ScAddress &rPos,
                    formula::FormulaGrammar::AddressConvention eConvention,
                    const OUString &aText)
        {
            if (mnCount++ < mnMaximum)
            {
                maName.append(aTabName);
                maName.append("\t");
                maName.append(rPos.Format(ScRefFlags::ADDR_ABS,
                                          nullptr, eConvention));
                maName.append("\t");
                maName.append(aText);
                mpList->InsertEntry(maName.makeStringAndClear());
            }
        }
        void Update()
        {
            if (mnCount > mnMaximum)
            {
                if (mpLabel)
                {
                    size_t nSkipped = mnCount - mnMaximum;
                    OUString aSkipped(mpLabel->GetText());
                    mpList->InsertEntry(
                        aSkipped.replaceFirst("$1", OUString::number(nSkipped)));
                }
            }
            mpList->SetUpdateMode(true);
        }
    };
}

void SearchResultsDlg::FillResults( ScDocument* pDoc, const ScRangeList &rMatchedRanges, bool bCellNotes )
{
    ListWrapper aList(mpList, mpLabel);
    std::vector<OUString> aTabNames = pDoc->GetAllTableNames();
    SCTAB nTabCount = aTabNames.size();

    // tdf#92160 - too many results blow the widget's mind
    size_t nMatchMax = rMatchedRanges.size();
    if (nMatchMax > 1000)
        nMatchMax = 1000;

    if (bCellNotes)
    {
        for (size_t i = 0, n = nMatchMax; i < n; ++i)
        {
            /* TODO: a CellNotes iterator would come handy and might speed
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
                            aList.Insert(aTabNames[aPos.Tab()], aPos,
                                         pDoc->GetAddressConvention(),
                                         pNote->GetText());
                    }
                }
            }
        }
    }
    else
    {
        for (size_t i = 0, n = nMatchMax; i < n; ++i)
        {
            ScCellIterator aIter(pDoc, *rMatchedRanges[i]);
            for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
            {
                ScAddress aPos = aIter.GetPos();
                if (aPos.Tab() >= nTabCount)
                    // Out-of-bound sheet index.
                    continue;

                aList.Insert(aTabNames[aPos.Tab()], aPos,
                             pDoc->GetAddressConvention(),
                             pDoc->GetString(aPos));
            }
        }
    }
    aList.Update();
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

IMPL_LINK_NOARG( SearchResultsDlg, ListSelectHdl, SvTreeListBox*, void )
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
    SetWindow( VclPtr<SearchResultsDlg>::Create(pBindings, _pParent) );
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
