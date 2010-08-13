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

#ifndef _GSTWINDOW_HXX
#define _GSTWINDOW_HXX

#include "gstcommon.hxx"
#include <cppuhelper/interfacecontainer.h>

#ifndef _COM_SUN_STAR_MEDIA_XPLAYERWINDOW_HDL_
#include "com/sun/star/media/XPlayerWindow.hdl"
#endif

namespace avmedia
{
namespace gst
{

class PlayerChildWindow;

// ---------------
// - Window -
// ---------------

class Player;

class Window : public ::cppu::WeakImplHelper2< ::com::sun::star::media::XPlayerWindow,
                                               ::com::sun::star::lang::XServiceInfo >
{
    friend class Player;

public:     Window( ::avmedia::gst::Player& rPlayer );
    ~Window();

    bool create( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments );

    // XPlayerWindow
    virtual void SAL_CALL update()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL setZoomLevel( ::com::sun::star::media::ZoomLevel ZoomLevel )
     throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::media::ZoomLevel SAL_CALL getZoomLevel()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setPointerType( sal_Int32 nPointerType )
     throw( ::com::sun::star::uno::RuntimeException );

    // XWindow
    virtual void SAL_CALL setPosSize( sal_Int32 X,
                                      sal_Int32 Y,
                                      sal_Int32 Width,
                                      sal_Int32 Height,
                                      sal_Int16 Flags )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setVisible( sal_Bool Visible )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setEnable( sal_Bool Enable )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setFocus()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addWindowListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeWindowListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addFocusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeFocusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addKeyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeKeyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addMouseListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeMouseListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addMouseMotionListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeMouseMotionListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addPaintListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removePaintListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    // XComponent
    virtual void SAL_CALL dispose()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
     throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
     throw( ::com::sun::star::uno::RuntimeException );


public:

    void fireMousePressedEvent( const ::com::sun::star::awt::MouseEvent& rEvt );

    void fireMouseReleasedEvent( const ::com::sun::star::awt::MouseEvent& rEvt );

    void fireMouseMovedEvent( const ::com::sun::star::awt::MouseEvent& rEvt );

    void fireKeyPressedEvent( const ::com::sun::star::awt::KeyEvent& rEvt );

    void fireKeyReleasedEvent( const ::com::sun::star::awt::KeyEvent& rEvt );

    void fireSetFocusEvent( const ::com::sun::star::awt::FocusEvent& rEvt );

protected:

    long getXWindowHandle() const;

private:

    ::osl::Mutex maMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper maListeners;
    ::com::sun::star::awt::Rectangle maWindowRect;
    Player& mrPlayer;
    PlayerChildWindow* mpPlayerWindow;
    ::com::sun::star::media::ZoomLevel meZoomLevel;
    sal_Int32 mnPointerType;

    void implLayoutVideoWindow();
};
} // namespace gst
} // namespace avmedia

#endif // _GSTWINDOW_HXX
