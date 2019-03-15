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
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <bitmaps.hlst>
#include <svx/rotmodit.hxx>

#include <svx/algitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/dlgutil.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/languageoptions.hxx>
#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <vcl/settings.hxx>

#define IID_BOTTOMLOCK 1
#define IID_TOPLOCK    2
#define IID_CELLLOCK   3

namespace svx {

const sal_uInt16 AlignmentTabPage::s_pRanges[] =
{
    SID_ATTR_ALIGN_HOR_JUSTIFY,SID_ATTR_ALIGN_VER_JUSTIFY,
    SID_ATTR_ALIGN_STACKED,SID_ATTR_ALIGN_LINEBREAK,
    SID_ATTR_ALIGN_INDENT,SID_ATTR_ALIGN_INDENT,
    SID_ATTR_ALIGN_DEGREES,SID_ATTR_ALIGN_DEGREES,
    SID_ATTR_ALIGN_LOCKPOS,SID_ATTR_ALIGN_LOCKPOS,
    SID_ATTR_ALIGN_HYPHENATION,SID_ATTR_ALIGN_HYPHENATION,
    SID_ATTR_ALIGN_ASIANVERTICAL,SID_ATTR_ALIGN_ASIANVERTICAL,
    SID_ATTR_FRAMEDIRECTION,SID_ATTR_FRAMEDIRECTION,
    SID_ATTR_ALIGN_SHRINKTOFIT,SID_ATTR_ALIGN_SHRINKTOFIT,
    0
};


namespace {

template<typename JustContainerType, typename JustEnumType>
void lcl_MaybeResetAlignToDistro(
    weld::ComboBox& rLB, sal_uInt16 nListPos, const SfxItemSet& rCoreAttrs, sal_uInt16 nWhichAlign, sal_uInt16 nWhichJM, JustEnumType eBlock)
{
    const SfxPoolItem* pItem;
    if (rCoreAttrs.GetItemState(nWhichAlign, true, &pItem) != SfxItemState::SET)
        // alignment not set.
        return;

    const SfxEnumItemInterface* p = static_cast<const SfxEnumItemInterface*>(pItem);
    JustContainerType eVal = static_cast<JustContainerType>(p->GetEnumValue());
    if (eVal != eBlock)
        // alignment is not 'justify'.  No need to go further.
        return;

    if (rCoreAttrs.GetItemState(nWhichJM, true, &pItem) != SfxItemState::SET)
        // justification method is not set.
        return;

    p = static_cast<const SfxEnumItemInterface*>(pItem);
    SvxCellJustifyMethod eMethod = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    if (eMethod == SvxCellJustifyMethod::Distribute)
    {
        // Select the 'distribute' entry in the specified list box.
        rLB.set_active(nListPos);
    }
}

void lcl_SetJustifyMethodToItemSet(SfxItemSet& rSet, sal_uInt16 nWhichJM, const weld::ComboBox& rLB, sal_uInt16 nListPos)
{
    SvxCellJustifyMethod eJM = SvxCellJustifyMethod::Auto;
    if (rLB.get_active() == nListPos)
        eJM = SvxCellJustifyMethod::Distribute;

    SvxJustifyMethodItem aItem(eJM, nWhichJM);
    rSet.Put(aItem);
}

}//namespace

AlignmentTabPage::AlignmentTabPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "cui/ui/cellalignment.ui", "CellAlignPage", &rCoreAttrs)
    , m_aVsRefEdge(nullptr)
    // text alignment
    , m_xLbHorAlign(m_xBuilder->weld_combo_box("comboboxHorzAlign"))
    , m_xFtIndent(m_xBuilder->weld_label("labelIndent"))
    , m_xEdIndent(m_xBuilder->weld_metric_spin_button("spinIndentFrom", FieldUnit::POINT))
    , m_xFtVerAlign(m_xBuilder->weld_label("labelVertAlign"))
    , m_xLbVerAlign(m_xBuilder->weld_combo_box("comboboxVertAlign"))
    //text rotation
    , m_xFtRotate(m_xBuilder->weld_label("labelDegrees"))
    , m_xNfRotate(m_xBuilder->weld_spin_button("spinDegrees"))
    , m_xFtRefEdge(m_xBuilder->weld_label("labelRefEdge"))
    //Asian mode
    , m_xCbStacked(m_xBuilder->weld_check_button("checkVertStack"))
    , m_xCbAsianMode(m_xBuilder->weld_check_button("checkAsianMode"))
    // Properties
    , m_xBoxDirection(m_xBuilder->weld_widget("boxDirection"))
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
    , m_xCtrlDial(new weld::CustomWeld(*m_xBuilder, "dialcontrol", m_aCtrlDial))
{
    m_aCtrlDial.SetLinkedField(m_xNfRotate.get());
    m_aCtrlDial.SetText(m_xFtABCD->get_label());

    InitVsRefEgde();

    m_xLbHorAlign->connect_changed(LINK(this, AlignmentTabPage, UpdateEnableHdl));
    m_xBtnWrap->connect_toggled(LINK(this, AlignmentTabPage, UpdateEnableClickHdl));
    m_xCbStacked->connect_toggled(LINK(this, AlignmentTabPage, UpdateEnableClickHdl));

    // Asian vertical mode
    m_xCbAsianMode->set_visible(SvtCJKOptions().IsVerticalTextEnabled());

    m_xLbFrameDir->append(SvxFrameDirection::Horizontal_LR_TB, SvxResId(RID_SVXSTR_FRAMEDIR_LTR));
    m_xLbFrameDir->append(SvxFrameDirection::Horizontal_RL_TB, SvxResId(RID_SVXSTR_FRAMEDIR_RTL));
    m_xLbFrameDir->append(SvxFrameDirection::Environment, SvxResId(RID_SVXSTR_FRAMEDIR_SUPER));

    // This page needs ExchangeSupport.
    SetExchangeSupport();
}

AlignmentTabPage::~AlignmentTabPage()
{
    disposeOnce();
}

void AlignmentTabPage::dispose()
{
    m_xCtrlDial.reset();
    m_xVsRefEdge.reset();
    m_xLbFrameDir.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> AlignmentTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<AlignmentTabPage>::Create(pParent, *rAttrSet);
}

bool AlignmentTabPage::FillItemSet( SfxItemSet* rSet )
{
    bool bChanged = SfxTabPage::FillItemSet(rSet);

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
                eJustify = SvxCellHorJustify::Block;
                break;
            case ALIGNDLG_HORALIGN_FILL:
                eJustify = SvxCellHorJustify::Repeat;
                break;
        }
        rSet->Put(SvxHorJustifyItem(eJustify, GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY)));
        bChanged = true;
    }

    if (m_xEdIndent->get_value_changed_from_saved())
    {
        rSet->Put(SfxUInt16Item(GetWhich(SID_ATTR_ALIGN_INDENT), m_xEdIndent->get_value(FieldUnit::TWIP)));
        bChanged = true;
    }

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
                eJustify = SvxCellVerJustify::Block;
                break;
        }
        rSet->Put(SvxVerJustifyItem(eJustify, GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY)));
        bChanged = true;
    }

    if (m_xNfRotate->get_value_changed_from_saved())
    {
        rSet->Put(SfxInt32Item(GetWhich(SID_ATTR_ALIGN_DEGREES), m_aCtrlDial.GetRotation()));
        bChanged = true;
    }

    if (m_aVsRefEdge.IsValueChangedFromSaved())
    {
        switch (m_aVsRefEdge.GetSelectedItemId())
        {
            case IID_CELLLOCK:
                rSet->Put(SvxRotateModeItem(SvxRotateMode::SVX_ROTATE_MODE_STANDARD, GetWhich(SID_ATTR_ALIGN_LOCKPOS)));
                break;
            case IID_TOPLOCK:
                rSet->Put(SvxRotateModeItem(SvxRotateMode::SVX_ROTATE_MODE_TOP, GetWhich(SID_ATTR_ALIGN_LOCKPOS)));
                break;
            case IID_BOTTOMLOCK:
                rSet->Put(SvxRotateModeItem(SvxRotateMode::SVX_ROTATE_MODE_BOTTOM, GetWhich(SID_ATTR_ALIGN_LOCKPOS)));
                break;
            default:
                m_aVsRefEdge.SetNoSelection();
                break;
        }
        bChanged = true;
    }

    if (m_xCbStacked->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(GetWhich(SID_ATTR_ALIGN_STACKED), m_xCbStacked->get_active()));
        bChanged = true;
    }

    if (m_xCbAsianMode->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(GetWhich(SID_ATTR_ALIGN_ASIANVERTICAL), m_xCbAsianMode->get_active()));
        bChanged = true;
    }

    if (m_xBtnWrap->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(GetWhich(SID_ATTR_ALIGN_LINEBREAK), m_xBtnWrap->get_active()));
        bChanged = true;
    }

    if (m_xBtnHyphen->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(GetWhich(SID_ATTR_ALIGN_HYPHENATION), m_xBtnHyphen->get_active()));
        bChanged = true;
    }

    if (m_xBtnShrink->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(GetWhich(SID_ATTR_ALIGN_SHRINKTOFIT), m_xBtnShrink->get_active()));
        bChanged = true;
    }

    if (m_xLbFrameDir->get_visible())
    {
        if (m_xLbFrameDir->get_value_changed_from_saved())
        {
            SvxFrameDirection eDir = m_xLbFrameDir->get_active_id();
            rSet->Put(SvxFrameDirectionItem(eDir, GetWhich(SID_ATTR_FRAMEDIRECTION)));
            bChanged = true;
        }
    }

    // Special treatment for distributed alignment; we need to set the justify
    // method to 'distribute' to distinguish from the normal justification.

    sal_uInt16 nWhichHorJM = GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD);
    lcl_SetJustifyMethodToItemSet(*rSet, nWhichHorJM, *m_xLbHorAlign, ALIGNDLG_HORALIGN_DISTRIBUTED);
    if (!bChanged)
        bChanged = HasAlignmentChanged(*rSet, nWhichHorJM);

    sal_uInt16 nWhichVerJM = GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY_METHOD);
    lcl_SetJustifyMethodToItemSet(*rSet, nWhichVerJM, *m_xLbVerAlign, ALIGNDLG_VERALIGN_DISTRIBUTED);
    if (!bChanged)
        bChanged = HasAlignmentChanged(*rSet, nWhichVerJM);

    return bChanged;
}

namespace
{
    void ResetBool(sal_uInt16 nWhich, const SfxItemSet* pSet, weld::CheckButton& rBtn)
    {
        SfxItemState eState = pSet->GetItemState(nWhich);
        switch (eState)
        {
            case SfxItemState::UNKNOWN:
                rBtn.hide();
                break;
            case SfxItemState::DISABLED:
            case SfxItemState::READONLY:
                rBtn.set_sensitive(false);
                break;
            case SfxItemState::DONTCARE:
                rBtn.set_state(TRISTATE_INDET);
                break;
            case SfxItemState::DEFAULT:
            case SfxItemState::SET:
            {
                const SfxBoolItem& rItem = static_cast<const SfxBoolItem&>(pSet->Get(nWhich));
                rBtn.set_state(static_cast<TriState>(rItem.GetValue()));
                break;
            }
        }
        rBtn.save_state();
    }
}

void AlignmentTabPage::Reset(const SfxItemSet* pCoreAttrs)
{
    SfxTabPage::Reset(pCoreAttrs);

    ResetBool(GetWhich(SID_ATTR_ALIGN_STACKED), pCoreAttrs, *m_xCbStacked);
    ResetBool(GetWhich(SID_ATTR_ALIGN_ASIANVERTICAL), pCoreAttrs, *m_xCbAsianMode);
    ResetBool(GetWhich(SID_ATTR_ALIGN_LINEBREAK), pCoreAttrs, *m_xBtnWrap);
    ResetBool(GetWhich(SID_ATTR_ALIGN_HYPHENATION), pCoreAttrs, *m_xBtnHyphen);
    ResetBool(GetWhich(SID_ATTR_ALIGN_SHRINKTOFIT), pCoreAttrs, *m_xBtnShrink);

    sal_uInt16 nWhich = GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY);
    SfxItemState eState = pCoreAttrs->GetItemState(nWhich);
    switch (eState)
    {
        case SfxItemState::UNKNOWN:
            m_xLbHorAlign->hide();
            break;
        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xLbHorAlign->set_sensitive(false);
            break;
        case SfxItemState::DONTCARE:
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
        case SfxItemState::READONLY:
            m_xEdIndent->set_sensitive(false);
            break;
        case SfxItemState::DONTCARE:
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
        case SfxItemState::READONLY:
            m_xLbVerAlign->set_sensitive(false);
            break;
        case SfxItemState::DONTCARE:
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
            m_xCtrlDial->hide();
            break;
        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xNfRotate->set_sensitive(false);
            m_xCtrlDial->set_sensitive(false);
            break;
        case SfxItemState::DONTCARE:
            m_aCtrlDial.SetNoRotation();
            break;
        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SfxInt32Item& rAlignItem = static_cast<const SfxInt32Item&>(pCoreAttrs->Get(nWhich));
            m_aCtrlDial.SetRotation(rAlignItem.GetValue());
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
        case SfxItemState::READONLY:
            m_xVsRefEdge->set_sensitive(false);
            break;
        case SfxItemState::DONTCARE:
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
        case SfxItemState::READONLY:
            m_xLbFrameDir->set_sensitive(false);
            break;
        case SfxItemState::DONTCARE:
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

    lcl_MaybeResetAlignToDistro<SvxCellHorJustify, SvxCellHorJustify>(
        *m_xLbHorAlign, ALIGNDLG_HORALIGN_DISTRIBUTED, *pCoreAttrs,
        GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY), GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD),
        SvxCellHorJustify::Block);

    lcl_MaybeResetAlignToDistro<SvxCellVerJustify, SvxCellVerJustify>(
        *m_xLbVerAlign, ALIGNDLG_VERALIGN_DISTRIBUTED, *pCoreAttrs,
        GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY), GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY_METHOD),
        SvxCellVerJustify::Block);

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

void AlignmentTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );
    if( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        InitVsRefEgde();
    }
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
    const sal_Int32 nHorAlign = m_xLbHorAlign->get_active();
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
    m_xOrientFrame->set_visible(m_xCtrlDial->get_visible() || m_xVsRefEdge->get_visible() ||
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
    m_xCtrlDial->set_sensitive(!bHorFill && !bStackedText);
    m_xNfRotate->set_sensitive(!bHorFill && !bStackedText);
}

bool AlignmentTabPage::HasAlignmentChanged( const SfxItemSet& rNew, sal_uInt16 nWhich ) const
{
    const SfxItemSet& rOld = GetItemSet();
    const SfxPoolItem* pItem;
    SvxCellJustifyMethod eMethodOld = SvxCellJustifyMethod::Auto;
    SvxCellJustifyMethod eMethodNew = SvxCellJustifyMethod::Auto;
    if (rOld.GetItemState(nWhich, true, &pItem) == SfxItemState::SET)
    {
        const SfxEnumItemInterface* p = static_cast<const SfxEnumItemInterface*>(pItem);
        eMethodOld = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    }

    if (rNew.GetItemState(nWhich, true, &pItem) == SfxItemState::SET)
    {
        const SfxEnumItemInterface* p = static_cast<const SfxEnumItemInterface*>(pItem);
        eMethodNew = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    }

    return eMethodOld != eMethodNew;
}

IMPL_LINK_NOARG(AlignmentTabPage, UpdateEnableClickHdl, weld::ToggleButton&, void)
{
    UpdateEnableControls();
}

IMPL_LINK_NOARG(AlignmentTabPage, UpdateEnableHdl, weld::ComboBox&, void)
{
    UpdateEnableControls();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
