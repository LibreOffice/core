'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_package_import.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-17 11:42:14 $
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
'*  short description : Import BASIC library via Package Manager GUI as UNO Package
'*
'\******************************************************************************

testcase tBasicPackageImport

    '///<h1>Import BASIC library as UNO package via Package Manager UI</h1>

    ' Assumption: All actions take place in the user/work directory
    ' macro taken from global/input/macros.txt::tBasicLibraryExport

    dim package_name as string

    ' CWS jl44 introduced .oxt for extensions   
    if ( gBuild < 9075 ) then
        package_name = "tBasicExport.uno.pkg"
        qaerrorlog( "Using old extension naming scheme" )
    else
        package_name = "tBasicExport.oxt"
    endif

    const LIBRARY_NAME = "tBasicExport"
    
    dim cDocumentName as string
    dim cLibraryName as string
    
    dim iNodeCount as integer
    dim iCurrentLib as integer
    dim iStatus as integer
    
    dim cFile as string
        cFile = hGetWorkPath() & package_name 
    
    '///<ul>  
    gApplication = "WRITER"
    
    '///+<li>Open the package manager ui</li>
    '///+<li>Add a package</li>
    '///+<li>Select &quot;tBasicLibraryExportAsPackage.oxt&quot;, import it</li>
    '///+<li>Close the package manager ui</li>
    iStatus = hExtensionAddGUI( cFile, "InstallForUser,NoLicense,NoUpdate" )
    
    ' Evaluate the returncode. Anything different from 0 causes the test to end.
    if ( iStatus <= 0 ) then
        warnlog( "Error adding extension. Aborting." )
        goto endsub
    endif
    
    '///+<li>Run the macro via Macro Organizer</li>
    hMacroOrganizerRunMacro( LIBRARY_NAME )
    
    '///+<li>The macro triggers a dialog, close it</li>
    kontext "active"
    if ( active.exists( 5 ) ) then
        active.ok()
    else
        warnlog( "The macro was not executed" )
    endif
    
    '///+<li>Go to Tools/Macros/Organize Macros/OpenOffice.org Basic</li>
    ToolsMacro_uno
    
    '///+<li>Select the first node (My Macros)</li>
    kontext "Makro"
    MakroAus.select( 1 )
    
    '///+<li>Click to open the organizer</li>
    Verwalten.click()
    
    '///+<li>Switch to the libraries-tab</li>
    hSelectBasicObjectOrganizerTab( 3 )
    
    '///+<li>Select My Macros & Dialogs</li>
    kontext "TabBibliotheken"
    Bibliothek.select( 1 )
    
    '///+<li>Find the new library, select it</li>
    printlog( "Select the new library" )
    kontext "TabBibliotheken"
    for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
        Bibliotheksliste.select( iCurrentLib )
        if ( Bibliotheksliste.getSelText() = LIBRARY_NAME ) then
            exit for
        endif
    next iCurrentLib
    cLibraryName = BibliotheksListe.getSelText()
    
    '///+<li>Click to import -> #i64007</li>
    try
        Hinzufuegen.click()
    catch
        warnlog( "#i64007 - Office crashes on clicking import button" )
    endcatch
    
    '///+<li>Cancel the FileOpen dialog</li>
    kontext "OeffnenDlg"
    OeffnenDlg.cancel()
    
    '///+<li>Cleanup: Delete the library</li>
    printlog( "Cleanup: Delete Library, close dialogs and remove package" )

    ' Note: It is assumed that the library is selected
    kontext "TabBibliotheken"
    cLibraryName = BibliotheksListe.getSelText()
    if ( cLibraryName <> LIBRARY_NAME ) then
        warnlog( "Incorrect library is selected" & cLibraryName )
    else
        Loeschen.click()
        
        kontext "active"
        if ( active.exists( 1 ) ) then
            active.yes()
        else
            warnlog( "No warning for library deletion" )
        endif
    endif    
    
    
    '///+<li>Cleanup: Close Libraries organizer</li>
    kontext "TabBibliotheken"
    TabBibliotheken.cancel()
    
    '///+<li>Cleanup: Close Macro Organizer</li>
    kontext "Makro"
    Makro.cancel()
    
    '///+<li>Remove package via package manager</li>
    hExtensionRemoveGUI( package_name )
    
    '///</ul>
    
endcase

