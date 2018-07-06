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
#include <svx/svdmark.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/svxgrahicitem.hxx>
#include <svl/cjkoptions.hxx>

#include <svx/dialogs.hrc>

#include <svx/svxdlg.hxx>
#include <svx/tabline.hxx>
#include <svl/style.hxx>
#include <svx/xtable.hxx>

#include <DrawDocShell.hxx>
#include <tabtempl.hxx>
#include <dlg_char.hxx>
#include <paragr.hxx>
#include <svx/flagsdef.hxx>

/**
 * Constructor of the Tab dialog: appends pages to the dialog
 */
SdTabTemplateDlg::SdTabTemplateDlg( vcl::Window* pParent,
                                const SfxObjectShell* pDocShell,
                                SfxStyleSheetBase& rStyleBase,
                                SdrModel const * pModel,
                                SdrView* pView )
    : SfxStyleDialog(pParent, "TemplateDialog",
        "modules/simpress/ui/templatedialog.ui",
        rStyleBase)
    , rDocShell(*pDocShell)
    , pSdrView(pView)
    , pColorList(pModel->GetColorList())
    , pGradientList(pModel->GetGradientList())
    , pHatchingList(pModel->GetHatchList())
    , pBitmapList(pModel->GetBitmapList())
    , pPatternList(pModel->GetPatternList())
    , pDashList(pModel->GetDashList())
    , pLineEndList(pModel->GetLineEndList())
    , m_nLineId(0)
    , m_nAreaId(0)
    , m_nShadowId(0)
    , m_nTransparencyId(0)
    , m_nFontId(0)
    , m_nFontEffectId(0)
    , m_nBackgroundId(0)
    , m_nTextId(0)
    , m_nDimensionId(0)
    , m_nConnectorId(0)
{
    // fill Listbox and set Select-Handler

    m_nLineId = AddTabPage("line", RID_SVXPAGE_LINE);
    m_nAreaId = AddTabPage("area", RID_SVXPAGE_AREA);
    m_nShadowId = AddTabPage("shadowing", RID_SVXPAGE_SHADOW);
    m_nTransparencyId = AddTabPage("transparency", RID_SVXPAGE_TRANSPARENCE);
    m_nFontId = AddTabPage("font", RID_SVXPAGE_CHAR_NAME);
    m_nFontEffectId = AddTabPage("fonteffect", RID_SVXPAGE_CHAR_EFFECTS);
    m_nBackgroundId = AddTabPage("background", RID_SVXPAGE_BKG);
    AddTabPage("indents", RID_SVXPAGE_STD_PARAGRAPH);
    m_nTextId = AddTabPage("text", RID_SVXPAGE_TEXTATTR);
    AddTabPage("animation", RID_SVXPAGE_TEXTANIMATION);
    m_nDimensionId = AddTabPage("dimensioning", RID_SVXPAGE_MEASURE);
    m_nConnectorId = AddTabPage("connector", RID_SVXPAGE_CONNECTION);
    AddTabPage("alignment", RID_SVXPAGE_ALIGN_PARAGRAPH);
    AddTabPage("tabs", RID_SVXPAGE_TABULATOR);
    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage("asiantypo", RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage("asiantypo");
}

void SdTabTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nLineId)
    {
        aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
        aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
        aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nAreaId)
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
    else if (nId == m_nShadowId)
    {
        aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTransparencyId)
    {
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nFontId)
    {
        SvxFontListItem aItem(*static_cast<const SvxFontListItem*>(
            rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) );

        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nFontEffectId)
    {
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nBackgroundId)
    {
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR)));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTextId)
    {
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nDimensionId)
    {
        aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nConnectorId)
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
