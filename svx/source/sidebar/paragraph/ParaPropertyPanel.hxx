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

#include <editeng/hyphenzoneitem.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svx/relfld.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XSidebar.hpp>

#include <svl/poolitem.hxx>
#include <tools/fldunit.hxx>
#include <vcl/EnumContext.hxx>

class ToolbarUnoDispatcher;

namespace svx::sidebar {

class ParaPropertyPanel
    : public PanelLayout,
      public ::sfx2::sidebar::IContextChangeReceiver,
      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~ParaPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create (
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);

    SfxBindings* GetBindings() { return mpBindings;}

    virtual void HandleContextChange (
        const vcl::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    static FieldUnit GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState );

    ParaPropertyPanel (
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        css::uno::Reference<css::ui::XSidebar> xSidebar);

private:
    // UI controls
    //Alignment
    std::unique_ptr<weld::Toolbar> mxTBxHorzAlign;
    std::unique_ptr<ToolbarUnoDispatcher> mxHorzAlignDispatch;
    std::unique_ptr<weld::Toolbar> mxTBxVertAlign;
    std::unique_ptr<ToolbarUnoDispatcher> mxVertAlignDispatch;
    //NumBullet&Backcolor
    std::unique_ptr<weld::Toolbar> mxTBxNumBullet;
    std::unique_ptr<ToolbarUnoDispatcher> mxNumBulletDispatch;
    std::unique_ptr<weld::Toolbar> mxTBxBackColor;
    std::unique_ptr<ToolbarUnoDispatcher> mxBackColorDispatch;

    std::unique_ptr<weld::Toolbar> mxTBxWriteDirection;
    std::unique_ptr<ToolbarUnoDispatcher> mxWriteDirectionDispatch;
    std::unique_ptr<weld::Toolbar> mxTBxParaSpacing;
    std::unique_ptr<ToolbarUnoDispatcher> mxParaSpacingDispatch;
    std::unique_ptr<weld::Toolbar> mxTBxLineSpacing;
    std::unique_ptr<ToolbarUnoDispatcher> mxLineSpacingDispatch;
    std::unique_ptr<weld::Toolbar> mxTBxIndent;
    std::unique_ptr<ToolbarUnoDispatcher> mxIndentDispatch;
    std::unique_ptr<weld::Toolbar> mxTBxHyphenation;

    //Paragraph spacing
    std::optional<SvxRelativeField> mxTopDist;
    std::optional<SvxRelativeField> mxBottomDist;
    std::optional<SvxRelativeField> mxLeftIndent;
    std::optional<SvxRelativeField> mxRightIndent;
    std::optional<SvxRelativeField> mxFLineIndent;

    // Hyphenation
    std::unique_ptr<weld::Label> mxHyphenationLabel;
    std::unique_ptr<weld::Label> mxAtLineEndLabel;
    std::unique_ptr<weld::Label> mxAtLineBeginLabel;
    std::unique_ptr<weld::Label> mxConsecutiveLabel;
    std::unique_ptr<weld::Label> mxCompoundLabel;
    std::unique_ptr<weld::Label> mxWordLengthLabel;
    std::unique_ptr<weld::Label> mxZoneLabel;
    std::optional<SvxRelativeField> mxAtLineEnd;
    std::optional<SvxRelativeField> mxAtLineBegin;
    std::optional<SvxRelativeField> mxConsecutive;
    std::optional<SvxRelativeField> mxCompound;
    std::optional<SvxRelativeField> mxWordLength;
    std::optional<SvxRelativeField> mxZone;
    bool mbKeepLine;
    bool mbUpdatingHyphenateButtons;

    // Data Member
    tools::Long                maTxtLeft;
    tools::Long                    maUpper;
    tools::Long                    maLower;
    tools::Long                    maZone;

    FieldUnit                       m_eMetricUnit;
    FieldUnit                       m_last_eMetricUnit;
    MapUnit                         m_eLRSpaceUnit;
    MapUnit                         m_eULSpaceUnit;
    MapUnit                         m_eHyphenZoneUnit;
    // Control Items
    ::sfx2::sidebar::ControllerItem  maLRSpaceControl;
    ::sfx2::sidebar::ControllerItem  maULSpaceControl;
    ::sfx2::sidebar::ControllerItem  m_aMetricCtl;
    ::sfx2::sidebar::ControllerItem  m_aAtLineEndControl;
    ::sfx2::sidebar::ControllerItem  m_aAtLineBeginControl;
    ::sfx2::sidebar::ControllerItem  m_aConsecutiveControl;
    ::sfx2::sidebar::ControllerItem  m_aCompoundControl;
    ::sfx2::sidebar::ControllerItem  m_aWordLengthControl;
    ::sfx2::sidebar::ControllerItem  m_aZoneControl;

    vcl::EnumContext maContext;
    SfxBindings* mpBindings;
    css::uno::Reference<css::ui::XSidebar> mxSidebar;

    DECL_LINK(ModifyIndentHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ULSpaceHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(HyphenationHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(HyphenationToggleButtonHdl_Impl, const OUString&, void);

    void StateChangedIndentImpl( SfxItemState eState, const SfxPoolItem* pState );
    void StateChangedULImpl( SfxItemState eState, const SfxPoolItem* pState );
    void StateChangedHyphenationImpl( SfxItemState eState, const SfxPoolItem* pState );

    void initial();
    void set_hyphenation_base_visible(bool bVisible);
    void set_hyphenation_other_visible(bool bVisible);
    void fill_hyphenzone(SvxHyphenZoneItem& rHyphen);
    void ReSize();
    void InitToolBoxIndent();
    void InitToolBoxSpacing();
    void InitToolBoxHyphenation();
    void limitMetricWidths();
};

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
