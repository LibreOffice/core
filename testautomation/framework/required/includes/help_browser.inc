'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: help_browser.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/******************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : global update/resource test
'*
'\******************************************************************************

testcase tHelp_DialogTest

    '///<h1>Update test for the main help window</h1>
    '///<ul>
    Dim i as Integer
    dim brc as boolean
    printlog "- open the help"

    '///+<li>Open Help</li>
    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test" )
        kontext "Active"
        if ( Active.exists( 2 ) ) then
        	printlog( "Msgbox: " & Active.getText() )
        	Active.ok()
        endif
        goto endsub
    endif


    '///+<li>activate 'Content'-page</li>
    brc = hSelectHelpTab( "content" )
    call DialogTest( ContentPage )

    '///+<li>activate 'Index'-page</li>
    brc = hSelectHelpTab( "index" )
    call DialogTest( IndexPage )

    '///+<li>activate 'Find'-page</li>
    brc = hSelectHelpTab( "find" )
    call DialogTest( FindPage )

    '///+<li>activate 'Bookmark'-page</li>
    brc = hSelectHelpTab( "bookmarks" )
    call DialogTest( BookmarksPage )

    '///+<li>close help ( does not work correctly with testtool )</li>
    Kontext "StarOfficeHelp"
    hCloseHelp()
    '///</ul>

endcase

'*******************************************************************************

testcase tHelp_ToolBar

    '///<h1>Update test for the toolbar on the main Help window</h1>
    '///<ul>
    Dim i as Integer
    dim brc as boolean

    '///+<li>Open Help</li>
    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test" )
        kontext "Active"
        if ( Active.exists( 2 ) ) then
        	printlog( "Msgbox: " & Active.getText() )
        	Active.ok()
        endif
        goto endsub
    endif


    kontext "starofficehelp"
    '///+<li>Index-button</li>
    printlog "- Index -> on"
    '///+<li>activate 'Index' ( => only the help-window must be shown )</li>
    Index.Click()
    waitslot

    printlog "- Index -> off"
    '///+<li>activate 'Index' ( => the full help-window must be shown )</li>
    Index.Click()

    ' Backward- and Forward-button
    '///+<li>open the 'Index'-page</li>
    printlog "- activate 'Index'-page"
    brc = hSelectHelpTab( "index" )
    sleep( 2 )

    SearchIndex.Select 4
    DisplayIndex.Click()
    waitslot

    '///+<li>backward</li>
    printlog "- backward"
    Backward.Click()
    waitslot

    '///+<li>forward</li>
    printlog "- forward"
    Forward.Click()
    waitslot

    '///+<li>first page</li>
    printlog "- first page"
    GoToStart.Click()

    '///+<li>print</li>
    printlog "- print"
    ' the print-dialog must be shown ( if direct-printing is activated => BUG )
    '///+<li>close the print-dialog</li>
    PrintButton.Click()

    kontext "active"

    if Active.Exists ( 1 ) then

        if Active.getRT() = 304 then
            qaerrorlog "- Printing failed: Did you define a default printer?"
            Active.OK()
            
            Kontext "DruckenDlg"
            if ( druckenDlg.exists() ) then
                printlog( "Printer dialog is still open, closing with cancel" )
                druckenDlg. cancel()
            endif
            
        endif

    else
        kontext "druckendlg"
        if DruckenDlg.NotExists (2) then
            warnlog "No print-dialog is open. => direct printing? - bug 93788!"
        else
            DruckenDlg.Cancel()
        endif

    endif
    
    '///+<li>Set Bookmark</li>
    printlog "- Set Bookmark"

    kontext "tb_help"
    SetBookmarks.Click()

    '///+<li>close the 'add bookmark'-dialog</li>
    kontext "addbookmark"
    call DialogTest ( AddBookmark )
    AddBookmark.Cancel()

    '///+<li>close help ( does not work correctly with testtool )</li>
    brc = hCloseHelp()
    '///</ul>

endcase
