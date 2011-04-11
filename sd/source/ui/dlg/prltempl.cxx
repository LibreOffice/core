/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif


#include "eetext.hxx"

#include <svx/dialogs.hrc>
#include <editeng/flstitem.hxx>
#include <svx/drawitem.hxx>
#include <svl/style.hxx>
#include <svx/tabline.hxx>
#include <editeng/bulitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/brshitem.hxx>
#include <vcl/graph.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/numitem.hxx>
#include <svl/cjkoptions.hxx>

#include "DrawDocShell.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "prltempl.hxx"
#include "prltempl.hrc"
#include "enumdlg.hrc"
#include "bulmaper.hxx"
#include <svl/intitem.hxx>
#include <svx/svxgrahicitem.hxx>
#include <svx/flagsdef.hxx>
#include "drawdoc.hxx"
#define IS_OUTLINE(x) (x >= PO_OUTLINE_1 && x <= PO_OUTLINE_9)

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

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
        // Leider sind die Itemsets unserer Stylesheets nicht discret..
        const sal_uInt16* pPtr = pOrgSet->GetRanges();
        sal_uInt16 p1, p2;
        while( *pPtr )
        {
            p1 = pPtr[0];
            p2 = pPtr[1];

            // erstmal das ganze discret machen
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
        const SfxItemSet* pParentItemSet = rStyleBase.GetItemSet().GetParent();;
        if( pParentItemSet )
            aInputSet.SetParent( pParentItemSet );

        pOutSet = new SfxItemSet( rStyleBase.GetItemSet() );
        pOutSet->ClearItem();

        const SfxPoolItem *pItem = NULL;

        // Fals in diesem Stylesheet kein Bullet Item ist, holen wir uns
        // das aus dem 'Outline 1' Stylesheet.
        if( SFX_ITEM_SET != aInputSet.GetItemState(EE_PARA_NUMBULLET, sal_False, &pItem ))
        {
            String aStyleName((SdResId(STR_PSEUDOSHEET_OUTLINE)));
            aStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " 1" ) );
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);

            if(pFirstStyleSheet)
                if( SFX_ITEM_SET == pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, sal_False, &pItem) )
                    aInputSet.Put( *pItem );
        }

        // gewaehlte Ebene im Dialog vorselektieren
        aInputSet.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, 1<<GetOutlineLevel()));

        SetInputSet( &aInputSet );
    }
    else
        SetInputSet( pOrgSet );

    FreeResource();

    SvxColorTableItem aColorTableItem(*( (const SvxColorTableItem*)
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

    pColorTab = aColorTableItem.GetColorTable();
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
    // usefull, except for the background style
    if( DlgId.GetId() != TAB_PRES_LAYOUT_TEMPLATE_BACKGROUND )
    {
        SvtCJKOptions aCJKOptions;
        if( aCJKOptions.IsAsianTypographyEnabled() )
            AddTabPage( RID_SVXPAGE_PARA_ASIAN );
        else
            RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

        AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );
    }

    // Titel setzen und
    // entsprechende Seiten zum Dialog hinzufuegen
    String aTitle;

    switch( ePO )
    {
        case PO_TITLE:
            aTitle = String(SdResId( STR_PSEUDOSHEET_TITLE ));
        break;

        case PO_SUBTITLE:
            aTitle = String(SdResId( STR_PSEUDOSHEET_SUBTITLE ));
        break;

        case PO_BACKGROUND:
            aTitle = String(SdResId( STR_PSEUDOSHEET_BACKGROUND ));
        break;

        case PO_BACKGROUNDOBJECTS:
            aTitle = String(SdResId( STR_PSEUDOSHEET_BACKGROUNDOBJECTS ));
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
            aTitle = String(SdResId( STR_PSEUDOSHEET_OUTLINE ));
            aTitle.Append( sal_Unicode(' ') );
            aTitle.Append( UniString::CreateFromInt32( ePO - PO_OUTLINE_1 + 1 ) );
        break;

        case PO_NOTES:
            aTitle = String(SdResId( STR_PSEUDOSHEET_NOTES ));
        break;
    }
    SetText( aTitle );

    nDlgType = 1; // Vorlagen-Dialog
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
            aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
            aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));

            rPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_AREA:
        {
            aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE));
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
                aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE));
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
        DBG_ASSERT( sal_False, "Falscher Po! [CL]");
    }
    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
