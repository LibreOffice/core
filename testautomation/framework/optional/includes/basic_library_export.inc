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
'*  short description : Export BASIC library (flat)
'*
'\******************************************************************************

testcase tBasicLibraryExport

    printlog( "Export a basic script as library (flat format)" )

    ' Assumption: All actions take place in the user/work directory
    ' macro taken from framework/tools/input/macros.txt::tBasicLibraryExport
    
    ' For test specific global constants (UPPERCASE) see .BAS-file

    const LIBRARIES_TAB = 3
    
    dim cDocumentName as string
    dim cLibraryName as string
    dim iNodeCount as integer
    
    dim cFile as string : cFile = hGetWorkPath() & LIBRARY_NAME
        
    dim cMsg as string
    
    dim iCurrentLib as integer

    printlog( "Delete potential leftovers from prior test run" )
    if ( dir( cFile ) <> "" ) then
        QAErrorLog( "Files from prior test run exist: " & cFile )
        hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "dialog.xlb"  )
        hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "Module1.xba" )
        hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "script.xlb"  )
        rmdir( hGetWorkPath() & LIBRARY_NAME ) : printlog( "Remove directory" )
    endif
    
    printlog( "Open a new text document" )
    hInitSingleDoc()
    hChangeDoc()
    
    printlog( "Open another text document" )
    gApplication = "WRITER"
    hCreateDocument()
    
    printlog( "Open the BASIC organizer (Tools/Macros/BASIC)" )
    ToolsMacro_uno
    
    kontext "Makro"
    printlog( "Select the document" )
    iNodeCount = hGetNodeCount( MakroAus )
    cDocumentName = hSelectNode( MakroAus, iNodeCount )

    printlog( "Click <Manage...>" )    
    Verwalten.click()
    
    printlog( "Open the Libraries tabpage" )
    hSelectBasicObjectOrganizerTab( 3 )
    
    kontext "TabBibliotheken"
    printlog( "Select document Untitled2" )
    Bibliothek.select( DOCUMENT_POSITION )
    
    ' verify that the correct document is selected
    if ( Bibliothek.getSelText() <> cDocumentName ) then
        warnlog( "Incorrect document selected on libraries tab, aborting" )
        goto endsub
    endif
    
    printlog( "Click <New>" )
    Neu.click()
    
    kontext "NeueBibliothek"
    printlog( "Name the library: " & LIBRARY_NAME )
    BibliotheksName.setText( LIBRARY_NAME )
    NeueBibliothek.ok()
    
    kontext "TabBibliotheken"
    printlog( "Verify that the new library is selected in the libraries list" )
    cLibraryName = BibliotheksListe.getSelText()
    
    printlog( "Click <Edit>" )
    Bearbeiten.click()

    printlog( "Insert a macro" )
    hInsertMacroFromFile( LIBRARY_NAME )
    
    printlog( "Close the BASIC IDE" )
    hCloseBasicIDE()
    
    printlog( "Open the BASIC macro organizer" )
    ToolsMacro_uno
    
    kontext "Makro"
    printlog( "Select the work document" )
    iNodeCount = hGetNodeCount( MakroAus )
    hSelectNode( MakroAus, iNodeCount )    
    
    printlog( "Click <Manage...>" )
    Verwalten.click()
    
    printlog( "Go to the libraries-tab" )
    hSelectBasicObjectOrganizerTab( LIBRARIES_TAB )
    
    kontext "TabBibliotheken"
    printlog( "Select the document at pos. " & DOCUMENT_POSITION )
    Bibliothek.select( DOCUMENT_POSITION )
    
    printlog( "Verify that the correct document is selected" )
    if ( Bibliothek.getSelText() <> cDocumentName ) then
        warnlog( "Incorrect document selected on libraries tab, aborting" )
        goto endsub
    endif
    
    printlog( "Select the new library" )
    kontext "TabBibliotheken"
    for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
        Bibliotheksliste.select( iCurrentLib )
        if ( Bibliotheksliste.getSelText = LIBRARY_NAME ) then
            exit for
        endif
    next iCurrentLib
    
    printlog( "Click export" )
    Export.click()
    
    kontext "ExportBasicLibraryDlg"
    if ( ExportBasicLibraryDlg.exists( 2 ) ) then

        printlog( "Export as library" )    
        ExportAsLibrary.check()
        ExportBasicLibraryDlg.ok()
        
        kontext "Active"
        printlog( "Check for non existing directory warning" )
        ' Note: Querying for an Active will also find the Libraries tabpage.
        '       So we need to check which active we've got. RT = 373 is the tabcontrol.
        if ( Active.exists( 1 ) ) then
            if ( Active.getRT() <> 373 ) then
                warnlog( "Unexpected messagebox: " & Active.getText() )
                active.ok()
            else
                printlog( "Back on tab-control, good." )
            endif
        else
            printlog( "No warning for non-existing directory, good." )
        endif

        Kontext "OeffnenDlg"
        if ( Oeffnendlg.exists( 1 ) ) then
            Oeffnen.click()
        else
            warnlog( "File Open dialog not displayed" )
        endif
        
    else
        warnlog( "The ""Export Library"" dialog was not displayed" )
    endif
        
    kontext "TabBibliotheken"
    printlog( "Close the macro/libraries organizer" )
    if ( TabBibliotheken.exists( 1 ) ) then
        TabBibliotheken.cancel()
        
        kontext "Makro"
        printlog( "Cancel macro organizer" )
        Makro.cancel()
        
        hFileCloseAll()
    else
        warnlog( "Dialog <TabBibliotheken> could not be accessed" )
        call exitRestartTheOffice()
    endif
    
endcase

