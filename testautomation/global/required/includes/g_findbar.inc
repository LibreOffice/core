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
'/************************************************************************
'*
'* owner : gregor.hartmann@oracle.com
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