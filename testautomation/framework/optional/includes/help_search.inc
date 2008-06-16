'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: help_search.inc,v $
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
'*  short description : Search for a string, apply restrictions, verify
'*
'\******************************************************************************

testcase tHelpSearch

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for languages other than en_US" )
        goto endsub
    endif

    '///<H1>Search for a string, apply restrictions, verify</H1>
    '///<h2>help_search.bas::tHelpSearch</h2>
    '///<ul>
    dim brc as boolean
    dim irc as integer
    
    dim sFileIn as string
    dim sFileOut as string
    dim aUIList( 100 ) as string
    
    dim cBasePath as string 
        cBasePath = gTesttoolPath & "framework\optional\input\help_browser\"
    dim cDataFile as string
    
    '///+<li>Open the help browser</li>
    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test" )
        goto endsub
    endif

    
    '///+<li>Switch to the Search-tab (find)</li>
    brc = hSelectHelpTab( "find" )
    if ( not brc ) then
        warnlog( "Could not access requested TabPage, aborting test" )
        call hClosehelp()
        goto endsub
    endif
    
    '----------- Search without filter -----------------------------------------
    printlog( "" )
    printlog( "Search for <java> without any filter" )
    
    '///+<li>Enter &quot;Java&quot; into the search entryfield</li>
    '///+<li>Uncheck Whole words only and headings only</li>
    SearchFind.setText( "java" )
    FindFullWords.unCheck()
    FindInHeadingsOnly.unCheck()
    
    '///+<li>Click &quot;Search&quot;</li>
    FindButton.click()

    '///+<li>Retrieve all entries from the list</li>
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
    '///+<li>Compare content to a reference or create a new ref-file</li>
    cDataFile = gProductName & "_search_without_filter_" & gIsoLang & ".txt"
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc       = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    if ( irc <> 0 ) then
        warnlog( "Lists are not identical, please review the log" )
    endif
    
    ' ------------ Search whole words only -------------------------------------
    printlog( "" )
    printlog( "Search for <java>, whole words only" )
    
    '///+<li>Apply filter: Whole words only</li>
    SearchFind.setText( "java" )
    FindFullWords.Check()
    FindInHeadingsOnly.unCheck()   
    
    '///+<li>Click &quot;Search&quot;</li>
    FindButton.click()     
  
    '///+<li>Retrieve all entries from the list</li>
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
    '///+<li>Compare content to a reference or create a new ref-file</li>
    cDataFile = gProductName & "_search_whole_words_only_" & gIsoLang & ".txt"    
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc       = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    if ( irc <> 0 ) then
        warnlog( "Lists are not identical, please review the log" )
    endif

    ' ------------ Search headings only ----------------------------------------
    printlog( "" )
    printlog( "Search for <java>, headings only" )    
        
    '///+<li>Apply filter: Whole words only</li>
    SearchFind.setText( "java" )
    FindFullWords.unCheck()
    FindInHeadingsOnly.Check()   
    
    '///+<li>Click &quot;Search&quot;</li>
    FindButton.click()     
  
    '///+<li>Retrieve all entries from the list</li>
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
    '///+<li>Compare content to a reference or create a new ref-file</li>
    cDataFile = gProductName & "_search_headings_only_" & gIsoLang & ".txt"    
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc       = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    if ( irc <> 0 ) then
        warnlog( "Lists are not identical, please review the log" )
    endif

    ' ------------ Search headings and whole words -----------------------------
    printlog( "" )
    printlog( "Search for <java>, limit results to whole words and headings" )    
        
    '///+<li>Apply filter: Headings and whole words</li>
    SearchFind.setText( "java" )
    FindFullWords.Check()
    FindInHeadingsOnly.Check()  
    
    '///+<li>Click &quot;Search&quot;</li>
    FindButton.click()      
  
    '///+<li>Retrieve all entries from the list</li>
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
    '///+<li>Compare content to a reference or create a new ref-file</li>
    cDataFile = gProductName & "_search_headings_and_whole_words_" & gIsoLang & ".txt"
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc       = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    if ( irc <> 0 ) then
        warnlog( "Lists are not identical, please review the log" )
    endif    
    
    
    hCloseHelp()

endcase



