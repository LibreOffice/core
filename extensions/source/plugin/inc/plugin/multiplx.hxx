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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_MULTIPLX_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_MULTIPLX_HXX

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/FocusEvent.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XActivateListener.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/PaintEvent.hpp>
#include <com/sun/star/awt/InputEvent.hpp>
#include <com/sun/star/awt/KeyGroup.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/KeyFunction.hpp>
#include <com/sun/star/awt/FocusChangeReason.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>

using namespace com::sun::star::uno;

struct MRCListenerMultiplexerHelper_Mutex
{
    ::osl::Mutex aMutex;
};

class MRCListenerMultiplexerHelper  :
    public MRCListenerMultiplexerHelper_Mutex,

    public ::cppu::WeakAggImplHelper<
      css::awt::XFocusListener,
      css::awt::XWindowListener,
      css::awt::XKeyListener,
      css::awt::XMouseListener,
      css::awt::XMouseMotionListener,
      css::awt::XPaintListener,
      css::awt::XTopWindowListener >
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
    MRCListenerMultiplexerHelper( const Reference< css::awt::XWindow >  & rControl, const Reference< css::awt::XWindow >  & rPeer );

    /**
     * Remove all listeners from the previous set peer and add the needed listeners to rPeer.
     * @param rPeer     The peer from which the original events are dispatched. Null is
     *                  allowed.
     */
    void setPeer( const Reference< css::awt::XWindow >  & rPeer );

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

    // css::lang::XEventListener
    void SAL_CALL   disposing(const css::lang::EventObject& Source) throw(std::exception) override;
    // css::awt::XFocusListener
    void SAL_CALL   focusGained(const css::awt::FocusEvent& e) throw(std::exception) override;
    void SAL_CALL   focusLost(const css::awt::FocusEvent& e) throw(std::exception) override;
    // css::awt::XWindowListener
    void SAL_CALL   windowResized(const css::awt::WindowEvent& e) throw(std::exception) override;
    void SAL_CALL   windowMoved(const css::awt::WindowEvent& e) throw(std::exception) override;
    void SAL_CALL   windowShown(const css::lang::EventObject& e) throw(std::exception) override;
    void SAL_CALL   windowHidden(const css::lang::EventObject& e) throw(std::exception) override;
    // css::awt::XKeyListener
    void SAL_CALL   keyPressed( const css::awt::KeyEvent& e ) throw(std::exception) override;
    void SAL_CALL   keyReleased( const css::awt::KeyEvent& e ) throw(std::exception) override;
    // css::awt::XMouseListener
    void SAL_CALL   mousePressed(const css::awt::MouseEvent& e) throw(std::exception) override;
    void SAL_CALL   mouseReleased(const css::awt::MouseEvent& e) throw(std::exception) override;
    void SAL_CALL   mouseEntered(const css::awt::MouseEvent& e) throw(std::exception) override;
    void SAL_CALL   mouseExited(const css::awt::MouseEvent& e) throw(std::exception) override;
    // css::awt::XMouseMotionListener
    void SAL_CALL   mouseDragged(const css::awt::MouseEvent& e) throw(std::exception) override;
    void SAL_CALL   mouseMoved(const css::awt::MouseEvent& e) throw(std::exception) override;
    // css::awt::XPaintListener
    void SAL_CALL   windowPaint(const css::awt::PaintEvent& e) throw(std::exception) override;
    // css::awt::XTopWindowListener
    void SAL_CALL   windowOpened( const css::lang::EventObject& e ) throw(std::exception) override;
    void SAL_CALL   windowClosing( const css::lang::EventObject& e ) throw(std::exception) override;
    void SAL_CALL   windowClosed( const css::lang::EventObject& e ) throw(std::exception) override;
    void SAL_CALL   windowMinimized( const css::lang::EventObject& e ) throw(std::exception) override;
    void SAL_CALL   windowNormalized( const css::lang::EventObject& e ) throw(std::exception) override;
    void SAL_CALL   windowActivated( const css::lang::EventObject& e ) throw(std::exception) override;
    void SAL_CALL   windowDeactivated( const css::lang::EventObject& e ) throw(std::exception) override;
protected:
    /**
     * Remove the listener with the uik rUik from the peer rPeer.
     * @param rPeer the peer from which the listener is removed.
     * @param rUik  the listener uik, which specify the type of the listener.
     */
    void        adviseToPeer( const Reference< css::awt::XWindow >  & rPeer, const Type & type );
    /**
     * Add the listener with the uik rUik to the peer rPeer.
     * @param rPeer the peer to which the listener is added.
     * @param rUik  the listener uik, which specify the type of the listener.
     */
    void        unadviseFromPeer( const Reference< css::awt::XWindow >  & rPeer, const Type & type );
private:
    /** The source of the events. Normally this is the peer object.*/
    Reference< css::awt::XWindow >         xPeer;
    WeakReference< css::awt::XControl >    xControl;
    ::cppu::OMultiTypeInterfaceContainerHelper  aListenerHolder;


    MRCListenerMultiplexerHelper( const MRCListenerMultiplexerHelper & ) = delete;
    MRCListenerMultiplexerHelper & operator = ( const MRCListenerMultiplexerHelper & ) = delete;
};

#endif // INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_MULTIPLX_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
