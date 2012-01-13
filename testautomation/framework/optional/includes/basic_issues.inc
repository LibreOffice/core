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
'*  short description : Regression tests for especially annoying BASIC issues
'*
'\******************************************************************************

function hIDERunMacro()

    kontext "MacroBar"
    if ( MacroBar.exists() ) then
        BasicRun.click()
    else
        warnlog( "MacroBar not accessible" )
    endif
    
end function

'*******************************************************************************

function hTestActive( cString as string , iMethod as integer , bExact as boolean ) as integer

    ' Return values:
    ' -1 = Exact match expected, fuzzy match found
    ' -2 = No match 
    ' -3 = content of messagebox not relevant
    ' -4 = Messagebox not displayed / Macro not executed
    ' 1 - 4 = Number of buttons on the dialog

    ' internal states, if state = 0 we return the buttoncount instead
    const STATE_SUCCESS                =  0
    const STATE_FUZZY_MATCH            = -1
    const STATE_NO_MATCH               = -2
    const STATE_IGNORE_MSGBOX          = -3
    const STATE_MACRO_EXECUTION_FAILED = -4
    
    const CFN = "hTestActive()::"

    dim cMsg as string     ' The string printed in the message box
    dim rc as integer      ' Returncode for error handling and returnvalue
    dim buttons as integer ' Number of buttons on the message box

    kontext "Active"
    if ( Active.exists( 2 ) ) then
    
        cMsg = Active.getText()
    
        if ( cString <> "" ) then
            if ( instr( active.getText(), cString ) ) then
                if ( active.getText() = cString ) then
                    rc = STATE_SUCCESS
                else
                    if ( bExact ) then
                        warnlog( CFN & "Message is no exact match: " & cMsg )
                        rc = STATE_FUZZY_MATCH
                    else
                        rc = STATE_SUCCESS
                    endif
                endif
            else
                warnlog( CFN & "Messagebox has wrong content: " & cMsg )
                rc = STATE_NO_MATCH
            endif
        else
            rc = STATE_IGNORE_MSGBOX
        endif
        
        buttons = Active.getButtonCount()
        select case( iMethod )
        case 1 : Active.OK()
        case 2 : Active.Cancel()
        case 3 : Acitve.Yes()
        case 4 : Active.No()
        end select
    else
        warnlog( CFN & "Messagebox is missing" )
        rc = STATE_MACRO_EXECUTION_FAILED
    endif
    
    if ( rc = STATE_SUCCESS ) then hTestActive() = buttons
    
end function
       
'*******************************************************************************

testcase t_macros()

    const RESTYPE_1 = 1
    const RESTYPE_2 = 2
    const RESTYPE_3 = 3

    const CLOSE_METHOD_OK     = 1
    const CLOSE_METHOD_CANCEL = 2
    const CLOSE_METHOD_NO     = 4

    const REQUIRES_EXACT_MATCH = TRUE
    const REQUIRES_FUZZY_MATCH = FALSE

    gApplication = "WRITER"
    call hNewDocument()
    hInitBasicIDE( "TestIssues" )
    
    printlog( "" )
    printlog( " ---------- i41695 ------------ " )
    hInsertMacroFromFile( "i41695" )
    hIDERunMacro()
    hTestActive( "i41695-1", CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )
    hTestActive( "i41695-2", CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )

    printlog( "" )
    printlog( " ---------- i83978 ------------ " )
    hInsertMacroFromFile( "i83978" )
    hIDERunMacro()
    hTestActive( "NoSuchElementException", CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )

    printlog( "" )    
    printlog( " ---------- i82830 ------------ " )
    hInsertMacroFromFile( "i82830" )
    hIDERunMacro()
    hTestActive( "12D687" , CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )
    hTestActive( "4553207", CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )

    if ( gISOLang = "en-US" ) then
        printlog( "" )    
        printlog( " ---------- i81674 ------------ " )
        hInsertMacroFromFile( "i81674" )
        hIDERunMacro()
        hTestActive( "250" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
        hTestActive( "Yes" , CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH  )
        hTestActive( "True", CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH  )
        hTestActive( "On"  , CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH  )
    endif
    
    printlog( "" )
    printlog( " ---------- i80532 ------------ " )
    hInsertMacroFromFile( "i80532" )
    hIDERunMacro()
    hTestActive( "-10", CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
    hTestActive( "1"  , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
    hTestActive( "-10", CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
    
    printlog( "" )
    printlog( " ---------- i84040 ------------ " )
    hInsertMacroFromFile( "i84040" )
    hIDERunMacro()
    hTestActive( "False", CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )
    hTestActive( "False", CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )
    
    printlog( "" )
    printlog( " ---------- i86265 ------------ " )
    hInsertMacroFromFile( "i86265" )
    hIDERunMacro()
    hTestActive( "i86265-1", CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )
    hTestActive( "i86265-2", CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )
    
    printlog( "" )
    printlog( " ---------- i92329 ------------ " )
    hInsertMacroFromFile( "i92329" )
    hIDERunMacro()
    hTestActive( "Return an Array" , CLOSE_METHOD_OK )
    
    printlog( "" )
    printlog( " ---------- i97038 ------------ " )
    hInsertMacroFromFile( "i97038" )
    hIDERunMacro()
    hTestActive( "1900" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
    hTestActive( "2"    , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
  
    printlog( "" )
    printlog( " ---------- i103691 ------------ " )
    hInsertMacroFromFile( "i103691" )
    hIDERunMacro()
    hTestActive( "Equal" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )

    printlog( "" )
    printlog( " ---------- i103697 ------------ " )
    hInsertMacroFromFile( "i103697" )
    hIDERunMacro()
    hTestActive( "i103697" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )

    printlog( "" )
    printlog( " ---------- i103990 ------------ " )
    hInsertMacroFromFile( "i103990" )
    hIDERunMacro()
    hTestActive( "42" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
    hTestActive( "43" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
    hTestActive( "Pi" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )

    printlog( "" )
    printlog( " ---------- i107070 ------------ " )
    hInsertMacroFromFile( "i107070" )
    hIDERunMacro()
    hTestActive( "Dbg_Methods" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
    hTestActive( "Dbg_Methods" , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )
    hTestActive( "."           , CLOSE_METHOD_OK , REQUIRES_FUZZY_MATCH )

    printlog( "" )
    printlog( " ---------- i106744 ------------ " )
    hInsertMacroFromFile( "i106744-1" )
    hIDERunMacro()
    hTestActive( "NOT EMPTY" , CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )

    printlog( "" )
    printlog( " ---------- i106744 ------------ " )
    hInsertMacroFromFile( "i106744-2" )
    hIDERunMacro()
    hTestActive( "NOT EMPTY" , CLOSE_METHOD_OK , REQUIRES_EXACT_MATCH )

    printlog( "" )
    printlog( " ---------- MessageBoxes ------------ " )
    hInsertMacroFromFile( "MessageBoxes" )
    hIDERunMacro()

    if ( hTestActive( "0x" , CLOSE_METHOD_OK     , REQUIRES_EXACT_MATCH ) <> RESTYPE_1 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "1x" , CLOSE_METHOD_OK     , REQUIRES_EXACT_MATCH ) <> RESTYPE_2 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "2x" , CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "3x" , CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "4x" , CLOSE_METHOD_NO     , REQUIRES_EXACT_MATCH ) <> RESTYPE_2 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "5x" , CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_2 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "16" , CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "32" , CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "48" , CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "64" , CLOSE_METHOD_OK     , REQUIRES_EXACT_MATCH ) <> RESTYPE_1 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "128", CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "256", CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "512", CLOSE_METHOD_CANCEL , REQUIRES_EXACT_MATCH ) <> RESTYPE_3 ) then warnlog( "Wrong resource type" )
    
    printlog( "" )    
    hCloseBasicIDE()
    hCloseDocument()

endcase

