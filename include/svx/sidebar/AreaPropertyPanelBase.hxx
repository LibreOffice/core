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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_AREA_AREAPROPERTYPANELBASE_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_AREA_AREAPROPERTYPANELBASE_HXX

#include <memory>
#include <svx/sidebar/AreaTransparencyGradientPopup.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svx/colorbox.hxx>
#include <svx/xgrad.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svl/intitem.hxx>
#include <svx/svxdllapi.h>
#include <sfx2/sidebar/Panel.hxx>

class ToolbarUnoDispatcher;
class XFillFloatTransparenceItem;
class XFillTransparenceItem;
class XFillStyleItem;
class XFillGradientItem;
class XFillColorItem;
class XFillHatchItem;
class XFillBitmapItem;

namespace svx::sidebar {

class SVX_DLLPUBLIC AreaPropertyPanelBase
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:

    virtual void dispose() override;

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    const static sal_Int32 DEFAULT_CENTERX;
    const static sal_Int32 DEFAULT_CENTERY;
    const static sal_Int32 DEFAULT_ANGLE;
    const static sal_Int32 DEFAULT_STARTVALUE;
    const static sal_Int32 DEFAULT_ENDVALUE;
    const static sal_Int32 DEFAULT_BORDER;

    const XGradient& GetGradient (const css::awt::GradientStyle eStyle) const;
    void SetGradient (const XGradient& rGradient);
    sal_Int32 GetSelectedTransparencyTypeIndex() const;

    // constructor/destructor
    AreaPropertyPanelBase(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual ~AreaPropertyPanelBase() override;

    virtual void setFillTransparence(const XFillTransparenceItem& rItem) = 0;
    virtual void setFillFloatTransparence(const XFillFloatTransparenceItem& rItem) = 0;
    virtual void setFillStyle(const XFillStyleItem& rItem) = 0;
    virtual void setFillStyleAndColor(const XFillStyleItem* pStyleItem, const XFillColorItem& aColorItem) = 0;
    virtual void setFillStyleAndGradient(const XFillStyleItem* pStyleItem, const XFillGradientItem& aGradientItem) = 0;
    virtual void setFillStyleAndHatch(const XFillStyleItem* pStyleItem, const XFillHatchItem& aHatchItem) = 0;
    virtual void setFillStyleAndBitmap(const XFillStyleItem* pStyleItem, const XFillBitmapItem& aHatchItem) = 0;

    void updateFillTransparence(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState);
    void updateFillFloatTransparence(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState);
    void updateFillStyle(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState);
    void updateFillGradient(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState);
    void updateFillHatch(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState);
    void updateFillColor(bool bDefaultOrSet, const SfxPoolItem* pState);
    void updateFillBitmap(bool BDisabled, bool bDefaultOrSet, const SfxPoolItem* pState);

private:
    void Initialize();

protected:
    sal_uInt16                                          meLastXFS;

    sal_Int32                                           mnLastPosHatch;
    sal_Int32                                           mnLastPosBitmap;
    sal_Int32                                           mnLastPosPattern;
    sal_uInt16                                          mnLastTransSolid;

    XGradient                                           maGradientLinear;
    XGradient                                           maGradientAxial;
    XGradient                                           maGradientRadial;
    XGradient                                           maGradientElliptical;
    XGradient                                           maGradientSquare;
    XGradient                                           maGradientRect;

    //ui controls
    std::unique_ptr<weld::Label> mxColorTextFT;
    std::unique_ptr<weld::ComboBox> mxLbFillType;
    std::unique_ptr<weld::ComboBox> mxLbFillAttr;
    std::unique_ptr<ColorListBox> mxLbFillGradFrom;
    std::unique_ptr<ColorListBox> mxLbFillGradTo;
    std::unique_ptr<weld::Toolbar> mxToolBoxColor; // for new color picker
    std::unique_ptr<ToolbarUnoDispatcher> mxColorDispatch;
    std::unique_ptr<weld::Label> mxTrspTextFT;
    std::unique_ptr<weld::ComboBox> mxLBTransType;
    std::unique_ptr<weld::MetricSpinButton> mxMTRTransparent;
    std::unique_ptr<weld::Scale> mxSldTransparent;
    std::unique_ptr<weld::Toolbar> mxBTNGradient;
    std::unique_ptr<weld::MetricSpinButton> mxMTRAngle;
    std::unique_ptr<weld::ComboBox> mxGradientStyle;
    std::unique_ptr<weld::Button> mxBmpImport;
    std::unique_ptr<AreaTransparencyGradientPopup> mxTrGrPopup;

    std::unique_ptr< XFillStyleItem >               mpStyleItem;
    std::unique_ptr< XFillColorItem >               mpColorItem;
    std::unique_ptr< XFillGradientItem >            mpFillGradientItem;
    std::unique_ptr< XFillHatchItem >               mpHatchItem;
    std::unique_ptr< XFillBitmapItem >              mpBitmapItem;

    OUString maImgAxial;
    OUString maImgElli;
    OUString maImgQuad;
    OUString maImgRadial;
    OUString maImgSquare;
    OUString  maImgLinear;

    VclPtr<sfx2::sidebar::Panel>                    mpPanel;

    std::unique_ptr< XFillFloatTransparenceItem >   mpFloatTransparenceItem;
    std::unique_ptr< SfxUInt16Item >                mpTransparanceItem;

    DECL_LINK(SelectFillTypeHdl, weld::ComboBox&, void );
    DECL_LINK(SelectFillAttrHdl, weld::ComboBox&, void );
    DECL_LINK(SelectFillColorHdl, ColorListBox&, void);
    DECL_LINK(ChangeGradientAngle, weld::MetricSpinButton&, void);
    DECL_LINK(ChangeTrgrTypeHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ModifyTransparentHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyTransSliderHdl, weld::Scale&, void);
    DECL_LINK(ClickImportBitmapHdl, weld::Button&, void);
    DECL_LINK(ToolbarHdl_Impl, const OString&, void);

    void Update();
    void ImpUpdateTransparencies();
    void SetTransparency(sal_uInt16 nVal);
    void SelectFillAttrHdl_Impl();
};

} // end of namespace svx::sidebar

#endif // INCLUDED_SVX_SOURCE_SIDEBAR_AREA_AREAPROPERTYPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
