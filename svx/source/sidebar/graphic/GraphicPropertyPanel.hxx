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

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/weld.hxx>

namespace svx::sidebar {

class GraphicPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~GraphicPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent,
        SfxBindings* pBindings);

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
        weld::Widget* pParent,
        SfxBindings* pBindings);

private:
    ::sfx2::sidebar::ControllerItem                     maBrightControl;
    ::sfx2::sidebar::ControllerItem                     maContrastControl;
    ::sfx2::sidebar::ControllerItem                     maTransparenceControl;
    ::sfx2::sidebar::ControllerItem                     maRedControl;
    ::sfx2::sidebar::ControllerItem                     maGreenControl;
    ::sfx2::sidebar::ControllerItem                     maBlueControl;
    ::sfx2::sidebar::ControllerItem                     maGammaControl;
    ::sfx2::sidebar::ControllerItem                     maModeControl;

    SfxBindings*                                        mpBindings;

    //ui controls
    std::unique_ptr<weld::MetricSpinButton> mxMtrBrightness;
    std::unique_ptr<weld::MetricSpinButton> mxMtrContrast;
    std::unique_ptr<weld::ComboBox> mxLBColorMode;
    std::unique_ptr<weld::MetricSpinButton> mxMtrTrans;
    std::unique_ptr<weld::MetricSpinButton> mxMtrRed;
    std::unique_ptr<weld::MetricSpinButton> mxMtrGreen;
    std::unique_ptr<weld::MetricSpinButton> mxMtrBlue;
    std::unique_ptr<weld::SpinButton> mxMtrGamma;

    DECL_LINK( ModifyBrightnessHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ModifyContrastHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ModifyTransHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ClickColorModeHdl, weld::ComboBox&, void );

    void Initialize();
};

} // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
