'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_folders6.inc,v $
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
'* short description : check the internal file dialog 
'*
'\*****************************************************************************

testcase tUpOneLevel3
    
    '///<h1>Jump to the filesystem root using pathseparator</h1>
    '///+<i>Note that his is performed four times due to a bug that existed in 
    '///+ the past.<br>
    '///+Windows and Unix interpret the pathseparator differently so this test 
    '///+ provides handling of all four combinations (win + /, win + \, unx + / 
    '///+ and unx + \)</i><br>
    '///<ul>
    
    ' We always want to start in a fixed directory structure so the first few dirnames are 
    ' known an can be used for verification
    dim cStartDir as string
        cStartDir = gTesttoolPath & "framework\optional\input\filetest\level1\level2\level3"
        cStartDir = convertpath( cStartDir )
        printlog( "Start-Directory is: " & cStartDir )
        
    ' Find out how many items the array for pathnames must hold (do not use gTesttoolPath here)
    dim iDirLevels as integer
        iDirLevels = hGetDirTreeLevel( cStartDir ) + 2
        
    ' Get the list of directories within the path
    dim cPathArray( iDirLevels ) as string
        iDirLevels = DirNameList( cStartDir , cPathArray() ) 
        
    ' Create an array for the two pathseparators - Windows and Unix/Linux
    dim cPathSeparator( 2 ) as string
        cPathSeparator( 1 ) = "\"
        cPathSeparator( 2 ) = "/"
        
    ' Some increment variables
    dim iPathSeparator as integer
    dim iPathRepeat as integer

    ' Variables needed to verify that we are a filesystem root
    dim iFileOpenItemCount as integer
    dim cCurrentPath as string
    dim iObjectPosition as integer
    
    ' Needed for bug (see below)
    const REPEAT_COUNT = 4
        
    '///+<li>Open the FileOpen dialog</li>
    FileOpen
    
    '///+<li>Change to the start-directory: ..framework\optional\input\filetest\level1\level2\level3</li>
    kontext "OeffnenDlg"
    DateiName.setText( cStartDir )
    Oeffnen.click()
    
    '///+<li>For both pathseparators (/ and \) do:</li>
    '///<ul>
    for iPathSeparator = 1 to 2
    
        printlog( "" )
        printlog( "Trying: " & cPathSeparator( iPathSeparator ) )
    
        '///<li>Repeat this four times (ancient bug, possible crash)</li>
        '///<ul>
        printlog( "Note: This is repeated four times due to ancient bug" )
        for iPathRepeat = 1 to REPEAT_COUNT
        
            '///+<li>Enter the pathseparator into the filename field</li>
            Dateiname.setText( cPathSeparator( iPathSeparator ) )
        
            '///+<li>Click &quot;open&quot;</li>
            Oeffnen.click()
            
            select case iPathSeparator
            case 1: ' backslash
            
                ' This is Unix, Linux
                if ( gPlatGroup = "unx" ) then
                
                    '///+<li>Handle the messagebox (Unix only)</li>
                    kontext "Active"
                    if ( active.exists() ) then
                        active.ok()
                        printlog( "Messagebox for Unix and \ present." )
                    else
                        warnlog( "Error missing for invalid pathspec." )
                    endif

                    '///+<li>- Verify that the directory has not changed for Unix</li>  
                    ' in the ../level3-directory we look for the file filetest.txt
                    kontext "OeffnenDlg"
                    cCurrentPath = hGetFirstNameFromFileList()
                    if ( cCurrentPath <> "filetest.txt" ) then
                        warnlog( "Path should not have changed." )
                    else
                        printlog( "Still in same directory, ok." )
                    endif
                
                else ' Windows, eComStation
                
                    '///+<li>- Verify that we are a the virtual root (drives list) on windows</li>                    
                    kontext "OeffnenDlg"
                    cCurrentPath = hGetFirstNameFromFileList()
                    if ( cCurrentPath <> "a:\" ) then
                        if ( iPathRepeat = 1 ) then
                            qaerrorlog( "Did not reach virtual root or no drive a:\ present" )
                        else 
                            warnlog( "Did not reach virtual root or no drive a:\ present" )
                        endif
                    else
                        printlog( "Switched to virtual root: a:\ - drive. ok" )
                    endif
                    
                endif
                
            case 2: ' slash
            
                ' Unix/Linux
                if ( gPlatGroup = "unx" ) then
    
                    '///+<li>- Verify that we are at top level of the drive</li>
                    ' Note: cPathArray( 1 ) = / so we need second element
                    '       Additionally we have a trailing slash that must be killed
                    cCurrentPath = left( cPathArray( 2 ) , len( cPathArray( 2 ) ) - 1 )
                    kontext "OeffnenDlg"
                    iObjectPosition = hFindFileObjectViaFileOpen( cCurrentPath )
                    if ( iObjectPosition <> 0 ) then
                        printlog( "Root for gTesttoolPath could be found. Good" )
                    else
                        warnlog( "Top directory of gTesttoolPath could not be found" )
                        printlog( "This probably means we are not at the root dir" )
                    endif
                    
                else ' Windows, eComStation

                    '///+<li>- Verify that we are a the virtual root (drives list) on windows</li>                    
                    kontext "OeffnenDlg"
                    cCurrentPath = hGetFirstNameFromFileList()
                    if ( cCurrentPath <> "a:\" ) then
                        warnlog( "Did not reach virtual root or no drive a:\ present" )
                    else
                        printlog( "Switched to virtual root: a:\ - drive. ok" )
                    endif
                    
                endif
                
            end select
            
        next iPathRepeat
        '///</ul>
        
    next iPathSeparator
    '///</ul>
    
    kontext "OeffnenDlg"
    OeffnenDlg.cancel()
    '///+<li>Cancel the FileOpen dialog</li>
    '///</ul>

endcase


