'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_package_export.inc,v $
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
'*  short description : Export BASIC libraries as package
'*
'\******************************************************************************

testcase tBasicPackageExport

    ' Assumption: All actions take place in the user/work directory
    printlog( "Export BASIC libraries as package/extension" )
    
    const PACKAGE_NAME = "tBasicExport.oxt"
    const LIBRARY_NAME = "tBasicExport"
    const DOCUMENT_POSITION = 4
    const PACKAGE_SIZE = 2000
    
    dim cDocumentName as string
    dim cLibraryName as string

    dim iNodeCount as integer
    dim iCurrentLib as integer
    
    dim cFile as string : cFile = hGetWorkPath() & PACKAGE_NAME
    dim cPackageFileName as string
        
    
    printlog( "Delete potential leftovers from prior test run" )
    hDeleteFile( cFile )
    hExtensionRemoveGUI( PACKAGE_NAME )

    printlog( "Open one single writer document, modify it" )
    hInitSingleDoc()
    hChangeDoc()

    printlog( "Open another document, we want to have two to see that they both " & _
              "appear in the BASIC organizer" )
    gApplication = "WRITER"
    hCreateDocument()

    printlog( "Open BASIC organizer" )    
    ToolsMacro_uno
    
    printlog( "Select the second document" )
    kontext "Makro"
    if ( Makro.exists( 1 ) ) then
        iNodeCount = hGetNodeCount( MakroAus )
        cDocumentName = hSelectNode( MakroAus, iNodeCount )

        printlog( "Click <Manage...>" )    
        Verwalten.click()
        
        printlog( "Go to the libraries tabpage" )
        hSelectBasicObjectOrganizerTab( 3 )
        
        kontext "TabBibliotheken"
        printlog( "Select the document at pos. " & DOCUMENT_POSITION )
        Bibliothek.select( DOCUMENT_POSITION )
        
        printlog( "Verify that the correct document has been selected" )
        if ( Bibliothek.getSelText() <> cDocumentName ) then
            warnlog( "Incorrect document selected on libraries tab, aborting test" )
            goto endsub
        endif
        
        printlog( "Click <New...>" )
        Neu.click()
        
        kontext "NeueBibliothek"
        if ( NeueBibliothek.exists( 1 ) ) then
            printlog( "Name the new library: " & LIBRARY_NAME )
            BibliotheksName.setText( LIBRARY_NAME )
            NeueBibliothek.ok()
        else
            warnlog( "Dialog <NeueBibliothek> did not open" )
            goto endsub
        endif
        
        kontext "TabBibliotheken"
        cLibraryName = BibliotheksListe.getSelText()
        if ( cLibraryName <> LIBRARY_NAME ) then
            warnlog( "The new library is not selected in the libraries list" )
        else
            printlog( "The correct library is selected in the libraries list" )
        endif
        
        printlog( "Click <Edit...>" )
        Bearbeiten.click()

        printlog( "Insert a test macro (something simple, e.g. a messagebox" )
        hInsertMacroFromFile( LIBRARY_NAME )
        
        printlog( "Close the BASIC IDE" )
        hCloseBasicIDE()
    else
        warnlog( "Dialog <Makro> did not open, aborting test" )
        goto endsub
    endif
    
    printlog( "Open the BASIC organizer" )
    ToolsMacro_uno
    
    printlog( "Select the work document" )
    kontext "Makro"
    if ( Makro.exists( 1 ) ) then
        iNodeCount = hGetNodeCount( MakroAus )
        hSelectNode( MakroAus, iNodeCount )    
        
        printlog( "Manage..." )
        Verwalten.click()
        
        printlog( "Libraries tab" )
        hSelectBasicObjectOrganizerTab( 3 )
        
        kontext "TabBibliotheken"
        if ( TabBibliotheken.exists( 1 ) and TabBibliotheken.isVisible() ) then
            Bibliothek.select( DOCUMENT_POSITION )
            
            printlog( "Select the new library" )
            kontext "TabBibliotheken"
            for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
                Bibliotheksliste.select( iCurrentLib )
                if ( Bibliotheksliste.getSelText() = LIBRARY_NAME ) then
                    exit for
                endif
            next iCurrentLib
            
            kontext "TabBibliotheken"
            printlog( "Export the package, close the document afterwards" )
            Export.click()
            
            kontext "ExportBasicLibraryDlg"
            if ( ExportBasicLibraryDlg.exists( 1 ) ) then
                printlog( "Select to export as package" )
                ExportAsPackage.check()
                ExportBasicLibraryDlg.ok()
                
                kontext "SpeichernDlg"
                if ( SpeichernDlg.exists( 1 ) ) then
                    printlog( "Save the file, automatic filename extension must be checked" )
                    DateiName.setText( hGetWorkPath() & LIBRARY_NAME ) ' automatic filename extension/uno-pkg is default
                    Speichern.click()
                else
                    warnlog( "Dialog <File Save> did not open" )
                endif
            else
                warnlog( "Dialog <Export Basic Library> did not open" )
            endif
            
            kontext "TabBibliotheken"
            printlog( "Close libraires tabpage" )
            TabBibliotheken.cancel()
        else
            warnlog( "Dialog <TabBibliotheken> is not available" )
        endif
        
        kontext "Makro"
        printlog( "Close BASIC organizer" )
        Makro.cancel()
    else
        warnlog( "Dialog <Makro> did not open" )
    endif
    
    printlog( "Close all files" )
    hFileCloseAll()   
    
    printlog( "Verify automatic filename extension (dialog should append .oxt)" )
    if ( FileExists( cFile ) ) then
        printlog( "The test-package was saved with .oxt extension, good." )
        cPackageFileName = cFile
    else
        if ( FileExists( hGetWorkPath() & LIBRARY_NAME ) ) then
            warnlog( "The test-package was saved without .oxt filename extension" )
            cPackageFileName = hGetWorkPath() & LIBRARY_NAME
        else
            warnlog( "The test extension was not saved to the users workdirectory" )
            goto endsub
        endif
    endif
    
    printlog( "Verify file size" )
'    if ( FileLen( cPackageFileName ) <> PACKAGE_SIZE ) then
'        warnlog( "#i105719# - The exported test-package has an incorrect file size." )
        if ( FileLen( cPackageFileName ) = 0 ) then
            printlog( "File has zero bytes, it is empty." )
        else
            printlog( "Size is.: " & FileLen( cPackageFileName ) )
            printlog( "Expected: " & PACKAGE_SIZE )
        endif
'    else
'        printlog( "File has correct size" )
'    endif
    
endcase

