/*************************************************************************
 *
 *  $RCSfile: prltempl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST
#define ITEMID_ESCAPEMENT   SID_ATTR_CHAR_ESCAPEMENT
#define ITEMID_CASEMAP      SID_ATTR_CHAR_CASEMAP

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST

#include <svx/dialogs.hrc>
#include <svx/flstitem.hxx>
#include <svx/chardlg.hxx>
#include <svx/paragrph.hxx>
#include <svx/drawitem.hxx>
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SVX_TAB_LINE_HXX //autogen
#include <svx/tabline.hxx>
#endif
#ifndef _SVX_BULITEM_HXX //autogen
#include <svx/bulitem.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#define ITEMID_BRUSH    0
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#define ITEMID_LRSPACE          EE_PARA_LRSPACE
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#include <svx/numpages.hxx>
#include <svx/numitem.hxx>

#include "docshell.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "prltempl.hxx"
#include "prltempl.hrc"
#include "enumdlg.hrc"
#include "enumdlg.hxx"
#include "bulmaper.hxx"

#define IS_OUTLINE(x) (x >= PO_OUTLINE_1 && x <= PO_OUTLINE_9)

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

__EXPORT SdPresLayoutTemplateDlg::SdPresLayoutTemplateDlg( SfxObjectShell* pDocSh,
                                Window* pParent,
                                SdResId DlgId,
                                SfxStyleSheetBase& rStyleBase,
                                PresentationObjects _ePO,
                                SfxStyleSheetBasePool* pSSPool ) :
        SfxTabDialog        ( pParent, DlgId ),
        aInputSet           ( *rStyleBase.GetItemSet().GetPool(), SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL ),
        ePO                 ( _ePO ),
        pDocShell           ( pDocSh ),
        pOrgSet             ( &rStyleBase.GetItemSet() ),
        pOutSet             ( NULL )
{
    if( DlgId.GetId() == TAB_PRES_LAYOUT_TEMPLATE_3 && IS_OUTLINE(ePO))
    {
        // Leider sind die Itemsets unserer Stylesheets nicht discret..
        const USHORT* pPtr = pOrgSet->GetRanges();
        USHORT p1, p2;
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

        USHORT nWhich = aInputSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        const SfxPoolItem *pItem = NULL;

        // Fals in diesem Stylesheet kein Bullet Item ist, holen wir uns
        // das aus dem 'Outline 1' Stylesheet.
        if( SFX_ITEM_SET != aInputSet.GetItemState(nWhich, FALSE, &pItem ))
        {
            String aStyleName((SdResId(STR_PSEUDOSHEET_OUTLINE)));
            aStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " 1" ) );
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SFX_STYLE_FAMILY_PSEUDO);

            if(pFirstStyleSheet)
                if( SFX_ITEM_SET == pFirstStyleSheet->GetItemSet().GetItemState(nWhich, FALSE, &pItem) )
                    aInputSet.Put( *pItem );
        }

        // Jetzt noch das mapping von 10er auf 9er und des lrspace.
        if( SFX_ITEM_SET == aInputSet.GetItemState(nWhich, FALSE) )
        {
            SdBulletMapper::PreMapNumBulletForDialog( aInputSet );

            SvxNumBulletItem* pBulletItem = (SvxNumBulletItem*)aInputSet.GetItem(nWhich);
            SvxNumRule* pRule = pBulletItem->GetNumRule();
            if(pRule)
            {
                SvxLRSpaceItem aLRItem(EE_PARA_LRSPACE);
                if( SFX_ITEM_SET == aInputSet.GetItemState(EE_PARA_LRSPACE) )
                    aLRItem = *(SvxLRSpaceItem*)aInputSet.GetItem(EE_PARA_LRSPACE);

                const SvxNumberFormat& aActFmt = pRule->GetLevel( GetOutlineLevel() );

                aLRItem.SetTxtLeft( aActFmt.GetAbsLSpace() );
                aLRItem.SetTxtFirstLineOfst( aActFmt.GetFirstLineOffset() );

                aInputSet.Put(aLRItem);
            }
        }

        // gewaehlte Ebene im Dialog vorselektieren
        aInputSet.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, 1<<GetOutlineLevel()));

        SetInputSet( &aInputSet );
    }
    else
        SetInputSet( pOrgSet );

    FreeResource();

    SvxColorTableItem aColorTableItem(*( (const SvxColorTableItem*)
        ( pDocShell->GetItem( SID_COLOR_TABLE ) ) ) );
    SvxGradientListItem aGradientListItem(*( (const SvxGradientListItem*)
        ( pDocShell->GetItem( SID_GRADIENT_LIST ) ) ) );
    SvxBitmapListItem aBitmapListItem(*( (const SvxBitmapListItem*)
        ( pDocShell->GetItem( SID_BITMAP_LIST ) ) ) );
    SvxHatchListItem aHatchListItem(*( (const SvxHatchListItem*)
        ( pDocShell->GetItem( SID_HATCH_LIST ) ) ) );
    SvxDashListItem aDashListItem(*( (const SvxDashListItem*)
        ( pDocShell->GetItem( SID_DASH_LIST ) ) ) );
    SvxLineEndListItem aLineEndListItem(*( (const SvxLineEndListItem*)
        ( pDocShell->GetItem( SID_LINEEND_LIST ) ) ) );

    pColorTab = aColorTableItem.GetColorTable();
    pDashList = aDashListItem.GetDashList();
    pLineEndList = aLineEndListItem.GetLineEndList();
    pGradientList = aGradientListItem.GetGradientList();
    pHatchingList = aHatchListItem.GetHatchList();
    pBitmapList = aBitmapListItem.GetBitmapList();

    switch( DlgId.GetId() )
    {
        case TAB_PRES_LAYOUT_TEMPLATE:
            AddTabPage( RID_SVXPAGE_LINE, SvxLineTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_CHAR_STD, SvxCharStdPage::Create, 0);
            AddTabPage( RID_SVXPAGE_STD_PARAGRAPH, SvxStdParagraphTabPage::Create, 0);
        break;

        case TAB_PRES_LAYOUT_TEMPLATE_1:
            AddTabPage( RID_SVXPAGE_LINE, SvxLineTabPage::Create, 0);
        break;

        case TAB_PRES_LAYOUT_TEMPLATE_2:
            AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0);
        break;

        case TAB_PRES_LAYOUT_TEMPLATE_3:
            AddTabPage( RID_SVXPAGE_CHAR_STD, SvxCharStdPage::Create, 0);
            AddTabPage( RID_SVXPAGE_STD_PARAGRAPH, SvxStdParagraphTabPage::Create, 0);
            if(IS_OUTLINE(ePO))
            {
                AddTabPage(RID_SVXPAGE_PICK_SINGLE_NUM, &SvxSingleNumPickTabPage::Create, 0);
                AddTabPage(RID_SVXPAGE_PICK_BULLET    , &SvxBulletPickTabPage::Create, 0);
                AddTabPage(RID_SVXPAGE_PICK_BMP       , &SvxBitmapPickTabPage::Create, 0);
                AddTabPage(RID_SVXPAGE_NUM_OPTIONS    , &SvxNumOptionsTabPage::Create, 0);
            }
            else
            {
                RemoveTabPage( RID_SVXPAGE_PICK_SINGLE_NUM );
                RemoveTabPage( RID_SVXPAGE_PICK_BULLET );
                RemoveTabPage( RID_SVXPAGE_PICK_BMP );
                RemoveTabPage( RID_SVXPAGE_NUM_OPTIONS );
            }
        break;
    }
    // Titel setzen und
    // entsprechende Seiten zum Dialog hinzufuegen
    String aTitle;

    switch( ePO )
    {
        case PO_TITLE:
            aTitle = String(SdResId( STR_PSEUDOSHEET_TITLE ));
            RemoveTabPage( TP_ENUM_BULLET );
        break;

        case PO_SUBTITLE:
            aTitle = String(SdResId( STR_PSEUDOSHEET_SUBTITLE ));
            RemoveTabPage( TP_ENUM_BULLET );
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
            RemoveTabPage( TP_ENUM_BULLET );
        break;
    }
    SetText( aTitle );

//  SetPalette( XOutputDevice::GetDitherPalette() );

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

void __EXPORT SdPresLayoutTemplateDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
        {
            ( (SvxLineTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxLineTabPage&) rPage ).SetDashList( pDashList );
            ( (SvxLineTabPage&) rPage ).SetLineEndList( pLineEndList );
            //( (SvxLineTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineTabPage&) rPage ).SetDlgType( &nDlgType );
            //( (SvxLineTabPage&) rPage ).SetPos( &nPos );
            ( (SvxLineTabPage&) rPage ).Construct();
            ( (SvxLineTabPage&) rPage ).SetDashChgd( &nDashListState );
            ( (SvxLineTabPage&) rPage ).SetLineEndChgd( &nLineEndListState );
        }
        break;

        case RID_SVXPAGE_AREA:
        {

            ( (SvxAreaTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxAreaTabPage&) rPage ).SetGradientList( pGradientList );
            ( (SvxAreaTabPage&) rPage ).SetHatchingList( pHatchingList );
            ( (SvxAreaTabPage&) rPage ).SetBitmapList( pBitmapList );
            ( (SvxAreaTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxAreaTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxAreaTabPage&) rPage ).SetPos( &nPos );
            ( (SvxAreaTabPage&) rPage ).SetGrdChgd( &nGradientListState );
            ( (SvxAreaTabPage&) rPage ).SetHtchChgd( &nHatchingListState );
            ( (SvxAreaTabPage&) rPage ).SetBmpChgd( &nBitmapListState );
            ( (SvxAreaTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxAreaTabPage&) rPage ).Construct();
        }
        break;

        case RID_SVXPAGE_SHADOW:
            ( (SvxShadowTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxShadowTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxShadowTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_TRANSPARENCE:
            ( (SvxTransparenceTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxTransparenceTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxTransparenceTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_CHAR_STD:
        {
            SvxFontListItem aItem(*( (const SvxFontListItem*)
                ( pDocShell->GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

            ( (SvxCharStdPage&) rPage ).SetFontList( aItem );
        }
        break;

        case RID_SVXPAGE_STD_PARAGRAPH:
        break;
    }
}

const SfxItemSet* SdPresLayoutTemplateDlg::GetOutputItemSet()
{
    if( pOutSet )
    {
        pOutSet->Put( *SfxTabDialog::GetOutputItemSet() );

        const SvxNumBulletItem *pSvxNumBulletItem = NULL;
        USHORT nWhich = pOutSet->GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);

        if( SFX_ITEM_SET == pOutSet->GetItemState(nWhich, FALSE, (const SfxPoolItem**)&pSvxNumBulletItem ))
            SdBulletMapper::MapFontsInNumRule( *pSvxNumBulletItem->GetNumRule(), *pOutSet );

        // Wenn das lrspace geaendert wurde muss die Aenderung in das
        // Bullet Item gemapt werden...
        if( SFX_ITEM_SET == pOutSet->GetItemState( EE_PARA_LRSPACE, FALSE ) )
        {
            BOOL bNumBulletHasChanged = pSvxNumBulletItem != NULL;
            if( !bNumBulletHasChanged )
                pSvxNumBulletItem = (SvxNumBulletItem*)aInputSet.GetItem(nWhich);

            if( pSvxNumBulletItem )
            {
                SvxLRSpaceItem& rLRItem = *(SvxLRSpaceItem*)pOutSet->GetItem( ITEMID_LRSPACE );

                SvxNumRule* pNumRule = pSvxNumBulletItem->GetNumRule();
                if(pNumRule)
                {
                    SvxNumberFormat aFrmt( pNumRule->GetLevel( GetOutlineLevel() ));

                    aFrmt.SetLSpace( rLRItem.GetTxtLeft() );
                    aFrmt.SetAbsLSpace( rLRItem.GetTxtLeft() );
                    aFrmt.SetFirstLineOffset( rLRItem.GetTxtFirstLineOfst() );
                    pNumRule->SetLevel( GetOutlineLevel(), aFrmt );
                }
            }

            if( !bNumBulletHasChanged && pSvxNumBulletItem )
                pOutSet->Put( SvxNumBulletItem( *pSvxNumBulletItem->GetNumRule(), nWhich ) );
        }

        SdBulletMapper::PostMapNumBulletForDialog( *pOutSet );

        return pOutSet;
    }
    else
        return SfxTabDialog::GetOutputItemSet();
}

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
USHORT SdPresLayoutTemplateDlg::GetOutlineLevel() const
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
        DBG_ASSERT( FALSE, "Falscher Po! [CL]");
    }
    return 0;
}



