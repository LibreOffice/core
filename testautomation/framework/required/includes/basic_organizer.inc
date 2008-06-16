'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_organizer.inc,v $
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
'/************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Update test for the Basic Organizer dialog
'*
'\******************************************************************************

testcase tUpdtBasicOrganizer

    '///<H1>Update test for the Basic organizer and its dialogs</H1>
    '///<ul>
    dim brc as boolean
    dim cTempString as string
    const TESTLIB = "zzzz"

    '///+<li>Open a new document</li>
    printlog( "Open a new document" )
    hCreateDocument()

    '///+<li>Open the Basic Macro-Organizer</li>
    printlog( "Open the Basic Macro Organizer" )
    ToolsMacro_uno

    kontext "Makro"
    if ( not makro.exists() ) then
        warnlog( "Macro Organizer is not open, aborting test" )
        brc = hDestroyDocument()
        goto endsub
    else
        printlog( "Successfully opened the MacroOrganizer" )
    endif

    '///+<li>Open the Libraries/Modules/Dialogs-Organizer</li>
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
        brc = hDestroyDocument()
        goto endsub
    endif
        
    '///+<li>Open all dialogs on the Modules-Page</li>
    '///<ol>
    kontext 
    active.setPage TabModule

    kontext "TabModule"
    printlog( "Modules Tab" )
    call DialogTest( TabModule )

    '///+<li>Select first item in modules list</li>
    Modulliste.typeKeys( "<HOME>" )
    try
        '///+<li>Click to add a new module, cancel the dialog</li>
        printlog( "New Modules-dialog" )
        NeuesModul.click()
        kontext "NeuesModul"
        call DialogTest( NeuesModul )
        NeuesModul.cancel()
    catch
        warnlog( "There is a problem accessing the NewModule-dialog" )
    endcatch

    '///</ol>
    '///+<li>Open all dialogs on the Dialogs-Page</li>
    '///<ol>
    kontext
    active.setPage( TabDialogs )

    Kontext "TabDialogs"
    printlog( "Dialogs Tab" )
    call DialogTest( TabDialogs )

    '///+<li>Select first item in dialogs list</li>
    ModuleList.typeKeys( "<HOME>" )

    try
        '///+<li>Click to add a new dialog, cancel the dialog</li>
        printlog( "New Libraries-dialog" )
        NewDialog.click()
        kontext "NeuerDialog"
        call DialogTest( NeuerDialog )
        NeuerDialog.cancel()
    catch
        warnlog( "There is a problem accessing the NewModule-dialog" )
    endcatch

    '///</ol>
    '///+<li>Open all dialogs on the Libraries-Page</li>
    '///<ol>
    Kontext
    active.setPage( TabBibliotheken )

    Kontext "TabBibliotheken"
    printlog( "Libraries Tab" )
    call DialogTest( TabBibliotheken )

    '///+<li>Select first item in libraries list</li>
    Bibliotheksliste.typeKeys( "<HOME>" )

    kontext "TabBibliotheken" 
    '///+<li>Click to add a new library, name it &quot;zzzz&quot;, click OK<br>
    '/// Note that the library should be selected (the last in the list)</li>

    if ( neu.isEnabled() ) then
        printlog( "New Library dialog" )
        Neu.click()
        kontext "NeueBibliothek"
        call DialogTest( NeueBibliothek )
        Bibliotheksname.setText( "zzzz" )
        NeueBibliothek.ok()
    else
        warnlog( "There is a problem accessing the New Libraries dialog" )
    endif
    
    kontext "TabBibliotheken" 
    '///+<li>Try to create a library of same name again:<br>
    '///+ Click to add a new library, name it &quot;zzzz&quot;, click OK<br>
    '///+ Click away the messagebox (A library of same name already exists)</li>
    if ( neu.isEnabled() ) then

        printlog( "New Library dialog" )
        Neu.click()
        kontext "NeueBibliothek"
        call DialogTest( NeueBibliothek )
        Bibliotheksname.setText( "zzzz" )
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
    '///+<li>Click &quot;Export...&quot;</li>
    printlog( "Export the package" )
    export.click()
    
    kontext "ExportBasicLibraryDlg"
    '///+<li>Check &quot;Export as package&quot;, click OK</li>
    printlog( "Export as package, click OK" )
    ExportAsPackage.check()
    ExportBasicLibraryDlg.ok()
    
    kontext "SpeichernDlg"
    '///+<li>Cancel the FileSave dialog</li>
    printlog( "Cancel FileSave dialog" )
    SpeichernDlg.cancel()
    
    kontext "TabBibliotheken"
    '///+<li>Click &quot;Export...&quot;</li>
    printlog( "Export the package" )
    export.click()
    
    kontext "ExportBasicLibraryDlg"
    '///+<li>Check &quot;Export as library&quot;, click OK</li>
    printlog( "Export as library, click OK" )
    ExportAsLibrary.check()
    ExportBasicLibraryDlg.ok()    
    
    kontext "OeffnenDlg"
    '///+<li>Cancel the FileOpen dialog</li>
    printlog( "Cancel FileOpen dialog" )
    OeffnenDlg.cancel()    

    kontext "TabBibliotheken"
    if ( passwort.isEnabled() ) then
        '///+<li>Click to set a password, cancel the dialog</li>
        printlog( "Password-Dialog" )
        passwort.click()
        kontext "PasswdDlg"
        PasswdDlg.cancel()
    else
        warnlog( "There is a problem accessing the Passwords-dialog" )
    endif

    kontext "TabBibliotheken"
    '///+<li>Click &quot;Delete&quot;, accept with yes</li>
    printlog( "Try to delete the library" )
    
    cTempString = BibliotheksListe.getSelText() 
    if ( cTempString = "zzzz" ) then
        printlog( "Delete" )
        Loeschen.click()
        Kontext "Active"
        Active.yes()
    else
        warnlog( "Could not delete the library, name is incorrect: " & cTempString )
    endif
    
    Kontext "TabBibliotheken"

    try
        '///+<li>Click &quot;Append&quot;, cancel dialog</li>
        printlog( "Append a new library" )
        Hinzufuegen.click()
        kontext "OeffnenDlg"
        call DialogTest( OeffnenDlg )
        OeffnenDlg.cancel()
    catch
        warnlog( "There is a problem appending a new library" )
    endcatch

    Kontext "TabBibliotheken"

    '///</ol>
    '///+<li>Close the Library/Modules/Dialogs-Organizer</li>
    printlog( "Cancel library/modules/dialogs organizer" )
    TabBibliotheken.cancel()

    '///+<li>Close the Macro-Organizer</li>
    printlog( "Cancel Macro Organizer" )
    Kontext "Makro"
    Makro.cancel()
    
    '///+<li>Close the document</li>
    printlog( "Close the document" )
    brc = hDestroyDocument()

    '///</ul>

endcase

