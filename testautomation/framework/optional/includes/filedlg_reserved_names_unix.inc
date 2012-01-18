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

testcase tSaveReservedNamesUNIX()

    printlog( "Names that are reserved on WINDOWS but are OK on UNIX" ) 
    
    if ( gPlatGroup = "w95" ) then
        printlog( "Test not relevant for Windows/DOS family of OSes" )
        goto endsub
    endif
    
    dim cStrangeName as string
    dim irc as integer
    dim iCurrentDocType as integer
    
    for iCurrentDocType = 1 to 2 
    
        printlog( "" )
        printlog( "Document Type is: " & hNumericDocType( iCurrentDocType ) )
        
        irc = hSaveLoadDelSuccess( "NUL" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "CLOCK$" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "CON" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "PRN" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "AUX" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "LPT1" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "COM1" , true )
        if ( irc <> 0 ) then warnlog( "failed" )
        
    next iCurrentDocType

endcase

