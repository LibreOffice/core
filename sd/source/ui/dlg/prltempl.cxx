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
#include <svx/svxdlg.hxx>
#include <editeng/flstitem.hxx>
#include <svx/drawitem.hxx>
#include <svl/style.hxx>
#include <svx/tabline.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/graphic.hxx>
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
                                vcl::Window* pParent,
                                SdResId DlgId,
                                SfxStyleSheetBase& rStyleBase,
                                PresentationObjects _ePO,
                                SfxStyleSheetBasePool* pSSPool ) :
        SfxTabDialog        ( pParent
                            , "DrawPRTLDialog"
                            , "modules/sdraw/ui/drawprtldialog.ui"),
        mpDocShell          ( pDocSh ),
        ePO                 ( _ePO ),
        aInputSet           ( *rStyleBase.GetItemSet().GetPool(), SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL ),
        pOutSet             ( nullptr ),
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

        const SfxPoolItem *pItem = nullptr;

        // If there is no bullet item in this stylesheet, we get it
        // from 'Outline 1' style sheet.
        if( SfxItemState::SET != aInputSet.GetItemState(EE_PARA_NUMBULLET, false, &pItem ))
        {
            OUString aStyleName(SD_RESSTR(STR_PSEUDOSHEET_OUTLINE) + " 1");
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);

            if(pFirstStyleSheet)
                if( SfxItemState::SET == pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, &pItem) )
                    aInputSet.Put( *pItem );
        }

        // preselect selected layer in dialog
        aInputSet.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, 1<<GetOutlineLevel()));

        SetInputSet( &aInputSet );
    }
    else
        SetInputSet( pOrgSet );

    SvxColorListItem aColorListItem(*static_cast<const SvxColorListItem*>( mpDocShell->GetItem( SID_COLOR_TABLE ) ) );
    SvxGradientListItem aGradientListItem(*static_cast<const SvxGradientListItem*>( mpDocShell->GetItem( SID_GRADIENT_LIST ) ) );
    SvxBitmapListItem aBitmapListItem(*static_cast<const SvxBitmapListItem*>( mpDocShell->GetItem( SID_BITMAP_LIST ) ) );
    SvxHatchListItem aHatchListItem(*static_cast<const SvxHatchListItem*>( mpDocShell->GetItem( SID_HATCH_LIST ) ) );
    SvxDashListItem aDashListItem(*static_cast<const SvxDashListItem*>( mpDocShell->GetItem( SID_DASH_LIST ) ) );
    SvxLineEndListItem aLineEndListItem(*static_cast<const SvxLineEndListItem*>( mpDocShell->GetItem( SID_LINEEND_LIST ) ) );

    pColorTab = aColorListItem.GetColorList();
    pDashList = aDashListItem.GetDashList();
    pLineEndList = aLineEndListItem.GetLineEndList();
    pGradientList = aGradientListItem.GetGradientList();
    pHatchingList = aHatchListItem.GetHatchList();
    pBitmapList = aBitmapListItem.GetBitmapList();

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");

    mnLine = AddTabPage( "RID_SVXPAGE_LINE", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINE ), nullptr );
    mnArea = AddTabPage( "RID_SVXPAGE_AREA", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), nullptr );
    mnShadow = AddTabPage( "RID_SVXPAGE_SHADOW", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_SHADOW ), nullptr );
    mnTransparency = AddTabPage( "RID_SVXPAGE_TRANSPARENCE", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), nullptr );
    mnFont = AddTabPage( "RID_SVXPAGE_CHAR_NAME", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), nullptr );
    mnEffects = AddTabPage( "RID_SVXPAGE_CHAR_EFFECTS", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), nullptr );
    mnParagr = AddTabPage( "RID_SVXPAGE_STD_PARAGRAPH", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_STD_PARAGRAPH ), nullptr );
    mnTextAtt = AddTabPage( "RID_SVXPAGE_TEXTATTR", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TEXTATTR ), nullptr );
    mnBullet = AddTabPage( "RID_SVXPAGE_PICK_BULLET", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PICK_BULLET ), nullptr );
    mnNum = AddTabPage( "RID_SVXPAGE_PICK_SINGLE_NUM", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PICK_SINGLE_NUM ), nullptr );
    mnBitmap = AddTabPage( "RID_SVXPAGE_PICK_BMP", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PICK_BMP ), nullptr );
    mnOptions = AddTabPage( "RID_SVXPAGE_NUM_OPTIONS", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUM_OPTIONS ), nullptr );
    mnTab =  AddTabPage( "RID_SVXPAGE_TABULATOR", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TABULATOR ), nullptr );
    mnAsian = AddTabPage( "RID_SVXPAGE_PARA_ASIAN", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PARA_ASIAN ), nullptr );
    mnAlign = AddTabPage( "RID_SVXPAGE_ALIGN_PARAGRAPH", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGN_PARAGRAPH ), nullptr );

    SvtCJKOptions aCJKOptions;
    if( !aCJKOptions.IsAsianTypographyEnabled() )
        RemoveTabPage( "RID_SVXPAGE_PARA_ASIAN" );

    if (DlgId.GetId() == TAB_PRES_LAYOUT_TEMPLATE_BACKGROUND)
    {
        RemoveTabPage( "RID_SVXPAGE_LINE");

        RemoveTabPage( "RID_SVXPAGE_SHADOW");
        RemoveTabPage( "RID_SVXPAGE_TRANSPARENCE");
        RemoveTabPage( "RID_SVXPAGE_CHAR_NAME");
        RemoveTabPage( "RID_SVXPAGE_CHAR_EFFECTS");
        RemoveTabPage( "RID_SVXPAGE_STD_PARAGRAPH");
        RemoveTabPage( "RID_SVXPAGE_TEXTATTR");
        RemoveTabPage( "RID_SVXPAGE_PICK_BULLET");
        RemoveTabPage( "RID_SVXPAGE_PICK_SINGLE_NUM");
        RemoveTabPage( "RID_SVXPAGE_PICK_BMP");
        RemoveTabPage( "RID_SVXPAGE_NUM_OPTIONS");
        RemoveTabPage( "RID_SVXPAGE_TABULATOR");
        RemoveTabPage( "RID_SVXPAGE_ALIGN_PARAGRAPH");
        RemoveTabPage( "RID_SVXPAGE_PARA_ASIAN" );
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

    nColorTableState = ChangeType::NONE;
    nBitmapListState = ChangeType::NONE;
    nGradientListState = ChangeType::NONE;
    nHatchingListState = ChangeType::NONE;
}

SdPresLayoutTemplateDlg::~SdPresLayoutTemplateDlg()
{
    disposeOnce();
}

void SdPresLayoutTemplateDlg::dispose()
{
    delete pOutSet;
    SfxTabDialog::dispose();
}

void SdPresLayoutTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(aInputSet.GetPool()));

    if (nId == mnLine)
        {
            aSet.Put (SvxColorListItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
            aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
        rPage.PageCreated(aSet);
        }

    else if (nId == mnArea)
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

    else if (nId == mnShadow)
        {
            aSet.Put (SvxColorListItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
        }

    else if (nId == mnTransparency)
        {
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
        }

    else if (nId == mnFont)
        {
            SvxFontListItem aItem(*static_cast<const SvxFontListItem*>(mpDocShell->GetItem( SID_ATTR_CHAR_FONTLIST) ) );
            aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
            rPage.PageCreated(aSet);
        }

    else if (nId == mnEffects)
        {
            rPage.PageCreated(aSet);
        }
}

const SfxItemSet* SdPresLayoutTemplateDlg::GetOutputItemSet() const
{
    if( pOutSet )
    {
        pOutSet->Put( *SfxTabDialog::GetOutputItemSet() );

        const SvxNumBulletItem *pSvxNumBulletItem = nullptr;
        if( SfxItemState::SET == pOutSet->GetItemState(EE_PARA_NUMBULLET, false, reinterpret_cast<const SfxPoolItem**>(&pSvxNumBulletItem) ))
            SdBulletMapper::MapFontsInNumRule( *pSvxNumBulletItem->GetNumRule(), *pOutSet );
        return pOutSet;
    }
    else
        return SfxTabDialog::GetOutputItemSet();
}

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
        SAL_WARN( "sd", "Wrong Po! [CL]");
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
