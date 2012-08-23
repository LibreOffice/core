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

#include "cmdid.h"
#include "swtypes.hxx"
#include "hintids.hxx"
#include "globals.hrc"
#include "helpid.h"
#include <sfx2/objsh.hxx>
#include <svx/htmlmode.hxx>
#include <svx/svxids.hrc>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <fmtline.hxx>
#include <numpara.hxx>
#include <numpara.hrc>

#include <app.hrc>


// Globals ******************************************************************

static sal_uInt16 aPageRg[] = {
    FN_NUMBER_NEWSTART, FN_NUMBER_NEWSTART_AT,
    0
};

SwParagraphNumTabPage::SwParagraphNumTabPage(Window* pParent,
                                                const SfxItemSet& rAttr ) :
    SfxTabPage(pParent, SW_RES(TP_NUMPARA), rAttr),
    aOutlineStartFL         ( this, SW_RES( FL_OUTLINE_START )),
    aOutlineLvFT            ( this, SW_RES( FT_OUTLINE_LEVEL )),
    aOutlineLvLB            ( this, SW_RES( LB_OUTLINE_LEVEL )),
    aNewStartFL             ( this, SW_RES( FL_NEW_START ) ),
    aNumberStyleFT          ( this, SW_RES( FT_NUMBER_STYLE ) ),
    aNumberStyleLB          ( this, SW_RES( LB_NUMBER_STYLE ) ),
       aNewStartCB              ( this, SW_RES( CB_NEW_START ) ),
    aNewStartNumberCB       ( this, SW_RES( CB_NUMBER_NEW_START ) ),
    aNewStartNF             ( this, SW_RES( NF_NEW_START ) ),
    aCountParaFL            ( this, SW_RES( FL_COUNT_PARA        ) ),
    aCountParaCB            ( this, SW_RES( CB_COUNT_PARA        ) ),
    aRestartParaCountCB     ( this, SW_RES( CB_RESTART_PARACOUNT ) ),
    aRestartFT              ( this, SW_RES( FT_RESTART_NO        ) ),
    aRestartNF              ( this, SW_RES( NF_RESTART_PARA      ) ),
    msOutlineNumbering( SW_RES( STR_OUTLINE_NUMBERING ) ),
    bModified(sal_False),
    bCurNumrule(sal_False)
{
    FreeResource();

    const SfxPoolItem* pItem;
    SfxObjectShell* pObjSh;
    if(SFX_ITEM_SET == rAttr.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != ( pObjSh = SfxObjectShell::Current()) &&
                    0 != (pItem = pObjSh->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = ((const SfxUInt16Item*)pItem)->GetValue();
        if(HTMLMODE_ON & nHtmlMode)
        {
            aCountParaFL        .Hide();
            aCountParaCB        .Hide();
            aRestartParaCountCB .Hide();
            aRestartFT          .Hide();
            aRestartNF          .Hide();
        }
    }
    aNewStartCB.SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    aNewStartNumberCB.SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    aNumberStyleLB.SetSelectHdl(LINK(this, SwParagraphNumTabPage, StyleHdl_Impl));
    aCountParaCB.SetClickHdl(LINK(this,
                    SwParagraphNumTabPage, LineCountHdl_Impl));
    aRestartParaCountCB.SetClickHdl(
                    LINK(this, SwParagraphNumTabPage, LineCountHdl_Impl));
    aNewStartNF.SetAccessibleName(aNewStartNumberCB.GetText());
    aNewStartNF.SetAccessibleRelationLabeledBy(&aNewStartNumberCB);
}

SwParagraphNumTabPage::~SwParagraphNumTabPage()
{
}

SfxTabPage* SwParagraphNumTabPage::Create(  Window* pParent,
                                const SfxItemSet& rSet )
{
    return new SwParagraphNumTabPage(pParent, rSet);
}

sal_uInt16* SwParagraphNumTabPage::GetRanges()
{
    return aPageRg;
}

sal_Bool    SwParagraphNumTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( aOutlineLvLB.GetSelectEntryPos() != aOutlineLvLB.GetSavedValue())
    {
        sal_uInt16 aOutlineLv = aOutlineLvLB.GetSelectEntryPos();
        const SfxUInt16Item* pOldOutlineLv = (const SfxUInt16Item*)GetOldItem( rSet, SID_ATTR_PARA_OUTLINE_LEVEL);
        SfxUInt16Item* pOutlineLv = (SfxUInt16Item*)pOldOutlineLv->Clone();
        pOutlineLv->SetValue( aOutlineLv );
        rSet.Put(*pOutlineLv);
        delete pOutlineLv;
        bModified = sal_True;
    }

    if( aNumberStyleLB.GetSelectEntryPos() != aNumberStyleLB.GetSavedValue())
    {
        String aStyle;
        if(aNumberStyleLB.GetSelectEntryPos())
            aStyle = aNumberStyleLB.GetSelectEntry();
        const SfxStringItem* pOldRule = (const SfxStringItem*)GetOldItem( rSet, SID_ATTR_PARA_NUMRULE);
        SfxStringItem* pRule = (SfxStringItem*)pOldRule->Clone();
        pRule->SetValue(aStyle);
        rSet.Put(*pRule);
        delete pRule;
        bModified = sal_True;
    }
    if(aNewStartCB.GetState() != aNewStartCB.GetSavedValue() ||
        aNewStartNumberCB.GetState() != aNewStartNumberCB.GetSavedValue()||
        aNewStartNF.GetText() != aNewStartNF.GetSavedValue())
    {
        bModified = sal_True;
        sal_Bool bNewStartChecked = STATE_CHECK == aNewStartCB.GetState();
        sal_Bool bNumberNewStartChecked = STATE_CHECK == aNewStartNumberCB.GetState();
        rSet.Put(SfxBoolItem(FN_NUMBER_NEWSTART, bNewStartChecked));
        rSet.Put(SfxUInt16Item(FN_NUMBER_NEWSTART_AT,
                  bNumberNewStartChecked && bNewStartChecked ? (sal_uInt16)aNewStartNF.GetValue() : USHRT_MAX));
    }

    if(aCountParaCB.GetSavedValue() != aCountParaCB.GetState() ||
        aRestartParaCountCB.GetSavedValue() != aRestartParaCountCB.GetState() ||
            aRestartNF.GetSavedValue() != aRestartNF.GetText() )
    {
        SwFmtLineNumber aFmt;
        aFmt.SetStartValue( static_cast< sal_uLong >(aRestartParaCountCB.GetState() == STATE_CHECK ?
                                aRestartNF.GetValue() : 0 ));
        aFmt.SetCountLines( aCountParaCB.IsChecked() );
        rSet.Put(aFmt);
        bModified = sal_True;
    }
    return bModified;
}

void    SwParagraphNumTabPage::Reset( const SfxItemSet& rSet )
{
    sal_Bool bHasNumberStyle = sal_False;

    SfxItemState eItemState = rSet.GetItemState( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) );

    sal_Int16 nOutlineLv;
    if( eItemState >= SFX_ITEM_AVAILABLE )
    {
        nOutlineLv = ((const SfxUInt16Item &)rSet.Get( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) )).GetValue();
        aOutlineLvLB.SelectEntryPos( nOutlineLv ) ;
    }
    else
    {
        aOutlineLvLB.SetNoSelection();
    }
    aOutlineLvLB.SaveValue();

    eItemState = rSet.GetItemState( GetWhich(SID_ATTR_PARA_NUMRULE) );

    String aStyle;
    if( eItemState >= SFX_ITEM_AVAILABLE )
    {
        aStyle = ((const SfxStringItem &)rSet.Get( GetWhich(SID_ATTR_PARA_NUMRULE) )).GetValue();
        if(!aStyle.Len())
            aStyle = aNumberStyleLB.GetEntry(0);

        if( aStyle.EqualsAscii("Outline"))
        {
            aNumberStyleLB.InsertEntry( msOutlineNumbering );
            aNumberStyleLB.SelectEntry( msOutlineNumbering );
            aNumberStyleLB.RemoveEntry(msOutlineNumbering);
            aNumberStyleLB.SaveValue();
        }
        else
            aNumberStyleLB.SelectEntry( aStyle );

        bHasNumberStyle = sal_True;
    }
    else
    {
        aNumberStyleLB.SetNoSelection();
    }

    aNumberStyleLB.SaveValue();

    eItemState = rSet.GetItemState( FN_NUMBER_NEWSTART );
    if(eItemState > SFX_ITEM_AVAILABLE )
    {
        bCurNumrule = sal_True;
        const SfxBoolItem& rStart = (const SfxBoolItem&)rSet.Get(FN_NUMBER_NEWSTART);
        aNewStartCB.SetState(
            rStart.GetValue() ?
                        STATE_CHECK : STATE_NOCHECK );
        aNewStartCB.EnableTriState(sal_False);
    }
    else
        aNewStartCB.SetState(bHasNumberStyle ? STATE_NOCHECK : STATE_DONTKNOW);
    aNewStartCB.SaveValue();

    eItemState = rSet.GetItemState( FN_NUMBER_NEWSTART_AT);
    if( eItemState > SFX_ITEM_AVAILABLE )
    {
        sal_uInt16 nNewStart = ((const SfxUInt16Item&)rSet.Get(FN_NUMBER_NEWSTART_AT)).GetValue();
        aNewStartNumberCB.Check(USHRT_MAX != nNewStart);
        if(USHRT_MAX == nNewStart)
            nNewStart = 1;

        aNewStartNF.SetValue(nNewStart);
        aNewStartNumberCB.EnableTriState(sal_False);
    }
    else
        aNewStartCB.SetState(STATE_DONTKNOW);
    NewStartHdl_Impl(&aNewStartCB);
    aNewStartNF.SaveValue();
    aNewStartNumberCB.SaveValue();
    StyleHdl_Impl(&aNumberStyleLB);
    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState(RES_LINENUMBER))
    {
        SwFmtLineNumber& rNum = (SwFmtLineNumber&)rSet.Get(RES_LINENUMBER);
        sal_uLong nStartValue = rNum.GetStartValue();
        sal_Bool bCount = rNum.IsCount();
        aCountParaCB.SetState( bCount ? STATE_CHECK : STATE_NOCHECK );
        aRestartParaCountCB.SetState( 0 != nStartValue ? STATE_CHECK : STATE_NOCHECK );
        aRestartNF.SetValue(nStartValue == 0 ? 1 : nStartValue);
        LineCountHdl_Impl(&aCountParaCB);
        aCountParaCB.EnableTriState(sal_False);
        aRestartParaCountCB.EnableTriState(sal_False);
    }
    else
    {
        aCountParaCB.SetState(STATE_DONTKNOW);
        aRestartParaCountCB.SetState(STATE_DONTKNOW);
    }
    aCountParaCB.SaveValue();
    aRestartParaCountCB.SaveValue();
    aRestartNF.SaveValue();

    bModified = sal_False;
}

void SwParagraphNumTabPage::DisableOutline()
{
    aOutlineLvFT.Disable();
    aOutlineLvLB.Disable();
}

void SwParagraphNumTabPage::DisableNumbering()
{
    aNumberStyleFT.Disable();
    aNumberStyleLB.Disable();
}

void SwParagraphNumTabPage::EnableNewStart()
{
    aNewStartCB.Show();
    aNewStartNumberCB.Show();
    aNewStartNF.Show();
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, NewStartHdl_Impl)
{
    sal_Bool bEnable = aNewStartCB.IsChecked();
    aNewStartNumberCB.Enable(bEnable);
    aNewStartNF.Enable(bEnable && aNewStartNumberCB.IsChecked());
    return 0;
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, LineCountHdl_Impl)
{
    aRestartParaCountCB.Enable(aCountParaCB.IsChecked());

    sal_Bool bEnableRestartValue = aRestartParaCountCB.IsEnabled() &&
                                                aRestartParaCountCB.IsChecked();
    aRestartFT.Enable(bEnableRestartValue);
    aRestartNF.Enable(bEnableRestartValue);

    return 0;
}

IMPL_LINK( SwParagraphNumTabPage, StyleHdl_Impl, ListBox*, pBox )
{
    sal_Bool bEnable = bCurNumrule || pBox->GetSelectEntryPos() > 0;
    aNewStartCB.Enable(bEnable);
    NewStartHdl_Impl(&aNewStartCB);

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
