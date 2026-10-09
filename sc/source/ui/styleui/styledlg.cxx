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

#undef SC_DLLIMPLEMENTATION

#include <svx/drawitem.hxx>
#include <svx/flagsdef.hxx>
#include <svx/ofaitem.hxx>
#include <svx/svdview.hxx>
#include <svx/numinf.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svl/style.hxx>
#include <svl/cjkoptions.hxx>
#include <osl/diagnose.h>

#include <styledlg.hxx>
#include <tabpages.hxx>
#include <tphf.hxx>
#include <tptable.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svl/intitem.hxx>
#include <editeng/flstitem.hxx>

#include <vcl/tabs.hrc>

ScStyleDlg::ScStyleDlg(weld::Window* pParent,
                       SfxStyleSheetBase& rStyleBase,
                       bool bPage)
    : SfxStyleDialogController(pParent,
                        bPage ?
                          u"modules/scalc/ui/pagetemplatedialog.ui"_ustr :
                          u"modules/scalc/ui/paratemplatedialog.ui"_ustr,
                        bPage ?
                          u"PageTemplateDialog"_ustr :
                          u"ParaTemplateDialog"_ustr,
                        rStyleBase )
    , m_bPage(bPage)
{
    if (m_bPage) // page styles
    {
        AddTabPage(u"page"_ustr, TabResId(RID_TAB_SCSTYLE_PAGE), RID_SVXPAGE_PAGE,
                   RID_M + RID_TAB_PAGE_IMG);
        AddTabPage(u"borders"_ustr, TabResId(RID_TAB_SCSTYLE_BORDER), RID_SVXPAGE_BORDER,
                   RID_M + RID_TAB_BORDER_IMG);
        AddTabPage(u"background"_ustr, TabResId(RID_TAB_SCSTYLE_BACKGROUND), RID_SVXPAGE_BKG,
                   RID_M + RID_TAB_BACKGROUND_IMG);
        AddTabPage(u"header"_ustr, TabResId(RID_TAB_SCSTYLE_HEADER), &ScHeaderPage::Create,
                   &ScHeaderPage::GetRanges, RID_M + RID_TAB_HEADER_IMG);
        AddTabPage(u"footer"_ustr, TabResId(RID_TAB_SCSTYLE_FOOTER), &ScFooterPage::Create,
                   &ScFooterPage::GetRanges, RID_M + RID_TAB_FOOTER_IMG);
        AddTabPage(u"sheet"_ustr, TabResId(RID_TAB_SCSTYLE_SHEET), &ScTablePage::Create,
                   &ScTablePage::GetRanges, RID_M + RID_TAB_SHEET_IMG);
    }
    else // cell format styles
    {
        AddTabPage(u"numbers"_ustr, TabResId(RID_TAB_SCSTYLE_NUMBERS), RID_SVXPAGE_NUMBERFORMAT,
                   RID_M + RID_TAB_NUMBERS_IMG);
        AddTabPage(u"font"_ustr, TabResId(RID_TAB_SCSTYLE_FONT), RID_SVXPAGE_CHAR_NAME,
                   RID_M + RID_TAB_FONT_IMG);
        AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_SCSTYLE_FONTEFFECTS),
                   RID_SVXPAGE_CHAR_EFFECTS, RID_M + RID_TAB_FONTEFFECTS_IMG);
        AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_SCSTYLE_ALIGNMENT), RID_SVXPAGE_ALIGNMENT,
                   RID_M + RID_TAB_ALIGNMENT_IMG);
        if (SvtCJKOptions::IsAsianTypographyEnabled())
        {
            AddTabPage(u"asiantypo"_ustr, TabResId(RID_TAB_SCSTYLE_ASIANTYPO),
                       RID_SVXPAGE_PARA_ASIAN, RID_M + RID_TAB_ASIANTYPO_IMG);
        }
        AddTabPage(u"borders"_ustr, TabResId(RID_TAB_SCSTYLE_BORDER), RID_SVXPAGE_BORDER,
                   RID_M + RID_TAB_BORDER_IMG);
        AddTabPage(u"background"_ustr, TabResId(RID_TAB_SCSTYLE_BACKGROUND), RID_SVXPAGE_BKG,
                   RID_M + RID_TAB_BACKGROUND_IMG);
        AddTabPage(u"protection"_ustr, TabResId(RID_TAB_SCSTYLE_PROTECTION),
                   &ScTabPageProtection::Create, &ScTabPageProtection::GetRanges,
                   RID_M + RID_TAB_PROTECTION_IMG);
    }
}

void ScStyleDlg::PageCreated(const OUString& rPageId, SfxTabPage& rTabPage)
{
    if (m_bPage)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        if (rPageId == "page")
        {
            aSet.Put (SfxUInt16Item(SID_ENUM_PAGE_MODE, SVX_PAGE_MODE_CENTER));
            rTabPage.PageCreated(aSet);
        }
        else if (rPageId == "header" || rPageId == "footer")
        {
            static_cast<ScHFPage&>(rTabPage).SetStyleDlg( this );
            static_cast<ScHFPage&>(rTabPage).SetPageStyle( GetStyleSheet().GetName() );
            static_cast<ScHFPage&>(rTabPage).DisableDeleteQueryBox();
        }
        else if (rPageId == "background")
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
            rTabPage.PageCreated(aSet);
        }
    }
    else if (SfxObjectShell* pDocSh = SfxObjectShell::Current())
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        if (rPageId == "numbers")
        {
            const SvxNumberInfoItem* pInfoItem
                = pDocSh->GetItem( SID_ATTR_NUMBERFORMAT_INFO );

            OSL_ENSURE( pInfoItem, "NumberInfoItem not found!" );

            aSet.Put ( *pInfoItem );
            rTabPage.PageCreated(aSet);
        }
        else if (rPageId == "font")
        {
            const SvxFontListItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

            assert(pInfoItem && "FontListItem not found!");

            aSet.Put(SvxFontListItem(pInfoItem->GetFontList(), SID_ATTR_CHAR_FONTLIST));
            if (rTabPage.GetItemSet().GetParent())
                aSet.Put(SfxUInt32Item(SID_FLAG_TYPE, SVX_RELATIVE_MODE));
            rTabPage.PageCreated(aSet);
        }
        else if (rPageId == "background")
        {
            rTabPage.PageCreated(aSet);
        }
    }
}

void ScStyleDlg::RefreshInputSet()
{
    SfxItemSet* pItemSet = GetInputSetImpl();
    pItemSet->ClearItem();
    pItemSet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
}

ScDrawStyleDlg::ScDrawStyleDlg(weld::Window* pParent, SfxStyleSheetBase& rStyleBase, SdrView* pView)
    : SfxStyleDialogController(pParent, u"modules/scalc/ui/drawtemplatedialog.ui"_ustr, u"DrawTemplateDialog"_ustr, rStyleBase)
    , mpView(pView)
{
    AddTabPage(u"line"_ustr, TabResId(RID_TAB_SCSTYLE_LINE), RID_SVXPAGE_LINE,
               RID_M + RID_TAB_LINE_IMG);
    AddTabPage(u"area"_ustr, TabResId(RID_TAB_SCSTYLE_AREA), RID_SVXPAGE_AREA,
               RID_M + RID_TAB_AREA_IMG);
    AddTabPage(u"shadowing"_ustr, TabResId(RID_TAB_SCSTYLE_SHADOW), RID_SVXPAGE_SHADOW,
               RID_M + RID_TAB_SHADOW_IMG);
    AddTabPage(u"transparency"_ustr, TabResId(RID_TAB_SCSTYLE_TRANSPARENCE),
               RID_SVXPAGE_TRANSPARENCE, RID_M + RID_TAB_TRANSPARENCE_IMG);
    AddTabPage(u"font"_ustr, TabResId(RID_TAB_SCSTYLE_FONT), RID_SVXPAGE_CHAR_NAME,
               RID_M + RID_TAB_FONT_IMG);
    AddTabPage(u"fonteffect"_ustr, TabResId(RID_TAB_SCSTYLE_FONTEFFECTS), RID_SVXPAGE_CHAR_EFFECTS,
               RID_M + RID_TAB_FONTEFFECTS_IMG);
    AddTabPage(u"indents"_ustr, TabResId(RID_TAB_SCSTYLE_INDENTS), RID_SVXPAGE_STD_PARAGRAPH,
               RID_M + RID_TAB_INDENTS_IMG);
    AddTabPage(u"text"_ustr, TabResId(RID_TAB_SCSTYLE_TEXT), RID_SVXPAGE_TEXTATTR,
               RID_M + RID_TAB_TEXT_IMG);
    AddTabPage(u"animation"_ustr, TabResId(RID_TAB_SCSTYLE_TEXTANIMATION), RID_SVXPAGE_TEXTANIMATION,
               RID_M + RID_TAB_TEXTANIMATION_IMG);
    AddTabPage(u"dimensioning"_ustr, TabResId(RID_TAB_SCSTYLE_DIMENSIONING), RID_SVXPAGE_MEASURE,
               RID_M + RID_TAB_DIMENSIONING_IMG);
    AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_SCSTYLE_ALIGNMENT), RID_SVXPAGE_ALIGN_PARAGRAPH,
               RID_M + RID_TAB_ALIGNMENT_IMG);
    if (SvtCJKOptions::IsAsianTypographyEnabled())
        AddTabPage(u"asiantypo"_ustr, TabResId(RID_TAB_SCSTYLE_ASIANTYPO), RID_SVXPAGE_PARA_ASIAN,
                   RID_M + RID_TAB_ASIANTYPO_IMG);
    AddTabPage(u"tabs"_ustr, TabResId(RID_TAB_SCSTYLE_TABS), RID_SVXPAGE_TABULATOR,
               RID_M + RID_TAB_TABS_IMG);
    AddTabPage(u"background"_ustr, TabResId(RID_TAB_SCSTYLE_HIGHLIGHTING), RID_SVXPAGE_BKG,
               RID_M + RID_TAB_HIGHLIGHTING_IMG);
}

void ScDrawStyleDlg::PageCreated(const OUString& rPageId, SfxTabPage& rTabPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    SdrModel& rModel = mpView->GetModel();

    if (rPageId == "line")
    {
        aSet.Put(SvxColorListItem(rModel.GetColorList(), SID_COLOR_TABLE));
        aSet.Put(SvxDashListItem(rModel.GetDashList(), SID_DASH_LIST));
        aSet.Put(SvxLineEndListItem(rModel.GetLineEndList(), SID_LINEEND_LIST));
        aSet.Put(SfxUInt16Item(SID_DLG_TYPE, 1));
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "area")
    {
        aSet.Put(SvxColorListItem(rModel.GetColorList(), SID_COLOR_TABLE));
        aSet.Put(SvxGradientListItem(rModel.GetGradientList(), SID_GRADIENT_LIST));
        aSet.Put(SvxHatchListItem(rModel.GetHatchList(), SID_HATCH_LIST));
        aSet.Put(SvxBitmapListItem(rModel.GetBitmapList(), SID_BITMAP_LIST));
        aSet.Put(SvxPatternListItem(rModel.GetPatternList(), SID_PATTERN_LIST));
        aSet.Put(SfxUInt16Item(SID_PAGE_TYPE, 0));
        aSet.Put(SfxUInt16Item(SID_DLG_TYPE, 1));
        aSet.Put(SfxUInt16Item(SID_TABPAGE_POS, 0));
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "shadowing")
    {
        aSet.Put(SvxColorListItem(rModel.GetColorList(), SID_COLOR_TABLE));
        aSet.Put(SfxUInt16Item(SID_PAGE_TYPE, 0));
        aSet.Put(SfxUInt16Item(SID_DLG_TYPE, 1));
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "transparency")
    {
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE, 0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE, 1));
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "font")
    {
        if (SfxObjectShell* pDocSh = SfxObjectShell::Current())
        {
            SvxFontListItem aItem(*pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));

            aSet.Put(SvxFontListItem(aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        }
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "fonteffect")
    {
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "background")
    {
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR)));
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "text")
    {
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "dimensioning")
    {
        aSet.Put(OfaPtrItem(SID_OBJECT_LIST, mpView));
        rTabPage.PageCreated(aSet);
    }
}

void ScDrawStyleDlg::RefreshInputSet()
{
    SfxItemSet* pItemSet = GetInputSetImpl();
    pItemSet->ClearItem();
    pItemSet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
