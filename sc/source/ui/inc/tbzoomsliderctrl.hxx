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
#ifndef INCLUDED_SC_SOURCE_UI_INC_TBZOOMSLIDERCTRL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TBZOOMSLIDERCTRL_HXX

#include <vcl/window.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <svx/zoomslideritem.hxx>

class ScZoomSliderControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    ScZoomSliderControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~ScZoomSliderControl() override;

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window *pParent ) override;
};

class ScZoomSliderWnd: public vcl::Window
{
private:
    struct ScZoomSliderWnd_Impl;
    ScZoomSliderWnd_Impl* mpImpl;
    Size aLogicalSize;
    css::uno::Reference<css::frame::XDispatchProvider> m_xDispatchProvider;

    sal_uInt16 Offset2Zoom(long nOffset) const;
    long Zoom2Offset(sal_uInt16 nZoom) const;
    void DoPaint(vcl::RenderContext& rRenderContext);

public:
    ScZoomSliderWnd(vcl::Window* pParent, const css::uno::Reference<css::frame::XDispatchProvider >& rDispatchProvider,
                    sal_uInt16 nCurrentZoom);
    virtual ~ScZoomSliderWnd() override;
    virtual void dispose() override;
    void UpdateFromItem( const SvxZoomSliderItem* pZoomSliderItem );

protected:
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
