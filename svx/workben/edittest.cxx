/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <eerdll.hxx>
#include <editeng/flditem.hxx>
#include <svx/svxacorr.hxx>
#include <tools/fsys.hxx>
#include <svx/xpoly.hxx>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/font.hxx>
#include <vcl/print.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/help.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>

#include <dialdll.hxx>

#define SERVICE_SIMPLEREGISTRY             "com.sun.star.registry.SimpleRegistry"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <sfx2/sfxuno.hxx>

// SVX
#include <editeng/lrspitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <fwdtitem.hxx>

// SVTOOLS
#include <svl/undo.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svtools/printdlg.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <vector>

#define TB_LEFT          1
#define TB_RIGHT         2
#define TB_CENTER        3
#define TB_UNDO          4
#define TB_REDO          5
#define TB_FONT1         6
#define TB_FONT2         7
#define TB_ITALIC        8
#define TB_BOLD          9
#define TB_UNDERLINE    10
#define TB_BLACK        11
#define TB_GREEN        12
#define TB_OPEN         13
#define TB_SAVE         14
#define TB_SBL          15
#define TB_SBSA         16
#define TB_LR           17
#define TB_DRAW         18
#define TB_DEFTAB       19
#define TB_OPEN2        20
#define TB_SAVE2        21
#define TB_STDSEL       33
#define TB_MOVE         34
#define TB_PARATTR1     35
#define TB_ROTATE       38
#define TB_RED          43
#define TB_FLAT         46
#define TB_BINOBJ1      47
#define TB_BINOBJ3      49
#define TB_BINOBJ4      50
#define TB_BINOBJ1b     51
#define TB_BINOBJ2b     52
#define TB_ATTRIBS      54
#define TB_IDLE         55
#define TB_BLOCK        56
#define TB_CLONEBIN     57
#define TB_INSERT       58
#define TB_PKERN        59
#define TB_KERN         60
#define TB_SUPER        61
#define TB_SUB          62
#define TB_PRINT        63
#define TB_FONT         64
#define TB_COLORS       65
#define TB_WLM          66
#define TB_OUTL         67
#define TB_INSFLD       68
#define TB_UPDFLD       69
#define TB_ONLINESPELL  70
#define TB_REDLINES     71
#define TB_AUTOCORRECT  72
#define TB_POLY         73
#define TB_HYPH         74

// VARS...
short nRotation = 0;
USHORT nZoom = 100;
sal_Bool bURLClicked = sal_False;

using namespace ::com::sun::star::connection;
using namespace ::vos;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::lang;



Reference< XMultiServiceFactory > createApplicationServiceManager()
{
    Reference< XMultiServiceFactory > xMS;
    try
    {
        Reference< XComponentContext >    xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
        if ( xComponentContext.is() )
            xMS = xMS.query( xComponentContext->getServiceManager() );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return xMS;
}

// --- class EditApp -------------------------------------------------

class StringList : public std::vector<String*>
{
public:
    ~StringList()
    {
        for( const_iterator it = begin(); it != end(); ++it )
            delete *it;
    }
};

StringList aSimpleHistory;

class EditApp : public Application
{
public:
    virtual void Main();
};

class MyEditEngine : public EditEngine
{
public:
    MyEditEngine( SfxItemPool* pPool ) : EditEngine( pPool ) { ; }
    virtual String  CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor );
    virtual void    FieldClicked( const SvxFieldItem& rField, USHORT nPara, USHORT nPos );
    virtual void    FieldSelected( const SvxFieldItem& rField, USHORT nPara, USHORT nPos );
};

XubString __EXPORT MyEditEngine::CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    const SvxFieldData* pField = rField.GetField();
    if ( !pField )
        return String( RTL_CONSTASCII_USTRINGPARAM( "<Empty>" ) );

    if ( pField->ISA( SvxDateField ) )
        return ((const SvxDateField*)pField)->GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM );
    else if ( pField->ISA( SvxURLField ) )
    {
        const SvxURLField* pURL = (const SvxURLField*)pField;
        if ( !bURLClicked )
        {
            delete rpTxtColor;
            rpTxtColor = new Color( COL_BLUE );
        }
        else
        {
            delete rpTxtColor;
            rpTxtColor = new Color( COL_RED );
            delete rpFldColor;
            rpFldColor = new Color( COL_YELLOW );
        }
        if ( pURL->GetFormat() == SVXURLFORMAT_URL )
            return pURL->GetURL();
        return pURL->GetRepresentation();
    }
    return String( RTL_CONSTASCII_USTRINGPARAM( "???" ) );
}

void __EXPORT MyEditEngine::FieldClicked( const SvxFieldItem& rField, USHORT nPara, USHORT nPos )
{
    EditEngine::FieldClicked( rField, nPara, nPos );    // Falls URL
    const SvxFieldData* pField = rField.GetField();
    if ( !pField )
        return;

    if ( pField->ISA( SvxURLField ) )
    {
        bURLClicked = TRUE;
        UpdateFields();
    }
}

void __EXPORT MyEditEngine::FieldSelected( const SvxFieldItem& rField, USHORT nPara, USHORT nPos )
{
    const SvxFieldData* pField = rField.GetField();
    if ( !pField )
        return;

    InfoBox( 0, String( RTL_CONSTASCII_USTRINGPARAM( "Feld selektiert!" ) ) ).Execute();
}


class MyView : public WorkWindow
{
private:
    EditEngine*     pEditEngine;
    EditView*       pEditView;

public:
                    MyView( Window* pParent, EditEngine* pEditEngine );
                    ~MyView();

    virtual void    Paint( const Rectangle& );
    virtual void    Resize();
    virtual void    KeyInput( const KeyEvent& rKeyEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    BOOL            Drop( const DropEvent& rEvt );
    BOOL            QueryDrop( DropEvent& rEvt );
};

MyView::MyView( Window* pParent, EditEngine* pEE ) : WorkWindow( pParent, WinBits( WB_STDWORK ) )
{
    pEditEngine = pEE;
    pEditView = new EditView( pEditEngine, this );
    pEditEngine->InsertView( pEditView );
    SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Another View..." ) ) );
//  EnableDrop();
    SetBackgroundBrush( Brush( Color( COL_LIGHTBLUE ) ) );
    Show();
}

MyView::~MyView()
{
    pEditEngine->RemoveView( pEditView );
    delete pEditView;
}

void __EXPORT MyView::Paint( const Rectangle& rRec )
{
    pEditView->Paint( rRec );
}

void __EXPORT MyView::Resize()
{
    Size aPaperSz( GetOutputSize() );
//  aPaperSz.Width() /= 2;
//  aPaperSz.Height() /= 2;
    pEditView->SetOutputArea( Rectangle( Point( 0,0 ), aPaperSz ) );
    pEditView->SetVisArea( Rectangle( Point( 0,0 ), aPaperSz ) );
    Invalidate();
    pEditView->ShowCursor();
}

// --- MyView::KeyInput() --------------------------------------
void __EXPORT MyView::KeyInput( const KeyEvent& rKEvt )
{
    pEditView->PostKeyEvent( rKEvt );
}

void __EXPORT MyView::MouseMove( const MouseEvent& rMEvt )
{
    Point aPos = PixelToLogic( rMEvt.GetPosPixel() );
    if ( pEditView->GetOutputArea().IsInside( aPos ) )
        SetPointer( pEditView->GetPointer() );
    else
        SetPointer( Pointer() );


    pEditView->MouseMove( rMEvt );
}

void __EXPORT MyView::MouseButtonDown( const MouseEvent& rMEvt )
{
    pEditView->MouseButtonDown( rMEvt );
}

void __EXPORT MyView::MouseButtonUp( const MouseEvent& rMEvt )
{
    pEditView->MouseButtonUp( rMEvt );
}

void __EXPORT MyView::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_STARTDRAG )
        pEditView->Command(rCEvt);
}

BOOL __EXPORT MyView::Drop( const DropEvent& rEvt )
{
    return pEditView->Drop( rEvt );
}

BOOL __EXPORT MyView::QueryDrop( DropEvent& rEvt )
{
    return pEditView->QueryDrop( rEvt );
}

// --- class EditMainWindow --------------------------------------------

class EditViewWindow : public Window
{
private:
    MyEditEngine*           pEditEngine;
    EditView*               pEditView;
    Pointer                 aStdPtr;
    Pointer                 aURLPtr;
    WorkWindow*             pTmpWindow;

    XubString               aTestStr;


public:
                    EditViewWindow( Window* pParent );
                    ~EditViewWindow();

    virtual void    Paint( const Rectangle& );
    virtual void    Resize();
    virtual void    KeyInput( const KeyEvent& rKeyEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    void            MarkOutputArea();
    BOOL            Drop( const DropEvent& rEvt );
    BOOL            QueryDrop( DropEvent& rEvt );

    EditView*       GetEditView()       { return pEditView; }
};

class EditMainWindow : public WorkWindow
{
private:
    ToolBox                 aToolBox;
    ScrollBar               aHScrollBar;
    ScrollBar               aVScrollBar;
    EditViewWindow          aViewWin;
    Printer*                pPrinter;

    WorkWindow*             pTmpWindow;

    EditTextObject*         pRTFObj;
    EditTextObject*         pBinObj;

    FileDialog*             pFileDialogBox;
    FileDialog*             pFileDialogBox2;


protected:
    void            SetScrollBars();
    void            SetScrollBarRanges();
    void            CreatePolygon();

    virtual void    GetFocus();

public:
                    EditMainWindow();
                    ~EditMainWindow();

    virtual void    Resize();

    DECL_LINK( TBSelect, ToolBox * );
    void            UpdateToolBox();

    DECL_LINK( HScrollHdl, ScrollBar * );
    DECL_LINK( VScrollHdl, ScrollBar * );
    DECL_LINK( ShowStatus, EditStatus * );
    void            SetTitle();
};


EditMainWindow::~EditMainWindow()
{
    delete pRTFObj;
    delete pBinObj;
    delete pFileDialogBox;
    delete pFileDialogBox2;
    delete pTmpWindow;
    delete pPrinter;
}

EditMainWindow::EditMainWindow() :
                WorkWindow( NULL, WB_APP | WB_STDWORK | WB_SVLOOK | WB_CLIPCHILDREN ) ,
                aViewWin( this ),
                aToolBox( this, WinBits( WB_BORDER | WB_SVLOOK | WB_SCROLL | WB_LINESPACING ) ),
                aHScrollBar( this, WinBits( WB_HSCROLL | WB_DRAG ) ),
                aVScrollBar( this, WinBits( WB_VSCROLL | WB_DRAG ) )
{
    pRTFObj = 0;
    pBinObj = 0;

    pFileDialogBox = new FileDialog( this, WinBits( WB_OPEN | WB_SVLOOK ) );
    pFileDialogBox->SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Open" ) ) );
    pFileDialogBox->AddFilter( String( RTL_CONSTASCII_USTRINGPARAM( "Text" )), String( RTL_CONSTASCII_USTRINGPARAM( "*.txt" ) ) );
    pFileDialogBox->AddFilter( String( RTL_CONSTASCII_USTRINGPARAM( "Rich Text Format" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.rtf" ) ) );
    pFileDialogBox->AddFilter( String( RTL_CONSTASCII_USTRINGPARAM( "HTML" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.htm;*.html" ) ) );
    pFileDialogBox->SetCurFilter( String( RTL_CONSTASCII_USTRINGPARAM( "Rich Text Format" ) ) );

    pFileDialogBox2 = new FileDialog( this, WinBits( WB_SAVEAS| WB_SVLOOK ) );
    pFileDialogBox2->SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Save" ) ) );
    pFileDialogBox2->AddFilter( String( RTL_CONSTASCII_USTRINGPARAM( "Text" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.txt" ) ) );
    pFileDialogBox2->AddFilter( String( RTL_CONSTASCII_USTRINGPARAM( "Rich Text Format" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.rtf" ) ) );
    pFileDialogBox2->AddFilter( String( RTL_CONSTASCII_USTRINGPARAM( "HTML" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.htm" ) ) );
    pFileDialogBox2->SetCurFilter( String( RTL_CONSTASCII_USTRINGPARAM( "Rich Text Format" ) ) );

    pPrinter = new Printer;
    pPrinter->SetMapMode( MAP_100TH_MM );
    aViewWin.SetMapMode( pPrinter->GetMapMode() );
    aViewWin.GetEditView()->GetEditEngine()->SetRefDevice( pPrinter );



    pTmpWindow = new WorkWindow( this, WB_STDWORK );

    aToolBox.SetButtonType( BUTTON_TEXT );
    aToolBox.SetLineCount( 4 );

    {
    aToolBox.InsertItem( TB_OPEN, String( RTL_CONSTASCII_USTRINGPARAM( "Open" ) ) );
    aToolBox.InsertItem( TB_SAVE, String( RTL_CONSTASCII_USTRINGPARAM( "Save" ) ) );
    aToolBox.InsertItem( TB_PRINT, String( RTL_CONSTASCII_USTRINGPARAM( "Print" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_LEFT, String( RTL_CONSTASCII_USTRINGPARAM( "L" ) ) );
    aToolBox.InsertItem( TB_CENTER, String( RTL_CONSTASCII_USTRINGPARAM( "C" ) ) );
    aToolBox.InsertItem( TB_RIGHT, String( RTL_CONSTASCII_USTRINGPARAM( "R" ) ) );
    aToolBox.InsertItem( TB_BLOCK, String( RTL_CONSTASCII_USTRINGPARAM( "B" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_UNDO, String( RTL_CONSTASCII_USTRINGPARAM( "Undo" ) ) );
    aToolBox.InsertItem( TB_REDO, String( RTL_CONSTASCII_USTRINGPARAM( "Redo" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_FONT1, String( RTL_CONSTASCII_USTRINGPARAM( "Font1" ) ) );
    aToolBox.InsertItem( TB_FONT2, String( RTL_CONSTASCII_USTRINGPARAM( "Font2" ) ) );
    aToolBox.InsertItem( TB_DEFTAB, String( RTL_CONSTASCII_USTRINGPARAM( "DefTab" ) ) );
    aToolBox.InsertBreak();
    aToolBox.InsertItem( TB_OPEN2, String( RTL_CONSTASCII_USTRINGPARAM( "Read" ) ) );
    aToolBox.InsertItem( TB_SAVE2, String( RTL_CONSTASCII_USTRINGPARAM( "Write" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_FONT, String( RTL_CONSTASCII_USTRINGPARAM( "XXX" ) ) );
    aToolBox.InsertItem( TB_ITALIC, String( RTL_CONSTASCII_USTRINGPARAM( "K" ) ) );
    aToolBox.InsertItem( TB_BOLD, String( RTL_CONSTASCII_USTRINGPARAM( "F" ) ) );
    aToolBox.InsertItem( TB_UNDERLINE, String( RTL_CONSTASCII_USTRINGPARAM( "U" ) ) );
    aToolBox.InsertItem( TB_SUPER, String( RTL_CONSTASCII_USTRINGPARAM( "SP" ) ) );
    aToolBox.InsertItem( TB_SUB, String( RTL_CONSTASCII_USTRINGPARAM( "SB" ) ) );
    aToolBox.InsertItem( TB_PKERN, String( RTL_CONSTASCII_USTRINGPARAM( "PK" ) ) );
    aToolBox.InsertItem( TB_KERN, String( RTL_CONSTASCII_USTRINGPARAM( "TK" ) ) );
    aToolBox.InsertItem( TB_WLM, String( RTL_CONSTASCII_USTRINGPARAM( "W!" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_BLACK, String( RTL_CONSTASCII_USTRINGPARAM( "Black" ) ) );
    aToolBox.InsertItem( TB_GREEN, String( RTL_CONSTASCII_USTRINGPARAM( "Green" ) ) );
    aToolBox.InsertItem( TB_RED, String( RTL_CONSTASCII_USTRINGPARAM( "Red" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_SBL, String( RTL_CONSTASCII_USTRINGPARAM( "SBL" ) ) );
    aToolBox.InsertItem( TB_SBSA, String( RTL_CONSTASCII_USTRINGPARAM( "SBSA" ) ) );
    aToolBox.InsertItem( TB_LR, String( RTL_CONSTASCII_USTRINGPARAM( "LR" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_DRAW, String( RTL_CONSTASCII_USTRINGPARAM( "Draw!" ) ) );
    aToolBox.InsertItem( TB_ROTATE, String( RTL_CONSTASCII_USTRINGPARAM( "Rotate!" ) ) );
    aToolBox.InsertItem( TB_MOVE, String( RTL_CONSTASCII_USTRINGPARAM( "Move->2" ) ) );
    aToolBox.InsertItem( TB_PARATTR1, String( RTL_CONSTASCII_USTRINGPARAM( "ParaAttr0" ) ) );
    aToolBox.InsertItem( TB_ATTRIBS, String( RTL_CONSTASCII_USTRINGPARAM( "GetAttribs" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_INSFLD, String( RTL_CONSTASCII_USTRINGPARAM( "InsFld" ) ) );
    aToolBox.InsertItem( TB_UPDFLD, String( RTL_CONSTASCII_USTRINGPARAM( "UpdFld" ) ) );
    }
    aToolBox.InsertBreak();
    {
    aToolBox.InsertItem( TB_ONLINESPELL, String( RTL_CONSTASCII_USTRINGPARAM( "Spell" ) ) );
    aToolBox.InsertItem( TB_REDLINES, String( RTL_CONSTASCII_USTRINGPARAM( "RedLine" ) ) );
    aToolBox.InsertItem( TB_AUTOCORRECT, String( RTL_CONSTASCII_USTRINGPARAM( "Auto*" ) ) );
    aToolBox.InsertItem( TB_HYPH, String( RTL_CONSTASCII_USTRINGPARAM( "Hyph" ) ) );
    aToolBox.InsertItem( TB_STDSEL, String( RTL_CONSTASCII_USTRINGPARAM( "StdSel" ) ) );
    aToolBox.InsertItem( TB_FLAT, String( RTL_CONSTASCII_USTRINGPARAM( "FlatMode" ) ) );
    aToolBox.InsertItem( TB_OUTL, String( RTL_CONSTASCII_USTRINGPARAM( "Ocomphelper" ) ) );
    aToolBox.InsertItem( TB_POLY, String( RTL_CONSTASCII_USTRINGPARAM( "Poly" ) ) );
    aToolBox.InsertItem( TB_COLORS, String( RTL_CONSTASCII_USTRINGPARAM( "Colors" ) ));
    aToolBox.InsertItem( TB_IDLE, String( RTL_CONSTASCII_USTRINGPARAM( "Idle!" ) ) );
    aToolBox.InsertItem( TB_INSERT, String( RTL_CONSTASCII_USTRINGPARAM( "Insert" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_BINOBJ1, String( RTL_CONSTASCII_USTRINGPARAM( ">BINObj" ) ) );
    aToolBox.InsertItem( TB_BINOBJ1b, String( RTL_CONSTASCII_USTRINGPARAM( ">>BINObj" ) ) );
    aToolBox.InsertItem( TB_BINOBJ2b, String( RTL_CONSTASCII_USTRINGPARAM( "<<BINObj" ) ) );
    aToolBox.InsertItem( TB_CLONEBIN, String( RTL_CONSTASCII_USTRINGPARAM( "Clone" ) ) );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_BINOBJ3, String( RTL_CONSTASCII_USTRINGPARAM( "StoreBin" ) ) );
    aToolBox.InsertItem( TB_BINOBJ4, String( RTL_CONSTASCII_USTRINGPARAM( "CreatBin" ) ) );
    }

    aToolBox.SetPosPixel( Point( 0, 0 ) );
    aToolBox.SetSelectHdl( LINK( this, EditMainWindow, TBSelect ) );

    SetBackgroundBrush( Brush( Color( COL_LIGHTGRAY ) ) );

    SetPen( PEN_NULL );
    Show();

    aVScrollBar.SetScrollHdl ( LINK( this, EditMainWindow, VScrollHdl ) );
    aVScrollBar.SetLineSize( 300 );
    aVScrollBar.SetPageSize( 2000 );
    aHScrollBar.SetScrollHdl ( LINK( this, EditMainWindow, HScrollHdl ) );
    aHScrollBar.SetLineSize( 300 );
    aHScrollBar.SetPageSize( 2000 );
    aHScrollBar.Enable();
    aVScrollBar.Enable();

    aViewWin.GetEditView()->GetEditEngine()->SetStatusEventHdl( LINK( this, EditMainWindow, ShowStatus ) );

    SetTitle();

    UpdateToolBox();
    aToolBox.Show();
    aViewWin.Show();
}

void EditMainWindow::SetTitle()
{
    String aHeading( String( RTL_CONSTASCII_USTRINGPARAM( "SvEdit!  -  Zoom " ) ) );
    aHeading += String::CreateFromInt32(nZoom);
    aHeading += '%';
    EditEngine* pEditEngine = aViewWin.GetEditView()->GetEditEngine();
    if ( pEditEngine->GetControlWord() & EE_CNTRL_STRETCHING )
    {
        USHORT nX, nY;
        pEditEngine->GetGlobalCharStretching( nX, nY );
        aHeading += String( RTL_CONSTASCII_USTRINGPARAM( ", Stretch=(" ) );
        aHeading += String::CreateFromInt32(nX);
        aHeading += String( RTL_CONSTASCII_USTRINGPARAM( ", " ) );
        aHeading += String::CreateFromInt32(nY);
        aHeading += ')';
    }
    SetText( aHeading );
}

void EditMainWindow::UpdateToolBox()
{
    EditView* pEditView = aViewWin.GetEditView();
    EditEngine* pEditEngine = pEditView->GetEditEngine();
    SfxItemSet aCurSet( pEditView->GetAttribs() );

    ULONG nControl = pEditEngine->GetControlWord();
    aToolBox.EnableItem( TB_UNDO, (BOOL)pEditEngine->GetUndoManager().GetUndoActionCount() );
    aToolBox.EnableItem( TB_REDO, (BOOL)pEditEngine->GetUndoManager().GetRedoActionCount() );
    aToolBox.EnableItem( TB_BINOBJ2b, (BOOL)(long)pBinObj );
    aToolBox.EnableItem( TB_MOVE, pEditEngine->GetParagraphCount() > 3 );
    aToolBox.CheckItem( TB_ONLINESPELL, nControl & EE_CNTRL_ONLINESPELLING ? TRUE : FALSE );
    aToolBox.CheckItem( TB_AUTOCORRECT, nControl & EE_CNTRL_AUTOCORRECT ? TRUE : FALSE );
//  aToolBox.CheckItem( TB_HYPH, nControl & EE_CNTRL_HYPHENATE ? TRUE : FALSE );
    aToolBox.CheckItem( TB_REDLINES, nControl & EE_CNTRL_NOREDLINES ? FALSE : TRUE );
    aToolBox.CheckItem( TB_STDSEL, pEditView->GetSelectionMode() == EE_SELMODE_STD );
    aToolBox.CheckItem( TB_FLAT, pEditEngine->IsFlatMode() );
    aToolBox.CheckItem( TB_OUTL, ( nControl & EE_CNTRL_OUTLINER ) ? TRUE : FALSE );
    aToolBox.CheckItem( TB_POLY, pEditEngine->GetPolygon() ? TRUE : FALSE );
    aToolBox.CheckItem( TB_COLORS, ( nControl & EE_CNTRL_NOCOLORS ) ? FALSE : TRUE );
    aToolBox.CheckItem( TB_IDLE, pEditEngine->IsIdleFormatterEnabled() );
    aToolBox.CheckItem( TB_INSERT, pEditView->IsInsertMode() );

    for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_ITEMS_END; nWhich++)
    {
//      if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
//          ;
//      else if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_DONTCARE )
//          ;
//      else if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
        {
            const SfxPoolItem& rItem = aCurSet.Get( nWhich );
            switch ( nWhich )
            {
                case EE_PARA_HYPHENATE:
                    aToolBox.CheckItem( TB_HYPH, ((SfxBoolItem&)rItem).GetValue() != 0 );
                break;
                case EE_PARA_LRSPACE:
                    aToolBox.CheckItem( TB_LR, ((SvxLRSpaceItem&)rItem).GetTxtLeft() != 0 );
                break;
                case EE_PARA_ULSPACE:
                    aToolBox.CheckItem( TB_SBSA, ((SvxULSpaceItem&)rItem).GetUpper() != 0 );
                break;
                case EE_PARA_SBL:
                    aToolBox.CheckItem( TB_SBL, ((SvxLineSpacingItem&)rItem).GetPropLineSpace() != 100 );
                break;
                case EE_PARA_JUST:
                {
                    SvxAdjust e = ((SvxAdjustItem&)rItem).GetAdjust();
                    aToolBox.CheckItem( TB_LEFT, e == SVX_ADJUST_LEFT );
                    aToolBox.CheckItem( TB_RIGHT, e == SVX_ADJUST_RIGHT);
                    aToolBox.CheckItem( TB_CENTER, e == SVX_ADJUST_CENTER);
                    aToolBox.CheckItem( TB_BLOCK, e == SVX_ADJUST_BLOCK );
                }
                break;
                case EE_PARA_TABS:
                break;
                case EE_CHAR_COLOR:
                {
                    Color aColor( ((SvxColorItem&)rItem).GetValue() );
                    aToolBox.CheckItem( TB_BLACK, aColor == COL_BLACK );
                    aToolBox.CheckItem( TB_GREEN, aColor == COL_GREEN );
                    aToolBox.CheckItem( TB_RED, aColor == COL_RED );
                }
                break;
                case EE_CHAR_FONTINFO:
                {
                    FontFamily e = ((SvxFontItem&)rItem).GetFamily();
                    aToolBox.CheckItem( TB_FONT1, e == FAMILY_ROMAN );
                    aToolBox.CheckItem( TB_FONT2, e == FAMILY_SWISS );
                }
                break;
                case EE_CHAR_FONTHEIGHT:
                break;
                case EE_CHAR_WEIGHT:
                {
                    FontWeight e = ((SvxWeightItem&)rItem).GetWeight();
                    aToolBox.CheckItem( TB_BOLD, e == WEIGHT_BOLD );
                }
                break;
                case EE_CHAR_UNDERLINE:
                {
                    aToolBox.CheckItem( TB_UNDERLINE, ((SvxUnderlineItem&)rItem).GetLineStyle() );
                }
                break;
                case EE_CHAR_WLM:
                {
                    aToolBox.CheckItem( TB_WLM, ((SvxWordLineModeItem&)rItem).GetValue() );
                }
                break;
                case EE_CHAR_PAIRKERNING:
                {
                    aToolBox.CheckItem( TB_PKERN, ((SvxAutoKernItem&)rItem).GetValue() );
                }
                break;
                case EE_CHAR_KERNING:
                {
                    aToolBox.CheckItem( TB_KERN, (BOOL)((SvxKerningItem&)rItem).GetValue() );
                }
                break;
                case EE_CHAR_ESCAPEMENT:
                {
                    aToolBox.CheckItem( TB_SUPER, ((SvxEscapementItem&)rItem).GetEsc() > 0 );
                    aToolBox.CheckItem( TB_SUB, ((SvxEscapementItem&)rItem).GetEsc() < 0 );
                }
                break;
                case EE_CHAR_STRIKEOUT:
                break;
                case EE_CHAR_ITALIC:
                {
                    FontItalic e = ((SvxPostureItem&)rItem).GetPosture();
                    aToolBox.CheckItem( TB_ITALIC, e == ITALIC_NORMAL );
                }
                break;
                case EE_CHAR_OUTLINE:
                break;
                case EE_CHAR_SHADOW:
                break;
            }
        }
    }
}

IMPL_LINK( EditMainWindow, TBSelect, ToolBox *, p )
{
    EditView* pEditView = aViewWin.GetEditView();
    EditEngine* pEditEngine = pEditView->GetEditEngine();
    USHORT nId = aToolBox.GetCurItemId();
    BOOL bChecked = aToolBox.IsItemChecked( nId );
    SfxPoolItem* pNewItem = 0;
    switch ( nId )
    {
        case TB_LEFT:   pNewItem = new SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST );
                        break;
        case TB_RIGHT:  pNewItem = new SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST );
                        break;
        case TB_CENTER: pNewItem = new SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST );
                        break;
        case TB_BLOCK:  pNewItem = new SvxAdjustItem( SVX_ADJUST_BLOCK, EE_PARA_JUST );
                        break;
        case TB_HYPH:   pNewItem = new SfxBoolItem( EE_PARA_HYPHENATE, !bChecked );
                        break;
        case TB_UNDO:   pEditView->Undo();
                        pEditView->ShowCursor();
                        break;
        case TB_REDO:   pEditView->Redo();
                        pEditView->ShowCursor();
                        break;
        case TB_ONLINESPELL: {
                            ULONG nControl = pEditView->GetEditEngine()->GetControlWord();
                            if ( bChecked )
                                nControl = nControl & ~EE_CNTRL_ONLINESPELLING;
                            else
                                nControl = nControl | EE_CNTRL_ONLINESPELLING;
                            pEditView->GetEditEngine()->SetControlWord( nControl );
                            }
                            break;
        case TB_REDLINES: {
                            ULONG nControl = pEditView->GetEditEngine()->GetControlWord();
                            if ( !bChecked )
                                nControl = nControl & ~EE_CNTRL_NOREDLINES;
                            else
                                nControl = nControl | EE_CNTRL_NOREDLINES;
                            pEditView->GetEditEngine()->SetControlWord( nControl );
                            }
                            break;
        case TB_AUTOCORRECT: {
                            ULONG nControl = pEditView->GetEditEngine()->GetControlWord();
                            if ( bChecked )
                                nControl = nControl & ~EE_CNTRL_AUTOCORRECT;
                            else
                                nControl = nControl | EE_CNTRL_AUTOCORRECT;
                            pEditView->GetEditEngine()->SetControlWord( nControl );
                            }
                            break;
        case TB_STDSEL: if ( bChecked )
                            pEditView->SetSelectionMode( EE_SELMODE_TXTONLY );
                        else
                            pEditView->SetSelectionMode( EE_SELMODE_STD );
                        break;
        case TB_FLAT:   pEditEngine->SetFlatMode( !pEditEngine->IsFlatMode() );
                        break;
        case TB_COLORS: {
                            ULONG nControl = pEditView->GetEditEngine()->GetControlWord();
                            if ( bChecked )
                                nControl = nControl | EE_CNTRL_NOCOLORS;
                            else
                                nControl = nControl & ~EE_CNTRL_NOCOLORS;
                            pEditView->GetEditEngine()->SetControlWord( nControl );
                        }
//                      aViewWin.Invalidate();
                        pEditView->GetEditEngine()->Draw( pEditView->GetWindow(), pEditView->GetOutputArea(), pEditView->GetVisArea().TopLeft() );
                        break;
        case TB_OUTL:   {
                            ULONG nControl = pEditView->GetEditEngine()->GetControlWord();
                            if ( !bChecked )
                                nControl = nControl | EE_CNTRL_OUTLINER;
                            else
                                nControl = nControl & ~EE_CNTRL_OUTLINER;
                            pEditView->GetEditEngine()->SetControlWord( nControl );
                        }
                        break;
        case TB_POLY:   {
                            if ( !bChecked )
                                CreatePolygon();
                            else
                                pEditView->GetEditEngine()->ClearPolygon();
                        }
                        break;
        case TB_IDLE:   pEditEngine->EnableIdleFormatter( !pEditEngine->IsIdleFormatterEnabled() );
                        break;
        case TB_INSFLD: {
                        static BYTE nFld = 0;
                        if ( nFld > 2 )
                            nFld = 0;
                        if ( nFld == 0 )
                        {
                            SvxFieldItem aFld( SvxURLField( String( RTL_CONSTASCII_USTRINGPARAM( "www.mopo.de" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "Bla" ) ), SVXURLFORMAT_REPR ), EE_FEATURE_FIELD  );
                            pEditView->InsertField( aFld );
                        }
                        else if ( nFld == 1 )
                            pEditView->InsertField( SvxFieldItem( SvxURLField( String( RTL_CONSTASCII_USTRINGPARAM( "www.mopo.de" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "Morgenpost" ) ), SVXURLFORMAT_URL ), EE_FEATURE_FIELD  ) );
                        else if ( nFld == 2 )
                            pEditView->InsertField( SvxFieldItem( SvxDateField( Date( 1,1,1995 ), SVXDATETYPE_FIX, SVXDATEFORMAT_D ), EE_FEATURE_FIELD ) );

                        nFld++;
        }
                        break;
        case TB_UPDFLD: pEditEngine->UpdateFields();
                        break;
        case TB_INSERT: pEditView->SetInsertMode( !pEditView->IsInsertMode() );
                        break;
        case TB_FONT1:  pNewItem = new SvxFontItem( FAMILY_ROMAN, String( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) ), String(), PITCH_DONTKNOW, RTL_TEXTENCODING_MS_1252, EE_CHAR_FONTINFO );
                        break;
        case TB_FONT2:  pNewItem = new SvxFontItem( FAMILY_SWISS, String( RTL_CONSTASCII_USTRINGPARAM( "Helv" ) ), String(), PITCH_DONTKNOW, RTL_TEXTENCODING_MS_1252, EE_CHAR_FONTINFO );
                        break;
        case TB_BOLD:   if ( bChecked )
                            pNewItem = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT);
                        else
                            pNewItem = new SvxWeightItem( WEIGHT_BOLD, EE_CHAR_WEIGHT);
                        break;
        case TB_ITALIC: if ( bChecked )
                            pNewItem = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC );
                        else
                            pNewItem = new SvxPostureItem( ITALIC_NORMAL, EE_CHAR_ITALIC );
                        break;
        case TB_UNDERLINE:  if ( bChecked )
                                pNewItem = new SvxUnderlineItem( UNDERLINE_NONE, EE_CHAR_UNDERLINE );
                            else
                                pNewItem = new SvxUnderlineItem( UNDERLINE_SINGLE, EE_CHAR_UNDERLINE );
                        break;
        case TB_WLM:    pNewItem = new SvxWordLineModeItem( !bChecked, EE_CHAR_WLM );
                        break;
        case TB_PKERN:  pNewItem = new SvxAutoKernItem( !bChecked, EE_CHAR_PAIRKERNING );
                        break;
        case TB_KERN:   if ( bChecked )
                                pNewItem = new SvxKerningItem( 0, EE_CHAR_KERNING );
                            else
                                pNewItem = new SvxKerningItem( 100, EE_CHAR_KERNING);
                        break;
        case TB_SUPER:  if ( bChecked )
                                pNewItem = new SvxEscapementItem( SVX_ESCAPEMENT_OFF, EE_CHAR_ESCAPEMENT);
                            else
//                              pNewItem = new SvxEscapementItem( SVX_ESCAPEMENT_SUPERSCRIPT, EE_CHAR_ESCAPEMENT);
                                pNewItem = new SvxEscapementItem( 50, 100, EE_CHAR_ESCAPEMENT );
                        break;
        case TB_SUB:    if ( bChecked )
                                pNewItem = new SvxEscapementItem( SVX_ESCAPEMENT_OFF, EE_CHAR_ESCAPEMENT);
                            else
//                              pNewItem = new SvxEscapementItem( SVX_ESCAPEMENT_SUBSCRIPT, EE_CHAR_ESCAPEMENT);
                                pNewItem = new SvxEscapementItem( -50, 100, EE_CHAR_ESCAPEMENT );
                        break;
        case TB_GREEN:  pNewItem = new SvxColorItem( Color(COL_GREEN), EE_CHAR_COLOR);
                        break;
        case TB_RED:    pNewItem = new SvxColorItem( Color(COL_RED), EE_CHAR_COLOR);
                        break;
        case TB_BLACK:  pNewItem = new SvxColorItem( Color(COL_BLACK), EE_CHAR_COLOR);
                        break;
        case TB_SBL:    pNewItem = new SvxLineSpacingItem( 0, EE_PARA_SBL );
                        if ( bChecked )
                        {
                            ((SvxLineSpacingItem*)pNewItem)->SetInterLineSpace( 0 );
                        }
                        else
                        {
                            ((SvxLineSpacingItem*)pNewItem)->SetPropLineSpace( 150 );
                        }
                        break;
        case TB_SBSA:   pNewItem = new SvxULSpaceItem( EE_PARA_ULSPACE );
                        if ( !bChecked )
                        {
                            ((SvxULSpaceItem*)pNewItem)->SetUpper( 400 );
                            ((SvxULSpaceItem*)pNewItem)->SetLower( 400 );
                        }
                        break;
        case TB_LR:     pNewItem = new SvxLRSpaceItem( EE_PARA_LRSPACE );
                        if ( !bChecked )
                        {
                            ((SvxLRSpaceItem*)pNewItem)->SetTxtLeft( 1000 );
                            ((SvxLRSpaceItem*)pNewItem)->SetTxtFirstLineOfst( -500 );
                            ((SvxLRSpaceItem*)pNewItem)->SetRight( 500 );
                        }
                        break;
        case TB_DEFTAB: if ( bChecked )
                            pEditEngine->SetDefTab( 2000 );
                        else
                            pEditEngine->SetDefTab( 600 );
                        pEditView->ShowCursor();
                        aToolBox.CheckItem( nId, !bChecked );
                        break;
        case TB_DRAW:   {
                            pTmpWindow->SetMapMode( aViewWin.GetMapMode() );
                            pTmpWindow->SetBackgroundBrush( Brush( Color( COL_GRAY ) ) );
                            pTmpWindow->SetFillInBrush( Brush( Color( COL_LIGHTGRAY ) ) );
                            pTmpWindow->Show();
                            pTmpWindow->Invalidate();
                            pTmpWindow->Update();
                            pTmpWindow->DrawText( Point( 20, 20 ), String( RTL_CONSTASCII_USTRINGPARAM( "It's a Test..." ) ) );
                            Rectangle aRect( Point( 500, 500 ), Size( 10000, 4000 ) );
                            pTmpWindow->DrawRect( aRect );
                            pEditEngine->Draw( pTmpWindow, aRect, Point( 0, 0 ) );
                            nRotation = 0;
                        }
                        break;
        case TB_ROTATE:   {
                            nRotation += 100;
                            if ( nRotation == 3600 )
                                nRotation = 0;
                            pTmpWindow->SetMapMode( aViewWin.GetMapMode() );
                            pTmpWindow->SetBackgroundBrush( Brush( Color( COL_LIGHTGRAY ) ) );
                            pTmpWindow->SetFillInBrush( Brush( Color( COL_LIGHTGRAY ) ) );
                            pTmpWindow->Show();
                            pTmpWindow->Invalidate();
                            pTmpWindow->Update();
                            pTmpWindow->DrawText( Point( 20, 20 ), String( RTL_CONSTASCII_USTRINGPARAM( "It's a Test..." ) ) );
                            Rectangle aRect( Point( 500, 500 ), Size( 10000, 4000 ) );
                            pEditEngine->Draw( pTmpWindow, Point( 2000, 4000 ), nRotation );
                        }
                        break;

        case TB_MOVE:
        {
            ESelection aESel = pEditView->GetSelection();
            USHORT nStartPara = Min( aESel.nStartPara, aESel.nEndPara );
            USHORT nEndPara = Max( aESel.nStartPara, aESel.nEndPara );
            pEditView->MoveParagraphs( Range(nStartPara,nEndPara), 2 );
        }
        break;
        case TB_PARATTR1:
        {
            pEditEngine->SetUpdateMode( FALSE );
            USHORT nPara = 0;
            SfxItemSet aSet( pEditEngine->GetEmptyItemSet() );
            aSet.Put( SvxFontItem( FAMILY_DECORATIVE, String( RTL_CONSTASCII_USTRINGPARAM( "StarBats" ) ), String(),PITCH_DONTKNOW, RTL_TEXTENCODING_MS_1252, EE_CHAR_FONTINFO ) );
            aSet.Put( SvxColorItem( Color(COL_MAGENTA), EE_CHAR_COLOR ) );
            aSet.Put( SvxFontHeightItem(600, 100, EE_CHAR_FONTHEIGHT) );
            aSet.Put( SvxPostureItem( ITALIC_NORMAL, EE_CHAR_ITALIC ) );
            aSet.Put( SvxLRSpaceItem( 0, 0, 1000, 0, EE_PARA_LRSPACE ) );
            pEditView->SetParaAttribs( aSet, nPara );
            pEditEngine->SetUpdateMode( TRUE );
            pEditView->ShowCursor();
        }
        break;
        case TB_BINOBJ1:
        {
            delete pBinObj;
            pBinObj = pEditEngine->CreateTextObject();
        }
        break;
        case TB_BINOBJ1b:
        {
            delete pBinObj;
            pBinObj = pEditView->CreateTextObject();
        }
        break;
        case TB_CLONEBIN:
        {
            if ( pBinObj )
            {
                EditTextObject* p = pBinObj->Clone();
                delete pBinObj;
                pBinObj = p;
            }
        }
        break;
        case TB_BINOBJ2b:
        {
            if ( pBinObj )
            {
                pEditView->InsertText( *pBinObj );
            }
        }
        break;
        case TB_BINOBJ3:
        {
            if ( !pFileDialogBox2->Execute() )
                return FALSE;
            DirEntry aDirEntry( pFileDialogBox2->GetPath() );
            SvFileStream aStrm( aDirEntry.GetFull(), STREAM_WRITE | STREAM_TRUNC );
            EditTextObject* pTmpObj = pEditEngine->CreateTextObject();
            pTmpObj->Store( aStrm );
            DBG_ASSERT( !aStrm.GetError(), "Store: Error!" );
            delete pTmpObj;
        }
        break;
        case TB_BINOBJ4:
        {
            if ( !pFileDialogBox->Execute() )
                return FALSE;
            DirEntry aDirEntry( pFileDialogBox->GetPath() );
            SvFileStream aStrm( aDirEntry.GetFull(), STREAM_READ );
            delete pBinObj;
            pBinObj = EditTextObject::Create( aStrm );
//          EditTextObject* pTmpObj = pEditEngine->CreateTextObject( aStrm );
//          pEditEngine->SetText( *pTmpObj );
//          pEditView->ShowCursor();
//          delete pTmpObj;
        }
        break;
        case TB_OPEN:
        case TB_OPEN2:
        {
            if ( !pFileDialogBox->Execute() )
                return FALSE;
            DirEntry aDirEntry( pFileDialogBox->GetPath() );
            aDirEntry.ToAbs();
            String aFileName( aDirEntry.GetFull() );
            SvFileStream aStrm( aFileName, STREAM_READ );
            if ( aStrm.GetError() )
                InfoBox( 0, String( RTL_CONSTASCII_USTRINGPARAM( "StreamError!" ) ) ).Execute();
            Application::EnterWait();
            if ( nId == TB_OPEN )
            {
                if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "rtf" ) )
                    pEditEngine->Read( aStrm, EE_FORMAT_RTF );
                if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "htm" ) )
                    pEditEngine->Read( aStrm, EE_FORMAT_HTML );
                if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "html" ) )
                    pEditEngine->Read( aStrm, EE_FORMAT_HTML );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "bin" ) )
                    pEditEngine->Read( aStrm, EE_FORMAT_BIN );
                else
                    pEditEngine->Read( aStrm, EE_FORMAT_TEXT );

                pEditView->ShowCursor();
            }
            else
            {
                if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "rtf" ) )
                    pEditView->Read( aStrm, EE_FORMAT_RTF );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "bin" ) )
                    pEditView->Read( aStrm, EE_FORMAT_BIN );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "html" ) )
                    pEditView->Read( aStrm, EE_FORMAT_HTML );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "htm" ) )
                    pEditView->Read( aStrm, EE_FORMAT_HTML );
                else
                    pEditView->Read( aStrm, EE_FORMAT_TEXT );
            }

            Application::LeaveWait();

        }
        break;
        case TB_PRINT:
        {
            pPrinter->SetPageQueueSize( 1 );
            PrintDialog aPrnDlg( this );
            aPrnDlg.SetPrinter( pPrinter );
            if ( aPrnDlg.Execute() )
            {
                pPrinter->StartJob( String( RTL_CONSTASCII_USTRINGPARAM( "SvEdit - PrinterTest" ) ) );
                pPrinter->StartPage();
                Size aSz( pEditEngine->CalcTextWidth(), pEditEngine->GetTextHeight() );
                Pen aPen( PEN_SOLID );
                aPen.SetColor( Color( COL_BLACK ) );
                pPrinter->SetPen( aPen );
                Rectangle aR( Point( 2000, 4000 ), aSz );
                pPrinter->DrawRect( aR );
                // So folgt auf IIISi, 75DPI kein Ausdruck!
//              pPrinter->SetClipRegion( Region( aR ) );
                pEditEngine->Draw( pPrinter, Point( 2000, 4000 ), nRotation );
//              pPrinter->SetClipRegion();
                pPrinter->EndPage();
                pPrinter->EndJob();
            }
        }
        break;
        case TB_SAVE:
        case TB_SAVE2:
        {
            if ( !pFileDialogBox2->Execute() )
                return FALSE;
            DirEntry aDirEntry( pFileDialogBox2->GetPath() );
//          DirEntry aDirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "d:\\xxx.rtf" ) ) );
            SvFileStream aStrm( aDirEntry.GetFull(), STREAM_WRITE | STREAM_TRUNC );
            Application::EnterWait();
            if ( nId == TB_SAVE )
            {
                if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "rtf") )
                    pEditEngine->Write( aStrm, EE_FORMAT_RTF );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "bin" ) )
                    pEditEngine->Write( aStrm, EE_FORMAT_BIN );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "html" ) )
                    pEditEngine->Write( aStrm, EE_FORMAT_HTML );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "htm" ) )
                    pEditEngine->Write( aStrm, EE_FORMAT_HTML );
                else
                    pEditEngine->Write( aStrm, EE_FORMAT_TEXT );
            }
            else
            {
                if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "rtf" ) )
                    pEditView->Write( aStrm, EE_FORMAT_RTF );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "bin" ) )
                    pEditView->Write( aStrm, EE_FORMAT_BIN );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "html" ) )
                    pEditView->Write( aStrm, EE_FORMAT_HTML );
                else if ( aDirEntry.GetExtension().EqualsIgnoreCaseAscii( "htm" ) )
                    pEditView->Write( aStrm, EE_FORMAT_HTML );
                else
                    pEditView->Write( aStrm, EE_FORMAT_TEXT );
            }
            Application::LeaveWait();
        }
        break;
        case TB_ATTRIBS:
        {
            SfxItemSet aCurSet = pEditView->GetAttribs();
            XubString aDebStr( String( RTL_CONSTASCII_USTRINGPARAM( "Attribute in Selektion:" ) ) );
            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "\nVorlage:" ) );
            XubString aStyle;
//          pEditView->GetStyleSheet( aStyle, eFam );
            aDebStr += aStyle;
            for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_ITEMS_END; nWhich++)
            {
                aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "\n" ) );
                aDebStr += String::CreateFromInt32( nWhich );
                aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "\t" ) );
                if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
                    aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "---" ) );
                else if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_DONTCARE )
                    aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "XXX" ) );
                else if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
                {
                    const SfxPoolItem& rItem = aCurSet.Get( nWhich );
                    switch ( nWhich )
                    {
                        case EE_PARA_LRSPACE:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "FI=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxLRSpaceItem&)rItem).GetTxtFirstLineOfst() );
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( ", LI=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxLRSpaceItem&)rItem).GetTxtLeft() );
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( ", RI=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxLRSpaceItem&)rItem).GetRight() );
                        break;
                        case EE_PARA_ULSPACE:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "SB=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxULSpaceItem&)rItem).GetUpper() );
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( ", SA=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxULSpaceItem&)rItem).GetLower() );
                        break;
                        case EE_PARA_SBL:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "SBL=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxLineSpacingItem&)rItem).GetInterLineSpace() );
                        break;
                        case EE_PARA_JUST:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "SvxAdust=" ) );
                            aDebStr += String::CreateFromInt32( (USHORT)((SvxAdjustItem&)rItem).GetAdjust() );
                        break;
                        case EE_PARA_TABS:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "Tabs = ?" ) );
                        break;
                        case EE_CHAR_COLOR:
                        {
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "Color= " ) );
                            Color aColor( ((SvxColorItem&)rItem).GetValue() );
                            aDebStr += String::CreateFromInt32( aColor.GetRed() );
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( ", " ) );
                            aDebStr += String::CreateFromInt32( aColor.GetGreen() );
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( ", " ) );
                            aDebStr += String::CreateFromInt32( aColor.GetBlue() );
                        }
                        break;
                        case EE_CHAR_FONTINFO:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "Font=" ) );
                            aDebStr += ((SvxFontItem&)rItem).GetFamilyName();
                        break;
                        case EE_CHAR_FONTHEIGHT:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "Groesse=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxFontHeightItem&)rItem).GetHeight() );
                        break;
                        case EE_CHAR_WEIGHT:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "FontWeight=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxWeightItem&)rItem).GetWeight() );
                        break;
                        case EE_CHAR_UNDERLINE:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "FontUnderline=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxUnderlineItem&)rItem).GetLineStyle() );
                        break;
                        case EE_CHAR_WLM:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "WordLineMode=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxWordLineModeItem&)rItem).GetValue() );
                        break;
                        case EE_CHAR_STRIKEOUT:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "FontStrikeout=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxCrossedOutItem&)rItem).GetStrikeout() );
                        break;
                        case EE_CHAR_ITALIC:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "FontPosture=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxPostureItem&)rItem).GetPosture() );
                        break;
                        case EE_CHAR_OUTLINE:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "FontOutline=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxContourItem&)rItem).GetValue() );
                        break;
                        case EE_CHAR_SHADOW:
                            aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "FontShadowed=" ) );
                            aDebStr += String::CreateFromInt32( ((SvxShadowedItem&)rItem).GetValue() );
                        break;
                    }
                }
                else
                    aDebStr += String( RTL_CONSTASCII_USTRINGPARAM( "?" ) );
            }
            InfoBox( 0, aDebStr ).Execute();
        }
        break;
    }

    if ( pNewItem )
    {
        SfxItemSet aSet = pEditView->GetEmptyItemSet();
        aSet.Put( *pNewItem );
        pEditView->SetAttribs( aSet );
        delete pNewItem;
    }
    UpdateToolBox();
    return 0;
}

void EditMainWindow::CreatePolygon()
{
    EditView* pEditView = aViewWin.GetEditView();
    Size aSz = pEditView->GetWindow()->GetOutputSize();
    Point aOffset( aSz.Width()/8, aSz.Height()/8 );
    aSz.Width() *= 3;
    aSz.Width() /= 4;
    aSz.Height() *= 3;
    aSz.Height() /= 4;
    Polygon aPoly( 5 );
    aPoly.SetPoint( Point( aSz.Width()/2, 0 ), 0 );
    aPoly.SetPoint( Point( aSz.Width(), aSz.Height()/3 ), 1 );
    aPoly.SetPoint( Point( aSz.Width()/2, aSz.Height() ), 2 );
    aPoly.SetPoint( Point( 0, aSz.Height()/2 ), 3 );
    aPoly.SetPoint( Point( aSz.Width()/2, 0 ), 4 );
    PolyPolygon aPPoly( aPoly );
    pEditView->GetEditEngine()->SetPolygon( aPPoly );
    pEditView->SetOutputArea( Rectangle( aOffset, aSz ) );
    ULONG nWord = pEditView->GetControlWord();
    nWord &= ~(EV_CNTRL_AUTOSCROLL);
    pEditView->SetControlWord( nWord );
    aViewWin.Invalidate();
}

void __EXPORT EditMainWindow::GetFocus()
{
    aViewWin.GrabFocus();
}

IMPL_LINK_INLINE_START( EditMainWindow, HScrollHdl, ScrollBar *, pScrl )
{
    EditView* pEditView = aViewWin.GetEditView();
    pEditView->Scroll( -pScrl->GetDelta(), 0, RGCHK_PAPERSZ1 );
    return 0;
}
IMPL_LINK_INLINE_END( EditMainWindow, HScrollHdl, ScrollBar *, pScrl )

IMPL_LINK_INLINE_START( EditMainWindow, VScrollHdl, ScrollBar *, pScrl )
{
    EditView* pEditView = aViewWin.GetEditView();
    pEditView->Scroll( 0, -pScrl->GetDelta(), RGCHK_PAPERSZ1 );
    return 0;
}
IMPL_LINK_INLINE_END( EditMainWindow, VScrollHdl, ScrollBar *, pScrl )

void EditMainWindow::SetScrollBarRanges()
{
    EditView* pEditView = aViewWin.GetEditView();
    long y = pEditView->GetEditEngine()->GetTextHeight();
    long x = pEditView->GetEditEngine()->GetPaperSize().Width();

    aHScrollBar.SetRange( Range( 0, x ) );
    aVScrollBar.SetRange( Range( 0, y ) );
}

void EditMainWindow::SetScrollBars()
{
    EditView* pEditView = aViewWin.GetEditView();
    Size aSz = GetOutputSizePixel();
    Size aRealSz( aSz );
    long nScrollbarWidthPixel = aVScrollBar.GetSizePixel().Width();
    long nTBHeight = aToolBox.GetSizePixel().Height();
    aSz.Height() -= nTBHeight;
    Size aOrgSz( aSz );

    // VScroll...
    aSz.Height() += 2;
    Point aPoint( ( aSz.Width()- nScrollbarWidthPixel+1 ), -1+nTBHeight);
    aSz.Width() = nScrollbarWidthPixel;
    aVScrollBar.SetPosSizePixel( aPoint, aSz );

    // HScroll...
    aSz = aOrgSz;
    Point aPoint2( 0, aRealSz.Height()-aHScrollBar.GetSizePixel().Height()+1 ); // TB-Height schon drin!
    aSz.Width() -= aVScrollBar.GetSizePixel().Width();
    aSz.Width() += 2;

    aSz.Height() = nScrollbarWidthPixel;
    aHScrollBar.SetPosSizePixel( aPoint2, aSz );

    aHScrollBar.SetVisibleSize( pEditView->GetOutputArea().GetWidth() );
    aVScrollBar.SetVisibleSize( pEditView->GetOutputArea().GetHeight() );

    SetScrollBarRanges();

    aVScrollBar.Show();
    aHScrollBar.Show();
}

void __EXPORT EditMainWindow::Resize()
{
    long nBorder = aVScrollBar.GetSizePixel().Width();
    long nExtra = 10;
    Size aTBSz = aToolBox.CalcWindowSizePixel();
    Size aOutSzPixel( GetOutputSizePixel() );
    aToolBox.SetSizePixel( Size( aOutSzPixel.Width(), aTBSz.Height()) );

    aViewWin.SetSizePixel( Size( aOutSzPixel.Width()-nBorder-2*nExtra, aOutSzPixel.Height()-aTBSz.Height()-2*nExtra-nBorder) );

    aViewWin.SetPosPixel( Point( nExtra, aTBSz.Height()+nExtra ) );

    SetScrollBars();

    EditView* pEditView = aViewWin.GetEditView();
    if ( pEditView && pEditView->GetEditEngine()->GetPolygon() )
        CreatePolygon();
}

IMPL_LINK( EditMainWindow, ShowStatus, EditStatus *, pStat )
{
    EditView* pEditView = aViewWin.GetEditView();
    EditEngine* pEditEngine = pEditView->GetEditEngine();
    if ( ( pStat->GetStatusWord() & EE_STAT_TEXTWIDTHCHANGED ) || ( pStat->GetStatusWord() & EE_STAT_TEXTHEIGHTCHANGED ) )
    {
        aViewWin.MarkOutputArea();
        if ( pEditEngine->GetTextHeight() < (ULONG)pEditView->GetOutputArea().GetHeight() )
        {
            // Wird durch RGCHK_PAPERSZ1 'geklippt'
            if ( !( pEditEngine->GetControlWord() & EE_CNTRL_AUTOPAGESIZE) )
                pEditView->Scroll( -(long)pEditEngine->CalcTextWidth(), -(long)pEditEngine->GetTextHeight(), RGCHK_PAPERSZ1 );
        }
        SetScrollBarRanges();
    }
    if ( pStat->GetStatusWord() & EE_STAT_HSCROLL )
        aHScrollBar.SetThumbPos( pEditView->GetVisArea().Left() );
    if ( pStat->GetStatusWord() & EE_STAT_VSCROLL )
        aVScrollBar.SetThumbPos( pEditView->GetVisArea().Top() );

    return 0;
}


EditViewWindow::~EditViewWindow()
{
    pEditEngine->RemoveView( pEditView );
    delete pEditView;
    // Beim Zerstoeren der Styles am Ende, EditEngine noch nicht kaputt,
    // wird der Handler gerufen, ich zerstore hier aber schon die View!
    pEditEngine->SetStatusEventHdl( Link() );
//  SvxAutoCorrect* pAutoCorrekt = pEditEngine->GetAutoCorrect();
//  pEditEngine->SetAutoCorrect( 0 );
//  delete pAutoCorrekt;
    delete pEditEngine;
}

EditViewWindow::EditViewWindow( Window* pParent ) :
                Window( pParent ), aURLPtr( POINTER_HAND )
{
    SetBackgroundBrush( Brush( Color( COL_WHITE ) ) );
    SetMapMode( MAP_100TH_MM );
//  EnableDrop();

    SfxItemPool* pPool = EditEngine::CreatePool();
    Font aFont = GetSettings().GetStyleSettings().GetAppFont();
    MapMode aPntMode( MAP_POINT );
    MapMode aCurrent( GetMapMode() );
    Size aSz( LogicToLogic( Size( 12, 0 ), &aPntMode, &aCurrent ) );
    aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) ) );
    pPool->SetPoolDefaultItem( SvxFontItem( aFont.GetFamily(), aFont.GetName(), String(),aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO ) );
    pPool->SetPoolDefaultItem( SvxFontHeightItem( aSz.Width(), 100, EE_CHAR_FONTHEIGHT ) );
    pEditEngine = new MyEditEngine( pPool );

    Size aPaperSz( 10000,8000 );
    pEditEngine->SetPaperSize( aPaperSz );

    pEditView = new EditView( pEditEngine, this );
    pEditView->SetBackgroundColor( Color( COL_WHITE ) );
    pEditView->SetOutputArea( Rectangle( Point( 100, 100 ), aPaperSz ) );
    pEditEngine->SetDefaultLanguage( LANGUAGE_ENGLISH );
    pEditEngine->InsertView( pEditView );

    SetPosSizePixel( Point( 10, 10 ), Size( 600, 400 ) );
    SetPen( PEN_NULL );
    Show();

    // Feldbefehle grau hinterlegen
    ULONG n = pEditEngine->GetControlWord();
    n = n | (EE_CNTRL_MARKFIELDS|EE_CNTRL_AUTOCOMPLETE);
    pEditEngine->SetControlWord( n );

    // Test: Autozentrierung
//  ULONG n = pEditEngine->GetControlWord();
//  n = n | EE_CNTRL_AUTOPAGESIZE;
//  pEditEngine->SetControlWord( n );

    // OneLineSpeling
#ifdef WNT
//  pEditEngine->CreateSpeller( DirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "n:\\offenv\\wnti" ) ) ),
//                              DirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "n:\\offenv\\wnti" ) ) ) );
//  pEditEngine->GetSpeller()->SetActualLanguage( LANGUAGE_GERMAN );
//  pEditEngine->GetSpeller()->SetDefaultLanguage( LANGUAGE_GERMAN );
//  pEditEngine->GetSpeller()->SetMinTrail( 2 );

    // AutoCorrect wird nie zerstoert
//  pEditEngine->SetAutoCorrect( new SvxAutoCorrect( String( RTL_CONSTASCII_USTRINGPARAM( "d:\\prj\\office\\autotext\\autocorr.dat" ) ) ) );
#endif
}

void __EXPORT EditViewWindow::Paint( const Rectangle& rRec )
{
    if ( pEditView->GetEditEngine()->GetPolygon() )
    {
        // Die Punkte des Polygons beziehen sich auf die View...
        MapMode aMapMode( GetMapMode() );
        aMapMode.SetOrigin( pEditView->GetOutputArea().TopLeft() );
        SetMapMode( aMapMode );
        DrawPolyPolygon( *pEditView->GetEditEngine()->GetPolygon() );
        aMapMode.SetOrigin( Point() );
        SetMapMode( aMapMode );
    }
    pEditView->Paint( rRec );
    MarkOutputArea();
}

void EditViewWindow::MarkOutputArea()
{
    static Rectangle aCurFrame;
    Rectangle aOutArea( pEditView->GetOutputArea() );
    aOutArea = LogicToPixel( aOutArea );
    aOutArea.Left()--;
    aOutArea.Right()++;
    aOutArea.Top()--;
    aOutArea.Bottom()++;
    aOutArea = PixelToLogic( aOutArea );
    SetPen( Pen( Color( COL_RED ) ) );
    SetFillInBrush( Brush( BRUSH_NULL ) );
    DrawRect( aOutArea );
    if ( !aCurFrame.IsEmpty() )
    {
        if ( aCurFrame.Left() < aOutArea.Left() )
            Invalidate( Rectangle( aCurFrame.TopLeft(), Size( aOutArea.Left()-aCurFrame.Left(), aCurFrame.GetHeight() ) ) );
        if ( aCurFrame.Right() > aOutArea.Right() )
        {
            long nW = aCurFrame.Right() - aOutArea.Right();
            Point aPos( aCurFrame.TopRight() );
            aPos.X() -= nW;
            Invalidate( Rectangle( aPos, Size( nW, aCurFrame.GetHeight() ) ) );
        }
        if ( aCurFrame.Top() < aOutArea.Top() )
            Invalidate( Rectangle( aCurFrame.TopLeft(), Size( aCurFrame.GetWidth(), aOutArea.Top() - aCurFrame.Top() ) ) );
        if ( aCurFrame.Bottom() > aOutArea.Bottom() )
        {
            long nH = aCurFrame.Bottom() - aOutArea.Bottom();
            Point aPos( aCurFrame.BottomLeft() );
            aPos.Y() -= nH;
            Invalidate( Rectangle( aPos, Size( aCurFrame.GetWidth(), nH ) ) );
        }
    }
    aCurFrame = aOutArea;
}

void __EXPORT EditViewWindow::Resize()
{
    Size aPaperSz( GetOutputSize() );
    pEditView->SetOutputArea( Rectangle( Point(0,0), aPaperSz ) );
    pEditEngine->SetPaperSize( Size( aPaperSz.Width()*1, aPaperSz.Height()*1 ) );
    pEditView->ShowCursor();
    Invalidate();
}

void __EXPORT EditViewWindow::KeyInput( const KeyEvent& rKEvt )
{
    sal_Unicode nCharCode = rKEvt.GetCharCode();

    USHORT nCode = rKEvt.GetKeyCode().GetCode();
    // Auswertung fuer besondere Einstellungen....
    if ( ( nCode == KEY_A) && rKEvt.GetKeyCode().IsMod1() )
        pEditView->SetSelection( ESelection( 0, 0, 0xFFFF, 0xFFFF ) );
    else if ( ( nCode == KEY_R ) && rKEvt.GetKeyCode().IsMod2() )
        Invalidate();
    else if ( ( nCode == KEY_L ) && rKEvt.GetKeyCode().IsMod2() )
    {
        ULONG n = pEditEngine->GetControlWord();
        n = n | EE_CNTRL_ONECHARPERLINE;
        pEditEngine->SetControlWord( n );
        pEditEngine->QuickFormatDoc();
    }
    else if ( ( nCode == KEY_Z ) && rKEvt.GetKeyCode().IsMod2() )
    {
        pEditView->RemoveAttribs();
    }
    else if ( ( nCode == KEY_V ) && rKEvt.GetKeyCode().IsMod2() )
    {
        pEditEngine->SetVertical( TRUE );
        Invalidate();
    }
    else if ( ( ( nCode == KEY_ADD ) || ( nCode == KEY_SUBTRACT ) )&& rKEvt.GetKeyCode().IsMod2() )
    {
        short nDiff = ( nCode == KEY_ADD ) ? (+5) : (-5);
        if ( nZoom > 1000 )
            nDiff *= 20;
        if ( nZoom <= 25 )
            nDiff /= 5;
        if ( ( nZoom + nDiff ) > 0 )
            nZoom += nDiff;
        MapMode aMapMode( MAP_100TH_MM, Point(0,0), Fraction(nZoom,100), Fraction(nZoom,100) );
        SetMapMode( aMapMode );
        ((EditMainWindow*)GetParent())->SetTitle();

        if ( rKEvt.GetKeyCode().IsMod1() )
        {
            InfoBox( 0, String( RTL_CONSTASCII_USTRINGPARAM( "ChangingRefMapMode..." ) ) ).Execute();
            pEditEngine->SetRefMapMode( aMapMode );
        }
        Invalidate();
    }
    else if ( rKEvt.GetKeyCode().IsMod2() &&
            ( ( nCode == KEY_UP) || ( nCode == KEY_DOWN ) ||
              ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) ) )
    {
        if( rKEvt.GetKeyCode().IsMod1() )
        {
            ULONG nCtrl = pEditEngine->GetControlWord();
            if ( ! ( nCtrl & EE_CNTRL_STRETCHING ) )
            {
                nCtrl |= EE_CNTRL_STRETCHING;
                pEditEngine->SetControlWord( nCtrl );
            }
            USHORT nX, nY;
            pEditEngine->GetGlobalCharStretching( nX, nY );
            if ( ( nCode == KEY_DOWN ) && ( nY > 5 ) )
                nY += 5;
            else if ( nCode == KEY_UP )
                nY -= 5;
            else if ( nCode == KEY_RIGHT )
                nX += 5;
            else if ( ( nCode == KEY_LEFT ) && ( nX > 5 ) )
                nX -= 5;
            pEditEngine->SetGlobalCharStretching( nX, nY );
            ((EditMainWindow*)GetParent())->SetTitle();
        }
        else    // ZeichenAttr
        {
            SfxItemSet aAttrs( pEditView->GetAttribs() );
            SfxItemSet aNewAttrs( pEditEngine->GetEmptyItemSet() );
            const SvxFontWidthItem& rItem = (const SvxFontWidthItem&)aAttrs.Get( EE_CHAR_FONTWIDTH );
            USHORT nProp = rItem.GetProp();
            if ( nCode == KEY_RIGHT )
                nProp += 5;
            else if ( ( nCode == KEY_LEFT ) && ( nProp > 5 ) )
                nProp -= 5;
            aNewAttrs.Put( SvxFontWidthItem( 0, nProp, EE_CHAR_FONTWIDTH ) );
            pEditView->SetAttribs( aNewAttrs );
        }
    }
    else if ( ( nCode == KEY_O ) && rKEvt.GetKeyCode().IsMod2() )
    {
        MapMode aMapMode( GetMapMode() );
        aMapMode.SetOrigin( Point( 41, 41 ) );
        SetMapMode( aMapMode );
        Invalidate();
    }
    else if ( ( nCode == KEY_B ) && rKEvt.GetKeyCode().IsMod2() )
    {
        SfxItemSet aSet = pEditView->GetEmptyItemSet();
        aSet.Put( SvxFontHeightItem(800, 100, EE_CHAR_FONTHEIGHT) );
        pEditView->SetAttribs( aSet );
    }
    else if ( ( nCode == KEY_P ) && rKEvt.GetKeyCode().IsMod2() )
    {
//              ESelection aSel = pEditView->GetSelection();
//              for ( ULONG n = aSel.nStartPara; n <= aSel.nEndPara; n++ )
//              {
//                  InfoBox( 0, pEditEngine->GetText( n ) ).Execute();
//              }
        InfoBox( 0, pEditView->GetSelected() ).Execute();

    }
    else if ( rKEvt.GetKeyCode().IsMod2() &&
                ( nCode >= KEY_1 ) && ( nCode <= KEY_9 ) )
    {
        ULONG nEECtrl = pEditEngine->GetControlWord();
        nEECtrl = nEECtrl | EE_CNTRL_AUTOPAGESIZE;
        pEditEngine->SetControlWord( nEECtrl );

        ULONG nEVCtrl = pEditView->GetControlWord();
        nEVCtrl |= EV_CNTRL_AUTOSIZE;
        pEditView->SetControlWord( nEVCtrl );

        pEditView->SetAnchorMode( (EVAnchorMode)(nCode-KEY_1) );
        pEditView->SetOutputArea( Rectangle( Point(0,0), GetOutputSize() ) );
        pEditEngine->SetMaxAutoPaperSize( GetOutputSize() );
        pEditEngine->SetPaperSize( Size( 0, 0 ) );
    }
    else if ( rKEvt.GetKeyCode().IsMod2() && ( nCode == KEY_0 ) )
    {
        ULONG nEVCtrl = pEditView->GetControlWord();
        nEVCtrl = nEVCtrl | EV_CNTRL_AUTOSIZE;
        pEditView->SetControlWord( nEVCtrl );

        Size aOutSz( GetOutputSize() );
        Size aPaperSz( aOutSz.Width() / 2, 0 );
        pEditView->SetOutputArea( Rectangle( Point(), aPaperSz ) );
        pEditEngine->SetPaperSize( aPaperSz );
        // Max-Height wird bei View verwendet,
        // MinSize/MaxSize nur, wenn AUTOPAGESIZE ( KEY_1 - KEY_9 )
        pEditEngine->SetMaxAutoPaperSize( Size( aOutSz.Width() / 2, aOutSz.Height() / 2 ) );
        pEditEngine->SetMinAutoPaperSize( Size( aOutSz.Width() / 8, aOutSz.Height() / 8 ) );
    }
    else if ( ( nCode == KEY_J ) && rKEvt.GetKeyCode().IsMod2() )
    {
        Size aTextSz( pEditEngine->CalcTextWidth(), pEditEngine->GetTextHeight() );
        Rectangle aRect( pEditView->GetOutputArea().TopLeft(), aTextSz );
        Brush aTmpBrush( Color( COL_LIGHTBLUE ), BRUSH_25 );
        Brush aOldBrush( GetFillInBrush() );
        SetFillInBrush( aTmpBrush );
        DrawRect( aRect );
        SetFillInBrush( aOldBrush );
        pEditView->Paint( aRect );
    }
    else if ( ( nCode == KEY_H ) && rKEvt.GetKeyCode().IsMod2() )
    {
        Push();
        Size aSz( pEditEngine->CalcTextWidth(), pEditEngine->GetTextHeight() );
        Pen aPen( PEN_SOLID );
        Brush aBrush( Color( COL_GRAY ), BRUSH_SOLID );
        aPen.SetColor( Color( COL_BLACK ) );
        SetPen( aPen );
        SetFillInBrush( aBrush );
        Rectangle aR( pEditView->GetOutputArea().TopLeft(), aSz );
        DrawRect( aR );
        Pop();
        pEditEngine->Draw( this, pEditView->GetOutputArea() );
        pEditView->ShowCursor( TRUE, TRUE );
    }

    // Eingabe an EditEngine...
    else
    {
        BOOL bDone = pEditView->PostKeyEvent( rKEvt );
        if ( !bDone )
            Window::KeyInput( rKEvt );
    }

    ((EditMainWindow*)GetParent())->UpdateToolBox();
}

void __EXPORT EditViewWindow::MouseMove( const MouseEvent& rMEvt )
{
    Point aPos = PixelToLogic( rMEvt.GetPosPixel() );
    if ( pEditView->GetOutputArea().IsInside( aPos ) )
    {
        const SvxFieldItem* pField = pEditView->GetFieldUnderMousePointer();
        if ( pField )
            SetPointer( aURLPtr );
        else
            SetPointer( pEditView->GetPointer() );

//      aPos -= pEditView->GetOutputArea().TopLeft();
//      aPos += pEditView->GetVisArea().TopLeft();
//      if ( pEditView->GetEditEngine()->IsTextPos( aPos, PixelToLogic( Size( 5, 0 ) ).Width() ) )
//          SetPointer( pEditView->GetPointer() );
//      else
//          SetPointer( Pointer( POINTER_REFHAND ) );

    }
    else
        SetPointer( aStdPtr );

//  static long x = 0;
//  x++;
//  DBG_ASSERT( x < 1000, String( RTL_CONSTASCII_USTRINGPARAM( "?" ) ) );
    pEditView->MouseMove( rMEvt );
}

void __EXPORT EditViewWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    pEditView->MouseButtonDown( rMEvt );
}

void __EXPORT EditViewWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    pEditView->MouseButtonUp( rMEvt );
    ((EditMainWindow*)GetParent())->UpdateToolBox();
}

void __EXPORT EditViewWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        if( pEditView->IsWrongSpelledWordAtPos( rCEvt.GetMousePosPixel() ) )
            pEditView->ExecuteSpellPopup( rCEvt.GetMousePosPixel() );
    }
    else
        pEditView->Command(rCEvt);

}

BOOL __EXPORT EditViewWindow::Drop( const DropEvent& rEvt )
{
    return pEditView->Drop( rEvt );
}

BOOL __EXPORT EditViewWindow::QueryDrop( DropEvent& rEvt )
{
    return pEditView->QueryDrop( rEvt );
}

// --- aEditApp ------------------------------------------------------

void __EXPORT EditApp::Main()
{
#ifdef WNT
    SvFactory::Init();
#endif

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xSMgr = createApplicationServiceManager();
    ::comphelper::setProcessServiceFactory( xSMgr );

    EditDLL aEditDll;
    SvxGlobalItemData aItemData;
    SvxFieldItem::GetClassManager().SV_CLASS_REGISTER( SvxDateField );
    SvxFieldItem::GetClassManager().SV_CLASS_REGISTER( SvxURLField );

    Help::EnableQuickHelp();

    EditMainWindow aWindow;
    Execute();
}

EditApp aEditApp;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
