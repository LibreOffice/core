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

