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
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <svx/colorbox.hxx>
#include <svx/xgrad.hxx>
#include <svx/itemwin.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/slider.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclptr.hxx>
#include <svl/intitem.hxx>
#include <svx/svxdllapi.h>
#include <sfx2/sidebar/Panel.hxx>

class XFillFloatTransparenceItem;
class XFillTransparenceItem;
class XFillStyleItem;
class XFillGradientItem;
class XFillColorItem;
class XFillHatchItem;
class XFillBitmapItem;

namespace svx { namespace sidebar {

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
    VclPtr<FixedText>                                          mpColorTextFT;
    VclPtr<SvxFillTypeBox>                                     mpLbFillType;
    VclPtr<SvxFillAttrBox>                                     mpLbFillAttr;
    VclPtr<SvxColorListBox>                                    mpLbFillGradFrom;
    VclPtr<SvxColorListBox>                                    mpLbFillGradTo;
    VclPtr<sfx2::sidebar::SidebarToolBox>                      mpToolBoxColor; // for new color picker
    VclPtr<FixedText>                                          mpTrspTextFT;
    VclPtr<ListBox>                                            mpLBTransType;
    VclPtr<MetricField>                                        mpMTRTransparent;
    VclPtr<Slider>                                             mpSldTransparent;
    VclPtr<ToolBox>                                            mpBTNGradient;
    VclPtr<MetricField>                                        mpMTRAngle;
    VclPtr<ListBox>                                            mpGradientStyle;
    VclPtr<PushButton>                                         mpBmpImport;

    std::unique_ptr< XFillStyleItem >               mpStyleItem;
    std::unique_ptr< XFillColorItem >               mpColorItem;
    std::unique_ptr< XFillGradientItem >            mpFillGradientItem;
    std::unique_ptr< XFillHatchItem >               mpHatchItem;
    std::unique_ptr< XFillBitmapItem >              mpBitmapItem;

    Image                                               maImgAxial;
    Image                                               maImgElli;
    Image                                               maImgQuad;
    Image                                               maImgRadial;
    Image                                               maImgSquare;
    Image                                               maImgLinear;

    VclPtr<AreaTransparencyGradientPopup>           mxTrGrPopup;
    VclPtr<sfx2::sidebar::Panel>                    mpPanel;

    std::unique_ptr< XFillFloatTransparenceItem >   mpFloatTransparenceItem;
    std::unique_ptr< SfxUInt16Item >                mpTransparanceItem;

    DECL_LINK(SelectFillTypeHdl, ListBox&, void );
    DECL_LINK(SelectFillAttrHdl, ListBox&, void );
    DECL_LINK(SelectFillColorHdl, SvxColorListBox&, void);
    DECL_LINK(ChangeGradientAngle, Edit&, void);
    DECL_LINK(ChangeTrgrTypeHdl_Impl, ListBox&, void);
    DECL_LINK(ModifyTransparentHdl_Impl, Edit&, void);
    DECL_LINK(ModifyTransSliderHdl, Slider*, void);
    DECL_LINK(ClickImportBitmapHdl, Button*, void);

    // for transparency gradient
    DECL_LINK( ClickTrGrHdl_Impl, ToolBox*, void );

    void Initialize();
    void Update();
    void ImpUpdateTransparencies();
    void SetTransparency(sal_uInt16 nVal);
    void SelectFillAttrHdl_Impl();
};

} } // end of namespace svx::sidebar

#endif // INCLUDED_SVX_SOURCE_SIDEBAR_AREA_AREAPROPERTYPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
