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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_GRAPHIC_GRAPHICPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_GRAPHIC_GRAPHICPROPERTYPANEL_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/fixed.hxx>

class FixedText;
class MetricField;
class ListBox;
class FloatingWindow;


namespace svx { namespace sidebar {

class GraphicPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~GraphicPropertyPanel() override;
    virtual void dispose() override;

    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    SfxBindings* GetBindings() { return mpBindings;}

    // constructor/destructor
    GraphicPropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

private:
    //ui controls
    VclPtr<MetricField>                                        mpMtrBrightness;
    VclPtr<MetricField>                                        mpMtrContrast;
    VclPtr<ListBox>                                            mpLBColorMode;
    VclPtr<MetricField>                                        mpMtrTrans;
    VclPtr<MetricField>                                        mpMtrRed;
    VclPtr<MetricField>                                        mpMtrGreen;
    VclPtr<MetricField>                                        mpMtrBlue;
    VclPtr<MetricField>                                        mpMtrGamma;

    ::sfx2::sidebar::ControllerItem                     maBrightControl;
    ::sfx2::sidebar::ControllerItem                     maContrastControl;
    ::sfx2::sidebar::ControllerItem                     maTransparenceControl;
    ::sfx2::sidebar::ControllerItem                     maRedControl;
    ::sfx2::sidebar::ControllerItem                     maGreenControl;
    ::sfx2::sidebar::ControllerItem                     maBlueControl;
    ::sfx2::sidebar::ControllerItem                     maGammaControl;
    ::sfx2::sidebar::ControllerItem                     maModeControl;

    SfxBindings* const                                  mpBindings;

    DECL_LINK( ModifyBrightnessHdl, Edit&, void );
    DECL_LINK( ModifyContrastHdl, Edit&, void );
    DECL_LINK( ModifyTransHdl, Edit&, void );
    DECL_LINK( ClickColorModeHdl, ListBox&, void );
    DECL_LINK( RedHdl, Edit&, void );
    DECL_LINK( GreenHdl, Edit&, void );
    DECL_LINK( BlueHdl, Edit&, void );
    DECL_LINK( GammaHdl, Edit&, void );

    void Initialize();
};


} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
