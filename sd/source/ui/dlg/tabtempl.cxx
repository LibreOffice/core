/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabtempl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:10:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <svx/flstitem.hxx>

#include <svx/svxids.hrc>

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST            SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST
#include <svx/drawitem.hxx>
#include <svtools/intitem.hxx>
#include <svx/ofaitem.hxx>
#include <svx/svxgrahicitem.hxx>
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif

#ifndef _EEITEMID_HXX //autogen
#include <svx/eeitemid.hxx>
#endif

#include <svx/dialogs.hrc>

#ifdef ITEMID_FONTLIST
#undef ITEMID_FONTLIST
#endif
#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST

#ifdef ITEMID_ESCAPEMENT
#undef ITEMID_ESCAPEMENT
#endif
#define ITEMID_ESCAPEMENT   SID_ATTR_CHAR_ESCAPEMENT

#ifdef ITEMID_CASEMAP
#undef ITEMID_CASEMAP
#endif
#define ITEMID_CASEMAP      SID_ATTR_CHAR_CASEMAP

#include <svx/svxdlg.hxx>

#ifndef _SVX_TAB_LINE_HXX //autogen
#include <svx/tabline.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif

#include "DrawDocShell.hxx"
#include "tabtempl.hxx"
#include "tabtempl.hrc"
#include "sdresid.hxx"
#include "dlg_char.hxx"
#include "paragr.hxx"
#include <svx/flagsdef.hxx>

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdTabTemplateDlg::SdTabTemplateDlg( Window* pParent,
                                const SfxObjectShell* pDocShell,
                                SfxStyleSheetBase& rStyleBase,
                                SdrModel* pModel,
                                SdrView* pView ) :
        SfxStyleDialog      ( pParent, SdResId( TAB_TEMPLATE ), rStyleBase, FALSE ),
        rDocShell           ( *pDocShell ),
        pSdrView            ( pView ),
        pColorTab           ( pModel->GetColorTable() ),
        pGradientList       ( pModel->GetGradientList() ),
        pHatchingList       ( pModel->GetHatchList() ),
        pBitmapList         ( pModel->GetBitmapList() ),
        pDashList           ( pModel->GetDashList() ),
        pLineEndList        ( pModel->GetLineEndList() )
{
    FreeResource();

    // Listbox fuellen und Select-Handler ueberladen

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

void SdTabTemplateDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
            aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
            aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_AREA:
            aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxGradientListItem(pGradientList,SID_GRADIENT_LIST));
            aSet.Put (SvxHatchListItem(pHatchingList,SID_HATCH_LIST));
            aSet.Put (SvxBitmapListItem(pBitmapList,SID_BITMAP_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,nPos));
            rPage.PageCreated(aSet);


        break;

        case RID_SVXPAGE_SHADOW:
                aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE)); //add CHINA001
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



