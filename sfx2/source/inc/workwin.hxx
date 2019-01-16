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
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XLayoutManagerListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weakref.hxx>

#include <rtl/ustring.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/toolbarids.hxx>
#include <sfx2/viewfrm.hxx>

class SfxSplitWindow;
class SfxWorkWindow;


// This struct makes all relevant Information available of Toolboxes
struct SfxObjectBar_Impl
{
    ToolbarId          eId;   // ConfigId of Toolbox
    SfxVisibilityFlags nMode; // special visibility flags
    bool               bDestroy;

    SfxObjectBar_Impl() :
        eId(ToolbarId::None),
        nMode(SfxVisibilityFlags::Invisible),
        bDestroy(false)
    {}
};

// This struct makes all relevant Information available of the status bar

struct SfxStatBar_Impl
{
    StatusBarId eId;

    SfxStatBar_Impl() :
        eId(StatusBarId::None)
    {}
};

enum class SfxChildVisibility
{
    NOT_VISIBLE  = 0,
    ACTIVE       = 1,   // not disabled through HidePopups
    NOT_HIDDEN   = 2,   // not disabled through HideChildWindow
    FITS_IN      = 4,    // not too large for output size of the parent
    VISIBLE      = 7,   // NOT_HIDDEN | ACTIVE | FITS_IN)
};
namespace o3tl
{
    template<> struct typed_flags<SfxChildVisibility> : is_typed_flags<SfxChildVisibility, 0x07> {};
}


struct SfxChild_Impl
{
    VclPtr<vcl::Window>             pWin;
    std::shared_ptr<SfxModelessDialogController> xController;
    Size                            aSize;
    SfxChildAlignment               eAlign;
    SfxChildVisibility              nVisible;
    bool                            bResize;
    bool                            bSetFocus;

    SfxChild_Impl( vcl::Window& rChild, const Size& rSize,
                   SfxChildAlignment eAlignment, bool bIsVisible ):
        pWin(&rChild), aSize(rSize), eAlign(eAlignment), bResize(false),
        bSetFocus( false )
    {
        nVisible = bIsVisible ? SfxChildVisibility::VISIBLE : SfxChildVisibility::NOT_VISIBLE;
    }

    SfxChild_Impl(const std::shared_ptr<SfxModelessDialogController>& rChild,
                  SfxChildAlignment eAlignment):
        pWin(nullptr), xController(rChild), eAlign(eAlignment), bResize(false),
        bSetFocus( false )
    {
        nVisible = xController->getDialog()->get_visible() ? SfxChildVisibility::VISIBLE : SfxChildVisibility::NOT_VISIBLE;
    }
};

struct SfxChildWin_Impl
{
    sal_uInt16 const                nSaveId;       // the ChildWindow-Id
    sal_uInt16                      nInterfaceId;  // the current context
    sal_uInt16                      nId;           // current Id
    SfxChildWindow*                 pWin;
    bool                            bCreate;
    SfxChildWinInfo                 aInfo;
    SfxChild_Impl*                  pCli;          // != 0 at direct Children
    SfxVisibilityFlags              nVisibility;
    bool                            bEnable;

    SfxChildWin_Impl( sal_uInt32 nID ) :
        nSaveId(static_cast<sal_uInt16>(nID & 0xFFFF) ),
        nInterfaceId(static_cast<sal_uInt16>(nID >> 16)),
        nId(nSaveId),
        pWin(nullptr),
        bCreate(false),
        pCli(nullptr),
        nVisibility( SfxVisibilityFlags::Invisible ),
        bEnable( true )
    {}
};

enum class SfxChildIdentifier
{
    DOCKINGWINDOW,
    SPLITWINDOW
};

enum class SfxDockingConfig
{
    SETDOCKINGRECTS,
    ALIGNDOCKINGWINDOW,
    TOGGLEFLOATMODE
};


#define SFX_SPLITWINDOWS_LEFT   0
#define SFX_SPLITWINDOWS_TOP    2
#define SFX_SPLITWINDOWS_RIGHT  1
#define SFX_SPLITWINDOWS_MAX    4


class LayoutManagerListener : public ::cppu::WeakImplHelper<
                                         css::frame::XLayoutManagerListener,
                                         css::lang::XComponent >
{
    public:
        LayoutManagerListener( SfxWorkWindow* pWrkWin );
        virtual ~LayoutManagerListener() override;

        void setFrame( const css::uno::Reference< css::frame::XFrame >& rFrame );


        //  XComponent

        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;
        virtual void SAL_CALL dispose() override;


        //  XEventListener

        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;


        // XLayoutManagerEventListener

        virtual void SAL_CALL layoutEvent( const css::lang::EventObject& aSource, ::sal_Int16 eLayoutEvent, const css::uno::Any& aInfo ) override;

    private:
        bool                                             m_bHasFrame;
        SfxWorkWindow*                                   m_pWrkWin;
        css::uno::WeakReference< css::frame::XFrame >    m_xFrame;
};

class SfxWorkWindow final
{
    friend class LayoutManagerListener;

    std::vector<sal_uInt16> aSortedList;
    SfxStatBar_Impl         aStatBar;
    std::vector< SfxObjectBar_Impl > aObjBarList;
    tools::Rectangle               aClientArea;
    tools::Rectangle               aUpperClientArea;
    VclPtr<SfxSplitWindow>  pSplit[SFX_SPLITWINDOWS_MAX];
    std::vector<std::unique_ptr<SfxChild_Impl>>
                            aChildren;
    std::vector<std::unique_ptr<SfxChildWin_Impl>>
                            aChildWins;
    SfxBindings*            pBindings;
    VclPtr<vcl::Window>     pWorkWin;
    VclPtr<vcl::Window>     pActiveChild;
    SfxVisibilityFlags      nUpdateMode;
    sal_uInt16              nChildren;
    SfxVisibilityFlags      nOrigMode;
    bool                    bSorted : 1;
    bool                    bDockingAllowed : 1;
    bool                    bInternalDockingAllowed : 1;
    bool                    bAllChildrenVisible : 1;
    bool                    bIsFullScreen : 1;
    bool                    bShowStatusBar : 1;
    sal_Int32               m_nLock;
    css::uno::Reference< css::lang::XComponent > m_xLayoutManagerListener;
    SfxFrame*               pMasterFrame;
    SfxFrame* const         pFrame;

    void                    CreateChildWin_Impl(SfxChildWin_Impl*,bool);
    void                    RemoveChildWin_Impl(SfxChildWin_Impl*);
    void                    Sort_Impl();
    SfxChild_Impl*          FindChild_Impl( const vcl::Window& rWindow ) const;
    bool                    RequestTopToolSpacePixel_Impl( SvBorder aBorder );
    tools::Rectangle               GetTopRect_Impl();
    SvBorder                Arrange_Impl();
    void                    SaveStatus_Impl(SfxChildWindow*, const SfxChildWinInfo&);
    static bool             IsPluginMode( SfxObjectShell const * pObjShell );

    void                    FlushPendingChildSizes();

public:
                            SfxWorkWindow( vcl::Window* pWin, SfxFrame* pFrm, SfxFrame* pMaster );
                            ~SfxWorkWindow();
    SfxBindings&            GetBindings()
                            { return *pBindings; }
    vcl::Window*                 GetWindow() const
                            { return pWorkWin; }
    tools::Rectangle               GetFreeArea( bool bAutoHide ) const;
    void                    SetDockingAllowed(bool bSet)
                            { bDockingAllowed = bSet; }
    void                    SetInternalDockingAllowed(bool bSet)
                            { bInternalDockingAllowed = bSet; }
    bool                    IsDockingAllowed() const
                            { return bDockingAllowed; }
    bool                    IsInternalDockingAllowed() const
                            { return bInternalDockingAllowed; }

    // Methods for all Child windows
    void                    DataChanged_Impl();
    void                    ReleaseChild_Impl( vcl::Window& rWindow );
    void                    ReleaseChild_Impl(SfxModelessDialogController&);
    SfxChild_Impl*          RegisterChild_Impl( vcl::Window& rWindow, SfxChildAlignment eAlign );
    SfxChild_Impl*          RegisterChild_Impl(std::shared_ptr<SfxModelessDialogController>& rController, SfxChildAlignment eAlign);
    void                    ShowChildren_Impl();
    void                    HideChildren_Impl();
    bool                    PrepareClose_Impl();
    void                    ArrangeChildren_Impl( bool bForce = true );
    void                    DeleteControllers_Impl();
    void                    HidePopups_Impl(bool bHide, sal_uInt16 nId=0);
    void                    ConfigChild_Impl(SfxChildIdentifier,
                                             SfxDockingConfig, sal_uInt16);
    void                    MakeChildrenVisible_Impl( bool bVis );
    void                    ArrangeAutoHideWindows( SfxSplitWindow *pSplit );
    bool                    IsAutoHideMode( const SfxSplitWindow *pSplit );
    void                    EndAutoShow_Impl( Point aPos );
    void                    SetFullScreen_Impl( bool bSet ) { bIsFullScreen = bSet; }

    // Methods for Objectbars
    void                    UpdateObjectBars_Impl();
    void                    UpdateObjectBars_Impl2();
    void                    ResetObjectBars_Impl();
    void                    SetObjectBar_Impl(sal_uInt16 nPos, SfxVisibilityFlags nFlags, ToolbarId eId);
    bool                    IsVisible_Impl();
    void                    MakeVisible_Impl( bool );
    void                    Lock_Impl( bool );

    // Methods for ChildWindows
    void                    UpdateChildWindows_Impl();
    void                    ResetChildWindows_Impl();
    void                    SetChildWindowVisible_Impl( sal_uInt32, bool, SfxVisibilityFlags );
    void                    ToggleChildWindow_Impl(sal_uInt16,bool);
    bool                    HasChildWindow_Impl(sal_uInt16);
    bool                    KnowsChildWindow_Impl(sal_uInt16);
    void                    ShowChildWindow_Impl(sal_uInt16, bool bVisible, bool bSetFocus);
    void                    SetChildWindow_Impl(sal_uInt16, bool bOn, bool bSetFocus);
    SfxChildWindow*         GetChildWindow_Impl(sal_uInt16);
    void                    InitializeChild_Impl(SfxChildWin_Impl*);
    SfxSplitWindow*         GetSplitWindow_Impl(SfxChildAlignment);

    bool                    IsVisible_Impl( SfxVisibilityFlags nMode ) const;
    bool                    IsFloating( sal_uInt16 nId );
    void                    SetActiveChild_Impl( vcl::Window *pChild );
    const VclPtr<vcl::Window>& GetActiveChild_Impl() const { return pActiveChild; }

    // Methods for StatusBar
    void                    ResetStatusBar_Impl();
    void                    SetStatusBar_Impl(StatusBarId eResId);
    void                    UpdateStatusBar_Impl();
    css::uno::Reference< css::task::XStatusIndicator > GetStatusIndicator();
    css::uno::Reference< css::frame::XFrame > GetFrameInterface();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
