/*************************************************************************
 *
 *  $RCSfile: appedit.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:08 $
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

#ifndef _TEXTVIEW_HXX //autogen
#include <svtools/textview.hxx>
#endif

#ifndef _TEXTENG_HXX //autogen
#include <svtools/texteng.hxx>
#endif

#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif

#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif

#include "basic.hrc"
#include "appedit.hxx"
#include "brkpnts.hxx"

TYPEINIT1(AppEdit,AppWin);
AppEdit::AppEdit( BasicFrame* pParent )
: AppWin( pParent )
, nCurTextWidth(5)
, pVScroll( NULL )
, pHScroll( NULL )
{
    String aEmpty;
    // evtl. den Untitled-String laden:

    pDataEdit = new TextEdit( this, WB_LEFT );
    // Icon definieren:
//  pIcon = new Icon( ResId( RID_WORKICON ) );
//  if( pIcon ) SetIcon( *pIcon );

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
    pDataEdit = NULL;       // Erst abklemmen, dann löschen
    delete pHold;
    delete pHScroll;
    delete pVScroll;
}

IMPL_LINK( AppEdit, Scroll, ScrollBar*, pScroll )
{
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
// Kopiert und angepasst.
    SetScrollBarRanges();

    Size aOutSz( pTextView->GetWindow()->GetOutputSizePixel() );
    pVScroll->SetVisibleSize( aOutSz.Height() );
    pVScroll->SetPageSize( aOutSz.Height() * 8 / 10 );
    pVScroll->SetLineSize( GetTextHeight() );
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
    // Extra-Methode, nicht InitScrollBars, da auch fuer EditEngine-Events.

    if ( !pHScroll || !pVScroll )
        return;

    pHScroll->SetRange( Range( 0, nCurTextWidth ) );

    pVScroll->SetRange( Range( 0, ((TextEdit*)pDataEdit)->aEdit.pTextEngine->GetTextHeight() ) );
}



USHORT AppEdit::GetLineNr(){ return pDataEdit->GetLineNr(); }

FileType AppEdit::GetFileType()
{
    return FT_BASIC_SOURCE;
}

// Set up the menu
long AppEdit::InitMenu( Menu* pMenu )
{
    AppWin::InitMenu (pMenu );

    USHORT UndoCount = ((TextEdit*)pDataEdit)->aEdit.pTextEngine->GetUndoManager().GetUndoActionCount();
    USHORT RedoCount = ((TextEdit*)pDataEdit)->aEdit.pTextEngine->GetUndoManager().GetRedoActionCount();

    pMenu->EnableItem( RID_EDITUNDO,    UndoCount > 0 );
    pMenu->EnableItem( RID_EDITREDO,    RedoCount > 0 );

    return TRUE;
}

long AppEdit::DeInitMenu( Menu* pMenu )
{
    AppWin::DeInitMenu (pMenu );

    pMenu->EnableItem( RID_EDITUNDO );
    pMenu->EnableItem( RID_EDITREDO );

    return TRUE;
}

// Sourcecode-Datei laden

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
        ULONG nHieght = rHSize.Height();
        rNewSize.Height() -= nHieght;
        rHStart.Y() = rNewSize.Height();
    }

    if ( pVScroll )
    {
        rVSize = pVScroll->GetSizePixel();
        ULONG nWidth = rVSize.Width();
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
// Kopiert und adaptiert
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
//              pModulWindow->GetBreakPointWindow().GetCurYOffset() = aStartDocPos.Y();
    }
    InitScrollBars();
    if ( nVisY != pTextView->GetStartDocPos().Y() )
        pTextView->GetWindow()->Invalidate();

}

void AppEdit::PostLoad()
{
}

// mit neuem Namen speichern

void AppEdit::PostSaveAs()
{
}

void AppEdit::Highlight( USHORT nLine, USHORT nCol1, USHORT nCol2 )
{
    ((TextEdit*)pDataEdit)->Highlight( nLine, nCol1, nCol2 );
    ToTop();
}

