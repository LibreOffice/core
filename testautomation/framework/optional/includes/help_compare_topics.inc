'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : gregor.hartmann@oracle.com
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


