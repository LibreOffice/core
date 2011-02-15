/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#define USE_NUMTABPAGES
#define _NUM_CXX
#include <hintids.hxx>

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/imgmgr.hxx>
#include <svx/gallery.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/numitem.hxx>
#include <swvset.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#include <uitool.hxx>
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
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

#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _FRMUI_HRC
#include <frmui.hrc>
#endif
#ifndef _NUM_HRC
#include <num.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#include <SwStyleNameMapper.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svl/stritem.hxx>
#include <svl/aeitem.hxx>
#include <svl/slstitm.hxx>

static sal_Bool bLastRelative = sal_False;

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
SwNumPositionTabPage::SwNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, SW_RES( TP_NUM_POSITION ), rSet ),
    aPositionFL(    this, SW_RES(FL_POSITION )),
    aLevelFL(       this, SW_RES(FL_LEVEL    )),
    aLevelLB(       this, SW_RES(LB_LEVEL   )),

    aDistBorderFT(  this, SW_RES(FT_BORDERDIST  )),
    aDistBorderMF(  this, SW_RES(MF_BORDERDIST  )),
    aRelativeCB(    this, SW_RES(CB_RELATIVE     )),
    aIndentFT(      this, SW_RES(FT_INDENT       )),
    aIndentMF(      this, SW_RES(MF_INDENT       )),
    aDistNumFT(     this, SW_RES(FT_NUMDIST     )),
    aDistNumMF(     this, SW_RES(MF_NUMDIST     )),
    aAlignFT(       this, SW_RES(FT_ALIGN    )),
    aAlignLB(       this, SW_RES(LB_ALIGN    )),
    // --> OD 2008-02-01 #newlistlevelattrs#
    aLabelFollowedByFT( this, SW_RES(FT_LABEL_FOLLOWED_BY) ),
    aLabelFollowedByLB( this, SW_RES(LB_LABEL_FOLLOWED_BY) ),
    aListtabFT( this, SW_RES(FT_LISTTAB) ),
    aListtabMF( this, SW_RES(MF_LISTTAB) ),
    aAlign2FT( this, SW_RES(FT_ALIGN_2) ),
    aAlign2LB( this, SW_RES(LB_ALIGN_2) ),
    aAlignedAtFT( this, SW_RES(FT_ALIGNED_AT) ),
    aAlignedAtMF( this, SW_RES(MF_ALIGNED_AT) ),
    aIndentAtFT( this, SW_RES(FT_INDENT_AT) ),
    aIndentAtMF( this, SW_RES(MF_INDENT_AT) ),
    // <--
    aStandardPB(    this, SW_RES(PB_STANDARD        )),

    aPreviewWIN(    this, SW_RES(WIN_PREVIEW     )),

    pActNum(0),
    pSaveNum(0),
    pWrtSh(0),
    pOutlineDlg(0),
    bPreset( sal_False ),
    bInInintControl(sal_False),
    // --> OD 2008-02-01 #newlistlevelattrs#
    bLabelAlignmentPosAndSpaceModeActive( false )
    // <--
{
    FreeResource();
    SetExchangeSupport();
    aPreviewWIN.SetBackground(Wallpaper(Color(COL_TRANSPARENT)));

    aStandardPB.SetAccessibleRelationMemberOf(&aPositionFL);


    aRelativeCB.Check();
    aAlignLB.SetSelectHdl(LINK(this, SwNumPositionTabPage, EditModifyHdl));
    // --> OD 2008-02-01 #newlistlevelattrs#
    aAlign2LB.SetSelectHdl(LINK(this, SwNumPositionTabPage, EditModifyHdl));
    for ( sal_uInt16 i = 0; i < aAlignLB.GetEntryCount(); ++i )
    {
        aAlign2LB.InsertEntry( aAlignLB.GetEntry( i ) );
    }
    aAlign2LB.SetDropDownLineCount( aAlign2LB.GetEntryCount() );
    aAlign2FT.SetText( aAlignFT.GetText() );
    // <--

    Link aLk = LINK(this, SwNumPositionTabPage, DistanceHdl);
    aDistBorderMF.SetUpHdl(aLk);
    aDistNumMF.SetUpHdl(aLk);
    aIndentMF.SetUpHdl(aLk);
    aDistBorderMF.SetDownHdl(aLk);
    aDistNumMF.SetDownHdl(aLk);
    aIndentMF.SetDownHdl(aLk);
    aDistBorderMF.SetLoseFocusHdl(aLk);
    aDistNumMF.SetLoseFocusHdl(aLk);
    aIndentMF.SetLoseFocusHdl(aLk);

    // --> OD 2008-02-01 #newlistlevelattrs#
    aLabelFollowedByLB.SetDropDownLineCount( aLabelFollowedByLB.GetEntryCount() );
    aLabelFollowedByLB.SetSelectHdl( LINK(this, SwNumPositionTabPage, LabelFollowedByHdl_Impl) );

    aLk = LINK(this, SwNumPositionTabPage, ListtabPosHdl_Impl);
    aListtabMF.SetUpHdl(aLk);
    aListtabMF.SetDownHdl(aLk);
    aListtabMF.SetLoseFocusHdl(aLk);

    aLk = LINK(this, SwNumPositionTabPage, AlignAtHdl_Impl);
    aAlignedAtMF.SetUpHdl(aLk);
    aAlignedAtMF.SetDownHdl(aLk);
    aAlignedAtMF.SetLoseFocusHdl(aLk);

    aLk = LINK(this, SwNumPositionTabPage, IndentAtHdl_Impl);
    aIndentAtMF.SetUpHdl(aLk);
    aIndentAtMF.SetDownHdl(aLk);
    aIndentAtMF.SetLoseFocusHdl(aLk);
    // <--

    aLevelLB.SetSelectHdl(LINK(this, SwNumPositionTabPage, LevelHdl));
    aRelativeCB.SetClickHdl(LINK(this, SwNumPositionTabPage, RelativeHdl));
    aStandardPB.SetClickHdl(LINK(this, SwNumPositionTabPage, StandardHdl));

    // Ebenen einfuegen
    for(sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        aLevelLB.InsertEntry(String::CreateFromInt32(i));
    String sEntry(String::CreateFromAscii("1 - "));
    sEntry += String::CreateFromInt32(MAXLEVEL);
    aLevelLB.InsertEntry(sEntry);
    aLevelLB.SelectEntry(sEntry);

    aRelativeCB.Check(bLastRelative);
    aPreviewWIN.SetPositionMode();
}
/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
SwNumPositionTabPage::~SwNumPositionTabPage()
{
    delete pActNum;
}
/*-----------------03.12.97 10:06-------------------

--------------------------------------------------*/
void SwNumPositionTabPage::InitControls()
{
    bInInintControl = sal_True;
    // --> OD 2008-02-01 #newlistlevelattrs#
    const bool bRelative = !bLabelAlignmentPosAndSpaceModeActive &&
                           aRelativeCB.IsEnabled() && aRelativeCB.IsChecked();
    const bool bSingleSelection = aLevelLB.GetSelectEntryCount() == 1 &&
                                  USHRT_MAX != nActNumLvl;

    aDistBorderMF.Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative || pOutlineDlg != 0 ) );
    aDistBorderFT.Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative || pOutlineDlg != 0 ) );
    // <--

    bool bSetDistEmpty = false;
    bool bSameDistBorderNum = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameDist      = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndent    = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAdjust    = true;

    // --> OD 2008-02-01 #newlistlevelattrs#
    bool bSameLabelFollowedBy = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameListtab = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAlignAt = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndentAt = bLabelAlignmentPosAndSpaceModeActive;
    // <--

    const SwNumFmt* aNumFmtArr[MAXLEVEL];
    const SwFmtVertOrient* pFirstOrient = 0;
    sal_uInt16 nMask = 1;
    sal_uInt16 nLvl = USHRT_MAX;
    long nFirstBorderText = 0;
    long nFirstBorderTextRelative = -1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        aNumFmtArr[i] = &pActNum->Get(i);
        if(nActNumLvl & nMask)
        {
            if(USHRT_MAX == nLvl)
            {
                nLvl = i;
                // --> OD 2008-02-01 #newlistlevelattrs#
                if ( !bLabelAlignmentPosAndSpaceModeActive )
                {
                    pFirstOrient = aNumFmtArr[nLvl]->GetGraphicOrientation();
                    nFirstBorderText = nLvl > 0 ?
                        aNumFmtArr[nLvl]->GetAbsLSpace() + aNumFmtArr[nLvl]->GetFirstLineOffset() -
                        aNumFmtArr[nLvl - 1]->GetAbsLSpace() + aNumFmtArr[nLvl - 1]->GetFirstLineOffset():
                            aNumFmtArr[nLvl]->GetAbsLSpace() + aNumFmtArr[nLvl]->GetFirstLineOffset();
                }
                // <--
            }

            if( i > nLvl)
            {
                // --> OD 2008-02-01 #newlistlevelattrs#
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
                // <--

            }
        }
        nMask <<= 1;

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
        aDistBorderMF.SetValue(aDistBorderMF.Normalize(nDistBorderNum),FUNIT_TWIP);
    }
    else
        bSetDistEmpty = sal_True;

    if(bSameDist)
        aDistNumMF   .SetValue(aDistNumMF.Normalize(aNumFmtArr[nLvl]->GetCharTextDistance()), FUNIT_TWIP);
    else
        aDistNumMF.SetText(aEmptyStr);
    if(bSameIndent)
        aIndentMF.SetValue(aIndentMF.Normalize(-aNumFmtArr[nLvl]->GetFirstLineOffset()), FUNIT_TWIP);
    else
        aIndentMF.SetText(aEmptyStr);

    if(bSameAdjust)
    {
        sal_uInt16 nPos = 1; // zentriert
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        aAlignLB.SelectEntryPos(nPos);
        // --> OD 2008-02-01 #newlistlevelattrs#
        aAlign2LB.SelectEntryPos( nPos );
        // <--
    }
    else
    {
        aAlignLB.SetNoSelection();
        // --> OD 2008-02-01 #newlistlevelattrs#
        aAlign2LB.SetNoSelection();
        // <--
    }

    // --> OD 2008-02-01 #newlistlevelattrs#
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
        aLabelFollowedByLB.SelectEntryPos( nPos );
    }
    else
    {
        aLabelFollowedByLB.SetNoSelection();
    }

    if ( aNumFmtArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
    {
        aListtabFT.Enable( true );
        aListtabMF.Enable( true );
        if ( bSameListtab )
        {
            aListtabMF.SetValue(aListtabMF.Normalize(aNumFmtArr[nLvl]->GetListtabPos()),FUNIT_TWIP);
        }
        else
        {
            aListtabMF.SetText(aEmptyStr);
        }
    }
    else
    {
        aListtabFT.Enable( false );
        aListtabMF.Enable( false );
        aListtabMF.SetText(aEmptyStr);
    }

    if ( bSameAlignAt )
    {
        aAlignedAtMF.SetValue(
            aAlignedAtMF.Normalize( aNumFmtArr[nLvl]->GetIndentAt() +
                                    aNumFmtArr[nLvl]->GetFirstLineIndent()),
            FUNIT_TWIP );
    }
    else
    {
        aAlignedAtMF.SetText(aEmptyStr);
    }

    if ( bSameIndentAt )
    {
        aIndentAtMF.SetValue(
            aIndentAtMF.Normalize( aNumFmtArr[nLvl]->GetIndentAt()), FUNIT_TWIP );
    }
    else
    {
        aIndentAtMF.SetText(aEmptyStr);
    }
    // <--

    if(sal_True == bSetDistEmpty)
        aDistBorderMF.SetText(aEmptyStr);

    bInInintControl = sal_False;
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
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
    //
    bModified = (!pActNum->GetNumFmt( 0 ) || bPreset);
    if(*pActNum != *pSaveNum ||
        nActNumLvl != nTmpNumLvl )
    {
        *pActNum = *pSaveNum;
        nActNumLvl = nTmpNumLvl;
        sal_uInt16 nMask = 1;
        aLevelLB.SetUpdateMode(sal_False);
        aLevelLB.SetNoSelection();
        aLevelLB.SelectEntryPos( MAXLEVEL, nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
            {
                if(nActNumLvl & nMask)
                    aLevelLB.SelectEntryPos( i, sal_True);
                nMask <<= 1 ;
            }
        aLevelLB.SetUpdateMode(sal_True);

        // --> OD 2008-02-01 #newlistlevelattrs#
        InitPosAndSpaceMode();
        ShowControlsDependingOnPosAndSpaceMode();
        // <--

        InitControls();
    }
    aRelativeCB.Enable(1 != nActNumLvl);
    aPreviewWIN.Invalidate();
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
int  SwNumPositionTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    SwOutlineTabDialog::SetActNumLevel(nActNumLvl);
    if(_pSet)
        FillItemSet(*_pSet);
    return sal_True;

}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
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

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
void SwNumPositionTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    if(pOutlineDlg)
    {
        pSaveNum = pOutlineDlg->GetNumRule();
        aLevelLB.EnableMultiSelection(sal_False);
    }
    else if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_ACT_NUMBER, sal_False, &pItem))
        pSaveNum = ((SwUINumRuleItem*)pItem)->GetNumRule();

    nActNumLvl = SwOutlineTabDialog::GetActNumLevel();
    sal_uInt16 nMask = 1;
    aLevelLB.SetUpdateMode(sal_False);
    aLevelLB.SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        aLevelLB.SelectEntryPos( MAXLEVEL, sal_True);
    }
    else
        for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        {
            if(nActNumLvl & nMask)
                aLevelLB.SelectEntryPos( i, sal_True);
            nMask <<= 1;
        }
    aLevelLB.SetUpdateMode(sal_True);

    if(!pActNum)
        pActNum = new  SwNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    aPreviewWIN.SetNumRule(pActNum);
    // --> OD 2008-02-01 #newlistlevelattrs#
    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();
    // <--
    InitControls();
    bModified = sal_False;
}

// --> OD 2008-01-11 #newlistlevelattrs#
void SwNumPositionTabPage::InitPosAndSpaceMode()
{
    if ( pActNum == 0 )
    {
        DBG_ASSERT( false,
                "<SwNumPositionTabPage::InitPosAndSpaceMode()> - misusage of method -> <pAktNum> has to be already set!" );
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
    aDistBorderFT.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aDistBorderMF.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aRelativeCB.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aIndentFT.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aIndentMF.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aDistNumFT.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aDistNumMF.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aAlignFT.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aAlignLB.Show( !bLabelAlignmentPosAndSpaceModeActive );

    aLabelFollowedByFT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aLabelFollowedByLB.Show( bLabelAlignmentPosAndSpaceModeActive );
    aListtabFT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aListtabMF.Show( bLabelAlignmentPosAndSpaceModeActive );
    aAlign2FT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aAlign2LB.Show( bLabelAlignmentPosAndSpaceModeActive );
    aAlignedAtFT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aAlignedAtMF.Show( bLabelAlignmentPosAndSpaceModeActive );
    aIndentAtFT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aIndentAtMF.Show( bLabelAlignmentPosAndSpaceModeActive );
}
// <--

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
SfxTabPage* SwNumPositionTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwNumPositionTabPage(pParent, rAttrSet);
}

/*-----------------04.12.97 12:51-------------------

--------------------------------------------------*/
void SwNumPositionTabPage::SetWrtShell(SwWrtShell* pSh)
{
    pWrtSh = pSh;

    // --> OD 2008-02-01 #newlistlevelattrs#
    const SwTwips nWidth = pWrtSh->GetAnyCurRect(RECT_FRM).Width();

    aDistBorderMF.SetMax(aDistBorderMF.Normalize( nWidth ), FUNIT_TWIP );
    aDistNumMF   .SetMax(aDistNumMF   .Normalize( nWidth ), FUNIT_TWIP );
    aIndentMF    .SetMax(aIndentMF    .Normalize( nWidth ), FUNIT_TWIP );
    // --> OD 2008-02-18 #newlistlevelattrs#
    aListtabMF.SetMax(aListtabMF.Normalize( nWidth ), FUNIT_TWIP );
    aAlignedAtMF.SetMax(aAlignedAtMF.Normalize( nWidth ), FUNIT_TWIP );
    aIndentAtMF.SetMax(aIndentAtMF.Normalize( nWidth ), FUNIT_TWIP );
    // <--
    const SwTwips nLast2 = nWidth /2;
    aDistBorderMF.SetLast( aDistBorderMF.Normalize(   nLast2 ), FUNIT_TWIP );
    aDistNumMF   .SetLast( aDistNumMF     .Normalize( nLast2 ), FUNIT_TWIP );
    aIndentMF    .SetLast( aIndentMF      .Normalize( nLast2 ), FUNIT_TWIP );
    // --> OD 2008-02-18 #newlistlevelattrs#
    aListtabMF.SetLast(aListtabMF.Normalize( nLast2 ), FUNIT_TWIP );
    aAlignedAtMF.SetLast(aAlignedAtMF.Normalize( nLast2 ), FUNIT_TWIP );
    aIndentAtMF.SetLast(aIndentAtMF.Normalize( nLast2 ), FUNIT_TWIP );
    // <--
    // <--

    const SwRect& rPrtRect = pWrtSh->GetAnyCurRect(RECT_PAGE);
    aPreviewWIN.SetPageWidth(rPrtRect.Width());
    FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &pWrtSh->GetView()));
    if(eMetric == FUNIT_MM)
    {
        aDistBorderMF .SetDecimalDigits(1);
        aDistNumMF    .SetDecimalDigits(1);
        aIndentMF     .SetDecimalDigits(1);
        // --> OD 2008-02-18 #newlistlevelattrs#
        aListtabMF.SetDecimalDigits(1);
        aAlignedAtMF.SetDecimalDigits(1);
        aIndentAtMF.SetDecimalDigits(1);
        // <--
    }
    aDistBorderMF .SetUnit( eMetric );
    aDistNumMF    .SetUnit( eMetric );
    aIndentMF     .SetUnit( eMetric );
    // --> OD 2008-02-18 #newlistlevelattrs#
    aListtabMF.SetUnit( eMetric );
    aAlignedAtMF.SetUnit( eMetric );
    aIndentAtMF.SetUnit( eMetric );
    // <--
}

/*-----------------03.12.97 11:06-------------------

--------------------------------------------------*/
IMPL_LINK( SwNumPositionTabPage, EditModifyHdl, Edit *, EMPTYARG )
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFmt aNumFmt(pActNum->Get(i));

            // --> OD 2008-02-01 #newlistlevelattrs#
            const sal_uInt16 nPos = aAlignLB.IsVisible()
                                ? aAlignLB.GetSelectEntryPos()
                                : aAlign2LB.GetSelectEntryPos();
            // <--
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
/*-----------------03.12.97 11:11-------------------

--------------------------------------------------*/
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
    aRelativeCB.Enable(1 != nActNumLvl);
    SetModified();
    // --> OD 2008-02-01 #newlistlevelattrs#
    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();
    // <--
    InitControls();
    return 0;
}
/*-----------------03.12.97 12:24-------------------

--------------------------------------------------*/
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
            if(pFld == &aDistBorderMF)
            {

                if(aRelativeCB.IsChecked() && aRelativeCB.IsEnabled())
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
            else if(pFld == &aDistNumMF)
            {
                aNumFmt.SetCharTextDistance( (short)nValue );
            }
            else if(pFld == &aIndentMF)
            {
                //jetzt muss mit dem FirstLineOffset auch der AbsLSpace veraendert werden
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
    if(!aDistBorderMF.IsEnabled())
        aDistBorderMF.SetText(aEmptyStr);

    return 0;
}

/*-----------------04.12.97 12:35-------------------

--------------------------------------------------*/
IMPL_LINK( SwNumPositionTabPage, RelativeHdl, CheckBox *, pBox )
{
    sal_Bool bOn = pBox->IsChecked();
    sal_Bool bSingleSelection = aLevelLB.GetSelectEntryCount() == 1 && USHRT_MAX != nActNumLvl;
    sal_Bool bSetValue = sal_False;
    long nValue = 0;
    if(bOn || bSingleSelection)
    {
        sal_uInt16 nMask = 1;
        sal_Bool bFirst = sal_True;
        bSetValue = sal_True;
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
                bFirst = sal_False;
            }
            nMask <<= 1;
        }

    }
    if(bSetValue)
        aDistBorderMF.SetValue(aDistBorderMF.Normalize(nValue), FUNIT_TWIP);
    else
        aDistBorderMF.SetText(aEmptyStr);
    aDistBorderMF.Enable(bOn || bSingleSelection||0 != pOutlineDlg);
    bLastRelative = bOn;
    return 0;
}

// --> OD 2008-02-01 #newlistlevelattrs#
IMPL_LINK( SwNumPositionTabPage, LabelFollowedByHdl_Impl, ListBox*, EMPTYARG )
{
    // determine value to be set at the chosen list levels
    SvxNumberFormat::SvxNumLabelFollowedBy eLabelFollowedBy =
                                                    SvxNumberFormat::LISTTAB;
    {
        const sal_uInt16 nPos = aLabelFollowedByLB.GetSelectEntryPos();
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
    aListtabFT.Enable( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    aListtabMF.Enable( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    if ( bSameListtabPos && eLabelFollowedBy == SvxNumberFormat::LISTTAB )
    {
        aListtabMF.SetValue(
            aListtabMF.Normalize( pActNum->Get( nFirstLvl ).GetListtabPos() ),
            FUNIT_TWIP );
    }
    else
    {
        aListtabMF.SetText( String() );
    }

    SetModified();

    return 0;
}
// <--

// --> OD 2008-02-01 #newlistlevelattrs#
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
// <--

// --> OD 2008-02-01 #newlistlevelattrs#
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
// <--

// --> OD 2008-02-01 #newlistlevelattrs#
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
// <--

/*-----------------05.12.97 15:33-------------------

--------------------------------------------------*/
IMPL_LINK( SwNumPositionTabPage, StandardHdl, PushButton *, EMPTYARG )
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFmt aNumFmt( pActNum->Get( i ) );
            // --> OD 2008-02-11 #newlistlevelattrs#
            SwNumRule aTmpNumRule( pWrtSh->GetUniqueNumRuleName(),
                                   aNumFmt.GetPositionAndSpaceMode(),
                                   pOutlineDlg ? OUTLINE_RULE : NUM_RULE );
            // <--
            SwNumFmt aTempFmt(aTmpNumRule.Get( i ));
            // --> OD 2008-02-05 #newlistlevelattrs#
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
            // <--

            pActNum->Set( i, aNumFmt );
        }
        nMask <<= 1;
    }

    InitControls();
    SetModified();
    return 0;
}

#if OSL_DEBUG_LEVEL > 1
void SwNumPositionTabPage::SetModified(sal_Bool bRepaint)
{
    bModified = sal_True;
    if(bRepaint)
    {
        aPreviewWIN.SetLevel(nActNumLvl);
        aPreviewWIN.Invalidate();
    }
}

#endif

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/


SwSvxNumBulletTabDialog::SwSvxNumBulletTabDialog(Window* pParent,
                    const SfxItemSet* pSwItemSet, SwWrtShell & rSh) :
    SfxTabDialog(pParent, SW_RES(DLG_SVXTEST_NUM_BULLET), pSwItemSet, sal_False, &aEmptyStr),
    rWrtSh(rSh),
    sRemoveText(SW_RES(ST_RESET)),
    nRetOptionsDialog(USHRT_MAX)
{
    FreeResource();
    GetUserButton()->SetText(sRemoveText);
    GetUserButton()->SetHelpId(HID_NUM_RESET);
    GetUserButton()->SetClickHdl(LINK(this, SwSvxNumBulletTabDialog, RemoveNumberingHdl));
    if(!rWrtSh.GetCurNumRule())
        GetUserButton()->Enable(sal_False);
    AddTabPage( RID_SVXPAGE_PICK_SINGLE_NUM );
    AddTabPage( RID_SVXPAGE_PICK_BULLET );
    AddTabPage( RID_SVXPAGE_PICK_NUM );
    AddTabPage( RID_SVXPAGE_PICK_BMP );
    AddTabPage( RID_SVXPAGE_NUM_OPTIONS );
    AddTabPage( RID_SVXPAGE_NUM_POSITION );

}
/*-----------------07.02.97 12.08-------------------

--------------------------------------------------*/

SwSvxNumBulletTabDialog::~SwSvxNumBulletTabDialog()
{
}

/*-----------------07.02.97 14.48-------------------

--------------------------------------------------*/

void SwSvxNumBulletTabDialog::PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage)
{
    //Namen der Vorlagen und Metric setzen
    String sNumCharFmt, sBulletCharFmt;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFmt );
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BUL_LEVEL, sBulletCharFmt );

    switch ( nPageId )
    {
    case RID_SVXPAGE_PICK_NUM:
        {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
            aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
            rPage.PageCreated(aSet);
        }
        break;
    case RID_SVXPAGE_PICK_BULLET :
        {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
            rPage.PageCreated(aSet);
        }
        break;

    case RID_SVXPAGE_NUM_OPTIONS:
        {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
            aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
            // Zeichenvorlagen sammeln
            ListBox rCharFmtLB(this);
            rCharFmtLB.Clear();
            rCharFmtLB.InsertEntry( ViewShell::GetShellRes()->aStrNone );
            SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
            ::FillCharStyleListBox(rCharFmtLB,  pDocShell);
            List aList;
            for(sal_uInt16 j = 0; j < rCharFmtLB.GetEntryCount(); j++)
            {

                 aList.Insert( new XubString(rCharFmtLB.GetEntry(j)), LIST_APPEND );
            }
            aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;

            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric) ) );
            rPage.PageCreated(aSet);
            for( sal_uInt16 i = (sal_uInt16)aList.Count(); i; --i )
                    delete (XubString*)aList.Remove(i);
            aList.Clear();
        }
        break;
    case RID_SVXPAGE_NUM_POSITION:
        {
            SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)) );
            rPage.PageCreated(aSet);
//          ((SvxNumPositionTabPage&)rPage).SetWrtShell(&rWrtSh);
        }
        break;
    }
}
/*-----------------17.02.97 16.52-------------------

--------------------------------------------------*/
short  SwSvxNumBulletTabDialog::Ok()
{
    short nRet = SfxTabDialog::Ok();
    pExampleSet->ClearItem(SID_PARAM_NUM_PRESET);
    return nRet;
}
/* -----------------02.12.98 08:35-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(SwSvxNumBulletTabDialog, RemoveNumberingHdl, PushButton*, EMPTYARG)
{
    EndDialog(RET_USER);
    return 0;
}


