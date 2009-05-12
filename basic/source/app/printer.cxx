/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: printer.cxx,v $
 * $Revision: 1.10 $
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

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

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

BasicPrinter::BasicPrinter() : Printer()
{
    nPage = 0; nLine = 9999;
    SetMapMode( MapMode( MAP_POINT ) );
    Size s( GetOutputSize() );
    // Use 10 point font
    Font aFont( FAMILY_MODERN, Size( 0, 10 ) );
    aFont.SetPitch( PITCH_FIXED );
    SetFont( aFont );
    // Output: 6 Lines/Inch = 12 Point
    nLines = (short) s.Height() / 12;
    nYoff  = 12;
    SetStartPrintHdl( LINK( this, BasicPrinter, StartPrintHdl ) );
    SetEndPrintHdl( LINK( this, BasicPrinter, EndPrintHdl ) );
    SetPrintPageHdl( LINK( this, BasicPrinter, PrintPageHdl ) );
}

void BasicPrinter::Header()
{
    if( nPage ) EndPage();
    nPage++;
    StartPage();
    String aHdr;
    String aPage( SttResId( IDS_PAGE ) );
    aPage.Append( String::CreateFromInt32(nPage) );
    aHdr = aFile.Copy( 0, 80 - aPage.Len() );
    aHdr.Expand( 80 - aPage.Len(), ' ' );
    aHdr += aPage;
    DrawText( Point( 0, 0 ), aHdr );
    nLine = 2;
}

void BasicPrinter::Print( const String& rFile, const String& rText, BasicFrame *pFrame )
{
    nPage = 0; nLine = 9999;
    aFile = rFile;
    // Setup dialog
    SttResId aResId( IDD_PRINT_DIALOG );
    pDlg = new PrintingDialog
          ( aBasicApp.pFrame, this, aResId, aFile );
    // Set position of dialog
    Size s1 = aBasicApp.pFrame->GetSizePixel();
    Size s2 = pDlg->GetSizePixel();
    pDlg->SetPosPixel( Point( (s1.Width() - s2.Width() )  / 2,
                                (s1.Height()- s2.Height() ) / 2 ) );
    // Disable PRINT-Menu
    MenuBar* pBar = pFrame->GetMenuBar();
    Menu* pFileMenu = pBar->GetPopupMenu( RID_APPFILE );
    pFileMenu->EnableItem( RID_FILEPRINT, FALSE );

    pDlg->ChangeMessage( 1 );
    pDlg->Show();
    StartJob( rFile );
    StartPage();
    xub_StrLen nDone=0;
    while( nDone < rText.Len() )
    {
        if( nLine >= nLines ) Header();
        xub_StrLen nLineEnd = std::min( rText.Search( '\n', nDone ), rText.Search( '\r', nDone ) );
        DrawText( Point( 0, nLine * nYoff ), rText, nDone, nLineEnd-nDone-1 );
        nDone = nLineEnd;
        if( nDone <= rText.Len() && rText.GetChar(nDone) == '\r' ) nDone++;
        if( nDone <= rText.Len() && rText.GetChar(nDone) == '\n' ) nDone++;
        nLine++;
        Application::Reschedule();
    }
    EndPage();
    EndJob();
    nPage = 1;
    while( IsPrinting() ) Application::Reschedule();
    delete pDlg; pDlg = NULL;
    pFileMenu->EnableItem( RID_FILEPRINT, TRUE );
}

IMPL_LINK_INLINE_START( BasicPrinter, StartPrintHdl, Printer *, pPrinter )
{
    (void) pPrinter; /* avoid warning about unused parameter */
    if( pDlg != NULL )
        pDlg->Show();
    return 0;
}
IMPL_LINK_INLINE_END( BasicPrinter, StartPrintHdl, Printer *, pPrinter )

IMPL_LINK_INLINE_START( BasicPrinter, EndPrintHdl, Printer *, pPrinter )
{
    (void) pPrinter; /* avoid warning about unused parameter */
    if( pDlg != NULL)
       pDlg->Hide();
    return 0;
}
IMPL_LINK_INLINE_END( BasicPrinter, EndPrintHdl, Printer *, pPrinter )

IMPL_LINK_INLINE_START( BasicPrinter, PrintPageHdl, Printer *, pPrinter )
{
    (void) pPrinter; /* avoid warning about unused parameter */
    if( pDlg != NULL)
        pDlg->ChangeMessage( nPage );
    return 0;
}
IMPL_LINK_INLINE_END( BasicPrinter, PrintPageHdl, Printer *, pPrinter )

IMPL_LINK_INLINE( BasicPrinter, Abort , void *, EMPTYARG,
{
    AbortJob();
    return 0L;
}
)

/////////////////////////////////////////////////////////////////////////

PrintingDialog::PrintingDialog
              ( Window* pParent, BasicPrinter* pPrn, ResId& rId, String& rName )
: ModelessDialog( pParent, rId )
, aName  ( rName )
, aText  ( this, ResId( RID_TEXT, *rId.GetResMgr() ) )
, aCancel( this, ResId( RID_CANCEL, *rId.GetResMgr() ) )
{
    FreeResource();
    aCancel.SetClickHdl( LINK( pPrn, BasicPrinter, Abort ) );
}

void PrintingDialog::ChangeMessage( short nPage )
{
    String aMsg( SttResId( IDS_PRINTMSG ) );
    aMsg += aName;
    aMsg += CUniString("\n");
    aMsg += String( SttResId( IDS_PAGE ) );
    aMsg += String::CreateFromInt32( nPage );
    aText.SetText( aMsg );
}

