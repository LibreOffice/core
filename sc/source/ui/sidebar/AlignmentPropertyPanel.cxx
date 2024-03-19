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

#include "AlignmentPropertyPanel.hxx"
#include <editeng/justifyitem.hxx>
#include <sc.hrc>
#include <attrib.hxx>
#include <scitems.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svx/rotmodit.hxx>
#include <svtools/unitconv.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;
using namespace css::uno;

// namespace open

namespace sc::sidebar {

AlignmentPropertyPanel::AlignmentPropertyPanel(
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : PanelLayout(pParent, "AlignmentPropertyPanel", "modules/scalc/ui/sidebaralignment.ui")
    , mxFTLeftIndent(m_xBuilder->weld_label("leftindentlabel"))
    , mxMFLeftIndent(m_xBuilder->weld_metric_spin_button("leftindent", FieldUnit::POINT))
    , mxCBXWrapText(m_xBuilder->weld_check_button("wraptext"))
    , mxCBXMergeCell(m_xBuilder->weld_check_button("mergecells"))
    , mxFtRotate(m_xBuilder->weld_label("orientationlabel"))
    , mxMtrAngle(m_xBuilder->weld_metric_spin_button("orientationdegrees", FieldUnit::DEGREE))
    , mxRefEdgeBottom(m_xBuilder->weld_toggle_button("bottom"))
    , mxRefEdgeTop(m_xBuilder->weld_toggle_button("top"))
    , mxRefEdgeStd(m_xBuilder->weld_toggle_button("standard"))
    , mxCBStacked(m_xBuilder->weld_check_button("stacked"))
    , mxTextOrientBox(m_xBuilder->weld_widget("textorientbox"))
    , mxHorizontalAlign(m_xBuilder->weld_toolbar("horizontalalignment"))
    , mxHorizontalAlignDispatch(new ToolbarUnoDispatcher(*mxHorizontalAlign, *m_xBuilder, rxFrame))
    , mxVertAlign(m_xBuilder->weld_toolbar("verticalalignment"))
    , mxVertAlignDispatch(new ToolbarUnoDispatcher(*mxVertAlign, *m_xBuilder, rxFrame))
    , mxWriteDirection(m_xBuilder->weld_toolbar("writedirection"))
    , mxWriteDirectionDispatch(new ToolbarUnoDispatcher(*mxWriteDirection, *m_xBuilder, rxFrame))
    , mxIndentButtons(m_xBuilder->weld_toolbar("indentbuttons"))
    , mxIndentButtonsDispatch(new ToolbarUnoDispatcher(*mxIndentButtons, *m_xBuilder, rxFrame))
    , maAlignHorControl(SID_H_ALIGNCELL, *pBindings, *this)
    , maLeftIndentControl(SID_ATTR_ALIGN_INDENT, *pBindings, *this)
    , maMergeCellControl(FID_MERGE_TOGGLE, *pBindings, *this)
    , maWrapTextControl(SID_ATTR_ALIGN_LINEBREAK, *pBindings, *this)
    , maAngleControl(SID_ATTR_ALIGN_DEGREES, *pBindings, *this)
    , maVrtStackControl(SID_ATTR_ALIGN_STACKED, *pBindings, *this)
    , maRefEdgeControl(SID_ATTR_ALIGN_LOCKPOS, *pBindings, *this)
    , mbMultiDisable(false)
    , mpBindings(pBindings)
{
    Initialize();
}

AlignmentPropertyPanel::~AlignmentPropertyPanel()
{
    mxIndentButtonsDispatch.reset();
    mxIndentButtons.reset();
    mxWriteDirectionDispatch.reset();
    mxWriteDirection.reset();
    mxVertAlignDispatch.reset();
    mxVertAlign.reset();
    mxHorizontalAlignDispatch.reset();
    mxHorizontalAlign.reset();

    mxFTLeftIndent.reset();
    mxMFLeftIndent.reset();
    mxCBXWrapText.reset();
    mxCBXMergeCell.reset();
    mxFtRotate.reset();
    mxMtrAngle.reset();
    mxCBStacked.reset();
    mxRefEdgeBottom.reset();
    mxRefEdgeTop.reset();
    mxRefEdgeStd.reset();
    mxTextOrientBox.reset();

    maAlignHorControl.dispose();
    maLeftIndentControl.dispose();
    maMergeCellControl.dispose();
    maWrapTextControl.dispose();
    maAngleControl.dispose();
    maVrtStackControl.dispose();
    maRefEdgeControl.dispose();
}

void AlignmentPropertyPanel::Initialize()
{
    mxFTLeftIndent->set_sensitive(false);
    mxMFLeftIndent->set_sensitive(false);
    Link<weld::MetricSpinButton&,void> aLink = LINK(this, AlignmentPropertyPanel, MFLeftIndentMdyHdl);
    mxMFLeftIndent->connect_value_changed( aLink );

    mxCBXMergeCell->connect_toggled( LINK(this, AlignmentPropertyPanel, CBOXMergnCellClkHdl) );

    mxCBXWrapText->connect_toggled( LINK(this, AlignmentPropertyPanel, CBOXWrapTextClkHdl) );

    //rotation
    mxMtrAngle->connect_value_changed(LINK( this, AlignmentPropertyPanel, AngleModifiedHdl));
    mxCBStacked->connect_toggled(LINK(this, AlignmentPropertyPanel, ClickStackHdl));

    Link<weld::Button&,void> aLink2 = LINK(this, AlignmentPropertyPanel, ReferenceEdgeHdl);
    mxRefEdgeBottom->connect_clicked(aLink2);
    mxRefEdgeTop->connect_clicked(aLink2);
    mxRefEdgeStd->connect_clicked(aLink2);
}

IMPL_LINK(AlignmentPropertyPanel, ReferenceEdgeHdl, weld::Button&, rToggle, void)
{
    SvxRotateMode eMode;
    if (&rToggle == mxRefEdgeBottom.get())
        eMode = SVX_ROTATE_MODE_BOTTOM;
    else if (&rToggle == mxRefEdgeTop.get())
        eMode = SVX_ROTATE_MODE_TOP;
    else /*if (&rToggle == mxRefEdgeStd.get())*/
        eMode = SVX_ROTATE_MODE_STANDARD;

    mxRefEdgeBottom->set_active(eMode == SVX_ROTATE_MODE_BOTTOM);
    mxRefEdgeTop->set_active(eMode == SVX_ROTATE_MODE_TOP);
    mxRefEdgeStd->set_active(eMode == SVX_ROTATE_MODE_STANDARD);

    SvxRotateModeItem aItem(eMode, ATTR_ROTATE_MODE);
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_ALIGN_LOCKPOS,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG( AlignmentPropertyPanel, AngleModifiedHdl, weld::MetricSpinButton&, void )
{
    Degree100 nAngle(mxMtrAngle->get_value(FieldUnit::DEGREE) * 100);
    ScRotateValueItem aAngleItem(nAngle);

    GetBindings()->GetDispatcher()->ExecuteList(
        SID_ATTR_ALIGN_DEGREES, SfxCallMode::RECORD, { &aAngleItem });
}

IMPL_LINK_NOARG( AlignmentPropertyPanel, ClickStackHdl, weld::Toggleable&, void )
{
    bool bVertical = mxCBStacked->get_active();
    ScVerticalStackCell aStackItem(bVertical);
    GetBindings()->GetDispatcher()->ExecuteList(
        SID_ATTR_ALIGN_STACKED, SfxCallMode::RECORD, { &aStackItem });
}

IMPL_LINK_NOARG(AlignmentPropertyPanel, MFLeftIndentMdyHdl, weld::MetricSpinButton&, void)
{
    sal_uInt16 nVal = mxMFLeftIndent->get_value(FieldUnit::NONE);
    ScIndentItem aItem(static_cast<sal_uInt16>(CalcToUnit(nVal, MapUnit::MapTwip)));

    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_ALIGN_INDENT,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(AlignmentPropertyPanel, CBOXMergnCellClkHdl, weld::Toggleable&, void)
{
    bool bState = mxCBXMergeCell->get_active();

    if( bState)
        GetBindings()->GetDispatcher()->Execute(FID_MERGE_ON, SfxCallMode::RECORD);
    else
        GetBindings()->GetDispatcher()->Execute(FID_MERGE_OFF, SfxCallMode::RECORD);
    GetBindings()->Invalidate(FID_MERGE_TOGGLE,true);
}

IMPL_LINK_NOARG(AlignmentPropertyPanel, CBOXWrapTextClkHdl, weld::Toggleable&, void)
{
    bool bState = mxCBXWrapText->get_active();
    ScLineBreakCell aItem(bState);
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_ALIGN_LINEBREAK,
            SfxCallMode::RECORD, { &aItem });
}

std::unique_ptr<PanelLayout> AlignmentPropertyPanel::Create (
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to AlignmentPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to AlignmentPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to AlignmentPropertyPanel::Create", nullptr, 2);

    return std::make_unique<AlignmentPropertyPanel>(pParent, rxFrame, pBindings);
}

void AlignmentPropertyPanel::HandleContextChange(
    const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
}

void AlignmentPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    switch(nSID)
    {
    case SID_H_ALIGNCELL:
        {
            SvxCellHorJustify meHorAlignState = SvxCellHorJustify::Standard;
            if(eState >= SfxItemState::DEFAULT)
                if (auto pItem = dynamic_cast<const SvxHorJustifyItem*>( pState) )
                    meHorAlignState = pItem->GetValue();

            if( meHorAlignState == SvxCellHorJustify::Repeat )
            {
                mxFtRotate->set_sensitive(false);
                mxMtrAngle->set_sensitive(false);
            }
            else
            {
                mxFtRotate->set_sensitive(!mbMultiDisable);
                mxMtrAngle->set_sensitive(!mbMultiDisable);
            }

            mxFTLeftIndent->set_sensitive( meHorAlignState == SvxCellHorJustify::Left );
            mxMFLeftIndent->set_sensitive( meHorAlignState == SvxCellHorJustify::Left );
        }
        break;
    case SID_ATTR_ALIGN_INDENT:
        if(eState >= SfxItemState::DEFAULT && dynamic_cast<const SfxUInt16Item*>( pState) )
        {
                const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pState);
                sal_uInt16 nVal = pItem->GetValue();
                mxMFLeftIndent->set_value( CalcToPoint(nVal, MapUnit::MapTwip, 1), FieldUnit::NONE );
        }
        else
        {
            mxMFLeftIndent->set_value(0, FieldUnit::NONE);
            mxMFLeftIndent->set_text(OUString());
        }
        break;
    case FID_MERGE_TOGGLE:
        if(eState >= SfxItemState::DEFAULT && dynamic_cast<const SfxBoolItem*>( pState) )
        {
            mxCBXMergeCell->set_sensitive(true);
            const SfxBoolItem* pItem = static_cast<const SfxBoolItem*>(pState);
            mxCBXMergeCell->set_active(pItem->GetValue());
        }
        else
        {
            mxCBXMergeCell->set_active(false);
            mxCBXMergeCell->set_sensitive(false);
        }
        break;

    case SID_ATTR_ALIGN_LINEBREAK:
        if(eState == SfxItemState::DISABLED)
        {
            mxCBXWrapText->set_active(false);
            mxCBXWrapText->set_sensitive(false);
        }
        else
        {
            mxCBXWrapText->set_sensitive(true);
            if(eState >= SfxItemState::DEFAULT && dynamic_cast<const ScLineBreakCell*>( pState) )
            {
                const ScLineBreakCell* pItem = static_cast<const ScLineBreakCell*>(pState);
                mxCBXWrapText->set_active(pItem->GetValue());
            }
            else if(eState == SfxItemState::INVALID)
            {
                mxCBXWrapText->set_state(TRISTATE_INDET);
            }
        }
        break;
    case SID_ATTR_ALIGN_STACKED:
        if (eState >= SfxItemState::DEFAULT)
        {
            const SfxBoolItem* pStackItem = static_cast<const ScVerticalStackCell*>(pState);
            mbMultiDisable = pStackItem->GetValue();
            mxCBStacked->set_active(mbMultiDisable);
            mxTextOrientBox->set_sensitive(!mbMultiDisable);
        }
        else
        {
            mbMultiDisable = true;
            mxTextOrientBox->set_sensitive(false);
            mxCBStacked->set_state(TRISTATE_INDET);
        }
        break;
    case SID_ATTR_ALIGN_LOCKPOS:
        if( eState >= SfxItemState::DEFAULT)
        {
            const SvxRotateModeItem* pItem = static_cast<const SvxRotateModeItem*>(pState);
            SvxRotateMode eMode = pItem->GetValue();
            mxRefEdgeBottom->set_active(eMode == SVX_ROTATE_MODE_BOTTOM);
            mxRefEdgeTop->set_active(eMode == SVX_ROTATE_MODE_TOP);
            mxRefEdgeStd->set_active(eMode == SVX_ROTATE_MODE_STANDARD);
        }
        break;
    case SID_ATTR_ALIGN_DEGREES:
        if (eState >= SfxItemState::DEFAULT)
        {
            Degree100 nTmp = static_cast<const ScRotateValueItem*>(pState)->GetValue();
            mxMtrAngle->set_value(toDegrees(nTmp), FieldUnit::DEGREE);
        }
        else
        {
            mxMtrAngle->set_text( OUString() );
        }
        break;
    }
}

// namespace close

} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
