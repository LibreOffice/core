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
'*  short description : Execute macros from trusted path
'*
'\******************************************************************************

testcase tSecTrustedPath( cWorkFile as string, iSecLevel as integer )


    dim irc as integer
    dim cMsg as string

    cWorkFile = convertToUrl( convertpath( cWorkFile ) )
    FileOpen( "URL", cWorkFile, "FrameName", "_default"
    irc = hFileWait( false )
    
    ' We have quite a bunch of possible combinations here. Some trigger a security warning
    ' others do not. Files within a trusted path should never prompt.
    
    '         |---------------------------------------------------------|
    '         | Trusted |     Security Level / Warning displayed        |
    '         |  Path   |   Low     |  Medium   |   High    | Very High |
    '         |---------------------------------------------------------|
    '         |  Yes    |     No    |    No     |     No    |    No     |
    '         |---------------------------------------------------------|
    '         |   No*)  |     No    |    Yes    |  Blocked  |  Blocked  |
    '         |---------------------------------------------------------|
    ' *) Covered by test "f_sec_macrosecurity.bas"
       

    if ( irc = -2 ) then
        kontext "SecurityWarning"
        if ( SecurityWarning.exists() ) then
            warnlog( "Macro Security Warning was not expected at this point" )
            SecurityWarning.ok()
        endif
    endif

    Sleep (1)
    printlog( "Verify that the macro is executed" )
    kontext "Active"
    if ( active.exists( 2 ) ) then
        cMsg = active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        if ( cMsg = "TTMacro3" ) then
            printlog( "Macro was executed" )
        else
            warnlog( "Invalid messagebox is open, please check" )
        endif

        active.ok()

    endif

    hDestroyDocument()



endcase

