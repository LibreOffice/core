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

#include <sfx2/viewfrm.hxx>
#include <svl/style.hxx>
#include <vcl/msgbox.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <charfmt.hxx>

#include <docstyle.hxx>

#include "fldbas.hxx"
#include "lineinfo.hxx"
#include "globals.hrc"
#include "linenum.hrc"
#include "linenum.hxx"
#include "uitool.hxx"

#include <IDocumentStylePoolAccess.hxx>

SwLineNumberingDlg::SwLineNumberingDlg(SwView *pVw) :
    SfxSingleTabDialog(&pVw->GetViewFrame()->GetWindow(), 0, 0),
    pSh(pVw->GetWrtShellPtr())
{
    // create TabPage
    SetTabPage(SwLineNumberingPage::Create(this, *(SfxItemSet*)0));

    GetOKButton()->SetClickHdl(LINK(this, SwLineNumberingDlg, OKHdl));
}

SwLineNumberingDlg::~SwLineNumberingDlg()
{
}

IMPL_LINK_NOARG(SwLineNumberingDlg, OKHdl)
{
    if (GetOKButton()->IsEnabled())
    {
        SfxTabPage* pCurPage = GetTabPage();
        if( pCurPage )
            pCurPage->FillItemSet(*(SfxItemSet*)0);

        EndDialog( RET_OK );
    }

    return 0;
}

SwLineNumberingPage::SwLineNumberingPage( Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent, SW_RES(TP_LINENUMBERING), rSet),
    aNumberingOnCB      ( this, SW_RES( CB_NUMBERING_ON )),
    aDisplayFL          ( this, SW_RES( FL_DISPLAY )),
    aCharStyleFT        ( this, SW_RES( FT_CHAR_STYLE )),
    aCharStyleLB        ( this, SW_RES( LB_CHAR_STYLE )),
    aFormatFT           ( this, SW_RES( FT_FORMAT )),
    aFormatLB           ( this, SW_RES( LB_FORMAT ), INSERT_NUM_EXTENDED_TYPES),
    aPosFT              ( this, SW_RES( FT_POS )),
    aPosLB              ( this, SW_RES( LB_POS )),
    aOffsetFT           ( this, SW_RES( FT_OFFSET )),
    aOffsetMF           ( this, SW_RES( MF_OFFSET )),
    aNumIntervalFT      ( this, SW_RES( FT_NUM_INVERVAL )),
    aNumIntervalNF      ( this, SW_RES( NF_NUM_INVERVAL )),
    aNumRowsFT          ( this, SW_RES( FT_NUM_ROWS )),
    aDivisorFL          ( this, SW_RES( FL_DIVISOR )),
    aDivisorFT          ( this, SW_RES( FT_DIVISOR )),
    aDivisorED          ( this, SW_RES( ED_DIVISOR )),
    aDivIntervalFT      ( this, SW_RES( FT_DIV_INTERVAL )),
    aDivIntervalNF      ( this, SW_RES( NF_DIV_INTERVAL )),
    aDivRowsFT          ( this, SW_RES( FT_DIV_ROWS )),
    aCountFL            ( this, SW_RES( FL_COUNT )),
    aCountEmptyLinesCB  ( this, SW_RES( CB_COUNT_EMPTYLINES )),
    aCountFrameLinesCB  ( this, SW_RES( CB_COUNT_FRAMELINES )),
    aRestartEachPageCB  ( this, SW_RES( CB_RESTART_PAGE ))

{
    String sIntervalName = aDivIntervalFT.GetAccessibleName();
    sIntervalName += rtl::OUString("(");
    sIntervalName += aDivRowsFT.GetAccessibleName();
    sIntervalName += rtl::OUString(")");
    aDivIntervalNF.SetAccessibleName(sIntervalName);
    sIntervalName = aNumIntervalFT.GetAccessibleName();
    sIntervalName += rtl::OUString("(");
    sIntervalName += aNumRowsFT.GetAccessibleName();
    sIntervalName += rtl::OUString(")");
    aNumIntervalNF.SetAccessibleName(sIntervalName);

    FreeResource();
    SwLineNumberingDlg *pDlg = (SwLineNumberingDlg *)GetParent();
    pSh = pDlg->GetWrtShell();
    // char styles
    ::FillCharStyleListBox(aCharStyleLB, pSh->GetView().GetDocShell());
}

SwLineNumberingPage::~SwLineNumberingPage()
{
}

SfxTabPage* SwLineNumberingPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SwLineNumberingPage( pParent, rSet );
}

void SwLineNumberingPage::Reset( const SfxItemSet&  )
{
    const SwLineNumberInfo &rInf = pSh->GetLineNumberInfo();
    IDocumentStylePoolAccess* pIDSPA = pSh->getIDocumentStylePoolAccess();

    String sStyleName(rInf.GetCharFmt( *pIDSPA )->GetName());
    const sal_uInt16 nPos = aCharStyleLB.GetEntryPos(sStyleName);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        aCharStyleLB.SelectEntryPos(nPos);
    else
    {
        if (sStyleName.Len())
        {
            aCharStyleLB.InsertEntry(sStyleName);
            aCharStyleLB.SelectEntry(sStyleName);
        }
    }

    // format
    sal_uInt16 nSelFmt = rInf.GetNumType().GetNumberingType();

    aFormatLB.SelectNumberingType(nSelFmt);

    // position
    aPosLB.SelectEntryPos((sal_uInt16)rInf.GetPos());

    // offset
    sal_uInt16 nOffset = rInf.GetPosFromLeft();
    if (nOffset == USHRT_MAX)
        nOffset = 0;

    aOffsetMF.SetValue(aOffsetMF.Normalize(nOffset), FUNIT_TWIP);

    // numbering offset
    aNumIntervalNF.SetValue(rInf.GetCountBy());

    // divider
    aDivisorED.SetText(rInf.GetDivider());

    // divider offset
    aDivIntervalNF.SetValue(rInf.GetDividerCountBy());

    // count
    aCountEmptyLinesCB.Check(rInf.IsCountBlankLines());
    aCountFrameLinesCB.Check(rInf.IsCountInFlys());
    aRestartEachPageCB.Check(rInf.IsRestartEachPage());

    aNumberingOnCB.Check(rInf.IsPaintLineNumbers());

    aNumberingOnCB.SetClickHdl(LINK(this, SwLineNumberingPage, LineOnOffHdl));
    aDivisorED.SetModifyHdl(LINK(this, SwLineNumberingPage, ModifyHdl));
    ModifyHdl();
    LineOnOffHdl();
}

/*--------------------------------------------------------------------
    Description: modify
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwLineNumberingPage, ModifyHdl)
{
    sal_Bool bHasValue = aDivisorED.GetText().Len() != 0;

    aDivIntervalFT.Enable(bHasValue);
    aDivIntervalNF.Enable(bHasValue);
    aDivRowsFT.Enable(bHasValue);

    return 0;
}

/*--------------------------------------------------------------------
    Description: On/Off
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwLineNumberingPage, LineOnOffHdl)
{
    sal_Bool bEnable = aNumberingOnCB.IsChecked();

    aCharStyleFT.Enable(bEnable);
    aCharStyleLB.Enable(bEnable);
    aFormatFT.Enable(bEnable);
    aFormatLB.Enable(bEnable);
    aPosFT.Enable(bEnable);
    aPosLB.Enable(bEnable);
    aOffsetFT.Enable(bEnable);
    aOffsetMF.Enable(bEnable);
    aNumIntervalFT.Enable(bEnable);
    aNumIntervalNF.Enable(bEnable);
    aNumRowsFT.Enable(bEnable);
    aDisplayFL.Enable(bEnable);
    aDivisorFT.Enable(bEnable);
    aDivisorED.Enable(bEnable);
    aDivIntervalFT.Enable(bEnable);
    aDivIntervalNF.Enable(bEnable);
    aDivRowsFT.Enable(bEnable);
    aDivisorFL.Enable(bEnable);
    aCountEmptyLinesCB.Enable(bEnable);
    aCountFrameLinesCB.Enable(bEnable);
    aRestartEachPageCB.Enable(bEnable);
    aCountFL.Enable(bEnable);

    return 0;
}

sal_Bool SwLineNumberingPage::FillItemSet( SfxItemSet& )
{
    SwLineNumberInfo aInf(pSh->GetLineNumberInfo());

    // char styles
    String sCharFmtName(aCharStyleLB.GetSelectEntry());
    SwCharFmt *pCharFmt = pSh->FindCharFmtByName(sCharFmtName);

    if (!pCharFmt)
    {
        SfxStyleSheetBasePool* pPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
        SfxStyleSheetBase* pBase;
        pBase = pPool->Find(sCharFmtName, SFX_STYLE_FAMILY_CHAR);
        if(!pBase)
            pBase = &pPool->Make(sCharFmtName, SFX_STYLE_FAMILY_CHAR);
        pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();
    }

    if (pCharFmt)
        aInf.SetCharFmt(pCharFmt);

    // format
    SvxNumberType aType;
    aType.SetNumberingType(aFormatLB.GetSelectedNumberingType());
    aInf.SetNumType(aType);

    // position
    aInf.SetPos((LineNumberPosition)aPosLB.GetSelectEntryPos());

    // offset
    aInf.SetPosFromLeft((sal_uInt16)aOffsetMF.Denormalize(aOffsetMF.GetValue(FUNIT_TWIP)));

    // numbering offset
    aInf.SetCountBy((sal_uInt16)aNumIntervalNF.GetValue());

    // divider
    aInf.SetDivider(aDivisorED.GetText());

    // divider offset
    aInf.SetDividerCountBy((sal_uInt16)aDivIntervalNF.GetValue());

    // count
    aInf.SetCountBlankLines(aCountEmptyLinesCB.IsChecked());
    aInf.SetCountInFlys(aCountFrameLinesCB.IsChecked());
    aInf.SetRestartEachPage(aRestartEachPageCB.IsChecked());

    aInf.SetPaintLineNumbers(aNumberingOnCB.IsChecked());

    pSh->SetLineNumberInfo(aInf);

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
