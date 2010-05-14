'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_macros.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'/************************************************************************
' **
' ** owner : joerg.skottke@sun.com
' **
' ** short description : check macro-dialogs / BASIC-IDE ( 1. part )
' **
'\******************************************************************************


testcase tMakro_In_All_Apps

    '///<h1>Create a module for all applications, open and close Basic-IDE</h1>

    dim brc as boolean
    dim cApp as string
    dim iApp as Integer
    dim bBug as Boolean
    dim cMsg as string
    bBug = FALSE
    
    const CMACRO = "AppMakro"
    
    '///<ul>

    for iApp = 1 to 7

        cApp = hNumericDoctype( iApp )
        printlog( cApp )

        '///+<li>Open a new document</li>
        printlog( "Open a new document" )
        brc = hCreateDocument()
        
        '///+<li>Open the BASIC Macro Organizer</li>
        '///+<li>Create a new library for the current document</li>
        '///+<li>Create a new module for the current document<br>
        '///+ -&gt; The Basic IDE opens</li>
        brc = hInitBasicIde( CMACRO )
        if ( not brc ) then
            warnlog( "Could not open the BASIC Macro Organizer, aborting" )
            call hDestroyDocument()
            goto endsub
        endif
        
        '///+<li>Close the BASIC-IDE</li>
        printlog "Close the BASIC-IDE"
        brc = hCloseBasicIde()
        
        '///+<li>Close the navigator (Master-Doc)</li>
        printlog( "Close the navigator (Master-doc)" )
        brc = hCloseNavigator()
        
        '///+<li>Close the document</li>
        printlog( "Close the document" )
        FileClose

        '///+<li>Close the messagebox (document changed)</li>
        printlog( "Close the messagebox (document changed)" )
        kontext "messagebox"
        if ( MessageBox.Exists( 1 ) ) then
            cMsg = MessageBox.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            printlog( "MessageBox: " & cMsg )
            MessageBox.No()
        else
            warnlog( "No warning for changed document" )
        end if

    next iApp
    '///</ul>
    
endcase

'*******************************************************************************

testcase tMakro_Dialog

    if ( gBuild < 9305 ) then
        warnlog( "#i87457# - Application macros assigned to userlayer" )
        goto endsub
    endif

    '///<h1>Dialogs: Macro Object Organizer, Tools/Customize</h1>
    '///<ul>
    
    dim brc as boolean
    dim cMsg as string
    
    dim iPos as integer
    dim iLibraryItemCount as integer
    
    dim iCurrentLibrary as Integer
    dim XLBFILE as String
        XLBFILE = gTesttoolPath & "framework\optional\input\AppLibrary1\script.xlb"
    const CFN = "tMakro_Dialog::"
    const CMACRO = "MacroDlg"
    
    
    gApplication = "WRITER"
    '///+<li>Open a new document</li>
    printlog( "Open a new document" )
    brc = hCreateDocument()
    
    '///+<li>Open the basic organizer</li>
    '///+<li>Create a new library for the current document</li>
    '///+<li>Create a new module for the current document<br>
    '///+ -&gt; The Basic-Ide opens</li>
    printlog( "Create a new library, a new module, open Basic-Ide" )
    brc = hInitBasicIde( CMACRO )
    if ( not brc ) then
        warnlog( CFN & "Could not open BASIC-IDE, aborting" )
        goto endsub
    endif
    
    '///+<li>Close the Basic IDE</li>
    printlog( "Close the Basic-Ide" )
    kontext "BasicIDE"
    hCloseBasicIde()
    
    '///+<li>Open the Basic Organizer</li>
    printlog( "Open the Basic Macro Organizer" )
    brc = hOpenBasicOrganizerFromDoc()

    '///+<li>Select the module we created before</li>
    printlog( "Select the last Module for the current document" )
    iPos = hSelectTheLastNode( MakroAus )
    
    '///+<li>Open the Macro Obkect Organizer (Click Organize...)</li>
    printlog( "Click Organize..." )
    Verwalten.Click()
    
    '///+<li>Select the Modules-tab</li>
    printlog( "Select Modules-tab" )
    brc = hSelectBasicObjectOrganizerTab( 1 )

    
    '///+<li>Create a new module</li>
    printlog( "Press 'new module'" )
    kontext "tabmodule"
    NeuesModul.Click()

    kontext "neuesmodul"
    '///+<li>Cancel the 'New Module' dialog</li>
    printlog( "Cancel the dialog" )
    NeuesModul.Cancel()

    '///+<li>Create a 'New Dialog'</li>
    brc = hSelectBasicObjectOrganizerTab( 2 )

    kontext "tabdialogs"
    printlog( "Press 'new dialog'" )
    NewDialog.Click()
       
    '///+<li>Cancel the dialog</li>
    printlog( "Cancel the dialog" )
    kontext "neuerdialog"
    NeuerDialog.Cancel()

    kontext
    '///+<li>Activate TabLibrary on it 'New' / 'Password' / 'Delete' / 'Append' / 'Edit'</li>
    printlog( "Select the libraries Tab" )
    brc = hSelectBasicObjectOrganizerTab( 3 )

    
    '///+<li>Press 'New' on the libraries tab</li>
    printlog( "Press 'New' on the libraries tab" )
    kontext "tabbibliotheken"
    Neu.Click()

    kontext "neuebibliothek"
    '///+<li>Close the dialog with 'OK'</li>
    printlog( "Close the dialog with 'OK'" )
    Call DialogTest( NeueBibliothek )
    NeueBibliothek.OK()
    
    '///+<li>Delete the new default library</li>
    printlog( "Delete 'Library1'" )
    kontext "tabbibliotheken"
    Loeschen.Click()
    
    '///+<li>Close the deletion warning</li>
    printlog( "Close the warning" )
    kontext "active"
    if ( Active.Exists() ) then
        cMsg = active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        printlog( "MessageBox: " & cMsg )
        Active.Yes()
    else
        warnlog( "Deletion warning is missing" )
    endif
  
    '///+<li>Check the number of entries in the Libraries-List, one expected</li>
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

    '///+<li>Append a library by pressing 'Append'</li>
    printlog( "Append a xlb-file" )
    kontext "tabbibliotheken"
    Hinzufuegen.Click()

    '///+<li>Select a valid library and open it</li>
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

    '///+<li>Make sure 'Insert as Reference' is unchecked</li>
    printlog( "Ensure 'Insert as Reference' is unchecked" )
    kontext "appendlibraries"
    InsertAsReference.UnCheck()

    '///+<li>Check 'Replace Existing Library'</li>
    printlog( "Check 'Replace existing library'" )
    ReplaceExistingLibraries.Check()

    '///+<li>Press 'OK' to append the library</li>
    printlog( "Press 'OK' to append the library" )
    kontext "appendlibraries"
    try
        AppendLibraries.OK()
    catch
        warnlog( "#i85254# Appending libraries leads to crash" )
    endcatch

    '///+<li>Check the number of entries in the Libraries-List, two expected</li>
    printlog( "Check the number of libraries in list (should be two)" )
    kontext "tabbibliotheken"
    iLibraryItemCount = Bibliotheksliste.getItemCount()
    if ( iLibraryItemCount <> 2 ) then
        qaerrorlog( "#i87457# Two entries in list expected, found " & iLibraryItemCount )
    endif

    '///+<li>Select 'AppLibrary1' and click 'Password'</li>
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
    
    '///+<li>Click to set a password for AppLibrary1, cancel dialog</li>
    printlog( "Set a password for AppLibrary1" )
    kontext "tabbibliotheken"
    if ( Passwort.IsEnabled() ) then
        Passwort.Click()
        kontext "passwddlg"
        PasswdDLG.Cancel()
    else
        warnlog( "Password is disabled for 'AppLibrary1'" )
    endif

    '///+<li>Delete the inserted library (AppLibrary1)</li>
    printlog( "Delete 'AppLibrary1'" )
    kontext "tabbibliotheken"
    Loeschen.Click()
    
    '///+<li>Close the warning with YES</li>
    printlog( "Close the warning with YES" )
    kontext "active"
    if ( Active.Exists() ) then
        cMsg = active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        printlog( "MessageBox: " & cMsg )
        Active.Yes()
    else
        warnlog( "Expected confirmation dialog for delete" )
    endif
    
    '///+<li>Click on the Edit-button -&gt; The Basic-Ide opens</li>
    kontext "tabbibliotheken"
    printlog( "Press 'edit' on 'Libraries'-tab" )
    Bearbeiten.Click()

    '///+<li>Close the Basic-Ide</li>
    kontext "BasicIDE"
    if ( BasicIDE.Exists( 5 ) ) then
    
        hInsertMacro( 3 )
    
        printlog( "Close the basic-document" )
        brc = hCloseBasicIde()

        kontext "active"
        if ( Active.Exists ( 2 ) ) then
            cMsg = active.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            warnlog( "Unexpected MessageBox: " & cMsg )
            Active.NO()
        endif

    else
        warnlog("#i38978# The Macro-Document is not open, aborting test")
        call hDestroyDocument()
        goto endsub
    endif

    '///+<li>Open the BASIC Macro Organizer from the current document</li>
    printlog( "Open the BASIC Macro Organizer" )
    brc = hOpenBasicOrganizerFromDoc()
    
    '///+<li>Go to the last macro in the list</li>
    printlog( "Select the last macro from the list" )
    kontext "makro"
    iPos = hSelectTheLastNode( MakroAus )
    
    try
        '///+<li>Assign the macro by pressing 'Assign' -&gt; Tools/customize opens</li>
        printlog( "Assign the macro, Tools/customize dialog should open" )
        Zuordnen.Click ()
    catch
        warnlog( "#i106853# Assign-Button is disabled or macro is missing" )
    endcatch

    '///+<li>Activate all tapages and cancel the Tools/Customize dialog</li>
    printlog( "Activate all tapages and cancel the Tools/Customize dialog" )
    
    brc = hToolsCustomizeSelectTab( "menu" )
    brc = hToolsCustomizeSelectTab( "keyboard" 
    if ( not brc ) then
        qaerrorlog( "#i61765# Keyboard tab missing when called via Tools/Macros->Assign" )
    endif
    brc = hToolsCustomizeSelectTab( "events" )
    brc = hToolsCustomizeSelectTab( "toolbars" )

    '///+<li>Cancel Tools/Customize dialog -&gt; Back to Macro Organizer</li>
    printlog( "Cancel Tools/Customize dialog -> Macro Organizer should be open" )    
    brc = hToolsCustomizeClose( 2 )
    
    
    '///+<li>Click to run the macro</li>
    printlog "Click to run macro"
    kontext "makro"
    try
        Ausfuehren.Click()
    catch
        warnlog( "Unable to run macro" )
        kontext "Makro"
        Makro.close()
    endcatch
    
    '///+<li>The macro thriggers a messagebox, close it</li>
    kontext "active"
    if ( active.exists() ) then
        cMsg = active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        if ( cMsg = "TTMacro3" ) then
            printlog( "The correct macro has been executed" )
        else
            warnlog( "Unknown Messagebox: " & cMsg ) 
        endif
        Active.OK()
    endif
    
    '///+<li>Open Macro organizer from the current document</li>
    printlog( "Open macro organizer from the current document" )
    brc = hOpenBasicOrganizerFromDoc()
    
    '///+<li>Select the last item in the treelist</li>
    printlog( "Select the last item in the treelist" )
    brc = hSelectTheLastNode( MakroAus )

    '///+<li>Delete the module we created earlier</li>
    printlog( "Delete the new module" )
    Loeschen.Click()
    
    '///+<li>Close the deletion warning</li>
    printlog( "Close the deletion warning" )
    kontext "active"
    if ( active.exists() ) then
        cMsg = active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        printlog( "MessageBox: " & cMsg )
        Active.Yes()
    else
        warnlog( "No confirmation dialog for delete" )
    endif
    
    '///+<li>Close the macro organizer</li>
    printlog( "Close the macro organizer" )
    kontext "makro"
    Makro.Close()
    
    '///+<li>Close the document</li>
    printlog( "Close the document" )
    Call hDestroyDocument()
    
    '///</ul>
    
endcase



