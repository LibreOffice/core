/*************************************************************************
 *
 *  $RCSfile: tplnedef.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:13 $
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
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINIMGR_HXX
#include <svtools/iniman.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFXFILEDLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#pragma hdrstop

#define _SVX_TPLNEDEF_CXX

#include "dialogs.hrc"
#include "tabline.hrc"
#include "dlgname.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_DASH_LIST        SID_DASH_LIST

#include "xattr.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"
#include "xtable.hxx"

#include "drawitem.hxx"
#include "tabline.hxx"
#include "dlgname.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#define DLGWIN this->GetParent()->GetParent()

#define BITMAP_WIDTH   32
#define BITMAP_HEIGHT  12
#define XOUT_WIDTH    150

/*************************************************************************
|*
|*  Dialog zum Definieren von Linienstilen
|*
\************************************************************************/

SvxLineDefTabPage::SvxLineDefTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_LINE_DEF ), rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlPreview ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() ),

    aXLStyle            ( XLINE_DASH ),
    aXWidth             ( XOUT_WIDTH ),
    aXColor             ( String(), COL_BLACK ),
    aXDash              ( String(), XDash( XDASH_RECT, 3, 7, 2, 40, 15 ) ),

    aFtType         ( this, ResId( FT_TYPE ) ),
    aLbType1        ( this, ResId( LB_TYPE_1 ) ),
    aLbType2        ( this, ResId( LB_TYPE_2 ) ),
    aFtNumber       ( this, ResId( FT_NUMBER ) ),
    aNumFldNumber1  ( this, ResId( NUM_FLD_1 ) ),
    aNumFldNumber2  ( this, ResId( NUM_FLD_2 ) ),
    aFtDistance     ( this, ResId( FT_DISTANCE ) ),
    aMtrDistance    ( this, ResId( MTR_FLD_DISTANCE ) ),
    aFtLength       ( this, ResId( FT_LENGTH ) ),
    aMtrLength1     ( this, ResId( MTR_FLD_LENGTH_1 ) ),
    aMtrLength2     ( this, ResId( MTR_FLD_LENGTH_2 ) ),
    aCbxSynchronize ( this, ResId( CBX_SYNCHRONIZE ) ),
    aGrpDefinition  ( this, ResId( GRP_DEFINITION ) ),
    aLbLineStyles   ( this, ResId( LB_LINESTYLES ) ),
    aGrpLinestyles  ( this, ResId( GRP_LINESTYLES ) ),
    aBtnAdd         ( this, ResId( BTN_ADD ) ),
    aBtnModify      ( this, ResId( BTN_MODIFY ) ),
    aBtnDelete      ( this, ResId( BTN_DELETE ) ),
    aCtlPreview     ( this, ResId( CTL_PREVIEW ), &XOut ),
    aGrpPreview     ( this, ResId( GRP_PREVIEW ) ),
    aBtnLoad        ( this, ResId( BTN_LOAD ) ),
    aBtnSave        ( this, ResId( BTN_SAVE ) ),
    rOutAttrs       ( rInAttrs )

{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Metrik einstellen
    GET_MODULE_FIELDUNIT( eFUnit );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
    }
    SetFieldUnit( aMtrDistance, eFUnit );
    SetFieldUnit( aMtrLength1, eFUnit );
    SetFieldUnit( aMtrLength2, eFUnit );

    // PoolUnit ermitteln
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    rXLSet.Put( aXLStyle );
    rXLSet.Put( aXWidth );
    rXLSet.Put( aXDash );
    rXLSet.Put( aXColor );
    XOut.SetLineAttr( aXLineAttr );

    aBtnAdd.SetClickHdl( LINK( this, SvxLineDefTabPage, ClickAddHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxLineDefTabPage, ClickModifyHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxLineDefTabPage, ClickDeleteHdl_Impl ) );
    aBtnLoad.SetClickHdl( LINK( this, SvxLineDefTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl( LINK( this, SvxLineDefTabPage, ClickSaveHdl_Impl ) );

    aNumFldNumber1.SetModifyHdl(
        LINK( this, SvxLineDefTabPage, ChangeNumber1Hdl_Impl ) );
    aNumFldNumber2.SetModifyHdl(
        LINK( this, SvxLineDefTabPage, ChangeNumber2Hdl_Impl ) );
    aLbLineStyles.SetSelectHdl(
        LINK( this, SvxLineDefTabPage, SelectLinestyleHdl_Impl ) );

    // Absolut (in mm) oder Relativ (in %)
    aCbxSynchronize.SetClickHdl(
        LINK( this, SvxLineDefTabPage, ChangeMetricHdl_Impl ) );

    // Wenn sich etwas aendert, muss Preview upgedatet werden werden
    Link aLink = LINK( this, SvxLineDefTabPage, SelectTypeHdl_Impl );
    aLbType1.SetSelectHdl( aLink );
    aLbType2.SetSelectHdl( aLink );
    aLink = LINK( this, SvxLineDefTabPage, ChangePreviewHdl_Impl );
    aMtrLength1.SetModifyHdl( aLink );
    aMtrLength2.SetModifyHdl( aLink );
    aMtrDistance.SetModifyHdl( aLink );

    pDashList = NULL;
}


// -----------------------------------------------------------------------

void SvxLineDefTabPage::Construct()
{
    // Linienstile
    aLbLineStyles.Fill( pDashList );
}

// -----------------------------------------------------------------------

void SvxLineDefTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        // ActivatePage() wird aufgerufen bevor der Dialog PageCreated() erhaelt !!!
        if( pDashList )
        {
            if( *pPageType == 1 && *pPosDashLb >= 0 &&
                *pPosDashLb != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbLineStyles.SelectEntryPos( *pPosDashLb );
            }
            // Damit evtl. vorhandener Linestyle verworfen wird
            SelectLinestyleHdl_Impl( this );

            // Ermitteln (evtl. abschneiden) des Namens und in
            // der GroupBox darstellen
            String          aString( SVX_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL; aURL.SetSmartURL( pDashList->GetName() );

            if ( aURL.getBase().Len() > 18 )
            {
                aString += aURL.getBase().Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += aURL.getBase();

            aGrpLinestyles.SetText( aString );

            *pPageType = 0; // 2
            *pPosDashLb = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}

// -----------------------------------------------------------------------

int SvxLineDefTabPage::DeactivatePage( SfxItemSet* pSet )
{
    CheckChanges_Impl();

    FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

void SvxLineDefTabPage::CheckChanges_Impl()
{
    // wird hier benutzt, um Aenderungen NICHT zu verlieren
    //XDashStyle eXDS;

    if( aNumFldNumber1.GetText()     != aNumFldNumber1.GetSavedValue() ||
        aMtrLength1.GetText()        != aMtrLength1.GetSavedValue() ||
        aLbType1.GetSelectEntryPos() != aLbType1.GetSavedValue() ||
        aNumFldNumber2.GetText()     != aNumFldNumber2.GetSavedValue() ||
        aMtrLength2.GetText()        != aMtrLength2.GetSavedValue() ||
        aLbType2.GetSelectEntryPos() != aLbType2.GetSavedValue() ||
        aMtrDistance.GetText()       != aMtrDistance.GetSavedValue() )
    {
        ResMgr* pMgr = DIALOG_MGR();
        SvxMessDialog aMessDlg( DLGWIN,
            String( ResId( RID_SVXSTR_LINESTYLE, pMgr ) ),
            String( ResId( RID_SVXSTR_ASK_CHANGE_LINESTYLE, pMgr ) ),
            &( WarningBox::GetStandardImage() ) );

        aMessDlg.SetButtonText( MESS_BTN_1,
                                String( ResId( RID_SVXSTR_CHANGE, pMgr ) ) );
        aMessDlg.SetButtonText( MESS_BTN_2,
                                String( ResId( RID_SVXSTR_ADD, pMgr ) ) );

        short nRet = aMessDlg.Execute();

        switch( nRet )
        {
            case RET_BTN_1: // Aendern
            {
                ClickModifyHdl_Impl( this );
                //aXDash = pDashList->Get( nPos )->GetDash();
            }
            break;

            case RET_BTN_2: // Hinzufuegen
            {
                ClickAddHdl_Impl( this );
                //nPos = aLbLineStyles.GetSelectEntryPos();
                //aXDash = pDashList->Get( nPos )->GetDash();
            }
            break;

            case RET_CANCEL:
            break;
            // return( TRUE ); // Abbruch
        }
    }



    USHORT nPos = aLbLineStyles.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        *pPosDashLb = nPos;
    }
}

// -----------------------------------------------------------------------

BOOL SvxLineDefTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    if( *pDlgType == 0 ) // Linien-Dialog
    {
        if( *pPageType == 2 )
        {
            //CheckChanges_Impl();

            FillDash_Impl();

            String aString( aLbLineStyles.GetSelectEntry() );
            rAttrs.Put( XLineStyleItem( XLINE_DASH ) );
            rAttrs.Put( XLineDashItem( aString, aDash ) );
        }
    }
    return( TRUE );
}

// -----------------------------------------------------------------------

void SvxLineDefTabPage::Reset( const SfxItemSet& rAttrs )
{
    if( rAttrs.GetItemState( GetWhich( XATTR_LINESTYLE ) ) != SFX_ITEM_DONTCARE )
    {
        XLineStyle eXLS = (XLineStyle) ( ( const XLineStyleItem& ) rAttrs.Get( GetWhich( XATTR_LINESTYLE ) ) ).GetValue();

        switch( eXLS )
        {
            case XLINE_NONE:
            case XLINE_SOLID:
                aLbLineStyles.SelectEntryPos( 0 );
                break;

            case XLINE_DASH:
            {
                const XLineDashItem& rDashItem = ( const XLineDashItem& ) rAttrs.Get( XATTR_LINEDASH );
                aDash = rDashItem.GetValue();

                aLbLineStyles.SetNoSelection();
                aLbLineStyles.SelectEntry( rDashItem.GetName() );
            }
                break;

            default:
                break;
        }
    }
    SelectLinestyleHdl_Impl( NULL );

    // Status der Buttons ermitteln
    if( pDashList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
}

// -----------------------------------------------------------------------

SfxTabPage* SvxLineDefTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxLineDefTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, SelectLinestyleHdl_Impl, void *, p )
{
    if( pDashList->Count() > 0 )
    {
        int nTmp = aLbLineStyles.GetSelectEntryPos();
        if( nTmp == LISTBOX_ENTRY_NOTFOUND )
        {
        }
        else
            aDash = pDashList->Get( nTmp )->GetDash();

        FillDialog_Impl();

        rXLSet.Put( XLineDashItem( String(), aDash ) );
        XOut.SetLineAttr( aXLineAttr );

        aCtlPreview.Invalidate();

        // Wird erst hier gesetzt, um den Style nur dann zu uebernehmen,
        // wenn in der ListBox ein Eintrag ausgewaehlt wurde
        // Wenn ueber Reset() gerufen wurde ist p == NULL
        if( p )
            *pPageType = 2;
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxLineDefTabPage, ChangePreviewHdl_Impl, void *, EMPTYARG )
{
    FillDash_Impl();
    aCtlPreview.Invalidate();

    return( 0L );
}
IMPL_LINK_INLINE_END( SvxLineDefTabPage, ChangePreviewHdl_Impl, void *, EMPTYARG )

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ChangeNumber1Hdl_Impl, void *, EMPTYARG )
{
    if( aNumFldNumber1.GetValue() == 0L )
    {
        aNumFldNumber2.SetMin( 1L );
        aNumFldNumber2.SetFirst( 1L );
    }
    else
    {
        aNumFldNumber2.SetMin( 0L );
        aNumFldNumber2.SetFirst( 0L );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ChangeNumber2Hdl_Impl, void *, EMPTYARG )
{
    if( aNumFldNumber2.GetValue() == 0L )
    {
        aNumFldNumber1.SetMin( 1L );
        aNumFldNumber1.SetFirst( 1L );
    }
    else
    {
        aNumFldNumber1.SetMin( 0L );
        aNumFldNumber1.SetFirst( 0L );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}


//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ChangeMetricHdl_Impl, void *, p )
{
    if( !aCbxSynchronize.IsChecked() && aMtrLength1.GetUnit() != eFUnit )
    {
        long nTmp1, nTmp2, nTmp3;

        // Wurde ueber Control geaendert
        if( p )
        {
            nTmp1 = GetCoreValue( aMtrLength1, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp2 = GetCoreValue( aMtrLength2, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp3 = GetCoreValue( aMtrDistance, ePoolUnit ) * XOUT_WIDTH / 100;
        }
        else
        {
            nTmp1 = GetCoreValue( aMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( aMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( aMtrDistance, ePoolUnit );
        }
        aMtrLength1.SetDecimalDigits( 2 );
        aMtrLength2.SetDecimalDigits( 2 );
        aMtrDistance.SetDecimalDigits( 2 );

        // Metrik einstellen
        aMtrLength1.SetUnit( eFUnit );
        aMtrLength2.SetUnit( eFUnit );
        aMtrDistance.SetUnit( eFUnit );

        SetMetricValue( aMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( aMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( aMtrDistance, nTmp3, ePoolUnit );
    }
    else if( aCbxSynchronize.IsChecked() && aMtrLength1.GetUnit() != FUNIT_CUSTOM )
    {
        long nTmp1, nTmp2, nTmp3;

        // Wurde ueber Control geaendert
        if( p )
        {
            nTmp1 = GetCoreValue( aMtrLength1, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp2 = GetCoreValue( aMtrLength2, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp3 = GetCoreValue( aMtrDistance, ePoolUnit ) * 100 / XOUT_WIDTH;
        }
        else
        {
            nTmp1 = GetCoreValue( aMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( aMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( aMtrDistance, ePoolUnit );
        }

        aMtrLength1.SetDecimalDigits( 0 );
        aMtrLength2.SetDecimalDigits( 0 );
        aMtrDistance.SetDecimalDigits( 0 );

        aMtrLength1.SetUnit( FUNIT_CUSTOM );
        aMtrLength2.SetUnit( FUNIT_CUSTOM );
        aMtrDistance.SetUnit( FUNIT_CUSTOM );


        SetMetricValue( aMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( aMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( aMtrDistance, nTmp3, ePoolUnit );
    }
    SelectTypeHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, SelectTypeHdl_Impl, void *, p )
{
    if ( p == &aLbType1 || !p )
    {
        if ( aLbType1.GetSelectEntryPos() == 0 )
        {
            aMtrLength1.Disable();
            aMtrLength1.SetText( String() );
        }
        else if ( !aMtrLength1.IsEnabled() )
        {
            aMtrLength1.Enable();
            aMtrLength1.Reformat();
        }
    }

    if ( p == &aLbType2 || !p )
    {
        if ( aLbType2.GetSelectEntryPos() == 0 )
        {
            aMtrLength2.Disable();
            aMtrLength2.SetText( String() );
        }
        else if ( !aMtrLength2.IsEnabled() )
        {
            aMtrLength2.Enable();
            aMtrLength2.Reformat();
        }
    }
    ChangePreviewHdl_Impl( p );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
    ResMgr* pMgr = DIALOG_MGR();
    String aNewName( ResId( RID_SVXSTR_LINESTYLE, pMgr ) );
    String aDesc( ResId( RID_SVXSTR_DESC_LINESTYLE, pMgr ) );
    String aName;
    XDashEntry* pEntry;

    long nCount = pDashList->Count();
    long j = 1;
    BOOL bDifferent = FALSE;

    while ( !bDifferent )
    {
        aName = aNewName;
        aName += sal_Unicode(' ');
        aName += UniString::CreateFromInt32( j++ );
        bDifferent = TRUE;

        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pDashList->Get( i )->GetName() )
                bDifferent = FALSE;
    }

    SvxNameDialog* pDlg = new SvxNameDialog( DLGWIN, aName, aDesc );
    BOOL bLoop = TRUE;

    while ( bLoop && pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );
        bDifferent = TRUE;

        for( long i = 0; i < nCount && bDifferent; i++ )
        {
            if( aName == pDashList->Get( i )->GetName() )
                bDifferent = FALSE;
        }

        if( bDifferent )
        {
            bLoop = FALSE;
            FillDash_Impl();

            pEntry = new XDashEntry( aDash, aName );

            long nCount = pDashList->Count();
            pDashList->Insert( pEntry, nCount );
            Bitmap* pBitmap = pDashList->GetBitmap( nCount );
            aLbLineStyles.Append( pEntry, pBitmap );

            aLbLineStyles.SelectEntryPos( aLbLineStyles.GetEntryCount() - 1 );

            // Flag fuer modifiziert setzen
            *pnDashListState |= CT_MODIFIED;

            *pPageType = 2;

            // Werte sichern fuer Changes-Erkennung ( -> Methode )
            aNumFldNumber1.SaveValue();
            aMtrLength1.SaveValue();
            aLbType1.SaveValue();
            aNumFldNumber2.SaveValue();
            aMtrLength2.SaveValue();
            aLbType2.SaveValue();
            aMtrDistance.SaveValue();
        }
        else
            WarningBox( DLGWIN, WinBits( WB_OK ),
                String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, pMgr ) ) ).Execute();
    }
    delete( pDlg );

    // Status der Buttons ermitteln
    if ( pDashList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbLineStyles.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr* pMgr = DIALOG_MGR();
        String aNewName( ResId( RID_SVXSTR_LINESTYLE, pMgr ) );
        String aDesc( ResId( RID_SVXSTR_DESC_LINESTYLE, pMgr ) );
        String aName( pDashList->Get( nPos )->GetName() );
        String aOldName = aName;

        SvxNameDialog* pDlg = new SvxNameDialog( DLGWIN, aName, aDesc );

        long nCount = pDashList->Count();
        BOOL bDifferent = FALSE;
        BOOL bLoop = TRUE;

        while ( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = TRUE;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pDashList->Get( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = FALSE;
            }

            if ( bDifferent )
            {
                bLoop = FALSE;
                FillDash_Impl();

                XDashEntry* pEntry = new XDashEntry( aDash, aName );

                delete pDashList->Replace( pEntry, nPos );
                Bitmap* pBitmap = pDashList->GetBitmap( nPos );
                aLbLineStyles.Modify( pEntry, nPos, pBitmap );

                aLbLineStyles.SelectEntryPos( nPos );

                // Flag fuer modifiziert setzen
                *pnDashListState |= CT_MODIFIED;

                *pPageType = 2;

                // Werte sichern fuer Changes-Erkennung ( -> Methode )
                aNumFldNumber1.SaveValue();
                aMtrLength1.SaveValue();
                aLbType1.SaveValue();
                aNumFldNumber2.SaveValue();
                aMtrLength2.SaveValue();
                aLbType2.SaveValue();
                aMtrDistance.SaveValue();
            }
            else
                WarningBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, pMgr ) ) ).Execute();
        }
        delete( pDlg );
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbLineStyles.GetSelectEntryPos();

    if ( nPos >= 0 )
    {
        QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
//!         SVX_RES( RID_SVXSTR_DEL_LINESTYLE ),
            String( SVX_RES( RID_SVXSTR_ASK_DEL_LINESTYLE ) ) );

        if ( aQueryBox.Execute() == RET_YES )
        {
            delete pDashList->Remove( nPos );
            aLbLineStyles.RemoveEntry( nPos );
            aLbLineStyles.SelectEntryPos( 0 );

            SelectLinestyleHdl_Impl( this );
            *pPageType = 0; // Style soll nicht uebernommen werden

            // Flag fuer modifiziert setzen
            *pnDashListState |= CT_MODIFIED;

            ChangePreviewHdl_Impl( this );
        }
    }

    // Status der Buttons ermitteln
    if ( !pDashList->Count() )
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ClickLoadHdl_Impl, void *, p )
{
    ResMgr* pMgr = DIALOG_MGR();
    USHORT nReturn = RET_YES;

    if ( *pnDashListState & CT_MODIFIED )
    {
        nReturn = WarningBox( DLGWIN, WinBits( WB_YES_NO_CANCEL ),
            String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, pMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pDashList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_OPEN | WB_3DLOOK );

        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sod" ) );
        pFileDlg->AddFilter( aStrFilterType, aStrFilterType );

        String aFile( SFX_APP()->GetAppIniManager()->Get( SFX_KEY_PALETTE_PATH ) );
        pFileDlg->SetPath( aFile );

        if( pFileDlg->Execute() == RET_OK )
        {
            INetURLObject aURL; aURL.SetSmartURL( pFileDlg->GetPath() );
            INetURLObject aPathURL( aURL ); aPathURL.removeSegment(); aPathURL.removeFinalSlash();

            // Liste speichern
            XDashList* pDshLst = new XDashList( aPathURL.PathToFileName(), pXPool );
            pDshLst->SetName( aURL.getName() );

            if( pDshLst->Load() )
            {
                if( pDshLst )
                {
                    // Pruefen, ob Tabelle geloescht werden darf:
                    if( pDashList != ( (SvxLineTabDialog*) DLGWIN )->GetDashList() )
                        delete pDashList;

                    pDashList = pDshLst;
                    ( (SvxLineTabDialog*) DLGWIN )->SetNewDashList( pDashList );

                    aLbLineStyles.Clear();
                    aLbLineStyles.Fill( pDashList );
                    Reset( rOutAttrs );

                    pDashList->SetName( aURL.getName() );

                    // Ermitteln (evtl. abschneiden) des Namens und in
                    // der GroupBox darstellen
                    String aString( ResId( RID_SVXSTR_TABLE, pMgr ) );
                    aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

                    if ( aURL.getBase().Len() > 18 )
                    {
                        aString += aURL.getBase().Copy( 0, 15 );
                        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
                    }
                    else
                        aString += aURL.getBase();

                    aGrpLinestyles.SetText( aString );

                    // Flag fuer gewechselt setzen
                    *pnDashListState |= CT_CHANGED;
                    // Flag fuer modifiziert entfernen
                    *pnDashListState &= ~CT_MODIFIED;
                }
            }
            else
                //aIStream.Close();
                ErrorBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, pMgr ) ) ).Execute();
        }
        delete( pFileDlg );
    }

    // Status der Buttons ermitteln
    if ( pDashList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ClickSaveHdl_Impl, void *, p )
{
    SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_SAVEAS | WB_3DLOOK );

    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sod" ) );
    pFileDlg->AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile; aFile.SetSmartURL( SFX_APP()->GetAppIniManager()->Get( SFX_KEY_PALETTE_PATH ) );

    if( pDashList->GetName().Len() )
    {
        aFile.Append( pDashList->GetName() );

        if( !aFile.getExtension().Len() )
            aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "sod" ) ) );
    }

    pFileDlg->SetPath( aFile.PathToFileName() );

    if( pFileDlg->Execute() == RET_OK )
    {
        INetURLObject aURL; aURL.SetSmartURL( pFileDlg->GetPath() );
        INetURLObject aPathURL( aURL ); aPathURL.removeSegment(); aPathURL.removeFinalSlash();

        pDashList->SetName( aURL.getName() );
        pDashList->SetPath( aPathURL.PathToFileName() );

        if( pDashList->Save() )
        {
            // Ermitteln (evtl. abschneiden) des Namens und in
            // der GroupBox darstellen
            String aString( SVX_RES( RID_SVXSTR_TABLE ) );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

            if ( aURL.getBase().Len() > 18 )
            {
                aString += aURL.getBase().Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += aURL.getBase();

            aGrpLinestyles.SetText( aString );

            // Flag fuer gespeichert setzen
            *pnDashListState |= CT_SAVED;
            // Flag fuer modifiziert entfernen
            *pnDashListState &= ~CT_MODIFIED;
        }
        else
        {
            ErrorBox( DLGWIN, WinBits( WB_OK ),
                String( SVX_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
        }
    }
    delete( pFileDlg );

    return( 0L );
}

//------------------------------------------------------------------------

void SvxLineDefTabPage::FillDash_Impl()
{
    XDashStyle eXDS;

/*  Alle Stile werden z.Z. nicht benutzt
    if( aRbtEnds1.IsChecked() )
        eXDS = XDASH_ROUND;
    else if( aRbtEnds2.IsChecked() )
        eXDS = XDASH_RECT;
    else
        eXDS = XDASH_RECT;
*/
    if( aCbxSynchronize.IsChecked() )
        eXDS = XDASH_RECTRELATIVE;
    else
        eXDS = XDASH_RECT;

    aDash.SetDashStyle( eXDS );
    aDash.SetDots( (BYTE) aNumFldNumber1.GetValue() );
    aDash.SetDotLen( aLbType1.GetSelectEntryPos() == 0 ? 0 :
                                GetCoreValue( aMtrLength1, ePoolUnit ) );
    aDash.SetDashes( (BYTE) aNumFldNumber2.GetValue() );
    aDash.SetDashLen( aLbType2.GetSelectEntryPos() == 0 ? 0 :
                                GetCoreValue( aMtrLength2, ePoolUnit ) );
    aDash.SetDistance( GetCoreValue( aMtrDistance, ePoolUnit ) );

    rXLSet.Put( XLineDashItem( String(), aDash ) );
    XOut.SetLineAttr( aXLineAttr );
}

//------------------------------------------------------------------------

void SvxLineDefTabPage::FillDialog_Impl()
{
    XDashStyle eXDS = aDash.GetDashStyle(); // XDASH_RECT, XDASH_ROUND
    if( eXDS == XDASH_RECTRELATIVE )
        aCbxSynchronize.Check();
    else
        aCbxSynchronize.Check( FALSE );

    aNumFldNumber1.SetValue( aDash.GetDots() );
    //aMtrLength1.SetValue( aDash.GetDotLen() );
    SetMetricValue( aMtrLength1, aDash.GetDotLen(), ePoolUnit );
    aLbType1.SelectEntryPos( aDash.GetDotLen() == 0 ? 0 : 1 );
    aNumFldNumber2.SetValue( aDash.GetDashes() );
    //aMtrLength2.SetValue( aDash.GetDashLen() );
    SetMetricValue( aMtrLength2, aDash.GetDashLen(), ePoolUnit );
    aLbType2.SelectEntryPos( aDash.GetDashLen() == 0 ? 0 : 1 );
    //aMtrDistance.SetValue( aDash.GetDistance() );
    SetMetricValue( aMtrDistance, aDash.GetDistance(), ePoolUnit );

    ChangeMetricHdl_Impl( NULL );

    // Werte sichern fuer Changes-Erkennung ( -> Methode )
    aNumFldNumber1.SaveValue();
    aMtrLength1.SaveValue();
    aLbType1.SaveValue();
    aNumFldNumber2.SaveValue();
    aMtrLength2.SaveValue();
    aLbType2.SaveValue();
    aMtrDistance.SaveValue();
}


