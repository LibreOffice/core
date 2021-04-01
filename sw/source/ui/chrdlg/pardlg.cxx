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
#include <svtools/htmlcfg.hxx>
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

SwParaDlg::SwParaDlg(weld::Window *pParent,
                    SwView& rVw,
                    const SfxItemSet& rCoreSet,
                    sal_uInt8 nDialogMode,
                    const OUString *pTitle,
                    bool bDraw,
                    const OString& sDefPage)
    : SfxTabDialogController(pParent,
                 "modules/swriter/ui/paradialog.ui",
                 "ParagraphPropertiesDialog",
                 &rCoreSet,  nullptr != pTitle)
    , rView(rVw)
    , bDrawParaDlg(bDraw)
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
    AddTabPage("labelTP_PARA_STD", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),
                                   pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
    AddTabPage("labelTP_PARA_ALIGN", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),
                                      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH));

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    if (!bDrawParaDlg && (!bHtmlMode || rHtmlOpt.IsPrintLayoutExtension()))
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
        AddTabPage("textflow", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),
                               pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH));

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
        RemoveTabPage("labelTP_BORDER");
        RemoveTabPage("area");
        RemoveTabPage("transparence");
    }
    else
    {
        if(!(nDialogMode & DLG_ENVELOP))
            AddTabPage("labelTP_NUMPARA", SwParagraphNumTabPage::Create, SwParagraphNumTabPage::GetRanges);
        else
            RemoveTabPage("labelTP_NUMPARA");

        AddTabPage("labelTP_DROPCAPS",  SwDropCapsPage::Create, SwDropCapsPage::GetRanges);

        if(!bHtmlMode || (nHtmlMode & (HTMLMODE_SOME_STYLES|HTMLMODE_FULL_STYLES)))
        {
            // add Area and Transparence TabPages
            AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );
        }
        else
        {
            RemoveTabPage("area");
            RemoveTabPage("transparence");
        }

        OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
        OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
        AddTabPage("labelTP_BORDER", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
    }

    if (!sDefPage.isEmpty())
        SetCurPageId(sDefPage);
}

SwParaDlg::~SwParaDlg()
{
}

void SwParaDlg::PageCreated(const OString& rId, SfxTabPage& rPage)
{
    SwWrtShell& rSh = rView.GetWrtShell();
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

        if (!bDrawParaDlg)
        {
            // See SvxStdParagraphTabPage::PageCreated: enable RegisterMode, AutoFirstLine, NegativeMode, ContextualMode
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x0002|0x0004|0x0008|0x0010));
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, MM50/10));

        }
        rPage.PageCreated(aSet);
    }
    else if (rId == "labelTP_PARA_ALIGN")
    {
        if (!bDrawParaDlg)
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
        SfxStyleSheetBasePool* pPool = rView.GetDocShell()->GetStyleSheetPool();
        const SfxStyleSheetBase* pBase = pPool->First(SfxStyleFamily::Pseudo);
        std::set<OUString> aNames;
        while(pBase)
        {
            aNames.insert(pBase->GetName());
            pBase = pPool->Next();
        }
        aNames.erase("No List");
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
        SfxItemSet aNew(*aSet.GetPool(),
            svl::Items<SID_COLOR_TABLE, SID_PATTERN_LIST,
            SID_OFFER_IMPORT, SID_OFFER_IMPORT>{});

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
