/*************************************************************************
 *
 *  $RCSfile: tabline.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:12 $
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINIMGR_HXX
#include <svtools/iniman.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#pragma hdrstop

#define _SVX_TABLINE_CXX
#include "dialogs.hrc"
#include "tabline.hrc"
#include "dlgname.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST

#include "tabline.hxx"
#include "dlgname.hxx"
#include "dialmgr.hxx"
#include "svdmodel.hxx"
#include "xtable.hxx"
#include "drawitem.hxx"

#define DLGWIN this->GetParent()->GetParent()

#define BITMAP_WIDTH   32
#define BITMAP_HEIGHT  12
#define XOUT_WIDTH    150

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SvxLineTabDialog::SvxLineTabDialog
(
    Window* pParent,
    const SfxItemSet* pAttr,
    SdrModel* pModel,
    const SdrObject* pSdrObj,
    BOOL bHasObj
) :

    SfxTabDialog    ( pParent, SVX_RES( RID_SVXDLG_LINE ), pAttr ),
    pDrawModel      ( pModel ),
    pObj            ( pSdrObj ),
    bObjSelected    ( bHasObj ),
    pColorTab       ( pModel->GetColorTable() ),
    pDashList       ( pModel->GetDashList() ),
    pLineEndList    ( pModel->GetLineEndList() ),
    pNewDashList    ( pModel->GetDashList() ),
    pNewLineEndList ( pModel->GetLineEndList() ),
    rOutAttrs       ( *pAttr )

{
    FreeResource();

    AddTabPage( RID_SVXPAGE_LINE, SvxLineTabPage::Create, 0);
    AddTabPage( RID_SVXPAGE_LINE_DEF, SvxLineDefTabPage::Create, 0);
    AddTabPage( RID_SVXPAGE_LINEEND_DEF, SvxLineEndDefTabPage::Create, 0);

    nLineEndListState = CT_NONE;
    nDashListState = CT_NONE;

    nDlgType = 0;
    nPageType = 0; // wird hier in erster Linie benutzt, um mit FillItemSet
                   // die richtigen Attribute zu erhalten ( noch Fragen? )
    nPosDashLb = 0;
    nPosLineEndLb = 0;

    SetCurPageId( RID_SVXPAGE_LINE );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxLineTabDialog, CancelHdl ) );
//! rBtnCancel.SetText( SVX_RESSTR( RID_SVXSTR_CLOSE ) );
}

// -----------------------------------------------------------------------

SvxLineTabDialog::~SvxLineTabDialog()
{
}

// -----------------------------------------------------------------------

void SvxLineTabDialog::SavePalettes()
{
    if( pNewDashList != pDrawModel->GetDashList() )
    {
        delete pDrawModel->GetDashList();
        pDrawModel->SetDashList( pNewDashList );
        SfxObjectShell::Current()->PutItem( SvxDashListItem( pNewDashList ) );
        pDashList = pDrawModel->GetDashList();
    }
    if( pNewLineEndList != pDrawModel->GetLineEndList() )
    {
        delete pDrawModel->GetLineEndList();
        pDrawModel->SetLineEndList( pNewLineEndList );
        SfxObjectShell::Current()->PutItem( SvxLineEndListItem( pNewLineEndList ) );
        pLineEndList = pDrawModel->GetLineEndList();
    }

    // Speichern der Tabellen, wenn sie geaendert wurden.

    const String aPath( SFX_APP()->GetAppIniManager()->Get( SFX_KEY_PALETTE_PATH ) );

    if( nDashListState & CT_MODIFIED )
    {
        pDashList->SetPath( aPath );
        pDashList->Save();

        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxDashListItem( pDashList ) );
    }

    if( nLineEndListState & CT_MODIFIED )
    {
        pLineEndList->SetPath( aPath );
        pLineEndList->Save();

        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxLineEndListItem( pLineEndList ) );
    }
}

// -----------------------------------------------------------------------

short SvxLineTabDialog::Ok()
{
    SavePalettes();

    // Es wird RET_OK zurueckgeliefert, wenn wenigstens eine
    // TabPage in FillItemSet() TRUE zurueckliefert. Dieses
    // geschieht z.Z. standardmaessig.
    return( SfxTabDialog::Ok() );
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxLineTabDialog, CancelHdl, void *, p )
{
    SavePalettes();

    EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_INLINE_END( SvxLineTabDialog, CancelHdl, void *, p )

// -----------------------------------------------------------------------

void SvxLineTabDialog::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
            ( (SvxLineTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxLineTabPage&) rPage ).SetDashList( pDashList );
            ( (SvxLineTabPage&) rPage ).SetLineEndList( pLineEndList );
            ( (SvxLineTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxLineTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineTabPage&) rPage ).SetPosDashLb( &nPosDashLb );
            ( (SvxLineTabPage&) rPage ).SetPosLineEndLb( &nPosLineEndLb );
            ( (SvxLineTabPage&) rPage ).SetDashChgd( &nDashListState );
            ( (SvxLineTabPage&) rPage ).SetLineEndChgd( &nLineEndListState );
            ( (SvxLineTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineTabPage&) rPage ).Construct();
            // ActivatePage() wird das erste mal nicht gerufen
            ( (SvxLineTabPage&) rPage ).ActivatePage( rOutAttrs );
        break;

        case RID_SVXPAGE_LINE_DEF:
            ( (SvxLineDefTabPage&) rPage ).SetDashList( pDashList );
            ( (SvxLineDefTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxLineDefTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineDefTabPage&) rPage ).SetPosDashLb( &nPosDashLb );
            ( (SvxLineDefTabPage&) rPage ).SetDashChgd( &nDashListState );
            ( (SvxLineDefTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineDefTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_LINEEND_DEF:
            ( (SvxLineEndDefTabPage&) rPage ).SetLineEndList( pLineEndList );
            ( (SvxLineEndDefTabPage&) rPage ).SetPolyObj( pObj );
            ( (SvxLineEndDefTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxLineEndDefTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineEndDefTabPage&) rPage ).SetPosLineEndLb( &nPosLineEndLb );
            ( (SvxLineEndDefTabPage&) rPage ).SetLineEndChgd( &nLineEndListState );
            ( (SvxLineEndDefTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineEndDefTabPage&) rPage ).Construct();
        break;
    }
}


