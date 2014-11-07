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


#include <svl/eitem.hxx>
#include <vcl/decoview.hxx>

#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <rtl/instance.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>

#include <sfx2/dockwin.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include "workwin.hxx"
#include "splitwin.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/msgpool.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <com/sun/star/ui/theWindowContentFactoryManager.hpp>

#define MAX_TOGGLEAREA_WIDTH        20
#define MAX_TOGGLEAREA_HEIGHT       20

using namespace ::com::sun::star;

// If you want to change the number you also have to:
// - Add new slot ids to sfxsids.hrc
// - Add new slots to frmslots.sdi
// - Add new slot definitions to sfx.sdi
static const int NUM_OF_DOCKINGWINDOWS = 10;

class SfxTitleDockingWindow : public SfxDockingWindow
{
    vcl::Window*             m_pWrappedWindow;

public:
                        SfxTitleDockingWindow(
                            SfxBindings* pBindings ,
                            SfxChildWindow* pChildWin ,
                            vcl::Window* pParent ,
                            WinBits nBits);
    virtual             ~SfxTitleDockingWindow();

    vcl::Window*             GetWrappedWindow() const { return m_pWrappedWindow; }
    void                SetWrappedWindow(vcl::Window* const pWindow);

    virtual void        StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual bool        Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void        Resize() SAL_OVERRIDE;
    virtual void        Resizing( Size& rSize ) SAL_OVERRIDE;
    virtual bool        Close() SAL_OVERRIDE;
};

namespace
{
    struct WindowState
    {
        OUString sTitle;
    };
}

static bool lcl_getWindowState( const uno::Reference< container::XNameAccess >& xWindowStateMgr, const OUString& rResourceURL, WindowState& rWindowState )
{
    bool bResult = false;

    try
    {
        uno::Any a;
        uno::Sequence< beans::PropertyValue > aWindowState;
        a = xWindowStateMgr->getByName( rResourceURL );
        if ( a >>= aWindowState )
        {
            for ( sal_Int32 n = 0; n < aWindowState.getLength(); n++ )
            {
                if ( aWindowState[n].Name == "UIName" )
                {
                    aWindowState[n].Value >>= rWindowState.sTitle;
                }
            }
        }

        bResult = true;
    }
    catch ( container::NoSuchElementException& )
    {
        bResult = false;
    }

    return bResult;
}

SfxDockingWrapper::SfxDockingWrapper( vcl::Window* pParentWnd ,
                                      sal_uInt16 nId ,
                                      SfxBindings* pBindings ,
                                      SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParentWnd , nId )
{
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    const OUString aDockWindowResourceURL( "private:resource/dockingwindow/" );

    SfxTitleDockingWindow* pTitleDockWindow = new SfxTitleDockingWindow( pBindings, this, pParentWnd,
        WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE);
    pWindow = pTitleDockWindow;
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    // Use factory manager to retrieve XWindow factory. That can be used to instantiate
    // the real window factory.
    uno::Reference< lang::XSingleComponentFactory > xFactoryMgr = ui::theWindowContentFactoryManager::get(xContext);

    SfxDispatcher* pDispatcher = pBindings->GetDispatcher();
    uno::Reference< frame::XFrame > xFrame( pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
    uno::Sequence< uno::Any > aArgs(2);
    beans::PropertyValue      aPropValue;
    aPropValue.Name  = "Frame";
    aPropValue.Value = uno::makeAny( xFrame );
    aArgs[0] <<= aPropValue;
    aPropValue.Name  = "ResourceURL";

    // create a resource URL from the nId provided by the sfx2
    OUString aResourceURL( aDockWindowResourceURL );
    aResourceURL += OUString::number(nId);
    aPropValue.Value = uno::makeAny( aResourceURL );
    aArgs[1] <<= aPropValue;

    uno::Reference< awt::XWindow > xWindow;
    try
    {
        xWindow = uno::Reference< awt::XWindow>(
            xFactoryMgr->createInstanceWithArgumentsAndContext( aArgs, xContext ),
            uno::UNO_QUERY );

        static uno::WeakReference< frame::XModuleManager2 >  m_xModuleManager;

        uno::Reference< frame::XModuleManager2 > xModuleManager( m_xModuleManager );
        if ( !xModuleManager.is() )
        {
            xModuleManager = frame::ModuleManager::create(xContext);
            m_xModuleManager = xModuleManager;
        }

        static uno::WeakReference< container::XNameAccess > m_xWindowStateConfiguration;

        uno::Reference< container::XNameAccess > xWindowStateConfiguration( m_xWindowStateConfiguration );
        if ( !xWindowStateConfiguration.is() )
        {
            xWindowStateConfiguration = ui::theWindowStateConfiguration::get( xContext );
            m_xWindowStateConfiguration = xWindowStateConfiguration;
        }

        OUString sModuleIdentifier = xModuleManager->identify( xFrame );

        uno::Reference< container::XNameAccess > xModuleWindowState(
                                                    xWindowStateConfiguration->getByName( sModuleIdentifier ),
                                                    uno::UNO_QUERY );
        if ( xModuleWindowState.is() )
        {
            WindowState aDockWinState;
            if ( lcl_getWindowState( xModuleWindowState, aResourceURL, aDockWinState ))
                pTitleDockWindow->SetText( aDockWinState.sTitle );
        }
    }
    catch ( beans::UnknownPropertyException& )
    {
    }
    catch ( uno::RuntimeException& )
    {
    }
    catch ( uno::Exception& )
    {
    }

    vcl::Window* pContentWindow = VCLUnoHelper::GetWindow(xWindow);
    if ( pContentWindow )
        pContentWindow->SetStyle( pContentWindow->GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
    pTitleDockWindow->SetWrappedWindow(pContentWindow);

    pWindow->SetOutputSizePixel( Size( 270, 240 ) );

    static_cast<SfxDockingWindow*>( pWindow )->Initialize( pInfo );
    SetHideNotDelete( true );
}

SfxChildWindow*  SfxDockingWrapper::CreateImpl(
vcl::Window *pParent, sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo* pInfo )
{
    SfxChildWindow *pWin = new SfxDockingWrapper(pParent, nId, pBindings, pInfo);
    return pWin;
}

void SfxDockingWrapper::RegisterChildWindow (bool bVis, SfxModule *pMod, sal_uInt16 nFlags)
{
    // pre-register a couple of docking windows
    for (int i=0; i < NUM_OF_DOCKINGWINDOWS; i++ )
    {
        sal_uInt16 nID = sal_uInt16(SID_DOCKWIN_START+i);
        SfxChildWinFactory *pFact = new SfxChildWinFactory( SfxDockingWrapper::CreateImpl, nID, 0xffff );
        pFact->aInfo.nFlags |= nFlags;
        pFact->aInfo.bVisible = bVis;
        SfxChildWindow::RegisterChildWindow(pMod, pFact);
    }
}

SfxChildWinInfo  SfxDockingWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    static_cast<SfxDockingWindow*>(GetWindow())->FillInfo( aInfo );
    return aInfo;
};

SfxTitleDockingWindow::SfxTitleDockingWindow( SfxBindings* pBind ,
                                              SfxChildWindow* pChildWin ,
                                              vcl::Window* pParent ,
                                              WinBits nBits ) :
                          SfxDockingWindow( pBind ,
                                            pChildWin ,
                                            pParent ,
                                            nBits ),
                          m_pWrappedWindow(0)
{
}

SfxTitleDockingWindow::~SfxTitleDockingWindow()
{
    delete m_pWrappedWindow;
}

void SfxTitleDockingWindow::SetWrappedWindow( vcl::Window* const pWindow )
{
    m_pWrappedWindow = pWindow;
    if (m_pWrappedWindow)
    {
        m_pWrappedWindow->SetParent(this);
        m_pWrappedWindow->SetSizePixel( GetOutputSizePixel() );
        m_pWrappedWindow->Show();
    }
}

bool SfxTitleDockingWindow::Notify( NotifyEvent& rNEvt )
{
    return SfxDockingWindow::Notify( rNEvt );
}

void SfxTitleDockingWindow::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::INITSHOW )
    {
        vcl::Window* pWindow = GetWrappedWindow();
        if ( pWindow )
        {
            pWindow->SetSizePixel( GetOutputSizePixel() );
            pWindow->Show();
        }
    }

    SfxDockingWindow::StateChanged(nType);
}

void SfxTitleDockingWindow::Resize()
{
    SfxDockingWindow::Resize();
    if (m_pWrappedWindow)
        m_pWrappedWindow->SetSizePixel( GetOutputSizePixel() );
}

void SfxTitleDockingWindow::Resizing( Size &rSize )
{
    SfxDockingWindow::Resizing( rSize );
    if (m_pWrappedWindow)
        m_pWrappedWindow->SetSizePixel( GetOutputSizePixel() );
}

bool SfxTitleDockingWindow::Close()
{
    return SfxDockingWindow::Close();
}

namespace
{
    struct ChildrenRegisteredMap : public rtl::Static< bool, ChildrenRegisteredMap > {};
}

static bool lcl_checkDockingWindowID( sal_uInt16 nID )
{
    if (nID < SID_DOCKWIN_START || nID >= sal_uInt16(SID_DOCKWIN_START+NUM_OF_DOCKINGWINDOWS))
        return false;
    else
        return true;
}

static SfxWorkWindow* lcl_getWorkWindowFromXFrame( const uno::Reference< frame::XFrame >& rFrame )
{
    // We need to find the corresponding SfxFrame of our XFrame
    SfxFrame* pFrame  = SfxFrame::GetFirst();
    SfxFrame* pXFrame = 0;
    while ( pFrame )
    {
        uno::Reference< frame::XFrame > xViewShellFrame( pFrame->GetFrameInterface() );
        if ( xViewShellFrame == rFrame )
        {
            pXFrame = pFrame;
            break;
        }
        else
            pFrame = SfxFrame::GetNext( *pFrame );
    }

    // If we have a SfxFrame we can retrieve the work window (Sfx layout manager for docking windows)
    if ( pXFrame )
        return pXFrame->GetWorkWindow_Impl();
    else
        return NULL;
}

/*
    Factory function used by the framework layout manager to "create" a docking window with a special name.
    The string rDockingWindowName MUST BE a valid ID! The ID is pre-defined by a certain slot range located
    in sfxsids.hrc (currently SID_DOCKWIN_START = 9800).
*/
void SAL_CALL SfxDockingWindowFactory( const uno::Reference< frame::XFrame >& rFrame, const OUString& rDockingWindowName )
{
    SolarMutexGuard aGuard;
    sal_uInt16 nID = sal_uInt16(rDockingWindowName.toInt32());

    // Check the range of the provided ID otherwise nothing will happen
    if ( lcl_checkDockingWindowID( nID ))
    {
        SfxWorkWindow* pWorkWindow = lcl_getWorkWindowFromXFrame( rFrame );
        if ( pWorkWindow )
        {
            SfxChildWindow* pChildWindow = pWorkWindow->GetChildWindow_Impl(nID);
            if ( !pChildWindow )
            {
                // Register window at the workwindow child window list
                pWorkWindow->SetChildWindow_Impl( nID, true, false );
            }
        }
    }
}

/*
    Function used by the framework layout manager to determine the visibility state of a docking window with
    a special name. The string rDockingWindowName MUST BE a valid ID! The ID is pre-defined by a certain slot
    range located in sfxsids.hrc (currently SID_DOCKWIN_START = 9800).
*/
bool SAL_CALL IsDockingWindowVisible( const uno::Reference< frame::XFrame >& rFrame, const OUString& rDockingWindowName )
{
    SolarMutexGuard aGuard;

    sal_uInt16 nID = sal_uInt16(rDockingWindowName.toInt32());

    // Check the range of the provided ID otherwise nothing will happen
    if ( lcl_checkDockingWindowID( nID ))
    {
        SfxWorkWindow* pWorkWindow = lcl_getWorkWindowFromXFrame( rFrame );
        if ( pWorkWindow )
        {
            SfxChildWindow* pChildWindow = pWorkWindow->GetChildWindow_Impl(nID);
            if ( pChildWindow )
                return true;
        }
    }

    return false;
}

class SfxDockingWindow_Impl
{
friend class SfxDockingWindow;

    SfxChildAlignment   eLastAlignment;
    SfxChildAlignment   eDockAlignment;
    bool                bConstructed;
    Size                aMinSize;
    SfxSplitWindow*     pSplitWin;
    bool                bSplitable;
    Timer               aMoveTimer;

    // The following members are only valid in the time from startDocking to
    // EndDocking:
    bool                bEndDocked;
    Size                aSplitSize;
    long                nHorizontalSize;
    long                nVerticalSize;
    sal_uInt16              nLine;
    sal_uInt16              nPos;
    sal_uInt16              nDockLine;
    sal_uInt16              nDockPos;
    bool                bNewLine;
    bool                bDockingPrevented;
    OString aWinState;

    SfxChildAlignment   GetLastAlignment() const
                        { return eLastAlignment; }
    void                SetLastAlignment(SfxChildAlignment eAlign)
                        { eLastAlignment = eAlign; }
    SfxChildAlignment   GetDockAlignment() const
                        { return eDockAlignment; }
    void                SetDockAlignment(SfxChildAlignment eAlign)
                        { eDockAlignment = eAlign; }
};



void SfxDockingWindow::Resize()

/*  [Description]

    This virtual method of the class FloatingWindow keeps track of changes in
    FloatingSize. If this method is overridden by a derived class,
    then the SfxFloatingWindow: Resize() must also be called.
*/
{
    DockingWindow::Resize();
    Invalidate();
    if ( pImp->bConstructed && pMgr )
    {
        if ( IsFloatingMode() )
        {
            // start timer for saving window status information
            pImp->aMoveTimer.Start();
        }
        else
        {
            Size aSize( GetSizePixel() );
            switch ( pImp->GetDockAlignment() )
            {
                case SFX_ALIGN_LEFT:
                case SFX_ALIGN_FIRSTLEFT:
                case SFX_ALIGN_LASTLEFT:
                case SFX_ALIGN_RIGHT:
                case SFX_ALIGN_FIRSTRIGHT:
                case SFX_ALIGN_LASTRIGHT:
                    pImp->nHorizontalSize = aSize.Width();
                    pImp->aSplitSize = aSize;
                    break;
                case SFX_ALIGN_TOP:
                case SFX_ALIGN_LOWESTTOP:
                case SFX_ALIGN_HIGHESTTOP:
                case SFX_ALIGN_BOTTOM:
                case SFX_ALIGN_HIGHESTBOTTOM:
                case SFX_ALIGN_LOWESTBOTTOM:
                    pImp->nVerticalSize = aSize.Height();
                    pImp->aSplitSize = aSize;
                    break;
                default:
                    break;
            }
        }
    }
}



bool SfxDockingWindow::PrepareToggleFloatingMode()

/*  [Description]

    This virtual method of the class DockingWindow makes it possible to
    intervene in the switching of the floating mode.
    If this method is overridden by a derived class,
    then the SfxDockingWindow::PrepareToggleFloatingMode() must be called
    afterwards, if not FALSE is returned.
*/

{
    if (!pImp->bConstructed)
        return true;

    if ( (Application::IsInModalMode() && IsFloatingMode()) || !pMgr )
        return false;

    if ( pImp->bDockingPrevented )
        return false;

    if (!IsFloatingMode())
    {
        // Test, if FloatingMode is permitted.
        if ( CheckAlignment(GetAlignment(),SFX_ALIGN_NOALIGNMENT) != SFX_ALIGN_NOALIGNMENT )
            return false;

        if ( pImp->pSplitWin )
        {
            // The DockingWindow is inside a SplitWindow and will be teared of.
            pImp->pSplitWin->RemoveWindow(this/*, sal_False*/);
            pImp->pSplitWin = 0;
        }
    }
    else if ( pMgr )
    {
        pImp->aWinState = GetFloatingWindow()->GetWindowState();

        // Test if it is allowed to dock,
        if (CheckAlignment(GetAlignment(),pImp->GetLastAlignment()) == SFX_ALIGN_NOALIGNMENT)
            return false;

        // Test, if the Workwindow allows for docking at the moment.
        SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
        if ( !pWorkWin->IsDockingAllowed() || !pWorkWin->IsInternalDockingAllowed() )
            return false;
    }

    return true;
}



void SfxDockingWindow::ToggleFloatingMode()

/*  [Description]

    This virtual method of the DockingWindow class sets the internal data of
    the SfxDockingWindow and ensures the correct alignment on the parent window.
    Through PrepareToggleFloatMode and Initialize it is ensured that
    pImp-> GetLastAlignment() always delivers an allowed alignment. If this
    method is overloaded from a derived class, then first the
    SfxDockingWindow:: ToggleFloatingMode() must be called.
*/
{
    if ( !pImp->bConstructed || !pMgr )
        return;                                 // No Handler call

    // Remember old alignment and then switch.
    // SV has already switched, but the alignment SfxDockingWindow is still
    // the old one. What I was before?
    SfxChildAlignment eLastAlign = GetAlignment();

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
    if ( pImp->bSplitable )
        eIdent = SFX_CHILDWIN_SPLITWINDOW;

    if (IsFloatingMode())
    {
        SetAlignment(SFX_ALIGN_NOALIGNMENT);
        if ( !pImp->aWinState.isEmpty() )
            GetFloatingWindow()->SetWindowState( pImp->aWinState );
        else
            GetFloatingWindow()->SetOutputSizePixel( GetFloatingSize() );
    }
    else
    {
        if (pImp->GetDockAlignment() == eLastAlign)
        {
            // If ToggleFloatingMode was called, but the DockAlignment still
            // is unchanged, then this means that it must have been a toggling
            // through DClick, so use last alignment
            SetAlignment (pImp->GetLastAlignment());
            if ( !pImp->bSplitable )
                SetSizePixel( CalcDockingSize(GetAlignment()) );
        }
        else
        {

            // Toggling was triggered by dragging
            pImp->nLine = pImp->nDockLine;
            pImp->nPos = pImp->nDockPos;
            SetAlignment (pImp->GetDockAlignment());
        }

        if ( pImp->bSplitable )
        {
            // The DockingWindow is now in a SplitWindow
            pImp->pSplitWin = pWorkWin->GetSplitWindow_Impl(GetAlignment());

            // The LastAlignment is still the last docked
            SfxSplitWindow *pSplit = pWorkWin->GetSplitWindow_Impl(pImp->GetLastAlignment());

            DBG_ASSERT( pSplit, "LastAlignment is not correct!" );
            if ( pSplit && pSplit != pImp->pSplitWin )
                pSplit->ReleaseWindow_Impl(this);
            if ( pImp->GetDockAlignment() == eLastAlign )
                pImp->pSplitWin->InsertWindow( this, pImp->aSplitSize );
            else
                pImp->pSplitWin->InsertWindow( this, pImp->aSplitSize, pImp->nLine, pImp->nPos, pImp->bNewLine );
            if ( !pImp->pSplitWin->IsFadeIn() )
                pImp->pSplitWin->FadeIn();
        }
    }

    // Keep the old alignment for the next toggle; set it only now due to the
    // unregister SplitWindow!
    pImp->SetLastAlignment(eLastAlign);

    // Reset DockAlignment, if EndDocking is still called
    pImp->SetDockAlignment(GetAlignment());

    // Dock or undock SfxChildWindow correctly.
    pWorkWin->ConfigChild_Impl( eIdent, SFX_TOGGLEFLOATMODE, pMgr->GetType() );
}



void SfxDockingWindow::StartDocking()

/*  [Description]

    This virtual method of the DockingWindow class takes the inner and outer
    docking rectangle from the parent window. If this method is overloaded by a
    a derived class, then SfxDockingWindow:StartDocking() has to be called at
    the end.
*/
{
    if ( !pImp->bConstructed || !pMgr )
        return;
    SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
    if ( pImp->bSplitable )
        eIdent = SFX_CHILDWIN_SPLITWINDOW;
    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    pWorkWin->ConfigChild_Impl( eIdent, SFX_SETDOCKINGRECTS, pMgr->GetType() );
    pImp->SetDockAlignment(GetAlignment());

    if ( pImp->pSplitWin )
    {
        // Get the current docking data
        pImp->pSplitWin->GetWindowPos(this, pImp->nLine, pImp->nPos);
        pImp->nDockLine = pImp->nLine;
        pImp->nDockPos = pImp->nPos;
        pImp->bNewLine = false;
    }
}



bool SfxDockingWindow::Docking( const Point& rPos, Rectangle& rRect )

/*  [Description]

    This virtual method of the DockingWindow class calculates the current
    tracking rectangle. For this purpose the method CalcAlignment(RPOs, rRect)
    is used, the behavior can be influenced by the derived classes (see below).
    This method should if possible not be overwritten.
*/
{
    if ( Application::IsInModalMode() )
        return true;

    if ( !pImp->bConstructed || !pMgr )
    {
        rRect.SetSize( Size() );
        return IsFloatingMode();
    }

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    if ( pImp->bDockingPrevented || !pWorkWin->IsInternalDockingAllowed() )
        return false;

    bool bFloatMode = false;

    if ( GetOuterRect().IsInside( rPos ) && !IsDockingPrevented() )
    {
        // Mouse within OuterRect: calculate Alignment and Rectangle
        SfxChildAlignment eAlign = CalcAlignment(rPos, rRect);
        if (eAlign == SFX_ALIGN_NOALIGNMENT)
            bFloatMode = true;
        pImp->SetDockAlignment(eAlign);
    }
    else
    {
        // Mouse is not within OuterRect: must be FloatingWindow
        // Is this allowed?
        if (CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT) != SFX_ALIGN_NOALIGNMENT)
            return false;
        bFloatMode = true;
        if ( SFX_ALIGN_NOALIGNMENT != pImp->GetDockAlignment() )
        {
            // Due to a bug the rRect may only be changed when the
            // alignment is changed!
            pImp->SetDockAlignment(SFX_ALIGN_NOALIGNMENT);
            rRect.SetSize(CalcDockingSize(SFX_ALIGN_NOALIGNMENT));
        }
    }

    if ( !pImp->bSplitable )
    {
        // For individually docked window the position is set through the
        // alignment and the docking rectangle.
        Size aSize = rRect.GetSize();
        Point aPos;

        switch ( pImp->GetDockAlignment() )
        {
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
                aPos = aInnerRect.TopLeft();
                if ( pImp->GetDockAlignment() == GetAlignment() )
                    aPos.X() -= aSize.Width();
                break;

            case SFX_ALIGN_TOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_HIGHESTTOP:
                aPos = Point(aOuterRect.Left(), aInnerRect.Top());
                if ( pImp->GetDockAlignment() == GetAlignment() )
                    aPos.Y() -= aSize.Height();
                break;

            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                aPos = Point(aInnerRect.Right() - rRect.GetSize().Width(),
                            aInnerRect.Top());
                if ( pImp->GetDockAlignment() == GetAlignment() )
                    aPos.X() += aSize.Width();
                break;

            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
                aPos = Point(aOuterRect.Left(),
                        aInnerRect.Bottom() - rRect.GetSize().Height());
                if ( pImp->GetDockAlignment() == GetAlignment() )
                    aPos.Y() += aSize.Height();
                break;
                     default:
                         break;
        }

        rRect.SetPos(aPos);
    }

    return bFloatMode;
}



void SfxDockingWindow::EndDocking( const Rectangle& rRect, bool bFloatMode )

/*  [Description]

    Virtual method of the DockingWindow class ensures the correct alignment on
    the parent window. If this method is overloaded by a derived class, then
    SfxDockingWindow::EndDocking() must be called first.
*/
{
    if ( !pImp->bConstructed || IsDockingCanceled() || !pMgr )
        return;

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    bool bReArrange = false;
    if ( pImp->bSplitable )
    {
        // If the alignment changes and the window is in a docked state in a
        // SplitWindow, then it must be re-registered. If it is docked again,
        // PrepareToggleFloatingMode() and ToggleFloatingMode() preform the
        // re-registered
        if ( !bFloatMode )
            bReArrange = true;
    }

    if ( bReArrange )
    {
        if ( GetAlignment() != pImp->GetDockAlignment() )
        {
            // before Show() is called must the reassignment have been made,
            // therefore the base class can not be called
            if ( IsFloatingMode() || !pImp->bSplitable )
                Show( false, SHOW_NOFOCUSCHANGE );

            // Set the size for toggling.
            pImp->aSplitSize = rRect.GetSize();
            if ( IsFloatingMode() )
            {
                SetFloatingMode( bFloatMode );
                if ( IsFloatingMode() || !pImp->bSplitable )
                    Show( true, SHOW_NOFOCUSCHANGE );
            }
            else
            {
                pImp->pSplitWin->RemoveWindow(this,false);
                pImp->nLine = pImp->nDockLine;
                pImp->nPos = pImp->nDockPos;
                pImp->pSplitWin->ReleaseWindow_Impl(this);
                pImp->pSplitWin = pWorkWin->GetSplitWindow_Impl(pImp->GetDockAlignment());
                pImp->pSplitWin->InsertWindow( this, pImp->aSplitSize, pImp->nDockLine, pImp->nDockPos, pImp->bNewLine );
                if ( !pImp->pSplitWin->IsFadeIn() )
                    pImp->pSplitWin->FadeIn();
            }
        }
        else if ( pImp->nLine != pImp->nDockLine || pImp->nPos != pImp->nDockPos || pImp->bNewLine )
        {
            // Moved within Splitwindows
            if ( pImp->nLine != pImp->nDockLine )
                pImp->aSplitSize = rRect.GetSize();
            pImp->pSplitWin->MoveWindow( this, pImp->aSplitSize, pImp->nDockLine, pImp->nDockPos, pImp->bNewLine );
        }
    }
    else
    {
        pImp->bEndDocked = true;
        DockingWindow::EndDocking(rRect, bFloatMode);
        pImp->bEndDocked = false;
    }

    SetAlignment( IsFloatingMode() ? SFX_ALIGN_NOALIGNMENT : pImp->GetDockAlignment() );
}



void SfxDockingWindow::Resizing( Size& /*rSize*/ )

/*  [Description]

    Virtual method of the DockingWindow class. Here, the interactive resize in
    FloatingMode can be influenced, for example by only allowing for discrete
    values for width and / or height. The base implementation prevents that the
    output size is smaller than one set with SetMinOutputSizePixel().
*/

{

}



SfxDockingWindow::SfxDockingWindow( SfxBindings *pBindinx, SfxChildWindow *pCW,
    vcl::Window* pParent, WinBits nWinBits) :
    DockingWindow (pParent, nWinBits),
    pBindings(pBindinx),
    pMgr(pCW),
    pImp(NULL)

/*  [Description]

    Constructor for the SfxDockingWindow class. A SfxChildWindow will be
    required because the docking is implemented in Sfx through SfxChildWindows.
*/

{
    if ( !GetHelpId().isEmpty() )
    {
        SetUniqueId( GetHelpId() );
        SetHelpId("");
    }
    else
    {
        SfxViewFrame* pViewFrame = pBindings->GetDispatcher()->GetFrame();
        SfxSlotPool* pSlotPool = pViewFrame->GetObjectShell()->GetModule()->GetSlotPool();
        const SfxSlot* pSlot = pSlotPool->GetSlot( pCW->GetType() );
        if ( pSlot )
        {
            OString aCmd("SFXDOCKINGWINDOW_");
            aCmd += pSlot->GetUnoName();
            SetUniqueId( aCmd );
        }
    }

    pImp = new SfxDockingWindow_Impl;
    pImp->bConstructed = false;
    pImp->pSplitWin = 0;
    pImp->bEndDocked = false;
    pImp->bDockingPrevented = false;

    pImp->bSplitable = true;

    // Initially set to default, the alignment is set in the subclass
    pImp->nLine = pImp->nDockLine = 0;
    pImp->nPos  = pImp->nDockPos = 0;
    pImp->bNewLine = false;
    pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxDockingWindow,TimerHdl));
}



SfxDockingWindow::SfxDockingWindow( SfxBindings *pBindinx, SfxChildWindow *pCW,
    vcl::Window* pParent, const ResId& rResId) :
    DockingWindow(pParent, rResId),
    pBindings(pBindinx),
    pMgr(pCW),
    pImp(NULL)

/*  [Description]

    Constructor for the SfxDockingWindow class. A SfxChildWindow will be
    required because the docking is implemented in Sfx through SfxChildWindows.
*/

{
    if ( !GetHelpId().isEmpty() )
    {
        SetUniqueId( GetHelpId() );
        SetHelpId("");
    }
    else
    {
        SfxViewFrame* pViewFrame = pBindings->GetDispatcher()->GetFrame();
        SfxSlotPool* pSlotPool = pViewFrame->GetObjectShell()->GetModule()->GetSlotPool();
        const SfxSlot* pSlot = pSlotPool->GetSlot( pCW->GetType() );
        if ( pSlot )
        {
            OString aCmd("SFXDOCKINGWINDOW_");
            aCmd += pSlot->GetUnoName();
            SetUniqueId( aCmd );
        }
    }

    pImp = new SfxDockingWindow_Impl;
    pImp->bConstructed = false;
    pImp->pSplitWin = 0;
    pImp->bEndDocked = false;
    pImp->bDockingPrevented = false;

    pImp->bSplitable = true;

    // Initially set to default, the alignment is set in the subclass
    pImp->nLine = pImp->nDockLine = 0;
    pImp->nPos  = pImp->nDockPos = 0;
    pImp->bNewLine = false;
    pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxDockingWindow,TimerHdl));
}



void SfxDockingWindow::Initialize(SfxChildWinInfo *pInfo)
/*  [Description]

    Initialization of the SfxDockingDialog class via a SfxChildWinInfo.
    The initialization is done only in a 2nd step after the constructor, this
    constructor should be called from the derived class or from the
    SfxChildWindows.
*/
{
    if ( !pMgr )
    {
        pImp->SetDockAlignment( SFX_ALIGN_NOALIGNMENT );
        pImp->bConstructed = true;
        return;
    }

    if (pInfo && (pInfo->nFlags & SFX_CHILDWIN_FORCEDOCK))
        pImp->bDockingPrevented = true;

    pImp->aSplitSize = GetOutputSizePixel();
    if ( !GetFloatingSize().Width() )
    {
        Size aMinSize( GetMinOutputSizePixel() );
        SetFloatingSize( pImp->aSplitSize );
        if ( pImp->aSplitSize.Width() < aMinSize.Width() )
            pImp->aSplitSize.Width() = aMinSize.Width();
        if ( pImp->aSplitSize.Height() < aMinSize.Height() )
            pImp->aSplitSize.Height() = aMinSize.Height();
    }

    bool bVertHorzRead( false );
    if (pInfo && !pInfo->aExtraString.isEmpty())
    {
        // get information about alignment, split size and position in SplitWindow
        OUString aStr;
        sal_Int32 nPos = pInfo->aExtraString.indexOf("AL:");
        if ( nPos != -1 )
        {
            // alignment information
            sal_Int32 n1 = pInfo->aExtraString.indexOf('(', nPos);
            if ( n1 != -1 )
            {
                sal_Int32 n2 = pInfo->aExtraString.indexOf(')', n1);
                if ( n2 != -1 )
                {
                    // extract alignment information from extrastring
                    aStr = pInfo->aExtraString.copy(nPos, n2 - nPos + 1);
                    pInfo->aExtraString = pInfo->aExtraString.replaceAt(nPos, n2 - nPos + 1, "");
                    aStr = aStr.replaceAt(nPos, n1-nPos+1, "");
                }
            }
        }

        if ( !aStr.isEmpty() )
        {
            // accept window state only if alignment is also set
            pImp->aWinState = pInfo->aWinState;

            // check for valid alignment
            SfxChildAlignment eLocalAlignment = (SfxChildAlignment) (sal_uInt16) aStr.toInt32();
            if ( pImp->bDockingPrevented )
                // docking prevented, ignore old configuration and take alignment from default
                aStr = "";
            else
                SetAlignment( eLocalAlignment );

            SfxChildAlignment eAlign = CheckAlignment(GetAlignment(),GetAlignment());
            if ( eAlign != GetAlignment() )
            {
                OSL_FAIL("Invalid Alignment!");
                SetAlignment( eAlign );
                aStr = "";
            }

            // get last alignment (for toggeling)
            nPos = aStr.indexOf(',');
            if ( nPos != -1 )
            {
                aStr = aStr.copy(nPos+1);
                pImp->SetLastAlignment( (SfxChildAlignment) (sal_uInt16) aStr.toInt32() );
            }

            nPos = aStr.indexOf(',');
            if ( nPos != -1 )
            {
                // get split size and position in SplitWindow
                Point aPos;
                aStr = aStr.copy(nPos+1);
                if ( GetPosSizeFromString( aStr, aPos, pImp->aSplitSize ) )
                {
                    pImp->nLine = pImp->nDockLine = (sal_uInt16) aPos.X();
                    pImp->nPos  = pImp->nDockPos  = (sal_uInt16) aPos.Y();
                    pImp->nVerticalSize = pImp->aSplitSize.Height();
                    pImp->nHorizontalSize = pImp->aSplitSize.Width();
                    if ( GetSplitSizeFromString( aStr, pImp->aSplitSize ))
                        bVertHorzRead = true;
                }
            }
        }
        else {
            OSL_FAIL( "Information is missing!" );
        }
    }

    if ( !bVertHorzRead )
    {
        pImp->nVerticalSize = pImp->aSplitSize.Height();
        pImp->nHorizontalSize = pImp->aSplitSize.Width();
    }

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    if ( GetAlignment() != SFX_ALIGN_NOALIGNMENT )
    {
        // check if SfxWorkWindow is able to allow docking at its border
        if (
            !pWorkWin->IsDockingAllowed() ||
            !pWorkWin->IsInternalDockingAllowed() ||
            ( (GetFloatStyle() & WB_STANDALONE) && Application::IsInModalMode()) )
        {
            SetAlignment( SFX_ALIGN_NOALIGNMENT );
        }
    }

    // detect floating mode
    // toggeling mode will not execute code in handlers, because pImp->bConstructed is not set yet
    bool bFloatMode = IsFloatingMode();
    if ( bFloatMode != ((GetAlignment() == SFX_ALIGN_NOALIGNMENT)) )
    {
        bFloatMode = !bFloatMode;
        SetFloatingMode( bFloatMode );
        if ( bFloatMode )
        {
            if ( !pImp->aWinState.isEmpty() )
                GetFloatingWindow()->SetWindowState( pImp->aWinState );
            else
                GetFloatingWindow()->SetOutputSizePixel( GetFloatingSize() );
        }
    }

    if ( IsFloatingMode() )
    {
        // validate last alignment
        SfxChildAlignment eLastAlign = pImp->GetLastAlignment();
        if ( eLastAlign == SFX_ALIGN_NOALIGNMENT)
            eLastAlign = CheckAlignment(eLastAlign, SFX_ALIGN_LEFT);
        if ( eLastAlign == SFX_ALIGN_NOALIGNMENT)
            eLastAlign = CheckAlignment(eLastAlign, SFX_ALIGN_RIGHT);
        if ( eLastAlign == SFX_ALIGN_NOALIGNMENT)
            eLastAlign = CheckAlignment(eLastAlign, SFX_ALIGN_TOP);
        if ( eLastAlign == SFX_ALIGN_NOALIGNMENT)
            eLastAlign = CheckAlignment(eLastAlign, SFX_ALIGN_BOTTOM);
        pImp->SetLastAlignment(eLastAlign);
    }
    else
    {
        // docked window must have NOALIGNMENT as last alignment
        pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);

        if ( pImp->bSplitable )
        {
            pImp->pSplitWin = pWorkWin->GetSplitWindow_Impl(GetAlignment());
            pImp->pSplitWin->InsertWindow(this, pImp->aSplitSize);
        }
        else
        {
            //?????? Currently not supported
            // Window is docked individually; size is calculated.
            // It must therefore be initialized with the DloatingSize if
            // someone relies on it that a reasonable size is set
            SetSizePixel(GetFloatingSize());
            SetSizePixel(CalcDockingSize(GetAlignment()));
        }
    }

    // save alignment
    pImp->SetDockAlignment( GetAlignment() );
}

void SfxDockingWindow::Initialize_Impl()
{
    if ( !pMgr )
    {
        pImp->bConstructed = true;
        return;
    }

    FloatingWindow* pFloatWin = GetFloatingWindow();
    bool bSet = false;
    if ( pFloatWin )
    {
        bSet = !pFloatWin->IsDefaultPos();
    }
    else
    {
        Point aPos = GetFloatingPos();
        if ( aPos != Point() )
            bSet = true;
    }

    if ( !bSet)
    {
        SfxViewFrame *pFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        vcl::Window* pEditWin = pFrame->GetViewShell()->GetWindow();
        Point aPos = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPos = GetParent()->ScreenToOutputPixel( aPos );
        SetFloatingPos( aPos );
    }

    if ( pFloatWin )
    {
        // initialize floating window
        if ( pImp->aWinState.isEmpty() )
            // window state never set before, get if from defaults
            pImp->aWinState = pFloatWin->GetWindowState();

        // trick: use VCL method SetWindowState to adjust position and size
        pFloatWin->SetWindowState( pImp->aWinState );

        // remember floating size for calculating alignment and tracking rectangle
        SetFloatingSize( pFloatWin->GetSizePixel() );

    }

    // allow calling of docking handlers
    pImp->bConstructed = true;
}



void SfxDockingWindow::FillInfo(SfxChildWinInfo& rInfo) const

/*  [Description]

    Fills a SfxChildWinInfo with specific data from SfxDockingWindow,
    so that it can be written in the INI file. It is assumed that rinfo
    receives all other possible relevant data in the ChildWindow class.
    Insertions are marked with size and the ZoomIn flag.
    If this method is overridden, the base implementation must be called first.
*/

{
    if ( !pMgr )
        return;

    if ( GetFloatingWindow() && pImp->bConstructed )
        pImp->aWinState = GetFloatingWindow()->GetWindowState();

    rInfo.aWinState = pImp->aWinState;
    rInfo.aExtraString = "AL:(";
    rInfo.aExtraString += OUString::number((sal_uInt16) GetAlignment());
    rInfo.aExtraString += ",";
    rInfo.aExtraString += OUString::number ((sal_uInt16) pImp->GetLastAlignment());
    if ( pImp->bSplitable )
    {
        Point aPos(pImp->nLine, pImp->nPos);
        rInfo.aExtraString += ",";
        rInfo.aExtraString += OUString::number( aPos.X() );
        rInfo.aExtraString += "/";
        rInfo.aExtraString += OUString::number( aPos.Y() );
        rInfo.aExtraString += "/";
        rInfo.aExtraString += OUString::number( pImp->nHorizontalSize );
        rInfo.aExtraString += "/";
        rInfo.aExtraString += OUString::number( pImp->nVerticalSize );
        rInfo.aExtraString += ",";
        rInfo.aExtraString += OUString::number( pImp->aSplitSize.Width() );
        rInfo.aExtraString += ";";
        rInfo.aExtraString += OUString::number( pImp->aSplitSize.Height() );
    }

    rInfo.aExtraString += ")";
}



SfxDockingWindow::~SfxDockingWindow()
{
    ReleaseChildWindow_Impl();
    delete pImp;
}

void SfxDockingWindow::ReleaseChildWindow_Impl()
{
    if ( pMgr && pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( NULL );

    if ( pMgr && pImp->pSplitWin && pImp->pSplitWin->IsItemValid( GetType() ) )
        pImp->pSplitWin->RemoveWindow(this);

    pMgr=NULL;
}



SfxChildAlignment SfxDockingWindow::CalcAlignment(const Point& rPos, Rectangle& rRect)

/*  [Description]

    This method calculates a resulting alignment for the given mouse position
    and tracking rectangle. When changing the alignment it can also be that
    the tracking rectangle is changed, so that an altered rectangle is
    returned. The user of this class can influence behaviour of this method,
    and thus the behavior of his DockinWindow class when docking where the
    called virtual method:

    SfxDockingWindow :: CalcDockingSize (SfxChildAlignment eAlign)

    is overridden (see below).
*/

{
    // calculate hypothetical sizes for different modes
    Size aFloatingSize(CalcDockingSize(SFX_ALIGN_NOALIGNMENT));
    Size aVerticalSize(CalcDockingSize(SFX_ALIGN_LEFT));
    Size aHorizontalSize(CalcDockingSize(SFX_ALIGN_TOP));

    // check if docking is permitted
    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    if ( !pWorkWin->IsDockingAllowed() )
    {
        rRect.SetSize( aFloatingSize );
        return pImp->GetDockAlignment();
    }

    // calculate borders to shrink inner area before checking for intersection with tracking rectangle
    long nLRBorder, nTBBorder;
    if ( pImp->bSplitable )
    {
        // take the smaller size of docked and floating mode
        Size aSize = pImp->aSplitSize;
        if ( GetFloatingSize().Height() < aSize.Height() )
            aSize.Height() = GetFloatingSize().Height();
        if ( GetFloatingSize().Width() < aSize.Width() )
            aSize.Width() = GetFloatingSize().Width();

        nLRBorder = aSize.Width();
        nTBBorder = aSize.Height();
    }
    else
    {
        nLRBorder = aVerticalSize.Width();
        nTBBorder = aHorizontalSize.Height();
    }

    // limit border to predefined constant values
    if ( nLRBorder > MAX_TOGGLEAREA_WIDTH )
        nLRBorder = MAX_TOGGLEAREA_WIDTH;
    if ( nTBBorder > MAX_TOGGLEAREA_WIDTH )
        nTBBorder = MAX_TOGGLEAREA_WIDTH;

    // shrink area for floating mode if possible
    Rectangle aInRect = GetInnerRect();
    if ( aInRect.GetWidth() > nLRBorder )
        aInRect.Left()   += nLRBorder/2;
    if ( aInRect.GetWidth() > nLRBorder )
        aInRect.Right()  -= nLRBorder/2;
    if ( aInRect.GetHeight() > nTBBorder )
        aInRect.Top()    += nTBBorder/2;
    if ( aInRect.GetHeight() > nTBBorder )
        aInRect.Bottom() -= nTBBorder/2;

    // calculate alignment resulting from docking rectangle
    bool bBecomesFloating = false;
    SfxChildAlignment eDockAlign = pImp->GetDockAlignment();
    Rectangle aDockingRect( rRect );
    if ( !IsFloatingMode() )
    {
        // don't use tracking rectangle for alignment check, because it will be too large
        // to get a floating mode as result - switch to floating size
        // so the calculation only depends on the position of the rectangle, not the current
        // docking state of the window
        aDockingRect.SetSize( GetFloatingSize() );

        // in this mode docking is never done by keyboard, so it's OK to use the mouse position
        aDockingRect.SetPos( pWorkWin->GetWindow()->OutputToScreenPixel( pWorkWin->GetWindow()->GetPointerPosPixel() ) );
    }

    Point aPos = aDockingRect.TopLeft();
    Rectangle aIntersect = GetOuterRect().GetIntersection( aDockingRect );
    if ( aIntersect.IsEmpty() )
        // docking rectangle completely outside docking area -> floating mode
        bBecomesFloating = true;
    else
    {
        // create a small test rect around the mouse position and use this one
        // instead of the passed rRect to not dock too easily or by accident
        Rectangle aSmallDockingRect;
        aSmallDockingRect.SetSize( Size( MAX_TOGGLEAREA_WIDTH, MAX_TOGGLEAREA_HEIGHT ) );
        Point aNewPos(rPos);
        aNewPos.X() -= aSmallDockingRect.GetWidth()/2;
        aNewPos.Y() -= aSmallDockingRect.GetHeight()/2;
        aSmallDockingRect.SetPos(rPos);
        Rectangle aIntersectRect = aInRect.GetIntersection( aSmallDockingRect );
        if ( aIntersectRect == aSmallDockingRect )
            // docking rectangle completely inside (shrunk) inner area -> floating mode
            bBecomesFloating = true;
    }

    if ( bBecomesFloating )
    {
        eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT);
    }
    else
    {
        // docking rectangle is in the "sensible area"
        Point aInPosTL( aPos.X()-aInRect.Left(), aPos.Y()-aInRect.Top() );
        Point aInPosBR( aPos.X()-aInRect.Left() + aDockingRect.GetWidth(), aPos.Y()-aInRect.Top() + aDockingRect.GetHeight() );
        Size  aInSize = aInRect.GetSize();
        bool  bNoChange = false;

        // check if alignment is still unchanged
        switch ( GetAlignment() )
        {
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
                if (aInPosTL.X() <= 0)
                {
                    eDockAlign = GetAlignment();
                    bNoChange = true;
                }
                break;
            case SFX_ALIGN_TOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_HIGHESTTOP:
                if ( aInPosTL.Y() <= 0)
                {
                    eDockAlign = GetAlignment();
                    bNoChange = true;
                }
                break;
            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                if ( aInPosBR.X() >= aInSize.Width())
                {
                    eDockAlign = GetAlignment();
                    bNoChange = true;
                }
                break;
            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
                if ( aInPosBR.Y() >= aInSize.Height())
                {
                    eDockAlign = GetAlignment();
                    bNoChange = true;
                }
                break;
            default:
                break;
        }

        if ( !bNoChange )
        {
            // alignment will change, test alignment according to distance of the docking rectangles edges
            bool bForbidden = true;
            if ( aInPosTL.X() <= 0)
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_LEFT);
                bForbidden = ( eDockAlign != SFX_ALIGN_LEFT &&
                               eDockAlign != SFX_ALIGN_FIRSTLEFT &&
                               eDockAlign != SFX_ALIGN_LASTLEFT );
            }

            if ( bForbidden && aInPosTL.Y() <= 0)
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_TOP);
                bForbidden = ( eDockAlign != SFX_ALIGN_TOP &&
                               eDockAlign != SFX_ALIGN_HIGHESTTOP &&
                               eDockAlign != SFX_ALIGN_LOWESTTOP );
            }

            if ( bForbidden && aInPosBR.X() >= aInSize.Width())
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_RIGHT);
                bForbidden = ( eDockAlign != SFX_ALIGN_RIGHT &&
                               eDockAlign != SFX_ALIGN_FIRSTRIGHT &&
                               eDockAlign != SFX_ALIGN_LASTRIGHT );
            }

            if ( bForbidden && aInPosBR.Y() >= aInSize.Height())
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_BOTTOM);
                bForbidden = ( eDockAlign != SFX_ALIGN_BOTTOM &&
                               eDockAlign != SFX_ALIGN_HIGHESTBOTTOM &&
                               eDockAlign != SFX_ALIGN_LOWESTBOTTOM );
            }

            // the calculated alignment was rejected by the window -> take floating mode
            if ( bForbidden )
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT);
        }
    }

    if ( eDockAlign == SFX_ALIGN_NOALIGNMENT )
    {
        // In the FloatingMode the tracking rectangle will get the floating
        // size. Due to a bug the rRect may only be changed when the
        // alignment is changed!
        if ( eDockAlign != pImp->GetDockAlignment() )
            aDockingRect.SetSize( aFloatingSize );
    }
    else if ( pImp->bSplitable )
    {
        sal_uInt16 nLine, nPos;
        SfxSplitWindow *pSplitWin = pWorkWin->GetSplitWindow_Impl(eDockAlign);
        aPos = pSplitWin->ScreenToOutputPixel( aPos );
        if ( pSplitWin->GetWindowPos( aPos, nLine, nPos ) )
        {
            // mouse over splitwindow, get line and position
            pImp->nDockLine = nLine;
            pImp->nDockPos = nPos;
            pImp->bNewLine = false;
        }
        else
        {
            // mouse touches inner border -> create new line
            if ( eDockAlign == GetAlignment() && pImp->pSplitWin &&
                 pImp->nLine == pImp->pSplitWin->GetLineCount()-1 && pImp->pSplitWin->GetWindowCount(pImp->nLine) == 1 )
            {
                // if this window is the only one in the last line, it can't be docked as new line in the same splitwindow
                pImp->nDockLine = pImp->nLine;
                pImp->nDockPos = pImp->nPos;
                pImp->bNewLine = false;
            }
            else
            {
                // create new line
                pImp->nDockLine = pSplitWin->GetLineCount();
                pImp->nDockPos = 0;
                pImp->bNewLine = true;
            }
        }

        bool bChanged = pImp->nLine != pImp->nDockLine || pImp->nPos != pImp->nDockPos || eDockAlign != GetAlignment();
        if ( !bChanged && !IsFloatingMode() )
        {
            // window only sightly moved, no change of any property
            rRect.SetSize( pImp->aSplitSize );
            rRect.SetPos( aDockingRect.TopLeft() );
            return eDockAlign;
        }

        // calculate new size and position
        Size aSize;
        Point aPoint = aDockingRect.TopLeft();
        Size aInnerSize = GetInnerRect().GetSize();
        if ( eDockAlign == SFX_ALIGN_LEFT || eDockAlign == SFX_ALIGN_RIGHT )
        {
            if ( pImp->bNewLine )
            {
                // set height to height of free area
                aSize.Height() = aInnerSize.Height();
                aSize.Width() = pImp->nHorizontalSize;
                if ( eDockAlign == SFX_ALIGN_LEFT )
                {
                    aPoint = aInnerRect.TopLeft();
                }
                else
                {
                    aPoint = aInnerRect.TopRight();
                    aPoint.X() -= aSize.Width();
                }
            }
            else
            {
                // get width from splitwindow
                aSize.Width() = pSplitWin->GetLineSize(nLine);
                aSize.Height() = pImp->aSplitSize.Height();
            }
        }
        else
        {
            if ( pImp->bNewLine )
            {
                // set width to width of free area
                aSize.Width() = aInnerSize.Width();
                aSize.Height() = pImp->nVerticalSize;
                if ( eDockAlign == SFX_ALIGN_TOP )
                {
                    aPoint = aInnerRect.TopLeft();
                }
                else
                {
                    aPoint = aInnerRect.BottomLeft();
                    aPoint.Y() -= aSize.Height();
                }
            }
            else
            {
                // get height from splitwindow
                aSize.Height() = pSplitWin->GetLineSize(nLine);
                aSize.Width() = pImp->aSplitSize.Width();
            }
        }

        aDockingRect.SetSize( aSize );
        aDockingRect.SetPos( aPoint );
    }
    else
    {
        // window can be docked, but outside our splitwindows
        // tracking rectangle only needs to be modified if alignment was changed
        if ( eDockAlign != pImp->GetDockAlignment() )
        {
            switch ( eDockAlign )
            {
                case SFX_ALIGN_LEFT:
                case SFX_ALIGN_RIGHT:
                case SFX_ALIGN_FIRSTLEFT:
                    aDockingRect.SetPos( aInnerRect.TopLeft() );
                    aDockingRect.SetSize( aVerticalSize );
                    break;
                case SFX_ALIGN_LASTLEFT:
                case SFX_ALIGN_FIRSTRIGHT:
                case SFX_ALIGN_LASTRIGHT:
                {
                    Point aPt( aInnerRect.TopRight() );
                    aPt.X() -= aDockingRect.GetWidth();
                    aDockingRect.SetPos( aPt );
                    aDockingRect.SetSize( aVerticalSize );
                    break;
                }

                case SFX_ALIGN_TOP:
                case SFX_ALIGN_BOTTOM:
                case SFX_ALIGN_LOWESTTOP:
                    aDockingRect.SetPos( aInnerRect.TopLeft() );
                    aDockingRect.SetSize( aHorizontalSize );
                    break;
                case SFX_ALIGN_HIGHESTTOP:
                case SFX_ALIGN_LOWESTBOTTOM:
                case SFX_ALIGN_HIGHESTBOTTOM:
                {
                    Point aPt( aInnerRect.BottomLeft() );
                    aPt.Y() -= aDockingRect.GetHeight();
                    aDockingRect.SetPos( aPt );
                    aDockingRect.SetSize( aHorizontalSize );
                    break;
                }
                        default:
                            break;
            }
        }
    }

    rRect = aDockingRect;
    return eDockAlign;
}



Size SfxDockingWindow::CalcDockingSize(SfxChildAlignment eAlign)

/*  [Description]

    Virtual method of the SfxDockingWindow class. This method determines how
    the size of the DockingWindows changes depending on the alignment. The base
    implementation uses the floating mode, the size of the marked Floating
    Size. For horizontal alignment, the width will be the width of the outer
    DockingRectangle, with vertical alignment the height will be the height of
    the inner DockingRectangle (resulting from the order in which the SFX child
    windows are displayed). The other size is set to the current floating-size,
    this could changed by a to intervening derived class. The docking size must
    be the same for Left/Right and Top/Bottom.
*/

{
    // Note: if the resizing is also possible in the docked state, then the
    // Floating-size does also have to be adjusted?

    Size aSize = GetFloatingSize();
    switch (eAlign)
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_HIGHESTBOTTOM:
            aSize.Width() = aOuterRect.Right() - aOuterRect.Left();
            break;
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_FIRSTLEFT:
        case SFX_ALIGN_LASTLEFT:
        case SFX_ALIGN_FIRSTRIGHT:
        case SFX_ALIGN_LASTRIGHT:
            aSize.Height() = aInnerRect.Bottom() - aInnerRect.Top();
            break;
        case SFX_ALIGN_NOALIGNMENT:
            break;
              default:
                  break;
    }

    return aSize;
}



SfxChildAlignment SfxDockingWindow::CheckAlignment(SfxChildAlignment,
    SfxChildAlignment eAlign)

/*  [Description]

    Virtual method of the SfxDockingWindow class. Here a derived class can
    disallow certain alignments. The base implementation does not
    prohibit alignment.
*/

{
    return eAlign;
}



bool SfxDockingWindow::Close()

/*  [Description]

    The window is closed when the ChildWindow is destroyed by running the
    ChildWindow-slots. If this is method is overridden by a derived class
    method, then the SfxDockingDialogWindow: Close() must be called afterwards
    if the Close() was not cancelled with "return sal_False".
*/
{
    // Execute with Parameters, since Toggle is ignored by some ChildWindows.
    if ( !pMgr )
        return true;

    SfxBoolItem aValue( pMgr->GetType(), false);
    pBindings->GetDispatcher_Impl()->Execute(
        pMgr->GetType(), SfxCallMode::RECORD | SfxCallMode::ASYNCHRON, &aValue, 0L );
    return true;
}



void SfxDockingWindow::Paint(const Rectangle& /*rRect*/)

/*  [Description]

    Returns a boundary line to the docked edge and a frame when the Window is in
    a docked state. In this way SVLOOK is considered.
*/

{
    if ( pImp->bSplitable || IsFloatingMode() )
        return;

    Rectangle aRect = Rectangle(Point(0, 0),
                                GetOutputSizePixel());
    switch (GetAlignment())
    {
        case SFX_ALIGN_TOP:
        {
            DrawLine(aRect.BottomLeft(), aRect.BottomRight());
            aRect.Bottom()--;
            break;
        }

        case SFX_ALIGN_BOTTOM:
        {
            DrawLine(aRect.TopLeft(), aRect.TopRight());
            aRect.Top()++;
            break;
        }

        case SFX_ALIGN_LEFT:
        {
            DrawLine(aRect.TopRight(), aRect.BottomRight());
            aRect.Right()--;
            break;
        }

        case SFX_ALIGN_RIGHT:
        {
            DrawLine(aRect.TopLeft(), aRect.BottomLeft());
            aRect.Left()++;
            break;
        }
              default:
                  break;
    }

    DecorationView aView( this );
    aView.DrawFrame( aRect, FRAME_DRAW_OUT );
}



void SfxDockingWindow::SetMinOutputSizePixel( const Size& rSize )

/*  [Description]

    With this method, a minimal OutputSize be can set, that is queried in
    the Resizing()-Handler.
*/

{
    pImp->aMinSize = rSize;
    DockingWindow::SetMinOutputSizePixel( rSize );
}



Size SfxDockingWindow::GetMinOutputSizePixel() const

/*  [Description]

    Set the minimum size which is returned.
*/

{
    return pImp->aMinSize;
}



bool SfxDockingWindow::Notify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == EVENT_GETFOCUS )
    {
        if (pMgr != NULL)
            pBindings->SetActiveFrame( pMgr->GetFrame() );

        if ( pImp->pSplitWin )
            pImp->pSplitWin->SetActiveWindow_Impl( this );
        else if (pMgr != NULL)
            pMgr->Activate_Impl();

        // In VCL Notify goes first to the window itself, also call the
        // base class, otherwise the parent learns nothing
        // if ( rEvt.GetWindow() == this )  PB: #i74693# not necessary any longer
        DockingWindow::Notify( rEvt );
        return true;
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // First, allow KeyInput for Dialog functions
        if ( !DockingWindow::Notify( rEvt ) && SfxViewShell::Current() )
            // then also for valid global accelerators.
            return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
        return true;
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS && !HasChildPathFocus() )
    {
        pBindings->SetActiveFrame( NULL );
        if (pMgr != NULL)
            pMgr->Deactivate_Impl();
    }

    return DockingWindow::Notify( rEvt );
}


sal_uInt16 SfxDockingWindow::GetWinBits_Impl() const
{
    sal_uInt16 nBits = 0;
    return nBits;
}



void SfxDockingWindow::SetItemSize_Impl( const Size& rSize )
{
    pImp->aSplitSize = rSize;

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
    if ( pImp->bSplitable )
        eIdent = SFX_CHILDWIN_SPLITWINDOW;
    pWorkWin->ConfigChild_Impl( eIdent, SFX_ALIGNDOCKINGWINDOW, pMgr->GetType() );
}

void SfxDockingWindow::Disappear_Impl()
{
    if ( pImp->pSplitWin && pImp->pSplitWin->IsItemValid( GetType() ) )
        pImp->pSplitWin->RemoveWindow(this);
}

void SfxDockingWindow::Reappear_Impl()
{
    if ( pImp->pSplitWin && !pImp->pSplitWin->IsItemValid( GetType() ) )
    {
        pImp->pSplitWin->InsertWindow( this, pImp->aSplitSize );
    }
}

bool SfxDockingWindow::IsAutoHide_Impl() const
{
    if ( pImp->pSplitWin )
        return !pImp->pSplitWin->IsFadeIn();
    else
        return false;
}

void SfxDockingWindow::AutoShow_Impl( bool bShow )
{
    if ( pImp->pSplitWin )
    {
        if ( bShow )
            pImp->pSplitWin->FadeIn();
        else
            pImp->pSplitWin->FadeOut();
    }
}

void SfxDockingWindow::FadeIn( bool /*bFadeIn*/ )
{
}

void SfxDockingWindow::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::INITSHOW )
        Initialize_Impl();

    DockingWindow::StateChanged( nStateChange );
}

void SfxDockingWindow::Move()
{
    if ( pImp )
        pImp->aMoveTimer.Start();
}

IMPL_LINK_NOARG(SfxDockingWindow, TimerHdl)
{
    pImp->aMoveTimer.Stop();
    if ( IsReallyVisible() && IsFloatingMode() )
    {
        if( !GetFloatingWindow()->IsRollUp() )
            SetFloatingSize( GetOutputSizePixel() );
        pImp->aWinState = GetFloatingWindow()->GetWindowState();
        SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
        if ( pImp->bSplitable )
            eIdent = SFX_CHILDWIN_SPLITWINDOW;
        SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
        pWorkWin->ConfigChild_Impl( eIdent, SFX_ALIGNDOCKINGWINDOW, pMgr->GetType() );
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
