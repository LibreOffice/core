/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <QuickFindPanel.hxx>
#include <svtools/colorcfg.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/scopeguard.hxx>
#include <svl/srchitem.hxx>
#include <view.hxx>
#include <doc.hxx>
#include <swmodule.hxx>
#include <pam.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <edtwin.hxx>
#include <fmtanchr.hxx>
#include <cntfrm.hxx>
#include <strings.hrc>
#include <vcl/event.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <swwait.hxx>
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>
#include <docufld.hxx>
#include <postithelper.hxx>
#include <viewopt.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/ESelection.hxx>
#include <editeng/outliner.hxx>
#include <unotools/textsearch.hxx>

#include <optional>

#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>

#include <svx/srchdlg.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/processfactory.hxx>

using namespace css;
using namespace ::cpo;

const int CharactersBeforeAndAfter = 40;
const OUString KitPageEntryPrefix = u"-$#~"_ustr;
const OUString KitPageEntrySuffix = u"~#$-"_ustr;

namespace
{
void getAnchorPos(SwPosition& rPos)
{
    // get the top most anchor position of the position
    if (SwFrameFormat* pFlyFormat = rPos.GetNode().GetFlyFormat())
    {
        SwNode* pAnchorNode;
        SwFrameFormat* pTmp = pFlyFormat;
        while (pTmp && (pAnchorNode = pTmp->GetAnchor().GetAnchorNode())
               && (pTmp = pAnchorNode->GetFlyFormat()))
        {
            pFlyFormat = pTmp;
        }
        if (const SwPosition* pPos = pFlyFormat->GetAnchor().GetContentAnchor())
            rPos = *pPos;
    }
}

// Returns the text of one entry in the search finds list: the match in square brackets, with up to
// CharactersBeforeAndAfter characters of the surrounding text on each side, cut at word boundaries.
OUString MakeSearchFindEntryText(const OUString& sNodeText, sal_Int32 nMarkIndex,
                                 sal_Int32 nPointIndex)
{
    // determine the text node text subview start index for the list entry text
    auto nStartIndex = nMarkIndex - CharactersBeforeAndAfter;
    if (nStartIndex < 0)
    {
        nStartIndex = 0;
    }
    else
    {
        // tdf#160539 format search finds results also to word boundaries
        sal_Unicode ch;
        do
        {
            ch = sNodeText[nStartIndex];
        } while (++nStartIndex < nMarkIndex && ch != ' ' && ch != '\t');
        if (nStartIndex < nMarkIndex)
        {
            // move past neighboring space and tab characters
            ch = sNodeText[nStartIndex];
            while (nStartIndex < nMarkIndex && (ch == ' ' || ch == '\t'))
                ch = sNodeText[++nStartIndex];
        }
        if (nStartIndex == nMarkIndex) // no white space found
            nStartIndex = nMarkIndex - CharactersBeforeAndAfter;
    }

    // determine the text node text subview end index for the list entry text
    auto nEndIndex = nPointIndex + CharactersBeforeAndAfter;
    if (nEndIndex >= sNodeText.getLength())
    {
        nEndIndex = sNodeText.getLength() - 1;
    }
    else
    {
        // tdf#160539 format search finds results also to word boundaries
        sal_Unicode ch;
        do
        {
            ch = sNodeText[nEndIndex];
        } while (--nEndIndex > nPointIndex && ch != ' ' && ch != '\t');
        if (nEndIndex > nPointIndex)
        {
            // move past neighboring space and tab characters
            ch = sNodeText[nEndIndex];
            while (nEndIndex > nPointIndex && (ch == ' ' || ch == '\t'))
                ch = sNodeText[--nEndIndex];
        }
        if (nEndIndex == nPointIndex) // no white space found
        {
            nEndIndex = nPointIndex + CharactersBeforeAndAfter;
            if (nEndIndex >= sNodeText.getLength())
                nEndIndex = sNodeText.getLength() - 1;
        }
    }

    auto nCount = nMarkIndex - nStartIndex;
    OUString sTextBeforeFind = OUString::Concat(sNodeText.subView(nStartIndex, nCount));
    auto nCount1 = nPointIndex - nMarkIndex;
    OUString sFind = OUString::Concat(sNodeText.subView(nMarkIndex, nCount1));
    auto nCount2 = nEndIndex - nPointIndex + 1;
    OUString sTextAfterFind = OUString::Concat(sNodeText.subView(nPointIndex, nCount2));
    return sTextBeforeFind + "[" + sFind + "]" + sTextAfterFind;
}

// The comment search works on the text of EditEngine::GetText, where a field such as a link
// shows its text. The selection of the edit engine counts a field as one character instead.

// Returns the offset of a position in the text of rEditEngine, counting one line feed between
// paragraphs, as the text of EditEngine::GetText has it.
sal_Int32 GetCommentTextOffset(const EditEngine& rEditEngine, sal_Int32 nPara, sal_Int32 nIndex)
{
    sal_Int32 nOffset = nIndex;
    for (sal_Int32 i = 0; i < nPara; ++i)
        nOffset += rEditEngine.GetText(i).getLength() + 1;
    return nOffset;
}

// Returns the paragraph and the index in it for an offset of GetCommentTextOffset.
EPaM GetCommentTextPosition(const EditEngine& rEditEngine, sal_Int32 nOffset)
{
    sal_Int32 nPara = 0;
    while (nPara + 1 < rEditEngine.GetParagraphCount()
           && nOffset > rEditEngine.GetText(nPara).getLength())
    {
        nOffset -= rEditEngine.GetText(nPara).getLength() + 1;
        ++nPara;
    }
    return EPaM(nPara, nOffset);
}

// Returns the index in paragraph nPara as the selection of rEditEngine counts it, for an index
// into the text of the paragraph where each field shows its text. An index inside the text of a
// field becomes the index of the field, or the index after the field when bEnd is true.
sal_Int32 GetCommentSelectionIndex(const EditEngine& rEditEngine, sal_Int32 nPara, sal_Int32 nIndex,
                                   bool bEnd)
{
    sal_Int32 nFieldTextExtra = 0;
    for (const EFieldInfo& rField : rEditEngine.GetFieldInfo(nPara))
    {
        const sal_Int32 nFieldStart = rField.aPosition.nIndex + nFieldTextExtra;
        if (nIndex <= nFieldStart)
            break;
        if (nIndex < nFieldStart + rField.aCurrentText.getLength())
            return rField.aPosition.nIndex + (bEnd ? 1 : 0);
        nFieldTextExtra += rField.aCurrentText.getLength() - 1;
    }
    return nIndex - nFieldTextExtra;
}

// Returns rMatch, a match in the text where each field shows its text, as a selection of
// rEditEngine.
ESelection GetCommentSelection(const EditEngine& rEditEngine, const ESelection& rMatch)
{
    return ESelection(
        rMatch.start.nPara,
        GetCommentSelectionIndex(rEditEngine, rMatch.start.nPara, rMatch.start.nIndex, false),
        rMatch.end.nPara,
        GetCommentSelectionIndex(rEditEngine, rMatch.end.nPara, rMatch.end.nIndex, true));
}

// Returns every match of rTextSearch in the text of rEditEngine, where each field shows its text.
// A match lies inside one paragraph.
std::vector<ESelection> FindInCommentText(const EditEngine& rEditEngine,
                                          utl::TextSearch& rTextSearch)
{
    std::vector<ESelection> aMatches;
    for (sal_Int32 nPara = 0; nPara < rEditEngine.GetParagraphCount(); ++nPara)
    {
        const OUString sParaText = rEditEngine.GetText(nPara);
        sal_Int32 nStart = 0;
        sal_Int32 nEnd = sParaText.getLength();
        while (nStart < sParaText.getLength()
               && rTextSearch.SearchForward(sParaText, &nStart, &nEnd) && nStart < nEnd)
        {
            aMatches.emplace_back(nPara, nStart, nPara, nEnd);
            nStart = nEnd;
            nEnd = sParaText.getLength();
        }
    }
    return aMatches;
}

// Returns where a match found in sOldText, from nStart up to nEnd, is in the current text of
// rEditEngine, where each field shows its text. An edit before or after the match moves the
// match along with the text around it.
// When the edit changed the matched text itself, the match of rSearchOptions that starts nearest
// to the old place is returned, if the text has one.
std::optional<ESelection> LocateCommentMatch(const OUString& sOldText, sal_Int32 nStart,
                                             sal_Int32 nEnd, const EditEngine& rEditEngine,
                                             const i18nutil::SearchOptions2& rSearchOptions)
{
    const OUString sText = rEditEngine.GetText();
    if (sText != sOldText)
    {
        // The edit lies between the text that the old and the new version start with and the
        // text that they end with.
        const sal_Int32 nShorterLength = std::min(sOldText.getLength(), sText.getLength());
        sal_Int32 nCommonStart = 0;
        while (nCommonStart < nShorterLength && sOldText[nCommonStart] == sText[nCommonStart])
            ++nCommonStart;
        sal_Int32 nCommonEnd = 0;
        while (nCommonEnd < nShorterLength - nCommonStart
               && sOldText[sOldText.getLength() - 1 - nCommonEnd]
                      == sText[sText.getLength() - 1 - nCommonEnd])
            ++nCommonEnd;

        if (nStart >= sOldText.getLength() - nCommonEnd)
        {
            nStart += sText.getLength() - sOldText.getLength();
            nEnd += sText.getLength() - sOldText.getLength();
        }
        else if (nEnd > nCommonStart)
        {
            utl::TextSearch aTextSearch(rSearchOptions);
            std::optional<ESelection> oNearest;
            sal_Int32 nNearestDistance = 0;
            for (const ESelection& rMatch : FindInCommentText(rEditEngine, aTextSearch))
            {
                const sal_Int32 nDistance = std::abs(
                    GetCommentTextOffset(rEditEngine, rMatch.start.nPara, rMatch.start.nIndex)
                    - nStart);
                if (!oNearest || nDistance < nNearestDistance)
                {
                    oNearest = rMatch;
                    nNearestDistance = nDistance;
                }
            }
            return oNearest;
        }
    }

    const EPaM aStart = GetCommentTextPosition(rEditEngine, nStart);
    const EPaM aEnd = GetCommentTextPosition(rEditEngine, nEnd);
    return ESelection(aStart.nPara, aStart.nIndex, aEnd.nPara, aEnd.nIndex);
}
}

namespace sw::sidebar
{
QuickFindPanel::SearchOptionsDialog::SearchOptionsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/swriter/ui/sidebarquickfindoptionsdialog.ui"_ustr,
                              u"SearchOptionsDialog"_ustr)
    , m_xMatchCaseCheckButton(m_xBuilder->weld_check_button(u"matchcase"_ustr))
    , m_xWholeWordsOnlyCheckButton(m_xBuilder->weld_check_button(u"wholewordsonly"_ustr))
    , m_xSimilarityCheckButton(m_xBuilder->weld_check_button(u"similarity"_ustr))
    , m_xSimilaritySettingsDialogButton(m_xBuilder->weld_button(u"similaritysettingsdialog"_ustr))
{
    m_xSimilarityCheckButton->connect_toggled(
        LINK(this, SearchOptionsDialog, SimilarityCheckButtonToggledHandler));
    m_xSimilaritySettingsDialogButton->connect_clicked(
        LINK(this, SearchOptionsDialog, SimilaritySettingsDialogButtonClickedHandler));
}

short QuickFindPanel::SearchOptionsDialog::executeSubDialog(VclAbstractDialog* dialog)
{
    assert(!m_executingSubDialog);
    comphelper::ScopeGuard g([this] { m_executingSubDialog = false; });
    m_executingSubDialog = true;
    return dialog->Execute();
}

IMPL_LINK_NOARG(QuickFindPanel::SearchOptionsDialog, SimilarityCheckButtonToggledHandler,
                weld::Toggleable&, void)
{
    m_xSimilaritySettingsDialogButton->set_sensitive(m_xSimilarityCheckButton->get_active());
}

IMPL_LINK_NOARG(QuickFindPanel::SearchOptionsDialog, SimilaritySettingsDialogButtonClickedHandler,
                weld::Button&, void)
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxSearchSimilarityDialog> pDlg(pFact->CreateSvxSearchSimilarityDialog(
        m_xDialog.get(), m_bIsLEVRelaxed, m_nLEVOther, m_nLEVShorter, m_nLEVLonger));

    if (executeSubDialog(pDlg.get()) == RET_OK)
    {
        m_bIsLEVRelaxed = pDlg->IsRelaxed();
        m_nLEVOther = pDlg->GetOther();
        m_nLEVShorter = pDlg->GetShorter();
        m_nLEVLonger = pDlg->GetLonger();
    }
}

QuickFindPanelWindow::QuickFindPanelWindow(SfxBindings* _pBindings, SfxChildWindow* pChildWin,
                                           vcl::Window* pParent, SfxChildWinInfo* pInfo)
    : SfxQuickFind(_pBindings, pChildWin, pParent, pInfo)
    , m_xQuickFindPanel(std::make_unique<QuickFindPanel>(m_xContainer.get(),
                                                         _pBindings->GetActiveFrame(), _pBindings))
{
    _pBindings->Invalidate(SID_QUICKFIND);
}

QuickFindPanelWrapper::QuickFindPanelWrapper(vcl::Window* pParent, sal_uInt16 nId,
                                             SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxQuickFindWrapper(pParent, nId)
{
    SetWindow(VclPtr<QuickFindPanelWindow>::Create(pBindings, this, pParent, pInfo));
    Initialize();
}

SFX_IMPL_DOCKINGWINDOW(QuickFindPanelWrapper, SID_QUICKFIND);

std::unique_ptr<PanelLayout>
QuickFindPanel::Create(weld::Widget* pParent,
                       const cpo::uno::Reference<css::frame::XFrame>& rxFrame,
                       SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            u"no parent Window given to QuickFindPanel::Create"_ustr, nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException(u"no XFrame given to QuickFindPanel::Create"_ustr,
                                             nullptr, 0);
    return std::make_unique<QuickFindPanel>(pParent, rxFrame, pBindings);
}

QuickFindPanel::QuickFindPanel(weld::Widget* pParent, const uno::Reference<frame::XFrame>& rxFrame,
                               SfxBindings* pBindings)
    : PanelLayout(pParent, u"QuickFindPanel"_ustr, u"modules/swriter/ui/sidebarquickfind.ui"_ustr)
    , m_xSearchFindEntry(m_xBuilder->weld_entry(u"Find"_ustr))
    , m_xSearchOptionsToolbar(m_xBuilder->weld_toolbar(u"searchoptionstoolbar"_ustr))
    , m_xFindAndReplaceToolbar(m_xBuilder->weld_toolbar(u"findandreplacetoolbar"_ustr))
    , m_xFindAndReplaceToolbarDispatch(
          new ToolbarUnoDispatcher(*m_xFindAndReplaceToolbar, *m_xBuilder, rxFrame))
    , m_xTopbar(m_xBuilder->weld_box(u"topbar"_ustr))
    , m_xQuickFindControls(m_xBuilder->weld_box(u"quickfindcontrols"_ustr))
    , m_xSearchFindsList(m_xBuilder->weld_tree_view(u"searchfinds"_ustr))
    , m_xSearchFindFoundTimesLabel(m_xBuilder->weld_label(u"numberofsearchfinds"_ustr))
    , m_xFindNextButton(m_xBuilder->weld_button(u"findnext"_ustr))
    , m_xFindPreviousButton(m_xBuilder->weld_button(u"findprevious"_ustr))
    , m_pWrtShell(::GetActiveWrtShell())
    , m_xAcceleratorExecute(svt::AcceleratorExecute::createAcceleratorHelper())
    , m_pBindings(pBindings)
{
    m_xAcceleratorExecute->init(comphelper::getProcessComponentContext(), rxFrame);

    if (comphelper::COKit::isActive())
    {
        sal_uInt64 nShellId = reinterpret_cast<sal_uInt64>(SfxViewShell::Current());
        jsdialog::SendQuickFindForView(nShellId);

        // disable search options for online as still tunneled dialog
        m_xSearchOptionsToolbar->set_visible(false);
        m_xTopbar->set_visible(false);
    }
    m_nMinimumPanelWidth
        = m_xBuilder->weld_widget(u"box"_ustr)->get_preferred_size().getWidth() + (6 * 2) + 6;
    m_xContainer->set_size_request(m_nMinimumPanelWidth, 1);

    m_xSearchFindEntry->connect_focus_in(LINK(this, QuickFindPanel, SearchFindEntryFocusInHandler));
    m_xSearchFindEntry->connect_activate(
        LINK(this, QuickFindPanel, SearchFindEntryActivateHandler));
    m_xSearchFindEntry->connect_changed(LINK(this, QuickFindPanel, SearchFindEntryChangedHandler));
    m_xSearchFindEntry->connect_key_press(
        LINK(this, QuickFindPanel, SearchFindEntryKeyInputHandler));

    m_xSearchOptionsToolbar->connect_clicked(
        LINK(this, QuickFindPanel, SearchOptionsToolbarClickedHandler));

    m_xFindAndReplaceToolbar->connect_clicked(
        LINK(this, QuickFindPanel, FindAndReplaceToolbarClickedHandler));

    if (!comphelper::COKit::isActive())
    {
        m_xSearchFindsList->connect_custom_get_size(
            LINK(this, QuickFindPanel, SearchFindsListCustomGetSizeHandler));
        m_xSearchFindsList->connect_custom_render(
            LINK(this, QuickFindPanel, SearchFindsListRender));
        m_xSearchFindsList->set_column_custom_renderer(1, true);
    }

    m_xSearchFindsList->connect_selection_changed(
        LINK(this, QuickFindPanel, SearchFindsListSelectionChangedHandler));
    m_xSearchFindsList->connect_row_activated(
        LINK(this, QuickFindPanel, SearchFindsListRowActivatedHandler));
    m_xSearchFindsList->connect_mouse_press(
        LINK(this, QuickFindPanel, SearchFindsListMousePressHandler));

    m_xQuickFindControls->set_visible(false);

    m_xFindNextButton->connect_clicked(LINK(this, QuickFindPanel, FindNextClickedHandler));
    m_xFindPreviousButton->connect_clicked(LINK(this, QuickFindPanel, FindPreviousClickedHandler));
}

IMPL_LINK_NOARG(QuickFindPanel, SearchOptionsToolbarClickedHandler, const OUString&, void)
{
    SearchOptionsDialog aDlg(GetFrameWeld());

    aDlg.m_xMatchCaseCheckButton->set_active(m_bMatchCase);
    aDlg.m_xWholeWordsOnlyCheckButton->set_active(m_bWholeWordsOnly);
    aDlg.m_xSimilarityCheckButton->set_active(m_bSimilarity);
    aDlg.m_xSimilaritySettingsDialogButton->set_sensitive(m_bSimilarity);
    if (m_bSimilarity)
    {
        aDlg.m_bIsLEVRelaxed = m_bIsLEVRelaxed;
        aDlg.m_nLEVOther = m_nLEVOther;
        aDlg.m_nLEVShorter = m_nLEVShorter;
        aDlg.m_nLEVLonger = m_nLEVLonger;
    }

    if (aDlg.run() == RET_OK)
    {
        m_bMatchCase = aDlg.m_xMatchCaseCheckButton->get_active();
        m_bWholeWordsOnly = aDlg.m_xWholeWordsOnlyCheckButton->get_active();
        m_bSimilarity = aDlg.m_xSimilarityCheckButton->get_active();
        if (m_bSimilarity)
        {
            m_bIsLEVRelaxed = aDlg.m_bIsLEVRelaxed;
            m_nLEVOther = aDlg.m_nLEVOther;
            m_nLEVShorter = aDlg.m_nLEVShorter;
            m_nLEVLonger = aDlg.m_nLEVLonger;
        }
        FillSearchFindsList();
    }
}

// tdf#162580 related: When upgrading from Find toolbar search to advanced Find and Replace
// search dialog, inherit (pre-fill) search field's term from current value of find bar's
// focused search entry

bool QuickFindPanel::UpgradeSearchToSearchDialog()
{
    m_pWrtShell->AssureStdMode();
    SvxSearchDialog* pSearchDialog = SwView::GetSearchDialog();
    if (!pSearchDialog)
    {
        m_pBindings->ExecuteSynchron(SID_SEARCH_DLG);
        pSearchDialog = SwView::GetSearchDialog();
    }
    if (pSearchDialog)
    {
        pSearchDialog->SetSearchLabel(EMPTY_OUSTRING);
        pSearchDialog->SetSearchLBEntryTextAndGrabFocus(m_xSearchFindEntry->get_text());
        pSearchDialog->Present();
        return true;
    }
    return false;
}

IMPL_LINK(QuickFindPanel, FindAndReplaceToolbarClickedHandler, const OUString&, rCommand, void)
{
    if (rCommand == "searchdialog")
        UpgradeSearchToSearchDialog();
}

IMPL_LINK(QuickFindPanel, SearchFindEntryKeyInputHandler, const KeyEvent&, rKeyEvent, bool)
{
    const OUString aCommand(m_xAcceleratorExecute->findCommand(
        svt::AcceleratorExecute::st_VCLKey2AWTKey(rKeyEvent.GetKeyCode())));
    if (aCommand == ".uno:SearchDialog")
        return UpgradeSearchToSearchDialog();
    return false;
}

QuickFindPanel::~QuickFindPanel()
{
    m_xSearchFindEntry.reset();
    m_xSearchFindsList.reset();
    m_xAcceleratorExecute.reset();
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindEntryFocusInHandler, weld::Widget&, void)
{
    if (m_xSearchFindEntry->get_text().getLength())
        m_xSearchFindEntry->select_region(0, m_xSearchFindEntry->get_text().getLength());
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindEntryChangedHandler, weld::Entry&, void)
{
    m_xSearchFindEntry->set_message_type(weld::EntryMessageType::Normal);
    m_xSearchFindsList->clear();
    m_xSearchFindFoundTimesLabel->set_label(OUString());
    m_xQuickFindControls->set_visible(false);
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindEntryActivateHandler, weld::Entry&, bool)
{
    FillSearchFindsList();
    return true;
}

IMPL_LINK(QuickFindPanel, SearchFindsListMousePressHandler, const MouseEvent&, rMEvt, bool)
{
    if (std::unique_ptr<weld::TreeIter> xEntry(m_xSearchFindsList->make_iterator());
        m_xSearchFindsList->get_dest_row_at_pos(rMEvt.GetPosPixel(), xEntry.get(), false, false))
    {
        return IsPageEntry(*xEntry);
    }
    return false;
}

IMPL_LINK(QuickFindPanel, SearchFindsListCustomGetSizeHandler, weld::TreeView::get_size_args,
          aPayload, Size)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const OUString& rId = std::get<1>(aPayload);

    const bool bPageEntry = IsPageEntry(rId);

    OUString aEntry(rId);
    if (!bPageEntry)
    {
        int nIndex = m_xSearchFindsList->find_id(rId);
        aEntry = m_xSearchFindsList->get_text(nIndex);
    }

    // To not have top and bottom clipping when the sidebar width is made smaller by the user
    // calculate the text rectangle using the minimum width the rectangle can become.
    int x, y, width, height;
    m_xSearchFindsList->get_extents_relative_to(*m_xContainer, x, y, width, height);

    const int leftTextMargin = 6;
    const int rightTextMargin = 6 + 3;
    tools::Long nScrollBarThickness
        = Application::GetSettings().GetStyleSettings().GetScrollBarSize();

    tools::Rectangle aInRect(Point(), Size(m_nMinimumPanelWidth - (x * 2) - leftTextMargin
                                               - nScrollBarThickness - rightTextMargin,
                                           1));

    tools::Rectangle aRect;
    if (!bPageEntry)
    {
        aRect = rRenderContext.GetTextRect(aInRect, aEntry,
                                           DrawTextFlags::VCenter | DrawTextFlags::MultiLine
                                               | DrawTextFlags::WordBreak);
    }
    else
    {
        aRect = rRenderContext.GetTextRect(aInRect, aEntry,
                                           DrawTextFlags::Center | DrawTextFlags::VCenter);
    }

    if (!bPageEntry)
    {
        aRect.AdjustTop(-3);
        aRect.AdjustBottom(+3);
    }

    return Size(1, aRect.GetHeight());
}

IMPL_LINK(QuickFindPanel, SearchFindsListRender, weld::TreeView::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    const bool bPageEntry = IsPageEntry(rId);

    OUString aEntry(rId);

    if (!bPageEntry)
    {
        int nIndex = m_xSearchFindsList->find_id(rId);
        aEntry = m_xSearchFindsList->get_text(nIndex);
    }

    if (!bPageEntry)
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        rRenderContext.SetFillColor(rStyleSettings.GetDialogColor());
        rRenderContext.SetTextColor(rStyleSettings.GetDialogTextColor());
    }

    tools::Rectangle aRect(rRect.TopLeft(),
                           Size(rRenderContext.GetOutputSize().Width(), rRect.GetHeight()));

    if (!bPageEntry)
    {
        aRect.AdjustTop(+3);
        aRect.AdjustBottom(-3);
    }

    // adjust for scrollbar when not using gtk
    if (m_pWrtShell->GetWin()->GetSystemData()->toolkit != SystemEnvData::Toolkit::Gtk)
    {
        tools::Long nScrollBarThickness
            = Application::GetSettings().GetStyleSettings().GetScrollBarSize();
        aRect.AdjustRight(-nScrollBarThickness);
    }

    if (!bPageEntry)
    {
        aRect.AdjustRight(-3);
        rRenderContext.DrawRect(aRect, 6, 6);

        aRect.AdjustLeft(+6);
        rRenderContext.DrawText(aRect, aEntry,
                                DrawTextFlags::VCenter | DrawTextFlags::MultiLine
                                    | DrawTextFlags::WordBreak);
    }
    else
    {
        aEntry = ParsePageEntry(aEntry); // remove '-' or KitPageEntryPrefix and KitPageEntrySuffix
        tools::Long aTextWidth = rRenderContext.GetTextWidth(aEntry);
        tools::Long aTextHeight = rRenderContext.GetTextHeight();

        auto popIt = rRenderContext.ScopedPush();
        svtools::ColorConfig aColorConfig;
        rRenderContext.SetLineColor(aColorConfig.GetColorValue(svtools::BUTTONTEXTCOLOR).nColor);
        rRenderContext.DrawLine(
            aRect.LeftCenter(),
            Point(aRect.Center().AdjustX(-(aTextWidth / 2)) - 4, aRect.Center().getY()));
        rRenderContext.DrawText(Point(aRect.Center().AdjustX(-(aTextWidth / 2)),
                                      aRect.Center().AdjustY(-(aTextHeight / 2) - 1)),
                                aEntry);
        rRenderContext.DrawLine(
            Point(aRect.Center().AdjustX(aTextWidth / 2) + 5, aRect.Center().getY()),
            aRect.RightCenter());
    }
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindsListSelectionChangedHandler, weld::TreeView&, void)
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xSearchFindsList->make_iterator());
    if (!m_xSearchFindsList->get_cursor(xEntry.get()))
        return;

    OUString sId = m_xSearchFindsList->get_id(*xEntry);

    // check for page number entry
    if (IsPageEntry(sId))
        return;

    const SearchFind& rSearchFind = m_vSearchFinds[sId.toUInt64()];
    if (rSearchFind.m_nPostItId)
    {
        // The comment can have been deleted after the search, or edited so that the match is no
        // longer in it. Then no match is picked, and the label shows the number of matches.
        if (!SelectCommentSearchFind(rSearchFind))
        {
            m_xSearchFindsList->unselect_all();
            SetSearchFindFoundTimesLabel();
            return;
        }
    }
    else
    {
        // Leave the comment that an earlier comment match put the cursor in.
        SwPostItMgr* pPostItMgr = m_pWrtShell->GetPostItMgr();
        if (pPostItMgr && pPostItMgr->HasActiveSidebarWin())
            pPostItMgr->SetActiveSidebarWin(nullptr);

        const std::unique_ptr<SwPaM>& rxPaM = rSearchFind.m_xPaM;

        m_pWrtShell->StartAction();
        bool bFound = false;
        for (SwPaM& rPaM : m_pWrtShell->GetCursor()->GetRingContainer())
        {
            if (*rxPaM->GetPoint() == *rPaM.GetPoint() && *rxPaM->GetMark() == *rPaM.GetMark())
            {
                bFound = true;
                break;
            }
            m_pWrtShell->GoNextCursor();
        }
        if (!bFound)
        {
            m_pWrtShell->AssureStdMode();
            m_pWrtShell->SetSelection(*rxPaM);
        }
        m_pWrtShell->EndAction();
    }

    // tdf#163100 Need more FIND details
    // Set the found times label to show "Match X of N matches found."
    auto nSearchFindFoundTimes = m_vSearchFinds.size();
    OUString sText = SwResId(STR_SEARCH_KEY_FOUND_XOFN, nSearchFindFoundTimes);
    sText = sText.replaceFirst("%1", OUString::number(sId.toUInt32() + 1));
    sText = sText.replaceFirst("%2", OUString::number(nSearchFindFoundTimes));
    m_xSearchFindFoundTimesLabel->set_label(sText);
    if (nSearchFindFoundTimes > 1)
        m_xQuickFindControls->set_visible(true);

    // A comment match is shown by the selection in the comment.
    if (rSearchFind.m_nPostItId)
        return;

    SwShellCursor* pShellCursor = m_pWrtShell->GetCursor_();
    std::vector<basegfx::B2DRange> vRanges;
    for (const SwRect& rRect : *pShellCursor)
    {
        tools::Rectangle aRect = rRect.SVRect();
        vRanges.emplace_back(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
    }
    m_pWrtShell->GetView().BringToAttention(std::move(vRanges));
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindsListRowActivatedHandler, weld::TreeView&, bool)
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xSearchFindsList->make_iterator());
    if (!m_xSearchFindsList->get_cursor(xEntry.get()))
        return false;

    // check for page number entry
    if (IsPageEntry(*xEntry))
        return false;

    m_pWrtShell->GetView().GetEditWin().GrabFocus();
    return true;
}

IMPL_LINK_NOARG(QuickFindPanel, FindNextClickedHandler, weld::Button&, void)
{
    NavigateSearchFinds(true); // true = next/down
}

IMPL_LINK_NOARG(QuickFindPanel, FindPreviousClickedHandler, weld::Button&, void)
{
    NavigateSearchFinds(false); // false = previous/up
}

void QuickFindPanel::NavigateSearchFinds(bool bNext)
{
    if (!m_xSearchFindsList || m_xSearchFindsList->n_children() == 0)
        return;

    std::unique_ptr<weld::TreeIter> xEntry(m_xSearchFindsList->make_iterator());

    bool bMoved = false;

    // no current selection, select first/last entry
    if (!m_xSearchFindsList->get_cursor(xEntry.get()))
    {
        bMoved = m_xSearchFindsList->iter_nth_from_start(
            *xEntry, bNext ? 0 : m_xSearchFindsList->n_children() - 1);
    }
    else
    {
        if (bNext)
            bMoved = m_xSearchFindsList->iter_next(*xEntry);
        else
            bMoved = m_xSearchFindsList->iter_previous(*xEntry);
    }

    if (!bMoved)
    {
        bMoved = m_xSearchFindsList->iter_nth_from_start(
            *xEntry, bNext ? 0 : m_xSearchFindsList->n_children() - 1);
    }

    // Keep going until we find a non-page entry
    while (IsPageEntry(*xEntry))
    {
        // skip page entry
        if (bNext)
            bMoved = m_xSearchFindsList->iter_next(*xEntry);
        else
            bMoved = m_xSearchFindsList->iter_previous(*xEntry);

        if (!bMoved)
        {
            bMoved = m_xSearchFindsList->iter_nth_from_start(
                *xEntry, bNext ? 0 : m_xSearchFindsList->n_children() - 1);
        }
    }
    m_xSearchFindsList->set_cursor(*xEntry);

    // todo: ideally we dont need to manually call this handler, but select disables event propagation see SalInstanceTreeView::select(const weld::TreeIter& rIter)
    m_xSearchFindsList->select(*xEntry);
    SearchFindsListSelectionChangedHandler(*m_xSearchFindsList);
}

void QuickFindPanel::FillSearchFindsList()
{
    m_vSearchFinds.clear();
    m_xSearchFindsList->clear();
    m_xSearchFindFoundTimesLabel->set_label(OUString());
    m_xQuickFindControls->set_visible(false);

    const OUString sFindEntry = m_xSearchFindEntry->get_text();
    if (sFindEntry.isEmpty())
        return;

    SwWait aWait(*m_pWrtShell->GetDoc()->GetDocShell(), true);

    m_pWrtShell->AssureStdMode();

    i18nutil::SearchOptions2 aSearchOptions;
    aSearchOptions.Locale = GetAppLanguageTag().getLocale();
    aSearchOptions.searchString = sFindEntry;
    aSearchOptions.replaceString.clear();
    if (m_bWholeWordsOnly)
        aSearchOptions.searchFlag |= css::util::SearchFlags::NORM_WORD_ONLY;
    if (m_bSimilarity)
    {
        aSearchOptions.AlgorithmType2 = css::util::SearchAlgorithms2::APPROXIMATE;
        if (m_bIsLEVRelaxed)
            aSearchOptions.searchFlag |= css::util::SearchFlags::LEV_RELAXED;
        aSearchOptions.changedChars = m_nLEVOther;
        aSearchOptions.insertedChars = m_nLEVShorter;
        aSearchOptions.deletedChars = m_nLEVLonger;
    }
    else
        aSearchOptions.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;
    TransliterationFlags nTransliterationFlags = TransliterationFlags::IGNORE_WIDTH;
    if (!m_bMatchCase)
        nTransliterationFlags |= TransliterationFlags::IGNORE_CASE;
    aSearchOptions.transliterateFlags = nTransliterationFlags;
    m_aSearchOptions = aSearchOptions;

    m_pWrtShell->StartAllAction();
    /*sal_Int32 nFound =*/m_pWrtShell->SearchPattern(
        aSearchOptions, false, SwDocPositions::Start, SwDocPositions::End,
        FindRanges::InBody | FindRanges::InSelAll, false);
    m_pWrtShell->EndAllAction();

    if (m_pWrtShell->HasMark())
    {
        for (SwPaM& rPaM : m_pWrtShell->GetCursor()->GetRingContainer())
        {
            SwPosition* pMarkPosition = rPaM.GetMark();
            SwPosition* pPointPosition = rPaM.GetPoint();
            SearchFind aSearchFind;
            aSearchFind.m_xPaM = std::make_unique<SwPaM>(*pMarkPosition, *pPointPosition);
            aSearchFind.m_sEntryText = MakeSearchFindEntryText(
                pMarkPosition->GetContentNode()->GetTextNode()->GetText(),
                pMarkPosition->GetContentIndex(), pPointPosition->GetContentIndex());
            m_vSearchFinds.push_back(std::move(aSearchFind));
        }
    }

    AppendCommentSearchFinds();

    if (!m_vSearchFinds.empty())
    {
        // tdf#160538 sort finds in frames and footnotes in the order they occur in the document
        const SwNodeOffset nEndOfInsertsIndex
            = m_pWrtShell->GetNodes().GetEndOfInserts().GetIndex();
        const SwNodeOffset nEndOfExtrasIndex = m_pWrtShell->GetNodes().GetEndOfExtras().GetIndex();
        std::stable_sort(m_vSearchFinds.begin(), m_vSearchFinds.end(),
                         [&nEndOfInsertsIndex, &nEndOfExtrasIndex,
                          this](const SearchFind& rA, const SearchFind& rB) {
                             const std::unique_ptr<SwPaM>& a = rA.m_xPaM;
                             const std::unique_ptr<SwPaM>& b = rB.m_xPaM;
                             SwPosition aPos(*a->Start());
                             SwPosition bPos(*b->Start());
                             // use page number for footnotes and endnotes
                             if (aPos.GetNodeIndex() >= nEndOfInsertsIndex
                                 && bPos.GetNodeIndex() < nEndOfInsertsIndex)
                                 return b->GetPageNum() >= a->GetPageNum();
                             // use anchor position for finds that are located in flys
                             if (nEndOfExtrasIndex >= aPos.GetNodeIndex())
                                 getAnchorPos(aPos);
                             if (nEndOfExtrasIndex >= bPos.GetNodeIndex())
                                 getAnchorPos(bPos);
                             if (aPos == bPos)
                             {
                                 // probably in same or nested fly frame
                                 // sort using layout position
                                 SwRect aCharRect, bCharRect;
                                 if (SwContentFrame* pFrame
                                     = a->GetMarkContentNode()->GetTextNode()->getLayoutFrame(
                                         m_pWrtShell->GetLayout()))
                                 {
                                     pFrame->GetCharRect(aCharRect, *a->GetMark());
                                 }
                                 if (SwContentFrame* pFrame
                                     = b->GetMarkContentNode()->GetTextNode()->getLayoutFrame(
                                         m_pWrtShell->GetLayout()))
                                 {
                                     pFrame->GetCharRect(bCharRect, *b->GetMark());
                                 }
                                 return aCharRect.Top() < bCharRect.Top();
                             }
                             return aPos < bPos;
                         });

        // fill list
        for (sal_uInt16 nPage = 0, i = 0; SearchFind & rSearchFind : m_vSearchFinds)
        {
            // tdf#161291 indicate page of search finds
            if (rSearchFind.m_xPaM->GetPageNum() != nPage)
            {
                nPage = rSearchFind.m_xPaM->GetPageNum();
                OUString sPageEntry = CreatePageEntry(nPage);
                m_xSearchFindsList->append(sPageEntry, sPageEntry);
            }

            OUString sId = OUString::number(i++);
            m_xSearchFindsList->append(sId, rSearchFind.m_sEntryText);
        }
    }

    // Any finds?
    auto nSearchFindFoundTimes = m_vSearchFinds.size();

    // set the search term entry background
    m_xSearchFindEntry->set_message_type(nSearchFindFoundTimes ? weld::EntryMessageType::Normal
                                                               : weld::EntryMessageType::Error);
    // make the search finds list focusable or not
    m_xSearchFindsList->set_sensitive(bool(nSearchFindFoundTimes));

    SetSearchFindFoundTimesLabel();
}

void QuickFindPanel::SetSearchFindFoundTimesLabel()
{
    // set the search term found label number of times found
    auto nSearchFindFoundTimes = m_vSearchFinds.size();
    OUString sText(SwResId(STR_SEARCH_KEY_FOUND_TIMES, nSearchFindFoundTimes));
    sText = sText.replaceFirst("%1", OUString::number(nSearchFindFoundTimes));
    m_xSearchFindFoundTimesLabel->set_label(sText);
    if (nSearchFindFoundTimes > 1)
        m_xQuickFindControls->set_visible(true);
}

void QuickFindPanel::AppendCommentSearchFinds()
{
    SwPostItMgr* pPostItMgr = m_pWrtShell->GetPostItMgr();
    if (!pPostItMgr)
        return;

    utl::TextSearch aTextSearch(m_aSearchOptions);
    const bool bShowHiddenChar = m_pWrtShell->GetViewOptions()->IsShowHiddenChar();
    for (const std::unique_ptr<SwAnnotationItem>& pItem : *pPostItMgr)
    {
        // Search the comments that are shown next to the document, and the resolved comments
        // also while resolved comments are hidden. A comment is hidden as well when its anchor is
        // not laid out, or when its anchor is in hidden text and hidden characters are not shown.
        if (!pItem->mpPostIt || (!pItem->mbShow && !pItem->mpPostIt->IsResolved())
            || pItem->mLayoutStatus == SwPostItHelper::INVISIBLE
            || (pItem->mLayoutStatus == SwPostItHelper::HIDDEN && !bShowHiddenChar))
            continue;

        sw::annotation::SwAnnotationWin& rWin = *pItem->mpPostIt;
        const SwPostItField* pField = rWin.GetPostItField();
        const OUString sAuthor = pField->GetPar1();
        const OUString sEntryPrefix
            = sAuthor.isEmpty() ? SwResId(STR_QUICKFIND_COMMENT)
                                : SwResId(STR_QUICKFIND_COMMENT_BY).replaceFirst("%1", sAuthor);
        const EditEngine& rEditEngine = rWin.GetOutliner()->GetEditEngine();
        const OUString sCommentText = rEditEngine.GetText();
        for (const ESelection& rMatch : FindInCommentText(rEditEngine, aTextSearch))
        {
            SearchFind aSearchFind;
            aSearchFind.m_xPaM = std::make_unique<SwPaM>(pItem->GetAnchorPosition());
            aSearchFind.m_nPostItId = pField->GetPostItId();
            aSearchFind.m_sCommentText = sCommentText;
            aSearchFind.m_nCommentStart
                = GetCommentTextOffset(rEditEngine, rMatch.start.nPara, rMatch.start.nIndex);
            aSearchFind.m_nCommentEnd
                = GetCommentTextOffset(rEditEngine, rMatch.end.nPara, rMatch.end.nIndex);
            aSearchFind.m_sEntryText
                = sEntryPrefix + " "
                  + MakeSearchFindEntryText(rEditEngine.GetText(rMatch.start.nPara),
                                            rMatch.start.nIndex, rMatch.end.nIndex);
            m_vSearchFinds.push_back(std::move(aSearchFind));
        }
    }
}

bool QuickFindPanel::SelectCommentSearchFind(const SearchFind& rSearchFind)
{
    SwPostItMgr* pPostItMgr = m_pWrtShell->GetPostItMgr();
    if (!pPostItMgr)
        return false;

    // The comment is gone when it was deleted after the search.
    sw::annotation::SwAnnotationWin* pWin = pPostItMgr->GetAnnotationWin(rSearchFind.m_nPostItId);
    if (!pWin)
        return false;

    // The comment text can have changed after the search.
    const std::optional<ESelection> oSelection = LocateCommentMatch(
        rSearchFind.m_sCommentText, rSearchFind.m_nCommentStart, rSearchFind.m_nCommentEnd,
        pWin->GetOutliner()->GetEditEngine(), m_aSearchOptions);
    if (!oSelection)
        return false;

    // A resolved comment is hidden while resolved comments are not shown. Picking a match in it
    // shows the resolved comments, as their toggle does.
    if (pWin->IsResolved() && !m_pWrtShell->GetViewOptions()->IsResolvedPostIts())
        m_pWrtShell->GetView().GetViewFrame().GetDispatcher()->Execute(SID_TOGGLE_RESOLVED_NOTES,
                                                                       SfxCallMode::SYNCHRON);

    pWin->GetOutlinerView()->SetSelection(
        GetCommentSelection(pWin->GetOutliner()->GetEditEngine(), *oSelection));
    if (!comphelper::COKit::isActive())
    {
        pPostItMgr->ShowSearchFindInComment(*pWin);
        return true;
    }

    // The client shows and edits comments itself, and the comment stays inactive here, so the
    // keys the client sends keep going to the document. The cursor moves to the comment anchor,
    // which brings the comment into view, and the client selects the match in its comment.
    if (pPostItMgr->HasActiveSidebarWin())
        pPostItMgr->SetActiveSidebarWin(nullptr);
    m_pWrtShell->GotoField(*pWin->GetFormatField());
    pPostItMgr->NotifySearchFindInComment(*pWin);
    return true;
}

OUString QuickFindPanel::CreatePageEntry(sal_Int32 nPageNum)
{
    if (comphelper::COKit::isActive())
    {
        return KitPageEntryPrefix + SwResId(ST_PGE) + u" "_ustr + OUString::number(nPageNum)
               + KitPageEntrySuffix;
    }
    return u"-"_ustr + SwResId(ST_PGE) + u" "_ustr + OUString::number(nPageNum);
}

bool QuickFindPanel::IsPageEntry(const weld::TreeIter& rEntry)
{
    return IsPageEntry(m_xSearchFindsList->get_id(rEntry));
}

bool QuickFindPanel::IsPageEntry(std::u16string_view sEntryId)
{
    if (comphelper::COKit::isActive())
    {
        return sEntryId.starts_with(KitPageEntryPrefix) && sEntryId.ends_with(KitPageEntrySuffix);
    }
    return sEntryId[0] == '-';
}

OUString QuickFindPanel::ParsePageEntry(const OUString& sEntryId)
{
    if (comphelper::COKit::isActive())
    {
        return sEntryId.copy(KitPageEntryPrefix.getLength(),
                             sEntryId.getLength()
                                 - KitPageEntrySuffix.getLength()); // remove '-$#~' and '~#$-'
    }
    return sEntryId.copy(1); // remove '-'
}
}
// end of namespace ::sw::sidebar
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
