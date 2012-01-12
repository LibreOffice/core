VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CollectedFiles"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
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

Private Const vbDot = 46
Private Const MAX_PATH = 260
Private Const INVALID_HANDLE_VALUE = -1
Private Const vbBackslash = "\"
Private Const ALL_FILES = "*.*"

Private Type FILETIME
   dwLowDateTime As Long
   dwHighDateTime As Long
End Type

Private Type SYSTEMTIME
    wYear As Integer
    wMonth As Integer
    wDayOfWeek As Integer
    wDay As Integer
    wHour As Integer
    wMinute As Integer
    wSecond As Integer
    wMilliseconds As Integer
End Type

Private Type WIN32_FIND_DATA
   dwFileAttributes As Long
   ftCreationTime As FILETIME
   ftLastAccessTime As FILETIME
   ftLastWriteTime As FILETIME
   nFileSizeHigh As Long
   nFileSizeLow As Long
   dwReserved0 As Long
   dwReserved1 As Long
   cFileName As String * MAX_PATH
   cAlternate As String * 14
End Type

Private Type FILE_PARAMS
   bRecurse As Boolean
   nSearched As Long
   sFileNameExt As String
   sFileRoot As String
End Type

Private Declare Function SystemTimeToFileTime Lib "kernel32" _
  (lpSystemTime As SYSTEMTIME, _
   lpFileTime As FILETIME) As Long

Private Declare Function CompareFileTime Lib "kernel32" _
  (lpFileTime1 As FILETIME, _
   lpFileTime2 As FILETIME) As Long

Private Declare Function FindClose Lib "kernel32" _
  (ByVal hFindFile As Long) As Long

Private Declare Function FindFirstFile Lib "kernel32" _
   Alias "FindFirstFileA" _
  (ByVal lpFileName As String, _
   lpFindFileData As WIN32_FIND_DATA) As Long

Private Declare Function FindNextFile Lib "kernel32" _
   Alias "FindNextFileA" _
  (ByVal hFindFile As Long, _
   lpFindFileData As WIN32_FIND_DATA) As Long

Private Declare Function GetTickCount Lib "kernel32" () As Long

Private Declare Function lstrlen Lib "kernel32" _
    Alias "lstrlenW" (ByVal lpString As Long) As Long

Private Declare Function PathMatchSpec Lib "shlwapi" _
   Alias "PathMatchSpecW" _
  (ByVal pszFileParam As Long, _
   ByVal pszSpec As Long) As Long

Private fp As FILE_PARAMS  'holds search parameters

Private mWordFilesCol As Collection
Private mExcelFilesCol As Collection
Private mPPFilesCol As Collection

Private mLessThan3 As Long
Private mLessThan6 As Long
Private mLessThan12 As Long
Private mMoreThan12 As Long
Private m3Months As FILETIME
Private m6Months As FILETIME
Private m12Months As FILETIME

Private mDocCount As Long
Private mDotCount As Long
Private mXlsCount As Long
Private mXltCount As Long
Private mPptCount As Long
Private mPotCount As Long
Private mIgnoredDocs As Long
Private mbDocSearch As Boolean
Private mbDotSearch  As Boolean
Private mbXlsSearch As Boolean
Private mbXltSearch As Boolean
Private mbPptSearch As Boolean
Private mbPotSearch As Boolean

Private mWordDriverPath As String
Private mExcelDriverPath As String
Private mPPDriverPath As String

Private Sub Class_Initialize()
    Set mWordFilesCol = New Collection
    Set mExcelFilesCol = New Collection
    Set mPPFilesCol = New Collection
End Sub
Private Sub Class_Terminate()
    Set mWordFilesCol = Nothing
    Set mExcelFilesCol = Nothing
    Set mPPFilesCol = Nothing
End Sub

Public Property Get DocCount() As Long
    DocCount = mDocCount
End Property
Public Property Get DotCount() As Long
    DotCount = mDotCount
End Property
Public Property Get XlsCount() As Long
    XlsCount = mXlsCount
End Property
Public Property Get XltCount() As Long
    XltCount = mXltCount
End Property
Public Property Get PptCount() As Long
    PptCount = mPptCount
End Property
Public Property Get PotCount() As Long
    PotCount = mPotCount
End Property
Public Property Get IgnoredDocCount() As Long
    IgnoredDocCount = mIgnoredDocs
End Property
Public Property Get DocsLessThan3Months() As Long
    DocsLessThan3Months = mLessThan3
End Property
Public Property Get DocsLessThan6Months() As Long
    DocsLessThan6Months = mLessThan6
End Property
Public Property Get DocsLessThan12Months() As Long
    DocsLessThan12Months = mLessThan12
End Property
Public Property Get DocsMoreThan12Months() As Long
    DocsMoreThan12Months = mMoreThan12
End Property

Public Property Get WordFiles() As Collection
    Set WordFiles = mWordFilesCol
End Property
Public Property Get ExcelFiles() As Collection
    Set ExcelFiles = mExcelFilesCol
End Property
Public Property Get PowerPointFiles() As Collection
    Set PowerPointFiles = mPPFilesCol
End Property

Public Function count() As Long
    count = mWordFilesCol.count + mExcelFilesCol.count + mPPFilesCol.count
End Function

Public Function Search(rootDir As String, FileSpecs As Collection, IncludeSubdirs As Boolean, _
                       ignoreOld As Boolean, Months As Integer) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Search"

    Dim tstart As Single   'timer var for this routine only
    Dim tend As Single     'timer var for this routine only
    Dim spec As Variant
    Dim allSpecs As String
    Dim fso As New FileSystemObject

    Search = True

    If FileSpecs.count = 0 Then Exit Function
    
    If FileSpecs.count > 1 Then
        For Each spec In FileSpecs
             allSpecs = allSpecs & "; " & spec
             SetSearchBoolean CStr(spec)
        Next
    Else
        allSpecs = FileSpecs(1)
        SetSearchBoolean CStr(FileSpecs(1))
    End If
        
    mWordDriverPath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CWORD_DRIVER_FILE)
    mExcelDriverPath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CEXCEL_DRIVER_FILE)
    mPPDriverPath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CPP_DRIVER_FILE)
   
    With fp
       .sFileRoot = QualifyPath(rootDir)
       .sFileNameExt = allSpecs
       .bRecurse = IncludeSubdirs
       .nSearched = 0
    End With
    
    Load SearchDocs

    ignoreOld = ignoreOld And InitFileTimes
    
    Dim limDate As FILETIME
    If ignoreOld Then
        If Months = 3 Then
            limDate = m3Months
        ElseIf Months = 6 Then
            limDate = m6Months
        ElseIf Months = 12 Then
            limDate = m12Months
        Else
            ignoreOld = False
        End If
    End If
    
    'tstart = GetTickCount()
    Search = SearchForFiles(QualifyPath(rootDir), IncludeSubdirs, ignoreOld, limDate)
    'tend = GetTickCount()
    
    Unload SearchDocs
    
    'Debug:
    'MsgBox "Specs " & allSpecs & vbLf & _
    '    Format$(fp.nSearched, "###,###,###,##0") & vbLf & _
    '     Format$(count, "###,###,###,##0") & vbLf & _
    '     FormatNumber((tend - tstart) / 1000, 2) & "  seconds"
    
FinalExit:
    Set fso = Nothing
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Sub SetSearchBoolean(spec As String)

    If spec = "*.doc" Then
        mbDocSearch = True
    End If
    If spec = "*.dot" Then
        mbDotSearch = True
    End If
    If spec = "*.xls" Then
        mbXlsSearch = True
    End If
    If spec = "*.xlt" Then
        mbXltSearch = True
    End If
    If spec = "*.ppt" Then
        mbPptSearch = True
    End If
    If spec = "*.pot" Then
        mbPotSearch = True
    End If
      
End Sub

Private Function SearchForFiles(sRoot As String, bRecurse As Boolean, _
                                bIgnoreOld As Boolean, limDate As FILETIME) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SearchForFiles"

    Dim WFD As WIN32_FIND_DATA
    Dim hFile As Long
    Dim path As String
    Dim sFileName As String
    Dim nTotal As Long

    SearchForFiles = False

    hFile = FindFirstFile(sRoot & ALL_FILES, WFD)

    If hFile = INVALID_HANDLE_VALUE Then GoTo FinalExit

    Do
        If (SearchDocs.g_SD_Abort) Then GoTo FinalExit
        sFileName = TrimNull(WFD.cFileName)
        'if a folder, and recurse specified, call
        'method again
        If (WFD.dwFileAttributes And vbDirectory) Then
            If (Asc(WFD.cFileName) <> vbDot) And bRecurse Then
                SearchForFiles sRoot & sFileName & vbBackslash, bRecurse, bIgnoreOld, limDate
            End If
        Else
            'must be a file..
            nTotal = mDocCount + mDotCount + mXlsCount + _
                     mXltCount + mPptCount + mPotCount
            SearchDocs.SD_UpdateProgress str$(nTotal), sRoot
            DoEvents

            If mbDocSearch Then
                 If MatchSpec(WFD.cFileName, "*.doc") Then
                    path = sRoot & sFileName
                    
                    'If StrComp(path, mWordDriverPath, vbTextCompare) <> 0 Then
                    If Not MatchSpec(path, mWordDriverPath) Then
                        If (IsTooOld(WFD, limDate, bIgnoreOld)) Then
                            mIgnoredDocs = mIgnoredDocs + 1
                        Else
                            mDocCount = mDocCount + 1
                            mWordFilesCol.add path
                        End If
                    End If
                    GoTo CONTINUE_LOOP
                 End If
            End If
            If mbDotSearch Then
                If MatchSpec(WFD.cFileName, "*.dot") Then
                    If (IsTooOld(WFD, limDate, bIgnoreOld)) Then
                        mIgnoredDocs = mIgnoredDocs + 1
                    Else
                         mDotCount = mDotCount + 1
                        mWordFilesCol.add sRoot & sFileName
                    End If
                    GoTo CONTINUE_LOOP
                End If
            End If
            If mbXlsSearch Then
                 If MatchSpec(WFD.cFileName, "*.xls") Then
                    'If StrComp(sFileName, CEXCEL_DRIVER_FILE, vbTextCompare) <> 0 Then
                    If Not MatchSpec(WFD.cFileName, CEXCEL_DRIVER_FILE) Then
                        If (IsTooOld(WFD, limDate, bIgnoreOld)) Then
                            mIgnoredDocs = mIgnoredDocs + 1
                        Else
                            mXlsCount = mXlsCount + 1
                            mExcelFilesCol.add sRoot & sFileName
                        End If
                    End If
                    GoTo CONTINUE_LOOP
                 End If
            End If
            If mbXltSearch Then
                 If MatchSpec(WFD.cFileName, "*.xlt") Then
                    If (IsTooOld(WFD, limDate, bIgnoreOld)) Then
                        mIgnoredDocs = mIgnoredDocs + 1
                    Else
                        mXltCount = mXltCount + 1
                        mExcelFilesCol.add sRoot & sFileName
                    End If
                    GoTo CONTINUE_LOOP
                 End If
            End If
            If mbPptSearch Then
                 If MatchSpec(WFD.cFileName, "*.ppt") Then
                    path = sRoot & sFileName
                    'If StrComp(path, mPPDriverPath, vbTextCompare) <> 0 Then
                    If Not MatchSpec(path, mPPDriverPath) Then
                        If (IsTooOld(WFD, limDate, bIgnoreOld)) Then
                            mIgnoredDocs = mIgnoredDocs + 1
                        Else
                            mPptCount = mPptCount + 1
                            mPPFilesCol.add path
                        End If
                    End If
                    GoTo CONTINUE_LOOP
                 End If
            End If
            If mbPotSearch Then
                 If MatchSpec(WFD.cFileName, "*.pot") Then
                    If (IsTooOld(WFD, limDate, bIgnoreOld)) Then
                        mIgnoredDocs = mIgnoredDocs + 1
                    Else
                        mPotCount = mPotCount + 1
                        mPPFilesCol.add sRoot & sFileName
                    End If
                    GoTo CONTINUE_LOOP
                 End If
            End If
        
        End If 'If WFD.dwFileAttributes

CONTINUE_LOOP:
        fp.nSearched = fp.nSearched + 1

    Loop While FindNextFile(hFile, WFD)

    SearchForFiles = True
FinalExit:
    Call FindClose(hFile)
    Exit Function

HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Private Function QualifyPath(sPath As String) As String

   If Right$(sPath, 1) <> vbBackslash Then
         QualifyPath = sPath & vbBackslash
   Else: QualifyPath = sPath
   End If
      
End Function

Private Function TrimNull(startstr As String) As String

   TrimNull = Left$(startstr, lstrlen(StrPtr(startstr)))
   
End Function

Private Function MatchSpec(sFile As String, sSpec As String) As Boolean

   MatchSpec = PathMatchSpec(StrPtr(sFile), StrPtr(sSpec))
   
End Function

Private Function IsTooOld(aWFD As WIN32_FIND_DATA, minDate As FILETIME, _
                          ignoreOld As Boolean) As Boolean

    IsTooOld = False
    
    Dim aFileTime As FILETIME
    
    If (aWFD.ftLastWriteTime.dwHighDateTime <> 0) Then
        aFileTime = aWFD.ftLastWriteTime
    ElseIf (aWFD.ftCreationTime.dwHighDateTime <> 0) Then
        aFileTime = aWFD.ftCreationTime
    Else
        ' No valid time found, don't ignore file
        mLessThan3 = mLessThan3 + 1
        Exit Function
    End If

    If (ignoreOld) Then
        If (CompareFileTime(aFileTime, minDate) < 0) Then
            IsTooOld = True
        End If
    End If
            
    If (CompareFileTime(aWFD.ftLastWriteTime, m12Months) < 0) Then
        mMoreThan12 = mMoreThan12 + 1
    ElseIf (CompareFileTime(aWFD.ftLastWriteTime, m6Months) < 0) Then
        mLessThan12 = mLessThan12 + 1
    ElseIf (CompareFileTime(aWFD.ftLastWriteTime, m3Months) < 0) Then
        mLessThan6 = mLessThan6 + 1
    Else
        mLessThan3 = mLessThan3 + 1
    End If

End Function

Private Function BasicDateToFileTime(basDate As Date, _
                                     fileDate As FILETIME) As Boolean
                                      
    Dim sysDate As SYSTEMTIME
    Dim retval As Long
    
    sysDate.wYear = DatePart("yyyy", basDate)
    sysDate.wMonth = DatePart("m", basDate)
    sysDate.wDay = DatePart("d", basDate)
    sysDate.wHour = DatePart("h", basDate)
    sysDate.wMinute = DatePart("m", basDate)
    retval = SystemTimeToFileTime(sysDate, fileDate)
    If (retval = 0) Then
        BasicDateToFileTime = False
    Else
        BasicDateToFileTime = True
    End If
End Function

Private Function InitFileTimes() As Boolean

    Dim nowDate As Date
    Dim basDate As Date

    InitFileTimes = True

    nowDate = Now()
    basDate = DateAdd("m", -3, nowDate)
    If Not BasicDateToFileTime(basDate, m3Months) Then InitFileTimes = False

    basDate = DateAdd("m", -6, nowDate)
    If Not BasicDateToFileTime(basDate, m6Months) Then InitFileTimes = False

    basDate = DateAdd("yyyy", -1, nowDate)
    If Not BasicDateToFileTime(basDate, m12Months) Then InitFileTimes = False

    mMoreThan12 = 0
    mLessThan12 = 0
    mLessThan6 = 0
    mLessThan3 = 0

End Function
