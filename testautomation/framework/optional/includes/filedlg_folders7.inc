'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_folders7.inc,v $
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
'* short description : check the internal file dialog ( 1. part )
'*
'\*****************************************************************************

testcase tUpOneLevel4

    '///<h1>Check all context-menuentries of the &quot;Level-Up&quot; button</h1>
    '///<ul>

    dim iCurrentItem as Integer      ' increment variable
    dim iCurrentItemCount as Integer ' actual number of entries in dropdown menu
    
    
    ' The directory that we want to start in so we know how many steps we have
    ' to the filesystem root (cStartDir is always good for at least 8 steps)
    dim cStartDir as string
        cStartDir = gTesttoolPath & "framework\optional\input\filetest\level1\level2\level3"
        cStartDir = convertpath( cStartDir )
        
    ' This is the threshold of the directory defined above. Usually we will have 
    ' 9 as the expected number of directories but depending on the mountpoint
    ' of the testtool workdirectory the number may vary.
    dim iExpectedItemCount as Integer
        iExpectedItemCount = hGetDirTreeLevel( cStartDir )
        
        ' Windows has one more level (Drives)
        if ( gPlatGroup = "w95" ) then
            iExpectedItemCount = iExpectedItemCount + 1
        endif
        
    printlog( "Test context menu of the Level Up button on the FileOpen dialog" )
    printlog( "Current threshold: " & iExpectedItemCount )
    
    '///+<li>File open</li>
    FileOpen
    
    '///+<li>Walk down to the starting directory: <br>
    '///+ framework\filedlg\input\filetest\level1\level2\level3</li>
    Kontext "OeffnenDlg"
    Dateiname.SetText( cStartDir )
    Oeffnen.Click()
    
    '///+<li>Open the context menu of the &quot;Level Up&quot; button</li>
    UebergeordneterOrdner.Open()
    
    '///+<li>Get the number of entries listed in the menu</li>
    iCurrentItemCount = hMenuItemGetCount()
    Printlog( "Current itemcount: " & iCurrentItemCount )
    
    '///+<li>Verify that at least 9 items exist:</li>  
    '///<ul>
    '///+<li>Too few entries</li>
    if ( iCurrentItemCount < iExpectedItemCount ) then
        warnlog( "Items missing in the menu." )
        printlog( "On Windows the node 'My Computer' might be missing" )
        printlog( "On UNIX filesystem root might be missing" )
    endif
    
    '///+<li>Correct number of entries</li>
    if ( iCurrentItemCount = iExpectedItemCount ) then
        printlog( "The number of entries in the context menu is correct" )
    endif
    
    '///+<li>Too many entries</li>
    if ( iCurrentItemCount > iExpectedItemCount ) then
        warnlog( "Found more menu-items than expected: " &  iCurrentItemCount)
    endif
    '///</ul>
    
    '///+<li>Trigger the first entry, for each entry the menu list gets one shorter
    '///+ as we walk up in the filesystem hierarchy.<br> 
    '///+ Note that the context menu is open at this point</li>
    '///<ul>
    for iCurrentItem = 1 to iCurrentItemCount - 1
        
        '///+<li>Find out how many items should be in the context menu (minus one per loop)</li>
        iExpectedItemCount = iCurrentItemCount - iCurrentItem
        
        '///+<li>Select the first item (it is dynamic)</li>
        hMenuSelectNr ( 1 )
        
        '///+<li>Open the context menu again</li>
        kontext "OeffnenDlg"
        UebergeordneterOrdner.Open()
        
        '///+<li>Verify the number of items</li>
        '///<ul>
        '///+<li>Too few items</li>
        if ( hMenuItemGetCount < iExpectedItemCount )  then
            printlog( "The number of entries is correct" )
        endif
        
        '///+<li>Too many items</li>
        if ( hMenuItemGetCount > iExpectedItemCount )  then
            printlog( "The number of entries is correct" )
        endif
        '///</ul>
        
    next iCurrentItem
    '///</ul>
    
    '///+<li>Select the first item from the context menu again (still open)</li>
    hMenuSelectNr ( 1 )
    
    '///+<li>Verify that we now have 8 items in the context menu</li>
    if ( iCurrentItemCount = 8 ) then
    
        '///+<li>Click the &quot;Level Up&quot; button</li>
        kontext "OeffnenDlg"
        UebergeordneterOrdner.Click()

    endif
    
    '///+<li>Now we should be at filesystem root. Verify that &quot;Level Up&quot; is disabled</li>
    kontext "OeffnenDlg"
    if ( UebergeordneterOrdner.isEnabled() ) then
        warnlog( "Level-Up is enabled, it should be disabled at filesystem root" )
        
    else
        printlog( "Good, button is disabled" )
    endif
    
    '///+<li>Close FileOpen dialog</li>
    kontext "OeffnenDlg"
    OeffnenDlg.Cancel()
    '///</ul>
    
endcase


