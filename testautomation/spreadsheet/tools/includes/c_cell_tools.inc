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
'* short description : tools for working with cells in calc
'*
'**************************************************************************************************
'*
' #1 fCalcGetCellValue            'Returns the value of a given cell
' #1 fCalcCompareCellValue        'Compares the value of a given cell with a given result
' #1 fCalcCompareCellFormular     'Compares the formular of a cell with a given result
'*
'\************************************************************************************************

function fCalcGetCellValue ( sCelladdress as string ) as string

    '///<b>The function returns the content of a given cell</b>
    '///+The cell is selected by fCalcSelectRange which is located in /spreadsheet/tools/includes/c_select_tools.inc
    '///+The content is copied to the clipboard by slot EditCopy
    '///+   and read out by GetClipboardText which is located in /global/tools/inc/t_tools1.inc
    use "spreadsheet\tools\includes\c_select_tools.inc"
    use "global\tools\includes\required\t_tools1.inc"
    
    call fCalcSelectRange (sCelladdress)   'Select the given cell
    kontext "DocumentCalc"                 'Setting kontext to Calcdocument
    EditCopy                               'Copy content to clipboard
    fCalcGetCellValue = GetClipboardText   'Returning clipboard to function    

end function

'-------------------------------------------------------------------------

function fCalcCompareCellValue ( sCelladdress as string, sresult as string ) as boolean

    '///<b>The function compares the value of a cell with a given result</b>
    '///+The cellvalue is read by fCalcGetCellValue which is located in /spreadsheet/tools/includes/c_cell_tools.inc
    '///+The value is compared with a given result (input)
    '///+The function returns true if the comparison is correct and false for incorrect    

    if fCalcGetCellValue ( sCelladdress ) = sresult then
        printlog "  The cellvalue is correct"
        fCalcCompareCellValue = true
    else
        warnlog "The cellvalue is " & fCalcGetCellValue ( sCelladdress ) & " but should be " & sresult
        fCalcCompareCellValue = false
    end if

end function

'-------------------------------------------------------------------------

function fCalcCompareCellFormular ( sCelladdress as string, sresult as string ) as boolean

    '///<b>The function compares the formular of a cell with a given result</b>
    '///+The value is compared with a given result (input)
    '///+The function returns true if the comparison is correct and false for incorrect
    
    dim sfunctionwithparameter as string
    
    call fCalcSelectRange (sCelladdress)
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    sfunctionwithparameter = GetClipboardText ()
    '/// Press twice <ESCAPE> to leave the cell
    'printlog "Press twice <ESCAPE> to leave the cell"
    kontext ( "DocumentCalc" )
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    if sfunctionwithparameter = sresult then
        printlog "  The function is correct"
        fCalcCompareCellFormular = true
    else
        warnlog "The function is " & sfunctionwithparameter & " instead of " &  sresult
        fCalcCompareCellFormular = false
    end if
    
end function
