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

#include <vcl/print.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <basic/ttstrhlp.hxx>

#include <algorithm>

#include "app.hxx"
#include "printer.hxx"
#include "basic.hrc"
#include "resids.hrc"
#include "basrid.hxx"

class PrintingDialog : public ModelessDialog {
    String aName;
    FixedText aText;
    CancelButton aCancel;
public:
    PrintingDialog( Window*, BasicPrinter*, ResId&, String& );
    void ChangeMessage( short );
};

BasicPrinter::BasicPrinter() : mpPrinter( new Printer() )
{
    nPage = 0; nLine = 9999;
    mpPrinter->SetMapMode( MapMode( MAP_POINT ) );
    Size s( mpPrinter->GetOutputSize() );
    // Use 10 point font
    Font aFont( FAMILY_MODERN, Size( 0, 10 ) );
    aFont.SetPitch( PITCH_FIXED );
    mpPrinter->SetFont( aFont );
    // Output: 6 Lines/Inch = 12 Point
    nLines = (short) s.Height() / 12;
    nYoff  = 12;
}

void BasicPrinter::Header()
{
    if( nPage ) mpListener->EndPage();
    nPage++;
    mpListener->StartPage();
    String aHdr;
    String aPage( SttResId( IDS_PAGE ) );
    aPage.Append( String::CreateFromInt32(nPage) );
    aHdr = aFile.Copy( 0, 80 - aPage.Len() );
    aHdr.Expand( 80 - aPage.Len(), ' ' );
    aHdr += aPage;
    mpPrinter->DrawText( Point( 0, 0 ), aHdr );
    nLine = 2;
}

void BasicPrinter::Print( const String& rFile, const String& rText, BasicFrame *pFrame )
{
    nPage = 0; nLine = 9999;
    aFile = rFile;
    // Disable PRINT-Menu
    MenuBar* pBar = pFrame->GetMenuBar();
    Menu* pFileMenu = pBar->GetPopupMenu( RID_APPFILE );
    pFileMenu->EnableItem( RID_FILEPRINT, sal_False );

    mpListener.reset( new vcl::OldStylePrintAdaptor( mpPrinter ) );
    mpListener->StartPage();
    xub_StrLen nDone=0;
    while( nDone < rText.Len() )
    {
        if( nLine >= nLines ) Header();
        xub_StrLen nLineEnd = std::min( rText.Search( '\n', nDone ), rText.Search( '\r', nDone ) );
        mpPrinter->DrawText( Point( 0, nLine * nYoff ), rText, nDone, nLineEnd-nDone-1 );
        nDone = nLineEnd;
        if( nDone <= rText.Len() && rText.GetChar(nDone) == '\r' ) nDone++;
        if( nDone <= rText.Len() && rText.GetChar(nDone) == '\n' ) nDone++;
        nLine++;
    }
    mpListener->EndPage();

    Printer::PrintJob( mpListener, mpPrinter->GetJobSetup() );
    nPage = 1;
    pFileMenu->EnableItem( RID_FILEPRINT, sal_True );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
