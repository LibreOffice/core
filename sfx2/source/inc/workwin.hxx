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
#ifndef _SFXWORKWIN_HXX
#define _SFXWORKWIN_HXX

#include <vector>
#include <deque>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XLayoutManagerListener.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

#include <sfx2/sfx.hrc>
#include <sfx2/childwin.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/minarray.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/viewfrm.hxx>

class SfxSplitWindow;
class SfxWorkWindow;

//====================================================================
// This struct makes all relevant Informationen available of Toolboxes

struct SfxObjectBar_Impl
{
    sal_uInt16        nId;   // Resource - and ConfigId of Toolbox
    sal_uInt16        nMode; // special visibility flags
    sal_uInt16        nPos;
    sal_uInt16        nIndex;
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
    sal_uInt16                  nId;
    sal_Bool                    bOn;
    sal_Bool                    bTemp;

    SfxStatBar_Impl() :
        nId(0),
        bOn(sal_True),
        bTemp(sal_False)
    {}
};

//------------------------------------------------------------------------------

#define CHILD_NOT_VISIBLE   0
#define CHILD_ACTIVE       1    // not disabled through HidePopups
#define CHILD_NOT_HIDDEN   2    // not disabled through HideChildWindow
#define CHILD_FITS_IN      4    // not too large for output size of the parent
#define CHILD_VISIBLE       (CHILD_NOT_HIDDEN | CHILD_ACTIVE | CHILD_FITS_IN)

struct SfxChild_Impl
{
    Window*                         pWin;
    Size                            aSize;
    SfxChildAlignment               eAlign;
    sal_uInt16                          nVisible;
    sal_Bool                            bResize;
    sal_Bool                            bCanGetFocus;
    sal_Bool                            bSetFocus;

    SfxChild_Impl( Window& rChild, const Size& rSize,
                   SfxChildAlignment eAlignment, sal_Bool bIsVisible ):
        pWin(&rChild), aSize(rSize), eAlign(eAlignment), bResize(sal_False),
        bCanGetFocus( sal_False ), bSetFocus( sal_False )
    {
        nVisible = bIsVisible ? CHILD_VISIBLE : CHILD_NOT_VISIBLE;
    }
};

struct SfxChildWin_Impl
{
    sal_uInt16                         nSaveId;       // the ChildWindow-Id
    sal_uInt16                         nInterfaceId;  // the current context
    sal_uInt16                         nId;           // current Id
    SfxChildWindow*                 pWin;
    sal_Bool                            bCreate;
    SfxChildWinInfo                 aInfo;
    SfxChild_Impl*                 pCli;          // != 0 at direct Children
    sal_uInt16                          nVisibility;
    sal_Bool                            bEnable;
    sal_Bool                            bDisabled;

    SfxChildWin_Impl( sal_uInt32 nID ) :
        nSaveId((sal_uInt16) (nID & 0xFFFF) ),
        nInterfaceId((sal_uInt16) (nID >> 16)),
        nId(nSaveId),
        pWin(0),
        bCreate(sal_False),
        pCli(0),
        nVisibility( sal_False ),
        bEnable( sal_True ),
        bDisabled( sal_False )
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


struct SfxObjectBarList_Impl
{
    std::deque<SfxObjectBar_Impl>   aArr;
    sal_uInt16                  nAct;

    SfxObjectBar_Impl       operator[] ( sal_uInt16 n )
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
    friend class LayoutManagerListener;

protected:
    std::vector<sal_uInt16> aSortedList;
    SfxStatBar_Impl         aStatBar;
    std::vector< SfxObjectBar_Impl > aObjBarList;
    Rectangle               aClientArea;
    Rectangle               aUpperClientArea;
    SfxWorkWindow*          pParent;
    SfxSplitWindow*         pSplit[SFX_SPLITWINDOWS_MAX];
    SfxChildList_Impl*      pChildren;
    SfxChildWindows_Impl*   pChildWins;
    SfxBindings*            pBindings;
    Window*                 pWorkWin;
    SfxShell*               pConfigShell;
    Window*                 pActiveChild;
    sal_uInt16                  nUpdateMode;
    sal_uInt16                  nChildren;
    sal_uInt16                  nOrigMode;
    sal_Bool                    bSorted : 1;
    sal_Bool                    bDockingAllowed : 1;
    sal_Bool                    bInternalDockingAllowed : 1;
    sal_Bool                    bAllChildrenVisible : 1;
    sal_Bool                    bIsFullScreen : 1;
    sal_Bool                    bShowStatusBar : 1;
    sal_Int32               m_nLock;
    rtl::OUString           m_aStatusBarResName;
    rtl::OUString           m_aLayoutManagerPropName;
    rtl::OUString           m_aTbxTypeName;
    rtl::OUString           m_aProgressBarResName;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xLayoutManagerListener;

protected:
    void                    CreateChildWin_Impl(SfxChildWin_Impl*,sal_Bool);
    void                    RemoveChildWin_Impl(SfxChildWin_Impl*);
    void                    Sort_Impl();
    SfxChild_Impl*          FindChild_Impl( const Window& rWindow ) const;
    virtual sal_Bool            RequestTopToolSpacePixel_Impl( SvBorder aBorder );
    virtual Rectangle       GetTopRect_Impl();
    SvBorder                Arrange_Impl();
    virtual void            SaveStatus_Impl(SfxChildWindow*, const SfxChildWinInfo&);
    static sal_Bool         IsPluginMode( SfxObjectShell* pObjShell );

public:
                            SfxWorkWindow( Window *pWin, SfxBindings& rBindings, SfxWorkWindow* pParent = NULL);
    virtual                 ~SfxWorkWindow();
    SfxBindings&            GetBindings()
                            { return *pBindings; }
    Window*                 GetWindow() const
                            { return pWorkWin; }
    Rectangle               GetFreeArea( sal_Bool bAutoHide ) const;
    void                    SetDockingAllowed(sal_Bool bSet)
                            { bDockingAllowed = bSet; }
    void                    SetInternalDockingAllowed(sal_Bool bSet)
                            { bInternalDockingAllowed = bSet; }
    sal_Bool                    IsDockingAllowed() const
                            { return bDockingAllowed; }
    sal_Bool                    IsInternalDockingAllowed() const
                            { return bInternalDockingAllowed; }
    SfxWorkWindow*          GetParent_Impl() const
                            { return pParent; }

    // Methods for all Child windows
    void                    DataChanged_Impl( const DataChangedEvent& rDCEvt );
    void                    ReleaseChild_Impl( Window& rWindow );
    SfxChild_Impl*          RegisterChild_Impl( Window& rWindow, SfxChildAlignment eAlign, sal_Bool bCanGetFocus=sal_False );
    void                    ShowChildren_Impl();
    void                    HideChildren_Impl();
    sal_Bool                    PrepareClose_Impl();
    virtual void            ArrangeChildren_Impl( sal_Bool bForce = sal_True );
    void                    DeleteControllers_Impl();
    void                    HidePopups_Impl(sal_Bool bHide, sal_Bool bParent=sal_False, sal_uInt16 nId=0);
    void                    ConfigChild_Impl(SfxChildIdentifier,
                                             SfxDockingConfig, sal_uInt16);
    void                    MakeChildrenVisible_Impl( sal_Bool bVis );
    void                    ArrangeAutoHideWindows( SfxSplitWindow *pSplit );
    sal_Bool                    IsAutoHideMode( const SfxSplitWindow *pSplit );
    void                    EndAutoShow_Impl( Point aPos );
    void                    SetFullScreen_Impl( sal_Bool bSet ) { bIsFullScreen = bSet; }
    sal_Bool                    IsFullScreen_Impl() const { return bIsFullScreen; }

    // Methods for Objectbars
    virtual void            UpdateObjectBars_Impl();
    void                    ResetObjectBars_Impl();
    void                    SetObjectBar_Impl( sal_uInt16 nPos, sal_uInt32 nResId,
                                    SfxInterface *pIFace, const String* pName=0 );
    bool                    KnowsObjectBar_Impl( sal_uInt16 nPos ) const;
    sal_Bool                    IsVisible_Impl();
    void                    MakeVisible_Impl( sal_Bool );
    void                    SetObjectBarVisibility_Impl( sal_uInt16 nVis );
    sal_Bool                    IsContainer_Impl() const;
    void                    Lock_Impl( sal_Bool );

    // Methods for ChildWindows
    void                    UpdateChildWindows_Impl();
    void                    ResetChildWindows_Impl();
    void                    SetChildWindowVisible_Impl( sal_uInt32, sal_Bool, sal_uInt16 );
    void                    ToggleChildWindow_Impl(sal_uInt16,sal_Bool);
    sal_Bool                    HasChildWindow_Impl(sal_uInt16);
    sal_Bool                    KnowsChildWindow_Impl(sal_uInt16);
    void                    ShowChildWindow_Impl(sal_uInt16, sal_Bool bVisible, sal_Bool bSetFocus);
    void                    SetChildWindow_Impl(sal_uInt16, sal_Bool bOn, sal_Bool bSetFocus);
    SfxChildWindow*         GetChildWindow_Impl(sal_uInt16);
    virtual void            InitializeChild_Impl(SfxChildWin_Impl*);
    SfxSplitWindow*         GetSplitWindow_Impl(SfxChildAlignment);

    sal_Bool                    IsVisible_Impl( sal_uInt16 nMode ) const;
    sal_Bool                    IsFloating( sal_uInt16 nId );
    void                    SetActiveChild_Impl( Window *pChild );
    virtual sal_Bool            ActivateNextChild_Impl( sal_Bool bForward = sal_True );
    bool                    AllowChildWindowCreation_Impl( const SfxChildWin_Impl& i_rCW ) const;

    // Methods for StatusBar
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
    virtual void        ArrangeChildren_Impl( sal_Bool bForce = sal_True );
    virtual void        UpdateObjectBars_Impl();
    virtual Rectangle   GetTopRect_Impl();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
