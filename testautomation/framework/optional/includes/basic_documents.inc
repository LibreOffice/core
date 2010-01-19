'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_documents.inc,v $
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
'/************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : BASIC organizers and documents
'*
'\******************************************************************************

private const IDOCS = 5
    
testcase tBasicDocuments

    qaerrorlog( "#i90435# Untitled documents unnumbered in BASIC organizer" )
    goto endsub

    const CFN = "tBasicDocuments::"


    dim brc as boolean
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
    
    printlog( "" )
    iSecLevel = hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_MEDIUM )
    
    hInitSingleDoc()
    
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "tBasicDocuments - initial document - discard after test" )

    gApplication = "WRITER"
    printlog( "" )
    brc = hCreateBasicWorkFiles()
    sFileWriter = hGetBasicWorkFile( "current" )

    gApplication = "CALC"
    printlog( "" )
    brc = hCreateBasicWorkFiles()
    sFileCalc = hGetBasicWorkFile( "current" )

    printlog( "" )
    brc = hFileOpen( sFileWriter )
    brc = hAllowMacroExecution()
    if ( not brc ) then 
        warnlog( "Missing Macro execution dialog. Please check the file/security settings" )
    endif
    brc = hFileOpen( sFileCalc )
    brc = hAllowMacroExecution()
    if ( not brc ) then 
        warnlog( "Missing Macro execution dialog. Please check the file/security settings" )
    endif

    printlog( "" )
    brc = hOpenBasicOrganizerFromDoc()

    hGetVisibleNodeNames( MakroAus , cNodeListA() )
    
    printlog( "" )
    iHitCount = hCountMatchesInList( cNodeListA() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif
    
    verwalten.click()
    
    printlog( "" )
    brc = hSelectBasicObjectOrganizerTab( 1 )
    ListAllDelete( cNodeListB() )
    hGetVisibleNodeNames( ModulListe , cNodeListB() )
    
    printlog( "" )
    iHitCount = hCountMatchesInList( cNodeListA() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif

    printlog( "" )
    brc = hSelectBasicObjectOrganizerTab( 2 )
    ListAllDelete( cNodeListB() )
    hGetVisibleNodeNames( ModuleList , cNodeListB() )
    if ( listcount( cNodeListB() ) <> 5 ) then
        warnlog( CFN & "#i49239# Incorrect object count in treelist" )
        hListPrint( cNodeListB() , "List of objects found in treelist" )
    endif     
    
    printlog( "" )
    iHitCount = hCountMatchesInList( cNodeListB() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif

    printlog( "" )
    brc = hSelectBasicObjectOrganizerTab( 3 )
    brc = hTestLibraryListBox( cNodeListA() )
    
    printlog( "" )
    printlog( CFN & "Closing dialogs..." )
    TabBibliotheken.cancel()
    
    Kontext "Makro"
    Makro.cancel()

    brc = hDestroyDocument()
    brc = hDestroyDocument()
    
    brc = hDeleteFile( sFileWriter )
    brc = hDeleteFile( sFileCalc )
    
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )
    
    hDestroyDocument()
    

endcase

'*******************************************************************************

function hTestLibraryListBox( cNodeList() ) as boolean

    const CFN = "hTestLibraryListBox::"

    

    dim iEntry as integer
    dim cEntry as string
    dim brc as boolean
    dim irc as integer
    
    brc = true
    
    if ( bibliothek.getItemCount() <> IDOCS ) then
        warnlog( CFN & "Incorrect number of items in listbox" )
        printlog( CFN & "Expected: " & bibliothek.getItemCount() )
        printlog( CFN & "Found...: " & listcount( cNodeList() ) )
        brc = false
    endif
        
    for iEntry = 3 to IDOCS
    
        Bibliothek.select( iEntry )
        cEntry = Bibliothek.getSelText()
        
        irc = hCompareSubStrings( cNodeList( iEntry ) , cEntry )
        if ( irc <> 1 ) then
            warnlog( CFN & "Comparision failed" )
            printlog( CFN & "Expected: " & cNodeList( iEntry ) & "<>" & cEntry )
            brc = false
        else
            printlog( CFN & "Comparision succeeded:" & cEntry )
        endif
        
    next iEntry
    
    
end function
