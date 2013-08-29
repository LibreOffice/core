/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <rangelst.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/navigat.hxx>
#include <svl/stritem.hxx>
#include <svl/urlbmk.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>

#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "dbdata.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "popmenu.hxx"
#include "scresid.hxx"
#include "scmod.hxx"
#include "navicfg.hxx"
#include "navcitem.hxx"
#include "navipi.hrc"
#include "navipi.hxx"
#include "navsett.hxx"
#include "markdata.hxx"

#include <algorithm>

//  Timeout, um Notizen zu suchen
#define SC_CONTENT_TIMEOUT  1000

//  Toleranz, wieviel ueber der eingeklappten Groesse noch klein ist
#define SCNAV_MINTOL        5

//  maximum values for UI
#define SCNAV_MAXCOL        (MAXCOLCOUNT)
// macro is sufficient since only used in ctor
#define SCNAV_COLDIGITS     (static_cast<xub_StrLen>( floor( log10( static_cast<double>(SCNAV_MAXCOL)))) + 1)   // 1...256...18278
// precomputed constant because it is used in every change of spin button field
static const xub_StrLen SCNAV_COLLETTERS = ::ScColToAlpha(SCNAV_MAXCOL).getLength();    // A...IV...ZZZ

#define SCNAV_MAXROW        (MAXROWCOUNT)

//------------------------------------------------------------------------

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
        nCol        ( 0 ),
        nKeyGroup   ( KEYGROUP_ALPHA )
{
    SetMaxTextLen( SCNAV_COLDIGITS );   // 1...256...18278 or A...IV...ZZZ
}

//------------------------------------------------------------------------

ColumnEdit::~ColumnEdit()
{
}

//------------------------------------------------------------------------

long ColumnEdit::Notify( NotifyEvent& rNEvt )
{
    long nHandled = SpinField::Notify( rNEvt );

    sal_uInt16 nType = rNEvt.GetType();
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

//------------------------------------------------------------------------

void ColumnEdit::LoseFocus()
{
    EvalText();
}


//------------------------------------------------------------------------

void ColumnEdit::Up()
{
    nCol++;

    if ( nCol <= SCNAV_MAXCOL )
        SetCol( nCol );
    else
        nCol--;
}

//------------------------------------------------------------------------

void ColumnEdit::Down()
{
    if ( nCol>1 )
        SetCol( nCol-1 );
}

//------------------------------------------------------------------------

void ColumnEdit::First()
{
    nCol = 1;
    SetText(OUString('A'));
}

//------------------------------------------------------------------------

void ColumnEdit::Last()
{
    OUString aStr;
    nCol = NumToAlpha( SCNAV_MAXCOL, aStr );
    SetText( aStr );
}


//------------------------------------------------------------------------

void ColumnEdit::EvalText()
{
    OUString aStrCol = GetText();

    if (!aStrCol.isEmpty())
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
    SCROW nRow = rDlg.aEdRow.GetRow();

    EvalText(); // setzt nCol

    if ( (nCol > 0) && (nRow > 0) )
        rDlg.SetCurrentCell( nCol-1, nRow-1 );
}

//------------------------------------------------------------------------

void ColumnEdit::SetCol( SCCOL nColNo )
{
    OUString aStr;

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

SCCOL ColumnEdit::AlphaToNum( OUString& rStr )
{
    SCCOL  nColumn = 0;

    if ( CharClass::isAsciiAlpha( rStr) )
    {
        rStr = rStr.toAsciiUpperCase();

        if (::AlphaToCol( nColumn, rStr))
            ++nColumn;

        if ( (rStr.getLength() > SCNAV_COLLETTERS) || (nColumn > SCNAV_MAXCOL) )
        {
            nColumn = SCNAV_MAXCOL;
            NumToAlpha( nColumn, rStr );
        }
    }
    else
        rStr = "";

    return nColumn;
}

//------------------------------------------------------------------------

SCCOL ColumnEdit::NumStrToAlpha( OUString& rStr )
{
    SCCOL  nColumn = 0;

    if ( CharClass::isAsciiNumeric(rStr) )
        nColumn = NumToAlpha( (SCCOL)rStr.toInt32(), rStr );
    else
        rStr = "";

    return nColumn;
}

//------------------------------------------------------------------------

SCCOL ColumnEdit::NumToAlpha( SCCOL nColNo, OUString& rStr )
{
    if ( nColNo > SCNAV_MAXCOL )
        nColNo = SCNAV_MAXCOL;
    else if ( nColNo < 1 )
        nColNo = 1;

    ::ScColToAlpha( rStr, nColNo - 1);

    return nColNo;
}

//==================================================================
//  class RowEdit
//==================================================================

RowEdit::RowEdit( ScNavigatorDlg* pParent, const ResId& rResId )
    :   NumericField( pParent, rResId ),
        rDlg        ( *pParent )
{
    SetMax( SCNAV_MAXROW);
    SetLast( SCNAV_MAXROW);
}

//------------------------------------------------------------------------

RowEdit::~RowEdit()
{
}

//------------------------------------------------------------------------

long RowEdit::Notify( NotifyEvent& rNEvt )
{
    long nHandled = NumericField::Notify( rNEvt );

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

//------------------------------------------------------------------------

void RowEdit::LoseFocus()
{
}

//------------------------------------------------------------------------

void RowEdit::ExecuteRow()
{
    SCCOL nCol = rDlg.aEdCol.GetCol();
    SCROW nRow = (SCROW)GetValue();

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

ScDocListBox::~ScDocListBox()
{
}

//------------------------------------------------------------------------

void ScDocListBox::Select()
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
    InitImageList();    // ImageList members of ScNavigatorDlg must be initialized before!

    SetSizePixel( CalcWindowSizePixel() );
    SetDropdownClickHdl( LINK(this, CommandToolBox, ToolBoxDropdownClickHdl) );
    SetItemBits( IID_DROPMODE, GetItemBits( IID_DROPMODE ) | TIB_DROPDOWNONLY );
}

//------------------------------------------------------------------------

CommandToolBox::~CommandToolBox()
{
}

//------------------------------------------------------------------------

void CommandToolBox::Select( sal_uInt16 nSelId )
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
            case IID_CHANGEROOT:
                rDlg.aLbEntries.ToggleRoot();
                UpdateButtons();
                break;
        }
}

void CommandToolBox::Select()
{
    Select( GetCurItemId() );
}

//------------------------------------------------------------------------

void CommandToolBox::Click()
{
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(CommandToolBox, ToolBoxDropdownClickHdl)
{
    //  Das Popupmenue fuer den Dropmodus muss im Click (Button Down)
    //  statt im Select (Button Up) aufgerufen werden.

    if ( GetCurItemId() == IID_DROPMODE )
    {
        ScPopupMenu aPop( ScResId( RID_POPUP_DROPMODE ) );
        aPop.CheckItem( RID_DROPMODE_URL + rDlg.GetDropMode() );
        aPop.Execute( this, GetItemRect(IID_DROPMODE), POPUPMENU_EXECUTE_DOWN );
        sal_uInt16 nId = aPop.GetSelected();

        EndSelection();     // vor SetDropMode (SetDropMode ruft SetItemImage)

        if ( nId >= RID_DROPMODE_URL && nId <= RID_DROPMODE_COPY )
            rDlg.SetDropMode( nId - RID_DROPMODE_URL );

        //  den gehighlighteten Button aufheben
        Point aPoint;
        MouseEvent aLeave( aPoint, 0, MOUSE_LEAVEWINDOW | MOUSE_SYNTHETIC );
        MouseMove( aLeave );
    }

    return 1;
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
        EnableItem( IID_CHANGEROOT, false );
        CheckItem( IID_CHANGEROOT, false );
    }
    else
    {
        EnableItem( IID_CHANGEROOT, sal_True );
        sal_Bool bRootSet = rDlg.aLbEntries.GetRootType() != SC_CONTENT_ROOT;
        CheckItem( IID_CHANGEROOT, bRootSet );
    }

    sal_uInt16 nImageId = 0;
    switch ( rDlg.nDropMode )
    {
        case SC_DROPMODE_URL:   nImageId = RID_IMG_DROP_URL;  break;
        case SC_DROPMODE_LINK:  nImageId = RID_IMG_DROP_LINK; break;
        case SC_DROPMODE_COPY:  nImageId = RID_IMG_DROP_COPY; break;
    }
    SetItemImage( IID_DROPMODE, Image(ScResId(nImageId)) );
}

void CommandToolBox::InitImageList()
{
    ImageList& rImgLst = rDlg.aCmdImageList;

    sal_uInt16 nCount = GetItemCount();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        sal_uInt16 nId = GetItemId(i);
        SetItemImage( nId, rImgLst.GetImage( nId ) );
    }
}

void CommandToolBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        //  update item images

        InitImageList();
        UpdateButtons();    // drop mode
    }

    ToolBox::DataChanged( rDCEvt );
}

//==================================================================
//  class ScNavigatorSettings
//==================================================================

ScNavigatorSettings::ScNavigatorSettings() :
    maExpandedVec( SC_CONTENT_COUNT, false ),
    mnRootSelected( SC_CONTENT_ROOT ),
    mnChildSelected( SC_CONTENT_NOCHILD )
{
}

//==================================================================
//  class ScNavigatorDlgWrapper
//==================================================================

SFX_IMPL_CHILDWINDOWCONTEXT( ScNavigatorDialogWrapper, SID_NAVIGATOR )

ScNavigatorDialogWrapper::ScNavigatorDialogWrapper(
                                    Window*          pParent,
                                    sal_uInt16           nId,
                                    SfxBindings*     pBind,
                                    SfxChildWinInfo* /* pInfo */ ) :
        SfxChildWindowContext( nId )
{
    pNavigator = new ScNavigatorDlg( pBind, this, pParent, true );
    SetWindow( pNavigator );

    //  Einstellungen muessen anderswo gemerkt werden,
    //  pInfo geht uns (ausser der Groesse) nichts mehr an

    Size aInfoSize = pParent->GetOutputSizePixel();     // von aussen vorgegebene Groesse
    Size aNavSize = pNavigator->GetOutputSizePixel();   // Default-Groesse

    aNavSize.Width()  = std::max( aInfoSize.Width(),  aNavSize.Width() );
    aNavSize.Height() = std::max( aInfoSize.Height(), aNavSize.Height() );
    pNavigator->nListModeHeight = std::max( aNavSize.Height(), pNavigator->nListModeHeight );

    //  Die Groesse kann in einem anderen Modul geaendert worden sein,
    //  deshalb muessen in Abhaengigkeit von der momentanen Groesse die
    //  Inhalte eingeblendet werden oder nicht

    sal_Bool bSmall = ( aInfoSize.Height() <= pNavigator->aInitSize.Height() + SCNAV_MINTOL );
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

    //  Die Groesse des Floats nicht neu setzen (sal_False bei SetListMode), damit der
    //  Navigator nicht aufgeklappt wird, wenn er minimiert war (#38872#).

    pNavigator->SetListMode( eNavMode, false );     // FALSE: Groesse des Float nicht setzen

    sal_uInt16 nCmdId;
    switch (eNavMode)
    {
        case NAV_LMODE_DOCS:        nCmdId = IID_DOCS;      break;
        case NAV_LMODE_AREAS:       nCmdId = IID_AREAS;     break;
        case NAV_LMODE_DBAREAS:     nCmdId = IID_DBAREAS;   break;
        case NAV_LMODE_SCENARIOS:   nCmdId = IID_SCENARIOS; break;
        default:                    nCmdId = 0;
    }
    if (nCmdId)
    {
        pNavigator->aTbxCmd.CheckItem( nCmdId );
        pNavigator->DoResize();
    }

    pNavigator->bFirstBig = ( nCmdId == 0 );    // dann spaeter
}

void ScNavigatorDialogWrapper::Resizing( Size& rSize )
{
    ((ScNavigatorDlg*)GetWindow())->Resizing(rSize);
}

//========================================================================
// class ScNavigatorPI
//========================================================================

#define CTRL_ITEMS 4

#define REGISTER_SLOT(i,id) \
    ppBoundItems[i]=new ScNavigatorControllerItem(id,*this,rBindings);

ScNavigatorDlg::ScNavigatorDlg( SfxBindings* pB, SfxChildWindowContext* pCW, Window* pParent,
    const bool bUseStyleSettingsBackground) :
        Window( pParent, ScResId(RID_SCDLG_NAVIGATOR) ),
        rBindings   ( *pB ),                                // is used in CommandToolBox ctor
        aCmdImageList( ScResId( IL_CMD ) ),
        aFtCol      ( this, ScResId( FT_COL ) ),
        aEdCol      ( this, ScResId( ED_COL ) ),
        aFtRow      ( this, ScResId( FT_ROW ) ),
        aEdRow      ( this, ScResId( ED_ROW ) ),
        aTbxCmd     ( this, ScResId( TBX_CMD ) ),
        aLbEntries  ( this, ScResId( LB_ENTRIES ) ),
        aWndScenarios( this,ScResId( STR_QHLP_SCEN_LISTBOX), ScResId(STR_QHLP_SCEN_COMMENT)),
        aLbDocuments( this, ScResId( LB_DOCUMENTS ) ),
        aStrDragMode ( ScResId( STR_DRAGMODE ) ),
        aStrDisplay  ( ScResId( STR_DISPLAY ) ),
        aStrActiveWin( ScResId( STR_ACTIVEWIN ) ),
        pContextWin ( pCW ),
        pMarkArea   ( NULL ),
        pViewData   ( NULL ),
        nListModeHeight( 0 ),
        nInitListHeight( 0 ),
        eListMode   ( NAV_LMODE_NONE ),
        nDropMode   ( SC_DROPMODE_URL ),
        nCurCol     ( 0 ),
        nCurRow     ( 0 ),
        nCurTab     ( 0 ),
        bFirstBig   ( false ),
        mbUseStyleSettingsBackground(bUseStyleSettingsBackground)
{
    ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
    nDropMode = rCfg.GetDragMode();
    //  eListMode wird von aussen gesetzt, Root weiter unten

    aLbDocuments.SetDropDownLineCount(9);
    OUString aOpen(" (");
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

    const long nListboxYPos =
        ::std::max(
            (aTbxCmd.GetPosPixel().Y() + aTbxCmd.GetSizePixel().Height()),
            (aEdRow.GetPosPixel().Y() + aEdRow.GetSizePixel().Height()) )
        + 4;
    aLbEntries.setPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y);

    nBorderOffset = aLbEntries.GetPosPixel().X();

    aInitSize.Width()  =  aTbxCmd.GetPosPixel().X()
                        + aTbxCmd.GetSizePixel().Width()
                        + nBorderOffset;
    aInitSize.Height() = aLbEntries.GetPosPixel().Y();

    nInitListHeight = aLbEntries.GetSizePixel().Height();
    nListModeHeight =  aInitSize.Height()
                     + nInitListHeight;

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
    StartListening( rBindings );

    aLbDocuments.Hide();        // bei NAV_LMODE_NONE gibts die nicht

    aLbEntries.InitWindowBits(sal_True);

    aLbEntries.SetSpaceBetweenEntries(0);
    aLbEntries.SetSelectionMode( SINGLE_SELECTION );
    aLbEntries.SetDragDropMode(     SV_DRAGDROP_CTRL_MOVE |
                                    SV_DRAGDROP_CTRL_COPY |
                                    SV_DRAGDROP_ENABLE_TOP );

    //  war eine Kategorie als Root ausgewaehlt?
    sal_uInt16 nLastRoot = rCfg.GetRootType();
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

    aLbEntries.SetAccessibleRelationLabeledBy(&aLbEntries);
    aTbxCmd.SetAccessibleRelationLabeledBy(&aTbxCmd);
    aLbDocuments.SetAccessibleName(aStrActiveWin);

    if (pContextWin == NULL)
    {
        // When the context window is missing then the navigator is
        // displayed in the sidebar and has the whole deck to fill.
        // Therefore hide the button that hides all controls below the
        // top two rows of buttons.
        aTbxCmd.Select(IID_ZOOMOUT);
        aTbxCmd.RemoveItem(aTbxCmd.GetItemPos(IID_ZOOMOUT));
    }
}

//------------------------------------------------------------------------

ScNavigatorDlg::~ScNavigatorDlg()
{
    aContentTimer.Stop();

    sal_uInt16 i;
    for ( i=0; i<CTRL_ITEMS; i++ )
        delete ppBoundItems[i];

    delete [] ppBoundItems;
    delete pMarkArea;

    EndListening( *(SFX_APP()) );
    EndListening( rBindings );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::Resizing( Size& rNewSize )  // Size = Outputsize?
{
    FloatingWindow* pFloat = pContextWin!=NULL ? pContextWin->GetFloatingWindow() : NULL;
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
}



void ScNavigatorDlg::Paint( const Rectangle& rRect )
{
    if (mbUseStyleSettingsBackground)
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        Color aBgColor = rStyleSettings.GetFaceColor();
        Wallpaper aBack( aBgColor );

        SetBackground( aBack );
        aFtCol.SetBackground( aBack );
        aFtRow.SetBackground( aBack );
    }
    else
    {
        aFtCol.SetBackground(Wallpaper());
        aFtRow.SetBackground(Wallpaper());
    }

    Window::Paint( rRect );
}

void ScNavigatorDlg::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        //  toolbox images are exchanged in CommandToolBox::DataChanged
        Invalidate();
    }

    Window::DataChanged( rDCEvt );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::Resize()
{
    DoResize();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::DoResize()
{
    Size aNewSize = GetOutputSizePixel();
    long nTotalHeight = aNewSize.Height();

    //  bei angedocktem Navigator wird das Fenster evtl. erst klein erzeugt,
    //  dann kommt ein Resize auf die wirkliche Groesse -> dann Inhalte einschalten

    sal_Bool bSmall = ( nTotalHeight <= aInitSize.Height() + SCNAV_MINTOL );
    if ( !bSmall && bFirstBig )
    {
        //  Inhalte laut Config wieder einschalten

        bFirstBig = false;
        NavListMode eNavMode = NAV_LMODE_AREAS;
        ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
        NavListMode eLastMode = (NavListMode) rCfg.GetListMode();
        if ( eLastMode == NAV_LMODE_SCENARIOS )
            eNavMode = NAV_LMODE_SCENARIOS;
        SetListMode( eNavMode, false );         // FALSE: Groesse des Float nicht setzen
    }

    //  auch wenn die Inhalte nicht sichtbar sind, die Groessen anpassen,
    //  damit die Breite stimmt

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

    sal_Bool bListMode = (eListMode != NAV_LMODE_NONE);
    if (pContextWin != NULL)
    {
        FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
        if ( pFloat && bListMode )
            nListModeHeight = nTotalHeight;
    }
}

//------------------------------------------------------------------------

void ScNavigatorDlg::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA(SfxSimpleHint) )
    {
        sal_uLong nHintId = ((SfxSimpleHint&)rHint).GetId();

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
                    aLbEntries.Refresh( SC_CONTENT_DRAWING );
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
        sal_uLong nEventId = ((SfxEventHint&)rHint).GetEventId();
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

void ScNavigatorDlg::SetDropMode(sal_uInt16 nNew)
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

void ScNavigatorDlg::SetCurrentCell( SCCOL nColNo, SCROW nRowNo )
{
    if ( (nColNo+1 != nCurCol) || (nRowNo+1 != nCurRow) )
    {
        // SID_CURRENTCELL == Item #0 Cache leeren, damit das Setzen der
        // aktuellen Zelle auch in zusammengefassten Bereichen funktioniert.
        ppBoundItems[0]->ClearCache();

        ScAddress aScAddress( nColNo, nRowNo, 0 );
        OUString aAddr(aScAddress.Format(SCA_ABS));

        sal_Bool bUnmark = false;
        if ( GetViewData() )
            bUnmark = !pViewData->GetMarkData().IsCellMarked( nColNo, nRowNo );

        SfxStringItem   aPosItem( SID_CURRENTCELL, aAddr );
        SfxBoolItem     aUnmarkItem( FN_PARAM_1, bUnmark );     // ggf. Selektion aufheben

        rBindings.GetDispatcher()->Execute( SID_CURRENTCELL,
                                  SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                                  &aPosItem, &aUnmarkItem, 0L );
    }
}

void ScNavigatorDlg::SetCurrentCellStr( const String rName )
{
    ppBoundItems[0]->ClearCache();
    SfxStringItem   aNameItem( SID_CURRENTCELL, rName );

    rBindings.GetDispatcher()->Execute( SID_CURRENTCELL,
                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                              &aNameItem, 0L );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetCurrentTable( SCTAB nTabNo )
{
    if ( nTabNo != nCurTab )
    {
        //  Tabelle fuer Basic ist 1-basiert
        SfxUInt16Item aTabItem( SID_CURRENTTAB, static_cast<sal_uInt16>(nTabNo) + 1 );
        rBindings.GetDispatcher()->Execute( SID_CURRENTTAB,
                                  SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                                  &aTabItem, 0L );
    }
}

void ScNavigatorDlg::SetCurrentTableStr( const OUString& rName )
{
    if (!GetViewData()) return;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nCount     = pDoc->GetTableCount();
    OUString aTabName;

    for ( SCTAB i=0; i<nCount; i++ )
    {
        pDoc->GetName( i, aTabName );
        if ( aTabName.equals(rName) )
        {
            SetCurrentTable( i );
            return;
        }
    }
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetCurrentObject( const String rName )
{
    SfxStringItem aNameItem( SID_CURRENTOBJECT, rName );
    rBindings.GetDispatcher()->Execute( SID_CURRENTOBJECT,
                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                              &aNameItem, 0L );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::SetCurrentDoc( const String& rDocName )        // aktivieren
{
    SfxStringItem aDocItem( SID_CURRENTDOC, rDocName );
    rBindings.GetDispatcher()->Execute( SID_CURRENTDOC,
                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                              &aDocItem, 0L );
}

//------------------------------------------------------------------------

ScTabViewShell* ScNavigatorDlg::GetTabViewShell() const
{
    return PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
}

//------------------------------------------------------------------------

ScNavigatorSettings* ScNavigatorDlg::GetNavigatorSettings()
{
    //  Don't store the settings pointer here, because the settings belong to
    //  the view, and the view may be closed while the navigator is open (reload).
    //  If the pointer is cached here again later for performance reasons, it has to
    //  be forgotten when the view is closed.

    ScTabViewShell* pViewSh = GetTabViewShell();
    return pViewSh ? pViewSh->GetNavigatorSettings() : NULL;
}

//------------------------------------------------------------------------

sal_Bool ScNavigatorDlg::GetViewData()
{
    ScTabViewShell* pViewSh = GetTabViewShell();
    pViewData = pViewSh ? pViewSh->GetViewData() : NULL;

    return ( pViewData != NULL );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::UpdateColumn( const SCCOL* pCol )
{
    if ( pCol )
        nCurCol = *pCol;
    else if ( GetViewData() )
        nCurCol = pViewData->GetCurX() + 1;

    aEdCol.SetCol( nCurCol );
    CheckDataArea();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::UpdateRow( const SCROW* pRow )
{
    if ( pRow )
        nCurRow = *pRow;
    else if ( GetViewData() )
        nCurRow = pViewData->GetCurY() + 1;

    aEdRow.SetRow( nCurRow );
    CheckDataArea();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::UpdateTable( const SCTAB* pTab )
{
    if ( pTab )
        nCurTab = *pTab;
    else if ( GetViewData() )
        nCurTab = pViewData->GetTabNo();

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

void ScNavigatorDlg::SetListMode( NavListMode eMode, sal_Bool bSetSize )
{
    if ( eMode != eListMode )
    {
        if ( eMode != NAV_LMODE_NONE )
            bFirstBig = false;              // nicht mehr automatisch umschalten

        eListMode = eMode;

        switch ( eMode )
        {
            case NAV_LMODE_NONE:
                ShowList( false, bSetSize );
                break;

            case NAV_LMODE_AREAS:
            case NAV_LMODE_DBAREAS:
            case NAV_LMODE_DOCS:
                aLbEntries.Refresh();
                ShowList( sal_True, bSetSize );
                break;

            case NAV_LMODE_SCENARIOS:
                ShowScenarios( sal_True, bSetSize );
                break;
        }

        aTbxCmd.UpdateButtons();

        if ( eMode != NAV_LMODE_NONE )
        {
            ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
            rCfg.SetListMode( (sal_uInt16) eMode );
        }
    }

    if ( pMarkArea )
        UnmarkDataArea();
}

//------------------------------------------------------------------------

void ScNavigatorDlg::ShowList( sal_Bool bShow, sal_Bool bSetSize )
{
    FloatingWindow* pFloat = pContextWin!=NULL ? pContextWin->GetFloatingWindow() : NULL;
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
        SfxNavigator* pNav = dynamic_cast<SfxNavigator*>(GetParent());
        if (pNav != NULL)
        {
            Size aFloating = pNav->GetFloatingSize();
            aFloating.Height() = aSize.Height();
            pNav->SetFloatingSize( aFloating );
        }
    }
}

//------------------------------------------------------------------------

void ScNavigatorDlg::ShowScenarios( sal_Bool bShow, sal_Bool bSetSize )
{
    FloatingWindow* pFloat = pContextWin!=NULL ? pContextWin->GetFloatingWindow() : NULL;
    Size aSize = GetParent()->GetOutputSizePixel();

    if ( bShow )
    {
        Size aMinSize = aInitSize;
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
    aLbDocuments.SetUpdateMode( false );

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

    aLbDocuments.SetUpdateMode( sal_True );

    aLbDocuments.SelectEntry( aSelEntry );
}

//------------------------------------------------------------------------

void ScNavigatorDlg::MarkDataArea()
{
    ScTabViewShell* pViewSh = GetTabViewShell();

    if ( pViewSh )
    {
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
    ScTabViewShell* pViewSh = GetTabViewShell();

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

        SCCOL nCol = aMarkRange.aStart.Col();
        SCROW nRow = aMarkRange.aStart.Row();

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

        SCCOL nCol = aMarkRange.aEnd.Col();
        SCROW nRow = aMarkRange.aEnd.Row();

        if ( (nCol+1 != aEdCol.GetCol()) || (nRow+1 != aEdRow.GetRow()) )
            SetCurrentCell( nCol, nRow );
    }
}

//------------------------------------------------------------------------

SfxChildAlignment ScNavigatorDlg::CheckAlignment(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
