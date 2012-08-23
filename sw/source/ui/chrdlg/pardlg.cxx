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

#include "hintids.hxx"
#include <svx/htmlmode.hxx>
#include <svl/style.hxx>
#include <svtools/htmlcfg.hxx>
#include <svl/cjkoptions.hxx>
#include "docsh.hxx"
#include "wrtsh.hxx"
#include "frmatr.hxx"
#include "view.hxx"
#include "globals.hrc"
#include "swuipardlg.hxx"
#include "pagedesc.hxx"
#include "paratr.hxx"
#include "drpcps.hxx"
#include "uitool.hxx"
#include "viewopt.hxx"
#include <numpara.hxx>
#include "chrdlg.hrc"
#include "poolfmt.hrc"
#include <svx/svxids.hrc>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>

SwParaDlg::SwParaDlg(Window *pParent,
                    SwView& rVw,
                    const SfxItemSet& rCoreSet,
                    sal_uInt8 nDialogMode,
                    const String *pTitle,
                    sal_Bool bDraw,
                    sal_uInt16 nDefPage):

    SfxTabDialog(pParent, bDraw ? SW_RES(DLG_DRAWPARA) : SW_RES(DLG_PARA),
                    &rCoreSet,  0 != pTitle),

    rView(rVw),
    nDlgMode(nDialogMode),
    bDrawParaDlg(bDraw)

{
    FreeResource();

    nHtmlMode = ::GetHtmlMode(rVw.GetDocShell());
    sal_Bool bHtmlMode = static_cast< sal_Bool >(nHtmlMode & HTMLMODE_ON);
    if(pTitle)
    {
        // Update title
        String aTmp( GetText() );
        aTmp += SW_RESSTR(STR_TEXTCOLL_HEADER);
        aTmp += *pTitle;
        aTmp += ')';
        SetText(aTmp);
    }
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");
    AddTabPage( TP_PARA_STD,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
    AddTabPage( TP_PARA_ALIGN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    if (!bDrawParaDlg && (!bHtmlMode || rHtmlOpt.IsPrintLayoutExtension()))
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
        AddTabPage( TP_PARA_EXT,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

    }
    else
        RemoveTabPage(TP_PARA_EXT);

    SvtCJKOptions aCJKOptions;
    if(!bHtmlMode && aCJKOptions.IsAsianTypographyEnabled())
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
        AddTabPage( TP_PARA_ASIAN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );
    }
    else
        RemoveTabPage(TP_PARA_ASIAN);

    sal_uInt16 nWhich(rCoreSet.GetPool()->GetWhich(SID_ATTR_LRSPACE));
    sal_Bool bLRValid = SFX_ITEM_AVAILABLE <= rCoreSet.GetItemState(nWhich);
    if(bHtmlMode || !bLRValid)
        RemoveTabPage(TP_TABULATOR);
    else
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");
        AddTabPage( TP_TABULATOR,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );

    }
    if (!bDrawParaDlg)
    {
        if(!(nDlgMode & DLG_ENVELOP))
            AddTabPage(TP_NUMPARA,   SwParagraphNumTabPage::Create,SwParagraphNumTabPage::GetRanges);
        else
            RemoveTabPage(TP_NUMPARA);
        if(!bHtmlMode || (nHtmlMode & HTMLMODE_FULL_STYLES))
        {
            AddTabPage(TP_DROPCAPS,  SwDropCapsPage::Create,        SwDropCapsPage::GetRanges);
        }
        else
        {
            RemoveTabPage(TP_DROPCAPS);
        }
        if(!bHtmlMode || (nHtmlMode & (HTMLMODE_SOME_STYLES|HTMLMODE_FULL_STYLES)))
        {
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
        }
        else
        {
            RemoveTabPage(TP_BACKGROUND);
        }

        OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
        AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
    }

    if (nDefPage)
        SetCurPageId(nDefPage);
}


SwParaDlg::~SwParaDlg()
{
}


void SwParaDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    SwWrtShell& rSh = rView.GetWrtShell();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    // Table borders cannot get any shade in Writer
    if (nId == TP_BORDER)
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_PARA));
        rPage.PageCreated(aSet);
    }
    else if( nId == TP_PARA_STD )
    {
        aSet.Put(SfxUInt16Item(SID_SVXSTDPARAGRAPHTABPAGE_PAGEWIDTH,
                            static_cast< sal_uInt16 >(rSh.GetAnyCurRect(RECT_PAGE_PRT).Width()) ));

        if (!bDrawParaDlg)
        {
            // See SvxStdParagraphTabPage::PageCreated: enable RegisterMode, AutoFirstLine, NegativeMode, ContextualMode
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x0002|0x0004|0x0008|0x0010));
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, MM50/10));

        }
        rPage.PageCreated(aSet);
    }
    else if( TP_PARA_ALIGN == nId)
    {
        if (!bDrawParaDlg)
        {
            aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,sal_True));
            rPage.PageCreated(aSet);
        }
    }
    else if( TP_PARA_EXT == nId )
    {
        // pagebreak only when the cursor is in the body-area and not in a table
        const sal_uInt16 eType = rSh.GetFrmType(0,sal_True);
        if( !(FRMTYPE_BODY & eType) ||
            rSh.GetSelectionType() & nsSelectionType::SEL_TBL )
        {
            aSet.Put(SfxBoolItem(SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK,sal_True));
            rPage.PageCreated(aSet);
        }
    }
    else if( TP_DROPCAPS == nId )
    {
        ((SwDropCapsPage&)rPage).SetFormat(sal_False);
    }
    else if( TP_BACKGROUND == nId )
    {
      if(!( nHtmlMode & HTMLMODE_ON ) ||
        nHtmlMode & HTMLMODE_SOME_STYLES)
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
            rPage.PageCreated(aSet);
        }
    }
    else if( TP_NUMPARA == nId)
    {
        SwTxtFmtColl* pTmpColl = rSh.GetCurTxtFmtColl();
        if( pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            ((SwParagraphNumTabPage&)rPage).DisableOutline() ;
        }

        ((SwParagraphNumTabPage&)rPage).EnableNewStart();
        ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
        SfxStyleSheetBasePool* pPool = rView.GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PSEUDO, SFXSTYLEBIT_ALL);
        const SfxStyleSheetBase* pBase = pPool->First();
        std::set<String> aNames;
        while(pBase)
        {
            aNames.insert(pBase->GetName());
            pBase = pPool->Next();
        }
        for(std::set<String>::const_iterator it = aNames.begin(); it != aNames.end(); ++it)
            rBox.InsertEntry(*it);
    }

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
