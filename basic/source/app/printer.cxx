/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


