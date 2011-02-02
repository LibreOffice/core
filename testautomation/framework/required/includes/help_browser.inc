'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/******************************************************************************
'*
'* owner : gregor.hartmann@oracle.com
'*
'* short description : global update/resource test
'*
'\******************************************************************************

testcase tHelp_DialogTest

    printlog( "Resource test for the Help Browser" )

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

    hSelectHelpTab( "content" )
    hSelectHelpTab( "index" )
    hSelectHelpTab( "find" )
    hSelectHelpTab( "bookmarks" )
    

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

    printlog( "Resource test for the help toolbar" )
    
    dim brc as boolean
    dim iExitCounter as integer : iExitCounter = 0

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
    
    do while( SearchIndex.getItemCount() < 100 )
        iExitCounter = iExitCounter + 1 : if ( iExitCounter = 20 ) then exit do
        wait( 100 )
    loop

    SearchIndex.Select( 4 )
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
    if ( Active.Exists ( 1 ) ) then
        qaerrorlog( "Unexpected messagebox: " & Active.getText() )
        qaerrorlog( "Probably missing default printer on this system, fix it!" )
        Active.OK()
    endif
    
    kontext "Printing"
    if ( Printing.exists( 2 ) ) then
        printlog( "Print dialog open, good." )
        Printing.cancel()
    else
        warnlog( "Print dialog not open" )
    endif
    
    kontext "tb_help"
    printlog "- Set Bookmark"
    SetBookmarks.Click()

    kontext "AddBookmark"
    call DialogTest ( AddBookmark )
    AddBookmark.Cancel()

    brc = hCloseHelp()

endcase

'*******************************************************************************

testcase tHelp_Registration

    ' Check i69670 which was a showstopper in OOo 2.0.4
    ' Menu-entry "Help / Registration" is disabled

    dim iCurrentMenuEntry as integer
    dim iMenuEntryCount as integer

    printlog "Check if all entries in Help Menu are enabled"
    call hNewDocument

    printlog "Open menu"
    hUseMenu()
    iMenuEntryCount = hMenuItemGetCount

    printlog "Select the last entry 'Help'"
    hMenuSelectNr( iMenuEntryCount )
    iMenuEntryCount = hMenuItemGetCount
    for iCurrentMenuEntry = 1 to iMenuEntryCount
        if ( hMenuItemIsEnabled( iCurrentMenuEntry ) ) then
            printlog "(" & iCurrentMenuEntry & "/" & iMenuEntryCount & _
            "): Menu entry is enabled:     Help-> " & hMenuItemGetText( iCurrentMenuEntry )
        else
	        if (lcase(gPlatform) = "osx") then
	            warnlog "#i86247# Help->Registration is disabled on MacOS X"
	        else
                warnlog "(" & iCurrentMenuEntry & "/" & iMenuEntryCount & _
                "): Menu entry is not enabled: Help-> " & hMenuItemGetText( iCurrentMenuEntry )
	        endif
        endif
    next iCurrentMenuEntry
    hMenuClose()
    call hCloseDocument

endcase

