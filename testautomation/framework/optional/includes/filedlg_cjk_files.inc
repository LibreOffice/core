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

testcase tSaveLoadDelFilesCJK()

    printlog( "Save, load and delete files from the hight UTF-8 characters list" )

    if ( not hTestLocale() ) then
        qaerrorlog( "Could not verify UTF-8 locale. Make sure an UTF-8 locale is used!" )
    endif
    
    dim cStrangeName as string
    dim iCounter as integer ' iterator
    dim iRandom  as long    ' random number
    dim brc as boolean
    
    printlog( "Invoke randomizer" )
    call randomize()
    
    printlog( "Generate filenames, run tests" )
    for iCounter = 1 to 2
        
        iRandom = int( 19968 + ( 20911 * rnd ) )
        printlog( "" )
        printlog( " * Using decimal char: " & iRandom )
        
        cStrangeName = hNameGen_append( iRandom )
        hSaveLoadDelSuccess( cStrangeName )
        
        cStrangeName = hNameGen_lead( iRandom )
        hSaveLoadDelSuccess( cStrangeName )
        
        kontext "active"
        if( active.exists() ) then
            qaerrorlog( "#i33964# Document does not exist when the last document is deleted from filepicker" )
            printlog( active.gettext() )
            active.ok()
        endif

        printlog( "Delete the file" )        
        hDeleteFile( hGetWorkPath() & cStrangeName )        
        
        printlog( "Close navigator, if it exists" )
        kontext "Navigator"
        hCloseDialog( Navigator, "close,optional" )
        
    next iCounter
    
    printlog( "Close the document" )
    hDestroyDocument()
    
endcase

