/*************************************************************************
 *
 *  $RCSfile: vclxwindow.cxx,v $
 *
 *  $Revision: 1.42 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 07:43:35 $
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
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_STYLE_HPP_
#include <com/sun/star/awt/Style.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXPOINTER_HXX_
#include <toolkit/awt/vclxpointer.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLESTATUSBAR_HXX_
#include <toolkit/awt/vclxaccessiblestatusbar.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLETABCONTROL_HXX_
#include <toolkit/awt/vclxaccessibletabcontrol.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLETABPAGEWINDOW_HXX_
#include <toolkit/awt/vclxaccessibletabpagewindow.hxx>
#endif
#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include <toolkit/helper/property.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

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
//  class VCLXWindow
//  ----------------------------------------------------

DBG_NAME(VCLXWindow);

VCLXWindow::VCLXWindow()
    : maEventListeners( *this ),
      maFocusListeners( *this ),
      maWindowListeners( *this ),
      maKeyListeners( *this ),
      maMouseListeners( *this ),
      maMouseMotionListeners( *this ),
      maPaintListeners( *this ),
      maContainerListeners( *this ),
      maTopWindowListeners( *this ),
      mnListenerLockLevel( 0 )
{
    DBG_CTOR( VCLXWindow, 0 );

    mbDisposing = sal_False;
    mbDesignMode = sal_False;
    mbSynthesizingVCLEvent = sal_False;
}

VCLXWindow::~VCLXWindow()
{
    DBG_DTOR( VCLXWindow, 0 );

    if ( GetWindow() )
    {
        GetWindow()->RemoveEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
        GetWindow()->SetWindowPeer( NULL, NULL );
        GetWindow()->SetAccessible( NULL );
    }
}

void VCLXWindow::SetWindow( Window* pWindow )
{
    if ( GetWindow() )
    {
        GetWindow()->RemoveEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
//        GetWindow()->DbgAssertNoEventListeners();
    }

    SetOutputDevice( pWindow );

    if ( GetWindow() )
    GetWindow()->AddEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
}

void VCLXWindow::suspendVclEventListening( )
{
    ++mnListenerLockLevel;
}

void VCLXWindow::resumeVclEventListening( )
{
    DBG_ASSERT( mnListenerLockLevel, "VCLXWindow::resumeVclEventListening: not suspended!" );
    --mnListenerLockLevel;
}

IMPL_LINK( VCLXWindow, WindowEventListener, VclSimpleEvent*, pEvent )
{
    if ( mnListenerLockLevel )
        return 0L;

    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow() && GetWindow(), "Window???" );
        ProcessWindowEvent( *(VclWindowEvent*)pEvent );
    }
    return 0;
}

void VCLXWindow::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_PAINT:
        {
            if ( GetPaintListeners().getLength() )
            {
                ::com::sun::star::awt::PaintEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.UpdateRect = AWTRectangle( *(Rectangle*)rVclWindowEvent.GetData() );
                aEvent.Count = 0;
                GetPaintListeners().windowPaint( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOVE:
        {
            if ( GetWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                GetWindowListeners().windowMoved( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_RESIZE:
        {
            if ( GetWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                GetWindowListeners().windowResized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_SHOW:
        {
            if ( GetWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                GetWindowListeners().windowShown( aEvent );
            }

            // For TopWindows this means opened...
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowOpened( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_HIDE:
        {
            if ( GetWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                GetWindowListeners().windowHidden( aEvent );
            }

            // For TopWindows this means closed...
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowClosed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_ACTIVATE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowActivated( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_DEACTIVATE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowDeactivated( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_CLOSE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowClosing( aEvent );
            }
        }
        break;
        case VCLEVENT_CONTROL_GETFOCUS:
        case VCLEVENT_WINDOW_GETFOCUS:
        {
            if  (   (   rVclWindowEvent.GetWindow()->IsCompoundControl()
                    &&  rVclWindowEvent.GetId() == VCLEVENT_CONTROL_GETFOCUS
                    )
                ||  (   !rVclWindowEvent.GetWindow()->IsCompoundControl()
                    &&  rVclWindowEvent.GetId() == VCLEVENT_WINDOW_GETFOCUS
                    )
                )
            {
                if ( GetFocusListeners().getLength() )
                {
                    ::com::sun::star::awt::FocusEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.FocusFlags = rVclWindowEvent.GetWindow()->GetGetFocusFlags();
                    aEvent.Temporary = sal_False;
                    GetFocusListeners().focusGained( aEvent );
                }
            }
        }
        break;
        case VCLEVENT_CONTROL_LOSEFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
        {
            if  (   (   rVclWindowEvent.GetWindow()->IsCompoundControl()
                    &&  rVclWindowEvent.GetId() == VCLEVENT_CONTROL_LOSEFOCUS
                    )
                ||  (   !rVclWindowEvent.GetWindow()->IsCompoundControl()
                    &&  rVclWindowEvent.GetId() == VCLEVENT_WINDOW_LOSEFOCUS
                    )
                )
            {
                if ( GetFocusListeners().getLength() )
                {
                    ::com::sun::star::awt::FocusEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.FocusFlags = rVclWindowEvent.GetWindow()->GetGetFocusFlags();
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
                    GetFocusListeners().focusLost( aEvent );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_MINIMIZE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowMinimized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_NORMALIZE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowNormalized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_KEYINPUT:
        {
            if ( GetKeyListeners().getLength() )
            {
                ::com::sun::star::awt::KeyEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitKeyEvent( aEvent, *(KeyEvent*)rVclWindowEvent.GetData() );
                GetKeyListeners().keyPressed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_KEYUP:
        {
            if ( GetKeyListeners().getLength() )
            {
                ::com::sun::star::awt::KeyEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitKeyEvent( aEvent, *(KeyEvent*)rVclWindowEvent.GetData() );
                GetKeyListeners().keyReleased( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_COMMAND:
        {
            CommandEvent* pCmdEvt = (CommandEvent*)rVclWindowEvent.GetData();
            if ( GetMouseListeners().getLength() && ( pCmdEvt->GetCommand() == COMMAND_CONTEXTMENU ) )
            {
                // COMMAND_CONTEXTMENU als mousePressed mit PopupTrigger = sal_True versenden...
                Point aWhere = static_cast< CommandEvent* >( rVclWindowEvent.GetData() )->GetMousePosPixel();
                if ( !pCmdEvt->IsMouseEvent() )
                {   // for keyboard events, we set the coordinates to -1,-1. This is a slight HACK, but the current API
                    // handles a context menu command as special case of a mouse event, which is simply wrong.
                    // Without extending the API, we would not have another chance to notify listeners of a
                    // keyboard-triggered context menu request
                    // 102205 - 16.08.2002 - fs@openoffice.org
                    aWhere = Point( -1, -1 );
                }

                MouseEvent aMEvt( aWhere, 1, MOUSE_SIMPLECLICK, MOUSE_LEFT, 0 );
                ::com::sun::star::awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, aMEvt );
                aEvent.PopupTrigger = sal_True;
                GetMouseListeners().mousePressed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEMOVE:
        {
            MouseEvent* pMouseEvt = (MouseEvent*)rVclWindowEvent.GetData();
            if ( GetMouseListeners().getLength() && ( pMouseEvt->IsEnterWindow() || pMouseEvt->IsLeaveWindow() ) )
            {
                ::com::sun::star::awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, *pMouseEvt );

                if ( pMouseEvt->IsEnterWindow() )
                    GetMouseListeners().mouseEntered( aEvent );
                else
                    GetMouseListeners().mouseExited( aEvent );
            }

            if ( GetMouseMotionListeners().getLength() && !pMouseEvt->IsEnterWindow() && !pMouseEvt->IsLeaveWindow() )
            {
                ::com::sun::star::awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, *pMouseEvt );
                aEvent.ClickCount = 0;  // #92138#

                if ( pMouseEvt->GetMode() & MOUSE_SIMPLEMOVE )
                    GetMouseMotionListeners().mouseMoved( aEvent );
                else
                    GetMouseMotionListeners().mouseDragged( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEBUTTONDOWN:
        {
            if ( GetMouseListeners().getLength() )
            {
                ::com::sun::star::awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, *(MouseEvent*)rVclWindowEvent.GetData() );
                GetMouseListeners().mousePressed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEBUTTONUP:
        {
            if ( GetMouseListeners().getLength() )
            {
                ::com::sun::star::awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, *(MouseEvent*)rVclWindowEvent.GetData() );
                GetMouseListeners().mouseReleased( aEvent );
            }
        }
        break;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXWindow::CreateAccessibleContext()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > xContext;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        WindowType nType = pWindow->GetType();

        if ( nType == WINDOW_MENUBARWINDOW || pWindow->IsMenuFloatingWindow() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc( pWindow->GetAccessible() );
            if ( xAcc.is() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > xCont( xAcc->getAccessibleContext() );
                if ( pWindow->GetType() == WINDOW_MENUBARWINDOW
                     || ( xCont.is() && xCont->getAccessibleRole() == ::com::sun::star::accessibility::AccessibleRole::POPUP_MENU ) )
                {
                    xContext = xCont;
                }
            }
        }
        else if ( nType == WINDOW_STATUSBAR )
        {
            xContext = (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleStatusBar( this );
        }
        else if ( nType == WINDOW_TABCONTROL )
        {
            xContext = (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleTabControl( this );
        }
        else if ( nType == WINDOW_TABPAGE && pWindow->GetAccessibleParentWindow() && pWindow->GetAccessibleParentWindow()->GetType() == WINDOW_TABCONTROL )
        {
            xContext = (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleTabPageWindow( this );
        }
        else
        {
            xContext = (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleComponent( this );
        }
    }

    return xContext;
}

/*
void VCLXWindow::FillAccessibleStateSet( AccessibleStateSetHelper& rStateSet )
{
}
*/

Size VCLXWindow::ImplCalcWindowSize( const Size& rOutSz ) const
{
    Size aSz = rOutSz;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        sal_Int32 nLeft, nTop, nRight, nBottom;
        pWindow->GetBorder( nLeft, nTop, nRight, nBottom );
        aSz.Width() += nLeft+nRight;
        aSz.Height() += nTop+nBottom;
    }
    return aSz;
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXWindow::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XComponent*, (::com::sun::star::awt::XWindow*)this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XWindow*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XWindowPeer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XVclWindowPeer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XView*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::accessibility::XAccessible*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, this ) );
    return (aRet.hasValue() ? aRet : VCLXDevice::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL2( VCLXWindow, VCLXDevice )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXWindow )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView>* ) NULL ),
    VCLXDevice::getTypes()
IMPL_XTYPEPROVIDER_END


// ::com::sun::star::lang::Component
void VCLXWindow::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    mxViewGraphics = NULL;

    if ( !mbDisposing )
    {
        mbDisposing = sal_True;

        ::com::sun::star::lang::EventObject aObj;
        aObj.Source = static_cast< ::cppu::OWeakObject* >( this );

        maEventListeners.disposeAndClear( aObj );
        maFocusListeners.disposeAndClear( aObj );
        maWindowListeners.disposeAndClear( aObj );
        maKeyListeners.disposeAndClear( aObj );
        maMouseListeners.disposeAndClear( aObj );
        maMouseMotionListeners.disposeAndClear( aObj );
        maPaintListeners.disposeAndClear( aObj );
        maContainerListeners.disposeAndClear( aObj );
        maTopWindowListeners.disposeAndClear( aObj );

        if ( GetWindow() )
        {
            OutputDevice* pOutDev = GetOutputDevice();
            SetWindow( NULL );  // Damit ggf. Handler abgemeldet werden (virtuell).
            SetOutputDevice( pOutDev );
            DestroyOutputDevice();
        }

        // #i14103# dispose the accessible context after the window has been destroyed,
        // otherwise the old value in the child event fired in VCLXAccessibleComponent::ProcessWindowEvent()
        // for VCLEVENT_WINDOW_CHILDDESTROYED contains a reference to an already disposed accessible object
        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComponent( mxAccessibleContext, ::com::sun::star::uno::UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
            DBG_ERROR( "VCLXWindow::dispose: could not dispose the accessible context!" );
        }
        mxAccessibleContext.clear();

        mbDisposing = sal_False;
    }
}

void VCLXWindow::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetEventListeners().addInterface( rxListener );
}

void VCLXWindow::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetEventListeners().removeInterface( rxListener );
}


// ::com::sun::star::awt::XWindow
void VCLXWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->SetPosSizePixel( X, Y, Width, Height, Flags );
}

::com::sun::star::awt::Rectangle VCLXWindow::getPosSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Rectangle aBounds;
    if ( GetWindow() )
        aBounds = AWTRectangle( Rectangle( GetWindow()->GetPosPixel(), GetWindow()->GetSizePixel() ) );

    return aBounds;
}

void VCLXWindow::setVisible( sal_Bool bVisible ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
/*
        if ( bVisible )
        {
            // #57167# TopWindows mit unsichtbaren Parent anzeigen...
            ::com::sun::star::uno::Any aTest = queryInterface( ::getCppuType(  (const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindow >*)  0 ) );
            if ( aTest.hasValue() )
            {
                Window* pParent = pWindow->GetWindow( WINDOW_PARENTOVERLAP );
                if ( pParent && !pParent->IsReallyVisible() )
                    pWindow->SetParent( pWindow->GetWindow( WINDOW_FRAME ) );
            }
        }
*/
        pWindow->Show( bVisible );
    }
}

void VCLXWindow::setEnable( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->Enable( bEnable, FALSE ); // #95824# without children!
        pWindow->EnableInput( bEnable );
    }
}

void VCLXWindow::setFocus(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->GrabFocus();
}

void VCLXWindow::addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetWindowListeners().addInterface( rxListener );

    // #100119# Get all resize events, even if height or width 0, or invisible
    if ( GetWindow() )
        GetWindow()->EnableAllResize( TRUE );
}

void VCLXWindow::removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetWindowListeners().removeInterface( rxListener );
}

void VCLXWindow::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetFocusListeners().addInterface( rxListener );
}

void VCLXWindow::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetFocusListeners().removeInterface( rxListener );
}

void VCLXWindow::addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetKeyListeners().addInterface( rxListener );
}

void VCLXWindow::removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetKeyListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseMotionListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseMotionListeners().removeInterface( rxListener );
}

void VCLXWindow::addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetPaintListeners().addInterface( rxListener );
}

void VCLXWindow::removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetPaintListeners().removeInterface( rxListener );
}

// ::com::sun::star::awt::XWindowPeer
::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > VCLXWindow::getToolkit(  ) throw(::com::sun::star::uno::RuntimeException)
{
    // no guard. nothing to guard here.
    // 82463 - 12/21/00 - fs
    return Application::GetVCLToolkit();
}

void VCLXWindow::setPointer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer >& rxPointer ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    VCLXPointer* pPointer = VCLXPointer::GetImplementation( rxPointer );
    if ( pPointer )
    {
        mxPointer = rxPointer;
        if ( GetWindow() )
            GetWindow()->SetPointer( pPointer->GetPointer() );
    }
}

void VCLXWindow::setBackground( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        Color aColor( (sal_uInt32)nColor );
        GetWindow()->SetBackground( aColor );
        GetWindow()->SetControlBackground( aColor );

        WindowType eWinType = GetWindow()->GetType();
        if ( ( eWinType == WINDOW_WINDOW ) ||
             ( eWinType == WINDOW_WORKWINDOW ) ||
             ( eWinType == WINDOW_FLOATINGWINDOW ) )
        {
            GetWindow()->Invalidate();
        }
    }
}

void VCLXWindow::invalidate( sal_Int16 nFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->Invalidate( (sal_uInt16) nFlags );
}

void VCLXWindow::invalidateRect( const ::com::sun::star::awt::Rectangle& rRect, sal_Int16 nFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->Invalidate( VCLRectangle(rRect), (sal_uInt16) nFlags );
}


// ::com::sun::star::awt::XVclWindowPeer
sal_Bool VCLXWindow::isChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rxPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Bool bIsChild = sal_False;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Window* pPeerWindow = VCLUnoHelper::GetWindow( rxPeer );
        bIsChild = pPeerWindow && pWindow->IsChild( pPeerWindow );
    }

    return bIsChild;
}

void VCLXWindow::setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    mbDesignMode = bOn;
}

sal_Bool VCLXWindow::isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    return mbDesignMode;
}

void VCLXWindow::enableClipSiblings( sal_Bool bClip ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->EnableClipSiblings( bClip );
}

void VCLXWindow::setForeground( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        Color aColor( (sal_uInt32)nColor );
        GetWindow()->SetControlForeground( aColor );
    }
}

void VCLXWindow::setControlFont( const ::com::sun::star::awt::FontDescriptor& rFont ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->SetControlFont( VCLUnoHelper::CreateFont( rFont, GetWindow()->GetControlFont() ) );
}

void VCLXWindow::getStyles( sal_Int16 nType, ::com::sun::star::awt::FontDescriptor& Font, sal_Int32& ForegroundColor, sal_Int32& BackgroundColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        const StyleSettings& rStyleSettings = GetWindow()->GetSettings().GetStyleSettings();

        switch ( nType )
        {
            case ::com::sun::star::awt::Style::FRAME:
            {
                Font = VCLUnoHelper::CreateFontDescriptor( rStyleSettings.GetAppFont() );
                ForegroundColor = rStyleSettings.GetWindowTextColor().GetColor();
                BackgroundColor = rStyleSettings.GetWindowColor().GetColor();
            }
            break;
            case ::com::sun::star::awt::Style::DIALOG:
            {
                Font = VCLUnoHelper::CreateFontDescriptor( rStyleSettings.GetAppFont() );
                ForegroundColor = rStyleSettings.GetDialogTextColor().GetColor();
                BackgroundColor = rStyleSettings.GetDialogColor().GetColor();
            }
            break;
            default: DBG_ERROR( "VCLWindow::getStyles() - unknown Type" );
        }

    }
}

void VCLXWindow::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        WindowType eWinType = pWindow->GetType();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
             case BASEPROPERTY_PLUGINPARENT:
             {
                // correct data type?
                 sal_Int64 nHandle;
                 if ( ! (Value >>= nHandle) )
                {
                    ::com::sun::star::uno::Exception *pException =
                        new ::com::sun::star::uno::RuntimeException;
                    pException->Message = ::rtl::OUString::createFromAscii( "incorrect window handle type" );
                    throw pException;
                }

                // set parent handle
                SetSystemParent_Impl( nHandle);
             }
             break;

            case BASEPROPERTY_ENABLED:
            {
                sal_Bool b;
                if ( Value >>= b )
                    setEnable( b );
            }
            break;
            case BASEPROPERTY_TEXT:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TITLE:
            {
                ::rtl::OUString aText;
                if ( Value >>= aText )
                    pWindow->SetText( aText );
            }
            break;
            case BASEPROPERTY_ACCESSIBLENAME:
            {
                ::rtl::OUString aText;
                if ( Value >>= aText )
                    pWindow->SetAccessibleName( aText );
            }
            break;
            case BASEPROPERTY_HELPURL:
            {
                ::rtl::OUString aURL;
                if ( Value >>= aURL )
                {
                    String aHelpURL(  aURL );
                    String aPattern( RTL_CONSTASCII_USTRINGPARAM( "HID:" ) );
                    if ( aHelpURL.CompareIgnoreCaseToAscii( aPattern, aPattern.Len() ) == COMPARE_EQUAL )
                    {
                        String aID = aHelpURL.Copy( aPattern.Len() );
                        pWindow->SetHelpId( aID.ToInt32() );
                    }
                }
            }
            break;
            case BASEPROPERTY_HELPTEXT:
            {
                ::rtl::OUString aHelpText;
                if ( Value >>= aHelpText )
                {
                    pWindow->SetQuickHelpText( aHelpText );
                }
            }
            break;
            case BASEPROPERTY_FONTDESCRIPTOR:
            {
                if ( bVoid )
                    pWindow->SetControlFont( Font() );
                else
                {
                    ::com::sun::star::awt::FontDescriptor aFont;
                    if ( Value >>= aFont )
                        pWindow->SetControlFont( VCLUnoHelper::CreateFont( aFont, pWindow->GetControlFont() ) );
                }
            }
            break;
            case BASEPROPERTY_FONTRELIEF:
            {
                sal_Int16 n;
                if ( Value >>= n )
                {
                    Font aFont = pWindow->GetControlFont();
                    aFont.SetRelief( (FontRelief)n );
                    pWindow->SetControlFont( aFont );
                }
            }
            break;
            case BASEPROPERTY_FONTEMPHASISMARK:
            {
                sal_Int16 n;
                if ( Value >>= n )
                {
                    Font aFont = pWindow->GetControlFont();
                    aFont.SetEmphasisMark( n );
                    pWindow->SetControlFont( aFont );
                }
            }
            break;
            case BASEPROPERTY_BACKGROUNDCOLOR:
                if ( bVoid )
                {
                    pWindow->SetControlBackground();
                }
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetBackground( aColor );
                        pWindow->SetControlBackground( aColor );
                        pWindow->Invalidate();  // Falls das Control nicht drauf reagiert
                    }
                }
            break;
            case BASEPROPERTY_TEXTCOLOR:
                if ( bVoid )
                {
                    pWindow->SetControlForeground();
                }
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetTextColor( aColor );
                        pWindow->SetControlForeground( aColor );
                    }
                }
            break;
            case BASEPROPERTY_TEXTLINECOLOR:
                if ( bVoid )
                {
                    pWindow->SetTextLineColor();
                }
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetTextLineColor( aColor );
                    }
                }
            break;
            case BASEPROPERTY_FILLCOLOR:
                if ( bVoid )
                    pWindow->SetFillColor();
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetFillColor( aColor );
                    }
                }
            break;
            case BASEPROPERTY_LINECOLOR:
                if ( bVoid )
                    pWindow->SetLineColor();
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetLineColor( aColor );
                    }
                }
            break;
            case BASEPROPERTY_BORDER:
            {
                WinBits nStyle = pWindow->GetStyle();
                sal_uInt16 nBorder;
                Value >>= nBorder;
                if ( !nBorder )
                {
                    pWindow->SetStyle( nStyle & ~WB_BORDER );
                }
                else
                {
                    pWindow->SetStyle( nStyle | WB_BORDER );
                    pWindow->SetBorderStyle( nBorder );
                }
            }
            break;
            case BASEPROPERTY_TABSTOP:
            {
                WinBits nStyle = pWindow->GetStyle() & ~WB_TABSTOP;
                if ( !bVoid )
                {
                    sal_Bool bTab;
                    Value >>= bTab;
                    if ( bTab )
                        nStyle |= WB_TABSTOP;
                    else
                        nStyle |= WB_NOTABSTOP;
                }
                pWindow->SetStyle( nStyle );
            }
            break;
            case BASEPROPERTY_ALIGN:
            {
                switch ( eWinType )
                {
                    case WINDOW_FIXEDTEXT:
                    case WINDOW_EDIT:
                    {
                        WinBits nStyle = pWindow->GetStyle();
                        nStyle &= ~(WB_LEFT|WB_CENTER|WB_RIGHT);
                        sal_uInt16 nAlign = 0;
                        if ( !bVoid )
                            Value >>= nAlign;
                        if ( nAlign == PROPERTY_ALIGN_LEFT )
                            nStyle |= WB_LEFT;
                        else if ( nAlign == PROPERTY_ALIGN_CENTER )
                            nStyle |= WB_CENTER;
                        else
                            nStyle |= WB_RIGHT;
                        pWindow->SetStyle( nStyle );
                    }
                    break;
                }
            }
            break;
            case BASEPROPERTY_MULTILINE:
            {
                if ( eWinType == WINDOW_FIXEDTEXT )
                {
                    WinBits nStyle = pWindow->GetStyle();
                    sal_Bool bMulti;
                    Value >>= bMulti;
                    if ( bMulti )
                        nStyle |= WB_WORDBREAK;
                    else
                        nStyle &= ~WB_WORDBREAK;
                    pWindow->SetStyle( nStyle );
                }
            }
            break;
            case BASEPROPERTY_ORIENTATION:
            {
                switch ( eWinType )
                {
                    case WINDOW_FIXEDLINE:
                    {
                        sal_Int32 nOrientation;
                        if ( Value >>= nOrientation )
                        {
                            WinBits nStyle = pWindow->GetStyle();
                            nStyle &= ~(WB_HORZ|WB_VERT);
                            if ( nOrientation == 0 )
                                nStyle |= WB_HORZ;
                            else
                                nStyle |= WB_VERT;

                            pWindow->SetStyle( nStyle );
                        }
                    }
                    break;
                }
            }
            break;
            case BASEPROPERTY_AUTOMNEMONICS:
            {
                sal_Bool bAutoMnemonics;
                Value >>= bAutoMnemonics;
                AllSettings aSettings = pWindow->GetSettings();
                StyleSettings aStyleSettings = aSettings.GetStyleSettings();
                if ( aStyleSettings.GetAutoMnemonic() != bAutoMnemonics )
                {
                    aStyleSettings.SetAutoMnemonic( bAutoMnemonics );
                    aSettings.SetStyleSettings( aStyleSettings );
                    pWindow->SetSettings( aSettings );
                }
            }
            break;
            case BASEPROPERTY_MOUSETRANSPARENT:
            {
                sal_Bool bMouseTransparent;
                Value >>= bMouseTransparent;
                pWindow->SetMouseTransparent( bMouseTransparent );
            }
            break;
        }
    }
}

::com::sun::star::uno::Any VCLXWindow::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    if ( GetWindow() )
    {
        WindowType eWinType = GetWindow()->GetType();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_ENABLED:
                aProp <<= (sal_Bool) GetWindow()->IsEnabled();
            break;
            case BASEPROPERTY_TEXT:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TITLE:
            {
                ::rtl::OUString aText = GetWindow()->GetText();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_ACCESSIBLENAME:
            {
                ::rtl::OUString aText = GetWindow()->GetAccessibleName();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_HELPTEXT:
            {
                ::rtl::OUString aText = GetWindow()->GetQuickHelpText();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_FONTDESCRIPTOR:
            {
                Font aFont = GetWindow()->GetControlFont();
                ::com::sun::star::awt::FontDescriptor aFD = VCLUnoHelper::CreateFontDescriptor( aFont );
                aProp <<= aFD;
            }
            break;
            case BASEPROPERTY_BACKGROUNDCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetControlBackground().GetColor();
            break;
            case BASEPROPERTY_DISPLAYBACKGROUNDCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetDisplayBackground().GetColor().GetColor();
            break;
            case BASEPROPERTY_FONTRELIEF:
                aProp <<= (sal_Int16) GetWindow()->GetControlFont().GetRelief();
            break;
            case BASEPROPERTY_FONTEMPHASISMARK:
                aProp <<= (sal_Int16) GetWindow()->GetControlFont().GetEmphasisMark();
            break;
            case BASEPROPERTY_TEXTCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetControlForeground().GetColor();
            break;
            case BASEPROPERTY_TEXTLINECOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetTextLineColor().GetColor();
            break;
            case BASEPROPERTY_FILLCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetFillColor().GetColor();
            break;
            case BASEPROPERTY_LINECOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetLineColor().GetColor();
            break;
            case BASEPROPERTY_BORDER:
            {
                sal_uInt16 nBorder = 0;
                if ( GetWindow()->GetStyle() & WB_BORDER )
                    nBorder = GetWindow()->GetBorderStyle();
                aProp <<= nBorder;
            }
            break;
            case BASEPROPERTY_TABSTOP:
                aProp <<= (sal_Bool) ( GetWindow()->GetStyle() & WB_TABSTOP ) ? sal_True : sal_False;
            break;
            case BASEPROPERTY_ALIGN:
            {
                switch ( eWinType )
                {
                    case WINDOW_FIXEDTEXT:
                    case WINDOW_EDIT:
                    {
                        WinBits nStyle = GetWindow()->GetStyle();
                        if ( nStyle & WB_LEFT )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_LEFT;
                        else if ( nStyle & WB_CENTER )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_CENTER;
                        else if ( nStyle & WB_RIGHT )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_RIGHT;
                    }
                    break;
                }
            }
            case BASEPROPERTY_MULTILINE:
            {
                if ( eWinType == WINDOW_FIXEDTEXT )
                    aProp <<= (sal_Bool) ( GetWindow()->GetStyle() & WB_WORDBREAK ) ? sal_True : sal_False;
            }
            break;
            case BASEPROPERTY_AUTOMNEMONICS:
            {
                sal_Bool bAutoMnemonics = GetWindow()->GetSettings().GetStyleSettings().GetAutoMnemonic();
                aProp <<= bAutoMnemonics;
            }
            break;
            case BASEPROPERTY_MOUSETRANSPARENT:
            {
                sal_Bool bMouseTransparent = GetWindow()->IsMouseTransparent();
                aProp <<= bMouseTransparent;
            }
            break;

        }
    }
    return aProp;
}


// ::com::sun::star::awt::XLayoutConstrains
::com::sun::star::awt::Size VCLXWindow::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    // Diese Methode sollte nur fuer Componenten gerufen werden, die zwar
    // ueber das ::com::sun::star::awt::Toolkit erzeugt werden koennen, aber fuer die es
    // kein Interface gibt.

    Size aSz;
    if ( GetWindow() )
    {
        WindowType nWinType = GetWindow()->GetType();
        WinBits nStyle = GetWindow()->GetStyle();
        switch ( nWinType )
        {
            case WINDOW_MESSBOX:
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX:
                aSz = Size( 250, 100 );
            break;

            case WINDOW_CONTROL:
                aSz.Width() = GetWindow()->GetTextWidth( GetWindow()->GetText() )+2*12;
                aSz.Height() = GetWindow()->GetTextHeight()+2*6;
            break;

            case WINDOW_PATTERNBOX:
            case WINDOW_NUMERICBOX:
            case WINDOW_METRICBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
            case WINDOW_LONGCURRENCYBOX:
                aSz.Width() = GetWindow()->GetTextWidth( GetWindow()->GetText() )+2*2;
                aSz.Height() = GetWindow()->GetTextHeight()+2*2;
            break;
            case WINDOW_SCROLLBARBOX:
            case WINDOW_SCROLLBAR:
            {
                long n = GetWindow()->GetSettings().GetStyleSettings().GetScrollBarSize();
                aSz = Size( n, n );
            }
            break;
            default:    DBG_ERROR( "getMinimumSize: Unknown Type" );
        }
    }

    return ::com::sun::star::awt::Size( aSz.Width(), aSz.Height() );
}

::com::sun::star::awt::Size VCLXWindow::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXWindow::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aNewSize( rNewSize );
    ::com::sun::star::awt::Size aMinSize = getMinimumSize();

    if ( aNewSize.Width < aMinSize.Width )
        aNewSize.Width = aMinSize.Width;
    if ( aNewSize.Height < aMinSize.Height )
        aNewSize.Height = aMinSize.Height;

    return aNewSize;
}


// ::com::sun::star::awt::XView
sal_Bool VCLXWindow::setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& rxDevice ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( VCLUnoHelper::GetOutputDevice( rxDevice ) )
        mxViewGraphics = rxDevice;
    else
        mxViewGraphics = NULL;

    return mxViewGraphics.is();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > VCLXWindow::getGraphics(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    return mxViewGraphics;
}

::com::sun::star::awt::Size VCLXWindow::getSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    if ( GetWindow() )
        aSz = GetWindow()->GetSizePixel();
    return ::com::sun::star::awt::Size( aSz.Width(), aSz.Height() );
}

void VCLXWindow::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( mxViewGraphics );
        Point aPos( nX, nY );

        if ( !pDev )
            pDev = pWindow->GetParent();

        if ( pWindow->GetParent() && !pWindow->IsVisible()
                && !pWindow->IsSystemWindow() && ( pWindow->GetParent() == pDev ) )
        {
            Point aOldPos( pWindow->GetPosPixel() );
            pWindow->SetPosPixel( aPos );

            // Erstmal ein Update auf den Parent, damit nicht beim Update
            // auf dieses Fenster noch ein Paint vom Parent abgearbeitet wird,
            // wo dann ggf. dieses Fenster sofort wieder gehidet wird.
            if( pWindow->GetParent() )
                pWindow->GetParent()->Update();

            pWindow->Show();
            pWindow->Update();
            pWindow->SetParentUpdateMode( sal_False );
            pWindow->Hide();
            pWindow->SetParentUpdateMode( sal_True );

            pWindow->SetPosPixel( aOldPos );
        }
        else if ( pDev )
        {
            Size aSz = pWindow->GetSizePixel();
            aSz = pDev->PixelToLogic( aSz );
            Point aP = pDev->PixelToLogic( aPos );

            ULONG nFlags = WINDOW_DRAW_NOCONTROLS;

            // #80064# Why only Mono?
            // if ( pDev->GetOutDevType() == OUTDEV_PRINTER )
            //  nFlags |= WINDOW_DRAW_MONO;

            pWindow->Draw( pDev, aP, aSz, nFlags );
        }
    }
}

void VCLXWindow::setZoom( float fZoomX, float fZoomY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->SetZoom( Fraction( fZoomX ) );
}

// ::com::sun::star::lang::XEventListener
void SAL_CALL VCLXWindow::disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    using namespace ::com::sun::star;

    // check if it comes from our AccessibleContext
    uno::Reference< uno::XInterface > aAC( mxAccessibleContext, uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xSource( _rSource.Source, uno::UNO_QUERY );

    if ( aAC.get() == xSource.get() )
    {   // yep, it does
        mxAccessibleContext = uno::Reference< accessibility::XAccessibleContext >();
    }
}

// ::com::sun::star::accessibility::XAccessible
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXWindow::getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException)
{
    using namespace ::com::sun::star;

    ::vos::OGuard aGuard( GetMutex() );

    if ( !mxAccessibleContext.is() && GetWindow() )
    {
        mxAccessibleContext = CreateAccessibleContext();

        // add as event listener to this component
        // in case somebody disposes it, we do not want to have a (though weak) reference to a dead
        // object
        uno::Reference< lang::XComponent > xComp( mxAccessibleContext, uno::UNO_QUERY );
        if ( xComp.is() )
            xComp->addEventListener( this );
    }

    return mxAccessibleContext;
}
