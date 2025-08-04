/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#include <swmodule.hxx>
#include <pam.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <edtwin.hxx>
#include <fmtanchr.hxx>
#include <cntfrm.hxx>
#include <strings.hrc>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <swwait.hxx>

#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/bindings.hxx>

#include <vcl/jsdialog/executor.hxx>
#include <comphelper/lok.hxx>

const int CharactersBeforeAndAfter = 40;

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
    , m_xQuickFindPanel(
          std::make_unique<QuickFindPanel>(m_xContainer.get(), _pBindings->GetActiveFrame()))
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
                       const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to QuickFindPanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to QuickFindPanel::Create", nullptr,
                                             0);
    return std::make_unique<QuickFindPanel>(pParent, rxFrame);
}

QuickFindPanel::QuickFindPanel(weld::Widget* pParent, const uno::Reference<frame::XFrame>& rxFrame)
    : PanelLayout(pParent, u"QuickFindPanel"_ustr, u"modules/swriter/ui/sidebarquickfind.ui"_ustr)
    , m_xSearchFindEntry(m_xBuilder->weld_entry(u"Find"_ustr))
    , m_xSearchOptionsToolbar(m_xBuilder->weld_toolbar(u"searchoptionstoolbar"_ustr))
    , m_xFindAndReplaceToolbar(m_xBuilder->weld_toolbar(u"findandreplacetoolbar"_ustr))
    , m_xFindAndReplaceToolbarDispatch(
          new ToolbarUnoDispatcher(*m_xFindAndReplaceToolbar, *m_xBuilder, rxFrame))
    , m_xSearchFindsList(m_xBuilder->weld_tree_view(u"searchfinds"_ustr))
    , m_xSearchFindFoundTimesLabel(m_xBuilder->weld_label("numberofsearchfinds"))
    , m_pWrtShell(::GetActiveWrtShell())
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        sal_uInt64 nShellId = reinterpret_cast<sal_uInt64>(SfxViewShell::Current());
        jsdialog::SendQuickFindForView(nShellId);

        // disable search options for online as still tunnled dialog
        m_xSearchOptionsToolbar->set_visible(false);
    }
    m_nMinimumPanelWidth
        = m_xBuilder->weld_widget(u"box"_ustr)->get_preferred_size().getWidth() + (6 * 2) + 6;
    m_xContainer->set_size_request(m_nMinimumPanelWidth, 1);

    m_xSearchFindEntry->connect_activate(
        LINK(this, QuickFindPanel, SearchFindEntryActivateHandler));
    m_xSearchFindEntry->connect_changed(LINK(this, QuickFindPanel, SearchFindEntryChangedHandler));

    m_xSearchOptionsToolbar->connect_clicked(
        LINK(this, QuickFindPanel, SearchOptionsToolbarClickedHandler));

    m_xFindAndReplaceToolbar->connect_clicked(
        LINK(this, QuickFindPanel, FindAndReplaceToolbarClickedHandler));

    if (!comphelper::LibreOfficeKit::isActive())
    {
        m_xSearchFindsList->connect_custom_get_size(
            LINK(this, QuickFindPanel, SearchFindsListCustomGetSizeHandler));
        m_xSearchFindsList->connect_custom_render(
            LINK(this, QuickFindPanel, SearchFindsListRender));
        m_xSearchFindsList->set_column_custom_renderer(1, true);
    }

    m_xSearchFindsList->connect_changed(
        LINK(this, QuickFindPanel, SearchFindsListSelectionChangedHandler));
    m_xSearchFindsList->connect_row_activated(
        LINK(this, QuickFindPanel, SearchFindsListRowActivatedHandler));
    m_xSearchFindsList->connect_mouse_press(
        LINK(this, QuickFindPanel, SearchFindsListMousePressHandler));
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
IMPL_LINK(QuickFindPanel, FindAndReplaceToolbarClickedHandler, const OUString&, rCommand, void)
{
    if (!SwView::GetSearchDialog())
    {
        SvxSearchItem* pSearchItem = SwView::GetSearchItem();
        if (!pSearchItem)
        {
            pSearchItem = new SvxSearchItem(SID_SEARCH_ITEM);
            SwView::SetSearchItem(pSearchItem);
        }
        pSearchItem->SetSearchString(m_xSearchFindEntry->get_text());
    }
    m_xFindAndReplaceToolbarDispatch->Select(rCommand);
}

QuickFindPanel::~QuickFindPanel()
{
    m_xSearchFindEntry.reset();
    m_xSearchFindsList.reset();
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindEntryChangedHandler, weld::Entry&, void)
{
    m_xSearchFindEntry->set_message_type(weld::EntryMessageType::Normal);
    m_xSearchFindsList->clear();
    m_xSearchFindFoundTimesLabel->set_label(OUString());
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
        return m_xSearchFindsList->get_id(*xEntry)[0] == '-';
    }
    return false;
}

IMPL_LINK(QuickFindPanel, SearchFindsListCustomGetSizeHandler, weld::TreeView::get_size_args,
          aPayload, Size)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const OUString& rId = std::get<1>(aPayload);

    const bool bPageEntry = rId[0] == '-';

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

    const bool bPageEntry = rId[0] == '-';

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
        aEntry = aEntry.copy(1); // remove '-'
        tools::Long aTextWidth = rRenderContext.GetTextWidth(aEntry);
        tools::Long aTextHeight = rRenderContext.GetTextHeight();

        rRenderContext.Push();
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
        rRenderContext.Pop();
    }
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindsListSelectionChangedHandler, weld::TreeView&, void)
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xSearchFindsList->make_iterator());
    if (!m_xSearchFindsList->get_cursor(xEntry.get()))
        return;

    OUString sId = m_xSearchFindsList->get_id(*xEntry);

    // check for page number entry
    if (sId[0] == '-')
        return;

    std::unique_ptr<SwPaM>& rxPaM = m_vPaMs[sId.toUInt64()];

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

    // tdf#163100 Need more FIND details
    // Set the found times label to show "Match X of N matches found."
    auto nSearchFindFoundTimes = m_vPaMs.size();
    OUString sText = SwResId(STR_SEARCH_KEY_FOUND_XOFN, nSearchFindFoundTimes);
    sText = sText.replaceFirst("%1", OUString::number(sId.toUInt32() + 1));
    sText = sText.replaceFirst("%2", OUString::number(nSearchFindFoundTimes));
    m_xSearchFindFoundTimesLabel->set_label(sText);

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
    if (m_xSearchFindsList->get_id(*xEntry)[0] == '-')
        return false;

    m_pWrtShell->GetView().GetEditWin().GrabFocus();
    return true;
}

void QuickFindPanel::FillSearchFindsList()
{
    m_vPaMs.clear();
    m_xSearchFindsList->clear();
    m_xSearchFindFoundTimesLabel->set_label(OUString());

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

    m_pWrtShell->SttSelect();
    /*sal_Int32 nFound =*/m_pWrtShell->SearchPattern(
        aSearchOptions, false, SwDocPositions::Start, SwDocPositions::End,
        FindRanges::InBody | FindRanges::InSelAll, false);
    m_pWrtShell->EndSelect();

    if (m_pWrtShell->HasMark())
    {
        for (SwPaM& rPaM : m_pWrtShell->GetCursor()->GetRingContainer())
        {
            SwPosition* pMarkPosition = rPaM.GetMark();
            SwPosition* pPointPosition = rPaM.GetPoint();
            std::unique_ptr<SwPaM> xPaM(std::make_unique<SwPaM>(*pMarkPosition, *pPointPosition));
            m_vPaMs.push_back(std::move(xPaM));
        }

        // tdf#160538 sort finds in frames and footnotes in the order they occur in the document
        const SwNodeOffset nEndOfInsertsIndex
            = m_pWrtShell->GetNodes().GetEndOfInserts().GetIndex();
        const SwNodeOffset nEndOfExtrasIndex = m_pWrtShell->GetNodes().GetEndOfExtras().GetIndex();
        std::stable_sort(m_vPaMs.begin(), m_vPaMs.end(),
                         [&nEndOfInsertsIndex, &nEndOfExtrasIndex,
                          this](const std::unique_ptr<SwPaM>& a, const std::unique_ptr<SwPaM>& b) {
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
        for (sal_uInt16 nPage = 0, i = 0; std::unique_ptr<SwPaM> & xPaM : m_vPaMs)
        {
            SwPosition* pMarkPosition = xPaM->GetMark();
            SwPosition* pPointPosition = xPaM->GetPoint();

            const SwContentNode* pContentNode = pMarkPosition->GetContentNode();
            const SwTextNode* pTextNode = pContentNode->GetTextNode();
            const OUString& sNodeText = pTextNode->GetText();

            auto nMarkIndex = pMarkPosition->GetContentIndex();
            auto nPointIndex = pPointPosition->GetContentIndex();

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

            // tdf#161291 indicate page of search finds
            if (xPaM->GetPageNum() != nPage)
            {
                nPage = xPaM->GetPageNum();
                OUString sPageEntry(u"-"_ustr + SwResId(ST_PGE) + u" "_ustr
                                    + OUString::number(nPage));
                m_xSearchFindsList->append(sPageEntry, sPageEntry);
            }

            auto nCount = nMarkIndex - nStartIndex;
            OUString sTextBeforeFind = OUString::Concat(sNodeText.subView(nStartIndex, nCount));
            auto nCount1 = nPointIndex - nMarkIndex;
            OUString sFind = OUString::Concat(sNodeText.subView(nMarkIndex, nCount1));
            auto nCount2 = nEndIndex - nPointIndex + 1;
            OUString sTextAfterFind = OUString::Concat(sNodeText.subView(nPointIndex, nCount2));
            OUString sStr = sTextBeforeFind + "[" + sFind + "]" + sTextAfterFind;

            OUString sId = OUString::number(i++);
            m_xSearchFindsList->append(sId, sStr);
        }
    }

    // Any finds?
    auto nSearchFindFoundTimes = m_vPaMs.size();

    // set the search term entry background
    m_xSearchFindEntry->set_message_type(nSearchFindFoundTimes ? weld::EntryMessageType::Normal
                                                               : weld::EntryMessageType::Error);
    // make the search finds list focusable or not
    m_xSearchFindsList->set_sensitive(bool(nSearchFindFoundTimes));

    // set the search term found label number of times found
    OUString sText(SwResId(STR_SEARCH_KEY_FOUND_TIMES, nSearchFindFoundTimes));
    sText = sText.replaceFirst("%1", OUString::number(nSearchFindFoundTimes));
    m_xSearchFindFoundTimesLabel->set_label(sText);
}
}
// end of namespace ::sw::sidebar
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
