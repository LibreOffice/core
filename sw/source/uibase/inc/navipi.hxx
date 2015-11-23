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

#include <vcl/lstbox.hxx>
#include <vcl/idle.hxx>
#include <svl/lstner.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/tbxctrl.hxx>
#include <conttree.hxx>
#include <popbox.hxx>

class SwWrtShell;
class SwNavigationPI;
class SwNavigationChild;
class SfxBindings;
class NumEditAction;
class SwView;
class SwNavigationConfig;
class SfxObjectShellLock;
class SfxChildWindowContext;
class SwNavigationPI;
enum class RegionMode;
class SpinField;

class SwNavHelpToolBox : public SwHelpToolBox
{
    virtual void    MouseButtonDown(const MouseEvent &rEvt) override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    public:
        SwNavHelpToolBox(SwNavigationPI* pParent, const ResId &rResId);
};

class SwNavigationPI : public vcl::Window,
                        public SfxControllerItem, public SfxListener
{
    friend class SwNavigationChild;
    friend class SwContentTree;
    friend class SwGlobalTree;

    VclPtr<SwNavHelpToolBox>    m_aContentToolBox;
    VclPtr<SwHelpToolBox>       m_aGlobalToolBox;
    ImageList                   m_aContentImageList;
    VclPtr<SwContentTree>       m_aContentTree;
    VclPtr<SwGlobalTree>        m_aGlobalTree;
    VclPtr<ListBox>             m_aDocListBox;
    Idle                m_aPageChgIdle;
    OUString            m_sContentFileName;
    OUString            m_aContextArr[3];
    OUString            m_aStatusArr[4];
    Point               m_aBoxBottomLeft; // Pos when Box is at bottom

    SfxObjectShellLock  *m_pxObjectShell;
    SwView              *m_pContentView;
    SwWrtShell          *m_pContentWrtShell;
    SwView              *m_pActContView;
    SwView              *m_pCreateView;
    VclPtr<SfxPopupWindow>      m_pPopupWindow;
    VclPtr<SfxPopupWindow>      m_pFloatingWindow;

    SfxChildWindowContext* m_pContextWin;

    SwNavigationConfig  *m_pConfig;
    SfxBindings         &m_rBindings;

    long    m_nDocLBIniHeight;
    long    m_nWishWidth;
    sal_uInt16  m_nAutoMarkIdx;
    RegionMode  m_nRegionMode; // 0 - URL, 1 - region with link 2 - region without link
    short   m_nZoomIn;
    short   m_nZoomOutInit;
    short   m_nZoomOut;

    bool    m_bIsZoomedIn : 1;
    bool    m_bPageCtrlsVisible : 1;
    bool    m_bGlobalMode : 1;

    bool _IsZoomedIn() const {return m_bIsZoomedIn;}
    void _ZoomOut();
    void _ZoomIn();

    void FillBox();
    void MakeMark();

    DECL_LINK_TYPED( DocListBoxSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( ToolBoxSelectHdl, ToolBox *, void );
    DECL_LINK_TYPED( ToolBoxClickHdl, ToolBox *, void );
    DECL_LINK_TYPED( ToolBoxDropdownClickHdl, ToolBox*, void );
    DECL_LINK_TYPED( EditAction, NumEditAction&, void );
    DECL_LINK_TYPED( EditGetFocus, Control&, void );
    DECL_LINK_TYPED( DoneLink, SfxPoolItem *, void );
    DECL_LINK_TYPED( MenuSelectHdl, Menu *, bool );
    DECL_LINK_TYPED( ChangePageHdl, Idle*, void );
    DECL_LINK_TYPED( PageEditModifyHdl, SpinField&, void );
    DECL_LINK_TYPED( PopupModeEndHdl, FloatingWindow*, void );
    DECL_LINK_TYPED( ClosePopupWindow, SfxPopupWindow *, void );
    void UsePage(SwWrtShell *);

    void InitImageList();
    void SetPopupWindow( SfxPopupWindow* );

    using Window::Notify;
    using Window::StateChanged;

protected:

    virtual         void Resize() override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    // release ObjectShellLock early enough for app end
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    NumEditAction&  GetPageEdit();
    bool            ToggleTree();
    void            SetGlobalMode(bool bSet) {m_bGlobalMode = bSet;}

public:

    SwNavigationPI(SfxBindings*, SfxChildWindowContext*, vcl::Window*);
    virtual ~SwNavigationPI();
    virtual void    dispose() override;

    void            GotoPage(); // jump to page; bindable function

    void            UpdateListBox();
    void            MoveOutline(sal_uInt16 nSource, sal_uInt16 nTarget, bool bWithCilds);

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                            const SfxPoolItem* pState ) override;

    static OUString CreateDropFileName( TransferableDataHelper& rData );
    static OUString CleanEntry(const OUString& rEntry);

    RegionMode      GetRegionDropMode() const {return m_nRegionMode;}
    void            SetRegionDropMode(RegionMode nNewMode);

    sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );

    bool            IsGlobalDoc() const;
    bool            IsGlobalMode() const {return    m_bGlobalMode;}

    SwView*         GetCreateView() const;
    void            CreateNavigationTool(const Rectangle& rRect, bool bSetFocus, vcl::Window *pParent);
};

class SwNavigationChild : public SfxChildWindowContext
{
public:
    SwNavigationChild( vcl::Window* ,
                        sal_uInt16 nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW_CONTEXT( SwNavigationChild )
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
