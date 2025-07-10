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

#include <editeng/flstitem.hxx>

#include <svx/svxids.hrc>

#include <svx/drawitem.hxx>
#include <svl/intitem.hxx>
#include <svx/ofaitem.hxx>
#include <svx/svdmodel.hxx>
#include <svl/cjkoptions.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dialogs.hrc>
#include <svl/style.hxx>

#include <tabtempl.hxx>
#include <svx/flagsdef.hxx>

#include <vcl/tabs.hrc>

/**
 * Constructor of the Tab dialog: appends pages to the dialog
 */
SdTabTemplateDlg::SdTabTemplateDlg(weld::Window* pParent,
                                   const SfxObjectShell* pDocShell,
                                   SfxStyleSheetBase& rStyleBase,
                                   SdrModel const * pModel,
                                   SdrView* pView)
    : SfxStyleDialogController(pParent, u"modules/simpress/ui/templatedialog.ui"_ustr,
                               u"TemplateDialog"_ustr, rStyleBase)
    , rDocShell(*pDocShell)
    , pSdrView(pView)
    , pColorList(pModel->GetColorList())
    , pGradientList(pModel->GetGradientList())
    , pHatchingList(pModel->GetHatchList())
    , pBitmapList(pModel->GetBitmapList())
    , pPatternList(pModel->GetPatternList())
    , pDashList(pModel->GetDashList())
    , pLineEndList(pModel->GetLineEndList())
{
    // fill Listbox and set Select-Handler

    AddTabPage(u"line"_ustr, TabResId(RID_TAB_LINE.aLabel), RID_SVXPAGE_LINE,
               RID_M + RID_TAB_LINE.sIconName);
    AddTabPage(u"area"_ustr, TabResId(RID_TAB_AREA.aLabel), RID_SVXPAGE_AREA,
               RID_M + RID_TAB_AREA.sIconName);
    AddTabPage(u"shadowing"_ustr, TabResId(RID_TAB_SHADOW.aLabel), RID_SVXPAGE_SHADOW,
               RID_M + RID_TAB_SHADOW.sIconName);
    AddTabPage(u"transparency"_ustr, TabResId(RID_TAB_TRANSPARENCE.aLabel),
               RID_SVXPAGE_TRANSPARENCE, RID_M + RID_TAB_TRANSPARENCE.sIconName);
    AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel), RID_SVXPAGE_CHAR_NAME,
               RID_M + RID_TAB_FONT.sIconName);
    AddTabPage(u"fonteffect"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel), RID_SVXPAGE_CHAR_EFFECTS,
               RID_M + RID_TAB_FONTEFFECTS.sIconName);
    AddTabPage(u"indents"_ustr, TabResId(RID_TAB_INDENTS.aLabel), RID_SVXPAGE_STD_PARAGRAPH,
               RID_M + RID_TAB_INDENTS.sIconName);
    AddTabPage(u"text"_ustr, TabResId(RID_TAB_TEXT.aLabel), RID_SVXPAGE_TEXTATTR,
               RID_M + RID_TAB_TEXT.sIconName);
    AddTabPage(u"animation"_ustr, TabResId(RID_TAB_TEXTANIMATION.aLabel), RID_SVXPAGE_TEXTANIMATION,
               RID_M + RID_TAB_TEXTANIMATION.sIconName);
    AddTabPage(u"dimensioning"_ustr, TabResId(RID_TAB_DIMENSIONING.aLabel), RID_SVXPAGE_MEASURE,
               RID_M + RID_TAB_DIMENSIONING.sIconName);
    AddTabPage(u"connector"_ustr, TabResId(RID_TAB_CONNECTOR.aLabel), RID_SVXPAGE_CONNECTION,
               RID_M + RID_TAB_CONNECTOR.sIconName);
    AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_ALIGNMENT.aLabel), RID_SVXPAGE_ALIGN_PARAGRAPH,
               RID_M + RID_TAB_ALIGNMENT.sIconName);
    if (SvtCJKOptions::IsAsianTypographyEnabled())
        AddTabPage(u"asiantypo"_ustr, TabResId(RID_TAB_ASIANTYPO.aLabel), RID_SVXPAGE_PARA_ASIAN,
                   RID_M + RID_TAB_ASIANTYPO.sIconName);
    AddTabPage(u"tabs"_ustr, TabResId(RID_TAB_TABS.aLabel), RID_SVXPAGE_TABULATOR,
               RID_M + RID_TAB_TABS.sIconName);
    AddTabPage(u"background"_ustr, TabResId(RID_TAB_HIGHLIGHTING.aLabel), RID_SVXPAGE_BKG,
               RID_M + RID_TAB_HIGHLIGHTING.sIconName);
}

void SdTabTemplateDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "line")
    {
        aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
        aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
        aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "area")
    {
        aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
        aSet.Put (SvxGradientListItem(pGradientList,SID_GRADIENT_LIST));
        aSet.Put (SvxHatchListItem(pHatchingList,SID_HATCH_LIST));
        aSet.Put (SvxBitmapListItem(pBitmapList,SID_BITMAP_LIST));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,0));
        aSet.Put (SvxPatternListItem(pPatternList,SID_PATTERN_LIST));
        rPage.PageCreated(aSet);
    }
    else if (rId == "shadowing")
    {
        aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "transparency")
    {
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "font")
    {
        SvxFontListItem aItem(*static_cast<const SvxFontListItem*>(
            rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) );

        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (rId == "fonteffect")
    {
        rPage.PageCreated(aSet);
    }
    else if (rId == "background")
    {
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "text")
    {
        rPage.PageCreated(aSet);
    }
    else if (rId == "dimensioning")
    {
        aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));
        rPage.PageCreated(aSet);
    }
    else if (rId == "connector")
    {
        aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));
        rPage.PageCreated(aSet);
    }
}

void SdTabTemplateDlg::RefreshInputSet()
{
    SfxItemSet* pInputSet = GetInputSetImpl();

    if( pInputSet )
    {
        pInputSet->ClearItem();
        pInputSet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
    }
    else
        SetInputSet(&GetStyleSheet().GetItemSet());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
