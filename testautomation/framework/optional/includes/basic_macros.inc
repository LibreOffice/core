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
' **
' ** owner : joerg.skottke@sun.com
' **
' ** short description : check macro-dialogs / BASIC-IDE ( 1. part )
' **
'\******************************************************************************


testcase tMakro_In_All_Apps

    printlog( "Attach a macro to all application types" )
    
    dim cApp as string
    dim iApp as Integer
    
    const CMACRO = "AppMakro"

    for iApp = 1 to 7

        cApp = hNumericDoctype( iApp )
        printlog( cApp )

        printlog( "Open a new document" )
        hCreateDocument()
        
        if ( not hInitBasicIde( CMACRO ) ) then
            warnlog( "Could not open the BASIC Macro Organizer, aborting" )
            call hDestroyDocument()
            goto endsub
        endif
        
        printlog "Close the BASIC-IDE"
        hCloseBasicIde()
        
        printlog( "Close the navigator (Master-doc)" )
        kontext "Navigator"
        hCloseDialog( Navigator, "close,optional" )
        
        printlog( "Close the document" )
        hUseAsyncSlot( "FileClose" )

        printlog( "Close the messagebox (document changed)" )
        kontext "messagebox"
        if ( MessageBox.Exists( 1 ) ) then
            printlog( "MessageBox: " & MessageBox.getText() )
            hCloseDialog( Messagebox, "no" )
        else
            warnlog( "No warning for changed document" )
        end if

    next iApp
    
endcase

'*******************************************************************************

testcase tMakro_Dialog

    dim iPos as integer
    dim iLibraryItemCount as integer
    
    dim iCurrentLibrary as Integer
    dim XLBFILE as String
        XLBFILE = gTesttoolPath & "framework\optional\input\AppLibrary1\script.xlb"
    const CFN = "tMakro_Dialog::"
    const CMACRO = "MacroDlg"
    
    
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
    ToolsMacro_uno : WaitSlot()

    kontext "Makro"
    printlog( "Select the last Module for the current document" )
    iPos = hSelectTheLastNode( MakroAus )
    
    printlog( "Click Organize..." )
    Verwalten.Click()
    
    printlog( "Select Modules-tab" )
    hSelectBasicObjectOrganizerTab( 1 )
    
    printlog( "Press 'new module'" )
    kontext "tabmodule"
    NeuesModul.Click()

    kontext "neuesmodul"
    printlog( "Cancel the dialog" )
    NeuesModul.Cancel()

    hSelectBasicObjectOrganizerTab( 2 )

    kontext "tabdialogs"
    printlog( "Press 'new dialog'" )
    NewDialog.Click()
       
    printlog( "Cancel the dialog" )
    kontext "neuerdialog"
    NeuerDialog.Cancel()

    kontext
    printlog( "Select the libraries Tab" )
    hSelectBasicObjectOrganizerTab( 3 )

    
    printlog( "Press 'New' on the libraries tab" )
    kontext "tabbibliotheken"
    Neu.Click()

    kontext "neuebibliothek"
    printlog( "Close the dialog with 'OK'" )
    Call DialogTest( NeueBibliothek )
    NeueBibliothek.OK()
    
    printlog( "Delete 'Library1'" )
    kontext "tabbibliotheken"
    Loeschen.Click()
    
    printlog( "Close the warning" )
    kontext "active"
    if ( Active.Exists() ) then
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
    Dateiname.SetText( ConvertPath ( XLBFILE ) )
    Oeffnen.Click()
    
    ' If the .xlb file cannot be found the test cannot continue. Warn and cleanup.
    kontext "Active"
    if ( Active.exists( 1 ) ) then
    
        try
            Active.getButtonCount()
            
            warnlog( "Script does not exist warning displayed. Aborting test" )
            printlog( Active.getText() )
            active.ok()
            
            kontext "OeffnenDlg"
            if ( OeffnenDlg.exists( 1 ) ) then
                printlog( "Closing File Open dialog" )
                OeffnenDlg.cancel()
            endif
            
            kontext "tabbibliotheken"
            if ( TabBibliotheken.exists( 1 ) ) then
                printlog( "Closing Libaries tab" )
                TabBibliotheken.close()
            endif
            
            kontext "Makro"
            if ( Makro.exists( 1 ) ) then
                printlog( "Closing Macro organizer" )
                Makro.close()
            endif
            
            printlog( "Exiting test" )
            goto endsub
        catch
            printlog( "Script found and loaded. Good." )
        endcatch
        
    endif

    printlog( "Ensure 'Insert as Reference' is unchecked" )
    kontext "appendlibraries"
    InsertAsReference.UnCheck()

    printlog( "Check 'Replace existing library'" )
    ReplaceExistingLibraries.Check()

    printlog( "Press 'OK' to append the library" )
    kontext "appendlibraries"
    try
        AppendLibraries.OK()
    catch
        warnlog( "#i85254# Appending libraries leads to crash" )
    endcatch

    printlog( "Check the number of libraries in list (should be two)" )
    kontext "tabbibliotheken"
    iLibraryItemCount = Bibliotheksliste.getItemCount()
    if ( iLibraryItemCount <> 2 ) then
        qaerrorlog( "#i87457# Two entries in list expected, found " & iLibraryItemCount )
    endif

    printlog( "Select the first item" )
    Bibliotheksliste.TypeKeys( "<HOME>" )
    for iCurrentLibrary = 1 to iLibraryItemCount 
        if ( Bibliotheksliste.getSelText() = "AppLibrary1" ) then
            printlog( "Found AppLibrary1" )
            exit for
        endif
        printlog( "This was not AppLibrary1, trying again" )
        Bibliotheksliste.Typekeys( "<DOWN>" )
    next iCurrentLibrary
    
    printlog( "Set a password for AppLibrary1" )
    kontext "tabbibliotheken"
    if ( Passwort.IsEnabled() ) then
        Passwort.Click()
        kontext "passwddlg"
        PasswdDLG.Cancel()
    else
        warnlog( "Password is disabled for 'AppLibrary1'" )
    endif

    printlog( "Delete 'AppLibrary1'" )
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
    if ( BasicIDE.Exists( 5 ) ) then
    
        hInsertMacro( 3 )
    
        printlog( "Close the basic-document" )
        hCloseBasicIde()

        kontext "active"
        if ( Active.Exists ( 2 ) ) then
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
    hSelectTheLastNode( MakroAus )
    
    printlog( "Assign the macro, Tools/customize dialog should open" )
    if ( not hClickButton( Zuordnen ) ) then
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
    try
        Ausfuehren.Click()
    catch
        warnlog( "Unable to run macro" )
        kontext "Makro"
        Makro.close()
    endcatch
    
    kontext "active"
    if ( active.exists() ) then
        if ( active.getText() = "TTMacro3" ) then
            printlog( "The correct macro has been executed" )
        else
            warnlog( "Unknown Messagebox: " & active.getText() )
        endif
        Active.OK()
    endif
    
    printlog( "Open macro organizer from the current document" )
    ToolsMacro_uno
    
    printlog( "Select the last item in the treelist" )
    hSelectTheLastNode( MakroAus )

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
    
    printlog( "Close the document" )
    Call hDestroyDocument()
    
endcase



