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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARAPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARAPROPERTYPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <editeng/lspcitem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svx/relfld.hxx>
#include <editeng/svxenum.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XSidebar.hpp>

#include <vcl/vclenum.hxx>
#include <svl/poolitem.hxx>
#include <tools/fldunit.hxx>

class ToolBox;

namespace svx { namespace sidebar {

class ParaPropertyPanel
    : public PanelLayout,
      public ::sfx2::sidebar::IContextChangeReceiver,
      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~ParaPropertyPanel();
    virtual void dispose() override;

    static VclPtr<vcl::Window> Create (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);

    virtual void DataChanged (const DataChangedEvent& rEvent) override;
    SfxBindings* GetBindings() { return mpBindings;}

    virtual void HandleContextChange (
        const ::sfx2::sidebar::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

    static FieldUnit GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState );

    ParaPropertyPanel (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);

private:
    // UI controls
    //Alignment
    VclPtr<ToolBox>            mpTBxVertAlign;
    //NumBullet&Backcolor
    VclPtr<ToolBox>            mpTBxNumBullet;
    VclPtr<ToolBox>            mpTBxOutline;
    VclPtr<ToolBox>            mpTBxBackColor;
    //Paragraph spacing
    VclPtr<SvxRelativeField>   mpTopDist;
    VclPtr<SvxRelativeField>   mpBottomDist;
    VclPtr<ToolBox>            mpTbxIndent_IncDec;
    VclPtr<SvxRelativeField>   mpLeftIndent;
    VclPtr<SvxRelativeField>   mpRightIndent;
    VclPtr<SvxRelativeField>   mpFLineIndent;

    // Resources
    Image  maSpace3;
    Image  maIndHang;

    // Data Member
    long                maTxtLeft;
    long                    maUpper;
    long                    maLower;

    FieldUnit                       m_eMetricUnit;
    FieldUnit                       m_last_eMetricUnit;
    SfxMapUnit                      m_eLRSpaceUnit;
    SfxMapUnit                      m_eULSpaceUnit;
    // Control Items
    ::sfx2::sidebar::ControllerItem  maLRSpaceControl;
    ::sfx2::sidebar::ControllerItem  maULSpaceControl;
    ::sfx2::sidebar::ControllerItem  maDecIndentControl;
    ::sfx2::sidebar::ControllerItem  maIncIndentControl;
    ::sfx2::sidebar::ControllerItem  m_aMetricCtl;

    css::uno::Reference<css::frame::XFrame> mxFrame;
    ::sfx2::sidebar::EnumContext maContext;
    SfxBindings* mpBindings;
    css::uno::Reference<css::ui::XSidebar> mxSidebar;

    DECL_LINK_TYPED(ModifyIndentHdl_Impl, Edit&, void);
    DECL_LINK_TYPED(ClickIndent_IncDec_Hdl_Impl, ToolBox*, void);
    DECL_LINK_TYPED(ULSpaceHdl_Impl, Edit&, void);

    void StateChangedIndentImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void StateChangedULImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void StateChangeIncDecImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

    void initial();
    void ReSize(bool bSize);
    void InitToolBoxIndent();
    void InitToolBoxSpacing();
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
