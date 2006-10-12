/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabarea.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:28:33 $
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
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

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
#include "cuitabarea.hxx"
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
    const SdrView* /* pSdrView */
) :

    SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_AREA ), pAttr ),

    mpDrawModel          ( pModel ),
//    mpView               ( pSdrView ),
    mpColorTab           ( pModel->GetColorTable() ),
    mpNewColorTab        ( pModel->GetColorTable() ),
    mpGradientList       ( pModel->GetGradientList() ),
    mpNewGradientList    ( pModel->GetGradientList() ),
    mpHatchingList       ( pModel->GetHatchList() ),
    mpNewHatchingList    ( pModel->GetHatchList() ),
    mpBitmapList         ( pModel->GetBitmapList() ),
    mpNewBitmapList      ( pModel->GetBitmapList() ),
    mrOutAttrs           ( *pAttr ),
    mnColorTableState ( CT_NONE ),
    mnBitmapListState ( CT_NONE ),
    mnGradientListState ( CT_NONE ),
    mnHatchingListState ( CT_NONE ),
    mnPageType( PT_AREA ),
    mnDlgType( 0 ),
    mnPos( 0 ),
    mbAreaTP( sal_False ),
    mbDeleteColorTable( TRUE )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create,  0);
    AddTabPage( RID_SVXPAGE_COLOR, SvxColorTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_GRADIENT, SvxGradientTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_HATCH, SvxHatchTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BITMAP, SvxBitmapTabPage::Create,  0);

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
    if( mpNewColorTab != mpDrawModel->GetColorTable() )
    {
        if(mbDeleteColorTable)
            delete mpDrawModel->GetColorTable();
        mpDrawModel->SetColorTable( mpNewColorTab );
        SfxObjectShell::Current()->PutItem( SvxColorTableItem( mpNewColorTab ) );
        mpColorTab = mpDrawModel->GetColorTable();
    }
    if( mpNewGradientList != mpDrawModel->GetGradientList() )
    {
        delete mpDrawModel->GetGradientList();
        mpDrawModel->SetGradientList( mpNewGradientList );
        SfxObjectShell::Current()->PutItem( SvxGradientListItem( mpNewGradientList ) );
        mpGradientList = mpDrawModel->GetGradientList();
    }
    if( mpNewHatchingList != mpDrawModel->GetHatchList() )
    {
        delete mpDrawModel->GetHatchList();
        mpDrawModel->SetHatchList( mpNewHatchingList );
        SfxObjectShell::Current()->PutItem( SvxHatchListItem( mpNewHatchingList ) );
        mpHatchingList = mpDrawModel->GetHatchList();
    }
    if( mpNewBitmapList != mpDrawModel->GetBitmapList() )
    {
        delete mpDrawModel->GetBitmapList();
        mpDrawModel->SetBitmapList( mpNewBitmapList );
        SfxObjectShell::Current()->PutItem( SvxBitmapListItem( mpNewBitmapList ) );
        mpBitmapList = mpDrawModel->GetBitmapList();
    }

    // Speichern der Tabellen, wenn sie geaendert wurden.

    const String aPath( SvtPathOptions().GetPalettePath() );

    if( mnHatchingListState & CT_MODIFIED )
    {
        mpHatchingList->SetPath( aPath );
        mpHatchingList->Save();

        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxHatchListItem( mpHatchingList ) );
    }

    if( mnBitmapListState & CT_MODIFIED )
    {
        mpBitmapList->SetPath( aPath );
        mpBitmapList->Save();

        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxBitmapListItem( mpBitmapList ) );
    }

    if( mnGradientListState & CT_MODIFIED )
    {
        mpGradientList->SetPath( aPath );
        mpGradientList->Save();
        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxGradientListItem( mpGradientList ) );
    }

    if( mnColorTableState & CT_MODIFIED )
    {
        mpColorTab->SetPath( aPath );
        mpColorTab->Save();

        // ToolBoxControls werden benachrichtigt:
        SfxObjectShell::Current()->PutItem( SvxColorTableItem( mpColorTab ) );
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

IMPL_LINK_INLINE_START( SvxAreaTabDialog, CancelHdl, void *, EMPTYARG)
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
            ( (SvxAreaTabPage&) rPage ).SetColorTable( mpColorTab );
            ( (SvxAreaTabPage&) rPage ).SetGradientList( mpGradientList );
            ( (SvxAreaTabPage&) rPage ).SetHatchingList( mpHatchingList );
            ( (SvxAreaTabPage&) rPage ).SetBitmapList( mpBitmapList );
            //CHINA001 ( (SvxAreaTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxAreaTabPage&) rPage ).SetPageType( mnPageType ); //add CHINA001
            //CHINA001 ( (SvxAreaTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxAreaTabPage&) rPage ).SetDlgType( mnDlgType );//add CHINA001
            //CHINA001 ( (SvxAreaTabPage&) rPage ).SetPos( &nPos );
            ( (SvxAreaTabPage&) rPage ).SetPos( mnPos );//add CHINA001
            ( (SvxAreaTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxAreaTabPage&) rPage ).SetGrdChgd( &mnGradientListState );
            ( (SvxAreaTabPage&) rPage ).SetHtchChgd( &mnHatchingListState );
            ( (SvxAreaTabPage&) rPage ).SetBmpChgd( &mnBitmapListState );
            ( (SvxAreaTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxAreaTabPage&) rPage ).Construct();
            // ActivatePage() wird das erste mal nicht gerufen
            ( (SvxAreaTabPage&) rPage ).ActivatePage( mrOutAttrs );

        break;

        case RID_SVXPAGE_SHADOW:
        {
            ( (SvxShadowTabPage&) rPage ).SetColorTable( mpColorTab );
            ( (SvxShadowTabPage&) rPage ).SetPageType( mnPageType );//CHINA001 ( (SvxShadowTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( mnDlgType );//CHINA001 ( (SvxShadowTabPage&) rPage ).SetDlgType( &mnDlgType );
            //( (SvxShadowTabPage&) rPage ).SetPos( &nPos );
            ( (SvxShadowTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxShadowTabPage&) rPage ).Construct();
        }
        break;

        case RID_SVXPAGE_GRADIENT:
            ( (SvxGradientTabPage&) rPage ).SetColorTable( mpColorTab );
            ( (SvxGradientTabPage&) rPage ).SetGradientList( mpGradientList );
            ( (SvxGradientTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxGradientTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxGradientTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxGradientTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxGradientTabPage&) rPage ).SetGrdChgd( &mnGradientListState );
            ( (SvxGradientTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxGradientTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_HATCH:
            ( (SvxHatchTabPage&) rPage ).SetColorTable( mpColorTab );
            ( (SvxHatchTabPage&) rPage ).SetHatchingList( mpHatchingList );
            ( (SvxHatchTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxHatchTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxHatchTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxHatchTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxHatchTabPage&) rPage ).SetHtchChgd( &mnHatchingListState );
            ( (SvxHatchTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxHatchTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_BITMAP:
            ( (SvxBitmapTabPage&) rPage ).SetColorTable( mpColorTab );
            ( (SvxBitmapTabPage&) rPage ).SetBitmapList( mpBitmapList );
            ( (SvxBitmapTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxBitmapTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxBitmapTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxBitmapTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxBitmapTabPage&) rPage ).SetBmpChgd( &mnBitmapListState );
            ( (SvxBitmapTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxBitmapTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_COLOR:
            ( (SvxColorTabPage&) rPage ).SetColorTable( mpColorTab );
            ( (SvxColorTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxColorTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxColorTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxColorTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxColorTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxColorTabPage&) rPage ).SetDeleteColorTable( mbDeleteColorTable );
            ( (SvxColorTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_TRANSPARENCE:
            ( (SvxTransparenceTabPage&) rPage ).SetPageType( mnPageType );//CHINA001 ( (SvxTransparenceTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxTransparenceTabPage&) rPage ).SetDlgType( mnDlgType );//CHINA001 ( (SvxTransparenceTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxTransparenceTabPage&) rPage ).Construct();
        break;

    }
}


