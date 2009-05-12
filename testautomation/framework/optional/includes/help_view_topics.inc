'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: help_view_topics.inc,v $
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
'/************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : Display each help topic
'*
'\******************************************************************************

testcase tHelpRessourceTest( iCurrentAboutItem as integer )

    '///<h1>Ressourcetest for OpenOffice.org help viewer</h1>

    dim iTopicCount as integer
    dim iCurrentTopic as integer
    dim cCurrentTopic as string
    dim cPreviousTopic as string
    dim cCurrentAboutItem as string
    dim brc as boolean
   
    '///<i>help.bas::tHelpRessourceTest</i><br>
    '///<ul>
    '///+<li>Open help</li>
    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test" )
        goto endsub
    endif

    '///+<li>Switch to the index tabpage</li>
    brc = hSelectHelpTab( "index" )
    if ( not brc ) then
        warnlog( "There was a problem switching to the index tabpage" )
    endif

    '///+<li>Set and printout the current application for which to display the index</li>
    Kontext "IndexPage"
    HelpAbout.select( iCurrentAboutItem )
    sleep( 3 )

    '///+<li>Retrieve the text from the Multilistbox</li>
    cCurrentAboutItem = HelpAbout.getSelText()
    printlog( " " )
    printlog( " * Current Application: " & cCurrentAboutItem )

    '///+<li>Get number of entries in combo-box (help topics)</li>
    iTopicCount = SearchIndex.getItemcount()
    printlog( "For this application " & iTopicCount & " pages will be loaded." )
    printlog( " " )

    '///+<li>Open every item in the index, one by one, give enough time to load</li>
    '///<ul>
    cPreviousTopic = "<not yet set>"
    for iCurrentTopic = 1 to iTopicCount 

        try
            '///+<li>select one item from the list</li>
            SearchIndex.Select( iCurrentTopic )
            
            '///+<li>Retrieve the name of the entry</li>
            cCurrentTopic = SearchIndex.getSelText() 
            'printlog( " - " & cCurrentTopic )    
        catch
            warnlog( "Failed to select entry at " & iCurrentTopic )
            printlog( "Previous entry: " & cPreviousTopic )
            exit for
        endcatch

        try
            '///+<li>Click the "Display" button</li>
            DisplayIndex.click()
            WaitSlot( 5000 )
        catch
            warnlog( "Failed to display index item (fatal):" )
            printlog( " - Index...........: " & iCurrentTopic  )
            printlog( " - Topic (current).: " & cCurrentTopic  )
            printlog( " - Topic (previous): " & cPreviousTopic )
            call ExitRestartTheOffice
            goto endsub
        endcatch
      
       
        cPreviousTopic = cCurrentTopic

    next iCurrentTopic
    '///</ul>

    '///+<li>Close the help-viewer</li>
    brc =  hCloseHelp()
    if ( not brc ) then
        warnlog( "There was a problem closing the help viewer" )
    endif
   
    '///+<li>Exit and restart the office</li>
    call ExitRestartTheOffice()
    '///</ul>
   
endcase


