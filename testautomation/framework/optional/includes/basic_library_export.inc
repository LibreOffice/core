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

    printlog( "Export a basic script as library (flat format)" )

    ' Assumption: All actions take place in the user/work directory
    ' macro taken from framework/tools/input/macros.txt::tBasicLibraryExport
    
    ' For constants (UPPERCASE) see .BAS-file
    
    dim cDocumentName as string
    dim cLibraryName as string
    dim iNodeCount as integer
    
    dim cFile as string : cFile = hGetWorkPath() & LIBRARY_NAME
        
    dim cMsg as string
    
    dim iCurrentLib as integer

    if ( dir( cFile ) <> "" ) then
        QAErrorLog( "Files from prior test run exist: " & cFile )
        hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "dialog.xlb"  )
        hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "Module1.xba" )
        hDeleteFile( hGetWorkPath() & LIBRARY_NAME & gPathSigne & "script.xlb"  )
        rmdir( hGetWorkPath() & LIBRARY_NAME ) : printlog( "Remove directory" )
    endif
    
    hInitSingleDoc()
    hChangeDoc()
    
    gApplication = "WRITER"
    hCreateDocument()
    
    ToolsMacro_uno
    
    kontext "Makro"
    iNodeCount = hGetNodeCount( MakroAus )
    cDocumentName = hSelectNode( MakroAus, iNodeCount )
    
    Verwalten.click()
    
    hSelectBasicObjectOrganizerTab( 3 )
    
    printlog( "Select document Untitled2" )
    kontext "TabBibliotheken"
    Bibliothek.select( DOCUMENT_POSITION )
    
    ' verify that the correct document is selected
    if ( Bibliothek.getSelText() <> cDocumentName ) then
        warnlog( "Incorrect document selected on libraries tab, aborting" )
        goto endsub
    endif
    
    Neu.click()
    
    kontext "NeueBibliothek"
    BibliotheksName.setText( LIBRARY_NAME )
    NeueBibliothek.ok()
    
    kontext "TabBibliotheken"
    cLibraryName = BibliotheksListe.getSelText()
    
    Bearbeiten.click()

    hInsertMacroFromFile( LIBRARY_NAME )
    
    hCloseBasicIDE()
    
    ToolsMacro_uno
    
    kontext "Makro"
    iNodeCount = hGetNodeCount( MakroAus )
    hSelectNode( MakroAus, iNodeCount )    
    
    Verwalten.click()
    
    hSelectBasicObjectOrganizerTab( 3 )
    
    kontext "TabBibliotheken"
    Bibliothek.select( DOCUMENT_POSITION )
    
    ' verify that the correct document is selected
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
    
    printlog( "Export as library" )
    kontext "ExportBasicLibraryDlg"
    if ( ExportBasicLibraryDlg.exists( 2 ) ) then
    
        ExportAsLibrary.check()
        ExportBasicLibraryDlg.ok()

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
        
    printlog( "Close the macro/libraries organizer" )
    kontext "TabBibliotheken"
    TabBibliotheken.cancel()
    
    printlog( "Cancel macro organizer" )
    kontext "Makro"
    Makro.cancel()
    
    while( getDocumentCount > 0 ) 
        hDestroyDocument()
    wend
    
endcase

