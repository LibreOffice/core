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

    warnlog( "#i78597# - Crash when loading multiple documents using multiselection - excluding test" )
    goto endsub

    '///<h1>Select a number of different documents and load</h1>
    '///<ul>
    
    ' This is a test requested by framework development to be used in multi-
    ' user environments like Terminal Servers. It is used to quickly create
    ' a certain load. Furthermore issues (mostly unconfirmed) have been reported
    ' that loading files using multiselection might lead to a crash.

    '///+<li>Determine the workdirectory</li>
    dim cWorkPath as string
        cWorkPath = gTesttoolPath & "framework\filedlg\input\multiselect\"
        cWorkPath = convertpath( cWorkPath )
        
    dim cMsg as string
        
    dim iCurrentFile as integer
    dim iInitialDocumentCount as integer
    dim iOpenDocuments as integer
    dim iWait as integer 
        iWait = 0
        
    dim brc as boolean
    
    const TESTFILE_COUNT = 60 ' the number of documents in cWorkPath
    
    '///<ul>
    '///+<li>Test enter: Only one document may be open</li>
    if ( getDocumentCount <> 1 ) then
    	printlog( "Restarting office, incorrect number of documents open" )
    	call exitRestartTheOffice()
    endif
    
    '///+<li>Test enter: It must be a WRITER document</li>
    kontext "DocumentWriter"
    if ( not documentwriter.exists() ) then
    	printlog( "Incorrect documenttype, need a WRITER document, restarting" )
    	call exitRestartTheOffice()
    endif
    
    '///+<li>And even more precisely: The first doc!</li>
    if ( not hIdentifyWriterDoc( "The first doc!" , false ) ) then
    	printlog( "Incorrect WRITER document, restarting" )
    	call exitRestartTheOffice()
    endif
    
    '///+<li>Close the navigator if present</li>
    hCloseNavigator()
    '///</ul>
    
    '///+<li>Store the number of open documents (should be exactly one now)</li>
    iInitialDocumentCount = getDocumentCount ' must be exactly 1 at this point
    if ( iInitialDocumentCount <> 1 ) then
    	warnlog( "Expected exactly one writer document (The first doc!)" )
    endif

    '///+<li>Open the filepicker (via menu or toolbar)</li>
    printlog( "FileOpen" )
    FileOpen

    '///+<li>Go to the workdirectory (framework\filedlg\input\multiselect\)</li>
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
    	sleep( 1 )

        if ( iWait = ( TESTFILE_COUNT + 1 ) ) then
        	warnlog( "The documents were not loaded within 60 seconds, abort" )
        	goto endsub
        endif
        
    wend

    printlog( "All documents loaded in " & iWait & " seconds" )

    '///+<li>Close all the files again</li>
    printlog( "Starting to close all documents" )
    for iCurrentFile = 1 to TESTFILE_COUNT
        sleep( 1 )
        FileClose
    next iCurrentFile
    printlog( "Finished closing documents" )
    
    '///+<li>Verify that the correct document is open &quot;The first doc!&quot;</li>
    if ( getDocumentCount <> 1 ) then
    	warnlog( "Only <The first doc!> should be open. This is not the case" )
    	iOpenDocuments = getDocumentCount
    	for iCurrentFile = 1 to iOpenDocuments - 1
    		hDestroyDocument()
        next iCurrentFile
        call ExitRestartTheOffice()
    else
        brc = hIdentifyWriterDoc( "The first doc!" , false )
        if ( not brc ) then
        	warnlog( "Incorrect document order / <The first doc!> is missing" )
        else
            printlog( "Ok, test succeeded" )
        endif
    endif
    
    hCloseNavigator()
    '///</ul>


endcase
