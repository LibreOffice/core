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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_AREA_AREAPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_AREA_AREAPROPERTYPANEL_HXX

#include "AreaTransparencyGradientPopup.hxx"
#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svx/xgrad.hxx>
#include <svx/itemwin.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/drawitem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclptr.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/ui/XUIElement.hpp>
#include <boost/scoped_ptr.hpp>


class XFillFloatTransparenceItem;


namespace svx { namespace sidebar {

class PopupContainer;
class AreaTransparencyGradientControl;

class AreaPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~AreaPropertyPanel();
    virtual void dispose() SAL_OVERRIDE;

    static AreaPropertyPanel* Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) SAL_OVERRIDE;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) SAL_OVERRIDE;

    SfxBindings* GetBindings() { return mpBindings;}

    const static sal_Int32 DEFAULT_CENTERX;
    const static sal_Int32 DEFAULT_CENTERY;
    const static sal_Int32 DEFAULT_ANGLE;
    const static sal_Int32 DEFAULT_STARTVALUE;
    const static sal_Int32 DEFAULT_ENDVALUE;
    const static sal_Int32 DEFAULT_BORDER;

    XGradient GetGradient (const css::awt::GradientStyle eStyle) const;
    void SetGradient (const XGradient& rGradient);
    sal_Int32 GetSelectedTransparencyTypeIndex (void) const;

private:
    sal_uInt16                                          meLastXFS;

    sal_Int32                                           mnLastPosGradient;
    sal_Int32                                           mnLastPosHatch;
    sal_Int32                                           mnLastPosBitmap;
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
    VclPtr<ToolBox>                                            mpToolBoxColor; // for new color picker
    VclPtr<FixedText>                                          mpTrspTextFT;
    VclPtr<ListBox>                                            mpLBTransType;
    VclPtr<MetricField>                                        mpMTRTransparent;
    VclPtr<ToolBox>                                            mpBTNGradient;

    ::boost::scoped_ptr< XFillStyleItem >               mpStyleItem;
    ::boost::scoped_ptr< XFillColorItem >               mpColorItem;
    ::boost::scoped_ptr< XFillGradientItem >            mpFillGradientItem;
    ::boost::scoped_ptr< XFillHatchItem >               mpHatchItem;
    ::boost::scoped_ptr< XFillBitmapItem >              mpBitmapItem;

    ::sfx2::sidebar::ControllerItem                     maStyleControl;
    ::sfx2::sidebar::ControllerItem                     maColorControl;
    ::sfx2::sidebar::ControllerItem                     maGradientControl;
    ::sfx2::sidebar::ControllerItem                     maHatchControl;
    ::sfx2::sidebar::ControllerItem                     maBitmapControl;
    ::sfx2::sidebar::ControllerItem                     maGradientListControl;
    ::sfx2::sidebar::ControllerItem                     maHatchListControl;
    ::sfx2::sidebar::ControllerItem                     maBitmapListControl;
    ::sfx2::sidebar::ControllerItem                     maFillTransparenceController;
    ::sfx2::sidebar::ControllerItem                     maFillFloatTransparenceController;

    Image                                               maImgAxial;
    Image                                               maImgElli;
    Image                                               maImgQuad;
    Image                                               maImgRadial;
    Image                                               maImgSquare;
    Image                                               maImgLinear;

    AreaTransparencyGradientPopup maTrGrPopup;

    ::boost::scoped_ptr< XFillFloatTransparenceItem >   mpFloatTransparenceItem;
    ::boost::scoped_ptr< SfxUInt16Item >                mpTransparanceItem;

    css::uno::Reference<css::frame::XFrame>                 mxFrame;
    SfxBindings*                                        mpBindings;

    DECL_LINK(SelectFillTypeHdl, ListBox* );
    DECL_LINK(SelectFillAttrHdl, ListBox* );
    DECL_LINK(ChangeTrgrTypeHdl_Impl, void*);
    DECL_LINK(ModifyTransparentHdl_Impl, void*);

    // for transparency gradient
    PopupControl* CreateTransparencyGradientControl (PopupContainer* pParent);
    DECL_LINK( ClickTrGrHdl_Impl, ToolBox* );

    // constructor/destuctor
    AreaPropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    void SetupIcons(void);
    void Initialize();
    void Update();
    void ImpUpdateTransparencies();
};


} } // end of namespace ::svx::sidebar



#endif // INCLUDED_SVX_SOURCE_SIDEBAR_AREA_AREAPROPERTYPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
