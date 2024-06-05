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

#include <editeng/numitem.hxx>
#include <cmdid.h>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <wview.hxx>
#include <uitool.hxx>
#include <wdocsh.hxx>
#include <uiitems.hxx>
#include <poolfmt.hxx>
#include <shellres.hxx>
#include <outline.hxx>
#include <num.hxx>

#include <SwStyleNameMapper.hxx>
#include <svx/dialogs.hrc>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/intitem.hxx>
#include <comphelper/lok.hxx>
#include <osl/diagnose.h>

static bool bLastRelative = false;

//See cui/uiconfig/ui/numberingpositionpage.ui for effectively a duplicate
//dialog to this one, except with a different preview window impl.
//TODO, determine if SwNumPositionTabPage and SvxNumPositionTabPage can be
//merged
SwNumPositionTabPage::SwNumPositionTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/outlinepositionpage.ui"_ustr, u"OutlinePositionPage"_ustr, &rSet)
    , m_pSaveNum(nullptr)
    , m_pWrtSh(nullptr)
    , m_pOutlineDlg(nullptr)
    , m_nActNumLvl(0)
    , m_bModified(false)
    , m_bPreset(false)
    , m_bInInintControl(false)
    , m_bLabelAlignmentPosAndSpaceModeActive(false)
    , m_xLevelLB(m_xBuilder->weld_tree_view(u"levellb"_ustr))
    , m_xPositionFrame(m_xBuilder->weld_widget(u"numberingframe"_ustr))
    , m_xDistBorderFT(m_xBuilder->weld_label(u"indent"_ustr))
    , m_xDistBorderMF(m_xBuilder->weld_metric_spin_button(u"indentmf"_ustr, FieldUnit::CM))
    , m_xRelativeCB(m_xBuilder->weld_check_button(u"relative"_ustr))
    , m_xIndentFT(m_xBuilder->weld_label(u"numberingwidth"_ustr))
    , m_xIndentMF(m_xBuilder->weld_metric_spin_button(u"numberingwidthmf"_ustr, FieldUnit::CM))
    , m_xDistNumFT(m_xBuilder->weld_label(u"numdist"_ustr))
    , m_xDistNumMF(m_xBuilder->weld_metric_spin_button(u"numdistmf"_ustr, FieldUnit::CM))
    , m_xAlignFT(m_xBuilder->weld_label(u"numalign"_ustr))
    , m_xAlignLB(m_xBuilder->weld_combo_box(u"numalignlb"_ustr))
    , m_xLabelFollowedByFT(m_xBuilder->weld_label(u"numfollowedby"_ustr))
    , m_xLabelFollowedByLB(m_xBuilder->weld_combo_box(u"numfollowedbylb"_ustr))
    , m_xListtabFT(m_xBuilder->weld_label(u"at"_ustr))
    , m_xListtabMF(m_xBuilder->weld_metric_spin_button(u"atmf"_ustr, FieldUnit::CM))
    , m_xAlign2FT(m_xBuilder->weld_label(u"num2align"_ustr))
    , m_xAlign2LB(m_xBuilder->weld_combo_box(u"num2alignlb"_ustr))
    , m_xAlignedAtFT(m_xBuilder->weld_label(u"alignedat"_ustr))
    , m_xAlignedAtMF(m_xBuilder->weld_metric_spin_button(u"alignedatmf"_ustr, FieldUnit::CM))
    , m_xIndentAtFT(m_xBuilder->weld_label(u"indentat"_ustr))
    , m_xIndentAtMF(m_xBuilder->weld_metric_spin_button(u"indentatmf"_ustr, FieldUnit::CM))
    , m_xStandardPB(m_xBuilder->weld_button(u"standard"_ustr))
    , m_xPreviewWIN(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, m_aPreviewWIN))
{
    SetExchangeSupport();

    m_xAlignedAtMF->set_range(0, SAL_MAX_INT32, FieldUnit::NONE);
    m_xListtabMF->set_range(0, SAL_MAX_INT32, FieldUnit::NONE);
    m_xIndentAtMF->set_range(0, SAL_MAX_INT32, FieldUnit::NONE);

    m_xLevelLB->set_selection_mode(SelectionMode::Multiple);

    m_xRelativeCB->set_active(true);
    m_xAlignLB->connect_changed(LINK(this, SwNumPositionTabPage, EditModifyHdl));
    m_xAlign2LB->connect_changed(LINK(this, SwNumPositionTabPage, EditModifyHdl));
    for (int i = 0; i < m_xAlignLB->get_count(); ++i)
    {
        m_xAlign2LB->append_text(m_xAlignLB->get_text(i));
    }
    m_xAlign2FT->set_label(m_xAlignFT->get_label());

    Link<weld::MetricSpinButton&, void> aLk = LINK(this, SwNumPositionTabPage, DistanceHdl);
    m_xDistBorderMF->connect_value_changed(aLk);
    m_xDistNumMF->connect_value_changed(aLk);
    m_xIndentMF->connect_value_changed(aLk);

    m_xLabelFollowedByLB->connect_changed( LINK(this, SwNumPositionTabPage, LabelFollowedByHdl_Impl) );

    aLk = LINK(this, SwNumPositionTabPage, ListtabPosHdl_Impl);
    m_xListtabMF->connect_value_changed(aLk);

    aLk = LINK(this, SwNumPositionTabPage, AlignAtHdl_Impl);
    m_xAlignedAtMF->connect_value_changed(aLk);

    aLk = LINK(this, SwNumPositionTabPage, IndentAtHdl_Impl);
    m_xIndentAtMF->connect_value_changed(aLk);

    m_xLevelLB->connect_changed(LINK(this, SwNumPositionTabPage, LevelHdl));
    m_xRelativeCB->connect_toggled(LINK(this, SwNumPositionTabPage, RelativeHdl));
    m_xStandardPB->connect_clicked(LINK(this, SwNumPositionTabPage, StandardHdl));

    // insert levels
    for(sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        m_xLevelLB->append_text(OUString::number(i));
    OUString sEntry = "1 - " + OUString::number(MAXLEVEL);
    m_xLevelLB->append_text(sEntry);
    m_xLevelLB->select_text(sEntry);

    m_xRelativeCB->set_active(bLastRelative);
    m_aPreviewWIN.SetPositionMode();
}

SwNumPositionTabPage::~SwNumPositionTabPage()
{
    m_pActNum.reset();
    m_pOutlineDlg = nullptr;
}

void SwNumPositionTabPage::InitControls()
{
    m_bInInintControl = true;
    const bool bRelative = !m_bLabelAlignmentPosAndSpaceModeActive &&
                           m_xRelativeCB->get_sensitive() && m_xRelativeCB->get_active();
    const bool bSingleSelection = m_xLevelLB->count_selected_rows() == 1 &&
                                  USHRT_MAX != m_nActNumLvl;

    m_xDistBorderMF->set_sensitive( !m_bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative || m_pOutlineDlg != nullptr ) );
    m_xDistBorderFT->set_sensitive( !m_bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative || m_pOutlineDlg != nullptr ) );

    bool bSetDistEmpty = false;
    bool bSameDistBorderNum = !m_bLabelAlignmentPosAndSpaceModeActive;
    bool bSameDist      = !m_bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndent    = !m_bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAdjust    = true;

    bool bSameLabelFollowedBy = m_bLabelAlignmentPosAndSpaceModeActive;
    bool bSameListtab = m_bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAlignAt = m_bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndentAt = m_bLabelAlignmentPosAndSpaceModeActive;

    const SwNumFormat* aNumFormatArr[MAXLEVEL];
    sal_uInt16 nMask = 1;
    sal_uInt16 nLvl = USHRT_MAX;
    tools::Long nFirstBorderTextRelative = -1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        aNumFormatArr[i] = &m_pActNum->Get(i);
        if(m_nActNumLvl & nMask)
        {
            if(USHRT_MAX == nLvl)
            {
                nLvl = i;
            }

            if( i > nLvl)
            {
                bSameAdjust &= aNumFormatArr[i]->GetNumAdjust() == aNumFormatArr[nLvl]->GetNumAdjust();
                if ( !m_bLabelAlignmentPosAndSpaceModeActive )
                {
                    if(bRelative)
                    {
                        const tools::Long nBorderTextRelative =
                            aNumFormatArr[i]->GetAbsLSpace() + aNumFormatArr[i]->GetFirstLineOffset() -
                            aNumFormatArr[i - 1]->GetAbsLSpace() + aNumFormatArr[i - 1]->GetFirstLineOffset();
                        if (nFirstBorderTextRelative == -1)
                            nFirstBorderTextRelative = nBorderTextRelative;
                        else
                            bSameDistBorderNum &= nFirstBorderTextRelative == nBorderTextRelative;
                    }
                    else
                    {
                        bSameDistBorderNum &=
                        aNumFormatArr[i]->GetAbsLSpace() - aNumFormatArr[i]->GetFirstLineOffset() ==
                        aNumFormatArr[i - 1]->GetAbsLSpace() - aNumFormatArr[i - 1]->GetFirstLineOffset();
                    }

                    bSameDist       &= aNumFormatArr[i]->GetCharTextDistance() == aNumFormatArr[nLvl]->GetCharTextDistance();
                    bSameIndent     &= aNumFormatArr[i]->GetFirstLineOffset() == aNumFormatArr[nLvl]->GetFirstLineOffset();
                }
                else
                {
                    bSameLabelFollowedBy &=
                        aNumFormatArr[i]->GetLabelFollowedBy() == aNumFormatArr[nLvl]->GetLabelFollowedBy();
                    bSameListtab &=
                        aNumFormatArr[i]->GetListtabPos() == aNumFormatArr[nLvl]->GetListtabPos();
                    bSameAlignAt &=
                        ( ( aNumFormatArr[i]->GetIndentAt() + aNumFormatArr[i]->GetFirstLineIndent() )
                            == ( aNumFormatArr[nLvl]->GetIndentAt() + aNumFormatArr[nLvl]->GetFirstLineIndent() ) );
                    bSameIndentAt &=
                        aNumFormatArr[i]->GetIndentAt() == aNumFormatArr[nLvl]->GetIndentAt();
                }
            }
        }
        nMask <<= 1;

    }
    if (MAXLEVEL <= nLvl)
    {
        OSL_ENSURE(false, "cannot happen.");
        return;
    }
    if(bSameDistBorderNum)
    {
        tools::Long nDistBorderNum;
        if(bRelative)
        {
            nDistBorderNum = static_cast<tools::Long>(aNumFormatArr[nLvl]->GetAbsLSpace())+ aNumFormatArr[nLvl]->GetFirstLineOffset();
            if(nLvl)
                nDistBorderNum -= static_cast<tools::Long>(aNumFormatArr[nLvl - 1]->GetAbsLSpace())+ aNumFormatArr[nLvl - 1]->GetFirstLineOffset();
        }
        else
        {
            nDistBorderNum = static_cast<tools::Long>(aNumFormatArr[nLvl]->GetAbsLSpace())+ aNumFormatArr[nLvl]->GetFirstLineOffset();
        }
        m_xDistBorderMF->set_value(m_xDistBorderMF->normalize(nDistBorderNum),FieldUnit::TWIP);
    }
    else
        bSetDistEmpty = true;

    if(bSameDist)
        m_xDistNumMF->set_value(m_xDistNumMF->normalize(aNumFormatArr[nLvl]->GetCharTextDistance()), FieldUnit::TWIP);
    else
        m_xDistNumMF->set_text(OUString());
    if(bSameIndent)
        m_xIndentMF->set_value(m_xIndentMF->normalize(-aNumFormatArr[nLvl]->GetFirstLineOffset()), FieldUnit::TWIP);
    else
        m_xIndentMF->set_text(OUString());

    if(bSameAdjust)
    {
        sal_Int32 nPos = 1; // centered
        if(aNumFormatArr[nLvl]->GetNumAdjust() == SvxAdjust::Left)
            nPos = 0;
        else if(aNumFormatArr[nLvl]->GetNumAdjust() == SvxAdjust::Right)
            nPos = 2;
        m_xAlignLB->set_active(nPos);
        m_xAlign2LB->set_active( nPos );
    }
    else
    {
        m_xAlignLB->set_active(-1);
        m_xAlign2LB->set_active(-1);
    }

    if ( bSameLabelFollowedBy )
    {
        sal_Int32 nPos = 0; // LISTTAB
        if ( aNumFormatArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::SPACE )
        {
            nPos = 1;
        }
        else if ( aNumFormatArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::NOTHING )
        {
            nPos = 2;
        }
        else if ( aNumFormatArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::NEWLINE )
        {
            nPos = 3;
        }
        m_xLabelFollowedByLB->set_active(nPos);
    }
    else
    {
        m_xLabelFollowedByLB->set_active(-1);
    }

    if ( aNumFormatArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
    {
        m_xListtabFT->set_sensitive(true);
        m_xListtabMF->set_sensitive(true);
        if ( bSameListtab )
        {
            m_xListtabMF->set_value(m_xListtabMF->normalize(aNumFormatArr[nLvl]->GetListtabPos()),FieldUnit::TWIP);
        }
        else
        {
            m_xListtabMF->set_text(OUString());
        }
    }
    else
    {
        m_xListtabFT->set_sensitive( false );
        m_xListtabMF->set_sensitive( false );
        m_xListtabMF->set_text(OUString());
    }

    if ( bSameAlignAt )
    {
        m_xAlignedAtMF->set_value(
            m_xAlignedAtMF->normalize( aNumFormatArr[nLvl]->GetIndentAt() +
                                    aNumFormatArr[nLvl]->GetFirstLineIndent()),
            FieldUnit::TWIP );
    }
    else
    {
        m_xAlignedAtMF->set_text(OUString());
    }

    if ( bSameIndentAt )
    {
        m_xIndentAtMF->set_value(
            m_xIndentAtMF->normalize( aNumFormatArr[nLvl]->GetIndentAt()), FieldUnit::TWIP );
    }
    else
    {
        m_xIndentAtMF->set_text(OUString());
    }

    if (bSetDistEmpty)
        m_xDistBorderMF->set_text(OUString());

    m_bInInintControl = false;
}

void SwNumPositionTabPage::ActivatePage(const SfxItemSet& )
{
    const SfxPoolItem* pItem;
    sal_uInt16 nTmpNumLvl =
        m_pOutlineDlg ? SwOutlineTabDialog::GetActNumLevel() : 0;
    const SfxItemSet* pExampleSet = GetDialogExampleSet();
    if(pExampleSet && pExampleSet->GetItemState(FN_PARAM_NUM_PRESET, false, &pItem) != SfxItemState::UNKNOWN)
    {
        m_bPreset = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    }
    m_bModified = (!m_pActNum->GetNumFormat( 0 ) || m_bPreset);
    if(*m_pActNum != *m_pSaveNum ||
        m_nActNumLvl != nTmpNumLvl )
    {
        *m_pActNum = *m_pSaveNum;
        m_nActNumLvl = nTmpNumLvl;
        sal_uInt16 nMask = 1;
        m_xLevelLB->unselect_all();
        if (m_nActNumLvl == USHRT_MAX)
            m_xLevelLB->select(MAXLEVEL);
        else
        {
            for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
            {
                if (m_nActNumLvl & nMask)
                    m_xLevelLB->select(i);
                nMask <<= 1 ;
            }
        }

        InitPosAndSpaceMode();
        ShowControlsDependingOnPosAndSpaceMode();

        InitControls();
    }
    m_xRelativeCB->set_sensitive(1 != m_nActNumLvl);
    m_aPreviewWIN.Invalidate();
}

DeactivateRC SwNumPositionTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    SwOutlineTabDialog::SetActNumLevel(m_nActNumLvl);
    if(_pSet)
        FillItemSet(_pSet);
    return DeactivateRC::LeavePage;

}

bool SwNumPositionTabPage::FillItemSet( SfxItemSet* rSet )
{
    if(m_pOutlineDlg)
        *m_pOutlineDlg->GetNumRule() = *m_pActNum;
    else if(m_bModified && m_pActNum)
    {
        *m_pSaveNum = *m_pActNum;
        rSet->Put(SwUINumRuleItem( *m_pSaveNum ));
        rSet->Put(SfxBoolItem(FN_PARAM_NUM_PRESET, false));
    }
    return m_bModified;
}

void SwNumPositionTabPage::Reset( const SfxItemSet* rSet )
{
    if (m_pOutlineDlg)
    {
        m_pSaveNum = m_pOutlineDlg->GetNumRule();
        m_xLevelLB->set_selection_mode(SelectionMode::Single);
    }
    else if(const SwUINumRuleItem* pNumberItem = rSet->GetItemIfSet(FN_PARAM_ACT_NUMBER, false))
        m_pSaveNum = const_cast<SwUINumRuleItem*>(pNumberItem)->GetNumRule();

    m_nActNumLvl = SwOutlineTabDialog::GetActNumLevel();
    sal_uInt16 nMask = 1;
    m_xLevelLB->unselect_all();
    if(m_nActNumLvl == USHRT_MAX)
    {
        m_xLevelLB->select(MAXLEVEL);
    }
    else
    {
        for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
        {
            if (m_nActNumLvl & nMask)
                m_xLevelLB->select(i);
            nMask <<= 1;
        }
    }

    if(!m_pActNum)
        m_pActNum.reset(new  SwNumRule(*m_pSaveNum));
    else if(*m_pSaveNum != *m_pActNum)
        *m_pActNum = *m_pSaveNum;
    m_aPreviewWIN.SetNumRule(m_pActNum.get());
    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();
    InitControls();
    m_bModified = false;
}

void SwNumPositionTabPage::InitPosAndSpaceMode()
{
    if ( m_pActNum == nullptr )
    {
        OSL_FAIL( "<SwNumPositionTabPage::InitPosAndSpaceMode()> - misusage of method -> <pAktNum> has to be already set!" );
        return;
    }

    SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode =
                                            SvxNumberFormat::LABEL_ALIGNMENT;
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if(m_nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFormat( m_pActNum->Get(i) );
            ePosAndSpaceMode = aNumFormat.GetPositionAndSpaceMode();
            if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                break;
            }
        }
        nMask <<= 1;
    }

    m_bLabelAlignmentPosAndSpaceModeActive =
                    ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT;
}

void SwNumPositionTabPage::ShowControlsDependingOnPosAndSpaceMode()
{
    m_xDistBorderFT->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );
    m_xDistBorderMF->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );
    m_xRelativeCB->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );
    m_xIndentFT->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );
    m_xIndentMF->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );
    m_xDistNumFT->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );
    m_xDistNumMF->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );
    m_xAlignFT->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );
    m_xAlignLB->set_visible( !m_bLabelAlignmentPosAndSpaceModeActive );

    m_xLabelFollowedByFT->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xLabelFollowedByLB->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xListtabFT->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xListtabMF->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xAlign2FT->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xAlign2LB->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xAlignedAtFT->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xAlignedAtMF->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xIndentAtFT->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
    m_xIndentAtMF->set_visible( m_bLabelAlignmentPosAndSpaceModeActive );
}

std::unique_ptr<SfxTabPage> SwNumPositionTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                                 const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwNumPositionTabPage>(pPage, pController, *rAttrSet);
}

void SwNumPositionTabPage::SetWrtShell(SwWrtShell* pSh)
{
    m_pWrtSh = pSh;

    const SwTwips nWidth = m_pWrtSh->GetAnyCurRect(CurRectType::Frame).Width();

    m_xDistBorderMF->set_max(m_xDistBorderMF->normalize( nWidth ), FieldUnit::TWIP );
    m_xDistNumMF->set_max(m_xDistNumMF->normalize( nWidth ), FieldUnit::TWIP);
    m_xIndentMF->set_max(m_xIndentMF->normalize( nWidth ), FieldUnit::TWIP );

    const SwRect& rPrtRect = m_pWrtSh->GetAnyCurRect(CurRectType::Page);
    m_aPreviewWIN.SetPageWidth(rPrtRect.Width());
    FieldUnit eMetric = ::GetDfltMetric( dynamic_cast<SwWebView*>( &m_pWrtSh->GetView()) != nullptr  );
    if(eMetric == FieldUnit::MM)
    {
        m_xDistBorderMF->set_digits(1);
        m_xDistNumMF->set_digits(1);
        m_xIndentMF->set_digits(1);
        m_xListtabMF->set_digits(1);
        m_xAlignedAtMF->set_digits(1);
        m_xIndentAtMF->set_digits(1);
    }
    m_xDistBorderMF->set_unit( eMetric );
    m_xDistNumMF->set_unit( eMetric );
    m_xIndentMF->set_unit( eMetric );
    m_xListtabMF->set_unit( eMetric );
    m_xAlignedAtMF->set_unit( eMetric );
    m_xIndentAtMF->set_unit( eMetric );
}

IMPL_LINK_NOARG(SwNumPositionTabPage, EditModifyHdl, weld::ComboBox&, void)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(m_nActNumLvl & nMask)
        {
            SwNumFormat aNumFormat(m_pActNum->Get(i));

            const int nPos = m_xAlignLB->get_visible()
                                ? m_xAlignLB->get_active()
                                : m_xAlign2LB->get_active();
            SvxAdjust eAdjust = SvxAdjust::Center;
            if(nPos == 0)
                eAdjust = SvxAdjust::Left;
            else if(nPos == 2)
                eAdjust = SvxAdjust::Right;
            aNumFormat.SetNumAdjust( eAdjust );
            m_pActNum->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
    SetModified();
}

IMPL_LINK( SwNumPositionTabPage, LevelHdl, weld::TreeView&, rBox, void )
{
    sal_uInt16 nSaveNumLvl = m_nActNumLvl;
    m_nActNumLvl = 0;
    auto aRows = rBox.get_selected_rows();
    if ((std::find(aRows.begin(), aRows.end(), MAXLEVEL) != aRows.end()) &&
            (aRows.size() == 1 || nSaveNumLvl != 0xffff))
    {
        m_nActNumLvl = 0xFFFF;
        for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
            rBox.unselect(i);
    }
    else if (!aRows.empty())
    {
        sal_uInt16 nMask = 1;
        for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
        {
            if (std::find(aRows.begin(), aRows.end(), i) != aRows.end())
                m_nActNumLvl |= nMask;
            nMask <<= 1;
        }
        rBox.unselect(MAXLEVEL);
    }
    else
    {
        m_nActNumLvl = nSaveNumLvl;
        sal_uInt16 nMask = 1;
        for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
        {
            if(m_nActNumLvl & nMask)
            {
                rBox.select(i);
                break;
            }
            nMask <<=1;
        }
    }
    m_xRelativeCB->set_sensitive(1 != m_nActNumLvl);
    SetModified();
    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();
    InitControls();
}

IMPL_LINK(SwNumPositionTabPage, DistanceHdl, weld::MetricSpinButton&, rField, void)
{
    if(m_bInInintControl)
        return;
    tools::Long nValue = static_cast< tools::Long >(rField.denormalize(rField.get_value(FieldUnit::TWIP)));
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(m_nActNumLvl & nMask)
        {
            SwNumFormat aNumFormat( m_pActNum->Get( i ) );
            if (&rField == m_xDistBorderMF.get())
            {

                if (m_xRelativeCB->get_active() && m_xRelativeCB->get_sensitive())
                {
                    if(0 == i)
                    {
                        auto const nTmp = aNumFormat.GetFirstLineOffset();
                        aNumFormat.SetAbsLSpace( nValue - nTmp );
                    }
                    else
                    {
                        tools::Long nTmp = m_pActNum->Get( i - 1 ).GetAbsLSpace() +
                                    m_pActNum->Get( i - 1 ).GetFirstLineOffset() -
                                    m_pActNum->Get( i ).GetFirstLineOffset();

                        aNumFormat.SetAbsLSpace( nValue + nTmp );
                    }
                }
                else
                {
                    aNumFormat.SetAbsLSpace( nValue - aNumFormat.GetFirstLineOffset());
                }
            }
            else if (&rField == m_xDistNumMF.get())
            {
                aNumFormat.SetCharTextDistance( nValue );
            }
            else if (&rField == m_xIndentMF.get())
            {
                // now AbsLSpace also has to be modified by FirstLineOffset
                tools::Long nDiff = nValue + aNumFormat.GetFirstLineOffset();
                auto const nAbsLSpace = aNumFormat.GetAbsLSpace();
                aNumFormat.SetAbsLSpace( nAbsLSpace + nDiff );
                aNumFormat.SetFirstLineOffset( -nValue );
            }

            m_pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    SetModified();
    if(!m_xDistBorderMF->get_sensitive())
        m_xDistBorderMF->set_text(OUString());
}

IMPL_LINK( SwNumPositionTabPage, RelativeHdl, weld::Toggleable&, rBox, void )
{
    bool bOn = rBox.get_active();
    bool bSingleSelection = m_xLevelLB->n_children() == 1 && USHRT_MAX != m_nActNumLvl;
    bool bSetValue = false;
    tools::Long nValue = 0;
    if(bOn || bSingleSelection)
    {
        sal_uInt16 nMask = 1;
        bool bFirst = true;
        bSetValue = true;
        for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        {
            if(m_nActNumLvl & nMask)
            {
                const SwNumFormat &rNumFormat = m_pActNum->Get(i);
                if(bFirst)
                {
                    nValue = rNumFormat.GetAbsLSpace();
                    if(bOn && i)
                        nValue -= m_pActNum->Get(i - 1).GetAbsLSpace();
                }
                else
                    bSetValue = nValue == rNumFormat.GetAbsLSpace() - m_pActNum->Get(i - 1).GetAbsLSpace();
                bFirst = false;
            }
            nMask <<= 1;
        }

    }
    if(bSetValue)
        m_xDistBorderMF->set_value(m_xDistBorderMF->normalize(nValue), FieldUnit::TWIP);
    else
        m_xDistBorderMF->set_text(OUString());
    m_xDistBorderMF->set_sensitive(bOn || bSingleSelection || m_pOutlineDlg);
    bLastRelative = bOn;
}

IMPL_LINK_NOARG(SwNumPositionTabPage, LabelFollowedByHdl_Impl, weld::ComboBox&, void)
{
    // determine value to be set at the chosen list levels
    SvxNumberFormat::LabelFollowedBy eLabelFollowedBy = SvxNumberFormat::LISTTAB;
    {
        const int nPos = m_xLabelFollowedByLB->get_active();
        if ( nPos == 1 )
        {
            eLabelFollowedBy = SvxNumberFormat::SPACE;
        }
        else if ( nPos == 2 )
        {
            eLabelFollowedBy = SvxNumberFormat::NOTHING;
        }
        else if ( nPos == 3 )
        {
            eLabelFollowedBy = SvxNumberFormat::NEWLINE;
        }
    }

    // set value at the chosen list levels
    bool bSameListtabPos = true;
    sal_uInt16 nFirstLvl = USHRT_MAX;
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( m_nActNumLvl & nMask )
        {
            SwNumFormat aNumFormat( m_pActNum->Get(i) );
            aNumFormat.SetLabelFollowedBy( eLabelFollowedBy );
            m_pActNum->Set( i, aNumFormat );

            if ( nFirstLvl == USHRT_MAX )
            {
                nFirstLvl = i;
            }
            else
            {
                bSameListtabPos &= aNumFormat.GetListtabPos() ==
                        m_pActNum->Get( nFirstLvl ).GetListtabPos();
            }
        }
        nMask <<= 1;
    }

    // enable/disable metric field for list tab stop position depending on
    // selected item following the list label.
    m_xListtabFT->set_sensitive( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    m_xListtabMF->set_sensitive( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    if ( bSameListtabPos && eLabelFollowedBy == SvxNumberFormat::LISTTAB )
    {
        m_xListtabMF->set_value(
            m_xListtabMF->normalize( m_pActNum->Get( nFirstLvl ).GetListtabPos() ),
            FieldUnit::TWIP );
    }
    else
    {
        m_xListtabMF->set_text(OUString());
    }

    SetModified();
}

IMPL_LINK( SwNumPositionTabPage, ListtabPosHdl_Impl, weld::MetricSpinButton&, rField, void )
{
    // determine value to be set at the chosen list levels
    const tools::Long nValue = static_cast< tools::Long >(rField.denormalize(rField.get_value(FieldUnit::TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( m_nActNumLvl & nMask )
        {
            SwNumFormat aNumFormat( m_pActNum->Get(i) );
            aNumFormat.SetListtabPos( nValue );
            m_pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    SetModified();
}

IMPL_LINK( SwNumPositionTabPage, AlignAtHdl_Impl, weld::MetricSpinButton&, rField, void )
{
    // determine value to be set at the chosen list levels
    const tools::Long nValue = static_cast< tools::Long >(rField.denormalize(rField.get_value(FieldUnit::TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( m_nActNumLvl & nMask )
        {
            SwNumFormat aNumFormat( m_pActNum->Get(i) );
            const tools::Long nFirstLineIndent = nValue - aNumFormat.GetIndentAt();
            aNumFormat.SetFirstLineIndent( nFirstLineIndent );
            m_pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    SetModified();
}

IMPL_LINK( SwNumPositionTabPage, IndentAtHdl_Impl, weld::MetricSpinButton&, rField, void )
{
    // determine value to be set at the chosen list levels
    const tools::Long nValue = static_cast< tools::Long >(rField.denormalize(rField.get_value(FieldUnit::TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( m_nActNumLvl & nMask )
        {
            SwNumFormat aNumFormat( m_pActNum->Get(i) );
            const tools::Long nAlignedAt = aNumFormat.GetIndentAt() +
                                    aNumFormat.GetFirstLineIndent();
            aNumFormat.SetIndentAt( nValue );
            const tools::Long nNewFirstLineIndent = nAlignedAt - nValue;
            aNumFormat.SetFirstLineIndent( nNewFirstLineIndent );
            m_pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    SetModified();
}

IMPL_LINK_NOARG(SwNumPositionTabPage, StandardHdl, weld::Button&, void)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(m_nActNumLvl & nMask)
        {
            SwNumFormat aNumFormat( m_pActNum->Get( i ) );
            SwNumRule aTmpNumRule( m_pWrtSh->GetUniqueNumRuleName(),
                                   aNumFormat.GetPositionAndSpaceMode(),
                                   m_pOutlineDlg ? OUTLINE_RULE : NUM_RULE );
            const SwNumFormat& aTempFormat(aTmpNumRule.Get( i ));
            aNumFormat.SetPositionAndSpaceMode( aTempFormat.GetPositionAndSpaceMode() );
            if ( aTempFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aNumFormat.SetAbsLSpace( aTempFormat.GetAbsLSpace());
                aNumFormat.SetCharTextDistance( aTempFormat.GetCharTextDistance() );
                aNumFormat.SetFirstLineOffset( aTempFormat.GetFirstLineOffset() );
            }
            else if ( aTempFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aNumFormat.SetNumAdjust( aTempFormat.GetNumAdjust() );
                aNumFormat.SetLabelFollowedBy( aTempFormat.GetLabelFollowedBy() );
                aNumFormat.SetListtabPos( aTempFormat.GetListtabPos() );
                aNumFormat.SetFirstLineIndent( aTempFormat.GetFirstLineIndent() );
                aNumFormat.SetIndentAt( aTempFormat.GetIndentAt() );
            }
            m_pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    InitControls();
    SetModified();
}

#ifdef DBG_UTIL
void SwNumPositionTabPage::SetModified()
{
    m_bModified = true;
    m_aPreviewWIN.SetLevel(m_nActNumLvl);
    m_aPreviewWIN.Invalidate();
}
#endif

SwSvxNumBulletTabDialog::SwSvxNumBulletTabDialog(weld::Window* pParent,
                    const SfxItemSet& rSwItemSet, SwWrtShell & rSh)
    : SfxTabDialogController(pParent, u"modules/swriter/ui/bulletsandnumbering.ui"_ustr, u"BulletsAndNumberingDialog"_ustr,
        &rSwItemSet)
    , m_rWrtSh(rSh)
    , m_xDummyCombo(m_xBuilder->weld_combo_box(u"dummycombo"_ustr))
{
    weld::Button* pButton = GetUserButton();
    pButton->connect_clicked(LINK(this, SwSvxNumBulletTabDialog, RemoveNumberingHdl));
    pButton->set_sensitive(m_rWrtSh.GetNumRuleAtCurrCursorPos() != nullptr);
    AddTabPage(u"singlenum"_ustr, RID_SVXPAGE_PICK_SINGLE_NUM );
    AddTabPage(u"bullets"_ustr, RID_SVXPAGE_PICK_BULLET );
    AddTabPage(u"outlinenum"_ustr, RID_SVXPAGE_PICK_NUM );
    AddTabPage(u"graphics"_ustr, RID_SVXPAGE_PICK_BMP );
    AddTabPage(u"customize"_ustr, RID_SVXPAGE_NUM_OPTIONS );
    AddTabPage(u"position"_ustr, RID_SVXPAGE_NUM_POSITION );
}

SwSvxNumBulletTabDialog::~SwSvxNumBulletTabDialog()
{
}

void SwSvxNumBulletTabDialog::PageCreated(const OUString& rPageId, SfxTabPage& rPage)
{
    // set styles' names and metric
    OUString sNumCharFormat, sBulletCharFormat;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFormat );
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BULLET_LEVEL, sBulletCharFormat );

    if (rPageId == "singlenum")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        rPage.PageCreated(aSet);
    }
    else if (rPageId == "bullets")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        rPage.PageCreated(aSet);
    }
    else if (rPageId == "customize")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        // collect char styles
        m_xDummyCombo->clear();
        m_xDummyCombo->append_text(SwViewShell::GetShellRes()->aStrNone);
        SwDocShell* pDocShell = m_rWrtSh.GetView().GetDocShell();
        ::FillCharStyleListBox(*m_xDummyCombo,  pDocShell);

        std::vector<OUString> aList;
        aList.reserve(m_xDummyCombo->get_count());
        for (sal_Int32 j = 0; j < m_xDummyCombo->get_count(); j++)
            aList.push_back(m_xDummyCombo->get_text(j));

        aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;

        FieldUnit eMetric = ::GetDfltMetric(dynamic_cast< const SwWebDocShell *>( pDocShell ) !=  nullptr);
        aSet.Put ( SfxUInt16Item(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric) ) );
        rPage.PageCreated(aSet);
    }
    else if (rPageId == "position")
    {
        SwDocShell* pDocShell = m_rWrtSh.GetView().GetDocShell();
        FieldUnit eMetric = ::GetDfltMetric(dynamic_cast< const SwWebDocShell *>( pDocShell ) !=  nullptr);
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put ( SfxUInt16Item(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)) );
        rPage.PageCreated(aSet);
    }
}

short  SwSvxNumBulletTabDialog::Ok()
{
    short nRet = SfxTabDialogController::Ok();
    m_xExampleSet->ClearItem(SID_PARAM_NUM_PRESET);
    return nRet;
}

IMPL_LINK_NOARG(SwSvxNumBulletTabDialog, RemoveNumberingHdl, weld::Button&, void)
{
    m_xDialog->response(RET_USER);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
