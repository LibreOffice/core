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

testcase tSLDForbiddenNamesWin()

    if ( lcase( gPlatGroup ) <> "w95" ) then
        printlog( "No testing for Unix(like)" )
        goto endsub
    endif

    
    ' This test tries to save files that contain characters that are not allowed
    ' in filenames on Windows. This covers the entire ASCII range from 0 to 31.
    ' Currently the ASCII chars 0, 9, 10, and 13 are excluded because we - for some
    ' reason - do not intercept them.
    
    
    dim cStrangeName as string
    dim iCounter as integer
    dim brc as boolean
    dim iDocumentType as integer
    
    
    for iDocumentType = 1 to 6 step 3
    
        printlog( "" )
        printlog( "Check if reserved filenames on Windows are handled ok" )
        printlog( "" )
        
        printlog( " - names with forbidden ASCII-chars" )
        for iCounter = 1 to 31
            
            select case iCounter
                
            case 9 :  printlog( " - skipping ASCII 009 (horizontal tab)" )
            case 10 : printlog( " - skipping ASCII 010 (linefeed)" )
            case 13 : printlog( " - skipping ASCII 013 (carriage return)" )
            case else
                printlog( CHR$(13) & "Using char at decimal position: " & iCounter )
                cStrangeName = hNameGen_append( iCounter )
                brc = hSaveFileExpectFailure( cStrangeName , 0 )
                if ( not brc ) then 
                    warnlog( "failed" )
                else
                    printlog( "Test succeeded" )
                endif
                
            end select
            
        next iCounter
        
    next iDocumentType
    
endcase

