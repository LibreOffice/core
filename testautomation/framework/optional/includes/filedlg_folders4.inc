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
'* short description : check the internal file dialog ( 1. part )
'*
'\*****************************************************************************

testcase tUpOneLevel1
    
    
    dim iClick as integer
    
    ' We always want to start in a fixed directory structure so the first few dirnames are 
    ' known an can be used for verification
    dim cStartDir as string
        cStartDir = gTesttoolPath & "framework\optional\input\filetest\level1\level2\level3"
        cStartDir = convertpath( cStartDir )
        printlog( "Start-Directory is: " & cStartDir )

        
    ' Find out how deep into the filesystem hierarchy we are so we know when we are at the 
    ' filesystem root. We add one because Windows has one more level where the drive icons
    ' are shown
    dim iDirLevels as integer
        iDirLevels = hGetDirTreeLevel( cStartDir ) 
        
    if ( instr( lcase( gtSYSName ) , "win" ) = 1 ) then
        iDirLevels = iDirLevels + 1
        printlog( "Current threshold (Windows) is: " & iDirLevels)
    elseif ( gtSysName = "ecomstation" ) then
        iDirLevels = iDirLevels + 1
        printlog( "Current threshold (eComStation) is: " & iDirLevels)
    else
        printlog( "Current threshold (Unix/Linux) is: " & iDirLevels)
    endif
        
    dim sCurrentDir as string
    dim sExpectedDir as string
    
    
    FileOpen
    
    Kontext "OeffnenDlg"
    Dateiname.SetText( cStartDir )
    Oeffnen.Click()
    
    printlog "Click the Level-Up button until we are in the filesystem root"
    for iClick = 1 to iDirLevels
    
        if ( UebergeordneterOrdner.exists() ) then
        
            if ( UebergeordneterOrdner.isEnabled() ) then
            
                UebergeordneterOrdner.Click()
            else
                warnlog( "Level-Up button is disabled" )
            endif
        else
            warnlog( "Level-Up button does not exist" )
        endif

        sCurrentDir = hGetFirstNameFromFileList()
        
        select case iClick
        case 1 : sExpectedDir = "level3"
        case 2 : sExpectedDir = "level2"
        case 3 : sExpectedDir = "level1"
        case 4 : sExpectedDir = "alldocs"
        case 5 : sExpectedDir = "includes"
        case 6 : sExpectedDir = "optional"
        end select
        
        if ( iClick < 7 ) then
            if ( sCurrentDir = sExpectedDir ) then
                printlog( "Current directory name is: " & sCurrentDir & " [ok]" )
            else
                warnlog( "Error finding directory name:" )
                printlog( "Current directory name is: " & sCurrentDir & " which is NOT ok" )
                printlog( "Expected directory name..: " & sExpectedDir )
            endif
        endif
    next iClick
    
    kontext "OeffnenDlg"
    
    if ( UebergeordneterOrdner.exists() ) then
    
        if ( not UebergeordneterOrdner.isEnabled() ) then
            printlog( "Level-Up button is disabled, good" )
        else
            warnlog( "The Level-up button should not be enabled for filesystem root" )
        endif
    else
        warnlog( "Level-Up button does not exist" )
    endif    

    OeffnenDlg.Cancel()
    
endcase



