/*************************************************************************
 *
 *  $RCSfile: multiplx.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:51 $
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
#ifndef _MRC_MULTIPLX_HXX
#define _MRC_MULTIPLX_HXX

#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XKEYLISTENER_HPP_
#include <com/sun/star/awt/XKeyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPAINTLISTENER_HPP_
#include <com/sun/star/awt/XPaintListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSEMOTIONLISTENER_HPP_
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FOCUSEVENT_HPP_
#include <com/sun/star/awt/FocusEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XACTIVATELISTENER_HPP_
#include <com/sun/star/awt/XActivateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XTopWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_PAINTEVENT_HPP_
#include <com/sun/star/awt/PaintEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_INPUTEVENT_HPP_
#include <com/sun/star/awt/InputEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYGROUP_HPP_
#include <com/sun/star/awt/KeyGroup.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEY_HPP_
#include <com/sun/star/awt/Key.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_WINDOWEVENT_HPP_
#include <com/sun/star/awt/WindowEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYFUNCTION_HPP_
#include <com/sun/star/awt/KeyFunction.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FOCUSCHANGEREASON_HPP_
#include <com/sun/star/awt/FocusChangeReason.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HPP_
#include <com/sun/star/awt/MouseButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#include <cppuhelper/implbase7.hxx>
#include <cppuhelper/interfacecontainer.hxx>

using namespace com::sun::star::uno;

struct MRCListenerMultiplexerHelper_Mutex
{
    ::osl::Mutex aMutex;
};

class MRCListenerMultiplexerHelper  :
    public MRCListenerMultiplexerHelper_Mutex,

    public ::cppu::WeakAggImplHelper7<
      ::com::sun::star::awt::XFocusListener,
      ::com::sun::star::awt::XWindowListener,
      ::com::sun::star::awt::XKeyListener,
      ::com::sun::star::awt::XMouseListener,
      ::com::sun::star::awt::XMouseMotionListener,
      ::com::sun::star::awt::XPaintListener,
      ::com::sun::star::awt::XTopWindowListener >
{
public:
    /**
     * Create a Multiplexer of XWindowEvents.
     *
     * @param rControl  The control. All listeners think that this is the original
     *                  broadcaster.
     * @param rPeer     The peer from which the original events are dispatched. Null is
     *                  allowed.
     */
    MRCListenerMultiplexerHelper( const Reference< ::com::sun::star::awt::XWindow >  & rControl, const Reference< ::com::sun::star::awt::XWindow >  & rPeer );

    /**
     * Remove all listeners from the previous set peer and add the needed listeners to rPeer.
     * @param rPeer     The peer from which the original events are dispatched. Null is
     *                  allowed.
     */
    void setPeer( const Reference< ::com::sun::star::awt::XWindow >  & rPeer );

    /**
     * Remove all listeners and send a disposing message.
     */
    void disposeAndClear();

    /**
     * Add the specified listener to the source.
     */
    void advise( const Type& type, const Reference< XInterface > & listener);
    /**
     * Remove the specified listener from the source.
     */
    void unadvise(const Type& type, const Reference< XInterface > & listener);

    // ::com::sun::star::lang::XEventListener
    void SAL_CALL   disposing(const ::com::sun::star::lang::EventObject& Source);
    // ::com::sun::star::awt::XFocusListener
    void SAL_CALL   focusGained(const ::com::sun::star::awt::FocusEvent& e);
    void SAL_CALL   focusLost(const ::com::sun::star::awt::FocusEvent& e);
    // ::com::sun::star::awt::XWindowListener
    void SAL_CALL   windowResized(const ::com::sun::star::awt::WindowEvent& e);
    void SAL_CALL   windowMoved(const ::com::sun::star::awt::WindowEvent& e);
    void SAL_CALL   windowShown(const ::com::sun::star::lang::EventObject& e);
    void SAL_CALL   windowHidden(const ::com::sun::star::lang::EventObject& e);
    // ::com::sun::star::awt::XKeyListener
    void SAL_CALL   keyPressed( const ::com::sun::star::awt::KeyEvent& e );
    void SAL_CALL   keyReleased( const ::com::sun::star::awt::KeyEvent& e );
    // ::com::sun::star::awt::XMouseListener
    void SAL_CALL   mousePressed(const ::com::sun::star::awt::MouseEvent& e);
    void SAL_CALL   mouseReleased(const ::com::sun::star::awt::MouseEvent& e);
    void SAL_CALL   mouseEntered(const ::com::sun::star::awt::MouseEvent& e);
    void SAL_CALL   mouseExited(const ::com::sun::star::awt::MouseEvent& e);
    // ::com::sun::star::awt::XMouseMotionListener
    void SAL_CALL   mouseDragged(const ::com::sun::star::awt::MouseEvent& e);
    void SAL_CALL   mouseMoved(const ::com::sun::star::awt::MouseEvent& e);
    // ::com::sun::star::awt::XPaintListener
    void SAL_CALL   windowPaint(const ::com::sun::star::awt::PaintEvent& e);
    // ::com::sun::star::awt::XTopWindowListener
    void SAL_CALL   windowOpened( const ::com::sun::star::lang::EventObject& e );
    void SAL_CALL   windowClosing( const ::com::sun::star::lang::EventObject& e );
    void SAL_CALL   windowClosed( const ::com::sun::star::lang::EventObject& e );
    void SAL_CALL   windowMinimized( const ::com::sun::star::lang::EventObject& e );
    void SAL_CALL   windowNormalized( const ::com::sun::star::lang::EventObject& e );
    void SAL_CALL   windowActivated( const ::com::sun::star::lang::EventObject& e );
    void SAL_CALL   windowDeactivated( const ::com::sun::star::lang::EventObject& e );
protected:
    /**
     * Remove the listener with the uik rUik from the peer rPeer.
     * @param rPeer the peer from which the listener is removed.
     * @param rUik  the listener uik, which specify the type of the listener.
     */
    void        adviseToPeer( const Reference< ::com::sun::star::awt::XWindow >  & rPeer, const Type & type );
    /**
     * Add the listener with the uik rUik to the peer rPeer.
     * @param rPeer the peer to which the listener is added.
     * @param rUik  the listener uik, which specify the type of the listener.
     */
    void        unadviseFromPeer( const Reference< ::com::sun::star::awt::XWindow >  & rPeer, const Type & type );
private:
    /** The source of the events. Normally this is the peer object.*/
    Reference< ::com::sun::star::awt::XWindow >         xPeer;
    WeakReference< ::com::sun::star::awt::XControl >    xControl;
    ::cppu::OMultiTypeInterfaceContainerHelper  aListenerHolder;


    MRCListenerMultiplexerHelper( const MRCListenerMultiplexerHelper & );
    MRCListenerMultiplexerHelper & operator = ( const MRCListenerMultiplexerHelper & );
};

#endif  // _MRC_MULTIPLX_HXX



