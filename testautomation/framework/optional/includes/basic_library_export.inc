'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_library_export.inc,v $
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
'*  short description : Export BASIC library (flat)
'*
'\******************************************************************************

testcase tBasicLibraryExport

    '///<h1>Export a BASIC library (flat)</h1>
    ' Assumption: All actions take place in the user/work directory
    ' macro taken from framework/tools/input/macros.txt::tBasicLibraryExport
    
    ' For constants (UPPERCASE) see .BAS-file
    
    dim cDocumentName as string
    dim cLibraryName as string
    
    dim iNodeCount as integer
    
    dim cFile as string
        cFile = hGetWorkPath() & LIBRARY_NAME
        
    dim cMsg as string
    
    dim iCurrentLib as integer
    
    ' cleanup
    hDeleteFile( cFile )
    
    '///<ul>
    '///+<li>Make sure we have exactly one open document (changed) to start with</li>
    hInitSingleDoc()
    hChangeDoc()
    
    '///+<li>Create a new writer document</li>
    gApplication = "WRITER"
    hCreateDocument()
    
    '///+<li>Go to Tools/Macros/Organize Macros/OpenOffice.org Basic</li>
    ToolsMacro_uno
    
    '///+<li>Select the last node (number 4, document untitled2)</li>
    kontext "Makro"
    iNodeCount = hGetNodeCount( MakroAus )
    cDocumentName = hSelectNode( MakroAus, iNodeCount )
    
    '///+<li>Click to open the organizer</li>
    Verwalten.click()
    
    '///+<li>Switch to the libraries-tab</li>
    hSelectBasicObjectOrganizerTab( 3 )
    
    '///+<li>Select the document (untitled2 at pos. 4)</li>
    printlog( "Select document Untitled2" )
    kontext "TabBibliotheken"
    Bibliothek.select( DOCUMENT_POSITION )
    
    ' verify that the correct document is selected
    if ( Bibliothek.getSelText() <> cDocumentName ) then
        warnlog( "Incorrect document selected on libraries tab, aborting" )
        goto endsub
    endif
    
    '///+<li>Click to create a new library</li>
    Neu.click()
    
    '///+<li>Name the library &quot;tBasicLibraryExport&quot;, confirm with ok</li>
    kontext "NeueBibliothek"
    BibliotheksName.setText( LIBRARY_NAME )
    NeueBibliothek.ok()
    
    '///+<li>Verify that the correct library is selected</li>
    kontext "TabBibliotheken"
    cLibraryName = BibliotheksListe.getSelText()
    
    '///+<li>Click to edit the library</li>
    Bearbeiten.click()

    '///+<li>Insert a macro (e.g. to print a messagebox)</li>
    hInsertMacroFromFile( LIBRARY_NAME )
    
    '///+<li>Close the Basic IDE</li>
    hCloseBasicIDE()
    
    '///+<li>Go to Tools/Macros/Organize Macros/OpenOffice.org Basic</li>
    ToolsMacro_uno
    
    '///+<li>Select the last node (untitled2)</li>
    kontext "Makro"
    iNodeCount = hGetNodeCount( MakroAus )
    hSelectNode( MakroAus, iNodeCount )    
    
    '///+<li>Click to open the organizer</li>
    Verwalten.click()
    
    '///+<li>Switch to the libraries-tab</li>
    hSelectBasicObjectOrganizerTab( 3 )
    
    '///+<li>Select the document (untitled2 at pos. 4)</li>
    kontext "TabBibliotheken"
    Bibliothek.select( DOCUMENT_POSITION )
    
    ' verify that the correct document is selected
    if ( Bibliothek.getSelText() <> cDocumentName ) then
        warnlog( "Incorrect document selected on libraries tab, aborting" )
        goto endsub
    endif
    
    '///+<li>Find the new library, select it</li>
    printlog( "Select the new library" )
    kontext "TabBibliotheken"
    for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
        Bibliotheksliste.select( iCurrentLib )
        if ( Bibliotheksliste.getSelText = LIBRARY_NAME ) then
            exit for
        endif
    next iCurrentLib
    
    '///+<li>Click Export</li>
    printlog( "Click export" )
    Export.click()
    
    '///+<li>Select to export as Library, click ok</li>
    printlog( "Export as library" )
    kontext "ExportBasicLibraryDlg"
    if ( ExportBasicLibraryDlg.exists( 2 ) ) then
    
        ExportAsLibrary.check()
        ExportBasicLibraryDlg.ok()

        '///+<li>Name the package &quot;tBasicLibraryExportAsPackage&quot;, save it</li>
        printlog( "Use default path (=workdir)" )
        Kontext "OeffnenDlg"
        if ( Oeffnendlg.exists( 2 ) ) then
            Oeffnen.click()
        else
            kontext "active"
            if ( active.exists() ) then
                cMsg = active.getText()
                cMsg = hRemoveLineBreaks( cMsg )
                warnlog( "Unexpected messagebox: " & cMsg )
                active.ok()
            endif
        endif
        
    else
        warnlog( "The ""Export Library"" dialog was not displayed" )
    endif
        
      
    '///+<li>Close the macro organizer</li>
    printlog( "Close the macro/libraries organizer" )
    kontext "TabBibliotheken"
    TabBibliotheken.cancel()
    
    '///+<li>Close OpenOffice.org Basic Macros dialog</li>
    printlog( "Cancel macro organizer" )
    kontext "Makro"
    Makro.cancel()
    
    '///+<li>Close the document</li>
    hDestroyDocument()
    
    '///</ul>
    
endcase

