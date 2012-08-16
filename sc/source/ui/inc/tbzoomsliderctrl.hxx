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
#ifndef _SC_ZOOMSLIDERTBCONTRL_HXX
#define _SC_ZOOMSLIDERTBCONTRL_HXX

#include <vcl/window.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <svx/zoomslideritem.hxx>


class ScZoomSliderControl: public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    ScZoomSliderControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~ScZoomSliderControl();

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual Window* CreateItemWindow( Window *pParent );
};

class ScZoomSliderWnd: public Window
{
private:
    struct                  ScZoomSliderWnd_Impl;
    ScZoomSliderWnd_Impl*   mpImpl;
    Size                    aLogicalSize;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xDispatchProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >            m_xFrame;

    sal_uInt16          Offset2Zoom( long nOffset ) const;
    long            Zoom2Offset( sal_uInt16 nZoom ) const;
    void            DoPaint( const Rectangle& rRect );

public:
    ScZoomSliderWnd( Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame , sal_uInt16 nCurrentZoom );
    ~ScZoomSliderWnd();
    void            UpdateFromItem( const SvxZoomSliderItem* pZoomSliderItem );

protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle& rRect );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
