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
'* short description : calc arrayconstants level2 test
'*
'************************************************************************
'*
' #1 tArrayconstants01     ' Direct input and OASIS format
' #1 tArrayconstants02     ' Import / Export to MSExcel
'*
'\***********************************************************************

testcase tArrayconstants01

    dim sFunctionTRUE as string
    dim sFunctionFALSE as string
    dim sFunctionSIN as string
    dim sError512 as string
    dim sError533 as string
    dim sdecimalseperator as string
    dim sLocalFile as string
    dim sCellcontent as string
    
    sFunctionTRUE = fFunctionName ("TRUE")
    sFunctionFALSE = fFunctionName ("FALSE")
    sFunctionSIN = fFunctionName ("SIN")
    sError512 = fError_l10n ("512")
    sError533 = fError_l10n ("533")
    sDecimalseperator = GetDecimalSeperator
    sLocalFile = convertpath ( gOfficepath & "user/work/arrayconstants.ods" )

    '///Open new spreadsheet document
    printlog "Open new spreadsheet document"
    call hNewDocument
    
    '///In cell A1 enter "={1;2;3}"
    printlog "In cell A1 enter ""={1;2;3}"""
    call fCalcSelectRange ("A1")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("={1;2;3} <RETURN>")
    '///Check that the cellvalue is "1"
    printlog "Check that the cellvalue is ""1"""
    call fCalcCompareCellValue ("A1","1")

    '///In cell A2 enter "={1;2;3|4;5;6}"
    printlog "In cell A2 enter ""={1;2;3|4;5;6}"""
    call fCalcSelectRange ("A2")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("={1;2;3|4;5;6} <RETURN>")
    '///Check that the cellvalue is "1"
    printlog "Check that the cellvalue is ""1"""
    call fCalcCompareCellValue ("A2","1")

    '///In cell A3 enter "={0;1;2|False;TRUE;"two"}"
    printlog "In cell A3 enter ""={0;1;2|" & sFunctionFALSE & ";" & sFunctionTRUE & ";""two""}"""
    call fCalcSelectRange ("A3")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("={0;1;2|" & sFunctionFALSE & ";" & sFunctionTRUE & ";""two""} <RETURN>")
    '///Check that the cellvalue is "0"
    printlog "Check that the cellvalue is ""0"""
    call fCalcCompareCellValue ("A3","0")

    '///Multiselect cells A4:C4, type "=sin({1;2;3})" and press <ctrl shift enter>
    printlog "Multiselect cells A4:C4, type ""=sin({1;2;3})"" and press <ctrl shift enter>"
    call fCalcSelectRange ("A4:C4")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("=" & sFunctionSIN & "({1;2;3}) <MOD1 SHIFT RETURN>")
    '///Check that the cellvalue  of A4 is "0.84"
    printlog "Check that the cellvalue  of A4 is ""0.8414709848"""
    call fCalcSelectRange ("A5")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys "=A4<TAB>=B4<TAB>=C4<RETURN>" 'because a part of a matrix is protected we need a helpcell with only the values
    call fCalcCompareCellValue ("A5","0" & sDecimalseperator & "8414709848")
    '///Check that the cell contents of B4 is "0.91"
    printlog "Check that the cell contents of B4 is ""0.9092974268"""
    call fCalcCompareCellValue ("B5","0" & sDecimalseperator & "9092974268")
    '///Check that the cell contents of C4 is "0.14"
    printlog "Check that the cell contents of C4 is ""0.1411200081"""
    call fCalcCompareCellValue ("C5","0" & sDecimalseperator & "1411200081")

    '///In cell A6 enter "={1;2|4;5;6}"
    printlog "In cell A6 enter ""={1;2|4;5;6}"""
    call fCalcSelectRange ("A6")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("={1;2|4;5;6} <RETURN>")
    '///Check that the cellvalue is "Err:512"
    printlog "Check that the cellvalue is ""Err:512"""
    call fCalcCompareCellValue ("A6",sError512)

    '///In cell A7 enter "={1;2+3}"
    printlog "In cell A7 enter ""={1;2+3}"""
    call fCalcSelectRange ("A7")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("={1;2+3} <RETURN>")
    '///Check that the cellvalue is "Err:512"
    printlog "Check that the cellvalue is ""Err:512"""
    call fCalcCompareCellValue ("A7",sError512)
    
    '///In cell A8 enter "={1;2;{3;4;5}}"
    printlog "In cell A8 enter ""={1;2;{3;4;5}}"""
    call fCalcSelectRange ("A8")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("={1;2;{3;4;5}} <RETURN>")
    kontext
    '///Cancel appearing MsgBox for correction of a wrong formular
    if active.exists then
        active.no
    end if
    '///Check that the cellvalue is "Err:533"
    printlog "Check that the cellvalue is ""Err:533"""
    call fCalcCompareCellValue ("A8",sError533)
    
    '///In cell A9 enter "={-1;2;3}"
    printlog "In cell A9 enter ""={-1;2;3}"""
    call fCalcSelectRange ("A9")
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("={-1;2;3} <RETURN>")
    '///Check that the cellvalue is "-1"
    printlog "Check that the cellvalue is ""-1"""
    call fCalcCompareCellValue ("A9",-1)

    '///Save document 
    printlog "Save document"
    call hFileSaveAsKill ( slocalfile )
    
    '/// Close document
    printlog "Close document"
    call hCloseDocument

    '/// Load Document
    printlog "Load Document"
    call hFileOpen ( slocalfile )
    sleep (2)
    
    '///Check that all formulars are the same as before.
    printlog "Check that all formulars are the same as before."
    
    '///+Check that the editline shows "={1;2;3}" for cell A1
    printlog "  Check that the editline shows ""={1;2;3}"" for cell A1"
    scellcontent = "={1;2;3}"
    call fCalcCompareCellFormular ("A1",scellcontent)
    
    '///+Check that the editline shows "={1;2;3|4;5;6}" for cell A2
    printlog "  Check that the editline shows ""={1;2;3|4;5;6}"" for cell A2"
    scellcontent = "={1;2;3|4;5;6}"
    call fCalcCompareCellFormular ("A2",scellcontent)
    
    '///+Check that the editline shows "={0;1;2|False;TRUE;"two"}" for cell A3
    printlog "  Check that the editline shows ""={0;1;2|" & sFunctionFALSE & ";" & sFunctionTRUE & ";""two""}"" for cell A3"
    scellcontent = "={0;1;2|" & sFunctionFALSE & ";" & sFunctionTRUE & ";""two""}"
    call fCalcCompareCellFormular ("A3",scellcontent)
    
    '///+Check that the editline shows "=SIN({1;2;3})" for cell A4:C4
    printlog "  Check that the editline shows ""=" & sFunctionSIN & "({1;2;3})"" for cell A4:C4"
    scellcontent = "=" & sFunctionSIN & "({1;2;3})"
    call fCalcCompareCellFormular ("A4:C4",scellcontent)
    
    '///+Check that the editline shows "={1;2|4;5;6}" for cell A6
    printlog "  Check that the editline shows ""={1;2|4;5;6}"" for cell A6"
    scellcontent = "={1;2|4;5;6}"
    call fCalcCompareCellFormular ("A6",scellcontent)
    
    '///+Check that the editline shows "={1;2+3}" for cell A7
    printlog "  Check that the editline shows ""={1;2+3}"" for cell A7"
    scellcontent = "={1;2+3}"
    call fCalcCompareCellFormular ("A7",scellcontent)
    
    '///+Check that the editline shows "={1;2;{3;4;5}}" for cell A8
    printlog "  Check that the editline shows ""={1;2;{3;4;5}}"" for cell A8"
    scellcontent = "={1;2;{3;4;5}}"
    call fCalcCompareCellFormular ("A8",scellcontent)
    
    '///+Check that the editline shows "={-1;2;3}" for cell A9
    printlog "  Check that the editline shows ""={-1;2;3}"" for cell A9"
    scellcontent = "={-1;2;3}"
    call fCalcCompareCellFormular ("A9",scellcontent)
    
    '///Close Document
    printlog "Close Document"
    call hCloseDocument
    hFileDelete( sLocalFile )

endcase
'
'--------------------------------------------------------------------
'
testcase tArrayconstants02

    Dim sImportFile as string
    Dim sFunctionSUM as string
    Dim sFunctionMDETERM as string
    Dim sCellcontent as string
    
    sImportFile = convertpath ( gTesttoolpath & "spreadsheet/optional/input/arrayconstants.xls" )
    sFunctionSUM = fFunctionName ("SUM")
    sFunctionMDETERM = fFunctionName ("MDETERM")
    
    '///Load xls testdocument <i>gTestToolPath</i>/spreadsheet/optional/input/arrayconstants.xls
    printlog "Load xls testdocument <i>gTestToolPath</i>/spreadsheet/optional/input/arrayconstants.xls"
    call hFileOpenLocally( sImportFile )
    
    '///Check that the cell contents of B4 is 1
    printlog "Check that the cell contents of B4 is 1"
    call fCalcCompareCellValue ("F4","1")
    '///Check that the cell contents of C5 is 4
    printlog "Check that the cell contents of C5 is 4"
    call fCalcCompareCellValue ("G5","4")
    '///Check that the editline shows "={1;2|3;4}" for cell B4
    printlog "Check that the editline shows ""={1;2|3;4}"" for cell B4"
    scellcontent = "={1;2|3;4}"
    call fCalcCompareCellFormular ("B4:C5",scellcontent)
    '///Check that the cell contents of B8 is 10
    printlog "Check that the cell contents of B8 is 10"
    call fCalcCompareCellValue ("B8","10")
    '///Check that the editline for cell B8 is "=SUM({1;2|3;4})"
    printlog "Check that the editline for cell B8 is ""=SUM({1;2|3;4})"""
    scellcontent = "=" & sFunctionSUM & "({1;2|3;4})"
    call fCalcCompareCellFormular ("B8",scellcontent)
    '///Check that the cell contents of B11 is -2
    printlog "Check that the cell contents of B11 is -2"
    call fCalcCompareCellValue ("B11","-2")
    '///Check that the editline for cell B11 is "=MDETERM({1;2|3;4})"
    printlog "Check that the editline for cell B11 is ""=MDETERM({1;2|3;4})"""
    scellcontent = "=" & sFunctionMDETERM & "({1;2|3;4})"
    call fCalcCompareCellFormular ("B11",scellcontent)
    '///Check that the cell contents of B14 is 1
    printlog "Check that the cell contents of B14 is 1"
    call fCalcCompareCellValue ("F14","1")
    '///Check that the editline for cell B14 is "{=name}"
    printlog "Check that the editline for cell B14 is ""=name"""
    scellcontent = "=name"
    call fCalcCompareCellFormular ("B14:C15",scellcontent)
    
    '/// Save document back to xls locally
    printlog "Save document back to xls locally"
    call hFileSaveAsKill ( gLastWorkFile )
    
    '/// Close document
    printlog "Close document"
    call hCloseDocument

    '/// Load Document
    printlog "Load Document"
    call hFileOpen ( gLastWorkFile )
    sleep (2)

    '///Check that all cellvalues are the same as before
    printlog "Check that all cellvalues are the same as before"
    '///+Check that the cell contents of B4 is 1
    printlog "Check that the cell contents of B4 is 1"
    call fCalcCompareCellValue ("F4","1")
    '///+Check that the cell contents of C5 is 4
    printlog "Check that the cell contents of C5 is 4"
    call fCalcCompareCellValue ("G5","4")
    '///+Check that the editline shows "={1;2|3;4}" for cell B4
    printlog "Check that the editline shows ""={1;2|3;4}"" for cell B4"
    scellcontent = "={1;2|3;4}"
    call fCalcCompareCellFormular ("B4:C5",scellcontent)
    '///+Check that the cell contents of B8 is 10
    printlog "Check that the cell contents of B8 is 10"
    call fCalcCompareCellValue ("B8","10")
    '///+Check that the editline for cell B8 is "=SUM({1;2|3;4})"
    printlog "Check that the editline for cell B8 is ""=SUM({1;2|3;4})"""
    scellcontent = "=" & sFunctionSUM & "({1;2|3;4})"
    call fCalcCompareCellFormular ("B8",scellcontent)
    '///+Check that the cell contents of B11 is -2
    printlog "Check that the cell contents of B11 is -2"
    call fCalcCompareCellValue ("B11","-2")
    '///+Check that the editline for cell B11 is "=MDETERM({1;2|3;4})"
    printlog "Check that the editline for cell B11 is ""=MDETERM({1;2|3;4})"""
    scellcontent = "=" & sFunctionMDETERM & "({1;2|3;4})"
    call fCalcCompareCellFormular ("B11",scellcontent)
    '///+Check that the cell contents of B14 is 1
    printlog "Check that the cell contents of B14 is 1"
    call fCalcCompareCellValue ("F14","1")
    '///+Check that the editline for cell B14 is "{=name}"
    printlog "Check that the editline for cell B14 is ""=name"""
    scellcontent = "=name"
    call fCalcCompareCellFormular ("B14:C15",scellcontent)

    '///Close document
    printlog "Close document"
    call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase
