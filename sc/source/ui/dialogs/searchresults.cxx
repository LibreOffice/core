/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <searchresults.hxx>

#include <svtools/simptabl.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <dociter.hxx>
#include <document.hxx>
#include <tabvwsh.hxx>
#include <strings.hrc>
#include <sc.hrc>
#include <scresid.hxx>

namespace sc {

SearchResultsDlg::SearchResultsDlg( SfxBindings* _pBindings, vcl::Window* pParent ) :
    ModelessDialog(pParent, "SearchResultsDialog", "modules/scalc/ui/searchresults.ui"),
    aSkipped( ScResId( SCSTR_SKIPPED ) ),
    mpBindings(_pBindings), mpDoc(nullptr)
{
    get(mpSearchResults, "lbSearchResults");

    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("results");
    Size aControlSize(150, 120);
    aControlSize = pContainer->LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    mpList = VclPtr<SvSimpleTable>::Create(*pContainer);
    long nTabs[] = {0, 40, 60};
    mpList->SetTabs(SAL_N_ELEMENTS(nTabs), nTabs);
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
    mpSearchResults.disposeAndClear();
    ModelessDialog::dispose();
}

namespace
{
    class ListWrapper {
        OUStringBuffer maName;
        VclPtr<SvSimpleTable> mpList;
    public:
        size_t mnCount = 0;
        static const size_t mnMaximum = 1000;
        ListWrapper(const VclPtr<SvSimpleTable> &pList) :
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
    };
}

void SearchResultsDlg::FillResults( ScDocument* pDoc, const ScRangeList &rMatchedRanges, bool bCellNotes )
{
    ListWrapper aList(mpList);
    std::vector<OUString> aTabNames = pDoc->GetAllTableNames();
    SCTAB nTabCount = aTabNames.size();

    // tdf#92160 - too many results blow the widget's mind
    size_t nMatchMax = rMatchedRanges.size();
    if (nMatchMax > ListWrapper::mnMaximum)
        nMatchMax = ListWrapper::mnMaximum;

    if (bCellNotes)
    {
        for (size_t i = 0, n = nMatchMax; i < n; ++i)
        {
            /* TODO: a CellNotes iterator would come handy and might speed
             * things up a little, though we only loop through the
             * search/replace result positions here. */
            ScRange const & rRange( rMatchedRanges[i] );
            // Bear in mind that mostly the range is one address position
            // or a column or a row joined.
            ScAddress aPos( rRange.aStart );
            for ( ; aPos.Tab() <= rRange.aEnd.Tab(); aPos.IncTab())
            {
                if (aPos.Tab() >= nTabCount)
                    break;  // can this even happen? we just searched on existing sheets ...
                for (aPos.SetCol( rRange.aStart.Col()); aPos.Col() <= rRange.aEnd.Col(); aPos.IncCol())
                {
                    for (aPos.SetRow( rRange.aStart.Row()); aPos.Row() <= rRange.aEnd.Row(); aPos.IncRow())
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
            ScCellIterator aIter(pDoc, rMatchedRanges[i]);
            for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
            {
                const ScAddress& aPos = aIter.GetPos();
                if (aPos.Tab() >= nTabCount)
                    // Out-of-bound sheet index.
                    continue;

                aList.Insert(aTabNames[aPos.Tab()], aPos,
                             pDoc->GetAddressConvention(),
                             pDoc->GetString(aPos));
            }
        }
    }

    OUString aTotal(ScResId(SCSTR_TOTAL, aList.mnCount));
    OUString aSearchResults = aTotal.replaceFirst("%1", OUString::number(aList.mnCount));
    if (aList.mnCount > ListWrapper::mnMaximum)
        aSearchResults += " " + ScGlobal::ReplaceOrAppend( aSkipped, "%1", OUString::number( ListWrapper::mnMaximum ) );
    mpSearchResults->SetText(aSearchResults);

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
