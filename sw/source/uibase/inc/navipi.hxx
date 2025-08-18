/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NAVIPI_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NAVIPI_HXX

#include <vcl/idle.hxx>
#include <svl/lstner.hxx>
#include <vcl/transfer.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/navigat.hxx>
#include <sfx2/weldutils.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include "conttree.hxx"
#include <ndarr.hxx>
#include <memory>
#include <optional>

class SwWrtShell;
class SfxBindings;
class SwNavigationConfig;
class SwView;
class SpinField;

class SwNavigationPI final : public PanelLayout
                     , public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
                     , public SfxListener
{
    friend class SwNavigatorWin;
    friend class SwContentTree;
    friend class SwGlobalTree;
    friend class SwView;

    ::sfx2::sidebar::ControllerItem m_aDocFullName;
    ::sfx2::sidebar::ControllerItem m_aPageStats;
    ::sfx2::sidebar::ControllerItem m_aNavElement;

    css::uno::Reference<css::frame::XFrame> m_xFrame;

    std::unique_ptr<weld::Toolbar> m_xContent1ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent2ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent3ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent4ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent5ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent6ToolBox;
    std::unique_ptr<ToolbarUnoDispatcher> m_xContent2Dispatch;
    std::unique_ptr<ToolbarUnoDispatcher> m_xContent3Dispatch;
    std::unique_ptr<weld::Menu> m_xHeadingsMenu;
    std::unique_ptr<weld::Menu> m_xUpdateMenu;
    std::unique_ptr<weld::Menu> m_xInsertMenu;
    std::unique_ptr<weld::Toolbar> m_xGlobalToolBox;
    std::unique_ptr<weld::SpinButton> m_xGotoPageSpinButton;
    std::unique_ptr<weld::Widget> m_xContentBox;
    std::unique_ptr<SwContentTree> m_xContentTree;
    std::unique_ptr<weld::Widget> m_xGlobalBox;
    std::unique_ptr<SwGlobalTree> m_xGlobalTree;
    std::unique_ptr<weld::ComboBox> m_xDocListBox;
    OUString            m_sContentFileName;

    VclPtr<SfxNavigator> m_xNavigatorDlg;

    std::optional<SfxObjectShellLock>  m_oObjectShell;
    SwView              *m_pContentView;
    SwWrtShell          *m_pContentWrtShell;
    SwView              *m_pActContView;
    SwView              *m_pCreateView;

    SwNavigationConfig  *m_pConfig;
    SfxBindings         &m_rBindings;

    Size        m_aExpandedSize;

    bool    m_bIsZoomedIn : 1;
    bool    m_bGlobalMode : 1;

    weld::ComboBox* m_pNavigateByComboBox;

    std::unique_ptr<weld::Toolbar> m_xHeadingsContentFunctionsToolbar;
    std::unique_ptr<weld::Toolbar> m_xDeleteFunctionToolbar;
    std::unordered_map<ContentTypeId, std::unique_ptr<weld::Toolbar>> m_aContentTypeUnoToolbarMap;
    std::unordered_map<ContentTypeId, std::unique_ptr<ToolbarUnoDispatcher>> m_aContentTypeToolbarUnoDispatcherMap;
    std::unordered_map<ContentTypeId, std::unique_ptr<weld::Toolbar>> m_aContentUnoToolbarMap;
    std::unordered_map<ContentTypeId, std::unique_ptr<ToolbarUnoDispatcher>> m_aContentToolbarUnoDispatcherMap;
    void InitContentFunctionsToolbar();
    void UpdateContentFunctionsToolbar();
    DECL_LINK(ContentFunctionsToolbarSelectHdl, const OUString&, void );

    bool IsZoomedIn() const {return m_bIsZoomedIn;}
    void ZoomOut();
    void ZoomIn();

    void FillBox();

    DECL_LINK( DocListBoxSelectHdl, weld::ComboBox&, void );
    DECL_LINK( ToolBoxSelectHdl, const OUString&, void );
    DECL_LINK( ToolBoxClickHdl, const OUString&, void );
    DECL_LINK( ToolBox5DropdownClickHdl, const OUString&, void );
    DECL_LINK( DoneLink, SfxPoolItem const *, void );
    DECL_LINK( HeadingsMenuSelectHdl, const OUString&, void );
    DECL_LINK( GlobalMenuSelectHdl, const OUString&, void );
    DECL_LINK( SetFocusChildHdl, weld::Container&, void );
    DECL_LINK( NavigateByComboBoxSelectHdl, weld::ComboBox&, void );
    DECL_LINK(GotoPageSpinButtonValueChangedHdl, weld::SpinButton&, void);

    void UpdateInitShow();

    // release ObjectShellLock early enough for app end
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    void            ToggleTree();
    void            SetGlobalMode(bool bSet) {m_bGlobalMode = bSet;}

    void UpdateNavigateBy();

    void SetContent3And4ToolBoxVisibility();

public:

    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent,
            const css::uno::Reference< css::frame::XFrame >& rxFrame,
            SfxBindings* pBindings);
    SwNavigationPI(weld::Widget* pParent,
            const css::uno::Reference< css::frame::XFrame >& rxFrame,
            SfxBindings* _pBindings, SfxNavigator* pNavigatorDlg);
    virtual ~SwNavigationPI() override;

    void            UpdateListBox();
    void            MoveOutline(SwOutlineNodes::size_type nSource, SwOutlineNodes::size_type nTarget);

    virtual void    NotifyItemUpdate(const sal_uInt16 nSId,
                                     const SfxItemState eState,
                                     const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override {};

    static OUString CreateDropFileName( const TransferableDataHelper& rData );
    static OUString CleanEntry(const OUString& rEntry);

    sal_Int8        AcceptDrop();
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );

    bool            IsGlobalDoc() const;
    bool            IsGlobalMode() const {return    m_bGlobalMode;}

    SwView*         GetCreateView() const;

    virtual weld::Window* GetFrameWeld() const override;

    void SelectNavigateByContentType(const OUString& rContentTypeName);
};

class SwNavigatorWin : public SfxNavigator
{
    friend class SwView;
private:
    std::unique_ptr<SwNavigationPI> m_xNavi;
public:
    SwNavigatorWin(SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                   vcl::Window* pParent, SfxChildWinInfo* pInfo);
    virtual void StateChanged(StateChangedType nStateChange) override;
    virtual void dispose() override
    {
        m_xNavi.reset();
        SfxNavigator::dispose();
    }
    virtual ~SwNavigatorWin() override
    {
        disposeOnce();
    }
};

class SwNavigatorWrapper final : public SfxNavigatorWrapper
{
public:
    SwNavigatorWrapper(vcl::Window *pParent, sal_uInt16 nId,
                       SfxBindings* pBindings, SfxChildWinInfo* pInfo);
    SFX_DECL_CHILDWINDOW(SwNavigatorWrapper);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
