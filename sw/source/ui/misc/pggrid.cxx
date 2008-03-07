/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pggrid.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:34:02 $
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



#include <sfx2/app.hxx>

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

#ifndef _XTABLE_HXX //autogen
#include <svx/xtable.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _PGGRID_HXX
#include <pggrid.hxx>
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif
#include <pggrid.hrc>

#include "wrtsh.hxx"
#include "doc.hxx"
#include "uiitems.hxx"
#include "swmodule.hxx"
#include "view.hxx"
/*-- 06.02.2002 15:25:39---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTextGridPage::SwTextGridPage(Window *pParent, const SfxItemSet &rSet) :
    SfxTabPage(pParent, SW_RES(TP_TEXTGRID_PAGE), rSet),
    aGridTypeFL             (this, SW_RES(FL_GRID_TYPE       )),
    aNoGridRB               (this, SW_RES(RB_NOGRID          )),
    aLinesGridRB            (this, SW_RES(RB_LINESGRID       )),
    aCharsGridRB            (this, SW_RES(RB_CHARSGRID       )),
    aSnapToCharsCB          (this, SW_RES(CB_SNAPTOCHARS      )),
    aExampleWN              (this, SW_RES(WN_EXAMPLE         )),
    aLayoutFL               (this, SW_RES(FL_LAYOUT          )),
    aLinesPerPageFT         (this, SW_RES(FT_LINESPERPAGE    )),
    aLinesPerPageNF         (this, SW_RES(NF_LINESPERPAGE    )),
    aTextSizeFT             (this, SW_RES(FT_TEXTSIZE        )),
    aTextSizeMF             (this, SW_RES(MF_TEXTSIZE        )),
    aCharsPerLineFT         (this, SW_RES(FT_CHARSPERLINE    )),
    aCharsPerLineNF         (this, SW_RES(NF_CHARSPERLINE    )),
    aCharWidthFT            (this, SW_RES(FT_CHARWIDTH        )),
    aCharWidthMF            (this, SW_RES(MF_CHARWIDTH        )),
    aRubySizeFT             (this, SW_RES(FT_RUBYSIZE        )),
    aRubySizeMF             (this, SW_RES(MF_RUBYSIZE        )),
    aRubyBelowCB            (this, SW_RES(CB_RUBYBELOW       )),
    aDisplayFL              (this, SW_RES(FL_DISPLAY         )),
    aDisplayCB              (this, SW_RES(CB_DISPLAY         )),
    aPrintCB                (this, SW_RES(CB_PRINT           )),
    aColorFT                (this, SW_RES(FT_COLOR           )),
    aColorLB                (this, SW_RES(LB_COLOR           )),
    m_nRubyUserValue(0),
    m_bRubyUserValue(sal_False),
    m_aPageSize(MM50, MM50),
    m_bVertical(sal_False),
    m_bSquaredMode(sal_False)
{
    FreeResource();

    aControls[0] =&aLayoutFL;
    aControls[1] =&aLinesPerPageFT;
    aControls[2] =&aLinesPerPageNF;
    aControls[3] =&aTextSizeFT;
    aControls[4] =&aTextSizeMF;
    aControls[5] =&aCharsPerLineFT;
    aControls[6] =&aCharsPerLineNF;
    aControls[7] =&aCharWidthFT;
    aControls[8] =&aCharWidthMF;
    aControls[9] =&aRubySizeFT;
    aControls[10] =&aRubySizeMF;
    aControls[11] =&aRubyBelowCB;
    aControls[12] =&aDisplayFL;
    aControls[13] =&aDisplayCB;
    aControls[14] =&aPrintCB;
    aControls[15] =&aColorFT;
    aControls[16] =&aColorLB;
    aControls[17] =0;

    Link aLink = LINK(this, SwTextGridPage, CharorLineChangedHdl);
    aCharsPerLineNF.SetUpHdl(aLink);
    aCharsPerLineNF.SetDownHdl(aLink);
    aCharsPerLineNF.SetLoseFocusHdl(aLink);
    aLinesPerPageNF.SetUpHdl(aLink);
    aLinesPerPageNF.SetDownHdl(aLink);
    aLinesPerPageNF.SetLoseFocusHdl(aLink);

    Link aSizeLink = LINK(this, SwTextGridPage, TextSizeChangedHdl);
    aTextSizeMF.SetUpHdl(aSizeLink);
    aTextSizeMF.SetDownHdl(aSizeLink);
    aTextSizeMF.SetLoseFocusHdl(aSizeLink);
    aRubySizeMF.SetUpHdl(aSizeLink);
    aRubySizeMF.SetDownHdl(aSizeLink);
    aRubySizeMF.SetLoseFocusHdl(aSizeLink);
    aCharWidthMF.SetUpHdl(aSizeLink);
    aCharWidthMF.SetDownHdl(aSizeLink);
    aCharWidthMF.SetLoseFocusHdl(aSizeLink);

    Link aGridTypeHdl = LINK(this, SwTextGridPage, GridTypeHdl);
    aNoGridRB   .SetClickHdl(aGridTypeHdl);
    aLinesGridRB.SetClickHdl(aGridTypeHdl);
    aCharsGridRB.SetClickHdl(aGridTypeHdl);

    Link aModifyLk = LINK(this, SwTextGridPage, GridModifyHdl);
    aColorLB.SetSelectHdl(aModifyLk);
    aPrintCB.SetClickHdl(aModifyLk);
    aRubyBelowCB.SetClickHdl(aModifyLk);


    aDisplayCB.SetClickHdl(LINK(this, SwTextGridPage, DisplayGridHdl));

    XColorTable* pColorTbl = XColorTable::GetStdColorTable();
    aColorLB.InsertAutomaticEntry();
    for( USHORT i = 0; i < pColorTbl->Count(); ++i )
    {
        XColorEntry* pEntry = pColorTbl->GetColor( i );
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();
        aColorLB.InsertEntry( aColor, sName );
    }
    aColorLB.SetUpdateMode( TRUE );
    //Get the default paper mode
    SwView *pView   = ::GetActiveView();
    if( pView )
    {
        SwWrtShell* pSh = pView->GetWrtShellPtr();
        if( pSh )
        {
            m_bSquaredMode = pSh->GetDoc()->IsSquaredPageMode();
        }
    }
    if( m_bSquaredMode )
    {
        aRubySizeFT.Show();
        aRubySizeMF.Show();
        aRubyBelowCB.Show();
        aSnapToCharsCB.Hide();
        aCharWidthFT.Hide();
        aCharWidthMF.Hide();
    }
    else
    {
        aRubySizeFT.Hide();
        aRubySizeMF.Hide();
        aRubyBelowCB.Hide();
        aSnapToCharsCB.Show();
        aCharWidthFT.Show();
        aCharWidthMF.Show();
    }
}
/*-- 06.02.2002 15:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTextGridPage::~SwTextGridPage()
{
}
/*-- 06.02.2002 15:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage *SwTextGridPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwTextGridPage(pParent, rSet);
}
/*-- 06.02.2002 15:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL    SwTextGridPage::FillItemSet(SfxItemSet &rSet)
{
    BOOL bRet = FALSE;
    if(aNoGridRB.GetSavedValue() != aNoGridRB.IsChecked()||
        aLinesGridRB.GetSavedValue() != aLinesGridRB.IsChecked()||
        aLinesPerPageNF.GetSavedValue().ToInt32()
           != aLinesPerPageNF.GetValue()||
        aTextSizeMF.GetSavedValue().ToInt32() != aTextSizeMF.GetValue()||
        aCharsPerLineNF.GetSavedValue().ToInt32()
           != aCharsPerLineNF.GetValue()||
        aSnapToCharsCB.GetSavedValue() != aSnapToCharsCB.IsChecked() ||
        aRubySizeMF.GetSavedValue().ToInt32() != aRubySizeMF.GetValue()||
        aCharWidthMF.GetSavedValue().ToInt32() != aCharWidthMF.GetValue()||
        aRubyBelowCB.GetSavedValue() != aRubyBelowCB.IsChecked()||
        aDisplayCB.GetSavedValue() != aDisplayCB.IsChecked()||
        aPrintCB.GetSavedValue() != aPrintCB.IsChecked()||
        aColorLB.GetSavedValue() != aColorLB.GetSelectEntryPos())
    {
        PutGridItem(rSet);
        bRet = TRUE;
    }

    return bRet;
}
/*-- 06.02.2002 15:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwTextGridPage::Reset(const SfxItemSet &rSet)
{
    if(SFX_ITEM_AVAILABLE <= rSet.GetItemState(RES_TEXTGRID, TRUE))
    {
        const SwTextGridItem& rGridItem = (const SwTextGridItem&)rSet.Get(RES_TEXTGRID);
        RadioButton* pButton = 0;
        switch(rGridItem.GetGridType())
        {
            case GRID_NONE :        pButton = &aNoGridRB;    break;
            case GRID_LINES_ONLY  : pButton = &aLinesGridRB; break;
            default:                pButton = &aCharsGridRB;
        }
        pButton->Check();
        GridTypeHdl(pButton);
        aSnapToCharsCB.Check(rGridItem.IsSnapToChars());
        aLinesPerPageNF.SetValue(rGridItem.GetLines());
        m_nRubyUserValue = rGridItem.GetBaseHeight();
        m_bRubyUserValue = sal_True;
        aTextSizeMF.SetValue(aTextSizeMF.Normalize(m_nRubyUserValue), FUNIT_TWIP);
        aRubySizeMF.SetValue(aRubySizeMF.Normalize(rGridItem.GetRubyHeight()), FUNIT_TWIP);
        aCharWidthMF.SetValue(aCharWidthMF.Normalize(rGridItem.GetBaseWidth()), FUNIT_TWIP);
        aRubyBelowCB.Check(rGridItem.IsRubyTextBelow());
        aDisplayCB.Check(rGridItem.IsDisplayGrid());
        aPrintCB.Check(rGridItem.IsPrintGrid());
        aColorLB.SelectEntry(rGridItem.GetColor());
    }
    UpdatePageSize(rSet);

    aNoGridRB.SaveValue();
    aLinesGridRB.SaveValue();
    aSnapToCharsCB.SaveValue();
    aLinesPerPageNF.SaveValue();
    aTextSizeMF.SaveValue();
    aCharsPerLineNF.SaveValue();
    aRubySizeMF.SaveValue();
    aCharWidthMF.SaveValue();
    aRubyBelowCB.SaveValue();
    aDisplayCB.SaveValue();
    aPrintCB.SaveValue();
    aColorLB.SaveValue();
}
/*-- 06.02.2002 15:25:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwTextGridPage::ActivatePage( const SfxItemSet& rSet )
{
    aExampleWN.Hide();
    aExampleWN.UpdateExample( rSet );
    UpdatePageSize(rSet);
    aExampleWN.Show();
    aExampleWN.Invalidate();
}
/*-- 06.02.2002 15:25:41---------------------------------------------------

  -----------------------------------------------------------------------*/
int SwTextGridPage::DeactivatePage( SfxItemSet* )
{
    return LEAVE_PAGE;
}
/* -----------------------------08.02.2002 11:57------------------------------

 ---------------------------------------------------------------------------*/
void SwTextGridPage::PutGridItem(SfxItemSet& rSet)
{
        SwTextGridItem aGridItem;
        aGridItem.SetGridType(aNoGridRB.IsChecked() ? GRID_NONE :
            aLinesGridRB.IsChecked() ? GRID_LINES_ONLY : GRID_LINES_CHARS );
        aGridItem.SetSnapToChars(aSnapToCharsCB.IsChecked());
        aGridItem.SetLines( static_cast< sal_uInt16 >(aLinesPerPageNF.GetValue()) );
        aGridItem.SetBaseHeight( static_cast< sal_uInt16 >(
            m_bRubyUserValue ? m_nRubyUserValue :
                aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP))) );
        aGridItem.SetRubyHeight( static_cast< sal_uInt16 >(aRubySizeMF.Denormalize(aRubySizeMF.GetValue(FUNIT_TWIP))) );
        aGridItem.SetBaseWidth( static_cast< sal_uInt16 >(aCharWidthMF.Denormalize(aCharWidthMF.GetValue(FUNIT_TWIP))) );
        aGridItem.SetRubyTextBelow(aRubyBelowCB.IsChecked());
        aGridItem.SetSquaredMode(m_bSquaredMode);
        aGridItem.SetDisplayGrid(aDisplayCB.IsChecked());
        aGridItem.SetPrintGrid(aPrintCB.IsChecked());
        aGridItem.SetColor(aColorLB.GetSelectEntryColor());
        rSet.Put(aGridItem);
}
/* -----------------------------08.02.2002 10:54------------------------------

 ---------------------------------------------------------------------------*/
void SwTextGridPage::UpdatePageSize(const SfxItemSet& rSet)
{
    if( SFX_ITEM_UNKNOWN !=  rSet.GetItemState( RES_FRAMEDIR, TRUE ))
    {
        const SvxFrameDirectionItem& rDirItem =
                    (const SvxFrameDirectionItem&)rSet.Get(RES_FRAMEDIR);
        m_bVertical = rDirItem.GetValue() == FRMDIR_VERT_TOP_RIGHT||
                    rDirItem.GetValue() == FRMDIR_VERT_TOP_LEFT;
    }

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PAGE_SIZE ))
    {
        const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get(
                                            SID_ATTR_PAGE_SIZE);
        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)rSet.Get(
                                                            RES_LR_SPACE );
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)rSet.Get(
                                                            RES_UL_SPACE );
        const SvxBoxItem& rBox = (const SvxBoxItem&) rSet.Get(RES_BOX);
        sal_Int32 nDistanceLR = rLRSpace.GetLeft() + rLRSpace.GetRight();
        sal_Int32 nDistanceUL = rULSpace.GetUpper() + rULSpace.GetLower();

        sal_Int32 nValue1 = rSize.GetSize().Height() - nDistanceUL -
                rBox.GetDistance(BOX_LINE_TOP) -
                                    rBox.GetDistance(BOX_LINE_BOTTOM);
        sal_Int32 nValue2 = rSize.GetSize().Width() - nDistanceLR -
                rBox.GetDistance(BOX_LINE_LEFT) -
                                    rBox.GetDistance(BOX_LINE_RIGHT);
        if(m_bVertical)
        {
            m_aPageSize.Width() = nValue1;
            m_aPageSize.Height() = nValue2;
        }
        else
        {
            m_aPageSize.Width() = nValue2;
            m_aPageSize.Height() = nValue1;
        }

        sal_Int32 nTextSize = static_cast< sal_Int32 >(m_bRubyUserValue ?
                    m_nRubyUserValue :
                        aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)));

        if ( m_bSquaredMode )
        {
            aCharsPerLineNF.SetValue(m_aPageSize.Width() / nTextSize);
        }
        else
        {
            sal_Int32 nTextWidth = static_cast< sal_Int32 >(aCharWidthMF.Denormalize(aCharWidthMF.GetValue(FUNIT_TWIP)));
            aLinesPerPageNF.SetValue(m_aPageSize.Height() / nTextSize);
            if (nTextWidth)
                aCharsPerLineNF.SetValue(m_aPageSize.Width() / nTextWidth);
            else
                aCharsPerLineNF.SetValue( 45 );
        }
    }
}
/* -----------------------------06.02.2002 15:24------------------------------

 ---------------------------------------------------------------------------*/
USHORT* SwTextGridPage::GetRanges()
{
    static USHORT __FAR_DATA aPageRg[] = {
        RES_TEXTGRID, RES_TEXTGRID,
        0};
    return aPageRg;
}
/* -----------------------------08.02.2002 10:56------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTextGridPage, CharorLineChangedHdl, SpinField*, pField)
{
    //if in squared mode
    if ( m_bSquaredMode )
    {
        if(&aCharsPerLineNF == pField)
        {
            long nWidth = (long)(m_aPageSize.Width() / aCharsPerLineNF.GetValue());
            aTextSizeMF.SetValue(aTextSizeMF.Normalize(nWidth), FUNIT_TWIP);
            //prevent rounding errors in the MetricField by saving the used value
            m_nRubyUserValue = nWidth;
            m_bRubyUserValue = sal_True;

        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)) +
                    aRubySizeMF.Denormalize(aRubySizeMF.GetValue(FUNIT_TWIP))));
            aLinesPerPageNF.SetMax(nMaxLines);
        }
    }
    else//in normal mode
    {
        if(&aLinesPerPageNF == pField)
        {
            long nHeight = static_cast< sal_Int32 >(m_aPageSize.Height() / aLinesPerPageNF.GetValue());
            aTextSizeMF.SetValue(aTextSizeMF.Normalize(nHeight), FUNIT_TWIP);
            aRubySizeMF.SetValue(0, FUNIT_TWIP);

            m_nRubyUserValue = nHeight;
            m_bRubyUserValue = sal_True;
        }
        else if (&aCharsPerLineNF == pField)
        {
            long nWidth = static_cast< sal_Int32 >(m_aPageSize.Width() / aCharsPerLineNF.GetValue());
            aCharWidthMF.SetValue(aCharWidthMF.Normalize(nWidth), FUNIT_TWIP);
        }
    }
    GridModifyHdl(0);
    return 0;
}
/* -----------------------------04.09.2006 15:46------------------------------
 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTextGridPage, TextSizeChangedHdl, SpinField*, pField)
{
    //if in squared mode
    if( m_bSquaredMode )
    {
        if (&aTextSizeMF == pField)
        {
            sal_Int32 nTextSize = static_cast< sal_Int32 >(aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)));
            aCharsPerLineNF.SetValue(m_aPageSize.Width() / nTextSize);
            m_bRubyUserValue = sal_False;
        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)) +
                    aRubySizeMF.Denormalize(aRubySizeMF.GetValue(FUNIT_TWIP))));
            aLinesPerPageNF.SetMax(nMaxLines);
        }
    }
    else
    {
        if (&aTextSizeMF == pField)
        {
            sal_Int32 nTextSize = static_cast< sal_Int32 >(aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)));
            aLinesPerPageNF.SetValue(m_aPageSize.Height() / nTextSize);
            m_bRubyUserValue = sal_False;
        }
        else if (&aCharWidthMF == pField)
        {
            sal_Int32 nTextWidth = static_cast< sal_Int32 >(aCharWidthMF.Denormalize(aCharWidthMF.GetValue(FUNIT_TWIP)));
            if (nTextWidth)
                aCharsPerLineNF.SetValue(m_aPageSize.Width() / nTextWidth);
            else
                aCharsPerLineNF.SetValue( 45 );
        }
        //rubySize is disabled
    }
    GridModifyHdl(0);
    return 0;
}
/* -----------------------------22.04.2002 14:53------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTextGridPage, GridTypeHdl, RadioButton*, pButton)
{
    sal_Bool bEnable = &aNoGridRB != pButton;
    sal_Int32 nIndex = 0;
    while(aControls[nIndex])
        aControls[nIndex++]->Enable(bEnable);
    //one special case
    if(bEnable)
        DisplayGridHdl(&aDisplayCB);

    bEnable = &aCharsGridRB == pButton;
    aSnapToCharsCB.Enable(bEnable);

    bEnable = &aLinesGridRB == pButton;
    if(bEnable && !m_bSquaredMode )
    {
        aCharsPerLineFT.Enable(sal_False);
        aCharsPerLineNF.Enable(sal_False);
        aCharWidthFT.Enable(sal_False);
        aCharWidthMF.Enable(sal_False);
    }

    GridModifyHdl(0);
    return 0;
}
/* -----------------------------22.04.2002 15:46------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTextGridPage, DisplayGridHdl, CheckBox*, EMPTYARG)
{
    aPrintCB.Enable(aDisplayCB.IsChecked());
    return 0;
}
/* -----------------------------08.02.2002 11:54------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwTextGridPage, GridModifyHdl, void*, EMPTYARG)
{
    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemSet aSet(rOldSet);
    const SfxItemSet* pExSet = GetTabDialog()->GetExampleSet();
    if(pExSet)
        aSet.Put(*pExSet);
    PutGridItem(aSet);
    aExampleWN.UpdateExample(aSet);
    return 0;
}

