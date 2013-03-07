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


#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

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
#include "tabtempl.hrc"
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
                                SdrView* pView ) :
        SfxStyleDialog      ( pParent, SdResId( TAB_TEMPLATE ), rStyleBase, sal_False ),
        rDocShell           ( *pDocShell ),
        pSdrView            ( pView ),
        pColorList          ( pModel->GetColorList() ),
        pGradientList       ( pModel->GetGradientList() ),
        pHatchingList       ( pModel->GetHatchList() ),
        pBitmapList         ( pModel->GetBitmapList() ),
        pDashList           ( pModel->GetDashList() ),
        pLineEndList        ( pModel->GetLineEndList() )
{
    FreeResource();

    // fill Listbox and overload Select-Handler

    AddTabPage( RID_SVXPAGE_LINE);
    AddTabPage( RID_SVXPAGE_AREA);
    AddTabPage( RID_SVXPAGE_SHADOW);
    AddTabPage( RID_SVXPAGE_TRANSPARENCE);
    AddTabPage( RID_SVXPAGE_CHAR_NAME );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );
    AddTabPage( RID_SVXPAGE_TEXTATTR );
    AddTabPage( RID_SVXPAGE_TEXTANIMATION );
    AddTabPage( RID_SVXPAGE_MEASURE);
    AddTabPage( RID_SVXPAGE_CONNECTION);
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );
    AddTabPage( RID_SVXPAGE_TABULATOR );
    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( RID_SVXPAGE_PARA_ASIAN );
    else
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

    nDlgType = 1;
    nPageType = 0;
    nPos = 0;

    nColorTableState = CT_NONE;
    nBitmapListState = CT_NONE;
    nGradientListState = CT_NONE;
    nHatchingListState = CT_NONE;
}

// -----------------------------------------------------------------------

SdTabTemplateDlg::~SdTabTemplateDlg()
{
}

// -----------------------------------------------------------------------

void SdTabTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
            aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
            aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
            aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_AREA:
            aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
            aSet.Put (SvxGradientListItem(pGradientList,SID_GRADIENT_LIST));
            aSet.Put (SvxHatchListItem(pHatchingList,SID_HATCH_LIST));
            aSet.Put (SvxBitmapListItem(pBitmapList,SID_BITMAP_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,nPos));
            rPage.PageCreated(aSet);


        break;

        case RID_SVXPAGE_SHADOW:
                aSet.Put (SvxColorListItem(pColorList,SID_COLOR_TABLE));
                aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
                aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
                rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_TRANSPARENCE:
                    aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
                    aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
                    rPage.PageCreated(aSet);
        break;

        case RID_SVXPAGE_CHAR_NAME:
        {
            SvxFontListItem aItem(*( (const SvxFontListItem*)
                ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

            aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
            rPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
            rPage.PageCreated(aSet);
        break;

        case RID_SVXPAGE_STD_PARAGRAPH:
        break;

        case RID_SVXPAGE_TEXTATTR:
        {
            aSet.Put(OfaPtrItem(SID_SVXTEXTATTRPAGE_VIEW,pSdrView));
            rPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_TEXTANIMATION:
        break;

        case RID_SVXPAGE_MEASURE:
            aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));
            rPage.PageCreated(aSet);
        break;

        case RID_SVXPAGE_CONNECTION:
        {
            aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));
            rPage.PageCreated(aSet);
        }
        break;
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
