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
'*  short description : Verify that hiding modules works
'*
'\******************************************************************************

testcase tBasicIdeModuleHide

    
    const CFN = "tBasicIdeModuleHide::"

    const TESTMACRO = "TTMacro1"
    const ERR_NO_LINES_INSERTED = 0

    const RC_SUCCESS = 0
    const RC_TIMEOUT = -1

    const TAB_MODULES = 1

    dim rc as integer
    dim brc as boolean
    dim cDefaultTabName as string

    gApplication = "WRITER"
    call hNewDocument()
    
    ToolsMacro_uno
    hCreateModuleForDoc()
    
    printlog( "Insert macro" )
    if ( hInsertMacroFromFile( TESTMACRO ) > ERR_NO_LINES_INSERTED ) then
       printlog( CFN & "Macro has been written successfully" )
    else
       warnlog( CFN & "Failed to insert macro" )
    endif

    printlog( "Hide the module" )
    if ( hHideModule() ) then 

        ' hTestMacro is expected to fail, so we jump to the catch statement
        if ( hInsertMacroFromFile( TESTMACRO ) <> ERR_NO_LINES_INSERTED ) then
            warnlog( "For some reason the original module is still visible" )
        else
            printlog( "The edit window is not accessible, good" )
        endif

        if ( hOpenBasicObjectOrganizer( TAB_MODULES ) ) then

            modulliste.typekeys( "<END><RIGHT><DOWN><RIGHT><DOWN>" )
            if ( hClickButton( Bearbeiten ) = RC_TIMEOUT ) then
                warnlog( "#i35097# Crash when editing last module" )
                goto endsub
            endif

            if ( not hBasicIDERunMacro( TESTMACRO ) ) then
                warnlog( "Incorrect macro executed/macro missing" )
            endif

            hCloseBasicIDE()
            hFileCloseAll()

        else
            warnlog( "restarting the office to recover from errors" )
            call exitRestartTheOffice()
        endif

    else
        warnlog( "Some unexpected error occurred while trying to hide the module" )
    endif
    
endcase
