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

class SwNavHelpToolBox : public SwHelpToolBox
{
    virtual void    MouseButtonDown(const MouseEvent &rEvt) SAL_OVERRIDE;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
    public:
        SwNavHelpToolBox(SwNavigationPI* pParent, const ResId &rResId);
};

class SwNavigationPI : public vcl::Window,
                        public SfxControllerItem, public SfxListener
{
    friend class SwNavigationChild;
    friend class SwContentTree;
    friend class SwGlobalTree;

    SwNavHelpToolBox    aContentToolBox;
    SwHelpToolBox       aGlobalToolBox;
    ImageList           aContentImageList;
    SwContentTree       aContentTree;
    SwGlobalTree        aGlobalTree;
    ListBox             aDocListBox;
    Timer               aPageChgTimer;
    OUString            sContentFileName;
    OUString            aContextArr[3];
    OUString            aStatusArr[4];
    Point               aBoxBottomLeft; // Pos when Box is at bottom

    SfxObjectShellLock  *pxObjectShell;
    SwView              *pContentView;
    SwWrtShell          *pContentWrtShell;
    SwView              *pActContView;
    SwView              *pCreateView;
    SfxPopupWindow      *pPopupWindow;
    SfxPopupWindow      *pFloatingWindow;

    SfxChildWindowContext* pContextWin;

    SwNavigationConfig  *pConfig;
    SfxBindings         &rBindings;

    long    nDocLBIniHeight;
    long    nWishWidth;
    sal_uInt16  nAutoMarkIdx;
    sal_uInt16  nRegionMode; // 0 - URL, 1 - region with link 2 - region without link
    short   nZoomIn;
    short   nZoomOutInit;
    short   nZoomOut;

    bool    bSmallMode : 1;
    bool    bIsZoomedIn : 1;
    bool    bPageCtrlsVisible : 1;
    bool    bGlobalMode : 1;

    bool _IsZoomedIn() const {return bIsZoomedIn;}
    void _ZoomOut();
    void _ZoomIn();

    void FillBox();
    void MakeMark();

    DECL_LINK( DocListBoxSelectHdl, ListBox * );
    DECL_LINK( ToolBoxSelectHdl, ToolBox * );
    DECL_LINK( ToolBoxClickHdl, ToolBox * );
    DECL_LINK( ToolBoxDropdownClickHdl, ToolBox* );
    DECL_LINK( EditAction, NumEditAction * );
    DECL_LINK( EditGetFocus, NumEditAction * );
    DECL_LINK( DoneLink, SfxPoolItem * );
    DECL_LINK( MenuSelectHdl, Menu * );
    DECL_LINK( ChangePageHdl, void* );
    DECL_LINK( PageEditModifyHdl, void* );
    DECL_LINK( PopupModeEndHdl, void * );
    DECL_LINK( ClosePopupWindow, SfxPopupWindow * );
    void UsePage(SwWrtShell *);

    void MakeVisible();
    void InitImageList();
    virtual SfxChildAlignment
                    CheckAlignment(SfxChildAlignment,SfxChildAlignment);
    void SetPopupWindow( SfxPopupWindow* );

    using Window::Notify;
    using Window::StateChanged;

protected:

    virtual         bool Close();
    virtual         void Resize() SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    // release ObjectShellLock early enough for app end
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    NumEditAction&  GetPageEdit();
    bool            ToggleTree();
    void            SetGlobalMode(bool bSet) {bGlobalMode = bSet;}

public:

    SwNavigationPI(SfxBindings*, SfxChildWindowContext*, vcl::Window*);
    virtual ~SwNavigationPI();

    void            GotoPage(); // jump to page; bindable function

    void            Update() { FillBox(); }
    void            UpdateListBox();
    void            MoveOutline(sal_uInt16 nSource, sal_uInt16 nTarget, bool bWithCilds);

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                            const SfxPoolItem* pState ) SAL_OVERRIDE;

    static OUString CreateDropFileName( TransferableDataHelper& rData );
    static OUString CleanEntry(const OUString& rEntry);

    sal_uInt16          GetRegionDropMode() const {return nRegionMode;}
    void            SetRegionDropMode(sal_uInt16 nNewMode);

    sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );

    bool            IsGlobalDoc() const;
    bool            IsGlobalMode() const {return    bGlobalMode;}

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
