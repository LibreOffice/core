/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once
#include <i18nutil/searchopt.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/quickfind.hxx>
#include <svx/svxdlg.hxx>
#include "wrtsh.hxx"

#include <sfx2/bindings.hxx>
#include <sfx2/weldutils.hxx>
#include <svtools/acceleratorexecute.hxx>

namespace sw::sidebar
{
class QuickFindPanel : public PanelLayout
{
    class SearchOptionsDialog final : public weld::GenericDialogController
    {
        friend class QuickFindPanel;

        std::unique_ptr<weld::CheckButton> m_xMatchCaseCheckButton;
        std::unique_ptr<weld::CheckButton> m_xWholeWordsOnlyCheckButton;
        std::unique_ptr<weld::CheckButton> m_xSimilarityCheckButton;
        std::unique_ptr<weld::Button> m_xSimilaritySettingsDialogButton;

        DECL_LINK(SimilarityCheckButtonToggledHandler, weld::Toggleable&, void);
        DECL_LINK(SimilaritySettingsDialogButtonClickedHandler, weld::Button&, void);

        short executeSubDialog(VclAbstractDialog* pVclAbstractDialog);

        bool m_executingSubDialog = false;

        bool m_bIsLEVRelaxed = true;
        sal_uInt16 m_nLEVOther = 2;
        sal_uInt16 m_nLEVShorter = 2;
        sal_uInt16 m_nLEVLonger = 2;

    public:
        SearchOptionsDialog(weld::Window* pParent);
    };

public:
    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent,
                                               const cpo::uno::Reference<css::frame::XFrame>& rxFrame,
                                               SfxBindings* pBindings);

    QuickFindPanel(weld::Widget* pParent, const cpo::uno::Reference<css::frame::XFrame>& rxFrame,
                   SfxBindings* pBindings);
    virtual ~QuickFindPanel() override;

private:
    friend class QuickFindPanelWindow;

    /// One match of the search term. A match in the document text has m_nPostItId 0 and
    /// m_xPaM covers the matched text. A match inside a comment has the comment's id in
    /// m_nPostItId and a collapsed m_xPaM at the comment anchor. m_sCommentText is the text of
    /// the comment at the time of the search, with one line feed between paragraphs, and the
    /// match runs from offset m_nCommentStart up to m_nCommentEnd in it. m_sEntryText is the
    /// text of its row in the list.
    struct SearchFind
    {
        std::unique_ptr<SwPaM> m_xPaM;
        sal_uInt32 m_nPostItId = 0;
        OUString m_sCommentText;
        sal_Int32 m_nCommentStart = 0;
        sal_Int32 m_nCommentEnd = 0;
        OUString m_sEntryText;
    };
    std::vector<SearchFind> m_vSearchFinds;
    /// The options of the search that filled m_vSearchFinds.
    i18nutil::SearchOptions2 m_aSearchOptions;

    std::unique_ptr<weld::Entry> m_xSearchFindEntry;
    std::unique_ptr<weld::Toolbar> m_xSearchOptionsToolbar;
    std::unique_ptr<weld::Toolbar> m_xFindAndReplaceToolbar;
    std::unique_ptr<ToolbarUnoDispatcher> m_xFindAndReplaceToolbarDispatch;
    std::unique_ptr<weld::Box> m_xTopbar;
    std::unique_ptr<weld::Box> m_xQuickFindControls;
    std::unique_ptr<weld::TreeView> m_xSearchFindsList;
    std::unique_ptr<weld::Label> m_xSearchFindFoundTimesLabel;
    std::unique_ptr<weld::Button> m_xFindNextButton;
    std::unique_ptr<weld::Button> m_xFindPreviousButton;

    SwWrtShell* m_pWrtShell;
    std::unique_ptr<svt::AcceleratorExecute> m_xAcceleratorExecute;

    SfxBindings* m_pBindings;

    int m_nMinimumPanelWidth;

    bool m_bMatchCase = false;
    bool m_bWholeWordsOnly = false;
    bool m_bSimilarity = false;
    bool m_bIsLEVRelaxed = true;
    sal_uInt16 m_nLEVOther = 2;
    sal_uInt16 m_nLEVShorter = 2;
    sal_uInt16 m_nLEVLonger = 2;

    DECL_LINK(SearchFindEntryFocusInHandler, weld::Widget&, void);
    DECL_LINK(SearchFindEntryActivateHandler, weld::Entry&, bool);
    DECL_LINK(SearchFindEntryChangedHandler, weld::Entry&, void);
    DECL_LINK(SearchFindEntryKeyInputHandler, const KeyEvent&, bool);
    DECL_LINK(SearchFindsListCustomGetSizeHandler, weld::TreeView::get_size_args, Size);
    DECL_LINK(SearchFindsListRender, weld::TreeView::render_args, void);
    DECL_LINK(SearchFindsListSelectionChangedHandler, weld::TreeView&, void);
    DECL_LINK(SearchFindsListRowActivatedHandler, weld::TreeView&, bool);
    DECL_LINK(SearchFindsListMousePressHandler, const MouseEvent&, bool);
    DECL_LINK(SearchOptionsToolbarClickedHandler, const OUString&, void);
    DECL_LINK(FindAndReplaceToolbarClickedHandler, const OUString&, void);
    DECL_LINK(FindNextClickedHandler, weld::Button&, void);
    DECL_LINK(FindPreviousClickedHandler, weld::Button&, void);

    void NavigateSearchFinds(bool bNext);
    void FillSearchFindsList();
    void SetSearchFindFoundTimesLabel();
    void AppendCommentSearchFinds();
    /// Returns false when the comment or the match in it is gone.
    bool SelectCommentSearchFind(const SearchFind& rSearchFind);
    static OUString CreatePageEntry(sal_Int32 nPageNum);
    bool IsPageEntry(const weld::TreeIter& rEntry);
    static bool IsPageEntry(std::u16string_view sEntryId);
    static OUString ParsePageEntry(const OUString& sEntryId);
    bool UpgradeSearchToSearchDialog();
};

class QuickFindPanelWrapper : public SfxQuickFindWrapper
{
public:
    QuickFindPanelWrapper(vcl::Window* pParent, sal_uInt16 nId, SfxBindings* pBindings,
                          SfxChildWinInfo* pInfo);
    SFX_DECL_CHILDWINDOW(QuickFindPanelWrapper);
};

class QuickFindPanelWindow : public SfxQuickFind
{
private:
    std::unique_ptr<QuickFindPanel> m_xQuickFindPanel;

public:
    QuickFindPanelWindow(SfxBindings* _pBindings, SfxChildWindow* pChildWin, vcl::Window* pParent,
                         SfxChildWinInfo* pInfo);
    virtual void dispose() override
    {
        m_xQuickFindPanel.reset();
        SfxQuickFind::dispose();
    }
    virtual ~QuickFindPanelWindow() override { disposeOnce(); }
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
