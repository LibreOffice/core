Attribute VB_Name = "BrowseDirectorysOnly"
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

' Modified as BIF_STATUSTEXT overflows for nested folders so is no longer
' shown.

'=====================================================================================
' Browse for a Folder using SHBrowseForFolder API function with a callback
' function BrowseCallbackProc.
'
' This Extends the functionality that was given in the
' MSDN Knowledge Base article Q179497 "HOWTO: Select a Directory
' Without the Common Dialog Control".
'
' After reading the MSDN knowledge base article Q179378 "HOWTO: Browse for
' Folders from the Current Directory", I was able to figure out how to add
' a callback function that sets the starting directory and displays the
' currently selected path in the "Browse For Folder" dialog.
'
'
' Stephen Fonnesbeck
' steev@xmission.com
' http://www.xmission.com/~steev
' Feb 20, 2000
'
'=====================================================================================
' Usage:
'
'    Dim folder As String
'    folder = BrowseForFolder(Me, "Select A Directory", "C:\startdir\anywhere")
'    If Len(folder) = 0 Then Exit Sub  'User Selected Cancel
'
'=====================================================================================

Option Explicit

Private Const BIF_STATUSTEXT = &H4&
Private Const BIF_RETURNONLYFSDIRS = 1
Private Const BIF_DONTGOBELOWDOMAIN = 2
Private Const MAX_PATH = 260

Private Const WM_USER = &H400
Private Const BFFM_INITIALIZED = 1
Private Const BFFM_SELCHANGED = 2
Private Const BFFM_SETSELECTION = (WM_USER + 102)

Private Declare Function SendMessage Lib "user32" Alias "SendMessageA" (ByVal hWnd As Long, ByVal wMsg As Long, ByVal wParam As Long, ByVal lParam As String) As Long
Private Declare Function SHBrowseForFolder Lib "shell32" (lpbi As BrowseInfo) As Long
Private Declare Function SHGetPathFromIDList Lib "shell32" (ByVal pidList As Long, ByVal lpBuffer As String) As Long
Private Declare Function lstrcat Lib "kernel32" Alias "lstrcatA" (ByVal lpString1 As String, ByVal lpString2 As String) As Long

Private Type BrowseInfo
  hWndOwner      As Long
  pIDLRoot       As Long
  pszDisplayName As Long
  lpszTitle      As Long
  ulFlags        As Long
  lpfnCallback   As Long
  lParam         As Long
  iImage         As Long
End Type

Private m_CurrentDirectory As String   'The current directory
'

Public Function BrowseForFolder(owner As Form, Title As String, StartDir As String) As String
  'Opens a Treeview control that displays the directories in a computer

  Dim lpIDList As Long
  Dim szTitle As String
  Dim sBuffer As String
  Dim tBrowseInfo As BrowseInfo
  m_CurrentDirectory = StartDir & vbNullChar

  szTitle = Title
  With tBrowseInfo
    .hWndOwner = owner.hWnd
    .lpszTitle = lstrcat(szTitle, "")
    .ulFlags = BIF_RETURNONLYFSDIRS + BIF_DONTGOBELOWDOMAIN '+ BIF_STATUSTEXT
    .lpfnCallback = GetAddressofFunction(AddressOf BrowseCallbackProc)  'get address of function.
  End With

  lpIDList = SHBrowseForFolder(tBrowseInfo)
  If (lpIDList) Then
    sBuffer = Space(MAX_PATH)
    SHGetPathFromIDList lpIDList, sBuffer
    sBuffer = Left(sBuffer, InStr(sBuffer, vbNullChar) - 1)
    BrowseForFolder = sBuffer
  Else
    BrowseForFolder = ""
  End If
  
End Function
 
Private Function BrowseCallbackProc(ByVal hWnd As Long, ByVal uMsg As Long, ByVal lp As Long, ByVal pData As Long) As Long
  
  Dim lpIDList As Long
  Dim ret As Long
  Dim sBuffer As String
  
  On Error Resume Next  'Sugested by MS to prevent an error from
                        'propagating back into the calling process.
     
  Select Case uMsg
  
    Case BFFM_INITIALIZED
      Call SendMessage(hWnd, BFFM_SETSELECTION, 1, m_CurrentDirectory)
            
  End Select
  
  BrowseCallbackProc = 0
  
End Function

' This function allows you to assign a function pointer to a vaiable.
Private Function GetAddressofFunction(add As Long) As Long
  GetAddressofFunction = add
End Function
