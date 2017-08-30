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

#include <memory>
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
#include "fupage.hxx"
#include <editeng/sizeitem.hxx>
#include <svx/papersizelistbox.hxx>
#include <svx/rulritem.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflhtit.hxx>
#include "EventMultiplexer.hxx"
#include <sfx2/sidebar/IContextChangeReceiver.hxx>

class SvxColorListBox;

static const long MINBODY = 284;

namespace sd { namespace sidebar {

class SlideBackground :
    public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    SlideBackground(
        Window * pParent,
        ViewShellBase& rBase,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings );
    virtual ~SlideBackground() override;
    virtual void dispose() override;
    SfxBindings* GetBindings() { return mpBindings; }
    // Window
    virtual void DataChanged (const DataChangedEvent& rEvent) override;
    virtual void NotifyItemUpdate(
        const sal_uInt16 nSID,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;
    virtual void HandleContextChange(
        const vcl::EnumContext& rContext) override;

private:

    ViewShellBase& mrBase;

    VclPtr<PaperSizeListBox> mpPaperSizeBox;
    VclPtr<ListBox> mpPaperOrientation;
    VclPtr<ListBox> mpMasterSlide;
    VclPtr<FixedText> mpBackgroundLabel;
    VclPtr<SvxFillTypeBox> mpFillStyle;
    VclPtr<SvxColorListBox> mpFillLB;
    VclPtr<SvxFillAttrBox> mpFillAttr;
    VclPtr<SvxColorListBox> mpFillGrad;
    VclPtr<Button> mpInsertImage;
    VclPtr<CheckBox> mpDspMasterBackground;
    VclPtr<CheckBox> mpDspMasterObjects;
    VclPtr<Button> mpCloseMaster;
    VclPtr<Button> mpEditMaster;
    VclPtr<FixedText> mpMasterLabel;
    VclPtr<ListBox> mpMarginSelectBox;
    VclPtr<VclVBox> m_pContainer;

    ::sfx2::sidebar::ControllerItem maPaperSizeController;
    ::sfx2::sidebar::ControllerItem maPaperOrientationController;
    ::sfx2::sidebar::ControllerItem maPaperMarginLRController;
    ::sfx2::sidebar::ControllerItem maPaperMarginULController;
    ::sfx2::sidebar::ControllerItem maBckColorController;
    ::sfx2::sidebar::ControllerItem maBckGradientController;
    ::sfx2::sidebar::ControllerItem maBckHatchController;
    ::sfx2::sidebar::ControllerItem maBckBitmapController;
    ::sfx2::sidebar::ControllerItem maBckFillStyleController;
    ::sfx2::sidebar::ControllerItem maBckImageController;
    ::sfx2::sidebar::ControllerItem maDspBckController;
    ::sfx2::sidebar::ControllerItem maDspObjController;
    ::sfx2::sidebar::ControllerItem maMetricController;
    ::sfx2::sidebar::ControllerItem maCloseMasterController;

    std::unique_ptr< SvxPageItem >          mpPageItem;
    std::unique_ptr< XFillColorItem >       mpColorItem;
    std::unique_ptr< XFillGradientItem >    mpGradientItem;
    std::unique_ptr< XFillHatchItem >       mpHatchItem;
    std::unique_ptr< XFillBitmapItem >      mpBitmapItem;

    bool mbEditModeChangePending;

    css::uno::Reference<css::frame::XFrame> mxFrame;
    vcl::EnumContext maContext;
    vcl::EnumContext maDrawOtherContext;
    vcl::EnumContext maDrawMasterContext;
    vcl::EnumContext maImpressOtherContext;
    vcl::EnumContext maImpressMasterContext;
    vcl::EnumContext maImpressHandoutContext;
    bool         mbTitle;
    std::unique_ptr<SvxLongLRSpaceItem> mpPageLRMarginItem;
    std::unique_ptr<SvxLongULSpaceItem> mpPageULMarginItem;
    FieldUnit meFieldUnit;
    long m_nPageLeftMargin;
    long m_nPageRightMargin;
    long m_nPageTopMargin;
    long m_nPageBottomMargin;
    OUString maCustomEntry;

    SfxBindings* mpBindings;

    MapUnit meUnit;
    Size m_aPageSize;

    DECL_LINK(FillBackgroundHdl, ListBox&, void);
    DECL_LINK(FillStyleModifyHdl, ListBox&, void);
    DECL_LINK(PaperSizeModifyHdl, ListBox&, void);
    DECL_LINK(FillColorHdl, SvxColorListBox&, void);
    DECL_LINK(AssignMasterPage, ListBox&, void);
    DECL_LINK(DspBackground, Button*, void);
    DECL_LINK(DspObjects, Button*, void);
    DECL_LINK(CloseMasterHdl, Button*, void);
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent&, void );
    DECL_LINK( ModifyMarginHdl, ListBox&, void );

    void Initialize();
    void Update();
    void UpdateMarginBox();
    void SetPanelTitle(const OUString& rTitle);

    Color GetColorSetOrDefault();
    XGradient GetGradientSetOrDefault();
    const OUString GetHatchingSetOrDefault();
    const OUString GetBitmapSetOrDefault();
    const OUString GetPatternSetOrDefault();
    bool IsImpress();
    void addListener();
    void removeListener();
    void ExecuteMarginLRChange(const long mnPageLeftMargin, const long mnPageRightMargin);
    void ExecuteMarginULChange(const long mnPageTopMargin, const long mnPageBottomMargin);
    void populateMasterSlideDropdown();
    void updateMasterSlideSelection();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
