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
#include <svx/flagsdef.hxx>

ScStyleDlg::ScStyleDlg(weld::Window* pParent,
                       SfxStyleSheetBase& rStyleBase,
                       bool bPage)
    : SfxStyleDialogController(pParent,
                        bPage ?
                          OUString("modules/scalc/ui/pagetemplatedialog.ui") :
                          OUString("modules/scalc/ui/paratemplatedialog.ui"),
                        bPage ?
                          OUString("PageTemplateDialog") :
                          OUString("ParaTemplateDialog"),
                        rStyleBase )
    , m_bPage(bPage)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    if (m_bPage) // page styles
    {
        AddTabPage("page", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ) );
        AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
        AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BKG ) );
        AddTabPage("header", &ScHeaderPage::Create,      &ScHeaderPage::GetRanges );
        AddTabPage("footer", &ScFooterPage::Create,      &ScFooterPage::GetRanges );
        AddTabPage("sheet", &ScTablePage::Create,     &ScTablePage::GetRanges );
    }
    else // cell format styles
    {
        AddTabPage("numbers", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_NUMBERFORMAT ));
        AddTabPage("font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ));
        AddTabPage("fonteffects", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ));
        AddTabPage("alignment", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_ALIGNMENT ));
        if ( SvtCJKOptions::IsAsianTypographyEnabled() )
        {
            AddTabPage("asiantypo", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN));
        }
        else
            RemoveTabPage("asiantypo");
        AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ));
        AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BKG ));
        AddTabPage("protection", &ScTabPageProtection::Create,    &ScTabPageProtection::GetRanges);
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
            const SfxPoolItem* pInfoItem
                = pDocSh->GetItem( SID_ATTR_NUMBERFORMAT_INFO );

            OSL_ENSURE( pInfoItem, "NumberInfoItem not found!" );

            aSet.Put ( static_cast<const SvxNumberInfoItem&>(*pInfoItem) );
            rTabPage.PageCreated(aSet);
        }
        else if (rPageId == "font")
        {
            const SfxPoolItem* pInfoItem
                = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

            OSL_ENSURE( pInfoItem, "FontListItem not found!" );

            aSet.Put (SvxFontListItem(static_cast<const SvxFontListItem&>(*pInfoItem).GetFontList(), SID_ATTR_CHAR_FONTLIST));
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
    : SfxStyleDialogController(pParent, "modules/scalc/ui/drawtemplatedialog.ui", "DrawTemplateDialog", rStyleBase)
    , mpView(pView)
{
    AddTabPage("line", RID_SVXPAGE_LINE);
    AddTabPage("area", RID_SVXPAGE_AREA);
    AddTabPage("shadowing", RID_SVXPAGE_SHADOW);
    AddTabPage("transparency", RID_SVXPAGE_TRANSPARENCE);
    AddTabPage("font", RID_SVXPAGE_CHAR_NAME);
    AddTabPage("fonteffect", RID_SVXPAGE_CHAR_EFFECTS);
    AddTabPage("background", RID_SVXPAGE_BKG);
    AddTabPage("indents", RID_SVXPAGE_STD_PARAGRAPH);
    AddTabPage("text", RID_SVXPAGE_TEXTATTR);
    AddTabPage("animation", RID_SVXPAGE_TEXTANIMATION);
    AddTabPage("dimensioning", RID_SVXPAGE_MEASURE);
    AddTabPage("alignment", RID_SVXPAGE_ALIGN_PARAGRAPH);
    AddTabPage("tabs", RID_SVXPAGE_TABULATOR);
    if (SvtCJKOptions::IsAsianTypographyEnabled())
        AddTabPage("asiantypo", RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage("asiantypo");
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
            SvxFontListItem aItem(*static_cast<const SvxFontListItem*>(
                pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST)));

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
