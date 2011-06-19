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
#include "precompiled_basic.hxx"
#include <tools/config.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/textview.hxx>
#include <svtools/texteng.hxx>
#include <svl/undo.hxx>

#include <basic/ttstrhlp.hxx>

#include "basic.hrc"
#include "appedit.hxx"
#include "brkpnts.hxx"

TYPEINIT1(AppEdit,AppWin);
AppEdit::AppEdit( BasicFrame* pParent )
: AppWin( pParent )
, pVScroll( NULL )
, pHScroll( NULL )
, nCurTextWidth(5)
{
    String aEmpty;
    // perhaps load the Untitled-String:

    pDataEdit = new TextEdit( this, WB_LEFT );
    LoadIniFile();

    pDataEdit->SetText( aEmpty );

    pDataEdit->Show();

    pVScroll = new ScrollBar( this, WB_VSCROLL|WB_DRAG );
    pVScroll->Show();
    pVScroll->SetScrollHdl( LINK( this, AppEdit, Scroll ) );
    pHScroll = new ScrollBar( this, WB_HSCROLL|WB_DRAG );
    pHScroll->Show();
    pHScroll->SetScrollHdl( LINK( this, AppEdit, Scroll ) );

    InitScrollBars();
}

AppEdit::~AppEdit()
{
  DataEdit *pHold = pDataEdit;
  pDataEdit = NULL;
  delete pHold;
  delete pHScroll;
  delete pVScroll;
}

void AppEdit::LoadIniFile()
{
    TextView *pTextView = ((TextEdit*)pDataEdit)->aEdit.pTextView;
    sal_Bool bWasModified = pTextView->GetTextEngine()->IsModified();
    pTextView->GetTextEngine()->SetModified( sal_False );

    FontList aFontList( pFrame );   // Just some Window is needed
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Misc");
    String aFontName = String( aConf.ReadKey( "ScriptFontName", "Courier" ), RTL_TEXTENCODING_UTF8 );
    String aFontStyle = String( aConf.ReadKey( "ScriptFontStyle", "normal" ), RTL_TEXTENCODING_UTF8 );
    String aFontSize = String( aConf.ReadKey( "ScriptFontSize", "12" ), RTL_TEXTENCODING_UTF8 );
    Font aFont = aFontList.Get( aFontName, aFontStyle );
    sal_uIntPtr nFontSize = aFontSize.ToInt32();
    aFont.SetHeight( nFontSize );

#if OSL_DEBUG_LEVEL > 1
    {
        Font aFont2( OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, Application::GetSettings().GetUILanguage(), 0, pFrame ));
    }
#endif
    aFont.SetTransparent( sal_False );
    pDataEdit->SetFont( aFont );

    if ( ((TextEdit*)pDataEdit)->GetBreakpointWindow() )
    {
        ((TextEdit*)pDataEdit)->GetBreakpointWindow()->SetFont( aFont );
        ((TextEdit*)pDataEdit)->GetBreakpointWindow()->Invalidate();
    }

    pTextView->GetTextEngine()->SetModified( bWasModified );    // Perhaps reset the flag
}

void AppEdit::Command( const CommandEvent& rCEvt )
{
    switch( rCEvt.GetCommand() ) {
        case COMMAND_WHEEL:
            {
                HandleScrollCommand( rCEvt, pHScroll, pVScroll );
            }
            break;
        default:
            AppWin::Command( rCEvt );
    }
}


IMPL_LINK( AppEdit, Scroll, ScrollBar*, pScroll )
{
    (void) pScroll; /* avoid warning about unused parameter */
    if ( !pHScroll || !pVScroll )
        return 0;

    TextView *pTextView = ((TextEdit*)pDataEdit)->aEdit.pTextView;
    pTextView->SetStartDocPos( Point( pHScroll->GetThumbPos(), pVScroll->GetThumbPos() ) );
    pTextView->Invalidate();

    if ( ((TextEdit*)pDataEdit)->GetBreakpointWindow() )
        ((TextEdit*)pDataEdit)->GetBreakpointWindow()->Scroll( 0, ((TextEdit*)pDataEdit)->GetBreakpointWindow()->GetCurYOffset() - pTextView->GetStartDocPos().Y() );

    return 0L;
}


void AppEdit::InitScrollBars()
{
    if ( !pHScroll || !pVScroll )
        return;

    TextView *pTextView = ((TextEdit*)pDataEdit)->aEdit.pTextView;

    SetScrollBarRanges();

    Size aOutSz( pTextView->GetWindow()->GetOutputSizePixel() );
    pVScroll->SetVisibleSize( aOutSz.Height() );
    pVScroll->SetPageSize( aOutSz.Height() * 8 / 10 );
    pVScroll->SetLineSize( GetTextHeight() +2 );    // +2 is empirical. don't know why
    pVScroll->SetThumbPos( pTextView->GetStartDocPos().Y() );
    pVScroll->Show();

    pHScroll->SetVisibleSize( aOutSz.Width() );
    pHScroll->SetPageSize( aOutSz.Width() * 8 / 10 );
    pHScroll->SetLineSize( GetTextWidth( CUniString("x") ) );
    pHScroll->SetThumbPos( pTextView->GetStartDocPos().X() );
    pHScroll->Show();
}

void AppEdit::SetScrollBarRanges()
{
  // Extra-Method, not InitScrollBars, but for EditEngine-Events.

  if ( !pHScroll || !pVScroll )
    return;

  pHScroll->SetRange( Range( 0, nCurTextWidth ) );
  pVScroll->SetRange( Range( 0, ((TextEdit*)pDataEdit)->aEdit.pTextEngine->GetTextHeight() ) );
}



sal_uInt16 AppEdit::GetLineNr()
{
  return pDataEdit->GetLineNr();
}

FileType AppEdit::GetFileType()
{
  return FT_BASIC_SOURCE;
}

// Set up the menu
long AppEdit::InitMenu( Menu* pMenu )
{
    AppWin::InitMenu (pMenu );

    if( pDataEdit )
    {
        size_t UndoCount = ((TextEdit*)pDataEdit)->aEdit.pTextEngine->GetUndoManager().GetUndoActionCount();
        size_t RedoCount = ((TextEdit*)pDataEdit)->aEdit.pTextEngine->GetUndoManager().GetRedoActionCount();

        pMenu->EnableItem( RID_EDITUNDO,    UndoCount > 0 );
        pMenu->EnableItem( RID_EDITREDO,    RedoCount > 0 );
    }

    return sal_True;
}

long AppEdit::DeInitMenu( Menu* pMenu )
{
    AppWin::DeInitMenu (pMenu );

    pMenu->EnableItem( RID_EDITUNDO );
    pMenu->EnableItem( RID_EDITREDO );

    return sal_True;
}

void AppEdit::Resize()
{
    if( !pDataEdit )
        return;

    Point rHStart,rVStart;
    Size rHSize,rVSize;
    Size rNewSize( GetOutputSizePixel() );

    if ( pHScroll )
    {
        rHSize = pHScroll->GetSizePixel();
        sal_uIntPtr nHieght = rHSize.Height();
        rNewSize.Height() -= nHieght;
        rHStart.Y() = rNewSize.Height();
    }

    if ( pVScroll )
    {
        rVSize = pVScroll->GetSizePixel();
        sal_uIntPtr nWidth = rVSize.Width();
        rNewSize.Width() -= nWidth;
        rVStart.X() = rNewSize.Width();
    }

    rHSize.Width() = rNewSize.Width();
    rVSize.Height() = rNewSize.Height();

    if ( pHScroll )
    {
        pHScroll->SetPosPixel( rHStart );
        pHScroll->SetSizePixel( rHSize );
    }

    if ( pVScroll )
    {
        pVScroll->SetPosPixel( rVStart );
        pVScroll->SetSizePixel( rVSize );
    }
    pDataEdit->SetPosPixel( Point() );
    pDataEdit->SetSizePixel( rNewSize );


    TextView *pTextView = ((TextEdit*)pDataEdit)->aEdit.pTextView;
    long nVisY = pTextView->GetStartDocPos().Y();
    pTextView->ShowCursor();
    Size aOutSz( pTextView->GetWindow()->GetOutputSizePixel() );
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
    InitScrollBars();
    if ( nVisY != pTextView->GetStartDocPos().Y() )
        pTextView->GetWindow()->Invalidate();

}

void AppEdit::PostLoad()
{
}

void AppEdit::PostSaveAs()
{
}

void AppEdit::Highlight( sal_uInt16 nLine, sal_uInt16 nCol1, sal_uInt16 nCol2 )
{
    ((TextEdit*)pDataEdit)->Highlight( nLine, nCol1, nCol2 );
    ToTop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
