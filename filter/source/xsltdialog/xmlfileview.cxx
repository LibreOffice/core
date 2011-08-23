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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <comphelper/oslfile2streamwrap.hxx>

#include <rtl/tencinfo.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svtools/textview.hxx>
#include <vcl/scrbar.hxx>
#include <tools/stream.hxx>
#include <tools/time.hxx>
#include <osl/file.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/htmltokn.h>
#include <svtools/txtattr.hxx>

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfileview.hxx"
#include "xmlfileview.hrc"
#include "xmlfilterhelpids.hrc"

using namespace rtl;
using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::xml;
using namespace com::sun::star::xml::sax;


#define MAX_SYNTAX_HIGHLIGHT 20
#define MAX_HIGHLIGHTTIME 200
#define SYNTAX_HIGHLIGHT_TIMEOUT 200


struct SwTextPortion
{
    USHORT nLine;
    USHORT nStart, nEnd;
    svtools::ColorConfigEntry eType;
};

SV_DECL_VARARR(SwTextPortions, SwTextPortion,16,16)
SV_IMPL_VARARR(SwTextPortions, SwTextPortion);

class XMLErrorHandler : public ::cppu::WeakImplHelper1< XErrorHandler >
{
public:
    XMLErrorHandler( XMLSourceFileDialog* pParent, ListBox& rListBox );

    // Methods
    virtual void SAL_CALL error( const Any& aSAXParseException ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL fatalError( const Any& aSAXParseException ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL warning( const Any& aSAXParseException ) throw (SAXException, RuntimeException);

private:
    XMLSourceFileDialog*	mpParent;
    ListBox&	mrListBox;
};

XMLErrorHandler::XMLErrorHandler( XMLSourceFileDialog* pParent, ListBox& rListBox )
:	mpParent( pParent ),
    mrListBox( rListBox )
{
}

// XMLErrorHandler
void SAL_CALL XMLErrorHandler::error( const Any& aSAXParseException ) throw (SAXException, RuntimeException)
{
    ::SolarMutexGuard aGuard;

    SAXParseException e;
    if( aSAXParseException >>= e )
    {
        String sErr( String::CreateFromInt32( e.LineNumber ) );
        sErr += String( RTL_CONSTASCII_USTRINGPARAM( " : " ) );
        sErr += String( e.Message );
        USHORT nEntry = mrListBox.InsertEntry( sErr );
        mrListBox.SetEntryData( nEntry, (void*)e.LineNumber );
    }
}

void SAL_CALL XMLErrorHandler::fatalError( const Any& aSAXParseException ) throw (SAXException, RuntimeException)
{
    ::SolarMutexGuard aGuard;

    SAXParseException e;
    if( aSAXParseException >>= e )
    {
        String sErr( String::CreateFromInt32( e.LineNumber ) );
        sErr += String( RTL_CONSTASCII_USTRINGPARAM( " : " ) );
        sErr += String( e.Message );
        USHORT nEntry = mrListBox.InsertEntry( sErr );
        mrListBox.SetEntryData( nEntry, (void*)e.LineNumber );
    }
}

void SAL_CALL XMLErrorHandler::warning( const Any& /* aSAXParseException */ ) throw (SAXException, RuntimeException)
{
/*
    SAXParseException e;
    if( aSAXParseException >>= e )
    {
        String sErr( String::CreateFromInt32( e.LineNumber ) );
        sErr += String( RTL_CONSTASCII_USTRINGPARAM( " : " ) );
        sErr += String( e.Message );
        USHORT nEntry = mrListBox.InsertEntry( sErr );
        mrListBox.SetEntryData( nEntry, (void*)e.LineNumber );
    }
*/
}


XMLFileWindow::XMLFileWindow( Window* pParent ) :
    Window( pParent, WB_BORDER|WB_CLIPCHILDREN ),
    pTextEngine(0),
    pOutWin(0),
    pHScrollbar(0),
    pVScrollbar(0),
    nCurTextWidth(0),
    nStartLine(USHRT_MAX),
    eSourceEncoding(gsl_getSystemTextEncoding()),
    bHighlighting(false)
{
    CreateTextEngine();
}

XMLFileWindow::~XMLFileWindow()
{
    if ( pTextEngine )
    {
        EndListening( *pTextEngine );
        pTextEngine->RemoveView( pTextView );

        delete pHScrollbar;
        delete pVScrollbar;

        delete pTextView;
        delete pTextEngine;
    }
    delete pOutWin;
}

void XMLFileWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    switch ( rDCEvt.GetType() )
    {
    case DATACHANGED_SETTINGS:
        // ScrollBars neu anordnen bzw. Resize ausloesen, da sich
        // ScrollBar-Groesse geaendert haben kann. Dazu muss dann im
        // Resize-Handler aber auch die Groesse der ScrollBars aus
        // den Settings abgefragt werden.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
            Resize();
        break;
    }
}

void XMLFileWindow::Resize()
{
    // ScrollBars, etc. passiert in Adjust...
    if ( pTextView )
    {
        long nVisY = pTextView->GetStartDocPos().Y();
        pTextView->ShowCursor();
        Size aOutSz( GetOutputSizePixel() );
        long nMaxVisAreaStart = pTextView->GetTextEngine()->GetTextHeight() - aOutSz.Height();
        if ( nMaxVisAreaStart < 0 )
            nMaxVisAreaStart = 0;
        if ( pTextView->GetStartDocPos().Y() > nMaxVisAreaStart )
        {
            Point aStartDocPos( pTextView->GetStartDocPos() );
            aStartDocPos.Y() = nMaxVisAreaStart;
            pTextView->SetStartDocPos( aStartDocPos );
            pTextView->ShowCursor();
        }
        long nScrollStd = GetSettings().GetStyleSettings().GetScrollBarSize();
        Size aScrollSz(aOutSz.Width() - nScrollStd, nScrollStd );
        Point aScrollPos(0, aOutSz.Height() - nScrollStd);

        pHScrollbar->SetPosSizePixel( aScrollPos, aScrollSz);

        aScrollSz.Width() = aScrollSz.Height();
        aScrollSz.Height() = aOutSz.Height() - aScrollSz.Height();
        aScrollPos = Point(aOutSz.Width() - nScrollStd, 0);

        pVScrollbar->SetPosSizePixel( aScrollPos, aScrollSz);
        aOutSz.Width() 	-= nScrollStd;
        aOutSz.Height() 	-= nScrollStd;
        pOutWin->SetOutputSizePixel(aOutSz);
        InitScrollBars();

        // Zeile im ersten Resize setzen
        if(USHRT_MAX != nStartLine)
        {
            if(nStartLine < pTextEngine->GetParagraphCount())
            {
                TextSelection aSel(TextPaM( nStartLine, 0 ), TextPaM( nStartLine, 0x0 ));
                pTextView->SetSelection(aSel);
                pTextView->ShowCursor();
            }
            nStartLine = USHRT_MAX;
        }

        if ( nVisY != pTextView->GetStartDocPos().Y() )
            InvalidateWindow();
    }

}

void TextViewOutWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    switch( rDCEvt.GetType() )
    {
    case DATACHANGED_SETTINGS:
        // den Settings abgefragt werden.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
        {
            const Color &rCol = GetSettings().GetStyleSettings().GetWindowColor();
            SetBackground( rCol );
            Font aFont( pTextView->GetTextEngine()->GetFont() );
            aFont.SetFillColor( rCol );
            pTextView->GetTextEngine()->SetFont( aFont );
        }
        break;
    }
}

void TextViewOutWin::MouseMove( const MouseEvent &rEvt )
{
    if ( pTextView )
        pTextView->MouseMove( rEvt );
}

void TextViewOutWin::MouseButtonUp( const MouseEvent &rEvt )
{
    if ( pTextView )
        pTextView->MouseButtonUp( rEvt );
}

void TextViewOutWin::MouseButtonDown( const MouseEvent &rEvt )
{
    GrabFocus();
    if ( pTextView )
        pTextView->MouseButtonDown( rEvt );
}

void TextViewOutWin::Command( const CommandEvent& rCEvt )
{
    switch(rCEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        break;
        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pWData = rCEvt.GetWheelData();
            if( !pWData || COMMAND_WHEEL_ZOOM != pWData->GetMode() )
            {
                ((XMLFileWindow*)GetParent())->HandleWheelCommand( rCEvt );
            }
        }
        break;

        default:
            if ( pTextView )
                pTextView->Command( rCEvt );
        else
            Window::Command(rCEvt);
    }
}

void TextViewOutWin::KeyInput( const KeyEvent& rKEvt )
{
    if(!TextEngine::DoesKeyChangeText( rKEvt ))
        pTextView->KeyInput( rKEvt );
}

void  TextViewOutWin::Paint( const Rectangle& rRect )
{
    pTextView->Paint( rRect );
}

void XMLFileWindow::CreateTextEngine()
{
    const Color &rCol = GetSettings().GetStyleSettings().GetWindowColor();
    pOutWin = new TextViewOutWin(this, 0);
    pOutWin->SetBackground(Wallpaper(rCol));
    pOutWin->SetPointer(Pointer(POINTER_TEXT));
    pOutWin->Show();

    //Scrollbars anlegen
    pHScrollbar = new ScrollBar(this, WB_3DLOOK |WB_HSCROLL|WB_DRAG);
    pHScrollbar->SetScrollHdl(LINK(this, XMLFileWindow, ScrollHdl));
    pHScrollbar->Show();

    pVScrollbar = new ScrollBar(this, WB_3DLOOK |WB_VSCROLL|WB_DRAG);
    pVScrollbar->SetScrollHdl(LINK(this, XMLFileWindow, ScrollHdl));
    pHScrollbar->EnableDrag();
    pVScrollbar->Show();

    pTextEngine = new TextEngine;
    pTextView = new TextView( pTextEngine, pOutWin );
    pTextView->SetAutoIndentMode(TRUE);
    pOutWin->SetTextView(pTextView);

    pTextEngine->SetUpdateMode( FALSE );
    pTextEngine->InsertView( pTextView );

    Font aFont;
    aFont.SetTransparent( FALSE );
    aFont.SetFillColor( rCol );
    SetPointFont( aFont );
    aFont = GetFont();
    aFont.SetFillColor( rCol );
    pOutWin->SetFont( aFont );
    pTextEngine->SetFont( aFont );

    aSyntaxIdleTimer.SetTimeout( SYNTAX_HIGHLIGHT_TIMEOUT );
    aSyntaxIdleTimer.SetTimeoutHdl( LINK( this, XMLFileWindow, SyntaxTimerHdl ) );

    pTextEngine->EnableUndo( FALSE );
    pTextEngine->SetUpdateMode( TRUE );

//	pTextView->ShowCursor( TRUE, TRUE );
    pTextView->HideCursor();

    InitScrollBars();
    StartListening( *pTextEngine );
}

void XMLFileWindow::SetScrollBarRanges()
{
    pHScrollbar->SetRange( Range( 0, nCurTextWidth-1 ) );
    pVScrollbar->SetRange( Range(0, pTextEngine->GetTextHeight()-1) );
}

void XMLFileWindow::InitScrollBars()
{
    SetScrollBarRanges();

    Size aOutSz( pOutWin->GetOutputSizePixel() );
    pVScrollbar->SetVisibleSize( aOutSz.Height() );
    pVScrollbar->SetPageSize(  aOutSz.Height() * 8 / 10 );
    pVScrollbar->SetLineSize( pOutWin->GetTextHeight() );
    pVScrollbar->SetThumbPos( pTextView->GetStartDocPos().Y() );
    pHScrollbar->SetVisibleSize( aOutSz.Width() );
    pHScrollbar->SetPageSize( aOutSz.Width() * 8 / 10 );
    pHScrollbar->SetLineSize( pOutWin->GetTextWidth( 'x' ) );
    pHScrollbar->SetThumbPos( pTextView->GetStartDocPos().X() );

}

IMPL_LINK(XMLFileWindow, ScrollHdl, ScrollBar*, pScroll)
{
    if(pScroll == pVScrollbar)
    {
        long nDiff = pTextView->GetStartDocPos().Y() - pScroll->GetThumbPos();
        GetTextView()->Scroll( 0, nDiff );
        pTextView->ShowCursor( FALSE, TRUE );
        pScroll->SetThumbPos( pTextView->GetStartDocPos().Y() );
    }
    else
    {
        long nDiff = pTextView->GetStartDocPos().X() - pScroll->GetThumbPos();
        GetTextView()->Scroll( nDiff, 0 );
        pTextView->ShowCursor( FALSE, TRUE );
        pScroll->SetThumbPos( pTextView->GetStartDocPos().X() );
    }
    return 0;
}

void XMLFileWindow::Notify( SfxBroadcaster& /* rBC */, const SfxHint& rHint )
{
    if ( rHint.ISA( TextHint ) )
    {
        const TextHint& rTextHint = (const TextHint&)rHint;
        if( rTextHint.GetId() == TEXT_HINT_VIEWSCROLLED )
        {
            pHScrollbar->SetThumbPos( pTextView->GetStartDocPos().X() );
            pVScrollbar->SetThumbPos( pTextView->GetStartDocPos().Y() );
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTHEIGHTCHANGED )
        {
            if ( (long)pTextEngine->GetTextHeight() < pOutWin->GetOutputSizePixel().Height() )
                pTextView->Scroll( 0, pTextView->GetStartDocPos().Y() );
            pVScrollbar->SetThumbPos( pTextView->GetStartDocPos().Y() );
            SetScrollBarRanges();
        }
        else if( rTextHint.GetId() == TEXT_HINT_FORMATPARA )
        {
            DoDelayedSyntaxHighlight( (USHORT)rTextHint.GetValue() );
        }
    }
}

void XMLFileWindow::InvalidateWindow()
{
    pOutWin->Invalidate();
    Window::Invalidate();

}

void XMLFileWindow::Command( const CommandEvent& rCEvt )
{
    switch(rCEvt.GetCommand())
    {
        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pWData = rCEvt.GetWheelData();
            if( !pWData || COMMAND_WHEEL_ZOOM != pWData->GetMode() )
                HandleScrollCommand( rCEvt, pHScrollbar, pVScrollbar );
        }
        break;
        default:
            Window::Command(rCEvt);
    }
}

void XMLFileWindow::HandleWheelCommand( const CommandEvent& rCEvt )
{
    pTextView->Command(rCEvt);
    HandleScrollCommand( rCEvt, pHScrollbar, pVScrollbar );
}

void XMLFileWindow::GetFocus()
{
    pOutWin->GrabFocus();
}

void XMLFileWindow::ShowWindow( const rtl::OUString& rFileName )
{
    String aFileName( rFileName );
    SvFileStream aStream( aFileName, STREAM_READ );

    // since the xml files we load are utf-8 encoded, we need to set
    // this encoding at the SvFileStream, else the TextEngine will
    // use its default encoding which is not UTF8
    const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( RTL_TEXTENCODING_UTF8 );
    rtl_TextEncoding eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );
    aStream.SetStreamCharSet( eDestEnc );

    if( Read( aStream ) )
    {
        long nPrevTextWidth = nCurTextWidth;
        nCurTextWidth = pTextEngine->CalcTextWidth() + 25;	// kleine Toleranz
        if ( nCurTextWidth != nPrevTextWidth )
            SetScrollBarRanges();
        
        TextPaM aPaM( pTextView->CursorStartOfDoc() );
        TextSelection aSelection( aPaM, aPaM );
        pTextView->SetSelection( aSelection, true );

        Window::Show();
    }
}

void XMLFileWindow::showLine( sal_Int32 nLine )
{
    TextPaM aPaM( pTextView->CursorStartOfDoc() );
    while( nLine-- )
        aPaM = pTextView->CursorDown( aPaM );

    TextSelection aSelection( pTextView->CursorEndOfLine( aPaM ), aPaM );
    pTextView->SetSelection( aSelection, true );
}


XMLSourceFileDialog::XMLSourceFileDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF  )
:	WorkWindow( pParent, ResId( DLG_XML_SOURCE_FILE_DIALOG, rResMgr ) ),
    mnOutputHeight( LogicToPixel( Size( 80, 80 ), MAP_APPFONT ).Height() ),
    mxMSF( rxMSF ),
    mrResMgr( rResMgr ),	
    maLBOutput( this ),
    maPBValidate( this, ResId( PB_VALIDATE, rResMgr ) )
{

    FreeResource();

    maPBValidate.SetClickHdl(LINK( this, XMLSourceFileDialog, ClickHdl_Impl ) );
    maLBOutput.SetSelectHdl(LINK(this, XMLSourceFileDialog, SelectHdl_Impl ) );
    mpTextWindow = new XMLFileWindow( this );
    mpTextWindow->SetHelpId( HID_XML_FILTER_OUTPUT_WINDOW );
    maLBOutput.SetHelpId( HID_XML_FILTER_TEST_VALIDATE_OUPUT );

    Resize();
}

XMLSourceFileDialog::~XMLSourceFileDialog()
{
    if( maFileURL.getLength() )
        osl::File::remove( maFileURL );

    delete mpTextWindow;
}

void XMLSourceFileDialog::ShowWindow( const rtl::OUString& rFileName, const filter_info_impl* pFilterInfo )
{
    EnterWait();
    if( maFileURL.getLength() )
    {
        osl::File::remove( maFileURL );
        delete mpTextWindow;
        mpTextWindow = new XMLFileWindow( this );
        maLBOutput.Hide();
        maLBOutput.Clear();
        maPBValidate.Enable(TRUE);
        Resize();
    }

    mpFilterInfo = pFilterInfo;
    maFileURL = rFileName;
    mpTextWindow->ShowWindow( rFileName );
    WorkWindow::Show();
    LeaveWait();
}

void XMLSourceFileDialog::Resize()
{
    bool bOutputVisible = maLBOutput.IsVisible() != 0;

    Point aSpacing( LogicToPixel( Point( 6, 6 ), MAP_APPFONT ) );
    Size aButton( maPBValidate.GetSizePixel() );

    Size aDialogSize( GetOutputSizePixel() );

//	Point aButtonPos( aSpacing.X(), aSpacing.Y() );
//	maPBValidate.SetPosSizePixel( aButtonPos, aButton );

    Size aOutputSize( aDialogSize.Width(), bOutputVisible ? mnOutputHeight : 0 );

    Point aTextWindowPos( 0, 2* aSpacing.Y() + aButton.Height() );
    Size aTextWindowSize( aDialogSize.Width(), aDialogSize.Height() - aTextWindowPos.Y() - aOutputSize.Height()  );

    mpTextWindow->SetPosSizePixel( aTextWindowPos, aTextWindowSize );

    if( bOutputVisible )
    {
        Point aOutputPos( 0, aTextWindowPos.Y() + aTextWindowSize.Height() );
        maLBOutput.SetPosSizePixel( aOutputPos, aOutputSize );
    }
}


IMPL_LINK(XMLSourceFileDialog, SelectHdl_Impl, ListBox *, pListBox )
{
    USHORT nEntry = pListBox->GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nEntry )
    {
        int nLine = (int)(sal_IntPtr)pListBox->GetEntryData(nEntry);
        if( -1 != nLine )
        {
            if( nLine > 0 )
                nLine--;

            showLine( nLine );
        }
    }
    return 0;
}

IMPL_LINK(XMLSourceFileDialog, ClickHdl_Impl, PushButton *, /* pButton */ )
{
    onValidate();
    return 0;
}

void XMLSourceFileDialog::onValidate()
{
    EnterWait();

    maLBOutput.Show();
    maPBValidate.Enable(FALSE);
    Resize();

    try
    {
        Reference< XImportFilter > xImporter( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.documentconversion.XSLTValidate" ) ), UNO_QUERY );
        if( xImporter.is() )
        {
            osl::File aInputFile( maFileURL );
            /* osl::File::RC rc = */ aInputFile.open( OpenFlag_Read );

            Reference< XInputStream > xIS( new comphelper::OSLInputStreamWrapper( aInputFile ) );
            
            Sequence< PropertyValue > aSourceData(3);
            aSourceData[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "InputStream" ) );
            aSourceData[0].Value <<= xIS;

            aSourceData[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
            aSourceData[1].Value <<= maFileURL;

            aSourceData[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ErrorHandler" ) );
            Reference< XErrorHandler > xHandle( new XMLErrorHandler( this, maLBOutput ) );
            aSourceData[2].Value <<= xHandle;

            Reference< XDocumentHandler > xWriter( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ), UNO_QUERY );	
            Reference< XOutputStream > xOS( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.Pipe" ) ) ), UNO_QUERY );
            Reference< XActiveDataSource > xDocSrc( xWriter, UNO_QUERY );
            xDocSrc->setOutputStream( xOS );

            Sequence< OUString > aFilterUserData( mpFilterInfo->getFilterUserData() );
            xImporter->importer( aSourceData, xWriter, aFilterUserData );
        }
    }				
    catch(Exception& e)
    {
        String sErr( e.Message );
        USHORT nEntry = maLBOutput.InsertEntry( sErr );
        maLBOutput.SetEntryData( nEntry, (void*)-1 );
    }

    if( 0 == maLBOutput.GetEntryCount() )
    {
        String sErr( RESID( STR_NO_ERRORS_FOUND ) );
        USHORT nEntry = maLBOutput.InsertEntry( sErr );
        maLBOutput.SetEntryData( nEntry, (void*)-1 );
    }

    LeaveWait();
}

void XMLSourceFileDialog::showLine( sal_Int32 nLine )
{
    mpTextWindow->showLine( nLine );
}


///////////////////////////////////////////////////////////////////////

void lcl_Highlight(const String& rSource, SwTextPortions& aPortionList)
{
    const sal_Unicode cOpenBracket = '<';
    const sal_Unicode cCloseBracket= '>';
    const sal_Unicode cSlash		= '/';
    const sal_Unicode cExclamation = '!';
//	const sal_Unicode cQuote		= '"';
//	const sal_Unicode cSQuote      = '\'';
    const sal_Unicode cMinus		= '-';
    const sal_Unicode cSpace		= ' ';
    const sal_Unicode cTab			= 0x09;
    const sal_Unicode cLF          = 0x0a;
    const sal_Unicode cCR          = 0x0d;


    const USHORT nStrLen = rSource.Len();
    USHORT nInsert = 0;			// Anzahl der eingefuegten Portions
    USHORT nActPos = 0;			//Position, an der '<' gefunden wurde
    USHORT nOffset = 0; 		//Offset von nActPos zur '<'
    USHORT nPortStart = USHRT_MAX; 	// fuer die TextPortion
    USHORT nPortEnd  = 	0;  //
    SwTextPortion aText;
    while(nActPos < nStrLen)
    {
        svtools::ColorConfigEntry eFoundType = svtools::HTMLUNKNOWN;
        if(rSource.GetChar(nActPos) == cOpenBracket && nActPos < nStrLen - 2 )
        {
            // 'leere' Portion einfuegen
            if(nPortEnd < nActPos - 1 )
            {
                aText.nLine = 0;
                // am Anfang nicht verschieben
                aText.nStart = nPortEnd;
                if(nInsert)
                    aText.nStart += 1;
                aText.nEnd = nActPos - 1;
                aText.eType = svtools::HTMLUNKNOWN;
                aPortionList.Insert(aText, nInsert++);
            }
            sal_Unicode cFollowFirst = rSource.GetChar((xub_StrLen)(nActPos + 1));
            sal_Unicode cFollowNext = rSource.GetChar((xub_StrLen)(nActPos + 2));
            if(cExclamation == cFollowFirst)
            {
                // "<!" SGML oder Kommentar
                if(cMinus == cFollowNext &&
                    nActPos < nStrLen - 3 && cMinus == rSource.GetChar((xub_StrLen)(nActPos + 3)))
                {
                    eFoundType = svtools::HTMLCOMMENT;
                }
                else
                    eFoundType = svtools::HTMLSGML;
                nPortStart = nActPos;
                nPortEnd = nActPos + 1;
            }
            else if(cSlash == cFollowFirst)
            {
                // "</" Slash ignorieren
                nPortStart = nActPos;
                nActPos++;
                nOffset++;
            }
            if(svtools::HTMLUNKNOWN == eFoundType)
            {
                //jetzt koennte hier ein keyword folgen
                USHORT nSrchPos = nActPos;
                while(++nSrchPos < nStrLen - 1)
                {
                    sal_Unicode cNext = rSource.GetChar(nSrchPos);
                    if( cNext == cSpace	||
                        cNext == cTab 	||
                        cNext == cLF 	||
                        cNext == cCR)
                        break;
                    else if(cNext == cCloseBracket)
                    {
                        break;
                    }
                }
                if(nSrchPos > nActPos + 1)
                {
                    //irgend ein String wurde gefunden
                    String sToken = rSource.Copy(nActPos + 1, nSrchPos - nActPos - 1 );
                    sToken.ToUpperAscii();
//					int nToken = ::GetHTMLToken(sToken);
//					if(nToken)
                    {
                        //Token gefunden
                        eFoundType = svtools::HTMLKEYWORD;
                        nPortEnd = nSrchPos;
                        nPortStart = nActPos;
                    }
/*
                    else
                    {
                        //was war das denn?
#ifdef DEBUG
                        DBG_ERROR("Token nicht erkannt!")
                        DBG_ERROR(ByteString(sToken, gsl_getSystemTextEncoding()).GetBuffer())
#endif
                    }
*/

                }
            }
            // jetzt muss noch '>' gesucht werden
            if(svtools::HTMLUNKNOWN != eFoundType)
            {
                BOOL bFound = FALSE;
                for(USHORT i = nPortEnd; i < nStrLen; i++)
                    if(cCloseBracket == rSource.GetChar(i))
                    {
                        bFound = TRUE;
                        nPortEnd = i;
                        break;
                    }
                if(!bFound && (eFoundType == svtools::HTMLCOMMENT))
                {
                    // Kommentar ohne Ende in dieser Zeile
                    bFound  = TRUE;
                    nPortEnd = nStrLen - 1;
                }

                if(bFound ||(eFoundType == svtools::HTMLCOMMENT))
                {
                    SwTextPortion aText2;
                    aText2.nLine = 0;
                    aText2.nStart = nPortStart + 1;
                    aText2.nEnd = nPortEnd;
                    aText2.eType = eFoundType;
                    aPortionList.Insert(aText2, nInsert++);
                    eFoundType = svtools::HTMLUNKNOWN;
                }

            }
        }
        nActPos++;
    }
    if(nInsert && nPortEnd < nActPos - 1)
    {
        aText.nLine = 0;
        aText.nStart = nPortEnd + 1;
        aText.nEnd = nActPos - 1;
        aText.eType = svtools::HTMLUNKNOWN;
        aPortionList.Insert(aText, nInsert++);
    }
}

void XMLFileWindow::DoDelayedSyntaxHighlight( USHORT nPara )
{
    if ( !bHighlighting )
    {
        aSyntaxLineTable.Insert( nPara, (void*)(USHORT)1 );
        aSyntaxIdleTimer.Start();
    }
}

void XMLFileWindow::ImpDoHighlight( const String& rSource, USHORT nLineOff )
{
    SwTextPortions aPortionList;
    lcl_Highlight(rSource, aPortionList);

    USHORT nCount = aPortionList.Count();
    if ( !nCount )
        return;

    SwTextPortion& rLast = aPortionList[nCount-1];
    if ( rLast.nStart > rLast.nEnd ) 	// Nur bis Bug von MD behoeben
    {
        nCount--;
        aPortionList.Remove( nCount);
        if ( !nCount )
            return;
    }

    // Evtl. Optimieren:
    // Wenn haufig gleiche Farbe, dazwischen Blank ohne Farbe,
    // ggf. zusammenfassen, oder zumindest das Blank,
    // damit weniger Attribute
    BOOL bOptimizeHighlight = TRUE; // war in der BasicIDE static
    if ( bOptimizeHighlight )
    {
        // Es muessen nur die Blanks und Tabs mit attributiert werden.
        // Wenn zwei gleiche Attribute hintereinander eingestellt werden,
        // optimiert das die TextEngine.
        USHORT nLastEnd = 0;
        for ( USHORT i = 0; i < nCount; i++ )
        {
            SwTextPortion& r = aPortionList[i];
            DBG_ASSERT( r.nLine == aPortionList[0].nLine, "doch mehrere Zeilen ?" );
            if ( r.nStart > r.nEnd ) 	// Nur bis Bug von MD behoeben
                continue;

            if ( r.nStart > nLastEnd )
            {
                // Kann ich mich drauf verlassen, dass alle ausser
                // Blank und Tab gehighlightet wird ?!
                r.nStart = nLastEnd;
            }
            nLastEnd = r.nEnd+1;
            if ( ( i == (nCount-1) ) && ( r.nEnd < rSource.Len() ) )
                r.nEnd = rSource.Len();
        }
    }

    svtools::ColorConfig aConfig;
    for ( USHORT i = 0; i < aPortionList.Count(); i++ )
    {
        SwTextPortion& r = aPortionList[i];
        if ( r.nStart > r.nEnd ) 	// Nur bis Bug von MD behoeben
            continue;
//		USHORT nCol = r.eType;
        if(r.eType !=  svtools::HTMLSGML    &&
            r.eType != svtools::HTMLCOMMENT &&
            r.eType != svtools::HTMLKEYWORD &&
            r.eType != svtools::HTMLUNKNOWN)
                r.eType = (svtools::ColorConfigEntry)svtools::HTMLUNKNOWN;
        Color aColor((ColorData)aConfig.GetColorValue((svtools::ColorConfigEntry)r.eType).nColor);
        USHORT nLine = nLineOff+r.nLine; //
        pTextEngine->SetAttrib( TextAttribFontColor( aColor ), nLine, r.nStart, r.nEnd+1 );
    }
}

IMPL_LINK( XMLFileWindow, SyntaxTimerHdl, Timer *, pTimer )
{
    Time aSyntaxCheckStart;
    DBG_ASSERT( pTextView, "Noch keine View, aber Syntax-Highlight ?!" );
    pTextEngine->SetUpdateMode( FALSE );

    bHighlighting = TRUE;
    USHORT nLine;
    USHORT nCount  = 0;
    // zuerst wird der Bereich um dem Cursor bearbeitet
    TextSelection aSel = pTextView->GetSelection();
    USHORT nCur = (USHORT)aSel.GetStart().GetPara();
    if(nCur > 40)
        nCur -= 40;
    else
        nCur = 0;
    if(aSyntaxLineTable.Count())
        for(USHORT i = 0; i < 80 && nCount < 40; i++, nCur++)
        {
            void * p = aSyntaxLineTable.Get(nCur);
            if(p)
            {
                DoSyntaxHighlight( nCur );
                aSyntaxLineTable.Remove( nCur );
                nCount++;
                if(!aSyntaxLineTable.Count())
                    break;
                if((Time().GetTime() - aSyntaxCheckStart.GetTime()) > MAX_HIGHLIGHTTIME )
                {
                    pTimer->SetTimeout( 2 * SYNTAX_HIGHLIGHT_TIMEOUT );
                    break;
                }
            }
        }

    // wenn dann noch etwas frei ist, wird von Beginn an weitergearbeitet
    void* p = aSyntaxLineTable.First();
    while ( p && nCount < MAX_SYNTAX_HIGHLIGHT)
    {
        nLine = (USHORT)aSyntaxLineTable.GetCurKey();
        DoSyntaxHighlight( nLine );
        USHORT nC = (USHORT)aSyntaxLineTable.GetCurKey();
        p = aSyntaxLineTable.Next();
        aSyntaxLineTable.Remove(nC);
        nCount ++;
        if(Time().GetTime() - aSyntaxCheckStart.GetTime() > MAX_HIGHLIGHTTIME)
        {
            pTimer->SetTimeout( 2 * SYNTAX_HIGHLIGHT_TIMEOUT );
            break;
        }
    }
    // os: #43050# hier wird ein TextView-Problem umpopelt:
    // waehrend des Highlightings funktionierte das Scrolling nicht
    TextView* pTmp = pTextEngine->GetActiveView();
    pTextEngine->SetActiveView(0);
    pTextEngine->SetUpdateMode( TRUE );
    pTextEngine->SetActiveView(pTmp);
    pTextView->ShowCursor(FALSE, FALSE);

    if(aSyntaxLineTable.Count() && !pTimer->IsActive())
        pTimer->Start();
    // SyntaxTimerHdl wird gerufen, wenn Text-Aenderung
    // => gute Gelegenheit, Textbreite zu ermitteln!
    long nPrevTextWidth = nCurTextWidth;
    nCurTextWidth = pTextEngine->CalcTextWidth() + 25;	// kleine Toleranz
    if ( nCurTextWidth != nPrevTextWidth )
        SetScrollBarRanges();
    bHighlighting = FALSE;

    return 0;
}

void XMLFileWindow::DoSyntaxHighlight( USHORT nPara )
{
    // Durch das DelayedSyntaxHighlight kann es passieren,
    // dass die Zeile nicht mehr existiert!
    if ( nPara < pTextEngine->GetParagraphCount() )
    {
        pTextEngine->RemoveAttribs( nPara );
        String aSource( pTextEngine->GetText( nPara ) );
        pTextEngine->SetUpdateMode( FALSE );
        ImpDoHighlight( aSource, nPara );
        // os: #43050# hier wird ein TextView-Problem umpopelt:
        // waehrend des Highlightings funktionierte das Scrolling nicht
        TextView* pTmp = pTextEngine->GetActiveView();
        pTmp->SetAutoScroll(FALSE);
        pTextEngine->SetActiveView(0);
        pTextEngine->SetUpdateMode( TRUE );
        pTextEngine->SetActiveView(pTmp);
        // Bug 72887 show the cursor
        pTmp->SetAutoScroll(TRUE);
        pTmp->ShowCursor( FALSE/*pTmp->IsAutoScroll()*/ );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
