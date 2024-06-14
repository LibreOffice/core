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

#include <sfx2/htmlmode.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/style.hxx>
#include <svl/cjkoptions.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swuipardlg.hxx>
#include <pardlg.hxx>
#include <drpcps.hxx>
#include <viewopt.hxx>
#include <numpara.hxx>
#include <strings.hrc>
#include <svx/svxids.hrc>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <osl/diagnose.h>
#include <officecfg/Office/Common.hxx>

SwParaDlg::SwParaDlg(weld::Window *pParent,
                    SwView& rVw,
                    const SfxItemSet& rCoreSet,
                    sal_uInt8 nDialogMode,
                    const OUString *pTitle,
                    bool bDraw,
                    const OUString& sDefPage)
    : SfxTabDialogController(pParent,
                 u"modules/swriter/ui/paradialog.ui"_ustr,
                 u"ParagraphPropertiesDialog"_ustr,
                 &rCoreSet,  nullptr != pTitle)
    , m_rView(rVw)
    , m_bDrawParaDlg(bDraw)
{
    sal_uInt16 nHtmlMode = ::GetHtmlMode(rVw.GetDocShell());
    bool bHtmlMode = (nHtmlMode & HTMLMODE_ON) == HTMLMODE_ON;
    if(pTitle)
    {
        // Update title
        m_xDialog->set_title(m_xDialog->get_title() + SwResId(STR_TEXTCOLL_HEADER) + *pTitle + ")");
    }
    // tabs common to paragraph and draw paragraphs (paragraphs inside a text box)
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");
    AddTabPage(u"labelTP_PARA_STD"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),
                                   pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
    AddTabPage(u"labelTP_PARA_ALIGN"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),
                                      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH));

    if (!m_bDrawParaDlg && (!bHtmlMode || officecfg::Office::Common::Filter::HTML::Export::PrintLayout::get()))
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
        AddTabPage(u"textflow"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),
                               pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH));

    }
    else
        RemoveTabPage(u"textflow"_ustr);

    if(!bHtmlMode && SvtCJKOptions::IsAsianTypographyEnabled())
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
        AddTabPage( u"labelTP_PARA_ASIAN"_ustr,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),
                                   pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );
    }
    else
        RemoveTabPage(u"labelTP_PARA_ASIAN"_ustr);

    if(bHtmlMode)
        RemoveTabPage(u"labelTP_TABULATOR"_ustr);
    else
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");
        AddTabPage( u"labelTP_TABULATOR"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );
    }

    // remove unwanted tabs for draw text box paragraph properties
    if (m_bDrawParaDlg)
    {
        RemoveTabPage(u"labelTP_NUMPARA"_ustr);
        RemoveTabPage(u"labelTP_DROPCAPS"_ustr);
        RemoveTabPage(u"labelTP_BORDER"_ustr);
        RemoveTabPage(u"area"_ustr);
        RemoveTabPage(u"transparence"_ustr);
    }
    else
    {
        if(!(nDialogMode & DLG_ENVELOP))
            AddTabPage(u"labelTP_NUMPARA"_ustr, SwParagraphNumTabPage::Create, SwParagraphNumTabPage::GetRanges);
        else
            RemoveTabPage(u"labelTP_NUMPARA"_ustr);

        AddTabPage(u"labelTP_DROPCAPS"_ustr,  SwDropCapsPage::Create, SwDropCapsPage::GetRanges);

        if(!bHtmlMode || (nHtmlMode & (HTMLMODE_SOME_STYLES|HTMLMODE_FULL_STYLES)))
        {
            // add Area and Transparence TabPages
            AddTabPage(u"area"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            AddTabPage(u"transparence"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );
        }
        else
        {
            RemoveTabPage(u"area"_ustr);
            RemoveTabPage(u"transparence"_ustr);
        }

        OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
        AddTabPage(u"labelTP_BORDER"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
    }

    if (!sDefPage.isEmpty())
        SetCurPageId(sDefPage);
}

SwParaDlg::~SwParaDlg()
{
}

void SwParaDlg::PageCreated(const OUString& rId, SfxTabPage& rPage)
{
    SwWrtShell& rSh = m_rView.GetWrtShell();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    // Table borders cannot get any shade in Writer
    if (rId == "labelTP_BORDER")
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::PARA)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "labelTP_PARA_STD")
    {
        aSet.Put(SfxUInt16Item(SID_SVXSTDPARAGRAPHTABPAGE_PAGEWIDTH,
                            static_cast< sal_uInt16 >(rSh.GetAnyCurRect(CurRectType::PagePrt).Width()) ));

        if (!m_bDrawParaDlg)
        {
            // See SvxStdParagraphTabPage::PageCreated: enable RegisterMode, AutoFirstLine, NegativeMode, ContextualMode
            constexpr tools::Long constTwips_0_5mm = o3tl::toTwips(5, o3tl::Length::mm10);
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x0002|0x0004|0x0008|0x0010));
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, constTwips_0_5mm));

        }
        rPage.PageCreated(aSet);
    }
    else if (rId == "labelTP_PARA_ALIGN")
    {
        if (!m_bDrawParaDlg)
        {
            aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,true));
            rPage.PageCreated(aSet);
        }
    }
    else if (rId == "textflow")
    {
        // pagebreak only when the cursor is in the body-area and not in a table
        const FrameTypeFlags eType = rSh.GetFrameType(nullptr,true);
        if( !(FrameTypeFlags::BODY & eType) ||
            rSh.GetSelectionType() & SelectionType::Table )
        {
            aSet.Put(SfxBoolItem(SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK,true));
            rPage.PageCreated(aSet);
        }
    }
    else if (rId == "labelTP_DROPCAPS")
    {
        static_cast<SwDropCapsPage&>(rPage).SetFormat(false);
    }
    else if (rId == "labelTP_NUMPARA")
    {
        SwTextFormatColl* pTmpColl = rSh.GetCurTextFormatColl();
        if( pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            static_cast<SwParagraphNumTabPage&>(rPage).DisableOutline() ;
        }

        static_cast<SwParagraphNumTabPage&>(rPage).EnableNewStart();
        weld::ComboBox& rBox = static_cast<SwParagraphNumTabPage&>(rPage).GetStyleBox();
        SfxStyleSheetBasePool* pPool = m_rView.GetDocShell()->GetStyleSheetPool();
        const SfxStyleSheetBase* pBase = pPool->First(SfxStyleFamily::Pseudo);
        std::set<OUString> aNames;
        while(pBase)
        {
            aNames.insert(pBase->GetName());
            pBase = pPool->Next();
        }
        aNames.erase(SwResId(STR_POOLNUMRULE_NOLIST));
        for (const auto& rName : aNames)
            rBox.append_text(rName);
    }
    // inits for Area and Transparency TabPages
    // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
    // the color table) need to be added as items (e.g. SvxColorTableItem) to make
    // these pages find the needed attributes for fill style suggestions.
    // These are added in SwDocStyleSheet::GetItemSet() for the SfxStyleFamily::Para on
    // demand, but could also be directly added from the DrawModel.
    else if (rId == "area")
    {
        SfxItemSetFixed
            <SID_COLOR_TABLE, SID_PATTERN_LIST,
            SID_OFFER_IMPORT, SID_OFFER_IMPORT>  aNew(*aSet.GetPool());

        aNew.Put(*GetInputSetImpl());

        // add flag for direct graphic content selection
        aNew.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

        rPage.PageCreated(aNew);
    }
    else if (rId == "transparence")
    {
        rPage.PageCreated(*GetInputSetImpl());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
