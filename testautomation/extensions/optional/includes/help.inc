'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'*  short description : Extensible help
'*
'\******************************************************************************

testcase tExtensibleHelp

    '///<h1>Extensible help</h1>

    '///<u><pre>Synopsis</pre></u>Beginning with CWS ab38 extensions can extend the help application with index entries and content<br>
    '///<u><pre>Specification document</pre></u><a href=http://specs.openoffice.org/appwide/help/ExtensibleHelp.odt>
    '///+http://specs.openoffice.org/appwide/help/ExtensibleHelp.odt</a><br>
    '///<u><pre>Files used:</pre></u>.../extensions/features/input/extensible_help/*.info, *.oxt<br>
    '///<u><pre>Test case specification</pre></u>
    
    ' This is the base path to the input directory where all workfiles are located
    dim cExtName as string
    dim cExtPath as string
        cExtPath = gTesttoolPath & "extensions\optional\input\extensible_help\"
        cExtPath = convertpath( cExtPath )
        
    ' This is the configuration file for the test extensions. This approach was chosen
    ' because the extensions have a lot in common but not enough to make a general 
    ' routine without any special cases. It is possible to add more extensions easily
    ' by just configuring them in the config file. Ideally no canges to the test case
    ' itself are required.
    const CONFIG_FILE = "extension.info"
    dim cCfgPath as string
        cCfgPath = cExtPath
        cCfgPath = convertpath( cCfgPath & CONFIG_FILE )
        
    ' In this array we store the config file. Always make sure it is large enough
    ' to hold the entire file
    dim cCfgArray( 60 ) as string
    
    ' Some returnvalues/status variables, general purpose
    dim iStatus as integer
    dim bStatus as boolean
    
    ' Control variables for the outer loop (actions for each extension)
    dim iCurrentExtension as integer
    dim iExtensionCount as integer
    
    ' Control variables for the inner loop (items to test for an extension)
    dim cCurrentHelpItem as string
    dim iCurrentHelpItem as integer
    dim iHelpItemCount as integer
    
    ' Values retrieved from the config file or the help content
    dim cItem as string
    dim cHead as string
    dim cLine as string
    
    ' Correct the position within the index list. If we have a main item with
    ' sub-items we might need to move a few entries down in the list to get the
    ' sub-items.
    dim cMove as string
    dim iMove as integer
    dim iMoveDown as integer
    
    do while ( getDocumentCount > 0 ) 
    	call hCloseDocument()
    loop
    
    '///<ul>  
    '///+<li>Open a writer document (so the help comes up for writer)</li>
    gApplication = "WRITER"
    call hNewDocument()
    if ( getDocumentCount <> 1 ) then
        warnlog( "Fatal: Unable to open new writer document" )
        sResetTheOffice
        goto endsub
    endif
    
    '///+<li>Read the config file - determine the number of test-extensions</li>
    iStatus = hGetDataFileSection( cCfgPath, cCfgArray(), "files", "", "" )
    iExtensionCount = int( hGetValueForKeyAsString( cCfgArray(), "count" ) )
    printlog( "Read configuration for " & iExtensionCount & " files." )
    
    '///+<li>Cycle through the configured test-extensions</li>
    '///<ul>
    for iCurrentExtension = 1 to iExtensionCount
    
        '///+<li>Read the config file - get settings for current extension</li>
        hGetDataFileSection( cCfgPath, cCfgArray(), iCurrentExtension, "", "" )
        cExtPath = cExtPath & hGetValueForKeyAsString( cCfgArray(), "file" )
        cExtName = hGetValueForKeyAsString( cCfgArray(), "name" )
        
        
        '///+<li>Install the test-extension, cleanup and quit on failure</li>
        iStatus = hExtensionAddGUI( cExtPath , "NoUpdate,NoLicense,InstallForUser,Verbose" )
        if ( iStatus < 1 ) then
            warnlog( "Fatal: Adding extension failed with rc = " & iStatus )
            sResetTheOffice
            goto endsub
        endif
        
        '///+<li>Open the help browser</li>
        HelpContents
        kontext "StarOfficeHelp"
        if ( StarOfficeHelp.exists( 2 ) ) then
        
            '///+<li>Switch to the index tabpage</li>
            TabControl.setPage( IndexPage )
        else
            warnlog( "Unable to open help browser" )
            hCloseDocument()
            sResetTheOffice()
            goto endsub
        endif
        
        '///+<li>Search the help index for the items described in the config file</li>
        iHelpItemCount = int( hGetValueForKeyAsString( cCfgArray(), "items" ) )
        printlog( "Testing " & iHelpItemCount & " items" )
        '///<ul>
        for iCurrentHelpItem = 1 to iHelpItemCount
        
            printlog( "" )
            printlog( "Testing item " & iCurrentHelpItem )
        
            '///+<li>Get the test strings from the config file</li>
            cItem = "hindex_" & iCurrentHelpItem
            cMove = "h_move_" & iCurrentHelpItem
            cHead = "header_" & iCurrentHelpItem
            cItem = hGetValueForKeyAsString( cCfgArray(), cItem )
            cHead = hGetValueForKeyAsString( cCfgArray(), cHead )
            iMove = int( hGetValueForKeyAsString( cCfgArray(), cMove )
            
            '///+<li>Enter the search term into the search field of the index page</li>
            kontext "StarOfficeHelp"
            SearchIndex.setText( cItem ) : WaitSlot()
            
            for iMoveDown = 1 to iMove
                SearchIndex.typeKeys( "<DOWN>" )
            next iMoveDown
            
            '///+<li>Display the associated help page (content) by clicking on &quot;Display&quot;</li>
            DisplayIndex.click() : WaitSlot()
            
            '///+<li>Get the header from the content</li>
            kontext "HelpContent"
            HelpContent.typeKeys( "<MOD1 HOME>" )
            HelpContent.typeKeys( "<SHIFT RIGHT>" , len( cHead ) )
            hUseAsyncSlot( "EditCopy" )
            cLine = GetClipBoardText
            
            '///+<li>Compare the config string with the header</li>
            if ( cLine <> cHead ) then
                if ( iCurrentHelpItem = 1 and gPlatGroup <> "Win95" ) then
                    qaerrorlog( "Header is incorrect, slower navigation on Unix(like)" )
                else
                    warnlog( "Header is incorrect. Please check." )
                endif
                printlog( "Found...: " & cLine )
                printlog( "Expected: " & cHead )
            else
                printlog( "The header is correct: " & cLine )
            endif
        
        next iCurrentHelpItem
        '///</ul>
        
        '///+<li>Close the help browser</li>
        kontext "StarOfficeHelp"
        StarOfficeHelp.typeKeys( "<MOD1 F4>" )

        '///+<li>Remove the extension</li>
        hExtensionRemoveGUI( cExtName )
        
        
    next iCurrentExtension
    '///</ul>
    
    '///+<li>Close the writer document</li>
    call hCloseDocument()
    if ( getDocumentCount <> 0 ) then
        warnlog( "Fatal: Failed to close writer document" )
        sResetTheOffice
        goto endsub
    endif
        
    '///</ul>

endcase

