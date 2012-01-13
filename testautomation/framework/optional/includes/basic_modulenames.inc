'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
'*
'*  short description : Test modulenames on the tabbar
'*
'\******************************************************************************

testcase tBasicIdeModuleNames

    printlog( "Modulenames in BASIC IDE" )

    const CMODULE_VALID1 = "tValidName"
    const CMODULE_VALID2 = "tValidToo"
    const TEST_MACRO = "TTMacro1"

    const RC_SUCCESS = 0
    const RC_FAILURE = 1
    const ERR_NO_LINES_INSERTED = 0

    dim iCurrentName as integer
    dim cTabName as string

    gApplication = "WRITER"
    hCreateDocument()

    hInitBasicIde( CMODULE_VALID1 )
    if ( hInsertMacroFromFile( TEST_MACRO ) > ERR_NO_LINES_INSERTED ) then
   
        cTabName = hGetTabNameFromOrganizer()

        Randomize
        iCurrentName = Int( 24 * RND ) + 1 ' Range from 1 to 24

        printlog( "Trying with invalid modulename" )
        cTabName = hCreateInvalidModuleName( iCurrentName )
        hRenameTab( cTabName )
        if ( hHandleInvalidNameWarning( cTabname ) ) then
            printlog( "Warning for invalid modulename displayed" )
        else
            warnlog( "Warning for invalid modulename is missing" )
        endif

        printlog( "Trying with valid modulename" )
        hRenameTab( CMODULE_VALID2 )
        if ( hHandleInvalidNameWarning( CMODULE_VALID2 ) ) then
            warnlog( "Failed to set valid modulename" )
        endif

    else
        warnlog( "Failed to insert macro" )
    endif
    
    hFileCloseAll()

endcase

'*******************************************************************************

testcase tInvalidModuleNames

    printlog( "Modulenames in BASIC Organizer" )

    const MAX_WAIT = 2
    const OK_BUTTON_ONLY = 1

    dim iCurrentName as Integer
    dim sSeparator as String 
    dim cModuleName as string

    Randomize
    iCurrentName = Int( 24 * RND ) + 1 ' Range from 1 to 24
    cModuleName = hCreateInvalidModuleName( iCurrentName )
    printlog( "Trying module name: " & cModuleName )

    hCreateDocument()
    ToolsMacro_uno

    ' hCreateModuleForDoc should usually end with the BASIC IDE open
    ' but we want it to fail by providing an invalid module name
    if ( hCreateModuleForDoc( cModuleName ) ) then
        warnlog( "Basic-Ide should not open for invalid module names" )
    else
        kontext "Active"
        if ( Active.exists( MAX_WAIT ) ) then
            if( Active.getButtonCount() = OK_BUTTON_ONLY ) then

                printlog( "Messagebox <Invalid name> displayed: " & Active.getText() )
                hCloseDialog( Active, "ok" )

                kontext "NeuesModul"
                if ( NeuesModul.exists( MAX_WAIT ) ) then
                    hCloseDialog( NeuesModul, "cancel" )

                    kontext "Makro"
                    if ( Makro.exists() ) then
                        hCloseDialog( Makro, "close" )
                    else
                        warnlog( "Test should be back on macro dialog" )
                    endif
                else
                    warnlog( "<NeuesModul> dialog should show after failed naming attempt" )
                endif
            else
                warnlog( "Unexpected messagebox: " & Active.getText() )
                hCloseDialog( Active, "close" )
            endif
        else
            warnlog( "<Invalid name> Messagebox missing" )
        endif
    endif

    hFileCloseAll()

endcase

'*******************************************************************************

testcase tValidModuleNames

    printlog( "Valid modulenames in BASIC Organizer: Names that are BASIC keywords" )

    dim iCurrentName as Integer
    dim sModuleName as String

    Randomize
    iCurrentName = Int( 8 * RND ) + 1 ' Range from 1 to 8

    select case iCurrentName
        case 1  : sModuleName = "option"
        case 2  : sModuleName = "sub"
        case 3  : sModuleName = "function"
        case 4  : sModuleName = "end"
        case 5  : sModuleName = "exit"
        case 6  : sModuleName = "_underscore1"
        case 7  : sModuleName = "underscore_2"
        case 8  : sModuleName = "ThisIsQuiteALongNameForAModuleDontYouThink"
    end select

    hCreateDocument()
    ToolsMacro_uno

    if ( hCreateModuleForDoc( sModuleName ) ) then
        printlog( "The module name was accepted: " & sModuleName )
        hFileCloseAll()
    else
        warnlog( "The module name was not accepted: " & sModuleName )
        goto endsub
    endif
    
endcase

'*******************************************************************************

function hHandleInvalidNameWarning( cTabName as string ) as boolean

    const CFN = "hHandleInvalidNameWarning(): "
    const MAX_WAIT = 1

    kontext "Active"
    if ( Active.exists( MAX_WAIT ) ) then

        printlog( "Invalid name message displayed" & Active.getText() )
        hCloseDialog( Active, "ok" )
        hHandleInvalidNameWarning() = true
        
        kontext "Active"
        if ( Active.exists( MAX_WAIT ) ) then
            printlog( "Unexpected messagebox displayed: " & Active.getText() )
            hHandleInvalidNameWarning() = false
        endif

        kontext "BasicIDE"
        tabbar.typekeys( "<ESCAPE>" , true )

    else
        printlog( "Invalid name warning not displayed" )
        hHandleInvalidNameWarning() = false
    endif

end function

'*******************************************************************************

function hCreateInvalidModuleName( iName as integer ) as string


    dim sSeparator as string
    
    ' Note: We cannot test for "<" and ">" because these are the characters
    '       that identify keystrokes for the .typeKeys() method.

    select case iName
        case 0  : sSeparator = "_"
        case 1  : sSeparator = "-"
        case 2  : sSeparator = "."
        case 3  : sSeparator = ","
        case 4  : sSeparator = "+"
        case 5  : sSeparator = ":"
        case 6  : sSeparator = "!"
        case 7  : sSeparator = "$"
        case 8  : sSeparator = "("
        case 9  : sSeparator = ")"
        case 10 : sSeparator = "="
        case 11 : sSeparator = "?"
        case 12 : sSeparator = "*"
        case 13 : sSeparator = " "
        case 14 : sSeparator = "&"
        case 15 : sSeparator = "\"
        case 16 : sSeparator = "/"
        case 17 : sSeparator = "§"
        case 18 : sSeparator = """"
        case 19 : sSeparator = "'"
        case 20 : sSeparator = "@"
        case 21 : sSeparator = "["
        case 22 : sSeparator = "]"
        case 23 : sSeparator = "%"
        case 24 : sSeparator = CHR$( 387 )
        case else : sSeparator = ""
    end select

    hCreateInvalidModuleName() = "ttModule" & sSeparator & "X"
    
end function
