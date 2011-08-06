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
'* short description : check the internal file dialog ( extended tests )
'*
'\******************************************************************************

testcase tCJKFolders()

    
    if ( not hTestLocale() ) then
        warnlog( "Test requires UTF-8 locale" )
        goto endsub
    endif
    
    
    
    dim cStrangeFolder as string
    dim cStrangeFile as string
    dim iCounter as integer ' iterator
    dim iRandom  as long    ' random number.(needs long for cjk chars)
    dim brc as boolean
    
    printlog( CHR$(13) )
    printlog( "Check if CJK-files/folders are handled ok" )
    printlog( CHR$(13) )
    
    printlog( "Open a new document" )
    brc = hCreateDocument()
    
    printlog( CHR$(13) + "Names with CJK-chars" )
    printlog( "" )
    
    call randomize()
    
    for iCounter = 1 to 3
        
        iRandom = int( 19968 + ( 20911 * rnd ) )
        printlog( " * Using decimal char: " & iRandom )
        
        cStrangeFolder = hNameGen_append( iRandom )
        brc = hCreateDirctoryExpectSuccess( cStrangeFolder )
        
        cStrangeFile = hNameGen_append( iRandom )
        cStrangeFile = convertpath( cStrangeFolder & "\" & cStrangeFile )
        brc = hSaveLoadDelSuccess( cStrangeFile , TRUE )
        
        cStrangeFile = hNameGen_lead( iRandom , true )
        cStrangeFile = convertpath( cStrangeFolder & "\" & cStrangeFile )
        brc = hSaveLoadDelSuccess( cStrangeName , true )
        
        brc = hDeleteFileExpectSuccess( cStrangeFolder )
        
    next iCounter
    
    printlog( "Close the document" )
    brc = hDestroyDocument()
    
endcase

