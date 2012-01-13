'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
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
'*************************************************************************

*****

testcase tMakro_Dialog

    printlog( "Update test for the BASIC organizer - libraries, modules and dialogs" )

    const CFN = "tMakro_Dialog::"
    const CMACRO = "MacroDlg"
    const RC_TIMEOUT = -1
    const DIALOG_TIMEOUT = 2
    const TAB_MODULES = 1
    const TAB_DIALOGS = 2
    const TAB_LIBRARIES = 3
    const REFERENCE_LIBRARY = "AppLibrary1"

    dim iLibraryItemCount as integer
    dim iCurrentLibrary as Integer
    dim XLBFILE as String
        XLBFILE = gTesttoolPath & "framework\optional\input\AppLibrary1\script.xlb"
        XLBFILE = convertpath( XLBFILE )
    
    gApplication = "WRITER"

    printlog( "Open a new document" )
    hCreateDocument()
    
    printlog( "Create a new library, a new module, open Basic-Ide" )
    if ( not hInitBasicIde( CMACRO ) ) then
        warnlog( CFN & "Could not open BASIC-IDE, aborting" )
        goto endsub
    endif
    
    printlog( "Close the Basic-Ide" )
    kontext "BasicIDE"
    hCloseBasicIde()
    
    printlog( "Open the Basic Macro Organizer" )
    ToolsMacro_uno

    kontext "Makro"
    if ( Makro.exists( DIALOG_TIMEOUT ) ) then

        printlog( "Select the last Module for the current document" )
        hSelectTheLastNode( MakroAus )

        printlog( "Click Organize..." )
        Verwalten.Click()

        printlog( "Select Modules-tab" )
        hSelectBasicObjectOrganizerTab( TAB_MODULES )

        printlog( "Press 'new module'" )
        kontext "tabmodule"
        NeuesModul.Click()

        kontext "neuesmodul"
        printlog( "Cancel the dialog" )
        NeuesModul.Cancel()

        hSelectBasicObjectOrganizerTab( TAB_DIALOGS )

        kontext "tabdialogs"
        printlog( "Press 'new dialog'" )
        NewDialog.Click()

        printlog( "Cancel the dialog" )
        kontext "neuerdialog"
        NeuerDialog.Cancel()

        kontext
        printlog( "Select the libraries Tab" )
        hSelectBasicObjectOrganizerTab( TAB_LIBRARIES )


        printlog( "Press 'New' on the libraries tab" )
        kontext "tabbibliotheken"
        Neu.Click()

        kontext "neuebibliothek"
        if ( NeueBibliothek.exists( DIALOG_TIMEOUT ) ) then
            printlog( "Close the dialog with 'OK'" )
            Call DialogTest( NeueBibliothek )
            hCloseDialog( NeueBibliothek, "ok" )
        endif

        printlog( "Delete 'Library1'" )
        kontext "tabbibliotheken"
        Loeschen.Click()

        printlog( "Close the warning" )
        kontext "active"
        if ( Active.Exists( DIALOG_TIMEOUT ) ) then
            printlog( "MessageBox: " & active.getText() )
            Active.Yes()
        else
            warnlog( "Deletion warning is missing" )
        endif

        printlog( "Check the number of libraries in list" )
        kontext "tabbibliotheken"
        iLibraryItemCount = Bibliotheksliste.getItemCount()
        if ( iLibraryItemCount <> 1 ) then
            if ( iLibraryItemCount = 11 ) then
                warnlog( "#i87457# Macros moved from 'OpenOffice.org Macros' to 'My Macros'" )
            else
                warnlog( "Incorrect item count. Should be 1, is: " & iLibraryItemCount )
            endif
        endif

        printlog( "Append a xlb-file" )
        kontext "tabbibliotheken"
        Hinzufuegen.Click()

        printlog( "Select 'AppLibrary1/script.xlb' from the files list"
        kontext "oeffnendlg"
        Dateiname.SetText( XLBFILE )
        Oeffnen.Click()

        kontext "AppendLibraries"
        if ( AppendLibraries.exists( DIALOG_TIMEOUT ) ) then
            printlog( "<Append libraries> dialog is open, good" )
        else

            ' If the .xlb file cannot be found the test cannot continue. Warn and cleanup.
            kontext "Active"
            if ( Active.exists() ) then

                warnlog( "Unexpected messagebox, the script appears to be missing" )
                printlog( Active.getText() )

                kontext "Active"
                hCloseDialog( Active, "ok" )

                kontext "OeffnenDlg"
                hCloseDialog( OeffnenDlg, "cancel,optional" )

                kontext "TabBibliotheken"
                hCloseDialog( TabBibliotheken, "close,optional" )

                kontext "Makro"
                hCloseDialog( Makro, "close,optional" )

                printlog( "Exiting test" )
                goto endsub

            endif
        endif

        printlog( "Ensure 'Insert as Reference' is unchecked" )
        kontext "appendlibraries"
        InsertAsReference.UnCheck()

        printlog( "Check 'Replace existing library'" )
        ReplaceExistingLibraries.Check()

        printlog( "Press 'OK' to append the library" )
        kontext "appendlibraries"

        hCloseDialog( AppendLibraries, "ok" )

        printlog( "Check the number of libraries in list (should be two)" )
        kontext "tabbibliotheken"
        iLibraryItemCount = Bibliotheksliste.getItemCount()
        if ( iLibraryItemCount <> 2 ) then
            qaerrorlog( "#i87457# Two entries in list expected, found " & iLibraryItemCount )
        endif

        printlog( "Select the first item" )
        Bibliotheksliste.TypeKeys( "<HOME>" )
        for iCurrentLibrary = 1 to iLibraryItemCount
            if ( Bibliotheksliste.getSelText() = REFERENCE_LIBRARY ) then
                printlog( "Found " & REFERENCE_LIBRARY )
                exit for
            endif
            printlog( "This was not " & REFERENCE_LIBRARY & ", trying again" )
            Bibliotheksliste.Typekeys( "<DOWN>" )
        next iCurrentLibrary

        printlog( "Set a password for " & REFERENCE_LIBRARY )
        kontext "tabbibliotheken"
        if ( hClickButton( Passwort ) <> RC_TIMEOUT ) then
            kontext "passwddlg"
            hCloseDialog( PasswdDLG, "cancel" )
        else
            warnlog( "Password is disabled for " & REFERENCE_LIBRARY )
        endif

        printlog( "Delete " & REFERENCE_LIBRARY )
        kontext "tabbibliotheken"
        Loeschen.Click()

        printlog( "Close the warning with YES" )
        kontext "active"
        if ( Active.Exists() ) then
            printlog( "MessageBox: " & active.getText() )
            Active.Yes()
        else
            warnlog( "Expected confirmation dialog for delete" )
        endif

        kontext "tabbibliotheken"
        printlog( "Press 'edit' on 'Libraries'-tab" )
        Bearbeiten.Click()

        kontext "BasicIDE"
        if ( BasicIDE.Exists( DIALOG_TIMEOUT ) ) then

            hInsertMacro( 3 )

            printlog( "Close the basic-document" )
            hCloseBasicIde()

            kontext "active"
            if ( Active.Exists ( DIALOG_TIMEOUT ) ) then
                warnlog( "Unexpected MessageBox: " & active.getText() )
                Active.NO()
            endif

        else
            warnlog("#i38978# The Macro-Document is not open, aborting test")
            call hDestroyDocument()
            goto endsub
        endif

        printlog( "Open the BASIC Macro Organizer" )
        ToolsMacro_uno

        printlog( "Select the last macro from the list" )
        kontext "makro"
        if ( Makro.exists( DIALOG_TIMEOUT ) ) then
            hSelectTheLastNode( MakroAus )

            printlog( "Assign the macro, Tools/customize dialog should open" )
            if ( hClickButton( Zuordnen ) = RC_TIMEOUT ) then
                warnlog( "#i106853# Assign-Button is disabled or macro is missing" )
            endif

            printlog( "Activate all tapages and cancel the Tools/Customize dialog" )
            if ( not hToolsCustomizeSelectTab( "keyboard" ) ) then
                qaerrorlog( "#i61765# Keyboard tab missing when called via Tools/Macros->Assign" )
            endif
            hToolsCustomizeSelectTab( "events" )
            hToolsCustomizeSelectTab( "toolbars" )

            printlog( "Cancel Tools/Customize dialog -> Macro Organizer should be open" )
            kontext "TabCustomizeToolbars"
            hCloseDialog( TabCustomizeToolbars, "cancel" )


            printlog "Click to run macro"
            kontext "makro"
            if ( hClickButton( Ausfuehren ) = RC_TIMEOUT ) then
                warnlog( "Unable to run macro, button is not enabled" )
                kontext "Makro"
                hCloseDialog( Makro, "close" )
            endif

            kontext "active"
            if ( active.exists() ) then
                if ( active.getText() = "TTMacro3" ) then
                    printlog( "The correct macro has been executed" )
                else
                    warnlog( "Unknown Messagebox: " & active.getText() )
                endif
                Active.OK()
            endif
        else
            warnlog( "Macro Organizer did not open" )
        endif

        printlog( "Open macro organizer from the current document" )
        ToolsMacro_uno : WaitSlot()

        printlog( "Select the last item in the treelist" )
        kontext "Makro"
        hSelectTheLastNode( MakroAus )
        WaitSlot(2000)
        printlog( "Delete the new module" )
        Loeschen.Click()

        printlog( "Close the deletion warning" )
        kontext "active"
        if ( active.exists() ) then
            printlog( "MessageBox: " & active.getText() )
            Active.Yes()
        else
            warnlog( "No confirmation dialog for delete" )
        endif

        printlog( "Close the macro organizer" )
        kontext "makro"
        Makro.Close()
    else
        warnlog( "Makro Organizer did not open" )
    endif
    
    printlog( "Close the document" )
    Call hDestroyDocument()
    
endcase



