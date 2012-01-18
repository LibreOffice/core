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
'* short description : Subroutines for the data validity test
'*
'************************************************************************
' **
' #0 check_message ' Check if the messages in validity test are OK
' **
'\***********************************************************************

sub check_message(MType as String, Celladdress as String, cellcontent as String, wrong as String)
'/// Sub routine for checking message 
    if MType <> "Macro" then
        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
           ViewToolbarsFormulaBar
        end if
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        Bereich.TypeKeys Celladdress
        sleep(1)
        Bereich.TypeKeys "<RETURN>"
    
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys cellcontent
        DocumentCalc.TypeKeys "<RETURN>"
        kontext "active"
        if active.exists(2) then
            warnlog "Value " & cellcontent & " is correct in cell "& Celladdress & ", so no " & MType & " message must not be appear"
            if MType = "Stop" then
                active.OK
            else
                active.Cancel
            end if
        else
            printlog "OK, no " & MType & " message appears, by inserting the correct value into " & Celladdress
        end if

        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
            ViewToolbarsFormulaBar
        end if
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        Bereich.TypeKeys Celladdress
        sleep(1)
        Bereich.TypeKeys "<RETURN>"
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys wrong
        DocumentCalc.TypeKeys "<RETURN>"
        kontext "active"
        if active.exists(2) then
            printlog "OK, value " & wrong & " is not correct in cell " & Celladdress & ", so the " & MType & " message has to be appear"
            if MType = "Stop" then
                active.OK
            else
                active.Cancel
            end if
        else
            warnlog "Value " & wrong & " is not correct for cell " & Celladdress & ", so the " & MType & " message has to be appears"
        end if
    else
        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
            ViewToolbarsFormulaBar
        end if
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        Bereich.TypeKeys Celladdress
        sleep(1)
        Bereich.TypeKeys "<RETURN>"
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys cellcontent
        DocumentCalc.TypeKeys "<RETURN>"
        DocumentCalc.TypeKeys "<UP><RIGHT>"
        FormatCells
        Kontext
        Active.setPage TabFontEffects
        Kontext "TabFontEffects"
        if FontColor.getSelIndex = 1 then
            printlog "OK, the macro detect the correct word in " & Celladdress
        else
            warnlog "The Fontcolor is " & FontColor.getSelText & " instead of automatic in cell " & Celladdress
        end if
        TabFontEffects.Cancel
           
        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
            ViewToolbarsFormulaBar
        end if
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        Bereich.TypeKeys Celladdress
        sleep(1)
        Bereich.TypeKeys "<RETURN>"
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys wrong
        DocumentCalc.TypeKeys "<RETURN>"
        DocumentCalc.TypeKeys "<UP><RIGHT>"
        FormatCells
        Kontext
        Active.setPage TabFontEffects
        Kontext "TabFontEffects"
        if FontColor.getSelIndex = 12 then
            printlog "OK, the macro detect the wrong word in " & Celladdress
        else
            warnlog "The Fontcolor is " & FontColor.getSelText & " instead of light green in cell " & Celladdress
        end if
        TabFontEffects.Cancel
    end if
end sub

