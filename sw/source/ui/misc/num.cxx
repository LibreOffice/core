/*************************************************************************
 *
 *  $RCSfile: num.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#define USE_NUMTABPAGES
#define _NUM_CXX

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXIMGMGR_HXX //autogen
#include <sfx2/imgmgr.hxx>
#endif
#ifndef _GALLERY_HXX_
#include <svx/gallery.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <svx/impgrf.hxx>
#endif
#ifndef _SVX_CHARMAP_HXX //autogen
#include <svx/charmap.hxx>
#endif
#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen wg. SvxNumBulletItem
#include <svx/numitem.hxx>
#endif


#ifndef _SWVSET_HXX //autogen
#include <swvset.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _DOCSTYLE_HXX
#include <docstyle.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _UINUMS_HXX
#include <uinums.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _OUTLINE_HXX
#include <outline.hxx>
#endif
#ifndef _NUM_HXX
#include <num.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif

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


static BOOL bLastRelative = FALSE;

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
SwNumPositionTabPage::SwNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, SW_RES( TP_NUM_POSITION ), rSet ),
    aLevelGB(       this, ResId(GB_LEVEL    )),
    aLevelLB(       this, ResId(LB_LEVEL    )),
    aPositionGB(    this, ResId(GB_POSITION )),
    aAlignFT(       this, ResId(FT_ALIGN    )),
    aAlignLB(       this, ResId(LB_ALIGN    )),
    aDistBorderFT(  this, ResId(FT_BORDERDIST   )),
    aDistBorderMF(  this, ResId(MF_BORDERDIST   )),
    aDistNumFT(     this, ResId(FT_NUMDIST      )),
    aDistNumMF(     this, ResId(MF_NUMDIST      )),
    aRelativeCB(    this, ResId(CB_RELATIVE     )),
    aIndentFT(      this, ResId(FT_INDENT       )),
    aIndentMF(      this, ResId(MF_INDENT       )),
    aStandardPB(    this, ResId(PB_STANDARD     )),
    aPreviewGB(     this, ResId(GB_PREVIEW      )),
    aPreviewWIN(    this, ResId(WIN_PREVIEW     )),
    bInInintControl(FALSE),
    pActNum(0),
    pOutlineDlg(0),
    pSaveNum(0)
{
    FreeResource();
    SetExchangeSupport();
    aPreviewWIN.SetBackground(Wallpaper(Color(COL_TRANSPARENT)));

    aRelativeCB.Check();
    aAlignLB.SetSelectHdl(LINK(this, SwNumPositionTabPage, EditModifyHdl));

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

    aLevelLB.SetSelectHdl(LINK(this, SwNumPositionTabPage, LevelHdl));
    aRelativeCB.SetClickHdl(LINK(this, SwNumPositionTabPage, RelativeHdl));
    aStandardPB.SetClickHdl(LINK(this, SwNumPositionTabPage, StandardHdl));

    // Ebenen einfuegen
    for(USHORT i = 1; i <= MAXLEVEL; i++)
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
    bInInintControl = TRUE;
    BOOL bRelative = aRelativeCB.IsChecked() && aRelativeCB.IsEnabled();
//  SetMinDist();
    BOOL bSingleSelection = aLevelLB.GetSelectEntryCount() == 1 && USHRT_MAX != nActNumLvl;
    aDistBorderMF.Enable(bSingleSelection || bRelative || 0 != pOutlineDlg);

    SwTwips nWidth = pWrtSh->GetAnyCurRect(RECT_FRM).Width();

    aDistBorderMF.SetMax(aDistBorderMF.Normalize( nWidth ), FUNIT_TWIP );
    aDistNumMF   .SetMax(aDistNumMF   .Normalize( nWidth ), FUNIT_TWIP );
    aIndentMF    .SetMax(aIndentMF    .Normalize( nWidth ), FUNIT_TWIP );
    SwTwips nLast2 = nWidth /2;
    aDistBorderMF.SetLast( aDistBorderMF.Normalize(   nLast2 ), FUNIT_TWIP );
    aDistNumMF   .SetLast( aDistNumMF     .Normalize( nLast2 ), FUNIT_TWIP );
    aIndentMF    .SetLast( aIndentMF      .Normalize( nLast2 ), FUNIT_TWIP );


    BOOL bSetDistEmpty = FALSE;
//  BOOL bSameDistBorder= TRUE;
    BOOL bSameDistBorderNum = TRUE;
    BOOL bSameDist      = TRUE;
    BOOL bSameIndent    = TRUE;
    BOOL bSameAdjust    = TRUE;

    const SwNumFmt* aNumFmtArr[MAXLEVEL];
    const SwFmtVertOrient* pFirstOrient = 0;
    USHORT nMask = 1;
    USHORT nLvl = USHRT_MAX;
//  SwTwips nFirstLSpace = 0;
    long nFirstBorderText = 0;
    long nFirstBorderTextRelative = -1;
    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        aNumFmtArr[i] = &pActNum->Get(i);
        if(nActNumLvl & nMask)
        {
            if(USHRT_MAX == nLvl)
            {
                nLvl = i;
                pFirstOrient = aNumFmtArr[nLvl]->GetGraphicOrientation();
//              nFirstLSpace = nLvl > 0 ?
//                  aNumFmtArr[nLvl]->GetAbsLSpace() - aNumFmtArr[nLvl - 1]->GetAbsLSpace():
//                      aNumFmtArr[nLvl]->GetAbsLSpace();
                nFirstBorderText = nLvl > 0 ?
                    aNumFmtArr[nLvl]->GetAbsLSpace() + aNumFmtArr[nLvl]->GetFirstLineOffset() -
                    aNumFmtArr[nLvl - 1]->GetAbsLSpace() + aNumFmtArr[nLvl - 1]->GetFirstLineOffset():
                        aNumFmtArr[nLvl]->GetAbsLSpace() + aNumFmtArr[nLvl]->GetFirstLineOffset();
            }

            if( i > nLvl)
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
//                  bSameDistBorder &= aNumFmtArr[i]->GetAbsLSpace() == aNumFmtArr[nLvl]->GetAbsLSpace();
                }

                bSameDist       &= aNumFmtArr[i]->GetCharTextDistance() == aNumFmtArr[nLvl]->GetCharTextDistance();
                bSameIndent     &= aNumFmtArr[i]->GetFirstLineOffset() == aNumFmtArr[nLvl]->GetFirstLineOffset();
                bSameAdjust     &= aNumFmtArr[i]->GetNumAdjust() == aNumFmtArr[nLvl]->GetNumAdjust();

            }
        }
//          else
//              aNumFmtArr[i] = 0;
        nMask <<= 1;

    }
    if(bSameDistBorderNum)
//  if(bSameDistBorder)
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

//      aDistBorderMF.SetValue(aDistBorderMF.Normalize(
//              bRelative ? (long)nFirstLSpace : (long)aNumFmtArr[nLvl]->GetAbsLSpace()),
//                                                      FUNIT_TWIP);
    }
    else
        bSetDistEmpty = TRUE;

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
        USHORT nPos = 1; // zentriert
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        aAlignLB.SelectEntryPos(nPos);
    }
    else
    {
        aAlignLB.SetNoSelection();
    }

    if(TRUE == bSetDistEmpty)
        aDistBorderMF.SetText(aEmptyStr);

    bInInintControl = FALSE;
}
/*-----------------03.12.97 12:21-------------------

--------------------------------------------------*/
/*void SwNumPositionTabPage::SetMinDist()
{
    // JP 03.04.97: Bug 32903 - MinWert fuer DistBorderMF setzen

    // ggfs. den akt. NumLevel anpassen
    USHORT nStart = 0;
    USHORT nEnd = MAXLEVEL;
    USHORT nMask = 1;
    USHORT nTmpLvl = USHRT_MAX;
    SwTwips nMinVal = 0;
    BOOL bInit = FALSE;

    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            if(USHRT_MAX == nTmpLvl)
            {
                nTmpLvl = i;
                const SwNumFmt& rNumFmt = pActNum->Get( nTmpLvl );

                nMinVal = - rNumFmt.GetFirstLineOffset();
                if( nTmpLvl )
                    nMinVal -= pActNum->Get( nTmpLvl - 1 ).GetAbsLSpace();
            }
            const SwNumFmt& rAktNumFmt = pActNum->Get( nStart );
            if( rAktNumFmt.GetAbsLSpace() < nMinVal )
            {
                bInit = TRUE;
                SwNumFmt aNumFmt( rAktNumFmt );
                aNumFmt.SetAbsLSpace( nMinVal );
                pActNum->Set( nStart, aNumFmt );
            }
        }
        nMask <<=1;
    }

    if(!aRelativeCB.IsChecked() && aRelativeCB.IsEnabled())
        nMinVal = 0;
    nMinVal = aDistBorderMF.Normalize( nMinVal );
    aDistBorderMF.SetMin( nMinVal, FUNIT_TWIP );
    aDistBorderMF.SetFirst( nMinVal, FUNIT_TWIP );
    if(bInit)
        InitControls();
}
 */

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
void SwNumPositionTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    UINT16 nTmpNumLvl =
        pOutlineDlg ? pOutlineDlg->GetActNumLevel() : 0;
//          ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet && pExampleSet->GetItemState(FN_PARAM_NUM_PRESET, FALSE, &pItem))
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
        USHORT nMask = 1;
        aLevelLB.SetUpdateMode(FALSE);
        aLevelLB.SetNoSelection();
        aLevelLB.SelectEntryPos( MAXLEVEL, nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(USHORT i = 0; i < MAXLEVEL; i++)
            {
                if(nActNumLvl & nMask)
                    aLevelLB.SelectEntryPos( i, TRUE);
                nMask <<= 1 ;
            }
        aLevelLB.SetUpdateMode(TRUE);
        InitControls();
    }
    aRelativeCB.Enable(1 != nActNumLvl);
    aPreviewWIN.Invalidate();
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
int  SwNumPositionTabPage::DeactivatePage(SfxItemSet *pSet)
{
    if(pOutlineDlg)
        pOutlineDlg->SetActNumLevel(nActNumLvl);
//  else
//      ((SwNumBulletTabDialog*)GetTabDialog())->SetActNumLevel(nActNumLvl);
    if(pSet)
        FillItemSet(*pSet);
    return TRUE;

}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
BOOL SwNumPositionTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(pOutlineDlg)
        *pOutlineDlg->GetNumRule() = *pActNum;
    else if(bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SwUINumRuleItem( *pSaveNum ));
        rSet.Put(SfxBoolItem(FN_PARAM_NUM_PRESET, FALSE));
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
        aLevelLB.EnableMultiSelection(FALSE);
    }
    else if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_ACT_NUMBER, FALSE, &pItem))
        pSaveNum = ((SwUINumRuleItem*)pItem)->GetNumRule();

    nActNumLvl =
        pOutlineDlg ? pOutlineDlg->GetActNumLevel() : 0;
//      ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();
    USHORT nMask = 1;
    aLevelLB.SetUpdateMode(FALSE);
    aLevelLB.SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        aLevelLB.SelectEntryPos( MAXLEVEL, TRUE);
    }
    else
        for(USHORT i = 0; i < MAXLEVEL; i++)
        {
            if(nActNumLvl & nMask)
                aLevelLB.SelectEntryPos( i, TRUE);
            nMask <<= 1;
        }
    aLevelLB.SetUpdateMode(TRUE);

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_CHILD_LEVELS, FALSE, &pItem))
        bHasChild = ((const SfxBoolItem*)pItem)->GetValue();
    if(!pActNum)
        pActNum = new  SwNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    aPreviewWIN.SetNumRule(pActNum);
    InitControls();
    bModified = FALSE;
}

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
    const SwRect& rPrtRect = pWrtSh->GetAnyCurRect(RECT_PAGE);
    aPreviewWIN.SetPageWidth(rPrtRect.Width());
    FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &pWrtSh->GetView()));
    if(eMetric == FUNIT_MM)
    {
        aDistBorderMF .SetDecimalDigits(1);
        aDistNumMF    .SetDecimalDigits(1);
        aIndentMF     .SetDecimalDigits(1);
    }
    aDistBorderMF .SetUnit( eMetric );
    aDistNumMF    .SetUnit( eMetric );
    aIndentMF     .SetUnit( eMetric );
}

/*-----------------03.12.97 11:06-------------------

--------------------------------------------------*/
IMPL_LINK( SwNumPositionTabPage, EditModifyHdl, Edit *, pEdit )
{

    USHORT nStart = 0;
    USHORT nEnd = MAXLEVEL;
    USHORT nMask = 1;
    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFmt aNumFmt(pActNum->Get(i));

            USHORT nPos = aAlignLB.GetSelectEntryPos();
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
    USHORT nSaveNumLvl = nActNumLvl;
    nActNumLvl = 0;
    if(pBox->IsEntryPosSelected( MAXLEVEL ) &&
            (pBox->GetSelectEntryCount() == 1 || nSaveNumLvl != 0xffff))
    {
        nActNumLvl = 0xFFFF;
        pBox->SetUpdateMode(FALSE);
        for( USHORT i = 0; i < MAXLEVEL; i++ )
            pBox->SelectEntryPos( i, FALSE );
        pBox->SetUpdateMode(TRUE);
    }
    else if(pBox->GetSelectEntryCount())
    {
        USHORT nMask = 1;
        for( USHORT i = 0; i < MAXLEVEL; i++ )
        {
            if(pBox->IsEntryPosSelected( i ))
                nActNumLvl |= nMask;
            nMask <<= 1;
        }
        pBox->SelectEntryPos( MAXLEVEL, FALSE );
    }
    else
    {
        nActNumLvl = nSaveNumLvl;
        USHORT nMask = 1;
        for( USHORT i = 0; i < MAXLEVEL; i++ )
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
    InitControls();
    return 0;
}
/*-----------------03.12.97 12:24-------------------

--------------------------------------------------*/
IMPL_LINK( SwNumPositionTabPage, DistanceHdl, MetricField *, pFld )
{
    if(bInInintControl)
        return 0;
    long nValue = pFld->Denormalize(pFld->GetValue(FUNIT_TWIP));
    USHORT nMask = 1;
    for(USHORT i = 0; i < MAXLEVEL; i++)
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
                        aNumFmt.SetAbsLSpace( USHORT(nValue - nTmp));
                    }
                    else
                    {
                        long nTmp = pActNum->Get( i - 1 ).GetAbsLSpace() +
                                    pActNum->Get( i - 1 ).GetFirstLineOffset() -
                                    pActNum->Get( i ).GetFirstLineOffset();

                        aNumFmt.SetAbsLSpace( USHORT(nValue + nTmp));
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
                aNumFmt.SetAbsLSpace(USHORT(nAbsLSpace + nDiff));
                aNumFmt.SetFirstLineOffset( -(short)nValue );
            }

            pActNum->Set( i, aNumFmt );
        }
        nMask <<= 1;
    }

    SetModified();
//  SetMinDist();
    if(!aDistBorderMF.IsEnabled())
        aDistBorderMF.SetText(aEmptyStr);

    return 0;
}

/*-----------------04.12.97 12:35-------------------

--------------------------------------------------*/
IMPL_LINK( SwNumPositionTabPage, RelativeHdl, CheckBox *, pBox )
{
    BOOL bOn = pBox->IsChecked();
//  SetMinDist();
    BOOL bSingleSelection = aLevelLB.GetSelectEntryCount() == 1 && USHRT_MAX != nActNumLvl;
    BOOL bSetValue = FALSE;
    long nValue = 0;
    if(bOn || bSingleSelection)
    {
        USHORT nMask = 1;
        BOOL bFirst = TRUE;
        bSetValue = TRUE;
        for(USHORT i = 0; i < MAXLEVEL; i++)
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
                bFirst = FALSE;
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
/*-----------------05.12.97 15:33-------------------

--------------------------------------------------*/
IMPL_LINK( SwNumPositionTabPage, StandardHdl, PushButton *, EMPTYARG )
{
    USHORT nMask = 1;
    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        if(nActNumLvl & nMask)
        {
            SwNumFmt aNumFmt( pActNum->Get( i ) );
            SwNumRule aTmpNumRule(pWrtSh->GetUniqueNumRuleName(),
                pOutlineDlg ? OUTLINE_RULE : NUM_RULE
                );
            SwNumFmt aTempFmt(aTmpNumRule.Get( i ));
            aNumFmt.SetAbsLSpace( aTempFmt.GetAbsLSpace());
            aNumFmt.SetCharTextDistance( aTempFmt.GetCharTextDistance() );
            aNumFmt.SetFirstLineOffset( aTempFmt.GetFirstLineOffset() );

            pActNum->Set( i, aNumFmt );
        }
        nMask <<= 1;
    }

    InitControls();
    SetModified();
    return 0;
}

#ifdef DEBUG
void SwNumPositionTabPage::SetModified(BOOL bRepaint)
{
    bModified = TRUE;
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
    SfxTabDialog(pParent, SW_RES(DLG_SVXTEST_NUM_BULLET), pSwItemSet, FALSE, &aEmptyStr),
    rWrtSh(rSh),
    nRetOptionsDialog(USHRT_MAX),
    sRemoveText(ResId(ST_RESET))
{
    FreeResource();
    GetUserButton()->SetText(sRemoveText);
    GetUserButton()->SetHelpId(HID_NUM_RESET);
    GetUserButton()->SetClickHdl(LINK(this, SwSvxNumBulletTabDialog, RemoveNumberingHdl));
    if(!rWrtSh.GetCurNumRule())
        GetUserButton()->Enable(FALSE);
    AddTabPage(RID_SVXPAGE_PICK_SINGLE_NUM,     &SvxSingleNumPickTabPage::Create, 0);
    AddTabPage(RID_SVXPAGE_PICK_BULLET, &SvxBulletPickTabPage::Create, 0);
    AddTabPage(RID_SVXPAGE_PICK_NUM,    &SvxNumPickTabPage::Create, 0);
    AddTabPage(RID_SVXPAGE_PICK_BMP,    &SvxBitmapPickTabPage::Create, 0);
    AddTabPage(RID_SVXPAGE_NUM_OPTIONS, &SvxNumOptionsTabPage::Create, 0);
    AddTabPage(RID_SVXPAGE_NUM_POSITION,&SvxNumPositionTabPage::Create, 0);

}
/*-----------------07.02.97 12.08-------------------

--------------------------------------------------*/

SwSvxNumBulletTabDialog::~SwSvxNumBulletTabDialog()
{
}

/*-----------------07.02.97 14.48-------------------

--------------------------------------------------*/

void SwSvxNumBulletTabDialog::PageCreated(USHORT nPageId, SfxTabPage& rPage)
{
    //Namen der Vorlagen und Metric setzen
    String sNumCharFmt; GetDocPoolNm( RES_POOLCHR_NUM_LEVEL, sNumCharFmt );
    String sBulletCharFmt; GetDocPoolNm( RES_POOLCHR_BUL_LEVEL, sBulletCharFmt );

    switch ( nPageId )
    {
    case RID_SVXPAGE_PICK_NUM:
        ((SvxNumPickTabPage&)rPage).SetCharFmtNames(sNumCharFmt, sBulletCharFmt);
        break;
    case RID_SVXPAGE_PICK_BULLET :
        ((SvxBulletPickTabPage&)rPage).SetCharFmtName(sBulletCharFmt);
        break;

    case RID_SVXPAGE_NUM_OPTIONS:
        {
            ((SvxNumOptionsTabPage&)rPage).SetCharFmts(sNumCharFmt, sBulletCharFmt);
            ListBox& rCharFmtLB = ((SvxNumOptionsTabPage&)rPage).GetCharFmtListBox();
            // Zeichenvorlagen sammeln
            rCharFmtLB.Clear();
            rCharFmtLB.InsertEntry( ViewShell::GetShellRes()->aStrNone );
            SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
            ::FillCharStyleListBox(rCharFmtLB,  pDocShell);
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            ((SvxNumOptionsTabPage&)rPage).SetMetric(eMetric);
        }
        break;
    case RID_SVXPAGE_NUM_POSITION:
        {
            SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            ((SvxNumPositionTabPage&)rPage).SetMetric(eMetric);
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

/*--------------------------------------------------------------------


      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:45  hr
      initial import

      Revision 1.198  2000/09/18 16:05:58  willem.vandorp
      OpenOffice header added.

      Revision 1.197  2000/07/27 21:16:39  jp
      opt: get template names direct from the doc and don't load it from the resource

      Revision 1.196  2000/06/30 15:08:15  os
      #76568# CreateFromInt32

      Revision 1.195  2000/05/23 19:22:35  jp
      Bugfixes for Unicode

      Revision 1.194  2000/04/18 15:08:17  os
      UNICODE

      Revision 1.193  2000/03/03 15:17:02  os
      StarView remainders removed

      Revision 1.192  2000/02/11 14:56:31  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.191  1999/08/19 14:26:00  OS
      #67374# embedded graphics in numberings


      Rev 1.190   19 Aug 1999 16:26:00   OS
   #67374# embedded graphics in numberings

      Rev 1.189   23 Apr 1999 16:09:20   OS
   #64476# Differenz zu numpages.cxx

      Rev 1.188   14 Apr 1999 08:57:14   OS
   #64476# noch ein Problem behoben

      Rev 1.187   13 Apr 1999 16:04:20   OS
   #64473# CheckBox an der richtigen Stelle enablen/disablen

      Rev 1.186   08 Apr 1999 13:30:58   OS
   #64476# SetMinDist wird nicht mehr gebraucht

      Rev 1.185   19 Mar 1999 07:38:10   OS
   #62656# Abstand Num->Text jetzt richtig

      Rev 1.184   18 Mar 1999 09:25:40   OS
   #63049# gemogelte Anzeige der Numerierungseinzuege

      Rev 1.183   02 Dec 1998 09:20:56   OS
   #58263# Entfernen-Button fuer Numerierungen

      Rev 1.182   17 Nov 1998 10:57:20   OS
   #58263# Numerierungs-Tabseiten aus dem Svx

      Rev 1.181   03 Nov 1998 07:25:04   OS
   #58263# Numerierungsdialog in den Svx

      Rev 1.180   12 Oct 1998 13:17:26   OS
   #57760# Fontgroesse fuer das Beispiel eleganter ermitteln

      Rev 1.179   22 Sep 1998 10:04:02   OS
   #56894# vollstaendig fuer fortlaufende Num. veraendern

      Rev 1.178   08 Sep 1998 17:01:42   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.177   06 Aug 1998 21:41:14   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.176   18 Jul 1998 14:50:46   OS
   richtige Ebenenanzahl fuer vollst. Numerierung #53233#

      Rev 1.175   09 Jul 1998 14:03:12   OS
   Kapitelnumerierung jetzt TabDialog

      Rev 1.174   07 Jul 1998 11:31:08   OS
   InitControls: erst Min/Max setzen, dann Inhalte setzn

      Rev 1.173   07 Jul 1998 09:16:40   OS
   DisableItem ist nicht zulaessig #52134#

      Rev 1.172   04 Jul 1998 15:44:10   OS
   Grafikhoehe und Ausrichtung richtig anzeigen #52140#

      Rev 1.171   25 Jun 1998 11:26:48   OS
   RelLSpace ausbgebaut #51379#

      Rev 1.170   24 Jun 1998 09:00:44   OS
   MS-Optimierungs-GPF #51425#


 --------------------------------------------------------------------*/


