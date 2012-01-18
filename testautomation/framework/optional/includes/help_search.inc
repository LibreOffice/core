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
'*  short description : Search for a string, apply restrictions, verify
'*
'\******************************************************************************

testcase tHelpSearch

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for languages other than en_US" )
        goto endsub
    endif

    const RC_LISTS_IDENTICAL = 0

    dim brc as boolean
    dim irc as integer
    
    dim sFileIn as string
    dim sFileOut as string
    dim aUIList( 100 ) as string
    
    dim cBasePath as string 
        cBasePath = gTesttoolPath & "framework\optional\input\help_browser\"
    dim cDataFile as string
    dim sProductName as string : sProductName = hStringReplaceChar( gProductName, " ", "_" )

    printlog( "Open Help Browser" )
    if ( not hOpenHelp() ) then
        warnlog( "Help not open, aborting test" )
        goto endsub
    endif

    printlog( "Go to the Find-Tabpage" )
    if ( not hSelectHelpTab( "find" ) ) then
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


    cDataFile = sProductName & "_search_without_filter_" & gIsoLang & ".txt"
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    hListResultEvaluation( irc , RC_LISTS_IDENTICAL )
    
    ' ------------ Search whole words only -------------------------------------
    printlog( "" )
    printlog( "Search for <java>, whole words only" )
    
    SearchFind.setText( "java" )
    FindFullWords.Check()
    FindInHeadingsOnly.unCheck()   
    
    FindButton.click()     
  
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
    cDataFile = sProductName & "_search_whole_words_only_" & gIsoLang & ".txt"
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    hListResultEvaluation( irc , RC_LISTS_IDENTICAL )

    ' ------------ Search headings only ----------------------------------------
    printlog( "" )
    printlog( "Search for <java>, headings only" )    
        
    SearchFind.setText( "java" )
    FindFullWords.unCheck()
    FindInHeadingsOnly.Check()   
    
    FindButton.click()     
  
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
    cDataFile = sProductName & "_search_headings_only_" & gIsoLang & ".txt"
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    hListResultEvaluation( irc , RC_LISTS_IDENTICAL )

    ' ------------ Search headings and whole words -----------------------------
    printlog( "" )
    printlog( "Search for <java>, limit results to whole words and headings" )    
        
    SearchFind.setText( "java" )
    FindFullWords.Check()
    FindInHeadingsOnly.Check()  
    
    FindButton.click()      
  
    ListAllDelete( aUIList() )
    hGetListItems( Result, aUIList() )
    
    cDataFile = sProductName & "_search_headings_and_whole_words_" & gIsoLang & ".txt"
    sFileOut  = hGetWorkFile( cDataFile )
    sFileIn   = convertpath( cBasePath & cDataFile )

    irc = hManageComparisionList( sFileIn, sFileOut, aUIList() )
    hListResultEvaluation( irc , RC_LISTS_IDENTICAL )
    
    hCloseHelp()

endcase



