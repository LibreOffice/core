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
#include <swvset.hxx>
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

static sal_Bool bLastRelative = sal_False;

//See cui/uiconfig/ui/numberingpositionpage.ui for effectively a duplicate
//dialog to this one, except with a different preview window impl.
//TODO, determine if SwNumPositionTabPage and SvxNumPositionTabPage can be
//merged
SwNumPositionTabPage::SwNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OutlinePositionPage",
        "modules/swriter/ui/outlinepositionpage.ui", rSet)
    , pActNum(0)
    , pSaveNum(0)
    , pWrtSh(0)
    , pOutlineDlg(0)
    , bPreset( sal_False )
    , bInInintControl(sal_False)
    , bLabelAlignmentPosAndSpaceModeActive( false )
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
    for ( sal_uInt16 i = 0; i < m_pAlignLB->GetEntryCount(); ++i )
    {
        m_pAlign2LB->InsertEntry( m_pAlignLB->GetEntry( i ) );
    }
    m_pAlign2LB->SetDropDownLineCount( m_pAlign2LB->GetEntryCount() );
    m_pAlign2FT->SetText( m_pAlignFT->GetText() );

    Link aLk = LINK(this, SwNumPositionTabPage, DistanceHdl);
    m_pDistBorderMF->SetUpHdl(aLk);
    m_pDistNumMF->SetUpHdl(aLk);
    m_pIndentMF->SetUpHdl(aLk);
    m_pDistBorderMF->SetDownHdl(aLk);
    m_pDistNumMF->SetDownHdl(aLk);
    m_pIndentMF->SetDownHdl(aLk);
    m_pDistBorderMF->SetLoseFocusHdl(aLk);
    m_pDistNumMF->SetLoseFocusHdl(aLk);
    m_pIndentMF->SetLoseFocusHdl(aLk);

    m_pLabelFollowedByLB->SetDropDownLineCount( m_pLabelFollowedByLB->GetEntryCount() );
    m_pLabelFollowedByLB->SetSelectHdl( LINK(this, SwNumPositionTabPage, LabelFollowedByHdl_Impl) );

    aLk = LINK(this, SwNumPositionTabPage, ListtabPosHdl_Impl);
    m_pListtabMF->SetUpHdl(aLk);
    m_pListtabMF->SetDownHdl(aLk);
    m_pListtabMF->SetLoseFocusHdl(aLk);

    aLk = LINK(this, SwNumPositionTabPage, AlignAtHdl_Impl);
    m_pAlignedAtMF->SetUpHdl(aLk);
    m_pAlignedAtMF->SetDownHdl(aLk);
    m_pAlignedAtMF->SetLoseFocusHdl(aLk);

    aLk = LINK(this, SwNumPositionTabPage, IndentAtHdl_Impl);
    m_pIndentAtMF->SetUpHdl(aLk);
    m_pIndentAtMF->SetDownHdl(aLk);
    m_pIndentAtMF->SetLoseFocusHdl(aLk);

    m_pLevelLB->SetSelectHdl(LINK(this, SwNumPositionTabPage, LevelHdl));
    m_pRelativeCB->SetClickHdl(LINK(this, SwNumPositionTabPage, RelativeHdl));
    m_pStandardPB->SetClickHdl(LINK(this, SwNumPositionTabPage, StandardHdl));

    // insert levels
    for(sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        m_pLevelLB->InsertEntry(OUString::number(i));
    String sEntry(OUString("1 - "));
    sEntry += OUString::number(MAXLEVEL);
    m_pLevelLB->InsertEntry(sEntry);
    m_pLevelLB->SelectEntry(sEntry);

    m_pRelativeCB->Check(bLastRelative);
    m_pPreviewWIN->SetPositionMode();
}

SwNumPositionTabPage::~SwNumPositionTabPage()
{
    delete pActNum;
}

void SwNumPositionTabPage::InitControls()
{
    bInInintControl = sal_True;
    const bool bRelative = !bLabelAlignmentPosAndSpaceModeActive &&
                           m_pRelativeCB->IsEnabled() && m_pRelativeCB->IsChecked();
    const bool bSingleSelection = m_pLevelLB->GetSelectEntryCount() == 1 &&
                                  USHRT_MAX != nActNumLvl;

    m_pDistBorderMF->Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative || pOutlineDlg != 0 ) );
    m_pDistBorderFT->Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative || pOutlineDlg != 0 ) );

    bool bSetDistEmpty = false;
    bool bSameDistBorderNum = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameDist      = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndent    = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAdjust    = true;

    bool bSameLabelFollowedBy = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameListtab = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAlignAt = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndentAt = bLabelAlignmentPosAndSpaceModeActive;

    const SwNumFmt* aNumFmtArr[MAXLEVEL];
    sal_uInt16 nMask = 1;
    sal_uInt16 nLvl = USHRT_MAX;
    long nFirstBorderTextRelative = -1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        aNumFmtArr[i] = &pActNum->Get(i);
        if(nActNumLvl & nMask)
        {
            if(USHRT_MAX == nLvl)
            {
                nLvl = i;
            }

            if( i > nLvl)
            {
                bSameAdjust &= aNumFmtArr[i]->GetNumAdjust() == aNumFmtArr[nLvl]->GetNumAdjust();
                if ( !bLabelAlignmentPosAndSpaceModeActive )
                {
                    if(bRelative)
                    {
                        if(nFirstBorderTextRelative == -1)
                            nFirstBorderTextRelative =
                            (aNumFmtArr[i]->GetAbsLSpace() + aNumFmtArr[i]->GetFirstLineOffset() -
                            aNumFmtArr[i - 1]->GetAbsLSpace() + aNumFmtArr[i - 1]->GetFirstLineOffset());
                        else
                            bSameDistBorderNum &= nFirstBorderTextRelative ==
                            (aNumFmtArr[i]->GetAbsLSpace() + aNumFmtArr[i]->GetFirstLineOffset() -
                            aNumFmtArr[i - 1]->GetAbsLSpace() + aNumFmtArr[i - 1]->GetFirstLineOffset());

                    }
                    else
                    {
                        bSameDistBorderNum &=
                        aNumFmtArr[i]->GetAbsLSpace() - aNumFmtArr[i]->GetFirstLineOffset() ==
                        aNumFmtArr[i - 1]->GetAbsLSpace() - aNumFmtArr[i - 1]->GetFirstLineOffset();
                    }

                    bSameDist       &= aNumFmtArr[i]->GetCharTextDistance() == aNumFmtArr[nLvl]->GetCharTextDistance();
                    bSameIndent     &= aNumFmtArr[i]->GetFirstLineOffset() == aNumFmtArr[nLvl]->GetFirstLineOffset();
                }
                else
                {
                    bSameLabelFollowedBy &=
                        aNumFmtArr[i]->GetLabelFollowedBy() == aNumFmtArr[nLvl]->GetLabelFollowedBy();
                    bSameListtab &=
                        aNumFmtArr[i]->GetListtabPos() == aNumFmtArr[nLvl]->GetListtabPos();
                    bSameAlignAt &=
                        ( ( aNumFmtArr[i]->GetIndentAt() + aNumFmtArr[i]->GetFirstLineIndent() )
                            == ( aNumFmtArr[nLvl]->GetIndentAt() + aNumFmtArr[nLvl]->GetFirstLineIndent() ) );
                    bSameIndentAt &=
                        aNumFmtArr[i]->GetIndentAt() == aNumFmtArr[nLvl]->GetIndentAt();
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
            nDistBorderNum = (long)aNumFmtArr[nLvl]->GetAbsLSpace()+ aNumFmtArr[nLvl]->GetFirstLineOffset();
            if(nLvl)
                nDistBorderNum -= (long)aNumFmtArr[nLvl - 1]->GetAbsLSpace()+ aNumFmtArr[nLvl - 1]->GetFirstLineOffset();
        }
        else
        {
            nDistBorderNum = (long)aNumFmtArr[nLvl]->GetAbsLSpace()+ aNumFmtArr[nLvl]->GetFirstLineOffset();
        }
        m_pDistBorderMF->SetValue(m_pDistBorderMF->Normalize(nDistBorderNum),FUNIT_TWIP);
    }
    else
        bSetDistEmpty = sal_True;

    if(bSameDist)
        m_pDistNumMF->SetValue(m_pDistNumMF->Normalize(aNumFmtArr[nLvl]->GetCharTextDistance()), FUNIT_TWIP);
    else
        m_pDistNumMF->SetText(aEmptyStr);
    if(bSameIndent)
        m_pIndentMF->SetValue(m_pIndentMF->Normalize(-aNumFmtArr[nLvl]->GetFirstLineOffset()), FUNIT_TWIP);
    else
        m_pIndentMF->SetText(aEmptyStr);

    if(bSameAdjust)
    {
        sal_uInt16 nPos = 1; // centered
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
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
        sal_uInt16 nPos = 0; // LISTTAB
        if ( aNumFmtArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::SPACE )
        {
            nPos = 1;
        }
        else if ( aNumFmtArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::NOTHING )
        {
            nPos = 2;
        }
        m_pLabelFollowedByLB->SelectEntryPos( nPos );
    }
    else
    {
        m_pLabelFollowedByLB->SetNoSelection();
    }

    if ( aNumFmtArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
    {
        m_pListtabFT->Enable( true );
        m_pListtabMF->Enable( true );
        if ( bSameListtab )
        {
            m_pListtabMF->SetValue(m_pListtabMF->Normalize(aNumFmtArr[nLvl]->GetListtabPos()),FUNIT_TWIP);
        }
        else
        {
            m_pListtabMF->SetText(aEmptyStr);
        }
    }
    else
    {
        m_pListtabFT->Enable( false );
        m_pListtabMF->Enable( false );
        m_pListtabMF->SetText(aEmptyStr);
    }

    if ( bSameAlignAt )
    {
        m_pAlignedAtMF->SetValue(
            m_pAlignedAtMF->Normalize( aNumFmtArr[nLvl]->GetIndentAt() +
                                    aNumFmtArr[nLvl]->GetFirstLineIndent()),
            FUNIT_TWIP );
    }
    else
    {
        m_pAlignedAtMF->SetText(aEmptyStr);
    }

    if ( bSameIndentAt )
    {
        m_pIndentAtMF->SetValue(
            m_pIndentAtMF->Normalize( aNumFmtArr[nLvl]->GetIndentAt()), FUNIT_TWIP );
    }
    else
    {
        m_pIndentAtMF->SetText(aEmptyStr);
    }

    if(sal_True == bSetDistEmpty)
        m_pDistBorderMF->SetText(aEmptyStr);

    bInInintControl = sal_False;
}

void SwNumPositionTabPage::ActivatePage(const SfxItemSet& )
{
    const SfxPoolItem* pItem;
    sal_uInt16 nTmpNumLvl =
        pOutlineDlg ? pOutlineDlg->GetActNumLevel() : 0;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet && pExampleSet->GetItemState(FN_PARAM_NUM_PRESET, sal_False, &pItem))
    {
        bPreset = ((const SfxBoolItem*)pItem)->GetValue();
    }
    bModified = (!pActNum->GetNumFmt( 0 ) || bPreset);
    if(*pActNum != *pSaveNum ||
        nActNumLvl != nTmpNumLvl )
    {
        *pActNum = *pSaveNum;
        nActNumLvl = nTmpNumLvl;
        sal_uInt16 nMask = 1;
        m_pLevelLB->SetUpdateMode(sal_False);
        m_pLevelLB->SetNoSelection();
        m_pLevelLB->SelectEntryPos( MAXLEVEL, nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
            {
                if(nActNumLvl & nMask)
                    m_pLevelLB->SelectEntryPos( i, sal_True);
                nMask <<= 1 ;
            }
        m_pLevelLB->SetUpdateMode(sal_True);

        InitPosAndSpaceMode();
        ShowControlsDependingOnPosAndSpaceMode();

        InitControls();
    }
    m_pRelativeCB->Enable(1 != nActNumLvl);
    m_pPreviewWIN->Invalidate();
}

int  SwNumPositionTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    SwOutlineTabDialog::SetActNumLevel(nActNumLvl);
    if(_pSet)
        FillItemSet(*_pSet);
    return sal_True;

}

sal_Bool SwNumPositionTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(pOutlineDlg)
        *pOutlineDlg->GetNumRule() = *pActNum;
    else if(bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SwUINumRuleItem( *pSaveNum ));
        rSet.Put(SfxBoolItem(FN_PARAM_NUM_PRESET, sal_False));
    }
    return bModified;
}

void SwNumPositionTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    if(pOutlineDlg)
    {
        pSaveNum = pOutlineDlg->GetNumRule();
        m_pLevelLB->EnableMultiSelection(sal_False);
    }
    else if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_ACT_NUMBER, sal_False, &pItem))
        pSaveNum = ((SwUINumRuleItem*)pItem)->GetNumRule();

    nActNumLvl = SwOutlineTabDialog::GetActNumLevel();
    sal_uInt16 nMask = 1;
    m_pLevelLB->SetUpdateMode(sal_False);
    m_pLevelLB->SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        m_pLevelLB->SelectEntryPos( MAXLEVEL, sal_True);
    }
    else
        for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        {
            if(nActNumLvl & nMask)
                m_pLevelLB->SelectEntryPos( i, sal_True);
            nMask <<= 1;
        }
    m_pLevelLB->SetUpdateMode(sal_True);

    if(!pActNum)
        pActNum = new  SwNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    m_pPreviewWIN->SetNumRule(pActNum);
    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();
    InitControls();
    bModified = sal_False;
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
            SvxNumberFormat aNumFmt( pActNum->Get(i) );
            ePosAndSpaceMode = aNumFmt.GetPositionAndSpaceMode();
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

SfxTabPage* SwNumPositionTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwNumPositionTabPage(pParent, rAttrSet);
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
            SwNumFmt aNumFmt(pActNum->Get(i));

            const sal_uInt16 nPos = m_pAlignLB->IsVisible()
                                ? m_pAlignLB->GetSelectEntryPos()
                                : m_pAlign2LB->GetSelectEntryPos();
            SvxAdjust eAdjust = SVX_ADJUST_CENTER;
            if(nPos == 0)
                eAdjust = SVX_ADJUST_LEFT;
            else if(nPos == 2)
                eAdjust = SVX_ADJUST_RIGHT;
            aNumFmt.SetNumAdjust( eAdjust );
            pActNum->Set(i, aNumFmt);
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
        pBox->SetUpdateMode(sal_False);
        for( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
            pBox->SelectEntryPos( i, sal_False );
        pBox->SetUpdateMode(sal_True);
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
        pBox->SelectEntryPos( MAXLEVEL, sal_False );
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

IMPL_LINK( SwNumPositionTabPage, DistanceHdl, MetricField *, pFld )
{
    if(bInInintControl)
        return 0;
    long nValue = static_cast< long >(pFld->Denormalize(pFld->GetValue(FUNIT_TWIP)));
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFmt aNumFmt( pActNum->Get( i ) );
            if(pFld == m_pDistBorderMF)
            {

                if(m_pRelativeCB->IsChecked() && m_pRelativeCB->IsEnabled())
                {
                    if(0 == i)
                    {
                        long nTmp = aNumFmt.GetFirstLineOffset();
                        aNumFmt.SetAbsLSpace( sal_uInt16(nValue - nTmp));
                    }
                    else
                    {
                        long nTmp = pActNum->Get( i - 1 ).GetAbsLSpace() +
                                    pActNum->Get( i - 1 ).GetFirstLineOffset() -
                                    pActNum->Get( i ).GetFirstLineOffset();

                        aNumFmt.SetAbsLSpace( sal_uInt16(nValue + nTmp));
                    }
                }
                else
                {
                    aNumFmt.SetAbsLSpace( (short)nValue - aNumFmt.GetFirstLineOffset());
                }
            }
            else if (pFld == m_pDistNumMF)
            {
                aNumFmt.SetCharTextDistance( (short)nValue );
            }
            else if (pFld == m_pIndentMF)
            {
                // now AbsLSpace also has to be modified by FirstLineOffset
                long nDiff = nValue + aNumFmt.GetFirstLineOffset();
                long nAbsLSpace = aNumFmt.GetAbsLSpace();
                aNumFmt.SetAbsLSpace(sal_uInt16(nAbsLSpace + nDiff));
                aNumFmt.SetFirstLineOffset( -(short)nValue );
            }

            pActNum->Set( i, aNumFmt );
        }
        nMask <<= 1;
    }

    SetModified();
    if(!m_pDistBorderMF->IsEnabled())
        m_pDistBorderMF->SetText(aEmptyStr);

    return 0;
}

IMPL_LINK( SwNumPositionTabPage, RelativeHdl, CheckBox *, pBox )
{
    sal_Bool bOn = pBox->IsChecked();
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
                const SwNumFmt &rNumFmt = pActNum->Get(i);
                if(bFirst)
                {
                    nValue = rNumFmt.GetAbsLSpace();
                    if(bOn && i)
                        nValue -= pActNum->Get(i - 1).GetAbsLSpace();
                }
                else
                    bSetValue = nValue == rNumFmt.GetAbsLSpace() - pActNum->Get(i - 1).GetAbsLSpace();
                bFirst = false;
            }
            nMask <<= 1;
        }

    }
    if(bSetValue)
        m_pDistBorderMF->SetValue(m_pDistBorderMF->Normalize(nValue), FUNIT_TWIP);
    else
        m_pDistBorderMF->SetText(aEmptyStr);
    m_pDistBorderMF->Enable(bOn || bSingleSelection||0 != pOutlineDlg);
    bLastRelative = bOn;
    return 0;
}

IMPL_LINK_NOARG(SwNumPositionTabPage, LabelFollowedByHdl_Impl)
{
    // determine value to be set at the chosen list levels
    SvxNumberFormat::LabelFollowedBy eLabelFollowedBy = SvxNumberFormat::LISTTAB;
    {
        const sal_uInt16 nPos = m_pLabelFollowedByLB->GetSelectEntryPos();
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
            SwNumFmt aNumFmt( pActNum->Get(i) );
            aNumFmt.SetLabelFollowedBy( eLabelFollowedBy );
            pActNum->Set( i, aNumFmt );

            if ( nFirstLvl == USHRT_MAX )
            {
                nFirstLvl = i;
            }
            else
            {
                bSameListtabPos &= aNumFmt.GetListtabPos() ==
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
        m_pListtabMF->SetText( String() );
    }

    SetModified();

    return 0;
}

IMPL_LINK( SwNumPositionTabPage, ListtabPosHdl_Impl, MetricField*, pFld )
{
    // determine value to be set at the chosen list levels
    const long nValue = static_cast< long >(pFld->Denormalize(pFld->GetValue(FUNIT_TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SwNumFmt aNumFmt( pActNum->Get(i) );
            aNumFmt.SetListtabPos( nValue );
            pActNum->Set( i, aNumFmt );
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK( SwNumPositionTabPage, AlignAtHdl_Impl, MetricField*, pFld )
{
    // determine value to be set at the chosen list levels
    const long nValue = static_cast< long >(pFld->Denormalize(pFld->GetValue(FUNIT_TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SwNumFmt aNumFmt( pActNum->Get(i) );
            const long nFirstLineIndent = nValue - aNumFmt.GetIndentAt();
            aNumFmt.SetFirstLineIndent( nFirstLineIndent );
            pActNum->Set( i, aNumFmt );
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK( SwNumPositionTabPage, IndentAtHdl_Impl, MetricField*, pFld )
{
    // determine value to be set at the chosen list levels
    const long nValue = static_cast< long >(pFld->Denormalize(pFld->GetValue(FUNIT_TWIP)));

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SwNumFmt aNumFmt( pActNum->Get(i) );
            const long nAlignedAt = aNumFmt.GetIndentAt() +
                                    aNumFmt.GetFirstLineIndent();
            aNumFmt.SetIndentAt( nValue );
            const long nNewFirstLineIndent = nAlignedAt - nValue;
            aNumFmt.SetFirstLineIndent( nNewFirstLineIndent );
            pActNum->Set( i, aNumFmt );
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK_NOARG(SwNumPositionTabPage, StandardHdl)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFmt aNumFmt( pActNum->Get( i ) );
            SwNumRule aTmpNumRule( pWrtSh->GetUniqueNumRuleName(),
                                   aNumFmt.GetPositionAndSpaceMode(),
                                   pOutlineDlg ? OUTLINE_RULE : NUM_RULE );
            SwNumFmt aTempFmt(aTmpNumRule.Get( i ));
            aNumFmt.SetPositionAndSpaceMode( aTempFmt.GetPositionAndSpaceMode() );
            if ( aTempFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aNumFmt.SetAbsLSpace( aTempFmt.GetAbsLSpace());
                aNumFmt.SetCharTextDistance( aTempFmt.GetCharTextDistance() );
                aNumFmt.SetFirstLineOffset( aTempFmt.GetFirstLineOffset() );
            }
            else if ( aTempFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aNumFmt.SetNumAdjust( aTempFmt.GetNumAdjust() );
                aNumFmt.SetLabelFollowedBy( aTempFmt.GetLabelFollowedBy() );
                aNumFmt.SetListtabPos( aTempFmt.GetListtabPos() );
                aNumFmt.SetFirstLineIndent( aTempFmt.GetFirstLineIndent() );
                aNumFmt.SetIndentAt( aTempFmt.GetIndentAt() );
            }
            pActNum->Set( i, aNumFmt );
        }
        nMask <<= 1;
    }

    InitControls();
    SetModified();
    return 0;
}

#ifdef DBG_UTIL
void SwNumPositionTabPage::SetModified(sal_Bool bRepaint)
{
    bModified = sal_True;
    if(bRepaint)
    {
        m_pPreviewWIN->SetLevel(nActNumLvl);
        m_pPreviewWIN->Invalidate();
    }
}
#endif

SwSvxNumBulletTabDialog::SwSvxNumBulletTabDialog(Window* pParent,
                    const SfxItemSet* pSwItemSet, SwWrtShell & rSh)
    : SfxTabDialog(pParent, "BulletsAndNumberingDialog",
        "modules/swriter/ui/bulletsandnumbering.ui",
        pSwItemSet, sal_False)
    , rWrtSh(rSh)
{
    GetUserButton()->SetClickHdl(LINK(this, SwSvxNumBulletTabDialog, RemoveNumberingHdl));
    GetUserButton()->Enable(rWrtSh.GetCurNumRule() != NULL);
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
    OUString sNumCharFmt, sBulletCharFmt;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFmt );
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BUL_LEVEL, sBulletCharFmt );

    if (nPageId == m_nSingleNumPageId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
        rPage.PageCreated(aSet);
    }
    else if (nPageId == m_nBulletPageId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
        rPage.PageCreated(aSet);
    }
    else if (nPageId == m_nOptionsPageId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
        // collect char styles
        ListBox rCharFmtLB(this);
        rCharFmtLB.Clear();
        rCharFmtLB.InsertEntry( ViewShell::GetShellRes()->aStrNone );
        SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
        ::FillCharStyleListBox(rCharFmtLB,  pDocShell);

        std::vector<OUString> aList;
        for(sal_uInt16 j = 0; j < rCharFmtLB.GetEntryCount(); j++)
             aList.push_back( rCharFmtLB.GetEntry(j) );

        aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;

        FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
        aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric) ) );
        rPage.PageCreated(aSet);
    }
    else if (nPageId == m_nPositionPageId)
    {
        SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
        FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
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

IMPL_LINK_NOARG(SwSvxNumBulletTabDialog, RemoveNumberingHdl)
{
    EndDialog(RET_USER);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
