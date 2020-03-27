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
#ifndef INCLUDED_SC_SOURCE_UI_SIDEBAR_CELLAPPEARANCEPROPERTYPANEL_HXX
#define INCLUDED_SC_SOURCE_UI_SIDEBAR_CELLAPPEARANCEPROPERTYPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/image.hxx>
#include <vcl/toolbox.hxx>
#include "CellBorderStyleControl.hxx"
#include "CellLineStyleControl.hxx"

namespace sc { namespace sidebar {

class CellAppearancePropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
private:
    friend class CellLineStylePopup;
    friend class CellBorderStylePopup;

public:
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    virtual void HandleContextChange(
        const vcl::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    SfxBindings* GetBindings() { return mpBindings;}

    // constructor/destructor
    CellAppearancePropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~CellAppearancePropertyPanel() override;
    virtual void dispose() override;

private:
    //ui controls

    VclPtr<ToolBox>                                mpTBCellBorder;
    VclPtr<ToolBox>                                mpTBLineStyle;
    VclPtr<ToolBox>                                mpTBLineColor;

    ::sfx2::sidebar::ControllerItem         maLineStyleControl;
    ::sfx2::sidebar::ControllerItem         maBorderOuterControl;
    ::sfx2::sidebar::ControllerItem         maBorderInnerControl;
    ::sfx2::sidebar::ControllerItem         maGridShowControl;
    ::sfx2::sidebar::ControllerItem         maBorderTLBRControl;
    ::sfx2::sidebar::ControllerItem         maBorderBLTRControl;

    // images
    Image const                             maIMGCellBorder;
    Image const                             maIMGLineStyle1;
    Image const                             maIMGLineStyle2;
    Image const                             maIMGLineStyle3;
    Image const                             maIMGLineStyle4;
    Image const                             maIMGLineStyle5;
    Image const                             maIMGLineStyle6;
    Image const                             maIMGLineStyle7;
    Image const                             maIMGLineStyle8;
    Image const                             maIMGLineStyle9;

    // BorderStyle defines
    sal_uInt16                              mnInWidth;
    sal_uInt16                              mnOutWidth;
    sal_uInt16                              mnDistance;
    sal_uInt16                              mnDiagTLBRInWidth;
    sal_uInt16                              mnDiagTLBROutWidth;
    sal_uInt16                              mnDiagTLBRDistance;
    sal_uInt16                              mnDiagBLTRInWidth;
    sal_uInt16                              mnDiagBLTROutWidth;
    sal_uInt16                              mnDiagBLTRDistance;

    bool                                    mbBorderStyleAvailable : 1;

    // CellBorder defines
    bool                                    mbLeft : 1;
    bool                                    mbRight : 1;
    bool                                    mbTop : 1;
    bool                                    mbBottom : 1;
    bool                                    mbVer : 1;
    bool                                    mbHor : 1;

    bool                                    mbOuterBorder : 1; // mbLeft || mbRight || mbTop || mbBottom
    bool                                    mbInnerBorder : 1; // mbVer || mbHor || bLeft || bRight || bTop || bBottom

    bool                                    mbDiagTLBR : 1;
    bool                                    mbDiagBLTR : 1;

    // popups
    VclPtr<CellLineStylePopup>              mxCellLineStylePopup;
    VclPtr<CellBorderStylePopup>            mxCellBorderStylePopup;

    vcl::EnumContext                        maContext;
    SfxBindings* const                      mpBindings;

    DECL_LINK(TbxCellBorderSelectHdl, ToolBox*, void);
    DECL_LINK(TbxLineStyleSelectHdl, ToolBox*, void);

    void Initialize();
    void SetStyleIcon();
    void UpdateControlState();
    void UpdateCellBorder(bool bTop, bool bBot, bool bLeft, bool bRight, bool bVer, bool bHor);
};

} } // end of namespace ::sc::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
