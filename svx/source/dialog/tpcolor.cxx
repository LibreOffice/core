/*************************************************************************
 *
 *  $RCSfile: tpcolor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-09-26 06:37:20 $
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
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SFXFILEDLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SV_COLRDLG_HXX
#include <svtools/colrdlg.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen wg. ErrorBox
#include <vcl/msgbox.hxx>
#endif
#pragma hdrstop

#define _SVX_TPCOLOR_CXX
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE

#include "dialogs.hrc"
#include "helpid.hrc"

#include "xattr.hxx"
#include "xpool.hxx"
#include "xtable.hxx"
#include "drawitem.hxx"
#include "tabarea.hxx"
#include "tabarea.hrc"
#include "dlgname.hxx"
#include "dlgname.hrc"
#include "dialmgr.hxx"

#define DLGWIN this->GetParent()->GetParent()

/*************************************************************************
|*
|*  Dialog zum Aendern und Definieren der Farben
|*
\************************************************************************/

SvxColorTabPage::SvxColorTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SfxTabPage          ( pParent, SVX_RES( RID_SVXPAGE_COLOR ), rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOutOld             ( &aCtlPreviewOld ),
    XOutNew             ( &aCtlPreviewNew ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() ),

    aXFStyleItem        ( XFILL_SOLID ),
    aXFillColorItem     ( String(), Color( COL_BLACK ) ),

    aValSetColorTable   ( this, ResId( CTL_COLORTABLE ) ),
    aGrpColorTable      ( this, ResId( GRP_COLORTABLE ) ),
    aFtName             ( this, ResId( FT_NAME ) ),
    aEdtName            ( this, ResId( EDT_NAME ) ),
    aFtColor            ( this, ResId( FT_COLOR ) ),
    aLbColor            ( this, ResId( LB_COLOR ) ),
    aLbColorModel       ( this, ResId( LB_COLORMODEL ) ),
    aFtColorModel1      ( this, ResId( FT_1 ) ),
    aMtrFldColorModel1  ( this, ResId( MTR_FLD_1 ) ),
    aFtColorModel2      ( this, ResId( FT_2 ) ),
    aMtrFldColorModel2  ( this, ResId( MTR_FLD_2 ) ),
    aFtColorModel3      ( this, ResId( FT_3 ) ),
    aMtrFldColorModel3  ( this, ResId( MTR_FLD_3 ) ),
    aFtColorModel4      ( this, ResId( FT_4 ) ),
    aMtrFldColorModel4  ( this, ResId( MTR_FLD_4 ) ),
    aGrpColorModel      ( this, ResId( GRP_COLORMODEL ) ),
    aBtnAdd             ( this, ResId( BTN_ADD ) ),
    aBtnModify          ( this, ResId( BTN_MODIFY ) ),
    aBtnWorkOn          ( this, ResId( BTN_WORK_ON ) ),
    aBtnDelete          ( this, ResId( BTN_DELETE ) ),
    aCtlPreviewOld      ( this, ResId( CTL_PREVIEW_OLD ), &XOutOld ),
    aCtlPreviewNew      ( this, ResId( CTL_PREVIEW_NEW ), &XOutNew ),
    aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
    aBtnLoad            ( this, ResId( BTN_LOAD ) ),
    aBtnSave            ( this, ResId( BTN_SAVE ) ),

    rOutAttrs           ( rInAttrs ),
    bDeleteColorTable   ( TRUE ),
    eCM                 ( CM_RGB )

{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Setzen des Output-Devices
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXFillColorItem );
    XOutOld.SetFillAttr( aXFillAttr );
    XOutNew.SetFillAttr( aXFillAttr );

    // Setzen der Linie auf None im OutputDevice
    XLineAttrSetItem aXLineAttr( pXPool );
    aXLineAttr.GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
    XOutOld.SetLineAttr( aXLineAttr );
    XOutNew.SetLineAttr( aXLineAttr );

    // Handler ueberladen
    aLbColor.SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectColorLBHdl_Impl ) );
    aValSetColorTable.SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectValSetHdl_Impl ) );
    aLbColorModel.SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectColorModelHdl_Impl ) );

    Link aLink = LINK( this, SvxColorTabPage, ModifiedHdl_Impl );
    aMtrFldColorModel1.SetModifyHdl( aLink );
    aMtrFldColorModel2.SetModifyHdl( aLink );
    aMtrFldColorModel3.SetModifyHdl( aLink );
    aMtrFldColorModel4.SetModifyHdl( aLink );

    aBtnAdd.SetClickHdl( LINK( this, SvxColorTabPage, ClickAddHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxColorTabPage, ClickModifyHdl_Impl ) );
    aBtnWorkOn.SetClickHdl(
        LINK( this, SvxColorTabPage, ClickWorkOnHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxColorTabPage, ClickDeleteHdl_Impl ) );
    aBtnLoad.SetClickHdl( LINK( this, SvxColorTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl( LINK( this, SvxColorTabPage, ClickSaveHdl_Impl ) );

    // ValueSet
    aValSetColorTable.SetStyle( aValSetColorTable.GetStyle() | WB_VSCROLL | WB_ITEMBORDER );
    aValSetColorTable.SetColCount( 8 );
    aValSetColorTable.SetLineCount( 10 );
    aValSetColorTable.SetExtraSpacing( 0 );
    aValSetColorTable.Show();

    pColorTab = NULL;
}

// -----------------------------------------------------------------------

void SvxColorTabPage::Construct()
{
    aLbColor.Fill( pColorTab );
    FillValueSet_Impl( aValSetColorTable );
}

// -----------------------------------------------------------------------

void SvxColorTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        *pbAreaTP = FALSE;

        if( pColorTab )
        {
            if( *pPageType == PT_COLOR && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbColor.SelectEntryPos( *pPos );
                aValSetColorTable.SelectItem( aLbColor.GetSelectEntryPos() + 1 );
                aEdtName.SetText( aLbColor.GetSelectEntry() );

                ChangeColorHdl_Impl( this );
            }
            else if( *pPageType == PT_COLOR && *pPos == LISTBOX_ENTRY_NOTFOUND )
            {
                const SfxPoolItem* pPoolItem = NULL;
                if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE, &pPoolItem ) )
                {
                    aLbColorModel.SelectEntryPos( CM_RGB );

                    aAktuellColor.SetColor ( ( ( const XFillColorItem* ) pPoolItem )->GetValue().GetColor() );

                    aEdtName.SetText( ( ( const XFillColorItem* ) pPoolItem )->GetName() );

                    aMtrFldColorModel1.SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
                    aMtrFldColorModel2.SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
                    aMtrFldColorModel3.SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );

                    // ItemSet fuellen und an XOut weiterleiten
                    rXFSet.Put( XFillColorItem( String(), aAktuellColor ) );
                    XOutOld.SetFillAttr( aXFillAttr );
                    XOutNew.SetFillAttr( aXFillAttr );

                    aCtlPreviewNew.Invalidate();
                    aCtlPreviewOld.Invalidate();
                }
            }

            // Ermitteln (evtl. abschneiden) des Namens und in
            // der GroupBox darstellen
            String          aString( SVX_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL( pColorTab->GetName(), INET_PROT_FILE );

            if ( aURL.getBase().Len() > 18 )
            {
                aString += aURL.getBase().Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += aURL.getBase();

            aGrpColorTable.SetText( aString );

            // Damit evtl. geaenderte Farbe verworfen wird
            SelectColorLBHdl_Impl( this );

            *pPageType = PT_COLOR;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
    else
    {
        // Buttons werden gehided, weil Paletten z.Z. nur
        // ueber den AreaDlg funktionieren!!!
        // ActivatePage() muss von anderen Dialogen explizit
        // gerufen werden, da ActivatePage() nicht gerufen wird,
        // wenn Seite als Erste im Dialog angezeigt wird
        aBtnLoad.Hide();
        aBtnSave.Hide();
    }
}

// -----------------------------------------------------------------------

int SvxColorTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( CheckChanges_Impl() == -1L )
        return( KEEP_PAGE );

    FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

long SvxColorTabPage::CheckChanges_Impl()
{
    // wird hier benutzt, um Aenderungen NICHT zu verlieren

    Color aTmpColor (aAktuellColor);
    if (eCM != CM_RGB)
        ConvertColorValues (aTmpColor, CM_RGB);

    USHORT nPos = aLbColor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        Color aColor = pColorTab->Get( nPos )->GetColor();
        String aString = aLbColor.GetSelectEntry();

        // aNewColor, da COL_USER != COL_irgendwas, auch wenn RGB-Werte gleich
        // Color aNewColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );

        if( ColorToPercent_Impl( aTmpColor.GetRed() ) != ColorToPercent_Impl( aColor.GetRed() ) ||
            ColorToPercent_Impl( aTmpColor.GetGreen() ) != ColorToPercent_Impl( aColor.GetGreen() ) ||
            ColorToPercent_Impl( aTmpColor.GetBlue() ) != ColorToPercent_Impl( aColor.GetBlue() ) ||
            aString != aEdtName.GetText() )
        {
            ResMgr* pMgr = DIALOG_MGR();
            SvxMessDialog aMessDlg( DLGWIN,
                String( ResId( RID_SVXSTR_COLOR, pMgr ) ),
                String( ResId( RID_SVXSTR_ASK_CHANGE_COLOR, pMgr ) ),
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
                    aColor = pColorTab->Get( nPos )->GetColor();
                }
                break;

                case RET_BTN_2: // Hinzufuegen
                {
                    ClickAddHdl_Impl( this );
                    nPos = aLbColor.GetSelectEntryPos();
                    aColor = pColorTab->Get( nPos )->GetColor();
                }
                break;

                case RET_CANCEL:
                    // return( -1L ); <-- wuerde die Seite nicht verlassen
                break;
                // return( TRUE ); // Abbruch
            }
        }
    }
    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        nPos = aLbColor.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            *pPos = nPos;
        }
    }
    return( 0L );
}

// -----------------------------------------------------------------------

BOOL SvxColorTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if( ( *pDlgType != 0 ) ||
        ( *pPageType == PT_COLOR && *pbAreaTP == FALSE ) )
    {
        String aString;
        Color  aColor;

        // CheckChanges_Impl(); <-- doppelte Abfrage ?

        USHORT nPos = aLbColor.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            aColor  = pColorTab->Get( nPos )->GetColor();
            aString = aLbColor.GetSelectEntry();
        }
        else
        {
            aColor.SetColor (aAktuellColor.GetColor());
            if (eCM != CM_RGB)
                ConvertColorValues (aColor, CM_RGB);
        }
        rOutAttrs.Put( XFillColorItem( aString, aColor ) );
        rOutAttrs.Put( XFillStyleItem( XFILL_SOLID ) );
    }

    return( TRUE );
}

// -----------------------------------------------------------------------

void SvxColorTabPage::Reset( const SfxItemSet& rOutAttrs )
{
    USHORT nState = rOutAttrs.GetItemState( XATTR_FILLCOLOR );

    if ( nState >= SFX_ITEM_DEFAULT )
    {
        XFillColorItem aColorItem( (const XFillColorItem&)rOutAttrs.Get( XATTR_FILLCOLOR ) );
        aLbColor.SelectEntry( aColorItem.GetValue() );
        aValSetColorTable.SelectItem( aLbColor.GetSelectEntryPos() + 1 );
        aEdtName.SetText( aLbColor.GetSelectEntry() );
    }

    // Farbmodell setzen
    String aStr = GetUserData();
    aLbColorModel.SelectEntryPos( (USHORT) aStr.ToInt32() );

    ChangeColorHdl_Impl( this );
    SelectColorModelHdl_Impl( this );

    aCtlPreviewOld.Invalidate();

    // Status der Buttons ermitteln
    if( pColorTab->Count() )
    {
        aBtnModify.Enable();
        aBtnWorkOn.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnWorkOn.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
}

// -----------------------------------------------------------------------

SfxTabPage* SvxColorTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxColorTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

//
// Wird aufgerufen, wenn Inhalt der MtrFileds für Farbwerte verändert wird
//
IMPL_LINK( SvxColorTabPage, ModifiedHdl_Impl, void *, EMPTYARG )
{
    // lese aktuelle MtrFields aus, wenn cmyk, dann k-Wert als Trans.-Farbe
    aAktuellColor.SetColor ( Color( (UINT8)PercentToColor_Impl( (USHORT) aMtrFldColorModel4.GetValue() ),
                                    (UINT8)PercentToColor_Impl( (USHORT) aMtrFldColorModel1.GetValue() ),
                                    (UINT8)PercentToColor_Impl( (USHORT) aMtrFldColorModel2.GetValue() ),
                                    (UINT8)PercentToColor_Impl( (USHORT) aMtrFldColorModel3.GetValue() ) ).GetColor() );

    Color aTmpColor(aAktuellColor);
    if (eCM != CM_RGB)
        ConvertColorValues (aTmpColor, CM_RGB);

    rXFSet.Put( XFillColorItem( String(), aTmpColor ) );
    XOutNew.SetFillAttr( aXFillAttr );

    aCtlPreviewNew.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

//
// Button 'Hinzufügen'
//
IMPL_LINK( SvxColorTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
    ResMgr* pMgr = DIALOG_MGR();
    String aDesc( ResId( RID_SVXSTR_DESC_COLOR, pMgr ) );
    String aName( aEdtName.GetText() );
    XColorEntry* pEntry;
    long nCount = pColorTab->Count();
    BOOL bDifferent = TRUE;

    // Pruefen, ob Name schon vorhanden ist
    for ( long i = 0; i < nCount && bDifferent; i++ )
        if ( aName == pColorTab->Get( i )->GetName() )
            bDifferent = FALSE;

    // Wenn ja, wird wiederholt ein neuer Name angefordert
    if ( !bDifferent )
    {
        WarningBox aWarningBox( DLGWIN, WinBits( WB_OK ),
            String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, pMgr ) ) );
        aWarningBox.Execute();

        SvxNameDialog* pDlg = new SvxNameDialog( DLGWIN, aName, aDesc );
        BOOL bLoop = TRUE;

        while ( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = TRUE;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pColorTab->Get( i )->GetName() )
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
        if (eCM != CM_RGB)
            ConvertColorValues (aAktuellColor, CM_RGB);
        pEntry = new XColorEntry( aAktuellColor, aName );

        pColorTab->Insert( pColorTab->Count(), pEntry );

        aLbColor.Append( pEntry );
        aValSetColorTable.InsertItem( aValSetColorTable.GetItemCount() + 1,
                pEntry->GetColor(), pEntry->GetName() );

        aLbColor.SelectEntryPos( aLbColor.GetEntryCount() - 1 );

        // Flag fuer modifiziert setzen
        *pnColorTableState |= CT_MODIFIED;

        SelectColorLBHdl_Impl( this );
    }
    // Status der Buttons ermitteln
    if( pColorTab->Count() )
    {
        aBtnModify.Enable();
        aBtnWorkOn.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

//
// Button 'Ändern'
//
IMPL_LINK( SvxColorTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbColor.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr* pMgr = DIALOG_MGR();
        String aDesc( ResId( RID_SVXSTR_DESC_COLOR, pMgr ) );
        String aName( aEdtName.GetText() );
        long nCount = pColorTab->Count();
        BOOL bDifferent = TRUE;

        // Pruefen, ob Name schon vorhanden ist
        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pColorTab->Get( i )->GetName() && nPos != i )
                bDifferent = FALSE;

        // Wenn ja, wird wiederholt ein neuer Name angefordert
        if ( !bDifferent )
        {
            WarningBox aWarningBox( DLGWIN, WinBits( WB_OK ),
                String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, pMgr ) ) );
            aWarningBox.Execute();

            SvxNameDialog* pDlg = new SvxNameDialog( DLGWIN, aName, aDesc );
            BOOL bLoop = TRUE;

            while ( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );
                bDifferent = TRUE;

                for ( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == pColorTab->Get( i )->GetName() &&  nPos != i )
                        bDifferent = FALSE;

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
            XColorEntry* pEntry = pColorTab->Get( nPos );

            Color aTmpColor (aAktuellColor);
            if (eCM != CM_RGB)
                ConvertColorValues (aTmpColor, CM_RGB);

            pEntry->SetColor( aTmpColor );
            pEntry->SetName( aName );

            aLbColor.Modify( pEntry, nPos );
            aLbColor.SelectEntryPos( nPos );
            /////
            aValSetColorTable.SetItemColor( nPos + 1, pEntry->GetColor() );
            aValSetColorTable.SetItemText( nPos + 1, pEntry->GetName() );
            aEdtName.SetText( aName );

            aCtlPreviewOld.Invalidate();

            // Flag fuer modifiziert setzen
            *pnColorTableState |= CT_MODIFIED;
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

//
// Button 'Bearbeiten'
//
IMPL_LINK( SvxColorTabPage, ClickWorkOnHdl_Impl, void *, EMPTYARG )
{
    SvColorDialog* pColorDlg = new SvColorDialog( DLGWIN );

    Color aTmpColor (aAktuellColor);
    if (eCM != CM_RGB)
        ConvertColorValues (aTmpColor, CM_RGB);

    pColorDlg->SetColor (aTmpColor);

    if( pColorDlg->Execute() == RET_OK )
    {
        USHORT nK = 0;
        Color aPreviewColor = pColorDlg->GetColor();
        aAktuellColor = aPreviewColor;
        if (eCM != CM_RGB)
            ConvertColorValues (aAktuellColor, eCM);

        aMtrFldColorModel1.SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
        aMtrFldColorModel2.SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
        aMtrFldColorModel3.SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
        aMtrFldColorModel4.SetValue( ColorToPercent_Impl( nK ) );

        // ItemSet fuellen und an XOut weiterleiten
        rXFSet.Put( XFillColorItem( String(), aPreviewColor ) );
        //XOutOld.SetFillAttr( aXFillAttr );
        XOutNew.SetFillAttr( aXFillAttr );

        aCtlPreviewNew.Invalidate();
    }
    delete( pColorDlg );

    return( 0L );
}

//------------------------------------------------------------------------

//
// Button 'Loeschen'
//
IMPL_LINK( SvxColorTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbColor.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
            String( SVX_RES( RID_SVXSTR_ASK_DEL_COLOR ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            // Jetzt wird richtig geloescht
            ULONG nCount = pColorTab->Count() - 1;
            XColorEntry* pEntry;

            pEntry = pColorTab->Remove( nPos );
            DBG_ASSERT( pEntry, "ColorEntry nicht vorhanden (1) !" );
            delete pEntry;

            for( ULONG i = nPos; i < nCount; i++ )
            {
                pEntry = pColorTab->Remove( i + 1 );
                DBG_ASSERT( pEntry, "ColorEntry nicht vorhanden (2) !" );
                pColorTab->Insert( i, pEntry );
            }

            // Listbox und ValueSet aktualisieren
            aLbColor.RemoveEntry( nPos );
            aValSetColorTable.Clear();
            FillValueSet_Impl( aValSetColorTable );

            // Positionieren
            aLbColor.SelectEntryPos( nPos );
            SelectColorLBHdl_Impl( this );

            aCtlPreviewOld.Invalidate();

            // Flag fuer modifiziert setzen
            *pnColorTableState |= CT_MODIFIED;
        }
    }
    // Status der Buttons ermitteln
    if( !pColorTab->Count() )
    {
        aBtnModify.Disable();
        aBtnWorkOn.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return( 0L );
}

// -----------------------------------------------------------------------

//
// Button 'Farbtabelle laden'
//
IMPL_LINK( SvxColorTabPage, ClickLoadHdl_Impl, void *, p )
{
    ResMgr* pMgr = DIALOG_MGR();
    USHORT nReturn = RET_YES;

    if( *pnColorTableState & CT_MODIFIED )
    {
        nReturn = WarningBox( DLGWIN, WinBits( WB_YES_NO_CANCEL ),
                    String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, pMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pColorTab->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_OPEN | WB_3DLOOK );

        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soc" ) );
        pFileDlg->AddFilter( aStrFilterType, aStrFilterType );

        String aFile( SvtPathOptions().GetPalettePath() );
        pFileDlg->SetPath( aFile );

        if( pFileDlg->Execute() == RET_OK )
        {
            INetURLObject aURL( pFileDlg->GetPath(), INET_PROT_FILE );
            INetURLObject aPathURL( aURL ); aPathURL.removeSegment(); aPathURL.removeFinalSlash();

            // Tabelle speichern
            XColorTable* pColTab = new XColorTable( aPathURL.PathToFileName(), pXPool );
            pColTab->SetName( aURL.getName() ); // XXX
            if( pColTab->Load() )
            {
                if( pColTab )
                {
                    // Pruefen, ob Tabelle geloescht werden darf:
                    if( pColorTab != ( (SvxAreaTabDialog*) DLGWIN )->GetColorTable() )
                    {
                        if( bDeleteColorTable )
                            delete pColorTab;
                        else
                            bDeleteColorTable = TRUE;
                    }

                    pColorTab = pColTab;
                    ( (SvxAreaTabDialog*) DLGWIN )->SetNewColorTable( pColorTab );

                    aLbColor.Clear();
                    aValSetColorTable.Clear();
                    Construct();
                    Reset( rOutAttrs );

                    pColorTab->SetName( aURL.getName() );

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

                    aGrpColorTable.SetText( aString );

                    // Flag fuer gewechselt setzen
                    *pnColorTableState |= CT_CHANGED;
                    // Flag fuer modifiziert entfernen
                    *pnColorTableState &= ~CT_MODIFIED;

                    if( aLbColor.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
                        aLbColor.SelectEntryPos( 0 );
                    else
                        aLbColor.SelectEntryPos( aLbColor.GetSelectEntryPos() );

                    ChangeColorHdl_Impl( this );
                    SelectColorLBHdl_Impl( this );
                }
            }
            else
            {
                //aIStream.Close();
                ErrorBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, pMgr ) ) ).Execute();
            }
        }
        delete( pFileDlg );
    }

    // Status der Buttons ermitteln
    if ( pColorTab->Count() )
    {
        aBtnModify.Enable();
        aBtnWorkOn.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnWorkOn.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return( 0L );
}

// -----------------------------------------------------------------------

//
// Button 'Farbtabelle speichern'
//
IMPL_LINK( SvxColorTabPage, ClickSaveHdl_Impl, void *, p )
{
    SfxFileDialog* pFileDlg = new SfxFileDialog( DLGWIN, WB_SAVEAS | WB_3DLOOK );

    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soc" ) );
    pFileDlg->AddFilter( aStrFilterType, aStrFilterType );
    INetURLObject aFile( SvtPathOptions().GetPalettePath(), INET_PROT_FILE );

    if( pColorTab->GetName().Len() )
    {
        aFile.Append( pColorTab->GetName() );

        if( !aFile.getExtension().Len() )
            aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "soc" ) ) );
    }

    pFileDlg->SetPath( aFile.PathToFileName() );

    if ( pFileDlg->Execute() == RET_OK )
    {
        INetURLObject   aURL( pFileDlg->GetPath(), INET_PROT_FILE );
        INetURLObject   aPathURL( aURL ); aPathURL.removeSegment(); aPathURL.removeFinalSlash();

        pColorTab->SetName( aURL.getName() );
        pColorTab->SetPath( aPathURL.PathToFileName() );

        if( pColorTab->Save() )
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
            aGrpColorTable.SetText( aString );

            // Flag fuer gespeichert setzen
            *pnColorTableState |= CT_SAVED;
            // Flag fuer modifiziert entfernen
            *pnColorTableState &= ~CT_MODIFIED;
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

IMPL_LINK( SvxColorTabPage, SelectColorLBHdl_Impl, void *, EMPTYARG )
{
    USHORT nPos = aLbColor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        aValSetColorTable.SelectItem( nPos + 1 );
        aEdtName.SetText( aLbColor.GetSelectEntry() );

        rXFSet.Put( XFillColorItem( String(),
                                    aLbColor.GetSelectEntryColor() ) );
        XOutOld.SetFillAttr( aXFillAttr );
        XOutNew.SetFillAttr( aXFillAttr );

        aCtlPreviewOld.Invalidate();
        aCtlPreviewNew.Invalidate();

        ChangeColorHdl_Impl( this );
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxColorTabPage, SelectValSetHdl_Impl, void *, EMPTYARG )
{
    USHORT nPos = aValSetColorTable.GetSelectItemId();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        aLbColor.SelectEntryPos( nPos - 1 );
        aEdtName.SetText( aLbColor.GetSelectEntry() );

        rXFSet.Put( XFillColorItem( String(),
                                    aLbColor.GetSelectEntryColor() ) );
        XOutOld.SetFillAttr( aXFillAttr );
        XOutNew.SetFillAttr( aXFillAttr );

        aCtlPreviewOld.Invalidate();
        aCtlPreviewNew.Invalidate();

        ChangeColorHdl_Impl( this );
    }
    return( 0L );
}

//------------------------------------------------------------------------

//
// Farbwerte je nach übergebenes Farbmodell umrechnen
//
void SvxColorTabPage::ConvertColorValues (Color& rColor, ColorModel eModell)
{
    switch (eModell)
    {
        case CM_RGB:
        {
            CmykToRgb_Impl (rColor, (USHORT)rColor.GetTransparency() );
            rColor.SetTransparency ((UINT8) 0);
        }
        break;

        case CM_CMYK:
        {
            USHORT nK;
            RgbToCmyk_Impl (rColor, nK );
            rColor.SetTransparency ((UINT8) nK);
        }
        break;
    }
}

//
// Auswahl Listbox 'Farbmodell' (RGB/CMY)
//
IMPL_LINK( SvxColorTabPage, SelectColorModelHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbColorModel.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if (eCM != (ColorModel) nPos)
        {
            // wenn Farbmodell geaendert wurde, dann Werte umrechnen
            ConvertColorValues (aAktuellColor, (ColorModel) nPos);
        }

        eCM = (ColorModel) nPos;

        switch( eCM )
        {
            case CM_RGB:
            {
                String aStr( aLbColorModel.GetSelectEntry() );
                String aColorStr( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "~X" ) ) );
                xub_StrLen nIdx = 1;
                aColorStr.SetChar( nIdx, aStr.GetChar( 0 ) );
                aFtColorModel1.SetText( aColorStr );
                aColorStr.SetChar( nIdx, aStr.GetChar( 1 ) );
                aFtColorModel2.SetText( aColorStr );
                aColorStr.SetChar( nIdx, aStr.GetChar( 2 ) );
                aFtColorModel3.SetText( aColorStr );

                aFtColorModel4.Hide();
                aMtrFldColorModel4.Hide();
                aMtrFldColorModel4.SetValue( 0L );

                aMtrFldColorModel1.SetHelpId( HID_TPCOLOR_RGB_1 );
                aMtrFldColorModel2.SetHelpId( HID_TPCOLOR_RGB_2 );
                aMtrFldColorModel3.SetHelpId( HID_TPCOLOR_RGB_3 );

                // Da der alte HelpText noch am Control steht wuerde
                // ein Umsetzen der HelpID alleine nichts bewirken
                aMtrFldColorModel1.SetHelpText( String() );
                aMtrFldColorModel2.SetHelpText( String() );
                aMtrFldColorModel3.SetHelpText( String() );

                // RGB-Werte im Bereich 0..255 verarbeiten (nicht in %),
                // dazu MetricField's entsprechend einstellen
                aMtrFldColorModel1.SetUnit (FUNIT_NONE);
                aMtrFldColorModel1.SetMin (0);
                aMtrFldColorModel1.SetMax (255);

                aMtrFldColorModel2.SetUnit (FUNIT_NONE);
                aMtrFldColorModel2.SetMin (0);
                aMtrFldColorModel2.SetMax (255);

                aMtrFldColorModel3.SetUnit (FUNIT_NONE);
                aMtrFldColorModel3.SetMin (0);
                aMtrFldColorModel3.SetMax (255);
            }
            break;

            case CM_CMYK:
            {
                String aStr( aLbColorModel.GetSelectEntry() );
                String aColorStr( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "~X" ) ) );
                xub_StrLen nIdx = 1;
                aColorStr.SetChar( nIdx, aStr.GetChar( 0 ) );
                aFtColorModel1.SetText( aColorStr );
                aColorStr.SetChar( nIdx, aStr.GetChar( 1 ) );
                aFtColorModel2.SetText( aColorStr );
                aColorStr.SetChar( nIdx,aStr.GetChar( 2 ) );
                aFtColorModel3.SetText( aColorStr );
                aColorStr.SetChar( nIdx,aStr.GetChar( 3 ) );
                aFtColorModel4.SetText( aColorStr );

                aFtColorModel4.Show();
                aMtrFldColorModel4.Show();

                aMtrFldColorModel1.SetHelpId( HID_TPCOLOR_CMYK_1 );
                aMtrFldColorModel2.SetHelpId( HID_TPCOLOR_CMYK_2 );
                aMtrFldColorModel3.SetHelpId( HID_TPCOLOR_CMYK_3 );

                // s.o.
                aMtrFldColorModel1.SetHelpText( String() );
                aMtrFldColorModel2.SetHelpText( String() );
                aMtrFldColorModel3.SetHelpText( String() );

                // CMYK-Werte im Bereich 0..100% verarbeiten,
                // dazu MetricField's entsprechend einstellen
                String aStrUnit( RTL_CONSTASCII_USTRINGPARAM( " %" ) );

                aMtrFldColorModel1.SetUnit (FUNIT_CUSTOM);
                aMtrFldColorModel1.SetCustomUnitText ( aStrUnit );
                aMtrFldColorModel1.SetMin (0);
                aMtrFldColorModel1.SetMax (100);

                aMtrFldColorModel2.SetUnit (FUNIT_CUSTOM);
                aMtrFldColorModel2.SetCustomUnitText ( aStrUnit );
                aMtrFldColorModel2.SetMin (0);
                aMtrFldColorModel2.SetMax (100);

                aMtrFldColorModel3.SetUnit (FUNIT_CUSTOM);
                aMtrFldColorModel3.SetCustomUnitText ( aStrUnit );
                aMtrFldColorModel3.SetMin (0);
                aMtrFldColorModel3.SetMax (100);
            }
            break;
        }

        aMtrFldColorModel1.SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
        aMtrFldColorModel2.SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
        aMtrFldColorModel3.SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
        aMtrFldColorModel4.SetValue( ColorToPercent_Impl( aAktuellColor.GetTransparency() ) );
    }
    return( 0L );
}

//------------------------------------------------------------------------

long SvxColorTabPage::ChangeColorHdl_Impl( void* )
{
    int nPos = aLbColor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        XColorEntry* pEntry = pColorTab->Get( nPos );

        aAktuellColor.SetColor ( pEntry->GetColor().GetColor() );
        if (eCM != CM_RGB)
            ConvertColorValues (aAktuellColor, eCM);

        aMtrFldColorModel1.SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
        aMtrFldColorModel2.SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
        aMtrFldColorModel3.SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
        aMtrFldColorModel4.SetValue( ColorToPercent_Impl( aAktuellColor.GetTransparency() ) );

        // ItemSet fuellen und an XOut weiterleiten
        rXFSet.Put( XFillColorItem( String(), pEntry->GetColor() ) );
        XOutOld.SetFillAttr( aXFillAttr );
        XOutNew.SetFillAttr( aXFillAttr );

        aCtlPreviewNew.Invalidate();
    }
    return( 0L );
}

//------------------------------------------------------------------------

void SvxColorTabPage::FillValueSet_Impl( ValueSet& rVs )
{
    long nCount = pColorTab->Count();
    XColorEntry* pColorEntry;

    for( long i = 0; i < nCount; i++ )
    {
        pColorEntry = pColorTab->Get( i );
        rVs.InsertItem( (USHORT) i + 1, pColorEntry->GetColor(), pColorEntry->GetName() );
    }
}

//------------------------------------------------------------------------

// Ein RGB-Wert wird in einen CMYK-Wert konvertiert, wobei die Color-
// Klasse vergewaltigt wird, da R in C, G in M und B in Y umgewandelt
// wird. Der Wert K wird in einer Extra-Variablen gehalten.
// Bei weiteren Farbmodellen sollte man hierfuer eigene Klassen entwickeln,
// die dann auch entsprechende Casts enthalten.

void SvxColorTabPage::RgbToCmyk_Impl( Color& rColor, USHORT& rK )
{
    USHORT nColor1, nColor2, nColor3;
    USHORT nProzent; // nur temporaer !!!

    nColor1 = 255 - rColor.GetRed();
    nProzent = ColorToPercent_Impl( nColor1 );

    nColor2 = 255 - rColor.GetGreen();
    nProzent = ColorToPercent_Impl( nColor2 );

    nColor3 = 255 - rColor.GetBlue();
    nProzent = ColorToPercent_Impl( nColor3 );

    rK = Min( Min( nColor1, nColor2 ), nColor3 );

    rColor.SetRed( nColor1 - rK );
    rColor.SetGreen( nColor2 - rK );
    rColor.SetBlue( nColor3 - rK );
}

//------------------------------------------------------------------------

// Umgekehrter Fall zu RgbToCmyk_Impl (s.o.)

void SvxColorTabPage::CmykToRgb_Impl( Color& rColor, const USHORT nK )
{
    long lTemp;

    lTemp = 255 - ( rColor.GetRed() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetRed( (BYTE)lTemp );

    lTemp = 255 - ( rColor.GetGreen() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetGreen( (BYTE)lTemp );

    lTemp = 255 - ( rColor.GetBlue() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetBlue( (BYTE)lTemp );
}

//------------------------------------------------------------------------

USHORT SvxColorTabPage::ColorToPercent_Impl( USHORT nColor )
{
    USHORT nWert;

    switch (eCM)
    {
        case CM_RGB :
            nWert = nColor;
            break;

        case CM_CMYK:
            nWert = (USHORT) ( (double) nColor * 100.0 / 255.0 + 0.5 );
            break;
    }

    return ( nWert );
}

//------------------------------------------------------------------------

USHORT SvxColorTabPage::PercentToColor_Impl( USHORT nPercent )
{
    USHORT nWert;

    switch (eCM)
    {
        case CM_RGB :
            nWert = nPercent;
            break;

        case CM_CMYK:
            nWert = (USHORT) ( (double) nPercent * 255.0 / 100.0 + 0.5 );
            break;
    }

    return ( nWert );
}

//------------------------------------------------------------------------

void SvxColorTabPage::FillUserData()
{
    // Das Farbmodell wird in der Ini-Datei festgehalten
    SetUserData( UniString::CreateFromInt32( eCM ) );
}


