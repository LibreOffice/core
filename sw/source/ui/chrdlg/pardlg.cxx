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

#include "hintids.hxx"
#include <sfx2/htmlmode.hxx>
#include <sfx2/tabdlg.hxx>
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

SwParaDlg::SwParaDlg(vcl::Window *pParent,
                    SwView& rVw,
                    const SfxItemSet& rCoreSet,
                    sal_uInt8 nDialogMode,
                    const OUString *pTitle,
                    bool bDraw,
                    const OString& sDefPage)
    : SfxTabDialog(pParent,
                 "ParagraphPropertiesDialog",
                 "modules/swriter/ui/paradialog.ui",
                 &rCoreSet,  nullptr != pTitle)
    , rView(rVw)
    , nDlgMode(nDialogMode)
    , bDrawParaDlg(bDraw)
    , m_nParaStd(0)
    , m_nParaAlign(0)
    , m_nParaExt(0)
    , m_nParaNumPara(0)
    , m_nParaDrpCps(0)
    , m_nParaBckGrnd(0)
    , m_nParaBorder(0)
    , m_nAreaId(0)
    , m_nTransparenceId(0)
{
    nHtmlMode = ::GetHtmlMode(rVw.GetDocShell());
    bool bHtmlMode = (nHtmlMode & HTMLMODE_ON) == HTMLMODE_ON;
    if(pTitle)
    {
        // Update title
        SetText(GetText() + SW_RESSTR(STR_TEXTCOLL_HEADER) + *pTitle + ")");
    }
    // tabs common to paragraph and draw paragraphs (paragraphs inside a text box)
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");
    m_nParaStd = AddTabPage("labelTP_PARA_STD", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),
                            pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
    m_nParaAlign = AddTabPage( "labelTP_PARA_ALIGN", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),
                               pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    if (!bDrawParaDlg && (!bHtmlMode || rHtmlOpt.IsPrintLayoutExtension()))
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
        m_nParaExt = AddTabPage( "textflow", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),
                                 pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

    }
    else
        RemoveTabPage("textflow");

    SvtCJKOptions aCJKOptions;
    if(!bHtmlMode && aCJKOptions.IsAsianTypographyEnabled())
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
        AddTabPage( "labelTP_PARA_ASIAN",  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),
                                   pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );
    }
    else
        RemoveTabPage("labelTP_PARA_ASIAN");

    const sal_uInt16 nWhich(rCoreSet.GetPool()->GetWhich(SID_ATTR_LRSPACE));
    bool bLRValid = SfxItemState::DEFAULT <= rCoreSet.GetItemState(nWhich);
    if(bHtmlMode || !bLRValid)
        RemoveTabPage("labelTP_TABULATOR");
    else
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");
        AddTabPage( "labelTP_TABULATOR", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );
    }

    // remove unwanted tabs for draw text box paragraph properties
    if (bDrawParaDlg)
    {
        RemoveTabPage("labelTP_NUMPARA");
        RemoveTabPage("labelTP_DROPCAPS");
        RemoveTabPage("labelTP_BACKGROUND");
        RemoveTabPage("labelTP_BORDER");
        RemoveTabPage("area");
        RemoveTabPage("transparence");
    }
    else
    {
        if(!(nDlgMode & DLG_ENVELOP))
            m_nParaNumPara = AddTabPage("labelTP_NUMPARA", SwParagraphNumTabPage::Create, SwParagraphNumTabPage::GetRanges);
        else
            RemoveTabPage("labelTP_NUMPARA");

        m_nParaDrpCps = AddTabPage("labelTP_DROPCAPS",  SwDropCapsPage::Create, SwDropCapsPage::GetRanges);

        if(!bHtmlMode || (nHtmlMode & (HTMLMODE_SOME_STYLES|HTMLMODE_FULL_STYLES)))
        {
            //UUUU remove?
            //OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            //OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            //m_nParaBckGrnd = AddTabPage("labelTP_BACKGROUND", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
            //
            //UUUU add Area and Transparence TabPages
            m_nAreaId = AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            m_nTransparenceId = AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );
        }
        else
        {
            //UUUU RemoveTabPage("labelTP_BACKGROUND");
            RemoveTabPage("area");
            RemoveTabPage("transparence");
        }

        OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
        m_nParaBorder = AddTabPage("labelTP_BORDER", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
    }

    if (!sDefPage.isEmpty())
        SetCurPageId(sDefPage);
}

SwParaDlg::~SwParaDlg()
{
}

void SwParaDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    SwWrtShell& rSh = rView.GetWrtShell();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    // Table borders cannot get any shade in Writer
    if (nId == m_nParaBorder)
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::PARA)));
        rPage.PageCreated(aSet);
    }
    else if( nId == m_nParaStd )
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
    else if( m_nParaAlign == nId)
    {
        if (!bDrawParaDlg)
        {
            aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,true));
            rPage.PageCreated(aSet);
        }
    }
    else if( m_nParaExt == nId )
    {
        // pagebreak only when the cursor is in the body-area and not in a table
        const FrameTypeFlags eType = rSh.GetFrameType(nullptr,true);
        if( !(FrameTypeFlags::BODY & eType) ||
            rSh.GetSelectionType() & nsSelectionType::SEL_TBL )
        {
            aSet.Put(SfxBoolItem(SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK,true));
            rPage.PageCreated(aSet);
        }
    }
    else if( m_nParaDrpCps == nId )
    {
        static_cast<SwDropCapsPage&>(rPage).SetFormat(false);
    }
    else if( m_nParaBckGrnd == nId )
    {
      if(!( nHtmlMode & HTMLMODE_ON ) ||
        nHtmlMode & HTMLMODE_SOME_STYLES)
        {
            // pagebreak only when the cursor is in the body-area and not in a table
            const FrameTypeFlags eType = rSh.GetFrameType(nullptr,true);
            if(!(FrameTypeFlags::BODY & eType) ||
                rSh.GetSelectionType() & nsSelectionType::SEL_TBL)
            {
                aSet.Put(SfxBoolItem(SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK,true));
                rPage.PageCreated(aSet);
            }
        }
    }
    else if( m_nParaNumPara == nId)
    {
        SwTextFormatColl* pTmpColl = rSh.GetCurTextFormatColl();
        if( pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            static_cast<SwParagraphNumTabPage&>(rPage).DisableOutline() ;
        }

        static_cast<SwParagraphNumTabPage&>(rPage).EnableNewStart();
        ListBox & rBox = static_cast<SwParagraphNumTabPage&>(rPage).GetStyleBox();
        SfxStyleSheetBasePool* pPool = rView.GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PSEUDO);
        const SfxStyleSheetBase* pBase = pPool->First();
        std::set<OUString> aNames;
        while(pBase)
        {
            aNames.insert(pBase->GetName());
            pBase = pPool->Next();
        }
        for(std::set<OUString>::const_iterator it = aNames.begin(); it != aNames.end(); ++it)
            rBox.InsertEntry(*it);
    }
    //UUUU inits for Area and Transparency TabPages
    // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
    // the color table) need to be added as items (e.g. SvxColorTableItem) to make
    // these pages find the needed attributes for fill style suggestions.
    // These are added in SwDocStyleSheet::GetItemSet() for the SFX_STYLE_FAMILY_PARA on
    // demand, but could also be directly added from the DrawModel.
    else if (m_nAreaId == nId)
    {
        SfxItemSet aNew(*aSet.GetPool(),
            SID_COLOR_TABLE, SID_BITMAP_LIST,
            SID_OFFER_IMPORT, SID_OFFER_IMPORT, 0, 0);

        aNew.Put(*GetInputSetImpl());

        // add flag for direct graphic content selection
        aNew.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

        rPage.PageCreated(aNew);
    }
    else if (m_nTransparenceId == nId)
    {
        rPage.PageCreated(*GetInputSetImpl());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
