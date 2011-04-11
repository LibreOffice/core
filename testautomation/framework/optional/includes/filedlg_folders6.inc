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
'* short description : check the internal file dialog 
'*
'\*****************************************************************************

testcase tUpOneLevel3
    
    
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
        
    FileOpen
    
    kontext "OeffnenDlg"
    DateiName.setText( cStartDir )
    Oeffnen.click()
    
    for iPathSeparator = 1 to 2
    
        printlog( "" )
        printlog( "Trying: " & cPathSeparator( iPathSeparator ) )
    
        printlog( "Note: This is repeated four times due to ancient bug" )
        for iPathRepeat = 1 to REPEAT_COUNT
        
            Dateiname.setText( cPathSeparator( iPathSeparator ) )
        
            Oeffnen.click()
            
            select case iPathSeparator
            case 1: ' backslash
            
                ' This is Unix, Linux
                if ( gPlatGroup = "unx" ) then
                
                    kontext "Active"
                    if ( active.exists() ) then
                        active.ok()
                        printlog( "Messagebox for Unix and \ present." )
                    else
                        warnlog( "Error missing for invalid pathspec." )
                    endif

                    ' in the ../level3-directory we look for the file filetest.txt
                    kontext "OeffnenDlg"
                    cCurrentPath = hGetFirstNameFromFileList()
                    if ( cCurrentPath <> "filetest.txt" ) then
                        warnlog( "Path should not have changed." )
                    else
                        printlog( "Still in same directory, ok." )
                    endif
                
                else ' Windows, eComStation
                
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
        
    next iPathSeparator
    
    kontext "OeffnenDlg"
    OeffnenDlg.cancel()

endcase


