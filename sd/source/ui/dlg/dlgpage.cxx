/*************************************************************************
 *
 *  $RCSfile: dlgpage.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:32 $
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

#ifndef _SVX_PAGE_HXX
#include <svx/page.hxx>
#endif
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

#include "docshell.hxx"


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

    AddTabPage( RID_SVXPAGE_PAGE, SvxPageDescPage::Create, 0);
    AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0 );

    nDlgType = 1; // Vorlagen-Dialog
    nPageType = 0;
    nPos = 0;

    nColorTableState = CT_NONE;
    nBitmapListState = CT_NONE;
    nGradientListState = CT_NONE;
    nHatchingListState = CT_NONE;

    if( bAreaPage)  // I have to add the page before I remove it !
        RemoveTabPage( RID_SVXPAGE_AREA );
}


/*************************************************************************
|*
|* Seite wird erzeugt
|*
\************************************************************************/

void SdPageDlg::PageCreated(USHORT nId, SfxTabPage& rPage)
{
    switch(nId)
    {
    case RID_SVXPAGE_PAGE:
        ( (SvxPageDescPage&) rPage).SetMode(SVX_PAGE_MODE_PRESENTATION);
        ( (SvxPageDescPage&) rPage).SetPaperFormatRanges( SVX_PAPER_A0, SVX_PAPER_E );
        break;
    case RID_SVXPAGE_AREA:
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
        break;
    }
}



