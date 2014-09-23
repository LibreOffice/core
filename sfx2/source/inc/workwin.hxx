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
#ifndef INCLUDED_SFX2_SOURCE_INC_WORKWIN_HXX
#define INCLUDED_SFX2_SOURCE_INC_WORKWIN_HXX

#include <vector>
#include <deque>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XLayoutManagerListener.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/propshlp.hxx>

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

#include <sfx2/sfx.hrc>
#include <sfx2/childwin.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/viewfrm.hxx>

class SfxSplitWindow;
class SfxWorkWindow;


// This struct makes all relevant Informationen available of Toolboxes

struct SfxObjectBar_Impl
{
    sal_uInt16        nId;   // Resource - and ConfigId of Toolbox
    sal_uInt16        nMode; // special visibility flags
    sal_uInt16        nPos;
    sal_uInt16        nIndex;
    bool          bDestroy;
    OUString          aName;
    SfxInterface*     pIFace;

    SfxObjectBar_Impl() :
        nId(0),
        nMode(0),
        nPos(0),
        nIndex(0),
        bDestroy(false),
        pIFace(0)
    {}
};


// This struct makes all relevant Informationen available of the status bar

struct SfxStatBar_Impl
{
    sal_uInt16                  nId;
    bool                    bOn;
    bool                    bTemp;

    SfxStatBar_Impl() :
        nId(0),
        bOn(true),
        bTemp(false)
    {}
};



#define CHILD_NOT_VISIBLE   0
#define CHILD_ACTIVE       1    // not disabled through HidePopups
#define CHILD_NOT_HIDDEN   2    // not disabled through HideChildWindow
#define CHILD_FITS_IN      4    // not too large for output size of the parent
#define CHILD_VISIBLE       (CHILD_NOT_HIDDEN | CHILD_ACTIVE | CHILD_FITS_IN)

struct SfxChild_Impl
{
    vcl::Window*                         pWin;
    Size                            aSize;
    SfxChildAlignment               eAlign;
    sal_uInt16                          nVisible;
    bool                            bResize;
    bool                            bCanGetFocus;
    bool                            bSetFocus;

    SfxChild_Impl( vcl::Window& rChild, const Size& rSize,
                   SfxChildAlignment eAlignment, bool bIsVisible ):
        pWin(&rChild), aSize(rSize), eAlign(eAlignment), bResize(false),
        bCanGetFocus( false ), bSetFocus( false )
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
    bool                            bCreate;
    SfxChildWinInfo                 aInfo;
    SfxChild_Impl*                 pCli;          // != 0 at direct Children
    sal_uInt16                          nVisibility;
    bool                            bEnable;
    bool                            bDisabled;

    SfxChildWin_Impl( sal_uInt32 nID ) :
        nSaveId((sal_uInt16) (nID & 0xFFFF) ),
        nInterfaceId((sal_uInt16) (nID >> 16)),
        nId(nSaveId),
        pWin(0),
        bCreate(false),
        pCli(0),
        nVisibility( sal_False ),
        bEnable( true ),
        bDisabled( false )
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

typedef std::vector<SfxChild_Impl*> SfxChildList_Impl;
typedef std::vector<SfxChildWin_Impl*> SfxChildWindows_Impl;


struct SfxObjectBarList_Impl
{
    std::deque<SfxObjectBar_Impl>   aArr;
    sal_uInt16                  nAct;

    SfxObjectBar_Impl       operator[] ( sal_uInt16 n )
                            { return aArr[n]; }
    SfxObjectBar_Impl       Actual()
                            { return aArr[nAct]; }
};

#define SFX_SPLITWINDOWS_LEFT   0
#define SFX_SPLITWINDOWS_TOP    2
#define SFX_SPLITWINDOWS_RIGHT  1
#define SFX_SPLITWINDOWS_BOTTOM 3
#define SFX_SPLITWINDOWS_MAX    4



class LayoutManagerListener : public ::cppu::WeakImplHelper2<
                                         css::frame::XLayoutManagerListener,
                                         css::lang::XComponent >
{
    public:
        LayoutManagerListener( SfxWorkWindow* pWrkWin );
        virtual ~LayoutManagerListener();

        void setFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );


        //  XComponent

        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


        //  XEventListener

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


        // XLayoutManagerEventListener

        virtual void SAL_CALL layoutEvent( const ::com::sun::star::lang::EventObject& aSource, ::sal_Int16 eLayoutEvent, const ::com::sun::star::uno::Any& aInfo ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        bool                                                                m_bHasFrame;
        SfxWorkWindow*                                                          m_pWrkWin;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > m_xFrame;
        OUString                                                           m_aLayoutManagerPropName;
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
    SfxChildList_Impl       aChildren;
    SfxChildWindows_Impl    aChildWins;
    SfxBindings*            pBindings;
    vcl::Window*                 pWorkWin;
    SfxShell*               pConfigShell;
    vcl::Window*                 pActiveChild;
    sal_uInt16                  nUpdateMode;
    sal_uInt16                  nChildren;
    sal_uInt16                  nOrigMode;
    bool                    bSorted : 1;
    bool                    bDockingAllowed : 1;
    bool                    bInternalDockingAllowed : 1;
    bool                    bAllChildrenVisible : 1;
    bool                    bIsFullScreen : 1;
    bool                    bShowStatusBar : 1;
    sal_Int32               m_nLock;
    OUString           m_aStatusBarResName;
    OUString           m_aLayoutManagerPropName;
    OUString           m_aTbxTypeName;
    OUString           m_aProgressBarResName;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xLayoutManagerListener;

protected:
    void                    CreateChildWin_Impl(SfxChildWin_Impl*,bool);
    void                    RemoveChildWin_Impl(SfxChildWin_Impl*);
    void                    Sort_Impl();
    SfxChild_Impl*          FindChild_Impl( const vcl::Window& rWindow ) const;
    virtual bool            RequestTopToolSpacePixel_Impl( SvBorder aBorder );
    virtual Rectangle       GetTopRect_Impl();
    SvBorder                Arrange_Impl();
    virtual void            SaveStatus_Impl(SfxChildWindow*, const SfxChildWinInfo&);
    static bool         IsPluginMode( SfxObjectShell* pObjShell );

public:
                            SfxWorkWindow( vcl::Window *pWin, SfxBindings& rBindings, SfxWorkWindow* pParent = NULL);
    virtual                 ~SfxWorkWindow();
    SfxBindings&            GetBindings()
                            { return *pBindings; }
    vcl::Window*                 GetWindow() const
                            { return pWorkWin; }
    Rectangle               GetFreeArea( bool bAutoHide ) const;
    void                    SetDockingAllowed(bool bSet)
                            { bDockingAllowed = bSet; }
    void                    SetInternalDockingAllowed(bool bSet)
                            { bInternalDockingAllowed = bSet; }
    bool                    IsDockingAllowed() const
                            { return bDockingAllowed; }
    bool                    IsInternalDockingAllowed() const
                            { return bInternalDockingAllowed; }
    SfxWorkWindow*          GetParent_Impl() const
                            { return pParent; }

    // Methods for all Child windows
    void                    DataChanged_Impl( const DataChangedEvent& rDCEvt );
    void                    ReleaseChild_Impl( vcl::Window& rWindow );
    SfxChild_Impl*          RegisterChild_Impl( vcl::Window& rWindow, SfxChildAlignment eAlign, bool bCanGetFocus=false );
    void                    ShowChildren_Impl();
    void                    HideChildren_Impl();
    bool                    PrepareClose_Impl();
    virtual void            ArrangeChildren_Impl( bool bForce = true );
    void                    DeleteControllers_Impl();
    void                    HidePopups_Impl(bool bHide, bool bParent=false, sal_uInt16 nId=0);
    void                    ConfigChild_Impl(SfxChildIdentifier,
                                             SfxDockingConfig, sal_uInt16);
    void                    MakeChildrenVisible_Impl( bool bVis );
    void                    ArrangeAutoHideWindows( SfxSplitWindow *pSplit );
    bool                    IsAutoHideMode( const SfxSplitWindow *pSplit );
    void                    EndAutoShow_Impl( Point aPos );
    void                    SetFullScreen_Impl( bool bSet ) { bIsFullScreen = bSet; }
    bool                    IsFullScreen_Impl() const { return bIsFullScreen; }

    // Methods for Objectbars
    virtual void            UpdateObjectBars_Impl();
    void                    ResetObjectBars_Impl();
    void                    SetObjectBar_Impl( sal_uInt16 nPos, sal_uInt32 nResId,
                                    SfxInterface *pIFace, const OUString* pName=0 );
    bool                    KnowsObjectBar_Impl( sal_uInt16 nPos ) const;
    bool                    IsVisible_Impl();
    void                    MakeVisible_Impl( bool );
    void                    SetObjectBarVisibility_Impl( sal_uInt16 nVis );
    bool                    IsContainer_Impl() const;
    void                    Lock_Impl( bool );

    // Methods for ChildWindows
    void                    UpdateChildWindows_Impl();
    void                    ResetChildWindows_Impl();
    void                    SetChildWindowVisible_Impl( sal_uInt32, bool, sal_uInt16 );
    void                    ToggleChildWindow_Impl(sal_uInt16,bool);
    bool                    HasChildWindow_Impl(sal_uInt16);
    bool                    KnowsChildWindow_Impl(sal_uInt16);
    void                    ShowChildWindow_Impl(sal_uInt16, bool bVisible, bool bSetFocus);
    void                    SetChildWindow_Impl(sal_uInt16, bool bOn, bool bSetFocus);
    SfxChildWindow*         GetChildWindow_Impl(sal_uInt16);
    virtual void            InitializeChild_Impl(SfxChildWin_Impl*);
    SfxSplitWindow*         GetSplitWindow_Impl(SfxChildAlignment);

    bool                    IsVisible_Impl( sal_uInt16 nMode ) const;
    bool                    IsFloating( sal_uInt16 nId );
    void                    SetActiveChild_Impl( vcl::Window *pChild );
    virtual bool            ActivateNextChild_Impl( bool bForward = true );
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
                        SfxFrameWorkWin_Impl( vcl::Window* pWin, SfxFrame* pFrm, SfxFrame* pMaster );
    virtual void        ArrangeChildren_Impl( bool bForce = true ) SAL_OVERRIDE;
    virtual void        UpdateObjectBars_Impl() SAL_OVERRIDE;
    virtual Rectangle   GetTopRect_Impl() SAL_OVERRIDE;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
