/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/quickfind.hxx>
#include <svx/svxdlg.hxx>
#include "wrtsh.hxx"
#include <sfx2/weldutils.hxx>

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
                                               const uno::Reference<frame::XFrame>& rxFrame);

    QuickFindPanel(weld::Widget* pParent, const uno::Reference<frame::XFrame>& rxFrame);
    virtual ~QuickFindPanel() override;

private:
    friend class QuickFindPanelWindow;
    std::vector<std::unique_ptr<SwPaM>> m_vPaMs;

    std::unique_ptr<weld::Entry> m_xSearchFindEntry;
    std::unique_ptr<weld::Toolbar> m_xSearchOptionsToolbar;
    std::unique_ptr<weld::Toolbar> m_xFindAndReplaceToolbar;
    std::unique_ptr<ToolbarUnoDispatcher> m_xFindAndReplaceToolbarDispatch;
    std::unique_ptr<weld::TreeView> m_xSearchFindsList;
    std::unique_ptr<weld::Label> m_xSearchFindFoundTimesLabel;

    SwWrtShell* m_pWrtShell;

    int m_nMinimumPanelWidth;

    bool m_bMatchCase = false;
    bool m_bWholeWordsOnly = false;
    bool m_bSimilarity = false;
    bool m_bIsLEVRelaxed = true;
    sal_uInt16 m_nLEVOther = 2;
    sal_uInt16 m_nLEVShorter = 2;
    sal_uInt16 m_nLEVLonger = 2;

    DECL_LINK(SearchFindEntryActivateHandler, weld::Entry&, bool);
    DECL_LINK(SearchFindEntryChangedHandler, weld::Entry&, void);
    DECL_LINK(SearchFindsListCustomGetSizeHandler, weld::TreeView::get_size_args, Size);
    DECL_LINK(SearchFindsListRender, weld::TreeView::render_args, void);
    DECL_LINK(SearchFindsListSelectionChangedHandler, weld::TreeView&, void);
    DECL_LINK(SearchFindsListRowActivatedHandler, weld::TreeView&, bool);
    DECL_LINK(SearchFindsListMousePressHandler, const MouseEvent&, bool);
    DECL_LINK(SearchOptionsToolbarClickedHandler, const OUString&, void);
    DECL_LINK(FindAndReplaceToolbarClickedHandler, const OUString&, void);

    void FillSearchFindsList();
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
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
