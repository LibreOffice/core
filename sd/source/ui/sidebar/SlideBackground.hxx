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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_SLIDEBACKGROUND_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_SLIDEBACKGROUND_HXX

#include <vcl/ctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svx/pagectrl.hxx>
#include "ViewShellBase.hxx"
#include <svx/itemwin.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "fupage.hxx"
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflhtit.hxx>

namespace sd { namespace sidebar {

class SlideBackground :
    public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    SlideBackground(
        Window * pParent,
        ViewShellBase& rBase,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings );
    virtual ~SlideBackground();
    virtual void dispose() override;
    SfxBindings* GetBindings() { return mpBindings; }
    // Window
    virtual void DataChanged (const DataChangedEvent& rEvent) override;
    virtual void NotifyItemUpdate(
        const sal_uInt16 nSID,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

private:

    ViewShellBase& mrBase;

    VclPtr<ListBox> mpPaperSizeBox;
    VclPtr<ListBox> mpPaperOrientation;
    VclPtr<ListBox> mpMasterSlide;
    VclPtr<ListBox> mpFillStyle;
    VclPtr<ColorLB> mpFillLB;
    VclPtr<SvxFillAttrBox> mpFillAttr;
    VclPtr<ColorLB> mpFillGrad;
    VclPtr<CheckBox> mpDspMasterBackground;
    VclPtr<CheckBox> mpDspMasterObjects;

    ::sfx2::sidebar::ControllerItem maPaperSizeController;
    ::sfx2::sidebar::ControllerItem maPaperOrientationController;
    ::sfx2::sidebar::ControllerItem maBckColorController;
    ::sfx2::sidebar::ControllerItem maBckGradientController;
    ::sfx2::sidebar::ControllerItem maBckHatchController;
    ::sfx2::sidebar::ControllerItem maBckBitmapController;
    ::sfx2::sidebar::ControllerItem maBckFillStyleController;
    ::sfx2::sidebar::ControllerItem maBckImageController;
    ::sfx2::sidebar::ControllerItem maDspBckController;
    ::sfx2::sidebar::ControllerItem maDspObjController;
    ::sfx2::sidebar::ControllerItem maMetricController;

    std::unique_ptr< XFillStyleItem > mpFillStyleItem;
    std::unique_ptr< XFillColorItem > mpColorItem;
    std::unique_ptr< XFillGradientItem > mpGradientItem;
    std::unique_ptr< XFillHatchItem > mpHatchItem;
    std::unique_ptr< XFillBitmapItem > mpBitmapItem;

    SfxBindings* mpBindings;

    SfxMapUnit meUnit;

    DECL_LINK_TYPED(FillBackgroundHdl, ListBox&, void);
    DECL_LINK_TYPED(FillStyleModifyHdl, ListBox&, void);
    DECL_LINK_TYPED(PaperSizeModifyHdl, ListBox&, void);
    DECL_LINK_TYPED(PaperOrientationModifyHdl, ListBox&, void);
    DECL_LINK_TYPED(FillColorHdl, ListBox&, void);
    DECL_LINK_TYPED(AssignMasterPage, ListBox&, void);
    DECL_LINK_TYPED(DspBackground, Button*, void);
    DECL_LINK_TYPED(DspObjects, Button*, void);

    void Initialize();
    void Update();
    Color GetColorSetOrDefault();
    XGradient GetGradientSetOrDefault();
    const OUString GetHatchingSetOrDefault();
    const OUString GetBitmapSetOrDefault();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
