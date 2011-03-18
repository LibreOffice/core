'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : oliver.craemer@oracle.com
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
