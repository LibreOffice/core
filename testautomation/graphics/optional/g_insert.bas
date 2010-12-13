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
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Graphics Function: Insert
'*
'\******************************************************************

public glLocale (15*20) as string
public ExtensionString as String

sub main
    PrintLog "------------------------- g_insert test -------------------------"
    Call hStatusIn ( "Graphics","g_insert.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\g_insert.inc"

    hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () )

    PrintLog "-------------------------" + gApplication + "-------------------"

    Call tInsertFloatingFrame
    Call tInsertFields
    Call tdInsertObjectOleObjects
    Call tdInsertSnappoint_Line
    Call tInsertSpecialCharacter
    Call tInsertSpreadsheet
    Call tInsertGraphics
    Call tInsertObjectPlugIn               ' Doesn't exist in the VCL
    Call tInsertObjectFormula              ' Is in the VCL in InsertObjectOLEObject
    Call tInsertChart
    Call tdInsertFile

    gApplication = "DRAW"
    PrintLog "-------------------------" + gApplication + "-------------------"

    Call tInsertFloatingFrame
    Call tInsertFields
    Call tdInsertObjectOleObjects

    Printlog "Disabled SnappPointLine test, because of unpredictable mouse moves (07/2002 TBO)"
    'Call tdInsertSnappoint_Line
    '"tInsertSpecialCharacter outcommented due to unpredictable results"
    'Call tInsertSpecialCharacter
    'Hyperlink
    '  Call tdInsertScannenQuelleAuswaehlen      'When no scanner is installed, no function
    '  Call tdInsertScannenAnfordern             'When no scanner is installed, no function
    Call tInsertSpreadsheet
    Call tInsertGraphics
    Call tInsertObjectPlugIn               ' Doesn't exist in the VCL
    Call tInsertObjectFormula              ' Is in the VCL in InsertObjectOLEObject
    '    Call tInsertChart
    Call tdInsertFile

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
