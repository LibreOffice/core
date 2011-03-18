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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : level 2 test for the getpivotdata function
'*
'************************************************************************
'*
' #1 tgetpivotdata_function_wizard     ' Testcase for manually input of the function
' #1 tgetpivotdata_xls_filter     ' Testcase for import/export of the function
'*
'\***********************************************************************

testcase tgetpivotdata_function_wizard

    dim stestdocument as string
    dim slocalfile as string
    dim sfunctionstring as string
    dim sfunctionname as string
    dim sfunctionwithparameter as string
    
    stestdocument = Convertpath (gTesttoolPath + "spreadsheet\optional\input\datapilot.ods" )
    slocalfile = Convertpath ( gOfficePath + "user\work\datapilot.ods" )
    setclipboard = ""
    
    '/// Get localised string for function GETPIVOTDATA by fFunctionname
    printlog "Get localised string for function GETPIVOTDATA by fFunctionname" 
    sfunctionname = fFunctionname("GETPIVOTDATA")
        
    '/// Load testdocument <i>gTestToolPath</i>/spreadsheet/optional/input/datapilot.ods
    printlog "Load testdocument datapilot.ods"
    call hFileOpenLocally( stestdocument )
        
    '/// Select cell I8 and open function wizard by INSERT FUNCTION
    printlog "Select cell I8 and open function wizard by INSERT FUNCTION"
    call fGotoCell ( "I8" )
    InsertFunction
    kontext ( "FunctionWizard" )
    '/// Select category SPREADSHEET (11th category in listbox)
    printlog "Select category SPREADSHEET (11th category in listbox)"
    CategoryLB.select 11
    '/// Select the function GETPIVOTDATA in the now filtered function list
    printlog "Select the function GETPIVOTDATA in the now filtered function list"
    try 
        FunctionLB.select sfunctionname
        printlog "  Function " & sfunctionname & " selected in category SPREADSHEET"
    catch
        warnlog "The function is not available under category SPREADSHEET"
    endcatch
    CategoryLB.select 2
    try
        FunctionLB.select sfunctionname
    catch
        warnlog "The function is generally not available"
        FunctionWizard.OK
        call hCloseDocument
        goto endsub
    endcatch
    '/// Press NEXT button
    printlog "Press NEXT button"
    Forward.click
    '/// Enter 2000 for parameter Data Field
    printlog "Enter 2000 for parameter Data Field"
    kontext "FunctionWizard"
    Editfield1.settext "2000"
    '/// Enter C29 for parameter DataPilot
    printlog "Enter C29 for parameter DataPilot"
    Editfield2.settext "C29"
    '/// Enter C3 for parameter Field Name / Item 1
    printlog "Enter C3 for parameter Field Name / Item 1"
    Editfield3.settext "C3"
    '/// Enter C16 for parameter Field Name / Item 2
    printlog "Enter C16 for parameter Field Name / Item 2"
    Editfield4.settext "C16"
    '/// Scroll down to enable the next field
    printlog "Scroll down to enable the next field"
    Editfield4.TypeKeys "<DOWN>"
    '/// Enter C26 for parameter Field Name / Item 3
    printlog "Enter C26 for parameter Field Name / Item 3"
    Editfield4.settext "C26"
    '/// Scroll down to enable the next field
    printlog "Scroll down to enable the next field"
    Editfield4.TypeKeys "<DOWN>"
    '/// Enter F27 for parameter Field Name / Item 4
    printlog "Enter F27 for parameter Field Name / Item 4"
    Editfield4.settext "F27"
    '/// Close functionwizard by pressing OK button
    printlog "Close functionwizard by pressing OK button"
    FunctionWizard.OK
    '/// Memorize cellcontent
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    sfunctionwithparameter = GetClipboardText ()
    '/// Press twice <ESCAPE> to leave the cell
    printlog "Press twice <ESCAPE> to leave the cell"
    kontext ( "DocumentCalc" )
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    '/// Verify that the function result is 100000
    printlog "Verify that the function result is 100000"
    call fCalcCompareCellValue ( "I8",100000 )
    '/// Save the document again
    printlog "Save the document again"
    if NOT hFileSaveAsKill (slocalfile) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    else
        printlog "  File saved successfully."
    end if    
    '/// Close the file with File/close
    printlog "Close the file with File/close"
    call hCloseDocument
    sleep(3)
    '/// Load localy saved document
    printlog "Load localy saved document"
    call hFileOpen ( slocalfile )
    '/// Verify that cell I8 still shows 100000
    printlog "Verify that cell I8 still shows 100000"
    call fCalcCompareCellValue ( "I8",100000 )
    '/// Verify that the function in cell I8 is still =GETPIVOTDATA("2000";C29;C3;C16;C26;F27)
    printlog "Verify that the function in cell I8 is still " & sfunctionwithparameter
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    if GetClipboardText () = sfunctionwithparameter then
        printlog "  The function was successfully saved and reloaded"
    else warnlog "The function is not " & sfunctionwithparameter & ", it is " & GetClipboardText ()
    end if
    '/// Press twice <ESCAPE> to leave the cell
    printlog "Press twice <ESCAPE> to leave the chart object"
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    '/// Close document
    printlog "Close document"
    call hCloseDocument
    sleep(3)
'///
endcase
'
'---------------------------------------------------------------------------
'
testcase tgetpivotdata_xls_filter

    dim stestdocument as string
    dim slocalfile as string
    dim sfunctionparameter as string
    dim sfunctionname as string
    dim sfunctionwithparameter as string
    dim sdecimalseperator as string
    
    stestdocument = Convertpath (gTesttoolPath + "spreadsheet\optional\input\getpivotdata2.xls" )
    'stestdocument = Convertpath ( gOfficePath + "user\work\test.ods" )
    slocalfile = Convertpath ( gOfficePath + "user\work\getpivotdata2.xls" )
    sdecimalseperator = GetDecimalSeperator
    setclipboard = ""
    sfunctionname = fFunctionname("GETPIVOTDATA")
    
    '/// Load testdocument <i>gTestToolPath</i>/spreadsheet/optional/input/getpivotdata2.xls
    printlog "Load testdocument getpivotdata2.xls"
    call hFileOpenLocally( stestdocument )
    '/// Verify that Cell G12 shows 2.5
    printlog "Verify that Cell G12 shows 2" & sdecimalseperator & "5"
    call fCalcCompareCellValue ("G12","2" & sdecimalseperator & "5")
    '/// Verify that the function is =GETPIVOTDATA("val";$F$5;"col1";"a";"col2";"x")
    printlog "Verify that the function is =GETPIVOTDATA(""val"";$F$5;""col1"";""a"";""col2"";""x"")"
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    sfunctionwithparameter = GetClipboardText ()
    '/// Press twice <ESCAPE> to leave the cell
    'printlog "Press twice <ESCAPE> to leave the cell"
    kontext ( "DocumentCalc" )
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    if sfunctionwithparameter = "=" & sfunctionname & "(""val"";$F$5;""col1"";""a"";""col2"";""x"")" then
        printlog "  The function is correct"
    else
        warnlog "The function is " & sfunctionwithparameter & " instead of =GETPIVOTDATA(""val"";$F$5;""col1"";""a"";""col2"";""x"")"
    end if

    '/// Verify that Cell G27 shows 0.05
    printlog "Verify that Cell G27 shows 0" & sdecimalseperator & "05"
    call fCalcCompareCellValue ("G27","0" & sdecimalseperator & "05")
    '/// Verify that the function is =GETPIVOTDATA("val";$F$19;"col1";"a";"col2";"y")
    printlog "Verify that the function is =GETPIVOTDATA(""val"";$F$19;""col1"";""a"";""col2"";""y"")"
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    sfunctionwithparameter = GetClipboardText ()
    '/// Press twice <ESCAPE> to leave the cell
    'printlog "Press twice <ESCAPE> to leave the cell"
    kontext ( "DocumentCalc" )
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    if sfunctionwithparameter = "=" & sfunctionname & "(""val"";$F$19;""col1"";""a"";""col2"";""y"")" then
        printlog "  The function is correct"
    else
        warnlog "The function is " & sfunctionwithparameter & " instead of =GETPIVOTDATA(""val"";$F$19;""col1"";""a"";""col2"";""y"")"
    end if
    
    '/// Verify that Cell G43 shows 3
    printlog "Verify that Cell G43 shows 3"
    call fCalcCompareCellValue ("G43","3")
    '/// Verify that the function is =GETPIVOTDATA("val";$F$33;"col1";"a";"col2";"z";"col3";"p"))
    printlog "Verify that the function is =GETPIVOTDATA(""val"";$F$33;""col1"";""a"";""col2"";""z"";""col3"";""p"")"
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    sfunctionwithparameter = GetClipboardText ()
    '/// Press twice <ESCAPE> to leave the cell
    'printlog "Press twice <ESCAPE> to leave the cell"
    kontext ( "DocumentCalc" )
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    if sfunctionwithparameter = "=" & sfunctionname & "(""val"";$F$33;""col1"";""a"";""col2"";""z"";""col3"";""p"")" then
        printlog "  The function is correct"
    else
        warnlog "The function is " & sfunctionwithparameter & " instead of =GETPIVOTDATA(""val"";$F$33;""col1"";""a"";""col2"";""z"";""col3"";""p"")"
    end if
    
    '/// Save document back to xls locally
    printlog "Save document back to xls locally"
    call hFileSaveAsKill ( slocalfile )
    
    '/// Close document
    printlog "Close document"
    call hCloseDocument

    '/// Load Document
    printlog "Load Document"
    call hFileOpen ( slocalfile )
    sleep (2)
    
'/// Verify that Cell G12 shows 2.5
    printlog "Verify that Cell G12 shows 2" & sdecimalseperator & "5"
    call fCalcCompareCellValue ("G12","2" & sdecimalseperator & "5")
    '/// Verify that the function is =GETPIVOTDATA("val";$F$5;"col1";"a";"col2";"x")
    printlog "Verify that the function is =GETPIVOTDATA(""val"";$F$5;""col1"";""a"";""col2"";""x"")"
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    sfunctionwithparameter = GetClipboardText ()
    '/// Press twice <ESCAPE> to leave the cell
    'printlog " Press twice <ESCAPE> to leave the cell"
    kontext ( "DocumentCalc" )
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    if sfunctionwithparameter = "=" & sfunctionname & "(""val"";$F$5;""col1"";""a"";""col2"";""x"")" then
        printlog "  The function is correct"
    else
        warnlog "The function is " & sfunctionwithparameter & " instead of =GETPIVOTDATA(""val"";$F$5;""col1"";""a"";""col2"";""x"")"
    end if

    '/// Verify that Cell G27 shows 0.05
    printlog "Verify that Cell G27 shows 0" & sdecimalseperator & "05"
    call fCalcCompareCellValue ("G27","0" & sdecimalseperator & "05")
    '/// Verify that the function is =GETPIVOTDATA("val";$F$19;"col1";"a";"col2";"y")
    printlog "Verify that the function is =GETPIVOTDATA(""val"";$F$19;""col1"";""a"";""col2"";""y"")"
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    sfunctionwithparameter = GetClipboardText ()
    '/// Press twice <ESCAPE> to leave the cell
    'printlog " Press twice <ESCAPE> to leave the cell"
    kontext ( "DocumentCalc" )
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    if sfunctionwithparameter = "=" & sfunctionname & "(""val"";$F$19;""col1"";""a"";""col2"";""y"")" then
        printlog "  The function is correct"
    else
        warnlog "The function is " & sfunctionwithparameter & " instead of =GETPIVOTDATA(""val"";$F$19;""col1"";""a"";""col2"";""y"")"
    end if
    
    '/// Verify that Cell G43 shows 3
    printlog "Verify that Cell G43 shows 3"
    call fCalcCompareCellValue ("G43","3")
    '/// Verify that the function is =GETPIVOTDATA("val";$F$33;"col1";"a";"col2";"z";"col3";"p"))
    printlog "Verify that the function is =GETPIVOTDATA(""val"";$F$33;""col1"";""a"";""col2"";""z"";""col3"";""p"")"
    kontext ( "RechenleisteCalc" ) 
    EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
    editcopy
    sfunctionwithparameter = GetClipboardText ()
    '/// Press twice <ESCAPE> to leave the cell
    'printlog " Press twice <ESCAPE> to leave the cell"
    kontext ( "DocumentCalc" )
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    if sfunctionwithparameter = "=" & sfunctionname & "(""val"";$F$33;""col1"";""a"";""col2"";""z"";""col3"";""p"")" then
        printlog "The function is correct"
    else
        warnlog "The function is " & sfunctionwithparameter & " instead of =GETPIVOTDATA(""val"";$F$33;""col1"";""a"";""col2"";""z"";""col3"";""p"")"
    end if
    
    '/// Close document
    printlog "Close document"
    call hCloseDocument
    sleep(3)
endcase
