'encoding UTF-8  Do not remove or change this line!
'**************************************************************
'  
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'  
'**************************************************************
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

