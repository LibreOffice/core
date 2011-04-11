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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Import BASIC library via Package Manager GUI as UNO Package
'*
'\******************************************************************************

testcase tBasicPackageImport

    ' Assumption: All actions take place in the user/work directory
    ' macro taken from global/input/macros.txt::tBasicLibraryExport
    
    printlog( "Import BASIC library via Package Manager GUI as UNO Package" )

    dim package_name as string : package_name = "tBasicExport.oxt"

    const LIBRARY_NAME = "tBasicExport"
    const MY_MACROS = 1
    const LIBRARIES_TAB = 3
    const SHORT_WAIT = 1
    
    dim cDocumentName as string
    dim cLibraryName as string
    
    dim iNodeCount as integer
    dim iCurrentLib as integer
    dim iStatus as integer
    
    dim cFile as string : cFile = hGetWorkPath() & package_name

    printlog( "Probe for test extension created by basic_packege_export" )
    if ( Not FileExists( cFile ) or ( FileLen( cFile ) = 0 ) ) then
        warnlog( "#i105719# - Test extension is missing or has zero size. This is most likely" & _
                 " a follow-up error from the prior test case. Test stopped." )
        goto endsub
    endif
    
    gApplication = "WRITER"
    
    iStatus = hExtensionAddGUI( cFile, "InstallForUser,NoLicense,NoUpdate" )
    
    ' Evaluate the returncode. Anything different from 0 causes the test to end.
    if ( iStatus <= 0 ) then
        warnlog( "Error adding extension. Aborting." )
        goto endsub
    endif
    
    hMacroOrganizerRunMacro( LIBRARY_NAME )
    
    kontext "active"
    if ( active.exists( 5 ) ) then
        active.ok()
    else
        warnlog( "The macro was not executed" )
    endif
    
    ToolsMacro_uno
    
    kontext "Makro"
    MakroAus.select( MY_MACROS )
    
    Verwalten.click()
    
    hSelectBasicObjectOrganizerTab( LIBRARIES_TAB )
    
    kontext "TabBibliotheken"
    Bibliothek.select( SHORT_WAIT )
    
    printlog( "Select the new library" )
    kontext "TabBibliotheken"
    for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
        Bibliotheksliste.select( iCurrentLib )
        if ( Bibliotheksliste.getSelText() = LIBRARY_NAME ) then
            exit for
        endif
    next iCurrentLib
    cLibraryName = BibliotheksListe.getSelText()

    hClickButton( Hinzufuegen )
    
    kontext "OeffnenDlg"
    hCloseDialog( OeffnenDlg, "cancel" )
    
    printlog( "Cleanup: Delete Library, close dialogs and remove package" )

    ' Note: It is assumed that the library is selected
    kontext "TabBibliotheken"
    cLibraryName = BibliotheksListe.getSelText()
    if ( cLibraryName <> LIBRARY_NAME ) then
        warnlog( "Incorrect library is selected" & cLibraryName )
    else
        Loeschen.click()
        
        kontext "active"
        if ( active.exists( SHORT_WAIT ) ) then
            active.yes()
        else
            warnlog( "No warning for library deletion" )
        endif
    endif       
    
    kontext "TabBibliotheken"
    hCloseDialog( TabBibliotheken, "cancel" )
    
    kontext "Makro"
    hCloseDialog( Makro, "cancel" )
    
    hExtensionRemoveGUI( package_name )
    
    
endcase

