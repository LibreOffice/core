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

    printlog( "Resource test for the Help Browser" )

    Dim i as Integer
    dim brc as boolean
    printlog "- open the help"

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


    brc = hSelectHelpTab( "content" )
    call DialogTest( ContentPage )

    brc = hSelectHelpTab( "index" )
    call DialogTest( IndexPage )

    brc = hSelectHelpTab( "find" )
    call DialogTest( FindPage )

    brc = hSelectHelpTab( "bookmarks" )
    call DialogTest( BookmarksPage )

    Kontext "StarOfficeHelp"
    hCloseHelp()

endcase

'*******************************************************************************

testcase tHelp_ToolBar

    Dim i as Integer
    dim brc as boolean

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
    printlog "- Index -> on"
    Index.Click()
    waitslot

    printlog "- Index -> off"
    Index.Click()

    ' Backward- and Forward-button
    printlog "- activate 'Index'-page"
    brc = hSelectHelpTab( "index" )
    sleep( 2 )

    SearchIndex.Select 4
    DisplayIndex.Click()
    waitslot

    printlog "- backward"
    Backward.Click()
    waitslot

    printlog "- forward"
    Forward.Click()
    waitslot

    printlog "- first page"
    GoToStart.Click()

    printlog "- print"
    ' the print-dialog must be shown ( if direct-printing is activated => BUG )
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
    
    printlog "- Set Bookmark"

    kontext "tb_help"
    SetBookmarks.Click()

    kontext "addbookmark"
    call DialogTest ( AddBookmark )
    AddBookmark.Cancel()

    brc = hCloseHelp()

endcase
