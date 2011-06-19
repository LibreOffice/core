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

#ifndef _UNOCONTROLS_MULTIPLEXER_HXX
#define _UNOCONTROLS_MULTIPLEXER_HXX

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

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
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
//____________________________________________________________________________________________________________
//  includes of my own project
//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________
//  "namespaces"
//____________________________________________________________________________________________________________

namespace unocontrols{

#define CSS_UNO     ::com::sun::star::uno
#define CSS_AWT     ::com::sun::star::awt
#define CSS_LANG    ::com::sun::star::lang

//____________________________________________________________________________________________________________
//  class
//____________________________________________________________________________________________________________

class OMRCListenerMultiplexerHelper : public CSS_AWT::XFocusListener
                                    , public CSS_AWT::XWindowListener
                                    , public CSS_AWT::XKeyListener
                                    , public CSS_AWT::XMouseListener
                                    , public CSS_AWT::XMouseMotionListener
                                    , public CSS_AWT::XPaintListener
                                    , public CSS_AWT::XTopWindowListener
                                    , public ::cppu::OWeakObject
{

//____________________________________________________________________________________________________________
//  public methods
//____________________________________________________________________________________________________________

public:

    //________________________________________________________________________________________________________
    //  construct/destruct
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      constructor
        @descr      Create a Multiplexer of XWindowEvents.

        @seealso    -

        @param      rControl    The control. All listeners think that this is the original broadcaster.
        @param      rPeer       The peer from which the original events are dispatched. Null is allowed.

        @return     -

        @onerror    -
    */

    OMRCListenerMultiplexerHelper(  const   CSS_UNO::Reference< CSS_AWT::XWindow >& xControl    ,
                                    const   CSS_UNO::Reference< CSS_AWT::XWindow >& xPeer       );

    /**_______________________________________________________________________________________________________
        @short      copy-constructor
        @descr

        @seealso    -

        @param      rCopyInstance   C++-Reference to instance to make copy from.

        @return     -

        @onerror    -
    */

    OMRCListenerMultiplexerHelper( const OMRCListenerMultiplexerHelper& aCopyInstance );

    /**_______________________________________________________________________________________________________
        @short      destructor
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    ~OMRCListenerMultiplexerHelper();

    //________________________________________________________________________________________________________
    //  XInterface
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual CSS_UNO::Any SAL_CALL queryInterface( const CSS_UNO::Type& aType )
        throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw();

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw();

    //________________________________________________________________________________________________________
    //  operator
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    operator CSS_UNO::Reference< CSS_UNO::XInterface >() const;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    OMRCListenerMultiplexerHelper& operator= ( const OMRCListenerMultiplexerHelper& aCopyInstance );

    //________________________________________________________________________________________________________
    //  container methods
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      Remove all listeners from the previous set peer and add the needed listeners to rPeer.
        @descr      -

        @seealso    -

        @param      rPeer       The peer from which the original events are dispatched. Null is allowed.

        @return     -

        @onerror    -
    */

    void setPeer( const CSS_UNO::Reference< CSS_AWT::XWindow >& xPeer );

    /**_______________________________________________________________________________________________________
        @short      Remove all listeners and send a disposing message.
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void disposeAndClear();

    /**_______________________________________________________________________________________________________
        @short      Add the specified listener to the source.
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void advise(    const   CSS_UNO::Type&                              aType       ,
                    const   CSS_UNO::Reference< CSS_UNO::XInterface >&  xListener   );

    /**_______________________________________________________________________________________________________
        @short      Remove the specified listener from the source.
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void unadvise(  const   CSS_UNO::Type&                              aType       ,
                    const   CSS_UNO::Reference< CSS_UNO::XInterface >&  xListener   );

    //________________________________________________________________________________________________________
    //  XEventListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL disposing(const CSS_LANG::EventObject& aSource)
        throw( CSS_UNO::RuntimeException ) ;

    //________________________________________________________________________________________________________
    //  XFocusListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL focusGained(const CSS_AWT::FocusEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL focusLost(const CSS_AWT::FocusEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    //________________________________________________________________________________________________________
    //  XWindowListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowResized(const CSS_AWT::WindowEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowMoved(const CSS_AWT::WindowEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowShown(const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowHidden(const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    //________________________________________________________________________________________________________
    //  XKeyListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL keyPressed( const CSS_AWT::KeyEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL keyReleased( const CSS_AWT::KeyEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    //________________________________________________________________________________________________________
    //  XMouseListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL mousePressed(const CSS_AWT::MouseEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL mouseReleased(const CSS_AWT::MouseEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL mouseEntered(const CSS_AWT::MouseEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL mouseExited(const CSS_AWT::MouseEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    //________________________________________________________________________________________________________
    //  XMouseMotionListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL mouseDragged(const CSS_AWT::MouseEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL mouseMoved(const CSS_AWT::MouseEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    //________________________________________________________________________________________________________
    //  XPaintListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowPaint(const CSS_AWT::PaintEvent& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    //________________________________________________________________________________________________________
    //  XTopWindowListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowOpened( const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowClosing( const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowClosed( const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowMinimized( const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowNormalized( const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowActivated( const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -
        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowDeactivated( const CSS_LANG::EventObject& aEvent )
        throw( CSS_UNO::RuntimeException ) ;

//____________________________________________________________________________________________________________
//  protected methods
//____________________________________________________________________________________________________________

protected:

    /**_______________________________________________________________________________________________________
        @short      Remove the listener from the peer.
        @descr      -

        @seealso    -

        @param      xPeer   The peer from which the listener is removed.
        @param      rType   The listener type, which specify the type of the listener.

        @return     -

        @onerror    -
    */

    void impl_adviseToPeer( const   CSS_UNO::Reference< CSS_AWT::XWindow >& xPeer   ,
                            const   CSS_UNO::Type&                          aType   );

    /**_______________________________________________________________________________________________________
        @short      Add the listener to the peer.
        @descr      -

        @seealso    -

        @param      xPeer   The peer to which the listener is added.
        @param      rType   The listener type, which specify the type of the listener.

        @return     -

        @onerror    -
    */

    void impl_unadviseFromPeer( const   CSS_UNO::Reference< CSS_AWT::XWindow >& xPeer   ,
                                const   CSS_UNO::Type&                          aType   );

//____________________________________________________________________________________________________________
//  private variables
//____________________________________________________________________________________________________________

private:

    ::osl::Mutex                                m_aMutex            ;
    CSS_UNO::Reference< CSS_AWT::XWindow >      m_xPeer             ;   /// The source of the events. Normally this is the peer object.
    CSS_UNO::WeakReference< CSS_AWT::XWindow >  m_xControl          ;
    ::cppu::OMultiTypeInterfaceContainerHelper  m_aListenerHolder   ;

};  // class OMRCListenerMultiplexerHelper

// The namespace aliases are only used in this header
#undef CSS_UNO
#undef CSS_AWT
#undef CSS_LANG


}   // namespace unocontrols

#endif  // ifndef _UNOCONTROLS_MULTIPLEXER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
