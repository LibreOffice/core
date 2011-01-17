/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _NAVIPI_HXX
#define _NAVIPI_HXX

// INCLUDE ---------------------------------------------------------------

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
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

//-----------------------------------------------------------------------
class SwNavigationPI;
class SwNavHelpToolBox : public SwHelpToolBox
{
    virtual void    MouseButtonDown(const MouseEvent &rEvt);
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    public:
        SwNavHelpToolBox(SwNavigationPI* pParent, const ResId &rResId);
};


// CLASS -----------------------------------------------------------------
class SwNavigationPI : public Window,
                        public SfxControllerItem, public SfxListener
{
    friend class SwNavigationChild;
    friend class SwContentTree;
    friend class SwGlobalTree;

    // --------- members -----------------------------
    SwNavHelpToolBox    aContentToolBox;
    SwHelpToolBox       aGlobalToolBox;
    ImageList           aContentImageList;
    ImageList           aContentImageListH;
    SwContentTree       aContentTree;
    SwGlobalTree        aGlobalTree;
    ListBox             aDocListBox;
    Timer               aPageChgTimer;
    String              sContentFileName;
    String              aContextArr[3];
    String              aStatusArr[4];
    Point               aBoxBottomLeft; // Pos., wenn Box unten ist

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
    sal_uInt16  nRegionMode; // 0 - URL, 1 - Bereich mit Link 2 - B. ohne Link
    short   nZoomIn;
    short   nZoomOutInit;
    short   nZoomOut;

    sal_Bool    bSmallMode : 1;
    sal_Bool    bIsZoomedIn : 1;
    sal_Bool    bPageCtrlsVisible : 1;
    sal_Bool    bGlobalMode : 1;

    // --------- methods -----------------------------
    sal_Bool _IsZoomedIn() const {return bIsZoomedIn;}
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
    DECL_LINK( ChangePageHdl, Timer* );
    DECL_LINK( PageEditModifyHdl, Edit* );
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

    virtual         sal_Bool Close();
    virtual         void Resize();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );


    // zum App-Ende rechtzeitig ObjectShellLock loslassen
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    NumEditAction&  GetPageEdit();
    sal_Bool            ToggleTree();
    void            SetGlobalMode(sal_Bool bSet) {bGlobalMode = bSet;}

public:

    SwNavigationPI(SfxBindings*, SfxChildWindowContext*, Window*);
    ~SwNavigationPI();

    void            GotoPage(); // Seite anspringen; bindbare Funktion

    void            Update() { FillBox(); }
    void            UpdateListBox();
    void            MoveOutline(sal_uInt16 nSource, sal_uInt16 nTarget, sal_Bool bWithCilds);

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                            const SfxPoolItem* pState );

    static String   CreateDropFileName( TransferableDataHelper& rData );
    static void     CleanEntry( String& rEntry );

    sal_uInt16          GetRegionDropMode() const {return nRegionMode;}
    void            SetRegionDropMode(sal_uInt16 nNewMode);

    sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );

    sal_Bool            IsGlobalDoc() const;
    sal_Bool            IsGlobalMode() const {return    bGlobalMode;}

    SwView*         GetCreateView() const;
    void            CreateNavigationTool(const Rectangle& rRect, sal_Bool bSetFocus);
};

class SwNavigationChild : public SfxChildWindowContext
{
public:
    SwNavigationChild( Window* ,
                        sal_uInt16 nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW_CONTEXT( SwNavigationChild )
};

#endif
