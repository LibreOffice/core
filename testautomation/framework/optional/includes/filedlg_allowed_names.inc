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

testcase tSaveLoadDelFiles()

    
    ' This test checks filenames that resemble Devices and files that contain
    ' the allowed ASCII chars between 32 and 255. The latter is tested by five
    ' random characters
    ' The test should work for all applications and on all languages/locales.
    
    dim cStrangeName as string
    dim iCounter ' iterator
    dim iRandom  ' random number. should end up to be a value between 32 and 255
    dim brc as boolean
    
    dim iCurrentDocType as integer ' increment
    
    for iCurrentDocType = 1 to 1
    
        printlog( "" )
        printlog( "Current Documenttype: " & hNumericDocType( iCurrentDocType ) )
        printlog( "Check if filenames that resemble devicenames are treated ok" )
        printlog( "" )
        
        printlog( "Open a new document" )
        
        brc = hSaveLoadDelSuccess( "COM0"  , true )
        brc = hSaveLoadDelSuccess( "COM10" , true )
        brc = hSaveLoadDelSuccess( "LPT0"  , true )
        brc = hSaveLoadDelSuccess( "LPT10" , true )
        
        ' This test randomizes through a number of characters, a full test of all
        ' possible ascii characters would simply take too long.
        printlog( "" )
        printlog( "Names with allowed ASCII-chars" )
        printlog( "" )
        
        call randomize()
        
        for iCounter = 1 to 3
            
            iRandom = int( 32 + ( 223 * rnd ) )
            printlog( "" )
            printlog( " * Using decimal char: " & iRandom )
            printlog( "" )
            
            
            ' Exclude some special characters that cannot work
            select case iRandom
                
            case  32 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  34 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  35 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  42 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  46 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  47 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  58 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  60 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  62 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  63 : printlog( " - skipping ASCII-Char: " & iRandom )
            case  92 : printlog( " - skipping ASCII-Char: " & iRandom )
            case 124 : printlog( " - skipping ASCII-Char: " & iRandom )
            case 126 : printlog( " - skipping ASCII-Char: " & iRandom )
                
            case else
                
                cStrangeName = hNameGen_append( iRandom )
                brc = hSaveLoadDelSuccess( cStrangeName , TRUE )
                
                cStrangeName = hNameGen_lead( iRandom )
                brc = hSaveLoadDelSuccess( cStrangeName , TRUE )
                
            end select
            
        next iCounter
        
        brc = hDestroyDocument()
        
    next iCurrentDocType

    
endcase


