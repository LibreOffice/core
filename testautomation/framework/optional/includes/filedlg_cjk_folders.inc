'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_cjk_folders.inc,v $
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
'* short description : check the internal file dialog ( extended tests )
'*
'\******************************************************************************

testcase tCJKFolders()

    '///<h1>Create and delete folders containing CJK characters</h1>
    
    if ( not hTestLocale() ) then
        warnlog( "Test requires UTF-8 locale" )
        goto endsub
    endif
    
    
    '///<ul>
    
    dim cStrangeFolder as string
    dim cStrangeFile as string
    dim iCounter as integer ' iterator
    dim iRandom  as long    ' random number.(needs long for cjk chars)
    dim brc as boolean
    
    printlog( CHR$(13) )
    printlog( "Check if CJK-files/folders are handled ok" )
    printlog( CHR$(13) )
    
    '///+<li>Create a new document</li>
    printlog( "Open a new document" )
    brc = hCreateDocument()
    
    printlog( CHR$(13) + "Names with CJK-chars" )
    printlog( "" )
    
    '///+<li>Invoke randomizer</li>
    call randomize()
    
    '///+<li>Repeat following steps for at least three asian characters:</li>
    '///<ul>
    for iCounter = 1 to 3
        
        iRandom = int( 19968 + ( 20911 * rnd ) )
        printlog( " * Using decimal char: " & iRandom )
        
        '///+<li>Create a folder with a name containing cjk characters</li>
        cStrangeFolder = hNameGen_append( iRandom )
        brc = hCreateDirctoryExpectSuccess( cStrangeFolder )
        
        '///+<li>Save a file w. trailing cjk characters to the newly created directory</li>
        '///+<li>Load the file again</li>
        '///+<li>Delete the file</li>
        cStrangeFile = hNameGen_append( iRandom )
        cStrangeFile = convertpath( cStrangeFolder & "\" & cStrangeFile )
        brc = hSaveLoadDelSuccess( cStrangeFile , TRUE )
        
        '///+<li>Save a file w. leading cjk characters to the newly created directory</li>
        '///+<li>Load the file again</li>
        '///+<li>Delete the file</li>        
        cStrangeFile = hNameGen_lead( iRandom , true )
        cStrangeFile = convertpath( cStrangeFolder & "\" & cStrangeFile )
        brc = hSaveLoadDelSuccess( cStrangeName , true )
        
        '///+<li>Delete the folder</li>
        brc = hDeleteFileExpectSuccess( cStrangeFolder )
        
    next iCounter
    '///</ul>
    
    '///+<li>Close the document</li>
    printlog( "Close the document" )
    brc = hDestroyDocument()
    '///</ul>
    
endcase

