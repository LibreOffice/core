/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dialogs.hrc>

#define _SVX_TABLINE_CXX
#include <cuires.hrc>
#include "tabline.hrc"
//#include "dlgname.hrc"

#include "cuitabarea.hxx"
#include "cuitabline.hxx"
#include "dlgname.hxx"
#include <dialmgr.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"

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
    sal_Bool bHasObj
) :

    SfxTabDialog    ( pParent, CUI_RES( RID_SVXDLG_LINE ), pAttr ),
    pDrawModel      ( pModel ),
    pObj            ( pSdrObj ),
    rOutAttrs       ( *pAttr ),
    maColorTab      ( pModel->GetColorTableFromSdrModel() ),
    maNewColorTab   ( pModel->GetColorTableFromSdrModel() ),
    maDashList      ( pModel->GetDashListFromSdrModel() ),
    maNewDashList   ( pModel->GetDashListFromSdrModel() ),
    maLineEndList   ( pModel->GetLineEndListFromSdrModel() ),
    maNewLineEndList( pModel->GetLineEndListFromSdrModel() ),
    bObjSelected    ( bHasObj ),
    nLineEndListState( CT_NONE ),
    nDashListState( CT_NONE ),
    mnColorTableState( CT_NONE ),
    nPageType( 0 ), // wird hier in erster Linie benutzt, um mit FillItemSet
                   // die richtigen Attribute zu erhalten ( noch Fragen? )
    nDlgType( 0 ),
    nPosDashLb( 0 ),
    nPosLineEndLb( 0 ),
    mnPos( 0 ),
    mbAreaTP( sal_False )
{
    FreeResource();

    bool bLineOnly = false;
    if( pObj && pObj->GetObjInventor() == SdrInventor )
    {
        switch( pObj->GetObjIdentifier() )
        {
        case OBJ_LINE:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_FREELINE:
        case OBJ_MEASURE:
        case OBJ_EDGE:
            bLineOnly = true;

        default:
            break;
        }

    }

    AddTabPage( RID_SVXPAGE_LINE, SvxLineTabPage::Create, 0);
    if( bLineOnly )
        AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create, 0 );
    else
        RemoveTabPage( RID_SVXPAGE_SHADOW );

    AddTabPage( RID_SVXPAGE_LINE_DEF, SvxLineDefTabPage::Create, 0);
    AddTabPage( RID_SVXPAGE_LINEEND_DEF, SvxLineEndDefTabPage::Create, 0);
//  AddTabPage( RID_SVXPAGE_COLOR, SvxColorTabPage::Create, 0 );

    SetCurPageId( RID_SVXPAGE_LINE );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxLineTabDialog, CancelHdlImpl ) );
//! rBtnCancel.SetText( CUI_RESSTR( RID_SVXSTR_CLOSE ) );
}

// -----------------------------------------------------------------------

SvxLineTabDialog::~SvxLineTabDialog()
{
}

// -----------------------------------------------------------------------

void SvxLineTabDialog::SavePalettes()
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    if( maNewColorTab != pDrawModel->GetColorTableFromSdrModel() )
    {
        pDrawModel->SetColorTableAtSdrModel( maNewColorTab );
        if ( pShell )
            pShell->PutItem( SvxColorTableItem( maNewColorTab, SID_COLOR_TABLE ) );
        maColorTab = pDrawModel->GetColorTableFromSdrModel();
    }
    if( maNewDashList != pDrawModel->GetDashListFromSdrModel() )
    {
        pDrawModel->SetDashListAtSdrModel( maNewDashList );
        if ( pShell )
            pShell->PutItem( SvxDashListItem( maNewDashList, SID_DASH_LIST ) );
        maDashList = pDrawModel->GetDashListFromSdrModel();
    }
    if( maNewLineEndList != pDrawModel->GetLineEndListFromSdrModel() )
    {
        pDrawModel->SetLineEndListAtSdrModel( maNewLineEndList );
        if ( pShell )
            pShell->PutItem( SvxLineEndListItem( maNewLineEndList, SID_LINEEND_LIST ) );
        maLineEndList = pDrawModel->GetLineEndListFromSdrModel();
    }

    // Speichern der Tabellen, wenn sie geaendert wurden.

    const String aPath( SvtPathOptions().GetPalettePath() );

    if( nDashListState & CT_MODIFIED )
    {
        maDashList->SetPath( aPath );
        maDashList->Save();

        // ToolBoxControls werden benachrichtigt:
        if ( pShell )
            pShell->PutItem( SvxDashListItem( maDashList, SID_DASH_LIST ) );
    }

    if( nLineEndListState & CT_MODIFIED )
    {
        maLineEndList->SetPath( aPath );
        maLineEndList->Save();

        // ToolBoxControls werden benachrichtigt:
        if ( pShell )
            pShell->PutItem( SvxLineEndListItem( maLineEndList, SID_LINEEND_LIST ) );
    }

    if( mnColorTableState & CT_MODIFIED )
    {
        maColorTab->SetPath( aPath );
        maColorTab->Save();

        // ToolBoxControls werden benachrichtigt:
        if ( pShell )
            pShell->PutItem( SvxColorTableItem( maColorTab, SID_COLOR_TABLE ) );
    }
}

// -----------------------------------------------------------------------

short SvxLineTabDialog::Ok()
{
    SavePalettes();

    // Es wird RET_OK zurueckgeliefert, wenn wenigstens eine
    // TabPage in FillItemSet() sal_True zurueckliefert. Dieses
    // geschieht z.Z. standardmaessig.
    return( SfxTabDialog::Ok() );
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxLineTabDialog, CancelHdlImpl, void *, EMPTYARG )
{
    SavePalettes();

    EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_INLINE_END( SvxLineTabDialog, CancelHdlImpl, void *, EMPTYARG )

// -----------------------------------------------------------------------

void SvxLineTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
            ( (SvxLineTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxLineTabPage&) rPage ).SetDashList( maDashList );
            ( (SvxLineTabPage&) rPage ).SetLineEndList( maLineEndList );
            ( (SvxLineTabPage&) rPage ).SetDlgType( nDlgType );//CHINA001 ( (SvxLineTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxLineTabPage&) rPage ).SetPageType( nPageType );//CHINA001 ( (SvxLineTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineTabPage&) rPage ).SetPosDashLb( &nPosDashLb );
            ( (SvxLineTabPage&) rPage ).SetPosLineEndLb( &nPosLineEndLb );
            ( (SvxLineTabPage&) rPage ).SetDashChgd( &nDashListState );
            ( (SvxLineTabPage&) rPage ).SetLineEndChgd( &nLineEndListState );
            ( (SvxLineTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineTabPage&) rPage ).Construct();
            ( (SvxLineTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            // ActivatePage() wird das erste mal nicht gerufen
            ( (SvxLineTabPage&) rPage ).ActivatePage( rOutAttrs );
        break;

        case RID_SVXPAGE_LINE_DEF:
            ( (SvxLineDefTabPage&) rPage ).SetDashList( maDashList );
            ( (SvxLineDefTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxLineDefTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineDefTabPage&) rPage ).SetPosDashLb( &nPosDashLb );
            ( (SvxLineDefTabPage&) rPage ).SetDashChgd( &nDashListState );
            ( (SvxLineDefTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineDefTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_LINEEND_DEF:
            ( (SvxLineEndDefTabPage&) rPage ).SetLineEndList( maLineEndList );
            ( (SvxLineEndDefTabPage&) rPage ).SetPolyObj( pObj );
            ( (SvxLineEndDefTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxLineEndDefTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineEndDefTabPage&) rPage ).SetPosLineEndLb( &nPosLineEndLb );
            ( (SvxLineEndDefTabPage&) rPage ).SetLineEndChgd( &nLineEndListState );
            ( (SvxLineEndDefTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineEndDefTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_SHADOW:
        {
            ( (SvxShadowTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxShadowTabPage&) rPage ).SetPageType( nPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( nDlgType );
            ( (SvxShadowTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxShadowTabPage&) rPage ).Construct();
        }
        break;
/*
        case RID_SVXPAGE_COLOR:
            ( (SvxColorTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxColorTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxColorTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxColorTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxColorTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxColorTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxColorTabPage&) rPage ).Construct();
        break;
*/
    }
}


