Attribute VB_Name = "RunServer"
'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: RunServer.bas,v $
' * $Revision: 1.2.66.2 $
' *
' * This file is part of OpenOffice.org.
' *
' * OpenOffice.org is free software: you can redistribute it and/or modify
' * it under the terms of the GNU Lesser General Public License version 3
' * only, as published by the Free Software Foundation.
' *
' * OpenOffice.org is distributed in the hope that it will be useful,
' * but WITHOUT ANY WARRANTY; without even the implied warranty of
' * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' * GNU Lesser General Public License version 3 for more details
' * (a copy is included in the LICENSE file that accompanied this code).
' *
' * You should have received a copy of the GNU Lesser General Public License
' * version 3 along with OpenOffice.org.  If not, see
' * <http://www.openoffice.org/license.html>
' * for a copy of the LGPLv3 License.
' *
' ************************************************************************/

Option Explicit

Private Declare Function WritePrivateProfileString Lib "kernel32" _
   Alias "WritePrivateProfileStringA" _
  (ByVal lpSectionName As String, _
   ByVal lpKeyName As Any, _
   ByVal lpString As Any, _
   ByVal lpFileName As String) As Long

Const CWORD_DRIVER = "_OOoDocAnalysisWordDriver.doc"
Const CEXCEL_DRIVER = "_OOoDocAnalysisExcelDriver.xls"
Const CPP_DRIVER = "_OOoDocAnalysisPPTDriver.ppt"

Const CWORD_APP = "word"
Const CEXCEL_APP = "excel"
Const CPP_APP = "pp"

Const CSTART_FILE = "PAW_Start_Analysis"
Const CSTOP_FILE = "PAW_Stop_Analysis"

Sub Main()

    Dim serverType As String
    serverType = LCase(Command$)
    If (serverType <> CWORD_APP) And (serverType <> CEXCEL_APP) And (serverType <> CPP_APP) Then
        MsgBox "Unknown server type: " & serverType
        GoTo FinalExit
    End If

    Dim fso As New FileSystemObject
    Dim driverName As String
    
    If (serverType = CWORD_APP) Then
        driverName = fso.GetAbsolutePathName(".\" & CWORD_DRIVER)
    ElseIf (serverType = CEXCEL_APP) Then
        driverName = fso.GetAbsolutePathName(".\" & CEXCEL_DRIVER)
    ElseIf (serverType = CPP_APP) Then
        driverName = fso.GetAbsolutePathName(".\" & CPP_DRIVER)
    End If

    If Not fso.FileExists(driverName) Then
        If (serverType = CWORD_APP) Then
            driverName = fso.GetAbsolutePathName(".\Resources\" & CWORD_DRIVER)
        ElseIf (serverType = CEXCEL_APP) Then
            driverName = fso.GetAbsolutePathName(".\Resources\" & CEXCEL_DRIVER)
        ElseIf (serverType = CPP_APP) Then
            driverName = fso.GetAbsolutePathName(".\Resources\" & CPP_DRIVER)
        End If
    End If

    If Not fso.FileExists(driverName) Then
        WriteToLog fso, "ALL", "LaunchDrivers: Could not find: " & driverName
        GoTo FinalExit
    End If

    If (serverType = CWORD_APP) Then
        OpenWordDriverDoc fso, driverName
    ElseIf (serverType = CEXCEL_APP) Then
        OpenExcelDriverDoc fso, driverName
    ElseIf (serverType = CPP_APP) Then
        OpenPPDriverDoc fso, driverName
    End If

FinalExit:

    Set fso = Nothing
End Sub

Sub OpenWordDriverDoc(fso As FileSystemObject, driverName As String)

    Dim wrdApp As Word.Application
    Dim wrdDriverDoc As Word.Document

    On Error GoTo HandleErrors

    Set wrdApp = New Word.Application
    Set wrdDriverDoc = wrdApp.Documents.Open(driverName)
    
    wrdApp.Run ("AnalysisTool.AnalysisDriver.AnalyseDirectory")
    If Err.Number <> 0 Then
        WriteToLog fso, CWORD_APP, "OpenWordDriverDoc: " & Err.Number & " " & Err.Description & " " & Err.Source
    End If
    
    wrdDriverDoc.Close wdDoNotSaveChanges
    wrdApp.Quit False

FinalExit:
    Set wrdDriverDoc = Nothing
    Set wrdApp = Nothing
    Exit Sub

HandleErrors:
    WriteToLog fso, CWORD_APP, "OpenWordDriverDoc: " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub OpenExcelDriverDoc(fso As FileSystemObject, driverName As String)

    Dim excelApp As Excel.Application
    Dim excelDriverDoc As Excel.Workbook

    On Error GoTo HandleErrors

    Set excelApp = New Excel.Application
    Set excelDriverDoc = Excel.Workbooks.Open(driverName)
    excelApp.Run ("AnalysisTool.AnalysisDriver.AnalyseDirectory")

    If Err.Number <> 0 Then
        WriteToLog fso, CEXCEL_APP, "OpenExcelDriverDoc: " & Err.Number & " " & Err.Description & " " & Err.Source
    End If

    excelDriverDoc.Close False
    excelApp.Quit

FinalExit:
    Set excelDriverDoc = Nothing
    Set excelApp = Nothing
    Exit Sub

HandleErrors:
    WriteToLog fso, CEXCEL_APP, "OpenExcelDriverDoc: " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub OpenPPDriverDoc(fso As FileSystemObject, driverName As String)

    Dim ppApp As PowerPoint.Application
    Dim ppDriverDoc As PowerPoint.Presentation
    Dim ppDummy(0) As Variant

    On Error GoTo HandleErrors

    Set ppApp = New PowerPoint.Application
    ppApp.Visible = msoTrue
    Set ppDriverDoc = ppApp.Presentations.Open(driverName) ', msoTrue, msoFalse, msoFalse)
    ppApp.Run ("AnalysisDriver.AnalyseDirectory")

    If Err.Number <> 0 Then
        WriteToLog fso, CPP_APP, "OpenPPDriverDoc: " & Err.Number & " " & Err.Description & " " & Err.Source
    End If

    ppDriverDoc.Close
    ppApp.Quit

FinalExit:
    Set ppDriverDoc = Nothing
    Set ppApp = Nothing
    Exit Sub

HandleErrors:
    WriteToLog fso, CPP_APP, "OpenPPDriverDoc: " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteToLog(fso As FileSystemObject, currApp As String, errMsg As String)

    On Error Resume Next

    Static ErrCount As Long
    Dim logFileName As String
    Dim tempPath As String

    tempPath = fso.GetSpecialFolder(TemporaryFolder).Path
    If (tempPath = "") Then tempPath = "."
    logFileName = fso.GetAbsolutePathName(tempPath & "\LauchDrivers.log")
    ErrCount = ErrCount + 1

    Call WritePrivateProfileString("ERRORS", currApp & "_log" & ErrCount, _
                                   errMsg, logFileName)
End Sub

