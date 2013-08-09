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
#include <svx/svxgrahicitem.hxx>
#include <svx/svdmodel.hxx>
#include <svl/cjkoptions.hxx>


#include <svx/dialogs.hrc>

#include <svx/svxdlg.hxx>
#include <svx/tabline.hxx>
#include <svl/style.hxx>
#include <svx/xtable.hxx>

#include "DrawDocShell.hxx"
#include "tabtempl.hxx"
#include "sdresid.hxx"
#include "dlg_char.hxx"
#include "paragr.hxx"
#include <svx/flagsdef.hxx>

/**
 * Constructor of the Tab dialog: appends pages to the dialog
 */
SdTabTemplateDlg::SdTabTemplateDlg( Window* pParent,
                                const SfxObjectShell* pDocShell,
                                SfxStyleSheetBase& rStyleBase,
                                SdrModel* pModel,
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
    , pDashList(pModel->GetDashList())
    , pLineEndList(pModel->GetLineEndList())
    , m_nLineId(0)
    , m_nAreaId(0)
    , m_nShadowId(0)
    , m_nTransparencyId(0)
    , m_nFontId(0)
    , m_nFontEffectId(0)
    , m_nIndentsId(0)
    , m_nTextId(0)
    , m_nAnimationId(0)
    , m_nDimensionId(0)
    , m_nConnectorId(0)
    , m_nAlignId(0)
    , m_nTabId(0)
    , m_nAsianTypoId(0)
{
    // fill Listbox and overload Select-Handler

    m_nLineId = AddTabPage("line", RID_SVXPAGE_LINE);
    m_nAreaId = AddTabPage("area", RID_SVXPAGE_AREA);
    m_nShadowId = AddTabPage("shadowing", RID_SVXPAGE_SHADOW);
    m_nTransparencyId = AddTabPage("transparency", RID_SVXPAGE_TRANSPARENCE);
    m_nFontId = AddTabPage("font", RID_SVXPAGE_CHAR_NAME);
    m_nFontEffectId = AddTabPage("fonteffect", RID_SVXPAGE_CHAR_EFFECTS);
    m_nIndentsId = AddTabPage("indents", RID_SVXPAGE_STD_PARAGRAPH);
    m_nTextId = AddTabPage("text", RID_SVXPAGE_TEXTATTR);
    m_nAnimationId = AddTabPage("animation", RID_SVXPAGE_TEXTANIMATION);
    m_nDimensionId = AddTabPage("dimensioning", RID_SVXPAGE_MEASURE);
    m_nConnectorId = AddTabPage("connector", RID_SVXPAGE_CONNECTION);
    m_nAlignId = AddTabPage("alignment", RID_SVXPAGE_ALIGN_PARAGRAPH);
    m_nTabId = AddTabPage("tabs", RID_SVXPAGE_TABULATOR);
    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        m_nAsianTypoId = AddTabPage("asiantype", RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage("asiantypo");

    nDlgType = 1;
    nPageType = 0;
    nPos = 0;

    nColorTableState = CT_NONE;
    nBitmapListState = CT_NONE;
    nGradientListState = CT_NONE;
    nHatchingListState = CT_NONE;
}

// -----------------------------------------------------------------------

void SdTabTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nLineId)
    {
        aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
        aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
        aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nAreaId)
    {
        aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
        aSet.Put (SvxGradientListItem(pGradientList,SID_GRADIENT_LIST));
        aSet.Put (SvxHatchListItem(pHatchingList,SID_HATCH_LIST));
        aSet.Put (SvxBitmapListItem(pBitmapList,SID_BITMAP_LIST));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
        aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,nPos));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nShadowId)
    {
        aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTransparencyId)
    {
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nFontId)
    {
        SvxFontListItem aItem(*( (const SvxFontListItem*)
            ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nFontEffectId)
    {
        aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTextId)
    {
        aSet.Put(OfaPtrItem(SID_SVXTEXTATTRPAGE_VIEW,pSdrView));
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

// -----------------------------------------------------------------------

const SfxItemSet* SdTabTemplateDlg::GetRefreshedSet()
{
    SfxItemSet* pRet = GetInputSetImpl();

    if( pRet )
    {
        pRet->ClearItem();
        pRet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
    }
    else
        pRet = new SfxItemSet( GetStyleSheet().GetItemSet() );

    return pRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
