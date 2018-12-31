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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXWINDOW_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXWINDOW_HXX

#include <toolkit/dllapi.h>
#include <toolkit/awt/vclxdevice.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/awt/XStyleSettingsSupplier.hpp>

#include <cppuhelper/implbase.hxx>

#include <tools/gen.hxx>
#include <tools/link.hxx>

#include <memory>
#include <vector>
#include <functional>

namespace comphelper { class OInterfaceContainerHelper2; }
namespace com { namespace sun { namespace star { namespace accessibility { class XAccessibleContext; } } } }

class VclWindowEvent;

namespace toolkit
{
    class IAccessibleFactory;
}

//  class VCLXWINDOW

class UnoPropertyArrayHelper;
class VCLXWindowImpl;
typedef cppu::ImplInheritanceHelper< VCLXDevice,
                                     css::awt::XWindow2,
                                     css::awt::XVclWindowPeer,
                                     css::awt::XLayoutConstrains,
                                     css::awt::XView,
                                     css::awt::XDockableWindow,
                                     css::accessibility::XAccessible,
                                     css::lang::XEventListener,
                                     css::beans::XPropertySetInfo,
                                     css::awt::XStyleSettingsSupplier
                                   > VCLXWindow_Base;

class TOOLKIT_DLLPUBLIC VCLXWindow : public VCLXWindow_Base
{
private:
    std::unique_ptr<VCLXWindowImpl>  mpImpl;

    UnoPropertyArrayHelper *GetPropHelper();

protected:
    Size            ImplCalcWindowSize( const Size& rOutSz ) const;
    DECL_LINK(WindowEventListener, VclWindowEvent&, void );

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                    CreateAccessibleContext();

    void            SetSynthesizingVCLEvent( bool b );
    bool            IsSynthesizingVCLEvent() const;

    void        SetSystemParent_Impl( const css::uno::Any& rHandle );

    ::toolkit::IAccessibleFactory&  getAccessibleFactory();

    // helper ...
    static void     PushPropertyIds( std::vector< sal_uInt16 > &aIds, int nFirstId, ...);
    // for use in controls/
    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds,
                                        bool bWithDefaults = false );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds );

    ::comphelper::OInterfaceContainerHelper2&  GetContainerListeners();
    ::comphelper::OInterfaceContainerHelper2&  GetTopWindowListeners();

public:
    typedef ::std::function<void ()>  Callback;

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
    virtual ~VCLXWindow() override;

    virtual void SetWindow( const VclPtr< vcl::Window > &pWindow );
    template< class derived_type > VclPtr< derived_type > GetAs() const {
        return VclPtr< derived_type >( static_cast< derived_type * >( GetOutputDevice().get() ) ); }
    template< class derived_type > VclPtr< derived_type > GetAsDynamic() const {
        return VclPtr< derived_type >( dynamic_cast< derived_type * >( GetOutputDevice().get() ) ); }
    VclPtr<vcl::Window> GetWindow() const { return GetAs<vcl::Window>(); }

    void    suspendVclEventListening( );
    void    resumeVclEventListening( );

    void    notifyWindowRemoved( vcl::Window const & _rWindow );

    // css::lang::XUnoTunnel
    static const css::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXWindow*                                          GetImplementation( const css::uno::Reference< css::uno::XInterface >& rxIFace );
    sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier ) override;

    // css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;
    void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;

    // css::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;
    css::awt::Rectangle SAL_CALL getPosSize(  ) override;
    void SAL_CALL setVisible( sal_Bool Visible ) override;
    void SAL_CALL setEnable( sal_Bool Enable ) override;
    void SAL_CALL setFocus(  ) override;
    void SAL_CALL addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& rrxListener ) override;
    void SAL_CALL removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& rrxListener ) override;
    void SAL_CALL addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& rrxListener ) override;
    void SAL_CALL removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& rrxListener ) override;
    void SAL_CALL addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& rrxListener ) override;
    void SAL_CALL removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& rrxListener ) override;
    void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& rrxListener ) override;
    void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& rrxListener ) override;
    void SAL_CALL addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& rrxListener ) override;
    void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& rrxListener ) override;
    void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& rrxListener ) override;
    void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& rrxListener ) override;

    // css::awt::XWindowPeer
    css::uno::Reference< css::awt::XToolkit > SAL_CALL getToolkit(  ) override;
    void SAL_CALL setPointer( const css::uno::Reference< css::awt::XPointer >& Pointer ) override;
    void SAL_CALL setBackground( sal_Int32 Color ) override;
    void SAL_CALL invalidate( sal_Int16 Flags ) override;
    void SAL_CALL invalidateRect( const css::awt::Rectangle& Rect, sal_Int16 Flags ) override;

    // css::awt::XVclWindowPeer
    sal_Bool SAL_CALL isChild( const css::uno::Reference< css::awt::XWindowPeer >& Peer ) override;
    void SAL_CALL setDesignMode( sal_Bool bOn ) override;
    sal_Bool SAL_CALL isDesignMode(  ) override;
    void SAL_CALL enableClipSiblings( sal_Bool bClip ) override;
    void SAL_CALL setForeground( sal_Int32 Color ) override;
    void SAL_CALL setControlFont( const css::awt::FontDescriptor& aFont ) override;
    void SAL_CALL getStyles( sal_Int16 nType, css::awt::FontDescriptor& Font, sal_Int32& ForegroundColor, sal_Int32& BackgroundColor ) override;
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize(  ) override;
    css::awt::Size SAL_CALL getPreferredSize(  ) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XView
    sal_Bool SAL_CALL setGraphics( const css::uno::Reference< css::awt::XGraphics >& aDevice ) override;
    css::uno::Reference< css::awt::XGraphics > SAL_CALL getGraphics(  ) override;
    css::awt::Size SAL_CALL getSize(  ) override;
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) override;
    void SAL_CALL setZoom( float fZoomX, float fZoomY ) override;

    // css::accessibility::XAccessible
    css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // css::awt::XDockableWindow
    void SAL_CALL addDockableWindowListener( const css::uno::Reference< css::awt::XDockableWindowListener >& xListener ) override;
    void SAL_CALL removeDockableWindowListener( const css::uno::Reference< css::awt::XDockableWindowListener >& xListener ) override;
    void SAL_CALL enableDocking( sal_Bool bEnable ) override;
    sal_Bool SAL_CALL isFloating(  ) override;
    void SAL_CALL setFloatingMode( sal_Bool bFloating ) override;
    void SAL_CALL lock(  ) override;
    void SAL_CALL unlock(  ) override;
    sal_Bool SAL_CALL isLocked(  ) override;
    void SAL_CALL startPopupMode( const css::awt::Rectangle& WindowRect ) override;
    sal_Bool SAL_CALL isInPopupMode(  ) override;

    // css::awt::XWindow2
    void SAL_CALL setOutputSize( const css::awt::Size& aSize ) override;
    css::awt::Size SAL_CALL getOutputSize(  ) override;
    sal_Bool SAL_CALL isVisible(  ) override;
    sal_Bool SAL_CALL isActive(  ) override;
    sal_Bool SAL_CALL isEnabled(  ) override;
    sal_Bool SAL_CALL hasFocus(  ) override;

    // css::beans::XPropertySetInfo
    css::uno::Sequence< css::beans::Property > SAL_CALL getProperties(  ) override;
    css::beans::Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;

    // XStyleSettingsSupplier
    virtual css::uno::Reference< css::awt::XStyleSettings > SAL_CALL getStyleSettings() override;
};

#endif // INCLUDED_TOOLKIT_AWT_VCLXWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
