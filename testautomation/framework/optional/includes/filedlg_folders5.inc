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
'* short description : check the internal file dialog ( 1. part )
'*
'\*****************************************************************************

testcase tUpOneLevel2
    
    
    dim iParentDir as integer
    
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
    
    printlog "Enter '..' until we are in the filesystem root"
    for iParentDir = 1 to iDirLevels
    
        Dateiname.setText( ".." )
        
        Oeffnen.click()               

        sCurrentDir = hGetFirstNameFromFileList()
        
        select case iParentDir
        case 1 : sExpectedDir = "level3"
        case 2 : sExpectedDir = "level2"
        case 3 : sExpectedDir = "level1"
        case 4 : sExpectedDir = "alldocs"
        case 5 : sExpectedDir = "includes"
        case 6 : sExpectedDir = "optional"
        end select
        
        if ( iParentDir < 7 ) then
            if ( sCurrentDir = sExpectedDir ) then
                printlog( "Current directory name is: " & sCurrentDir & " [ok]" )
            else
                warnlog( "Error finding directory name:" )
                printlog( "Current directory name is: " & sCurrentDir & " which is NOT ok" )
                printlog( "Expected directory name..: " & sExpectedDir )
            endif
        endif
    next iParentDir
    
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
