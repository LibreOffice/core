/*************************************************************************
 *
 *  $RCSfile: navipi.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#ifndef SC_RANGELST_HXX
#include <rangelst.hxx>
#endif

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/navigat.hxx>
#include <svtools/stritem.hxx>
#include <svtools/urlbmk.hxx>
#include <vcl/drag.hxx>
#include <vcl/sound.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>

#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "dbcolect.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "popmenu.hxx"
#include "scresid.hxx"
#include "scmod.hxx"
#include "navicfg.hxx"
#include "navcitem.hxx"
#include "navipi.hrc"
#include "navipi.hxx"



//  Timeout, um Notizen zu suchen
#define SC_CONTENT_TIMEOUT  1000

//  Toleranz, wieviel ueber der eingeklappten Groesse noch klein ist
#define SCNAV_MINTOL        5

//------------------------------------------------------------------------

//  static
void ScNavigatorDlg::ReleaseFocus()
{
    SfxViewShell* pCurSh = SfxViewShell::Current();

    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();
        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

//==================================================================
//  class ColumnEdit
//==================================================================

ColumnEdit::ColumnEdit( ScNavigatorDlg* pParent, const ResId& rResId )
    :   SpinField   ( pParent, rResId ),
        rDlg        ( *pParent ),
        nKeyGroup   ( KEYGROUP_ALPHA ),
        nCol        ( 0 )
{
    SetMaxTextLen( 3 ); // 1 bis 256 bzw. A bis IV
}

//------------------------------------------------------------------------

__EXPORT ColumnEdit::~ColumnEdit()
{
}

//------------------------------------------------------------------------

#ifdef VCL

long __EXPORT ColumnEdit::Notify( NotifyEvent& rNEvt )
{
    SpinField::Notify( rNEvt );
    long nHandled = 0;

    USHORT nType = rNEvt.GetType();
    if ( nType == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        KeyCode aCode = pKEvt->GetKeyCode();

        if ( !aCode.IsMod1() && !aCode.IsMod2() )
        {
            //! Eingabeueberpruefung (nur Zahlen oder nur Buchstaben, max 2 bzw 3 Stellen)
            //! war vor VCL per nicht weitergeleitetem KeyInput
            //! dafuer was neues ausdenken!!!

            if ( aCode.GetCode() == KEY_RETURN )
            {
                ScNavigatorDlg::ReleaseFocus();
                ExecuteCol();
                nHandled = 1;
            }
        }
    }
    else if ( nType == EVENT_LOSEFOCUS )    // LoseFocus wird bei VCL nicht gerufen
        EvalText();                         // nCol setzen

    return nHandled;
}

#else

void __EXPORT ColumnEdit::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode   = rKEvt.GetKeyCode();
    USHORT  nGroup  = aCode.GetGroup();
    USHORT  nLen    = GetText().Len();
    BOOL    bSel    = (GetSelection().Len() > 0);

    if ( aCode.IsMod1() || aCode.IsMod2() )
    {
        SpinField::KeyInput( rKEvt );
    }
    else if ( aCode == KEY_RETURN )
    {
        ScNavigatorDlg::ReleaseFocus();
        ExecuteCol();
    }
    else if ( nGroup == KEYGROUP_NUM || nGroup ==  KEYGROUP_ALPHA )
    {
        if ( nLen == 0 || bSel ) // neue Eingabe: nur KeyGroup merken
        {
            nKeyGroup = nGroup;
            ReplaceSelected( rKEvt.GetCharCode() );
        }
        else
        {
            // Abhaengig von aktueller Gruppe und Laenge Eingabe zulassen

            if ( nKeyGroup == KEYGROUP_NUM && nGroup == KEYGROUP_NUM )
            {
                if ( nLen < 3 )
                    SpinField::KeyInput( rKEvt );
            }
            else if ( nKeyGroup == KEYGROUP_ALPHA && nGroup == KEYGROUP_ALPHA )
            {
                if ( nLen < 2 )
                    SpinField::KeyInput( rKEvt );
            }
        }
    }
    else
        SpinField::KeyInput( rKEvt );
}

#endif

//------------------------------------------------------------------------

void __EXPORT ColumnEdit::LoseFocus()
{
    EvalText();
}


//------------------------------------------------------------------------

void __EXPORT ColumnEdit::Up()
{
    nCol += 1;

#ifdef OS2
    if ( nCol > 256 )
        nCol = 1;
#endif

    if ( nCol <= 256 )
        SetCol( nCol );
    else
        nCol -= 1;
}

//------------------------------------------------------------------------

void __EXPORT ColumnEdit::Down()
{
    if ( nCol>1 )
        SetCol( nCol-1 );
#ifdef OS2
    else
        SetCol( 256 );
#endif
}

//------------------------------------------------------------------------

void __EXPORT ColumnEdit::First()
{
    nCol=1;
    SetText( 'A' );
}

//------------------------------------------------------------------------

void __EXPORT ColumnEdit::Last()
{
    nCol=256;
    SetText( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("IV")) );
}


//------------------------------------------------------------------------

void ColumnEdit::EvalText()
{
    String aStrCol = GetText();

    if ( aStrCol.Len() > 0 )
    {
        //  nKeyGroup wird bei VCL mangels KeyInput nicht mehr gesetzt

        if ( CharClass::isAsciiNumeric(aStrCol) )
            nCol = NumStrToAlpha( aStrCol );
        else
            nCol = AlphaToNum( aStrCol );
    }
    else
        nCol = 0;

    SetText( aStrCol );
    nKeyGroup = KEYGROUP_ALPHA;
}

//------------------------------------------------------------------------

void ColumnEdit::ExecuteCol()
{
    USHORT nRow = rDlg.aEdRow.GetRow();

    EvalText(); // setzt nCol

    if ( (nCol > 0) && (nRow > 0) )
        rDlg.SetCurrentCell( nCol-1, nRow-1 );
}

//------------------------------------------------------------------------

void ColumnEdit::SetCol( USHORT nColNo )
{
    String aStr;

    if ( nColNo == 0 )
    {
        nCol = 0;
        SetText( aStr );
    }
    else
    {
        nColNo = NumToAlpha( nColNo, aStr );
        nCol = nColNo;
        SetText( aStr );
    }
}

//------------------------------------------------------------------------

USHORT ColumnEdit::AlphaToNum( String& rStr )
{
    USHORT  nColumn = 0;

    ByteString aByteStr( rStr, RTL_TEXTENCODING_ASCII_US );
    if ( aByteStr.IsAlphaAscii() )
    {
        rStr.ToUpperAscii();
        aByteStr.ToUpperAscii();

        if ( aByteStr.Len() == 1 )
        {
            nColumn = (( (aByteStr.GetChar(0)) ) - 'A' ) + 1;
        }
        else if ( rStr.Len() == 2 )
        {
            nColumn =  (((rStr.GetChar(0) - 'A') + 1) * 26)
                     + (rStr.GetChar(1) - 'A') + 1;
            if ( nColumn > 256 )
            {
                nColumn = 256;
                rStr.AssignAscii(RTL_CONSTASCII_STRINGPARAM( "IV" ));
            }
        }
        else
            rStr.Erase();
    }

    return nColumn;
}

//------------------------------------------------------------------------

USHORT ColumnEdit::NumStrToAlpha( String& rStr )
{
    USHORT  nColumn = 0;

    if ( CharClass::isAsciiNumeric(rStr) )
        nColumn = NumToAlpha( rStr.ToInt32(), rStr );
    else
        rStr.Erase();

    return nColumn;
}

//------------------------------------------------------------------------

USHORT ColumnEdit::NumToAlpha( USHORT nColNo, String& rStr )
{
    if ( nColNo > 256 )
    {
        rStr.AssignAscii(RTL_CONSTASCII_STRINGPARAM( "IV" ));
        nColNo = 256;
    }
    else if ( nColNo == 0 )
    {
        rStr = 'A';
        nColNo = 1;
    }
    else
    {
        nColNo--;
        if ( nColNo < 26 )
            rStr = (sal_Unicode) ( 'A' + nColNo );
        else
        {
            rStr  = (sal_Unicode) ( 'A' + ( nColNo / 26 ) - 1 );
            rStr += (sal_Unicode) ( 'A' + ( nColNo % 26 ) );
        }
        nColNo++;
    }

    return nColNo;
}

//==================================================================
//  class RowEdit
//==================================================================

RowEdit::RowEdit( ScNavigatorDlg* pParent, const ResId& rResId )
    :   NumericField( pParent, rResId ),
        rDlg        ( *pParent )
{
}

//------------------------------------------------------------------------

__EXPORT RowEdit::~RowEdit()
{
}

//------------------------------------------------------------------------

#ifdef VCL

long __EXPORT RowEdit::Notify( NotifyEvent& rNEvt )
{
    NumericField::Notify( rNEvt );
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        KeyCode aCode = pKEvt->GetKeyCode();
        if ( aCode.GetCode() == KEY_RETURN && !aCode.IsMod1() && !aCode.IsMod2() )
        {
            ScNavigatorDlg::ReleaseFocus();
            ExecuteRow();
            nHandled = 1;
        }
    }

    return nHandled;
}

#else

void __EXPORT RowEdit::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();

    if ( aCode.IsMod1() || aCode.IsMod2() || aCode != KEY_RETURN )
    {
        NumericField::KeyInput( rKEvt );
    }
    else // if ( rKEvt.GetKeyCode() == KEY_RETURN )
    {
        ScNavigatorDlg::ReleaseFocus();
        ExecuteRow();
    }
}

#endif

//------------------------------------------------------------------------

void __EXPORT RowEdit::LoseFocus()
{
}

//------------------------------------------------------------------------

void RowEdit::ExecuteRow()
{
    USHORT nCol = rDlg.aEdCol.GetCol();
    USHORT nRow = (USHORT)GetValue();

    if ( (nCol > 0) && (nRow > 0) )
        rDlg.SetCurrentCell( nCol-1, nRow-1 );
}

//==================================================================
//  class ScDocListBox
//==================================================================

ScDocListBox::ScDocListBox( ScNavigatorDlg* pParent, const ResId& rResId )
    :   ListBox ( pParent, rResId ),
        rDlg    ( *pParent )
{
}

//------------------------------------------------------------------------

__EXPORT ScDocListBox::~ScDocListBox()
{
}

//------------------------------------------------------------------------

void __EXPORT ScDocListBox::Select()
{
    ScNavigatorDlg::ReleaseFocus();

    String aDocName = GetSelectEntry();
    rDlg.aLbEntries.SelectDoc( aDocName );
}

//==================================================================
//  class CommandToolBox
//==================================================================

CommandToolBox::CommandToolBox( ScNavigatorDlg* pParent, const ResId& rResId )
    :   ToolBox ( pParent, rResId ),
        rDlg    ( *pParent )
{
    //  #52973# grosse Images haben wir nicht, darum nur fuer CHANGEOUTSTYLE anmelden
    SFX_IMAGEMANAGER()->RegisterToolBox( this, SFX_TOOLBOX_CHANGEOUTSTYLE );

    SetSizePixel( CalcWindowSizePixel() );
//  EnableItem( IID_UP, FALSE );
//  EnableItem( IID_DOWN, FALSE );
}

//------------------------------------------------------------------------

__EXPORT CommandToolBox::~CommandToolBox()
{
    SFX_IMAGEMANAGER()->ReleaseToolBox( this );
}

//------------------------------------------------------------------------

void CommandToolBox::Select( USHORT nSelId )
{
    //  Modus umschalten ?

    if ( nSelId == IID_ZOOMOUT || nSelId == IID_SCENARIOS )
    {
        NavListMode eOldMode = rDlg.eListMode;
        NavListMode eNewMode = eOldMode;

        if ( nSelId == IID_SCENARIOS )                  // auf Szenario
        {
            if ( eOldMode == NAV_LMODE_SCENARIOS )
                eNewMode = NAV_LMODE_AREAS;
            else
                eNewMode = NAV_LMODE_SCENARIOS;
        }
        else                                            // ein/aus
        {
            if ( eOldMode == NAV_LMODE_NONE )
                eNewMode = NAV_LMODE_AREAS;
            else
                eNewMode = NAV_LMODE_NONE;
        }
        rDlg.SetListMode( eNewMode );
        UpdateButtons();
    }
    else
        switch ( nSelId )
        {
            case IID_DATA:
                rDlg.MarkDataArea();
                break;
            case IID_UP:
                rDlg.StartOfDataArea();
                break;
            case IID_DOWN:
                rDlg.EndOfDataArea();
                break;
            // IID_DROPMODE ist in Click
            case IID_CHANGEROOT:
                rDlg.aLbEntries.ToggleRoot();
                UpdateButtons();
                break;
        }
}

void __EXPORT CommandToolBox::Select()
{
    Select( GetCurItemId() );
}

//------------------------------------------------------------------------

void __EXPORT CommandToolBox::Click()
{
    //  Das Popupmenue fuer den Dropmodus muss im Click (Button Down)
    //  statt im Select (Button Up) aufgerufen werden.

    if ( GetCurItemId() == IID_DROPMODE )
    {
        Point aMenuPos = GetItemRect(IID_DROPMODE).BottomLeft();

        ScPopupMenu aPop( ScResId( RID_POPUP_DROPMODE ) );
        aPop.CheckItem( RID_DROPMODE_URL + rDlg.GetDropMode() );
        aPop.Execute( this, aMenuPos );
        USHORT nId = aPop.GetSelected();

        EndSelection();     // vor SetDropMode (SetDropMode ruft SetItemImage)

        if ( nId >= RID_DROPMODE_URL && nId <= RID_DROPMODE_COPY )
            rDlg.SetDropMode( nId - RID_DROPMODE_URL );

        //  #49956# den gehighlighteten Button aufheben
        Point aPoint;
        MouseEvent aLeave( aPoint, 0, MOUSE_LEAVEWINDOW | MOUSE_SYNTHETIC );
        MouseMove( aLeave );
    }
}

//------------------------------------------------------------------------

void CommandToolBox::UpdateButtons()
{
    NavListMode eMode = rDlg.eListMode;
    CheckItem( IID_SCENARIOS,   eMode == NAV_LMODE_SCENARIOS );
    CheckItem( IID_ZOOMOUT,     eMode != NAV_LMODE_NONE );

    //  Umschalten-Button:
    if ( eMode == NAV_LMODE_SCENARIOS || eMode == NAV_LMODE_NONE )
    {
        EnableItem( IID_CHANGEROOT, FALSE );
        CheckItem( IID_CHANGEROOT, FALSE );
    }
    else
    {
        EnableItem( IID_CHANGEROOT, TRUE );
        BOOL bRootSet = rDlg.aLbEntries.GetRootType() != SC_CONTENT_ROOT;
        CheckItem( IID_CHANGEROOT, bRootSet );
    }

    USHORT nImageId = 0;
    switch ( rDlg.nDropMode )
    {
        case SC_DROPMODE_URL:   nImageId = RID_IMG_DROP_URL;    break;
        case SC_DROPMODE_LINK:  nImageId = RID_IMG_DROP_LINK;   break;
        case SC_DROPMODE_COPY:  nImageId = RID_IMG_DROP_COPY;   break;
    }
    SetItemImage( IID_DROPMODE, Image(ScResId(nImageId)) );
}

//==================================================================
//  class ScNavigatorDlgWrapper
//==================================================================

SFX_IMPL_CHILDWINDOW_CONTEXT( ScNavigatorDialogWrapper, SID_NAVIGATOR, ScTabViewShell )

#define IS_MODE(bit)(((nFlags)&(bit))==(bit))

ScNavigatorDialogWrapper::ScNavigatorDialogWrapper(
                                    Window*          pParent,
                                    USHORT           nId,
                                    SfxBindings*     pBind,
                                    SfxChildWinInfo* pInfo ) :
        SfxChildWindowContext( nId )
{
    pNavigator = new ScNavigatorDlg( pBind, this, pParent );
    SetWindow( pNavigator );

    //  Einstellungen muessen anderswo gemerkt werden,
    //  pInfo geht uns (ausser der Groesse) nichts mehr an

    Size aInfoSize = pParent->GetOutputSizePixel();     // von aussen vorgegebene Groesse
    Size aNavSize = pNavigator->GetOutputSizePixel();   // Default-Groesse

    aNavSize.Width()  = Max( aInfoSize.Width(),  aNavSize.Width() );
    aNavSize.Height() = Max( aInfoSize.Height(), aNavSize.Height() );
    pNavigator->nListModeHeight = Max( aNavSize.Height(), pNavigator->nListModeHeight );

    //  Die Groesse kann in einem anderen Modul geaendert worden sein,
    //  deshalb muessen in Abhaengigkeit von der momentanen Groesse die
    //  Inhalte eingeblendet werden oder nicht

    BOOL bSmall = ( aInfoSize.Height() <= pNavigator->aInitSize.Height() + SCNAV_MINTOL );
    NavListMode eNavMode = NAV_LMODE_NONE;
    if (!bSmall)
    {
        //  wenn Szenario aktiv war, wieder einschalten

        ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
        NavListMode eLastMode = (NavListMode) rCfg.GetListMode();
        if ( eLastMode == NAV_LMODE_SCENARIOS )
            eNavMode = NAV_LMODE_SCENARIOS;
        else
            eNavMode = NAV_LMODE_AREAS;
    }

    //  Die Groesse des Floats nicht neu setzen (FALSE bei SetListMode), damit der
    //  Navigator nicht aufgeklappt wird, wenn er minimiert war (#38872#).

    pNavigator->SetListMode( eNavMode, FALSE );     // FALSE: Groesse des Float nicht setzen

    USHORT nCmdId = 0;
    switch (eNavMode)
    {
        case NAV_LMODE_DOCS:        nCmdId = IID_DOCS;      break;
        case NAV_LMODE_AREAS:       nCmdId = IID_AREAS;     break;
        case NAV_LMODE_DBAREAS:     nCmdId = IID_DBAREAS;   break;
        case NAV_LMODE_SCENARIOS:   nCmdId = IID_SCENARIOS; break;
    }
    if (nCmdId)
    {
        pNavigator->aTbxCmd.CheckItem( nCmdId );
        pNavigator->DoResize();
    }

    pNavigator->bFirstBig = ( nCmdId == 0 );    // dann spaeter

/*???
    FloatingWindow* pFloat = GetFloatingWindow();
    if ( pFloat )
        pFloat->SetMinOutputSizePixel( pNavigator->GetMinOutputSizePixel() );
*/

//!?    pNavigator->Show();
}

void __EXPORT ScNavigatorDialogWrapper::Resizing( Size& rSize )
{
    ((ScNavigatorDlg*)GetWindow())->Resizing(rSize);
}

BOOL __EXPORT ScNavigatorDialogWrapper::Drop( const DropEvent& rEvt )
{
    return GetWindow()->Drop(rEvt);
}

BOOL __EXPORT ScNavigatorDialogWrapper::QueryDrop( DropEvent& rEvt )
{
    return GetWindow()->QueryDrop(rEvt);
}

//========================================================================
// class ScNavigatorPI
//========================================================================

#define CTRL_ITEMS 4

#define REGISTER_SLOT(i,id) \
    ppBoundItems[i]=new ScNavigatorControllerItem(id,*this,rBindings);

ScNavigatorDlg::ScNavigatorDlg( SfxBindings* pB, SfxChildWindowContext* pCW, Window* pParent ) :
        Window( pParent, ScResId(RID_SCDLG_NAVIGATOR) ),
        aFtCol      ( this, ScResId( FT_COL ) ),
        aEdCol      ( this, ScResId( ED_COL ) ),
        aFtRow      ( this, ScResId( FT_ROW ) ),
        aEdRow      ( this, ScResId( ED_ROW ) ),
        aTbxCmd     ( this, ScResId( TBX_CMD ) ),
        aLbEntries  ( this, ScResId( LB_ENTRIES ) ),
        aLbDocuments( this, ScResId( LB_DOCUMENTS ) ),
        aWndScenarios( this,ScResId( STR_QHLP_SCEN_LISTBOX), ScResId(STR_QHLP_SCEN_COMMENT)),
        aStrDragMode ( ScResId( STR_DRAGMODE ) ),
        aStrDisplay  ( ScResId( STR_DISPLAY ) ),
        aStrActiveWin( ScResId( STR_ACTIVEWIN ) ),
        pContextWin ( pCW ),
        eListMode   ( NAV_LMODE_NONE ),
        nDropMode   ( SC_DROPMODE_URL ),
        rBindings   ( *pB ),
        nListModeHeight( 0 ),
        nInitListHeight( 0 ),
        pViewData   ( NULL ),
        pMarkArea   ( NULL ),
        nCurCol     ( 0 ),
        nCurRow     ( 0 ),
        nCurTab     ( 0 ),
        bFirstBig   ( FALSE )
{
    ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
    nDropMode = rCfg.GetDragMode();
    //  eListMode wird von aussen gesetzt, Root weiter unten

    aLbDocuments.SetDropDownLineCount(9);
    String aOpen = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
    aStrActive = aOpen;
    aStrActive += String( ScResId( STR_ACTIVE ) );
    aStrActive += ')';                                      // " (aktiv)"
    aStrNotActive = aOpen;
    aStrNotActive += String( ScResId( STR_NOTACTIVE ) );
    aStrNotActive += ')';                                   // " (inaktiv)"
    aStrHidden = aOpen;
    aStrHidden += String( ScResId( STR_HIDDEN ) );
    aStrHidden += ')';                                      // " (versteckt)"

    aTitleBase = GetText();

    Size aSize( GetOutputSizePixel() );

    nBorderOffset = aLbEntries.GetPosPixel().X();

    aInitSize.Width()  =  aTbxCmd.GetPosPixel().X()
                        + aTbxCmd.GetSizePixel().Width()
                        + nBorderOffset;
    aInitSize.Height() = aLbEntries.GetPosPixel().Y();

    nInitListHeight = aLbEntries.GetSizePixel().Height();
    nListModeHeight =  aInitSize.Height()
                     + nInitListHeight;

    //  kein Resize, eh der ganze Kontext-Kram initialisiert ist!
//  SetOutputSizePixel( aInitSize );        //???
/*! FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
    if ( pFloat)
        pFloat->SetMinOutputSizePixel( aInitSize );
*/
    ppBoundItems = new ScNavigatorControllerItem* [CTRL_ITEMS];

    rBindings.ENTERREGISTRATIONS();
    //-----------------------------
    REGISTER_SLOT( 0, SID_CURRENTCELL       );
    REGISTER_SLOT( 1, SID_CURRENTTAB        );
    REGISTER_SLOT( 2, SID_CURRENTDOC        );
    REGISTER_SLOT( 3, SID_SELECT_SCENARIO   );
    //-----------------------------
    rBindings.LEAVEREGISTRATIONS();

    StartListening( *(SFX_APP()) );
    StartListening( SFX_BINDINGS() );

    aLbDocuments.Hide();        // bei NAV_LMODE_NONE gibts die nicht

    aLbEntries.InitWindowBits(TRUE);

    aLbEntries.SetSpaceBetweenEntries(0);
    aLbEntries.SetSelectionMode( SINGLE_SELECTION );
    aLbEntries.SetDragDropMode(     SV_DRAGDROP_CTRL_MOVE |
                                    SV_DRAGDROP_CTRL_COPY |
                                    SV_DRAGDROP_ENABLE_TOP );

    //  war eine Kategorie als Root ausgewaehlt?
    USHORT nLastRoot = rCfg.GetRootType();
    if ( nLastRoot )
        aLbEntries.SetRootType( nLastRoot );

    aLbEntries.Refresh();
    GetDocNames();

    aTbxCmd.UpdateButtons();

    UpdateColumn();
    UpdateRow();
    UpdateTable();
    aLbEntries.Hide();
    aWndScenarios.Hide();
    aWndScenarios.SetPosPixel( aLbEntries.GetPosPixel() );

    aContentTimer.SetTimeoutHdl( LINK( this, ScNavigatorDlg, TimeHdl ) );
    aContentTimer.SetTimeout( SC_CONTENT_TIMEOUT );

    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScNavigatorDlg::~ScNavigatorDlg()
{
    aContentTimer.Stop();

    USHORT i;
    for ( i=0; i<CTRL_ITEMS; i++ )
        delete ppBoundItems[i];

    delete [] ppBoundItems;
    delete pMarkArea;

    EndListening( *(SFX_APP()) );
    EndListening( SFX_BINDINGS() );
}

//------------------------------------------------------------------------

void __EXPORT ScNavigatorDlg::Resizing( Size& rNewSize )  // Size = Outputsize?
{
    FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
    if ( pFloat )
    {
        Size aMinOut = pFloat->GetMinOutputSizePixel();

        if ( rNewSize.Width() < aMinOut.Width() )
            rNewSize.Width() = aMinOut.Width();

        if ( eListMode == NAV_LMODE_NONE )
            rNewSize.Height() = aInitSize.Height();
        else
        {
            if ( rNewSize.Height() < aMinOut.Height() )
                rNewSize.Height() = aMinOut.Height();
        }
    }
//  else
//      SfxDockingWindow::Resizing(rNewSize);
}



void ScNavigatorDlg::Paint( const Rectangle& rRec )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetFaceColor();
    Wallpaper aBack( aBgColor );

    SetBackground( aBack );
    aFtCol.SetBackground( aBack );
    aFtRow.SetBackground( aBack );

    Window::Paint( rRec );
}

//------------------------------------------------------------------------

void __EXPORT ScNavigatorDlg::Resize()
{
    DoResize();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::DoResize()
{
    Size aNewSize = GetOutputSizePixel();
    long nTotalHeight = aNewSize.Height();

    //  #41403# bei angedocktem Navigator wird das Fenster evtl. erst klein erzeugt,
    //  dann kommt ein Resize auf die wirkliche Groesse -> dann Inhalte einschalten

    BOOL bSmall = ( nTotalHeight <= aInitSize.Height() + SCNAV_MINTOL );
    if ( !bSmall && bFirstBig )
    {
        //  Inhalte laut Config wieder einschalten

        bFirstBig = FALSE;
        NavListMode eNavMode = NAV_LMODE_AREAS;
        ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
        NavListMode eLastMode = (NavListMode) rCfg.GetListMode();
        if ( eLastMode == NAV_LMODE_SCENARIOS )
            eNavMode = NAV_LMODE_SCENARIOS;
        SetListMode( eNavMode, FALSE );         // FALSE: Groesse des Float nicht setzen
    }

    //  auch wenn die Inhalte nicht sichtbar sind, die Groessen anpassen,
    //  damit die Breite stimmt

    //@@ 03.11.97 changes begin
    Point aEntryPos = aLbEntries.GetPosPixel();
    Point aListPos = aLbDocuments.GetPosPixel();
    aNewSize.Width() -= 2*nBorderOffset;
    Size aDocSize = aLbDocuments.GetSizePixel();
    aDocSize.Width() = aNewSize.Width();

    if(!bSmall)
    {

        long nListHeight = aLbDocuments.GetSizePixel().Height();
        aNewSize.Height() -= ( aEntryPos.Y() + nListHeight + 2*nBorderOffset );
        if(aNewSize.Height()<0) aNewSize.Height()=0;

        aListPos.Y() = aEntryPos.Y() + aNewSize.Height() + nBorderOffset;

        if(aListPos.Y() > aLbEntries.GetPosPixel().Y())
                            aLbDocuments.SetPosPixel( aListPos );

    }
    aLbEntries.SetSizePixel( aNewSize );
    aWndScenarios.SetSizePixel( aNewSize );
    aLbDocuments.SetSizePixel( aDocSize );

    //@@ 03.11.97 end

    BOOL bListMode = (eListMode != NAV_LMODE_NONE);
    FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
    if ( pFloat && bListMode )
        nListModeHeight = nTotalHeight;
}

//------------------------------------------------------------------------

void __EXPORT ScNavigatorDlg::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                      const SfxHint& rHint, const TypeId& rHintType )
{
    if ( rHint.ISA(SfxSimpleHint) )
    {
        ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();

        if ( nHintId == SC_HINT_DOCNAME_CHANGED )
        {
            aLbEntries.ActiveDocChanged();
        }
        else if ( NAV_LMODE_NONE == eListMode )
        {
            //  Tabellen hier nicht mehr
        }
        else
        {
            switch ( nHintId )
            {
                case SC_HINT_TABLES_CHANGED:
                    aLbEntries.Refresh( SC_CONTENT_TABLE );
                    break;

                case SC_HINT_DBAREAS_CHANGED:
                    aLbEntries.Refresh( SC_CONTENT_DBAREA );
                    break;

                case SC_HINT_AREAS_CHANGED:
                    aLbEntries.Refresh( SC_CONTENT_RANGENAME );
                    break;

                case SC_HINT_DRAW_CHANGED:
                    aLbEntries.Refresh( SC_CONTENT_GRAPHIC );
                    aLbEntries.Refresh( SC_CONTENT_OLEOBJECT );
                    break;

                case SC_HINT_AREALINKS_CHANGED:
                    aLbEntries.Refresh( SC_CONTENT_AREALINK );
                    break;

                //  SFX_HINT_DOCCHANGED kommt nicht nur bei Dokument-Wechsel

                case SC_HINT_NAVIGATOR_UPDATEALL:
                    UpdateAll();
                    break;

                case FID_DATACHANGED:
                case FID_ANYDATACHANGED:
                    aContentTimer.Start();      // Notizen nicht sofort suchen
                    break;

                default:
                    break;
            }
        }
    }
    else if ( rHint.ISA(SfxEventHint) )
    {
        ULONG nEventId = ((SfxEventHint&)rHint).GetEventId();
        if ( nEventId == SFX_EVENT_ACTIVATEDOC )
        {
            aLbEntries.ActiveDocChanged();
            UpdateAll();
        }
    }
}

//------------------------------------------------------------------------

IMPL_LINK( ScNavigatorDlg, TimeHdl, Timer*, pTimer )
{
    if ( pTimer != &aContentTimer )
        return 0;

    aLbEntries.Refresh( SC_CONTENT_NOTE );
    return 0;
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetDropMode(USHORT nNew)
{
    nDropMode = nNew;
    aTbxCmd.UpdateButtons();

    ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
    rCfg.SetDragMode(nDropMode);
}

//------------------------------------------------------------------------

void ScNavigatorDlg::CursorPosChanged()
{
    //! Eintraege selektieren ???

//  if ( GetDBAtCursor( aStrDbName ) )
//  if ( GetAreaAtCursor( aStrAreaName ) )
}

//------------------------------------------------------------------------

BOOL ScNavigatorDlg::GetDBAtCursor( String& rStrName )
{
    BOOL bFound = FALSE;

    if ( GetViewData() )
    {
        USHORT nCol = aEdCol.GetCol();
        USHORT nRow = aEdRow.GetRow();

        if ( nCol > 0 && nRow > 0 )
        {
            ScDocument* pDoc  = pViewData->GetDocument();
            ScDBData*   pData = pDoc->GetDBAtCursor( nCol-1, nRow-1, pViewData->GetTabNo() );

            bFound = ( pData != NULL );
            if ( bFound )
                pData->GetName( rStrName );
        }
    }

    return bFound;
}

//------------------------------------------------------------------------

BOOL ScNavigatorDlg::GetAreaAtCursor( String& rStrName )
{
    BOOL bFound = FALSE;

    if ( GetViewData() )
    {
        USHORT nCol = aEdCol.GetCol();
        USHORT nRow = aEdRow.GetRow();

        if ( nCol > 0 && nRow > 0 )
        {
            ScDocument*  pDoc  = pViewData->GetDocument();
            ScRangeData* pData = pDoc->GetRangeAtCursor( nCol-1, nRow-1, pViewData->GetTabNo() );

            bFound = ( pData != NULL );
            if ( bFound )
                pData->GetName( rStrName );
        }
    }

    return bFound;
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetCurrentCell( USHORT nColNo, USHORT nRowNo )
{
    if ( (nColNo+1 != nCurCol) || (nRowNo+1 != nCurRow) )
    {
        // SID_CURRENTCELL == Item #0 Cache leeren, damit das Setzen der
        // aktuellen Zelle auch in zusammengefassten Bereichen funktioniert.
        ppBoundItems[0]->ClearCache();

        ScAddress aScAddress( nColNo, nRowNo, 0 );
        String  aAddr;
        aScAddress.Format( aAddr, SCA_ABS );

        BOOL bUnmark = FALSE;
        if ( GetViewData() )
            bUnmark = !pViewData->GetMarkData().IsCellMarked( nColNo, nRowNo );

        SfxStringItem   aPosItem( SID_CURRENTCELL, aAddr );
        SfxBoolItem     aUnmarkItem( FN_PARAM_1, bUnmark );     // ggf. Selektion aufheben

        SFX_DISPATCHER().Execute( SID_CURRENTCELL,
                                  SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                                  &aPosItem, &aUnmarkItem, 0L );
    }
}

void ScNavigatorDlg::SetCurrentCellStr( const String rName )
{
    ppBoundItems[0]->ClearCache();
    SfxStringItem   aNameItem( SID_CURRENTCELL, rName );

    SFX_DISPATCHER().Execute( SID_CURRENTCELL,
                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                              &aNameItem, 0L );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetCurrentTable( USHORT nTabNo )
{
    if ( nTabNo != nCurTab )
    {
        //  Tabelle fuer Basic ist 1-basiert
        SfxUInt16Item aTabItem( SID_CURRENTTAB, nTabNo + 1 );
        SFX_DISPATCHER().Execute( SID_CURRENTTAB,
                                  SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                                  &aTabItem, 0L );
    }
}

void ScNavigatorDlg::SetCurrentTableStr( const String rName )
{
    if (!GetViewData()) return;

    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nCount    = pDoc->GetTableCount();
    String aTabName;

    for ( USHORT i=0; i<nCount; i++ )
    {
        pDoc->GetName( i, aTabName );
        if ( aTabName == rName )
        {
            SetCurrentTable( i );
            return;
        }
    }

    Sound::Beep();                  // Tabelle nicht gefunden
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetCurrentObject( const String rName )
{
    SfxStringItem aNameItem( SID_CURRENTOBJECT, rName );
    SFX_DISPATCHER().Execute( SID_CURRENTOBJECT,
                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                              &aNameItem, 0L );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetCurrentDoc( const String& rDocName )        // aktivieren
{
    SfxStringItem aDocItem( SID_CURRENTDOC, rDocName );
    SFX_DISPATCHER().Execute( SID_CURRENTDOC,
                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                              &aDocItem, 0L );
}

//------------------------------------------------------------------------

BOOL ScNavigatorDlg::GetViewData()
{
    ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    pViewData = pViewSh ? pViewSh->GetViewData() : NULL;

    return ( pViewData != NULL );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::UpdateColumn( const USHORT* pCol )
{
    if ( pCol )
        nCurCol = *pCol;
    else if ( GetViewData() )
        nCurCol = pViewData->GetCurX() + 1;

    aEdCol.SetCol( nCurCol );
    CheckDataArea();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::UpdateRow( const USHORT* pRow )
{
    if ( pRow )
        nCurRow = *pRow;
    else if ( GetViewData() )
        nCurRow = pViewData->GetCurY() + 1;

    aEdRow.SetRow( nCurRow );
    CheckDataArea();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::UpdateTable( const USHORT* pTab )
{
    if ( pTab )
        nCurTab = *pTab;
    else if ( GetViewData() )
        nCurTab = pViewData->GetTabNo();

//  aLbTables.SetTab( nCurTab );
    CheckDataArea();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::UpdateAll()
{
    switch ( eListMode )
    {
        case NAV_LMODE_DOCS:
        case NAV_LMODE_DBAREAS:
        case NAV_LMODE_AREAS:
            aLbEntries.Refresh();
            break;

        case NAV_LMODE_NONE:
            //! ???
            break;

        default:
            break;
    }

    aContentTimer.Stop();       // dann nicht nochmal
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetListMode( NavListMode eMode, BOOL bSetSize )
{
    if ( eMode != eListMode )
    {
        if ( eMode != NAV_LMODE_NONE )
            bFirstBig = FALSE;              // nicht mehr automatisch umschalten

        eListMode = eMode;

        switch ( eMode )
        {
            case NAV_LMODE_NONE:
                ShowList( FALSE, bSetSize );
                break;

            case NAV_LMODE_AREAS:
            case NAV_LMODE_DBAREAS:
            case NAV_LMODE_DOCS:
                aLbEntries.Refresh();
                ShowList( TRUE, bSetSize );
                break;

            case NAV_LMODE_SCENARIOS:
                ShowScenarios( TRUE, bSetSize );
                break;
        }

        aTbxCmd.UpdateButtons();

        if ( eMode != NAV_LMODE_NONE )
        {
            ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
            rCfg.SetListMode( (USHORT) eMode );
        }
    }

    if ( pMarkArea )
        UnmarkDataArea();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::ShowList( BOOL bShow, BOOL bSetSize )
{
    FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
    Size aSize = GetParent()->GetOutputSizePixel();

    if ( bShow )
    {
        Size aMinSize = aInitSize;

        aMinSize.Height() += nInitListHeight;
        if ( pFloat )
            pFloat->SetMinOutputSizePixel( aMinSize );
        aSize.Height() = nListModeHeight;
        aLbEntries.Show();
        aLbDocuments.Show();
    }
    else
    {
        if ( pFloat )
        {
            pFloat->SetMinOutputSizePixel( aInitSize );
            nListModeHeight = aSize.Height();
        }
        aSize.Height() = aInitSize.Height();
        aLbEntries.Hide();
        aLbDocuments.Hide();
    }
    aWndScenarios.Hide();

    if ( pFloat )
    {
        if ( bSetSize )
            pFloat->SetOutputSizePixel( aSize );
    }
    else
    {
        SfxNavigator* pNav = (SfxNavigator*)GetParent();
        Size aFloating = pNav->GetFloatingSize();
        aFloating.Height() = aSize.Height();
        pNav->SetFloatingSize( aFloating );
    }
}

//------------------------------------------------------------------------

void ScNavigatorDlg::ShowScenarios( BOOL bShow, BOOL bSetSize )
{
    FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
    Size aSize = GetParent()->GetOutputSizePixel();

    if ( bShow )
    {
        SfxBindings& rBindings = SFX_BINDINGS();
        Size         aMinSize  = aInitSize;

        aMinSize.Height() += nInitListHeight;
        if ( pFloat )
            pFloat->SetMinOutputSizePixel( aMinSize );
        aSize.Height() = nListModeHeight;
        rBindings.Invalidate( SID_SELECT_SCENARIO );
        rBindings.Update( SID_SELECT_SCENARIO );
        aWndScenarios.Show();
        aLbDocuments.Show();
    }
    else
    {
        if ( pFloat )
        {
            pFloat->SetMinOutputSizePixel( aInitSize );
            nListModeHeight = aSize.Height();
        }
        aSize.Height() = aInitSize.Height();
        aWndScenarios.Hide();
        aLbDocuments.Hide();
    }
    aLbEntries.Hide();

    if ( pFloat )
    {
        if ( bSetSize )
            pFloat->SetOutputSizePixel( aSize );
    }
    else
    {
        SfxNavigator* pNav = (SfxNavigator*)GetParent();
        Size aFloating = pNav->GetFloatingSize();
        aFloating.Height() = aSize.Height();
        pNav->SetFloatingSize( aFloating );
    }
}


//------------------------------------------------------------------------
//
//      Dokumente fuer Dropdown-Listbox
//
//------------------------------------------------------------------------

void ScNavigatorDlg::GetDocNames( const String* pManualSel )
{
    aLbDocuments.Clear();
    aLbDocuments.SetUpdateMode( FALSE );

    ScDocShell* pCurrentSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );

    String aSelEntry;
    SfxObjectShell* pSh = SfxObjectShell::GetFirst();
    while ( pSh )
    {
        if ( pSh->ISA(ScDocShell) )
        {
            String aName = pSh->GetTitle();
            String aEntry = aName;
            if (pSh == pCurrentSh)
                aEntry += aStrActive;
            else
                aEntry += aStrNotActive;
            aLbDocuments.InsertEntry( aEntry );

            if ( pManualSel ? ( aName == *pManualSel )
                            : ( pSh == pCurrentSh ) )
                aSelEntry = aEntry;                     // kompletter Eintrag zum Selektieren
        }

        pSh = SfxObjectShell::GetNext( *pSh );
    }

    aLbDocuments.InsertEntry( aStrActiveWin );

    String aHidden =  aLbEntries.GetHiddenTitle();
    if (aHidden.Len())
    {
        String aEntry = aHidden;
        aEntry += aStrHidden;
        aLbDocuments.InsertEntry( aEntry );

        if ( pManualSel && aHidden == *pManualSel )
            aSelEntry = aEntry;
    }

    aLbDocuments.SetUpdateMode( TRUE );

    aLbDocuments.SelectEntry( aSelEntry );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::MarkDataArea()
{
    ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());

    if ( pViewSh )
    {
        ScDocument* pDoc = (pViewData = pViewSh->GetViewData())->GetDocument();

        if ( !pMarkArea )
            pMarkArea = new ScArea;

        pViewSh->MarkDataArea();
        ScRange aMarkRange;
        pViewSh->GetViewData()->GetMarkData().GetMarkArea(aMarkRange);
        pMarkArea->nColStart = aMarkRange.aStart.Col();
        pMarkArea->nRowStart = aMarkRange.aStart.Row();
        pMarkArea->nColEnd = aMarkRange.aEnd.Col();
        pMarkArea->nRowEnd = aMarkRange.aEnd.Row();
        pMarkArea->nTab = aMarkRange.aStart.Tab();
    }
}

//------------------------------------------------------------------------

void ScNavigatorDlg::UnmarkDataArea()
{
    ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());

    if ( pViewSh )
    {
        pViewSh->Unmark();
        DELETEZ( pMarkArea );
    }
}

//------------------------------------------------------------------------

void ScNavigatorDlg::CheckDataArea()
{
    if ( aTbxCmd.IsItemChecked( IID_DATA ) && pMarkArea )
    {
        if (   nCurTab   != pMarkArea->nTab
            || nCurCol <  pMarkArea->nColStart+1
            || nCurCol >  pMarkArea->nColEnd+1
            || nCurRow <  pMarkArea->nRowStart+1
            || nCurRow >  pMarkArea->nRowEnd+1 )
        {
            aTbxCmd.SetItemState( IID_DATA, TriState(STATE_CHECK) );
            aTbxCmd.Select( IID_DATA );
        }
    }
}

//------------------------------------------------------------------------

void ScNavigatorDlg::StartOfDataArea()
{
    //  pMarkArea auswerten ???

    if ( GetViewData() )
    {
        ScMarkData& rMark = pViewData->GetMarkData();
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );

        USHORT nCol = aMarkRange.aStart.Col();
        USHORT nRow = aMarkRange.aStart.Row();

        if ( (nCol+1 != aEdCol.GetCol()) || (nRow+1 != aEdRow.GetRow()) )
            SetCurrentCell( nCol, nRow );
    }
}

//------------------------------------------------------------------------

void ScNavigatorDlg::EndOfDataArea()
{
    //  pMarkArea auswerten ???

    if ( GetViewData() )
    {
        ScMarkData& rMark = pViewData->GetMarkData();
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );

        USHORT nCol = aMarkRange.aEnd.Col();
        USHORT nRow = aMarkRange.aEnd.Row();

        if ( (nCol+1 != aEdCol.GetCol()) || (nRow+1 != aEdRow.GetRow()) )
            SetCurrentCell( nCol, nRow );
    }
}

//------------------------------------------------------------------------

SfxChildAlignment __EXPORT ScNavigatorDlg::CheckAlignment(
                            SfxChildAlignment eActAlign, SfxChildAlignment eAlign )
{
    SfxChildAlignment eRetAlign;

    //! kein Andocken, wenn Listbox nicht da ???

    switch (eAlign)
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_HIGHESTBOTTOM:
            eRetAlign = eActAlign;              // nicht erlaubt
            break;

        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_FIRSTLEFT:
        case SFX_ALIGN_LASTLEFT:
        case SFX_ALIGN_FIRSTRIGHT:
        case SFX_ALIGN_LASTRIGHT:
            eRetAlign = eAlign;                 // erlaubt
            break;

        default:
            eRetAlign = eAlign;
            break;
    }
    return eRetAlign;
}

//------------------------------------------------------------------------
//
//  Drop auf den Navigator - andere Datei laden (File oder Bookmark)
//
//------------------------------------------------------------------------

BOOL __EXPORT ScNavigatorDlg::Drop( const DropEvent& rEvt )
{
    BOOL bReturn = FALSE;

    if ( !aLbEntries.IsInDrag() )       // kein Verschieben innerhalb der TreeListBox
    {
        String aFileName;

        SvDataObjectRef pObject = SvDataObject::PasteDragServer(rEvt);

        ULONG nFormat = INetBookmark::HasFormat(*pObject);
        INetBookmark aBookmark;
        if (aBookmark.Paste(*pObject,nFormat))
            aFileName = aBookmark.GetURL();
        else
        {
            //  FORMAT_FILE direkt aus DragServer

            USHORT nCount = DragServer::GetItemCount();
            for ( USHORT i = 0; i < nCount && !aFileName.Len(); ++i )
                if (DragServer::HasFormat( i, FORMAT_FILE ))
                    aFileName = DragServer::PasteFile( i );
        }

        if ( aFileName.Len() )
            bReturn = aLbEntries.LoadFile( aFileName );
    }
    return bReturn;
}

BOOL __EXPORT ScNavigatorDlg::QueryDrop( DropEvent& rEvt )
{
    BOOL bReturn = FALSE;

    if ( !aLbEntries.IsInDrag() )       // kein Verschieben innerhalb der TreeListBox
    {
        SvDataObjectRef pObject = SvDataObject::PasteDragServer(rEvt);
        if ( pObject->HasFormat(FORMAT_FILE)
            || INetBookmark::HasFormat(*pObject) )
        {
            rEvt.SetAction(DROP_COPY);      // Kopier-Cursor anzeigen
            bReturn = TRUE;
        }
    }

    return bReturn;
}



