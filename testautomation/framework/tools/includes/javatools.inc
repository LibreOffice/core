'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
'/******************************************************************************
'*
'*  owner : joerg.Skottke@Sun.Com
'*
'*  short description : Tools to ease working with files including Java
'*
'\******************************************************************************

function hBatchLoadJavaFiles( aFileList() , cIdent as string )

    '///<h3>Load and close a list of files with recovery on error</h3>
    '///<u>Input</u>: A list containing files to load<br>
    '///<u>Returns</u>: No returnvalue
    '///<ul>
    
    const CFN = "hBatchLoadJavaFiles::"
    
    dim iSourceFiles as integer
        iSourceFiles = listcount( aFileList() )
        
    dim iCurrentFile as integer
    dim cCurrentFile as string 
    dim brc as boolean 
    
    '///+<li>Loop through the list, starting at index 2</li>
    for iCurrentFile = 2 to iSourceFiles
    
        cCurrentFile = aFileList( iCurrentFile )
    
        printlog( "" )
        printlog( "Processing file: "  & cCurrentFile )
    
        '///+<li>Load a file, verify</li>
        brc = hFileOpen( aFileList( iCurrentFile ) )
        if ( not brc ) then
            warnlog( "Error while loading: " & cCurrentFile ) 
        endif
        
        '///+<li>close the file, verify</li>
        brc = hDestroyDocument()
        if ( not brc ) then
            warnlog( "Error while closing: " & cCurrentFile ) 
        endif
        
        '///+<li>There should be one document left: The first doc</li>
        if ( getDocumentCount() = 1 ) then
            brc = hIdentifyWriterDoc( cIdent , false )
            if ( not brc ) then
                 warnlog( "Document <" & cIdent & "> is missing, restarting" )
                 call ExitRestartTheOffice()
            endif
            
        elseif ( getDocumentCount() <> 1 ) then
            warnlog( "The number of open documents is incorrect, restarting" )
            call ExitRestartTheOffice()
            
        endif
        
        '///+<li>Check for hs_err_pidXXXX.log files (Java Exceptions)</li>
        brc = hFindCopyRemoveJavaLogs( aFileList( 1 ) )
        if ( not brc ) then
            warnlog( "Java Exceptions were created." )
            printlog( "Find the files in your local OOo-work directory." )
        endif
        
    next iCurrentFile
    '///</ul>
    
end function

'*******************************************************************************

function hFindCopyRemoveJavaLogs( cSourcePath as string ) as boolean

    '///<h3>Search/move hs_err_pidXXXX.log files within a directory recursively</h3>
    '///<u>Input</u>: Start directory<br>
    '///<u>Returns</u>: TRUE if no errors were found
    '///<ul>
    
    ' Reason 1: The files are createn in the CVS tree and must be removed
    ' Reason 2: The files have to be analyzed so the bugs can be fixed
    ' Reason 3: The files must be moved away after each error so they can be
    '           assigned to the correct documents
    
    const CFN = "hFindCopyRemoveJavaLogs::"

    dim aSourceFiles( 1000 ) as string
    dim iSourceFiles as integer
    dim aTargetPath as string
        aTargetPath = hGetWorkPath()
        
    dim iCurrentFile as integer
    dim brc as boolean
        brc = true
        
    dim iSPLen as integer ' length of the source-path string + "/" + next letter
        iSPLen = len( cSourcePath ) + 2

    '///+<li>Look for leftover hs_err_pidXXXX.log files</li>
    iSourceFiles = GetAllFileList( cSourcePath, "hs_err*.log", aSourceFiles() )    
    if ( iSourceFiles > 1 ) then
    
        '///+<li>Print the list to the log</li>
        hListPrint( aSourceFiles(), "New hs_err_pidXXXX.log files exist" )
        brc = false
        
        '///+<li>Copy the hs_err...log files to the local work directory</li>
        for iCurrentFile = 2 to listcount( aSourceFiles() )
        
            ' Create the name of the file we want to copy the hs_err...log to 
            aTargetPath = aTargetPath & mid( aSourceFiles( iCurrentFile ) , iSPLen )
            FileCopy( aSourceFiles( iCurrentFile ) , aTargetPath )
            
            '///+<li>Delete the original log file(s)</li>
            hDeleteFile( aSourceFiles( iCurrentFile ) ) 
            
        next iCurrentFile
        
    else
        Printlog( CFN & "No hs_err_pidXXXX.log file(s) found. Good."        
    endif
    '///</ul>
    
    hFindCopyRemoveJavaLogs() = brc 
    
end function
