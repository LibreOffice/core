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
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svx/papersizelistbox.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <vcl/EnumContext.hxx>

namespace sd { class ViewShellBase; }
namespace sd::tools { class EventMultiplexerEvent; }

class ColorListBox;
class SvxPageItem;
class SvxLongLRSpaceItem;
class SvxLongULSpaceItem;
class XFillColorItem;
class XGradient;
class XFillGradientItem;
class XFillBitmapItem;
class XFillHatchItem;

const tools::Long MINBODY = 284;

namespace sd::sidebar {

class SlideBackground :
    public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    SlideBackground(
        weld::Widget* pParent,
        ViewShellBase& rBase,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings );
    virtual ~SlideBackground() override;
    SfxBindings* GetBindings() { return mpBindings; }
    // Window
    virtual void NotifyItemUpdate(
        const sal_uInt16 nSID,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    virtual void HandleContextChange(
        const vcl::EnumContext& rContext) override;
    virtual void DumpAsPropertyTree(::tools::JsonWriter&) override;

private:

    ViewShellBase& mrBase;

    std::unique_ptr<SvxPaperSizeListBox> mxPaperSizeBox;
    std::unique_ptr<weld::ComboBox> mxPaperOrientation;
    std::unique_ptr<weld::ComboBox> mxMasterSlide;
    std::unique_ptr<weld::Label> mxBackgroundLabel;
    std::unique_ptr<weld::ComboBox> mxFillStyle;
    std::unique_ptr<ColorListBox> mxFillLB;
    std::unique_ptr<weld::ComboBox> mxFillAttr;
    std::unique_ptr<ColorListBox> mxFillGrad1;
    std::unique_ptr<ColorListBox> mxFillGrad2;
    std::unique_ptr<weld::Button> mxInsertImage;
    std::unique_ptr<weld::CheckButton> mxDspMasterBackground;
    std::unique_ptr<weld::CheckButton> mxDspMasterObjects;
    std::unique_ptr<weld::Button> mxCloseMaster;
    std::unique_ptr<weld::Button> mxEditMaster;
    std::unique_ptr<weld::Label> mxMasterLabel;
    std::unique_ptr<weld::ComboBox> mxMarginSelectBox;
    std::unique_ptr<weld::Label> mxCustomEntry;
    std::unique_ptr<weld::Label> mxMarginLabel;

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

    bool mbSwitchModeToNormal;
    bool mbSwitchModeToMaster;

    css::uno::Reference<css::frame::XFrame> mxFrame;
    vcl::EnumContext maContext;
    vcl::EnumContext maDrawOtherContext;
    vcl::EnumContext maDrawMasterContext;
    vcl::EnumContext maImpressOtherContext;
    vcl::EnumContext maImpressMasterContext;
    vcl::EnumContext maImpressHandoutContext;
    vcl::EnumContext maImpressNotesContext;
    bool         mbTitle;
    std::unique_ptr<SvxLongLRSpaceItem> mpPageLRMarginItem;
    std::unique_ptr<SvxLongULSpaceItem> mpPageULMarginItem;
    ::tools::Long m_nPageLeftMargin;
    ::tools::Long m_nPageRightMargin;
    ::tools::Long m_nPageTopMargin;
    ::tools::Long m_nPageBottomMargin;
    FieldUnit meFUnit;
    OUString maCustomEntry;

    SfxBindings* mpBindings;

    MapUnit meUnit;

    DECL_LINK(FillBackgroundHdl, weld::ComboBox&, void);
    DECL_LINK(FillStyleModifyHdl, weld::ComboBox&, void);
    DECL_LINK(PaperSizeModifyHdl, weld::ComboBox&, void);
    DECL_LINK(FillColorHdl, ColorListBox&, void);
    DECL_LINK(AssignMasterPage, weld::ComboBox&, void);
    DECL_LINK(DspBackground, weld::Button&, void);
    DECL_LINK(DspObjects, weld::Button&, void);
    DECL_LINK(CloseMasterHdl, weld::Button&, void);
    DECL_LINK(EditMasterHdl, weld::Button&, void);
    DECL_LINK(SelectBgHdl, weld::Button&, void);
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent&, void );
    DECL_LINK( ModifyMarginHdl, weld::ComboBox&, void );

    void Initialize();
    void Update();
    void UpdateMarginBox();
    void SetPanelTitle(const OUString& rTitle);
    void SetMarginsFieldUnit();

    Color const & GetColorSetOrDefault();
    XGradient const & GetGradientSetOrDefault();
    OUString const & GetHatchingSetOrDefault();
    OUString const & GetBitmapSetOrDefault();
    OUString const & GetPatternSetOrDefault();
    bool IsDraw();
    bool IsImpress();
    void addListener();
    void removeListener();
    void ExecuteMarginLRChange(const ::tools::Long mnPageLeftMargin, const ::tools::Long mnPageRightMargin);
    void ExecuteMarginULChange(const ::tools::Long mnPageTopMargin, const ::tools::Long mnPageBottomMargin);
    void populateMasterSlideDropdown();
    void updateMasterSlideSelection();

    static FieldUnit GetCurrentUnit(SfxItemState eState, const SfxPoolItem* pState);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
