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

testcase tCompareHelpApps()

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for languages other than en_US" )
        goto endsub
    endif

    
    ' file related variables
    dim sFileOut as string
    dim sFilein as string
    dim sFileName as string
    dim sProductName as string : sProductName = hStringReplaceChar( gProductName, " ", "_" )
    
    ' the array that holds the names of the applications
    dim aTopicsFromUI( 10 ) as string ' max index = 7 expected
    
    ' some incremant variables and temporary stuff
    dim iTopicsFromUI as integer
    dim iCurrentItem as integer
    dim sCurrentItem as string
    dim irc as integer
    dim brc as boolean
    
    ' define input and output paths, presetting variables

    sFileName = sProductName & "_help_applications_" & gIsoLang & ".txt"
    sFileOut = hGetWorkFile( sFilename )
    sFileIn  = gTesttoolPath & "framework\optional\input\help_browser\"  
    sFileIn  = sFileIn & sFileName
    sFileIn  = convertpath( sFileIn )
        
    aTopicsFromUI( 0 ) = "0"

    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test" )
        goto endsub
    endif

    
    hSelectHelpTab( "index" )
    
    iTopicsFromUI = HelpAbout.getItemCount()
    
    for iCurrentItem = 1 to iTopicsFromUI
    
        HelpAbout.select( iCurrentItem )
        sCurrentItem = HelpAbout.getSelText()
        hListAppend( sCurrentItem, aTopicsFromUI() )
        
    next iCurrentItem
    
    call hCloseHelp()
    
    ' Compare the list against a reference or create a new list, if the 
    ' reference does not exist
    printlog( "" )
    printlog( "Beginning comparision" )
    
    irc = hManageComparisionList( sFileIn, sFileOut, aTopicsFromUI() )
    hListResultEvaluation( irc , 0 )
    
endcase


