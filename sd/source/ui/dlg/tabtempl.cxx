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

/**
 * Constructor of the Tab dialog: appends pages to the dialog
 */
SdTabTemplateDlg::SdTabTemplateDlg(weld::Window* pParent,
                                   const SfxObjectShell* pDocShell,
                                   SfxStyleSheetBase& rStyleBase,
                                   SdrModel const * pModel,
                                   SdrView* pView)
    : SfxStyleDialogController(pParent, "modules/simpress/ui/templatedialog.ui",
                               "TemplateDialog", rStyleBase)
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
    AddTabPage("connector", RID_SVXPAGE_CONNECTION);
    AddTabPage("alignment", RID_SVXPAGE_ALIGN_PARAGRAPH);
    AddTabPage("tabs", RID_SVXPAGE_TABULATOR);
    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage("asiantypo", RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage("asiantypo");
}

void SdTabTemplateDlg::PageCreated(const OString& rId, SfxTabPage &rPage)
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
