/*************************************************************************
 *
 *  $RCSfile: tabdlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-16 16:08:47 $
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

#include <limits.h>
#include <stdlib.h>

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif
#pragma hdrstop

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#include "sfxtypes.hxx"
#include "appdata.hxx"
#include "minarray.hxx"
#include "tabdlg.hxx"
#include "viewfrm.hxx"
#include "app.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "sfxresid.hxx"
#include "sfxhelp.hxx"

#include "dialog.hrc"
#include "helpid.hrc"

/*  -----------------------------------------------------------------
    Verwaltungsdatenstruktur f"ur jede Seite
    ----------------------------------------------------------------- */
struct Data_Impl
{
    USHORT nId;                  // Die ID
    CreateTabPage fnCreatePage;  // Pointer auf die Factory
    GetTabPageRanges fnGetRanges;// Pointer auf die Ranges-Funktion
    SfxTabPage* pTabPage;        // die TabPage selber
    BOOL bOnDemand;              // Flag: ItemSet onDemand
    BOOL bRefresh;               // Flag: Seite mu\s neu initialisiert werden

    // Konstruktor
    Data_Impl( USHORT Id, CreateTabPage fnPage,
               GetTabPageRanges fnRanges, BOOL bDemand ) :

        nId         ( Id ),
        fnCreatePage( fnPage ),
        fnGetRanges ( fnRanges ),
        pTabPage    ( 0 ),
        bOnDemand   ( bDemand ),
        bRefresh    ( FALSE )
    {}
};


DECL_PTRARRAY(SfxTabDlgData_Impl, Data_Impl *, 4,4)

struct TabDlg_Impl
{
    BOOL                bModified       : 1,
                        bModal          : 1,
                        bInOK           : 1,
                        bHideResetBtn   : 1;
    SfxTabDlgData_Impl* pData;

    PushButton*         pApplyButton;

    TabDlg_Impl( BYTE nCnt ) :

        bModified       ( FALSE ),
        bModal          ( TRUE ),
        bInOK           ( FALSE ),
        bHideResetBtn   ( FALSE ),
        pData           ( new SfxTabDlgData_Impl( nCnt ) ),
        pApplyButton    ( NULL )
    {}
};

struct TabPageImpl
{
    BOOL    _bStandard;

    TabPageImpl() : _bStandard( FALSE ) {}
};

/*  -----------------------------------------------------------------
    Hilfsfunktion: Finden einer Page anhand seiner Id
    ----------------------------------------------------------------- */
Data_Impl* Find( SfxTabDlgData_Impl& rArr, USHORT nId, USHORT* pPos = 0 );

Data_Impl* Find( SfxTabDlgData_Impl& rArr, USHORT nId, USHORT* pPos )
{
    const USHORT nCount = rArr.Count();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        Data_Impl* pObj = rArr[i];

        if ( pObj->nId == nId )
        {
            if ( pPos )
                *pPos = i;
            return pObj;
        }
    }
    return 0;
}

// class SfxTabPage-------------------------------------------------------

SfxTabPage::SfxTabPage( Window *pParent,
                        const ResId &rResId, const SfxItemSet &rAttrSet ) :

/*  [Beschreibung]

    Konstruktor
*/

    TabPage( pParent, rResId ),

    pSet                ( &rAttrSet ),
    bHasExchangeSupport ( FALSE ),
    pTabDlg             ( NULL ),
    pImpl               ( new TabPageImpl )

{
}
// -----------------------------------------------------------------------
SfxTabPage:: SfxTabPage( Window *pParent, WinBits nStyle, const SfxItemSet &rAttrSet ) :
    TabPage(pParent, nStyle),
    pSet                ( &rAttrSet ),
    bHasExchangeSupport ( FALSE ),
    pTabDlg             ( NULL ),
    pImpl               ( new TabPageImpl )
{
}
// -----------------------------------------------------------------------

SfxTabPage::~SfxTabPage()

/*  [Beschreibung]

    Destruktor
*/

{
    delete pImpl;
}

// -----------------------------------------------------------------------

void SfxTabPage::ActivatePage( const SfxItemSet& )

/*  [Beschreibung]

    Defaultimplementierung der virtuellen ActivatePage-Methode
    Diese wird gerufen, wenn eine Seite des Dialogs den Datenaustausch
    zwischen Pages unterst"utzt.

    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
}

// -----------------------------------------------------------------------

int SfxTabPage::DeactivatePage( SfxItemSet* )

/*  [Beschreibung]

    Defaultimplementierung der virtuellen DeactivatePage-Methode
    Diese wird vor dem Verlassen einer Seite durch den Sfx gerufen;
    die Anwendung kann "uber den Returnwert steuern,
    ob die Seite verlassen werden soll.
    Falls die Seite "uber bHasExchangeSupport
    anzeigt, da\s sie einen Datenaustausch zwischen Seiten
    unterst"utzt, wird ein Pointer auf das Austausch-Set als
    Parameter "ubergeben. Dieser nimmt die Daten f"ur den Austausch
    entgegen; das Set steht anschlie\send als Parameter in
    <SfxTabPage::ActivatePage(const SfxItemSet &)> zur Verf"ugung.

    [R"uckgabewert]

    LEAVE_PAGE; Verlassen der Seite erlauben
*/

{
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SfxTabPage::FillUserData()

/*  [Beschreibung]

   virtuelle Methode, wird von der Basisklasse im Destruktor gerufen
   um spezielle Informationen der TabPage in der Ini-Datei zu speichern.
   Beim "Uberladen muss ein String zusammengestellt werden, der mit
   <SetUserData()> dann weggeschrieben wird.
*/

{
}

// -----------------------------------------------------------------------

BOOL SfxTabPage::IsReadOnly() const

/*  [Description]

*/

{
    return FALSE;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxTabPage::GetItem( const SfxItemSet& rSet, USHORT nSlot )

/*  [Beschreibung]

    static Methode: hiermit wird der Code der TabPage-Implementierungen
    vereinfacht.

*/

{
    const SfxItemPool* pPool = rSet.GetPool();
    USHORT nWh = pPool->GetWhich( nSlot );
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( nWh, TRUE, &pItem );

    if ( !pItem && nWh != nSlot )
        pItem = &pPool->GetDefaultItem( nWh );
    return pItem;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxTabPage::GetOldItem( const SfxItemSet& rSet,
                                           USHORT nSlot )

/*  [Beschreibung]

    Diese Methode gibt f"ur Vergleiche den alten Wert eines
    Attributs zur"uck.
*/

{
    const SfxItemSet& rOldSet = GetItemSet();
    USHORT nWh = GetWhich( nSlot );
    const SfxPoolItem* pItem = 0;

    if ( pImpl->_bStandard && rOldSet.GetParent() )
        pItem = GetItem( *rOldSet.GetParent(), nSlot );
    else if ( rSet.GetParent() &&
              SFX_ITEM_DONTCARE == rSet.GetItemState( nWh ) )
        pItem = GetItem( *rSet.GetParent(), nSlot );
    else
        pItem = GetItem( rOldSet, nSlot );
    return pItem;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxTabPage::GetExchangeItem( const SfxItemSet& rSet,
                                                USHORT nSlot )

/*  [Beschreibung]

    Diese Methode gibt f"ur Vergleiche den alten Wert eines
    Attributs zur"uck. Dabei wird ber"ucksichtigt, ob der Dialog
    gerade mit OK beendet wurde.
*/

{
    if ( pTabDlg && !pTabDlg->IsInOK() && pTabDlg->GetExampleSet() )
        return GetItem( *pTabDlg->GetExampleSet(), nSlot );
    else
        return GetOldItem( rSet, nSlot );
}

// class SfxTabDialog ----------------------------------------------------

#define INI_LIST \
    aTabCtrl    ( this, ResId(ID_TABCONTROL ) ),\
    aOKBtn      ( this ),\
    pUserBtn    ( pUserButtonText? new PushButton(this): 0 ),\
    aCancelBtn  ( this ),\
    aHelpBtn    ( this ),\
    aResetBtn   ( this ),\
    aBaseFmtBtn ( this ),\
    pSet        ( pItemSet ),\
    pOutSet     ( 0 ),\
    pExampleSet ( 0 ),\
    pRanges     ( 0 ),\
    bItemsReset ( FALSE ),\
    bFmt        ( bEditFmt ),\
    nResId      ( rResId.GetId() ), \
    nAppPageId  ( USHRT_MAX ), \
    pImpl       ( new TabDlg_Impl( (BYTE)aTabCtrl.GetPageCount() ) )

// -----------------------------------------------------------------------

SfxTabDialog::SfxTabDialog

/*  [Beschreibung]

    Konstruktor
*/

(
    SfxViewFrame* pViewFrame,       // Frame, zu dem der Dialog geh"ort
    Window* pParent,                // Parent-Fenster
    const ResId& rResId,            // ResourceId
    const SfxItemSet* pItemSet,     // Itemset mit den Daten;
                                    // kann NULL sein, wenn Pages onDemand
    BOOL bEditFmt,      // Flag: es werden Vorlagen bearbeitet
                        // wenn ja -> zus"atzlicher Button f"ur Standard
    const String* pUserButtonText   // Text fuer BenutzerButton;
                                    // wenn != 0, wird der UserButton erzeugt
) :
    TabDialog( pParent, rResId ),
    INI_LIST,
    pFrame( pViewFrame )
{
    Init_Impl( bFmt, pUserButtonText );
}

// -----------------------------------------------------------------------

SfxTabDialog::SfxTabDialog

/*  [Beschreibung]

    Konstruktor, tempor"ar ohne Frame
*/

(
    Window* pParent,                // Parent-Fenster
    const ResId& rResId,            // ResourceId
    const SfxItemSet* pItemSet,     // Itemset mit den Daten; kann NULL sein,
                                    // wenn Pages onDemand
    BOOL bEditFmt,      // Flag: es werden Vorlagen bearbeitet
                        // wenn ja -> zus"atzlicher Button f"ur Standard
    const String* pUserButtonText   // Text f"ur BenutzerButton;
                                    // wenn != 0, wird der UserButton erzeugt
) :
    TabDialog( pParent, rResId ),
    INI_LIST,
    pFrame( 0 )
{
    Init_Impl( bFmt, pUserButtonText );
    DBG_WARNING( "bitte den Ctor mit ViewFrame verwenden" );
}

// -----------------------------------------------------------------------

SfxTabDialog::~SfxTabDialog()
{
    // Konfiguration in Ini-Manager abspeichern
#if SUPD<613//MUSTINI
    SfxIniManager* pIniMgr = SFX_APP()->GetAppIniManager();
    String aDlgData( pIniMgr->GetString( GetPosPixel(), Size() ) );
    aDlgData += pIniMgr->GetToken();
    aDlgData += String::CreateFromInt32( aTabCtrl.GetCurPageId() );
    pIniMgr->Set( aDlgData, SFX_KEY_DIALOG, nResId );
    const USHORT nCount = pImpl->pData->Count();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        Data_Impl* pDataObject = pImpl->pData->GetObject(i);

        if ( pDataObject->pTabPage )
        {
            pDataObject->pTabPage->FillUserData();
            String aPageData(pDataObject->pTabPage->GetUserData());
            if ( aPageData.Len() )
            {
                pIniMgr->Set( aPageData, SFX_KEY_PAGE, pDataObject->nId);
            }

            if ( pDataObject->bOnDemand )
                delete (SfxItemSet*)&pDataObject->pTabPage->GetItemSet();
            delete pDataObject->pTabPage;
        }
        delete pDataObject;
    }
    delete pImpl->pApplyButton;
    delete pImpl->pData;
    delete pImpl;
    delete pUserBtn;
    delete pOutSet;
    delete pExampleSet;
    delete pRanges;

    SfxHelpPI *pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->ResetTopic();
#else
    String aDlgData(',');
    aDlgData += String::CreateFromInt32( aTabCtrl.GetCurPageId() );
    const USHORT nCount = pImpl->pData->Count();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        Data_Impl* pDataObject = pImpl->pData->GetObject(i);

        if ( pDataObject->pTabPage )
        {
            pDataObject->pTabPage->FillUserData();
            String aPageData(pDataObject->pTabPage->GetUserData());

            if ( pDataObject->bOnDemand )
                delete (SfxItemSet*)&pDataObject->pTabPage->GetItemSet();
            delete pDataObject->pTabPage;
        }
        delete pDataObject;
    }
    delete pImpl->pApplyButton;
    delete pImpl->pData;
    delete pImpl;
    delete pUserBtn;
    delete pOutSet;
    delete pExampleSet;
    delete pRanges;

    SfxHelpPI *pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->ResetTopic();
#endif
}

// -----------------------------------------------------------------------

void SfxTabDialog::Init_Impl( BOOL bFmt, const String* pUserButtonText )

/*  [Beschreibung]

    interne Initialisierung des Dialogs
*/

{
    aOKBtn.SetClickHdl( LINK( this, SfxTabDialog, OkHdl ) );
    aResetBtn.SetClickHdl( LINK( this, SfxTabDialog, ResetHdl ) );
    aResetBtn.SetText( String( SfxResId( STR_RESET ) ) );
    aTabCtrl.SetActivatePageHdl(
            LINK( this, SfxTabDialog, ActivatePageHdl ) );
    aTabCtrl.SetDeactivatePageHdl(
            LINK( this, SfxTabDialog, DeactivatePageHdl ) );
    aTabCtrl.Show();
    aOKBtn.Show();
    aCancelBtn.Show();
    aHelpBtn.Show();
    aResetBtn.Show();
    aResetBtn.SetHelpId( HID_TABDLG_RESET_BTN );

    if ( pUserBtn )
    {
        pUserBtn->SetText( *pUserButtonText );
        pUserBtn->SetClickHdl( LINK( this, SfxTabDialog, UserHdl ) );
        pUserBtn->Show();
    }

    if ( bFmt )
    {
        String aStd( SfxResId( STR_STANDARD ) );
        aStd.Insert( '~', 0 );
        aBaseFmtBtn.SetText( aStd );
        aBaseFmtBtn.SetClickHdl( LINK( this, SfxTabDialog, BaseFmtHdl ) );
        aBaseFmtBtn.SetHelpId( HID_TABDLG_STANDARD_BTN );

        // bFmt = tempor"ares Flag im Ctor() "ubergeben,
        // wenn bFmt == 2, dann auch TRUE,
        // zus"atzlich Ausblendung vom StandardButton,
        // nach der Initialisierung wieder auf TRUE setzen
        if ( bFmt != 2 )
            aBaseFmtBtn.Show();
        else
            bFmt = TRUE;
    }

    if ( pSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }
}

// -----------------------------------------------------------------------

void SfxTabDialog::RemoveResetButton()
{
    aResetBtn.Hide();
    pImpl->bHideResetBtn = TRUE;
}

// -----------------------------------------------------------------------

short SfxTabDialog::Execute()
{
    if ( !aTabCtrl.GetPageCount() )
        return RET_CANCEL;
    Start_Impl();
    return TabDialog::Execute();
}

// -----------------------------------------------------------------------

void SfxTabDialog::Start( BOOL bShow )
{
    aCancelBtn.SetClickHdl( LINK( this, SfxTabDialog, CancelHdl ) );
    pImpl->bModal = FALSE;
    Start_Impl();

    if ( bShow )
        Show();
}

// -----------------------------------------------------------------------

void SfxTabDialog::SetApplyHandler(const Link& _rHdl)
{
    DBG_ASSERT( pImpl->pApplyButton, "SfxTabDialog::GetApplyHandler: no apply button enabled!" );
    if ( pImpl->pApplyButton )
        pImpl->pApplyButton->SetClickHdl( _rHdl );
}

// -----------------------------------------------------------------------

Link SfxTabDialog::GetApplyHandler() const
{
    DBG_ASSERT( pImpl->pApplyButton, "SfxTabDialog::GetApplyHandler: no button enabled!" );
    if ( !pImpl->pApplyButton )
        return Link();

    return pImpl->pApplyButton->GetClickHdl();
}

// -----------------------------------------------------------------------

void SfxTabDialog::EnableApplyButton(BOOL bEnable)
{
    if ( IsApplyButtonEnabled() == bEnable )
        // nothing to do
        return;

    // create or remove the apply button
    if ( bEnable )
    {
        pImpl->pApplyButton = new PushButton( this );
        // in the z-order, the apply button should be behind the ok button, thus appearing at the right side of it
        pImpl->pApplyButton->SetZOrder(&aOKBtn, WINDOW_ZORDER_BEHIND);

        pImpl->pApplyButton->SetText( String( SfxResId( STR_APPLY ) ) );
        pImpl->pApplyButton->Show();
    }
    else
    {
        delete pImpl->pApplyButton;
        pImpl->pApplyButton = NULL;
    }

    // adjust the layout
    if (IsReallyShown())
        AdjustLayout();
}

// -----------------------------------------------------------------------

BOOL SfxTabDialog::IsApplyButtonEnabled() const
{
    return ( NULL != pImpl->pApplyButton );
}

// -----------------------------------------------------------------------

const PushButton* SfxTabDialog::GetApplyButton() const
{
    return pImpl->pApplyButton;
}

// -----------------------------------------------------------------------

PushButton* SfxTabDialog::GetApplyButton()
{
    return pImpl->pApplyButton;
}

// -----------------------------------------------------------------------

void SfxTabDialog::Start_Impl()
{
    DBG_ASSERT( pImpl->pData->Count() == aTabCtrl.GetPageCount(),
                "nicht alle Seiten angemeldet" );
    Point aPos;
    USHORT nActPage = aTabCtrl.GetPageId( 0 );

    // Konfiguration vorhanden?
#if SUPD<613//MUSTINI
    SfxApplication *pSfxApp = SFX_APP();
    SfxIniManager* pIniMgr = pSfxApp->GetAppIniManager();
    String aDlgData( pIniMgr->Get( SFX_KEY_DIALOG, nResId ) );

    if ( aDlgData.Len() > 0 )
    {
        // ggf. Position aus Konfig
        Size aDummySize;

        if ( pIniMgr->GetPosSize( aDlgData.GetToken( 0, pIniMgr->GetToken() ), aPos, aDummySize ) )
            SetPosPixel( aPos );

        // initiale TabPage aus Programm/Hilfe/Konfig
        nActPage = (USHORT)aDlgData.GetToken( 1, pIniMgr->GetToken() ).ToInt32();
        if ( USHRT_MAX != nAppPageId )
            nActPage = nAppPageId;
        else
        {
            USHORT nAutoTabPageId = pSfxApp->Get_Impl()->nAutoTabPageId;

            if ( nAutoTabPageId )
                nActPage = nAutoTabPageId;
        }

        if ( TAB_PAGE_NOTFOUND == aTabCtrl.GetPagePos( nActPage ) )
            nActPage = aTabCtrl.GetPageId( 0 );
    }
    else if ( USHRT_MAX != nAppPageId &&
              TAB_PAGE_NOTFOUND != aTabCtrl.GetPagePos( nAppPageId ) )
        nActPage = nAppPageId;
#else
    if ( USHRT_MAX != nAppPageId &&
              TAB_PAGE_NOTFOUND != aTabCtrl.GetPagePos( nAppPageId ) )
        nActPage = nAppPageId;
#endif

    aTabCtrl.SetCurPageId( nActPage );
    ActivatePageHdl( &aTabCtrl );
}

// -----------------------------------------------------------------------

void SfxTabDialog::AddTabPage

/*  [Beschreibung]

    Hinzuf"ugen einer Seite zu dem Dialog.
    Mu\s korrespondieren zu einem entsprechende Eintrag im
    TabControl in der Resource des Dialogs.
*/

(
    USHORT nId,                     // ID der Seite
    CreateTabPage pCreateFunc,      // Pointer auf die Factory-Methode
    GetTabPageRanges pRangesFunc,   // Pointer auf die Methode f"ur das
                                    // Erfragen der Ranges onDemand
    BOOL bItemsOnDemand             // gibt an, ob das Set dieser Seite beim
                                    // Erzeugen der Seite erfragt wird
)
{
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}

// -----------------------------------------------------------------------

void SfxTabDialog::AddTabPage

/*  [Beschreibung]

    Hinzuf"ugen einer Seite zu dem Dialog.
    Der Ridertext wird "ubergeben, die Seite hat keine Entsprechung im
    TabControl in der Resource des Dialogs.
*/

(
    USHORT nId,
    const String& rRiderText,
    CreateTabPage pCreateFunc,
    GetTabPageRanges pRangesFunc,
    BOOL bItemsOnDemand,
    USHORT nPos
)
{
    DBG_ASSERT( TAB_PAGE_NOTFOUND == aTabCtrl.GetPagePos( nId ),
                "Doppelte Page-Ids in der Tabpage" );
    aTabCtrl.InsertPage( nId, rRiderText, nPos );
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}

// -----------------------------------------------------------------------
#ifdef SV_HAS_RIDERBITMAPS

void SfxTabDialog::AddTabPage

/*  [Beschreibung]

    Hinzuf"ugen einer Seite zu dem Dialog.
    Die Riderbitmap wird "ubergeben, die Seite hat keine Entsprechung im
    TabControl in der Resource des Dialogs.
*/

(
    USHORT nId,
    const Bitmap &rRiderBitmap,
    CreateTabPage pCreateFunc,
    GetTabPageRanges pRangesFunc,
    BOOL bItemsOnDemand,
    USHORT nPos
)
{
    DBG_ASSERT( TAB_PAGE_NOTFOUND == aTabCtrl.GetPagePos( nId ),
                "Doppelte Page-Ids in der Tabpage" );
    aTabCtrl.InsertPage( nId, rRiderBitmap, nPos );
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}
#endif

// -----------------------------------------------------------------------

void SfxTabDialog::RemoveTabPage( USHORT nId )

/*  [Beschreibung]

    L"oschen der TabPage mit der ID nId
*/

{
    USHORT nPos = 0;
    aTabCtrl.RemovePage( nId );
    Data_Impl* pDataObject = Find( *pImpl->pData, nId, &nPos );

    if ( pDataObject )
    {
        if ( pDataObject->pTabPage )
        {
            pDataObject->pTabPage->FillUserData();
            String aPageData(pDataObject->pTabPage->GetUserData());
            if ( aPageData.Len() )
            {
#if SUPD<613//MUSTINI
                SfxIniManager* pIniMgr = SFX_APP()->GetAppIniManager();
                pIniMgr->Set( aPageData, SFX_KEY_PAGE, pDataObject->nId);
#endif
            }

            if ( pDataObject->bOnDemand )
                delete (SfxItemSet*)&pDataObject->pTabPage->GetItemSet();
            delete pDataObject->pTabPage;
        }

        delete pDataObject;
        pImpl->pData->Remove( nPos );
    }
    else
    {
        DBG_WARNINGFILE( "TabPage-Id nicht bekannt" );
    }
}

// -----------------------------------------------------------------------

void SfxTabDialog::PageCreated

/*  [Beschreibung]

    Defaultimplemetierung der virtuellen Methode.
    Diese wird unmittelbar nach dem Erzeugen einer Seite gerufen.
    Hier kann der Dialog direkt an der TabPage Methoden rufen.
*/

(
    USHORT,         // Id der erzeugten Seite
    SfxTabPage&     // Referenz auf die erzeugte Seite
)
{
}

// -----------------------------------------------------------------------

SfxItemSet* SfxTabDialog::GetInputSetImpl()

/*  [Beschreibung]

    Abgeleitete Klassen legen ggf. fuer den InputSet neuen Speicher an.
    Dieser mu\s im Destruktor auch wieder freigegeben werden. Dazu mu\s
    diese Methode gerufen werden.
*/

{
    return (SfxItemSet*)pSet;
}

// -----------------------------------------------------------------------

SfxTabPage* SfxTabDialog::GetTabPage( USHORT nPageId ) const

/*  [Beschreibung]

    TabPage mit der "Ubergebenen Id zur"uckgeben.
*/

{
    USHORT nPos = 0;
    Data_Impl* pDataObject = Find( *pImpl->pData, nPageId, &nPos );

    if ( pDataObject )
        return pDataObject->pTabPage;
    return NULL;
}

// -----------------------------------------------------------------------

BOOL SfxTabDialog::IsInOK() const

/*  [Beschreibung]

*/

{
    return pImpl->bInOK;
}

// -----------------------------------------------------------------------

short SfxTabDialog::Ok()

/*  [Beschreibung]

    Ok-Handler des Dialogs
    Das OutputSet wird erstellt und jede Seite wird mit
    dem bzw. ihrem speziellen OutputSet durch Aufruf der Methode
    <SfxTabPage::FillItemSet(SfxItemSet &)> dazu aufgefordert,
    die vom Benuzter eingestellten Daten in das Set zu tun.

    [R"uckgabewert]

    RET_OK: wenn mindestens eine Seite TRUE als Returnwert von
            FillItemSet geliefert hat, sonst RET_CANCEL.
*/

{
    pImpl->bInOK = TRUE;

    if ( !pOutSet )
    {
        if ( !pExampleSet && pSet )
            pOutSet = pSet->Clone( FALSE ); // ohne Items
        else if ( pExampleSet )
            pOutSet = new SfxItemSet( *pExampleSet );
    }
    BOOL bModified = FALSE;

    const USHORT nCount = pImpl->pData->Count();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        Data_Impl* pDataObject = pImpl->pData->GetObject(i);
        SfxTabPage* pTabPage = pDataObject->pTabPage;

        if ( pTabPage )
        {
            if ( pDataObject->bOnDemand )
            {
                SfxItemSet& rSet = (SfxItemSet&)pTabPage->GetItemSet();
                rSet.ClearItem();
                bModified |= pTabPage->FillItemSet( rSet );
            }
            else if ( pSet && !pTabPage->HasExchangeSupport() )
            {
                SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

                if ( pTabPage->FillItemSet( aTmp ) )
                {
                    bModified |= TRUE;
                    pExampleSet->Put( aTmp );
                    pOutSet->Put( aTmp );
                }
            }
        }
    }

    if ( pImpl->bModified || ( pOutSet && pOutSet->Count() > 0 ) )
        bModified |= TRUE;

    if ( bFmt == 2 )
        bModified |= TRUE;
    return bModified ? RET_OK : RET_CANCEL;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, CancelHdl, Button*, pButton )
{
    Close();
    return 0;
}

// -----------------------------------------------------------------------

SfxItemSet* SfxTabDialog::CreateInputItemSet( USHORT )

/*  [Beschreibung]

    Defaultimplemetierung der virtuellen Methode.
    Diese wird gerufen, wenn Pages ihre Sets onDenamd anlegen
*/

{
    DBG_WARNINGFILE( "CreateInputItemSet nicht implementiert" );
    return 0;
}

// -----------------------------------------------------------------------

const SfxItemSet* SfxTabDialog::GetRefreshedSet()

/*  [Beschreibung]

    Defaultimplemetierung der virtuellen Methode.
    Diese wird gerufen, wenn <SfxTabPage::DeactivatePage(SfxItemSet *)>
    <SfxTabPage::REFRESH_SET> liefert.
*/

{
    DBG_ERRORFILE( "GetRefreshedSet nicht implementiert" );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, OkHdl, Button *, EMPTYARG )

/*  [Beschreibung]

    Handler des Ok-Buttons
    Dieser ruft f"ur die aktuelle Seite
    <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    Liefert diese <SfxTabPage::LEAVE_PAGE>, wird <SfxTabDialog::Ok()> gerufen
    und so der Dialog beendet.
*/

{
    pImpl->bInOK = TRUE;

    if ( OK_Impl() )
    {
        if ( pImpl->bModal )
            EndDialog( Ok() );
        else
        {
            Ok();
            Close();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

BOOL SfxTabDialog::PrepareLeaveCurrentPage()
{
    SfxTabPage* pPage =
        (SfxTabPage*)aTabCtrl.GetTabPage( aTabCtrl.GetCurPageId() );

    BOOL bEnd = !pPage;
    if ( pPage )
    {
        int nRet = SfxTabPage::LEAVE_PAGE;
        if ( pSet )
        {
            SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );
            BOOL bRet = FALSE;

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );

            if ( ( SfxTabPage::LEAVE_PAGE & nRet ) == SfxTabPage::LEAVE_PAGE
                 && aTmp.Count() )
            {
                pExampleSet->Put( aTmp );
                pOutSet->Put( aTmp );
            }
            else if ( bRet )
                pImpl->bModified |= TRUE;
        }
        else
            nRet = pPage->DeactivatePage( NULL );
        bEnd = nRet;
    }

    return bEnd;
}


// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, UserHdl, Button *, EMPTYARG )

/*  [Beschreibung]

    Handler des User-Buttons
    Dieser ruft f"ur die aktuelle Seite
    <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    Liefert diese <SfxTabPage::LEAVE_PAGE>, wird <SfxTabDialog::Ok()> gerufen.
    Mit dem Return-Wert von <SfxTabDialog::Ok()> wird dann der Dialog beendet.
*/

{
    SfxTabPage* pPage =
        (SfxTabPage *)aTabCtrl.GetTabPage( aTabCtrl.GetCurPageId() );

    FASTBOOL bEnd = !pPage;

    if ( pPage )
    {
        int nRet = SfxTabPage::LEAVE_PAGE;

        if ( pSet )
        {
            SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );

            if ( ( SfxTabPage::LEAVE_PAGE & nRet ) == SfxTabPage::LEAVE_PAGE
                 && aTmp.Count() )
            {
                pExampleSet->Put( aTmp );
                pOutSet->Put( aTmp );
            }
        }
        else
            nRet = pPage->DeactivatePage( NULL );
        bEnd = nRet;
    }

    if ( bEnd )
    {
        short nRet = Ok();

        if ( RET_OK == nRet )
            nRet = RET_USER;
        else
            nRet = RET_USER_CANCEL;
        EndDialog( nRet );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, ResetHdl, Button *, EMPTYARG )

/*  [Beschreibung]

    Handler hinter dem Zur"ucksetzen-Button.
    Die aktuelle Page wird mit ihren initialen Daten
    neu initialisiert; alle Einstellungen, die der Benutzer
    auf dieser Seite get"atigt hat, werden aufgehoben.
*/

{
    Data_Impl* pDataObject = Find( *pImpl->pData, aTabCtrl.GetCurPageId() );
    DBG_ASSERT( pDataObject, "Id nicht bekannt" );

    if ( pDataObject->bOnDemand )
    {
        // CSet auf AIS hat hier Probleme, daher getrennt
        const SfxItemSet* pSet = &pDataObject->pTabPage->GetItemSet();
        pDataObject->pTabPage->Reset( *(SfxItemSet*)pSet );
    }
    else
        pDataObject->pTabPage->Reset( *pSet );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, BaseFmtHdl, Button *, EMPTYARG )

/*  [Beschreibung]

    Handler hinter dem Standard-Button.
    Dieser Button steht beim Bearbeiten von StyleSheets zur Verf"ugung.
    Alle in dem bearbeiteten StyleSheet eingestellten Attribute
    werden gel"oscht.
*/

{
    const USHORT nId = aTabCtrl.GetCurPageId();
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "Id nicht bekannt" );
    bFmt = 2;

    if ( pDataObject->fnGetRanges )
    {
        if ( !pExampleSet )
            pExampleSet = new SfxItemSet( *pSet );

        const SfxItemPool* pPool = pSet->GetPool();
        const USHORT* pRanges = (pDataObject->fnGetRanges)();
        SfxItemSet aTmpSet( *pExampleSet );

        while ( *pRanges )
        {
            const USHORT* pU = pRanges + 1;

            if ( *pRanges == *pU )
            {
                // Range mit zwei gleichen Werten -> nur ein Item setzen
                USHORT nWh = pPool->GetWhich( *pRanges );
                pExampleSet->ClearItem( nWh );
                aTmpSet.ClearItem( nWh );
                // am OutSet mit InvalidateItem,
                // damit die "Anderung wirksam wird
                pOutSet->InvalidateItem( nWh );
            }
            else
            {
                // richtiger Range mit mehreren Werten
                USHORT nTmp = *pRanges, nTmpEnd = *pU;
                DBG_ASSERT( nTmp <= nTmpEnd, "Range ist falsch sortiert" );

                if ( nTmp > nTmpEnd )
                {
                    // wenn wirklich falsch sortiert, dann neu setzen
                    USHORT nTmp1 = nTmp;
                    nTmp = nTmpEnd;
                    nTmpEnd = nTmp1;
                }

                while ( nTmp <= nTmpEnd )
                {
                    // "uber den Range iterieren, und die Items setzen
                    USHORT nWh = pPool->GetWhich( nTmp );
                    pExampleSet->ClearItem( nWh );
                    aTmpSet.ClearItem( nWh );
                    // am OutSet mit InvalidateItem,
                    // damit die "Anderung wirksam wird
                    pOutSet->InvalidateItem( nWh );
                    nTmp++;
                }
            }
            // zum n"achsten Paar gehen
            pRanges += 2;
        }
        // alle Items neu gesetzt -> dann an der aktuellen Page Reset() rufen
        DBG_ASSERT( pDataObject->pTabPage, "die Page ist weg" );
        pDataObject->pTabPage->Reset( aTmpSet );
        pDataObject->pTabPage->pImpl->_bStandard = TRUE;
    }
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, ActivatePageHdl, TabControl *, pTabCtrl )

/*  [Beschreibung]

    Handler, der vor dem Umschalten auf eine andere Seite
    durch Starview gerufen wird.
    Existiert die Seite noch nicht, so wird sie erzeugt und
    die virtuelle Methode <SfxTabDialog::PageCreated( USHORT, SfxTabPage &)>
    gerufen. Existiert die Seite bereits, so wird ggf.
    <SfxTabPage::Reset(const SfxItemSet &)> oder
    <SfxTabPage::ActivatePage(const SfxItemSet &)> gerufen.
*/

{
    DBG_ASSERT( pImpl->pData->Count(), "keine Pages angemeldet" );
    const USHORT nId = pTabCtrl->GetCurPageId();
    SfxApplication *pSfxApp = SFX_APP();
#if SUPD<613//MUSTINI
    SfxIniManager* pIniMgr = pSfxApp->GetAppIniManager();
    if ( BOOL( pIniMgr->Get( SFX_GROUP_USER, DEFINE_CONST_UNICODE("HelpAuthor") ).ToInt32() ) )
    {
        String aText( DEFINE_CONST_UNICODE("slot:") );
        aText += pSfxApp->Get_Impl()->nExecutingSID;
        aText += 0x0023; // '#' = 23h
        aText += String::CreateFromInt32(nId);
        GetpApp()->ShowStatusText( aText );
        Clipboard::Clear();
        Clipboard::CopyString( aText );
    }
#endif

    // Tab Page schon da?
    SfxTabPage* pTabPage = (SfxTabPage *)pTabCtrl->GetTabPage( nId );
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "Id nicht bekannt" );

    // ggf. TabPage erzeugen:
    if ( !pTabPage )
    {
        const SfxItemSet* pTmpSet = 0;

        if ( pSet )
        {
            if ( bItemsReset && pSet->GetParent() )
                pTmpSet = pSet->GetParent();
            else
                pTmpSet = pSet;
        }

        if ( pTmpSet && !pDataObject->bOnDemand )
            pTabPage = (pDataObject->fnCreatePage)( pTabCtrl, *pTmpSet );
        else
            pTabPage = (pDataObject->fnCreatePage)
                            ( pTabCtrl, *CreateInputItemSet( nId ) );
        DBG_ASSERT( NULL == pDataObject->pTabPage, "TabPage mehrfach erzeugt" );
        pDataObject->pTabPage = pTabPage;
        pDataObject->pTabPage->SetTabDialog( this );
#if SUPD<613//MUSTINI
        pTabPage->SetUserData(pIniMgr->Get( SFX_KEY_PAGE, pDataObject->nId ));
#endif
        Size aSiz = pTabPage->GetSizePixel();
        Size aCtrlSiz = pTabCtrl->GetOutputSizePixel();
        // Gr"o/se am TabControl nur dann setzen, wenn < als TabPage
        if ( aCtrlSiz.Width() < aSiz.Width() ||
             aCtrlSiz.Height() < aSiz.Height() )
            pTabCtrl->SetOutputSizePixel( aSiz );
        PageCreated( nId, *pTabPage );

        if ( pDataObject->bOnDemand )
            pTabPage->Reset( (SfxItemSet &)pTabPage->GetItemSet() );
        else
            pTabPage->Reset( *pSet );
        pTabCtrl->SetTabPage( nId, pTabPage );
    }
    else if ( pDataObject->bRefresh )
        pTabPage->Reset( *pSet );
    pDataObject->bRefresh = FALSE;

    if ( pExampleSet )
        pTabPage->ActivatePage( *pExampleSet );

    SfxHelpPI *pHelpPI = pSfxApp->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->LoadTopic( pTabPage->GetHelpId() );

    BOOL bReadOnly = pTabPage->IsReadOnly();
    ( bReadOnly || pImpl->bHideResetBtn ) ? aResetBtn.Hide() : aResetBtn.Show();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, DeactivatePageHdl, TabControl *, pTabCtrl )

/*  [Beschreibung]

    Handler, der vor dem Verlassen einer Seite durch Starview gerufen wird.

    [Querverweise]

    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
    USHORT nId = pTabCtrl->GetCurPageId();
    SfxApplication *pSfxApp = SFX_APP();
#if SUPD<613//MUSTINI
    SfxIniManager* pIniMgr = pSfxApp->GetAppIniManager();
    if ( BOOL( pIniMgr->Get( SFX_GROUP_USER, DEFINE_CONST_UNICODE("HelpAuthor") ).ToInt32() ) )
        GetpApp()->HideStatusText();
#endif
    SfxTabPage *pPage = (SfxTabPage*)pTabCtrl->GetTabPage( nId );
    DBG_ASSERT( pPage, "keine aktive Page" );
#ifdef DBG_UTIL
    Data_Impl* pDataObject = Find( *pImpl->pData, pTabCtrl->GetCurPageId() );
    DBG_ASSERT( pDataObject, "keine Datenstruktur zur aktuellen Seite" );
    if ( pPage->HasExchangeSupport() && pDataObject->bOnDemand )
    {
        DBG_WARNING( "Datenaustausch bei ItemsOnDemand ist nicht gewuenscht!" );
    }
#endif

    int nRet = SfxTabPage::LEAVE_PAGE;

    if ( !pExampleSet && pPage->HasExchangeSupport() && pSet )
        pExampleSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );

    if ( pSet )
    {
        SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

        if ( pPage->HasExchangeSupport() )
            nRet = pPage->DeactivatePage( &aTmp );
//!     else
//!         pPage->FillItemSet( aTmp );

        if ( ( SfxTabPage::LEAVE_PAGE & nRet ) == SfxTabPage::LEAVE_PAGE &&
             aTmp.Count() )
        {
            pExampleSet->Put( aTmp );
            pOutSet->Put( aTmp );
        }
    }
    else
    {
        if ( pPage->HasExchangeSupport() ) //!!!
        {
            if ( !pExampleSet )
            {
                SfxItemPool* pPool = pPage->GetItemSet().GetPool();
                pExampleSet =
                    new SfxItemSet( *pPool, GetInputRanges( *pPool ) );
            }
            nRet = pPage->DeactivatePage( pExampleSet );
        }
        else
            nRet = pPage->DeactivatePage( NULL );
    }

    if ( nRet & SfxTabPage::REFRESH_SET )
    {
        pSet = GetRefreshedSet();
        DBG_ASSERT( pSet, "GetRefreshedSet() liefert NULL" );
        // alle Pages als neu zu initialsieren flaggen
        const USHORT nCount = pImpl->pData->Count();

        for ( USHORT i = 0; i < nCount; ++i )
        {
            Data_Impl* pObj = (*pImpl->pData)[i];

            if ( pObj->pTabPage != pPage ) // eigene Page nicht mehr refreshen
                pObj->bRefresh = TRUE;
            else
                pObj->bRefresh = FALSE;
        }
    }
    if ( nRet & SfxTabPage::LEAVE_PAGE )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

const SfxItemSet* SfxTabDialog::GetOutputItemSet

/*  [Beschreibung]

    Liefert die Pages, die ihre Sets onDemand liefern, das OutputItemSet.

    [Querverweise]

    <SfxTabDialog::AddTabPage(USHORT, CreateTabPage, GetTabPageRanges, BOOL)>
    <SfxTabDialog::AddTabPage(USHORT, const String &, CreateTabPage, GetTabPageRanges, BOOL, USHORT)>
    <SfxTabDialog::AddTabPage(USHORT, const Bitmap &, CreateTabPage, GetTabPageRanges, BOOL, USHORT)>
*/

(
    USHORT nId  // die Id, unter der die Seite bei AddTabPage()
                // hinzugef"ugt wurde.
) const
{
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "TabPage nicht gefunden" );

    if ( pDataObject )
    {
        if ( !pDataObject->pTabPage )
            return NULL;

        if ( pDataObject->bOnDemand )
            return &pDataObject->pTabPage->GetItemSet();
        // else
        return pOutSet;
    }
    return NULL;
}

// -----------------------------------------------------------------------

int SfxTabDialog::FillOutputItemSet()
{
    int nRet = SfxTabPage::LEAVE_PAGE;
    if ( OK_Impl() )
        Ok();
    else
        nRet = SfxTabPage::KEEP_PAGE;
    return nRet;
}

// -----------------------------------------------------------------------

#ifdef WNT
int __cdecl TabDlgCmpUS_Impl( const void* p1, const void* p2 )
#else
#if defined(OS2) && defined(ICC)
int _Optlink TabDlgCmpUS_Impl( const void* p1, const void* p2 )
#else
int TabDlgCmpUS_Impl( const void* p1, const void* p2 )
#endif
#endif

/*  [Beschreibung]

    Vergleichsfunktion f"ur qsort
*/

{
    return *(USHORT*)p1 - *(USHORT*)p2;
}

// -----------------------------------------------------------------------

void SfxTabDialog::ShowPage( USHORT nId )

/*  [Beschreibung]

    Es wird die TabPage mit der "ubergebenen Id aktiviert.
*/

{
    aTabCtrl.SetCurPageId( nId );
    ActivatePageHdl( &aTabCtrl );
}

// -----------------------------------------------------------------------

const USHORT* SfxTabDialog::GetInputRanges( const SfxItemPool& rPool )

/*  [Beschreibung]

    Bildet das Set "uber die Ranges aller Seiten des Dialogs.
    Die Pages m"ussen die statische Methode f"ur das Erfragen ihrer
    Ranges bei AddTabPage angegeben haben, liefern also ihre Sets onDemand.

    [Querverweise]

    <SfxTabDialog::AddTabPage(USHORT, CreateTabPage, GetTabPageRanges, BOOL)>
    <SfxTabDialog::AddTabPage(USHORT, const String &, CreateTabPage, GetTabPageRanges, BOOL, USHORT)>
    <SfxTabDialog::AddTabPage(USHORT, const Bitmap &, CreateTabPage, GetTabPageRanges, BOOL, USHORT)>

    [R"uckgabewert]

    Pointer auf nullterminiertes Array von USHORTs
    Dieses Array geh"ort dem Dialog und wird beim
    Zerst"oren des Dialogs gel"oscht.
*/

{
    if ( pSet )
    {
        DBG_ERRORFILE( "Set bereits vorhanden!" );
        return pSet->GetRanges();
    }

    if ( pRanges )
        return pRanges;
    SvUShorts aUS( 16, 16 );
    USHORT nCount = pImpl->pData->Count();

    USHORT i;
    for ( i = 0; i < nCount; ++i )
    {
        Data_Impl* pDataObject = pImpl->pData->GetObject(i);

        if ( pDataObject->fnGetRanges )
        {
            const USHORT* pTmpRanges = (pDataObject->fnGetRanges)();
            const USHORT* pIter = pTmpRanges;

            USHORT nLen;
            for( nLen = 0; *pIter; ++nLen, ++pIter )
                ;
            aUS.Insert( pTmpRanges, nLen, aUS.Count() );
        }
    }

    //! Doppelte Ids entfernen?
#ifndef TF_POOLABLE
    if ( rPool.HasMap() )
#endif
    {
        nCount = aUS.Count();

        for ( i = 0; i < nCount; ++i )
            aUS[i] = rPool.GetWhich( aUS[i] );
    }

    // sortieren
    if ( aUS.Count() > 1 )
        qsort( (void*)aUS.GetData(),
               aUS.Count(), sizeof(USHORT), TabDlgCmpUS_Impl );

    // Ranges erzeugen
    //!! Auskommentiert, da fehlerhaft
    /*
    pRanges = new USHORT[aUS.Count() * 2 + 1];
    int j = 0;
    i = 0;

    while ( i < aUS.Count() )
    {
        pRanges[j++] = aUS[i];
        // aufeinanderfolgende Zahlen
        for( ; i < aUS.Count()-1; ++i )
            if ( aUS[i] + 1 != aUS[i+1] )
                break;
        pRanges[j++] = aUS[i++];
    }
    pRanges[j] = 0;     // terminierende NULL
    */

    pRanges = new USHORT[aUS.Count() + 1];
    memcpy(pRanges, aUS.GetData(), sizeof(USHORT) * aUS.Count());
    pRanges[aUS.Count()] = 0;
    return pRanges;
}

// -----------------------------------------------------------------------

void SfxTabDialog::SetInputSet( const SfxItemSet* pInSet )

/*  [Beschreibung]

    Mit dieser Methode kann nachtr"aglich der Input-Set initial oder
    neu gesetzt werden.
*/

{
    FASTBOOL bSet = ( pSet != NULL );

    pSet = pInSet;

    if ( !bSet && !pExampleSet && !pOutSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }
}


