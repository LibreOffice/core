/*************************************************************************
 *
 *  $RCSfile: listenermultiplexer.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:08 $
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

#ifndef _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_
#define _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_


#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLCONTAINERLISTENER_HPP_
#include <com/sun/star/awt/XVclContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XKEYLISTENER_HPP_
#include <com/sun/star/awt/XKeyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSEMOTIONLISTENER_HPP_
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPAINTLISTENER_HPP_
#include <com/sun/star/awt/XPaintListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XTopWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLISTENER_HPP_
#include <com/sun/star/awt/XTextListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XACTIONLISTENER_HPP_
#include <com/sun/star/awt/XActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XITEMLISTENER_HPP_
#include <com/sun/star/awt/XItemListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSPINLISTENER_HPP_
#include <com/sun/star/awt/XSpinListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XADJUSTMENTLISTENER_HPP_
#include <com/sun/star/awt/XAdjustmentListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMENULISTENER_HPP_
#include <com/sun/star/awt/XMenuListener.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _TOOLKIT_HELPER_MUTEXHELPER_HXX_
#include <toolkit/helper/mutexhelper.hxx>
#endif

#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif

//  ----------------------------------------------------
//  class ListenerMultiplexerBase
//  ----------------------------------------------------

class ListenerMultiplexerBase : public MutexHelper,
                                public ::cppu::OInterfaceContainerHelper,
                                public ::com::sun::star::uno::XInterface
{
private:
    ::cppu::OWeakObject&    mrContext;

protected:
    ::cppu::OWeakObject&    GetContext() { return mrContext; }

public:
    ListenerMultiplexerBase( ::cppu::OWeakObject& rSource );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { mrContext.acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { mrContext.release(); }
};


//  ----------------------------------------------------
//  class EventListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( EventListenerMultiplexer, ::com::sun::star::lang::XEventListener )
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class FocusListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( FocusListenerMultiplexer, ::com::sun::star::awt::XFocusListener )
    void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END


//  ----------------------------------------------------
//  class WindowListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( WindowListenerMultiplexer, ::com::sun::star::awt::XWindowListener )
    void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END



//  ----------------------------------------------------
//  class VclContainerListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( VclContainerListenerMultiplexer, ::com::sun::star::awt::XVclContainerListener )
    void SAL_CALL windowAdded( const ::com::sun::star::awt::VclContainerEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowRemoved( const ::com::sun::star::awt::VclContainerEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class KeyListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( KeyListenerMultiplexer, ::com::sun::star::awt::XKeyListener )
    void SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class MouseListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( MouseListenerMultiplexer, ::com::sun::star::awt::XMouseListener )
    void SAL_CALL mousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL mouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL mouseEntered( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL mouseExited( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class MouseMotionListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( MouseMotionListenerMultiplexer, ::com::sun::star::awt::XMouseMotionListener )
    void SAL_CALL mouseDragged( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL mouseMoved( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class PaintListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( PaintListenerMultiplexer, ::com::sun::star::awt::XPaintListener )
    void SAL_CALL windowPaint( const ::com::sun::star::awt::PaintEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TopWindowListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener )
    void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TextListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( TextListenerMultiplexer, ::com::sun::star::awt::XTextListener )
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class ActionListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( ActionListenerMultiplexer, ::com::sun::star::awt::XActionListener )
    void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class ItemListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( ItemListenerMultiplexer, ::com::sun::star::awt::XItemListener )
    void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class ContainerListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( ContainerListenerMultiplexer, ::com::sun::star::container::XContainerListener )
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class SpinListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( SpinListenerMultiplexer, ::com::sun::star::awt::XSpinListener )
    void SAL_CALL up( const ::com::sun::star::awt::SpinEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL down( const ::com::sun::star::awt::SpinEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL first( const ::com::sun::star::awt::SpinEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL last( const ::com::sun::star::awt::SpinEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class AdjustmentListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( AdjustmentListenerMultiplexer, ::com::sun::star::awt::XAdjustmentListener )
    void SAL_CALL adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class MenuListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener )
    void SAL_CALL highlight( const ::com::sun::star::awt::MenuEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL deactivate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END




#endif // _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_

