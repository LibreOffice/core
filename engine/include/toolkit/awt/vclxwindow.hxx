/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <toolkit/dllapi.h>
#include <toolkit/awt/vclxdevice.hxx>
#include <vcl/vclptr.hxx>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/awt/XStyleSettingsSupplier.hpp>

#include <cppuhelper/implbase.hxx>

#include <tools/gen.hxx>
#include <tools/link.hxx>

#include <memory>
#include <vector>
#include <functional>

template <class ListenerT> class ListenerMultiplexerBase;
namespace com::sun::star::awt { class XTopWindowListener; }
namespace com::sun::star::awt { class XVclContainerListener; }
namespace vcl { class Window; }

class VclWindowEvent;


class UnoPropertyArrayHelper;
class VCLXWindowImpl;
typedef cppu::ImplInheritanceHelper< VCLXDevice,
                                     css::awt::XWindow2,
                                     css::awt::XVclWindowPeer,
                                     css::awt::XLayoutConstrains,
                                     css::awt::XView,
                                     css::awt::XDockableWindow,
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
    DECL_DLLPRIVATE_LINK(WindowEventListener, VclWindowEvent&, void );

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    void            SetSynthesizingVCLEvent( bool b );
    bool            IsSynthesizingVCLEvent() const;

    void        SetSystemParent_Impl( const cpo::uno::Any& rHandle );

    // helper ...
    static void     PushPropertyIds( std::vector< sal_uInt16 > &aIds, int nFirstId, ...);
    // for use in controls/
    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds,
                                        bool bWithDefaults = false );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds );

    ListenerMultiplexerBase<css::awt::XVclContainerListener>& GetContainerListeners();
    ListenerMultiplexerBase<css::awt::XTopWindowListener>& GetTopWindowListeners();

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
    template< class derived_type > derived_type* GetAs() const {
        return static_cast< derived_type * >( GetWindow() ); }
    template< class derived_type > derived_type* GetAsDynamic() const {
        return dynamic_cast< derived_type * >( GetWindow() ); }
    vcl::Window* GetWindow() const
    {
        auto p = GetOutputDevice().get();
        return p ? p->GetOwnerWindow() : nullptr;
    }

    void    suspendVclEventListening( );
    void    resumeVclEventListening( );

    void    notifyWindowRemoved( vcl::Window const & _rWindow );

    bool IsDisposed() const;

    // css::lang::XComponent
    void dispose(  ) override;
    void addEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;

    // css::awt::XWindow
    void setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;
    css::awt::Rectangle getPosSize(  ) override;
    void setVisible( bool Visible ) override;
    void setEnable( bool Enable ) override;
    void setFocus(  ) override;
    void addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& rrxListener ) override;
    void removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& rrxListener ) override;
    void addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& rrxListener ) override;
    void removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& rrxListener ) override;
    void addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& rrxListener ) override;
    void removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& rrxListener ) override;
    void addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& rrxListener ) override;
    void removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& rrxListener ) override;
    void addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& rrxListener ) override;
    void removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& rrxListener ) override;
    void addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& rrxListener ) override;
    void removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& rrxListener ) override;

    // css::awt::XWindowPeer
    css::uno::Reference< css::awt::XToolkit > getToolkit(  ) override;
    void setPointer( const css::uno::Reference< css::awt::XPointer >& Pointer ) override;
    void setBackground( sal_Int32 Color ) override;
    void invalidate( sal_Int16 Flags ) override;
    void invalidateRect( const css::awt::Rectangle& Rect, sal_Int16 Flags ) override;

    // css::awt::XVclWindowPeer
    bool isChild( const css::uno::Reference< css::awt::XWindowPeer >& Peer ) override;
    void setDesignMode( bool bOn ) override;
    bool isDesignMode(  ) override;
    void enableClipSiblings( bool bClip ) override;
    void setForeground( sal_Int32 Color ) override;
    void setControlFont( const css::awt::FontDescriptor& aFont ) override;
    void getStyles( sal_Int16 nType, css::awt::FontDescriptor& Font, sal_Int32& ForegroundColor, sal_Int32& BackgroundColor ) override;
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize(  ) override;
    css::awt::Size getPreferredSize(  ) override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XView
    bool setGraphics( const css::uno::Reference< css::awt::XGraphics >& aDevice ) override;
    css::uno::Reference< css::awt::XGraphics > getGraphics(  ) override;
    css::awt::Size getSize(  ) override;
    void draw( sal_Int32 nX, sal_Int32 nY ) override;
    void setZoom( float fZoomX, float fZoomY ) override;

    // css::awt::XDockableWindow
    void addDockableWindowListener( const css::uno::Reference< css::awt::XDockableWindowListener >& xListener ) override;
    void removeDockableWindowListener( const css::uno::Reference< css::awt::XDockableWindowListener >& xListener ) override;
    void enableDocking( bool bEnable ) override;
    bool isFloating(  ) override;
    void setFloatingMode( bool bFloating ) override;
    void lock(  ) override;
    void unlock(  ) override;
    bool isLocked(  ) override;
    void startPopupMode( const css::awt::Rectangle& WindowRect ) override;
    bool isInPopupMode(  ) override;

    // css::awt::XWindow2
    void setOutputSize( const css::awt::Size& aSize ) override;
    css::awt::Size getOutputSize(  ) override;
    bool isVisible(  ) override;
    bool isActive(  ) override;
    bool isEnabled(  ) override;
    bool hasFocus(  ) override;

    // css::beans::XPropertySetInfo
    cpo::uno::Sequence< css::beans::Property > getProperties(  ) override;
    css::beans::Property getPropertyByName( const OUString& aName ) override;
    bool hasPropertyByName( const OUString& Name ) override;

    // XStyleSettingsSupplier
    virtual css::uno::Reference< css::awt::XStyleSettings > getStyleSettings() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
