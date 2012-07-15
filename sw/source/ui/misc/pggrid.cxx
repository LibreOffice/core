/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#include <sfx2/app.hxx>

#include <cmdid.h>
#include <hintids.hxx>
#include <swtypes.hxx>
#include <globals.hrc>
#include <svx/xtable.hxx>
#include <uitool.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/ruler.hxx>
#include <pggrid.hxx>
#include <tgrditem.hxx>
#include <pggrid.hrc>

#include "wrtsh.hxx"
#include "doc.hxx"
#include "uiitems.hxx"
#include "swmodule.hxx"
#include "view.hxx"

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
    aLinesRangeFT           (this, SW_RES(FT_LINERANGE    )),
    aTextSizeFT             (this, SW_RES(FT_TEXTSIZE        )),
    aTextSizeMF             (this, SW_RES(MF_TEXTSIZE        )),
    aCharsPerLineFT         (this, SW_RES(FT_CHARSPERLINE    )),
    aCharsPerLineNF         (this, SW_RES(NF_CHARSPERLINE    )),
    aCharsRangeFT           (this, SW_RES(FT_CHARRANGE       )),
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
    m_bSquaredMode(sal_False),
    m_bHRulerChanged( sal_False ),
    m_bVRulerChanged( sal_False )
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
    aControls[17] =&aLinesRangeFT;
    aControls[18] =&aCharsRangeFT;
    aControls[19] =0;

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

    XColorListRef pColorLst = XColorList::GetStdColorList();
    aColorLB.InsertAutomaticEntryColor( Color( COL_AUTO ) );
    for( sal_uInt16 i = 0; i < pColorLst->Count(); ++i )
    {
        XColorEntry* pEntry = pColorLst->GetColor( i );
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();
        aColorLB.InsertEntry( aColor, sName );
    }
    aColorLB.SetUpdateMode( sal_True );
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

SwTextGridPage::~SwTextGridPage()
{
}

SfxTabPage *SwTextGridPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwTextGridPage(pParent, rSet);
}

sal_Bool    SwTextGridPage::FillItemSet(SfxItemSet &rSet)
{
    sal_Bool bRet = sal_False;
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
        bRet = sal_True;
    }

    // draw ticks of ruler
    SwView * pView = ::GetActiveView();
    if ( m_bHRulerChanged )
        pView->GetHLineal().DrawTicks();
    if ( m_bVRulerChanged )
        pView->GetVLineal().DrawTicks();
    return bRet;
}

void    SwTextGridPage::Reset(const SfxItemSet &rSet)
{
    if(SFX_ITEM_AVAILABLE <= rSet.GetItemState(RES_TEXTGRID, sal_True))
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
        aDisplayCB.Check(rGridItem.IsDisplayGrid());
        GridTypeHdl(pButton);
        aSnapToCharsCB.Check(rGridItem.IsSnapToChars());
        aLinesPerPageNF.SetValue(rGridItem.GetLines());
        SetLinesOrCharsRanges( aLinesRangeFT , aLinesPerPageNF.GetMax() );
        m_nRubyUserValue = rGridItem.GetBaseHeight();
        m_bRubyUserValue = sal_True;
        aTextSizeMF.SetValue(aTextSizeMF.Normalize(m_nRubyUserValue), FUNIT_TWIP);
        aRubySizeMF.SetValue(aRubySizeMF.Normalize(rGridItem.GetRubyHeight()), FUNIT_TWIP);
        aCharWidthMF.SetValue(aCharWidthMF.Normalize(rGridItem.GetBaseWidth()), FUNIT_TWIP);
        aRubyBelowCB.Check(rGridItem.IsRubyTextBelow());
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

void    SwTextGridPage::ActivatePage( const SfxItemSet& rSet )
{
    aExampleWN.Hide();
    aExampleWN.UpdateExample( rSet );
    UpdatePageSize(rSet);
    aExampleWN.Show();
    aExampleWN.Invalidate();
}

int SwTextGridPage::DeactivatePage( SfxItemSet* )
{
    return LEAVE_PAGE;
}

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
/// Amelia
            SwView * pView = ::GetActiveView();
        if ( aGridItem.GetGridType() != GRID_NONE )
        {
            if ( aGridItem.GetGridType() == GRID_LINES_CHARS )
            {
                m_bHRulerChanged = sal_True;
            }
            m_bVRulerChanged = sal_True;
            pView->GetHLineal().SetCharWidth((long)(aCharWidthMF.GetValue(FUNIT_TWIP)/56.7));
            pView->GetVLineal().SetLineHeight((long)(aTextSizeMF.GetValue(FUNIT_TWIP)/56.7));
        }
}

void SwTextGridPage::UpdatePageSize(const SfxItemSet& rSet)
{
    if( SFX_ITEM_UNKNOWN !=  rSet.GetItemState( RES_FRAMEDIR, sal_True ))
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
        aCharsPerLineNF.SetMax( aCharsPerLineNF.GetValue() );
            aLinesPerPageNF.SetMax( m_aPageSize.Height() /
        (   aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)) +
                    aRubySizeMF.Denormalize(aRubySizeMF.GetValue(FUNIT_TWIP))));
            SetLinesOrCharsRanges( aCharsRangeFT , aCharsPerLineNF.GetMax() );
            SetLinesOrCharsRanges( aLinesRangeFT , aLinesPerPageNF.GetMax() );
        }
        else
        {
            sal_Int32 nTextWidth = static_cast< sal_Int32 >(aCharWidthMF.Denormalize(aCharWidthMF.GetValue(FUNIT_TWIP)));
            aLinesPerPageNF.SetValue(m_aPageSize.Height() / nTextSize);
            if (nTextWidth)
                aCharsPerLineNF.SetValue(m_aPageSize.Width() / nTextWidth);
            else
                aCharsPerLineNF.SetValue( 45 );
        SetLinesOrCharsRanges( aCharsRangeFT , aCharsPerLineNF.GetMax() );
        SetLinesOrCharsRanges( aLinesRangeFT , aLinesPerPageNF.GetMax() );
        }
    }
}

void SwTextGridPage::SetLinesOrCharsRanges(FixedText & rField, const sal_Int32 nValue )
{
    String aFieldStr = rtl::OUString("( 1 -");
    aFieldStr += rtl::OUString::valueOf(nValue);
    aFieldStr += rtl::OUString(" )");
    rField.SetText( aFieldStr );
}

sal_uInt16* SwTextGridPage::GetRanges()
{
    static sal_uInt16 aPageRg[] = {
        RES_TEXTGRID, RES_TEXTGRID,
        0};
    return aPageRg;
}

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
        SetLinesOrCharsRanges( aLinesRangeFT , aLinesPerPageNF.GetMax() );
    SetLinesOrCharsRanges( aCharsRangeFT , aCharsPerLineNF.GetMax() );
    }
    else//in normal mode
    {
        if(&aLinesPerPageNF == pField)
        {
            long nHeight = static_cast< sal_Int32 >(m_aPageSize.Height() / aLinesPerPageNF.GetValue());
            aTextSizeMF.SetValue(aTextSizeMF.Normalize(nHeight), FUNIT_TWIP);
            aRubySizeMF.SetValue(0, FUNIT_TWIP);
            SetLinesOrCharsRanges( aLinesRangeFT , aLinesPerPageNF.GetMax() );

            m_nRubyUserValue = nHeight;
            m_bRubyUserValue = sal_True;
        }
        else if (&aCharsPerLineNF == pField)
        {
            long nWidth = static_cast< sal_Int32 >(m_aPageSize.Width() / aCharsPerLineNF.GetValue());
            aCharWidthMF.SetValue(aCharWidthMF.Normalize(nWidth), FUNIT_TWIP);
            SetLinesOrCharsRanges( aCharsRangeFT , aCharsPerLineNF.GetMax() );
        }
    }
    GridModifyHdl(0);
    return 0;
}

IMPL_LINK(SwTextGridPage, TextSizeChangedHdl, SpinField*, pField)
{
    //if in squared mode
    if( m_bSquaredMode )
    {
        if (&aTextSizeMF == pField)
        {
            m_bRubyUserValue = sal_False;

            // fdo#50941: set maximum characters per line
            sal_Int32 nTextSize = static_cast< sal_Int32 >(aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)));
            if (nTextSize > 0)
            {
                sal_Int32 nMaxChars = m_aPageSize.Width() / nTextSize;
                aCharsPerLineNF.SetValue(nMaxChars);
                aCharsPerLineNF.SetMax(nMaxChars);
                SetLinesOrCharsRanges( aCharsRangeFT , aCharsPerLineNF.GetMax() );
            }
        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)) +
                    aRubySizeMF.Denormalize(aRubySizeMF.GetValue(FUNIT_TWIP))));
            aLinesPerPageNF.SetMax(nMaxLines);
            SetLinesOrCharsRanges( aLinesRangeFT , aLinesPerPageNF.GetMax() );
        }
    }
    else
    {
        if (&aTextSizeMF == pField)
        {
            sal_Int32 nTextSize = static_cast< sal_Int32 >(aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)));
            aLinesPerPageNF.SetValue(m_aPageSize.Height() / nTextSize);
            m_bRubyUserValue = sal_False;
            SetLinesOrCharsRanges( aLinesRangeFT , aLinesPerPageNF.GetMax() );
        }
        else if (&aCharWidthMF == pField)
        {
            sal_Int32 nTextWidth = static_cast< sal_Int32 >(aCharWidthMF.Denormalize(aCharWidthMF.GetValue(FUNIT_TWIP)));
            sal_Int32 nMaxChar = 45 ;
            if (nTextWidth)
                nMaxChar = m_aPageSize.Width() / nTextWidth;
            aCharsPerLineNF.SetValue( nMaxChar );
            SetLinesOrCharsRanges( aCharsRangeFT , aCharsPerLineNF.GetMax() );
        }
        //rubySize is disabled
    }
    GridModifyHdl(0);
    return 0;
}

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
        aCharsRangeFT.Enable(sal_False);
        aCharWidthFT.Enable(sal_False);
        aCharWidthMF.Enable(sal_False);
    }

    GridModifyHdl(0);
    return 0;
}

IMPL_LINK_NOARG(SwTextGridPage, DisplayGridHdl)
{
    sal_Bool bChecked = aDisplayCB.IsChecked();
    aPrintCB.Enable(bChecked);
    aPrintCB.Check(bChecked);
    return 0;
}

IMPL_LINK_NOARG(SwTextGridPage, GridModifyHdl)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
