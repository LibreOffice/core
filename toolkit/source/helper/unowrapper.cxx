/*************************************************************************
 *
 *  $RCSfile: unowrapper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-15 11:43:32 $
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

#ifndef _COM_SUN_STAR_AWT_WINDOWEVENT_HPP_
#include <com/sun/star/awt/WindowEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HPP_
#include <com/sun/star/awt/MouseButton.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <toolkit/helper/unowrapper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/awt/vclxcontainer.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxgraphics.hxx>

#include <vcl/svapp.hxx>

#include <tools/debug.hxx>

//  ----------------------------------------------------
//  class UnoWrapper
//  ----------------------------------------------------

extern "C" {

UnoWrapperBase* CreateUnoWrapper()
{
    return new UnoWrapper( NULL );
}

};  // extern "C"


// Mit Out-Parameter besser als Rueckgabewert, wegen Ref-Objekt...

void ImplInitWindowEvent( ::com::sun::star::awt::WindowEvent& rEvent, Window* pWindow )
{
    Point aPos = pWindow->GetPosPixel();
    Size aSz = pWindow->GetSizePixel();

    rEvent.X = aPos.X();
    rEvent.Y = aPos.Y();

    rEvent.Width = aSz.Width();
    rEvent.Height = aSz.Height();

    pWindow->GetBorder( rEvent.LeftInset, rEvent.TopInset, rEvent.RightInset, rEvent.BottomInset );
}

void ImplInitKeyEvent( ::com::sun::star::awt::KeyEvent& rEvent, const KeyEvent& rEvt )
{
    rEvent.Modifiers = 0;
    if ( rEvt.GetKeyCode().IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if ( rEvt.GetKeyCode().IsMod1() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if ( rEvt.GetKeyCode().IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;

    rEvent.KeyCode = rEvt.GetKeyCode().GetCode();
    rEvent.KeyChar = (unsigned char)rEvt.GetCharCode();
    rEvent.KeyFunc = rEvt.GetKeyCode().GetFunction();
}

void ImplInitMouseEvent( ::com::sun::star::awt::MouseEvent& rEvent, const MouseEvent& rEvt )
{
    rEvent.Modifiers = 0;
    if ( rEvt.IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if ( rEvt.IsMod1() )
    rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if ( rEvt.IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;

    rEvent.Buttons = 0;
    if ( rEvt.IsLeft() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::LEFT;
    if ( rEvt.IsRight() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::RIGHT;
    if ( rEvt.IsMiddle() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::MIDDLE;

    rEvent.X = rEvt.GetPosPixel().X();
    rEvent.Y = rEvt.GetPosPixel().Y();
    rEvent.ClickCount = rEvt.GetClicks();
    rEvent.PopupTrigger = sal_False;
}

//  ----------------------------------------------------
//  class UnoWrapper
//  ----------------------------------------------------

UnoWrapper::UnoWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>& rxToolkit )
{
    mxToolkit = rxToolkit;
}

void UnoWrapper::Destroy()
{
    delete this;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> UnoWrapper::GetVCLToolkit()
{
    if ( !mxToolkit.is() )
        mxToolkit = VCLUnoHelper::CreateToolkit();
    return mxToolkit.get();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> UnoWrapper::GetWindowInterface( Window* pWindow, BOOL bCreate )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xPeer = pWindow->GetWindowPeer();
    if ( !xPeer.is() && bCreate )
    {
        ::com::sun::star::awt::XWindowPeer* pPeer = NULL;
        // Basis-Component erzeugen...
        WindowType eType = pWindow->GetType();
        if ( ( eType == WINDOW_TABPAGE ) ||
            ( eType == WINDOW_WINDOW ) ||
            ( eType == WINDOW_FLOATINGWINDOW ) )
        {
            pPeer = new VCLXContainer;
        }
        else if ( ( eType == WINDOW_SYSWINDOW ) || ( eType == WINDOW_WORKWINDOW ) || eType == WINDOW_DOCKINGWINDOW )
        {
            pPeer = new VCLXTopWindow;
        }
        else if ( ( eType == WINDOW_DIALOG ) || ( eType == WINDOW_MODALDIALOG ) || ( eType == WINDOW_MODELESSDIALOG ) )
        {
            pPeer = new VCLXDialog;
        }
        else
        {
            pPeer = new VCLXWindow;
        }
        xPeer = pPeer;
        SetWindowInterface( pWindow, xPeer );
    }
    return xPeer;
}

void UnoWrapper::SetWindowInterface( Window* pWindow, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xIFace )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( xIFace );

    DBG_ASSERT( pVCLXWindow, "SetComponentInterface - unsupported type" );
    if ( pVCLXWindow )
    {
        pVCLXWindow->SetWindow( pWindow );
        pWindow->SetWindowPeer( xIFace, pVCLXWindow );
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics> UnoWrapper::CreateGraphics( OutputDevice* pOutDev )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics> xGrf;
    VCLXGraphics* pGrf = new VCLXGraphics;
    xGrf = pGrf;
    pGrf->Init( pOutDev );
    return xGrf;
}

void UnoWrapper::ReleaseAllGraphics( OutputDevice* pOutDev )
{
    List* pLst = pOutDev->GetUnoGraphicsList();
    if ( pLst )
    {
        for ( sal_uInt32 n = 0; n < pLst->Count(); n++ )
        {
            VCLXGraphics* pGrf = (VCLXGraphics*)pLst->GetObject( n );
            pGrf->SetOutputDevice( NULL );
        }
    }

}

// MT: Wurde im Window-CTOR gerufen, damit Container-Listener
// vom Parent reagieren, aber hat sowieso nicht richtig funktioniert,
// weil im Window-CTOR das Interface noch nicht da ist!
// => Nur Listener rufen, wenn ueber das ::com::sun::star::awt::Toolkit erzeugt

/*
void ImplSmartWindowCreated( Window* pNewWindow )
{
    UNOWindowData* pParentUNOData = pNewWindow->GetParent() ?
                            pNewWindow->GetParent()->GetUNOData() : NULL;

    if ( pParentUNOData && pParentUNOData->GetListeners( EL_CONTAINER ) )
    {
        UNOWindowData* pUNOData = pNewWindow->GetUNOData();
        if ( !pUNOData )
            pUNOData = ImplSmartCreateUNOData( pNewWindow );

        ::com::sun::star::awt::VclContainerEvent aEvent;
        aEvent.Source = (UsrObject*)pParentUNOData->GetWindowPeer();
        aEvent.Id = VCLCOMPONENT_ADDED;
        aEvent.Child = (UsrObject*)pUNOData->GetWindowPeer();

        EventList* pLst = pParentUNOData->GetListeners( EL_CONTAINER );
        for ( sal_uInt32 n = 0; n < pLst->Count(); n++ )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > * pRef = pLst->GetObject( n );
            ((::com::sun::star::awt::XVclContainerListener*)(::com::sun::star::lang::XEventListener*)*pRef)->windowAdded( aEvent );
        }
    }
}
*/

sal_Bool lcl_ImplIsParent( Window* pParentWindow, Window* pPossibleChild )
{
    Window* pWindow = pPossibleChild;
    while ( pWindow && ( pWindow != pParentWindow ) )
        pWindow = pWindow->GetParent();

    return pWindow ? sal_True : sal_False;
}

void UnoWrapper::WindowDestroyed( Window* pWindow )
{
    // ggf. existieren noch von ::com::sun::star::loader::Java erzeugte Childs, die sonst erst
    // im Garbage-Collector zerstoert werden...
    Window* pChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        Window* pNextChild = pChild->GetWindow( WINDOW_NEXT );

        Window* pClient = pChild->GetWindow( WINDOW_CLIENT );
        if ( pClient->GetWindowPeer() )
            pClient->GetWindowPeer()->dispose();

        pChild = pNextChild;
    }

    // ::com::sun::star::chaos::System-Windows suchen...
    Window* pOverlap = pWindow->GetWindow( WINDOW_OVERLAP );
    pOverlap = pOverlap->GetWindow( WINDOW_FIRSTOVERLAP );
    while ( pOverlap )
    {
        Window* pNextOverlap = pOverlap->GetWindow( WINDOW_NEXT );
        Window* pClient = pOverlap->GetWindow( WINDOW_CLIENT );

        if ( pClient->GetWindowPeer() && lcl_ImplIsParent( pWindow, pClient ) )
            pClient->GetWindowPeer()->dispose();

        pOverlap = pNextOverlap;
    }

    Window* pParent = pWindow->GetParent();
    if ( pParent && pParent->GetWindowPeer() && pParent->GetWindowPeer()->GetContainerListeners().getLength() )
    {
        ::com::sun::star::awt::VclContainerEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pParent->GetWindowPeer();
        aEvent.Child = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        pParent->GetWindowPeer()->GetContainerListeners().windowRemoved( aEvent );
    }

    if ( pWindow && pWindow->GetWindowPeer() )
    {
        pWindow->GetWindowPeer()->SetWindow( NULL );
        pWindow->SetWindowPeer( NULL, NULL );
    }
}

void UnoWrapper::WindowEvent_Move( Window* pWindow )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetWindowListeners().getLength() )
    {
        ::com::sun::star::awt::WindowEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        ImplInitWindowEvent( aEvent, pWindow );
        pWindow->GetWindowPeer()->GetWindowListeners().windowMoved( aEvent );
    }
}

void UnoWrapper::WindowEvent_Resize( Window* pWindow )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetWindowListeners().getLength() )
    {
        ::com::sun::star::awt::WindowEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        ImplInitWindowEvent( aEvent, pWindow );
        pWindow->GetWindowPeer()->GetWindowListeners().windowResized( aEvent );
    }
}

void UnoWrapper::WindowEvent_Show( Window* pWindow, sal_Bool bShow )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetWindowListeners().getLength() )
    {
        ::com::sun::star::awt::WindowEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        ImplInitWindowEvent( aEvent, pWindow );

        if ( bShow )
            pWindow->GetWindowPeer()->GetWindowListeners().windowShown( aEvent );
        else
            pWindow->GetWindowPeer()->GetWindowListeners().windowHidden( aEvent );
    }

    // Bei TopWindows hat das die Bedeutung von Opened, Closed...
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetTopWindowListeners().getLength() )
    {
        ::com::sun::star::lang::EventObject aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();

        if ( bShow )
            pWindow->GetWindowPeer()->GetTopWindowListeners().windowOpened( aEvent );
        else
            pWindow->GetWindowPeer()->GetTopWindowListeners().windowClosed( aEvent );
    }
}

void UnoWrapper::WindowEvent_Activate( Window* pWindow, sal_Bool bActivated )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetTopWindowListeners().getLength() )
    {
        ::com::sun::star::lang::EventObject aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();

        if ( bActivated )
            pWindow->GetWindowPeer()->GetTopWindowListeners().windowActivated( aEvent );
        else
            pWindow->GetWindowPeer()->GetTopWindowListeners().windowDeactivated( aEvent );
    }
}

void UnoWrapper::WindowEvent_MouseButtonDown( Window* pWindow, const MouseEvent& rEvt )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetMouseListeners().getLength() )
    {
        ::com::sun::star::awt::MouseEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        ImplInitMouseEvent( aEvent, rEvt );
        pWindow->GetWindowPeer()->GetMouseListeners().mousePressed( aEvent );
    }
}

void UnoWrapper::WindowEvent_MouseButtonUp( Window* pWindow, const MouseEvent& rEvt )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetMouseListeners().getLength() )
    {
        ::com::sun::star::awt::MouseEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        ImplInitMouseEvent( aEvent, rEvt );
        pWindow->GetWindowPeer()->GetMouseListeners().mouseReleased( aEvent );
    }
}

void UnoWrapper::WindowEvent_Command( Window* pWindow, const CommandEvent& rEvt )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetMouseListeners().getLength() &&
            rEvt.IsMouseEvent() && ( rEvt.GetCommand() == COMMAND_CONTEXTMENU ) )
    {
        // COMMAND_CONTEXTMENU als mousePressed mit PopupTrigger = sal_True versenden...
        MouseEvent aMEvt( rEvt.GetMousePosPixel(), 1, MOUSE_SIMPLECLICK, MOUSE_LEFT, 0 );
        ::com::sun::star::awt::MouseEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        ImplInitMouseEvent( aEvent, aMEvt );
        aEvent.PopupTrigger = sal_True;
        pWindow->GetWindowPeer()->GetMouseListeners().mousePressed( aEvent );
    }
}

void UnoWrapper::WindowEvent_MouseMove( Window* pWindow, const MouseEvent& rEvt )
{
    if ( pWindow->GetWindowPeer() )
    {
        if ( pWindow->GetWindowPeer()->GetMouseListeners().getLength() && ( rEvt.IsEnterWindow() || rEvt.IsLeaveWindow() ) )
        {
            ::com::sun::star::awt::MouseEvent aEvent;
            aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
            ImplInitMouseEvent( aEvent, rEvt );

            if ( rEvt.IsEnterWindow() )
                pWindow->GetWindowPeer()->GetMouseListeners().mouseEntered( aEvent );
            else
                pWindow->GetWindowPeer()->GetMouseListeners().mouseExited( aEvent );
        }

        if ( pWindow->GetWindowPeer()->GetMouseMotionListeners().getLength() && !rEvt.IsEnterWindow() && !rEvt.IsLeaveWindow() )
        {
            ::com::sun::star::awt::MouseEvent aEvent;
            aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
            ImplInitMouseEvent( aEvent, rEvt );

            if ( rEvt.GetMode() & MOUSE_SIMPLEMOVE )
                pWindow->GetWindowPeer()->GetMouseMotionListeners().mouseMoved( aEvent );
            else
                pWindow->GetWindowPeer()->GetMouseMotionListeners().mouseDragged( aEvent );
        }
    }
}

void UnoWrapper::WindowEvent_KeyInput( Window* pWindow, const KeyEvent& rEvt )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetKeyListeners().getLength() )
    {
        ::com::sun::star::awt::KeyEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        ImplInitKeyEvent( aEvent, rEvt );
        pWindow->GetWindowPeer()->GetKeyListeners().keyPressed( aEvent );
    }
}

void UnoWrapper::WindowEvent_KeyUp( Window* pWindow, const KeyEvent& rEvt )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetKeyListeners().getLength() )
    {
        ::com::sun::star::awt::KeyEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        ImplInitKeyEvent( aEvent, rEvt );
        pWindow->GetWindowPeer()->GetKeyListeners().keyReleased( aEvent );
    }
}

void UnoWrapper::WindowEvent_GetFocus( Window* pWindow )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetFocusListeners().getLength() )
    {
        ::com::sun::star::awt::FocusEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        aEvent.FocusFlags = pWindow->GetGetFocusFlags();
        aEvent.Temporary = sal_False;
        pWindow->GetWindowPeer()->GetFocusListeners().focusGained( aEvent );
    }
}

void UnoWrapper::WindowEvent_LoseFocus( Window* pWindow )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetFocusListeners().getLength() )
    {
        ::com::sun::star::awt::FocusEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        aEvent.FocusFlags = pWindow->GetGetFocusFlags();
        aEvent.Temporary = sal_False;

        Window* pNext = Application::GetFocusWindow();
        if ( pNext )
        {
            // Bei zusammengesetzten Controls interessiert sich keiner fuer das Innenleben:
            Window* pNextC = pNext;
            while ( pNextC && !pNextC->IsCompoundControl() )
                pNextC = pNextC->GetParent();
            if ( pNextC )
                pNext = pNextC;

            pNext->GetComponentInterface( sal_True );
            aEvent.NextFocus = (::cppu::OWeakObject*)pNext->GetWindowPeer();
        }
        pWindow->GetWindowPeer()->GetFocusListeners().focusLost( aEvent );
    }
}

void UnoWrapper::WindowEvent_Paint( Window* pWindow, const Rectangle& rRect )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetPaintListeners().getLength() )
    {
        ::com::sun::star::awt::PaintEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        aEvent.UpdateRect = AWTRectangle( rRect );
        aEvent.Count = 0;
        pWindow->GetWindowPeer()->GetPaintListeners().windowPaint( aEvent );
    }
}

void UnoWrapper::WindowEvent_Close( Window* pWindow )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetTopWindowListeners().getLength() )
    {
        ::com::sun::star::lang::EventObject aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        pWindow->GetWindowPeer()->GetTopWindowListeners().windowClosing( aEvent );
    }
}

void UnoWrapper::WindowEvent_Minimize( Window* pWindow )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetTopWindowListeners().getLength() )
    {
        ::com::sun::star::lang::EventObject aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        pWindow->GetWindowPeer()->GetTopWindowListeners().windowMinimized( aEvent );
    }
}

void UnoWrapper::WindowEvent_Normalize( Window* pWindow )
{
    if ( pWindow->GetWindowPeer() && pWindow->GetWindowPeer()->GetTopWindowListeners().getLength() )
    {
        ::com::sun::star::lang::EventObject aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        pWindow->GetWindowPeer()->GetTopWindowListeners().windowNormalized( aEvent );
    }
}
