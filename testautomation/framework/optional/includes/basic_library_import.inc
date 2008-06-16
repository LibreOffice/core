'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_library_import.inc,v $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Import BASIC library (flat)
'*
'\******************************************************************************

testcase tBasicLibraryImport

    '///<h1>Import a basic library (flat)</h1>
    '///<ul>
    
    dim cMsg as string
    dim iCurrentLib as integer

    '///+<li>Open Basic Organizer</li>
    ToolsMacro_uno
    
    '///+<li>Select My Macros</li>
    kontext "Makro"
    MakroAus.Select( 1 )
    
    '///+<li>Open the library Organizer</li>
    Verwalten.click()
    
    '///+<li>Switch to the libraries-tab</li>
    hSelectBasicObjectOrganizerTab( 3 )    
    
    '///+<li>Make sure My Macros is selected</li>
    printlog( "Select My macros" )
    kontext "TabBibliotheken"
    Bibliothek.select( 1 )
    
    '///+<li>Click to import a library</li>
    printlog( "Click to add a library" )
    Hinzufuegen.click()
    
    '///+<li>Enter the name of the library</li>
    printlog( "Enter the name of the library: " & LIBRARY_NAME )
    kontext "OeffnenDlg"
    DateiName.setText( LIBRARY_NAME )
    
    printlog( "Step into the directory" )
    OeffnenDlg.typeKeys( "<RETURN>" )
    
    '///+<li>Enter &quot;dialog.xlb&quot;</li>
    printlog( "Enter &quot;dialog.xlb&quot;" )
    DateiName.setText( "dialog.xlb" )
    
    '///+<li>Open the file</li>
    printlog( "Open dialog.xlb" )
    Oeffnen.click()
    
    '///+<li>Handle failing autocompletion</li>
    kontext "active"
    if ( active.exists( 1 ) ) then
        if ( active.getRT() <> 373 ) then
            cMSG = Active.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            warnlog( "Autocompletion failed: " & cMsg )
            Active.ok()
            Kontext "OeffnenDlg"
            DateiName.setText( "dialog.xlb" )
            Oeffnen.click()
        endif
    endif
    
    '///+<li>Confirm to append libraries, accept defaults</li>
    printlog( "Confirm to append the library" )
    Kontext "AppendLibraries"
    AppendLibraries.ok()
    
    '///+<li>Find the new library, select it</li>
    printlog( "Select the new library" )
    kontext "TabBibliotheken"
    for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
        Bibliotheksliste.select( iCurrentLib )
        if ( Bibliotheksliste.getSelText = LIBRARY_NAME ) then
            exit for
        endif
    next iCurrentLib        
    
    '///+<li>On libraries-tab click delete-button (correct lib must be selected)</li>
    printlog( "Delete the library, confirm with YES" )
    Kontext "TabBibliotheken"
    Loeschen.click()
    
    '///+<li>Confirm deletion with YES</li>
    Kontext "Active"
    if ( Active.exists( 1 ) ) then
        active.yes()
    else 
        warnlog( "Deletion warning for libraries is missing" )
    endif
    
    '///+<li>Close the macro organizer (Libraries Tab)</li>
    printlog( "Close macro/library organizer" )
    kontext "TabBibliotheken"
    TabBibliotheken.cancel()
    
    '///+<li>Close OpenOffice.org Basic Macros dialog</li>
    printlog( "Close macro organizer" )
    kontext "Makro"
    Makro.cancel()
    
    '///+<li>Cleanup: Close the document</li>
    hDestroyDocument()

    '///+<li>Cleanup: Delete the library (directory)</li>    
    hDeleteFile( hGetWorkPath() & LIBRARY_NAME )
    
    '///</ul>

    
endcase

