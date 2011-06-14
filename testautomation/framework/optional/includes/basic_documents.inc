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
'*  short description : BASIC organizers and documents
'*
'\******************************************************************************

private const IDOCS = 5
    
testcase tBasicDocuments

    const CFN = "tBasicDocuments::"

    dim sFileWriter as string
    dim sFileCalc as string
    dim iDocumentCount as integer
    dim cNodeListA( 10 ) as string
    dim cNodeListB( 10 ) as string
    dim iSecLevel as integer
    dim iHitCount as integer
    
    ' NOTE: hCreateBasicWorkFiles() creates files of the naming scheme 
    ' basic.odt, ...odc etc. All these files will show up with just "basic"
    ' in the Macro Organizer
    const DOC_NAME = "basic"
    
    printlog( "Set macro security level to medium" )
    iSecLevel = hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_MEDIUM )

    printlog( "Open one single unchanged Writer document" )
    hInitSingleDoc()
    
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "tBasicDocuments - initial document - discard after test" )

    gApplication = "WRITER"
    printlog( "Create a Writer workfilename (build path and filename)" )
    hCreateBasicWorkFiles()
    sFileWriter = hGetBasicWorkFile( "current" )

    gApplication = "CALC"
    printlog( "Create a Calc workfilename (build path and filename)" )
    hCreateBasicWorkFiles()
    sFileCalc = hGetBasicWorkFile( "current" )

    printlog( "Open the writer file" )
    hFileOpen( sFileWriter )
    if ( not hAllowMacroExecution() ) then
        warnlog( "Missing Macro execution dialog. Please check the file/security settings" )
    endif

    printlog( "Open the Calc file" )
    hFileOpen( sFileCalc )
    if ( not hAllowMacroExecution() ) then
        warnlog( "Missing Macro execution dialog. Please check the file/security settings" )
    endif

    printlog( "Open the BASIC organizer" )
    ToolsMacro_uno
    WaitSlot()

    printlog( "Retreive the names of all nodes from the treelist" )
    kontext "Makro"
    hGetVisibleNodeNames( MakroAus , cNodeListA() )
    
    printlog( "Verify that both documents are listed - one Writer, one Calc with identical names" )
    iHitCount = hCountMatchesInList( cNodeListA() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif

    printlog( "Click Manage..." )
    hClickButton( verwalten )
    
    printlog( "Get the nodes list from the object organizer tabs - first tab" )
    hSelectBasicObjectOrganizerTab( 1 )
    ListAllDelete( cNodeListB() )
    hGetVisibleNodeNames( ModulListe , cNodeListB() )
    
    printlog( "Verify that both documents are listed on both relevant tabpages" )
    iHitCount = hCountMatchesInList( cNodeListA() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif

    printlog( "Switch to second tab" )
    hSelectBasicObjectOrganizerTab( 2 )
    ListAllDelete( cNodeListB() )
    hGetVisibleNodeNames( ModuleList , cNodeListB() )
    if ( listcount( cNodeListB() ) <> 5 ) then
        warnlog( CFN & "#i49239# Incorrect object count in treelist" )
        hListPrint( cNodeListB() , "List of objects found in treelist" )
    endif     
    
    printlog( "Verify" )
    iHitCount = hCountMatchesInList( cNodeListB() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif

    printlog( "Third tab (should not list the document" )
    hSelectBasicObjectOrganizerTab( 3 )
    hTestLibraryListBox( cNodeListA() )
    
    printlog( CFN & "Closing dialogs..." )
    hCloseDialog( TabBibliotheken, "cancel" )

    printlog( "Cleanup: Close macro organizer" )
    Kontext "Makro"
    hCloseDialog( Makro, "cancel" )

    printlog( "Cleanup: Close all files" )
    hFileCloseAll()

    printlog( "Cleanup: Remove workfiles" )
    hDeleteFile( sFileWriter )
    hDeleteFile( sFileCalc )

    printlog( "Rest macro security level" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )
    
endcase

'*******************************************************************************

function hTestLibraryListBox( cNodeList() ) as boolean

    const CFN = "hTestLibraryListBox::"
    const EXACT_MATCH = 1

    dim iEntry as integer
    dim cEntry as string
    dim irc as integer

    hTestLibraryListBox() = true
    
    if ( bibliothek.getItemCount() <> IDOCS ) then
        warnlog( CFN & "Incorrect number of items in listbox" )
        printlog( CFN & "Expected: " & bibliothek.getItemCount() )
        printlog( CFN & "Found...: " & listcount( cNodeList() ) )
        hTestLibraryListBox() = false
    endif
        
    for iEntry = 3 to IDOCS
    
        Bibliothek.select( iEntry )
        cEntry = Bibliothek.getSelText()
        
        if ( hCompareSubStrings( cNodeList( iEntry ) , cEntry ) <> EXACT_MATCH ) then
            warnlog( CFN & "Comparision failed" )
            printlog( CFN & "Expected: " & cNodeList( iEntry ) & "<>" & cEntry )
            hTestLibraryListBox() = false
        else
            printlog( CFN & "Comparision succeeded:" & cEntry )
        endif
        
    next iEntry
    
end function
