/*************************************************************************
 *
 *  $RCSfile: dlgpage.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:43:46 $
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

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#include <svtools/intitem.hxx> //add CHINA001
//CHINA001 #ifndef _SVX_PAGE_HXX
//CHINA001 #include <svx/page.hxx>
//CHINA001 #endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SVX_TAB_AREA_HXX
#include <svx/tabarea.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX
#include <svx/drawitem.hxx>
#endif

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif
#include "dlgpage.hxx"

#include "DrawDocShell.hxx"

#ifndef _AEITEM_HXX //CHINA001
#include <svtools/aeitem.hxx> //CHINA001
#endif //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
#include <svx/svxenum.hxx> //CHINA001

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdPageDlg::SdPageDlg( SfxObjectShell* pDocSh, Window* pParent, const SfxItemSet* pAttr, BOOL bAreaPage ) :
        SfxTabDialog ( pParent, SdResId( TAB_PAGE ), pAttr ),
        rOutAttrs           ( *pAttr ),
        pDocShell           ( pDocSh )
{
    SvxColorTableItem aColorTableItem(*( (const SvxColorTableItem*)
        ( pDocShell->GetItem( SID_COLOR_TABLE ) ) ) );
    SvxGradientListItem aGradientListItem(*( (const SvxGradientListItem*)
        ( pDocShell->GetItem( SID_GRADIENT_LIST ) ) ) );
    SvxBitmapListItem aBitmapListItem(*( (const SvxBitmapListItem*)
        ( pDocShell->GetItem( SID_BITMAP_LIST ) ) ) );
    SvxHatchListItem aHatchListItem(*( (const SvxHatchListItem*)
        ( pDocShell->GetItem( SID_HATCH_LIST ) ) ) );

    pColorTab = aColorTableItem.GetColorTable();
    pGradientList = aGradientListItem.GetGradientList();
    pHatchingList = aHatchListItem.GetHatchList();
    pBitmapList = aBitmapListItem.GetBitmapList();

    FreeResource();

    AddTabPage( RID_SVXPAGE_PAGE); //CHINA001 AddTabPage( RID_SVXPAGE_PAGE, SvxPageDescPage::Create, 0);
    AddTabPage( RID_SVXPAGE_AREA); //CHINA001 AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0 );


    nDlgType = 1; // Vorlagen-Dialog
    nPageType = 0;
    nPos = 0;

    nColorTableState = CT_NONE;
    nBitmapListState = CT_NONE;
    nGradientListState = CT_NONE;
    nHatchingListState = CT_NONE;

    if(!bAreaPage)  // I have to add the page before I remove it !
        RemoveTabPage( RID_SVXPAGE_AREA );
}


/*************************************************************************
|*
|* Seite wird erzeugt
|*
\************************************************************************/

void SdPageDlg::PageCreated(USHORT nId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch(nId)
    {
    case RID_SVXPAGE_PAGE:
        //CHINA001 ( (SvxPageDescPage&) rPage).SetMode(SVX_PAGE_MODE_PRESENTATION);
        //CHINA001 ( (SvxPageDescPage&) rPage).SetPaperFormatRanges( SVX_PAPER_A0, SVX_PAPER_E );
        aSet.Put (SfxAllEnumItem((const USHORT)SID_ENUM_PAGE_MODE, SVX_PAGE_MODE_PRESENTATION)); //CHINA001
        aSet.Put (SfxAllEnumItem((const USHORT)SID_PAPER_START, SVX_PAPER_A0)); //CHINA001
        aSet.Put (SfxAllEnumItem((const USHORT)SID_PAPER_END, SVX_PAPER_E)); //CHINA001
        rPage.PageCreated(aSet); //CHINA001
        break;
    case RID_SVXPAGE_AREA:
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetColorTable( pColorTab );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetGradientList( pGradientList );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetHatchingList( pHatchingList );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetBitmapList( pBitmapList );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetPageType( &nPageType );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetDlgType( &nDlgType );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetPos( &nPos );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetGrdChgd( &nGradientListState );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetHtchChgd( &nHatchingListState );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetBmpChgd( &nBitmapListState );
//CHINA001      ( (SvxAreaTabPage&) rPage ).SetColorChgd( &nColorTableState );
//CHINA001      ( (SvxAreaTabPage&) rPage ).Construct();
            aSet.Put (SvxColorTableItem(pColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxGradientListItem(pGradientList,SID_GRADIENT_LIST));
            aSet.Put (SvxHatchListItem(pHatchingList,SID_HATCH_LIST));
            aSet.Put (SvxBitmapListItem(pBitmapList,SID_BITMAP_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,nPos));
            rPage.PageCreated(aSet);
        break;
    }
}



