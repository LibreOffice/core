VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "StringDataManager"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: StringDataManager.cls,v $
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
Private langDict As Scripting.Dictionary
Private mFileName As String

Const C_PRODUCTNAME = "<PRODUCTNAME>"
Const C_PRODUCTVERSION = "<PRODUCTVERSION>"
Const C_NEXTPRODUCTVERSION = "<NEXTPRODUCTVERSION>"
Const C_NEWLINE = "<CR>"

' Load strings from the data file (in the form "id=string") into
' dictionary object.
Function InitStringData(fileName As String) As Boolean
    On Error GoTo HandleErrors
    Dim stringFile As TextStream
    Dim aLine As String
    Dim valueOffset As Long
    Dim id, Str As String
    Dim fso As FileSystemObject
    
    'Make sure the string data file exists before opening.
    Set fso = New Scripting.FileSystemObject
    If Not fso.FileExists(fileName) Then
        InitStringData = False
        Exit Function
    End If
    Set stringFile = fso.OpenTextFile(fileName, ForReading, False, TristateTrue)
    If IsEmpty(stringFile) Then
        'WriteDebug
    End If
    mFileName = fileName

    'Read each line and parse the id and string, then put into dictionary
    Do While Not stringFile.AtEndOfStream
        aLine = stringFile.ReadLine
        valueOffset = InStr(aLine, "=")
        id = Left(aLine, valueOffset - 1)
        Str = Right(aLine, Len(aLine) - valueOffset)
        langDict.Add id, Str
    Loop
    stringFile.Close
    
    Dim aProductName As String
    Dim aProductVersion As String
    Dim aNextProductVersion As String
    Dim aKey As Variant
    Dim aItem As String
    Dim aOldItem As String
                    
    aProductName = langDict.item("RID_STR_COMMON_PRODUCTNAME")
    aProductVersion = langDict.item("RID_STR_COMMON_PRODUCTVERSION")
    aNextProductVersion = langDict.item("RID_STR_COMMON_NEXTPRODUCTVERSION")

    For Each aKey In langDict
        aOldItem = langDict.item(aKey)
        aItem = ReplaceTopicTokens(aOldItem, C_PRODUCTNAME, aProductName)
        aItem = ReplaceTopicTokens(aItem, C_PRODUCTVERSION, aProductVersion)
        aItem = ReplaceTopicTokens(aItem, C_NEXTPRODUCTVERSION, aNextProductVersion)
        aItem = ReplaceTopicTokens(aItem, C_NEWLINE, vbLF)
        If (Not (aOldItem = aItem)) Then
            langDict.item(aKey) = aItem
        End If
    Next
    
    InitStringData = True
    
FinalExit:
    Exit Function
HandleErrors:
    WriteDebug "InitStringData : " & Err.Number & " " & Err.Description & " " & Err.Source
    InitStringData = False
End Function

'Set String Data from an existing dictionary
Public Property Set StringData(data As Scripting.Dictionary)
    Set langDict = data
End Property

'Get String Data dictionary
Public Property Get StringData() As Scripting.Dictionary
    Set StringData = langDict
End Property

'Initialize a given string variable by id
Function InitString(ByRef resRef As String, resName As String)
    resRef = langDict.item(resName)
End Function

Private Sub Class_Initialize()
    Set langDict = New Scripting.Dictionary 'Allocate the string dictonary
End Sub

Private Sub Class_Terminate()
    langDict.RemoveAll
    Set langDict = Nothing 'Empty the dictionary and remove the instance
End Sub
