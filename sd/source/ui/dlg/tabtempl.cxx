/*************************************************************************
 *
 *  $RCSfile: tabtempl.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:04:16 $
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

#include <svx/flstitem.hxx>

#pragma hdrstop
#include <svx/svxids.hrc> //add CHINA001

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE //CHINA001
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST //CHINA001
#define ITEMID_HATCH_LIST       SID_HATCH_LIST //CHINA001
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST //CHINA001
#define ITEMID_DASH_LIST            SID_DASH_LIST   //CHINA001
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST    //CHINA001
#include <svx/drawitem.hxx> //add CHINA001
#include <svtools/intitem.hxx> //add CHINA001
#include <svx/ofaitem.hxx> //add CHINA001
#include <svx/svxgrahicitem.hxx> //CHINA001
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

//CHINA001 #include <svx/chardlg.hxx>
//CHINA001 #include <svx/paragrph.hxx>
//CHINA001 #include <svx/tabstpge.hxx>
#include <svx/svxdlg.hxx>

//CHINA001 #ifndef _SVX_CONNECT_HXX //autogen
//CHINA001 #include <svx/connect.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_MEASURE_HXX //autogen
//CHINA001 #include <svx/measure.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_TEXTATTR_HXX //autogen
//CHINA001 #include <svx/textattr.hxx>
//CHINA001 #endif
#ifndef _SVX_TAB_LINE_HXX //autogen
#include <svx/tabline.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
//CHINA001 #ifndef _SVX_TABSTPGE_HXX //autogen
//CHINA001 #include <svx/tabstpge.hxx>
//CHINA001 #endif
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif

#include "DrawDocShell.hxx"
#include "tabtempl.hxx"
#include "tabtempl.hrc"
#include "sdresid.hxx"
#include "dlg_char.hxx"
#include "paragr.hxx"
#include <svx/flagsdef.hxx> //CHINA001
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
        pColorTab           ( pModel->GetColorTable() ),
        pDashList           ( pModel->GetDashList() ),
        pLineEndList        ( pModel->GetLineEndList() ),
        pGradientList       ( pModel->GetGradientList() ),
        pHatchingList       ( pModel->GetHatchList() ),
        pBitmapList         ( pModel->GetBitmapList() ),
        rDocShell           ( *pDocShell ),
        pSdrView            ( pView )
{
    FreeResource();

    // Listbox fuellen und Select-Handler ueberladen

    AddTabPage( RID_SVXPAGE_LINE);//CHINA001 AddTabPage( RID_SVXPAGE_LINE, SvxLineTabPage::Create,
                                    //CHINA001 SvxLineTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_AREA);//CHINA001 AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create,
                                    //CHINA001 SvxAreaTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_SHADOW);//CHINA001 AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create,
                                    //CHINA001 SvxShadowTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_TRANSPARENCE);//CHINA001 AddTabPage( RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create,
                                    //CHINA001 SvxTransparenceTabPage::GetRanges )
    AddTabPage( RID_SVXPAGE_CHAR_NAME ); //CHINA001 AddTabPage( RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create,
                                    //CHINA001 SvxCharNamePage::GetRanges );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS ); //CHINA001 AddTabPage( RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create,
                                    //CHINA001 SvxCharEffectsPage::GetRanges );
    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );//CHINA001 AddTabPage( RID_SVXPAGE_STD_PARAGRAPH,
                    //CHINA001 SvxStdParagraphTabPage::Create,
                    //CHINA001 SvxStdParagraphTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_TEXTATTR );//CHINA001 AddTabPage( RID_SVXPAGE_TEXTATTR, SvxTextAttrPage::Create,
                    //CHINA001 SvxTextAttrPage::GetRanges );
    AddTabPage( RID_SVXPAGE_TEXTANIMATION );
    AddTabPage( RID_SVXPAGE_MEASURE);//CHINA001 AddTabPage( RID_SVXPAGE_MEASURE, SvxMeasurePage::Create,
                    //CHINA001 {SvxMeasurePage::GetRanges );
    AddTabPage( RID_SVXPAGE_CONNECTION);//CHINA001 AddTabPage( RID_SVXPAGE_CONNECTION, SvxConnectionPage::Create,
                    //CHINA001 SvxConnectionPage::GetRanges );
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );//CHINA001 AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH, SvxParaAlignTabPage::Create,
                    //CHINA001 SvxParaAlignTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_TABULATOR );//CHINA001 AddTabPage( RID_SVXPAGE_TABULATOR, SvxTabulatorTabPage::Create,
                    //CHINA001 SvxTabulatorTabPage::GetRanges );
    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( RID_SVXPAGE_PARA_ASIAN, SvxAsianTabPage::Create,0);
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
    SfxAllItemSet aSet(*(GetRefreshedSet()->GetPool()));
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
//CHINA001          ( (SvxLineTabPage&) rPage ).SetColorTable( pColorTab );
//CHINA001          ( (SvxLineTabPage&) rPage ).SetDashList( pDashList );
//CHINA001          ( (SvxLineTabPage&) rPage ).SetLineEndList( pLineEndList );
//CHINA001          //( (SvxLineTabPage&) rPage ).SetPageType( &nPageType );
//CHINA001          ( (SvxLineTabPage&) rPage ).SetDlgType( &nDlgType );
//CHINA001          //( (SvxLineTabPage&) rPage ).SetPos( &nPos );
//CHINA001          ( (SvxLineTabPage&) rPage ).Construct();
//CHINA001          // ( (SvxLineTabPage&) rPage ).ActivatePage();
            aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
            aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_AREA:
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetColorTable( pColorTab );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetGradientList( pGradientList );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetHatchingList( pHatchingList );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetBitmapList( pBitmapList );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetPageType( &nPageType );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetDlgType( &nDlgType );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetPos( &nPos );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetGrdChgd( &nGradientListState );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetHtchChgd( &nHatchingListState );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetBmpChgd( &nBitmapListState );
//CHINA001          ( (SvxAreaTabPage&) rPage ).SetColorChgd( &nColorTableState );
//CHINA001          ( (SvxAreaTabPage&) rPage ).Construct();
//CHINA001          // ( (SvxAreaTabPage&) rPage ).ActivatePage();

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
//CHINA001          ( (SvxShadowTabPage&) rPage ).SetColorTable( pColorTab );
//CHINA001          ( (SvxShadowTabPage&) rPage ).SetPageType( &nPageType );
//CHINA001          ( (SvxShadowTabPage&) rPage ).SetDlgType( &nDlgType );
//CHINA001          ( (SvxShadowTabPage&) rPage ).SetColorChgd( &nColorTableState );
//CHINA001          ( (SvxShadowTabPage&) rPage ).Construct();
                aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE)); //add CHINA001
                aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
                aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
                rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_TRANSPARENCE:
//CHINA001          ( (SvxTransparenceTabPage&) rPage ).SetPageType( &nPageType );
//CHINA001          ( (SvxTransparenceTabPage&) rPage ).SetDlgType( &nDlgType );
//CHINA001          ( (SvxTransparenceTabPage&) rPage ).Construct();
                    aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
                    aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
                    rPage.PageCreated(aSet);
        break;

        case RID_SVXPAGE_CHAR_NAME:
        {
            SvxFontListItem aItem(*( (const SvxFontListItem*)
                ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

            //CHINA001 ( (SvxCharNamePage&) rPage ).SetFontList( aItem );
            aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
            rPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            //CHINA001 ( (SvxCharEffectsPage&) rPage ).DisableControls( DISABLE_CASEMAP );
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP)); //CHINA001
            rPage.PageCreated(aSet);
        break;

        case RID_SVXPAGE_STD_PARAGRAPH:
        break;

        case RID_SVXPAGE_TEXTATTR:
        {

            //CHINA001 ( (SvxTextAttrPage&) rPage ).SetView( pSdrView );
            //CHINA001 ( (SvxTextAttrPage&) rPage ).Construct();
            aSet.Put(OfaPtrItem(SID_SVXTEXTATTRPAGE_VIEW,pSdrView)); //add CHINA001
            rPage.PageCreated(aSet); //add CHINA001
        }
        break;

        case RID_SVXPAGE_TEXTANIMATION:
        break;

        case RID_SVXPAGE_MEASURE:
//CHINA001          ( (SvxMeasurePage&) rPage ).SetView( pSdrView );
//CHINA001          ( (SvxMeasurePage&) rPage ).Construct();
            aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));//add CHINA001
            rPage.PageCreated(aSet); //add CHINA001
        break;

        case RID_SVXPAGE_CONNECTION:
        {
//CHINA001          ( (SvxConnectionPage&) rPage ).SetView( pSdrView );
//CHINA001          ( (SvxConnectionPage&) rPage ).Construct();
            aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));//add CHINA001
            rPage.PageCreated(aSet); //add CHINA001
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



