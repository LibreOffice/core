/*************************************************************************
 *
 *  $RCSfile: tpgradnt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 10:48:03 $
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

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXFILEDLG_HXX
#include <sfx2/iodlg.hxx>
#endif

#define _SVX_TPGRADNT_CXX
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST

#include "dialogs.hrc"

#include "xattr.hxx"
#include "xtable.hxx"
#include "xpool.hxx"
#include "drawitem.hxx"
#include "tabarea.hxx"
#include "tabarea.hrc"
#include "dlgname.hxx"
#include "dlgname.hrc"
#include "dialmgr.hxx"

#define DLGWIN this->GetParent()->GetParent()

#define BITMAP_WIDTH  32
#define BITMAP_HEIGHT 12

/*************************************************************************
|*
|*  Dialog zum Aendern und Definieren der Farbverlaeufe
|*
\************************************************************************/

SvxGradientTabPage::SvxGradientTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :
    SfxTabPage          ( pParent, SVX_RES( RID_SVXPAGE_GRADIENT ), rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    //XOut              ( &aCtlPreview, pXPool ),
    XOut                ( &aCtlPreview ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() ),

    aXFStyleItem        ( XFILL_GRADIENT ),
    aXGradientItem      ( String(), XGradient( COL_BLACK, COL_WHITE ) ),

    aFtType             ( this, ResId( FT_TYPE ) ),
    aLbGradientType     ( this, ResId( LB_GRADIENT_TYPES ) ),
    aFtCenterX          ( this, ResId( FT_CENTER_X ) ),
    aMtrCenterX         ( this, ResId( MTR_CENTER_X ) ),
    aFtCenterY          ( this, ResId( FT_CENTER_Y ) ),
    aMtrCenterY         ( this, ResId( MTR_CENTER_Y ) ),
    aFtAngle            ( this, ResId( FT_ANGLE ) ),
    aMtrAngle           ( this, ResId( MTR_ANGLE ) ),
    aFtBorder           ( this, ResId( FT_BORDER ) ),
    aMtrBorder          ( this, ResId( MTR_BORDER ) ),
    aFtColorFrom        ( this, ResId( FT_COLOR_FROM ) ),
    aMtrColorFrom       ( this, ResId( MTR_COLOR_FROM ) ),
    aLbColorFrom        ( this, ResId( LB_COLOR_FROM ) ),
    aFtColorTo          ( this, ResId( FT_COLOR_TO ) ),
    aMtrColorTo         ( this, ResId( MTR_COLOR_TO ) ),
    aLbColorTo          ( this, ResId( LB_COLOR_TO ) ),
    aGrpColor           ( this, ResId( GRP_COLOR ) ),
    aLbGradients        ( this, ResId( LB_GRADIENTS ) ),
    aGrpGradients       ( this, ResId( GRP_GRADIENTS ) ),
    aCtlPreview         ( this, ResId( CTL_PREVIEW ), &XOut ),
    aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
    aBtnAdd             ( this, ResId( BTN_ADD ) ),
    aBtnModify          ( this, ResId( BTN_MODIFY ) ),
    aBtnDelete          ( this, ResId( BTN_DELETE ) ),
    aBtnLoad            ( this, ResId( BTN_LOAD ) ),
    aBtnSave            ( this, ResId( BTN_SAVE ) ),
    rOutAttrs           ( rInAttrs )

{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Solange NICHT vom Item unterstuetzt

    //aMtrColorTo.Disable();
    aMtrColorTo.SetValue( 100 );
    //aMtrColorFrom.Disable();
    aMtrColorFrom.SetValue( 100 );

    // Setzen des Output-Devices
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXGradientItem );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );

    // Setzen der Linie auf None im OutputDevice
    XLineAttrSetItem aXLineAttr( pXPool );
    aXLineAttr.GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
    XOut.SetLineAttr( aXLineAttr.GetItemSet() );

    // Handler ueberladen
    aLbGradients.SetSelectHdl(
        LINK( this, SvxGradientTabPage, ChangeGradientHdl_Impl ) );
    aBtnAdd.SetClickHdl( LINK( this, SvxGradientTabPage, ClickAddHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickModifyHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickDeleteHdl_Impl ) );

    Link aLink = LINK( this, SvxGradientTabPage, ModifiedHdl_Impl );
    aLbGradientType.SetSelectHdl( aLink );
    aMtrCenterX.SetModifyHdl( aLink );
    aMtrCenterY.SetModifyHdl( aLink );
    aMtrAngle.SetModifyHdl( aLink );
    aMtrBorder.SetModifyHdl( aLink );
    aMtrColorFrom.SetModifyHdl( aLink );
    aLbColorFrom.SetSelectHdl( aLink );
    aMtrColorTo.SetModifyHdl( aLink );
    aLbColorTo.SetSelectHdl( aLink );

    aBtnLoad.SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickSaveHdl_Impl ) );

    pColorTab = NULL;
    pGradientList = NULL;
}

// -----------------------------------------------------------------------

void SvxGradientTabPage::Construct()
{
    // Farbtabelle
    aLbColorFrom.Fill( pColorTab );
    aLbColorTo.CopyEntries( aLbColorFrom );

    // Farbverlauftabelle
    aLbGradients.Fill( pGradientList );
}

// -----------------------------------------------------------------------

void SvxGradientTabPage::ActivatePage( const SfxItemSet& rSet )
{
    int nPos;
    int nCount;

    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        *pbAreaTP = FALSE;

        if( pColorTab )
        {
            // ColorTable
            if( *pnColorTableState & CT_CHANGED ||
                *pnColorTableState & CT_MODIFIED )
            {
                if( *pnColorTableState & CT_CHANGED )
                    pColorTab = ( (SvxAreaTabDialog*) DLGWIN )->GetNewColorTable();

                // LbColorFrom
                nPos = aLbColorFrom.GetSelectEntryPos();
                aLbColorFrom.Clear();
                aLbColorFrom.Fill( pColorTab );
                nCount = aLbColorFrom.GetEntryCount();
                if( nCount == 0 )
                    ; // Dieser Fall sollte nicht auftreten
                else if( nCount <= nPos )
                    aLbColorFrom.SelectEntryPos( 0 );
                else
                    aLbColorFrom.SelectEntryPos( nPos );

                // LbColorTo
                nPos = aLbColorTo.GetSelectEntryPos();
                aLbColorTo.Clear();
                aLbColorTo.CopyEntries( aLbColorFrom );
                nCount = aLbColorTo.GetEntryCount();
                if( nCount == 0 )
                    ; // Dieser Fall sollte nicht auftreten
                else if( nCount <= nPos )
                    aLbColorTo.SelectEntryPos( 0 );
                else
                    aLbColorTo.SelectEntryPos( nPos );

                ModifiedHdl_Impl( this );
            }

            // Ermitteln (evtl. abschneiden) des Namens und in
            // der GroupBox darstellen
            String          aString( SVX_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL( pGradientList->GetName(), INET_PROT_FILE );

            if ( aURL.getBase().Len() > 18 )
            {
                aString += aURL.getBase().Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += aURL.getBase();

            aGrpGradients.SetText( aString );

            if ( *pPageType == PT_GRADIENT && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbGradients.SelectEntryPos( *pPos );
            }
            // Farben koennten geloescht worden sein
            ChangeGradientHdl_Impl( this );

            *pPageType = PT_GRADIENT;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}

// -----------------------------------------------------------------------

int SvxGradientTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    FillItemSet( *pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

long SvxGradientTabPage::CheckChanges_Impl()
{
    // wird hier benutzt, um Aenderungen NICHT zu verlieren
    XGradient aTmpGradient( aLbColorFrom.GetSelectEntryColor(),
                          aLbColorTo.GetSelectEntryColor(),
                          (XGradientStyle) aLbGradientType.GetSelectEntryPos(),
                          aMtrAngle.GetValue() * 10, // sollte in Resource geaendert werden
                          (USHORT) aMtrCenterX.GetValue(),
                          (USHORT) aMtrCenterY.GetValue(),
                          (USHORT) aMtrBorder.GetValue(),
                          (USHORT) aMtrColorFrom.GetValue(),
                          (USHORT) aMtrColorTo.GetValue() );

    USHORT nPos = aLbGradients.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        XGradient aGradient = pGradientList->Get( nPos )->GetGradient();
        String aString = aLbGradients.GetSelectEntry();

        if( !( aTmpGradient == aGradient ) )
        {
            ResMgr* pMgr = DIALOG_MGR();
            SvxMessDialog aMessDlg( DLGWIN,
                String( ResId( RID_SVXSTR_GRADIENT, pMgr ) ),
                String( ResId( RID_SVXSTR_ASK_CHANGE_GRADIENT, pMgr ) ),
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
                    aGradient = pGradientList->Get( nPos )->GetGradient();
                }
                break;

                case RET_BTN_2: // Hinzufuegen
                {
                    ClickAddHdl_Impl( this );
                    nPos = aLbGradients.GetSelectEntryPos();
                    aGradient = pGradientList->Get( nPos )->GetGradient();
                }
                break;

                case RET_CANCEL:
                    // return( -1L ); <-- wuerde die Seite nicht verlassen
                break;
                // return( TRUE ); // Abbruch
            }
        }
    }
    nPos = aLbGradients.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        *pPos = nPos;
    }
    return 0L;
}

// -----------------------------------------------------------------------

BOOL SvxGradientTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if( *pDlgType == 0 && *pPageType == PT_GRADIENT && *pbAreaTP == FALSE )
    {
        // CheckChanges(); <-- doppelte Abfrage ?

        XGradient*  pXGradient = NULL;
        String      aString;
        USHORT      nPos = aLbGradients.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            pXGradient = new XGradient( pGradientList->Get( nPos )->GetGradient() );
            aString = aLbGradients.GetSelectEntry();

        }
        else
        // Farbverlauf wurde (unbekannt) uebergeben
        {
            pXGradient = new XGradient( aLbColorFrom.GetSelectEntryColor(),
                        aLbColorTo.GetSelectEntryColor(),
                        (XGradientStyle) aLbGradientType.GetSelectEntryPos(),
                        aMtrAngle.GetValue() * 10, // sollte in Resource geaendert werden
                        (USHORT) aMtrCenterX.GetValue(),
                        (USHORT) aMtrCenterY.GetValue(),
                        (USHORT) aMtrBorder.GetValue(),
                        (USHORT) aMtrColorFrom.GetValue(),
                        (USHORT) aMtrColorTo.GetValue() );
        }
        DBG_ASSERT( pXGradient, "XGradient konnte nicht erzeugt werden" );
        rOutAttrs.Put( XFillStyleItem( XFILL_GRADIENT ) );
        rOutAttrs.Put( XFillGradientItem( aString, *pXGradient ) );

        delete pXGradient;
    }
    return TRUE;
}

// -----------------------------------------------------------------------

void SvxGradientTabPage::Reset( const SfxItemSet& rOutAttrs )
{
    // aLbGradients.SelectEntryPos( 0 );
    ChangeGradientHdl_Impl( this );

    // Status der Buttons ermitteln
    if( pGradientList->Count() )
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

SfxTabPage* SvxGradientTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return new SvxGradientTabPage( pWindow, rOutAttrs );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGradientTabPage, ModifiedHdl_Impl, void *, pControl )
{
    XGradientStyle eXGS = (XGradientStyle) aLbGradientType.GetSelectEntryPos();

    XGradient aXGradient( aLbColorFrom.GetSelectEntryColor(),
                          aLbColorTo.GetSelectEntryColor(),
                          eXGS,
                          aMtrAngle.GetValue() * 10, // sollte in Resource geaendert werden
                          (USHORT) aMtrCenterX.GetValue(),
                          (USHORT) aMtrCenterY.GetValue(),
                          (USHORT) aMtrBorder.GetValue(),
                          (USHORT) aMtrColorFrom.GetValue(),
                          (USHORT) aMtrColorTo.GetValue() );

    // Enablen/Disablen von Controls
    if( pControl == &aLbGradientType || pControl == this )
        SetControlState_Impl( eXGS );

    // Anzeigen im XOutDev
    rXFSet.Put( XFillGradientItem( String(), aXGradient ) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );

    aCtlPreview.Invalidate();

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGradientTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
    ResMgr* pMgr = DIALOG_MGR();
    String aNewName( ResId( RID_SVXSTR_GRADIENT, pMgr ) );
    String aDesc( ResId( RID_SVXSTR_DESC_GRADIENT, pMgr ) );
    String aName;

    long nCount = pGradientList->Count();
    long j = 1;
    BOOL bDifferent = FALSE;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += sal_Unicode(' ');
        aName += UniString::CreateFromInt32( j++ );
        bDifferent = TRUE;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pGradientList->Get( i )->GetName() )
                bDifferent = FALSE;
    }

    SvxNameDialog* pDlg     = new SvxNameDialog( DLGWIN, aName, aDesc );
    WarningBox*    pWarnBox = NULL;
    USHORT         nError   = RID_SVXSTR_WARN_NAME_DUPLICATE;

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bDifferent = TRUE;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pGradientList->Get( i )->GetName() )
                bDifferent = FALSE;

        if( bDifferent )
        {
            nError = 0;
            break;
        }

        if( !pWarnBox )
            pWarnBox = new WarningBox( DLGWIN,
                                       WinBits( WB_OK_CANCEL ),
                                       String( ResId( nError, pMgr ) ) );

        if( pWarnBox->Execute() != RET_OK )
            break;
    }
    //Rectangle aDlgRect( pDlg->GetPosPixel(), pDlg->GetSizePixel() );
    delete pDlg;
    delete pWarnBox;

    if( !nError )
    {
        XGradient aXGradient( aLbColorFrom.GetSelectEntryColor(),
                              aLbColorTo.GetSelectEntryColor(),
                              (XGradientStyle) aLbGradientType.GetSelectEntryPos(),
                              aMtrAngle.GetValue() * 10, // sollte in Resource geaendert werden
                              (USHORT) aMtrCenterX.GetValue(),
                              (USHORT) aMtrCenterY.GetValue(),
                              (USHORT) aMtrBorder.GetValue(),
                              (USHORT) aMtrColorFrom.GetValue(),
                              (USHORT) aMtrColorTo.GetValue() );
        XGradientEntry* pEntry = new XGradientEntry( aXGradient, aName );

        pGradientList->Insert( pEntry, nCount );

        aLbGradients.Append( pEntry );

        aLbGradients.SelectEntryPos( aLbGradients.GetEntryCount() - 1 );

#ifdef WNT
        // hack: #31355# W.P.
        Rectangle aRect( aLbGradients.GetPosPixel(), aLbGradients.GetSizePixel() );
        if( TRUE ) {                // ??? overlapped with pDlg
                                    // and srolling
            Invalidate( aRect );
            //aLbGradients.Invalidate();
        }
#endif

        // Flag fuer modifiziert setzen
        *pnGradientListState |= CT_MODIFIED;

        ChangeGradientHdl_Impl( this );
    }

    // Status der Buttons ermitteln
    if( pGradientList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGradientTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbGradients.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr* pMgr = DIALOG_MGR();
        String aNewName( ResId( RID_SVXSTR_GRADIENT, pMgr ) );
        String aDesc( ResId( RID_SVXSTR_DESC_GRADIENT, pMgr ) );
        String aName( pGradientList->Get( nPos )->GetName() );
        String aOldName = aName;

        SvxNameDialog* pDlg = new SvxNameDialog( DLGWIN, aName, aDesc );

        long nCount = pGradientList->Count();
        BOOL bDifferent = FALSE;
        BOOL bLoop = TRUE;

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = TRUE;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pGradientList->Get( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = FALSE;
            }

            if( bDifferent )
            {
                bLoop = FALSE;
                XGradient aXGradient( aLbColorFrom.GetSelectEntryColor(),
                                      aLbColorTo.GetSelectEntryColor(),
                                      (XGradientStyle) aLbGradientType.GetSelectEntryPos(),
                                      aMtrAngle.GetValue() * 10, // sollte in Resource geaendert werden
                                      (USHORT) aMtrCenterX.GetValue(),
                                      (USHORT) aMtrCenterY.GetValue(),
                                      (USHORT) aMtrBorder.GetValue(),
                                      (USHORT) aMtrColorFrom.GetValue(),
                                      (USHORT) aMtrColorTo.GetValue() );

                XGradientEntry* pEntry = new XGradientEntry( aXGradient, aName );

                delete pGradientList->Replace( pEntry, nPos );

                aLbGradients.Modify( pEntry, nPos );

                aLbGradients.SelectEntryPos( nPos );

                // Flag fuer modifiziert setzen
                *pnGradientListState |= CT_MODIFIED;
            }
            else
                WarningBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, pMgr ) ) ).Execute();
        }
        delete pDlg;
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGradientTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbGradients.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
//!         SVX_RES( RID_SVXSTR_DEL_GRADIENT ),
            String( SVX_RES( RID_SVXSTR_ASK_DEL_GRADIENT ) ) );

        if ( aQueryBox.Execute() == RET_YES )
        {
            delete pGradientList->Remove( nPos );
            aLbGradients.RemoveEntry( nPos );
            aLbGradients.SelectEntryPos( 0 );

            aCtlPreview.Invalidate();

            ChangeGradientHdl_Impl( this );

            // Flag fuer modifiziert setzen
            *pnGradientListState |= CT_MODIFIED;
        }
    }
    // Status der Buttons ermitteln
    if( !pGradientList->Count() )
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxGradientTabPage, ClickLoadHdl_Impl, void *, p )
{
    ResMgr* pMgr = DIALOG_MGR();
    USHORT nReturn = RET_YES;

    if ( *pnGradientListState & CT_MODIFIED )
    {
        nReturn = WarningBox( DLGWIN, WinBits( WB_YES_NO_CANCEL ),
            String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, pMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pGradientList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_OPEN | WB_3DLOOK );

        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sog" ) );
        pFileDlg->AddFilter( aStrFilterType, aStrFilterType );
        String aFile( SvtPathOptions().GetPalettePath() );
        pFileDlg->SetPath( aFile );

        if( pFileDlg->Execute() == RET_OK )
        {
            EnterWait();

            INetURLObject aURL( pFileDlg->GetPath(), INET_PROT_FILE );
            INetURLObject aPathURL( aURL ); aPathURL.removeSegment(); aPathURL.removeFinalSlash();

            // Liste speichern
            XGradientList* pGrdList = new XGradientList( aPathURL.PathToFileName(), pXPool );
            pGrdList->SetName( aURL.getName() );

            if ( pGrdList->Load() )
            {
                if ( pGrdList )
                {
                    // Pruefen, ob Tabelle geloescht werden darf:
                    if ( pGradientList !=
                         ( (SvxAreaTabDialog*) DLGWIN )->GetGradientList() )
                        delete pGradientList;

                    pGradientList = pGrdList;
                    ( (SvxAreaTabDialog*) DLGWIN )->
                        SetNewGradientList( pGradientList );

                    aLbGradients.Clear();
                    aLbGradients.Fill( pGradientList );
                    Reset( rOutAttrs );

                    pGradientList->SetName( aURL.getName() );

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

                    aGrpGradients.SetText( aString );


                    // Flag fuer gewechselt setzen
                    *pnGradientListState |= CT_CHANGED;
                    // Flag fuer modifiziert entfernen
                    *pnGradientListState &= ~CT_MODIFIED;
                }
                LeaveWait();
            }
            else
            {
                LeaveWait();
                //aIStream.Close();
                ErrorBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, pMgr ) ) ).Execute();
            }
        }
        delete pFileDlg;
    }

    // Status der Buttons ermitteln
    if( pGradientList->Count() )
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
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxGradientTabPage, ClickSaveHdl_Impl, void *, p )
{
    SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_SAVEAS | WB_3DLOOK );

    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sog" ) );
    pFileDlg->AddFilter( aStrFilterType, aStrFilterType );
    INetURLObject aFile( SvtPathOptions().GetPalettePath(), INET_PROT_FILE );

    if( pGradientList->GetName().Len() )
    {
        aFile.Append( pGradientList->GetName() );

        if( !aFile.getExtension().Len() )
            aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "sog" ) ) );
    }

    pFileDlg->SetPath( aFile.PathToFileName() );

    if( pFileDlg->Execute() == RET_OK )
    {

        INetURLObject   aURL( pFileDlg->GetPath(), INET_PROT_FILE );
        INetURLObject   aPathURL( aURL ); aPathURL.removeSegment(); aPathURL.removeFinalSlash();

        pGradientList->SetName( aURL.getName() );
        pGradientList->SetPath( aPathURL.PathToFileName() );

        if( pGradientList->Save() )
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

            aGrpGradients.SetText( aString );

            // Flag fuer gespeichert setzen
            *pnGradientListState |= CT_SAVED;
            // Flag fuer modifiziert entfernen
            *pnGradientListState &= ~CT_MODIFIED;
        }
        else
        {
            ErrorBox( DLGWIN, WinBits( WB_OK ),
                String( SVX_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
        }
    }
    delete pFileDlg;

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGradientTabPage, ChangeGradientHdl_Impl, void *, EMPTYARG )
{
    XGradient* pGradient = NULL;
    int nPos = aLbGradients.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pGradient = new XGradient( ( (XGradientEntry*) pGradientList->Get( nPos ) )->GetGradient() );
    else
    {
        const SfxPoolItem* pPoolItem = NULL;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), TRUE, &pPoolItem ) )
        {
            if( ( XFILL_GRADIENT == (XFillStyle) ( ( const XFillStyleItem* ) pPoolItem )->GetValue() ) &&
                ( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), TRUE, &pPoolItem ) ) )
            {
                pGradient = new XGradient( ( ( const XFillGradientItem* ) pPoolItem )->GetValue() );
            }
        }
        if( !pGradient )
        {
            aLbGradients.SelectEntryPos( 0 );
            nPos = aLbGradients.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pGradient = new XGradient( ( (XGradientEntry*) pGradientList->Get( nPos ) )->GetGradient() );
        }
    }

    if( pGradient )
    {
        XGradientStyle eXGS = pGradient->GetGradientStyle();

        aLbGradientType.SelectEntryPos( eXGS );
        // Wenn der EIntrag nicht in der Listbox ist, werden die Farben
        // temporaer hinzugenommen
        aLbColorFrom.SetNoSelection();
        aLbColorFrom.SelectEntry( pGradient->GetStartColor() );

        if ( aLbColorFrom.GetSelectEntryCount() == 0 )
        {
            aLbColorFrom.InsertEntry( pGradient->GetStartColor(),
                                      String() );
            aLbColorFrom.SelectEntry( pGradient->GetStartColor() );
        }
        aLbColorTo.SetNoSelection();
        aLbColorTo.SelectEntry( pGradient->GetEndColor() );

        if ( aLbColorTo.GetSelectEntryCount() == 0 )
        {
            aLbColorTo.InsertEntry( pGradient->GetEndColor(), String() );
            aLbColorTo.SelectEntry( pGradient->GetEndColor() );
        }

        aMtrAngle.SetValue( pGradient->GetAngle() / 10 ); // sollte in Resource geaendert werden
        aMtrBorder.SetValue( pGradient->GetBorder() );
        aMtrCenterX.SetValue( pGradient->GetXOffset() );
        aMtrCenterY.SetValue( pGradient->GetYOffset() );
        aMtrColorFrom.SetValue( pGradient->GetStartIntens() );
        aMtrColorTo.SetValue( pGradient->GetEndIntens() );

        // Controls Disablen/Enablen
        SetControlState_Impl( eXGS );

        // ItemSet fuellen und an XOut weiterleiten
        rXFSet.Put( XFillGradientItem( String(), *pGradient ) );
        XOut.SetFillAttr( aXFillAttr.GetItemSet() );

        aCtlPreview.Invalidate();
        delete pGradient;
    }
    return 0L;
}

//------------------------------------------------------------------------

void SvxGradientTabPage::SetControlState_Impl( XGradientStyle eXGS )
{
    switch( eXGS )
    {
        case XGRAD_LINEAR:
        case XGRAD_AXIAL:
            aFtCenterX.Disable();
            aMtrCenterX.Disable();
            aFtCenterY.Disable();
            aMtrCenterY.Disable();
            aFtAngle.Enable();
            aMtrAngle.Enable();
            break;

        case XGRAD_RADIAL:
            aFtCenterX.Enable();
            aMtrCenterX.Enable();
            aFtCenterY.Enable();
            aMtrCenterY.Enable();
            aFtAngle.Disable();
            aMtrAngle.Disable();
            break;

        case XGRAD_ELLIPTICAL:
            aFtCenterX.Enable();
            aMtrCenterX.Enable();
            aFtCenterY.Enable();
            aMtrCenterY.Enable();
            aFtAngle.Enable();
            aMtrAngle.Enable();
            break;

        case XGRAD_SQUARE:
        case XGRAD_RECT:
            aFtCenterX.Enable();
            aMtrCenterX.Enable();
            aFtCenterY.Enable();
            aMtrCenterY.Enable();
            aFtAngle.Enable();
            aMtrAngle.Enable();
            break;
    }
}


