/*************************************************************************
 *
 *  $RCSfile: svdem.cxx,v $
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
#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/drag.hxx>
#include <vcl/print.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/slider.hxx>
#include <vcl/group.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/status.hxx>
#include <stdmenu.hxx>
#include <ctrltool.hxx>
#include <ctrlbox.hxx>
#include <tabbar.hxx>
#include <valueset.hxx>
#include <headbar.hxx>
#include <prgsbar.hxx>
#include <calendar.hxx>
#include <prnsetup.hxx>
#include <printdlg.hxx>

// -----------------------------------------------------------------------

class MyApp : public Application
{
public:
    void Main();
};

// -----------------------------------------------------------------------

class ShowBitmap : public WorkWindow
{
    Bitmap          aBmp;

public:
                    ShowBitmap( Window* pParent, const Bitmap& rBmp );

    virtual void    Paint( const Rectangle& );
    virtual BOOL    Close();
};

// -----------------------------------------------------------------------

class ShowFont : public Control
{
public:
                    ShowFont( Window* pParent );

    virtual void    Paint( const Rectangle& );
    void            SetFont( const Font& rFont )
                        { Invalidate(); Control::SetFont( rFont ); }
};

// --- class OrientSlider ------------------------------------------------

class OrientSlider : public Slider
{
public:
                OrientSlider( Window* pParent );

    short       GetOrientation() const { return (short)GetThumbPos(); }
};

// -----------------------------------------------------------------------

OrientSlider::OrientSlider( Window* pParent ) :
    Slider( pParent, WB_HORZ | WB_DRAG )
{
    SetThumbPos( 0 );
    SetLineSize( 10 );
    SetPageSize( 100 );
    SetRange( Range( 0, 3600 ) );
}

// -----------------------------------------------------------------------

class MyFontDialog : public ModalDialog
{
private:
    FontList*       pList;
    Font            aCurFont;
    Printer         aPrinter;
    FontNameBox     aFontBox;
    FontStyleBox    aStyleBox;
    FontSizeBox     aSizeBox;
    ListBox         aUnderlineBox;
    ListBox         aStrikeoutBox;
    CheckBox        aWordLineBox;
    CheckBox        aShadowBox;
    CheckBox        aOutlineBox;
    ColorListBox    aColorBox;
    GroupBox        aEffectBox;
    OrientSlider    aLineOrientSlider;
    ShowFont        aShowFont;
    GroupBox        aSampleBox;
    FixedText       aMapText;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;

public:
                    MyFontDialog( Window* pParent );

                    DECL_LINK( SelectFont, ComboBox* );
                    DECL_LINK( SelectStyle, ComboBox* );
                    DECL_LINK( AttrHdl, Window * );
    void            SetAttr();
    short           Execute();
};

// -----------------------------------------------------------------------

class MyTabBar : public TabBar
{
public:
                    MyTabBar( Window* pParent,
                              WinBits nWinStyle = WB_STDTABBAR ) :
                        TabBar( pParent, nWinStyle ) {}

    virtual long    DeactivatePage();
    virtual long    AllowRenaming();
    virtual void    Split();
    virtual void    Command( const CommandEvent& rCEvt );
    virtual BOOL    QueryDrop( DropEvent& rDEvt );
};

// -----------------------------------------------------------------------

class MyCalendar : public WorkWindow
{
    MenuBar     aMenuBar;
    PopupMenu   aWeekStartMenu;
    PopupMenu   aWeekCountMenu;
    Calendar    aCalendar;
    Color       aInfoColor;
    Color       aHolidayColor;
    Color       aFrameColor;

public:
                MyCalendar( Window* pParent );
                ~MyCalendar();

                DECL_LINK( RequestDateInfoHdl, Calendar* );
                DECL_LINK( DoubleClickHdl, Calendar* );
                DECL_LINK( MenuSelectHdl, Menu* );

    void        Resize();
};

// -----------------------------------------------------------------------

class MyWin : public WorkWindow
{
private:
    Printer         aPrn;
    ToolBox         aBox;
    StatusBar       aBar;
    HeaderBar       aHeadBar;
    ColorListBox    aColorList;
    LineListBox     aLineList;
    ValueSet        aValueSet;
    CalendarField   aCalendarField;
    CalendarField   aCalendarField2;
    MyTabBar        aTabBar;
    ProgressBar     aPrgsBar;
    PushButton      aFontBtn;
    PushButton      aCalendarBtn;
    PushButton      aPrnSetupBtn;
    PushButton      aPrnDlgBtn;
    Size            aBoxSize;
    MyCalendar*     pCalendar;
    PopupMenu*      pMenu;
    FontNameMenu*   pNameMenu;
    FontStyleMenu*  pStyleMenu;
    FontSizeMenu*   pSizeMenu;

public:
                    MyWin( Window* pParent, WinBits aWinStyle );
                    ~MyWin();

                    DECL_LINK( Test, PushButton* );
                    DECL_LINK( SelectHdl, Window* );
                    DECL_LINK( CalSelectHdl, CalendarField* );
    void            ContextMenu( const Point& rPos );

    void            Command( const CommandEvent& rCEvt );
    void            MouseButtonDown( const MouseEvent& rMEvt );
    void            KeyInput( const KeyEvent& rKEvt );
    void            Paint( const Rectangle& rRect );
    void            Resize();
};

// -----------------------------------------------------------------------

void MyApp::Main()
{
    Help aHelp;
    SetHelp( &aHelp );
    Help::EnableContextHelp();
    Help::EnableExtHelp();
    Help::EnableBalloonHelp();
    Help::EnableQuickHelp();

    MyWin aMainWin( NULL, WinBits( WB_APP | WB_STDWORK | WB_CLIPCHILDREN ) );
    aMainWin.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "SVTOOLS - Workbench" ) ) );
    aMainWin.GrabFocus();
    aMainWin.Show();

    Execute();
}

// -----------------------------------------------------------------------

ShowBitmap::ShowBitmap( Window* pParent, const Bitmap& rBmp ) :
    WorkWindow( pParent, WB_STDWORK ),
    aBmp( rBmp )
{
    SetOutputSizePixel( rBmp.GetSizePixel() );
    SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Bitmap-Viewer" ) ) );
    Show();
}

// -----------------------------------------------------------------------

void ShowBitmap::Paint( const Rectangle& )
{
    DrawBitmap( Point(), GetOutputSizePixel(), aBmp );
}

// -----------------------------------------------------------------------

BOOL ShowBitmap::Close()
{
    Hide();
    delete this;
    return TRUE;
}

// -----------------------------------------------------------------------

ShowFont::ShowFont( Window* pParent ) :
    Control( pParent, WB_BORDER )
{
    SetMapMode( MapMode( MAP_POINT, Point(),
                         Fraction( 1, 10 ), Fraction( 1, 10 ) ) );
    SetBackground( Wallpaper( Color( COL_WHITE ) ) );
}

// -----------------------------------------------------------------------

void ShowFont::Paint( const Rectangle& )
{
    const Font& rFont = GetFont();
    String      aText;
    Size        aWindowSize( GetOutputSize() );
    long        x,y;

    if ( rFont.GetLineOrientation() )
    {
        aText.Append( String::CreateFromInt32( rFont.GetLineOrientation()/10 ) );
        aText.AppendAscii( " degree." );

        x = aWindowSize.Width()/2;
        y = aWindowSize.Height()/2;
    }
    else
    {
        aText = rFont.GetName();
        if ( !aText.Len() )
            aText.AssignAscii( "Sample" );

        x = aWindowSize.Width()/2 - GetTextWidth( aText )/2;
        y = aWindowSize.Height()/2 - GetTextHeight()/2;
    }

    DrawText( Point( x, y ), aText );
}

// -----------------------------------------------------------------------

MyFontDialog::MyFontDialog( Window* pParent ) :
    ModalDialog( pParent, WB_SVLOOK | WB_STDMODAL ),
    aFontBox( this ),
    aStyleBox( this ),
    aSizeBox( this ),
    aUnderlineBox( this, WB_DROPDOWN ),
    aStrikeoutBox( this, WB_DROPDOWN ),
    aWordLineBox( this ),
    aShadowBox( this ),
    aOutlineBox( this ),
    aColorBox( this, WB_DROPDOWN ),
    aEffectBox( this ),
    aLineOrientSlider( this ),
    aShowFont( this ),
    aSampleBox( this ),
    aMapText( this, WB_LEFT | WB_WORDBREAK ),
    aOKBtn( this, WB_DEFBUTTON ),
    aCancelBtn( this )
{
    pList = NULL;

    aFontBox.EnableWYSIWYG( TRUE );
    aFontBox.EnableSymbols( TRUE );
    aFontBox.SetPosSizePixel( Point( 10, 10 ), Size( 140, 140 ) );
    aFontBox.SetSelectHdl( LINK( this, MyFontDialog, SelectFont ) );
    aFontBox.SetLoseFocusHdl( LINK( this, MyFontDialog, SelectFont ) );
    aFontBox.Show();

    aStyleBox.SetPosSizePixel( Point( 160, 10 ), Size( 100, 140 ) );
    aStyleBox.SetSelectHdl( LINK( this, MyFontDialog, SelectStyle ) );
    aStyleBox.SetLoseFocusHdl( LINK( this, MyFontDialog, SelectStyle ) );
    aStyleBox.Show();

    aSizeBox.SetPosSizePixel( Point( 270, 10 ), Size( 60, 140 ) );
    aSizeBox.SetSelectHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aSizeBox.SetLoseFocusHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aSizeBox.Show();

    aUnderlineBox.SetPosSizePixel( Point( 15, 180 ), Size( 130, 100 ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_NONE" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_SINGLE" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_DOUBLE" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_DOTTED" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_DONTKNOW" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_DASH" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_LONGDASH" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_DASHDOT" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_DASHDOTDOT" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_SMALLWAVE" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_WAVE" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_DOUBLEWAVE" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_BOLD" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_BOLDDOTTED" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_BOLDDASH" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_BOLDLONGDASH" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_BOLDDASHDOT" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_BOLDDASHDOTDOT" ) ) );
    aUnderlineBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "UNDERLINE_BOLDWAVE" ) ) );
    aUnderlineBox.SetSelectHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aUnderlineBox.Show();

    aStrikeoutBox.SetPosSizePixel( Point( 15, 210 ), Size( 130, 100 ) );
    aStrikeoutBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "STRIKEOUT_NONE" ) ) );
    aStrikeoutBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "STRIKEOUT_SINGLE" ) ) );
    aStrikeoutBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "STRIKEOUT_DOUBLE" ) ) );
    aStrikeoutBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "STRIKEOUT_DONTKNOW" ) ) );
    aStrikeoutBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "STRIKEOUT_BOLD" ) ) );
    aStrikeoutBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "STRIKEOUT_SLASH" ) ) );
    aStrikeoutBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "STRIKEOUT_X" ) ) );
    aStrikeoutBox.SetSelectHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aStrikeoutBox.Show();

    aWordLineBox.SetPosSizePixel( Point( 15, 240 ), Size( 130, 19 ) );
    aWordLineBox.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Only ~Words" ) ) );
    aWordLineBox.SetClickHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aWordLineBox.Show();

    aShadowBox.SetPosSizePixel( Point( 15, 260 ), Size( 130, 19 ) );
    aShadowBox.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Shadow" ) ) );
    aShadowBox.SetClickHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aShadowBox.Show();

    aOutlineBox.SetPosSizePixel( Point( 15, 280 ), Size( 130, 19 ) );
    aOutlineBox.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Outline" ) ) );
    aOutlineBox.SetClickHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aOutlineBox.Show();

    {
    aColorBox.SetPosSizePixel( Point( 15, 305 ), Size( 130, 100 ) );
    aColorBox.SetSelectHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aColorBox.SetUpdateMode( FALSE );
    aColorBox.InsertEntry( Color( COL_BLACK ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Black" ) ) );
    aColorBox.InsertEntry( Color( COL_BLUE ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Blue" ) ) );
    aColorBox.InsertEntry( Color( COL_GREEN ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Green" ) ) );
    aColorBox.InsertEntry( Color( COL_CYAN ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Cyan" ) ) );
    aColorBox.InsertEntry( Color( COL_RED ),           XubString( RTL_CONSTASCII_USTRINGPARAM( "Red" ) ) );
    aColorBox.InsertEntry( Color( COL_MAGENTA ),       XubString( RTL_CONSTASCII_USTRINGPARAM( "Magenta" ) ) );
    aColorBox.InsertEntry( Color( COL_BROWN ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Brown" ) ) );
    aColorBox.InsertEntry( Color( COL_GRAY ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Gray" ) ) );
    aColorBox.InsertEntry( Color( COL_LIGHTGRAY ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightGray" ) ) );
    aColorBox.InsertEntry( Color( COL_LIGHTBLUE ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightBlue" ) ) );
    aColorBox.InsertEntry( Color( COL_LIGHTGREEN ),    XubString( RTL_CONSTASCII_USTRINGPARAM( "LightGreen" ) ) );
    aColorBox.InsertEntry( Color( COL_LIGHTCYAN ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightCyan" ) ) );
    aColorBox.InsertEntry( Color( COL_LIGHTRED ),      XubString( RTL_CONSTASCII_USTRINGPARAM( "LightRed" ) ) );
    aColorBox.InsertEntry( Color( COL_LIGHTMAGENTA ),  XubString( RTL_CONSTASCII_USTRINGPARAM( "LightMagenta" ) ) );
    aColorBox.InsertEntry( Color( COL_YELLOW ),        XubString( RTL_CONSTASCII_USTRINGPARAM( "Yellow" ) ) );
    aColorBox.InsertEntry( Color( COL_WHITE ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "White" ) ) );
    aColorBox.SetUpdateMode( TRUE );
    aColorBox.Show();
    }

    aEffectBox.SetPosSizePixel( Point( 10, 160 ), Size( 140, 175 ) );
    aEffectBox.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Effects" ) ) );
    aEffectBox.Show();

    Size aSliderSize = aLineOrientSlider.GetSizePixel();
    aLineOrientSlider.SetPosSizePixel( Point( 160, 335-aSliderSize.Height() ),
                                       Size( 250, aSliderSize.Height() ) );
    aLineOrientSlider.SetSlideHdl( LINK( this, MyFontDialog, AttrHdl ) );
    aLineOrientSlider.Show();

    aShowFont.SetPosSizePixel( Point( 165, 180 ), Size( 240, 70 ) );
    aShowFont.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Sample" ) ) );
    aShowFont.Show();

    aSampleBox.SetPosSizePixel( Point( 160, 160 ), Size( 250, 100 ) );
    aSampleBox.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Sample" ) ) );
    aSampleBox.Show();

    aMapText.SetPosSizePixel( Point( 160, 270 ), Size( 250, 35 ) );
    aMapText.Show();

    aOKBtn.SetPosSizePixel( Point( 340, 10 ), Size( 70, 25 ) );
    aOKBtn.Show();

    aCancelBtn.SetPosSizePixel( Point( 340, 40 ), Size( 70, 25 ) );
    aCancelBtn.Show();

    SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "FontDialog" ) ) );
    SetOutputSizePixel( Size( 420, 345 ) );
}

// -----------------------------------------------------------------------

IMPL_LINK( MyFontDialog, SelectFont, ComboBox*, EMPTYARG )
{
    aStyleBox.Fill( aFontBox.GetText(), pList );
    FontInfo aInfo = pList->Get( aFontBox.GetText(), aStyleBox.GetText() );
    aSizeBox.Fill( aInfo, pList );
    SetAttr();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( MyFontDialog, SelectStyle, ComboBox*, EMPTYARG )
{
    FontInfo aInfo = pList->Get( aFontBox.GetText(), aStyleBox.GetText() );
    aSizeBox.Fill( aInfo, pList );
    SetAttr();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( MyFontDialog, AttrHdl, Window*, EMPTYARG )
{
    SetAttr();
    return 0;
}

// -----------------------------------------------------------------------

void MyFontDialog::SetAttr()
{
    FontInfo aFont( pList->Get( aFontBox.GetText(), aStyleBox.GetText() ) );
    aFont.SetSize( Size( 0, aSizeBox.GetValue() ) );
    aFont.SetUnderline( (FontUnderline)aUnderlineBox.GetSelectEntryPos() );
    aFont.SetStrikeout( (FontStrikeout)aStrikeoutBox.GetSelectEntryPos() );
    aFont.SetColor( Color( (ColorName)aColorBox.GetSelectEntryPos() ) );
    aFont.SetWordLineMode( aWordLineBox.IsChecked() );
    aFont.SetShadow( aShadowBox.IsChecked() );
    aFont.SetOutline( aOutlineBox.IsChecked() );
    aFont.SetLineOrientation( aLineOrientSlider.GetOrientation() );
    aFont.SetTransparent( TRUE );
    aMapText.SetText( pList->GetFontMapText( aFont ) );
    aShowFont.SetFont( aFont );
}

// -----------------------------------------------------------------------

short MyFontDialog::Execute()
{
    pList = new FontList( &aPrinter, this );
    aFontBox.Fill( pList );
    aSizeBox.SetValue( 120 );
    aUnderlineBox.SelectEntryPos( 0 );
    aStrikeoutBox.SelectEntryPos( 0 );
    aColorBox.SelectEntryPos( 0 );
    SelectFont( &aFontBox );
    short nRet = ModalDialog::Execute();
    delete pList;
    return nRet;
}

// -----------------------------------------------------------------------

long MyTabBar::DeactivatePage()
{
    if ( GetCurPageId() == 6 )
    {
        QueryBox aQueryBox( this, WB_YES_NO | WB_DEF_YES,
                            XubString( RTL_CONSTASCII_USTRINGPARAM( "Deactivate" ) ) );
        if ( aQueryBox.Execute() == RET_YES )
            return TRUE;
        else
            return FALSE;
    }
    else
        return TRUE;
}

// -----------------------------------------------------------------------

long MyTabBar::AllowRenaming()
{
    XubString aStr( RTL_CONSTASCII_USTRINGPARAM( "Allow renaming: " ) );
    aStr += GetEditText();
    QueryBox aQueryBox( this, WB_YES_NO_CANCEL | WB_DEF_YES, aStr );
    long nRet = aQueryBox.Execute();
    if ( nRet == RET_YES )
        return TAB_RENAMING_YES;
    else if ( nRet == RET_NO )
        return TAB_RENAMING_NO;
    else // ( nRet == RET_CANCEL )
        return TAB_RENAMING_CANCEL;
}

// -----------------------------------------------------------------------

void MyTabBar::Split()
{
    Size    aSize = GetSizePixel();
    long    nWidth = GetSplitSize();
    long    nMaxWidth = GetParent()->GetOutputSizePixel().Width()-50;
    if ( nWidth < GetMinSize() )
        nWidth = GetMinSize();
    else if ( nWidth > nMaxWidth )
        nWidth = nMaxWidth;
    SetSizePixel( Size( nWidth, aSize.Height() ) );
}

// -----------------------------------------------------------------------

void MyTabBar::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_STARTDRAG )
    {
        Region aRegion;
        if ( StartDrag( rCEvt, aRegion ) )
        {
            DragServer::Clear();
            DragServer::CopyString( XubString( RTL_CONSTASCII_USTRINGPARAM( "TabBar" ) ) );
            if ( GetSelectPageCount() > 1 )
            {
                ExecuteDrag( Pointer( POINTER_MOVEFILES ),
                             Pointer( POINTER_COPYFILES ),
                             DRAG_ALL, &aRegion );
            }
            else
            {
                ExecuteDrag( Pointer( POINTER_MOVEFILE ),
                             Pointer( POINTER_COPYFILE ),
                             DRAG_ALL, &aRegion );
            }
            HideDropPos();
        }
    }
}

// -----------------------------------------------------------------------

BOOL MyTabBar::QueryDrop( DropEvent& rDEvt )
{
    if ( rDEvt.IsLeaveWindow() )
        HideDropPos();
    else
        ShowDropPos( rDEvt.GetPosPixel() );
/*
    if ( rDEvt.IsLeaveWindow() )
        EndSwitchPage();
    else
        SwitchPage( rDEvt.GetPosPixel() );
*/

    return TRUE;
}

// -----------------------------------------------------------------------

MyCalendar::MyCalendar( Window* pParent ) :
    WorkWindow( pParent, WB_STDWORK ),
    aCalendar( this, WB_TABSTOP | WB_WEEKNUMBER | WB_BOLDTEXT | WB_FRAMEINFO | WB_MULTISELECT ),
    aInfoColor( COL_LIGHTBLUE ),
    aHolidayColor( COL_LIGHTRED ),
    aFrameColor( COL_LIGHTRED )
{
    const International& rIntn = aCalendar.GetInternational();
    aMenuBar.InsertItem( 1, XubString( RTL_CONSTASCII_USTRINGPARAM( "Wochen~anfang" ) ) );
    aMenuBar.InsertItem( 2, XubString( RTL_CONSTASCII_USTRINGPARAM( "~Erste Woche" ) ) );
    aMenuBar.SetPopupMenu( 1, &aWeekStartMenu );
    aMenuBar.SetPopupMenu( 2, &aWeekCountMenu );
    for ( USHORT i = 0; i < 7; i++ )
        aWeekStartMenu.InsertItem( 10+i, rIntn.GetDayText( (DayOfWeek)i ), MIB_AUTOCHECK | MIB_RADIOCHECK );
    aWeekStartMenu.CheckItem( 10+(USHORT)rIntn.GetWeekStart() );
    aWeekCountMenu.InsertItem( 20, XubString( RTL_CONSTASCII_USTRINGPARAM( "~1. Januar" ) ), MIB_AUTOCHECK | MIB_RADIOCHECK );
    aWeekCountMenu.InsertItem( 21, XubString( RTL_CONSTASCII_USTRINGPARAM( "Erste 4 ~Tage-Woche" ) ), MIB_AUTOCHECK | MIB_RADIOCHECK );
    aWeekCountMenu.InsertItem( 22, XubString( RTL_CONSTASCII_USTRINGPARAM( "Erste ~volle Woche" ) ), MIB_AUTOCHECK | MIB_RADIOCHECK );
    aWeekCountMenu.CheckItem( 20+(USHORT)rIntn.GetWeekCountStart() );
    aMenuBar.SetSelectHdl( LINK( this, MyCalendar, MenuSelectHdl ) );
    SetMenuBar( &aMenuBar );

    Date aCurDate = aCalendar.GetCurDate();
    aCalendar.SetRequestDateInfoHdl( LINK( this, MyCalendar, RequestDateInfoHdl ) );
    aCalendar.SetDoubleClickHdl( LINK( this, MyCalendar, DoubleClickHdl ) );
    aCalendar.SetSaturdayColor( Color( COL_LIGHTGREEN ) );
    aCalendar.SetSundayColor( aHolidayColor );
    aCalendar.AddDateInfo( Date(  1,  1, 0 ), XubString( RTL_CONSTASCII_USTRINGPARAM( "Neujahr" ) ), &aHolidayColor, NULL );
    aCalendar.AddDateInfo( Date( 24, 12, 0 ), XubString( RTL_CONSTASCII_USTRINGPARAM( "Heiligabend" ) ), &aInfoColor, NULL );
    aCalendar.AddDateInfo( Date( 25, 12, 0 ), XubString( RTL_CONSTASCII_USTRINGPARAM( "1. Weihnachttag" ) ), &aHolidayColor, NULL );
    aCalendar.AddDateInfo( Date( 26, 12, 0 ), XubString( RTL_CONSTASCII_USTRINGPARAM( "2. Weihnachttag" ) ), &aHolidayColor, NULL );
    aCalendar.AddDateInfo( Date( 31, 12, 0 ), XubString( RTL_CONSTASCII_USTRINGPARAM( "Silvester" ) ), &aInfoColor, NULL );
    aCalendar.SetPosPixel( Point() );
    aCalendar.SetFirstDate( Date( 1, 1, aCurDate.GetYear() ) );
    aCalendar.Show();

    SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Calendar" ) ) );
    SetOutputSizePixel( aCalendar.CalcWindowSizePixel( 3, 4 ) );
}

// -----------------------------------------------------------------------

MyCalendar::~MyCalendar()
{
    SetMenuBar( NULL );
    aMenuBar.SetPopupMenu( 1, NULL );
    aMenuBar.SetPopupMenu( 2, NULL );
}

// -----------------------------------------------------------------------

IMPL_LINK( MyCalendar, RequestDateInfoHdl, Calendar*, EMPTYARG )
{
    USHORT nRequestYear = aCalendar.GetRequestYear();
    if ( (nRequestYear >= 1954) && (nRequestYear <= 1989) )
        aCalendar.AddDateInfo( Date(  17, 6, nRequestYear ), XubString( RTL_CONSTASCII_USTRINGPARAM( "Tag der deutschen Einheit" ) ), &aHolidayColor, NULL );
    else if ( nRequestYear >=  1990 )
        aCalendar.AddDateInfo( Date(  3, 10, nRequestYear ), XubString( RTL_CONSTASCII_USTRINGPARAM( "Tag der deutschen Einheit" ) ), &aHolidayColor, NULL );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( MyCalendar, DoubleClickHdl, Calendar*, EMPTYARG )
{
    Date aDate = aCalendar.GetCurDate();
    String aStr( RTL_CONSTASCII_USTRINGPARAM( "Info: " ) );
    aStr += Application::GetAppInternational().GetDate( aDate );
    aCalendar.AddDateInfo( aDate, aStr, NULL, &aFrameColor, DIB_BOLD );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( MyCalendar, MenuSelectHdl, Menu*, pMenu )
{
    International   aIntn = aCalendar.GetInternational();
    USHORT          nItemId = pMenu->GetCurItemId();

    if ( (nItemId >= 10) && (nItemId <= 19) )
        aIntn.SetWeekStart( (DayOfWeek)(nItemId-10) );
    else if ( (nItemId >= 20) && (nItemId <= 29) )
        aIntn.SetWeekCountStart( (WeekCountStart)(nItemId-20) );
    aCalendar.SetInternational( aIntn );

    return 0;
}

// -----------------------------------------------------------------------

void MyCalendar::Resize()
{
    aCalendar.SetSizePixel( GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

MyWin::MyWin( Window* pParent, WinBits aWinStyle ) :
    WorkWindow(pParent, aWinStyle | WB_SVLOOK ),
    aBox( this, WB_BORDER | WB_SVLOOK ),
    aBar( this, WB_BORDER | WB_SVLOOK | WB_RIGHT ),
    aHeadBar( this, WB_BORDER | WB_SVLOOK | WB_DRAG | WB_BUTTONSTYLE ),
    aColorList( this ),
    aLineList( this ),
    aValueSet( this, WB_TABSTOP | WB_NAMEFIELD | WB_NONEFIELD | WB_BORDER | WB_ITEMBORDER | WB_VSCROLL /* | WB_FLATVALUESET */ ),
    aCalendarField( this, WB_TABSTOP | WB_SPIN | WB_REPEAT | WB_DROPDOWN | WB_BORDER ),
    aCalendarField2( this, WB_TABSTOP | WB_SPIN | WB_REPEAT | WB_DROPDOWN | WB_BORDER ),
    aTabBar( this, WB_BORDER | WB_MULTISELECT | WB_SCROLL | WB_SIZEABLE | WB_DRAG ),
    aPrgsBar( this ),
    aFontBtn( this ),
    aCalendarBtn( this ),
    aPrnSetupBtn( this ),
    aPrnDlgBtn( this )
{
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFaceColor() ) );

    pCalendar = NULL;
    pMenu = NULL;

    Bitmap aBmp;
    aBox.InsertItem(  1, aBmp );
    aBox.InsertItem(  2, aBmp );
    aBox.InsertItem(  3, aBmp );
    aBox.InsertItem(  4, aBmp );
    aBox.InsertSeparator();
    aBox.InsertItem(  5, aBmp );
    aBox.InsertItem(  6, aBmp );
    aBox.InsertItem(  7, aBmp );
    aBox.InsertItem(  8, aBmp );
    aBox.InsertSpace();
    aBox.InsertItem(  9, aBmp );
    aBox.SetPosPixel( Point( 0, 0 ) );
    aBoxSize = aBox.GetSizePixel();
    aBox.Show();

    aBar.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Ready" ) ) );
    aBar.InsertItem( 1, 35 );
    aBar.InsertItem( 2, 55 );
    aBar.InsertItem( 3, 55 );
    aBar.SetItemText( 1, XubString( RTL_CONSTASCII_USTRINGPARAM( "Text" ) ) );
    aBar.SetItemText( 2, XubString( RTL_CONSTASCII_USTRINGPARAM( "21.01.93" ) ) );
    aBar.SetItemText( 3, XubString( RTL_CONSTASCII_USTRINGPARAM( "12:00:00" ) ) );
    aBar.Show();

    long nY = aBox.GetSizePixel().Height()+10;
    {
    aHeadBar.SetPosPixel( Point( 0, nY ) );
    aHeadBar.InsertItem( 1, XubString( RTL_CONSTASCII_USTRINGPARAM( "Sender" ) ), 150 );
    aHeadBar.InsertItem( 2, XubString( RTL_CONSTASCII_USTRINGPARAM( "Subject" ) ), 150, HIB_CENTER | HIB_VCENTER | HIB_CLICKABLE );
    aHeadBar.InsertItem( 3, XubString( RTL_CONSTASCII_USTRINGPARAM( "Date" ) ), 75 );
    aHeadBar.InsertItem( 4, XubString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ), 60, HIB_RIGHT | HIB_VCENTER | HIB_CLICKABLE );
    aHeadBar.InsertItem( 9999, String(), HEADERBAR_FULLSIZE, HIB_RIGHT | HIB_VCENTER | HIB_FIXEDPOS );
    aHeadBar.SetSelectHdl( LINK( this, MyWin, SelectHdl ) );
    aHeadBar.Show();
    nY += aHeadBar.GetSizePixel().Height() += 10;
    }

    {
    aColorList.SetPosSizePixel( Point( 10, nY ), Size( 130, 180 ) );
    aColorList.SetUpdateMode( FALSE );
    aColorList.InsertEntry( Color( COL_BLACK ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Black" ) ) );
    aColorList.InsertEntry( Color( COL_BLUE ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Blue" ) ) );
    aColorList.InsertEntry( Color( COL_GREEN ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Green" ) ) );
    aColorList.InsertEntry( Color( COL_CYAN ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Cyan" ) ) );
    aColorList.InsertEntry( Color( COL_RED ),           XubString( RTL_CONSTASCII_USTRINGPARAM( "Red" ) ) );
    aColorList.InsertEntry( Color( COL_MAGENTA ),       XubString( RTL_CONSTASCII_USTRINGPARAM( "Magenta" ) ) );
    aColorList.InsertEntry( Color( COL_BROWN ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Brown" ) ) );
    aColorList.InsertEntry( Color( COL_GRAY ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Gray" ) ) );
    aColorList.InsertEntry( Color( COL_LIGHTGRAY ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightGray" ) ) );
    aColorList.InsertEntry( Color( COL_LIGHTBLUE ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightBlue" ) ) );
    aColorList.InsertEntry( Color( COL_LIGHTGREEN ),    XubString( RTL_CONSTASCII_USTRINGPARAM( "LightGreen" ) ) );
    aColorList.InsertEntry( Color( COL_LIGHTCYAN ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightCyan" ) ) );
    aColorList.InsertEntry( Color( COL_LIGHTRED ),      XubString( RTL_CONSTASCII_USTRINGPARAM( "LightRed" ) ) );
    aColorList.InsertEntry( Color( COL_LIGHTMAGENTA ),  XubString( RTL_CONSTASCII_USTRINGPARAM( "LightMagenta" ) ) );
    aColorList.InsertEntry( Color( COL_YELLOW ),        XubString( RTL_CONSTASCII_USTRINGPARAM( "Yellow" ) ) );
    aColorList.InsertEntry( Color( COL_WHITE ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "White" ) ) );
    aColorList.SetUpdateMode( TRUE );
    aColorList.SetSelectHdl( LINK( this, MyWin, SelectHdl ) );
    aColorList.Show();
    }

    {
    aLineList.SetPosSizePixel( Point( 150, nY ), Size( 130, 180 ) );
    aLineList.SetUnit( FUNIT_POINT );
    aLineList.SetSourceUnit( FUNIT_TWIP );
    aLineList.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Hairline" ) ) );
    aLineList.InsertEntry( 1500 );
    aLineList.InsertEntry( 3000 );
    aLineList.InsertEntry( 4500 );
    aLineList.InsertEntry( 6000 );
    aLineList.InsertEntry( 7500 );
    aLineList.InsertEntry( 9000 );
    aLineList.InsertEntry( 1500, 1500, 1500 );
    aLineList.InsertEntry( 3000, 1500, 1500 );
    aLineList.InsertEntry( 4500, 1500, 1500 );
    aLineList.InsertEntry( 3000, 3000, 1500 );
    aLineList.InsertEntry( 4500, 3000, 1500 );
    aLineList.InsertEntry( 4500, 4500, 1500 );
    aLineList.Show();
    }

    {
    aValueSet.SetPosSizePixel( Point( 290, nY ), Size( 130, 180 ) );
    aValueSet.InsertItem(  9, Color( COL_BLACK ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Black" ) ) );
    aValueSet.InsertItem( 10, Color( COL_BLUE ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Blue" ) ) );
    aValueSet.InsertItem( 11, Color( COL_GREEN ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Green" ) ) );
    aValueSet.InsertItem( 12, Color( COL_CYAN ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Cyan" ) ) );
    aValueSet.InsertItem( 13, Color( COL_RED ),           XubString( RTL_CONSTASCII_USTRINGPARAM( "Red" ) ) );
    aValueSet.InsertItem( 14, Color( COL_MAGENTA ),       XubString( RTL_CONSTASCII_USTRINGPARAM( "Magenta" ) ) );
    aValueSet.InsertItem( 15, Color( COL_BROWN ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "Brown" ) ) );
    aValueSet.InsertItem( 16, Color( COL_GRAY ),          XubString( RTL_CONSTASCII_USTRINGPARAM( "Gray" ) ) );
    aValueSet.InsertItem( 17, Color( COL_LIGHTGRAY ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightGray" ) ) );
    aValueSet.InsertItem( 18, Color( COL_LIGHTBLUE ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightBlue" ) ) );
    aValueSet.InsertItem( 19, Color( COL_LIGHTGREEN ),    XubString( RTL_CONSTASCII_USTRINGPARAM( "LightGreen" ) ) );
    aValueSet.InsertItem( 20, Color( COL_LIGHTCYAN ),     XubString( RTL_CONSTASCII_USTRINGPARAM( "LightCyan" ) ) );
    aValueSet.InsertItem( 21, Color( COL_LIGHTRED ),      XubString( RTL_CONSTASCII_USTRINGPARAM( "LightRed" ) ) );
    aValueSet.InsertItem( 22, Color( COL_LIGHTMAGENTA ),  XubString( RTL_CONSTASCII_USTRINGPARAM( "LightMagenta" ) ) );
    aValueSet.InsertItem( 23, Color( COL_YELLOW ),        XubString( RTL_CONSTASCII_USTRINGPARAM( "Yellow" ) ) );
    aValueSet.InsertItem( 24, Color( COL_WHITE ),         XubString( RTL_CONSTASCII_USTRINGPARAM( "White" ) ) );
    aValueSet.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
    aValueSet.SetColCount( 4 );
    aValueSet.SetLineCount( 4 );
    aValueSet.SetSelectHdl( LINK( this, MyWin, SelectHdl ) );
    aValueSet.Show();
    }

    {
    aCalendarField.EnableEmptyFieldValue( TRUE );
    aCalendarField.SetCalendarStyle( aCalendarField.GetCalendarStyle() | WB_RANGESELECT );
    aCalendarField.SetSelectHdl( LINK( this, MyWin, CalSelectHdl ) );
//    aCalendarField.SetDate( Date() );
    aCalendarField.SetEmptyDate();
    aCalendarField.EnableToday();
    aCalendarField.EnableNone();
    aCalendarField.SetPosSizePixel( Point( 430, nY ), Size( 130, 20 ) );
    aCalendarField.Show();
    }

    {
    aCalendarField2.SetDate( Date() );
    aCalendarField2.SetPosSizePixel( Point( 570, nY ), Size( 130, 20 ) );
    aCalendarField2.Show();
    }

    nY += 200;
    {
    aTabBar.SetPosSizePixel( Point( 10, nY ),
                             Size( 300, aTabBar.GetSizePixel().Height() ) );
    aTabBar.InsertPage(  1, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 1" ) ) );
    aTabBar.InsertPage(  2, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 2" ) ) );
    aTabBar.InsertPage(  3, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 3" ) ) );
    aTabBar.InsertPage(  4, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 4" ) ) );
    aTabBar.InsertPage(  5, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 5" ) ) );
    aTabBar.InsertPage(  6, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 6" ) ) );
    aTabBar.InsertPage(  7, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 7" ) ) );
    aTabBar.InsertPage(  8, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 8" ) ) );
    aTabBar.InsertPage(  9, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 9" ) ) );
    aTabBar.InsertPage( 10, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 10" ) ) );
    aTabBar.InsertPage( 11, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 11" ) ) );
    aTabBar.InsertPage( 12, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 12" ) ) );
    aTabBar.InsertPage( 13, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 13" ) ) );
    aTabBar.InsertPage( 14, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 14" ) ) );
    aTabBar.InsertPage( 15, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 15" ) ) );
    aTabBar.InsertPage( 16, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 16" ) ) );
    aTabBar.InsertPage( 17, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 17" ) ) );
    aTabBar.InsertPage( 18, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 18" ) ) );
    aTabBar.InsertPage( 19, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 19" ) ) );
    aTabBar.InsertPage( 20, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 20" ) ) );
    aTabBar.InsertPage( 21, XubString( RTL_CONSTASCII_USTRINGPARAM( "This is a long Page Text" ) ) );
    aTabBar.InsertPage( 22, XubString( RTL_CONSTASCII_USTRINGPARAM( "Short Text" ) ) );
    aTabBar.InsertPage( 23, XubString( RTL_CONSTASCII_USTRINGPARAM( "And now a very very long Page Text" ) ) );
    aTabBar.InsertPage( 24, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 24" ) ) );
    aTabBar.InsertPage( 25, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 25" ) ) );
    aTabBar.InsertPage( 26, XubString( RTL_CONSTASCII_USTRINGPARAM( "And now a very long Page Text" ) ) );
    aTabBar.InsertPage( 27, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 27" ) ) );
    aTabBar.InsertPage( 28, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 28" ) ) );
    aTabBar.InsertPage( 29, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 29" ) ) );
    aTabBar.InsertPage( 30, XubString( RTL_CONSTASCII_USTRINGPARAM( "Page 30" ) ) );
    aTabBar.EnableEditMode();
    aTabBar.Show();
    }

    nY += 35;
    {
    aPrgsBar.SetPosPixel( Point( 10, nY ) );
    aPrgsBar.Show();
    }

    nY += 40;
    {
    aFontBtn.SetPosSizePixel( Point( 10, nY ), Size( 100, 30 ) );
    aFontBtn.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Font..." ) ) );
    aFontBtn.SetClickHdl( LINK( this, MyWin, Test ) );
    aFontBtn.Show();

    aCalendarBtn.SetPosSizePixel( Point( 120, nY ), Size( 100, 30 ) );
    aCalendarBtn.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Calendar" ) ) );
    aCalendarBtn.SetClickHdl( LINK( this, MyWin, Test ) );
    aCalendarBtn.Show();

    aPrnSetupBtn.SetPosSizePixel( Point( 230, nY ), Size( 100, 30 ) );
    aPrnSetupBtn.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "PrnSetup..." ) ) );
    aPrnSetupBtn.SetClickHdl( LINK( this, MyWin, Test ) );
    aPrnSetupBtn.Show();

    aPrnDlgBtn.SetPosSizePixel( Point( 340, nY ), Size( 100, 30 ) );
    aPrnDlgBtn.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Print...." ) ) );
    aPrnDlgBtn.SetClickHdl( LINK( this, MyWin, Test ) );
    aPrnDlgBtn.Show();
    }
}

// -----------------------------------------------------------------------

MyWin::~MyWin()
{
    if ( pCalendar )
        delete pCalendar;

    if ( pMenu )
    {
        delete pMenu;
        delete pNameMenu;
        delete pStyleMenu;
        delete pSizeMenu;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( MyWin, Test, PushButton*, pBtn )
{
    if ( pBtn == &aFontBtn )
    {
        MyFontDialog* pDlg = new MyFontDialog( this );
        pDlg->Execute();
        delete pDlg;
    }
    else if ( pBtn == &aCalendarBtn )
    {
        if ( !pCalendar )
            pCalendar = new MyCalendar( this );
        pCalendar->ToTop();
        pCalendar->Show();
    }
    else if ( pBtn == &aPrnSetupBtn )
    {
        PrinterSetupDialog* pDlg = new PrinterSetupDialog( this );
        pDlg->SetPrinter( &aPrn );
        pDlg->Execute();
        delete pDlg;
    }
    else if ( pBtn == &aPrnDlgBtn )
    {
        PrintDialog* pDlg = new PrintDialog( this );
        pDlg->SetPrinter( &aPrn );
        pDlg->EnableRange( PRINTDIALOG_ALL );
        pDlg->EnableRange( PRINTDIALOG_RANGE );
        pDlg->Execute();
        delete pDlg;
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( MyWin, SelectHdl, Window*, pCtrl )
{
    if ( pCtrl == &aColorList )
    {
        Color aColor = aColorList.GetSelectEntryColor();
        aValueSet.SetColor( aColor );
        aLineList.SetColor( aColor );
    }
    else if ( pCtrl == &aValueSet )
    {
        USHORT nId = aValueSet.GetSelectItemId();
        if ( nId > 8 )
        {
            Color aColor = aValueSet.GetItemColor( nId );
            aValueSet.SetFillColor( aColor );
        }
    }
    else if ( pCtrl == &aHeadBar )
    {
        USHORT nCurItemId = aHeadBar.GetCurItemId();
        for ( USHORT i = 0; i < aHeadBar.GetItemCount(); i++ )
        {
            USHORT nItemId = aHeadBar.GetItemId( i );
            HeaderBarItemBits nBits = aHeadBar.GetItemBits( nItemId );
            if ( nItemId == nCurItemId )
            {
                HeaderBarItemBits nOldBits = nBits;
                nBits &= ~(HIB_DOWNARROW | HIB_UPARROW);
                if ( nOldBits & HIB_DOWNARROW )
                    nBits |= HIB_UPARROW;
                else
                    nBits |= HIB_DOWNARROW;
            }
            else
                nBits &= ~(HIB_DOWNARROW | HIB_UPARROW);
            aHeadBar.SetItemBits( nItemId, nBits );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( MyWin, CalSelectHdl, CalendarField*, pCtrl )
{
    if ( pCtrl == &aCalendarField )
    {
        Calendar* pCalendar = pCtrl->GetCalendar();
        aCalendarField2.SetDate( pCalendar->GetSelectDate( pCalendar->GetSelectDateCount()-1 ) );
    }

    return 0;
}

// -----------------------------------------------------------------------

void MyWin::ContextMenu( const Point& rPos )
{
    FontList aList( this );

    if ( !pMenu )
    {
        pMenu       = new PopupMenu;
        pNameMenu   = new FontNameMenu;
        pStyleMenu  = new FontStyleMenu;
        pSizeMenu   = new FontSizeMenu;

        pMenu->InsertItem( 1, XubString( RTL_CONSTASCII_USTRINGPARAM( "Font" ) ) );
        pMenu->InsertItem( 2, XubString( RTL_CONSTASCII_USTRINGPARAM( "Attribute" ) ) );
        pMenu->InsertItem( 3, XubString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) );
        pMenu->SetPopupMenu( 1, pNameMenu );
        pMenu->SetPopupMenu( 2, pStyleMenu );
        pMenu->SetPopupMenu( 3, pSizeMenu );

        pNameMenu->Fill( &aList );
        pNameMenu->SetCurName( aList.GetFontName( 0 ).GetName() );

        pStyleMenu->InsertSeparator();
        pStyleMenu->InsertItem( 1, XubString( RTL_CONSTASCII_USTRINGPARAM( "~Underline" ) ), MIB_CHECKABLE | MIB_AUTOCHECK );
        pStyleMenu->InsertItem( 2, XubString( RTL_CONSTASCII_USTRINGPARAM( "Stri~keout" ) ), MIB_CHECKABLE | MIB_AUTOCHECK );
        pStyleMenu->InsertItem( 3, XubString( RTL_CONSTASCII_USTRINGPARAM( "~Shadow" ) ), MIB_CHECKABLE | MIB_AUTOCHECK );
        pStyleMenu->InsertItem( 4, XubString( RTL_CONSTASCII_USTRINGPARAM( "~Outline" ) ), MIB_CHECKABLE | MIB_AUTOCHECK );
    }

    pStyleMenu->Fill( pNameMenu->GetCurName(), &aList );
    pSizeMenu->Fill( aList.Get( pNameMenu->GetCurName(),
                                pStyleMenu->GetCurStyle() ), &aList );

    pMenu->Execute( this, rPos );
}

// -----------------------------------------------------------------------

void MyWin::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
        ContextMenu( OutputToScreenPixel( rCEvt.GetMousePosPixel() ) );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    aValueSet.StartSelection();
    WorkWindow::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    if ( rKEvt.GetKeyCode().GetCode() == KEY_P )
    {
        for ( USHORT i = 0; i <= 130; i += 2 )
        {
            for ( USHORT j = 0; j < 6000; j++ )
            {
                aPrgsBar.SetValue( i );
                Application::Reschedule();
            }
        }
    }
    else if ( rKEvt.GetCharCode() == '+' )
        aHeadBar.SetOffset( aHeadBar.GetOffset()+1 );
    else if ( rKEvt.GetCharCode() == '-' )
        aHeadBar.SetOffset( aHeadBar.GetOffset()-1 );

    WorkWindow::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void MyWin::Paint( const Rectangle& rRect )
{
    WorkWindow::Paint( rRect );
}

// -----------------------------------------------------------------------

void MyWin::Resize()
{
    Size aWinSize = GetOutputSizePixel();

    aBox.SetSizePixel( Size( aWinSize.Width(), aBoxSize.Height() ) );

    Size aSize = aBar.GetSizePixel();
    aBar.SetPosSizePixel( Point( 0, aWinSize.Height()-aSize.Height() ),
                          Size( aWinSize.Width(), aSize.Height() ) );

    Size aBarSize = aSize;
    Point aPos = aHeadBar.GetPosPixel();
    aSize = aHeadBar.GetSizePixel();
    aHeadBar.SetSizePixel( Size( aWinSize.Width(), aSize.Height() ) );
    aHeadBar.SetDragSize( aWinSize.Height() - aSize.Height() - aPos.Y() - aBarSize.Height() );

    aPos = aPrgsBar.GetPosPixel();
    aSize = aPrgsBar.GetSizePixel();
    if ( aPos.X() < aWinSize.Width()-10 )
        aPrgsBar.SetSizePixel( Size( aWinSize.Width()-aPos.X()-10, aSize.Height() ) );
}

// -----------------------------------------------------------------------

MyApp aMyApp;
