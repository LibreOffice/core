'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_multiselection.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
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
'*  short description : Select a huge number of documents and load
'*
'\******************************************************************************

testcase tFiledlgMultiselection1()

    '///<h1>Select a number of different documents and load</h1>
    '///<ul>
    
    ' This is a test requested by framework development to be used in multi-
    ' user environments like Terminal Servers. It is used to quickly create
    ' a certain load. Furthermore issues (mostly unconfirmed) have been reported
    ' that loading files using multiselection might lead to a crash.

    '///+<li>Determine the workdirectory</li>
    dim cWorkPath as string : cWorkPath = hGetWorkPath() & "multiselection"
    dim cMsg as string
        
    dim iCurrentFile as integer
    dim iInitialDocumentCount as integer
    dim iOpenDocuments as integer
    dim iWait as integer  : iWait = 0
    
    dim iDocType as integer
    dim iDocument as integer
    dim cFilter( 4 ) as string
        cFilter( 1 ) = "writer8"
        cFilter( 2 ) = "calc8"
        cFilter( 3 ) = "impress8"
        cFilter( 4 ) = "draw8"
        
    dim cFile as string
        
    dim brc as boolean
    
    const TESTFILE_COUNT = 60 ' the number of documents in cWorkPath
    const DOC_IDENTIFIER = "Initial document for multi selection test"

    '///<ul>

    ' Minimum cleanup
    while( getDocumentCount > 0 ) 
        hDestroyDocument()
    wend

    ' create the workdirectory
    mkdir( cWorkPath )
    
    ' create a bulk of files
    for iDocType = 1 to 4
        hNumericDocType( iDocType )
        hNewDocument()
        hChangeDoc()
        for iDocument = 1 to 15
            cFile = cWorkPath & "\" & "test_" & cFilter( iDocType ) & iDocument 
            hFileSaveAsWithFilterKill( cFile, cFilter( iDocType ) )
        next iDocument
        hCloseDocument()
    next iDocType
            
    gApplication = "WRITER"
    hCreateDocument()
    DocumentWriter.typeKeys( DOC_IDENTIFIER )
    
    '///+<li>Close the navigator if present</li>
    hCloseNavigator()
    '///</ul>
    
    '///+<li>Open the filepicker (via menu or toolbar)</li>
    printlog( "FileOpen" )
    FileOpen

    '///+<li>Go to the workdirectory</li>
    printlog( "Go to the workdirectory: " & cWorkPath )
    kontext "OeffnenDlg"
    DateiName.typeKeys( cWorkPath )
    Oeffnen.click()
    
    '///+<li>Check for messagebox - if one comes up we cannot continue</li>
    kontext "Active"
    if ( active.exists( 2 ) ) then
    	cMsg = active.getText()
    	cMsg = hRemoveLineBreaks( cMsg )
    	warnlog( "Unexpected messagebox: " & cMsg )
    	active.ok()
    	kontext "OeffnenDlg"
    	OeffnenDlg.cancel()
    	goto endsub
    endif
    
    '///+<li>Select all documents and click &quot;Open&quot;</li>
    printlog( "Select all documents and load them simultaneously" )
    kontext "OeffnenDlg"
    DateiAuswahl.typeKeys( "<HOME>" )
    DateiAuswahl.typeKeys( "<SHIFT END>" )
    Oeffnen.Click()
    
    '///+<li>Wait max 60 seconds for the documents to load</li>
    while ( getDocumentCount < ( TESTFILE_COUNT + 1 ) ) 
    
    	iWait = iWait + 1
    	sleep( 1 ) ' This is intentional, do not remove or replace

        if ( iWait = ( TESTFILE_COUNT + 1 ) ) then
        	warnlog( "The documents were not loaded within 60 seconds, abort" )
        	goto endsub
        endif
        
    wend

    printlog( "All documents loaded in " & iWait & " seconds" )

    '///+<li>Close all the files again</li>
    printlog( "Starting to close all documents" )
    for iCurrentFile = 1 to TESTFILE_COUNT
        WaitSlot() ' sleep( 1 )
        FileClose
    next iCurrentFile
    printlog( "Finished closing documents" )
    
    '///+<li>Verify that the correct document is open &quot;The first doc!&quot;</li>
    if ( getDocumentCount <> 1 ) then
    	warnlog( "Only the identification document should be open." )
    	iOpenDocuments = getDocumentCount
    	for iCurrentFile = 1 to iOpenDocuments - 1
    		hDestroyDocument()
        next iCurrentFile
        call ExitRestartTheOffice()
    else
        brc = hIdentifyWriterDoc( DOC_IDENTIFIER , false )
        if ( not brc ) then
            warnlog( "The identification document is missing" )
        else
            printlog( "Ok, test succeeded" )
            hDestroyDocument()
        endif
    endif
    
    ' remove workfiles again
    for iDocType = 1 to 4
        hNumericDocType( iDocType )
        for iDocument = 1 to 15
            cFile = cWorkPath & "\" & "test_" & cFilter( iDocType ) & iDocument 
            hDeleteFile( cFile )
        next iDocument
    next iDocType
    
    rmdir( cWorkPath )
    
    
    hCloseNavigator()
    '///</ul>


endcase
