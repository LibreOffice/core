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
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/weldutils.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include "conttree.hxx"
#include <ndarr.hxx>
#include <memory>

class SwWrtShell;
class SwNavigationChild;
class SfxBindings;
class SwNavigationConfig;
class SwView;
class SfxObjectShellLock;
enum class RegionMode;
class SpinField;

class SwNavigationPI : public PanelLayout
                     , public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
                     , public SfxListener
{
    friend class SwNavigationChild;
    friend class SwContentTree;
    friend class SwGlobalTree;

    ::sfx2::sidebar::ControllerItem m_aDocFullName;
    ::sfx2::sidebar::ControllerItem m_aPageStats;

    std::unique_ptr<weld::Toolbar> m_xContent1ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent2ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent3ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent4ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent5ToolBox;
    std::unique_ptr<weld::Toolbar> m_xContent6ToolBox;
    std::unique_ptr<ToolbarUnoDispatcher> m_xContent2Dispatch;
    std::unique_ptr<ToolbarUnoDispatcher> m_xContent3Dispatch;
    std::unique_ptr<weld::Menu> m_xHeadingsMenu;
    std::unique_ptr<weld::Menu> m_xDragModeMenu;
    std::unique_ptr<weld::Menu> m_xUpdateMenu;
    std::unique_ptr<weld::Menu> m_xInsertMenu;
    std::unique_ptr<weld::Toolbar> m_xGlobalToolBox;
    std::unique_ptr<weld::SpinButton> m_xEdit;
    std::unique_ptr<weld::Widget> m_xContentBox;
    std::unique_ptr<SwContentTree> m_xContentTree;
    std::unique_ptr<weld::Widget> m_xGlobalBox;
    std::unique_ptr<SwGlobalTree> m_xGlobalTree;
    std::unique_ptr<weld::ComboBox> m_xDocListBox;
    Idle                m_aPageChgIdle;
    OUString            m_sContentFileName;
    OUString            m_aStatusArr[4];

    std::unique_ptr<SfxObjectShellLock>  m_pxObjectShell;
    SwView              *m_pContentView;
    SwWrtShell          *m_pContentWrtShell;
    SwView              *m_pActContView;
    SwView              *m_pCreateView;

    SwNavigationConfig  *m_pConfig;
    SfxBindings         &m_rBindings;

    RegionMode  m_nRegionMode; // 0 - URL, 1 - region with link 2 - region without link
    Size        m_aExpandedSize;

    bool    m_bIsZoomedIn : 1;
    bool    m_bGlobalMode : 1;

    bool IsZoomedIn() const {return m_bIsZoomedIn;}
    void ZoomOut();
    void ZoomIn();

    void FillBox();

    DECL_LINK( DocListBoxSelectHdl, weld::ComboBox&, void );
    DECL_LINK( ToolBoxSelectHdl, const OString&, void );
    DECL_LINK( ToolBoxClickHdl, const OString&, void );
    DECL_LINK( ToolBox5DropdownClickHdl, const OString&, void );
    DECL_LINK( ToolBox6DropdownClickHdl, const OString&, void );
    DECL_LINK( DoneLink, SfxPoolItem const *, void );
    DECL_LINK( DropModeMenuSelectHdl, const OString&, void );
    DECL_LINK( HeadingsMenuSelectHdl, const OString&, void );
    DECL_LINK( GlobalMenuSelectHdl, const OString&, void );
    DECL_LINK( ChangePageHdl, Timer*, void );
    DECL_LINK( PageEditModifyHdl, weld::SpinButton&, void );
    DECL_LINK( EditActionHdl, weld::Entry&, bool );
    DECL_LINK( SetFocusChildHdl, weld::Container&, void );

    bool EditAction();
    void UsePage();

protected:

    // release ObjectShellLock early enough for app end
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    void            ToggleTree();
    void            SetGlobalMode(bool bSet) {m_bGlobalMode = bSet;}

public:

    static VclPtr<PanelLayout> Create(vcl::Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
            SfxBindings* pBindings);
    SwNavigationPI(vcl::Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
            SfxBindings* _pBindings);
    virtual ~SwNavigationPI() override;
    virtual void    dispose() override;

    void            UpdateListBox();
    void            MoveOutline(SwOutlineNodes::size_type nSource, SwOutlineNodes::size_type nTarget);

    virtual void    NotifyItemUpdate(const sal_uInt16 nSId,
                                     const SfxItemState eState,
                                     const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override {};

    virtual void    StateChanged(StateChangedType nStateChange) override;

    static OUString CreateDropFileName( TransferableDataHelper& rData );
    static OUString CleanEntry(const OUString& rEntry);

    RegionMode      GetRegionDropMode() const {return m_nRegionMode;}
    void            SetRegionDropMode(RegionMode nNewMode);

    sal_Int8        AcceptDrop();
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );

    bool            IsGlobalDoc() const;
    bool            IsGlobalMode() const {return    m_bGlobalMode;}

    SwView*         GetCreateView() const;
};

class SwNavigationChild : public SfxChildWindowContext
{
public:
    SwNavigationChild( vcl::Window* ,
                        sal_uInt16 nId,
                        SfxBindings*  );

    //! soon obsolete !
    static  std::unique_ptr<SfxChildWindowContext> CreateImpl(vcl::Window *pParent,
                SfxBindings *pBindings, SfxChildWinInfo* pInfo );
    static  void RegisterChildWindowContext(SfxModule *pMod);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
