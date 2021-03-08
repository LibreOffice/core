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

#include <tools/mapunit.hxx>
#include <tools/urlobj.hxx>
#include <editeng/numitem.hxx>
#include <svl/eitem.hxx>
#include <svl/itempool.hxx>
#include <svx/colorbox.hxx>
#include <svx/strarray.hxx>
#include <svx/gallery.hxx>
#include <editeng/brushitem.hxx>
#include <svl/intitem.hxx>
#include <vcl/graph.hxx>
#include <svtools/unitconv.hxx>
#include <svx/svxids.hrc>

#include <algorithm>
#include <memory>
#include <vector>
#include <sfx2/opengrf.hxx>

#include <strings.hrc>
#include <svl/stritem.hxx>
#include <sal/log.hxx>
#include <vcl/virdev.hxx>
#include <svx/SvxNumOptionsTabPageHelper.hxx>
#include <View.hxx>
#include <drawdoc.hxx>
#include <cui/cuicharmap.hxx>
#include <BulletAndPositionDlg.hxx>
#include <sdresid.hxx>
#include <DrawViewShell.hxx>

#define SHOW_NUMBERING 0
#define SHOW_BULLET 1
#define SHOW_BITMAP 2

#define MAX_BMP_WIDTH 16
#define MAX_BMP_HEIGHT 16

static bool bLastRelative = false;

static const vcl::Font& lcl_GetDefaultBulletFont()
{
    static vcl::Font aDefBulletFont = [&]() {
        vcl::Font tmp("OpenSymbol", "", Size(0, 14));
        tmp.SetCharSet(RTL_TEXTENCODING_SYMBOL);
        tmp.SetFamily(FAMILY_DONTKNOW);
        tmp.SetPitch(PITCH_DONTKNOW);
        tmp.SetWeight(WEIGHT_DONTKNOW);
        tmp.SetTransparent(true);
        return tmp;
    }();
    return aDefBulletFont;
}

class SdDrawDocument;

SvxBulletAndPositionDlg::SvxBulletAndPositionDlg(weld::Window* pWindow, const SfxItemSet& rSet,
                                                 const ::sd::View* pView)
    : GenericDialogController(pWindow, "cui/ui/bulletandposition.ui", "BulletAndPosition")
    , rFirstStateSet(rSet)
    , bLastWidthModified(false)
    , bModified(false)
    , bInInitControl(false)
    , bLabelAlignmentPosAndSpaceModeActive(false)
    , bApplyToMaster(false)
    , nBullet(0xff)
    , nActNumLvl(1)
    , p_Window(pWindow)
    , nNumItemId(SID_ATTR_NUMBERING_RULE)
    , m_xGrid(m_xBuilder->weld_widget("grid2"))
    , m_xLevelLB(m_xBuilder->weld_tree_view("levellb"))
    , m_xFmtLB(m_xBuilder->weld_combo_box("numfmtlb"))
    , m_xPrefixFT(m_xBuilder->weld_label("prefixft"))
    , m_xPrefixED(m_xBuilder->weld_entry("prefix"))
    , m_xSuffixFT(m_xBuilder->weld_label("suffixft"))
    , m_xSuffixED(m_xBuilder->weld_entry("suffix"))
    , m_xBeforeAfter(m_xBuilder->weld_frame("beforeafter"))
    , m_xBulColorFT(m_xBuilder->weld_label("colorft"))
    , m_xBulColLB(new ColorListBox(m_xBuilder->weld_menu_button("color"),
                                   [this] { return m_xDialog.get(); }))
    , m_xBulRelSizeFT(m_xBuilder->weld_label("relsizeft"))
    , m_xBulRelSizeMF(m_xBuilder->weld_metric_spin_button("relsize", FieldUnit::PERCENT))
    , m_xStartFT(m_xBuilder->weld_label("startatft"))
    , m_xStartED(m_xBuilder->weld_spin_button("startat"))
    , m_xBulletFT(m_xBuilder->weld_label("bulletft"))
    , m_xBulletPB(m_xBuilder->weld_button("bullet"))
    , m_xBitmapMB(m_xBuilder->weld_menu_button("bitmap"))
    , m_xWidthFT(m_xBuilder->weld_label("widthft"))
    , m_xWidthMF(m_xBuilder->weld_metric_spin_button("widthmf", FieldUnit::CM))
    , m_xHeightFT(m_xBuilder->weld_label("heightft"))
    , m_xHeightMF(m_xBuilder->weld_metric_spin_button("heightmf", FieldUnit::CM))
    , m_xRatioCB(m_xBuilder->weld_check_button("keepratio"))
    , m_xPreviewWIN(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWIN))
    , m_xDistBorderFT(m_xBuilder->weld_label("indent"))
    , m_xDistBorderMF(m_xBuilder->weld_metric_spin_button("indentmf", FieldUnit::CM))
    , m_xRelativeCB(m_xBuilder->weld_check_button("relative"))
    , m_xIndentFT(m_xBuilder->weld_label("numberingwidth"))
    , m_xIndentMF(m_xBuilder->weld_metric_spin_button("numberingwidthmf", FieldUnit::CM))
    , m_xLeftTB(m_xBuilder->weld_toggle_button("left"))
    , m_xCenterTB(m_xBuilder->weld_toggle_button("center"))
    , m_xRightTB(m_xBuilder->weld_toggle_button("right"))
    , m_xSlideRB(m_xBuilder->weld_radio_button("sliderb"))
    , m_xSelectionRB(m_xBuilder->weld_radio_button("selectionrb"))
    , m_xApplyToMaster(m_xBuilder->weld_toggle_button("applytomaster"))
    , m_xReset(m_xBuilder->weld_button("reset"))
{
    m_xBulColLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    m_xBulRelSizeMF->set_min(SVX_NUM_REL_SIZE_MIN, FieldUnit::PERCENT);
    m_xBulRelSizeMF->set_increments(5, 50, FieldUnit::PERCENT);
    aActBulletFont = lcl_GetDefaultBulletFont();

    m_xBulletPB->connect_clicked(LINK(this, SvxBulletAndPositionDlg, BulletHdl_Impl));
    m_xFmtLB->connect_changed(LINK(this, SvxBulletAndPositionDlg, NumberTypeSelectHdl_Impl));
    m_xBitmapMB->connect_selected(LINK(this, SvxBulletAndPositionDlg, GraphicHdl_Impl));
    m_xBitmapMB->connect_toggled(LINK(this, SvxBulletAndPositionDlg, PopupActivateHdl_Impl));
    m_xLevelLB->set_selection_mode(SelectionMode::Multiple);
    m_xLevelLB->connect_changed(LINK(this, SvxBulletAndPositionDlg, LevelHdl_Impl));
    m_xWidthMF->connect_value_changed(LINK(this, SvxBulletAndPositionDlg, SizeHdl_Impl));
    m_xHeightMF->connect_value_changed(LINK(this, SvxBulletAndPositionDlg, SizeHdl_Impl));
    m_xRatioCB->connect_toggled(LINK(this, SvxBulletAndPositionDlg, RatioHdl_Impl));
    m_xStartED->connect_changed(LINK(this, SvxBulletAndPositionDlg, EditModifyHdl_Impl));
    m_xPrefixED->connect_changed(LINK(this, SvxBulletAndPositionDlg, EditModifyHdl_Impl));
    m_xSuffixED->connect_changed(LINK(this, SvxBulletAndPositionDlg, EditModifyHdl_Impl));
    m_xBulRelSizeMF->connect_value_changed(LINK(this, SvxBulletAndPositionDlg, BulRelSizeHdl_Impl));
    m_xBulColLB->SetSelectHdl(LINK(this, SvxBulletAndPositionDlg, BulColorHdl_Impl));
    m_xLeftTB->connect_toggled(LINK(this, SvxBulletAndPositionDlg, SelectLeftAlignmentHdl_Impl));
    m_xCenterTB->connect_toggled(
        LINK(this, SvxBulletAndPositionDlg, SelectCenterAlignmentHdl_Impl));
    m_xRightTB->connect_toggled(LINK(this, SvxBulletAndPositionDlg, SelectRightAlignmentHdl_Impl));
    m_xApplyToMaster->connect_toggled(LINK(this, SvxBulletAndPositionDlg, ApplyToMasterHdl_Impl));
    m_xReset->connect_clicked(LINK(this, SvxBulletAndPositionDlg, ResetHdl_Impl));

    aInvalidateTimer.SetInvokeHandler(
        LINK(this, SvxBulletAndPositionDlg, PreviewInvalidateHdl_Impl));
    aInvalidateTimer.SetTimeout(50);

    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));

    // Fill ListBox with predefined / translated numbering types.
    sal_uInt32 nCount = SvxNumberingTypeTable::Count();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        m_xFmtLB->append(OUString::number(SvxNumberingTypeTable::GetValue(i)),
                         SvxNumberingTypeTable::GetString(i));
    }

    // Get advanced numbering types from the component.
    // Watch out for the ugly
    // 136 == 0x88 == SVX_NUM_BITMAP|0x80 == SVX_NUM_BITMAP|LINK_TOKEN
    // to not remove that.
    SvxNumOptionsTabPageHelper::GetI18nNumbering(*m_xFmtLB, (SVX_NUM_BITMAP | LINK_TOKEN));

    m_xFmtLB->set_active(0);
    m_xRelativeCB->set_active(true);

    Link<weld::MetricSpinButton&, void> aLk3
        = LINK(this, SvxBulletAndPositionDlg, DistanceHdl_Impl);
    m_xDistBorderMF->connect_value_changed(aLk3);
    m_xIndentMF->connect_value_changed(aLk3);

    m_xRelativeCB->connect_toggled(LINK(this, SvxBulletAndPositionDlg, RelativeHdl_Impl));
    m_xRelativeCB->set_active(bLastRelative);

    Size aSize(m_xGrid->get_preferred_size());
    m_xGrid->set_size_request(aSize.Width(), -1);

    // PageCreated
    FieldUnit eMetric = pView->GetDoc().GetUIUnit();
    SfxAllItemSet aSet(*(rSet.GetPool()));
    aSet.Put(SfxUInt16Item(SID_METRIC_ITEM, static_cast<sal_uInt16>(eMetric)));

    const SfxStringItem* pNumCharFmt = aSet.GetItem<SfxStringItem>(SID_NUM_CHAR_FMT, false);
    const SfxStringItem* pBulletCharFmt = aSet.GetItem<SfxStringItem>(SID_BULLET_CHAR_FMT, false);
    const SfxUInt16Item* pMetricItem = aSet.GetItem<SfxUInt16Item>(SID_METRIC_ITEM, false);

    if (pNumCharFmt && pBulletCharFmt)
        SetCharFmts(pNumCharFmt->GetValue(), pBulletCharFmt->GetValue());

    if (pMetricItem)
        SetMetric(static_cast<FieldUnit>(pMetricItem->GetValue()));

    // tdf#130526: Hide "Apply To Master"-button in Draw and rename "Slide" to "Page"
    DocumentType aDocumentType = pView->GetDoc().GetDocumentType();
    if (aDocumentType == DocumentType::Draw)
    {
        m_xApplyToMaster->hide();
        m_xSlideRB->set_label(SdResId(STR_PAGE_NAME));
    }
    // tdf#137406: Crash when clicking "Apply to Master" in Slide Master mode on Bullets and Numbering dialog
    EditMode aEditmode = static_cast<::sd::DrawViewShell*>(pView->GetViewShell())->GetEditMode();
    if (aDocumentType == DocumentType::Impress && aEditmode == EditMode::MasterPage)
        m_xApplyToMaster->hide();

    // End PageCreated

    Reset(&rSet);

    // ActivatePage part

    const SfxPoolItem* pItem;
    const SfxItemSet* pExampleSet = &rSet;
    sal_uInt16 nTmpNumLvl = 1;
    bool bPreset = false;
    if (pExampleSet)
    {
        if (SfxItemState::SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, false, &pItem))
            bPreset = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        if (SfxItemState::SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, false, &pItem))
            nTmpNumLvl = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    }
    if (SfxItemState::SET == rSet.GetItemState(nNumItemId, false, &pItem))
    {
        pSaveNum.reset(new SvxNumRule(*static_cast<const SvxNumBulletItem*>(pItem)->GetNumRule()));
    }

    bModified = (!pActNum->Get(0) || bPreset);
    if (*pActNum != *pSaveNum || nActNumLvl != nTmpNumLvl)
    {
        nActNumLvl = nTmpNumLvl;
        sal_uInt16 nMask = 1;
        if (nActNumLvl == SAL_MAX_UINT16)
            m_xLevelLB->select(pActNum->GetLevelCount());
        if (nActNumLvl != SAL_MAX_UINT16)
        {
            for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
            {
                if (nActNumLvl & nMask)
                    m_xLevelLB->select(i);
                nMask <<= 1;
            }
        }
        *pActNum = *pSaveNum;

        m_xRelativeCB->set_sensitive(nActNumLvl != 1);

        InitPosAndSpaceMode();
        InitControls();
    }

    m_aPreviewWIN.SetLevel(nActNumLvl);
    m_aPreviewWIN.Invalidate();

    // End of the ActivatePage part
}

SvxBulletAndPositionDlg::~SvxBulletAndPositionDlg() {}

void SvxBulletAndPositionDlg::SetMetric(FieldUnit eMetric)
{
    if (eMetric == FieldUnit::MM)
    {
        m_xWidthMF->set_digits(1);
        m_xHeightMF->set_digits(1);
        m_xDistBorderMF->set_digits(1);
        m_xIndentMF->set_digits(1);
    }
    m_xWidthMF->set_unit(eMetric);
    m_xHeightMF->set_unit(eMetric);
    m_xDistBorderMF->set_unit(eMetric);
    m_xIndentMF->set_unit(eMetric);
}

SfxItemSet* SvxBulletAndPositionDlg::GetOutputItemSet(SfxItemSet* pSet)
{
    pSet->Put(SfxUInt16Item(SID_PARAM_CUR_NUM_LEVEL, nActNumLvl));
    if (bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        pSet->Put(SvxNumBulletItem(*pSaveNum, nNumItemId));
        pSet->Put(SfxBoolItem(SID_PARAM_NUM_PRESET, false));
    }
    return pSet;
};

bool SvxBulletAndPositionDlg::IsApplyToMaster() const { return bApplyToMaster; }
bool SvxBulletAndPositionDlg::IsSlideScope() const { return m_xSlideRB->get_active(); }

void SvxBulletAndPositionDlg::Reset(const SfxItemSet* rSet)
{
    const SfxPoolItem* pItem;
    // in Draw the item exists as WhichId, in Writer only as SlotId
    SfxItemState eState = rSet->GetItemState(SID_ATTR_NUMBERING_RULE, false, &pItem);
    if (eState != SfxItemState::SET)
    {
        nNumItemId = rSet->GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet->GetItemState(nNumItemId, false, &pItem);

        if (eState != SfxItemState::SET)
        {
            pItem = &static_cast<const SvxNumBulletItem&>(rSet->Get(nNumItemId));
            eState = SfxItemState::SET;
        }
    }
    DBG_ASSERT(eState == SfxItemState::SET, "no item found!");
    pSaveNum.reset(new SvxNumRule(*static_cast<const SvxNumBulletItem*>(pItem)->GetNumRule()));

    // insert levels
    if (!m_xLevelLB->n_children())
    {
        for (sal_uInt16 i = 1; i <= pSaveNum->GetLevelCount(); i++)
            m_xLevelLB->append_text(OUString::number(i));
        if (pSaveNum->GetLevelCount() > 1)
        {
            OUString sEntry = "1 - " + OUString::number(pSaveNum->GetLevelCount());
            m_xLevelLB->append_text(sEntry);
            m_xLevelLB->select_text(sEntry);
        }
        else
            m_xLevelLB->select(0);
    }
    else
        m_xLevelLB->select(m_xLevelLB->n_children() - 1);

    sal_uInt16 nMask = 1;
    m_xLevelLB->unselect_all();
    if (nActNumLvl == SAL_MAX_UINT16)
    {
        m_xLevelLB->select(pSaveNum->GetLevelCount());
    }
    else
    {
        for (sal_uInt16 i = 0; i < pSaveNum->GetLevelCount(); i++)
        {
            if (nActNumLvl & nMask)
                m_xLevelLB->select(i);
            nMask <<= 1;
        }
    }

    if (!pActNum)
        pActNum.reset(new SvxNumRule(*pSaveNum));
    else if (*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    m_aPreviewWIN.SetNumRule(pActNum.get());

    bool bContinuous = pActNum->IsFeatureSupported(SvxNumRuleFlags::CONTINUOUS);

    // again misusage: in Draw there is numeration only until the bitmap
    // without SVX_NUM_NUMBER_NONE
    //remove types that are unsupported by Draw/Impress
    if (!bContinuous)
    {
        sal_Int32 nFmtCount = m_xFmtLB->get_count();
        for (sal_Int32 i = nFmtCount; i; i--)
        {
            sal_uInt16 nEntryData = m_xFmtLB->get_id(i - 1).toUInt32();
            if (/*SVX_NUM_NUMBER_NONE == nEntryData ||*/
                (SVX_NUM_BITMAP | LINK_TOKEN) == nEntryData)
                m_xFmtLB->remove(i - 1);
        }
    }
    //one must be enabled
    if (!pActNum->IsFeatureSupported(SvxNumRuleFlags::ENABLE_LINKED_BMP))
    {
        auto nPos = m_xFmtLB->find_id(OUString::number(SVX_NUM_BITMAP | LINK_TOKEN));
        if (nPos != -1)
            m_xFmtLB->remove(nPos);
    }
    else if (!pActNum->IsFeatureSupported(SvxNumRuleFlags::ENABLE_EMBEDDED_BMP))
    {
        auto nPos = m_xFmtLB->find_id(OUString::number(SVX_NUM_BITMAP));
        if (nPos != -1)
            m_xFmtLB->remove(nPos);
    }

    // MegaHack: because of a not-fixable 'design mistake/error' in Impress
    // delete all kinds of numeric enumerations
    if (pActNum->IsFeatureSupported(SvxNumRuleFlags::NO_NUMBERS))
    {
        sal_Int32 nFmtCount = m_xFmtLB->get_count();
        for (sal_Int32 i = nFmtCount; i; i--)
        {
            sal_uInt16 nEntryData = m_xFmtLB->get_id(i - 1).toUInt32();
            if (/*nEntryData >= SVX_NUM_CHARS_UPPER_LETTER &&*/ nEntryData <= SVX_NUM_NUMBER_NONE)
                m_xFmtLB->remove(i - 1);
        }
    }

    InitPosAndSpaceMode();

    InitControls();
    bModified = false;
}

void SvxBulletAndPositionDlg::InitControls()
{
    bInInitControl = true;

    const bool bRelative = !bLabelAlignmentPosAndSpaceModeActive && m_xRelativeCB->get_sensitive()
                           && m_xRelativeCB->get_active();
    const bool bSingleSelection
        = m_xLevelLB->count_selected_rows() == 1 && SAL_MAX_UINT16 != nActNumLvl;

    m_xDistBorderMF->set_sensitive(!bLabelAlignmentPosAndSpaceModeActive
                                   && (bSingleSelection || bRelative));
    m_xDistBorderFT->set_sensitive(!bLabelAlignmentPosAndSpaceModeActive
                                   && (bSingleSelection || bRelative));

    bool bShowBullet = true;
    bool bShowBitmap = true;
    bool bSameType = true;
    bool bSameStart = true;
    bool bSamePrefix = true;
    bool bSameSuffix = true;
    bool bSameSize = true;
    bool bSameBulColor = true;
    bool bSameBulRelSize = true;
    bool bSameDistBorderNum = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSetDistEmpty = false;
    bool bSameIndent = !bLabelAlignmentPosAndSpaceModeActive;

    const SvxNumberFormat* aNumFmtArr[SVX_MAX_NUM];
    OUString sFirstCharFmt;
    SvxAdjust eFirstAdjust = SvxAdjust::Left;
    Size aFirstSize(0, 0);
    sal_uInt16 nMask = 1;
    sal_uInt16 nLvl = SAL_MAX_UINT16;

    bool bBullColor = pActNum->IsFeatureSupported(SvxNumRuleFlags::BULLET_COLOR);
    bool bBullRelSize = pActNum->IsFeatureSupported(SvxNumRuleFlags::BULLET_REL_SIZE);
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        aNumFmtArr[i] = &pActNum->GetLevel(i);

        if (nActNumLvl & nMask)
        {
            bShowBullet &= aNumFmtArr[i]->GetNumberingType() == SVX_NUM_CHAR_SPECIAL;
            bShowBitmap &= (aNumFmtArr[i]->GetNumberingType() & (~LINK_TOKEN)) == SVX_NUM_BITMAP;
            eFirstAdjust = aNumFmtArr[i]->GetNumAdjust();
            if (SAL_MAX_UINT16 == nLvl)
            {
                nLvl = i;
                sFirstCharFmt = aNumFmtArr[i]->GetCharFormatName();
                if (bShowBitmap)
                    aFirstSize = aNumFmtArr[i]->GetGraphicSize();
            }
            if (i > nLvl)
            {
                bSameType
                    &= aNumFmtArr[i]->GetNumberingType() == aNumFmtArr[nLvl]->GetNumberingType();
                bSameStart = aNumFmtArr[i]->GetStart() == aNumFmtArr[nLvl]->GetStart();

                bSamePrefix = aNumFmtArr[i]->GetPrefix() == aNumFmtArr[nLvl]->GetPrefix();
                bSameSuffix = aNumFmtArr[i]->GetSuffix() == aNumFmtArr[nLvl]->GetSuffix();
                //bSameAdjust &= eFirstAdjust == aNumFmtArr[i]->GetNumAdjust();
                if (bShowBitmap && bSameSize)
                    bSameSize &= aNumFmtArr[i]->GetGraphicSize() == aFirstSize;
                bSameBulColor
                    &= aNumFmtArr[i]->GetBulletColor() == aNumFmtArr[nLvl]->GetBulletColor();
                bSameBulRelSize
                    &= aNumFmtArr[i]->GetBulletRelSize() == aNumFmtArr[nLvl]->GetBulletRelSize();
                bSameIndent //?
                    &= aNumFmtArr[i]->GetFirstLineOffset()
                       == aNumFmtArr[nLvl]->GetFirstLineOffset();
            }
        }

        nMask <<= 1;
    }
    SwitchNumberType(bShowBullet ? 1 : bShowBitmap ? 2 : 0);

    sal_uInt16 nNumberingType;
    if (nLvl != SAL_MAX_UINT16)
        nNumberingType = aNumFmtArr[nLvl]->GetNumberingType();
    else
    {
        nNumberingType = SVX_NUM_NUMBER_NONE;
        bSameDistBorderNum = false;
        bSameIndent = false;
        bSameBulRelSize = false;
        bSameBulColor = false;
        bSameStart = false;
        bSamePrefix = false;
        bSameSuffix = false;
    }

    CheckForStartValue_Impl(nNumberingType);

    if (bShowBitmap)
    {
        if (bSameSize)
        {
            SetMetricValue(*m_xHeightMF, aFirstSize.Height(), eCoreUnit);
            SetMetricValue(*m_xWidthMF, aFirstSize.Width(), eCoreUnit);
        }
        else
        {
            m_xHeightMF->set_text("");
            m_xWidthMF->set_text("");
        }
    }

    if (bSameType)
    {
        sal_uInt16 nLBData = nNumberingType;
        m_xFmtLB->set_active_id(OUString::number(nLBData));
    }
    else
        m_xFmtLB->set_active(-1);

    if (bBullRelSize)
    {
        if (bSameBulRelSize)
            m_xBulRelSizeMF->set_value(aNumFmtArr[nLvl]->GetBulletRelSize(), FieldUnit::PERCENT);
        else
            m_xBulRelSizeMF->set_text("");
    }
    if (bBullColor)
    {
        if (bSameBulColor)
            m_xBulColLB->SelectEntry(aNumFmtArr[nLvl]->GetBulletColor());
        else
            m_xBulColLB->SetNoSelection();
    }
    switch (nBullet)
    {
        case SHOW_NUMBERING:
            if (bSameStart)
            {
                m_xStartED->set_value(aNumFmtArr[nLvl]->GetStart());
            }
            else
                m_xStartED->set_text("");
            break;
        case SHOW_BULLET:
            break;
        case SHOW_BITMAP:
            break;
    }

    switch (eFirstAdjust)
    {
        case SvxAdjust::Left:
            m_xLeftTB->set_active(true);
            m_xCenterTB->set_active(false);
            m_xRightTB->set_active(false);
            break;
        case SvxAdjust::Center:
            m_xLeftTB->set_active(false);
            m_xCenterTB->set_active(true);
            m_xRightTB->set_active(false);
            break;
        case SvxAdjust::Right:
            m_xLeftTB->set_active(false);
            m_xCenterTB->set_active(false);
            m_xRightTB->set_active(true);
            break;
        default:
            break;
    }

    if (bSamePrefix)
        m_xPrefixED->set_text(aNumFmtArr[nLvl]->GetPrefix());
    else
        m_xPrefixED->set_text("");
    if (bSameSuffix)
        m_xSuffixED->set_text(aNumFmtArr[nLvl]->GetSuffix());
    else
        m_xSuffixED->set_text("");

    if (bSameDistBorderNum)
    {
        tools::Long nDistBorderNum;
        if (bRelative)
        {
            nDistBorderNum = static_cast<tools::Long>(aNumFmtArr[nLvl]->GetAbsLSpace())
                             + aNumFmtArr[nLvl]->GetFirstLineOffset();
            if (nLvl)
                nDistBorderNum -= static_cast<tools::Long>(aNumFmtArr[nLvl - 1]->GetAbsLSpace())
                                  + aNumFmtArr[nLvl - 1]->GetFirstLineOffset();
        }
        else
        {
            nDistBorderNum = static_cast<tools::Long>(aNumFmtArr[nLvl]->GetAbsLSpace())
                             + aNumFmtArr[nLvl]->GetFirstLineOffset();
        }
        SetMetricValue(*m_xDistBorderMF, nDistBorderNum, eCoreUnit);
    }
    else
        bSetDistEmpty = true;

    if (bSetDistEmpty)
        m_xDistBorderMF->set_text("");

    if (bSameIndent)
        SetMetricValue(*m_xIndentMF, -aNumFmtArr[nLvl]->GetFirstLineOffset(), eCoreUnit);
    else
        m_xIndentMF->set_text("");

    m_xSelectionRB->set_active(true);

    m_aPreviewWIN.SetLevel(nActNumLvl);
    m_aPreviewWIN.Invalidate();
    bInInitControl = false;
}

// 0 - Number; 1 - Bullet; 2 - Bitmap
void SvxBulletAndPositionDlg::SwitchNumberType(sal_uInt8 nType)
{
    if (nBullet == nType)
        return;
    nBullet = nType;
    bool bBullet = (nType == SHOW_BULLET);
    bool bBitmap = (nType == SHOW_BITMAP);
    bool bEnableBitmap = (nType == SHOW_BITMAP);
    bool bNumeric = !(bBitmap || bBullet);
    m_xPrefixFT->set_visible(bNumeric);
    m_xPrefixED->set_visible(bNumeric);
    m_xSuffixFT->set_visible(bNumeric);
    m_xSuffixED->set_visible(bNumeric);
    m_xBeforeAfter->set_visible(bNumeric);

    m_xStartFT->set_visible(!(bBullet || bBitmap));
    m_xStartED->set_visible(!(bBullet || bBitmap));

    m_xBulletFT->set_visible(bBullet);
    m_xBulletPB->set_visible(bBullet);
    bool bBullColor = pActNum->IsFeatureSupported(SvxNumRuleFlags::BULLET_COLOR);
    m_xBulColorFT->set_visible(!bBitmap && bBullColor);
    m_xBulColLB->set_visible(!bBitmap && bBullColor);
    bool bBullResSize = pActNum->IsFeatureSupported(SvxNumRuleFlags::BULLET_REL_SIZE);
    m_xBulRelSizeFT->set_visible(!bBitmap && bBullResSize);
    m_xBulRelSizeMF->set_visible(!bBitmap && bBullResSize);

    m_xBitmapMB->set_visible(bBitmap);

    m_xWidthFT->set_visible(bBitmap);
    m_xWidthMF->set_visible(bBitmap);
    m_xHeightFT->set_visible(bBitmap);
    m_xHeightMF->set_visible(bBitmap);
    m_xRatioCB->set_visible(bBitmap);

    m_xWidthFT->set_sensitive(bEnableBitmap);
    m_xWidthMF->set_sensitive(bEnableBitmap);
    m_xHeightFT->set_sensitive(bEnableBitmap);
    m_xHeightMF->set_sensitive(bEnableBitmap);
    m_xRatioCB->set_sensitive(bEnableBitmap);
}

void SvxBulletAndPositionDlg::CheckForStartValue_Impl(sal_uInt16 nNumberingType)
{
    bool bIsNull = m_xStartED->get_value() == 0;
    bool bNoZeroAllowed = nNumberingType < SVX_NUM_ARABIC
                          || SVX_NUM_CHARS_UPPER_LETTER_N == nNumberingType
                          || SVX_NUM_CHARS_LOWER_LETTER_N == nNumberingType;
    m_xStartED->set_min(bNoZeroAllowed ? 1 : 0);
    if (bIsNull && bNoZeroAllowed)
        EditModifyHdl_Impl(*m_xStartED);
}

IMPL_LINK(SvxBulletAndPositionDlg, LevelHdl_Impl, weld::TreeView&, rBox, void)
{
    sal_uInt16 nSaveNumLvl = nActNumLvl;
    nActNumLvl = 0;
    auto aSelectedRows = rBox.get_selected_rows();
    if (std::find(aSelectedRows.begin(), aSelectedRows.end(), pActNum->GetLevelCount())
            != aSelectedRows.end()
        && (aSelectedRows.size() == 1 || nSaveNumLvl != 0xffff))
    {
        nActNumLvl = 0xFFFF;
        for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
            rBox.unselect(i);
    }
    else if (!aSelectedRows.empty())
    {
        sal_uInt16 nMask = 1;
        for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if (std::find(aSelectedRows.begin(), aSelectedRows.end(), i) != aSelectedRows.end())
                nActNumLvl |= nMask;
            nMask <<= 1;
        }
        rBox.unselect(pActNum->GetLevelCount());
    }
    else
        nActNumLvl = nSaveNumLvl;

    InitControls();
}

IMPL_LINK_NOARG(SvxBulletAndPositionDlg, PreviewInvalidateHdl_Impl, Timer*, void)
{
    m_aPreviewWIN.Invalidate();
}

IMPL_LINK(SvxBulletAndPositionDlg, NumberTypeSelectHdl_Impl, weld::ComboBox&, rBox, void)
{
    OUString sSelectStyle;
    bool bBmp = false;
    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            // PAGEDESC does not exist
            SvxNumType nNumType = static_cast<SvxNumType>(rBox.get_active_id().toUInt32());
            aNumFmt.SetNumberingType(nNumType);
            sal_uInt16 nNumberingType = aNumFmt.GetNumberingType();
            if (SVX_NUM_BITMAP == (nNumberingType & (~LINK_TOKEN)))
            {
                bBmp |= nullptr != aNumFmt.GetBrush();
                aNumFmt.SetIncludeUpperLevels(0);
                aNumFmt.SetSuffix("");
                aNumFmt.SetPrefix("");
                if (!bBmp)
                    aNumFmt.SetGraphic("");
                pActNum->SetLevel(i, aNumFmt);
                SwitchNumberType(SHOW_BITMAP);
            }
            else if (SVX_NUM_CHAR_SPECIAL == nNumberingType)
            {
                aNumFmt.SetIncludeUpperLevels(0);
                aNumFmt.SetSuffix("");
                aNumFmt.SetPrefix("");
                if (!aNumFmt.GetBulletFont())
                    aNumFmt.SetBulletFont(&aActBulletFont);
                if (!aNumFmt.GetBulletChar())
                    aNumFmt.SetBulletChar(SVX_DEF_BULLET);
                pActNum->SetLevel(i, aNumFmt);
                SwitchNumberType(SHOW_BULLET);
                // allocation of the drawing pattern is automatic
                sSelectStyle = m_sBulletCharFormatName;
            }
            else
            {
                aNumFmt.SetPrefix(m_xPrefixED->get_text());
                aNumFmt.SetSuffix(m_xSuffixED->get_text());
                SwitchNumberType(SHOW_NUMBERING);
                pActNum->SetLevel(i, aNumFmt);
                CheckForStartValue_Impl(nNumberingType);

                // allocation of the drawing pattern is automatic
                sSelectStyle = m_sNumCharFmtName;
            }
        }
        nMask <<= 1;
    }

    SetModified();
}

IMPL_LINK(SvxBulletAndPositionDlg, BulColorHdl_Impl, ColorListBox&, rColorBox, void)
{
    Color nSetColor = rColorBox.GetSelectEntryColor();

    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetBulletColor(nSetColor);
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
}

IMPL_LINK(SvxBulletAndPositionDlg, BulRelSizeHdl_Impl, weld::MetricSpinButton&, rField, void)
{
    sal_uInt16 nRelSize = rField.get_value(FieldUnit::PERCENT);

    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetBulletRelSize(nRelSize);
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
}

IMPL_LINK(SvxBulletAndPositionDlg, GraphicHdl_Impl, const OString&, rIdent, void)
{
    OUString aGrfName;
    Size aSize;
    bool bSucc(false);
    SvxOpenGraphicDialog aGrfDlg(SdResId(RID_SVXSTR_EDIT_GRAPHIC), p_Window);

    OString sNumber;
    if (rIdent.startsWith("gallery", &sNumber))
    {
        auto idx = sNumber.toUInt32();
        if (idx < aGrfNames.size())
        {
            aGrfName = aGrfNames[idx];
            Graphic aGraphic;
            if (GalleryExplorer::GetGraphicObj(GALLERY_THEME_BULLETS, idx, &aGraphic))
            {
                aSize = SvxNumberFormat::GetGraphicSizeMM100(&aGraphic);
                bSucc = true;
            }
        }
    }
    else if (rIdent == "fromfile")
    {
        aGrfDlg.EnableLink(false);
        aGrfDlg.AsLink(false);
        if (!aGrfDlg.Execute())
        {
            // memorize selected filter
            aGrfName = aGrfDlg.GetPath();

            Graphic aGraphic;
            if (!aGrfDlg.GetGraphic(aGraphic))
            {
                aSize = SvxNumberFormat::GetGraphicSizeMM100(&aGraphic);
                bSucc = true;
            }
        }
    }
    if (!bSucc)
        return;

    aSize = OutputDevice::LogicToLogic(aSize, MapMode(MapUnit::Map100thMM), MapMode(eCoreUnit));

    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetCharFormatName(m_sNumCharFmtName);
            aNumFmt.SetGraphic(aGrfName);

            // set size for a later comparison
            const SvxBrushItem* pBrushItem = aNumFmt.GetBrush();
            // initiate asynchronous loading
            sal_Int16 eOrient = aNumFmt.GetVertOrient();
            aNumFmt.SetGraphicBrush(pBrushItem, &aSize, &eOrient);
            aInitSize[i] = aNumFmt.GetGraphicSize();

            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    m_xRatioCB->set_sensitive(true);
    m_xWidthFT->set_sensitive(true);
    m_xHeightFT->set_sensitive(true);
    m_xWidthMF->set_sensitive(true);
    m_xHeightMF->set_sensitive(true);
    SetMetricValue(*m_xWidthMF, aSize.Width(), eCoreUnit);
    SetMetricValue(*m_xHeightMF, aSize.Height(), eCoreUnit);

    SetModified();
    //needed due to asynchronous loading of graphics in the SvxBrushItem
    aInvalidateTimer.Start();
}

IMPL_LINK_NOARG(SvxBulletAndPositionDlg, PopupActivateHdl_Impl, weld::ToggleButton&, void)
{
    if (m_xGalleryMenu)
        return;

    m_xGalleryMenu = m_xBuilder->weld_menu("gallerysubmenu");
    weld::WaitObject aWait(p_Window);

    if (!GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames))
        return;

    GalleryExplorer::BeginLocking(GALLERY_THEME_BULLETS);

    Graphic aGraphic;
    OUString sGrfName;
    ScopedVclPtrInstance<VirtualDevice> pVD;
    size_t i = 0;
    for (const auto& grfName : aGrfNames)
    {
        sGrfName = grfName;
        OUString sItemId = "gallery" + OUString::number(i);
        INetURLObject aObj(sGrfName);
        if (aObj.GetProtocol() == INetProtocol::File)
            sGrfName = aObj.PathToFileName();
        if (GalleryExplorer::GetGraphicObj(GALLERY_THEME_BULLETS, i, &aGraphic))
        {
            BitmapEx aBitmap(aGraphic.GetBitmapEx());
            Size aSize(aBitmap.GetSizePixel());
            if (aSize.Width() > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
            {
                bool bWidth = aSize.Width() > aSize.Height();
                double nScale = bWidth
                                    ? double(MAX_BMP_WIDTH) / static_cast<double>(aSize.Width())
                                    : double(MAX_BMP_HEIGHT) / static_cast<double>(aSize.Height());
                aBitmap.Scale(nScale, nScale);
            }
            pVD->SetOutputSizePixel(aBitmap.GetSizePixel(), false);
            pVD->DrawBitmapEx(Point(), aBitmap);

            // We want to show only icon names not full path.
            aObj.removeExtension();
            OUString sIconName = aObj.GetLastName(INetURLObject::DecodeMechanism::WithCharset);

            m_xGalleryMenu->append(sItemId, sIconName, *pVD);
        }
        else
        {
            m_xGalleryMenu->append(sItemId, sGrfName);
        }
        ++i;
    }
    GalleryExplorer::EndLocking(GALLERY_THEME_BULLETS);
}

IMPL_LINK_NOARG(SvxBulletAndPositionDlg, BulletHdl_Impl, weld::Button&, void)
{
    SvxCharacterMap aMap(p_Window, nullptr, nullptr);

    sal_uInt16 nMask = 1;
    const vcl::Font* pFmtFont = nullptr;
    bool bSameBullet = true;
    sal_UCS4 cBullet = 0;
    bool bFirst = true;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            const SvxNumberFormat& rCurFmt = pActNum->GetLevel(i);
            if (bFirst)
            {
                cBullet = rCurFmt.GetBulletChar();
            }
            else if (rCurFmt.GetBulletChar() != cBullet)
            {
                bSameBullet = false;
                break;
            }
            if (!pFmtFont)
                pFmtFont = rCurFmt.GetBulletFont();
            bFirst = false;
        }
        nMask <<= 1;
    }

    if (pFmtFont)
        aMap.SetCharFont(*pFmtFont);
    else
        aMap.SetCharFont(aActBulletFont);
    if (bSameBullet)
        aMap.SetChar(cBullet);
    if (aMap.run() != RET_OK)
        return;

    // change Font Numrules
    aActBulletFont = aMap.GetCharFont();

    sal_uInt16 _nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & _nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetBulletFont(&aActBulletFont);
            aNumFmt.SetBulletChar(aMap.GetChar());
            pActNum->SetLevel(i, aNumFmt);
        }
        _nMask <<= 1;
    }

    SetModified();
}

IMPL_LINK(SvxBulletAndPositionDlg, SizeHdl_Impl, weld::MetricSpinButton&, rField, void)
{
    bool bWidth = &rField == m_xWidthMF.get();
    bLastWidthModified = bWidth;
    bool bRatio = m_xRatioCB->get_active();
    tools::Long nWidthVal = static_cast<tools::Long>(
        m_xWidthMF->denormalize(m_xWidthMF->get_value(FieldUnit::MM_100TH)));
    tools::Long nHeightVal = static_cast<tools::Long>(
        m_xHeightMF->denormalize(m_xHeightMF->get_value(FieldUnit::MM_100TH)));
    nWidthVal = OutputDevice::LogicToLogic(nWidthVal, MapUnit::Map100thMM, eCoreUnit);
    nHeightVal = OutputDevice::LogicToLogic(nHeightVal, MapUnit::Map100thMM, eCoreUnit);
    double fSizeRatio;

    bool bRepaint = false;
    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if (SVX_NUM_BITMAP == (aNumFmt.GetNumberingType() & (~LINK_TOKEN)))
            {
                Size aSize(aNumFmt.GetGraphicSize());
                Size aSaveSize(aSize);

                if (aInitSize[i].Height())
                    fSizeRatio = static_cast<double>(aInitSize[i].Width())
                                 / static_cast<double>(aInitSize[i].Height());
                else
                    fSizeRatio = double(1);

                if (bWidth)
                {
                    tools::Long nDelta = nWidthVal - aInitSize[i].Width();
                    aSize.setWidth(nWidthVal);
                    if (bRatio)
                    {
                        aSize.setHeight(
                            aInitSize[i].Height()
                            + static_cast<tools::Long>(static_cast<double>(nDelta) / fSizeRatio));
                        m_xHeightMF->set_value(m_xHeightMF->normalize(OutputDevice::LogicToLogic(
                                                   aSize.Height(), eCoreUnit, MapUnit::Map100thMM)),
                                               FieldUnit::MM_100TH);
                    }
                }
                else
                {
                    tools::Long nDelta = nHeightVal - aInitSize[i].Height();
                    aSize.setHeight(nHeightVal);
                    if (bRatio)
                    {
                        aSize.setWidth(
                            aInitSize[i].Width()
                            + static_cast<tools::Long>(static_cast<double>(nDelta) * fSizeRatio));
                        m_xWidthMF->set_value(m_xWidthMF->normalize(OutputDevice::LogicToLogic(
                                                  aSize.Width(), eCoreUnit, MapUnit::Map100thMM)),
                                              FieldUnit::MM_100TH);
                    }
                }
                const SvxBrushItem* pBrushItem = aNumFmt.GetBrush();
                sal_Int16 eOrient = aNumFmt.GetVertOrient();
                if (aSize != aSaveSize)
                    bRepaint = true;
                aNumFmt.SetGraphicBrush(pBrushItem, &aSize, &eOrient);
                pActNum->SetLevel(i, aNumFmt);
            }
        }
        nMask <<= 1;
    }
    SetModified(bRepaint);
}

IMPL_LINK(SvxBulletAndPositionDlg, RatioHdl_Impl, weld::ToggleButton&, rBox, void)
{
    if (rBox.get_active())
    {
        if (bLastWidthModified)
            SizeHdl_Impl(*m_xWidthMF);
        else
            SizeHdl_Impl(*m_xHeightMF);
    }
}

IMPL_LINK(SvxBulletAndPositionDlg, SelectLeftAlignmentHdl_Impl, weld::ToggleButton&, rButton, void)
{
    if (rButton.get_active())
    {
        SetAlignmentHdl_Impl(SvxAdjust::Left);

        m_xCenterTB->set_active(false);
        m_xRightTB->set_active(false);

        SetModified();
    }
}

IMPL_LINK(SvxBulletAndPositionDlg, SelectCenterAlignmentHdl_Impl, weld::ToggleButton&, rButton,
          void)
{
    if (rButton.get_active())
    {
        SetAlignmentHdl_Impl(SvxAdjust::Center);

        m_xLeftTB->set_active(false);
        m_xRightTB->set_active(false);

        SetModified();
    }
}

IMPL_LINK(SvxBulletAndPositionDlg, SelectRightAlignmentHdl_Impl, weld::ToggleButton&, rButton, void)
{
    if (rButton.get_active())
    {
        SetAlignmentHdl_Impl(SvxAdjust::Right);

        m_xLeftTB->set_active(false);
        m_xCenterTB->set_active(false);

        SetModified();
    }
}

void SvxBulletAndPositionDlg::SetAlignmentHdl_Impl(SvxAdjust eAdjust)
{
    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetNumAdjust(eAdjust);
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
}

IMPL_LINK(SvxBulletAndPositionDlg, ApplyToMasterHdl_Impl, weld::ToggleButton&, rButton, void)
{
    bApplyToMaster = rButton.get_active();
}

IMPL_LINK_NOARG(SvxBulletAndPositionDlg, ResetHdl_Impl, weld::Button&, void)
{
    Reset(&rFirstStateSet);
}

IMPL_LINK(SvxBulletAndPositionDlg, EditModifyHdl_Impl, weld::Entry&, rEdit, void)
{
    EditModifyHdl_Impl(&rEdit);
}

IMPL_LINK(SvxBulletAndPositionDlg, DistanceHdl_Impl, weld::MetricSpinButton&, rFld, void)
{
    if (bInInitControl)
        return;
    tools::Long nValue = GetCoreValue(rFld, eCoreUnit);
    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if (&rFld == m_xDistBorderMF.get())
            {
                if (m_xRelativeCB->get_active())
                {
                    if (0 == i)
                    {
                        auto const nTmp = aNumFmt.GetFirstLineOffset();
                        aNumFmt.SetAbsLSpace(nValue - nTmp);
                    }
                    else
                    {
                        tools::Long nTmp = pActNum->GetLevel(i - 1).GetAbsLSpace()
                                           + pActNum->GetLevel(i - 1).GetFirstLineOffset()
                                           - pActNum->GetLevel(i).GetFirstLineOffset();

                        aNumFmt.SetAbsLSpace(nValue + nTmp);
                    }
                }
                else
                {
                    aNumFmt.SetAbsLSpace(nValue - aNumFmt.GetFirstLineOffset());
                }
            }
            else if (&rFld == m_xIndentMF.get())
            {
                // together with the FirstLineOffset the AbsLSpace must be changed, too
                tools::Long nDiff = nValue + aNumFmt.GetFirstLineOffset();
                auto const nAbsLSpace = aNumFmt.GetAbsLSpace();
                aNumFmt.SetAbsLSpace(nAbsLSpace + nDiff);
                aNumFmt.SetFirstLineOffset(-nValue);
            }

            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }

    SetModified();
    if (!m_xDistBorderMF->get_sensitive())
    {
        m_xDistBorderMF->set_text("");
    }

    sal_Int32 aLastLevelLSpace
        = pActNum->GetLevel(pActNum->GetLevelCount() - 1).GetAbsLSpace() / 40;
    m_aPreviewWIN.set_size_request(aLastLevelLSpace, 300);
}

IMPL_LINK(SvxBulletAndPositionDlg, RelativeHdl_Impl, weld::ToggleButton&, rBox, void)
{
    bool bOn = rBox.get_active();
    bool bSingleSelection = m_xLevelLB->count_selected_rows() == 1 && SAL_MAX_UINT16 != nActNumLvl;
    bool bSetValue = false;
    tools::Long nValue = 0;
    if (bOn || bSingleSelection)
    {
        sal_uInt16 nMask = 1;
        bool bFirst = true;
        bSetValue = true;
        for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if (nActNumLvl & nMask)
            {
                const SvxNumberFormat& rNumFmt = pActNum->GetLevel(i);
                if (bFirst)
                {
                    nValue = rNumFmt.GetAbsLSpace() + rNumFmt.GetFirstLineOffset();
                    if (bOn && i)
                        nValue -= (pActNum->GetLevel(i - 1).GetAbsLSpace()
                                   + pActNum->GetLevel(i - 1).GetFirstLineOffset());
                }
                else
                    bSetValue = nValue
                                == (rNumFmt.GetAbsLSpace() + rNumFmt.GetFirstLineOffset())
                                       - (pActNum->GetLevel(i - 1).GetAbsLSpace()
                                          + pActNum->GetLevel(i - 1).GetFirstLineOffset());
                bFirst = false;
            }
            nMask <<= 1;
        }
    }
    if (bSetValue)
        SetMetricValue(*m_xDistBorderMF, nValue, eCoreUnit);
    else
        m_xDistBorderMF->set_text("");
    m_xDistBorderMF->set_sensitive(bOn || bSingleSelection);
    m_xDistBorderFT->set_sensitive(bOn || bSingleSelection);
    bLastRelative = bOn;
}

void SvxBulletAndPositionDlg::EditModifyHdl_Impl(const weld::Entry* pEdit)
{
    bool bPrefix = pEdit == m_xPrefixED.get();
    bool bSuffix = pEdit == m_xSuffixED.get();
    bool bStart = pEdit == m_xStartED.get();
    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if (bPrefix)
                aNumFmt.SetPrefix(m_xPrefixED->get_text());
            else if (bSuffix)
                aNumFmt.SetSuffix(m_xSuffixED->get_text());
            else if (bStart)
                aNumFmt.SetStart(m_xStartED->get_value());
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
}

void SvxBulletAndPositionDlg::SetModified(bool bRepaint)
{
    bModified = true;
    if (bRepaint)
    {
        m_aPreviewWIN.SetLevel(nActNumLvl);
        m_aPreviewWIN.Invalidate();
    }
}

void SvxBulletAndPositionDlg::InitPosAndSpaceMode()
{
    if (pActNum == nullptr)
    {
        SAL_WARN("cui.tabpages", "<SvxNumPositionTabPage::InitPosAndSpaceMode()> - misusage of "
                                 "method -> <pAktNum> has to be already set!");
        return;
    }

    SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode = SvxNumberFormat::LABEL_ALIGNMENT;
    sal_uInt16 nMask = 1;
    for (sal_uInt16 i = 0; i < pActNum->GetLevelCount(); ++i)
    {
        if (nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            ePosAndSpaceMode = aNumFmt.GetPositionAndSpaceMode();
            if (ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT)
            {
                break;
            }
        }
        nMask <<= 1;
    }

    bLabelAlignmentPosAndSpaceModeActive = ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
