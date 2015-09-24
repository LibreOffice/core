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

#include <hintids.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/imgmgr.hxx>
#include <svx/gallery.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/numitem.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <wview.hxx>
#include <uitool.hxx>
#include <wdocsh.hxx>
#include <uiitems.hxx>
#include <docstyle.hxx>
#include <charfmt.hxx>
#include <uinums.hxx>
#include <poolfmt.hxx>
#include <shellres.hxx>
#include <outline.hxx>
#include <num.hxx>
#include <viewopt.hxx>
#include <frmmgr.hxx>

#include <misc.hrc>
#include <frmui.hrc>
#include <globals.hrc>
#include <helpid.h>
#include <SwStyleNameMapper.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svl/stritem.hxx>
#include <svl/aeitem.hxx>
#include <svl/slstitm.hxx>

static bool bLastRelative = false;

//See cui/uiconfig/ui/numberingpositionpage.ui for effectively a duplicate
//dialog to this one, except with a different preview window impl.
//TODO, determine if SwNumPositionTabPage and SvxNumPositionTabPage can be
//merged
SwNumPositionTabPage::SwNumPositionTabPage(vcl::Window* pParent,
                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OutlinePositionPage",
        "modules/swriter/ui/outlinepositionpage.ui", &rSet)
    , pActNum(0)
    , pSaveNum(0)
    , pWrtSh(0)
    , pOutlineDlg(0)
    , nActNumLvl(0)
    , bModified(false)
    , bPreset(false)
    , bInInintControl(false)
    , bLabelAlignmentPosAndSpaceModeActive(false)
{
    get(m_pLevelLB, "levellb");
    m_pLevelLB->EnableMultiSelection(true);
    get(m_pPositionFrame, "numberingframe");
    get(m_pDistBorderFT, "indent");
    get(m_pDistBorderMF, "indentmf");
    get(m_pRelativeCB, "relative");
    get(m_pIndentFT, "numberingwidth");
    get(m_pIndentMF, "numberingwidthmf");
    get(m_pDistNumFT, "numdist");
    get(m_pDistNumMF, "numdistmf");
    get(m_pAlignFT, "numalign");
    get(m_pAlignLB, "numalignlb");

    get(m_pLabelFollowedByFT, "numfollowedby");
    get(m_pLabelFollowedByLB, "numfollowedbylb");
    get(m_pListtabFT, "at");
    get(m_pListtabMF, "atmf");
    get(m_pAlign2FT, "num2align");
    get(m_pAlign2LB, "num2alignlb");
    get(m_pAlignedAtFT, "alignedat");
    get(m_pAlignedAtMF, "alignedatmf");
    get(m_pIndentAtFT, "indentat");
    get(m_pIndentAtMF, "indentatmf");
    get(m_pStandardPB, "standard");

    get(m_pPreviewWIN, "preview");

    SetExchangeSupport();
    m_pPreviewWIN->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));

    m_pStandardPB->SetAccessibleRelationMemberOf(m_pPositionFrame->get_label_widget());

    m_pRelativeCB->Check();
    m_pAlignLB->SetSelectHdl(LINK(this, SwNumPositionTabPage, EditModifyHdl));
    m_pAlign2LB->SetSelectHdl(LINK(this, SwNumPositionTabPage, EditModifyHdl));
    for ( sal_Int32 i = 0; i < m_pAlignLB->GetEntryCount(); ++i )
    {
        m_pAlign2LB->InsertEntry( m_pAlignLB->GetEntry( i ) );
    }
    m_pAlign2LB->SetDropDownLineCount( m_pAlign2LB->GetEntryCount() );
    m_pAlign2FT->SetText( m_pAlignFT->GetText() );

    Link<> aLk = LINK(this, SwNumPositionTabPage, DistanceHdl);
    Link<Control&,void> aLk2 = LINK(this, SwNumPositionTabPage, DistanceLoseFocusHdl);
    m_pDistBorderMF->SetUpHdl(aLk);
    m_pDistNumMF->SetUpHdl(aLk);
    m_pIndentMF->SetUpHdl(aLk);
    m_pDistBorderMF->SetDownHdl(aLk);
    m_pDistNumMF->SetDownHdl(aLk);
    m_pIndentMF->SetDownHdl(aLk);
    m_pDistBorderMF->SetLoseFocusHdl(aLk2);
    m_pDistNumMF->SetLoseFocusHdl(aLk2);
    m_pIndentMF->SetLoseFocusHdl(aLk2);

    m_pLabelFollowedByLB->SetDropDownLineCount( m_pLabelFollowedByLB->GetEntryCount() );
    m_pLabelFollowedByLB->SetSelectHdl( LINK(this, SwNumPositionTabPage, LabelFollowedByHdl_Impl) );

    aLk = LINK(this, SwNumPositionTabPage, ListtabPosHdl_Impl);
    m_pListtabMF->SetUpHdl(aLk);
    m_pListtabMF->SetDownHdl(aLk);
    m_pListtabMF->SetLoseFocusHdl(aLk2);

    aLk = LINK(this, SwNumPositionTabPage, AlignAtHdl_Impl);
    m_pAlignedAtMF->SetUpHdl(aLk);
    m_pAlignedAtMF->SetDownHdl(aLk);
    m_pAlignedAtMF->SetLoseFocusHdl(aLk2);

    aLk = LINK(this, SwNumPositionTabPage, IndentAtHdl_Impl);
    m_pIndentAtMF->SetUpHdl(aLk);
    m_pIndentAtMF->SetDownHdl(aLk);
    m_pIndentAtMF->SetLoseFocusHdl(aLk2);

    m_pLevelLB->SetSelectHdl(LINK(this, SwNumPositionTabPage, LevelHdl));
    m_pRelativeCB->SetClickHdl(LINK(this, SwNumPositionTabPage, RelativeHdl));
    m_pStandardPB->SetClickHdl(LINK(this, SwNumPositionTabPage, StandardHdl));

    // insert levels
    for(sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        m_pLevelLB->InsertEntry(OUString::number(i));
    OUString sEntry("1 - ");
    sEntry += OUString::number(MAXLEVEL);
    m_pLevelLB->InsertEntry(sEntry);
    m_pLevelLB->SelectEntry(sEntry);

    m_pRelativeCB->Check(bLastRelative);
    m_pPreviewWIN->SetPositionMode();
}

SwNumPositionTabPage::~SwNumPositionTabPage()
{
    disposeOnce();
}

void SwNumPositionTabPage::dispose()
{
    delete pActNum;
    m_pLevelLB.clear();
    m_pPositionFrame.clear();
    m_pDistBorderFT.clear();
    m_pDistBorderMF.clear();
    m_pRelativeCB.clear();
    m_pIndentFT.clear();
    m_pIndentMF.clear();
    m_pDistNumFT.clear();
    m_pDistNumMF.clear();
    m_pAlignFT.clear();
    m_pAlignLB.clear();
    m_pLabelFollowedByFT.clear();
    m_pLabelFollowedByLB.clear();
    m_pListtabFT.clear();
    m_pListtabMF.clear();
    m_pAlign2FT.clear();
    m_pAlign2LB.clear();
    m_pAlignedAtFT.clear();
    m_pAlignedAtMF.clear();
    m_pIndentAtFT.clear();
    m_pIndentAtMF.clear();
    m_pStandardPB.clear();
    m_pPreviewWIN.clear();
    pOutlineDlg.clear();
    SfxTabPage::dispose();
}

void SwNumPositionTabPage::InitControls()
{
    bInInintControl = true;
    const bool bRelative = !bLabelAlignmentPosAndSpaceModeActive &&
                           m_pRelativeCB->IsEnabled() && m_pRelativeCB->IsChecked();
    const bool bSingleSelection = m_pLevelLB->GetSelectEntryCount() == 1 &&
                                  USHRT_MAX != nActNumLvl;

    m_pDistBorderMF->Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative || pOutlineDlg.get() != 0 ) );
    m_pDistBorderFT->Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative || pOutlineDlg.get() != 0 ) );

    bool bSetDistEmpty = false;
    bool bSameDistBorderNum = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameDist      = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndent    = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAdjust    = true;

    bool bSameLabelFollowedBy = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameListtab = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAlignAt = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndentAt = bLabelAlignmentPosAndSpaceModeActive;

    const SwNumFormat* aNumFormatArr[MAXLEVEL];
    sal_uInt16 nMask = 1;
    sal_uInt16 nLvl = USHRT_MAX;
    long nFirstBorderTextRelative = -1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        aNumFormatArr[i] = &pActNum->Get(i);
        if(nActNumLvl & nMask)
        {
            if(USHRT_MAX == nLvl)
            {
                nLvl = i;
            }

            if( i > nLvl)
            {
                bSameAdjust &= aNumFormatArr[i]->GetNumAdjust() == aNumFormatArr[nLvl]->GetNumAdjust();
                if ( !bLabelAlignmentPosAndSpaceModeActive )
                {
                    if(bRelative)
                    {
                        const long nBorderTextRelative =
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
        long nDistBorderNum;
        if(bRelative)
        {
            nDistBorderNum = (long)aNumFormatArr[nLvl]->GetAbsLSpace()+ aNumFormatArr[nLvl]->GetFirstLineOffset();
            if(nLvl)
                nDistBorderNum -= (long)aNumFormatArr[nLvl - 1]->GetAbsLSpace()+ aNumFormatArr[nLvl - 1]->GetFirstLineOffset();
        }
        else
        {
            nDistBorderNum = (long)aNumFormatArr[nLvl]->GetAbsLSpace()+ aNumFormatArr[nLvl]->GetFirstLineOffset();
        }
        m_pDistBorderMF->SetValue(m_pDistBorderMF->Normalize(nDistBorderNum),FUNIT_TWIP);
    }
    else
        bSetDistEmpty = true;

    if(bSameDist)
        m_pDistNumMF->SetValue(m_pDistNumMF->Normalize(aNumFormatArr[nLvl]->GetCharTextDistance()), FUNIT_TWIP);
    else
        m_pDistNumMF->SetText(aEmptyOUStr);
    if(bSameIndent)
        m_pIndentMF->SetValue(m_pIndentMF->Normalize(-aNumFormatArr[nLvl]->GetFirstLineOffset()), FUNIT_TWIP);
    else
        m_pIndentMF->SetText(aEmptyOUStr);

    if(bSameAdjust)
    {
        sal_Int32 nPos = 1; // centered
        if(aNumFormatArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFormatArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        m_pAlignLB->SelectEntryPos(nPos);
        m_pAlign2LB->SelectEntryPos( nPos );
    }
    else
    {
        m_pAlignLB->SetNoSelection();
        m_pAlign2LB->SetNoSelection();
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
        m_pLabelFollowedByLB->SelectEntryPos( nPos );
    }
    else
    {
        m_pLabelFollowedByLB->SetNoSelection();
    }

    if ( aNumFormatArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
    {
        m_pListtabFT->Enable( true );
        m_pListtabMF->Enable( true );
        if ( bSameListtab )
        {
            m_pListtabMF->SetValue(m_pListtabMF->Normalize(aNumFormatArr[nLvl]->GetListtabPos()),FUNIT_TWIP);
        }
        else
        {
            m_pListtabMF->SetText(aEmptyOUStr);
        }
    }
    else
    {
        m_pListtabFT->Enable( false );
        m_pListtabMF->Enable( false );
        m_pListtabMF->SetText(aEmptyOUStr);
    }

    if ( bSameAlignAt )
    {
        m_pAlignedAtMF->SetValue(
            m_pAlignedAtMF->Normalize( aNumFormatArr[nLvl]->GetIndentAt() +
                                    aNumFormatArr[nLvl]->GetFirstLineIndent()),
            FUNIT_TWIP );
    }
    else
    {
        m_pAlignedAtMF->SetText(aEmptyOUStr);
    }

    if ( bSameIndentAt )
    {
        m_pIndentAtMF->SetValue(
            m_pIndentAtMF->Normalize( aNumFormatArr[nLvl]->GetIndentAt()), FUNIT_TWIP );
    }
    else
    {
        m_pIndentAtMF->SetText(aEmptyOUStr);
    }

    if(bSetDistEmpty)
        m_pDistBorderMF->SetText(aEmptyOUStr);

    bInInintControl = false;
}

void SwNumPositionTabPage::ActivatePage(const SfxItemSet& )
{
    const SfxPoolItem* pItem;
    sal_uInt16 nTmpNumLvl =
        pOutlineDlg ? SwOutlineTabDialog::GetActNumLevel() : 0;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet && pExampleSet->GetItemState(FN_PARAM_NUM_PRESET, false, &pItem) != SfxItemState::UNKNOWN)
    {
        bPreset = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    }
    bModified = (!pActNum->GetNumFormat( 0 ) || bPreset);
    if(*pActNum != *pSaveNum ||
        nActNumLvl != nTmpNumLvl )
    {
        *pActNum = *pSaveNum;
        nActNumLvl = nTmpNumLvl;
        sal_uInt16 nMask = 1;
        m_pLevelLB->SetUpdateMode(false);
        m_pLevelLB->SetNoSelection();
        m_pLevelLB->SelectEntryPos( MAXLEVEL, nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
            {
                if(nActNumLvl & nMask)
                    m_pLevelLB->SelectEntryPos( i );
                nMask <<= 1 ;
            }
        m_pLevelLB->SetUpdateMode(true);

        InitPosAndSpaceMode();
        ShowControlsDependingOnPosAndSpaceMode();

        InitControls();
    }
    m_pRelativeCB->Enable(1 != nActNumLvl);
    m_pPreviewWIN->Invalidate();
}

SfxTabPage::sfxpg SwNumPositionTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    SwOutlineTabDialog::SetActNumLevel(nActNumLvl);
    if(_pSet)
        FillItemSet(_pSet);
    return LEAVE_PAGE;

}

bool SwNumPositionTabPage::FillItemSet( SfxItemSet* rSet )
{
    if(pOutlineDlg)
        *pOutlineDlg->GetNumRule() = *pActNum;
    else if(bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet->Put(SwUINumRuleItem( *pSaveNum ));
        rSet->Put(SfxBoolItem(FN_PARAM_NUM_PRESET, false));
    }
    return bModified;
}

void SwNumPositionTabPage::Reset( const SfxItemSet* rSet )
{
    const SfxPoolItem* pItem;
    if(pOutlineDlg)
    {
        pSaveNum = pOutlineDlg->GetNumRule();
        m_pLevelLB->EnableMultiSelection(false);
    }
    else if(SfxItemState::SET == rSet->GetItemState(FN_PARAM_ACT_NUMBER, false, &pItem))
        pSaveNum = const_cast<SwUINumRuleItem*>(static_cast<const SwUINumRuleItem*>(pItem))->GetNumRule();

    nActNumLvl = SwOutlineTabDialog::GetActNumLevel();
    sal_uInt16 nMask = 1;
    m_pLevelLB->SetUpdateMode(false);
    m_pLevelLB->SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        m_pLevelLB->SelectEntryPos( MAXLEVEL );
    }
    else
        for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        {
            if(nActNumLvl & nMask)
                m_pLevelLB->SelectEntryPos( i );
            nMask <<= 1;
        }
    m_pLevelLB->SetUpdateMode(true);

    if(!pActNum)
        pActNum = new  SwNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    m_pPreviewWIN->SetNumRule(pActNum);
    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();
    InitControls();
    bModified = false;
}

void SwNumPositionTabPage::InitPosAndSpaceMode()
{
    if ( pActNum == 0 )
    {
        OSL_FAIL( "<SwNumPositionTabPage::InitPosAndSpaceMode()> - misusage of method -> <pAktNum> has to be already set!" );
        return;
    }

    SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode =
                                            SvxNumberFormat::LABEL_ALIGNMENT;
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFormat( pActNum->Get(i) );
            ePosAndSpaceMode = aNumFormat.GetPositionAndSpaceMode();
            if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                break;
            }
        }
        nMask <<= 1;
    }

    bLabelAlignmentPosAndSpaceModeActive =
                    ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT;
}

void SwNumPositionTabPage::ShowControlsDependingOnPosAndSpaceMode()
{
    m_pDistBorderFT->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pDistBorderMF->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pRelativeCB->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pIndentFT->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pIndentMF->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pDistNumFT->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pDistNumMF->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pAlignFT->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pAlignLB->Show( !bLabelAlignmentPosAndSpaceModeActive );

    m_pLabelFollowedByFT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pLabelFollowedByLB->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pListtabFT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pListtabMF->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pAlign2FT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pAlign2LB->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pAlignedAtFT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pAlignedAtMF->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pIndentAtFT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pIndentAtMF->Show( bLabelAlignmentPosAndSpaceModeActive );
}

VclPtr<SfxTabPage> SwNumPositionTabPage::Create( vcl::Window* pParent,
                                                 const SfxItemSet* rAttrSet)
{
    return VclPtr<SwNumPositionTabPage>::Create(pParent, *rAttrSet);
}

void SwNumPositionTabPage::SetWrtShell(SwWrtShell* pSh)
{
    pWrtSh = pSh;

    const SwTwips nWidth = pWrtSh->GetAnyCurRect(RECT_FRM).Width();

    m_pDistBorderMF->SetMax(m_pDistBorderMF->Normalize( nWidth ), FUNIT_TWIP );
    m_pDistNumMF->SetMax(m_pDistNumMF->Normalize( nWidth ), FUNIT_TWIP);
    m_pIndentMF->SetMax(m_pIndentMF->Normalize( nWidth ), FUNIT_TWIP );
    m_pListtabMF->SetMax(m_pListtabMF->Normalize( nWidth ), FUNIT_TWIP );
    m_pAlignedAtMF->SetMax(m_pAlignedAtMF->Normalize( nWidth ), FUNIT_TWIP );
    m_pIndentAtMF->SetMax(m_pIndentAtMF->Normalize( nWidth ), FUNIT_TWIP );
    const SwTwips nLast2 = nWidth /2;
    m_pDistBorderMF->SetLast( m_pDistBorderMF->Normalize(   nLast2 ), FUNIT_TWIP );
    m_pDistNumMF->SetLast(m_pDistNumMF->Normalize( nLast2 ), FUNIT_TWIP);
    m_pIndentMF->SetLast(m_pIndentMF->Normalize( nLast2 ), FUNIT_TWIP );
    m_pListtabMF->SetLast(m_pListtabMF->Normalize( nLast2 ), FUNIT_TWIP );
    m_pAlignedAtMF->SetLast(m_pAlignedAtMF->Normalize( nLast2 ), FUNIT_TWIP );
    m_pIndentAtMF->SetLast(m_pIndentAtMF->Normalize( nLast2 ), FUNIT_TWIP );

    const SwRect& rPrtRect = pWrtSh->GetAnyCurRect(RECT_PAGE);
    m_pPreviewWIN->SetPageWidth(rPrtRect.Width());
    FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &pWrtSh->GetView()));
    if(eMetric == FUNIT_MM)
    {
        m_pDistBorderMF->SetDecimalDigits(1);
        m_pDistNumMF->SetDecimalDigits(1);
        m_pIndentMF->SetDecimalDigits(1);
        m_pListtabMF->SetDecimalDigits(1);
        m_pAlignedAtMF->SetDecimalDigits(1);
        m_pIndentAtMF->SetDecimalDigits(1);
    }
    m_pDistBorderMF->SetUnit( eMetric );
    m_pDistNumMF->SetUnit( eMetric );
    m_pIndentMF->SetUnit( eMetric );
    m_pListtabMF->SetUnit( eMetric );
    m_pAlignedAtMF->SetUnit( eMetric );
    m_pIndentAtMF->SetUnit( eMetric );
}

IMPL_LINK_NOARG(SwNumPositionTabPage, EditModifyHdl)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFormat aNumFormat(pActNum->Get(i));

            const sal_Int32 nPos = m_pAlignLB->IsVisible()
                                ? m_pAlignLB->GetSelectEntryPos()
                                : m_pAlign2LB->GetSelectEntryPos();
            SvxAdjust eAdjust = SVX_ADJUST_CENTER;
            if(nPos == 0)
                eAdjust = SVX_ADJUST_LEFT;
            else if(nPos == 2)
                eAdjust = SVX_ADJUST_RIGHT;
            aNumFormat.SetNumAdjust( eAdjust );
            pActNum->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SwNumPositionTabPage, LevelHdl, ListBox *, pBox )
{
    sal_uInt16 nSaveNumLvl = nActNumLvl;
    nActNumLvl = 0;
    if(pBox->IsEntryPosSelected( MAXLEVEL ) &&
            (pBox->GetSelectEntryCount() == 1 || nSaveNumLvl != 0xffff))
    {
        nActNumLvl = 0xFFFF;
        pBox->SetUpdateMode(false);
        for( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
            pBox->SelectEntryPos( i, false );
        pBox->SetUpdateMode(true);
    }
    else if(pBox->GetSelectEntryCount())
    {
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
        {
            if(pBox->IsEntryPosSelected( i ))
                nActNumLvl |= nMask;
            nMask <<= 1;
        }
        pBox->SelectEntryPos( MAXLEVEL, false );
    }
    else
    {
        nActNumLvl = nSaveNumLvl;
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
        {
            if(nActNumLvl & nMask)
            {
                pBox->SelectEntryPos(i);
                break;
            }
            nMask <<=1;
        }
    }
    m_pRelativeCB->Enable(1 != nActNumLvl);
    SetModified();
    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();
    InitControls();
    return 0;
}

IMPL_LINK_TYPED( SwNumPositionTabPage, DistanceLoseFocusHdl, Control&, rControl, void )
{
    DistanceHdl(static_cast<MetricField*>(&rControl));
}
IMPL_LINK( SwNumPositionTabPage, DistanceHdl, MetricField *, pField )
{
    if(bInInintControl)
        return 0;
    long nValue = static_cast< long >(pField->Denormalize(pField->GetValue(FUNIT_TWIP)));
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFormat aNumFormat( pActNum->Get( i ) );
            if(pField == m_pDistBorderMF)
            {

                if(m_pRelativeCB->IsChecked() && m_pRelativeCB->IsEnabled())
                {
                    if(0 == i)
                    {
                        long nTmp = aNumFormat.GetFirstLineOffset();
                        aNumFormat.SetAbsLSpace( nValue - nTmp );
                    }
                    else
                    {
                        long nTmp = pActNum->Get( i - 1 ).GetAbsLSpace() +
                                    pActNum->Get( i - 1 ).GetFirstLineOffset() -
                                    pActNum->Get( i ).GetFirstLineOffset();

                        aNumFormat.SetAbsLSpace( nValue + nTmp );
                    }
                }
                else
                {
                    aNumFormat.SetAbsLSpace( (short)nValue - aNumFormat.GetFirstLineOffset());
                }
            }
            else if (pField == m_pDistNumMF)
            {
                aNumFormat.SetCharTextDistance( nValue );
            }
            else if (pField == m_pIndentMF)
            {
                // now AbsLSpace also has to be modified by FirstLineOffset
                long nDiff = nValue + aNumFormat.GetFirstLineOffset();
                long nAbsLSpace = aNumFormat.GetAbsLSpace();
                aNumFormat.SetAbsLSpace( nAbsLSpace + nDiff );
                aNumFormat.SetFirstLineOffset( -nValue );
            }

            pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    SetModified();
    if(!m_pDistBorderMF->IsEnabled())
        m_pDistBorderMF->SetText(aEmptyOUStr);

    return 0;
}

IMPL_LINK_TYPED( SwNumPositionTabPage, RelativeHdl, Button *, pBox, void )
{
    bool bOn = static_cast<CheckBox*>(pBox)->IsChecked();
    bool bSingleSelection = m_pLevelLB->GetSelectEntryCount() == 1 && USHRT_MAX != nActNumLvl;
    bool bSetValue = false;
    long nValue = 0;
    if(bOn || bSingleSelection)
    {
        sal_uInt16 nMask = 1;
        bool bFirst = true;
        bSetValue = true;
        for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        {
            if(nActNumLvl & nMask)
            {
                const SwNumFormat &rNumFormat = pActNum->Get(i);
                if(bFirst)
                {
                    nValue = rNumFormat.GetAbsLSpace();
                    if(bOn && i)
                        nValue -= pActNum->Get(i - 1).GetAbsLSpace();
                }
                else
                    bSetValue = nValue == rNumFormat.GetAbsLSpace() - pActNum->Get(i - 1).GetAbsLSpace();
                bFirst = false;
            }
            nMask <<= 1;
        }

    }
    if(bSetValue)
        m_pDistBorderMF->SetValue(m_pDistBorderMF->Normalize(nValue), FUNIT_TWIP);
    else
        m_pDistBorderMF->SetText(aEmptyOUStr);
    m_pDistBorderMF->Enable(bOn || bSingleSelection || pOutlineDlg);
    bLastRelative = bOn;
}

IMPL_LINK_NOARG(SwNumPositionTabPage, LabelFollowedByHdl_Impl)
{
    // determine value to be set at the chosen list levels
    SvxNumberFormat::LabelFollowedBy eLabelFollowedBy = SvxNumberFormat::LISTTAB;
    {
        const sal_Int32 nPos = m_pLabelFollowedByLB->GetSelectEntryPos();
        if ( nPos == 1 )
        {
            eLabelFollowedBy = SvxNumberFormat::SPACE;
        }
        else if ( nPos == 2 )
        {
            eLabelFollowedBy = SvxNumberFormat::NOTHING;
        }
    }

    // set value at the chosen list levels
    bool bSameListtabPos = true;
    sal_uInt16 nFirstLvl = USHRT_MAX;
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SwNumFormat aNumFormat( pActNum->Get(i) );
            aNumFormat.SetLabelFollowedBy( eLabelFollowedBy );
            pActNum->Set( i, aNumFormat );

            if ( nFirstLvl == USHRT_MAX )
            {
                nFirstLvl = i;
            }
            else
            {
                bSameListtabPos &= aNumFormat.GetListtabPos() ==
                        pActNum->Get( nFirstLvl ).GetListtabPos();
            }
        }
        nMask <<= 1;
    }

    // enable/disable metric field for list tab stop position depending on
    // selected item following the list label.
    m_pListtabFT->Enable( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    m_pListtabMF->Enable( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    if ( bSameListtabPos && eLabelFollowedBy == SvxNumberFormat::LISTTAB )
    {
        m_pListtabMF->SetValue(
            m_pListtabMF->Normalize( pActNum->Get( nFirstLvl ).GetListtabPos() ),
            FUNIT_TWIP );
    }
    else
    {
        m_pListtabMF->SetText( OUString() );
    }

    SetModified();

    return 0;
}

IMPL_LINK( SwNumPositionTabPage, ListtabPosHdl_Impl, MetricField*, pField )
{
    // determine value to be set at the chosen list levels
    const long nValue = static_cast< long >(pField->Denormalize(pField->GetValue(FUNIT_TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SwNumFormat aNumFormat( pActNum->Get(i) );
            aNumFormat.SetListtabPos( nValue );
            pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK( SwNumPositionTabPage, AlignAtHdl_Impl, MetricField*, pField )
{
    // determine value to be set at the chosen list levels
    const long nValue = static_cast< long >(pField->Denormalize(pField->GetValue(FUNIT_TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SwNumFormat aNumFormat( pActNum->Get(i) );
            const long nFirstLineIndent = nValue - aNumFormat.GetIndentAt();
            aNumFormat.SetFirstLineIndent( nFirstLineIndent );
            pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK( SwNumPositionTabPage, IndentAtHdl_Impl, MetricField*, pField )
{
    // determine value to be set at the chosen list levels
    const long nValue = static_cast< long >(pField->Denormalize(pField->GetValue(FUNIT_TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SwNumFormat aNumFormat( pActNum->Get(i) );
            const long nAlignedAt = aNumFormat.GetIndentAt() +
                                    aNumFormat.GetFirstLineIndent();
            aNumFormat.SetIndentAt( nValue );
            const long nNewFirstLineIndent = nAlignedAt - nValue;
            aNumFormat.SetFirstLineIndent( nNewFirstLineIndent );
            pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK_NOARG_TYPED(SwNumPositionTabPage, StandardHdl, Button*, void)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFormat aNumFormat( pActNum->Get( i ) );
            SwNumRule aTmpNumRule( pWrtSh->GetUniqueNumRuleName(),
                                   aNumFormat.GetPositionAndSpaceMode(),
                                   pOutlineDlg ? OUTLINE_RULE : NUM_RULE );
            SwNumFormat aTempFormat(aTmpNumRule.Get( i ));
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
            pActNum->Set( i, aNumFormat );
        }
        nMask <<= 1;
    }

    InitControls();
    SetModified();
}

#ifdef DBG_UTIL
void SwNumPositionTabPage::SetModified(bool bRepaint)
{
    bModified = true;
    if(bRepaint)
    {
        m_pPreviewWIN->SetLevel(nActNumLvl);
        m_pPreviewWIN->Invalidate();
    }
}
#endif

SwSvxNumBulletTabDialog::SwSvxNumBulletTabDialog(vcl::Window* pParent,
                    const SfxItemSet* pSwItemSet, SwWrtShell & rSh)
    : SfxTabDialog(pParent, "BulletsAndNumberingDialog",
        "modules/swriter/ui/bulletsandnumbering.ui",
        pSwItemSet)
    , rWrtSh(rSh)
{
    GetUserButton()->SetClickHdl(LINK(this, SwSvxNumBulletTabDialog, RemoveNumberingHdl));
    GetUserButton()->Enable(rWrtSh.GetNumRuleAtCurrCrsrPos() != NULL);
    m_nSingleNumPageId = AddTabPage("singlenum", RID_SVXPAGE_PICK_SINGLE_NUM );
    m_nBulletPageId = AddTabPage("bullets", RID_SVXPAGE_PICK_BULLET );
    AddTabPage("outlinenum", RID_SVXPAGE_PICK_NUM );
    AddTabPage("graphics", RID_SVXPAGE_PICK_BMP );
    m_nOptionsPageId = AddTabPage("options", RID_SVXPAGE_NUM_OPTIONS );
    m_nPositionPageId = AddTabPage("position", RID_SVXPAGE_NUM_POSITION );
}

SwSvxNumBulletTabDialog::~SwSvxNumBulletTabDialog()
{
}

void SwSvxNumBulletTabDialog::PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage)
{
    // set styles' names and metric
    OUString sNumCharFormat, sBulletCharFormat;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFormat );
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BUL_LEVEL, sBulletCharFormat );

    if (nPageId == m_nSingleNumPageId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        rPage.PageCreated(aSet);
    }
    else if (nPageId == m_nBulletPageId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        rPage.PageCreated(aSet);
    }
    else if (nPageId == m_nOptionsPageId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        // collect char styles
        ScopedVclPtrInstance< ListBox > rCharFormatLB(this);
        rCharFormatLB->Clear();
        rCharFormatLB->InsertEntry( SwViewShell::GetShellRes()->aStrNone );
        SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
        ::FillCharStyleListBox(*rCharFormatLB.get(),  pDocShell);

        std::vector<OUString> aList;
        for(sal_Int32 j = 0; j < rCharFormatLB->GetEntryCount(); j++)
             aList.push_back( rCharFormatLB->GetEntry(j) );

        aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;

        FieldUnit eMetric = ::GetDfltMetric(pDocShell->ISA(SwWebDocShell));
        aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric) ) );
        rPage.PageCreated(aSet);
    }
    else if (nPageId == m_nPositionPageId)
    {
        SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
        FieldUnit eMetric = ::GetDfltMetric(pDocShell->ISA(SwWebDocShell));
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)) );
        rPage.PageCreated(aSet);
    }
}

short  SwSvxNumBulletTabDialog::Ok()
{
    short nRet = SfxTabDialog::Ok();
    pExampleSet->ClearItem(SID_PARAM_NUM_PRESET);
    return nRet;
}

IMPL_LINK_NOARG_TYPED(SwSvxNumBulletTabDialog, RemoveNumberingHdl, Button*, void)
{
    EndDialog(RET_USER);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
