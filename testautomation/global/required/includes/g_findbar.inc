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
'* short description : Update test for the findbar
'*
'\***********************************************************************

testcase tUpdtFindBar()

    ' This test is incomplete. It needs to be integrated into all update tests
    ' and it needs to be added to the toolbar management functions

    printlog( "Update test for the findbar" )

    const TEST_STRING = "VCL Testtool"

    select case( gApplication )
    case "IMPRESS", "DRAW", "DATABASE"
        kontext "FindBar"
        if ( FindBar.exists() ) then
            warnlog( "The FindBar should not be visible by default" )
        endif
    end select

    hNewDocument()

    kontext "FindBar"
    if ( FindBar.exists() ) then

        printlog( "FindBar has " & FindBar.getItemCount() & " items" )

        printlog( "Insert text into search field" )
        FindText.setText( TEST_STRING )

        printlog( "Click <Find next> (down arrow)" )
        DownSearch.click()

        kontext "Active"
        if ( Active.exists( 1 ) ) then
            printlog( "Expected message: " & Active.getText()
            call DialogTest( Active )

            kontext "Active"
            Active.ok()
        else
            warnlog( "Messagebox missing <No matches found>" )
        endif

        printlog( "Click <Find last> (up arrow)" )
        kontext "FindBar"
        UpSearch.click()

        kontext "Active"
        if ( Active.exists( 1 ) ) then
            printlog( "Expected message: " & Active.getText()
            call DialogTest( Active )

            kontext "Active"
            Active.ok()
        else
            warnlog( "Messagebox missing <No matches found>" )
        endif

        qaerrorlog( "#i111984 - exclude <SearchDialog> button from testing" )
        goto skip_SearchDialog

            printlog( "Search dialog button may not be visible by default" )
            kontext "FindBar"
            if ( SearchDialog.exists() ) then
                if ( SearchDialog.isVisible() ) then
                    SearchDialog.click()
                    warnlog( "Button <Search Dialog> is visible, it should be hidden by default" )
                else
                    printlog( "Button <Search Dialog> is hidden, good" )
                endif
            else
                warnlog( "<Search Dialog> button should not be visible" )
            endif

            printlog( "Add <Search Dialog> button to toolbar" )
            hToggleSearchDialog()

            kontext "FindBar"
            SearchDialog.click()

            kontext "FindAndReplace"
            if ( FindAndReplace.exists( 1 ) ) then
                call DialogTest( FindAndReplace )

                kontext "FindAndReplace"
                if ( SearchFor.getSelText() = TEST_STRING ) then
                    printlog( "Test string has been inserted into <SearchFor> Listbox. Good." )
                else
                    warnlog( "Search string should have been copied from findbar to search dialog" )
                endif
                FindAndReplace.close()
            else
                warnlog( "Dialog <Search And Replace> is missing" )
            endif

            printlog( "Remove <Search Dialog> button from toolbar" )
            hToggleSearchDialog()
            
        skip_SearchDialog:

    else
        warnlog( "FindBar is not visible" )
    endif

    hCloseDocument()

endcase

'*******************************************************************************

function hToggleSearchDialog()

    printlog( "Make button <Search Dialog> visible via context menu" )

    const MENU_VISIBLE_BUTTONS = 1
    const MENU_SEARCH_AND_REPLACE = 4

    kontext "FindBar"
    FindBar.openContextMenu()
    wait( 1000 )

    hMenuSelectNr( MENU_VISIBLE_BUTTONS )
    wait( 1000 )

    hMenuSelectNr( MENU_SEARCH_AND_REPLACE )
    wait( 1000 )

end function