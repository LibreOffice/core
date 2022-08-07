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
#pragma once

#include <vcl/customweld.hxx>
#include <vcl/image.hxx>
#include <vcl/window.hxx>
#include <svl/poolitem.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <sfx2/tbxctrl.hxx>

namespace com::sun::star::frame { class XDispatchProvider; }

class SvxZoomSliderItem;

class ScZoomSliderControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    ScZoomSliderControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
    virtual ~ScZoomSliderControl() override;

    virtual void StateChangedAtToolBoxControl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    virtual VclPtr<InterimItemWindow> CreateItemWindow( vcl::Window *pParent ) override;
};

class ScZoomSlider final : public weld::CustomWidgetController
{
private:
    sal_uInt16                   mnCurrentZoom;
    sal_uInt16                   mnMinZoom;
    sal_uInt16                   mnMaxZoom;
    std::vector< tools::Long >      maSnappingPointOffsets;
    std::vector< sal_uInt16 >    maSnappingPointZooms;
    Image                    maSliderButton;
    Image                    maIncreaseButton;
    Image                    maDecreaseButton;
    bool                     mbOmitPaint;
    css::uno::Reference<css::frame::XDispatchProvider> m_xDispatchProvider;

    sal_uInt16 Offset2Zoom(tools::Long nOffset) const;
    tools::Long Zoom2Offset(sal_uInt16 nZoom) const;

    void DoPaint(vcl::RenderContext& rRenderContext);
public:
    ScZoomSlider(css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider,
                 sal_uInt16 nCurrentZoom);

    void UpdateFromItem(const SvxZoomSliderItem* pZoomSliderItem);

    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool MouseMove( const MouseEvent& rMEvt ) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
};

class ScZoomSliderWnd final : public InterimItemWindow
{
private:
    std::unique_ptr<ScZoomSlider> mxWidget;
    std::unique_ptr<weld::CustomWeld> mxWeld;

public:
    ScZoomSliderWnd(vcl::Window* pParent, const css::uno::Reference<css::frame::XDispatchProvider>& rDispatchProvider,
                    sal_uInt16 nCurrentZoom);
    virtual ~ScZoomSliderWnd() override;
    virtual void dispose() override;
    void UpdateFromItem( const SvxZoomSliderItem* pZoomSliderItem );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
