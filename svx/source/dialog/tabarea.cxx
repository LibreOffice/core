/*************************************************************************
 *
 *  $RCSfile: tabarea.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-09-26 06:36:01 $
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

// include ---------------------------------------------------------------

#pragma hdrstop
#include <tools/ref.hxx>
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVDMARK_HXX //autogen
#include <svdmark.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svdobj.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svdview.hxx>
#endif

#define _SVX_TABAREA_CXX

#include "dialogs.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST

#include "xtable.hxx"
#include "globl3d.hxx"
#include "svdmodel.hxx"
#include "drawitem.hxx"
#include "tabarea.hxx"
#include "tabarea.hrc"
#include "dlgname.hxx"
#include "dlgname.hrc"
#include "dialmgr.hxx"

#define DLGWIN this->GetParent()->GetParent()

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
|************************************************************************/

SvxAreaTabDialog::SvxAreaTabDialog
(
    Window* pParent,
    const SfxItemSet* pAttr,
    SdrModel* pModel,
    const SdrView* pSdrView
) :

    SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_AREA ), pAttr ),

    rOutAttrs           ( *pAttr ),
    pDrawModel          ( pModel ),
    pView               ( pSdrView ),
    pColorTab           ( pModel->GetColorTable() ),
    pGradientList       ( pModel->GetGradientList() ),
    pHatchingList       ( pModel->GetHatchList() ),
    pBitmapList         ( pModel->GetBitmapList() ),
    pNewColorTab        ( pModel->GetColorTable() ),
    pNewGradientList    ( pModel->GetGradientList() ),
    pNewHatchingList    ( pModel->GetHatchList() ),
    pNewBitmapList      ( pModel->GetBitmapList() ),
    bDeleteColorTable   ( TRUE )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create,  0);
    AddTabPage( RID_SVXPAGE_COLOR, SvxColorTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_GRADIENT, SvxGradientTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_HATCH, SvxHatchTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BITMAP, SvxBitmapTabPage::Create,  0);

    nColorTableState = CT_NONE;
    nBitmapListState = CT_NONE;
    nGradientListState = CT_NONE;
    nHatchingListState = CT_NONE;

    nDlgType = 0;
    nPageType = PT_AREA;
    nPos = 0;

    SetCurPageId( RID_SVXPAGE_AREA );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxAreaTabDialog, CancelHdl ) );
//! rBtnCancel.SetText( SVX_RESSTR( RID_SVXSTR_CLOSE ) );
}

// -----------------------------------------------------------------------

SvxAreaTabDialog::~SvxAreaTabDialog()
{
}


// -----------------------------------------------------------------------

void SvxAreaTabDialog::SavePalettes()
{
    if( pNewColorTab != pDrawModel->GetColorTable() )
    {
        if(bDeleteColorTable)
            delete pDrawModel->GetColorTable();
        pDrawModel->SetColorTable( pNewColorTab );
        SfxObjectShell::Current()->PutItem( SvxColorTableItem( pNewColorTab ) );
        pColorTab = pDrawModel->GetColorTable();
    }
    if( pNewGradientList != pDrawModel->GetGradientList() )
    {
        delete pDrawModel->GetGradientList();
        pDrawModel->SetGradientList( pNewGradientList );
        SfxObjectShell::Current()->PutItem( SvxGradientListItem( pNewGradientList ) );
        pGradientList = pDrawModel->GetGradientList();
    }
    if( pNewHatchingList != pDrawModel->GetHatchList() )
    {
        delete pDrawModel->GetHatchList();
        pDrawModel->SetHatchList( pNewHatchingList );
        SfxObjectShell::Current()->PutItem( SvxHatchListItem( pNewHatchingList ) );
        pHatchingList = pDrawModel->GetHatchList();
    }
    if( pNewBitmapList != pDrawModel->GetBitmapList() )
    {
        delete pDrawModel->GetBitmapList();
        pDrawModel->SetBitmapList( pNewBitmapList );
        SfxObjectShell::Current()->PutItem( SvxBitmapListItem( pNewBitmapList ) );
        pBitmapList = pDrawModel->GetBitmapList();
    }

    // Speichern der Tabellen, wenn sie geaendert wurden.

    const String aPath( SvtPathOptions().GetPalettePath() );

    if( nHatchingListState & CT_MODIFIED )
    {
        pHatchingList->SetPath( aPath );
        pHatchingList->Save();

        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxHatchListItem( pHatchingList ) );
    }

    if( nBitmapListState & CT_MODIFIED )
    {
        pBitmapList->SetPath( aPath );
        pBitmapList->Save();

        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxBitmapListItem( pBitmapList ) );
    }

    if( nGradientListState & CT_MODIFIED )
    {
        pGradientList->SetPath( aPath );
        pGradientList->Save();
        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxGradientListItem( pGradientList ) );
    }

    if( nColorTableState & CT_MODIFIED )
    {
        pColorTab->SetPath( aPath );
        pColorTab->Save();

        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxColorTableItem( pColorTab ) );
    }
}
// -----------------------------------------------------------------------

short SvxAreaTabDialog::Ok()
{
    SavePalettes();

    // Es wird RET_OK zurueckgeliefert, wenn wenigstens eine
    // TabPage in FillItemSet() TRUE zurueckliefert. Dieses
    // geschieht z.Z. standardmaessig.
    return( SfxTabDialog::Ok() );
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxAreaTabDialog, CancelHdl, void *, p )
{
    SavePalettes();

    EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_INLINE_END( SvxAreaTabDialog, CancelHdl, void *, p )

// -----------------------------------------------------------------------

void SvxAreaTabDialog::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_AREA:
            ( (SvxAreaTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxAreaTabPage&) rPage ).SetGradientList( pGradientList );
            ( (SvxAreaTabPage&) rPage ).SetHatchingList( pHatchingList );
            ( (SvxAreaTabPage&) rPage ).SetBitmapList( pBitmapList );
            ( (SvxAreaTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxAreaTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxAreaTabPage&) rPage ).SetPos( &nPos );
            ( (SvxAreaTabPage&) rPage ).SetAreaTP( &bAreaTP );
            ( (SvxAreaTabPage&) rPage ).SetGrdChgd( &nGradientListState );
            ( (SvxAreaTabPage&) rPage ).SetHtchChgd( &nHatchingListState );
            ( (SvxAreaTabPage&) rPage ).SetBmpChgd( &nBitmapListState );
            ( (SvxAreaTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxAreaTabPage&) rPage ).Construct();
            // ActivatePage() wird das erste mal nicht gerufen
            ( (SvxAreaTabPage&) rPage ).ActivatePage( rOutAttrs );

        break;

        case RID_SVXPAGE_SHADOW:
        {
            ( (SvxShadowTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxShadowTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( &nDlgType );
            //( (SvxShadowTabPage&) rPage ).SetPos( &nPos );
            ( (SvxShadowTabPage&) rPage ).SetAreaTP( &bAreaTP );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxShadowTabPage&) rPage ).Construct();
        }
        break;

        case RID_SVXPAGE_GRADIENT:
            ( (SvxGradientTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxGradientTabPage&) rPage ).SetGradientList( pGradientList );
            ( (SvxGradientTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxGradientTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxGradientTabPage&) rPage ).SetPos( &nPos );
            ( (SvxGradientTabPage&) rPage ).SetAreaTP( &bAreaTP );
            ( (SvxGradientTabPage&) rPage ).SetGrdChgd( &nGradientListState );
            ( (SvxGradientTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxGradientTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_HATCH:
            ( (SvxHatchTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxHatchTabPage&) rPage ).SetHatchingList( pHatchingList );
            ( (SvxHatchTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxHatchTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxHatchTabPage&) rPage ).SetPos( &nPos );
            ( (SvxHatchTabPage&) rPage ).SetAreaTP( &bAreaTP );
            ( (SvxHatchTabPage&) rPage ).SetHtchChgd( &nHatchingListState );
            ( (SvxHatchTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxHatchTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_BITMAP:
            ( (SvxBitmapTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxBitmapTabPage&) rPage ).SetBitmapList( pBitmapList );
            ( (SvxBitmapTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxBitmapTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxBitmapTabPage&) rPage ).SetPos( &nPos );
            ( (SvxBitmapTabPage&) rPage ).SetAreaTP( &bAreaTP );
            ( (SvxBitmapTabPage&) rPage ).SetBmpChgd( &nBitmapListState );
            ( (SvxBitmapTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxBitmapTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_COLOR:
            ( (SvxColorTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxColorTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxColorTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxColorTabPage&) rPage ).SetPos( &nPos );
            ( (SvxColorTabPage&) rPage ).SetAreaTP( &bAreaTP );
            ( (SvxColorTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxColorTabPage&) rPage ).SetDeleteColorTable( bDeleteColorTable );
            ( (SvxColorTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_TRANSPARENCE:
            ( (SvxTransparenceTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxTransparenceTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxTransparenceTabPage&) rPage ).Construct();
        break;

    }
}


