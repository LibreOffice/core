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

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#define _TOOLKIT_AWT_VCLXWINDOW_HXX_

#include <toolkit/dllapi.h>
#include <toolkit/awt/vclxdevice.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/awt/XStyleSettingsSupplier.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase9.hxx>
#include <osl/mutex.hxx>

#include <tools/gen.hxx>    // Size
#include <tools/link.hxx>

#include <stdarg.h>
#include <list>
#include <boost/function.hpp>

class Window;
class VclSimpleEvent;
class VclWindowEvent;
struct SystemParentData;

namespace toolkit
{
    class IAccessibleFactory;
}


//  ----------------------------------------------------
//  class VCLXWINDOW
//  ----------------------------------------------------

class UnoPropertyArrayHelper;
class VCLXWindowImpl;
typedef ::cppu::ImplInheritanceHelper9  <   VCLXDevice
                                        ,   ::com::sun::star::awt::XWindow2
                                        ,   ::com::sun::star::awt::XVclWindowPeer
                                        ,   ::com::sun::star::awt::XLayoutConstrains
                                        ,   ::com::sun::star::awt::XView
                                        ,   ::com::sun::star::awt::XDockableWindow
                                        ,   ::com::sun::star::accessibility::XAccessible
                                        ,   ::com::sun::star::lang::XEventListener
                                        ,   ::com::sun::star::beans::XPropertySetInfo
                                        ,   ::com::sun::star::awt::XStyleSettingsSupplier
                                        >   VCLXWindow_Base;

class TOOLKIT_DLLPUBLIC VCLXWindow : public VCLXWindow_Base
{
private:
    VCLXWindowImpl*                 mpImpl;

    UnoPropertyArrayHelper *GetPropHelper();

protected:
    Size            ImplCalcWindowSize( const Size& rOutSz ) const;
    DECL_LINK(      WindowEventListener, VclSimpleEvent* );

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                    CreateAccessibleContext();

    void            SetSynthesizingVCLEvent( sal_Bool b );
    sal_Bool            IsSynthesizingVCLEvent() const;

    void        SetSystemParent_Impl( const com::sun::star::uno::Any& rHandle );

    ::toolkit::IAccessibleFactory&  getAccessibleFactory();

    // helper ...
    static void     PushPropertyIds( std::list< sal_uInt16 > &aIds, int nFirstId, ...);
    // for use in controls/
    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds,
                                        bool bWithDefaults = false );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds );

    ::cppu::OInterfaceContainerHelper&  GetContainerListeners();
    ::cppu::OInterfaceContainerHelper&  GetTopWindowListeners();

public:
    typedef ::boost::function0< void >  Callback;

protected:
    /** executes the given callback asynchronously

        At the moment the callback is called, the Solar Mutex is not locked. In particular, this implies that
        you cannot rely on |this| not being disposed. However, you *can* rely on |this| being still alive (i.e.
        having a ref count > 0).

        As a consequence, this can be used for doing listener notifications, using event multiplexers. Those multiplexers
        care for the disposed state themself, and are alive as long as |this| is alive.
    */
    void    ImplExecuteAsyncWithoutSolarLock(
                const Callback& i_callback
            );

public:
    VCLXWindow( bool bWithDefaultProps = false );
    ~VCLXWindow();

    virtual void    SetWindow( Window* pWindow );
    Window*         GetWindow() const                                   { return (Window*)GetOutputDevice(); }

    void    suspendVclEventListening( );
    void    resumeVclEventListening( );

    void    notifyWindowRemoved( Window& _rWindow );

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXWindow*                                          GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setVisible( sal_Bool Visible ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setEnable( sal_Bool Enable ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFocus(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rrxListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindowPeer
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > SAL_CALL getToolkit(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPointer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer >& Pointer ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setBackground( sal_Int32 Color ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL invalidate( sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL invalidateRect( const ::com::sun::star::awt::Rectangle& Rect, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XVclWindowPeer
    sal_Bool SAL_CALL isChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Peer ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL enableClipSiblings( sal_Bool bClip ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setForeground( sal_Int32 Color ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setControlFont( const ::com::sun::star::awt::FontDescriptor& aFont ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getStyles( sal_Int16 nType, ::com::sun::star::awt::FontDescriptor& Font, sal_Int32& ForegroundColor, sal_Int32& BackgroundColor ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XView
    sal_Bool SAL_CALL setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& aDevice ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setZoom( float fZoomX, float fZoomY ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::accessibility::XAccessible
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDockableWindow
    void SAL_CALL addDockableWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeDockableWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL enableDocking( sal_Bool bEnable ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isFloating(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFloatingMode( sal_Bool bFloating ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL lock(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL unlock(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isLocked(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL startPopupMode( const ::com::sun::star::awt::Rectangle& WindowRect ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isInPopupMode(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindow2
    void SAL_CALL setOutputSize( const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getOutputSize(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isVisible(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isActive(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isEnabled(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL hasFocus(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySetInfo
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name ) throw (::com::sun::star::uno::RuntimeException);

    // XStyleSettingsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XStyleSettings > SAL_CALL getStyleSettings() throw (::com::sun::star::uno::RuntimeException);
};

#endif // _TOOLKIT_AWT_VCLXWINDOW_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
