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
'* short description : Global tools for spreadsheet
'*
'************************************************************************
'*
' #1 fGotoCell     ' Set active cell in a spreadhseet to a defined cell adress
' 
'*
'\***********************************************************************

function fGotoCell (sCelladdress as String) as integer
'/// The function sets the active cell in a spreadsheet to a defined cell address.
'/// Input: sCelladdress as String
'/// Output: <ul><li>0 = active cell has changed successfully</li>
'///+ <li>1 = active cell hasn't changed</li> 

    Dim sActualPlace as string

    const CFN = "qa:qatesttool:global:tools:inc:t_spreadsheet_tools1.inc:fGotoCell: "
    
    'function will return 1 if something goes wrong
    fGotoCell = 1
    Kontext "RechenleisteCalc"
    '/// If the spreadsheet <i>formula toolbar</i> isn't available make them
    '/// + available with View / Toolbars / Formula Bar 
    if NOT RechenleisteCalc.Exists(1) then    
        ViewToolbarsFormulaBar
    end if
    try
        Kontext "RechenleisteCalc"
        sActualPlace = AktiverZellbereich.GetSelText
        if UCase(sActualPlace) = UCase(sCelladdress) then
            fGotocell = 0            
            exit function
        else
            sActualPlace = ""
            '/// Type the cell address into the <i>name box</i>            
            AktiverZellbereich.SetText sCelladdress
            '///+ and press RETURN
            AktiverZellbereich.TypeKeys "<RETURN>"
            sleep(1)        
            '/// If the <i>name box<i>' address has been changed to the expected
            '///+ cell address the function was successfull.
            sActualPlace = AktiverZellbereich.GetSelText
            if UCase(sActualPlace) = UCase(sCelladdress) then
                fGotocell = 0
            else
                warnlog CFN & "The cell address has not been changed!"
            end if
        end if
    catch
        warnlog CFN & "Something unexpected happened! The cell address has not been changed!"
        fGotocell = 1
    endcatch
end function

