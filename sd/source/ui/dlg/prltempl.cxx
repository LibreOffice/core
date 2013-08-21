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

#include <editeng/outliner.hxx>

#include <svx/dialogs.hrc>
#include <editeng/flstitem.hxx>
#include <svx/drawitem.hxx>
#include <svl/style.hxx>
#include <svx/tabline.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/graph.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/numitem.hxx>
#include <svl/cjkoptions.hxx>

#include "DrawDocShell.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "prltempl.hxx"
#include "prltempl.hrc"
#include "bulmaper.hxx"
#include <svl/intitem.hxx>
#include <svx/svxgrahicitem.hxx>
#include <svx/flagsdef.hxx>
#include "drawdoc.hxx"
#define IS_OUTLINE(x) (x >= PO_OUTLINE_1 && x <= PO_OUTLINE_9)

/**
 * Constructor of Tab dialog: appends pages to the dialog
 */
SdPresLayoutTemplateDlg::SdPresLayoutTemplateDlg( SfxObjectShell* pDocSh,
                                Window* pParent,
                                SdResId DlgId,
                                SfxStyleSheetBase& rStyleBase,
                                PresentationObjects _ePO,
                                SfxStyleSheetBasePool* pSSPool ) :
        SfxTabDialog        ( pParent, DlgId ),
        mpDocShell          ( pDocSh ),
        ePO                 ( _ePO ),
        aInputSet           ( *rStyleBase.GetItemSet().GetPool(), SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL ),
        pOutSet             ( NULL ),
        pOrgSet             ( &rStyleBase.GetItemSet() )
{
    if( IS_OUTLINE(ePO))
    {
        // Unfortunately, the Itemsets of our style sheets are not discreet..
        const sal_uInt16* pPtr = pOrgSet->GetRanges();
        sal_uInt16 p1, p2;
        while( *pPtr )
        {
            p1 = pPtr[0];
            p2 = pPtr[1];

            // first, we make it discreet
            while(pPtr[2] && (pPtr[2] - p2 == 1))
            {
                p2 = pPtr[3];
                pPtr += 2;
            }
            aInputSet.MergeRange( p1, p2 );
            pPtr += 2;
        }

        aInputSet.Put( rStyleBase.GetItemSet() );

        // need parent-relationship
        const SfxItemSet* pParentItemSet = rStyleBase.GetItemSet().GetParent();
        if( pParentItemSet )
            aInputSet.SetParent( pParentItemSet );

        pOutSet = new SfxItemSet( rStyleBase.GetItemSet() );
        pOutSet->ClearItem();

        const SfxPoolItem *pItem = NULL;

        // If there is no bullet item in this stylesheet, we get it
        // from 'Outline 1' style sheet.
        if( SFX_ITEM_SET != aInputSet.GetItemState(EE_PARA_NUMBULLET, sal_False, &pItem ))
        {
            OUString aStyleName(SD_RESSTR(STR_PSEUDOSHEET_OUTLINE) + " 1");
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);

            if(pFirstStyleSheet)
                if( SFX_ITEM_SET == pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, sal_False, &pItem) )
                    aInputSet.Put( *pItem );
        }

        // preselect selected layer in dialog
        aInputSet.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, 1<<GetOutlineLevel()));

        SetInputSet( &aInputSet );
    }
    else
        SetInputSet( pOrgSet );

    FreeResource();

    SvxColorListItem aColorListItem(*( (const SvxColorListItem*)
        ( mpDocShell->GetItem( SID_COLOR_TABLE ) ) ) );
    SvxGradientListItem aGradientListItem(*( (const SvxGradientListItem*)
        ( mpDocShell->GetItem( SID_GRADIENT_LIST ) ) ) );
    SvxBitmapListItem aBitmapListItem(*( (const SvxBitmapListItem*)
        ( mpDocShell->GetItem( SID_BITMAP_LIST ) ) ) );
    SvxHatchListItem aHatchListItem(*( (const SvxHatchListItem*)
        ( mpDocShell->GetItem( SID_HATCH_LIST ) ) ) );
    SvxDashListItem aDashListItem(*( (const SvxDashListItem*)
        ( mpDocShell->GetItem( SID_DASH_LIST ) ) ) );
    SvxLineEndListItem aLineEndListItem(*( (const SvxLineEndListItem*)
        ( mpDocShell->GetItem( SID_LINEEND_LIST ) ) ) );

    pColorTab = aColorListItem.GetColorList();
    pDashList = aDashListItem.GetDashList();
    pLineEndList = aLineEndListItem.GetLineEndList();
    pGradientList = aGradientListItem.GetGradientList();
    pHatchingList = aHatchListItem.GetHatchList();
    pBitmapList = aBitmapListItem.GetBitmapList();

    switch( DlgId.GetId() )
    {
        case TAB_PRES_LAYOUT_TEMPLATE:
        {
            AddTabPage( RID_SVXPAGE_LINE);
            AddTabPage( RID_SVXPAGE_AREA);
            AddTabPage( RID_SVXPAGE_SHADOW);
            AddTabPage( RID_SVXPAGE_TRANSPARENCE);
            AddTabPage( RID_SVXPAGE_CHAR_NAME );
            AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
            AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );
            AddTabPage( RID_SVXPAGE_TEXTATTR );
            AddTabPage( RID_SVXPAGE_PICK_BULLET );
            AddTabPage( RID_SVXPAGE_PICK_SINGLE_NUM );
            AddTabPage( RID_SVXPAGE_PICK_BMP );
            AddTabPage( RID_SVXPAGE_NUM_OPTIONS );
            AddTabPage( RID_SVXPAGE_TABULATOR );
        }
        break;

        case TAB_PRES_LAYOUT_TEMPLATE_BACKGROUND:        // background
            AddTabPage( RID_SVXPAGE_AREA);
        break;
    }

    // the tabpages Alignment, Tabs and Asian Typography are very
    // useful, except for the background style
    if( DlgId.GetId() != TAB_PRES_LAYOUT_TEMPLATE_BACKGROUND )
    {
        SvtCJKOptions aCJKOptions;
        if( aCJKOptions.IsAsianTypographyEnabled() )
            AddTabPage( RID_SVXPAGE_PARA_ASIAN );
        else
            RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

        AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );
    }

    // set title and add corresponding pages to dialog
    OUString aTitle;

    switch( ePO )
    {
        case PO_TITLE:
            aTitle = OUString(SdResId( STR_PSEUDOSHEET_TITLE ));
        break;

        case PO_SUBTITLE:
            aTitle = OUString(SdResId( STR_PSEUDOSHEET_SUBTITLE ));
        break;

        case PO_BACKGROUND:
            aTitle = OUString(SdResId( STR_PSEUDOSHEET_BACKGROUND ));
        break;

        case PO_BACKGROUNDOBJECTS:
            aTitle = OUString(SdResId( STR_PSEUDOSHEET_BACKGROUNDOBJECTS ));
        break;

        case PO_OUTLINE_1:
        case PO_OUTLINE_2:
        case PO_OUTLINE_3:
        case PO_OUTLINE_4:
        case PO_OUTLINE_5:
        case PO_OUTLINE_6:
        case PO_OUTLINE_7:
        case PO_OUTLINE_8:
        case PO_OUTLINE_9:
            aTitle = OUString(SdResId( STR_PSEUDOSHEET_OUTLINE )) + " " +
                OUString::number( ePO - PO_OUTLINE_1 + 1 );
        break;

        case PO_NOTES:
            aTitle = OUString(SdResId( STR_PSEUDOSHEET_NOTES ));
        break;
    }
    SetText( aTitle );

    nDlgType = 1; // template dialog
    nPageType = 0;
    nPos = 0;

    nColorTableState = CT_NONE;
    nBitmapListState = CT_NONE;
    nGradientListState = CT_NONE;
    nHatchingListState = CT_NONE;
}

// -----------------------------------------------------------------------

SdPresLayoutTemplateDlg::~SdPresLayoutTemplateDlg()
{
    delete pOutSet;
}

// -----------------------------------------------------------------------

void SdPresLayoutTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{   SfxAllItemSet aSet(*(aInputSet.GetPool()));
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
        {
            aSet.Put (SvxColorListItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
            aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));

            rPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_AREA:
        {
            aSet.Put (SvxColorListItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxGradientListItem(pGradientList,SID_GRADIENT_LIST));
            aSet.Put (SvxHatchListItem(pHatchingList,SID_HATCH_LIST));
            aSet.Put (SvxBitmapListItem(pBitmapList,SID_BITMAP_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,nPos));
            rPage.PageCreated(aSet);

        }
        break;

        case RID_SVXPAGE_SHADOW:
                aSet.Put (SvxColorListItem(pColorTab,SID_COLOR_TABLE));
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
                ( mpDocShell->GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

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
    }
}

const SfxItemSet* SdPresLayoutTemplateDlg::GetOutputItemSet() const
{
    if( pOutSet )
    {
        pOutSet->Put( *SfxTabDialog::GetOutputItemSet() );

        const SvxNumBulletItem *pSvxNumBulletItem = NULL;
        if( SFX_ITEM_SET == pOutSet->GetItemState(EE_PARA_NUMBULLET, sal_False, (const SfxPoolItem**)&pSvxNumBulletItem ))
            SdBulletMapper::MapFontsInNumRule( *pSvxNumBulletItem->GetNumRule(), *pOutSet );
        return pOutSet;
    }
    else
        return SfxTabDialog::GetOutputItemSet();
}

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
sal_uInt16 SdPresLayoutTemplateDlg::GetOutlineLevel() const
{
    switch( ePO )
    {
    case PO_OUTLINE_1: return 0;
    case PO_OUTLINE_2: return 1;
    case PO_OUTLINE_3: return 2;
    case PO_OUTLINE_4: return 3;
    case PO_OUTLINE_5: return 4;
    case PO_OUTLINE_6: return 5;
    case PO_OUTLINE_7: return 6;
    case PO_OUTLINE_8: return 7;
    case PO_OUTLINE_9: return 8;
    default:
        DBG_ASSERT( sal_False, "Wrong Po! [CL]");
    }
    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
