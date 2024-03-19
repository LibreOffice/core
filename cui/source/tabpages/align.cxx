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

#include <align.hxx>

#include <editeng/svxenum.hxx>
#include <svx/svxids.hrc>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <bitmaps.hlst>
#include <svx/rotmodit.hxx>
#include <svx/sdangitm.hxx>

#include <editeng/frmdiritem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/intitem.hxx>
#include <vcl/image.hxx>

#define IID_BOTTOMLOCK 1
#define IID_TOPLOCK    2
#define IID_CELLLOCK   3

namespace svx {

const WhichRangesContainer AlignmentTabPage::s_pRanges(
    svl::Items<
    SID_ATTR_ALIGN_STACKED, SID_ATTR_ALIGN_LINEBREAK, // 10229 - 10230
    SID_ATTR_ALIGN_INDENT, SID_ATTR_ALIGN_INDENT, // 10460 - 10460
    SID_ATTR_ALIGN_DEGREES, SID_ATTR_ALIGN_DEGREES, // 10577 - 10577
    SID_ATTR_ALIGN_LOCKPOS, SID_ATTR_ALIGN_LOCKPOS, // 10578 - 10578
    SID_ATTR_ALIGN_HYPHENATION, SID_ATTR_ALIGN_HYPHENATION, // 10931 - 10931
    SID_ATTR_FRAMEDIRECTION, SID_ATTR_FRAMEDIRECTION, // 10944 - 10944
    SID_ATTR_ALIGN_ASIANVERTICAL, SID_ATTR_ALIGN_ASIANVERTICAL, // 10949 - 10949
    SID_ATTR_ALIGN_SHRINKTOFIT, SID_ATTR_ALIGN_SHRINKTOFIT, // 11015 - 11015
    SID_ATTR_ALIGN_HOR_JUSTIFY, SID_ATTR_ALIGN_VER_JUSTIFY>); // 11571 - 11572


namespace {

template<typename JustContainerType, typename JustEnumType>
void lcl_MaybeResetAlignToDistro(
    weld::ComboBox& rLB, sal_uInt16 nListId, const SfxItemSet& rCoreAttrs, TypedWhichId<SfxEnumItemInterface> nWhichAlign, TypedWhichId<SfxEnumItemInterface> nWhichJM, JustEnumType eBlock)
{
    const SfxEnumItemInterface* p = rCoreAttrs.GetItemIfSet(nWhichAlign);
    if (!p)
        // alignment not set.
        return;

    JustContainerType eVal = static_cast<JustContainerType>(p->GetEnumValue());
    if (eVal != eBlock)
        // alignment is not 'justify'.  No need to go further.
        return;

    p = rCoreAttrs.GetItemIfSet(nWhichJM);
    if (!p)
        // justification method is not set.
        return;

    SvxCellJustifyMethod eMethod = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    if (eMethod == SvxCellJustifyMethod::Distribute)
    {
        // Select the 'distribute' entry in the specified list box.
        rLB.set_active_id(OUString::number(nListId));
    }
}

void lcl_SetJustifyMethodToItemSet(SfxItemSet& rSet, const SfxItemSet& rOldSet, sal_uInt16 nWhichJM, const weld::ComboBox& rLB, sal_uInt16 nListId)
{
    // tdf#138698 unsupported, e.g. dbaccess
    if (rLB.find_id(OUString::number(nListId)) == -1)
        return;

    // feature supported , e.g. calc
    SvxCellJustifyMethod eJM = SvxCellJustifyMethod::Auto;
    if (rLB.get_active_id().toInt32() == nListId)
        eJM = SvxCellJustifyMethod::Distribute;

    // tdf#129300 If it would create no change, don't force it
    const SvxJustifyMethodItem& rOldItem = static_cast<const SvxJustifyMethodItem&>(rOldSet.Get(nWhichJM));
    if (rOldItem.GetValue() == eJM)
    {
        rSet.InvalidateItem(nWhichJM);
        return;
    }

    SvxJustifyMethodItem aItem(eJM, nWhichJM);
    rSet.Put(aItem);
}

}//namespace

AlignmentTabPage::AlignmentTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pPage, pController, "cui/ui/cellalignment.ui", "CellAlignPage", &rCoreAttrs)
    , m_aVsRefEdge(nullptr)
    // text alignment
    , m_xLbHorAlign(m_xBuilder->weld_combo_box("comboboxHorzAlign"))
    , m_xFtIndent(m_xBuilder->weld_label("labelIndent"))
    , m_xEdIndent(m_xBuilder->weld_metric_spin_button("spinIndentFrom", FieldUnit::POINT))
    , m_xFtVerAlign(m_xBuilder->weld_label("labelVertAlign"))
    , m_xLbVerAlign(m_xBuilder->weld_combo_box("comboboxVertAlign"))
    //text rotation
    , m_xFtRotate(m_xBuilder->weld_label("labelDegrees"))
    , m_xNfRotate(m_xBuilder->weld_metric_spin_button("spinDegrees", FieldUnit::DEGREE))
    , m_xFtRefEdge(m_xBuilder->weld_label("labelRefEdge"))
    //Asian mode
    , m_xCbStacked(m_xBuilder->weld_check_button("checkVertStack"))
    , m_xCbAsianMode(m_xBuilder->weld_check_button("checkAsianMode"))
    // Properties
    , m_xBtnWrap(m_xBuilder->weld_check_button("checkWrapTextAuto"))
    , m_xBtnHyphen(m_xBuilder->weld_check_button("checkHyphActive"))
    , m_xBtnShrink(m_xBuilder->weld_check_button("checkShrinkFitCellSize"))
    , m_xLbFrameDir(new svx::FrameDirectionListBox(m_xBuilder->weld_combo_box("comboTextDirBox")))
    //ValueSet hover strings
    , m_xFtBotLock(m_xBuilder->weld_label("labelSTR_BOTTOMLOCK"))
    , m_xFtTopLock(m_xBuilder->weld_label("labelSTR_TOPLOCK"))
    , m_xFtCelLock(m_xBuilder->weld_label("labelSTR_CELLLOCK"))
    , m_xFtABCD(m_xBuilder->weld_label("labelABCD"))
    , m_xAlignmentFrame(m_xBuilder->weld_widget("alignment"))
    , m_xOrientFrame(m_xBuilder->weld_widget("orientation"))
    , m_xPropertiesFrame(m_xBuilder->weld_widget("properties"))
    , m_xVsRefEdge(new weld::CustomWeld(*m_xBuilder, "references", m_aVsRefEdge))
    , m_xCtrlDial(new DialControl)
    , m_xCtrlDialWin(new weld::CustomWeld(*m_xBuilder, "dialcontrol", *m_xCtrlDial))
{
    m_xCtrlDial->SetLinkedField(m_xNfRotate.get());
    m_xCtrlDial->SetText(m_xFtABCD->get_label());

    InitVsRefEgde();

    m_xLbHorAlign->connect_changed(LINK(this, AlignmentTabPage, UpdateEnableHdl));

    m_xCbStacked->connect_toggled(LINK(this, AlignmentTabPage, StackedClickHdl));
    m_xCbAsianMode->connect_toggled(LINK(this, AlignmentTabPage, AsianModeClickHdl));
    m_xBtnWrap->connect_toggled(LINK(this, AlignmentTabPage, WrapClickHdl));
    m_xBtnHyphen->connect_toggled(LINK(this, AlignmentTabPage, HyphenClickHdl));
    m_xBtnShrink->connect_toggled(LINK(this, AlignmentTabPage, ShrinkClickHdl));

    // Asian vertical mode
    m_xCbAsianMode->set_visible(SvtCJKOptions::IsVerticalTextEnabled());

    m_xLbFrameDir->append(SvxFrameDirection::Horizontal_LR_TB, SvxResId(RID_SVXSTR_FRAMEDIR_LTR));
    m_xLbFrameDir->append(SvxFrameDirection::Horizontal_RL_TB, SvxResId(RID_SVXSTR_FRAMEDIR_RTL));
    m_xLbFrameDir->append(SvxFrameDirection::Environment, SvxResId(RID_SVXSTR_FRAMEDIR_SUPER));

    // This page needs ExchangeSupport.
    SetExchangeSupport();
}

AlignmentTabPage::~AlignmentTabPage()
{
    m_xCtrlDialWin.reset();
    m_xCtrlDial.reset();
    m_xVsRefEdge.reset();
    m_xLbFrameDir.reset();
}

std::unique_ptr<SfxTabPage> AlignmentTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<AlignmentTabPage>(pPage, pController, *rAttrSet);
}

bool AlignmentTabPage::FillItemSet( SfxItemSet* rSet )
{
    const SfxItemSet& rOldSet = GetItemSet();

    bool bChanged = SfxTabPage::FillItemSet(rSet);

    sal_uInt16 nWhich = GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY);
    if (m_xLbHorAlign->get_value_changed_from_saved())
    {
        SvxCellHorJustify eJustify(SvxCellHorJustify::Standard);
        switch (m_xLbHorAlign->get_active_id().toInt32())
        {
            case ALIGNDLG_HORALIGN_STD:
                eJustify = SvxCellHorJustify::Standard;
                break;
            case ALIGNDLG_HORALIGN_LEFT:
                eJustify = SvxCellHorJustify::Left;
                break;
            case ALIGNDLG_HORALIGN_CENTER:
                eJustify = SvxCellHorJustify::Center;
                break;
            case ALIGNDLG_HORALIGN_RIGHT:
                eJustify = SvxCellHorJustify::Right;
                break;
            case ALIGNDLG_HORALIGN_BLOCK:
            case ALIGNDLG_HORALIGN_DISTRIBUTED:
                eJustify = SvxCellHorJustify::Block;
                break;
            case ALIGNDLG_HORALIGN_FILL:
                eJustify = SvxCellHorJustify::Repeat;
                break;
        }
        rSet->Put(SvxHorJustifyItem(eJustify, nWhich));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    nWhich = GetWhich(SID_ATTR_ALIGN_INDENT);
    if (m_xEdIndent->get_value_changed_from_saved())
    {
        const SfxUInt16Item* pIndentItem = static_cast<const SfxUInt16Item*>(GetOldItem(
                                                *rSet, SID_ATTR_ALIGN_INDENT));
        assert(pIndentItem);
        std::unique_ptr<SfxUInt16Item> pNewIndentItem(pIndentItem->Clone());
        pNewIndentItem->SetValue(m_xEdIndent->get_value(FieldUnit::TWIP));
        rSet->Put(std::move(pNewIndentItem));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    nWhich = GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY);
    if (m_xLbVerAlign->get_value_changed_from_saved())
    {
        SvxCellVerJustify eJustify(SvxCellVerJustify::Standard);
        switch (m_xLbVerAlign->get_active_id().toInt32())
        {
            case ALIGNDLG_VERALIGN_STD:
                eJustify = SvxCellVerJustify::Standard;
                break;
            case ALIGNDLG_VERALIGN_TOP:
                eJustify = SvxCellVerJustify::Top;
                break;
            case ALIGNDLG_VERALIGN_MID:
                eJustify = SvxCellVerJustify::Center;
                break;
            case ALIGNDLG_VERALIGN_BOTTOM:
                eJustify = SvxCellVerJustify::Bottom;
                break;
            case ALIGNDLG_VERALIGN_BLOCK:
            case ALIGNDLG_VERALIGN_DISTRIBUTED:
                eJustify = SvxCellVerJustify::Block;
                break;
        }
        rSet->Put(SvxVerJustifyItem(eJustify, nWhich));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    nWhich = GetWhich(SID_ATTR_ALIGN_DEGREES);
    if (m_xNfRotate->get_value_changed_from_saved())
    {
        const SdrAngleItem* pAngleItem = static_cast<const SdrAngleItem*>(GetOldItem(
                                            *rSet, SID_ATTR_ALIGN_DEGREES));
        assert(pAngleItem);
        std::unique_ptr<SdrAngleItem> pNewAngleItem(pAngleItem->Clone());
        pNewAngleItem->SetValue(m_xCtrlDial->GetRotation());
        rSet->Put(std::move(pNewAngleItem));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    TypedWhichId<SvxRotateModeItem> nWhichLockPos(GetWhich(SID_ATTR_ALIGN_LOCKPOS));
    if (m_aVsRefEdge.IsValueChangedFromSaved())
    {
        switch (m_aVsRefEdge.GetSelectedItemId())
        {
            case IID_CELLLOCK:
                rSet->Put(SvxRotateModeItem(SvxRotateMode::SVX_ROTATE_MODE_STANDARD, nWhichLockPos));
                break;
            case IID_TOPLOCK:
                rSet->Put(SvxRotateModeItem(SvxRotateMode::SVX_ROTATE_MODE_TOP, nWhichLockPos));
                break;
            case IID_BOTTOMLOCK:
                rSet->Put(SvxRotateModeItem(SvxRotateMode::SVX_ROTATE_MODE_BOTTOM, nWhichLockPos));
                break;
            default:
                m_aVsRefEdge.SetNoSelection();
                break;
        }
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhichLockPos, false))
        rSet->InvalidateItem(nWhichLockPos);

    nWhich = GetWhich(SID_ATTR_ALIGN_STACKED);
    if (m_xCbStacked->get_state_changed_from_saved())
    {
        const SfxBoolItem* pStackItem = static_cast<const SfxBoolItem*>(GetOldItem(
                                            *rSet, SID_ATTR_ALIGN_STACKED));
        assert(pStackItem);
        std::unique_ptr<SfxBoolItem> pNewStackItem(pStackItem->Clone());
        pNewStackItem->SetValue(m_xCbStacked->get_active());
        rSet->Put(std::move(pNewStackItem));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    nWhich = GetWhich(SID_ATTR_ALIGN_ASIANVERTICAL);
    if (m_xCbAsianMode->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(nWhich, m_xCbAsianMode->get_active()));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    nWhich = GetWhich(SID_ATTR_ALIGN_LINEBREAK);
    if (m_xBtnWrap->get_state_changed_from_saved())
    {
        const SfxBoolItem* pWrapItem = static_cast<const SfxBoolItem*>(GetOldItem(
                                            *rSet, SID_ATTR_ALIGN_LINEBREAK));
        assert(pWrapItem);
        std::unique_ptr<SfxBoolItem> pNewWrapItem(pWrapItem->Clone());
        pNewWrapItem->SetValue(m_xBtnWrap->get_active());
        rSet->Put(std::move(pNewWrapItem));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    nWhich = GetWhich(SID_ATTR_ALIGN_HYPHENATION);
    if (m_xBtnHyphen->get_state_changed_from_saved())
    {
        const SfxBoolItem* pHyphItem = static_cast<const SfxBoolItem*>(GetOldItem(
                                            *rSet, SID_ATTR_ALIGN_HYPHENATION));
        assert(pHyphItem);
        std::unique_ptr<SfxBoolItem> pNewHyphItem(pHyphItem->Clone());
        pNewHyphItem->SetValue(m_xBtnHyphen->get_active());
        rSet->Put(std::move(pNewHyphItem));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    nWhich = GetWhich(SID_ATTR_ALIGN_SHRINKTOFIT);
    if (m_xBtnShrink->get_state_changed_from_saved())
    {
        const SfxBoolItem* pShrinkItem = static_cast<const SfxBoolItem*>(GetOldItem(
                                            *rSet, SID_ATTR_ALIGN_SHRINKTOFIT));
        assert(pShrinkItem);
        std::unique_ptr<SfxBoolItem> pNewShrinkItem(pShrinkItem->Clone());
        pNewShrinkItem->SetValue(m_xBtnShrink->get_active());
        rSet->Put(std::move(pNewShrinkItem));
        bChanged = true;
    }
    else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
        rSet->InvalidateItem(nWhich);

    if (m_xLbFrameDir->get_visible())
    {
        nWhich = GetWhich(SID_ATTR_FRAMEDIRECTION);
        if (m_xLbFrameDir->get_value_changed_from_saved())
        {
            SvxFrameDirection eDir = m_xLbFrameDir->get_active_id();
            rSet->Put(SvxFrameDirectionItem(eDir, nWhich));
            bChanged = true;
        }
        else if (SfxItemState::DEFAULT == rOldSet.GetItemState(nWhich, false))
            rSet->InvalidateItem(nWhich);
    }

    // Special treatment for distributed alignment; we need to set the justify
    // method to 'distribute' to distinguish from the normal justification.
    TypedWhichId<SfxEnumItemInterface> nWhichHorJM(GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD));
    lcl_SetJustifyMethodToItemSet(*rSet, rOldSet, nWhichHorJM, *m_xLbHorAlign, ALIGNDLG_HORALIGN_DISTRIBUTED);
    if (!bChanged)
        bChanged = HasAlignmentChanged(*rSet, nWhichHorJM);

    TypedWhichId<SfxEnumItemInterface> nWhichVerJM(GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY_METHOD));
    lcl_SetJustifyMethodToItemSet(*rSet, rOldSet, nWhichVerJM, *m_xLbVerAlign, ALIGNDLG_VERALIGN_DISTRIBUTED);
    if (!bChanged)
        bChanged = HasAlignmentChanged(*rSet, nWhichVerJM);

    return bChanged;
}

namespace
{
    void ResetBool(sal_uInt16 nWhich, const SfxItemSet* pSet, weld::CheckButton& rBtn, weld::TriStateEnabled& rTriState)
    {
        SfxItemState eState = pSet->GetItemState(nWhich);
        switch (eState)
        {
            case SfxItemState::UNKNOWN:
                rBtn.hide();
                rTriState.bTriStateEnabled = false;
                break;
            case SfxItemState::DISABLED:
                rBtn.set_sensitive(false);
                rTriState.bTriStateEnabled = false;
                break;
            case SfxItemState::INVALID:
                rBtn.set_state(TRISTATE_INDET);
                rTriState.bTriStateEnabled = true;
                break;
            case SfxItemState::DEFAULT:
            case SfxItemState::SET:
            {
                const SfxBoolItem& rItem = static_cast<const SfxBoolItem&>(pSet->Get(nWhich));
                rBtn.set_state(static_cast<TriState>(rItem.GetValue()));
                rTriState.bTriStateEnabled = false;
                break;
            }
        }
        rBtn.save_state();
    }
}

void AlignmentTabPage::Reset(const SfxItemSet* pCoreAttrs)
{
    SfxTabPage::Reset(pCoreAttrs);

    ResetBool(GetWhich(SID_ATTR_ALIGN_STACKED), pCoreAttrs, *m_xCbStacked, m_aStackedState);
    ResetBool(GetWhich(SID_ATTR_ALIGN_ASIANVERTICAL), pCoreAttrs, *m_xCbAsianMode, m_aAsianModeState);
    ResetBool(GetWhich(SID_ATTR_ALIGN_LINEBREAK), pCoreAttrs, *m_xBtnWrap, m_aWrapState);
    ResetBool(GetWhich(SID_ATTR_ALIGN_HYPHENATION), pCoreAttrs, *m_xBtnHyphen, m_aHyphenState);
    ResetBool(GetWhich(SID_ATTR_ALIGN_SHRINKTOFIT), pCoreAttrs, *m_xBtnShrink, m_aShrinkState);

    sal_uInt16 nWhich = GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY);
    SfxItemState eState = pCoreAttrs->GetItemState(nWhich);
    switch (eState)
    {
        case SfxItemState::UNKNOWN:
            m_xLbHorAlign->hide();
            break;
        case SfxItemState::DISABLED:
            m_xLbHorAlign->set_sensitive(false);
            break;
        case SfxItemState::INVALID:
            m_xLbHorAlign->set_active(-1);
            break;
        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxHorJustifyItem& rJustifyItem = static_cast<const SvxHorJustifyItem&>(pCoreAttrs->Get(nWhich));
            switch (rJustifyItem.GetValue())
            {
                case SvxCellHorJustify::Standard:
                    m_xLbHorAlign->set_active_id(OUString::number(ALIGNDLG_HORALIGN_STD));
                    break;
                case SvxCellHorJustify::Left:
                    m_xLbHorAlign->set_active_id(OUString::number(ALIGNDLG_HORALIGN_LEFT));
                    break;
                case SvxCellHorJustify::Center:
                    m_xLbHorAlign->set_active_id(OUString::number(ALIGNDLG_HORALIGN_CENTER));
                    break;
                case SvxCellHorJustify::Right:
                    m_xLbHorAlign->set_active_id(OUString::number(ALIGNDLG_HORALIGN_RIGHT));
                    break;
                case SvxCellHorJustify::Block:
                    m_xLbHorAlign->set_active_id(OUString::number(ALIGNDLG_HORALIGN_BLOCK));
                    break;
                case SvxCellHorJustify::Repeat:
                    m_xLbHorAlign->set_active_id(OUString::number(ALIGNDLG_HORALIGN_FILL));
                    break;
            }
            break;
        }
    }

    nWhich = GetWhich(SID_ATTR_ALIGN_INDENT);
    eState = pCoreAttrs->GetItemState(nWhich);
    switch (eState)
    {
        case SfxItemState::UNKNOWN:
            m_xEdIndent->hide();
            m_xFtIndent->hide();
            break;
        case SfxItemState::DISABLED:
            m_xEdIndent->set_sensitive(false);
            break;
        case SfxItemState::INVALID:
            m_xEdIndent->set_text("");
            break;
        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SfxUInt16Item& rIndentItem = static_cast<const SfxUInt16Item&>(pCoreAttrs->Get(nWhich));
            m_xEdIndent->set_value(rIndentItem.GetValue(), FieldUnit::TWIP);
            break;
        }
    }

    nWhich = GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY);
    eState = pCoreAttrs->GetItemState(nWhich);
    switch (eState)
    {
        case SfxItemState::UNKNOWN:
            m_xLbVerAlign->hide();
            m_xFtVerAlign->hide();
            break;
        case SfxItemState::DISABLED:
            m_xLbVerAlign->set_sensitive(false);
            break;
        case SfxItemState::INVALID:
            m_xLbVerAlign->set_active(-1);
            break;
        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxVerJustifyItem& rJustifyItem = static_cast<const SvxVerJustifyItem&>(pCoreAttrs->Get(nWhich));
            switch (rJustifyItem.GetValue())
            {
                case SvxCellVerJustify::Standard:
                    m_xLbVerAlign->set_active_id(OUString::number(ALIGNDLG_VERALIGN_STD));
                    break;
                case SvxCellVerJustify::Top:
                    m_xLbVerAlign->set_active_id(OUString::number(ALIGNDLG_VERALIGN_TOP));
                    break;
                case SvxCellVerJustify::Center:
                    m_xLbVerAlign->set_active_id(OUString::number(ALIGNDLG_VERALIGN_MID));
                    break;
                case SvxCellVerJustify::Bottom:
                    m_xLbVerAlign->set_active_id(OUString::number(ALIGNDLG_VERALIGN_BOTTOM));
                    break;
                case SvxCellVerJustify::Block:
                    m_xLbVerAlign->set_active_id(OUString::number(ALIGNDLG_VERALIGN_BLOCK));
                    break;
            }
            break;
        }
    }

    nWhich = GetWhich(SID_ATTR_ALIGN_DEGREES);
    eState = pCoreAttrs->GetItemState(nWhich);
    switch (eState)
    {
        case SfxItemState::UNKNOWN:
            m_xNfRotate->hide();
            m_xCtrlDialWin->hide();
            break;
        case SfxItemState::DISABLED:
            m_xNfRotate->set_sensitive(false);
            m_xCtrlDialWin->set_sensitive(false);
            break;
        case SfxItemState::INVALID:
            m_xCtrlDial->SetNoRotation();
            break;
        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SdrAngleItem& rAlignItem = static_cast<const SdrAngleItem&>(pCoreAttrs->Get(nWhich));
            m_xCtrlDial->SetRotation(rAlignItem.GetValue());
            break;
        }
    }

    nWhich = GetWhich(SID_ATTR_ALIGN_LOCKPOS);
    eState = pCoreAttrs->GetItemState(nWhich);
    switch (eState)
    {
        case SfxItemState::UNKNOWN:
            m_xVsRefEdge->hide();
            break;
        case SfxItemState::DISABLED:
            m_xVsRefEdge->set_sensitive(false);
            break;
        case SfxItemState::INVALID:
            m_aVsRefEdge.SetNoSelection();
            break;
        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxRotateModeItem& rRotateModeItem = static_cast<const SvxRotateModeItem&>(pCoreAttrs->Get(nWhich));
            switch (rRotateModeItem.GetValue())
            {
                case SvxRotateMode::SVX_ROTATE_MODE_STANDARD:
                    m_aVsRefEdge.SelectItem(IID_CELLLOCK);
                    break;
                case SvxRotateMode::SVX_ROTATE_MODE_TOP:
                    m_aVsRefEdge.SelectItem(IID_TOPLOCK);
                    break;
                case SvxRotateMode::SVX_ROTATE_MODE_BOTTOM:
                    m_aVsRefEdge.SelectItem(IID_BOTTOMLOCK);
                    break;
                default:
                    m_aVsRefEdge.SetNoSelection();
                    break;
            }
            break;
        }
    }
    m_aVsRefEdge.SaveValue();

    //text direction
    nWhich = GetWhich(SID_ATTR_FRAMEDIRECTION);
    eState = pCoreAttrs->GetItemState(nWhich);
    switch (eState)
    {
        case SfxItemState::UNKNOWN:
            m_xLbFrameDir->hide();
            break;
        case SfxItemState::DISABLED:
            m_xLbFrameDir->set_sensitive(false);
            break;
        case SfxItemState::INVALID:
            m_xLbFrameDir->set_active(-1);
            break;
        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxFrameDirectionItem& rFrameDirItem = static_cast<const SvxFrameDirectionItem&>(pCoreAttrs->Get(nWhich));
            m_xLbFrameDir->set_active_id(rFrameDirItem.GetValue());
            break;
        }
    }

    // Special treatment for distributed alignment; we need to set the justify
    // method to 'distribute' to distinguish from the normal justification.
    TypedWhichId<SfxEnumItemInterface> nHorJustifyMethodWhich(GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD));
    SfxItemState eHorJustifyMethodState = pCoreAttrs->GetItemState(nHorJustifyMethodWhich);
    if (eHorJustifyMethodState == SfxItemState::UNKNOWN)
    {
        // feature unknown, e.g. dbaccess, remove the option
        int nDistribId = m_xLbHorAlign->find_id(OUString::number(ALIGNDLG_HORALIGN_DISTRIBUTED));
        if (nDistribId != -1)
            m_xLbHorAlign->remove(nDistribId);
    }
    else
    {
        // feature known, e.g. calc
        lcl_MaybeResetAlignToDistro<SvxCellHorJustify, SvxCellHorJustify>(
            *m_xLbHorAlign, ALIGNDLG_HORALIGN_DISTRIBUTED, *pCoreAttrs,
            TypedWhichId<SfxEnumItemInterface>(GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY)), nHorJustifyMethodWhich,
            SvxCellHorJustify::Block);
    }

    TypedWhichId<SfxEnumItemInterface> nVerJustifyMethodWhich( GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY_METHOD) );
    SfxItemState eVerJustifyMethodState = pCoreAttrs->GetItemState(nVerJustifyMethodWhich);
    if (eVerJustifyMethodState == SfxItemState::UNKNOWN)
    {
        // feature unknown, e.g. dbaccess, remove the option
        int nDistribId = m_xLbVerAlign->find_id(OUString::number(ALIGNDLG_VERALIGN_DISTRIBUTED));
        if (nDistribId != -1)
            m_xLbVerAlign->remove(nDistribId);
    }
    else
    {
        // feature known, e.g. calc
        lcl_MaybeResetAlignToDistro<SvxCellVerJustify, SvxCellVerJustify>(
            *m_xLbVerAlign, ALIGNDLG_VERALIGN_DISTRIBUTED, *pCoreAttrs,
            TypedWhichId<SfxEnumItemInterface>(GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY)), nVerJustifyMethodWhich,
            SvxCellVerJustify::Block);
    }

    m_xLbHorAlign->save_value();
    m_xLbFrameDir->save_value();
    m_xLbVerAlign->save_value();
    m_xNfRotate->save_value();
    m_xEdIndent->save_value();

    UpdateEnableControls();
}

DeactivateRC AlignmentTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void AlignmentTabPage::InitVsRefEgde()
{
    // remember selection - is deleted in call to ValueSet::Clear()
    sal_uInt16 nSel = m_aVsRefEdge.GetSelectedItemId();

    Image aBottomLock(StockImage::Yes, RID_SVXBMP_BOTTOMLOCK);
    Image aTopLock(StockImage::Yes, RID_SVXBMP_TOPLOCK);
    Image aCellLock(StockImage::Yes, RID_SVXBMP_CELLLOCK);

    m_aVsRefEdge.Clear();
    m_aVsRefEdge.SetStyle(m_aVsRefEdge.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER);

    m_aVsRefEdge.SetColCount(3);
    m_aVsRefEdge.InsertItem(IID_BOTTOMLOCK, aBottomLock,  m_xFtBotLock->get_label());
    m_aVsRefEdge.InsertItem(IID_TOPLOCK,    aTopLock,     m_xFtTopLock->get_label());
    m_aVsRefEdge.InsertItem(IID_CELLLOCK,   aCellLock,    m_xFtCelLock->get_label());
    m_aVsRefEdge.SetOptimalSize();

    m_aVsRefEdge.SelectItem( nSel );
}

void AlignmentTabPage::UpdateEnableControls()
{
    const sal_Int32 nHorAlign = m_xLbHorAlign->get_active_id().toInt32();
    bool bHorLeft  = (nHorAlign == ALIGNDLG_HORALIGN_LEFT);
    bool bHorBlock = (nHorAlign == ALIGNDLG_HORALIGN_BLOCK);
    bool bHorFill  = (nHorAlign == ALIGNDLG_HORALIGN_FILL);
    bool bHorDist  = (nHorAlign == ALIGNDLG_HORALIGN_DISTRIBUTED);

    // indent edit field only for left alignment
    m_xFtIndent->set_sensitive( bHorLeft );
    m_xEdIndent->set_sensitive( bHorLeft );

    // stacked disabled for fill alignment
    m_xCbStacked->set_sensitive(!bHorFill);

    // hyphenation only for automatic line breaks or for block alignment
    m_xBtnHyphen->set_sensitive( m_xBtnWrap->get_active() || bHorBlock );

    // shrink only without automatic line break, and not for block, fill or distribute.
    m_xBtnShrink->set_sensitive( (m_xBtnWrap->get_state() == TRISTATE_FALSE) && !bHorBlock && !bHorFill && !bHorDist );

    // visibility of frames
    m_xAlignmentFrame->set_visible(m_xLbHorAlign->get_visible() || m_xEdIndent->get_visible() ||
        m_xLbVerAlign->get_visible());
    m_xOrientFrame->set_visible(m_xCtrlDialWin->get_visible() || m_xVsRefEdge->get_visible() ||
        m_xCbStacked->get_visible() || m_xCbAsianMode->get_visible());
    m_xPropertiesFrame->set_visible(m_xBtnWrap->get_visible() || m_xBtnHyphen->get_visible() ||
        m_xBtnShrink->get_visible() || m_xLbFrameDir->get_visible());

    bool bStackedText = m_xCbStacked->get_active();
    // windows to be disabled, if stacked text is turned ON
    m_xFtRotate->set_sensitive(!bStackedText);
    m_xFtRefEdge->set_sensitive(!bStackedText);
    m_xVsRefEdge->set_sensitive(!bStackedText);
    // windows to be disabled, if stacked text is turned OFF
    m_xCbAsianMode->set_sensitive(bStackedText);
    // rotation/stacked disabled for fill alignment/stacked
    m_xCtrlDialWin->set_sensitive(!bHorFill && !bStackedText);
    m_xNfRotate->set_sensitive(!bHorFill && !bStackedText);
}

bool AlignmentTabPage::HasAlignmentChanged( const SfxItemSet& rNew, TypedWhichId<SfxEnumItemInterface> nWhich ) const
{
    const SfxItemSet& rOld = GetItemSet();
    SvxCellJustifyMethod eMethodOld = SvxCellJustifyMethod::Auto;
    SvxCellJustifyMethod eMethodNew = SvxCellJustifyMethod::Auto;
    if (const SfxEnumItemInterface* p = rOld.GetItemIfSet(nWhich))
    {
        eMethodOld = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    }

    if (const SfxEnumItemInterface* p = rNew.GetItemIfSet(nWhich))
    {
        eMethodNew = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    }

    return eMethodOld != eMethodNew;
}

IMPL_LINK(AlignmentTabPage, StackedClickHdl, weld::Toggleable&, rToggle, void)
{
    m_aStackedState.ButtonToggled(rToggle);
    UpdateEnableControls();
}

IMPL_LINK(AlignmentTabPage, AsianModeClickHdl, weld::Toggleable&, rToggle, void)
{
    m_aAsianModeState.ButtonToggled(rToggle);
}

IMPL_LINK(AlignmentTabPage, WrapClickHdl, weld::Toggleable&, rToggle, void)
{
    m_aWrapState.ButtonToggled(rToggle);
    UpdateEnableControls();
}

IMPL_LINK(AlignmentTabPage, HyphenClickHdl, weld::Toggleable&, rToggle, void)
{
    m_aHyphenState.ButtonToggled(rToggle);
}

IMPL_LINK(AlignmentTabPage, ShrinkClickHdl, weld::Toggleable&, rToggle, void)
{
    m_aShrinkState.ButtonToggled(rToggle);
}

IMPL_LINK_NOARG(AlignmentTabPage, UpdateEnableHdl, weld::ComboBox&, void)
{
    UpdateEnableControls();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
