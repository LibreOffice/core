/*************************************************************************
 *
 *  $RCSfile: gridwin.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-23 19:24:39 $
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

//SV
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
//#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS


// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/adjitem.hxx>
#include <svx/algitem.hxx>
#include <svx/dbexch.hrc>
#include <svx/editview.hxx>
#include <svx/editstat.hxx>
#include <svx/flditem.hxx>
#include <svx/svdetc.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/stritem.hxx>
#include <svtools/svlbox.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/urlbmk.hxx>
#include <tools/urlobj.hxx>
#include <vcl/sound.hxx>
#include <vcl/system.hxx>
#include <offmgr/app.hxx>
#include <vcl/exchange.hxx>
#include <vcl/graph.hxx>
#include <sot/formats.hxx>

#include <svx/svdview.hxx>      // fuer Command-Handler (COMMAND_INSERTTEXT)
#include <svx/outliner.hxx>     // fuer Command-Handler (COMMAND_INSERTTEXT)

#include <com/sun/star/sheet/MemberResultFlags.hpp>

#include "gridwin.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "tabview.hxx"
#include "select.hxx"
#include "scmod.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "dbcolect.hxx"
#include "stlpool.hxx"
#include "printfun.hxx"
#include "cbutton.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "editutil.hxx"
#include "scresid.hxx"
#include "inputhdl.hxx"
#include "uiitems.hxx"          // Filter-Dialog - auslagern !!!
#include "filtdlg.hxx"
#include "impex.hxx"            // Sylk-ID fuer CB
#include "cell.hxx"             // fuer Edit-Felder
#include "patattr.hxx"
#include "notemark.hxx"
#include "rfindlst.hxx"
#include "docpool.hxx"
#include "output.hxx"
#include "docfunc.hxx"
#include "dbdocfun.hxx"
#ifdef AUTOFILTER_POPUP
#include <vcl/menu.hxx>
#endif
#include "dpobject.hxx"
#include "dpoutput.hxx"
#include "transobj.hxx"

using namespace com::sun::star;

#define SC_AUTOFILTER_ALL       0
#define SC_AUTOFILTER_CUSTOM    1
#define SC_AUTOFILTER_TOP10     2

//  Modi fuer die FilterListBox
enum ScFilterBoxMode
{
    SC_FILTERBOX_FILTER,
    SC_FILTERBOX_DATASELECT,
    SC_FILTERBOX_SCENARIO
};

extern SfxViewShell* pScActiveViewShell;            // global.cxx
extern USHORT nScClickMouseModifier;                // global.cxx
extern USHORT nScFillModeMouseModifier;             // global.cxx

#define SC_FILTERLISTBOX_LINES  12

// STATIC DATA -----------------------------------------------------------

static long nFilterBoxTabs[] =
    {   1, // Number of Tabs
        0
    };

//==================================================================

/*
 * Der Autofilter wird auf dem Mac per AutoFilterPopup realisiert.
 * Die AutoFilterListBox wird trotzdem fuer die Auswahlliste gebraucht.
 */

//==================================================================
class ScFilterListBox : public SvTabListBox
{
    ScGridWindow*   pGridWin;
    USHORT          nCol;
    USHORT          nRow;
    BOOL            bButtonDown;
    BOOL            bInit;
    BOOL            bCancelled;
    ULONG           nSel;
    ScFilterBoxMode eMode;

protected:
    virtual void    LoseFocus();
    virtual void    SelectHdl();

public:
                ScFilterListBox( Window* pParent, ScGridWindow* pGrid,
                                    USHORT nNewCol, USHORT nNewRow, ScFilterBoxMode eNewMode );
                ~ScFilterListBox();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );

    virtual void    KeyInput( const KeyEvent& rKEvt );

    USHORT          GetCol() const          { return nCol; }
    USHORT          GetRow() const          { return nRow; }
    ScFilterBoxMode GetMode() const         { return eMode; }
    BOOL            IsDataSelect() const    { return (eMode == SC_FILTERBOX_DATASELECT); }
    void            EndInit();
    void            SetCancelled()          { bCancelled = TRUE; }
};

//-------------------------------------------------------------------

//  ListBox in einem FloatingWindow (pParent)
ScFilterListBox::ScFilterListBox( Window* pParent, ScGridWindow* pGrid,
                                    USHORT nNewCol, USHORT nNewRow, ScFilterBoxMode eNewMode ) :
    SvTabListBox( pParent, WinBits(0) ),    // ohne Border
    pGridWin( pGrid ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    eMode( eNewMode ),
    bButtonDown( FALSE ),
    bInit( TRUE ),
    bCancelled( FALSE ),
    nSel( 0 )
{
}

__EXPORT ScFilterListBox::~ScFilterListBox()
{
    if (IsMouseCaptured())
        ReleaseMouse();
}

void ScFilterListBox::EndInit()
{
    SvLBoxEntry* pEntry = FirstSelected();
    if (pEntry)
        nSel = GetModel()->GetAbsPos( pEntry );
    else
        nSel = 0;

    bInit = FALSE;
}

void __EXPORT ScFilterListBox::LoseFocus()
{
#ifndef UNX
    Hide();
#endif
}

void __EXPORT ScFilterListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    bButtonDown = TRUE;
    SvTabListBox::MouseButtonDown( rMEvt );
}

void __EXPORT ScFilterListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    bButtonDown = FALSE;
    SvTabListBox::MouseButtonUp( rMEvt );

    if (!bCancelled)
        pGridWin->FilterSelect( nSel );
}

void __EXPORT ScFilterListBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();
    if ( !aCode.GetModifier() )             // ohne alle Modifiers
    {
        USHORT nKey = aCode.GetCode();
        if ( nKey == KEY_RETURN )
        {
            SelectHdl();                    // auswaehlen
            return;
        }
        else if ( nKey == KEY_ESCAPE )
        {
            pGridWin->ClickExtern();        // loescht die List-Box !!!
            return;
        }
    }

    SvTabListBox::KeyInput( rKEvt );
}

void __EXPORT ScFilterListBox::SelectHdl()
{
    if ( !IsTravelSelect() && !bInit && !bCancelled )
    {
        SvLBoxEntry* pEntry = FirstSelected();
        if (pEntry)
        {
            nSel = GetModel()->GetAbsPos( pEntry );
            if (!bButtonDown)
                pGridWin->FilterSelect( nSel );
        }
    }
}

#ifdef AUTOFILTER_POPUP

//==================================================================
class AutoFilterPopup : public PopupMenu
{
public:
        AutoFilterPopup( ScGridWindow* _pWin, USHORT _nCol, USHORT _nRow, BOOL bDatSel )
            :   pWin( _pWin ),
                nCol( _nCol ),
                nRow( _nRow ),
                bIsDataSelect( bDatSel ),
                bSelected( FALSE )          {}
        ~AutoFilterPopup();

protected:
    virtual void    Select();

private:
    ScGridWindow* pWin;
    USHORT        nCol;
    USHORT        nRow;
    BOOL          bIsDataSelect;
    BOOL          bSelected;
};

//-------------------------------------------------------------------

void __EXPORT AutoFilterPopup::Select()
{
    //  Button vor dem ausfuehren wieder zeichnen
    if ( !bIsDataSelect )
    {
        pWin->HideCursor();
        pWin->aComboButton.Draw( FALSE );
        pWin->ShowCursor();
    }
    bSelected = TRUE;

    USHORT nCurId = GetCurItemId();
    pWin->ReleaseMouse();
    String aStr = GetItemText( nCurId );
    if ( bIsDataSelect )
        pWin->ExecDataSelect( nCol, nRow, aStr );
    else
        pWin->ExecFilter( nCurId-1, nCol, nRow, aStr );
}

AutoFilterPopup::~AutoFilterPopup()
{
    if ( !bIsDataSelect && !bSelected )
    {
        pWin->HideCursor();
        pWin->aComboButton.Draw( FALSE );
        pWin->ShowCursor();
    }
}

#endif // #ifdef AUTOFILTER_POPUP

//==================================================================

BOOL lcl_IsEditableMatrix( ScDocument* pDoc, const ScRange& rRange )
{
    //  wenn es ein editierbarer Bereich ist, und rechts unten eine Matrix-Zelle
    //  mit Origin links oben liegt, enthaelt der Bereich genau die Matrix.
    //! Direkt die MatrixEdges Funktionen von der Column herausreichen ???

    if ( !pDoc->IsBlockEditable( rRange.aStart.Tab(), rRange.aStart.Col(),rRange.aStart.Row(),
                                    rRange.aEnd.Col(),rRange.aEnd.Row() ) )
        return FALSE;

    ScAddress aPos;
    const ScBaseCell* pCell = pDoc->GetCell( rRange.aEnd );
    return ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA &&
            ((ScFormulaCell*)pCell)->GetMatrixOrigin(aPos) && aPos == rRange.aStart );
}

//==================================================================

//  WB_DIALOGCONTROL noetig fuer UNO-Controls
ScGridWindow::ScGridWindow( Window* pParent, ScViewData* pData, ScSplitPos eWhichPos ) :
            Window( pParent, WB_CLIPCHILDREN | WB_DIALOGCONTROL ),
            DropTargetHelper( this ),
            DragSourceHelper( this ),
            pViewData( pData ),
            eWhich( eWhichPos ),
            pNoteMarker( NULL ),
            nCursorHideCount( 0 ),
            bMarking( FALSE ),
            bEEMouse( FALSE ),
            nButtonDown( 0 ),
            nMouseStatus( SC_GM_NONE ),
            bPivotMouse( FALSE ),
            bDPMouse( FALSE ),
            bRFMouse( FALSE ),
            nPagebreakMouse( SC_PD_NONE ),
            nPageScript( 0 ),
            bDragRect( FALSE ),
            pFilterBox( NULL ),
            pFilterFloat( NULL ),
            nCurrentPointer( 0 ),
            bIsInScroll( FALSE ),
            bIsInPaint( FALSE ),
            aComboButton( this ),
            aCurMousePos( 0,0 ),
            nPaintCount( 0 ),
            bNeedsRepaint( FALSE ),
            bAutoMarkVisible( FALSE )
{
    switch(eWhich)
    {
        case SC_SPLIT_TOPLEFT:
            eHWhich = SC_SPLIT_LEFT;
            eVWhich = SC_SPLIT_TOP;
            break;
        case SC_SPLIT_TOPRIGHT:
            eHWhich = SC_SPLIT_RIGHT;
            eVWhich = SC_SPLIT_TOP;
            break;
        case SC_SPLIT_BOTTOMLEFT:
            eHWhich = SC_SPLIT_LEFT;
            eVWhich = SC_SPLIT_BOTTOM;
            break;
        case SC_SPLIT_BOTTOMRIGHT:
            eHWhich = SC_SPLIT_RIGHT;
            eVWhich = SC_SPLIT_BOTTOM;
            break;
        default:
            DBG_ERROR("GridWindow: falsche Position");
    }

    SetBackground();

    SetMapMode(pViewData->GetLogicMode(eWhich));
//  EnableDrop();
    EnableChildTransparentMode();
    SetDialogControlFlags( WINDOW_DLGCTRL_RETURN | WINDOW_DLGCTRL_WANTFOCUS );

    SetHelpId( HID_SC_WIN_GRIDWIN );
    SetUniqueId( HID_SC_WIN_GRIDWIN );
}

__EXPORT ScGridWindow::~ScGridWindow()
{
    delete pFilterBox;
    if (pFilterFloat)
        pFilterFloat->EndPopupMode();
    delete pFilterFloat;
    delete pNoteMarker;
}

void __EXPORT ScGridWindow::Resize( const Size& rSize )
{
    //  gar nix
}

void ScGridWindow::ClickExtern()
{
    DELETEZ(pFilterBox);
    if (pFilterFloat)
        pFilterFloat->EndPopupMode();
    DELETEZ(pFilterFloat);
}

IMPL_LINK( ScGridWindow, PopupModeEndHdl, FloatingWindow*, pFloat )
{
    if (pFilterBox)
        pFilterBox->SetCancelled();     // nicht mehr auswaehlen
    GrabFocus();
    return 0;
}

void ScGridWindow::DoScenarioMenue( const ScRange& rScenRange )
{
    delete pFilterBox;
    if (pFilterFloat)
        pFilterFloat->EndPopupMode();
    delete pFilterFloat;

    USHORT nCol = rScenRange.aEnd.Col();        // Zelle unterhalb des Buttons
    USHORT nRow = rScenRange.aStart.Row();
    if (nRow == 0)
    {
        nRow = rScenRange.aEnd.Row() + 1;       // Bereich ganz oben -> Button unterhalb
        if (nRow>MAXROW) nRow = MAXROW;
        //! Texthoehe addieren (wenn sie an der View gespeichert ist...)
    }

    USHORT i;
    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();
    long nSizeX  = 0;
    long nSizeY  = 0;
    long nHeight = 0;
    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
    Point aPos = pViewData->GetScrPos( nCol, nRow, eWhich );
    Rectangle aCellRect( OutputToScreenPixel(aPos), Size(nSizeX,nSizeY) );
    aCellRect.Top()    -= nSizeY;
    aCellRect.Bottom() -= nSizeY - 1;
    //  Die ListBox direkt unter der schwarzen Linie auf dem Zellgitter
    //  (wenn die Linie verdeckt wird, sieht es komisch aus...)

    pFilterFloat = new FloatingWindow( this, WinBits(WB_BORDER) );      // nicht resizable etc.
    pFilterFloat->SetPopupModeEndHdl( LINK( this, ScGridWindow, PopupModeEndHdl ) );
    pFilterBox = new ScFilterListBox( pFilterFloat, this, nCol, nRow, SC_FILTERBOX_SCENARIO );

    nSizeX += 1;

    {
        Font    aOldFont = GetFont(); SetFont( pFilterBox->GetFont() );
        MapMode aOldMode = GetMapMode(); SetMapMode( MAP_PIXEL );

        nHeight  = GetTextHeight();
        nHeight *= SC_FILTERLISTBOX_LINES;

        SetMapMode( aOldMode );
        SetFont( aOldFont );
    }

    //  SetSize spaeter
    pFilterBox->SetSelectionMode( SINGLE_SELECTION );
    pFilterBox->SetTabs( nFilterBoxTabs, MapUnit( MAP_APPFONT ));


    //  ParentSize Abfrage fehlt

    Size aSize( nSizeX, nHeight );
    pFilterBox->SetSizePixel( aSize );
    pFilterBox->Show();                 // Show muss vor SetUpdateMode kommen !!!
    pFilterBox->SetUpdateMode(FALSE);

    //  SetOutputSizePixel/StartPopupMode erst unten, wenn die Groesse feststeht

    //  Listbox fuellen

    long nMaxText = 0;
    String aCurrent;
    String aTabName;
    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nEntryCount = 0;
    for (i=nTab+1; i<nTabCount && pDoc->IsScenario(i); i++)
    {
        if (pDoc->HasScenarioRange( i, rScenRange ))
            if (pDoc->GetName( i, aTabName ))
            {
                pFilterBox->InsertEntry( aTabName );
                if (pDoc->IsActiveScenario(i))
                    aCurrent = aTabName;
                long nTextWidth = pFilterBox->GetTextWidth( aTabName );
                if ( nTextWidth > nMaxText )
                    nMaxText = nTextWidth;
                ++nEntryCount;
            }
    }
    if (nEntryCount > SC_FILTERLISTBOX_LINES)
        nMaxText += GetSettings().GetStyleSettings().GetScrollBarSize();
    nMaxText += 4;          // fuer Rand
    if ( nMaxText > 300 )
        nMaxText = 300;     // auch nicht uebertreiben (Pixel)

    if (nMaxText > nSizeX)  // Groesse auf benoetigte Groesse anpassen
    {
        long nDiff = nMaxText - nSizeX;
        aSize = Size( nMaxText, nHeight );
        pFilterBox->SetSizePixel( aSize );
        pFilterFloat->SetOutputSizePixel( aSize );

        //  auch Startposition verschieben
        long nNewX = aCellRect.Left() - nDiff;
        if ( nNewX < 0 )
            nNewX = 0;
        aCellRect.Left() = nNewX;
    }

    pFilterFloat->SetOutputSizePixel( aSize );
    pFilterFloat->StartPopupMode( aCellRect, FLOATWIN_POPUPMODE_DOWN );

    pFilterBox->SetUpdateMode(TRUE);
    pFilterBox->GrabFocus();

    //  Select erst nach GrabFocus, damit das Focus-Rechteck richtig landet
    SvLBoxEntry* pSelect = NULL;
    if (aCurrent.Len())
    {
        ULONG nPos = pFilterBox->GetEntryPos( aCurrent );
        pSelect = pFilterBox->GetEntry( nPos );
    }
    if (!pSelect)
        pSelect = pFilterBox->GetEntry(0);          // einer sollte immer selektiert sein
    if (pSelect)
        pFilterBox->Select(pSelect);

    pFilterBox->EndInit();

    // Szenario-Auswahl kommt aus MouseButtonDown:
    //  der naechste MouseMove auf die Filterbox ist wie ein ButtonDown

    nMouseStatus = SC_GM_FILTER;
    CaptureMouse();
}

void ScGridWindow::DoAutoFilterMenue( USHORT nCol, USHORT nRow, BOOL bDataSelect )
{
    //  bei AUTOFILTER_POPUP-define fuer AutoFilter das Popup nehmen
#ifdef AUTOFILTER_POPUP
    if ( !bDataSelect )
    {
        DoAutoFilterPopup( nCol, nRow, bDataSelect );
        return;
    }
#endif

    /*
     * Standard-Variante mit TreeListBox
     */

    delete pFilterBox;
    if (pFilterFloat)
        pFilterFloat->EndPopupMode();
    delete pFilterFloat;

    USHORT i;
    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();
    long nSizeX  = 0;
    long nSizeY  = 0;
    long nHeight = 0;
    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
    Point aPos = pViewData->GetScrPos( nCol, nRow, eWhich );

    Rectangle aCellRect( OutputToScreenPixel(aPos), Size(nSizeX,nSizeY) );

    aPos.X() -= 1;
    aPos.Y() += nSizeY - 1;

    pFilterFloat = new FloatingWindow( this, WinBits(WB_BORDER) );      // nicht resizable etc.
    pFilterFloat->SetPopupModeEndHdl( LINK( this, ScGridWindow, PopupModeEndHdl ) );
    pFilterBox = new ScFilterListBox( pFilterFloat, this, nCol, nRow,
                        bDataSelect ? SC_FILTERBOX_DATASELECT : SC_FILTERBOX_FILTER );

    nSizeX += 1;

    {
        Font    aOldFont = GetFont(); SetFont( pFilterBox->GetFont() );
        MapMode aOldMode = GetMapMode(); SetMapMode( MAP_PIXEL );

        nHeight  = GetTextHeight();
        nHeight *= SC_FILTERLISTBOX_LINES;

        SetMapMode( aOldMode );
        SetFont( aOldFont );
    }

    //  SetSize spaeter
    pFilterBox->SetSelectionMode( SINGLE_SELECTION );
    pFilterBox->SetTabs( nFilterBoxTabs, MapUnit( MAP_APPFONT ));

    BOOL bEmpty = FALSE;
    TypedStrCollection aStrings( 128, 128 );
    if ( bDataSelect )                                  // Auswahl-Liste
    {
        //  Liste fuellen
        aStrings.SetCaseSensitive( TRUE );
        pDoc->GetDataEntries( nCol, nRow, nTab, aStrings );
        if ( aStrings.GetCount() == 0 )
            bEmpty = TRUE;
    }
    else                                                // AutoFilter
    {
        //! wird der Titel ueberhaupt ausgewertet ???
        String aString;
        pDoc->GetString( nCol, nRow, nTab, aString );
        pFilterBox->SetText( aString );

        //  Standard-Eintraege
        pFilterBox->InsertEntry( String( ScResId( SCSTR_ALL ) ) );
        pFilterBox->InsertEntry( String( ScResId( SCSTR_STDFILTER ) ) );
        pFilterBox->InsertEntry( String( ScResId( SCSTR_TOP10FILTER ) ) );

        //  Liste fuellen
        pDoc->GetFilterEntries( nCol, nRow, nTab, aStrings );
    }

    if (!bEmpty)
    {
        //  Position und Groesse an Fenster anpassen
        //! vorher Abfrage, ob die Eintraege hineinpassen (Breite)

        Size aParentSize = GetParent()->GetOutputSizePixel();
        Size aSize( nSizeX, nHeight );

        if ( aSize.Height() > aParentSize.Height() )
            aSize.Height() = aParentSize.Height();
        if ( aPos.Y() + aSize.Height() > aParentSize.Height() )
            aPos.Y() = aParentSize.Height() - aSize.Height();

        pFilterBox->SetSizePixel( aSize );
        pFilterBox->Show();                 // Show muss vor SetUpdateMode kommen !!!
        pFilterBox->SetUpdateMode(FALSE);

        pFilterFloat->SetOutputSizePixel( aSize );
        pFilterFloat->StartPopupMode( aCellRect, FLOATWIN_POPUPMODE_DOWN );

        //  Listbox fuellen
        USHORT nCount = aStrings.GetCount();
        BOOL bWait = ( nCount > 100 );

        if (bWait)
            EnterWait();

        for (i=0; i<nCount; i++)
            pFilterBox->InsertEntry( aStrings[i]->GetString() );

        if (bWait)
            LeaveWait();

        pFilterBox->SetUpdateMode(TRUE);
    }

    SvLBoxEntry* pSelect = NULL;

    if (!bDataSelect)                       // AutoFilter: aktiven Eintrag selektieren
    {
        ScDBData* pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab );
        if (pDBData)
        {
            ScQueryParam aParam;
            pDBData->GetQueryParam( aParam );       // kann nur MAXQUERY Eintraege ergeben

            BOOL bValid = TRUE;
            for (i=0; i<MAXQUERY && bValid; i++)            // bisherige Filter-Einstellungen
                if (aParam.GetEntry(i).bDoQuery)
                {
                    //!         Abfrage mit DrawButtons zusammenfassen!

                    ScQueryEntry& rEntry = aParam.GetEntry(i);
                    if (i>0)
                        if (rEntry.eConnect != SC_AND)
                            bValid = FALSE;
                    if (rEntry.nField == nCol)
                    {
                        if (rEntry.eOp == SC_EQUAL)
                        {
                            String* pStr = rEntry.pStr;
                            if (pStr)
                            {
                                ULONG nPos = pFilterBox->GetEntryPos( *pStr );
                                pSelect = pFilterBox->GetEntry( nPos );
                            }
                        }
                        else if (rEntry.eOp == SC_TOPVAL && rEntry.pStr &&
                                    rEntry.pStr->EqualsAscii("10"))
                            pSelect = pFilterBox->GetEntry( SC_AUTOFILTER_TOP10 );
                        else
                            pSelect = pFilterBox->GetEntry( SC_AUTOFILTER_CUSTOM );
                    }
                }

            if (!bValid)
                pSelect = pFilterBox->GetEntry( SC_AUTOFILTER_CUSTOM );
        }
    }

        //  neu (309): irgendwas muss immer selektiert sein:
    if (!pSelect)
        pSelect = pFilterBox->GetEntry(0);

    //  keine leere Auswahl-Liste anzeigen:

    if ( bEmpty )
    {
        DELETEZ(pFilterBox);                // war nix
        if (pFilterFloat)
            pFilterFloat->EndPopupMode();
        DELETEZ(pFilterFloat);
        Sound::Beep();                      // bemerkbar machen
    }
    else
    {
//      pFilterBox->Show();                 // schon vorne
        pFilterBox->GrabFocus();

            //  Select erst nach GrabFocus, damit das Focus-Rechteck richtig landet
        if (pSelect)
            pFilterBox->Select(pSelect);

        pFilterBox->EndInit();

        if (!bDataSelect)
        {
            // AutoFilter (aus MouseButtonDown):
            //  der naechste MouseMove auf die Filterbox ist wie ein ButtonDown

            nMouseStatus = SC_GM_FILTER;
            CaptureMouse();
        }
    }
}

#ifdef AUTOFILTER_POPUP

    /*
     * Macintosh-Variante mit Popup-Menue
     */

void ScGridWindow::DoAutoFilterPopup( USHORT nCol, USHORT nRow, BOOL bDataSelect )
{
    AutoFilterPopup*    pPopupMenu = new AutoFilterPopup( this, nCol, nRow, bDataSelect );
    ScDocument*         pDoc = pViewData->GetDocument();
    USHORT              nTab = pViewData->GetTabNo();
    Point               aPos = pViewData->GetScrPos( nCol, nRow, eWhich );
    BOOL                bValid = TRUE;
    String              aStrSelect;
    USHORT              nSelPos = 0;

    TypedStrCollection aStrings( 128, 128 );
    USHORT nFirst = 1;
    if ( bDataSelect )                              // Auswahl-Liste
    {
        //  Liste fuellen
        aStrings.SetCaseSensitive( TRUE );
        pDoc->GetDataEntries( nCol, nRow, nTab, aStrings );

        //  nichts selektieren
        bValid = FALSE;
    }
    else                                            // AutoFilter
    {
        //  Standard-Eintraege
        pPopupMenu->InsertItem( 1, String( ScResId(SCSTR_ALL) ) );
        pPopupMenu->InsertItem( 2, String( ScResId(SCSTR_STDFILTER) ) );
        pPopupMenu->InsertItem( 3, String( ScResId(SCSTR_TOP10FILTER) ) );
        nFirst = 4;

        //  Liste fuellen
        pDoc->GetFilterEntries( nCol, nRow, nTab, aStrings );

        //  aktiven Eintrag suchen
        ScDBData* pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab );
        if (pDBData)
        {
            ScQueryParam aParam;
            pDBData->GetQueryParam( aParam );

            for (USHORT i=0; i<MAXQUERY && bValid; i++)         // bisherige Filter-Einstellungen
            {
                ScQueryEntry& rEntry = aParam.GetEntry(i);
                if (rEntry.bDoQuery)
                {
                    if (i>0)
                        if (rEntry.eConnect != SC_AND)
                            bValid = FALSE;
                    if (rEntry.nField == nCol)
                    {
                        if (rEntry.eOp == SC_EQUAL)
                        {
                            String* pStr = rEntry.pStr;
                            if (pStr)
                                aStrSelect = *pStr;
                        }
                        else if (rEntry.eOp == SC_TOPVAL && rEntry.pStr &&
                                    *rEntry.pStr == "10")
                            nSelPos = SC_AUTOFILTER_TOP10;
                        else
                            nSelPos = SC_AUTOFILTER_CUSTOM;
                    }
                }
            }
            if (!bValid)
            {
                aStrSelect.Erase();
                nSelPos = 0;
            }
        }
    }

    //  Menue fuellen

    USHORT nCount = aStrings.GetCount();
    for ( USHORT i=0; i<nCount; i++ )
    {
        const String& rStr = aStrings[i]->GetString();

        if ( bValid && nSelPos == 0 )
            if ( aStrSelect == rStr )
                nSelPos = i+nFirst;

        pPopupMenu->InsertItem( i+nFirst, rStr );
    }

    if ( bValid )
    {
        pPopupMenu->SetDefaultItem( nSelPos );
        pPopupMenu->CheckItem( nSelPos );
    }

    //  ausfuehren nur, wenn ueberhaupt was drin ist (bei Auswahl-Liste)
    if ( nCount+nFirst > 1 )
        pPopupMenu->Execute( OutputToScreenPixel( aPos ) );
    else
        Sound::Beep();

    delete pPopupMenu;
}

#endif // #ifdef AUTOFILTER_POPUP

void ScGridWindow::FilterSelect( ULONG nSel )
{
    String aString;
    SvLBoxEntry* pEntry = pFilterBox->GetEntry( nSel );
    if (pEntry)
    {
        SvLBoxString* pStringEntry = (SvLBoxString*) pEntry->GetFirstItem( SV_ITEM_ID_LBOXSTRING );
        if ( pStringEntry )
            aString = pStringEntry->GetText();
    }

    USHORT nCol = pFilterBox->GetCol();
    USHORT nRow = pFilterBox->GetRow();
    switch ( pFilterBox->GetMode() )
    {
        case SC_FILTERBOX_DATASELECT:
            ExecDataSelect( nCol, nRow, aString );
            break;
        case SC_FILTERBOX_FILTER:
            ExecFilter( nSel, nCol, nRow, aString );
            break;
        case SC_FILTERBOX_SCENARIO:
            pViewData->GetView()->UseScenario( aString );
            break;
    }

    if (pFilterFloat)
        pFilterFloat->EndPopupMode();

    GrabFocus();        // unter OS/2 stimmt der Focus sonst nicht
}

void ScGridWindow::ExecDataSelect( USHORT nCol, USHORT nRow, const String& rStr )
{
    USHORT nTab = pViewData->GetTabNo();
    if ( rStr.Len() )
        pViewData->GetView()->EnterData( nCol, nRow, nTab, rStr );
}

void ScGridWindow::ExecFilter( ULONG nSel,
                               USHORT nCol, USHORT nRow,
                               const String& aValue )
{
    USHORT nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();

    ScDBData* pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab );
    if (pDBData)
    {
        USHORT i;
        ScQueryParam aParam;
        pDBData->GetQueryParam( aParam );       // kann nur MAXQUERY Eintraege ergeben

        if (SC_AUTOFILTER_CUSTOM == nSel)
        {
//          lcl_StartFilterDialog( pViewData, aParam, Application::GetAppWindow() );
            pViewData->GetView()->SetCursor(nCol,nRow);     //! auch ueber Slot ??
            pViewData->GetDispatcher().Execute( SID_FILTER, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
        }
        else
        {
            BOOL bDeleteOld = FALSE;
            USHORT nQueryPos = 0;
            BOOL bFound = FALSE;
            if (!aParam.bInplace)
                bDeleteOld = TRUE;
            if (aParam.bRegExp)
                bDeleteOld = TRUE;
            for (i=0; i<MAXQUERY && !bDeleteOld; i++)           // bisherige Filter-Einstellungen
                if (aParam.GetEntry(i).bDoQuery)
                {
                    //!         Abfrage mit DrawButtons zusammenfassen!

                    ScQueryEntry& rEntry = aParam.GetEntry(i);
                    if (i>0)
                        if (rEntry.eConnect != SC_AND)
                            bDeleteOld = TRUE;

                    if (rEntry.nField == nCol)
                    {
                        if (bFound)                         // diese Spalte zweimal?
                            bDeleteOld = TRUE;
                        nQueryPos = i;
                        bFound = TRUE;
                    }
                    if (!bFound)
                        nQueryPos = i + 1;
                }

            if (bDeleteOld)
            {
                USHORT nEC = aParam.GetEntryCount();
                for (USHORT i=0; i<nEC; i++)
                    aParam.GetEntry(i).bDoQuery = FALSE;
                nQueryPos = 0;
                aParam.bInplace = TRUE;
                aParam.bRegExp = FALSE;
            }

            if ( nQueryPos < MAXQUERY || SC_AUTOFILTER_ALL == nSel )    // loeschen geht immer
            {
                if (nSel)
                {
                    ScQueryEntry& rNewEntry = aParam.GetEntry(nQueryPos);

                    rNewEntry.bDoQuery       = TRUE;
                    rNewEntry.bQueryByString = TRUE;
                    rNewEntry.nField         = nCol;
                    if ( nSel == SC_AUTOFILTER_TOP10 )
                    {
                        rNewEntry.eOp   = SC_TOPVAL;
                        *rNewEntry.pStr = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("10"));
                    }
                    else
                    {
                        rNewEntry.eOp   = SC_EQUAL;
                        *rNewEntry.pStr = aValue;
                    }
                    if (nQueryPos > 0)
                        rNewEntry.eConnect   = SC_AND;
                }
                else
                {
                    if (bFound)
                        aParam.DeleteQuery(nQueryPos);
                }

                pViewData->GetView()->Query( aParam, NULL, TRUE );
                pDBData->SetQueryParam( aParam );                           // speichern
            }
            else                    //  "Zuviele Bedingungen"
                pViewData->GetView()->ErrorMessage( STR_FILTER_TOOMANY );
        }
    }
    else
        DBG_ERROR("Wo ist der Datenbankbereich?");
}

void ScGridWindow::SetPointer( const Pointer& rPointer )
{
    nCurrentPointer = 0;
    Window::SetPointer( rPointer );
}

void ScGridWindow::MoveMouseStatus( ScGridWindow& rDestWin )
{
    if (nButtonDown)
    {
        rDestWin.nButtonDown = nButtonDown;
        rDestWin.nMouseStatus = nMouseStatus;
    }

    if (bRFMouse)
    {
        rDestWin.bRFMouse = bRFMouse;
        rDestWin.bRFSize  = bRFSize;
        rDestWin.nRFIndex = nRFIndex;
        rDestWin.nRFAddX  = nRFAddX;
        rDestWin.nRFAddY  = nRFAddY;
        bRFMouse = FALSE;
    }

    if (nPagebreakMouse)
    {
        rDestWin.nPagebreakMouse  = nPagebreakMouse;
        rDestWin.nPagebreakBreak  = nPagebreakBreak;
        rDestWin.nPagebreakPrev   = nPagebreakPrev;
        rDestWin.aPagebreakSource = aPagebreakSource;
        rDestWin.aPagebreakDrag   = aPagebreakDrag;
        nPagebreakMouse = SC_PD_NONE;
    }
}

BOOL ScGridWindow::TestMouse( const MouseEvent& rMEvt, BOOL bAction )
{
    //  MouseEvent buttons must only be checked if bAction==TRUE
    //  to allow changing the mouse pointer in MouseMove,
    //  but not start AutoFill with right button (#74229#).
    //  with bAction==TRUE, SetFillMode / SetDragMode is called

    if ( bAction && !rMEvt.IsLeft() )
        return FALSE;

    BOOL bNewPointer = FALSE;

    if (pViewData->IsActive())
    {
        //  Auto-Fill

        ScRange aMarkRange;
        if (pViewData->GetSimpleArea( aMarkRange, FALSE ))
        {
            if ( aMarkRange.aStart.Tab() == pViewData->GetTabNo() )
            {
                //  Block-Ende wie in DrawAutoFillMark
                USHORT nX = aMarkRange.aEnd.Col();
                USHORT nY = aMarkRange.aEnd.Row();

                Point aFillPos = pViewData->GetScrPos( nX, nY, eWhich, TRUE );
                long nSizeXPix;
                long nSizeYPix;
                pViewData->GetMergeSizePixel( nX, nY, nSizeXPix, nSizeYPix );
                aFillPos.X() += nSizeXPix;
                aFillPos.Y() += nSizeYPix;

                Point aMousePos = rMEvt.GetPosPixel();
                //  Abfrage hier passend zu DrawAutoFillMark
                //  (ein Pixel mehr als markiert)
                if ( aMousePos.X() >= aFillPos.X()-3 && aMousePos.X() <= aFillPos.X()+4 &&
                     aMousePos.Y() >= aFillPos.Y()-3 && aMousePos.Y() <= aFillPos.Y()+4 )
                {
                    SetPointer( Pointer( POINTER_CROSS ) );     //! dickeres Kreuz ?
                    if (bAction)
                    {
                        if ( lcl_IsEditableMatrix( pViewData->GetDocument(), aMarkRange ) )
                            pViewData->SetDragMode(
                                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), nX, nY, SC_FILL_MATRIX );
                        else
                            pViewData->SetFillMode(
                                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), nX, nY );
                    }
                    bNewPointer = TRUE;
                }
            }
        }

        //  Embedded-Rechteck

        ScDocument* pDoc = pViewData->GetDocument();
        if (pDoc->IsEmbedded())
        {
            ScTripel aStart;
            ScTripel aEnd;
            pDoc->GetEmbedded( aStart, aEnd );
            if ( pViewData->GetTabNo() == aStart.GetTab() )
            {
                Point aStartPos = pViewData->GetScrPos( aStart.GetCol(), aStart.GetRow(), eWhich );
                Point aEndPos   = pViewData->GetScrPos( aEnd.GetCol()+1, aEnd.GetRow()+1, eWhich );
                Point aMousePos = rMEvt.GetPosPixel();
                BOOL bTop = ( aMousePos.X() >= aStartPos.X()-3 && aMousePos.X() <= aStartPos.X()+1 &&
                              aMousePos.Y() >= aStartPos.Y()-3 && aMousePos.Y() <= aStartPos.Y()+1 );
                BOOL bBottom = ( aMousePos.X() >= aEndPos.X()-3 && aMousePos.X() <= aEndPos.X()+1 &&
                                 aMousePos.Y() >= aEndPos.Y()-3 && aMousePos.Y() <= aEndPos.Y()+1 );
                if ( bTop || bBottom )
                {
                    SetPointer( Pointer( POINTER_CROSS ) );
                    if (bAction)
                    {
                        BYTE nMode = bTop ? SC_FILL_EMBED_LT : SC_FILL_EMBED_RB;
                        pViewData->SetDragMode(
                                    aStart.GetCol(), aStart.GetRow(),
                                    aEnd.GetCol(), aEnd.GetRow(), nMode );
                    }
                    bNewPointer = TRUE;
                }
            }
        }
    }

    if (!bNewPointer && bAction)
    {
//      SetPointer( POINTER_ARROW );            // in Fu...
        pViewData->ResetFillMode();
    }

    return bNewPointer;
}

void __EXPORT ScGridWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    aCurMousePos = rMEvt.GetPosPixel();

    //  Filter-Popup beendet sich mit eigenem Mausklick, nicht erst beim Klick
    //  in das GridWindow, darum ist die folgende Abfrage nicht mehr noetig:
#if 0
    // merken, dass FilterBox geloescht wird, damit sichergestellt
    // ist, dass in diesem Handler nicht an gleicher Stelle wieder
    // eine neue geoeffnet wird.
    BOOL    bWasFilterBox = ( pFilterBox != NULL &&
                                ((Window*)pFilterBox)->IsVisible() &&
                                !pFilterBox->IsDataSelect() );
    USHORT  nOldColFBox   = bWasFilterBox ? pFilterBox->GetCol() : 0;
    USHORT  nOldRowFBox   = bWasFilterBox ? pFilterBox->GetRow() : 0;
#endif

    ClickExtern();  // loescht FilterBox, wenn vorhanden

    HideNoteMarker();   // Notiz-Anzeige

    bEEMouse = FALSE;

    ScModule* pScMod = SC_MOD();
    if (pScMod->IsModalMode(pViewData->GetSfxDocShell()))
    {
        Sound::Beep();
        return;
    }

    pScActiveViewShell = pViewData->GetViewShell();         // falls auf Link geklickt wird
    nScClickMouseModifier = rMEvt.GetModifier();            // um Control-Klick immer zu erkennen

    BOOL bDetective = pViewData->GetViewShell()->IsAuditShell();
    BOOL bRefMode = pViewData->IsRefMode();                 // Referenz angefangen
    BOOL bFormulaMode = pScMod->IsFormulaMode();            // naechster Klick -> Referenz
    BOOL bEditMode = pViewData->HasEditView(eWhich);        // auch bei Mode==SC_INPUT_TYPE

    //  DeactivateIP passiert nur noch bei MarkListHasChanged

    //  im GrabFocus Aufruf kann eine Fehlermeldung hochkommen
    //  (z.B. beim Umbenennen von Tabellen per Tab-Reiter)

    if (!nButtonDown)
        nButtonDown = rMEvt.GetButtons();       // vorher, damit StopMarking klappt

//  pViewData->GetViewShell()->GetViewFrame()->GetWindow().GrabFocus();
    if (bEditMode || !bFormulaMode)
        GrabFocus();

    if ( nMouseStatus == SC_GM_IGNORE )
    {
        nButtonDown = 0;
        nMouseStatus = SC_GM_NONE;
        return;
    }

    if ( bDetective )               // Detektiv-Fuell-Modus
    {
        if ( rMEvt.IsLeft() && !rMEvt.GetModifier() )
        {
            Point   aPos = rMEvt.GetPosPixel();
            short   nPosX;
            short   nPosY;
            pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

            SfxInt16Item aPosXItem( SID_RANGE_COL, nPosX );
            SfxInt16Item aPosYItem( SID_RANGE_ROW, nPosY );
            pViewData->GetDispatcher().Execute( SID_FILL_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                        &aPosXItem, &aPosYItem, (void*)0L );

        }
        nButtonDown = 0;
        nMouseStatus = SC_GM_NONE;
        return;
    }

    BOOL bDouble = (rMEvt.GetClicks() == 2);
    if (!bDouble)
        nMouseStatus = SC_GM_NONE;

    if (!bFormulaMode)
    {
        if ( pViewData->GetActivePart() != eWhich )
            pViewData->GetView()->ActivatePart( eWhich );
    }
    else
    {
        ScViewSelectionEngine* pSelEng = pViewData->GetView()->GetSelEngine();
        pSelEng->SetWindow(this);
        pSelEng->SetWhich(eWhich);
        pSelEng->SetVisibleArea( Rectangle(Point(), GetOutputSizePixel()) );
    }

    if (bEditMode && (pViewData->GetRefTabNo() == pViewData->GetTabNo()))
    {
        Point   aPos = rMEvt.GetPosPixel();
        short   nPosX;
        short   nPosY;
        pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

        EditView*   pEditView;
        USHORT      nEditCol;
        USHORT      nEditRow;
        pViewData->GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        USHORT nEndCol = pViewData->GetEditEndCol();
        USHORT nEndRow = pViewData->GetEditEndRow();

        if ( nPosX >= (short) nEditCol && nPosX <= (short) nEndCol &&
             nPosY >= (short) nEditRow && nPosY <= (short) nEndRow )
        {
            //  #53966# beim Klick in die Tabellen-EditView immer den Focus umsetzen
            if (bFormulaMode)   // sonst ist es oben schon passiert
                GrabFocus();

            pScMod->SetInputMode( SC_INPUT_TABLE );
            bEEMouse = TRUE;
            bEditMode = pEditView->MouseButtonDown( rMEvt );
            return;
        }
    }

    if (pScMod->GetIsWaterCan())
    {
        //!     was is mit'm Mac ???
        if ( rMEvt.GetModifier() + rMEvt.GetButtons() == MOUSE_RIGHT )
        {
            nMouseStatus = SC_GM_WATERUNDO;
            return;
        }
    }

    // Reihenfolge passend zum angezeigten Cursor:
    //  RangeFinder, AutoFill, PageBreak, Drawing

    if ( HitRangeFinder( rMEvt.GetPosPixel(), bRFSize, &nRFIndex, &nRFAddX, &nRFAddY ) )
    {
        bRFMouse = TRUE;        // die anderen Variablen sind oben initialisiert

        if ( pViewData->GetActivePart() != eWhich )
            pViewData->GetView()->ActivatePart( eWhich );   //! schon oben immer ???

        // CaptureMouse();
        StartTracking();
        return;
    }

    BOOL bAutoFill = TestMouse( rMEvt, TRUE );
    if (bAutoFill)
        pScMod->InputEnterHandler();                                // Autofill etc.

    if (!bAutoFill)
    {
        nPagebreakMouse = HitPageBreak( rMEvt.GetPosPixel(), &aPagebreakSource,
                                            &nPagebreakBreak, &nPagebreakPrev );
        if (nPagebreakMouse)
        {
            bPagebreakDrawn = FALSE;
            // CaptureMouse();
            StartTracking();
            PagebreakMove( rMEvt, FALSE );
            return;
        }
    }

    if (!bFormulaMode && !bEditMode && rMEvt.IsLeft())
    {
        if ( !bAutoFill && DrawMouseButtonDown(rMEvt) )
        {
            //if (DrawHasMarkedObj())
            //  pViewData->GetViewShell()->SetDrawShellOrSub();     // Draw-Objekt selektiert
            return;
        }

        pViewData->GetViewShell()->SetDrawShell( FALSE );               // kein Draw-Objekt selektiert

        //  TestMouse schon oben passiert
    }

    Point aPos = rMEvt.GetPosPixel();
    short nPosX;
    short nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    USHORT nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();


            //
            //      Autofilter - Knoepfe
            //

    if ( !bDouble && !bFormulaMode )                //! auf rechte Taste abfragen ???
    {
        ScMergeFlagAttr* pAttr = (ScMergeFlagAttr*)
                                    pDoc->GetAttr( nPosX, nPosY, nTab, ATTR_MERGE_FLAG );
        if (pAttr->HasAutoFilter())
        {
            Point   aScrPos  = pViewData->GetScrPos(nPosX,nPosY,eWhich);
            long    nSizeX;
            long    nSizeY;
            Point   aDiffPix = aPos;

            aDiffPix -= aScrPos;
            pViewData->GetMergeSizePixel( nPosX, nPosY, nSizeX, nSizeY );

            //  Breite des Buttons ist nicht von der Zellhoehe abhaengig
            Size aButSize = aComboButton.GetSizePixel();
            long nButWidth  = Min( aButSize.Width(),  nSizeX );
            long nButHeight = Min( aButSize.Height(), nSizeY );

            if ( aDiffPix.X() >= nSizeX - nButWidth &&
                 aDiffPix.Y() >= nSizeY - nButHeight )
            {
                BOOL  bFilterActive = IsAutoFilterActive( nPosX, nPosY,
                                                          pViewData->GetTabNo() );
                Color aColor( bFilterActive ? COL_LIGHTBLUE : COL_BLACK );

                aComboButton.SetOptSizePixel();
                aComboButton.SetColor( aColor );
                DrawComboButton( aScrPos, nSizeX, nSizeY, TRUE );

#if 0
                if (   bWasFilterBox
                    && (short)nOldColFBox == nPosX
                    && (short)nOldRowFBox == nPosY )
                {
                    // Verhindern, dass an gleicher Stelle eine
                    // FilterBox geoeffnet wird, wenn diese gerade
                    // geloescht wurde

                    nMouseStatus = SC_GM_FILTER; // fuer ButtonDraw im MouseButtonUp();
                    return;
                }
#endif
                DoAutoFilterMenue( nPosX, nPosY, FALSE );

                return;
            }
        }
        if (pAttr->HasButton())
        {
            DoPushButton( nPosX, nPosY, rMEvt );    // setzt evtl. bPivotMouse / bDPMouse
            return;
        }
    }

            //
            //      Szenario-Auswahl
            //

    ScRange aScenRange;
    if ( HasScenarioButton( aPos, aScenRange ) )
    {
        DoScenarioMenue( aScenRange );
        return;
    }

            //
            //      Doppelklick angefangen ?
            //

    // StopMarking kann aus DrawMouseButtonDown gerufen werden

    if ( nMouseStatus != SC_GM_IGNORE && !bRefMode )
    {
        if (bDouble)
        {
            if (nMouseStatus == SC_GM_TABDOWN)
                nMouseStatus = SC_GM_DBLDOWN;
        }
        else
            nMouseStatus = SC_GM_TABDOWN;
    }

            //
            //      Links in Edit-Zellen
            //

    BOOL bAlt = rMEvt.IsMod2();
    if ( !bAlt && rMEvt.IsLeft() &&
            GetEditUrl(rMEvt.GetPosPixel()) )           // Klick auf Link: Cursor nicht bewegen
    {
        SetPointer( Pointer( POINTER_REFHAND ) );
        nMouseStatus = SC_GM_URLDOWN;                   // auch nur dann beim ButtonUp ausfuehren
        return;
    }

            //
            //      Gridwin - SelectionEngine
            //

    if ( rMEvt.IsLeft() )
    {
        ScViewSelectionEngine* pSelEng = pViewData->GetView()->GetSelEngine();
        pSelEng->SetWindow(this);
        pSelEng->SetWhich(eWhich);
        pSelEng->SetVisibleArea( Rectangle(Point(), GetOutputSizePixel()) );

        //  SelMouseButtonDown an der View setzt noch das bMoveIsShift Flag
        if ( pViewData->GetView()->SelMouseButtonDown( rMEvt ) )
        {
            if (IsMouseCaptured())
            {
                //  Tracking statt CaptureMouse, damit sauber abgebrochen werden kann
                //! Irgendwann sollte die SelectionEngine selber StartTracking rufen!?!
                ReleaseMouse();
                StartTracking();
            }
            pViewData->GetMarkData().SetMarking(TRUE);
            return;
        }
    }
}

void __EXPORT ScGridWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    aCurMousePos = rMEvt.GetPosPixel();
    ScDocument* pDoc = pViewData->GetDocument();
    ScMarkData& rMark = pViewData->GetMarkData();

    if (nButtonDown != rMEvt.GetButtons())
        nMouseStatus = SC_GM_IGNORE;            // reset und return

    nButtonDown = 0;

    if (nMouseStatus == SC_GM_IGNORE)
    {
        nMouseStatus = SC_GM_NONE;
                                        // Selection-Engine: Markieren abbrechen
        pViewData->GetView()->GetSelEngine()->Reset();
        rMark.SetMarking(FALSE);
        if (pViewData->IsAnyFillMode())
        {
            pViewData->GetView()->StopRefMode();
            pViewData->ResetFillMode();
        }
        StopMarking();
        DrawEndAction();                // Markieren/Verschieben auf Drawing-Layer abbrechen
        ReleaseMouse();
        return;
    }

    if (nMouseStatus == SC_GM_FILTER)
    {
        if ( pFilterBox && pFilterBox->GetMode() == SC_FILTERBOX_FILTER )
        {
            HideCursor();
            aComboButton.Draw( FALSE );
            ShowCursor();
        }
        nMouseStatus = SC_GM_NONE;
        ReleaseMouse();
        return;                         // da muss nix mehr passieren
    }

    ScModule* pScMod = SC_MOD();
    if (pScMod->IsModalMode(pViewData->GetSfxDocShell()))
        return;

    SfxBindings& rBindings = pViewData->GetBindings();
    if (bEEMouse)
    {
        EditView*   pEditView;
        USHORT      nEditCol;
        USHORT      nEditRow;
        pViewData->GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        pEditView->MouseButtonUp( rMEvt );
        pScMod->InputSelection( pEditView );            // fuer Klammern etc.
        pViewData->GetView()->InvalidateAttribs();
        rBindings.Invalidate( SID_HYPERLINK_GETLINK );
        bEEMouse = FALSE;
        return;
    }

    if (bPivotMouse)
    {
        PivotMouseButtonUp( rMEvt );
        bPivotMouse = FALSE;
        return;
    }

    if (bDPMouse)
    {
        DPMouseButtonUp( rMEvt );       // resets bDPMouse
        return;
    }

    if (bRFMouse)
    {
        RFMouseMove( rMEvt, TRUE );     // Range wieder richtigherum
        bRFMouse = FALSE;
        SetPointer( Pointer( POINTER_ARROW ) );
        ReleaseMouse();
        return;
    }

    if (nPagebreakMouse)
    {
        PagebreakMove( rMEvt, TRUE );
        nPagebreakMouse = SC_PD_NONE;
        SetPointer( Pointer( POINTER_ARROW ) );
        ReleaseMouse();
        return;
    }

    if (nMouseStatus == SC_GM_WATERUNDO)    // Undo im Giesskannenmodus
    {
        SfxUndoManager* pMgr = pViewData->GetDocShell()->GetUndoManager();
        if ( pMgr->GetUndoActionCount() && pMgr->GetUndoActionId() == STR_UNDO_APPLYCELLSTYLE )
            pMgr->Undo();
        else
            Sound::Beep();
        return;
    }

    if (DrawMouseButtonUp(rMEvt))
        return;

    rMark.SetMarking(FALSE);

    SetPointer( Pointer( POINTER_ARROW ) );

    if (pViewData->IsFillMode() ||
        ( pViewData->GetFillMode() == SC_FILL_MATRIX && rMEvt.IsMod1() ))
    {
        nScFillModeMouseModifier = rMEvt.GetModifier();
        USHORT nStartCol;
        USHORT nStartRow;
        USHORT nEndCol;
        USHORT nEndRow;
        pViewData->GetFillData( nStartCol, nStartRow, nEndCol, nEndRow );
//      DBG_ASSERT( nStartCol==pViewData->GetRefStartX() && nStartRow==pViewData->GetRefStartY(),
//                              "Block falsch fuer AutoFill" );
        ScRange aDelRange;
        BOOL bIsDel = pViewData->GetDelMark( aDelRange );

        ScViewFunc* pView = pViewData->GetView();
        pView->StopRefMode();
        pViewData->ResetFillMode();

        if ( bIsDel )
        {
            pView->MarkRange( aDelRange, FALSE );
            pView->DeleteContents( IDF_CONTENTS );
            USHORT nTab = pViewData->GetTabNo();
            ScRange aBlockRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab );
            if ( aBlockRange != aDelRange )
            {
                if ( aDelRange.aStart.Row() == nStartRow )
                    aBlockRange.aEnd.SetCol( aDelRange.aStart.Col() - 1 );
                else
                    aBlockRange.aEnd.SetRow( aDelRange.aStart.Row() - 1 );
                pView->MarkRange( aBlockRange, FALSE );
            }
        }
        else
            pViewData->GetDispatcher().Execute( FID_FILL_AUTO, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
    }
    else if (pViewData->GetFillMode() == SC_FILL_MATRIX)
    {
        USHORT nTab = pViewData->GetTabNo();
        USHORT nStartCol;
        USHORT nStartRow;
        USHORT nEndCol;
        USHORT nEndRow;
        pViewData->GetFillData( nStartCol, nStartRow, nEndCol, nEndRow );
        ScRange aBlockRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab );
        USHORT nFillCol = pViewData->GetRefEndX();
        USHORT nFillRow = pViewData->GetRefEndY();
        ScAddress aEndPos( nFillCol, nFillRow, nTab );

        pViewData->GetView()->StopRefMode();
        pViewData->ResetFillMode();

        if ( aEndPos != aBlockRange.aEnd )
        {
            pViewData->GetDocShell()->GetDocFunc().ResizeMatrix( aBlockRange, aEndPos, FALSE );
            pViewData->GetView()->MarkRange( ScRange( aBlockRange.aStart, aEndPos ) );
        }
    }
    else if (pViewData->IsAnyFillMode())
    {
                                                // Embedded-Area veraendert
        pViewData->GetView()->StopRefMode();
        pViewData->ResetFillMode();
        pViewData->GetDocShell()->UpdateOle(pViewData);
    }

    BOOL bRefMode = pViewData->IsRefMode();
    if (bRefMode)
        pScMod->EndReference();

        //
        //  Giesskannen-Modus (Gestalter)
        //

    if (pScMod->GetIsWaterCan())
    {
        //  Abfrage auf Undo schon oben

        ScStyleSheetPool* pStylePool = (ScStyleSheetPool*)
                                       (pViewData->GetDocument()->
                                            GetStyleSheetPool());
        if ( pStylePool )
        {
            SfxStyleSheet* pStyleSheet = (SfxStyleSheet*)
                                         pStylePool->GetActualStyleSheet();

            if ( pStyleSheet )
            {
                SfxStyleFamily eFamily = pStyleSheet->GetFamily();

                switch ( eFamily )
                {
                    case SFX_STYLE_FAMILY_PARA:
                        pViewData->GetView()->SetStyleSheetToMarked( pStyleSheet );
                        pViewData->GetView()->DoneBlockMode();
                        break;

                    case SFX_STYLE_FAMILY_PAGE:
                        pViewData->GetDocument()->SetPageStyle( pViewData->GetTabNo(),
                                                                pStyleSheet->GetName() );

                        ScPrintFunc( pViewData->GetDocShell(),
                                     pViewData->GetViewShell()->GetPrinter(),
                                     pViewData->GetTabNo() ).UpdatePages();

                        rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                        break;

                    default:
                        break;
                }
            }
        }
    }

            //
            //      double click (only left button)
            //

    BOOL bDouble = ( rMEvt.GetClicks() == 2 && rMEvt.IsLeft() );
    if ( bDouble && !bRefMode && nMouseStatus == SC_GM_DBLDOWN && !pScMod->IsRefDialogOpen() )
    {
        //  data pilot table
        Point aPos = rMEvt.GetPosPixel();
        short nPosX, nPosY;
        pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
        ScDPObject* pDPObj  = pDoc->GetDPAtCursor( nPosX, nPosY, pViewData->GetTabNo() );
        if (pDPObj)
        {
            ScAddress aPos( nPosX, nPosY, pViewData->GetTabNo() );
            ScDPPositionData aData;
            pDPObj->GetPositionData( aData, aPos );

            if ( ( aData.nFlags & sheet::MemberResultFlags::HASMEMBER ) &&
                 ! ( aData.nFlags & sheet::MemberResultFlags::SUBTOTAL ) )
            {
                ScDPObject aNewObj( *pDPObj );
                pDPObj->ToggleDetails( aData, &aNewObj );
                ScDBDocFunc aFunc( *pViewData->GetDocShell() );
                aFunc.DataPilotUpdate( pDPObj, &aNewObj, TRUE, FALSE );
                pViewData->GetView()->CursorPosChanged();       // shells may be switched
            }
            else
                Sound::Beep();      // nothing to expand/collapse

            return;
        }

        //  edit cell contents
        pViewData->GetViewShell()->UpdateInputHandler();
        pScMod->SetInputMode( SC_INPUT_TABLE );
        if (pViewData->HasEditView(eWhich))
        {
            //  Text-Cursor gleich an die geklickte Stelle setzen
            EditView* pEditView = pViewData->GetEditView( eWhich );
            MouseEvent aEditEvt( rMEvt.GetPosPixel(), 1, MOUSE_SYNTHETIC, MOUSE_LEFT, 0 );
            pEditView->MouseButtonDown( aEditEvt );
            pEditView->MouseButtonUp( aEditEvt );
        }
    }

            //
            //      Links in Edit-Zellen
            //

    BOOL bAlt = rMEvt.IsMod2();
    if ( !bAlt && !bRefMode && !bDouble && nMouseStatus == SC_GM_URLDOWN )
    {
        //  beim ButtonUp nur ausfuehren, wenn ButtonDown auch ueber einer URL war

        String aName, aUrl, aTarget;
        if ( GetEditUrl( rMEvt.GetPosPixel(), &aName, &aUrl, &aTarget ) )
        {
            nMouseStatus = SC_GM_NONE;              // keinen Doppelklick anfangen
            ScGlobal::OpenURL( aUrl, aTarget );
            return;
        }
    }

            //
            //      Gridwin - SelectionEngine
            //

    if ( pViewData->GetView()->GetSelEngine()->SelMouseButtonUp( rMEvt ) )
    {
//      rMark.MarkToSimple();
        pViewData->GetView()->UpdateAutoFillMark();

        SfxDispatcher* pDisp = pViewData->GetViewShell()->GetDispatcher();
        BOOL bRefMode = pScMod->IsFormulaMode();
        DBG_ASSERT( pDisp || bRefMode, "Cursor auf nicht aktiver View bewegen ?" );

        if ( pDisp && !bRefMode )                       // noch ein Execute fuer Basic
        {
            String aAddr;                               // CurrentCell
            if( rMark.IsMarked() )
            {
//              BOOL bKeep = rMark.IsMultiMarked();     //! wohin damit ???

                ScRange aScRange;
                rMark.GetMarkArea( aScRange );
                aScRange.Format( aAddr, SCR_ABS );

                //! SID_MARKAREA gibts nicht mehr ???
                //! was passiert beim Markieren mit dem Cursor ???
            }
            else                                        // nur Cursor bewegen
            {
                ScAddress aScAddress( pViewData->GetCurX(), pViewData->GetCurY(), 0 );
                aScAddress.Format( aAddr, SCA_ABS );
            }

            SfxStringItem aPosItem( SID_CURRENTCELL, aAddr );
            pDisp->Execute( SID_CURRENTCELL, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                        &aPosItem, (void*)0L );

            pViewData->GetView()->InvalidateAttribs();
        }
        return;
    }
}

void ScGridWindow::FakeButtonUp()
{
    if ( nButtonDown )
    {
        MouseEvent aEvent( aCurMousePos );      // nButtons = 0 -> ignore
        MouseButtonUp( aEvent );
    }
}

void __EXPORT ScGridWindow::MouseMove( const MouseEvent& rMEvt )
{
    aCurMousePos = rMEvt.GetPosPixel();

    if ( rMEvt.IsLeaveWindow() )
        HideNoteMarker();

    ScModule* pScMod = SC_MOD();
    if (pScMod->IsModalMode(pViewData->GetSfxDocShell()))
        return;

        //  Ob aus dem Edit-Modus Drag&Drop gestartet wurde, bekommt man leider
        //  nicht anders mit:

    if (bEEMouse && nButtonDown && !rMEvt.GetButtons())
    {
        bEEMouse = FALSE;
        nButtonDown = 0;
        nMouseStatus = SC_GM_NONE;
        return;
    }

    if (nMouseStatus == SC_GM_IGNORE)
        return;

    if (nMouseStatus == SC_GM_WATERUNDO)    // Undo im Giesskannenmodus -> nur auf Up warten
        return;

    if ( pViewData->GetViewShell()->IsAuditShell() )        // Detektiv-Fuell-Modus
    {
        SetPointer( Pointer( POINTER_FILL ) );
        return;
    }

    if (nMouseStatus == SC_GM_FILTER && pFilterBox)
    {
        Point aRelPos = pFilterBox->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) );
        if ( Rectangle(Point(),pFilterBox->GetOutputSizePixel()).IsInside(aRelPos) )
        {
            nButtonDown = 0;
            nMouseStatus = SC_GM_NONE;
            if ( pFilterBox->GetMode() == SC_FILTERBOX_FILTER )
            {
                HideCursor();
                aComboButton.Draw( FALSE );
                ShowCursor();
            }
            ReleaseMouse();
            pFilterBox->MouseButtonDown( MouseEvent( aRelPos, 1, MOUSE_SIMPLECLICK, MOUSE_LEFT ) );
            return;
        }
    }

    BOOL bRefMode = pViewData->IsRefMode();                 // Referenz angefangen
    BOOL bFormulaMode = pScMod->IsFormulaMode();            // naechster Klick -> Referenz

    if (bEEMouse)
    {
        EditView*   pEditView;
        USHORT      nEditCol;
        USHORT      nEditRow;
        pViewData->GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        pEditView->MouseMove( rMEvt );
        return;
    }

    if (bPivotMouse)
    {
        PivotMouseMove( rMEvt );
        return;
    }

    if (bDPMouse)
    {
        DPMouseMove( rMEvt );
        return;
    }

    if (bRFMouse)
    {
        RFMouseMove( rMEvt, FALSE );
        return;
    }

    if (nPagebreakMouse)
    {
        PagebreakMove( rMEvt, FALSE );
        return;
    }

    //  anderen Mauszeiger anzeigen?

    BOOL bEditMode = pViewData->HasEditView(eWhich);

                    //! Testen ob RefMode-Dragging !!!
    if ( bEditMode && (pViewData->GetRefTabNo() == pViewData->GetTabNo()) )
    {
        Point   aPos = rMEvt.GetPosPixel();
        short   nPosX;
        short   nPosY;
        pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

        EditView*   pEditView;
        USHORT      nEditCol;
        USHORT      nEditRow;
        pViewData->GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        USHORT nEndCol = pViewData->GetEditEndCol();
        USHORT nEndRow = pViewData->GetEditEndRow();

        if ( nPosX >= (short) nEditCol && nPosX <= (short) nEndCol &&
             nPosY >= (short) nEditRow && nPosY <= (short) nEndRow )
        {
            //  Field can only be URL field
            BOOL bAlt = rMEvt.IsMod2();
            if ( !bAlt && !nButtonDown && pEditView && pEditView->GetFieldUnderMousePointer() )
                SetPointer( Pointer( POINTER_REFHAND ) );
            else
                SetPointer( Pointer( POINTER_TEXT ) );
            return;
        }
    }

    BOOL bWater = SC_MOD()->GetIsWaterCan();
    if (bWater)
        SetPointer( Pointer(POINTER_FILL) );

    if (!bWater)
    {
        BOOL bCross = FALSE;

        //  Range-Finder

        BOOL bCorner;
        if ( HitRangeFinder( rMEvt.GetPosPixel(), bCorner ) )
        {
            if (bCorner)
                SetPointer( Pointer( POINTER_CROSS ) );
            else
                SetPointer( Pointer( POINTER_HAND ) );
            bCross = TRUE;
        }

        //  Page-Break-Modus

        USHORT nBreakType;
        if ( !nButtonDown && pViewData->IsPagebreakMode() &&
                ( nBreakType = HitPageBreak( rMEvt.GetPosPixel() ) ) )
        {
            PointerStyle eNew = POINTER_ARROW;
            switch ( nBreakType )
            {
                case SC_PD_RANGE_L:
                case SC_PD_RANGE_R:
                case SC_PD_BREAK_H:
                    eNew = POINTER_ESIZE;
                    break;
                case SC_PD_RANGE_T:
                case SC_PD_RANGE_B:
                case SC_PD_BREAK_V:
                    eNew = POINTER_SSIZE;
                    break;
                case SC_PD_RANGE_TL:
                case SC_PD_RANGE_BR:
                    eNew = POINTER_SESIZE;
                    break;
                case SC_PD_RANGE_TR:
                case SC_PD_RANGE_BL:
                    eNew = POINTER_NESIZE;
                    break;
            }
            SetPointer( Pointer( eNew ) );
            bCross = TRUE;
        }

        //  Fill-Cursor anzeigen ?

        if ( !bFormulaMode && !nButtonDown )
            if (TestMouse( rMEvt, FALSE ))
                bCross = TRUE;

        if ( nButtonDown && pViewData->IsAnyFillMode() )
        {
            SetPointer( Pointer( POINTER_CROSS ) );
            bCross = TRUE;
            nScFillModeMouseModifier = rMEvt.GetModifier(); // ausgewertet bei AutoFill und Matrix
        }

        if (!bCross)
        {
            BOOL bAlt = rMEvt.IsMod2();

            if (bEditMode)                                  // Edit-Mode muss zuerst kommen!
                SetPointer( Pointer( POINTER_ARROW ) );
            else if ( !bAlt && !nButtonDown &&
                        GetEditUrl(rMEvt.GetPosPixel()) )
                SetPointer( Pointer( POINTER_REFHAND ) );
            else if ( DrawMouseMove(rMEvt) )                // setzt Pointer um
                return;
        }
    }

    if ( pViewData->GetView()->GetSelEngine()->SelMouseMove( rMEvt ) )
        return;
}

void ScGridWindow::Tracking( const TrackingEvent& rTEvt )
{
    //  Weil die SelectionEngine kein Tracking kennt, die Events nur auf
    //  die verschiedenen MouseHandler verteilen...

    const MouseEvent& rMEvt = rTEvt.GetMouseEvent();

    if ( rTEvt.IsTrackingCanceled() )       // alles abbrechen...
    {
        if (!pViewData->GetView()->IsInActivatePart())
        {
            if (bPivotMouse)
                bPivotMouse = FALSE;            // gezeichnet wird per bDragRect
            if (bDPMouse)
                bDPMouse = FALSE;               // gezeichnet wird per bDragRect
            if (bDragRect)
            {
                pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );
                bDragRect = FALSE;
            }
            if (bRFMouse)
            {
                RFMouseMove( rMEvt, TRUE );     // richtig abbrechen geht dabei nicht...
                bRFMouse = FALSE;
            }
            if (nPagebreakMouse)
            {
                if (bPagebreakDrawn)
                    DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
                                    aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), FALSE );
                bPagebreakDrawn = FALSE;
                nPagebreakMouse = SC_PD_NONE;
            }

            SetPointer( Pointer( POINTER_ARROW ) );
            StopMarking();
            MouseButtonUp( rMEvt );     // mit Status SC_GM_IGNORE aus StopMarking

            BOOL bRefMode = pViewData->IsRefMode();
            if (bRefMode)
                SC_MOD()->EndReference();       // #63148# Dialog nicht verkleinert lassen
        }
    }
    else if ( rTEvt.IsTrackingEnded() )
    {
        //  MouseButtonUp immer mit passenden Buttons (z.B. wegen Testtool, #63148#)
        //  Schliesslich behauptet der Tracking-Event ja, dass normal beendet und nicht
        //  abgebrochen wurde.

        MouseEvent aUpEvt( rMEvt.GetPosPixel(), rMEvt.GetClicks(),
                            rMEvt.GetMode(), nButtonDown, rMEvt.GetModifier() );
        MouseButtonUp( aUpEvt );
    }
    else
        MouseMove( rMEvt );
}

void ScGridWindow::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    if ( pFilterBox || nPagebreakMouse )
        return;

    HideNoteMarker();

    CommandEvent aDragEvent( rPosPixel, COMMAND_STARTDRAG, TRUE );

    if (bEEMouse)
    {
        EditView*   pEditView;
        USHORT      nEditCol;
        USHORT      nEditRow;
        pViewData->GetEditView( eWhich, pEditView, nEditCol, nEditRow );

        // #63263# don't remove the edit view while switching views
        ScModule* pScMod = SC_MOD();
        pScMod->SetInEditCommand( TRUE );

        pEditView->Command( aDragEvent );

        ScInputHandler* pHdl = pScMod->GetInputHdl();
        if (pHdl)
            pHdl->DataChanged();

        pScMod->SetInEditCommand( FALSE );
        if (!pViewData->IsActive())             // dropped to different view?
        {
            ScInputHandler* pHdl = pScMod->GetInputHdl( pViewData->GetViewShell() );
            if ( pHdl && pViewData->HasEditView( eWhich ) )
            {
                pHdl->CancelHandler();
                ShowCursor();   // missing from KillEditView
            }
        }
    }
    else
        if ( !DrawCommand(aDragEvent) )
            pViewData->GetView()->GetSelEngine()->Command( aDragEvent );
}

void __EXPORT ScGridWindow::Command( const CommandEvent& rCEvt )
{
    USHORT nCmd = rCEvt.GetCommand();
    ScModule* pScMod = SC_MOD();
    DBG_ASSERT( nCmd != COMMAND_STARTDRAG, "ScGridWindow::Command called with COMMAND_STARTDRAG" );

    if ( nCmd == COMMAND_STARTEXTTEXTINPUT ||
         nCmd == COMMAND_ENDEXTTEXTINPUT ||
         nCmd == COMMAND_EXTTEXTINPUT ||
         nCmd == COMMAND_CURSORPOS )
    {
        BOOL bEditView = pViewData->HasEditView( eWhich );
        if (!bEditView)
        {
            //  only if no cell editview is active, look at drawview
            SdrView* pSdrView = pViewData->GetView()->GetSdrView();
            if ( pSdrView )
            {
                OutlinerView* pOlView = pSdrView->GetTextEditOutlinerView();
                if ( pOlView && pOlView->GetWindow() == this )
                {
                    pOlView->Command( rCEvt );
                    return;                             // done
                }
            }
        }

        ScInputHandler* pHdl = pScMod->GetInputHdl( pViewData->GetViewShell() );
        if ( pHdl )
        {
            pHdl->InputCommand( rCEvt, TRUE );
            return;                                     // done
        }

        Window::Command( rCEvt );
        return;
    }

    if ( nCmd == COMMAND_VOICE )
    {
        //  Der Handler wird nur gerufen, wenn ein Text-Cursor aktiv ist,
        //  also muss es eine EditView oder ein editiertes Zeichenobjekt geben

        ScInputHandler* pHdl = pScMod->GetInputHdl( pViewData->GetViewShell() );
        if ( pHdl && pViewData->HasEditView( eWhich ) )
        {
            EditView* pEditView = pViewData->GetEditView( eWhich ); // ist dann nicht 0
            pHdl->DataChanging();
            pEditView->Command( rCEvt );
            pHdl->DataChanged();
            return;                                     // erledigt
        }
        SdrView* pSdrView = pViewData->GetView()->GetSdrView();
        if ( pSdrView )
        {
            OutlinerView* pOlView = pSdrView->GetTextEditOutlinerView();
            if ( pOlView && pOlView->GetWindow() == this )
            {
                pOlView->Command( rCEvt );
                return;                                 // erledigt
            }
        }
        Window::Command(rCEvt);     //  sonst soll sich die Basisklasse drum kuemmern...
        return;
    }

    BOOL bDisable = pScMod->IsFormulaMode() ||
                    pScMod->IsModalMode(pViewData->GetSfxDocShell());
    if (bDisable)
        return;

    if ( nCmd == COMMAND_WHEEL || nCmd == COMMAND_STARTAUTOSCROLL || nCmd == COMMAND_AUTOSCROLL )
    {
        BOOL bDone = pViewData->GetView()->ScrollCommand( rCEvt, eWhich );
        if (!bDone)
            Window::Command(rCEvt);
        return;
    }

    if ( nCmd == COMMAND_CONTEXTMENU && !SC_MOD()->GetIsWaterCan() )
    {
        if (pViewData->IsAnyFillMode())
        {
            pViewData->GetView()->StopRefMode();
            pViewData->ResetFillMode();
        }
        ReleaseMouse();
        StopMarking();

        Point aPosPixel = rCEvt.GetMousePosPixel();
        BOOL bDone = FALSE;
        BOOL bEdit = pViewData->HasEditView(eWhich);
        if ( !bEdit )
        {
                // Edit-Zelle mit Spelling-Errors ?
            if ( GetEditUrlOrError( TRUE, aPosPixel ) )
            {
                //  GetEditUrlOrError hat den Cursor schon bewegt

                pScMod->SetInputMode( SC_INPUT_TABLE );
                bEdit = pViewData->HasEditView(eWhich);     // hat's geklappt ?

                DBG_ASSERT( bEdit, "kann nicht in Edit-Modus schalten" );
            }
        }
        if ( bEdit )
        {
            EditView* pEditView = pViewData->GetEditView( eWhich );     // ist dann nicht 0

            //  if edit mode was just started above, online spelling may be incomplete
            pEditView->GetEditEngine()->CompleteOnlineSpelling();

            if( pEditView->IsWrongSpelledWordAtPos( aPosPixel ) )
            {
                //  Wenn man unter OS/2 neben das Popupmenue klickt, kommt MouseButtonDown
                //  vor dem Ende des Menue-Execute, darum muss SetModified vorher kommen
                //  (Bug #40968#)
                ScInputHandler* pHdl = pScMod->GetInputHdl();
                if (pHdl)
                    pHdl->SetModified();

                pEditView->ExecuteSpellPopup( aPosPixel );

                bDone = TRUE;
            }
        }

        if (!bDone)
        {
            SfxDispatcher::ExecutePopup();
        }
    }
}

void __EXPORT ScGridWindow::KeyInput(const KeyEvent& rKEvt)
{
    // wenn semi-Modeless-SfxChildWindow-Dialog oben, keine KeyInputs:
    if ( !SC_MOD()->IsRefDialogOpen() && !pViewData->IsAnyFillMode() )
    {
        ScTabViewShell* pViewSh = pViewData->GetViewShell();

        if (pViewData->GetDocShell()->GetProgress())
            return;

        if (DrawKeyInput(rKEvt))
            return;

        if (!pViewData->GetView()->IsDrawSelMode() && !DrawHasMarkedObj())  //  keine Eingaben im Zeichenmodus
        {                                                           //! DrawShell abfragen !!!
            if (pViewSh->TabKeyInput(rKEvt))
                return;
        }
        else
            if (pViewSh->SfxViewShell::KeyInput(rKEvt))             // von SfxViewShell
                return;

        KeyCode aCode = rKEvt.GetKeyCode();
        if ( aCode.GetCode() == KEY_ESCAPE && aCode.GetModifier() == 0 )
        {
            pViewData->GetDocShell()->DoInPlaceActivate(FALSE);
            return;
        }
    }

    Window::KeyInput(rKEvt);
}

void ScGridWindow::StopMarking()
{
    DrawEndAction();                // Markieren/Verschieben auf Drawing-Layer abbrechen

    if (nButtonDown)
    {
        pViewData->GetMarkData().SetMarking(FALSE);
        nMouseStatus = SC_GM_IGNORE;
    }
}

void ScGridWindow::UpdateInputContext()
{
    BOOL bReadOnly = pViewData->GetDocShell()->IsReadOnly();
    ULONG nOptions = bReadOnly ? 0 : ( INPUTCONTEXT_TEXT | INPUTCONTEXT_EXTTEXTINPUT );

    //  when font from InputContext is used,
    //  it must be taken from the cursor position's cell attributes

    InputContext aContext;
    aContext.SetOptions( nOptions );
    SetInputContext( aContext );
}

//--------------------------------------------------------

BOOL ScGridWindow::QueryDropPrivate( DropEvent& rEvt )
{
    if (rEvt.IsLeaveWindow())
    {
        if (bDragRect)
            pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );
        bDragRect = FALSE;
        return TRUE;
    }

#ifdef OLD_DND
    const ScDragData& rData = SC_MOD()->GetDragData();
    if ( rData.nSizeX != 0 && rData.nSizeY != 0 )
    {
        Point aPos = rEvt.GetPosPixel();

        ScDocument* pSourceDoc = rData.pDoc;
        ScDocument* pThisDoc   = pViewData->GetDocument();
        if (pSourceDoc == pThisDoc)
        {
            if ( pThisDoc->HasChartAtPoint(pViewData->GetTabNo(), PixelToLogic(aPos)) )
            {
                if (bDragRect)          // Rechteck loeschen
                {
                    pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );
                    bDragRect = FALSE;
                }

                //! Chart mit Rechteck markieren ?

                BOOL bOk = TRUE;
                if ( rEvt.GetAction() == DROP_LINK )
                    bOk = rEvt.SetAction( DROP_COPY );          // Link auf Chart geht nicht
                return bOk;
            }
        }
        else
            if ( rEvt.GetAction() == DROP_MOVE )
                rEvt.SetAction( DROP_COPY );                    // anderes Doc: Default=COPY


        if ( rData.nFlags & SC_DROP_TABLE )             // ganze Tabelle ?
        {
            BOOL bOk = pThisDoc->IsDocEditable();
            return bOk;                                 // keinen Rahmen zeichnen
        }

        short   nPosX;
        short   nPosY;
        pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

        short nNewDragX = nPosX-rData.nHandleX;
        if (nNewDragX<0) nNewDragX=0;
        if (nNewDragX+(rData.nSizeX-1) > MAXCOL)
            nNewDragX = MAXCOL-(rData.nSizeX-1);
        short nNewDragY = nPosY-rData.nHandleY;
        if (nNewDragY<0) nNewDragY=0;
        if (nNewDragY+(rData.nSizeY-1) > MAXROW)
            nNewDragY = MAXROW-(rData.nSizeY-1);

        if ( nNewDragX != (short) nDragStartX || nNewDragY != (short) nDragStartY || !bDragRect )
        {
            if (bDragRect)
                pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );

            nDragStartX = nNewDragX;
            nDragStartY = nNewDragY;
            nDragEndX = nDragStartX+rData.nSizeX-1;
            nDragEndY = nDragStartY+rData.nSizeY-1;
            bDragRect = TRUE;

            pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );

            //  Zielposition als Tip-Hilfe anzeigen
#if 0
            if (Help::IsQuickHelpEnabled())
            {
                USHORT nTab = pViewData->GetTabNo();
                ScRange aRange( nDragStartX, nDragStartY, nTab, nDragEndX, nDragEndY, nTab );
                String aHelpStr;
                aRange.Format( aHelpStr, SCA_VALID );   // nicht-3D

                Point aPos = Pointer::GetPosPixel();
                USHORT nAlign = QUICKHELP_BOTTOM|QUICKHELP_RIGHT;
                Rectangle aRect( aPos, aPos );
                Help::ShowQuickHelp(aRect, aHelpStr, nAlign);
            }
#endif
        }
    }
#endif

    return TRUE;
}

                                // sensitiver Bereich (Pixel)
#define SCROLL_SENSITIVE 20

BOOL ScGridWindow::DropScroll( const Point& rMousePos )
{
/*  doch auch auf nicht aktiven Views...
    if ( !pViewData->IsActive() )
        return FALSE;
*/
    short nDx = 0;
    short nDy = 0;
    Size aSize = GetOutputSizePixel();

    if (aSize.Width() > SCROLL_SENSITIVE * 3)
    {
        if ( rMousePos.X() < SCROLL_SENSITIVE && pViewData->GetPosX(WhichH(eWhich)) > 0 )
            nDx = -1;
        if ( rMousePos.X() >= aSize.Width() - SCROLL_SENSITIVE
                && pViewData->GetPosX(WhichH(eWhich)) < MAXCOL )
            nDx = 1;
    }
    if (aSize.Height() > SCROLL_SENSITIVE * 3)
    {
        if ( rMousePos.Y() < SCROLL_SENSITIVE && pViewData->GetPosY(WhichV(eWhich)) > 0 )
            nDy = -1;
        if ( rMousePos.Y() >= aSize.Height() - SCROLL_SENSITIVE
                && pViewData->GetPosY(WhichV(eWhich)) < MAXROW )
            nDy = 1;
    }

    if ( nDx || nDy )
    {
        if (bDragRect)
            pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );

        if ( nDx )
            pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy )
            pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );

        if (bDragRect)
            pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );
    }

    return FALSE;
}

BOOL lcl_TestScenarioRedliningDrop( ScDocument* pDoc, const ScRange& aDragRange)
{
    //  Testet, ob bei eingeschalteten RedLining,
    //  bei einem Drop ein Scenario betroffen ist.

    BOOL bReturn = FALSE;
    USHORT nTab = aDragRange.aStart.Tab();
    USHORT nTabCount = pDoc->GetTableCount();

    if(pDoc->GetChangeTrack()!=NULL)
    {
        if( pDoc->IsScenario(nTab) && pDoc->HasScenarioRange(nTab, aDragRange))
        {
            bReturn = TRUE;
        }
        else
        {
            for(USHORT i=nTab+1; i<nTabCount && pDoc->IsScenario(i); i++)
            {
                if(pDoc->HasScenarioRange(i, aDragRange))
                {
                    bReturn = TRUE;
                    break;
                }
            }
        }
    }
    return bReturn;
}


#ifdef OLD_DND
ScRange lcl_MakeDropRange(short nPosX, short nPosY,USHORT nTab, const ScDragData& rData)
{
    // Liefert den Range fuer einen Drop zurueck

    short nCol1 = nPosX-rData.nHandleX;
    if (nCol1<0) nCol1=0;
    if (nCol1+(rData.nSizeX-1) > MAXCOL)
        nCol1 = MAXCOL-(rData.nSizeX-1);
    short nRow1 = nPosY-rData.nHandleY;
    if (nRow1<0) nRow1=0;
    if (nRow1+(rData.nSizeY-1) > MAXROW)
        nRow1 = MAXROW-(rData.nSizeY-1);

    USHORT nCol2 = nCol1+rData.nSizeX-1;
    USHORT nRow2 = nRow1+rData.nSizeY-1;

    return ScRange(nCol1, nRow1, nTab,
                    nCol2, nRow2, nTab);
}
#endif

BOOL __EXPORT ScGridWindow::QueryDrop( DropEvent& rEvt )
{
    const ScDragData& rData = SC_MOD()->GetDragData();
    if (rEvt.IsLeaveWindow())
    {
        DrawMarkDropObj( NULL );
        if (rData.pCellTransfer)
            return QueryDropPrivate( rEvt );    // hide drop marker for internal D&D
        else
            return TRUE;
    }

    if ( pViewData->GetDocShell()->IsReadOnly() )
        return FALSE;

    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();

    short   nPosX;
    short   nPosY;

    Point aPos = rEvt.GetPosPixel();
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

#ifdef OLD_DND
    ScRange aDropRange = lcl_MakeDropRange(nPosX,nPosY,nTab,rData);
    ScRange aSourceRange =lcl_MakeDropRange(rData.nStartX,rData.nStartY,nTab,rData);

    if( lcl_TestScenarioRedliningDrop(pDoc, aDropRange)||
        lcl_TestScenarioRedliningDrop(pDoc, aSourceRange))
    {
        if (bDragRect)
        {
            bDragRect = FALSE;
            pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );
        }
        return FALSE;
    }
#endif

    BOOL bReturn = FALSE;

    if (rData.pCellTransfer)
    {
#ifdef OLD_DND
        if ( rData.nSizeX<=MAXCOL && rData.nSizeY<=MAXROW && !rEvt.IsLeaveWindow() )
            DropScroll( rEvt.GetPosPixel() );
#endif
        bReturn = QueryDropPrivate( rEvt );
    }
    else
    {
        if ( rData.aLinkDoc.Len() )
        {
            String aThisName;
            ScDocShell* pDocSh = pViewData->GetDocShell();
            if (pDocSh && pDocSh->HasName())
                aThisName = pDocSh->GetMedium()->GetName();

            if ( rData.aLinkDoc != aThisName )
                bReturn = TRUE;
        }
        else if (rData.aJumpTarget.Len())
        {
            //  interne Bookmarks (aus Navigator)
            //  lokale Spruenge aus unbenanntem Dokument nur in dasselbe Dokument erlaubt

            if ( rData.pJumpLocalDoc )
                bReturn = ( rData.pJumpLocalDoc == pViewData->GetDocument() );
            else
                bReturn = TRUE;
        }
        else
        {
            //  wenn nicht im Dokument, dann Default = COPY

#ifdef OLD_DND
            if ( !IsMyModel(rData.pSdrView) )               // Drawing innerhalb des Doc?
                if ( rEvt.IsDefaultAction() && rEvt.GetAction() == DROP_MOVE )
                    rEvt.SetAction( DROP_COPY );
#endif

            SvDataObjectRef pObject = SvDataObject::PasteDragServer(rEvt);
            DropAction eAction = rEvt.GetAction();

            ScDocument* pThisDoc = pViewData->GetDocument();
            SdrObject* pHitObj = pThisDoc->GetObjectAtPoint(
                pViewData->GetTabNo(), PixelToLogic(rEvt.GetPosPixel()) );
            if ( pHitObj && eAction == DROP_LINK && !rData.pDrawTransfer )
            {
                if ( pObject->HasFormat(SOT_FORMATSTR_ID_SVXB)
                    || pObject->HasFormat(FORMAT_GDIMETAFILE)
                    || pObject->HasFormat(FORMAT_BITMAP) )
                {
                    //  Grafik auf Zeichenobjekt gezogen
                    DrawMarkDropObj( pHitObj );
                    bReturn = TRUE;
                }
            }
            if (!bReturn)
                DrawMarkDropObj( NULL );

            if ( !bReturn )
            {
                switch (eAction)
                {
                    case DROP_MOVE:
                    case DROP_COPY:
                        //  einfuegen: alles und jeden
                        //      aber FORMAT_FILE nicht als MOVE
                        //      (sonst wuerde das File vom Beamer geloescht, #40299#)

                        if(pThisDoc->GetChangeTrack()!=NULL &&
                            pObject->HasFormat(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE))
                        {
                            bReturn=FALSE;
                        }
                        else
                        {
                            bReturn = pObject->HasFormat(SOT_FORMATSTR_ID_EMBED_SOURCE);
                            bReturn |=pObject->HasFormat(SOT_FORMATSTR_ID_LINK_SOURCE);
                            bReturn |=pObject->HasFormat(SOT_FORMATSTR_ID_EMBED_SOURCE_OLE);
                            bReturn |=pObject->HasFormat(SOT_FORMATSTR_ID_LINK_SOURCE_OLE);
                            bReturn |=ScImportExport::IsFormatSupported( pObject );
                            bReturn |=pObject->HasFormat(SOT_FORMATSTR_ID_LINK);        // DDE
                            bReturn |=pObject->HasFormat(SOT_FORMATSTR_ID_DRAWING);
                            bReturn |=pObject->HasFormat(SOT_FORMATSTR_ID_SVXB);
                            bReturn |=pObject->HasFormat(FORMAT_RTF);
                            bReturn |=pObject->HasFormat(FORMAT_GDIMETAFILE);
                            bReturn |=pObject->HasFormat(FORMAT_BITMAP);
                            bReturn |=( pObject->HasFormat(FORMAT_FILE) && eAction != DROP_MOVE );
                            bReturn |=pObject->HasFormat(
                                Exchange::RegisterFormatName(
                                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
                                        SBA_FIELDEXCHANGE_FORMAT))));
                            bReturn |=pObject->HasFormat(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE);
                            bReturn |=( INetBookmark::HasFormat(*pObject) && eAction != DROP_MOVE );
                        }
                        break;
                    case DROP_LINK:
                        //  linken: OLE und DDE, Datei weil es eine Grafik (Gallery) sein koennte
                        //  Bookmarks, weil sie vom Internet Explorer als Link angeboten werden
                        bReturn =  pObject->HasFormat(SOT_FORMATSTR_ID_LINK_SOURCE)
                                || pObject->HasFormat(SOT_FORMATSTR_ID_LINK_SOURCE_OLE)
                                || pObject->HasFormat(SOT_FORMATSTR_ID_LINK)
                                || pObject->HasFormat(FORMAT_FILE)
                                || INetBookmark::HasFormat(*pObject);
                        break;
                    default:
                        bReturn = FALSE;        // hamma nich
                }
            }
        }

                        //  Scrolling nur, wenn wir was damit anfangen koennen
        if (bReturn)
            DropScroll( rEvt.GetPosPixel() );
    }

    return bReturn;
}

//--------------------------------------------------------

BOOL ScGridWindow::DropPrivate( const DropEvent& rEvt )
{
    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();

    BOOL bRet = TRUE;

#ifdef OLD_DND
    ScDocument* pSourceDoc = rData.pDoc;
    ScDocument* pThisDoc   = pViewData->GetDocument();
    ScViewFunc* pView      = pViewData->GetView();
    BOOL bIsNavi = ( rData.nFlags & SC_DROP_NAVIGATOR ) != 0;
    BOOL bIsMove = ( rEvt.GetAction()==DROP_MOVE && !bIsNavi );
    BOOL bIsLink = ( rEvt.GetAction()==DROP_LINK );

    ScRange aSource( rData.nStartX, rData.nStartY, rData.nTabNo,
                        rData.nStartX+rData.nSizeX-1, rData.nStartY+rData.nSizeY-1, rData.nTabNo );

    if (bDragRect)
        pView->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );

    if (pSourceDoc == pThisDoc)
    {
        if ( rData.nFlags & SC_DROP_TABLE )         // Tabelle ?
        {
            if ( !pThisDoc->IsDocEditable() )
                bRet = FALSE;
            else
            {
                USHORT nSrcTab = rData.nTabNo;
                USHORT nDestTab = pViewData->GetTabNo();
                pViewData->GetDocShell()->MoveTable( nSrcTab, nDestTab, !bIsMove, TRUE );   // mit Undo
                pView->SetTabNo( nDestTab, TRUE );
            }
        }
        else                                        // Block verschieben/kopieren
        {
            Point aPos = rEvt.GetPosPixel();
            String aChartName;
            if (pThisDoc->HasChartAtPoint( pViewData->GetTabNo(), PixelToLogic(aPos), &aChartName ))
            {
                String aRangeName;
                aSource.Format( aRangeName, SCR_ABS_3D, pThisDoc );
                SfxStringItem aNameItem( SID_CHART_NAME, aChartName );
                SfxStringItem aRangeItem( SID_CHART_SOURCE, aRangeName );
                USHORT nId = (rEvt.GetAction() == DROP_COPY) ?
                            SID_CHART_ADDSOURCE : SID_CHART_SOURCE;
                pViewData->GetDispatcher().Execute( nId, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                            &aRangeItem, &aNameItem, (void*) NULL );
            }
            else if ( nDragStartX != rData.nStartX || nDragStartY != rData.nStartY ||
                        rData.nTabNo != pViewData->GetTabNo() )
            {
                ScAddress aDest( nDragStartX, nDragStartY, pViewData->GetTabNo() );
                if ( bIsLink )
                    pView->LinkBlock( aSource, aDest );
                else
                    pView->MoveBlockTo( aSource, aDest, bIsMove );
            }

            pScMod->SetDragIntern();            // bei ExecuteDrag nicht loeschen
            bRet = TRUE;
        }
    }
    else                                            // zwischen Dokumenten
    {
        if ( rData.nFlags & SC_DROP_TABLE )         // Tabelle kopieren ?
        {
            if ( !pThisDoc->IsDocEditable() )
                bRet = FALSE;
            else
            {
                USHORT nTab = rData.nTabNo;
                ScDocShell* pSrcShell = (ScDocShell*)pSourceDoc->GetDocumentShell();
//@new 22.12.97
                USHORT nTabs[MAXTAB+1];

                const ScMarkData& rMark     = rData.aMarkData;
                USHORT      nTabCount   = pSourceDoc->GetTableCount();
                USHORT      nTabSelCount = 0;

                int i;
                for(i=0;i<nTabCount;i++)
                {
                    if(rMark.GetTableSelect(i))
                    {
                        nTabs[nTabSelCount++]=i;
                        for(USHORT j=i+1;j<nTabCount;j++)
                        {
                            if((!pSourceDoc->IsVisible(j))&&(pSourceDoc->IsScenario(j)))
                            {
                                nTabs[nTabSelCount++]=j;
                                i=j;
                            }
                            else break;
                        }
                    }
                }

//@new 22.12.97
                pView->ImportTables( pSrcShell,nTabSelCount, nTabs, bIsLink,
                                        pViewData->GetTabNo() ); //@ 15.12.97
                bRet = TRUE;
            }
        }
        else
        {
            ScDocument* pDragDoc = new ScDocument( SCDOCMODE_CLIP );

            if ( bIsLink )                          // Dde-Link einfuegen
            {
                //  wie PasteDDE
                //! oder externe Referenzen einfuegen ???

                SfxObjectShell* pSourceSh = pSourceDoc->GetDocumentShell();
                DBG_ASSERT(pSourceSh, "Drag-Dokument hat keine Shell");
                if (pSourceSh)
                {
                    String aApp = Application::GetAppName();
                    String aTopic = pSourceSh->GetTitle( SFX_TITLE_FULLNAME );
                    String aItem;
                    aSource.Format( aItem, SCR_ABS_3D, pSourceDoc );

                    //! use tokens
                    String aFormula( '=' );
                    aFormula += ScCompiler::pSymbolTableNative[SC_OPCODE_DDE];
                    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM("(\""));
                    aFormula += aApp;
                    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\";\""));
                    aFormula += aTopic;
                    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\";\""));
                    aFormula += aItem;
                    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\")"));

                    pView->DoneBlockMode();
                    pView->InitBlockMode( nDragStartX, nDragStartY, pViewData->GetTabNo() );
                    pView->MarkCursor( nDragEndX, nDragEndY, pViewData->GetTabNo() );

                    pView->EnterMatrix( aFormula );
                    pView->CursorPosChanged();
                    bRet = TRUE;
                }
            }
            else                                    // verschieben
            {
                //  wie CopyToClip:
                //! HasSelectedBlockMatrixFragment ohne selektierte Tabelle
                //! oder Teil einer Matrix nicht losdraggen

                pViewData->GetMarkData().ResetMark();

                USHORT nEndX = rData.nStartX + rData.nSizeX - 1;
                USHORT nEndY = rData.nStartY + rData.nSizeY - 1;
                pSourceDoc->CopyTabToClip( rData.nStartX,rData.nStartY, nEndX,nEndY,
                                            rData.nTabNo, pDragDoc );
                USHORT nMergeX = nEndX;
                USHORT nMergeY = nEndY;
                pDragDoc->ExtendMerge( rData.nStartX, rData.nStartY, nMergeX, nMergeY, rData.nTabNo, TRUE );

                // einfuegen:

                pView->SetCursor( nDragStartX, nDragStartY );
                pView->PasteFromClip( IDF_ALL, pDragDoc );
                pViewData->GetMarkData().ResetMark();
                pView->CursorPosChanged();
                Invalidate();
                delete pDragDoc;
                bRet = TRUE;
            }
        }
    }
#endif

    bDragRect = FALSE;                  // bei naechstem IsLeave-QueryDrop nicht zeichnen
    return bRet;
}

extern BOOL bPasteIsDrop;       //! viewfun4 -> in irgendein Headerfile !!!
extern BOOL bPasteIsMove;       //! viewfun7 -> in irgendein Headerfile !!!

BOOL __EXPORT ScGridWindow::Drop( const DropEvent& rEvt )
{
    DrawMarkDropObj( NULL );

    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();
    if (rData.pCellTransfer)
        return DropPrivate( rEvt );

    if ( rData.aLinkDoc.Len() )
    {
        //  versuchen, einen Link einzufuegen

        BOOL bOk = TRUE;
        String aThisName;
        ScDocShell* pDocSh = pViewData->GetDocShell();
        if (pDocSh && pDocSh->HasName())
            aThisName = pDocSh->GetMedium()->GetName();

        if ( rData.aLinkDoc == aThisName )              // Fehler - kein Link im selben Dokument
            bOk = FALSE;
        else
        {
            ScViewFunc* pView = pViewData->GetView();
            if ( rData.aLinkTable.Len() )
                pView->InsertTableLink( rData.aLinkDoc, EMPTY_STRING, EMPTY_STRING,
                                        rData.aLinkTable );
            else if ( rData.aLinkArea.Len() )
            {
                Point   aPos = rEvt.GetPosPixel();
                short   nPosX;
                short   nPosY;
                pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
                pView->MoveCursorAbs( nPosX, nPosY, SC_FOLLOW_NONE, FALSE, FALSE );

                pView->InsertAreaLink( rData.aLinkDoc, EMPTY_STRING, EMPTY_STRING,
                                        rData.aLinkArea );
            }
            else
            {
                DBG_ERROR("Drop mit Link: weder Tabelle noch Area");
                bOk = FALSE;
            }
        }

        return bOk;             // nichts anderes mehr probieren
    }

    Point aLogicPos = PixelToLogic(rEvt.GetPosPixel());

#ifdef OLD_DND
    if (rData.pSdrModel)
    {
        bPasteIsMove = (rEvt.GetAction() == DROP_MOVE && !(rData.nFlags & SC_DROP_NAVIGATOR));
        pViewData->GetView()->PasteDraw( aLogicPos, rData.pSdrModel );
        pScMod->SetDragIntern( bPasteIsMove );
        bPasteIsMove = FALSE;
        return TRUE;
    }
#endif

    Point   aPos = rEvt.GetPosPixel();
    short   nPosX;
    short   nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

    if (rData.aJumpTarget.Len())
    {
        //  interner Bookmark (aus Navigator)
        //  Bookmark-Clipboardformat ist in PasteDataObject

        if ( !rData.pJumpLocalDoc || rData.pJumpLocalDoc == pViewData->GetDocument() )
        {
            pViewData->GetViewShell()->InsertBookmark( rData.aJumpText, rData.aJumpTarget,
                                                        nPosX, nPosY );
            return TRUE;
        }
    }

    SvDataObjectRef pObject = SvDataObject::PasteDragServer(rEvt);

    ScDocument* pThisDoc = pViewData->GetDocument();
    SdrObject* pHitObj = pThisDoc->GetObjectAtPoint( pViewData->GetTabNo(), PixelToLogic(aPos) );
    if ( pHitObj && rEvt.GetAction() == DROP_LINK )
    {
        //  auf Zeichenobjekt gezogen
        //  PasteOnDrawObject testet auf erlaubte Formate
        if ( pViewData->GetView()->PasteOnDrawObject( pObject, pHitObj, TRUE ) )
            return TRUE;
    }

    BOOL bRet = FALSE;

    bPasteIsDrop = TRUE;
    if ( rEvt.GetAction() == DROP_LINK )
        bRet = pViewData->GetView()->
                    LinkDataObject( pObject, nPosX, nPosY, this, &aLogicPos );
    else
        bRet = pViewData->GetView()->
                    PasteDataObject( pObject, nPosX, nPosY, this, &aLogicPos );
    bPasteIsDrop = FALSE;

    return bRet;
}

//--------------------------------------------------------

sal_Int8 ScGridWindow::AcceptPrivateDrop( const AcceptDropEvent& rEvt )
{
    if ( rEvt.mbLeaving )
    {
        if (bDragRect)
            pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );
        bDragRect = FALSE;
        return rEvt.mnAction;
    }

    const ScDragData& rData = SC_MOD()->GetDragData();
    if ( rData.pCellTransfer )
    {
        Point aPos = rEvt.maPosPixel;

        ScDocument* pSourceDoc = NULL;      //! rData.pDoc;
        ScDocument* pThisDoc   = pViewData->GetDocument();
        if (pSourceDoc == pThisDoc)
        {
            if ( pThisDoc->HasChartAtPoint(pViewData->GetTabNo(), PixelToLogic(aPos)) )
            {
                if (bDragRect)          // Rechteck loeschen
                {
                    pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );
                    bDragRect = FALSE;
                }

                //! highlight chart? (selection border?)

                sal_Int8 nRet = rEvt.mnAction;
//!             if ( rEvt.GetAction() == DROP_LINK )
//!                 bOk = rEvt.SetAction( DROP_COPY );          // can't link onto chart
                return nRet;
            }
        }
//!     else
//!         if ( rEvt.GetAction() == DROP_MOVE )
//!             rEvt.SetAction( DROP_COPY );                    // different doc: default=COPY


//!     if ( rData.nFlags & SC_DROP_TABLE )                     // whole sheet?
//!     {
//!         BOOL bOk = pThisDoc->IsDocEditable();
//!         return bOk ? rEvt.mnAction : 0;                     // don't draw selection frame
//!     }

        short   nPosX;
        short   nPosY;
        pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

        ScRange aSourceRange = rData.pCellTransfer->GetRange();
        USHORT nSizeX = aSourceRange.aEnd.Col() - aSourceRange.aStart.Col() + 1;
        USHORT nSizeY = aSourceRange.aEnd.Row() - aSourceRange.aStart.Row() + 1;

        short nNewDragX = nPosX - rData.pCellTransfer->GetDragHandleX();
        if (nNewDragX<0) nNewDragX=0;
        if (nNewDragX+(nSizeX-1) > MAXCOL)
            nNewDragX = MAXCOL-(nSizeX-1);
        short nNewDragY = nPosY - rData.pCellTransfer->GetDragHandleY();
        if (nNewDragY<0) nNewDragY=0;
        if (nNewDragY+(nSizeY-1) > MAXROW)
            nNewDragY = MAXROW-(nSizeY-1);

        if ( nNewDragX != (short) nDragStartX || nNewDragY != (short) nDragStartY || !bDragRect )
        {
            if (bDragRect)
                pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );

            nDragStartX = nNewDragX;
            nDragStartY = nNewDragY;
            nDragEndX = nDragStartX+nSizeX-1;
            nDragEndY = nDragStartY+nSizeY-1;
            bDragRect = TRUE;

            pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );

            //  Zielposition als Tip-Hilfe anzeigen
#if 0
            if (Help::IsQuickHelpEnabled())
            {
                USHORT nTab = pViewData->GetTabNo();
                ScRange aRange( nDragStartX, nDragStartY, nTab, nDragEndX, nDragEndY, nTab );
                String aHelpStr;
                aRange.Format( aHelpStr, SCA_VALID );   // nicht-3D

                Point aPos = Pointer::GetPosPixel();
                USHORT nAlign = QUICKHELP_BOTTOM|QUICKHELP_RIGHT;
                Rectangle aRect( aPos, aPos );
                Help::ShowQuickHelp(aRect, aHelpStr, nAlign);
            }
#endif
        }
    }

    return rEvt.mnAction;
}

sal_Int8 ScGridWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    const ScDragData& rData = SC_MOD()->GetDragData();
    if ( rEvt.mbLeaving )
    {
        DrawMarkDropObj( NULL );
        if ( rData.pCellTransfer )
            return AcceptPrivateDrop( rEvt );   // hide drop marker for internal D&D
        else
            return rEvt.mnAction;
    }

    if ( pViewData->GetDocShell()->IsReadOnly() )
        return DND_ACTION_NONE;

    sal_Int8 nRet = DND_ACTION_NONE;

    if (rData.pCellTransfer)
    {
#ifdef OLD_DND
        if ( rData.nSizeX<=MAXCOL && rData.nSizeY<=MAXROW && !rEvt.IsLeaveWindow() )
            DropScroll( rEvt.GetPosPixel() );
#endif
        nRet = AcceptPrivateDrop( rEvt );
    }
    else
    {
        switch ( rEvt.mnAction )
        {
            case DND_ACTION_COPY:
            case DND_ACTION_MOVE:
            case DND_ACTION_COPYMOVE:
                {
                    BOOL bMove = ( rEvt.mnAction == DND_ACTION_MOVE );
                    if ( IsDropFormatSupported( SOT_FORMATSTR_ID_EMBED_SOURCE ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_LINK_SOURCE ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ) ||
                         IsDropFormatSupported( SOT_FORMAT_STRING ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_SYLK ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_LINK ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_HTML ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_HTML_SIMPLE ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_DIF ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_DRAWING ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_SVXB ) ||
                         IsDropFormatSupported( SOT_FORMAT_RTF ) ||
                         IsDropFormatSupported( SOT_FORMAT_GDIMETAFILE ) ||
                         IsDropFormatSupported( SOT_FORMAT_BITMAP ) ||
                         IsDropFormatSupported( SOT_FORMATSTR_ID_SBA_DATAEXCHANGE ) ||
                         ( !bMove && IsDropFormatSupported( SOT_FORMAT_FILE ) ) )
                    {
                        nRet = rEvt.mnAction;
                    }
                }
                //! allow SOT_FORMAT_FILE and INetBookmark formats if not moving
                break;
        }
    }

    return nRet;
}

ULONG lcl_GetDropFormatId( const uno::Reference<datatransfer::XTransferable>& xTransfer )
{
    ULONG nFormatId = 0;
    TransferableDataHelper aDataHelper( xTransfer );

    //! if INetBookmark but not SOT_FORMATSTR_ID_SBA_DATAEXCHANGE, use INetBookmark

    if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ) )
        nFormatId = SOT_FORMATSTR_ID_DRAWING;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ) )
        nFormatId = SOT_FORMATSTR_ID_SVXB;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE ) )
    {
        //! if this is a Writer object, use RTF
        nFormatId = SOT_FORMATSTR_ID_EMBED_SOURCE;
    }
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE ) )
        nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SBA_DATAEXCHANGE ) )
        nFormatId = SOT_FORMATSTR_ID_SBA_DATAEXCHANGE;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE ) )
        nFormatId = SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_BIFF_5 ) )
        nFormatId = SOT_FORMATSTR_ID_BIFF_5;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ) )
        nFormatId = SOT_FORMATSTR_ID_EMBED_SOURCE_OLE;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ) )
        nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE_OLE;
    else if ( aDataHelper.HasFormat( SOT_FORMAT_RTF ) )
        nFormatId = SOT_FORMAT_RTF;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_HTML ) )
        nFormatId = SOT_FORMATSTR_ID_HTML;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_HTML_SIMPLE ) )
        nFormatId = SOT_FORMATSTR_ID_HTML_SIMPLE;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SYLK ) )
        nFormatId = SOT_FORMATSTR_ID_SYLK;
    else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK ) )
        nFormatId = SOT_FORMATSTR_ID_LINK;
    else if ( aDataHelper.HasFormat( SOT_FORMAT_STRING ) )
        nFormatId = SOT_FORMAT_STRING;
    else if ( aDataHelper.HasFormat( SOT_FORMAT_GDIMETAFILE ) )
        nFormatId = SOT_FORMAT_GDIMETAFILE;
    else if ( aDataHelper.HasFormat( SOT_FORMAT_BITMAP ) )
        nFormatId = SOT_FORMAT_BITMAP;
    else if ( aDataHelper.HasFormat( SOT_FORMAT_FILE ) )
        nFormatId = SOT_FORMAT_FILE;

    return nFormatId;
}

sal_Int8 ScGridWindow::ExecutePrivateDrop( const ExecuteDropEvent& rEvt )
{
    // hide drop marker
    if (bDragRect)
        pViewData->GetView()->DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, eWhich );
    bDragRect = FALSE;

    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();
    ScTransferObj* pTransObj = rData.pCellTransfer;
    if ( !pTransObj )
        return 0;

//! ScDocument* pSourceDoc = ...
    ScDocument* pSourceDoc = pViewData->GetDocument();      //! Test !!!
    ScDocument* pThisDoc   = pViewData->GetDocument();
    ScViewFunc* pView      = pViewData->GetView();
    USHORT nFlags = 0;  //! rData.nFlags

    BOOL bIsMove = ( rEvt.mnAction == DND_ACTION_MOVE );
    BOOL bIsLink = ( rEvt.mnAction == DND_ACTION_LINK );

    ScRange aSource = pTransObj->GetRange();
    BOOL bDone = FALSE;

    if (pSourceDoc == pThisDoc)
    {
        if ( nFlags & SC_DROP_TABLE )           // whole sheet?
        {
            if ( pThisDoc->IsDocEditable() )
            {
                USHORT nSrcTab = aSource.aStart.Tab();
                USHORT nDestTab = pViewData->GetTabNo();
                pViewData->GetDocShell()->MoveTable( nSrcTab, nDestTab, !bIsMove, TRUE );   // with Undo
                pView->SetTabNo( nDestTab, TRUE );
                bDone = TRUE;
            }
        }
        else                                        // move/copy block
        {
            Point aPos = rEvt.maPosPixel;
            String aChartName;
            if (pThisDoc->HasChartAtPoint( pViewData->GetTabNo(), PixelToLogic(aPos), &aChartName ))
            {
                String aRangeName;
                aSource.Format( aRangeName, SCR_ABS_3D, pThisDoc );
                SfxStringItem aNameItem( SID_CHART_NAME, aChartName );
                SfxStringItem aRangeItem( SID_CHART_SOURCE, aRangeName );
                USHORT nId = bIsMove ? SID_CHART_SOURCE : SID_CHART_ADDSOURCE;
                pViewData->GetDispatcher().Execute( nId, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                            &aRangeItem, &aNameItem, (void*) NULL );
            }
            else if ( nDragStartX != aSource.aStart.Col() || nDragStartY != aSource.aStart.Row() ||
                        aSource.aStart.Tab() != pViewData->GetTabNo() )
            {
                ScAddress aDest( nDragStartX, nDragStartY, pViewData->GetTabNo() );
                if ( bIsLink )
                    pView->LinkBlock( aSource, aDest );
                else
                    pView->MoveBlockTo( aSource, aDest, bIsMove );
            }

            pScMod->SetDragIntern();            // don't delete source in ExecuteDrag
            bDone = TRUE;
        }
    }

    sal_Int8 nRet = bDone ? rEvt.mnAction : DND_ACTION_NONE;
    return nRet;
}

sal_Int8 ScGridWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    DrawMarkDropObj( NULL );    // drawing layer

    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();
    if (rData.pCellTransfer)
        return ExecutePrivateDrop( rEvt );

    Point   aPos = rEvt.maPosPixel;
    short   nPosX;
    short   nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    Point aLogicPos = PixelToLogic( aPos );

    BOOL bDone = FALSE;
    ULONG nFormatId = lcl_GetDropFormatId( rEvt.maDropEvent.Transferable );
    if ( nFormatId )
        bDone = pViewData->GetView()->PasteDataFormat(
                    nFormatId, rEvt.maDropEvent.Transferable, nPosX, nPosY, &aLogicPos );

    sal_Int8 nRet = bDone ? rEvt.mnAction : DND_ACTION_NONE;
    return nRet;
}

//--------------------------------------------------------

void ScGridWindow::UpdateEditViewPos()
{
    if (pViewData->HasEditView(eWhich))
    {
        EditView* pView;
        USHORT nCol;
        USHORT nRow;
        pViewData->GetEditView( eWhich, pView, nCol, nRow );

        //  EditView verstecken?

        BOOL bHide = ( nCol<pViewData->GetPosX(eHWhich) || nRow<pViewData->GetPosY(eVWhich) );
        if ( SC_MOD()->IsFormulaMode() )
            if ( pViewData->GetTabNo() != pViewData->GetRefTabNo() )
                bHide = TRUE;

        if (bHide)
        {
            Rectangle aRect = pView->GetOutputArea();
            long nHeight = aRect.Bottom() - aRect.Top();
            aRect.Top() = PixelToLogic(GetOutputSizePixel(), pViewData->GetLogicMode()).
                            Height() * 2;
            aRect.Bottom() = aRect.Top() + nHeight;
            pView->SetOutputArea( aRect );
            pView->HideCursor();
        }
        else
        {
            // bForceToTop = TRUE for editing
            Rectangle aPixRect = pViewData->GetEditArea( eWhich, nCol, nRow, this, NULL, TRUE );
            Point aScrPos = PixelToLogic( aPixRect.TopLeft(), pViewData->GetLogicMode() );

            Rectangle aRect = pView->GetOutputArea();
            aRect.SetPos( aScrPos );
            pView->SetOutputArea( aRect );
            pView->ShowCursor();
        }
    }
}

void ScGridWindow::ScrollPixel( long nDifX, long nDifY )
{
    ClickExtern();

    bIsInScroll = TRUE;
    BOOL bXor=DrawBeforeScroll();

    SetMapMode(MAP_PIXEL);
    Scroll( nDifX, nDifY, SCROLL_CHILDREN );
    SetMapMode( GetDrawMapMode() );             // verschobenen MapMode erzeugen

    UpdateEditViewPos();

    DrawAfterScroll(bXor);
    bIsInScroll = FALSE;
}

//  Formeln neu zeichnen -------------------------------------------------

void ScGridWindow::UpdateFormulas()
{
    if (pViewData->GetView()->IsMinimized())
        return;

    if ( nPaintCount )
    {
        //  nicht anfangen, verschachtelt zu painten
        //  (dann wuerde zumindest der MapMode nicht mehr stimmen)

        bNeedsRepaint = TRUE;           // -> am Ende vom Paint nochmal Invalidate auf alles
        aRepaintPixel = Rectangle();    // alles
        return;
    }

    USHORT  nX1 = pViewData->GetPosX( eHWhich );
    USHORT  nY1 = pViewData->GetPosY( eVWhich );
    USHORT  nX2 = nX1 + pViewData->VisibleCellsX( eHWhich );
    USHORT  nY2 = nY1 + pViewData->VisibleCellsY( eVWhich );

    if (nX2 > MAXCOL) nX2 = MAXCOL;
    if (nY2 > MAXROW) nY2 = MAXROW;

    Draw( nX1, nY1, nX2, nY2, SC_UPDATE_CHANGED );
}

void ScGridWindow::UpdateAutoFillMark(BOOL bMarked, const ScRange& rMarkRange)
{
    if ( bMarked != bAutoMarkVisible || ( bMarked && rMarkRange.aEnd != aAutoMarkPos ) )
    {
        HideCursor();
        bAutoMarkVisible = bMarked;
        if ( bMarked )
            aAutoMarkPos = rMarkRange.aEnd;
        ShowCursor();
    }
}

void ScGridWindow::HideCursor()
{
    ++nCursorHideCount;
    if (nCursorHideCount==1)
    {
        BOOL bDrawCursor = TRUE;
        BOOL bDrawAuto   = TRUE;

        //! irgendwie erkennen, ob wirklich neu gepainted werden muss
        //! immer flackert zu viel, weil ein Control sich immer komplett zeichnet

        if (!bIsInPaint)
        {
            //  Controls unter dem Cursor ?

            ScMarkData& rMark = pViewData->GetMarkData();
            BOOL bMarked = ( rMark.IsMarked() || rMark.IsMultiMarked() );

            ScDocument* pDoc = pViewData->GetDocument();
            USHORT nTab = pViewData->GetTabNo();
            USHORT nX = pViewData->GetCurX();
            USHORT nY = pViewData->GetCurY();

            Point aScrPos = pViewData->GetScrPos( nX, nY, eWhich );
            long nSizeXPix;
            long nSizeYPix;
            pViewData->GetMergeSizePixel( nX, nY, nSizeXPix, nSizeYPix );
            aScrPos.X() -= 2;
            aScrPos.Y() -= 2;
            Rectangle aPixRect( aScrPos, Size( nSizeXPix+4,nSizeYPix+4 ) );

            if ( bAutoMarkVisible && aAutoMarkPos == ScAddress(nX,nY,nTab) )
            {
                aPixRect.Right() += 2;      // Anfasser mit einschliessen
                aPixRect.Bottom() += 2;
            }

            MapMode aDrawMode = GetDrawMapMode();
            Rectangle aLogicRect = PixelToLogic( aPixRect, aDrawMode );

            if (pDoc->HasControl( nTab, aLogicRect ))
            {
                Invalidate( PixelToLogic( aPixRect ) );
                if (bMarked)
                    pDoc->InvalidateControls( this, nTab, aLogicRect );

                Update();
                bDrawCursor = FALSE;        // nicht per XOR malen
            }

            //  Controls unter dem AutoFill-Anfasser ?

            if ( bAutoMarkVisible && aAutoMarkPos.Tab() == nTab )
            {
                if ( aAutoMarkPos == ScAddress(nX,nY,nTab) )
                    bDrawAuto = bDrawCursor;                        // schon erledigt
                else
                {
                    USHORT nAutoX = aAutoMarkPos.Col();
                    USHORT nAutoY = aAutoMarkPos.Row();
                    Point aFillPos = pViewData->GetScrPos( nAutoX, nAutoY, eWhich, TRUE );
                    long nSizeXPix;
                    long nSizeYPix;
                    pViewData->GetMergeSizePixel( nAutoX, nAutoY, nSizeXPix, nSizeYPix );
                    aFillPos.X() += nSizeXPix;
                    aFillPos.Y() += nSizeYPix;

                    aFillPos.X() -= 2;
                    aFillPos.Y() -= 2;
                    Rectangle aFillRect( aFillPos, Size(6,6) );

                    Rectangle aLogicFill = PixelToLogic( aFillRect, aDrawMode );
                    if (pDoc->HasControl( nTab, aLogicFill ))
                    {
                        Invalidate( PixelToLogic( aFillRect ) );
                        if (bMarked)
                            pDoc->InvalidateControls( this, nTab, aLogicFill );

                        Update();
                        bDrawAuto = FALSE;
                    }
                }
            }
        }

        if (bDrawCursor)
            DrawCursor();
        if (bDrawAuto)
            DrawAutoFillMark();
    }
}

void ScGridWindow::ShowCursor()
{
    if (nCursorHideCount==0)
    {
        DBG_ERROR("zuviel ShowCursor");
        return;
    }

    --nCursorHideCount;
    if (nCursorHideCount==0)
    {
        DrawAutoFillMark();
        DrawCursor();
    }
}

void __EXPORT ScGridWindow::GetFocus()
{
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    pViewShell->GotFocus();

    if ( !SC_MOD()->IsFormulaMode() )
    {
        pViewShell->UpdateInputHandler();
//      StopMarking();      // falls Dialog (Fehler), weil dann kein ButtonUp
                            // MO: nur wenn nicht im RefInput-Modus
                            //     -> GetFocus/MouseButtonDown-Reihenfolge
                            //        auf dem Mac
    }

    Window::GetFocus();
}

void __EXPORT ScGridWindow::LoseFocus()
{
    pViewData->GetViewShell()->LostFocus();

    Window::LoseFocus();
}

Point ScGridWindow::GetMousePosPixel() const  { return aCurMousePos; }

//------------------------------------------------------------------------

BOOL ScGridWindow::HitRangeFinder( const Point& rMouse, BOOL& rCorner,
                                USHORT* pIndex, short* pAddX, short* pAddY )
{
    BOOL bFound = FALSE;
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( pViewData->GetViewShell() );
    if (pHdl)
    {
        ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
        if ( pRangeFinder && !pRangeFinder->IsHidden() &&
                pRangeFinder->GetDocName() == pViewData->GetDocShell()->GetTitle() )
        {
            short nPosX, nPosY;
            pViewData->GetPosFromPixel( rMouse.X(), rMouse.Y(), eWhich, nPosX, nPosY );
            //  zusammengefasste (einzeln/Bereich) ???
            ScAddress aAddr( nPosX, nPosY, pViewData->GetTabNo() );

//          Point aNext = pViewData->GetScrPos( nPosX+1, nPosY+1, eWhich );

            Point aNext = pViewData->GetScrPos( nPosX, nPosY, eWhich, TRUE );
            long nSizeXPix;
            long nSizeYPix;
            pViewData->GetMergeSizePixel( nPosX, nPosY, nSizeXPix, nSizeYPix );
            aNext.X() += nSizeXPix;
            aNext.Y() += nSizeYPix;

            BOOL bCellCorner = ( rMouse.X() >= aNext.X() - 8 && rMouse.X() <= aNext.X() &&
                                 rMouse.Y() >= aNext.Y() - 8 && rMouse.Y() <= aNext.Y() );
            //  Corner wird nur erkannt, wenn noch innerhalb der Zelle

            USHORT nCount = (USHORT)pRangeFinder->Count();
            for (USHORT i=nCount; i;)
            {
                //  rueckwaerts suchen, damit der zuletzt gepaintete Rahmen gefunden wird
                --i;
                ScRangeFindData* pData = pRangeFinder->GetObject(i);
                if ( pData && pData->aRef.In(aAddr) )
                {
                    if (pIndex) *pIndex = i;
                    if (pAddX)  *pAddX = nPosX - pData->aRef.aStart.Col();
                    if (pAddY)  *pAddY = nPosY - pData->aRef.aStart.Row();
                    bFound = TRUE;
                    rCorner = ( bCellCorner && aAddr == pData->aRef.aEnd );
                    break;
                }
            }
        }
    }
    return bFound;
}

#define SCE_TOP     1
#define SCE_BOTTOM  2
#define SCE_LEFT    4
#define SCE_RIGHT   8
#define SCE_ALL     15

void lcl_PaintOneRange( ScDocShell* pDocSh, const ScRange& rRange, USHORT nEdges )
{
    //  der Range ist immer richtigherum

    USHORT nCol1 = rRange.aStart.Col();
    USHORT nRow1 = rRange.aStart.Row();
    USHORT nTab1 = rRange.aStart.Tab();
    USHORT nCol2 = rRange.aEnd.Col();
    USHORT nRow2 = rRange.aEnd.Row();
    USHORT nTab2 = rRange.aEnd.Tab();

    if ( nCol2 > nCol1 + 1 && nRow2 > nRow1 + 1 )
    {
        //  nur an den Raendern entlang
        //  (die Ecken werden evtl. zweimal getroffen)

        if ( nEdges & SCE_TOP )
            pDocSh->PostPaint( nCol1, nRow1, nTab1, nCol2, nRow1, nTab2, PAINT_MARKS );
        if ( nEdges & SCE_LEFT )
            pDocSh->PostPaint( nCol1, nRow1, nTab1, nCol1, nRow2, nTab2, PAINT_MARKS );
        if ( nEdges & SCE_RIGHT )
            pDocSh->PostPaint( nCol2, nRow1, nTab1, nCol2, nRow2, nTab2, PAINT_MARKS );
        if ( nEdges & SCE_BOTTOM )
            pDocSh->PostPaint( nCol1, nRow2, nTab1, nCol2, nRow2, nTab2, PAINT_MARKS );
    }
    else    // alles am Stueck
        pDocSh->PostPaint( rRange, PAINT_MARKS );
}

void lcl_PaintRefChanged( ScDocShell* pDocSh, const ScRange& rOldUn, const ScRange& rNewUn )
{
    //  Repaint fuer die Teile des Rahmens in Old, die bei New nicht mehr da sind

    ScRange aOld = rOldUn;
    ScRange aNew = rNewUn;
    aOld.Justify();
    aNew.Justify();

    if ( aOld.aStart == aOld.aEnd )                 //! Tab ignorieren?
        pDocSh->GetDocument()->ExtendMerge(aOld);
    if ( aNew.aStart == aNew.aEnd )                 //! Tab ignorieren?
        pDocSh->GetDocument()->ExtendMerge(aNew);

    USHORT nOldCol1 = aOld.aStart.Col();
    USHORT nOldRow1 = aOld.aStart.Row();
    USHORT nOldCol2 = aOld.aEnd.Col();
    USHORT nOldRow2 = aOld.aEnd.Row();
    USHORT nNewCol1 = aNew.aStart.Col();
    USHORT nNewRow1 = aNew.aStart.Row();
    USHORT nNewCol2 = aNew.aEnd.Col();
    USHORT nNewRow2 = aNew.aEnd.Row();
    USHORT nTab1 = aOld.aStart.Tab();       // Tab aendert sich nicht
    USHORT nTab2 = aOld.aEnd.Tab();

    if ( nNewRow2 < nOldRow1 || nNewRow1 > nOldRow2 ||
         nNewCol2 < nOldCol1 || nNewCol1 > nOldCol2 ||
         ( nNewCol1 != nOldCol1 && nNewRow1 != nOldRow1 &&
           nNewCol2 != nOldCol2 && nNewRow2 != nOldRow2 ) )
    {
        //  komplett weggeschoben oder alle Seiten veraendert
        //  (Abfrage <= statt < geht schief bei einzelnen Zeilen/Spalten)

        lcl_PaintOneRange( pDocSh, aOld, SCE_ALL );
    }
    else        //  alle vier Kanten einzeln testen
    {
        //  oberer Teil
        if ( nNewRow1 < nOldRow1 )                  //  nur obere Linie loeschen
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow1, nTab1, nOldCol2, nOldRow1, nTab2 ), SCE_ALL );
        else if ( nNewRow1 > nOldRow1 )             //  den Teil, der oben wegkommt
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow1, nTab1, nOldCol2, nNewRow1-1, nTab2 ),
                    SCE_ALL &~ SCE_BOTTOM );

        //  unterer Teil
        if ( nNewRow2 > nOldRow2 )                  //  nur untere Linie loeschen
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow2, nTab1, nOldCol2, nOldRow2, nTab2 ), SCE_ALL );
        else if ( nNewRow2 < nOldRow2 )             //  den Teil, der unten wegkommt
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nNewRow2+1, nTab1, nOldCol2, nOldRow2, nTab2 ),
                    SCE_ALL &~ SCE_TOP );

        //  linker Teil
        if ( nNewCol1 < nOldCol1 )                  //  nur linke Linie loeschen
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow1, nTab1, nOldCol1, nOldRow2, nTab2 ), SCE_ALL );
        else if ( nNewCol1 > nOldCol1 )             //  den Teil, der links wegkommt
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow1, nTab1, nNewCol1-1, nOldRow2, nTab2 ),
                    SCE_ALL &~ SCE_RIGHT );

        //  rechter Teil
        if ( nNewCol2 > nOldCol2 )                  //  nur rechte Linie loeschen
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol2, nOldRow1, nTab1, nOldCol2, nOldRow2, nTab2 ), SCE_ALL );
        else if ( nNewCol2 < nOldCol2 )             //  den Teil, der rechts wegkommt
            lcl_PaintOneRange( pDocSh, ScRange(
                    nNewCol2+1, nOldRow1, nTab1, nOldCol2, nOldRow2, nTab2 ),
                    SCE_ALL &~ SCE_LEFT );
    }
}

void ScGridWindow::RFMouseMove( const MouseEvent& rMEvt, BOOL bUp )
{
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( pViewData->GetViewShell() );
    if (!pHdl)
        return;
    ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
    if (!pRangeFinder || nRFIndex >= pRangeFinder->Count())
        return;
    ScRangeFindData* pData = pRangeFinder->GetObject( nRFIndex );
    if (!pData)
        return;

    //  Mauszeiger

    if (bRFSize)
        SetPointer( Pointer( POINTER_CROSS ) );
    else
        SetPointer( Pointer( POINTER_HAND ) );

    //  Scrolling

    BOOL bTimer = FALSE;
    Point aPos = rMEvt.GetPosPixel();
    short nDx = 0;
    short nDy = 0;
    if ( aPos.X() < 0 ) nDx = -1;
    if ( aPos.Y() < 0 ) nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPos.X() >= aSize.Width() )
        nDx = 1;
    if ( aPos.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx || nDy )
    {
        if ( nDx ) pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy ) pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );
        bTimer = TRUE;
    }

    //  Umschalten bei Fixierung (damit Scrolling funktioniert)

    if ( eWhich == pViewData->GetActivePart() )     //??
    {
        if ( pViewData->GetHSplitMode() == SC_SPLIT_FIX )
            if ( nDx > 0 )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_TOPRIGHT );
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
            }

        if ( pViewData->GetVSplitMode() == SC_SPLIT_FIX )
            if ( nDy > 0 )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT );
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
            }
    }

    //  Verschieben

    short   nPosX;
    short   nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

    ScRange aOld = pData->aRef;
    ScRange aNew = aOld;
    if ( bRFSize )
    {
        aNew.aEnd.SetCol((USHORT)nPosX);
        aNew.aEnd.SetRow((USHORT)nPosY);
    }
    else
    {
        long nStartX = nPosX - nRFAddX;
        if ( nStartX < 0 ) nStartX = 0;
        long nStartY = nPosY - nRFAddY;
        if ( nStartY < 0 ) nStartY = 0;
        long nEndX = nStartX + aOld.aEnd.Col() - aOld.aStart.Col();
        if ( nEndX > MAXCOL )
        {
            nStartX -= ( nEndX - MAXROW );
            nEndX = MAXCOL;
        }
        long nEndY = nStartY + aOld.aEnd.Row() - aOld.aStart.Row();
        if ( nEndY > MAXROW )
        {
            nStartY -= ( nEndY - MAXROW );
            nEndY = MAXROW;
        }

        aNew.aStart.SetCol((USHORT)nStartX);
        aNew.aStart.SetRow((USHORT)nStartY);
        aNew.aEnd.SetCol((USHORT)nEndX);
        aNew.aEnd.SetRow((USHORT)nEndY);
    }

    if ( bUp )
        aNew.Justify();             // beim ButtonUp wieder richtigherum

    if ( aNew != aOld )
    {
        pHdl->UpdateRange( nRFIndex, aNew );

        ScDocShell* pDocSh = pViewData->GetDocShell();

        //  nur das neuzeichnen, was sich veraendert hat...
        lcl_PaintRefChanged( pDocSh, aOld, aNew );

        //  neuen Rahmen nur drueberzeichnen (synchron)
        pDocSh->Broadcast( ScIndexHint( SC_HINT_SHOWRANGEFINDER, nRFIndex ) );

        Update();   // was man bewegt, will man auch sofort sehen
    }

    //  Timer fuer Scrolling

    if (bTimer)
        pViewData->GetView()->SetTimer( this, rMEvt );          // Event wiederholen
    else
        pViewData->GetView()->ResetTimer();
}

//------------------------------------------------------------------------

BOOL ScGridWindow::GetEditUrl( const Point& rPos,
                                String* pName, String* pUrl, String* pTarget )
{
    return GetEditUrlOrError( FALSE, rPos, pName, pUrl, pTarget );
}

BOOL ScGridWindow::GetEditUrlOrError( BOOL bSpellErr, const Point& rPos,
                                String* pName, String* pUrl, String* pTarget )
{
    //! nPosX/Y mit uebergeben?
    short nPosX;
    short nPosY;
    pViewData->GetPosFromPixel( rPos.X(), rPos.Y(), eWhich, nPosX, nPosY );

    USHORT nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    ScBaseCell* pCell = NULL;

    BOOL bFound = FALSE;
    do
    {
        pDoc->GetCell( nPosX, nPosY, nTab, pCell );
        if ( !pCell || pCell->GetCellType() == CELLTYPE_NOTE )
        {
            if ( nPosX <= 0 )
                return FALSE;                           // alles leer bis links
            else
                --nPosX;                                // weitersuchen
        }
        else if ( pCell->GetCellType() == CELLTYPE_EDIT )
            bFound = TRUE;
        else
            return FALSE;                               // andere Zelle
    }
    while ( !bFound );

    ScHideTextCursor aHideCursor( pViewData, eWhich );  // before GetEditArea (MapMode is changed)

    const ScPatternAttr* pPattern = pDoc->GetPattern( nPosX, nPosY, nTab );
    // bForceToTop = FALSE, use the cell's real position
    Rectangle aEditRect = pViewData->GetEditArea( eWhich, nPosX, nPosY, this, pPattern, FALSE );
    if (rPos.Y() < aEditRect.Top())
        return FALSE;

        //  vertikal kann (noch) nicht angeklickt werden:

    SvxCellOrientation eOrient = (SvxCellOrientation) ((SvxOrientationItem&)pPattern->
                                            GetItem(ATTR_ORIENTATION)).GetValue();
    if (eOrient != SVX_ORIENTATION_STANDARD)
        return FALSE;

    BOOL bBreak = ((SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue() ||
                    ((SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                        GetItem( ATTR_HOR_JUSTIFY )).GetValue() == SVX_HOR_JUSTIFY_BLOCK);
    SvxCellHorJustify eHorJust = (SvxCellHorJustify)((SvxHorJustifyItem&)pPattern->
                        GetItem(ATTR_HOR_JUSTIFY)).GetValue();

        //  EditEngine

    ScFieldEditEngine aEngine( pDoc->GetEditPool() );
    aEngine.SetRefMapMode( MAP_100TH_MM );
    SfxItemSet aDefault( aEngine.GetEmptyItemSet() );
    pPattern->FillEditItemSet( &aDefault );
    SvxAdjust eSvxAdjust = SVX_ADJUST_LEFT;
    switch (eHorJust)
    {
        case SVX_HOR_JUSTIFY_LEFT:
        case SVX_HOR_JUSTIFY_REPEAT:            // nicht implementiert
        case SVX_HOR_JUSTIFY_STANDARD:          // immer Text
                eSvxAdjust = SVX_ADJUST_LEFT;
                break;
        case SVX_HOR_JUSTIFY_RIGHT:
                eSvxAdjust = SVX_ADJUST_RIGHT;
                break;
        case SVX_HOR_JUSTIFY_CENTER:
                eSvxAdjust = SVX_ADJUST_CENTER;
                break;
        case SVX_HOR_JUSTIFY_BLOCK:
                eSvxAdjust = SVX_ADJUST_BLOCK;
                break;
    }
    aDefault.Put( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );
    aEngine.SetDefaults( aDefault );
    if (bSpellErr)
        aEngine.SetControlWord( aEngine.GetControlWord() | EE_CNTRL_ONLINESPELLING );

    MapMode aEditMode = pViewData->GetLogicMode(eWhich);            // ohne Drawing-Skalierung
    Rectangle aLogicEdit = PixelToLogic( aEditRect, aEditMode );
    long nThisColLogic = aLogicEdit.Right() - aLogicEdit.Left() + 1;

    Size aPaperSize = Size( 1000000, 1000000 );
    if (bBreak)
        aPaperSize.Width() = nThisColLogic;
    aEngine.SetPaperSize( aPaperSize );

    const EditTextObject* pData;
    ((ScEditCell*)pCell)->GetData(pData);
    if (pData)
        aEngine.SetText(*pData);

    long nStartX = aLogicEdit.Left();

    long nTextWidth = aEngine.CalcTextWidth();
    long nTextHeight = aEngine.GetTextHeight();
    if ( nTextWidth < nThisColLogic )
    {
        if (eHorJust == SVX_HOR_JUSTIFY_RIGHT)
            nStartX += nThisColLogic - nTextWidth;
        else if (eHorJust == SVX_HOR_JUSTIFY_CENTER)
            nStartX += (nThisColLogic - nTextWidth) / 2;
    }

    aLogicEdit.Left() = nStartX;
    if (!bBreak)
        aLogicEdit.Right() = nStartX + nTextWidth;
    aLogicEdit.Bottom() = aLogicEdit.Top() + nTextHeight;

    Point aLogicClick = PixelToLogic(rPos,aEditMode);
    if ( aLogicEdit.IsInside(aLogicClick) )
    {
//      aEngine.SetUpdateMode(FALSE);
        EditView aTempView( &aEngine, this );
        aTempView.SetOutputArea( aLogicEdit );

        BOOL bRet = FALSE;
        MapMode aOld = GetMapMode();
        SetMapMode(aEditMode);                  // kein return mehr

        if (bSpellErr)                          // Spelling-Fehler suchen
        {
            bRet = aTempView.IsWrongSpelledWordAtPos( rPos );
            if ( bRet )
                pViewData->GetView()->SetCursor( nPosX, nPosY );        // Cursor setzen
        }
        else                                    // URL suchen
        {
            const SvxFieldItem* pFieldItem = aTempView.GetFieldUnderMousePointer();

            if (pFieldItem)
            {
                const SvxFieldData* pField = pFieldItem->GetField();
                if ( pField && pField->ISA(SvxURLField) )
                {
                    if ( pName || pUrl || pTarget )
                    {
                        const SvxURLField* pURLField = (const SvxURLField*)pField;
                        if (pName)
                            *pName = pURLField->GetRepresentation();
                        if (pUrl)
                            *pUrl = pURLField->GetURL();
                        if (pTarget)
                            *pTarget = pURLField->GetTargetFrame();
                    }
                    bRet = TRUE;
                }
            }
        }

        SetMapMode(aOld);

        //  text cursor is restored in ScHideTextCursor dtor

        return bRet;
    }
    return FALSE;
}

BOOL ScGridWindow::HasScenarioButton( const Point& rPosPixel, ScRange& rScenRange )
{
    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();
    USHORT nTabCount = pDoc->GetTableCount();
    if ( nTab+1<nTabCount && pDoc->IsScenario(nTab+1) && !pDoc->IsScenario(nTab) )
    {
        Size aButSize = pViewData->GetScenButSize();
        long nBWidth  = aButSize.Width();
        if (!nBWidth)
            return FALSE;                   // noch kein Button gezeichnet -> da ist auch keiner
        long nBHeight = aButSize.Height();
        long nHSpace  = (long)( SC_SCENARIO_HSPACE * pViewData->GetPPTX() );

        //! Ranges an der Table cachen!!!!

        USHORT i;
        ScMarkData aMarks;
        for (i=nTab+1; i<nTabCount && pDoc->IsScenario(i); i++)
            pDoc->MarkScenario( i, nTab, aMarks, FALSE, SC_SCENARIO_SHOWFRAME );
        ScRangeList aRanges;
        aMarks.FillRangeListWithMarks( &aRanges, FALSE );


        USHORT nRangeCount = (USHORT)aRanges.Count();
        for (i=0; i<nRangeCount; i++)
        {
            ScRange aRange = *aRanges.GetObject(i);
            //  Szenario-Rahmen immer dann auf zusammengefasste Zellen erweitern, wenn
            //  dadurch keine neuen nicht-ueberdeckten Zellen mit umrandet werden
            pDoc->ExtendTotalMerge( aRange );

            BOOL bTextBelow = ( aRange.aStart.Row() == 0 );

            Point aButtonPos;
            if ( bTextBelow )
            {
                aButtonPos = pViewData->GetScrPos( aRange.aEnd.Col()+1, aRange.aEnd.Row()+1,
                                                    eWhich, TRUE );
            }
            else
            {
                aButtonPos = pViewData->GetScrPos( aRange.aEnd.Col()+1, aRange.aStart.Row(),
                                                    eWhich, TRUE );
                aButtonPos.Y() -= nBHeight;
            }
            aButtonPos.X() -= nBWidth - nHSpace;    // in beiden Faellen gleich

            Rectangle aButRect( aButtonPos, Size(nBWidth,nBHeight) );
            if ( aButRect.IsInside( rPosPixel ) )
            {
                rScenRange = aRange;
                return TRUE;
            }
        }
    }

    return FALSE;
}




