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

#include <tools/ref.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/dialogs.hrc>

#define _SVX_TABAREA_CXX

#include <cuires.hrc>
#include <svx/xtable.hxx>
#include "svx/globl3d.hxx"
#include <svx/svdmodel.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "tabarea.hrc"
#include "dlgname.hxx"
//#include "dlgname.hrc"
#include <dialmgr.hxx>

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
    bool bShadow
) :

    SfxTabDialog( pParent, CUI_RES( RID_SVXDLG_AREA ), pAttr ),

    mpDrawModel          ( pModel ),
    maColorTab           ( pModel->GetColorTableFromSdrModel() ),
    maNewColorTab        ( pModel->GetColorTableFromSdrModel() ),
    maGradientList       ( pModel->GetGradientListFromSdrModel() ),
    maNewGradientList    ( pModel->GetGradientListFromSdrModel() ),
    maHatchingList       ( pModel->GetHatchListFromSdrModel() ),
    maNewHatchingList    ( pModel->GetHatchListFromSdrModel() ),
    maBitmapList         ( pModel->GetBitmapListFromSdrModel() ),
    maNewBitmapList      ( pModel->GetBitmapListFromSdrModel() ),
    mrOutAttrs           ( *pAttr ),
    mnColorTableState ( CT_NONE ),
    mnBitmapListState ( CT_NONE ),
    mnGradientListState ( CT_NONE ),
    mnHatchingListState ( CT_NONE ),
    mnPageType( PT_AREA ),
    mnDlgType( 0 ),
    mnPos( 0 ),
    mbAreaTP( sal_False )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0 );

    if(bShadow)
    {
        AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create, 0 );
    }
    else
    {
        RemoveTabPage( RID_SVXPAGE_SHADOW );
    }

    AddTabPage( RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create,  0);
    AddTabPage( RID_SVXPAGE_COLOR, SvxColorTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_GRADIENT, SvxGradientTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_HATCH, SvxHatchTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BITMAP, SvxBitmapTabPage::Create,  0);

    SetCurPageId( RID_SVXPAGE_AREA );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxAreaTabDialog, CancelHdlImpl ) );
//! rBtnCancel.SetText( CUI_RESSTR( RID_SVXSTR_CLOSE ) );
}

// -----------------------------------------------------------------------

SvxAreaTabDialog::~SvxAreaTabDialog()
{
}


// -----------------------------------------------------------------------

void SvxAreaTabDialog::SavePalettes()
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    /*uno::Reference< frame::XDispatchProvider > xDispatchProvider;
    if ( !pShell )
    {
        uno::Reference< frame::XModel> xModel = mpDrawModel->getUnoModel();
        if ( xModel.is() )
            xDispatchProvider.set(xModel->getCurrentController(),uno::UNO_QUERY);
    }*/
    if( maNewColorTab != mpDrawModel->GetColorTableFromSdrModel() )
    {
        mpDrawModel->SetColorTableAtSdrModel( maNewColorTab );
        SvxColorTableItem aColorTableItem( maNewColorTab, SID_COLOR_TABLE );
        if ( pShell )
            pShell->PutItem( aColorTableItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aColorTableItem,SID_COLOR_TABLE);
        }
        maColorTab = mpDrawModel->GetColorTableFromSdrModel();
    }
    if( maNewGradientList != mpDrawModel->GetGradientListFromSdrModel() )
    {
        mpDrawModel->SetGradientListAtSdrModel( maNewGradientList );
        SvxGradientListItem aItem( maNewGradientList, SID_GRADIENT_LIST );
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem,SID_GRADIENT_LIST);
        }
        maGradientList = mpDrawModel->GetGradientListFromSdrModel();
    }
    if( maNewHatchingList != mpDrawModel->GetHatchListFromSdrModel() )
    {
        mpDrawModel->SetHatchListAtSdrModel( maNewHatchingList );
        SvxHatchListItem aItem( maNewHatchingList, SID_HATCH_LIST );
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem,SID_HATCH_LIST);
        }
        maHatchingList = mpDrawModel->GetHatchListFromSdrModel();
    }
    if( maNewBitmapList != mpDrawModel->GetBitmapListFromSdrModel() )
    {
        mpDrawModel->SetBitmapListAtSdrModel( maNewBitmapList );
        SvxBitmapListItem aItem( maNewBitmapList, SID_BITMAP_LIST );
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem,SID_BITMAP_LIST);
        }
        maBitmapList = mpDrawModel->GetBitmapListFromSdrModel();
    }

    // Speichern der Tabellen, wenn sie geaendert wurden.

    const String aPath( SvtPathOptions().GetPalettePath() );

    if( mnHatchingListState & CT_MODIFIED )
    {
        maHatchingList->SetPath( aPath );
        maHatchingList->Save();

        SvxHatchListItem aItem( maHatchingList, SID_HATCH_LIST );
        // ToolBoxControls werden benachrichtigt:
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem);
        }
    }

    if( mnBitmapListState & CT_MODIFIED )
    {
        maBitmapList->SetPath( aPath );
        maBitmapList->Save();

        SvxBitmapListItem aItem( maBitmapList, SID_BITMAP_LIST );
        // ToolBoxControls werden benachrichtigt:
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem);
        }
    }

    if( mnGradientListState & CT_MODIFIED )
    {
        maGradientList->SetPath( aPath );
        maGradientList->Save();

        SvxGradientListItem aItem( maGradientList, SID_GRADIENT_LIST );
        // ToolBoxControls werden benachrichtigt:
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem);
        }
    }

    if( mnColorTableState & CT_MODIFIED )
    {
        maColorTab->SetPath( aPath );
        maColorTab->Save();

        SvxColorTableItem aItem( maColorTab, SID_COLOR_TABLE );
        // ToolBoxControls werden benachrichtigt:
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem);
        }
    }
}
// -----------------------------------------------------------------------

short SvxAreaTabDialog::Ok()
{
    SavePalettes();

    // Es wird RET_OK zurueckgeliefert, wenn wenigstens eine
    // TabPage in FillItemSet() sal_True zurueckliefert. Dieses
    // geschieht z.Z. standardmaessig.
    return( SfxTabDialog::Ok() );
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxAreaTabDialog, CancelHdlImpl, void *, EMPTYARG)
{
    SavePalettes();

    EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_INLINE_END( SvxAreaTabDialog, CancelHdlImpl, void *, p )

// -----------------------------------------------------------------------

void SvxAreaTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_AREA:
            ( (SvxAreaTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxAreaTabPage&) rPage ).SetGradientList( maGradientList );
            ( (SvxAreaTabPage&) rPage ).SetHatchingList( maHatchingList );
            ( (SvxAreaTabPage&) rPage ).SetBitmapList( maBitmapList );
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
            ( (SvxShadowTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxShadowTabPage&) rPage ).SetPageType( mnPageType );//CHINA001 ( (SvxShadowTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( mnDlgType );//CHINA001 ( (SvxShadowTabPage&) rPage ).SetDlgType( &mnDlgType );
            //( (SvxShadowTabPage&) rPage ).SetPos( &nPos );
            ( (SvxShadowTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxShadowTabPage&) rPage ).Construct();
        }
        break;

        case RID_SVXPAGE_GRADIENT:
            ( (SvxGradientTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxGradientTabPage&) rPage ).SetGradientList( maGradientList );
            ( (SvxGradientTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxGradientTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxGradientTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxGradientTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxGradientTabPage&) rPage ).SetGrdChgd( &mnGradientListState );
            ( (SvxGradientTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxGradientTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_HATCH:
            ( (SvxHatchTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxHatchTabPage&) rPage ).SetHatchingList( maHatchingList );
            ( (SvxHatchTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxHatchTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxHatchTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxHatchTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxHatchTabPage&) rPage ).SetHtchChgd( &mnHatchingListState );
            ( (SvxHatchTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxHatchTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_BITMAP:
            ( (SvxBitmapTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxBitmapTabPage&) rPage ).SetBitmapList( maBitmapList );
            ( (SvxBitmapTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxBitmapTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxBitmapTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxBitmapTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxBitmapTabPage&) rPage ).SetBmpChgd( &mnBitmapListState );
            ( (SvxBitmapTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxBitmapTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_COLOR:
            ( (SvxColorTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxColorTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxColorTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxColorTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxColorTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxColorTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxColorTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_TRANSPARENCE:
            ( (SvxTransparenceTabPage&) rPage ).SetPageType( mnPageType );//CHINA001 ( (SvxTransparenceTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxTransparenceTabPage&) rPage ).SetDlgType( mnDlgType );//CHINA001 ( (SvxTransparenceTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxTransparenceTabPage&) rPage ).Construct();
        break;

    }
}


