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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Update test for the Basic Organizer dialog
'*
'\******************************************************************************

testcase tUpdtBasicOrganizer

    printlog( "Update test for the Basic Organizer dialog" )

    dim cTempString as string
    const TESTLIB = "zzzz"

    printlog( "Open a new document" )
    hCreateDocument()

    printlog( "Open the Basic Macro Organizer" )
    ToolsMacro_uno

    kontext "Makro"
    if ( not makro.exists() ) then
        warnlog( "Macro Organizer is not open, aborting test" )
        hDestroyDocument()
        goto endsub
    else
        printlog( "Successfully opened the MacroOrganizer" )
    endif

    kontext "Makro"
    printlog( "Click <Organizer> to open the library/modules organizer" )
    if ( Verwalten.isEnabled() ) then
        Verwalten.click()
    else
        warnlog( "Cannot click the <Organize...> button" )
    endif
    
    ' Try to recover from previous error
    Kontext "TabModule"
    if ( not TabModule.exists() ) then
        printlog( "TabModule is not open, the test cannot continue" )
        kontext "Makro"
        Makro.cancel()
        hDestroyDocument()
        goto endsub
    endif
        
    kontext 
    active.setPage TabModule

    kontext "TabModule"
    printlog( "Modules Tab" )
    call DialogTest( TabModule )

    Modulliste.typeKeys( "<HOME>" )
    try
        printlog( "New Modules-dialog" )
        NeuesModul.click()
        kontext "NeuesModul"
        call DialogTest( NeuesModul )
        NeuesModul.cancel()
    catch
        warnlog( "There is a problem accessing the NewModule-dialog" )
    endcatch

    kontext
    active.setPage( TabDialogs )

    Kontext "TabDialogs"
    printlog( "Dialogs Tab" )
    call DialogTest( TabDialogs )

    ModuleList.typeKeys( "<HOME>" )

    try
        printlog( "New Libraries-dialog" )
        NewDialog.click()
        kontext "NeuerDialog"
        call DialogTest( NeuerDialog )
        NeuerDialog.cancel()
    catch
        warnlog( "There is a problem accessing the NewModule-dialog" )
    endcatch

    Kontext
    active.setPage( TabBibliotheken )

    Kontext "TabBibliotheken"
    printlog( "Libraries Tab" )
    call DialogTest( TabBibliotheken )

    Bibliotheksliste.typeKeys( "<HOME>" )

    kontext "TabBibliotheken" 

    if ( neu.isEnabled() ) then
        printlog( "New Library dialog" )
        Neu.click()
        kontext "NeueBibliothek"
        call DialogTest( NeueBibliothek )
        Bibliotheksname.setText( TESTLIB )
        NeueBibliothek.ok()
    else
        warnlog( "There is a problem accessing the New Libraries dialog" )
    endif
    
    kontext "TabBibliotheken" 
    if ( neu.isEnabled() ) then

        printlog( "New Library dialog" )
        Neu.click()
        kontext "NeueBibliothek"
        call DialogTest( NeueBibliothek )
        Bibliotheksname.setText( TESTLIB )
        NeueBibliothek.ok()
    else
        warnlog( "There is a problem accessing the New Libraries dialog" )
    endif  
    
    kontext "active"
    if ( active.exists() ) then
        printlog( "Closing warning" )
        active.ok()
    else
        warnlog( "Warning missing for duplicate library name" )
    endif

    kontext "TabBibliotheken" 
    printlog( "Export the package" )
    export.click()
    
    kontext "ExportBasicLibraryDlg"
    printlog( "Export as package, click OK" )
    ExportAsPackage.check()
    ExportBasicLibraryDlg.ok()
    
    kontext "SpeichernDlg"
    printlog( "Cancel FileSave dialog" )
    SpeichernDlg.cancel()
    
    kontext "TabBibliotheken"
    printlog( "Export the package" )
    export.click()
    
    kontext "ExportBasicLibraryDlg"
    printlog( "Export as library, click OK" )
    ExportAsLibrary.check()
    ExportBasicLibraryDlg.ok()    
    
    kontext "OeffnenDlg"
    printlog( "Cancel FileOpen dialog" )
    OeffnenDlg.cancel()    

    kontext "TabBibliotheken"
    if ( passwort.isEnabled() ) then
        printlog( "Password-Dialog" )
        passwort.click()
        kontext "PasswdDlg"
        PasswdDlg.cancel()
    else
        warnlog( "There is a problem accessing the Passwords-dialog" )
    endif

    kontext "TabBibliotheken"
    printlog( "Try to delete the library" )
    
    cTempString = BibliotheksListe.getSelText() 
    if ( cTempString = TESTLIB ) then
        printlog( "Delete" )
        Loeschen.click()
        Kontext "Active"
        Active.yes()
    else
        warnlog( "Could not delete the library, name is incorrect: " & cTempString )
    endif
    
    Kontext "TabBibliotheken"

    try
        printlog( "Append a new library" )
        Hinzufuegen.click()
        kontext "OeffnenDlg"
        call DialogTest( OeffnenDlg )
        OeffnenDlg.cancel()
    catch
        warnlog( "There is a problem appending a new library" )
    endcatch

    Kontext "TabBibliotheken"

    printlog( "Cancel library/modules/dialogs organizer" )
    TabBibliotheken.cancel()

    printlog( "Cancel Macro Organizer" )
    Kontext "Makro"
    Makro.cancel()
    
    printlog( "Close the document" )
    hDestroyDocument()

endcase

