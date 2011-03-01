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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

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
#include "sfxhelp.hxx"
#include <sfx2/objsh.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#define MAX_TOGGLEAREA_WIDTH        20
#define MAX_TOGGLEAREA_HEIGHT       20

using namespace ::com::sun::star;

// implemented in 'sfx2/source/appl/childwin.cxx'
extern sal_Bool GetPosSizeFromString( const String& rStr, Point& rPos, Size& rSize );
extern sal_Bool GetSplitSizeFromString( const String& rStr, Size& rSize );

// If you want to change the number you also have to:
// - Add new slot ids to sfxsids.hrc
// - Add new slots to frmslots.sdi
// - Add new slot definitions to sfx.sdi
static const int NUM_OF_DOCKINGWINDOWS = 10;

class SfxTitleDockingWindow;
class SfxTitleDockingWindow : public SfxDockingWindow
{
    Window*             m_pWrappedWindow;
    USHORT              m_nID;

public:
                        SfxTitleDockingWindow(
                            SfxBindings* pBindings ,
                            SfxChildWindow* pChildWin ,
                            Window* pParent ,
                            WinBits nBits,
                            USHORT  nID);
    virtual             ~SfxTitleDockingWindow();

    Window*             GetWrappedWindow() const { return m_pWrappedWindow; }
    void                SetWrappedWindow(Window* const pWindow);

    virtual void        StateChanged( StateChangedType nType );
    virtual long        Notify( NotifyEvent& rNEvt );
    virtual void        Resize();
    virtual void        Resizing( Size& rSize );
    virtual BOOL        Close();
};

namespace
{
    struct WindowState
    {
        ::rtl::OUString sTitle;
    };
}

static bool lcl_getWindowState( const uno::Reference< container::XNameAccess >& xWindowStateMgr, const ::rtl::OUString& rResourceURL, WindowState& rWindowState )
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
                if ( aWindowState[n].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UIName" ) ))
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

SfxDockingWrapper::SfxDockingWrapper( Window* pParentWnd ,
                                      USHORT nId ,
                                      SfxBindings* pBindings ,
                                      SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParentWnd , nId )
{
    uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    const rtl::OUString aDockWindowResourceURL( RTL_CONSTASCII_USTRINGPARAM( "private:resource/dockingwindow/" ));

    SfxTitleDockingWindow* pTitleDockWindow = new SfxTitleDockingWindow( pBindings, this, pParentWnd,
        WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE, nId);
    pWindow = pTitleDockWindow;
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    // Use factory manager to retrieve XWindow factory. That can be used to instanciate
    // the real window factory.
    uno::Reference< lang::XSingleComponentFactory > xFactoryMgr(
            xServiceManager->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.ui.WindowContentFactoryManager"))),
                uno::UNO_QUERY );

    if (xFactoryMgr.is())
    {
        SfxDispatcher* pDispatcher = pBindings->GetDispatcher();
        uno::Reference< frame::XFrame > xFrame( pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
        uno::Sequence< uno::Any > aArgs(2);
        beans::PropertyValue      aPropValue;
        aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
        aPropValue.Value = uno::makeAny( xFrame );
        aArgs[0] <<= aPropValue;
        aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ));

        // create a resource URL from the nId provided by the sfx2
        ::rtl::OUString aResourceURL( aDockWindowResourceURL );
        aResourceURL += ::rtl::OUString::valueOf(sal_Int32(nId));
        aPropValue.Value = uno::makeAny( aResourceURL );
        aArgs[1] <<= aPropValue;

        uno::Reference< awt::XWindow > xWindow;
        try
        {
            uno::Reference< beans::XPropertySet >    xProps( xServiceManager, uno::UNO_QUERY );
            uno::Reference< uno::XComponentContext > xContext;

            if ( xProps.is() )
                xProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
            if ( xContext.is() )
            {
                xWindow = uno::Reference< awt::XWindow>(
                            xFactoryMgr->createInstanceWithArgumentsAndContext( aArgs, xContext ),
                          uno::UNO_QUERY );
            }

            static uno::WeakReference< frame::XModuleManager >  m_xModuleManager;

            uno::Reference< frame::XModuleManager > xModuleManager( m_xModuleManager );
            if ( !xModuleManager.is() )
            {
                xModuleManager = uno::Reference< frame::XModuleManager >(
                                    xServiceManager->createInstance(
                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ModuleManager" ))),
                                    uno::UNO_QUERY );
                m_xModuleManager = xModuleManager;
            }

            static uno::WeakReference< container::XNameAccess > m_xWindowStateConfiguration;

            uno::Reference< container::XNameAccess > xWindowStateConfiguration( m_xWindowStateConfiguration );
            if ( !xWindowStateConfiguration.is() )
            {
                xWindowStateConfiguration = uno::Reference< container::XNameAccess >(
                                                xServiceManager->createInstance(
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.WindowStateConfiguration" ))),
                                                uno::UNO_QUERY );
                m_xWindowStateConfiguration = xWindowStateConfiguration;
            }

            ::rtl::OUString sModuleIdentifier = xModuleManager->identify( xFrame );

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

        Window* pContentWindow = VCLUnoHelper::GetWindow(xWindow);
        if ( pContentWindow )
            pContentWindow->SetStyle( pContentWindow->GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
        pTitleDockWindow->SetWrappedWindow(pContentWindow);
    }

    pWindow->SetOutputSizePixel( Size( 270, 240 ) );

    ( ( SfxDockingWindow* ) pWindow )->Initialize( pInfo );
    SetHideNotDelete( TRUE );
}

SfxChildWindow*  SfxDockingWrapper::CreateImpl(
Window *pParent, sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo* pInfo )
{
    SfxChildWindow *pWin = new SfxDockingWrapper(pParent, nId, pBindings, pInfo);
    return pWin;
}

sal_uInt16 SfxDockingWrapper::GetChildWindowId ()
{
    DBG_ASSERT( false, "This method shouldn't be called!" );
    return 0;
}

void SfxDockingWrapper::RegisterChildWindow (sal_Bool bVis, SfxModule *pMod, sal_uInt16 nFlags)
{
    // pre-register a couple of docking windows
    for (int i=0; i < NUM_OF_DOCKINGWINDOWS; i++ )
    {
        USHORT nID = USHORT(SID_DOCKWIN_START+i);
        SfxChildWinFactory *pFact = new SfxChildWinFactory( SfxDockingWrapper::CreateImpl, nID, 0xffff );
        pFact->aInfo.nFlags |= nFlags;
        pFact->aInfo.bVisible = bVis;
        SfxChildWindow::RegisterChildWindow(pMod, pFact);
    }
}

SfxChildWinInfo  SfxDockingWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    ((SfxDockingWindow*)GetWindow())->FillInfo( aInfo );
    return aInfo;
};

SfxTitleDockingWindow::SfxTitleDockingWindow( SfxBindings* pBind ,
                                              SfxChildWindow* pChildWin ,
                                              Window* pParent ,
                                              WinBits nBits,
                                              USHORT  nID ) :
                          SfxDockingWindow( pBind ,
                                            pChildWin ,
                                            pParent ,
                                            nBits ),
                          m_pWrappedWindow(0),
                          m_nID(nID)
{
}

SfxTitleDockingWindow::~SfxTitleDockingWindow()
{
    delete m_pWrappedWindow;
}

void SfxTitleDockingWindow::SetWrappedWindow( Window* const pWindow )
{
    m_pWrappedWindow = pWindow;
    if (m_pWrappedWindow)
    {
        m_pWrappedWindow->SetParent(this);
        m_pWrappedWindow->SetSizePixel( GetOutputSizePixel() );
        m_pWrappedWindow->Show();
    }
}

long SfxTitleDockingWindow::Notify( NotifyEvent& rNEvt )
{
    return SfxDockingWindow::Notify( rNEvt );
}

void SfxTitleDockingWindow::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        Window* pWindow = GetWrappedWindow();
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

BOOL SfxTitleDockingWindow::Close()
{
    return SfxDockingWindow::Close();
}

namespace
{
    struct ChildrenRegisteredMap : public rtl::Static< bool, ChildrenRegisteredMap > {};
}

static bool lcl_checkDockingWindowID( USHORT nID )
{
    if (nID < SID_DOCKWIN_START || nID >= USHORT(SID_DOCKWIN_START+NUM_OF_DOCKINGWINDOWS))
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
void SAL_CALL SfxDockingWindowFactory( const uno::Reference< frame::XFrame >& rFrame, const rtl::OUString& rDockingWindowName )
{
    SolarMutexGuard aGuard;
    USHORT nID = USHORT(rDockingWindowName.toInt32());

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
bool SAL_CALL IsDockingWindowVisible( const uno::Reference< frame::XFrame >& rFrame, const rtl::OUString& rDockingWindowName )
{
    SolarMutexGuard aGuard;

    USHORT nID = USHORT(rDockingWindowName.toInt32());

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
    BOOL                bConstructed;
    Size                aMinSize;
    SfxSplitWindow*     pSplitWin;
    BOOL                bSplitable;
    Timer               aMoveTimer;

    // Folgende members sind nur in der Zeit von StartDocking bis EndDocking
    // g"ultig:
    BOOL                bEndDocked;
    Size                aSplitSize;
    long                nHorizontalSize;
    long                nVerticalSize;
    USHORT              nLine;
    USHORT              nPos;
    USHORT              nDockLine;
    USHORT              nDockPos;
    BOOL                bNewLine;
    BOOL                bDockingPrevented;
    ByteString          aWinState;

    SfxChildAlignment   GetLastAlignment() const
                        { return eLastAlignment; }
    void                SetLastAlignment(SfxChildAlignment eAlign)
                        { eLastAlignment = eAlign; }
    SfxChildAlignment   GetDockAlignment() const
                        { return eDockAlignment; }
    void                SetDockAlignment(SfxChildAlignment eAlign)
                        { eDockAlignment = eAlign; }
};

//-------------------------------------------------------------------------

void SfxDockingWindow::Resize()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow merkt sich ggf. eine
    ver"anderte FloatingSize.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    auch SfxDockingWindow::Resize() gerufen werden.
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

//-------------------------------------------------------------------------

BOOL SfxDockingWindow::PrepareToggleFloatingMode()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow erm"oglicht ein Eingreifen
    in das Umschalten des floating mode.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    danach SfxDockingWindow::PrepareToggleFloatingMode() gerufen werden,
    wenn nicht FALSE zur"uckgegeben wird.
*/

{
    if (!pImp->bConstructed)
        return TRUE;

    if ( (Application::IsInModalMode() && IsFloatingMode()) || !pMgr )
        return FALSE;

    if ( pImp->bDockingPrevented )
        return FALSE;

    if (!IsFloatingMode())
    {
        // Testen, ob FloatingMode erlaubt ist
        if ( CheckAlignment(GetAlignment(),SFX_ALIGN_NOALIGNMENT) != SFX_ALIGN_NOALIGNMENT )
            return FALSE;

        if ( pImp->pSplitWin )
        {
            // Das DockingWindow sitzt in einem SplitWindow und wird abgerissen
            pImp->pSplitWin->RemoveWindow(this/*, FALSE*/);
            pImp->pSplitWin = 0;
        }
    }
    else if ( pMgr )
    {
        pImp->aWinState = GetFloatingWindow()->GetWindowState();

        // Testen, ob es erlaubt ist, anzudocken
        if (CheckAlignment(GetAlignment(),pImp->GetLastAlignment()) == SFX_ALIGN_NOALIGNMENT)
            return FALSE;

        // Testen, ob das Workwindow gerade ein Andocken erlaubt
        SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
        if ( !pWorkWin->IsDockingAllowed() || !pWorkWin->IsInternalDockingAllowed() )
            return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::ToggleFloatingMode()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow setzt die internen
    Daten des SfxDockingWindow und sorgt f"ur korrektes Alignment am
    parent window.
    Durch PrepareToggleFloatMode und Initialize ist sichergestellt, da\s
    pImp->GetLastAlignment() immer eine erlaubtes Alignment liefert.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    zuerst SfxDockingWindow::ToggleFloatingMode() gerufen werden.
*/
{
    if ( !pImp->bConstructed || !pMgr )
        return;                 // Kein Handler-Aufruf

    // Altes Alignment merken und dann umschalten.
    // Sv hat jetzt schon umgeschaltet, aber Alignment am SfxDockingWindow
    // ist noch das alte!
    // Was war ich bisher ?
    SfxChildAlignment eLastAlign = GetAlignment();

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
    if ( pImp->bSplitable )
        eIdent = SFX_CHILDWIN_SPLITWINDOW;

    if (IsFloatingMode())
    {
        SetAlignment(SFX_ALIGN_NOALIGNMENT);
        if ( pImp->aWinState.Len() )
            GetFloatingWindow()->SetWindowState( pImp->aWinState );
        else
            GetFloatingWindow()->SetOutputSizePixel( GetFloatingSize() );
    }
    else
    {
        if (pImp->GetDockAlignment() == eLastAlign)
        {
            // Wenn ToggleFloatingMode aufgerufen wurde, das DockAlignment
            // aber noch unver"andert ist, mu\s das ein Toggeln durch DClick
            // gewesen sein, also LastAlignment verwenden
            SetAlignment (pImp->GetLastAlignment());
            if ( !pImp->bSplitable )
                SetSizePixel( CalcDockingSize(GetAlignment()) );
        }
        else
        {
            // Toggeln wurde durch Draggen ausgel"ost
            pImp->nLine = pImp->nDockLine;
            pImp->nPos = pImp->nDockPos;
            SetAlignment (pImp->GetDockAlignment());
        }

        if ( pImp->bSplitable )
        {
            // Das DockingWindow kommt jetzt in ein SplitWindow
            pImp->pSplitWin = pWorkWin->GetSplitWindow_Impl(GetAlignment());

            // Das LastAlignment ist jetzt immer noch das zuletzt angedockte
            SfxSplitWindow *pSplit = pWorkWin->GetSplitWindow_Impl(pImp->GetLastAlignment());

            DBG_ASSERT( pSplit, "LastAlignment kann nicht stimmen!" );
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

    // altes Alignment festhalten f"ur n"achstes Togglen; erst jetzt setzen
    // wg. Abmelden beim SplitWindow!
    pImp->SetLastAlignment(eLastAlign);

    // DockAlignment zur"ucksetzen, falls noch EndDocking gerufen wird
    pImp->SetDockAlignment(GetAlignment());

    // SfxChildWindow korrekt andocken bzw. entdocken
    if ( pMgr )
        pWorkWin->ConfigChild_Impl( eIdent, SFX_TOGGLEFLOATMODE, pMgr->GetType() );
}

//-------------------------------------------------------------------------

void SfxDockingWindow::StartDocking()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow holt vom parent window
    das innere und "au\sere docking rectangle.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    am Ende SfxDockingWindow::StartDocking() gerufen werden.
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
        // Die aktuellen Docking-Daten besorgen
        pImp->pSplitWin->GetWindowPos(this, pImp->nLine, pImp->nPos);
        pImp->nDockLine = pImp->nLine;
        pImp->nDockPos = pImp->nPos;
        pImp->bNewLine = FALSE;
    }
}

//-------------------------------------------------------------------------

BOOL SfxDockingWindow::Docking( const Point& rPos, Rectangle& rRect )

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow berechnet das aktuelle
    tracking rectangle. Dazu benutzt sie die Methode CalcAlignment(rPos,rRect),
    deren Verhalten von abgeleiteten Klassen beeinflu\st werden kann (s.u.).
    Diese Methode sollte nach M"oglichkeit nicht "uberschrieben werden.
*/
{
    if ( Application::IsInModalMode() )
        return TRUE;

    if ( !pImp->bConstructed || !pMgr )
    {
        rRect.SetSize( Size() );
        return IsFloatingMode();
    }

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    if ( pImp->bDockingPrevented || !pWorkWin->IsInternalDockingAllowed() )
        return FALSE;

    BOOL bFloatMode = FALSE;

    if ( GetOuterRect().IsInside( rPos ) && !IsDockingPrevented() )
    {
        // Maus innerhalb OuterRect : Alignment und Rectangle berechnen
        SfxChildAlignment eAlign = CalcAlignment(rPos, rRect);
        if (eAlign == SFX_ALIGN_NOALIGNMENT)
            bFloatMode = TRUE;
        pImp->SetDockAlignment(eAlign);
    }
    else
    {
        // Maus nicht innerhalb OuterRect : muss FloatingWindow sein
        // Ist das erlaubt ?
        if (CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT) != SFX_ALIGN_NOALIGNMENT)
            return FALSE;
        bFloatMode = TRUE;
        if ( SFX_ALIGN_NOALIGNMENT != pImp->GetDockAlignment() )
        {
            // wg. SV-Bug darf rRect nur ver"andert werden, wenn sich das
            // Alignment "andert !
            pImp->SetDockAlignment(SFX_ALIGN_NOALIGNMENT);
            rRect.SetSize(CalcDockingSize(SFX_ALIGN_NOALIGNMENT));
        }
    }

    if ( !pImp->bSplitable )
    {
        // Bei individuell angedocktem Window wird die Position durch das
        // Alignment und die docking rects festgelegt.
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

//-------------------------------------------------------------------------

void SfxDockingWindow::EndDocking( const Rectangle& rRect, BOOL bFloatMode )

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow sorgt f"ur das korrekte
    Alignment am parent window.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    zuerst SfxDockingWindow::EndDocking() gerufen werden.
*/
{
    if ( !pImp->bConstructed || IsDockingCanceled() || !pMgr )
        return;

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    BOOL bReArrange = FALSE;
    if ( pImp->bSplitable )
    {
        // Wenn sich das Alignment "andert und das Fenster befindet sich
        // im angedockten Zustand in einem SplitWindow, mu\s umgemeldet werden
        // Wenn neu angedockt wird, machen PrepareToggleFloatingMode()
        // und ToggleFloatingMode() das Ummelden.
        if ( !bFloatMode )
            bReArrange = TRUE;
    }

    if ( bReArrange )
    {
        if ( GetAlignment() != pImp->GetDockAlignment() )
        {
            // Vor dem Show() mu\s das Ummelden passiert sein, daher kann nicht
            // die Basisklasse gerufen werden
            if ( IsFloatingMode() || !pImp->bSplitable )
                Show( FALSE, SHOW_NOFOCUSCHANGE );

            // Die Gr"o\se f"urs Toggeln setzen
            pImp->aSplitSize = rRect.GetSize();
            if ( IsFloatingMode() )
            {
                SetFloatingMode( bFloatMode );
                if ( IsFloatingMode() || !pImp->bSplitable )
                    Show( TRUE, SHOW_NOFOCUSCHANGE );
            }
            else
            {
                pImp->pSplitWin->RemoveWindow(this,FALSE);
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
            // Ich wurde innerhalb meines Splitwindows verschoben.
            if ( pImp->nLine != pImp->nDockLine )
                pImp->aSplitSize = rRect.GetSize();
            pImp->pSplitWin->MoveWindow( this, pImp->aSplitSize, pImp->nDockLine, pImp->nDockPos, pImp->bNewLine );
        }
    }
    else
    {
        pImp->bEndDocked = TRUE;
        DockingWindow::EndDocking(rRect, bFloatMode);
        pImp->bEndDocked = FALSE;
    }

    SetAlignment( IsFloatingMode() ? SFX_ALIGN_NOALIGNMENT : pImp->GetDockAlignment() );
}

//-------------------------------------------------------------------------

void SfxDockingWindow::Resizing( Size& /*rSize*/ )

/*  [Beschreibung]

    Virtuelle Methode der Klasse DockingWindow.
    Hier kann das interaktive Umgr"o\sern im FloatingMode beeinflu\t werden,
    z.B. indem nur diskrete Werte f"ur Breite und/oder H"ohe zugelassen werden.
    Die Basisimplementation verhindert, da\s die OutputSize kleiner wird als
    eine mit SetMinOutputSizePixel() gesetzte Gr"o\se.
*/

{

}

//-------------------------------------------------------------------------

SfxDockingWindow::SfxDockingWindow( SfxBindings *pBindinx, SfxChildWindow *pCW,
    Window* pParent, WinBits nWinBits) :
    DockingWindow (pParent, nWinBits),
    pBindings(pBindinx),
    pMgr(pCW),
    pImp(NULL)

/*  [Beschreibung]

    ctor der Klasse SfxDockingWindow. Es wird ein SfxChildWindow ben"otigt,
    da das Andocken im Sfx "uber SfxChildWindows realisiert wird.
*/

{
    ULONG nId = GetHelpId();
    if ( !nId && pCW )
        nId = pCW->GetType();
    SetHelpId( 0 );
    SetUniqueId( nId );

    pImp = new SfxDockingWindow_Impl;
    pImp->bConstructed = FALSE;
    pImp->pSplitWin = 0;
    pImp->bEndDocked = FALSE;
    pImp->bDockingPrevented = FALSE;

    pImp->bSplitable = TRUE;

    // Zun"achst auf Defaults setzen; das Alignment wird in der Subklasse gesetzt
    pImp->nLine = pImp->nDockLine = 0;
    pImp->nPos  = pImp->nDockPos = 0;
    pImp->bNewLine = FALSE;
    pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxDockingWindow,TimerHdl));
}

//-------------------------------------------------------------------------

SfxDockingWindow::SfxDockingWindow( SfxBindings *pBindinx, SfxChildWindow *pCW,
    Window* pParent, const ResId& rResId) :
    DockingWindow(pParent, rResId),
    pBindings(pBindinx),
    pMgr(pCW),
    pImp(NULL)

/*  [Beschreibung]

    ctor der Klasse SfxDockingWindow. Es wird ein SfxChildWindow ben"otigt,
    da das Andocken im Sfx "uber SfxChildWindows realisiert wird.
*/

{
    ULONG nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );

    pImp = new SfxDockingWindow_Impl;
    pImp->bConstructed = FALSE;
    pImp->pSplitWin = 0;
    pImp->bEndDocked = FALSE;
    pImp->bDockingPrevented = FALSE;

    pImp->bSplitable = TRUE;

    // Zun"achst auf Defaults setzen; das Alignment wird in der Subklasse gesetzt
    pImp->nLine = pImp->nDockLine = 0;
    pImp->nPos  = pImp->nDockPos = 0;
    pImp->bNewLine = FALSE;
    pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxDockingWindow,TimerHdl));
}

//-------------------------------------------------------------------------

void SfxDockingWindow::Initialize(SfxChildWinInfo *pInfo)
/*  [Beschreibung]

    Initialisierung der Klasse SfxDockingWindow "uber ein SfxChildWinInfo.
    Die Initialisierung erfolgt erst in einem 2.Schritt nach dem ctor und sollte
    vom ctor der abgeleiteten Klasse oder vom ctor des SfxChildWindows
    aufgerufen werden.
*/
{
    if ( !pMgr )
    {
        pImp->SetDockAlignment( SFX_ALIGN_NOALIGNMENT );
        pImp->bConstructed = TRUE;
        return;
    }

    if ( pInfo->nFlags & SFX_CHILDWIN_FORCEDOCK )
        pImp->bDockingPrevented = TRUE;

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

    sal_Bool bVertHorzRead( sal_False );
    if ( pInfo->aExtraString.Len() )
    {
        // get information about alignment, split size and position in SplitWindow
        String aStr;
        USHORT nPos = pInfo->aExtraString.SearchAscii("AL:");
        if ( nPos != STRING_NOTFOUND )
        {
            // alignment information
            USHORT n1 = pInfo->aExtraString.Search('(', nPos);
            if ( n1 != STRING_NOTFOUND )
            {
                USHORT n2 = pInfo->aExtraString.Search(')', n1);
                if ( n2 != STRING_NOTFOUND )
                {
                    // extract alignment information from extrastring
                    aStr = pInfo->aExtraString.Copy(nPos, n2 - nPos + 1);
                    pInfo->aExtraString.Erase(nPos, n2 - nPos + 1);
                    aStr.Erase(nPos, n1-nPos+1);
                }
            }
        }

        if ( aStr.Len() )
        {
            // accept window state only if alignment is also set
            pImp->aWinState = pInfo->aWinState;

            // check for valid alignment
            SfxChildAlignment eLocalAlignment = (SfxChildAlignment) (USHORT) aStr.ToInt32();
            if ( pImp->bDockingPrevented )
                // docking prevented, ignore old configuration and take alignment from default
                aStr.Erase();
            else
                SetAlignment( eLocalAlignment );

            SfxChildAlignment eAlign = CheckAlignment(GetAlignment(),GetAlignment());
            if ( eAlign != GetAlignment() )
            {
                DBG_ERROR("Invalid Alignment!");
                SetAlignment( eAlign );
                aStr.Erase();
            }

            // get last alignment (for toggeling)
            nPos = aStr.Search(',');
            if ( nPos != STRING_NOTFOUND )
            {
                aStr.Erase(0, nPos+1);
                pImp->SetLastAlignment( (SfxChildAlignment) (USHORT) aStr.ToInt32() );
            }

            nPos = aStr.Search(',');
            if ( nPos != STRING_NOTFOUND )
            {
                // get split size and position in SplitWindow
                Point aPos;
                aStr.Erase(0, nPos+1);
                if ( GetPosSizeFromString( aStr, aPos, pImp->aSplitSize ) )
                {
                    pImp->nLine = pImp->nDockLine = (USHORT) aPos.X();
                    pImp->nPos  = pImp->nDockPos  = (USHORT) aPos.Y();
                    pImp->nVerticalSize = pImp->aSplitSize.Height();
                    pImp->nHorizontalSize = pImp->aSplitSize.Width();
                    if ( GetSplitSizeFromString( aStr, pImp->aSplitSize ))
                        bVertHorzRead = sal_True;
                }
            }
        }
        else {
            DBG_ERROR( "Information is missing!" );
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
    BOOL bFloatMode = IsFloatingMode();
    if ( bFloatMode != ((GetAlignment() == SFX_ALIGN_NOALIGNMENT)) )
    {
        bFloatMode = !bFloatMode;
        SetFloatingMode( bFloatMode );
        if ( bFloatMode )
        {
            if ( pImp->aWinState.Len() )
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
            // Fenster ist individuell angedockt; Gr"o\se berechnen.
            // Dazu mu\s sie mit der FloatingSize initialisiert werden, falls
            // irgendwer sich darauf verl"a\st, da\s eine vern"unftige Gr"o\se
            // gesetzt ist
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
        pImp->bConstructed = TRUE;
        return;
    }

    FloatingWindow* pFloatWin = GetFloatingWindow();
    BOOL bSet = FALSE;
    if ( pFloatWin )
    {
        bSet = !pFloatWin->IsDefaultPos();
    }
    else
    {
        Point aPos = GetFloatingPos();
        if ( aPos != Point() )
            bSet = TRUE;
    }

    if ( !bSet)
    {
        SfxViewFrame *pFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        Window* pEditWin = pFrame->GetViewShell()->GetWindow();
        Point aPos = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPos = GetParent()->ScreenToOutputPixel( aPos );
        SetFloatingPos( aPos );
    }

    if ( pFloatWin )
    {
        // initialize floating window
        if ( !pImp->aWinState.Len() )
            // window state never set before, get if from defaults
            pImp->aWinState = pFloatWin->GetWindowState();

        // trick: use VCL method SetWindowState to adjust position and size
        pFloatWin->SetWindowState( pImp->aWinState );

        // remember floating size for calculating alignment and tracking rectangle
        SetFloatingSize( pFloatWin->GetSizePixel() );

        // some versions of VCL didn't call resize in the current situation
        //Resize();
    }

    // allow calling of docking handlers
    pImp->bConstructed = TRUE;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::FillInfo(SfxChildWinInfo& rInfo) const

/*  [Beschreibung]

    F"ullt ein SfxChildWinInfo mit f"ur SfxDockingWindow spezifischen Daten,
    damit sie in die INI-Datei geschrieben werden koennen.
    Es wird angenommen, da\s rInfo alle anderen evt. relevanten Daten in
    der ChildWindow-Klasse erh"alt.
    Eingetragen werden hier gemerkten Gr"o\sen, das ZoomIn-Flag und die
    f"ur das Docking relevanten Informationen.
    Wird diese Methode "uberschrieben, mu\s zuerst die Basisimplementierung
    gerufen werden.
*/

{
    if ( !pMgr )
        return;

    if ( GetFloatingWindow() && pImp->bConstructed )
        pImp->aWinState = GetFloatingWindow()->GetWindowState();

    rInfo.aWinState = pImp->aWinState;
    rInfo.aExtraString = DEFINE_CONST_UNICODE("AL:(");
    rInfo.aExtraString += String::CreateFromInt32((USHORT) GetAlignment());
    rInfo.aExtraString += ',';
    rInfo.aExtraString += String::CreateFromInt32 ((USHORT) pImp->GetLastAlignment());
    if ( pImp->bSplitable )
    {
        Point aPos(pImp->nLine, pImp->nPos);
        rInfo.aExtraString += ',';
        rInfo.aExtraString += String::CreateFromInt32( aPos.X() );
        rInfo.aExtraString += '/';
        rInfo.aExtraString += String::CreateFromInt32( aPos.Y() );
        rInfo.aExtraString += '/';
        rInfo.aExtraString += String::CreateFromInt32( pImp->nHorizontalSize );
        rInfo.aExtraString += '/';
        rInfo.aExtraString += String::CreateFromInt32( pImp->nVerticalSize );
        rInfo.aExtraString += ',';
        rInfo.aExtraString += String::CreateFromInt32( pImp->aSplitSize.Width() );
        rInfo.aExtraString += ';';
        rInfo.aExtraString += String::CreateFromInt32( pImp->aSplitSize.Height() );
    }

    rInfo.aExtraString += ')';
}

//-------------------------------------------------------------------------

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

//-------------------------------------------------------------------------

SfxChildAlignment SfxDockingWindow::CalcAlignment(const Point& rPos, Rectangle& rRect)

/*  [Beschreibung]

    Diese Methode berechnet f"ur gegebene Mausposition und tracking rectangle,
    welches Alignment sich daraus ergeben w"urde. Beim Wechsel des Alignments
    kann sich auch das tracking rectangle "andern, so dass ein ver"andertes
    rectangle zur"uckgegeben wird.

    Der Klassenbenutzer kann das Verhalten dieser Methode und damit das Verhalten
    seiner DockinWindow-Klasse beim Docken beeinflussen, indem er die hier
    aufgerufene virtuelle Methode

        SfxDockingWindow::CalcDockingSize(SfxChildAlignment eAlign)

    "uberschreibt (s.u.).
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
    BOOL bBecomesFloating = FALSE;
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
        bBecomesFloating = TRUE;
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
            // docking rectangle completely inside (shrinked) inner area -> floating mode
            bBecomesFloating = TRUE;
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
        BOOL  bNoChange = FALSE;

        // check if alignment is still unchanged
        switch ( GetAlignment() )
        {
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
                if (aInPosTL.X() <= 0)
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_TOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_HIGHESTTOP:
                if ( aInPosTL.Y() <= 0)
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                if ( aInPosBR.X() >= aInSize.Width())
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
                if ( aInPosBR.Y() >= aInSize.Height())
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            default:
                break;
        }

        if ( !bNoChange )
        {
            // alignment will change, test alignment according to distance of the docking rectangles edges
            BOOL bForbidden = TRUE;
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
        //Im FloatingMode erh"alt das tracking rectangle die floating size
        // wg. SV-Bug darf rRect nur ver"andert werden, wenn sich das
        // Alignment "andert !
        if ( eDockAlign != pImp->GetDockAlignment() )
            aDockingRect.SetSize( aFloatingSize );
    }
    else if ( pImp->bSplitable )
    {
        USHORT nLine, nPos;
        SfxSplitWindow *pSplitWin = pWorkWin->GetSplitWindow_Impl(eDockAlign);
        aPos = pSplitWin->ScreenToOutputPixel( aPos );
        if ( pSplitWin->GetWindowPos( aPos, nLine, nPos ) )
        {
            // mouse over splitwindow, get line and position
            pImp->nDockLine = nLine;
            pImp->nDockPos = nPos;
            pImp->bNewLine = FALSE;
        }
        else
        {
            if ( 0 )
            {
                // mouse touches outer border -> treated as floating mode
                eDockAlign = SFX_ALIGN_NOALIGNMENT;
                aDockingRect.SetSize( aFloatingSize );
                rRect = aDockingRect;
                return eDockAlign;
            }

            // mouse touches inner border -> create new line
            if ( eDockAlign == GetAlignment() && pImp->pSplitWin &&
                 pImp->nLine == pImp->pSplitWin->GetLineCount()-1 && pImp->pSplitWin->GetWindowCount(pImp->nLine) == 1 )
            {
                // if this window is the only one in the last line, it can't be docked as new line in the same splitwindow
                pImp->nDockLine = pImp->nLine;
                pImp->nDockPos = pImp->nPos;
                pImp->bNewLine = FALSE;
            }
            else
            {
                // create new line
                pImp->nDockLine = pSplitWin->GetLineCount();
                pImp->nDockPos = 0;
                pImp->bNewLine = TRUE;
            }
        }

        BOOL bChanged = pImp->nLine != pImp->nDockLine || pImp->nPos != pImp->nDockPos || eDockAlign != GetAlignment();
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

//-------------------------------------------------------------------------

Size SfxDockingWindow::CalcDockingSize(SfxChildAlignment eAlign)

/*  [Beschreibung]

    Virtuelle Methode der Klasse SfxDockingWindow.
    Hier wird festgelegt, wie sich die Gr"o\se des DockingWindows abh"angig vom
    Alignment "andert.
    Die Basisimplementation setzt im Floating Mode die Gr"o\se auf die gemerkte
    Floating Size.
    Bei horizontalem Alignment wird die Breite auf die Breite des "au\seren
    DockingRects, bei vertikalem Alignment die H"ohe auf die H"ohe des inneren
    DockingRects (ergibt sich aus der Reihenfolge, in der im SFX ChildWindows
    ausgegeben werden). Die jeweils andere Gr"o\se wird auf die aktuelle
    Floating Size gesetzt, hier k"onnte eine abgeleitete Klasse "andernd
    eingreifen.
    Die DockingSize mu\s f"ur Left/Right und Top/Bottom jeweils gleich sein.
*/

{
    // Achtung: falls das Resizing auch im angedockten Zustand geht, mu\s dabei
    // auch die Floating Size angepa\st werden !?

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

//-------------------------------------------------------------------------

SfxChildAlignment SfxDockingWindow::CheckAlignment(SfxChildAlignment,
    SfxChildAlignment eAlign)

/*  [Beschreibung]

    Virtuelle Methode der Klasse SfxDockingWindow.
    Hier kann eine abgeleitete Klasse bestimmte Alignments verbieten.
    Die Basisimplementation verbietet kein Alignment.
*/

{
    return eAlign;
}

//-------------------------------------------------------------------------

BOOL SfxDockingWindow::Close()

/*  [Beschreibung]

    Das Fenster wird geschlossen, indem das ChildWindow durch Ausf"uhren des
    ChildWindow-Slots zerst"ort wird.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    danach SfxDockingWindow::Close() gerufen werden, wenn nicht das Close()
    mit "return FALSE" abgebrochen wird.

*/
{
    // Execute mit Parametern, da Toggle von einigen ChildWindows ignoriert
    // werden kann
    if ( !pMgr )
        return TRUE;

    SfxBoolItem aValue( pMgr->GetType(), FALSE);
    pBindings->GetDispatcher_Impl()->Execute(
        pMgr->GetType(), SFX_CALLMODE_RECORD | SFX_CALLMODE_ASYNCHRON, &aValue, 0L );
    return TRUE;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::Paint(const Rectangle& /*rRect*/)

/*  [Beschreibung]

    Es wird im angedockten Zustand eine Begrenzungslinie an der angedockten
    Kante und ein Rahmen ausgegeben. Dabei wird SVLOOK ber"ucksichtigt.
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

//-------------------------------------------------------------------------

void SfxDockingWindow::SetMinOutputSizePixel( const Size& rSize )

/*  [Beschreibung]

    Mit dieser Methode kann eine minimale OutpuSize gesetzt werden, die
    im Resizing()-Handler abgefragt wird.
*/

{
    pImp->aMinSize = rSize;
    DockingWindow::SetMinOutputSizePixel( rSize );
}

//-------------------------------------------------------------------------

Size SfxDockingWindow::GetMinOutputSizePixel() const

/*  [Beschreibung]

    Die gesetzte minimale Gr"o\se wird zur"uckgegeben.
*/

{
    return pImp->aMinSize;
}

//-------------------------------------------------------------------------

long SfxDockingWindow::Notify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == EVENT_GETFOCUS )
    {
        pBindings->SetActiveFrame( pMgr->GetFrame() );

        if ( pImp->pSplitWin )
            pImp->pSplitWin->SetActiveWindow_Impl( this );
        else
            pMgr->Activate_Impl();

        Window* pWindow = rEvt.GetWindow();
        ULONG nHelpId  = 0;
        while ( !nHelpId && pWindow )
        {
            nHelpId = pWindow->GetHelpId();
            pWindow = pWindow->GetParent();
        }

        if ( nHelpId )
            SfxHelp::OpenHelpAgent( &pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame(), nHelpId );

        // In VCL geht Notify zun"achst an das Fenster selbst,
        // also base class rufen, sonst erf"ahrt der parent nichts
        // if ( rEvt.GetWindow() == this )  PB: #i74693# not necessary any longer
        DockingWindow::Notify( rEvt );
        return TRUE;
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // KeyInput zuerst f"ur Dialogfunktionen zulassen
        if ( !DockingWindow::Notify( rEvt ) && SfxViewShell::Current() )
            // dann auch global g"ultige Acceleratoren verwenden
            return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
        return TRUE;
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS && !HasChildPathFocus() )
    {
        pBindings->SetActiveFrame( NULL );
        pMgr->Deactivate_Impl();
    }

    return DockingWindow::Notify( rEvt );
}


USHORT SfxDockingWindow::GetWinBits_Impl() const
{
    USHORT nBits = 0;
    return nBits;
}

//-------------------------------------------------------------------------

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

BOOL SfxDockingWindow::IsAutoHide_Impl() const
{
    if ( pImp->pSplitWin )
        return !pImp->pSplitWin->IsFadeIn();
    else
        return FALSE;
}

BOOL SfxDockingWindow::IsPinned_Impl() const
{
    if ( pImp->pSplitWin )
        return pImp->pSplitWin->IsPinned();
    else
        return TRUE;
}
void SfxDockingWindow::AutoShow( BOOL bShow )
{
    AutoShow_Impl(bShow);
}

void SfxDockingWindow::AutoShow_Impl( BOOL bShow )
{
    if ( pImp->pSplitWin )
    {
        if ( bShow )
            pImp->pSplitWin->FadeIn();
        else
            pImp->pSplitWin->FadeOut();
    }
}

SfxSplitWindow* SfxDockingWindow::GetSplitWindow_Impl() const
{
    return pImp->pSplitWin;
}

void SfxDockingWindow::FadeIn( BOOL /*bFadeIn*/ )
{
}

void SfxDockingWindow::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
        Initialize_Impl();

    DockingWindow::StateChanged( nStateChange );
}

void SfxDockingWindow::Move()
{
    if ( pImp )
        pImp->aMoveTimer.Start();
}

IMPL_LINK( SfxDockingWindow, TimerHdl, Timer*, EMPTYARG)
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
