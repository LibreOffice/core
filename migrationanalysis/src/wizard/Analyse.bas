Attribute VB_Name = "Analyse"
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
Option Explicit

Private Const C_STAT_NOT_STARTED    As Integer = 1
Private Const C_STAT_RETRY          As Integer = 2
Private Const C_STAT_ERROR          As Integer = 3
Private Const C_STAT_DONE           As Integer = 4
Private Const C_STAT_ABORTED        As Integer = 5

Private Const C_MAX_RETRIES         As Integer = 5
Private Const C_ABORT_TIMEOUT       As Integer = 30

Private Const MAX_WAIT_TIME     As Long = 600

Private Const C_STAT_FINISHED   As String = "finished"
Private Const C_STAT_ANALYSED   As String = "analysed="
Private Const C_STAT_ANALYSING  As String = "analysing="
Private Const CSINGLE_FILE      As String = "singlefile"
Private Const CFILE_LIST        As String = "filelist"
Private Const CSTAT_FILE        As String = "statfilename"
Private Const CLAST_CHECKPOINT  As String = "LastCheckpoint"
Private Const CNEXT_FILE        As String = "NextFile"
Private Const C_ABORT_ANALYSIS  As String = "AbortAnalysis"

Private Const CAPPNAME_WORD         As String = "word"
Private Const CAPPNAME_EXCEL        As String = "excel"
Private Const CAPPNAME_POWERPOINT   As String = "powerpoint"
Private Const C_EXENAME_WORD        As String = "winword.exe"
Private Const C_EXENAME_EXCEL       As String = "excel.exe"
Private Const C_EXENAME_POWERPOINT  As String = "powerpnt.exe"

Const CNEW_RESULTS_FILE = "newresultsfile"
Const C_LAUNCH_DRIVER = ".\resources\LaunchDrivers.exe"

'from http://support.microsoft.com/kb/q129796

Private Type STARTUPINFO
    cb As Long
    lpReserved As String
    lpDesktop As String
    lpTitle As String
    dwX As Long
    dwY As Long
    dwXSize As Long
    dwYSize As Long
    dwXCountChars As Long
    dwYCountChars As Long
    dwFillAttribute As Long
    dwFlags As Long
    wShowWindow As Integer
    cbReserved2 As Integer
    lpReserved2 As Long
    hStdInput As Long
    hStdOutput As Long
    hStdError As Long
End Type

Private Type PROCESS_INFORMATION
    hProcess As Long
    hThread As Long
    dwProcessID As Long
    dwThreadID As Long
End Type

Private Declare Function WaitForSingleObject Lib "kernel32" (ByVal _
    hHandle As Long, ByVal dwMilliseconds As Long) As Long

Private Declare Function CreateProcessA Lib "kernel32" (ByVal _
    lpApplicationName As String, ByVal lpCommandLine As String, ByVal _
    lpProcessAttributes As Long, ByVal lpThreadAttributes As Long, _
    ByVal bInheritHandles As Long, ByVal dwCreationFlags As Long, _
    ByVal lpEnvironment As Long, ByVal lpCurrentDirectory As String, _
    lpStartupInfo As STARTUPINFO, lpProcessInformation As _
    PROCESS_INFORMATION) As Long

Private Declare Function CloseHandle Lib "kernel32" _
    (ByVal hObject As Long) As Long

Private Declare Function GetExitCodeProcess Lib "kernel32" _
    (ByVal hProcess As Long, lpExitCode As Long) As Long

Private Declare Function TerminateProcess Lib "kernel32" (ByVal hProcess As Long, _
        ByVal uExitCode As Long) As Long

Private Const NORMAL_PRIORITY_CLASS = &H20&
Private Const WAIT_TIMEOUT  As Long = &H102
Private Const ABORTED       As Long = -2

' from http://vbnet.mvps.org/index.html?code/system/toolhelpprocesses.htm
Public Const TH32CS_SNAPPROCESS As Long = 2&
Public Const MAX_PATH As Long = 260

Public Type PROCESSENTRY32
    dwSize As Long
    cntUsage As Long
    th32ProcessID As Long
    th32DefaultHeapID As Long
    th32ModuleID As Long
    cntThreads As Long
    th32ParentProcessID As Long
    pcPriClassBase As Long
    dwFlags As Long
    szExeFile As String * MAX_PATH
End Type
    
Public Declare Function CreateToolhelp32Snapshot Lib "kernel32" _
   (ByVal lFlags As Long, ByVal lProcessID As Long) As Long

Public Declare Function ProcessFirst Lib "kernel32" _
    Alias "Process32First" _
   (ByVal hSnapShot As Long, uProcess As PROCESSENTRY32) As Long

Public Declare Function ProcessNext Lib "kernel32" _
    Alias "Process32Next" _
   (ByVal hSnapShot As Long, uProcess As PROCESSENTRY32) As Long


Public Function IsOfficeAppRunning(curApplication As String) As Boolean
'DV: we need some error handling here
    Dim hSnapShot As Long
    Dim uProcess As PROCESSENTRY32
    Dim success As Long
    Dim bRet As Boolean
    Dim bAppFound As Boolean
    Dim exeName As String
    Dim curExeName As String

    bRet = True
    On Error GoTo FinalExit

    curExeName = LCase$(curApplication)

    If (curExeName = CAPPNAME_WORD) Then
        exeName = C_EXENAME_WORD
    ElseIf (curExeName = CAPPNAME_EXCEL) Then
        exeName = C_EXENAME_EXCEL
    ElseIf (curExeName = CAPPNAME_POWERPOINT) Then
        exeName = C_EXENAME_POWERPOINT
    Else
        GoTo FinalExit
    End If

    hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0&)

    If hSnapShot = -1 Then GoTo FinalExit

    uProcess.dwSize = Len(uProcess)
    success = ProcessFirst(hSnapShot, uProcess)
    bAppFound = False

    While ((success = 1) And Not bAppFound)
        Dim i As Long
        i = InStr(1, uProcess.szExeFile, Chr(0))
        curExeName = LCase$(Left$(uProcess.szExeFile, i - 1))
        If (curExeName = exeName) Then
            bAppFound = True
        Else
            success = ProcessNext(hSnapShot, uProcess)
        End If
    Wend
    bRet = bAppFound
    
    Call CloseHandle(hSnapShot)

FinalExit:
    IsOfficeAppRunning = bRet

End Function

Private Sub CalculateProgress(statusFileName As String, fso As FileSystemObject, _
                              lastIndex As Long, docOffset As Long, _
                              myDocList As Collection)

    On Error GoTo FinalExit

    Dim curFile As String
    Dim fileCont As TextStream
    Dim myFile As file

    If (fso.FileExists(statusFileName)) Then
        Dim statLine As String

        Set fileCont = fso.OpenTextFile(statusFileName, ForReading, False, TristateTrue)
        statLine = fileCont.ReadLine

        If (Left(statLine, Len(C_STAT_ANALYSED)) = C_STAT_ANALYSED) Then
            curFile = Mid(statLine, Len(C_STAT_ANALYSED) + 1)
        ElseIf (Left(statLine, Len(C_STAT_ANALYSING)) = C_STAT_ANALYSING) Then
            curFile = Mid(statLine, Len(C_STAT_ANALYSING) + 1)
        End If
    End If
    
    ' when we don't have a file, we will show the name of the last used file in
    ' the progress window
    If (curFile = "") Then curFile = myDocList.item(lastIndex)

    If (GetDocumentIndex(curFile, myDocList, lastIndex)) Then
        Set myFile = fso.GetFile(curFile)
        Call ShowProgress.SP_UpdateProgress(myFile.Name, myFile.ParentFolder.path, lastIndex + docOffset)
    End If

FinalExit:
    If Not (fileCont Is Nothing) Then fileCont.Close
    Set fileCont = Nothing
    Set myFile = Nothing

End Sub

Function CheckAliveStatus(statFileName As String, _
                          curApplication As String, _
                          lastDate As Date, _
                          fso As FileSystemObject) As Boolean

    Dim isAlive As Boolean
    Dim currDate As Date
    Dim statFile As file
    Dim testing As Long

    isAlive = False

    If Not fso.FileExists(statFileName) Then
        currDate = Now()
        If (val(DateDiff("s", lastDate, currDate)) > MAX_WAIT_TIME) Then
            isAlive = False
        Else
            isAlive = True
        End If
    Else
        Set statFile = fso.GetFile(statFileName)
        currDate = statFile.DateLastModified
        If (currDate > lastDate) Then
            lastDate = currDate
            isAlive = True
        Else
            currDate = Now()
            If (lastDate >= currDate) Then   ' There might be some inaccuracies in file and system dates
                isAlive = True
            ElseIf (val(DateDiff("s", lastDate, currDate)) > MAX_WAIT_TIME) Then
                isAlive = False
            Else
                isAlive = IsOfficeAppRunning(curApplication)
            End If
        End If
    End If

    CheckAliveStatus = isAlive
End Function

Sub TerminateOfficeApps(fso As FileSystemObject, aParameter As String)

    Dim msoKillFileName As String

    msoKillFileName = fso.GetAbsolutePathName(".\resources\msokill.exe")
    If fso.FileExists(msoKillFileName) Then
        Shell msoKillFileName & aParameter
    Else
    End If
End Sub

Public Function launchDriver(statFileName As String, cmdLine As String, _
                             curApplication As String, fso As FileSystemObject, _
                             myDocList As Collection, myOffset As Long, _
                             myIniFilePath As String) As Long

    Dim proc As PROCESS_INFORMATION
    Dim start As STARTUPINFO
    Dim ret As Long
    Dim currDate As Date
    Dim lastIndex As Long

    currDate = Now()
    lastIndex = 1

    ' Initialize the STARTUPINFO structure:
    start.cb = Len(start)
            
    ' Start the shelled application:
    ret = CreateProcessA(vbNullString, cmdLine$, 0&, 0&, 1&, _
                         NORMAL_PRIORITY_CLASS, 0&, vbNullString, start, proc)

    ' Wait for the shelled application to finish:
    Do
        ret = WaitForSingleObject(proc.hProcess, 100)
        If ret <> WAIT_TIMEOUT Then
            Exit Do
        End If
        If Not CheckAliveStatus(statFileName, curApplication, currDate, fso) Then
            ' Try to close open office dialogs and then wait a little bit
            TerminateOfficeApps fso, " --close"
            ret = WaitForSingleObject(proc.hProcess, 1000)

            ' next try to kill all office programs and then wait a little bit
            TerminateOfficeApps fso, " --kill"
            ret = WaitForSingleObject(proc.hProcess, 1000)

            ret = TerminateProcess(proc.hProcess, "0")
            ret = WAIT_TIMEOUT
            Exit Do
        End If
        If (ShowProgress.g_SP_Abort) Then
            WriteToLog C_ABORT_ANALYSIS, True, myIniFilePath
            Call HandleAbort(proc.hProcess, curApplication)
            ret = ABORTED
            Exit Do
        End If
        Call CalculateProgress(statFileName, fso, lastIndex, myOffset, myDocList)
        DoEvents                                'allow other processes
    Loop While True
    
    If (ret <> WAIT_TIMEOUT) And (ret <> ABORTED) Then
        Call GetExitCodeProcess(proc.hProcess, ret&)
    End If
    Call CloseHandle(proc.hThread)
    Call CloseHandle(proc.hProcess)
    launchDriver = ret
End Function

Function CheckAnalyseStatus(statusFileName As String, _
                            lastFile As String, _
                            fso As FileSystemObject) As Integer

    Dim currStatus As Integer
    Dim fileCont As TextStream

    If Not fso.FileExists(statusFileName) Then
        currStatus = C_STAT_NOT_STARTED
    Else
        Dim statText As String
        Set fileCont = fso.OpenTextFile(statusFileName, ForReading, False, TristateTrue)
        statText = fileCont.ReadLine
        If (statText = C_STAT_FINISHED) Then
            currStatus = C_STAT_DONE
        ElseIf (Left(statText, Len(C_STAT_ANALYSED)) = C_STAT_ANALYSED) Then
            currStatus = C_STAT_RETRY
            lastFile = Mid(statText, Len(C_STAT_ANALYSED) + 1)
        ElseIf (Left(statText, Len(C_STAT_ANALYSING)) = C_STAT_ANALYSING) Then
            currStatus = C_STAT_RETRY
            lastFile = Mid(statText, Len(C_STAT_ANALYSING) + 1)
        Else
            currStatus = C_STAT_ERROR
        End If
        fileCont.Close
    End If

    CheckAnalyseStatus = currStatus
End Function

Function WriteDocsToAnalyze(myDocList As Collection, myApp As String, _
                            fso As FileSystemObject) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteDocsToAnalyze"

    Dim TempPath As String
    Dim fileName As String
    Dim fileContent As TextStream

    fileName = ""
    TempPath = fso.GetSpecialFolder(TemporaryFolder).path

    If (TempPath = "") Then
        TempPath = "."
    End If

    Dim vFileName As Variant
    Dim Index As Long
    Dim limit As Long
    
    limit = myDocList.count
    If (limit > 0) Then
        fileName = fso.GetAbsolutePathName(TempPath & "\FileList" & myApp & ".txt")
        Set fileContent = fso.OpenTextFile(fileName, ForWriting, True, TristateTrue)
    
        For Index = 1 To limit
            vFileName = myDocList(Index)
            fileContent.WriteLine (vFileName)
        Next
        
        fileContent.Close
    End If

FinalExit:
    Set fileContent = Nothing
    WriteDocsToAnalyze = fileName
    Exit Function

HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

' This function looks for the given document name in the document collection
' and returns TRUE and the position of the document in that collection if found,
' FALSE otherwise
Function GetDocumentIndex(myDocument As String, _
                          myDocList As Collection, _
                          lastIndex As Long) As Boolean

    Dim currentFunctionName As String
    currentFunctionName = "GetDocumentIndex"
    
    On Error GoTo HandleErrors
    
    Dim lastEntry As Long
    Dim curIndex As Long
    Dim curEntry As String
    Dim entryFound As Boolean
    
    entryFound = False
    lastEntry = myDocList.count
    curIndex = lastIndex

    ' We start the search at the position of the last found
    ' document
    While Not entryFound And curIndex <= lastEntry
        curEntry = myDocList.item(curIndex)
        If (curEntry = myDocument) Then
            lastIndex = curIndex
            entryFound = True
        Else
            curIndex = curIndex + 1
        End If
    Wend

    ' When we could not find the document, we start the search
    ' from the beginning of the list
    If Not entryFound Then
        curIndex = 1
        While Not entryFound And curIndex <= lastIndex
            curEntry = myDocList.item(curIndex)
            If (curEntry = myDocument) Then
                lastIndex = curIndex
                entryFound = True
            Else
                curIndex = curIndex + 1
            End If
        Wend
    End If

FinalExit:
    GetDocumentIndex = entryFound
    Exit Function
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function AnalyseList(myDocList As Collection, _
                     myApp As String, _
                     myIniFilePath As String, _
                     myOffset As Long, _
                     analysisAborted As Boolean) As Boolean

    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AnalyseList"

    Dim cmdLine As String
    Dim filelist As String
    Dim statFileName As String
    Dim finished As Boolean
    Dim analyseStatus As Integer
    Dim nRetries As Integer
    Dim lastFile As String
    Dim lastHandledFile As String
    Dim launchStatus As Long
    Dim fso As New FileSystemObject
    Dim progressTitle As String

    filelist = WriteDocsToAnalyze(myDocList, myApp, fso)
    cmdLine = fso.GetAbsolutePathName(C_LAUNCH_DRIVER) & " " & myApp
    finished = False

    Dim TempPath As String
    TempPath = fso.GetSpecialFolder(TemporaryFolder).path
    If (TempPath = "") Then TempPath = "."
    statFileName = fso.GetAbsolutePathName(TempPath & "\StatFile" & myApp & ".txt")
    If (fso.FileExists(statFileName)) Then fso.DeleteFile (statFileName)

    WriteToLog CFILE_LIST, filelist, myIniFilePath
    WriteToLog CSTAT_FILE, statFileName, myIniFilePath
    WriteToLog CLAST_CHECKPOINT, "", myIniFilePath
    WriteToLog CNEXT_FILE, "", myIniFilePath
    WriteToLog C_ABORT_ANALYSIS, "", myIniFilePath

    ' In this loop we will restart the driver until we have finished the analysis
    nRetries = 0
    While Not finished And nRetries < C_MAX_RETRIES
        launchStatus = launchDriver(statFileName, cmdLine, myApp, fso, _
                                    myDocList, myOffset, myIniFilePath)
        If (launchStatus = ABORTED) Then
            finished = True
            analyseStatus = C_STAT_ABORTED
            analysisAborted = True
        Else
            analyseStatus = CheckAnalyseStatus(statFileName, lastHandledFile, fso)
        End If
        If (analyseStatus = C_STAT_DONE) Then
            finished = True
        ElseIf (analyseStatus = C_STAT_RETRY) Then
            If (lastHandledFile = lastFile) Then
                nRetries = nRetries + 1
            Else
                lastFile = lastHandledFile
                nRetries = 1
            End If
        Else
            nRetries = nRetries + 1
        End If
    Wend
    
    If (analyseStatus = C_STAT_DONE) Then
        AnalyseList = True
    Else
        AnalyseList = False
    End If

    'The next driver should not overwrite this result file
    WriteToLog CNEW_RESULTS_FILE, "False", myIniFilePath

FinalExit:
    Set fso = Nothing
    Exit Function

HandleErrors:
    AnalyseList = False
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Sub HandleAbort(hProcess As Long, curApplication As String)

    On Error Resume Next

    Dim ret As Long
    Dim curDate As Date
    Dim stillWaiting As Boolean
    Dim killApplication As Boolean
    Dim waitTime As Long

    curDate = Now()
    stillWaiting = True
    killApplication = False

    While stillWaiting
        stillWaiting = IsOfficeAppRunning(curApplication)
        If (stillWaiting) Then
            waitTime = val(DateDiff("s", curDate, Now()))
            If (waitTime > C_ABORT_TIMEOUT) Then
                stillWaiting = False
                killApplication = True
            End If
        End If
    Wend

    If (killApplication) Then
        ShowProgress.g_SP_AllowOtherDLG = True
        TerminateMSO.Show vbModal, ShowProgress
    End If
    
    ret = TerminateProcess(hProcess, "0")
End Sub
