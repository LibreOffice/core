/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <o3tl/safeint.hxx>
#include <searchresults.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/srchdlg.hxx>
#include <dociter.hxx>
#include <document.hxx>
#include <tabvwsh.hxx>
#include <strings.hrc>
#include <sc.hrc>
#include <scresid.hxx>

namespace sc {

SearchResultsDlg::SearchResultsDlg(SfxBindings* _pBindings, weld::Window* pParent)
    : SfxDialogController(pParent, u"modules/scalc/ui/searchresults.ui"_ustr, u"SearchResultsDialog"_ustr)
    , aSkipped(ScResId(SCSTR_SKIPPED))
    , mpBindings(_pBindings)
    , mpDoc(nullptr)
    , mbSorted(false)
    , mxList(m_xBuilder->weld_tree_view(u"results"_ustr))
    , mxSearchResults(m_xBuilder->weld_label(u"lbSearchResults"_ustr))
    , mxShowDialog(m_xBuilder->weld_check_button(u"cbShow"_ustr))
{
    mxList->set_size_request(mxList->get_approximate_digit_width() * 50, mxList->get_height_rows(15));
    mxShowDialog->connect_toggled(LINK(this, SearchResultsDlg, OnShowToggled));
    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(mxList->get_approximate_digit_width() * 10),
        o3tl::narrowing<int>(mxList->get_approximate_digit_width() * 10)
    };
    mxList->set_column_fixed_widths(aWidths);
    mxList->connect_changed(LINK(this, SearchResultsDlg, ListSelectHdl));
    mxList->connect_column_clicked(LINK(this, SearchResultsDlg, HeaderBarClick));
}

SearchResultsDlg::~SearchResultsDlg()
{
    // tdf#133807 if the search dialog is shown then re-present that dialog
    // when this results dialog is dismissed
    SfxViewFrame* pViewFrame = mpBindings->GetDispatcher()->GetFrame();
    if (!pViewFrame)
        return;
    SfxChildWindow* pChildWindow = pViewFrame->GetChildWindow(
            SvxSearchDialogWrapper::GetChildWindowId());
    if (!pChildWindow)
        return;
    SvxSearchDialog* pSearchDlg = static_cast<SvxSearchDialog*>(pChildWindow->GetController().get());
    if (!pSearchDlg)
        return;
    pSearchDlg->Present();
}

namespace
{
    class ListWrapper {
        weld::TreeView& mrList;
    public:
        size_t mnCount = 0;
        static const size_t mnMaximum = 1000;
        ListWrapper(weld::TreeView& rList)
            : mrList(rList)
        {
            mrList.clear();
            mrList.freeze();
        }
        ~ListWrapper()
        {
            mrList.thaw();
        }
        void Insert(const OUString &rTabName,
                    const ScAddress &rPos,
                    formula::FormulaGrammar::AddressConvention eConvention,
                    const OUString &rText)
        {
            if (mnCount++ < mnMaximum)
            {
                mrList.append_text(rTabName);
                int nPos = mrList.n_children() - 1;
                mrList.set_text(nPos, rPos.Format(ScRefFlags::ADDR_ABS,
                                      nullptr, eConvention), 1);
                mrList.set_text(nPos, rText, 2);
            }
        }
    };
}

void SearchResultsDlg::FillResults( ScDocument& rDoc, const ScRangeList &rMatchedRanges, bool bCellNotes,
        bool bEmptyCells, bool bMatchedRangesWereClamped )
{
    ListWrapper aList(*mxList);
    std::vector<OUString> aTabNames = rDoc.GetAllTableNames();
    SCTAB nTabCount = aTabNames.size();

    // tdf#92160 - too many results blow the widget's mind
    size_t nMatchMax = rMatchedRanges.size();
    if (nMatchMax > ListWrapper::mnMaximum)
        nMatchMax = ListWrapper::mnMaximum;

    if (bCellNotes || bEmptyCells)
    {
        for (size_t i = 0, n = nMatchMax; i < n; ++i)
        {
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
                        if (bCellNotes)
                        {
                            const ScPostIt* pNote = rDoc.GetNote( aPos);
                            if (pNote)
                                aList.Insert(aTabNames[aPos.Tab()], aPos,
                                        rDoc.GetAddressConvention(),
                                        pNote->GetText());
                        }
                        else  // bEmptyCells
                        {
                            aList.Insert(aTabNames[aPos.Tab()], aPos,
                                    rDoc.GetAddressConvention(),
                                    rDoc.GetString(aPos));
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (size_t i = 0, n = nMatchMax; i < n; ++i)
        {
            ScCellIterator aIter(rDoc, rMatchedRanges[i]);
            for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
            {
                const ScAddress& aPos = aIter.GetPos();
                if (aPos.Tab() >= nTabCount)
                    // Out-of-bound sheet index.
                    continue;

                aList.Insert(aTabNames[aPos.Tab()], aPos,
                             rDoc.GetAddressConvention(),
                             rDoc.GetString(aPos));
            }
        }
    }

    OUString aSearchResultsMsg;
    if (bMatchedRangesWereClamped)
    {
        aSearchResultsMsg = ScResId(SCSTR_RESULTS_CLAMPED);
        aSearchResultsMsg = aSearchResultsMsg.replaceFirst("%1", OUString::number(1000));
    }
    else
    {
        OUString aTotal(ScResId(SCSTR_TOTAL, aList.mnCount));
        aSearchResultsMsg = aTotal.replaceFirst("%1", OUString::number(aList.mnCount));
        if (aList.mnCount > ListWrapper::mnMaximum)
            aSearchResultsMsg += " " + ScGlobal::ReplaceOrAppend( aSkipped, u"%1", OUString::number( ListWrapper::mnMaximum ) );
    }
    mxSearchResults->set_label(aSearchResultsMsg);

    mpDoc = &rDoc;
}

void SearchResultsDlg::Close()
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
                SfxCallMode::SYNCHRON | SfxCallMode::RECORD, { &aItem });
        }
    }

    SfxDialogController::Close();
}

IMPL_LINK(SearchResultsDlg, HeaderBarClick, int, nColumn, void)
{
    if (!mbSorted)
    {
        mxList->make_sorted();
        mbSorted = true;
    }

    bool bSortAtoZ = mxList->get_sort_order();

    //set new arrow positions in headerbar
    if (nColumn == mxList->get_sort_column())
    {
        bSortAtoZ = !bSortAtoZ;
        mxList->set_sort_order(bSortAtoZ);
    }
    else
    {
        int nOldSortColumn = mxList->get_sort_column();
        if (nOldSortColumn != -1)
            mxList->set_sort_indicator(TRISTATE_INDET, nOldSortColumn);
        mxList->set_sort_column(nColumn);
    }

    if (nColumn != -1)
    {
        //sort lists
        mxList->set_sort_indicator(bSortAtoZ ? TRISTATE_TRUE : TRISTATE_FALSE, nColumn);
    }
}

IMPL_LINK_NOARG( SearchResultsDlg, ListSelectHdl, weld::TreeView&, void )
{
    if (!mpDoc)
        return;

    int nEntry = mxList->get_selected_index();
    OUString aTabStr = mxList->get_text(nEntry, 0);
    OUString aPosStr = mxList->get_text(nEntry, 1);

    SCTAB nTab = -1;
    if (!mpDoc->GetTable(aTabStr, nTab))
        // No sheet with specified name.
        return;

    ScAddress aPos;
    ScRefFlags nRes = aPos.Parse(aPosStr, *mpDoc, mpDoc->GetAddressConvention());
    if (!(nRes & ScRefFlags::VALID))
        // Invalid address string.
        return;

    // Jump to the cell.
    if (ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell())
    {
        pScViewShell->SetTabNo(nTab);
        pScViewShell->SetCursor(aPos.Col(), aPos.Row());
        pScViewShell->AlignToCursor(aPos.Col(), aPos.Row(), SC_FOLLOW_JUMP);
    }
}

IMPL_STATIC_LINK( SearchResultsDlg, OnShowToggled, weld::Toggleable&, rButton, void )
{
    if (ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell())
    {
        ScViewOptions aViewOpt( pScViewShell->GetViewData().GetOptions() );
        aViewOpt.SetOption( VOPT_SUMMARY, rButton.get_active() );
        pScViewShell->GetViewData().SetOptions( aViewOpt );
    }
}

SearchResultsDlgWrapper::SearchResultsDlgWrapper(
    vcl::Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* /*pInfo*/)
    : SfxChildWindow(_pParent, nId)
    , m_xDialog(std::make_shared<SearchResultsDlg>(pBindings, _pParent->GetFrameWeld()))
{
    SetController(m_xDialog);
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
