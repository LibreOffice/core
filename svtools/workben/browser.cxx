/*************************************************************************
 *
 *  $RCSfile: browser.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:07 $
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
#pragma hdrstop

//#undef VCL

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif

#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SVX_BRWBOX_HXX //autogen
#include <brwbox.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#define MID_BROWSER             10
#define MID_MODE                11
#define MID_EVENTVIEW           12
#define MID_INVALIDATE          13
#define MID_CLEAR               14
#define MID_ROW                 20
#define MID_INSERTROW_BEFORE    21
#define MID_INSERTROW_AT        22
#define MID_INSERTROW_BEHIND    23
#define MID_REMOVEROW_BEFORE    24
#define MID_REMOVEROW_AT        25
#define MID_REMOVEROW_BEHIND    26
#define MID_MODIFYROW_BEFORE    27
#define MID_MODIFYROW_AT        28
#define MID_MODIFYROW_BEHIND    29
#define MID_COL                 30
#define MID_INSERTCOL           31
#define MID_REMOVECOL           32
#define MID_MOVECOLLEFT         33
#define MID_MOVECOLRIGHT        34
#define MID_SELECTIONS          60
#define MID_SELECTROW           61
#define MID_SELECTALL           62
#define MID_SELECTNONE          63
#define MID_INVERSE             64
#define MID_EXTRAS              70
#define MID_STARMONEY_1         71

//==================================================================

class BrowseModeDialog: public ModalDialog
{
    RadioButton     aSingleSel;
    RadioButton     aMultiSel;
    GroupBox        aSelGroup;

    CheckBox        aKeepHighlight;
    CheckBox        aColumnCursor;
    CheckBox        aThumbDragging;
    GroupBox        aOptGroup;

    RadioButton     aNoHLines;
    RadioButton     aDottedHLines;
    RadioButton     aFullHLines;
    GroupBox        aHLinGroup;

    RadioButton     aNoVLines;
    RadioButton     aDottedVLines;
    RadioButton     aFullVLines;
    GroupBox        aVLinGroup;

    OKButton        aOKButton;
    CancelButton    aCancelButton;

public:
                    BrowseModeDialog( Window *pParent );

    void            SetMode( BrowserMode eMode );
    BrowserMode     GetMode() const;
};

//==================================================================

DECLARE_LIST(DataList,long);

class Browser: public BrowseBox
{
friend class AppWindow;

    DataList        aRows;
    BOOL            bInverse;
    Edit*           pEdit;

protected:
    virtual long    GetRowCount() const;
    virtual BOOL    SeekRow( long nRow );
    virtual void    PaintField( OutputDevice& rDev, const Rectangle& rRect,
                                USHORT nColumnId ) const;

    virtual void    Select();
    virtual void    DoubleClick();
    virtual void    CursorMoved();
    virtual void    StartScroll();
    virtual void    EndScroll();
    virtual void    Command( const CommandEvent &eEvt );

    virtual BOOL    StartDragging( Pointer& rMovePtr,
                                   Pointer& rCopyPtr );
    virtual void    EndDragging( const DropAction &rAction );
    virtual void    MouseButtonDown( const BrowserMouseEvent &rEvt );

public:
                    Browser( AppWindow* pParent, BrowserMode eMode );
                    ~Browser();

    void            SetInverseSelection( BOOL bInverseSel )
                    { bInverse = bInverseSel; }
};

//------------------------------------------------------------------

class AppWindow: public WorkWindow
{
    MenuBar         aMenu;
    PopupMenu       aBrwMenu, aRowMenu, aColMenu, aSelMenu, aExtMenu;
    Edit            aEdit;
    Browser         aBrowser;
    BrowserMode     eCurMode;
    FloatingWindow *pEventView;
    ULONG           nNewRowNo;

private:
    DECL_LINK( Modify, void * );
    DECL_LINK( MenuSelect, Menu * );

protected:
    void            Resize();
    void            Activate();

public:
                    AppWindow();
                    ~AppWindow();

    void            Event( const String &rEvent );
};

//------------------------------------------------------------------

class App: public Application
{
protected:
#ifdef VCL
    void            Main();
#else
    void            Main( int, char *[] );
#endif

public:
                    App();
                    ~App();
};

//==================================================================

App aApp;

//==================================================================

BrowseModeDialog::BrowseModeDialog( Window *pParent ):
    ModalDialog( pParent, WinBits( WB_MOVEABLE | WB_CLOSEABLE | WB_SVLOOK ) ),

    aSingleSel( this ),
    aMultiSel( this ),
    aSelGroup( this ),

    aKeepHighlight( this ),
    aColumnCursor( this ),
    aThumbDragging( this ),
    aOptGroup( this ),

    aNoHLines( this ),
    aDottedHLines( this ),
    aFullHLines( this ),
    aHLinGroup( this ),

    aNoVLines( this, WinBits( WB_GROUP ) ),
    aDottedVLines( this ),
    aFullVLines( this ),
    aVLinGroup( this ),

    aOKButton( this ),
    aCancelButton( this )
{
    SetOutputSizePixel( Size( 290, 220 ) );
    SetText( String( "Browse Mode", RTL_TEXTENCODING_IBM_850 ) );

    aSingleSel.SetText( String( "single", RTL_TEXTENCODING_IBM_850 ) );
    aMultiSel.SetText( String( "multi", RTL_TEXTENCODING_IBM_850 ) );
    aSelGroup.SetText( String( " Selection ", RTL_TEXTENCODING_IBM_850 ) );

    aKeepHighlight.SetText( String( "keep highlight", RTL_TEXTENCODING_IBM_850 ) );
    aColumnCursor.SetText( String( "column cursor", RTL_TEXTENCODING_IBM_850 ) );
    aThumbDragging.SetText( String( "thumb dragging", RTL_TEXTENCODING_IBM_850 ) );
    aOptGroup.SetText( String( " Options ", RTL_TEXTENCODING_IBM_850 ) );

    aNoHLines.SetText( String( "none", RTL_TEXTENCODING_IBM_850 ) );
    aDottedHLines.SetText( String( "dotted", RTL_TEXTENCODING_IBM_850 ) );
    aFullHLines.SetText( String( "full", RTL_TEXTENCODING_IBM_850 ) );
    aHLinGroup.SetText( String( " Horizontal ", RTL_TEXTENCODING_IBM_850 ) );

    aNoVLines.SetText( String( "none", RTL_TEXTENCODING_IBM_850 ) );
    aDottedVLines.SetText( String( "dotted", RTL_TEXTENCODING_IBM_850 ) );
    aFullVLines.SetText( String( "full", RTL_TEXTENCODING_IBM_850 ) );
    aVLinGroup.SetText( String( " Vertical ", RTL_TEXTENCODING_IBM_850 ) );

    aSingleSel.SetPosSizePixel( Point( 20, 30 ), Size( 80, 14 ) );
    aMultiSel.SetPosSizePixel( Point( 20, 70 ), Size( 80, 14 ) );
    aSelGroup.SetPosSizePixel( Point( 10, 10 ), Size( 100, 80 ) );
    aSelGroup.SetText( String( " Selection ", RTL_TEXTENCODING_IBM_850 ) );

    aKeepHighlight.SetPosSizePixel( Point( 130, 30 ), Size( 140, 14 ) );
    aColumnCursor.SetPosSizePixel( Point( 130, 50 ), Size( 140, 14 ) );
    aThumbDragging.SetPosSizePixel( Point( 130, 70 ), Size( 140, 14 ) );
    aOptGroup.SetPosSizePixel( Point( 120, 10 ), Size( 160, 80 ) );
    aOptGroup.SetText( String( " Options ", RTL_TEXTENCODING_IBM_850 ) );

    aNoHLines.SetPosSizePixel( Point( 20, 120 ), Size( 80, 14 ) );
    aDottedHLines.SetPosSizePixel( Point( 20, 140 ), Size( 80, 14 ) );
    aFullHLines.SetPosSizePixel( Point( 20, 160 ), Size( 80, 14 ) );
    aHLinGroup.SetPosSizePixel( Point( 10, 100 ), Size( 100, 80 ) );
    aHLinGroup.SetText( String( " horizontal ", RTL_TEXTENCODING_IBM_850 ) );

    aNoVLines.SetPosSizePixel( Point( 130, 120 ), Size( 80, 14 ) );
    aDottedVLines.SetPosSizePixel( Point( 130, 140 ), Size( 80, 14 ) );
    aFullVLines.SetPosSizePixel( Point( 130, 160 ), Size( 80, 14 ) );
    aVLinGroup.SetPosSizePixel( Point( 120, 100 ), Size( 100, 80 ) );
    aVLinGroup.SetText( String( " vertical ", RTL_TEXTENCODING_IBM_850 ) );

    aOKButton.SetPosSizePixel( Point( 10, 190 ), Size( 100, 22 ) );
    aCancelButton.SetPosSizePixel( Point( 120, 190 ), Size( 100, 22 ) );

    aSingleSel.Check();
    aNoHLines.Check();
    aNoVLines.Check();

    aSingleSel.Show();
    aMultiSel.Show();
    aSelGroup.Show();

    aKeepHighlight.Show();
    aColumnCursor.Show();
    aThumbDragging.Show();
    aOptGroup.Show();

    aNoHLines.Show();
    aDottedHLines.Show();
    aFullHLines.Show();
    aHLinGroup.Show();

    aNoVLines.Show();
    aDottedVLines.Show();
    aFullVLines.Show();
    aVLinGroup.Show();

    aOKButton.Show();
    aCancelButton.Show();
}

//------------------------------------------------------------------

void BrowseModeDialog::SetMode( BrowserMode eMode )
{
    if ( ( eMode & BROWSER_COLUMNSELECTION ) == BROWSER_COLUMNSELECTION )
        aColumnCursor.Check();
    if ( ( eMode & BROWSER_MULTISELECTION ) == BROWSER_MULTISELECTION )
        aMultiSel.Check();
    if ( ( eMode & BROWSER_THUMBDRAGGING ) == BROWSER_THUMBDRAGGING )
        aThumbDragging.Check();
    if ( ( eMode & BROWSER_KEEPHIGHLIGHT ) == BROWSER_KEEPHIGHLIGHT )
        aKeepHighlight.Check();
    if ( ( eMode & BROWSER_HLINESFULL ) == BROWSER_HLINESFULL )
        aFullHLines.Check();
    if ( ( eMode & BROWSER_VLINESFULL ) == BROWSER_VLINESFULL )
        aFullVLines.Check();
    if ( ( eMode & BROWSER_HLINESDOTS ) == BROWSER_HLINESDOTS )
        aDottedHLines.Check();
    if ( ( eMode & BROWSER_VLINESDOTS ) == BROWSER_VLINESDOTS )
        aDottedVLines.Check();
}

//------------------------------------------------------------------

BrowserMode BrowseModeDialog::GetMode() const
{
    BrowserMode eMode = 0;

    if ( aColumnCursor.IsChecked() )
        eMode |= BROWSER_COLUMNSELECTION;
    if ( aMultiSel.IsChecked() )
        eMode |= BROWSER_MULTISELECTION;

    if ( aKeepHighlight.IsChecked() )
        eMode |= BROWSER_KEEPHIGHLIGHT;
    if ( aThumbDragging.IsChecked() )
        eMode |= BROWSER_THUMBDRAGGING;

    if ( aDottedHLines.IsChecked() )
        eMode |= BROWSER_HLINESDOTS;
    if ( aFullHLines.IsChecked() )
        eMode |= BROWSER_HLINESFULL;

    if ( aDottedVLines.IsChecked() )
        eMode |= BROWSER_VLINESDOTS;
    if ( aFullVLines.IsChecked() )
        eMode |= BROWSER_VLINESFULL;

    return eMode;
}

//==================================================================

Browser::Browser( AppWindow* pParent, BrowserMode eMode ):
    BrowseBox( pParent, WinBits(WB_DRAG), eMode ),
    bInverse(FALSE),
    pEdit( 0 )
{
    for ( long n = 0; n < 100; ++n )
        aRows.Insert( n, LIST_APPEND );

    //InsertHandleColumn( 30 );
    InsertDataColumn( 1, String( "eins", RTL_TEXTENCODING_IBM_850 ), 85 ); // FreezeColumn( 1 );
    InsertDataColumn( 2, String( "zwei", RTL_TEXTENCODING_IBM_850 ), 85 ); // FreezeColumn( 2 );
    InsertDataColumn( 3, String( "drei", RTL_TEXTENCODING_IBM_850 ), 85 );
    InsertDataColumn( 4, String( "vier", RTL_TEXTENCODING_IBM_850 ), 85 );
    InsertDataColumn( 5, String( "fuenf", RTL_TEXTENCODING_IBM_850 ), 85 );
    InsertDataColumn( 6, String( "sechs", RTL_TEXTENCODING_IBM_850 ), 85 );
    InsertDataColumn( 7, String( "sieben", RTL_TEXTENCODING_IBM_850 ), 85 );
    InsertDataColumn( 8, String( "acht", RTL_TEXTENCODING_IBM_850 ), 85 );
    InsertDataColumn( 9, String( "neun", RTL_TEXTENCODING_IBM_850 ), 85 );
    InsertDataColumn(10, String( "zehn", RTL_TEXTENCODING_IBM_850 ), 85 );
}

//------------------------------------------------------------------

Browser::~Browser()
{
    delete pEdit;
}

//------------------------------------------------------------------

BOOL Browser::StartDragging( Pointer& rMovePtr, Pointer& rCopyPtr )
{
    rMovePtr = Pointer( POINTER_MOVEDATA );
    rCopyPtr = Pointer( POINTER_COPYDATA );
    return TRUE;
}

//------------------------------------------------------------------

void Browser::MouseButtonDown( const BrowserMouseEvent &rEvt )
{
    if ( 3 == rEvt.GetClicks() )
        InfoBox( 0, String( GetColumnAtXPosPixel(rEvt.GetPosPixel().X() ) ) ).Execute();
    else
        BrowseBox::MouseButtonDown( rEvt );
}

//------------------------------------------------------------------

void Browser::EndDragging( const DropAction &rAction )
{
}

//------------------------------------------------------------------

void Browser::StartScroll()
{
    ( (AppWindow*) GetParent() )->Event( String( "StartScroll", RTL_TEXTENCODING_IBM_850 ) );
    if ( pEdit )
        pEdit->Hide();
    BrowseBox::StartScroll();
}

//------------------------------------------------------------------

void Browser::EndScroll()
{
    BrowseBox::EndScroll();
    ( (AppWindow*) GetParent() )->Event( String( "EndScroll", RTL_TEXTENCODING_IBM_850 ) );
    if ( pEdit )
        pEdit->Show();
}
//------------------------------------------------------------------

void Browser::Command( const CommandEvent &rEvt )
{
    String aEvent( String( "Command at ", RTL_TEXTENCODING_IBM_850 ) );
    aEvent += rEvt.GetMousePosPixel().X();
    aEvent += String( ":", RTL_TEXTENCODING_IBM_850 );
    aEvent += rEvt.GetMousePosPixel().Y();
    ( (AppWindow*) GetParent() )->Event( aEvent );
    BrowseBox::Command(rEvt);
}

//------------------------------------------------------------------

void Browser::Select()
{
    String aEvent( String( "Select: ", RTL_TEXTENCODING_IBM_850 ) );
    DELETEZ(pEdit);
    for ( long nRow = FirstSelectedRow(bInverse); nRow >= 0; nRow = NextSelectedRow() )
    {
        if ( bInverse )
            aEvent += String( String( "~", RTL_TEXTENCODING_IBM_850 ) );
        aEvent += String( nRow );
        aEvent += String( ", ", RTL_TEXTENCODING_IBM_850 );
    }
    aEvent.Erase( aEvent.Len() - 2 );

    ( (AppWindow*) GetParent() )->Event( aEvent );
}

//------------------------------------------------------------------

void Browser::DoubleClick()
{
    String aEvent( String( "DoubleClick: ", RTL_TEXTENCODING_IBM_850 ) );
    aEvent += String( GetCurRow() );
    aEvent += String( String( ", ", RTL_TEXTENCODING_IBM_850 ) );
    aEvent += String( FirstSelectedRow() );
    ( (AppWindow*) GetParent() )->Event( aEvent );

    SetNoSelection();
    if ( !pEdit )
        pEdit = new Edit( &GetDataWindow() );
    Rectangle aRect( GetFieldRect( GetCurColumnId() ) );
    pEdit->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
    pEdit->Show();
    pEdit->GrabFocus();
}

//------------------------------------------------------------------

void Browser::CursorMoved()
{
    String aEvent( String( "Cursor: ", RTL_TEXTENCODING_IBM_850 ) );
    aEvent += String( GetCurRow() );
    aEvent += String( ":", RTL_TEXTENCODING_IBM_850 );
    aEvent += String( GetCurColumnId() );
    ( (AppWindow*) GetParent() )->Event( aEvent );
    if ( IsFieldVisible( GetCurRow(), GetCurColumnId(), TRUE ) )
        ( (AppWindow*) GetParent() )->Event( String( "completely visible", RTL_TEXTENCODING_IBM_850 ) );
    else if ( IsFieldVisible( 1, GetCurColumnId(), FALSE) )
        ( (AppWindow*) GetParent() )->Event( String( "partly visible", RTL_TEXTENCODING_IBM_850 ) );
    else
        ( (AppWindow*) GetParent() )->Event( String( "not visible", RTL_TEXTENCODING_IBM_850 ) );

    DELETEZ(pEdit);
}

//------------------------------------------------------------------

long Browser::GetRowCount() const
{
    return aRows.Count();
}

//------------------------------------------------------------------

BOOL Browser::SeekRow( long nRow )
{
    if ( nRow >= 0 && nRow < (long) aRows.Count() )
    {
        aRows.Seek(nRow);
        return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------

void Browser::PaintField( OutputDevice& rDev, const Rectangle& rRect,
                          USHORT nColumnId ) const
{
    rDev.SetClipRegion( rRect );
    String aText( aRows.GetCurObject() );
    aText += String( ".", RTL_TEXTENCODING_IBM_850 );
    aText += String( nColumnId );
    rDev.DrawText( rRect.TopLeft(), aText );
}

//==================================================================

AppWindow::AppWindow():
    WorkWindow( 0, WinBits(WB_APP|WB_STDWORK) ),
    aEdit( this, WinBits( WB_BORDER ) ),
    aBrowser( this, 0 ),
    eCurMode( 0 ),
    pEventView( 0 ),
    nNewRowNo( 2000L )
{
    SetText( String( "BrowseBox Testframe", RTL_TEXTENCODING_IBM_850 ) );

    aMenu.InsertItem( MID_BROWSER, String( "~Browser", RTL_TEXTENCODING_IBM_850 ) );
     aMenu.SetPopupMenu( MID_BROWSER, &aBrwMenu );
     aBrwMenu.InsertItem( MID_MODE, String( "~Mode...", RTL_TEXTENCODING_IBM_850 ) );
     aBrwMenu.InsertItem( MID_EVENTVIEW, String( "~Event-Viewer", RTL_TEXTENCODING_IBM_850 ) );
     aBrwMenu.InsertSeparator();
     aBrwMenu.InsertItem( MID_INVALIDATE, String( "~Invalidate", RTL_TEXTENCODING_IBM_850 ) );
     aBrwMenu.InsertItem( MID_CLEAR, String( "~Clear", RTL_TEXTENCODING_IBM_850 ) );
    aMenu.InsertItem( MID_ROW, String( "~Row", RTL_TEXTENCODING_IBM_850 ) );
     aMenu.SetPopupMenu( MID_ROW, &aRowMenu );
     aRowMenu.InsertItem( MID_INSERTROW_BEFORE, String( "Insert before current", RTL_TEXTENCODING_IBM_850 ) );
     aRowMenu.InsertItem( MID_INSERTROW_AT, String( "~Insert at current", RTL_TEXTENCODING_IBM_850 ) );
     aRowMenu.InsertItem( MID_INSERTROW_BEHIND, String( "~Insert behind current", RTL_TEXTENCODING_IBM_850 ) );
     aRowMenu.InsertSeparator();
     aRowMenu.InsertItem( MID_REMOVEROW_BEFORE, String( "Remove before current", RTL_TEXTENCODING_IBM_850 ) );
     aRowMenu.InsertItem( MID_REMOVEROW_AT, String( "~Remove at current", RTL_TEXTENCODING_IBM_850 ) );
     aRowMenu.InsertItem( MID_REMOVEROW_BEHIND, String( "~Remove behind current", RTL_TEXTENCODING_IBM_850 ) );
     aRowMenu.InsertSeparator();
     aRowMenu.InsertItem( MID_MODIFYROW_BEFORE, String( "Modify before current", RTL_TEXTENCODING_IBM_850 ) );
     aRowMenu.InsertItem( MID_MODIFYROW_AT, String( "~Modify at current", RTL_TEXTENCODING_IBM_850 ) );
     aRowMenu.InsertItem( MID_MODIFYROW_BEHIND, String( "~Modify behind current", RTL_TEXTENCODING_IBM_850 ) );
    aMenu.InsertItem( MID_COL, String( "~Column", RTL_TEXTENCODING_IBM_850 ) );
     aMenu.SetPopupMenu( MID_COL, &aColMenu );
     aColMenu.InsertItem( MID_INSERTCOL, String( "~Insert", RTL_TEXTENCODING_IBM_850 ) );
     aColMenu.InsertItem( MID_REMOVECOL, String( "Re~move", RTL_TEXTENCODING_IBM_850 ) );
     aColMenu.InsertItem( MID_MOVECOLLEFT, String( "Move ~Left", RTL_TEXTENCODING_IBM_850 ) );
     aColMenu.InsertItem( MID_MOVECOLRIGHT, String( "Move ~Right", RTL_TEXTENCODING_IBM_850 ) );
    aMenu.InsertItem( MID_SELECTIONS, String( "Selections", RTL_TEXTENCODING_IBM_850 ) );
     aMenu.SetPopupMenu( MID_SELECTIONS, &aSelMenu );
     aSelMenu.InsertItem( MID_SELECTROW, String( "Row", RTL_TEXTENCODING_IBM_850 ) );
     aSelMenu.InsertItem( MID_SELECTALL, String( "All", RTL_TEXTENCODING_IBM_850 ) );
     aSelMenu.InsertItem( MID_SELECTNONE, String( "None", RTL_TEXTENCODING_IBM_850 ) );
     aSelMenu.InsertItem( MID_INVERSE, String( "Inverse", RTL_TEXTENCODING_IBM_850 ), MENU_APPEND, MIB_CHECKABLE );
    aMenu.InsertItem( MID_EXTRAS, String( "Extras", RTL_TEXTENCODING_IBM_850 ) );
     aMenu.SetPopupMenu( MID_EXTRAS, &aExtMenu );
     aExtMenu.InsertItem( MID_STARMONEY_1, String( "StarMoney: SelectRow(+1)+DeleteRow", RTL_TEXTENCODING_IBM_850 ) );
    aMenu.PushSelectHdl( LINK(this, AppWindow, MenuSelect) );

    aEdit.SetModifyHdl( LINK(this,AppWindow,Modify) );

    aEdit.Show();
    aBrowser.Show();
    Show();
}

//------------------------------------------------------------------

AppWindow::~AppWindow()
{
    GetpApp()->SetAppMenu( 0 );
    if ( pEventView )
        delete pEventView;
}

//------------------------------------------------------------------

IMPL_LINK_INLINE_START( AppWindow, Modify, void *, pCaller )
{
    Edit *pEdit = (Edit*) pCaller;
    aBrowser.GoToRow( pEdit->GetText().ToInt32() );
    aBrowser.GrabFocus();

    return TRUE;
}
IMPL_LINK_INLINE_END( AppWindow, Modify, void *, pCaller )

//------------------------------------------------------------------

IMPL_LINK( AppWindow, MenuSelect, Menu *, pMenu )
{
    ULONG nPos;

    switch ( pMenu->GetCurItemId() )
    {
        case MID_MODE:
        {
            BrowseModeDialog *pDlg = new BrowseModeDialog( this );
            pDlg->SetMode( eCurMode );
            if ( pDlg->Execute() == RET_OK )
            {
                eCurMode = pDlg->GetMode();
                aBrowser.SetMode( eCurMode | BROWSER_AUTOSIZE_LASTCOL );
            }
            delete pDlg;
            break;
        }

        case MID_INVALIDATE:
            aBrowser.Invalidate();
            break;

        case MID_INSERTROW_BEFORE:
            if ( aBrowser.GetCurRow() == 0 )
            {
                Sound::Beep();
                break;
            }
            nPos = aBrowser.GetCurRow() - 1;
            aBrowser.aRows.Insert( nNewRowNo++, nPos );
            aBrowser.RowInserted( aBrowser.GetCurRow()-1 );
            break;

        case MID_INSERTROW_AT:
            nPos = aBrowser.GetCurRow();
            if ( nPos == BROWSER_ENDOFSELECTION )
                nPos = 0;
            aBrowser.aRows.Insert( nNewRowNo++, nPos );
            aBrowser.RowInserted( nPos );
            break;

        case MID_INSERTROW_BEHIND:
            nPos = aBrowser.GetCurRow() + 1;
            aBrowser.aRows.Insert( nNewRowNo++, nPos );
            aBrowser.RowInserted( nPos );
            break;

        case MID_REMOVEROW_BEFORE:
            if ( aBrowser.GetCurRow() == 0 )
            {
                Sound::Beep();
                break;
            }
            nPos = aBrowser.GetCurRow() - 1;
            aBrowser.aRows.Remove( nPos );
            aBrowser.RowRemoved( nPos );
            break;

        case MID_REMOVEROW_AT:
            nPos = aBrowser.GetCurRow();
            aBrowser.aRows.Remove( nPos );
            aBrowser.RowRemoved( nPos );
            break;

        case MID_REMOVEROW_BEHIND:
            if ( (aBrowser.GetCurRow()+1) >= aBrowser.GetRowCount() )
            {
                Sound::Beep();
                break;
            }
            nPos = aBrowser.GetCurRow() + 1;
            aBrowser.aRows.Remove( nPos );
            aBrowser.RowRemoved( nPos );
            break;

        case MID_MODIFYROW_BEFORE:
            if ( aBrowser.GetCurRow() == 0 )
            {
                Sound::Beep();
                break;
            }
            nPos = aBrowser.GetCurRow() - 1;
            aBrowser.aRows.Replace( nNewRowNo++, nPos );
            aBrowser.RowModified( nPos );
            break;

        case MID_MODIFYROW_AT:
            nPos = aBrowser.GetCurRow();
            aBrowser.aRows.Replace( nNewRowNo++, nPos );
            aBrowser.RowModified( nPos );
            break;

        case MID_MODIFYROW_BEHIND:
            if ( (aBrowser.GetCurRow()+1) >= aBrowser.GetRowCount() )
            {
                Sound::Beep();
                break;
            }
            nPos = aBrowser.GetCurRow() + 1;
            aBrowser.aRows.Replace( nNewRowNo++, nPos );
            aBrowser.RowModified( nPos );
            break;

        case MID_EVENTVIEW:
            if ( pEventView )
            {
                delete pEventView;
                pEventView = 0;
            }
            else
            {
                pEventView = new FloatingWindow( this );
                pEventView->SetPosPixel( Point( 100, 100 ) );
                pEventView->SetOutputSizePixel(
                    Size( 320, 8*GetTextHeight() ) );
                pEventView->Show();
                aBrowser.CursorMoved();
                aBrowser.Select();
            }
            break;

        case MID_SELECTROW:
            aBrowser.SelectRow( aBrowser.GetCurRow(),
                !aBrowser.IsRowSelected( aBrowser.GetCurRow() ) );
            break;

        case MID_SELECTALL:
            aBrowser.SelectAll();
            break;

        case MID_SELECTNONE:
            aBrowser.SetNoSelection();
            break;

        case MID_INVERSE:
        {
            BOOL bChecked = pMenu->IsItemChecked( MID_INVERSE );
            pMenu->CheckItem( MID_INVERSE, !bChecked );
            aBrowser.SetInverseSelection( !bChecked );
            break;
        }

        case MID_CLEAR:
            aBrowser.Clear();
            break;

        case MID_STARMONEY_1:
        {
            nPos = aBrowser.GetCurRow();
            aBrowser.SelectRow( nPos + 1, TRUE );
            aBrowser.aRows.Remove( nPos );
            aBrowser.RowRemoved( nPos );
            break;
        }
    }

    return TRUE;
}

//------------------------------------------------------------------

void AppWindow::Resize()
{
    Size aOutSz( GetOutputSizePixel() );

    aEdit.SetPosSizePixel(
        Point( 0, 0 ),
        Size( aOutSz.Width(), 24 ) );

    aBrowser.SetPosSizePixel(
        Point( 0, aEdit.GetSizePixel().Height() ),
        Size( aOutSz.Width(), aOutSz.Height() - 24 ) );
}

//------------------------------------------------------------------

void AppWindow::Activate()
{
    GetpApp()->SetAppMenu( &aMenu );
    aBrowser.GrabFocus();
}

//------------------------------------------------------------------

void AppWindow::Event( const String &rEvent )
{
    if ( pEventView )
    {
        pEventView->Scroll( 0, -GetTextHeight() );
        pEventView->Update();
        pEventView->DrawText( Point(0, 7*GetTextHeight() ), rEvent );
    }
}

//==================================================================

App::App()
{
}

//------------------------------------------------------------------

App::~App()
{
}

//------------------------------------------------------------------

#ifdef VCL
void App::Main( )
#else
void App::Main( int, char *[] )
#endif
{
    EnableSVLook();

    AppWindow aAppWin;


    Execute();
}

