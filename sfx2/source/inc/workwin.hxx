/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SFXWORKWIN_HXX
#define _SFXWORKWIN_HXX

#include <vector>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XLayoutManagerListener.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>     // SvUShorts
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

#include <sfx2/sfx.hrc>
#include <sfx2/childwin.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/minarray.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/viewfrm.hxx>

class SfxInPlaceEnv_Impl;
class SfxPlugInEnv_Impl;
class SfxSplitWindow;
class SfxWorkWindow;

//====================================================================
// This struct makes all relevant Informationen available of Toolboxes

struct SfxObjectBar_Impl
{
    USHORT        nId;   // Resource - and ConfigId of Toolbox
    USHORT        nMode; // special visibility flags
    USHORT        nPos;
    USHORT        nIndex;
    sal_Bool      bDestroy;
    String        aName;
    SfxInterface* pIFace;

    SfxObjectBar_Impl() :
        nId(0),
        nMode(0),
        bDestroy(sal_False),
        pIFace(0)
    {}
};

//------------------------------------------------------------------------------
// This struct makes all relevant Informationen available of the status bar

struct SfxStatBar_Impl
{
    USHORT                  nId;
    BOOL                    bOn;
    BOOL                    bTemp;

    SfxStatBar_Impl() :
        nId(0),
        bOn(TRUE),
        bTemp(FALSE)
    {}
};

//------------------------------------------------------------------------------

#define CHILD_NOT_VISIBLE   0
#define CHILD_ACTIVE       1    // not disabled through HidePopups
#define CHILD_NOT_HIDDEN   2    // not disabled through HideChildWindow
#define CHILD_FITS_IN      4    // not too large for output size of the parent
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
    BOOL                            bSetFocus;

    SfxChild_Impl( Window& rChild, const Size& rSize,
                   SfxChildAlignment eAlignment, BOOL bIsVisible ):
        pWin(&rChild), aSize(rSize), eAlign(eAlignment), bResize(FALSE),
        bCanGetFocus( FALSE ), bSetFocus( FALSE )
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
    USHORT                         nSaveId;       // the ChildWindow-Id
    USHORT                         nInterfaceId;  // the current context
    USHORT                         nId;           // current Id
    SfxChildWindow*                 pWin;
    BOOL                            bCreate;
    SfxChildWinInfo                 aInfo;
    SfxChild_Impl*                 pCli;          // != 0 at direct Children
    USHORT                          nVisibility;
    BOOL                            bEnable;
    BOOL                            bDisabled;

    SfxChildWin_Impl( sal_uInt32 nID ) :
        nSaveId((USHORT) (nID & 0xFFFF) ),
        nInterfaceId((USHORT) (nID >> 16)),
        nId(nSaveId),
        pWin(0),
        bCreate(FALSE),
        pCli(0),
        nVisibility( FALSE ),
        bEnable( TRUE ),
        bDisabled( FALSE )
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

DECL_PTRARRAY( SfxChildList_Impl, SfxChild_Impl*, 2, 2 )
DECL_PTRARRAY( SfxChildWindows_Impl, SfxChildWin_Impl*, 2, 2 )

SV_DECL_OBJARR( SfxObjectBarArr_Impl, SfxObjectBar_Impl, 1, 2 )

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

class LayoutManagerListener : public ::com::sun::star::frame::XLayoutManagerListener,
                              public ::com::sun::star::lang::XTypeProvider,
                              public ::com::sun::star::lang::XComponent,
                              public ::cppu::OWeakObject
{
    public:
        LayoutManagerListener( SfxWorkWindow* pWrkWin );
        virtual ~LayoutManagerListener();

        SFX_DECL_XINTERFACE_XTYPEPROVIDER

        void setFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

        //---------------------------------------------------------------------------------------------------------
        //  XComponent
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XEventListener
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        // XLayoutManagerEventListener
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL layoutEvent( const ::com::sun::star::lang::EventObject& aSource, ::sal_Int16 eLayoutEvent, const ::com::sun::star::uno::Any& aInfo ) throw (::com::sun::star::uno::RuntimeException);

    private:
        sal_Bool                                                                m_bHasFrame;
        SfxWorkWindow*                                                          m_pWrkWin;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > m_xFrame;
        rtl::OUString                                                           m_aLayoutManagerPropName;
};

class SfxWorkWindow
{
    friend class UIElementWrapper;
    friend class LayoutManagerListener;

protected:
    SvUShorts               aSortedList;
    SfxStatBar_Impl         aStatBar;
    std::vector< SfxObjectBar_Impl > aObjBarList;
    Rectangle               aClientArea;
    Rectangle               aUpperClientArea;
    SfxWorkWindow*          pParent;
    SfxSplitWindow*         pSplit[SFX_SPLITWINDOWS_MAX];
    SfxChildList_Impl*      pChilds;
    SfxChildWindows_Impl*   pChildWins;
    SfxBindings*            pBindings;
    Window*                 pWorkWin;
    SfxShell*               pConfigShell;
    Window*                 pActiveChild;
    USHORT                  nUpdateMode;
    USHORT                  nChilds;
    USHORT                  nOrigMode;
    BOOL                    bSorted : 1;
    BOOL                    bDockingAllowed : 1;
    BOOL                    bInternalDockingAllowed : 1;
    BOOL                    bAllChildsVisible : 1;
    BOOL                    bIsFullScreen : 1;
    BOOL                    bShowStatusBar : 1;
    sal_Int32               m_nLock;
    rtl::OUString           m_aStatusBarResName;
    rtl::OUString           m_aLayoutManagerPropName;
    rtl::OUString           m_aTbxTypeName;
    rtl::OUString           m_aProgressBarResName;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xLayoutManagerListener;

protected:
    void                    CreateChildWin_Impl(SfxChildWin_Impl*,BOOL);
    void                    RemoveChildWin_Impl(SfxChildWin_Impl*);
    void                    Sort_Impl();
    void                    AlignChild_Impl( Window& rWindow, const Size& rNewSize,
                                     SfxChildAlignment eAlign );
    SfxChild_Impl*          FindChild_Impl( const Window& rWindow ) const;
    virtual BOOL            RequestTopToolSpacePixel_Impl( SvBorder aBorder );
    virtual Rectangle       GetTopRect_Impl();
    SvBorder                Arrange_Impl();
    virtual void            SaveStatus_Impl(SfxChildWindow*, const SfxChildWinInfo&);
    static sal_Bool         IsPluginMode( SfxObjectShell* pObjShell );

public:
                            SfxWorkWindow( Window *pWin, SfxBindings& rBindings, SfxWorkWindow* pParent = NULL);
    virtual                 ~SfxWorkWindow();
    SystemWindow*           GetTopWindow() const;
    SfxBindings&            GetBindings()
                            { return *pBindings; }
    Window*                 GetWindow() const
                            { return pWorkWin; }
    Rectangle               GetFreeArea( BOOL bAutoHide ) const;
    void                    SetDockingAllowed(BOOL bSet)
                            { bDockingAllowed = bSet; }
    void                    SetInternalDockingAllowed(BOOL bSet)
                            { bInternalDockingAllowed = bSet; }
    BOOL                    IsDockingAllowed() const
                            { return bDockingAllowed; }
    BOOL                    IsInternalDockingAllowed() const
                            { return bInternalDockingAllowed; }
    SfxWorkWindow*          GetParent_Impl() const
                            { return pParent; }
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >          CreateDispatch( const String& );

    // Methods for all Child windows
    void                    DataChanged_Impl( const DataChangedEvent& rDCEvt );
    void                    ReleaseChild_Impl( Window& rWindow );
    SfxChild_Impl*          RegisterChild_Impl( Window& rWindow, SfxChildAlignment eAlign, BOOL bCanGetFocus=FALSE );
    void                    ShowChilds_Impl();
    void                    HideChilds_Impl();
    void                    Close_Impl();
    BOOL                    PrepareClose_Impl();
    virtual void            ArrangeChilds_Impl( BOOL bForce = TRUE );
    void                    DeleteControllers_Impl();
    void                    SaveStatus_Impl();
    void                    HidePopups_Impl(BOOL bHide, BOOL bParent=FALSE, USHORT nId=0);
    void                    ConfigChild_Impl(SfxChildIdentifier,
                                             SfxDockingConfig, USHORT);
    void                    MakeChildsVisible_Impl( BOOL bVis );
    void                    ArrangeAutoHideWindows( SfxSplitWindow *pSplit );
    BOOL                    IsAutoHideMode( const SfxSplitWindow *pSplit );
    void                    EndAutoShow_Impl( Point aPos );
    void                    SetFullScreen_Impl( BOOL bSet ) { bIsFullScreen = bSet; }
    BOOL                    IsFullScreen_Impl() const { return bIsFullScreen; }

    // Methods for Objectbars
    virtual void            UpdateObjectBars_Impl();
    void                    ResetObjectBars_Impl();
    void                    SetObjectBar_Impl( USHORT nPos, sal_uInt32 nResId,
                                    SfxInterface *pIFace, const String* pName=0 );
    Window*                 GetObjectBar_Impl( USHORT nPos, sal_uInt32 nResId );
    bool                    KnowsObjectBar_Impl( USHORT nPos ) const;
    BOOL                    IsVisible_Impl();
    void                    MakeVisible_Impl( BOOL );
    void                    SetObjectBarVisibility_Impl( USHORT nVis );
    BOOL                    IsContainer_Impl() const;
    void                    Lock_Impl( BOOL );
    void                    NextObjectBar_Impl( USHORT nPos );
    USHORT                  HasNextObjectBar_Impl( USHORT nPos, String* pStr=0 );
    void                    SetObjectBarCustomizeMode_Impl( BOOL );

    // Methods for ChildWindows
    void                    UpdateChildWindows_Impl();
    void                    ResetChildWindows_Impl();
    void                    SetChildWindowVisible_Impl( sal_uInt32, BOOL, USHORT );
    void                    ToggleChildWindow_Impl(USHORT,BOOL);
    BOOL                    HasChildWindow_Impl(USHORT);
    BOOL                    KnowsChildWindow_Impl(USHORT);
    void                    ShowChildWindow_Impl(USHORT, BOOL bVisible, BOOL bSetFocus);
    void                    SetChildWindow_Impl(USHORT, BOOL bOn, BOOL bSetFocus);
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
    bool                    AllowChildWindowCreation_Impl( const SfxChildWin_Impl& i_rCW ) const;

    // Methods for StatusBar
    void                    SetTempStatusBar_Impl( BOOL bSet );
    void                    ResetStatusBar_Impl();
    void                    SetStatusBar_Impl(sal_uInt32 nResId, SfxShell *pShell, SfxBindings& );
    void                    UpdateStatusBar_Impl();
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > GetStatusIndicator();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > GetFrameInterface();
};

class SfxFrameWorkWin_Impl : public SfxWorkWindow
{
    SfxFrame*           pMasterFrame;
    SfxFrame*           pFrame;
public:
                        SfxFrameWorkWin_Impl( Window* pWin, SfxFrame* pFrm, SfxFrame* pMaster );
    virtual void        ArrangeChilds_Impl( BOOL bForce = TRUE );
    virtual void        UpdateObjectBars_Impl();
    virtual Rectangle   GetTopRect_Impl();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
