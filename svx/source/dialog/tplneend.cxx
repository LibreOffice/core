/*************************************************************************
 *
 *  $RCSfile: tplneend.cxx,v $
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

#ifndef _SHL_HXX //autogen
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
#ifndef _SFXFILEDLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#pragma hdrstop

#define _SVX_TPLNEEND_CXX

#include "dialogs.hrc"
#include "dlgname.hrc"
#include "tabline.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST

#include "svdobj.hxx"
#include "svdopath.hxx"
#include "drawitem.hxx"
#include "xpool.hxx"
#include "xtable.hxx"
#include "tabline.hxx"
#include "dlgname.hxx"
#include "dialmgr.hxx"

#define DLGWIN this->GetParent()->GetParent()

#define BITMAP_WIDTH   32
#define BITMAP_HEIGHT  12
#define XOUT_WIDTH    150

/*************************************************************************
|*
|*  Dialog zum Definieren von Linienenden-Stilen
|*
\************************************************************************/

SvxLineEndDefTabPage::SvxLineEndDefTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_LINEEND_DEF ), rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlPreview ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() ),

    aXLStyle            ( XLINE_SOLID ),
    aXWidth             ( XOUT_WIDTH ),
    aXColor             ( String(), COL_BLACK ),

    aFiTip              ( this, ResId( FI_TIP ) ),
    aGrpTip             ( this, ResId( GRP_TIP ) ),
    aEdtName            ( this, ResId( EDT_NAME ) ),
    aLbLineEnds         ( this, ResId( LB_LINEENDS ) ),
    aGrpLineEnds        ( this, ResId( GRP_LINEENDS ) ),
    aBtnModify          ( this, ResId( BTN_MODIFY ) ),
    aBtnAdd             ( this, ResId( BTN_ADD ) ),
    aBtnDelete          ( this, ResId( BTN_DELETE ) ),
    aCtlPreview         ( this, ResId( CTL_PREVIEW ), &XOut ),
    aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
    aBtnLoad            ( this, ResId( BTN_LOAD ) ),
    aBtnSave            ( this, ResId( BTN_SAVE ) ),
    rOutAttrs           ( rInAttrs )

{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    rXLSet.Put( aXLStyle );
    rXLSet.Put( aXWidth );
    rXLSet.Put( aXColor );
    rXLSet.Put( XLineStartWidthItem( aCtlPreview.GetOutputSize().Height()  / 2 ) );
    rXLSet.Put( XLineEndWidthItem( aCtlPreview.GetOutputSize().Height() / 2 ) );
    XOut.SetLineAttr( aXLineAttr );

    aBtnAdd.SetClickHdl(
        LINK( this, SvxLineEndDefTabPage, ClickAddHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxLineEndDefTabPage, ClickModifyHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxLineEndDefTabPage, ClickDeleteHdl_Impl ) );
    aBtnLoad.SetClickHdl(
        LINK( this, SvxLineEndDefTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl(
        LINK( this, SvxLineEndDefTabPage, ClickSaveHdl_Impl ) );

    aLbLineEnds.SetSelectHdl(
        LINK( this, SvxLineEndDefTabPage, SelectLineEndHdl_Impl ) );

    pLineEndList = NULL;
    pPolyObj     = NULL;
}

//------------------------------------------------------------------------

SvxLineEndDefTabPage::~SvxLineEndDefTabPage()
{
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::Construct()
{
    aLbLineEnds.Fill( pLineEndList );
    if( !pPolyObj )
        aBtnAdd.Disable();
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        // ActivatePage() wird aufgerufen bevor der Dialog PageCreated() erhaelt !!!
        if( pLineEndList )
        {
            if( *pPageType == 1 && *pPosLineEndLb >= 0 &&
                *pPosLineEndLb != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbLineEnds.SelectEntryPos( *pPosLineEndLb );
                SelectLineEndHdl_Impl( this );
            }
            // Ermitteln (evtl. abschneiden) des Namens und in
            // der GroupBox darstellen
            String          aString( SVX_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL; aURL.SetSmartURL( pLineEndList->GetName() );

            if ( aURL.getBase().Len() > 18 )
            {
                aString += aURL.getBase().Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += aURL.getBase();

            aGrpLineEnds.SetText( aString );

            *pPageType = 0; // 3
            *pPosLineEndLb = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}

// -----------------------------------------------------------------------

int SvxLineEndDefTabPage::DeactivatePage( SfxItemSet* pSet )
{
    CheckChanges_Impl();

    FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::CheckChanges_Impl()
{
    USHORT nPos = aLbLineEnds.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        String aString = aEdtName.GetText();

        if( aString != aLbLineEnds.GetSelectEntry() )
        {
            QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
//!             SVX_RES( RID_SVXSTR_LINEEND ),
                String( SVX_RES( RID_SVXSTR_ASK_CHANGE_LINEEND ) ) );

            if ( aQueryBox.Execute() == RET_YES )
                ClickModifyHdl_Impl( this );
        }
    }
    nPos = aLbLineEnds.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        *pPosLineEndLb = nPos;
}

// -----------------------------------------------------------------------

BOOL SvxLineEndDefTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if( *pDlgType == 0 ) // Linien-Dialog
    {
        if( *pPageType == 3 )
        {
            CheckChanges_Impl();

            long nPos = aLbLineEnds.GetSelectEntryPos();
            XLineEndEntry* pEntry = pLineEndList->Get( nPos );

            rOutAttrs.Put( XLineStartItem( pEntry->GetName(), pEntry->GetLineEnd() ) );
            rOutAttrs.Put( XLineEndItem( pEntry->GetName(), pEntry->GetLineEnd() ) );
        }
    }
    return( TRUE );
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::Reset( const SfxItemSet& rOutAttrs )
{
    aLbLineEnds.SelectEntryPos( 0 );

    // Update lineend
    if( pLineEndList->Count() > 0 )
    {
        int nPos = aLbLineEnds.GetSelectEntryPos();

        XLineEndEntry* pEntry = pLineEndList->Get( nPos );

        aEdtName.SetText( aLbLineEnds.GetSelectEntry() );

        rXLSet.Put( XLineStartItem( String(), pEntry->GetLineEnd() ) );
        rXLSet.Put( XLineEndItem( String(), pEntry->GetLineEnd() ) );
        XOut.SetLineAttr( aXLineAttr );

        aCtlPreview.Invalidate();
    }

    // Status der Buttons ermitteln
    if( pLineEndList->Count() )
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

SfxTabPage* SvxLineEndDefTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxLineEndDefTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, SelectLineEndHdl_Impl, void *, EMPTYARG )
{
    if( pLineEndList->Count() > 0 )
    {
        int nPos = aLbLineEnds.GetSelectEntryPos();

        XLineEndEntry* pEntry = pLineEndList->Get( nPos );

        aEdtName.SetText( aLbLineEnds.GetSelectEntry() );

        rXLSet.Put( XLineStartItem( String(), pEntry->GetLineEnd() ) );
        rXLSet.Put( XLineEndItem( String(), pEntry->GetLineEnd() ) );
        XOut.SetLineAttr( aXLineAttr );

        aCtlPreview.Invalidate();

        // Wird erst hier gesetzt, um den Style nur dann zu uebernehmen,
        // wenn in der ListBox ein Eintrag ausgewaehlt wurde
        *pPageType = 3;
    }
    return( 0L );
}

//------------------------------------------------------------------------

long SvxLineEndDefTabPage::ChangePreviewHdl_Impl( void* )
{
    aCtlPreview.Invalidate();
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbLineEnds.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr* pMgr = DIALOG_MGR();
        String aDesc( ResId( RID_SVXSTR_DESC_LINEEND, pMgr ) );
        String aName( aEdtName.GetText() );
        long nCount = pLineEndList->Count();
        BOOL bDifferent = TRUE;

        // Pruefen, ob Name schon vorhanden ist
        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pLineEndList->Get( i )->GetName() )
                bDifferent = FALSE;

        // Wenn ja, wird wiederholt ein neuer Name angefordert
        if ( !bDifferent )
        {
            WarningBox aWarningBox( DLGWIN, WinBits( WB_OK ),
                String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, pMgr ) ) );
            aWarningBox.Execute();

            SvxNameDialog* pDlg = new SvxNameDialog( DLGWIN, aName, aDesc );
            BOOL bLoop = TRUE;

            while( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );
                bDifferent = TRUE;

                for( long i = 0; i < nCount && bDifferent; i++ )
                {
                    if( aName == pLineEndList->Get( i )->GetName() )
                        bDifferent = FALSE;
                }

                if( bDifferent )
                    bLoop = FALSE;
                else
                    aWarningBox.Execute();
            }
            delete( pDlg );
        }

        // Wenn nicht vorhanden, wird Eintrag aufgenommen
        if( bDifferent )
        {
            XLineEndEntry* pEntry = pLineEndList->Get( nPos );

            pEntry->SetName( aName );
            aEdtName.SetText( aName );

            aLbLineEnds.Modify( pEntry, nPos, pLineEndList->GetBitmap( nPos ) );
            aLbLineEnds.SelectEntryPos( nPos );

            // Flag fuer modifiziert setzen
            *pnLineEndListState |= CT_MODIFIED;

            *pPageType = 3;
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
    if( pPolyObj )
    {
        const SdrObject* pNewObj;
        SdrObject* pConvPolyObj = NULL;
        UINT16 nId = pPolyObj->GetObjIdentifier();

        if( pPolyObj->ISA( SdrPathObj ) )
        {
            pNewObj = pPolyObj;
        }
        else
        {
            SdrObjTransformInfoRec aInfoRec;
            pPolyObj->TakeObjInfo( aInfoRec );

            if( aInfoRec.bCanConvToPath )
            {
                pNewObj = pConvPolyObj = pPolyObj->ConvertToPolyObj( TRUE, FALSE );

                if( !pNewObj || !pNewObj->ISA( SdrPathObj ) )
                    return( 0L ); // Abbruch, zusaetzliche Sicherheit, die bei
                            // Gruppenobjekten aber nichts bringt.
            }
            else return( 0L ); // Abbruch
        }

        const XPolygon& rXPoly = ( (SdrPathObj*) pNewObj )->GetPathPoly().GetObject( 0 );
        XPolygon aXPoly( rXPoly );

        // Normalisieren
        Point aPoint( aXPoly.GetBoundRect().TopLeft() );
        aPoint.X() *= -1;
        aPoint.Y() *= -1;
        aXPoly.Translate( aPoint );

        // Loeschen des angelegten PolyObjektes
        if( pConvPolyObj )
            delete pConvPolyObj;

        XLineEndEntry* pEntry;

        ResMgr* pMgr = DIALOG_MGR();
        String aNewName( ResId( RID_SVXSTR_LINEEND, pMgr ) );
        String aDesc( ResId( RID_SVXSTR_DESC_LINEEND, pMgr ) );
        String aName;

        long nCount = pLineEndList->Count();
        long j = 1;
        BOOL bDifferent = FALSE;

        while ( !bDifferent )
        {
            aName = aNewName;
            aName += sal_Unicode(' ');
            aName += UniString::CreateFromInt32( j++ );
            bDifferent = TRUE;

            for( long i = 0; i < nCount && bDifferent; i++ )
                if ( aName == pLineEndList->Get( i )->GetName() )
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
                if( aName == pLineEndList->Get( i )->GetName() )
                    bDifferent = FALSE;
            }

            if( bDifferent )
            {
                bLoop = FALSE;
                pEntry = new XLineEndEntry( aXPoly, aName );

                long nCount = pLineEndList->Count();
                pLineEndList->Insert( pEntry, nCount );
                Bitmap* pBitmap = pLineEndList->GetBitmap( nCount );

                // Zur ListBox hinzufuegen
                aLbLineEnds.Append( pEntry, pBitmap );
                aLbLineEnds.SelectEntryPos( aLbLineEnds.GetEntryCount() - 1 );

                // Flag fuer modifiziert setzen
                *pnLineEndListState |= CT_MODIFIED;

                SelectLineEndHdl_Impl( this );
            }
            else
                WarningBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, pMgr ) ) ).Execute();
        }
        delete pDlg;
    }
    else
        aBtnAdd.Disable();

    // Status der Buttons ermitteln
    if ( pLineEndList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbLineEnds.GetSelectEntryPos();

    if( nPos >= 0 )
    {
        QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
//!         SVX_RES( RID_SVXSTR_DEL_LINEEND ),
            String( SVX_RES( RID_SVXSTR_ASK_DEL_LINEEND ) ) );

        if ( aQueryBox.Execute() == RET_YES )
        {
            delete pLineEndList->Remove( nPos );
            aLbLineEnds.RemoveEntry( nPos );
            aLbLineEnds.SelectEntryPos( 0 );

            SelectLineEndHdl_Impl( this );
            *pPageType = 0; // LineEnd soll nicht uebernommen werden

            // Flag fuer modifiziert setzen
            *pnLineEndListState |= CT_MODIFIED;

            ChangePreviewHdl_Impl( this );
        }
    }
    // Status der Buttons ermitteln
    if( !pLineEndList->Count() )
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickLoadHdl_Impl, void *, EMPTYARG )
{
    ResMgr* pMgr = DIALOG_MGR();
    USHORT nReturn = RET_YES;

    if ( *pnLineEndListState & CT_MODIFIED )
    {
        nReturn = WarningBox( DLGWIN, WinBits( WB_YES_NO_CANCEL ),
            String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, pMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pLineEndList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_OPEN | WB_3DLOOK );

        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soe" ) );
        pFileDlg->AddFilter( aStrFilterType, aStrFilterType );

        String aFile( SFX_APP()->GetAppIniManager()->Get( SFX_KEY_PALETTE_PATH ) );
        pFileDlg->SetPath( aFile );

        if( pFileDlg->Execute() == RET_OK )
        {
            INetURLObject aURL; aURL.SetSmartURL( pFileDlg->GetPath() );
            INetURLObject aPathURL( aURL ); aPathURL.removeSegment(); aPathURL.removeFinalSlash();

            // Liste speichern
            XLineEndList* pLeList = new XLineEndList( aPathURL.PathToFileName(), pXPool );
            pLeList->SetName( aURL.getName() );
            if( pLeList->Load() )
            {
                if( pLeList )
                {
                    // Pruefen, ob Tabelle geloescht werden darf:
                    if( pLineEndList != ( (SvxLineTabDialog*) DLGWIN )->GetLineEndList() )
                        delete pLineEndList;

                    pLineEndList = pLeList;
                    ( (SvxLineTabDialog*) DLGWIN )->SetNewLineEndList( pLineEndList );
                    aLbLineEnds.Clear();
                    aLbLineEnds.Fill( pLineEndList );
                    Reset( rOutAttrs );

                    pLineEndList->SetName( aURL.getName() );

                    // Ermitteln (evtl. abschneiden) des Namens und in
                    // der GroupBox darstellen
                    String aString( ResId( RID_SVXSTR_TABLE, pMgr ) );
                    aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                    if( aURL.getBase().Len() > 18 )
                    {
                        aString += aURL.getBase().Copy( 0, 15 );
                        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
                    }
                    else
                        aString += aURL.getBase();

                    aGrpLineEnds.SetText( aString );

                    // Flag fuer gewechselt setzen
                    *pnLineEndListState |= CT_CHANGED;
                    // Flag fuer modifiziert entfernen
                    *pnLineEndListState &= ~CT_MODIFIED;
                }
            }
            else
                ErrorBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, pMgr ) ) ).Execute();
        }
        delete( pFileDlg );
    }

    // Status der Buttons ermitteln
    if ( pLineEndList->Count() )
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

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickSaveHdl_Impl, void *, EMPTYARG )
{
    SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_SAVEAS | WB_3DLOOK );

    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soe" ) );
    pFileDlg->AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile; aFile.SetSmartURL( SFX_APP()->GetAppIniManager()->Get( SFX_KEY_PALETTE_PATH ) );

    if( pLineEndList->GetName().Len() )
    {
        aFile.Append( pLineEndList->GetName() );

        if( !aFile.getExtension().Len() )
            aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "soe" ) ) );
    }

    pFileDlg->SetPath( aFile.PathToFileName() );

    if( pFileDlg->Execute() == RET_OK )
    {
        INetURLObject   aURL; aURL.SetSmartURL( pFileDlg->GetPath() );
        INetURLObject   aPathURL( aURL ); aPathURL.removeSegment(); aPathURL.removeFinalSlash();

        pLineEndList->SetName( aURL.getName() );
        pLineEndList->SetPath( aPathURL.PathToFileName() );

        if( pLineEndList->Save() )
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
            aGrpLineEnds.SetText( aString );

            // Flag fuer gespeichert setzen
            *pnLineEndListState |= CT_SAVED;
            // Flag fuer modifiziert entfernen
            *pnLineEndListState &= ~CT_MODIFIED;
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


