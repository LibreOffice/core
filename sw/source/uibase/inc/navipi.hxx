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

#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/idle.hxx>
#include <svl/lstner.hxx>
#include <vcl/transfer.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include "conttree.hxx"
#include <ndarr.hxx>
#include <memory>

class SwWrtShell;
class SwNavigationPI;
class SwNavigationChild;
class SfxBindings;
class NumEditAction;
class SwView;
class SwNavigationConfig;
class SfxObjectShellLock;
class SfxChildWindowContext;
enum class RegionMode;
class SpinField;

class SwNavHelpToolBox : public ToolBox
{
    VclPtr<SwNavigationPI> m_xDialog;
    virtual void    MouseButtonDown(const MouseEvent &rEvt) override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    dispose() override;
public:
    SwNavHelpToolBox(Window* pParent);
    void SetDialog(SwNavigationPI* pDialog)
    {
        m_xDialog = pDialog;
    }
    ~SwNavHelpToolBox() override;
};

class SwNavigationPI : public PanelLayout,
                       public SfxControllerItem, public SfxListener
{
    friend class SwNavigationChild;
    friend class SwContentTree;
    friend class SwGlobalTree;
    friend class SwNavigationPIUIObject;

    VclPtr<SwNavHelpToolBox>    m_aContentToolBox;
    VclPtr<ToolBox>             m_aGlobalToolBox;
    VclPtr<NumEditAction>       m_xEdit;
    VclPtr<VclContainer>        m_aContentBox;
    VclPtr<SwContentTree>       m_aContentTree;
    VclPtr<VclContainer>        m_aGlobalBox;
    VclPtr<SwGlobalTree>        m_aGlobalTree;
    VclPtr<ListBox>             m_aDocListBox;
    Idle                m_aPageChgIdle;
    OUString            m_sContentFileName;
    OUString            m_aContextArr[3];
    OUString            m_aStatusArr[4];

    std::unique_ptr<SfxObjectShellLock>  m_pxObjectShell;
    SwView              *m_pContentView;
    SwWrtShell          *m_pContentWrtShell;
    SwView              *m_pActContView;
    SwView              *m_pCreateView;
    VclPtr<SfxPopupWindow>      m_pPopupWindow;
    VclPtr<SfxPopupWindow>      m_pFloatingWindow;

    SwNavigationConfig  *m_pConfig;
    SfxBindings         &m_rBindings;

    sal_uInt16  m_nAutoMarkIdx;
    RegionMode  m_nRegionMode; // 0 - URL, 1 - region with link 2 - region without link
    Size        m_aExpandedSize;

    bool    m_bIsZoomedIn : 1;
    bool    m_bGlobalMode : 1;

    bool IsZoomedIn() const {return m_bIsZoomedIn;}
    void ZoomOut();
    void ZoomIn();

    void FillBox();
    void MakeMark();

    DECL_LINK( DocListBoxSelectHdl, ListBox&, void );
    DECL_LINK( ToolBoxSelectHdl, ToolBox *, void );
    DECL_LINK( ToolBoxClickHdl, ToolBox *, void );
    DECL_LINK( ToolBoxDropdownClickHdl, ToolBox*, void );
    DECL_LINK( EditAction, NumEditAction&, void );
    DECL_LINK( EditGetFocus, Control&, void );
    DECL_LINK( DoneLink, SfxPoolItem const *, void );
    DECL_LINK( MenuSelectHdl, Menu *, bool );
    DECL_LINK( ChangePageHdl, Timer*, void );
    DECL_LINK( PageEditModifyHdl, SpinField&, void );
    DECL_LINK( PopupModeEndHdl, FloatingWindow*, void );
    DECL_LINK( ClosePopupWindow, SfxPopupWindow *, void );
    void UsePage();

    void SetPopupWindow( SfxPopupWindow* );

protected:

    // release ObjectShellLock early enough for app end
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    NumEditAction&  GetPageEdit();
    void            ToggleTree();
    void            SetGlobalMode(bool bSet) {m_bGlobalMode = bSet;}

public:

    SwNavigationPI(SfxBindings*, vcl::Window*);
    virtual ~SwNavigationPI() override;
    virtual void    dispose() override;

    void            UpdateListBox();
    void            MoveOutline(SwOutlineNodes::size_type nSource, SwOutlineNodes::size_type nTarget, bool bWithCilds);

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                            const SfxPoolItem* pState ) override;

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
    void            CreateNavigationTool(const tools::Rectangle& rRect, bool bSetFocus, vcl::Window *pParent);

    FactoryFunction GetUITestFactory() const override;
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
