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
'### Support Module for running macros in Word. Excel and Powerpoint
'### using automation

CONST CDA_TITLE = "Document Analysis Run Macro"
CONST CDA_ANALYSIS_INI = "analysis.ini"
Const CDA_ERR_STD_DELAY = 10
Const CDA_APPNAME_WORD = "Word"
Const CDA_APPNAME_EXCEL = "Excel"
Const CDA_APPNAME_POWERPOINT = "Powerpoint"

Dim daWrd 
Dim daDoc 
Dim daXl 
Dim daWb
Dim daPP
Dim daPres
Dim daWshShell 
Dim daFso
Dim daTitle

daTitle = CDA_TITLE

'# Setup Scripting objects
set daFso = WScript.CreateObject("Scripting.FileSystemObject")
set daWshShell = Wscript.CreateObject("Wscript.Shell")


'##### Run Macro FUNCTIONS ######

'###################### 
Sub DASetTitle(newTitle)
	daTitle = newTitle
End Sub

'###################### 
Sub DAsetupWrdServer
   
On Error Resume Next

Set daWrd = wscript.CreateObject("Word.Application")
If Err.Number <> 0 Then
	DAErrMsg "Failed to create Word Automation server: " & vbLf & vbLf & "Error: " _ 
		& CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
	FinalExit
End If

End Sub

'###################### 
Sub DAOpenWrdDriver(driver)
Dim sWordDriverDocPath
   
On Error Resume Next
daWrd.Visible = False

'# Open a driver doc
sWordDriverDocPath = daFso.GetAbsolutePathName(driver)
'DAdiagMsg "sWordDriverDocPath : " & sWordDriverDocPath  , CDIAG_STD_DELAY

If Not daFso.FileExists(sWordDriverDocPath) Then
	DAErrMsg "Driver doc does not exist: " & sWordDriverDocPath, CDA_ERR_STD_DELAY 
    	FinalExit
End If

Set daDoc = daWrd.Documents.Open(sWordDriverDocPath)
If Err.Number <> 0 Then
	DAErrMsg "Failed to open driver doc: " & vbLf & sWordDriverDocPath & vbLf & vbLf & "Error: " _ 
		& CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
	FinalExit
End If

End Sub

'###################### 
Function DArunWrdDriver(driver, macro)

On Error Resume Next
'# Run macro
DArunWrdDriver = True
daWrd.Run ("AnalysisTool." & macro)
If Err.Number <> 0 Then
	DAErrMsg "Failed to run macro: " & macro & vbLf & vbLf & "Error: " _
		& CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
	DArunWrdDriver = False
End If

End Function

'###################### 
Sub DAsaveWrdDriver(saveDriver)
'DAdiagMsg "saveDriver : " & saveDriver  , CDIAG_STD_DELAY
'DAdiagMsg "Abs(saveDriver) : " & daFso.GetAbsolutePathName( saveDriver)  , CDIAG_STD_DELAY
	daDoc.SaveAs daFso.GetAbsolutePathName( saveDriver)
End Sub

'###################### 
Sub DAsetupExcelServer
   
On Error Resume Next

Set daXl = wscript.CreateObject("Excel.Application")
If Err.Number <> 0 Then
	DAErrMsg "Failed to create Excel Automation server: " & vbLf & vbLf & "Error: " _ 
		& CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
	FinalExit
End If

End Sub

'###################### 
Sub DAOpenExcelDriver(driver)
    Dim sExcelDriverDocPath

    On Error Resume Next
    daXl.Visible = False

    '# Open driver doc
    sExcelDriverDocPath = daFso.GetAbsolutePathName(driver)
    If Not daFso.FileExists(sExcelDriverDocPath) Then
        DAErrMsg "Driver doc does not exist: " & sExcelDriverDocPath, CDA_ERR_STD_DELAY 
        FinalExit
    End If

    Set daWb = daXl.Workbooks.Open(sExcelDriverDocPath)
    If Err.Number <> 0 Then
        DAErrMsg "Failed to open driver doc: " & vbLf & sExcelDriverDocPath & vbLf & vbLf & "Error: " _ 
            & CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
        FinalExit
    End If

End Sub

'###################### 
Function DArunExcelDriver(driver, macro)
On Error Resume Next

'# Run macro
DArunExcelDriver = True
daXl.Run ("AnalysisTool." & macro)
If Err.Number <> 0 Then
	DAErrMsg "Failed to run macro: " & macro & vbLf & vbLf & "Error: " _
		& CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
	DArunExcelDriver = False
End If

End Function

'###################### 
Sub DAsaveExcelDriver(saveDriver)
	'# Not overwritting - Excel hangs, need to remove file first
	if daFso.FileExists(daFso.GetAbsolutePathName(saveDriver)) Then
		daFso.DeleteFile(daFso.GetAbsolutePathName(saveDriver))
	End If
	daWb.SaveAs daFso.GetAbsolutePathName(saveDriver)
End Sub

'###################### 
Sub DAsetupPPServer
   
On Error Resume Next

Set daPP = wscript.CreateObject("PowerPoint.Application")
If Err.Number <> 0 Then
	DAErrMsg "Failed to create PowerPoint Automation server: " & vbLf & vbLf & "Error: " _ 
		& CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
	FinalExit
End If

End Sub

'###################### 
Sub DAOpenPPDriver(driver)
Dim sPPDriverDocPath
  
On Error Resume Next


'# Open driver doc
sPPDriverDocPath = daFso.GetAbsolutePathName(driver)
If Not daFso.FileExists(sPPDriverDocPath ) Then
	DAErrMsg "Driver doc does not exist: " & sPPDriverDocPath, CDA_ERR_STD_DELAY 
    	FinalExit
End If


'## MS: KB Article 155073 ##
'# PPT7: OLE Automation Error Using Open Method
'# MUST show the PowerPoint application window at least once before calling the Application.Presentations.Open method 
daPP.Visible = True
daPP.WindowState = 2 'Minimize PowerPoint

daPP.Presentations.Open sPPDriverDocPath
If Err.Number <> 0 Then
	DAErrMsg "Failed to open driver doc: " & vbLf & sPPDriverDocPath & vbLf & vbLf & "Error: " _ 
		& CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
	FinalExit
End If

set daPres = daPP.Presentations(1)

End Sub

'###################### 
Function DArunPPDriver(driver, macro)

On Error Resume Next
'# Run macro
DArunPPDriver = True
daPP.Run (daFso.GetFileName(driver) & "!" & macro)
If Err.Number <> 0 Then
	DAErrMsg "Failed to run macro: " & macro & vbLf & vbLf & "Error: " _
		& CStr(Err.Number) & " " & Err.Description, CDA_ERR_STD_DELAY 
	DArunPPDriver = False
End If

End Function

'###################### 
Sub DAsavePPDriver(saveDriver)
	daPres.SaveAs daFso.GetAbsolutePathName(saveDriver)
End Sub


'###################### 

Sub DACloseApps()
    '# Quit apps
    On Error Resume Next
    If Not daWrd Is Nothing Then 
        daDoc.Close wdDoNotSaveChanges
        daWrd.Quit
    End If
    If Not daXl Is Nothing Then 
        daWb.Close False
        daXl.Quit
    End If
    If Not daPP Is Nothing Then
        daPres.Close 
        daPP.Quit
    End If	
    
    Set daDoc = Nothing
    Set daWb = Nothing
    Set daPres = Nothing
    
    Set daWrd = Nothing
    Set daXl = Nothing
    Set daPP = Nothing
End Sub

'###################### 

Sub DACleanUp()
    '# Quit apps
    On Error Resume Next
    
    DACloseApps
    
    Set daFso = Nothing
    Set daWshShell = Nothing
End Sub


'###################### 
Sub DAdiagMsg( msg, delay)
	'# WSHShell.echo: Popup if run with Wscript.exe, command line output if run with Cscript.exe
	WScript.Echo msg

	'WSHShell.popup msg, delay, daTitle, 64 	
End Sub

'###################### 
Sub DAErrMsg( msg, delay)
	daWshShell.Popup msg, delay, daTitle, 16

	'WScript.Echo msg
End Sub


'###################### 
Sub DAVerifyAnalysisIni()
	if daFso.FileExists(daFso.GetAbsolutePathName(".\" & CDA_ANALYSIS_INI)) Then Exit Sub

	DAErrMsg CDA_ANALYSIS_INI & " does not exist. " & vbLf & vbLf & _
		"You need to create it manually or use the DocAnalysisWizard to create one for you." & vbLf & _
		"Once this is done you can rerun the Document Analysis command line.", CDA_ERR_STD_DELAY 
    	FinalExit
End Sub

'######################
Sub DAExportFile(fileName, projectFile, app_name)
    On Error Resume Next

    Dim myProject

    '# Setup App Specifc VB Project
    Set myProject = DAgetProject(fileName, projectFile, app_name)
     
    Dim myComponent    
    Set myComponent = myProject.VBComponents(projectFile)
    If Err.Number <> 0 Then 
	DAErrMsg "Missing Project File [" & projectFile & "] - Path:" & vbLf & vbLf & fileName, CERR_STD_DELAY
    	Set myComponent = Nothing
	Set myProject = Nothing
	FinalExit
    End If

    myProject.VBComponents(projectFile).Export fileName
    If Err.Number <> 0 Then 
	DAErrMsg "Error exporting Project File [" & projectFile & "] - Path:" & vbLf & vbLf & fileName, CERR_STD_DELAY
    	Set myComponent = Nothing
	Set myProject = Nothing
	FinalExit
    End If

    Set myComponent = Nothing
    Set myProject = Nothing

End Sub

'######################
Sub DAImportFile(fileName, projectFile, app_name)
    On Error Resume Next

    Dim myProject

    '# Setup App Specifc VB Project
    Set myProject = DAgetProject(fileName, projectFile, app_name)

    '# Check if module already exists raise error
    Dim myComponent
    Set myComponent = myProject.VBComponents(projectFile)
    If Err.Number = 0 Then
        DAErrMsg "Duplicate Project File [" & projectFile & "] - Path:" & vbLf & vbLf & fileName, CERR_STD_DELAY
        Set myComponent = Nothing
        Set myProject = Nothing
        FinalExit
    End If

    '#If module not there need to clear out of index error
    Err.Clear 

    If Not daFso.FileExists(fileName) Then 
        DAErrMsg "Missing File " & fileName, CERR_STD_DELAY
        Set myComponent = Nothing
        Set myProject = Nothing
        FinalExit
    End If

    Call myProject.VBComponents.Import(fileName)

    If Err.Number <> 0 Then
        DAErrMsg "Error importing Project File [" & projectFile & "] - Path:" & vbLf & vbLf & fileName, CERR_STD_DELAY
    	Set myComponent = Nothing
        Set myProject = Nothing
        FinalExit
    End If

    Set myComponent = Nothing
    Set myProject = Nothing
End Sub

'#################

Sub DARemoveModule(fileName, projectFile, app_name)
     On Error Resume Next

    Dim myProject

    '# Setup App Specifc VB Project
    Set myProject = DAgetProject(fileName, projectFile, app_name)
    
    '# Check if module already exists raise error
    Dim myComponent
    Set myComponent = myProject.VBComponents(projectFile)


    myProject.VBComponents.Remove myComponent
    
    If Err.Number <> 0 Then 
	DAErrMsg "Error removing Project File [" & projectFile & "] - Path:" & vbLf & vbLf & fileName, CERR_STD_DELAY
    	Set myComponent = Nothing
	Set myProject = Nothing
	FinalExit
    End If

    Set myComponent = Nothing
    Set myProject = Nothing
End Sub

'######################
Function DAgetProject(fileName, projectFile, app_name)
    On Error Resume Next

    If app_name = CDA_APPNAME_WORD Then 
	Set DAgetProject = daWrd.ActiveDocument.VBProject 
   
    ElseIf app_name = CDA_APPNAME_EXCEL Then
	Set DAgetProject = daXl.ActiveWorkbook.VBProject 
   
    ElseIf app_name = CDA_APPNAME_POWERPOINT Then
	Set DAgetProject = daPP.ActivePresentation.VBProject    
    End If

    If Err.Number <> 0 Then 
	DAErrMsg "Cannot access VBProject for Project File [" & projectFile & "] - Path:" & vbLf & vbLf & fileName, _
		CERR_STD_DELAY
	Set DAgetProject = Nothing
	FinalExit
    End If

End Function

