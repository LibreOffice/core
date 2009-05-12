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

    printlog( "Import a BASIC library" )

    dim cMsg as string
    dim iCurrentLib as integer

    ToolsMacro_uno
    
    kontext "Makro"
    MakroAus.Select( 1 )
    
    Verwalten.click()
    
    hSelectBasicObjectOrganizerTab( 3 )    
    
    printlog( "Select My macros" )
    kontext "TabBibliotheken"
    Bibliothek.select( 1 )
    
    printlog( "Click to add a library" )
    Hinzufuegen.click()
    
    printlog( "Enter the name of the library: " & LIBRARY_NAME )
    kontext "OeffnenDlg"
    DateiName.setText( LIBRARY_NAME )
    
    printlog( "Step into the directory" )
    OeffnenDlg.typeKeys( "<RETURN>" )
    
    printlog( "Enter &quot;dialog.xlb&quot;" )
    DateiName.setText( "dialog.xlb" )
    
    printlog( "Open dialog.xlb" )
    Oeffnen.click()
    
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
    
    printlog( "Confirm to append the library" )
    Kontext "AppendLibraries"
    AppendLibraries.ok()
    
    printlog( "Select the new library" )
    kontext "TabBibliotheken"
    for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
        Bibliotheksliste.select( iCurrentLib )
        if ( Bibliotheksliste.getSelText = LIBRARY_NAME ) then
            exit for
        endif
    next iCurrentLib        
    
    printlog( "Delete the library, confirm with YES" )
    Kontext "TabBibliotheken"
    Loeschen.click()
    
    Kontext "Active"
    if ( Active.exists( 1 ) ) then
        active.yes()
    else 
        warnlog( "Deletion warning for libraries is missing" )
    endif
    
    printlog( "Close macro/library organizer" )
    kontext "TabBibliotheken"
    TabBibliotheken.cancel()
    
    printlog( "Close macro organizer" )
    kontext "Makro"
    Makro.cancel()
    
    hDestroyDocument()

    hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "dialog.xlb"  )
    hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "Module1.xba" )
    hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "script.xlb"  )
    rmdir( hGetWorkPath() & LIBRARY_NAME ) : printlog( "Remove directory" )
       
endcase

