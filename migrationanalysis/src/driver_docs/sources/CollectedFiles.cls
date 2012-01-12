VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "CollectedFiles"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
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

Private mDocCount As Long
Private mDotCount As Long
Private mXlsCount As Long
Private mXltCount As Long
Private mPptCount As Long
Private mPotCount As Long
Private mbDocSearch As Boolean
Private mbDotSearch  As Boolean
Private mbXlsSearch As Boolean
Private mbXltSearch As Boolean
Private mbPptSearch As Boolean
Private mbPotSearch As Boolean

Private mBannedList As Collection

Private Sub Class_Initialize()
    Set mWordFilesCol = New Collection
    Set mExcelFilesCol = New Collection
    Set mPPFilesCol = New Collection
    Set mBannedList = New Collection
End Sub
Private Sub Class_Terminate()
    Set mWordFilesCol = Nothing
    Set mExcelFilesCol = Nothing
    Set mPPFilesCol = Nothing
    Set mBannedList = Nothing
End Sub

Public Property Get BannedList() As Collection
    Set BannedList = mBannedList
End Property
Public Property Let BannedList(ByVal theList As Collection)
    Set mBannedList = theList
End Property

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


Public Function Search(rootDir As String, _
    FileSpecs As Collection, IncludeSubdirs As Boolean)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Search"
   
   Dim tstart As Single   'timer var for this routine only
   Dim tend As Single     'timer var for this routine only
   Dim spec As Variant
   Dim allSpecs As String
   Dim fso As New FileSystemObject
   
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
   
    With fp
       .sFileRoot = QualifyPath(rootDir)
       .sFileNameExt = allSpecs
       .bRecurse = IncludeSubdirs
       .nSearched = 0
    End With
    
    tstart = GetTickCount()
    Call SearchForFiles(fp.sFileRoot)
    tend = GetTickCount()
    
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
Function isBannedFile(thePath As String) As Boolean
    
    Dim aPath As Variant
    Dim theResult As Boolean
    theResult = False
    For Each aPath In mBannedList
        If aPath = thePath Then
            theResult = True
            GoTo FinalExit
        End If
    Next
    
FinalExit:
    isBannedFile = theResult
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

Private Sub SearchForFiles(sRoot As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SearchForFiles"
    
    Dim WFD As WIN32_FIND_DATA
    Dim hFile As Long
    Dim path As String
    Dim WordDriverPathTemp As String
    Dim ExcelDriverPathTemp As String
    Dim PPDriverPathTemp As String
    
    hFile = FindFirstFile(sRoot & ALL_FILES, WFD)
    
    If hFile = INVALID_HANDLE_VALUE Then GoTo FinalExit
   
    Do
        'if a folder, and recurse specified, call
        'method again
        If (WFD.dwFileAttributes And vbDirectory) Then
            If Asc(WFD.cFileName) <> vbDot Then
                If fp.bRecurse Then
                    SearchForFiles sRoot & TrimNull(WFD.cFileName) & vbBackslash
                End If
            End If
        Else
            'must be a file..
            If mbDocSearch Then
                 If MatchSpec(WFD.cFileName, "*.doc") Then
                    path = sRoot & TrimNull(WFD.cFileName)
                    'If StrComp(path, mWordDriverPath, vbTextCompare) <> 0 Then
                    If Not isBannedFile(path) Then
                       mDocCount = mDocCount + 1
                       mWordFilesCol.Add path
                       GoTo CONTINUE_LOOP
                    End If
                 End If
            End If
            If mbDotSearch Then
                If MatchSpec(WFD.cFileName, "*.dot") Then
                       mDotCount = mDotCount + 1
                       mWordFilesCol.Add sRoot & TrimNull(WFD.cFileName)
                        GoTo CONTINUE_LOOP
                End If
            End If
            If mbXlsSearch Then
                 If MatchSpec(WFD.cFileName, "*.xls") Then
                    path = sRoot & TrimNull(WFD.cFileName)
                    'If StrComp(TrimNull(WFD.cFileName), CEXCEL_DRIVER_FILE, vbTextCompare) <> 0 Then
                    If Not isBannedFile(path) Then
                        mXlsCount = mXlsCount + 1
                        mExcelFilesCol.Add sRoot & TrimNull(WFD.cFileName)
                       GoTo CONTINUE_LOOP
                    End If
                 End If
            End If
            If mbXltSearch Then
                 If MatchSpec(WFD.cFileName, "*.xlt") Then
                    mXltCount = mXltCount + 1
                    mExcelFilesCol.Add sRoot & TrimNull(WFD.cFileName)
                    GoTo CONTINUE_LOOP
                 End If
            End If
            If mbPptSearch Then
                 If MatchSpec(WFD.cFileName, "*.ppt") Then
                    path = sRoot & TrimNull(WFD.cFileName)
                    'If StrComp(path, mPPDriverPath, vbTextCompare) <> 0 Then
                    If Not isBannedFile(path) Then
                       mPptCount = mPptCount + 1
                       mPPFilesCol.Add path
                       GoTo CONTINUE_LOOP
                    End If
                 End If
            End If
            If mbPotSearch Then
                 If MatchSpec(WFD.cFileName, "*.pot") Then
                    mPotCount = mPotCount + 1
                    mPPFilesCol.Add sRoot & TrimNull(WFD.cFileName)
                    GoTo CONTINUE_LOOP
                 End If
            End If
        
        End If 'If WFD.dwFileAttributes
    
CONTINUE_LOOP:
        fp.nSearched = fp.nSearched + 1

    Loop While FindNextFile(hFile, WFD)
    
FinalExit:
    Call FindClose(hFile)
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub


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




