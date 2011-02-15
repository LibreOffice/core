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
'* short description : Display each help topic
'*
'\******************************************************************************

testcase tHelpRessourceTest( iCurrentAboutItem as integer )

    printlog( "Resource test for help topics - display all topics" )

    dim iTopicCount as integer
    dim iCurrentTopic as integer
    dim cCurrentTopic as string
    dim cPreviousTopic as string
    dim cCurrentAboutItem as string
    dim brc as boolean
   
    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test" )
        goto endsub
    endif

    brc = hSelectHelpTab( "index" )
    if ( not brc ) then
        warnlog( "There was a problem switching to the index tabpage" )
    endif

    Kontext "IndexPage"
    HelpAbout.select( iCurrentAboutItem )
    sleep( 3 )

    cCurrentAboutItem = HelpAbout.getSelText()
    printlog( " " )
    printlog( " * Current Application: " & cCurrentAboutItem )

    iTopicCount = SearchIndex.getItemcount()
    printlog( "For this application " & iTopicCount & " pages will be loaded." )
    printlog( " " )

    cPreviousTopic = "<not yet set>"
    for iCurrentTopic = 1 to iTopicCount 

        try
            SearchIndex.Select( iCurrentTopic )
            cCurrentTopic = SearchIndex.getSelText() 
        catch
            warnlog( "Failed to select entry at " & iCurrentTopic )
            printlog( "Previous entry: " & cPreviousTopic )
            exit for
        endcatch

        try
            DisplayIndex.click()
            WaitSlot( 5000 )
        catch
            warnlog( "#i105476# - Failed to display index item (fatal):" )
            printlog( " - Index...........: " & iCurrentTopic  )
            printlog( " - Topic (current).: " & cCurrentTopic  )
            printlog( " - Topic (previous): " & cPreviousTopic )
            call ExitRestartTheOffice()
            goto endsub
        endcatch
       
        cPreviousTopic = cCurrentTopic

    next iCurrentTopic

    brc =  hCloseHelp()
    if ( not brc ) then
        warnlog( "There was a problem closing the help viewer" )
    endif
   
    call ExitRestartTheOffice()
   
endcase


