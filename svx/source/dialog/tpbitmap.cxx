/*************************************************************************
 *
 *  $RCSfile: tpbitmap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 14:54:00 $
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

//svdraw.hxx
#define _SVDRAW_HXX
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
#define _SDR_NOVIEWS
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#pragma hdrstop

#define _SVX_TPBITMAP_CXX
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST

#include "dialogs.hrc"

#include "xattr.hxx"
#include "xpool.hxx"
#include "xtable.hxx"
#include "xoutbmp.hxx"
#include "drawitem.hxx"
#include "tabarea.hxx"
#include "tabarea.hrc"
#include "dlgname.hxx"
#include "dlgname.hrc"
#include "dialmgr.hxx"
#include "impgrf.hxx"

#define DLGWIN this->GetParent()->GetParent()

/*************************************************************************
|*
|*  Dialog zum Aendern und Definieren der Bitmaps
|*
\************************************************************************/

SvxBitmapTabPage::SvxBitmapTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SvxTabPage          ( pParent, SVX_RES( RID_SVXPAGE_BITMAP ), rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlPreview ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() ),
    aXFStyleItem        ( XFILL_BITMAP ),
    aXBitmapItem        ( String(), XOBitmap() ),

    aCtlPixel           ( this, ResId( CTL_PIXEL ) ),
    aGrpPixelEdit       ( this, ResId( GRP_PIXEL_EDIT ) ),
    aFtColor            ( this, ResId( FT_COLOR ) ),
    aLbColor            ( this, ResId( LB_COLOR ) ),
    aFtBackgroundColor  ( this, ResId( FT_BACKGROUND_COLOR ) ),
    aLbBackgroundColor  ( this, ResId( LB_BACKGROUND_COLOR ) ),
    aGrpColor           ( this, ResId( GRP_COLOR ) ),
    aLbBitmaps          ( this, ResId( LB_BITMAPS ) ),
    aGrpBitmaps         ( this, ResId( GRP_BITMAPS ) ),
    aCtlPreview         ( this, ResId( CTL_PREVIEW ), &XOut ),
    aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
    aBtnAdd             ( this, ResId( BTN_ADD ) ),
    aBtnImport          ( this, ResId( BTN_IMPORT ) ),
    aBtnModify          ( this, ResId( BTN_MODIFY ) ),
    aBtnDelete          ( this, ResId( BTN_DELETE ) ),
    aBtnLoad            ( this, ResId( BTN_LOAD ) ),
    aBtnSave            ( this, ResId( BTN_SAVE ) ),
    aBitmapCtl          ( this, aCtlPreview.GetSizePixel() ),
    rOutAttrs           ( rInAttrs )

{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Setzen des Output-Devices
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXBitmapItem );
    //XOut.SetFillAttr( aXFillAttr );

    // Setzen der Linie auf None im OutputDevice
    XLineAttrSetItem aXLineAttr( pXPool );
    aXLineAttr.GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
    XOut.SetLineAttr( aXLineAttr.GetItemSet() );

    aBtnAdd.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickAddHdl_Impl ) );
    aBtnImport.SetClickHdl(
        LINK( this, SvxBitmapTabPage, ClickImportHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxBitmapTabPage, ClickModifyHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxBitmapTabPage, ClickDeleteHdl_Impl ) );
    aBtnLoad.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickSaveHdl_Impl ) );

    aLbBitmaps.SetSelectHdl(
        LINK( this, SvxBitmapTabPage, ChangeBitmapHdl_Impl ) );
    aLbColor.SetSelectHdl(
        LINK( this, SvxBitmapTabPage, ChangePixelColorHdl_Impl ) );
    aLbBackgroundColor.SetSelectHdl(
        LINK( this, SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl ) );

    pColorTab = NULL;
    pBitmapList = NULL;
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::Construct()
{
    // Farbtabellen
    aLbColor.Fill( pColorTab );
    aLbBackgroundColor.CopyEntries( aLbColor );

    // Bitmaptabelle
    aLbBitmaps.Fill( pBitmapList );
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::ActivatePage( const SfxItemSet& rSet )
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

                // LbColor
                nPos = aLbColor.GetSelectEntryPos();
                aLbColor.Clear();
                aLbColor.Fill( pColorTab );
                nCount = aLbColor.GetEntryCount();
                if( nCount == 0 )
                    ; // Dieser Fall sollte nicht auftreten
                else if( nCount <= nPos )
                    aLbColor.SelectEntryPos( 0 );
                else
                    aLbColor.SelectEntryPos( nPos );

                // LbColorBackground
                nPos = aLbBackgroundColor.GetSelectEntryPos();
                aLbBackgroundColor.Clear();
                aLbBackgroundColor.CopyEntries( aLbColor );
                nCount = aLbBackgroundColor.GetEntryCount();
                if( nCount == 0 )
                    ; // Dieser Fall sollte nicht auftreten
                else if( nCount <= nPos )
                    aLbBackgroundColor.SelectEntryPos( 0 );
                else
                    aLbBackgroundColor.SelectEntryPos( nPos );

                ChangePixelColorHdl_Impl( this );
                ChangeBackgrndColorHdl_Impl( this );
            }

            // Ermitteln (evtl. abschneiden) des Namens und in
            // der GroupBox darstellen
            String          aString( SVX_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL( pBitmapList->GetPath() );

            aURL.Append( pBitmapList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if( aURL.getBase().Len() > 18 )
            {
                aString += aURL.getBase().Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += aURL.getBase();

            aGrpBitmaps.SetText( aString );

            if( *pPageType == PT_BITMAP && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbBitmaps.SelectEntryPos( *pPos );
            }
            // Farben koennten geloescht worden sein
            ChangeBitmapHdl_Impl( this );

            *pPageType = PT_BITMAP;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}

// -----------------------------------------------------------------------

int SvxBitmapTabPage::DeactivatePage( SfxItemSet* pSet)
{
    if ( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    FillItemSet( *pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

BOOL SvxBitmapTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if( *pDlgType == 0 && *pbAreaTP == FALSE ) // Flaechen-Dialog
    {
        if( *pPageType == PT_BITMAP )
        {
            // CheckChanges_Impl(); <-- doppelte Abfrage ?

            XOBitmap aXOBitmap;
            String aString;
            USHORT nPos = aLbBitmaps.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aXOBitmap = pBitmapList->Get( nPos )->GetXBitmap();
                aString = aLbBitmaps.GetSelectEntry();

            }
            else
            {
                aXOBitmap = aBitmapCtl.GetXBitmap();
            }
            rOutAttrs.Put( XFillStyleItem( XFILL_BITMAP ) );
            rOutAttrs.Put( XFillBitmapItem( aString, aXOBitmap ) );
        }
    }
    return TRUE;
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::Reset( const SfxItemSet& rOutAttrs )
{
    // aLbBitmaps.SelectEntryPos( 0 );

    aBitmapCtl.SetLines( aCtlPixel.GetLineCount() );
    aBitmapCtl.SetPixelColor( aLbColor.GetSelectEntryColor() );
    aBitmapCtl.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );
    aBitmapCtl.SetBmpArray( aCtlPixel.GetPixelPtr() );

    // Bitmap holen und darstellen
    XFillBitmapItem aBmpItem( (const String &) String(), aBitmapCtl.GetXBitmap() );
    rXFSet.Put( aBmpItem );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();

    ChangeBitmapHdl_Impl( this );

    // Status der Buttons ermitteln
    if( pBitmapList->Count() )
    {
        aBtnAdd.Enable();
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

SfxTabPage* SvxBitmapTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return new SvxBitmapTabPage( pWindow, rOutAttrs );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ChangeBitmapHdl_Impl, void *, EMPTYARG )
{
    XOBitmap* pXOBitmap = NULL;
    int nPos = aLbBitmaps.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pXOBitmap = new XOBitmap( ( (XBitmapEntry*) pBitmapList->Get( nPos ) )->GetXBitmap() );
    else
    {
        const SfxPoolItem* pPoolItem = NULL;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), TRUE, &pPoolItem ) )
        {
            XFillStyle eXFS = (XFillStyle) ( ( const XFillStyleItem* ) pPoolItem )->GetValue();
            if( ( XFILL_BITMAP == eXFS ) &&
                ( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), TRUE, &pPoolItem ) ) )
            {
                pXOBitmap = new XOBitmap( ( ( const XFillBitmapItem* ) pPoolItem )->GetValue() );
            }
        }
        if( !pXOBitmap )
        {
            aLbBitmaps.SelectEntryPos( 0 );
            nPos = aLbBitmaps.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pXOBitmap = new XOBitmap( ( (XBitmapEntry*) pBitmapList->Get( nPos ) )->GetXBitmap() );
        }
    }
    if( pXOBitmap )
    {
        //WorkWindow        aTmpWW( DLGWIN );
        //VirtualDevice aVD( aTmpWW );
        //USHORT    nLines = aCtlPixel.GetLineCount();
        //Color aPixelColor, aBackColor;
        //BOOL  bPixelColor = FALSE;
        //USHORT    nWidth  = pBitmap->GetSizePixel().Width();
        //USHORT    nHeight = pBitmap->GetSizePixel().Height();

        aLbColor.SetNoSelection();
        aLbBackgroundColor.SetNoSelection();

        if( pXOBitmap->GetBitmapType() == XBITMAP_IMPORT )
        {
            aCtlPixel.Reset();
            aCtlPixel.SetPaintable( FALSE );
            aCtlPixel.Disable();
            aGrpPixelEdit.Disable();
            aFtColor.Disable();
            aLbColor.Disable();
            aGrpColor.Disable();
            aFtBackgroundColor.Disable();
            aLbBackgroundColor.Disable();
            aBtnModify.Disable();
            aBtnAdd.Disable();
        }
        else if( pXOBitmap->GetBitmapType() == XBITMAP_8X8 )
        {
            aCtlPixel.SetPaintable( TRUE );
            aCtlPixel.Enable();
            aGrpPixelEdit.Enable();
            aFtColor.Enable();
            aLbColor.Enable();
            aGrpColor.Enable();
            aFtBackgroundColor.Enable();
            aLbBackgroundColor.Enable();
            aBtnModify.Enable();
            aBtnAdd.Enable();

            // Setzen des PixelControls
            aCtlPixel.SetXBitmap( *pXOBitmap );

            Color aPixelColor = pXOBitmap->GetPixelColor();
            Color aBackColor  = pXOBitmap->GetBackgroundColor();

            aBitmapCtl.SetPixelColor( aPixelColor );
            aBitmapCtl.SetBackgroundColor( aBackColor );

            // Wenn der Eintrag nicht in der Listbox ist, wird die Farbe
            // temporaer hinzugenommen
            if( 0 == aLbBitmaps.GetSelectEntryPos() )
            {
                aLbColor.SelectEntry( Color( COL_BLACK ) );
                ChangePixelColorHdl_Impl( this );
            }
            else
                aLbColor.SelectEntry( aPixelColor );
            if( aLbColor.GetSelectEntryCount() == 0 )
            {
                aLbColor.InsertEntry( aPixelColor, String() );
                aLbColor.SelectEntry( aPixelColor );
            }
            aLbBackgroundColor.SelectEntry( aBackColor );
            if( aLbBackgroundColor.GetSelectEntryCount() == 0 )
            {
                aLbBackgroundColor.InsertEntry( aBackColor, String() );
                aLbBackgroundColor.SelectEntry( aBackColor );
            }
        }
        aCtlPixel.Invalidate();

        // Bitmap darstellen
        XFillBitmapItem aXBmpItem( (const String &) String(), *pXOBitmap );
        rXFSet.Put( aXBmpItem );

        XOut.SetFillAttr( aXFillAttr.GetItemSet() );
        aCtlPreview.Invalidate();

        bBmpChanged = FALSE;
        delete pXOBitmap;
    }
    return 0L;
}

// -----------------------------------------------------------------------

long SvxBitmapTabPage::CheckChanges_Impl()
{
    USHORT nPos = aLbBitmaps.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        String aString = aLbBitmaps.GetSelectEntry();

        if( bBmpChanged )
        {
            ResMgr* pMgr = DIALOG_MGR();
            SvxMessDialog aMessDlg( DLGWIN,
                String( ResId( RID_SVXSTR_BITMAP, pMgr ) ),
                String( ResId( RID_SVXSTR_ASK_CHANGE_BITMAP, pMgr ) ),
                &(WarningBox::GetStandardImage()) );

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
                }
                break;

                case RET_BTN_2: // Hinzufuegen
                {
                    ClickAddHdl_Impl( this );
                    nPos = aLbBitmaps.GetSelectEntryPos();
                }
                break;

                case RET_CANCEL:
                    // return( -1L );
                break;
                // return( TRUE ); // Abbruch
            }
        }
    }
    nPos = aLbBitmaps.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        *pPos = nPos;
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
    ResMgr* pMgr = DIALOG_MGR();
    String aNewName( ResId( RID_SVXSTR_BITMAP, pMgr ) );
    String aDesc( ResId( RID_SVXSTR_DESC_NEW_BITMAP, pMgr ) );
    String aName;

    long nCount = pBitmapList->Count();
    long j = 1;
    BOOL bDifferent = FALSE;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += sal_Unicode(' ');
        aName += UniString::CreateFromInt32( j++ );
        bDifferent = TRUE;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pBitmapList->Get( i )->GetName() )
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
            if( aName == pBitmapList->Get( i )->GetName() )
                bDifferent = FALSE;

        if( bDifferent ) {
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
        XBitmapEntry* pEntry;
        if( aCtlPixel.IsEnabled() )
            pEntry = new XBitmapEntry( aBitmapCtl.GetXBitmap(), aName );
        else // Es muss sich um eine nicht vorhandene importierte Bitmap handeln
        {
            const SfxPoolItem* pPoolItem = NULL;
            if( SFX_ITEM_SET == rOutAttrs.GetItemState( XATTR_FILLBITMAP, TRUE, &pPoolItem ) )
            {
                XOBitmap aXOBitmap( ( ( const XFillBitmapItem* ) pPoolItem )->GetValue() );
                pEntry = new XBitmapEntry( aXOBitmap, aName );
            }
            else { DBG_ERROR( "pEntry is undfined -> GPF" ) }
        }
        pBitmapList->Insert( pEntry );

        aLbBitmaps.Append( pEntry );
        aLbBitmaps.SelectEntryPos( aLbBitmaps.GetEntryCount() - 1 );

#ifdef WNT
        // hack: #31355# W.P.
        Rectangle aRect( aLbBitmaps.GetPosPixel(), aLbBitmaps.GetSizePixel() );
        if( TRUE ) {                // ??? overlapped with pDlg
                                    // and srolling
            Invalidate( aRect );
            //aLbBitmaps.Invalidate();
        }
#endif

        // Flag fuer modifiziert setzen
        *pnBitmapListState |= CT_MODIFIED;

        ChangeBitmapHdl_Impl( this );
    }

    // Status der Buttons ermitteln
    if( pBitmapList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return 0L;
}


/******************************************************************************/
/******************************************************************************/


//------------------------------------------------------------------------

#ifdef WNT
#pragma optimize ( "", off )
#endif

IMPL_LINK( SvxBitmapTabPage, ClickImportHdl_Impl, void *, EMPTYARG )
{
    ResMgr* pMgr = DIALOG_MGR();
    SvxImportGraphicDialog* pDlg = new SvxImportGraphicDialog( DLGWIN,
        UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Import" ) ), ENABLE_STANDARD );

    if( pDlg->Execute() == RET_OK )
    {
        Graphic         aGraphic;
        GraphicFilter&  rFilter = pDlg->GetFilter();
        INetURLObject   aURL( pDlg->GetPath() );
        SvStream*       pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL(), STREAM_READ | STREAM_SHARE_DENYNONE );
        USHORT          nError = 1;

        if( pIStm )
        {
            EnterWait();
            nError = rFilter.ImportGraphic( aGraphic, aURL.GetMainURL(), *pIStm );
            LeaveWait();
            delete pIStm;
        }

        if( !nError )
        {
            String aName( aURL.getName() );
            String aDesc( ResId(RID_SVXSTR_DESC_EXT_BITMAP, pMgr) );
            WarningBox*    pWarnBox = NULL;
            SvxNameDialog* pDlg =
                new SvxNameDialog( DLGWIN, aName, aDesc );
            nError = RID_SVXSTR_WARN_NAME_DUPLICATE;

            while( pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );

                BOOL bDifferent = TRUE;
                long nCount     = pBitmapList->Count();

                for( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == pBitmapList->Get( i )->GetName() )
                        bDifferent = FALSE;

                if( bDifferent ) {
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
                Bitmap aBmp( aGraphic.GetBitmap() );
                XBitmapEntry* pEntry =
                    new XBitmapEntry( XOBitmap( aBmp ), aName );
                pBitmapList->Insert( pEntry );

                aLbBitmaps.Append( pEntry );
                aLbBitmaps.SelectEntryPos( aLbBitmaps.GetEntryCount() - 1 );

#ifdef WNT
                // hack: #31355# W.P.
                Rectangle aRect( aLbBitmaps.GetPosPixel(), aLbBitmaps.GetSizePixel() );
                if( TRUE ) {                // ??? overlapped with pDlg
                                            // and srolling
                    Invalidate( aRect );
                    //aLbBitmaps.Invalidate();
                }
#endif

                // Flag fuer modifiziert setzen
                *pnBitmapListState |= CT_MODIFIED;

                ChangeBitmapHdl_Impl( this );
            }
        }
        else
            // Graphik konnte nicht geladen werden
            ErrorBox( DLGWIN,
                      WinBits( WB_OK ),
                      String( ResId( RID_SVXSTR_READ_DATA_ERROR, pMgr ) ) ).Execute();
    }
    delete pDlg;

    return 0L;
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbBitmaps.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr* pMgr = DIALOG_MGR();
        String aNewName( ResId( RID_SVXSTR_BITMAP, pMgr ) );
        String aDesc( ResId( RID_SVXSTR_DESC_NEW_BITMAP, pMgr ) );
        String aName( pBitmapList->Get( nPos )->GetName() );
        String aOldName = aName;

        SvxNameDialog* pDlg = new SvxNameDialog( DLGWIN, aName, aDesc );

        long nCount = pBitmapList->Count();
        BOOL bDifferent = FALSE;
        BOOL bLoop = TRUE;

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = TRUE;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pBitmapList->Get( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = FALSE;
            }

            if( bDifferent )
            {
                bLoop = FALSE;
                XBitmapEntry* pEntry = pBitmapList->Get( nPos );

                pEntry->SetName( aName );
                pEntry->SetXBitmap( aBitmapCtl.GetXBitmap() );

                aLbBitmaps.Modify( pEntry, nPos );
                aLbBitmaps.SelectEntryPos( nPos );

                // Flag fuer modifiziert setzen
                *pnBitmapListState |= CT_MODIFIED;

                bBmpChanged = FALSE;
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

IMPL_LINK( SvxBitmapTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbBitmaps.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
//!         SVX_RES( RID_SVXSTR_DEL_BITMAP ),
            String( SVX_RES( RID_SVXSTR_ASK_DEL_BITMAP ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            delete pBitmapList->Remove( nPos );
            aLbBitmaps.RemoveEntry( nPos );
            aLbBitmaps.SelectEntryPos( 0 );

            aCtlPreview.Invalidate();
            aCtlPixel.Invalidate();

            ChangeBitmapHdl_Impl( this );

            // Flag fuer modifiziert setzen
            *pnBitmapListState |= CT_MODIFIED;
        }
    }
    // Status der Buttons ermitteln
    if( !pBitmapList->Count() )
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ClickLoadHdl_Impl, void *, p )
{
    USHORT nReturn = RET_YES;
    ResMgr* pMgr = DIALOG_MGR();

    if ( *pnBitmapListState & CT_MODIFIED )
    {
        nReturn = WarningBox( DLGWIN, WinBits( WB_YES_NO_CANCEL ),
            String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, pMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pBitmapList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_OPEN | WB_3DLOOK );

        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sob" ) );
        pFileDlg->AddFilter( aStrFilterType, aStrFilterType );

        String aFile( SvtPathOptions().GetPalettePath() );
        pFileDlg->SetPath( aFile );

        if( pFileDlg->Execute() == RET_OK )
        {
            EnterWait();

            INetURLObject aURL( pFileDlg->GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            // Tabelle speichern
            XBitmapList* pBmpList = new XBitmapList( aPathURL.GetMainURL(), pXPool );
            pBmpList->SetName( aURL.getName() );
            if( pBmpList->Load() )
            {
                if( pBmpList )
                {
                    // Pruefen, ob Tabelle geloescht werden darf:
                    if( pBitmapList != ( (SvxAreaTabDialog*) DLGWIN )->GetBitmapList() )
                        delete pBitmapList;

                    pBitmapList = pBmpList;
                    ( (SvxAreaTabDialog*) DLGWIN )->SetNewBitmapList( pBitmapList );

                    aLbBitmaps.Clear();
                    aLbBitmaps.Fill( pBitmapList );
                    Reset( rOutAttrs );

                    pBitmapList->SetName( aURL.getName() );

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

                    aGrpBitmaps.SetText( aString );

                    // Flag fuer gewechselt setzen
                    *pnBitmapListState |= CT_CHANGED;
                    // Flag fuer modifiziert entfernen
                    *pnBitmapListState &= ~CT_MODIFIED;
                }
                LeaveWait();
            }
            else
            {
                LeaveWait();
                ErrorBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, pMgr ) ) ).Execute();
            }
        }

        delete pFileDlg;
    }

    // Status der Buttons ermitteln
    if( pBitmapList->Count() )
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

IMPL_LINK( SvxBitmapTabPage, ClickSaveHdl_Impl, void *, p )
{
    SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_SAVEAS | WB_3DLOOK );

    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sob" ) );
    pFileDlg->AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pBitmapList->GetName().Len() )
    {
        aFile.Append( pBitmapList->GetName() );

        if( !aFile.getExtension().Len() )
            aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "sob" ) ) );
    }

    pFileDlg->SetPath( aFile.GetMainURL() );

    if( pFileDlg->Execute() == RET_OK )
    {
        INetURLObject   aURL( pFileDlg->GetPath() );
        INetURLObject   aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pBitmapList->SetName( aURL.getName() );
        pBitmapList->SetPath( aPathURL.GetMainURL() );

        if( pBitmapList->Save() )
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

            aGrpBitmaps.SetText( aString );

            // Flag fuer gespeichert setzen
            *pnBitmapListState |= CT_SAVED;
            // Flag fuer modifiziert entfernen
            *pnBitmapListState &= ~CT_MODIFIED;
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

IMPL_LINK( SvxBitmapTabPage, ChangePixelColorHdl_Impl, void *, EMPTYARG )
{
    aCtlPixel.SetPixelColor( aLbColor.GetSelectEntryColor() );
    aCtlPixel.Invalidate();

    aBitmapCtl.SetPixelColor( aLbColor.GetSelectEntryColor() );

    // Bitmap holen und darstellen
    rXFSet.Put( XFillBitmapItem( String(), aBitmapCtl.GetXBitmap() ) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();

    bBmpChanged = TRUE;

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl, void *, EMPTYARG )
{
    aCtlPixel.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );
    aCtlPixel.Invalidate();

    aBitmapCtl.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );

    // Bitmap holen und darstellen
    rXFSet.Put( XFillBitmapItem( String(), aBitmapCtl.GetXBitmap() ) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();

    bBmpChanged = TRUE;

    return 0L;
}

//------------------------------------------------------------------------

void SvxBitmapTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    if( pWindow == &aCtlPixel )
    {
        aBitmapCtl.SetBmpArray( aCtlPixel.GetPixelPtr() );

        // Bitmap holen und darstellen
        rXFSet.Put( XFillBitmapItem( String(), aBitmapCtl.GetXBitmap() ) );
        XOut.SetFillAttr( aXFillAttr.GetItemSet() );
        aCtlPreview.Invalidate();

        bBmpChanged = TRUE;
    }
}

