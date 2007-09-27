/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: num.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:22:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#define USE_NUMTABPAGES
#define _NUM_CXX

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
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svtools/stritem.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/slstitm.hxx>

static BOOL bLastRelative = FALSE;

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
SwNumPositionTabPage::SwNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, SW_RES( TP_NUM_POSITION ), rSet ),
    aLevelFL(       this, SW_RES(FL_LEVEL    )),
    aLevelLB(       this, SW_RES(LB_LEVEL   )),

    aPositionFL(    this, SW_RES(FL_POSITION )),
    aDistBorderFT(  this, SW_RES(FT_BORDERDIST  )),
    aDistBorderMF(  this, SW_RES(MF_BORDERDIST  )),
    aRelativeCB(    this, SW_RES(CB_RELATIVE     )),
    aIndentFT(      this, SW_RES(FT_INDENT       )),
    aIndentMF(      this, SW_RES(MF_INDENT       )),
    aDistNumFT(     this, SW_RES(FT_NUMDIST     )),
    aDistNumMF(     this, SW_RES(MF_NUMDIST     )),
    aAlignFT(       this, SW_RES(FT_ALIGN    )),
    aAlignLB(       this, SW_RES(LB_ALIGN    )),
    aStandardPB(    this, SW_RES(PB_STANDARD        )),

    aPreviewWIN(    this, SW_RES(WIN_PREVIEW     )),

    pActNum(0),
    pSaveNum(0),
    pWrtSh(0),
    pOutlineDlg(0),
    bPreset( FALSE ),
    bInInintControl(FALSE)
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
void SwNumPositionTabPage::ActivatePage(const SfxItemSet& )
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
int  SwNumPositionTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    SwOutlineTabDialog::SetActNumLevel(nActNumLvl);
    if(_pSet)
        FillItemSet(*_pSet);
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

    nActNumLvl = SwOutlineTabDialog::GetActNumLevel();
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
IMPL_LINK( SwNumPositionTabPage, EditModifyHdl, Edit *, EMPTYARG )
{
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
    long nValue = static_cast< long >(pFld->Denormalize(pFld->GetValue(FUNIT_TWIP)));
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

#if OSL_DEBUG_LEVEL > 1
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
    sRemoveText(SW_RES(ST_RESET)),
    nRetOptionsDialog(USHRT_MAX)
{
    FreeResource();
    GetUserButton()->SetText(sRemoveText);
    GetUserButton()->SetHelpId(HID_NUM_RESET);
    GetUserButton()->SetClickHdl(LINK(this, SwSvxNumBulletTabDialog, RemoveNumberingHdl));
    if(!rWrtSh.GetCurNumRule())
        GetUserButton()->Enable(FALSE);
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

void SwSvxNumBulletTabDialog::PageCreated(USHORT nPageId, SfxTabPage& rPage)
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
            for(USHORT j = 0; j < rCharFmtLB.GetEntryCount(); j++)
            {

                 aList.Insert( new XubString(rCharFmtLB.GetEntry(j)), LIST_APPEND );
            }
            aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;

            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< USHORT >(eMetric) ) );
            rPage.PageCreated(aSet);
            for( USHORT i = (USHORT)aList.Count(); i; --i )
                    delete (XubString*)aList.Remove(i);
            aList.Clear();
        }
        break;
    case RID_SVXPAGE_NUM_POSITION:
        {
            SwDocShell* pDocShell = rWrtSh.GetView().GetDocShell();
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< USHORT >(eMetric)) );
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


