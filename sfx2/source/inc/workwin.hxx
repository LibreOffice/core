/*************************************************************************
 *
 *  $RCSfile: workwin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SFXWORKWIN_HXX
#define _SFXWORKWIN_HXX

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>     // SvUShorts

#ifndef _IFACE_HXX //autogen
#include <so3/iface.hxx>
#endif

#include "sfx.hrc"
#include "childwin.hxx"
#include "shell.hxx"
#include "minarray.hxx"
#include "ctrlitem.hxx"
#include "stbmgr.hxx"

class SfxToolBoxManager;
class SfxInPlaceEnv_Impl;
class SfxPlugInEnv_Impl;
class SfxSplitWindow;
class SfxWorkWindow;
class XDispatch;

//====================================================================
// Dieser struct h"alt alle relevanten Informationen "uber Toolboxen bereit.

struct SfxObjectBar_Impl
{
    USHORT              nId;    // Resource - und ConfigId der Toolbox
    USHORT              nMode;  // spezielle Sichtbarkeitsflags
    SfxToolBoxManager*  pTbx;
    String              aName;
    ResMgr*             pResMgr;
    SfxInterface*       pIFace;

    SfxObjectBar_Impl() :
        nId(0),
        nMode(0),
        pTbx(0),
        pResMgr(0),
        pIFace(0)
    {}
};

//------------------------------------------------------------------------------
// Dieser struct h"alt alle relevanten Informationen "uber die Statuszeile bereit.
struct SfxStatBar_Impl
{
    ResMgr*                 pResMgr;
    SfxShell*               pShell;
    SfxBindings*            pBindings;
    USHORT                  nId;
    SfxStatusBarManager*    pStatusBar;
    BOOL                    bOn;
    BOOL                    bTemp;

    SfxStatBar_Impl() :
        nId(0),
        pResMgr(0),
        pShell(0),
        pBindings(0),
        pStatusBar(0),
        bOn(TRUE),
        bTemp(FALSE)
    {}
};

//------------------------------------------------------------------------------

#define CHILD_NOT_VISIBLE   0
#define CHILD_ACTIVE        1   // nicht durch HidePopups ausgeschaltet
#define CHILD_NOT_HIDDEN    2   // nicht durch HideChildWindow ausgeschaltet
#define CHILD_FITS_IN       4   // nicht zu gro\s f"ur OutputSize des parent
#define CHILD_VISIBLE       (CHILD_NOT_HIDDEN | CHILD_ACTIVE | CHILD_FITS_IN)
#define CHILD_ISVISIBLE     (CHILD_NOT_HIDDEN | CHILD_ACTIVE)

struct SfxChild_Impl
{
    Window*                         pWin;
    Size                            aSize;
    SfxChildAlignment               eAlign;
    USHORT                          nVisible;
    BOOL                            bResize;
    BOOL                            bCanGetFocus;

    SfxChild_Impl( Window& rChild, const Size& rSize,
                   SfxChildAlignment eAlignment, BOOL bIsVisible ):
        pWin(&rChild), aSize(rSize), eAlign(eAlignment), bResize(FALSE),
        bCanGetFocus( FALSE )
    {
        nVisible = bIsVisible ? CHILD_VISIBLE : CHILD_NOT_VISIBLE;
    }
};

//--------------------------------------------------------------------
class SfxChildWinController_Impl : public SfxControllerItem
{
    SfxWorkWindow*  pWorkwin;

    public:
                    SfxChildWinController_Impl( USHORT nId, SfxWorkWindow *pWin );
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                            const SfxPoolItem* pState );
};

struct SfxChildWin_Impl
{
    USHORT                          nSaveId;        // die ChildWindow-Id
    USHORT                          nInterfaceId;   // der aktuelle Context
    USHORT                          nId;            // aktuelle Id
    SfxChildWindow*                 pWin;
    BOOL                            bCreate;
    SfxChildWinInfo                 aInfo;
    SfxChild_Impl*                  pCli;           // != 0 bei direkten Children
    USHORT                          nVisibility;
    BOOL                            bEnable;
    SfxChildWinController_Impl*     pControl;
    BOOL                            bDisabled;

    SfxChildWin_Impl( ULONG nID ) :
        nSaveId((USHORT) (nID & 0xFFFF) ),
        nInterfaceId((USHORT) (nID >> 16)),
        nId(nSaveId),
        pWin(0),
        bCreate(FALSE),
        pCli(0),
        nVisibility( FALSE ),
        bEnable( TRUE ),
        bDisabled( FALSE ),
        pControl( 0 )
    {}
};

enum SfxChildIdentifier
{
    SFX_CHILDWIN_STATBAR,
    SFX_CHILDWIN_OBJECTBAR,
    SFX_CHILDWIN_DOCKINGWINDOW,
    SFX_CHILDWIN_SPLITWINDOW
};

enum SfxDockingConfig
{
    SFX_SETDOCKINGRECTS,
    SFX_ALIGNDOCKINGWINDOW,
    SFX_TOGGLEFLOATMODE,
    SFX_MOVEDOCKINGWINDOW
};

DECL_PTRARRAY( SfxChildList_Impl, SfxChild_Impl*, 2, 2 );
DECL_PTRARRAY( SfxChildWindows_Impl, SfxChildWin_Impl*, 2, 2 );

SV_DECL_OBJARR( SfxObjectBarArr_Impl, SfxObjectBar_Impl, 1, 2 );

struct SfxObjectBarList_Impl
{
    SfxObjectBarArr_Impl    aArr;
    USHORT                  nAct;

    SfxObjectBar_Impl       operator[] ( USHORT n )
                            { return aArr[n]; }
    SfxObjectBar_Impl       Actual()
                            { return aArr[nAct]; }
};

struct SfxSplitWin_Impl
{
    SfxSplitWindow*         pSplitWin;
    SfxChildWindows_Impl*   pChildWins;
};

#define SFX_SPLITWINDOWS_LEFT   0
#define SFX_SPLITWINDOWS_TOP    2
#define SFX_SPLITWINDOWS_RIGHT  1
#define SFX_SPLITWINDOWS_BOTTOM 3
#define SFX_SPLITWINDOWS_MAX    4

//--------------------------------------------------------------------

class SfxWorkWindow
{
protected:
    SfxStatBar_Impl         aStatBar;
    SfxWorkWindow*          pParent;
    SvUShorts               aSortedList;
    SfxObjectBar_Impl       aObjBars[SFX_OBJECTBAR_MAX];
    SfxObjectBarList_Impl   aObjBarLists[SFX_OBJECTBAR_MAX];
    SfxSplitWindow*         pSplit[SFX_SPLITWINDOWS_MAX];
    SfxChildList_Impl*      pChilds;
    SfxChildWindows_Impl*   pChildWins;
    SfxBindings*            pBindings;
    Window*                 pWorkWin;
    USHORT                  nUpdateMode;
    USHORT                  nChilds;
    BOOL                    bSorted;
    USHORT                  nOrigMode;
    Rectangle               aClientArea;
    BOOL                    bDockingAllowed;
    BOOL                    bAllChildsVisible;
    SfxShell*               pConfigShell;
    Window*                 pActiveChild;

protected:
    void                    CreateChildWin_Impl(SfxChildWin_Impl*);
    void                    RemoveChildWin_Impl(SfxChildWin_Impl*);
    void                    Sort_Impl();
    void                    AlignChild_Impl( Window& rWindow, const Size& rNewSize,
                                     SfxChildAlignment eAlign );
    SfxChild_Impl*          FindChild_Impl( const Window& rWindow ) const;
    virtual BOOL            RequestTopToolSpacePixel_Impl( SvBorder aBorder );
    virtual Rectangle       GetTopRect_Impl();
    SvBorder                Arrange_Impl( Rectangle & rRect);
    virtual void            SaveStatus_Impl(SfxChildWindow*, const SfxChildWinInfo&);

public:
                            SfxWorkWindow( Window *pWin, SfxBindings& rBindings, SfxWorkWindow* pParent = NULL);
                            ~SfxWorkWindow();
    SystemWindow*           GetTopWindow() const;
    SfxBindings&            GetBindings()
                            { return *pBindings; }
    Window*                 GetWindow() const
                            { return pWorkWin; }
    Rectangle               GetFreeArea( BOOL bAutoHide ) const;
    void                    SetDockingAllowed(BOOL bSet)
                            { bDockingAllowed = bSet; }
    BOOL                    IsDockingAllowed() const
                            { return bDockingAllowed; }
    SfxWorkWindow*          GetParent_Impl() const
                            { return pParent; }
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >          CreateDispatch( const String& );

    // Methoden f"ur alle Child-Fenster
    void                    ReleaseChild_Impl( Window& rWindow );
    SfxChild_Impl*          RegisterChild_Impl( Window& rWindow, SfxChildAlignment eAlign, BOOL bCanGetFocus=FALSE );
    void                    ShowChilds_Impl();
    void                    HideChilds_Impl();
    void                    Close_Impl();
    BOOL                    PrepareClose_Impl();
    virtual void            ArrangeChilds_Impl();
    void                    DeleteControllers_Impl();
    void                    HidePopups_Impl(BOOL bHide, BOOL bParent=FALSE, USHORT nId=0);
    void                    ConfigChild_Impl(SfxChildIdentifier,
                                             SfxDockingConfig, USHORT);
    void                    MakeChildsVisible_Impl( BOOL bVis );
    void                    ArrangeAutoHideWindows( SfxSplitWindow *pSplit );
    BOOL                    IsAutoHideMode( const SfxSplitWindow *pSplit );
    void                    EndAutoShow_Impl();

    // Methoden f"ur Objectbars
    virtual void            UpdateObjectBars_Impl();
    void                    ResetObjectBars_Impl();
    void                    SetObjectBar_Impl( USHORT nPos, const ResId& rId,
                                    SfxInterface *pIFace, const String* pName=0 );
    Window*                 GetObjectBar_Impl( USHORT nPos, ResId& rResId );
    FASTBOOL                KnowsObjectBar_Impl( USHORT nPos ) const;
    void                    SetObjectBarVisibility_Impl( USHORT nVis );
    BOOL                    IsContainer_Impl() const;
    void                    NextObjectBar_Impl( USHORT nPos );
    USHORT                  HasNextObjectBar_Impl( USHORT nPos, String* pStr=0 );

    // Methoden f"ur ChildWindows
    void                    UpdateChildWindows_Impl();
    void                    ResetChildWindows_Impl();
    void                    SetChildWindowVisible_Impl( ULONG, BOOL, USHORT );
    void                    ToggleChildWindow_Impl(USHORT);
    BOOL                    HasChildWindow_Impl(USHORT);
    BOOL                    KnowsChildWindow_Impl(USHORT);
    void                    ShowChildWindow_Impl(USHORT, BOOL bVisible=TRUE);
    void                    SetChildWindow_Impl(USHORT, BOOL bOn);
    SfxChildWindow*         GetChildWindow_Impl(USHORT);
    virtual void            InitializeChild_Impl(SfxChildWin_Impl*);
    SfxSplitWindow*         GetSplitWindow_Impl(SfxChildAlignment);

    BOOL                    IsVisible_Impl( USHORT nMode ) const;
    void                    DisableChildWindow_Impl( USHORT nId, BOOL bDisable );
    BOOL                    IsFloating( USHORT nId );
    void                    ChangeWindow_Impl( Window *pNew );
    void                    SetActiveChild_Impl( Window *pChild );
    Window*                 GetActiveChild_Impl();
    virtual BOOL            ActivateNextChild_Impl( BOOL bForward = TRUE );

    // Methoden f"ur StatusBar
    void                    SetTempStatusBar_Impl( BOOL bSet );
    void                    ResetStatusBar_Impl();
    void                    SetStatusBar_Impl(const ResId&, SfxShell *pShell, SfxBindings& );
    void                    UpdateStatusBar_Impl();
    SfxStatusBarManager*    GetStatusBarManager_Impl();
};

class SfxPIWorkWin_Impl : public SfxWorkWindow
{
    SfxPlugInEnv_Impl*  pEnv;
    Window*             pDialogParent;

public:
                        SfxPIWorkWin_Impl(
                            Window *pWin, SfxBindings &rBindings,
                            SfxPlugInEnv_Impl*  pE);
    virtual             Window* GetModalDialogParent() const;

private:
    virtual void        ArrangeChilds_Impl();
    virtual void        UpdateObjectBars_Impl();
    virtual void        SaveStatus_Impl(SfxChildWindow*, const SfxChildWinInfo&);
    virtual void        InitializeChild_Impl(SfxChildWin_Impl*);
    virtual Rectangle   GetTopRect_Impl();
};


class SfxIPWorkWin_Impl : public SfxWorkWindow
{
    SfxInPlaceEnv_Impl* pEnv;

public:
                        SfxIPWorkWin_Impl( WorkWindow *pWin, SfxBindings &rBindings,
                                SfxInPlaceEnv_Impl* pE);
private:
    virtual BOOL        RequestTopToolSpacePixel_Impl( SvBorder aBorder );
    virtual void        UpdateObjectBars_Impl();
    virtual Rectangle   GetTopRect_Impl();
    virtual void        ArrangeChilds_Impl();
    virtual void        SaveStatus_Impl(SfxChildWindow*, const SfxChildWinInfo&);
};

class SfxFrameWorkWin_Impl : public SfxWorkWindow
{
    SfxFrame*           pFrame;
public:
                        SfxFrameWorkWin_Impl( Window* pWin, SfxFrame* pTask );
    virtual void        ArrangeChilds_Impl();
    virtual void        UpdateObjectBars_Impl();
    virtual Rectangle   GetTopRect_Impl();
};


#endif
