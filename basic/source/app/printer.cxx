/*************************************************************************
 *
 *  $RCSfile: printer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:09 $
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
#include "ttstrhlp.hxx"
#endif

#include "app.hxx"
#include "printer.hxx"
#include "basic.hrc"
#include "resids.hrc"

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
    // 10-Punkt-Schrift verwenden
    Font aFont( FAMILY_MODERN, Size( 0, 10 ) );
    aFont.SetPitch( PITCH_FIXED );
    SetFont( aFont );
    // Ausgabe: 6 Zeilen/Zoll = 12 Punkt
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
    String aPage( ResId( IDS_PAGE ) );
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
    // Dialog einrichten
    ResId aResId( IDD_PRINT_DIALOG );
    pDlg = new PrintingDialog
          ( aBasicApp.pFrame, this, aResId, aFile );
    // Position des Dialogs setzen
    Size s1 = aBasicApp.pFrame->GetSizePixel();
    Size s2 = pDlg->GetSizePixel();
    pDlg->SetPosPixel( Point( (s1.Width() - s2.Width() )  / 2,
                                (s1.Height()- s2.Height() ) / 2 ) );
    // PRINT-Menu disablen
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
        xub_StrLen nLineEnd = min( rText.Search( '\n', nDone ), rText.Search( '\r', nDone ) );
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
    if( pDlg ) pDlg->Show();
    return 0;
}
IMPL_LINK_INLINE_END( BasicPrinter, StartPrintHdl, Printer *, pPrinter )

IMPL_LINK_INLINE_START( BasicPrinter, EndPrintHdl, Printer *, pPrinter )
{
    if( pDlg ) pDlg->Hide();
    return 0;
}
IMPL_LINK_INLINE_END( BasicPrinter, EndPrintHdl, Printer *, pPrinter )

IMPL_LINK_INLINE_START( BasicPrinter, PrintPageHdl, Printer *, pPrinter )
{
    if( pDlg ) pDlg->ChangeMessage( nPage );
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
             : ModelessDialog( pParent, rId ),
               aCancel( this, ResId( RID_CANCEL ) ),
               aText  ( this, ResId( RID_TEXT ) ),
               aName  ( rName )
{
    FreeResource();
    aCancel.SetClickHdl( LINK( pPrn, BasicPrinter, Abort ) );
}

void PrintingDialog::ChangeMessage( short nPage )
{
    String aMsg( ResId( IDS_PRINTMSG ) );
    aMsg += aName;
    aMsg += CUniString("\n");
    aMsg += String( ResId( IDS_PAGE ) );
    aMsg += String::CreateFromInt32( nPage );
    aText.SetText( aMsg );
}

