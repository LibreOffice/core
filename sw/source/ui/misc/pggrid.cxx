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

#include "wrtsh.hxx"
#include "doc.hxx"
#include "uiitems.hxx"
#include "swmodule.hxx"
#include "view.hxx"

SwTextGridPage::SwTextGridPage(vcl::Window *pParent, const SfxItemSet &rSet) :
    SfxTabPage(pParent, "TextGridPage", "modules/swriter/ui/textgridpage.ui", &rSet),
    m_nRubyUserValue(0),
    m_bRubyUserValue(false),
    m_aPageSize(MM50, MM50),
    m_bVertical(false),
    m_bSquaredMode(false),
    m_bHRulerChanged( false ),
    m_bVRulerChanged( false )
{
    get(m_pNoGridRB,"radioRB_NOGRID");
    get(m_pLinesGridRB,"radioRB_LINESGRID");
    get(m_pCharsGridRB,"radioRB_CHARSGRID");
    get(m_pSnapToCharsCB,"checkCB_SNAPTOCHARS");

    get(m_pExampleWN,"drawingareaWN_EXAMPLE");
    get(m_pLayoutFL,"frameFL_LAYOUT");

    get(m_pLinesPerPageNF,"spinNF_LINESPERPAGE");
    get(m_pLinesRangeFT,"labelFT_LINERANGE");

    get(m_pTextSizeMF,"spinMF_TEXTSIZE");
    get(m_pCharsPerLineFT,"labelFT_CHARSPERLINE");
    get(m_pCharsPerLineNF,"spinNF_CHARSPERLINE");
    get(m_pCharsRangeFT,"labelFT_CHARRANGE");
    get(m_pCharWidthFT,"labelFT_CHARWIDTH");
    get(m_pCharWidthMF,"spinMF_CHARWIDTH");
    get(m_pRubySizeFT,"labelFT_RUBYSIZE");
    get(m_pRubySizeMF,"spinMF_RUBYSIZE");
    get(m_pRubyBelowCB,"checkCB_RUBYBELOW");

    get(m_pDisplayFL,"frameFL_DISPLAY");
    get(m_pDisplayCB,"checkCB_DISPLAY");
    get(m_pPrintCB,"checkCB_PRINT");
    get(m_pColorLB,"listLB_COLOR");

    Link<SpinField&,void> aLink = LINK(this, SwTextGridPage, CharorLineChangedHdl);
    Link<Control&,void> aLink2 = LINK(this, SwTextGridPage, CharorLineLoseFocusdHdl);
    m_pCharsPerLineNF->SetUpHdl(aLink);
    m_pCharsPerLineNF->SetDownHdl(aLink);
    m_pCharsPerLineNF->SetLoseFocusHdl(aLink2);
    m_pLinesPerPageNF->SetUpHdl(aLink);
    m_pLinesPerPageNF->SetDownHdl(aLink);
    m_pLinesPerPageNF->SetLoseFocusHdl(aLink2);

    Link<SpinField&,void> aSizeLink = LINK(this, SwTextGridPage, TextSizeChangedHdl);
    Link<Control&,void> aSizeLink2 = LINK(this, SwTextGridPage, TextSizeLoseFocusHdl);
    m_pTextSizeMF->SetUpHdl(aSizeLink);
    m_pTextSizeMF->SetDownHdl(aSizeLink);
    m_pTextSizeMF->SetLoseFocusHdl(aSizeLink2);
    m_pRubySizeMF->SetUpHdl(aSizeLink);
    m_pRubySizeMF->SetDownHdl(aSizeLink);
    m_pRubySizeMF->SetLoseFocusHdl(aSizeLink2);
    m_pCharWidthMF->SetUpHdl(aSizeLink);
    m_pCharWidthMF->SetDownHdl(aSizeLink);
    m_pCharWidthMF->SetLoseFocusHdl(aSizeLink2);

    Link<Button*,void> aGridTypeHdl = LINK(this, SwTextGridPage, GridTypeHdl);
    m_pNoGridRB->SetClickHdl(aGridTypeHdl);
    m_pLinesGridRB->SetClickHdl(aGridTypeHdl);
    m_pCharsGridRB->SetClickHdl(aGridTypeHdl);

    m_pColorLB->SetSelectHdl(LINK(this, SwTextGridPage, GridModifyHdl));
    m_pPrintCB->SetClickHdl(LINK(this, SwTextGridPage, GridModifyClickHdl));
    m_pRubyBelowCB->SetClickHdl(LINK(this, SwTextGridPage, GridModifyClickHdl));

    m_pDisplayCB->SetClickHdl(LINK(this, SwTextGridPage, DisplayGridHdl));

    XColorListRef pColorLst = XColorList::GetStdColorList();
    m_pColorLB->InsertAutomaticEntryColor( Color( COL_AUTO ) );
    const long nCount = pColorLst->Count();
    for( long i = 0; i < nCount; ++i )
    {
        XColorEntry* pEntry = pColorLst->GetColor( i );
        Color aColor = pEntry->GetColor();
        OUString sName = pEntry->GetName();
        m_pColorLB->InsertEntry( aColor, sName );
    }
    m_pColorLB->SetUpdateMode( true );
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

        m_pRubySizeFT->Show();
        m_pRubySizeMF->Show();
        m_pRubyBelowCB->Show();
        m_pSnapToCharsCB->Hide();
        m_pCharWidthFT->Hide();
        m_pCharWidthMF->Hide();
    }
    else
    {
        m_pRubySizeFT->Hide();
        m_pRubySizeMF->Hide();
        m_pRubyBelowCB->Hide();
        m_pSnapToCharsCB->Show();
        m_pCharWidthFT->Show();
        m_pCharWidthMF->Show();
    }
}

SwTextGridPage::~SwTextGridPage()
{
    disposeOnce();
}

void SwTextGridPage::dispose()
{
    m_pNoGridRB.clear();
    m_pLinesGridRB.clear();
    m_pCharsGridRB.clear();
    m_pSnapToCharsCB.clear();
    m_pExampleWN.clear();
    m_pLayoutFL.clear();
    m_pLinesPerPageNF.clear();
    m_pLinesRangeFT.clear();
    m_pTextSizeMF.clear();
    m_pCharsPerLineFT.clear();
    m_pCharsPerLineNF.clear();
    m_pCharsRangeFT.clear();
    m_pCharWidthFT.clear();
    m_pCharWidthMF.clear();
    m_pRubySizeFT.clear();
    m_pRubySizeMF.clear();
    m_pRubyBelowCB.clear();
    m_pDisplayFL.clear();
    m_pDisplayCB.clear();
    m_pPrintCB.clear();
    m_pColorLB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwTextGridPage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwTextGridPage>::Create(pParent, *rSet);
}

bool SwTextGridPage::FillItemSet(SfxItemSet *rSet)
{
    bool bRet = false;
    if(m_pNoGridRB->IsValueChangedFromSaved() ||
        m_pLinesGridRB->IsValueChangedFromSaved() ||
        m_pLinesPerPageNF->IsValueChangedFromSaved() ||
        m_pTextSizeMF->IsValueChangedFromSaved() ||
        m_pCharsPerLineNF->IsValueChangedFromSaved() ||
        m_pSnapToCharsCB->IsValueChangedFromSaved() ||
        m_pRubySizeMF->IsValueChangedFromSaved() ||
        m_pCharWidthMF->IsValueChangedFromSaved() ||
        m_pRubyBelowCB->IsValueChangedFromSaved() ||
        m_pDisplayCB->IsValueChangedFromSaved() ||
        m_pPrintCB->IsValueChangedFromSaved() ||
        m_pColorLB->IsValueChangedFromSaved())
    {
        PutGridItem(*rSet);
        bRet = true;
    }

    // draw ticks of ruler
    SwView * pView = ::GetActiveView();
    if ( m_bHRulerChanged )
        pView->GetHRuler().DrawTicks();
    if ( m_bVRulerChanged )
        pView->GetVRuler().DrawTicks();
    return bRet;
}

void    SwTextGridPage::Reset(const SfxItemSet *rSet)
{
    if(SfxItemState::DEFAULT <= rSet->GetItemState(RES_TEXTGRID))
    {
        const SwTextGridItem& rGridItem = static_cast<const SwTextGridItem&>(rSet->Get(RES_TEXTGRID));
        RadioButton* pButton = nullptr;
        switch(rGridItem.GetGridType())
        {
            case GRID_NONE :        pButton = m_pNoGridRB;    break;
            case GRID_LINES_ONLY  : pButton = m_pLinesGridRB; break;
            default:                pButton = m_pCharsGridRB;
        }
        pButton->Check();
        m_pDisplayCB->Check(rGridItem.IsDisplayGrid());
        GridTypeHdl(pButton);
        m_pSnapToCharsCB->Check(rGridItem.IsSnapToChars());
        m_pLinesPerPageNF->SetValue(rGridItem.GetLines());
        SetLinesOrCharsRanges( *m_pLinesRangeFT , m_pLinesPerPageNF->GetMax() );
        m_nRubyUserValue = rGridItem.GetBaseHeight();
        m_bRubyUserValue = true;
        m_pTextSizeMF->SetValue(m_pTextSizeMF->Normalize(m_nRubyUserValue), FUNIT_TWIP);
        m_pRubySizeMF->SetValue(m_pRubySizeMF->Normalize(rGridItem.GetRubyHeight()), FUNIT_TWIP);
        m_pCharWidthMF->SetValue(m_pCharWidthMF->Normalize(rGridItem.GetBaseWidth()), FUNIT_TWIP);
        m_pRubyBelowCB->Check(rGridItem.IsRubyTextBelow());
        m_pPrintCB->Check(rGridItem.IsPrintGrid());
        m_pColorLB->SelectEntry(rGridItem.GetColor());
    }
    UpdatePageSize(*rSet);

    m_pNoGridRB->SaveValue();
    m_pLinesGridRB->SaveValue();
    m_pSnapToCharsCB->SaveValue();
    m_pLinesPerPageNF->SaveValue();
    m_pTextSizeMF->SaveValue();
    m_pCharsPerLineNF->SaveValue();
    m_pRubySizeMF->SaveValue();
    m_pCharWidthMF->SaveValue();
    m_pRubyBelowCB->SaveValue();
    m_pDisplayCB->SaveValue();
    m_pPrintCB->SaveValue();
    m_pColorLB->SaveValue();
}

void    SwTextGridPage::ActivatePage( const SfxItemSet& rSet )
{
    m_pExampleWN->Hide();
    m_pExampleWN->UpdateExample( rSet );
    UpdatePageSize(rSet);
    m_pExampleWN->Show();
    m_pExampleWN->Invalidate();
}

SfxTabPage::sfxpg SwTextGridPage::DeactivatePage( SfxItemSet* )
{
    return LEAVE_PAGE;
}

void SwTextGridPage::PutGridItem(SfxItemSet& rSet)
{
        SwTextGridItem aGridItem;
        aGridItem.SetGridType(m_pNoGridRB->IsChecked() ? GRID_NONE :
            m_pLinesGridRB->IsChecked() ? GRID_LINES_ONLY : GRID_LINES_CHARS );
        aGridItem.SetSnapToChars(m_pSnapToCharsCB->IsChecked());
        aGridItem.SetLines( static_cast< sal_uInt16 >(m_pLinesPerPageNF->GetValue()) );
        aGridItem.SetBaseHeight( static_cast< sal_uInt16 >(
            m_bRubyUserValue ? m_nRubyUserValue :
                m_pTextSizeMF->Denormalize(m_pTextSizeMF->GetValue(FUNIT_TWIP))) );
        aGridItem.SetRubyHeight( static_cast< sal_uInt16 >(m_pRubySizeMF->Denormalize(m_pRubySizeMF->GetValue(FUNIT_TWIP))) );
        aGridItem.SetBaseWidth( static_cast< sal_uInt16 >(m_pCharWidthMF->Denormalize(m_pCharWidthMF->GetValue(FUNIT_TWIP))) );
        aGridItem.SetRubyTextBelow(m_pRubyBelowCB->IsChecked());
        aGridItem.SetSquaredMode(m_bSquaredMode);
        aGridItem.SetDisplayGrid(m_pDisplayCB->IsChecked());
        aGridItem.SetPrintGrid(m_pPrintCB->IsChecked());
        aGridItem.SetColor(m_pColorLB->GetSelectEntryColor());
        rSet.Put(aGridItem);

        SwView * pView = ::GetActiveView();
        if ( aGridItem.GetGridType() != GRID_NONE )
        {
            if ( aGridItem.GetGridType() == GRID_LINES_CHARS )
            {
                m_bHRulerChanged = true;
            }
            m_bVRulerChanged = true;
            pView->GetHRuler().SetCharWidth((long)(m_pCharWidthMF->GetValue(FUNIT_TWIP)/56.7));
            pView->GetVRuler().SetLineHeight((long)(m_pTextSizeMF->GetValue(FUNIT_TWIP)/56.7));
        }
}

void SwTextGridPage::UpdatePageSize(const SfxItemSet& rSet)
{
    if( SfxItemState::UNKNOWN !=  rSet.GetItemState( RES_FRAMEDIR ))
    {
        const SvxFrameDirectionItem& rDirItem =
                    static_cast<const SvxFrameDirectionItem&>(rSet.Get(RES_FRAMEDIR));
        m_bVertical = rDirItem.GetValue() == FRMDIR_VERT_TOP_RIGHT||
                    rDirItem.GetValue() == FRMDIR_VERT_TOP_LEFT;
    }

    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_PAGE_SIZE ))
    {
        const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(rSet.Get(
                                            SID_ATTR_PAGE_SIZE));
        const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(rSet.Get(
                                                            RES_LR_SPACE ));
        const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>(rSet.Get(
                                                            RES_UL_SPACE ));
        const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>( rSet.Get(RES_BOX));
        sal_Int32 nDistanceLR = rLRSpace.GetLeft() + rLRSpace.GetRight();
        sal_Int32 nDistanceUL = rULSpace.GetUpper() + rULSpace.GetLower();

        sal_Int32 nValue1 = rSize.GetSize().Height() - nDistanceUL -
                rBox.GetDistance(SvxBoxItemLine::TOP) -
                                    rBox.GetDistance(SvxBoxItemLine::BOTTOM);
        sal_Int32 nValue2 = rSize.GetSize().Width() - nDistanceLR -
                rBox.GetDistance(SvxBoxItemLine::LEFT) -
                                    rBox.GetDistance(SvxBoxItemLine::RIGHT);
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
                        m_pTextSizeMF->Denormalize(m_pTextSizeMF->GetValue(FUNIT_TWIP)));

        if ( m_bSquaredMode )
        {
            m_pCharsPerLineNF->SetValue(m_aPageSize.Width() / nTextSize);
        m_pCharsPerLineNF->SetMax( m_pCharsPerLineNF->GetValue() );
            m_pLinesPerPageNF->SetMax( m_aPageSize.Height() /
        (   m_pTextSizeMF->Denormalize(m_pTextSizeMF->GetValue(FUNIT_TWIP)) +
                    m_pRubySizeMF->Denormalize(m_pRubySizeMF->GetValue(FUNIT_TWIP))));
            SetLinesOrCharsRanges( *m_pCharsRangeFT , m_pCharsPerLineNF->GetMax() );
            SetLinesOrCharsRanges( *m_pLinesRangeFT , m_pLinesPerPageNF->GetMax() );
        }
        else
        {
            sal_Int32 nTextWidth = static_cast< sal_Int32 >(m_pCharWidthMF->Denormalize(m_pCharWidthMF->GetValue(FUNIT_TWIP)));
            m_pLinesPerPageNF->SetValue(m_aPageSize.Height() / nTextSize);
            if (nTextWidth)
                m_pCharsPerLineNF->SetValue(m_aPageSize.Width() / nTextWidth);
            else
                m_pCharsPerLineNF->SetValue( 45 );
        SetLinesOrCharsRanges( *m_pCharsRangeFT , m_pCharsPerLineNF->GetMax() );
        SetLinesOrCharsRanges( *m_pLinesRangeFT , m_pLinesPerPageNF->GetMax() );
        }
    }
}

void SwTextGridPage::SetLinesOrCharsRanges(FixedText & rField, const sal_Int32 nValue )
{
    OUString aFieldStr("( 1 -");
    aFieldStr += OUString::number(nValue);
    aFieldStr += " )";
    rField.SetText( aFieldStr );
}

const sal_uInt16* SwTextGridPage::GetRanges()
{
    static const sal_uInt16 aPageRg[] = {
        RES_TEXTGRID, RES_TEXTGRID,
        0};
    return aPageRg;
}

IMPL_LINK_TYPED(SwTextGridPage, CharorLineLoseFocusdHdl, Control&, rControl, void)
{
    CharorLineChangedHdl(static_cast<SpinField&>(rControl));
}
IMPL_LINK_TYPED(SwTextGridPage, CharorLineChangedHdl, SpinField&, rField, void)
{
    //if in squared mode
    if ( m_bSquaredMode )
    {
        if(m_pCharsPerLineNF == &rField)
        {
            long nWidth = (long)(m_aPageSize.Width() / m_pCharsPerLineNF->GetValue());
            m_pTextSizeMF->SetValue(m_pTextSizeMF->Normalize(nWidth), FUNIT_TWIP);
            //prevent rounding errors in the MetricField by saving the used value
            m_nRubyUserValue = nWidth;
            m_bRubyUserValue = true;

        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   m_pTextSizeMF->Denormalize(m_pTextSizeMF->GetValue(FUNIT_TWIP)) +
                    m_pRubySizeMF->Denormalize(m_pRubySizeMF->GetValue(FUNIT_TWIP))));
            m_pLinesPerPageNF->SetMax(nMaxLines);
        }
        SetLinesOrCharsRanges( *m_pLinesRangeFT , m_pLinesPerPageNF->GetMax() );
        SetLinesOrCharsRanges( *m_pCharsRangeFT , m_pCharsPerLineNF->GetMax() );
    }
    else//in normal mode
    {
        if(m_pLinesPerPageNF == &rField)
        {
            long nHeight = static_cast< sal_Int32 >(m_aPageSize.Height() / m_pLinesPerPageNF->GetValue());
            m_pTextSizeMF->SetValue(m_pTextSizeMF->Normalize(nHeight), FUNIT_TWIP);
            m_pRubySizeMF->SetValue(0, FUNIT_TWIP);
            SetLinesOrCharsRanges( *m_pLinesRangeFT , m_pLinesPerPageNF->GetMax() );

            m_nRubyUserValue = nHeight;
            m_bRubyUserValue = true;
        }
        else if (m_pCharsPerLineNF == &rField)
        {
            long nWidth = static_cast< sal_Int32 >(m_aPageSize.Width() / m_pCharsPerLineNF->GetValue());
            m_pCharWidthMF->SetValue(m_pCharWidthMF->Normalize(nWidth), FUNIT_TWIP);
            SetLinesOrCharsRanges( *m_pCharsRangeFT , m_pCharsPerLineNF->GetMax() );
        }
    }
    GridModifyHdl(*m_pColorLB);
}

IMPL_LINK_TYPED(SwTextGridPage, TextSizeLoseFocusHdl, Control&, rControl, void)
{
    TextSizeChangedHdl(static_cast<SpinField&>(rControl));
}
IMPL_LINK_TYPED(SwTextGridPage, TextSizeChangedHdl, SpinField&, rField, void)
{
    //if in squared mode
    if( m_bSquaredMode )
    {
        if (m_pTextSizeMF == &rField)
        {
            m_bRubyUserValue = false;

            // fdo#50941: set maximum characters per line
            sal_Int32 nTextSize = static_cast< sal_Int32 >(m_pTextSizeMF->Denormalize(m_pTextSizeMF->GetValue(FUNIT_TWIP)));
            if (nTextSize > 0)
            {
                sal_Int32 nMaxChars = m_aPageSize.Width() / nTextSize;
                m_pCharsPerLineNF->SetValue(nMaxChars);
                m_pCharsPerLineNF->SetMax(nMaxChars);
                SetLinesOrCharsRanges( *m_pCharsRangeFT , m_pCharsPerLineNF->GetMax() );
            }
        }
        //set maximum line per page
        {
            sal_Int32 nMaxLines = static_cast< sal_Int32 >(m_aPageSize.Height() /
                (   m_pTextSizeMF->Denormalize(m_pTextSizeMF->GetValue(FUNIT_TWIP)) +
                    m_pRubySizeMF->Denormalize(m_pRubySizeMF->GetValue(FUNIT_TWIP))));
            m_pLinesPerPageNF->SetMax(nMaxLines);
            SetLinesOrCharsRanges( *m_pLinesRangeFT , m_pLinesPerPageNF->GetMax() );
        }
    }
    else
    {
        if (m_pTextSizeMF == &rField)
        {
            sal_Int32 nTextSize = static_cast< sal_Int32 >(m_pTextSizeMF->Denormalize(m_pTextSizeMF->GetValue(FUNIT_TWIP)));
            m_pLinesPerPageNF->SetValue(m_aPageSize.Height() / nTextSize);
            m_bRubyUserValue = false;
            SetLinesOrCharsRanges( *m_pLinesRangeFT , m_pLinesPerPageNF->GetMax() );
        }
        else if (m_pCharWidthMF == &rField)
        {
            sal_Int32 nTextWidth = static_cast< sal_Int32 >(m_pCharWidthMF->Denormalize(m_pCharWidthMF->GetValue(FUNIT_TWIP)));
            sal_Int32 nMaxChar = 45 ;
            if (nTextWidth)
                nMaxChar = m_aPageSize.Width() / nTextWidth;
            m_pCharsPerLineNF->SetValue( nMaxChar );
            SetLinesOrCharsRanges( *m_pCharsRangeFT , m_pCharsPerLineNF->GetMax() );
        }
        //rubySize is disabled
    }
    GridModifyHdl(*m_pColorLB);
}

IMPL_LINK_TYPED(SwTextGridPage, GridTypeHdl, Button*, pButton, void)
{
    bool bEnable = m_pNoGridRB.get() != pButton;
    m_pLayoutFL->Enable(bEnable);
    m_pDisplayFL->Enable(bEnable);

    //one special case
    if(bEnable)
        DisplayGridHdl(m_pDisplayCB);

    bEnable = m_pCharsGridRB == pButton;
    m_pSnapToCharsCB->Enable(bEnable);

    bEnable = m_pLinesGridRB == pButton;
    if(bEnable && !m_bSquaredMode )
    {
        m_pCharsPerLineFT->Enable(false);
        m_pCharsPerLineNF->Enable(false);
        m_pCharsRangeFT->Enable(false);
        m_pCharWidthFT->Enable(false);
        m_pCharWidthMF->Enable(false);
    }

    GridModifyHdl(*m_pColorLB);
}

IMPL_LINK_NOARG_TYPED(SwTextGridPage, DisplayGridHdl, Button*, void)
{
    bool bChecked = m_pDisplayCB->IsChecked();
    m_pPrintCB->Enable(bChecked);
    m_pPrintCB->Check(bChecked);
}

IMPL_LINK_NOARG_TYPED(SwTextGridPage, GridModifyClickHdl, Button*, void)
{
    GridModifyHdl(*m_pColorLB);
}
IMPL_LINK_NOARG_TYPED(SwTextGridPage, GridModifyHdl, ListBox&, void)
{
    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemSet aSet(rOldSet);
    const SfxItemSet* pExSet = GetTabDialog()->GetExampleSet();
    if(pExSet)
        aSet.Put(*pExSet);
    PutGridItem(aSet);
    m_pExampleWN->UpdateExample(aSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
