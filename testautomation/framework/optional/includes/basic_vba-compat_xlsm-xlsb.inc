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

*****

function hTestMacroType1( cMessage as string ) as boolean

    ' exact matching

    kontext "Active"
    if ( Active.exists( 2 ) ) then
        if ( Active.getText() = cMessage ) then
            printlog( "Correct macro has been executed" )
            hTestMacroType1() = true
        else
            warnlog( "Incorrect macro executed: " )
            printlog( "Expected: " & cMessage )
            printlog( "Found...: " & Active.getText() )
            hTestMacroType1() = false
        endif

        Active.ok()
        WaitSlot()

    else
        warnlog( "Messagebox missing, macro was not executed" )
        hTestMacroType1() = false
    endif

end function

'*******************************************************************************

function hTestMacroType2( cMessage as string ) as boolean

    ' fuzzy matching

    dim iCurrentMessage as integer

    for iCurrentMessage = 1 to 2

        kontext "Active"
        if ( Active.exists( 2 ) ) then
            if ( instr( Active.getText() , cMessage ) > 0 ) then
                printlog( "Correct macro has been executed" )
                hTestMacroType2() = true
            else
                warnlog( "Incorrect macro executed: " )
                printlog( "Expected: " & cMessage )
                printlog( "Found...: " & Active.getText() )
                hTestMacroType2() = false
            endif

            Active.ok()
            WaitSlot()

        else
            warnlog( "Messagebox missing, macro not executed" )
            hTestMacroType2() = false
        endif
    next iCurrentMessage

end function