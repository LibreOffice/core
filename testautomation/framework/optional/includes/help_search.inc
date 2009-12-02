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

    dim brc as boolean
    dim irc as integer
    
    dim sFileIn as string
    dim sFileOut as string
    dim aUIList( 100 ) as string
    
    dim cBasePath as string 
        cBasePath = gTesttoolPath & "framework\optional\input\help_browser\"
    dim cDataFile as string
    
    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test" )
        goto endsub
    endif

    
    brc = hSelectHelpTab( "find" )
    if ( not brc ) then
        warnlog( "Could not access requested TabPage, aborting test" )
        call hClosehelp()
        goto endsub
    endif
    
    '----------- Search without filter -----------------------------------------
    printlog( "" )
    printlog( "Search for <java> without any filter" )
    
    SearchFind.setText( "java" )
    FindFullWords.unCheck()
    FindInHeadingsOnly.unCheck()
    
    FindButton.click()

    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
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
    
    SearchFind.setText( "java" )
    FindFullWords.Check()
    FindInHeadingsOnly.unCheck()   
    
    FindButton.click()     
  
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
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
        
    SearchFind.setText( "java" )
    FindFullWords.unCheck()
    FindInHeadingsOnly.Check()   
    
    FindButton.click()     
  
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
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
        
    SearchFind.setText( "java" )
    FindFullWords.Check()
    FindInHeadingsOnly.Check()  
    
    FindButton.click()      
  
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
    cDataFile = gProductName & "_search_headings_and_whole_words_" & gIsoLang & ".txt"
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc       = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    if ( irc <> 0 ) then
        warnlog( "Lists are not identical, please review the log" )
    endif    
    
    
    hCloseHelp()

endcase



