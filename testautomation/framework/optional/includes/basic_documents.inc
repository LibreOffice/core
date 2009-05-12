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

    '///<H1>Documents in BASIC organizers</H1>
    '///<i>Compare the names of the documents listed in various treelists
    '///+ in Basic Organizer and Basic Object Organizer (Manage...)</i>
    '///<ul>

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
    
    '///+<li>Set the Macro Security Level to &quot;Medium&quot;</li>
    printlog( "" )
    iSecLevel = hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_MEDIUM )
    
    '///+<li>Create one initial document</li>
    hInitSingleDoc()
    
    '///+<li>Write identifier string to the document</li>
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "tBasicDocuments - initial document - discard after test" )

    '///+<li>Create a writer document containing a macro, name it basic.odt</li>
    gApplication = "WRITER"
    printlog( "" )
    brc = hCreateBasicWorkFiles()
    sFileWriter = hGetBasicWorkFile( "current" )

    '///+<li>Create a calc document containing a macro, name it basic.ods</li>
    gApplication = "CALC"
    printlog( "" )
    brc = hCreateBasicWorkFiles()
    sFileCalc = hGetBasicWorkFile( "current" )

    '///+<li>Reload the files we just created</li>
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

    '///+<li>Go to Tools/Macro->Basic Macros</li>
    printlog( "" )
    brc = hOpenBasicOrganizerFromDoc()

    '///+<li>Verify that the treelist contains five items</li>
    hGetVisibleNodeNames( MakroAus , cNodeListA() )
    
    '///+<li>Verify the names of the docs: Untitled, basic(odt), basic(ods)</li>
    printlog( "" )
    iHitCount = hCountMatchesInList( cNodeListA() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif
    
    '///+<li>Click the <Manage...> button</li>
    verwalten.click()
    
    '///+<li>Switch to the modules-Tab (should be open by default)</li>
    printlog( "" )
    brc = hSelectBasicObjectOrganizerTab( 1 )
    ListAllDelete( cNodeListB() )
    hGetVisibleNodeNames( ModulListe , cNodeListB() )
    
    '///+<li>Verify that we have two files with the same name in the list</li>
    printlog( "" )
    iHitCount = hCountMatchesInList( cNodeListA() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif

    '///+<li>Switch to the Dialogs-Tab (the second one)</li>
    '///+<li>Verify that the treelist contains five items</li>
    printlog( "" )
    brc = hSelectBasicObjectOrganizerTab( 2 )
    ListAllDelete( cNodeListB() )
    hGetVisibleNodeNames( ModuleList , cNodeListB() )
    if ( listcount( cNodeListB() ) <> 5 ) then
        warnlog( CFN & "#i49239# Incorrect object count in treelist" )
        hListPrint( cNodeListB() , "List of objects found in treelist" )
    endif     
    
    '///+<li>Verify that we have two files with the same name in the list</li>
    printlog( "" )
    iHitCount = hCountMatchesInList( cNodeListB() , DOC_NAME )
    if ( iHitCount <> 2 ) then
        warnlog( "There should be two documents with the name " & DOC_NAME & "listed." )
    endif

    '///+<li>Switch to Libraries Tab and look at the entries in the listbox</li>
    '///+<li>Verify that we have two files with the same name in the list</li>
    printlog( "" )
    brc = hSelectBasicObjectOrganizerTab( 3 )
    brc = hTestLibraryListBox( cNodeListA() )
    
    '///+<li>Close the Basic Object Organizer</li>
    printlog( "" )
    printlog( CFN & "Closing dialogs..." )
    TabBibliotheken.cancel()
    
    '///+<li>Close the Macro Organizer</li>
    Kontext "Makro"
    Makro.cancel()

    '///+<li>Close the two documents we worked with</li>
    brc = hDestroyDocument()
    brc = hDestroyDocument()
    
    '///+<li>Delete the two workfiles</li>
    brc = hDeleteFile( sFileWriter )
    brc = hDeleteFile( sFileCalc )
    
    '///+<li>Reset the Macro security level</li>
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )
    
    '///+<li>Close the initial document</li>
    hDestroyDocument()
    
    '///</ul>

endcase

'*******************************************************************************

function hTestLibraryListBox( cNodeList() ) as boolean

    const CFN = "hTestLibraryListBox::"

    '///<h3>Compare library-Listbox with Macro-Organizer listbox</h3>
    '///<i>Part of: tBasicDocuments</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>List of items found in Basic Macro Organizer Treelist (MakroAus)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE if all items are identical</li>
    '///+<li>FALSE on any other error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    dim iEntry as integer
    dim cEntry as string
    dim brc as boolean
    dim irc as integer
    
    brc = true
    
    '///+<li>Check that the number of objects is ok</li>
    if ( bibliothek.getItemCount() <> IDOCS ) then
        warnlog( CFN & "Incorrect number of items in listbox" )
        printlog( CFN & "Expected: " & bibliothek.getItemCount() )
        printlog( CFN & "Found...: " & listcount( cNodeList() ) )
        brc = false
    endif
        
    '///+<li>Compare objects 3 -5 (1+2 always differ)</li>
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
    
    '///</ul>
    
end function
