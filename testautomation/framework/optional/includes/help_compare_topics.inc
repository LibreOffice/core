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
'* short description : 
'*
'\******************************************************************************

testcase tCompareHelpTopics()

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for languages other than en_US" )
        goto endsub
    endif



    ' NOTE: As some of the entries might exists twice or multiple times it is quite
    '       possible that the results are inaccurate. The new implementation of
    '       gCompare2Lists() (hListCompare() as used here) should be able to
    '       handle this. If problems occur, this shuld be the first place to
    '       look.

    const MAX_ENTRIES = 25000

    ' variables related to filenames
    dim sFileOut as string
    dim sFileIn as string
    dim sfileName as string
    
    ' The list that will hold all the entries
    dim aTopicsFromUI( MAX_ENTRIES ) as string
    
    ' some increment operators and temporary variables
    dim iCurrentItem as integer
    dim sCurrentItem as string
    dim sProductName as string
    dim iCurrentApp as integer
    dim iAboutItems as integer
    dim cAboutItem as string
    dim irc as integer
    dim bNextItem as boolean
    
    ' define input and output paths, presetting variables
    sProductName = hStringReplaceChar( gProductName, " ", "_" )
    sFileName = sProductName & "_help_topics_" & gIsoLang & ".txt"
    sFileOut = hGetWorkFile( sFilename )
    sFileIn  = gTesttoolPath & "framework\optional\input\help_browser\"  
    sFileIn  = convertpath( sFileIn & sFileName )
    
    aTopicsFromUI( 0 ) = "0"    
    
    if ( not hOpenHelp() ) then
        warnlog( "Help not open, aborting test" )
        goto endsub
    endif

    hSelectHelpTab( "index" )
    
    ' NOTE: This testcase will not warn about missing About-Items, this is done
    '       by the update-test. But you will get an enormous list of missing
    '       Items.
    iAboutItems = HelpAbout.getItemCount()
    
    for iCurrentApp = 1 to iAboutItems
    
        HelpAbout.select( icurrentApp )
        sleep( 5 )
        cAboutItem = HelpAbout.getSeltext()
    
        iCurrentItem = 1
        bNextItem = true
        
        while( bNextItem ) 

            try
                SearchIndex.select( iCurrentItem )
                sCurrentItem = cAboutItem & " : " & SearchIndex.getSelText()
                hListAppend( sCurrentItem, aTopicsFromUI() )
                iCurrentItem = iCurrentItem + 1
            catch
                printlog( cAboutItem & ": Read " & iCurrentItem & " items" )
                bNextItem = false
            endcatch

        wend
        
    next iCurrentApp
    
    call hCloseHelp()
    
    ' hManageComparisionList takes care of a lot of things like checking
    ' for reference file and comparing or creating a new ref-file
    printlog( "" )
    printlog( "Beginning comparision. This will take a while ..." )
    
    irc = hManageComparisionList( sFileIn, sFileOut, aTopicsFromUI() )    
    hListResultEvaluation( irc , 0 )
    
endcase


