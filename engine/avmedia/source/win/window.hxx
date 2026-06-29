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

#include <sal/config.h>

#include <WinDef.h>

#include "wincommon.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <mutex>

#include <com/sun/star/media/XPlayerWindow.hpp>

namespace avmedia::win {

class Player;


class Window : public ::cppu::WeakImplHelper< css::media::XPlayerWindow,
                                              css::lang::XServiceInfo >
{
public:

            Window( Player& rPlayer );
            ~Window() override;

    bool    create( const css::uno::Sequence< cpo::uno::Any >& aArguments );
    void    updatePointer();

    // XPlayerWindow
    virtual void update(  ) override;
    virtual bool setZoomLevel( css::media::ZoomLevel ZoomLevel ) override;
    virtual css::media::ZoomLevel getZoomLevel(  ) override;
    virtual void setPointerType( sal_Int32 nPointerType ) override;

    // XWindow
    virtual void setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;
    virtual css::awt::Rectangle getPosSize(  ) override;
    virtual void setVisible( bool Visible ) override;
    virtual void setEnable( bool Enable ) override;
    virtual void setFocus(  ) override;
    virtual void addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    virtual void removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    virtual void addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    virtual void removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    virtual void addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    virtual void removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    virtual void addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    virtual void removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    virtual void addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    virtual void removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    virtual void addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;
    virtual void removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

public:

    void fireMousePressedEvent( const css::awt::MouseEvent& rEvt );
    void fireMouseReleasedEvent( const css::awt::MouseEvent& rEvt );
    void fireMouseMovedEvent( const css::awt::MouseEvent& rEvt );
    void fireKeyPressedEvent( const css::awt::KeyEvent& rEvt );
    void fireKeyReleasedEvent( const css::awt::KeyEvent& rEvt );
    void fireSetFocusEvent( const css::awt::FocusEvent& rEvt );
    HWND getParentWnd() const { return mnParentWnd; }
    Player& getPlayer() const { return mrPlayer; }

private:

    std::mutex                                       maMutex;
    comphelper::OInterfaceContainerHelper4<css::awt::XWindowListener>  maWindowListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XFocusListener>  maFocusListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XKeyListener>  maKeyListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XMouseListener>  maMouseListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XMouseMotionListener>  maMouseMotionListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XPaintListener>  maPaintListeners;
    comphelper::OInterfaceContainerHelper4<css::lang::XEventListener>  maEventListeners;
    css::media::ZoomLevel                       meZoomLevel;
    Player&                                     mrPlayer;
    HWND                                        mnFrameWnd;
    HWND                                        mnParentWnd;
    int                                         mnPointerType;

    const css::awt::Rectangle                   getParentPosSize() const;
    void                                        ImplLayoutVideoWindow();
};

} // namespace avmedia::win

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
