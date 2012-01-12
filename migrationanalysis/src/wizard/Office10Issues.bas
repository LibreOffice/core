Attribute VB_Name = "Office10Issues"
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

'Disable Option Explicit so this will compile on earlier Office versions
'Option Explicit
Public Declare Function RegCloseKey Lib "advapi32.dll" _
    (ByVal hKey As Long) As Long
Public Declare Function RegQueryValueEx Lib "advapi32.dll" _
    Alias "RegQueryValueExA" (ByVal hKey As Long, ByVal lpValueName As String, _
    ByVal lpReserved As Long, lpType As Long, lpData As Any, _
    lpcbData As Long) As Long
Public Declare Function RegSetValueEx Lib "advapi32.dll" _
    Alias "RegSetValueExA" (ByVal hKey As Long, ByVal lpValueName As String, _
    ByVal Reserved As Long, ByVal dwType As Long, lpData As Any, _
    ByVal cbData As Long) As Long
Public Declare Function RegCreateKeyEx Lib "advapi32.dll" Alias "RegCreateKeyExA" (ByVal _
    hKey As Long, ByVal lpSubKey As String, ByVal Reserved As Long, ByVal lpClass _
    As String, ByVal dwOptions As Long, ByVal samDesired As Long, lpSecurityAttributes _
    As SECURITY_ATTRIBUTES, phkResult As Long, lpdwDisposition As Long) As Long
Public Declare Function RegOpenKey Lib "advapi32.dll" _
    Alias "RegOpenKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, _
    phkResult As Long) As Long
Public Declare Function RegCreateKey Lib "advapi32.dll" _
    Alias "RegCreateKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, _
    phkResult As Long) As Long
Public Declare Function RegDeleteValue Lib "advapi32.dll" _
    Alias "RegDeleteValueA" (ByVal hKey As Long, _
    ByVal lpValueName As String) As Long
Public Declare Function RegOpenKeyEx Lib "advapi32.dll" Alias "RegOpenKeyExA" (ByVal _
    hKey As Long, ByVal lpSubKey As String, ByVal ulOptions As Long, ByVal samDesired _
    As Long, phkResult As Long) As Long

Type SECURITY_ATTRIBUTES
    nLength As Long
    lpSecurityDescriptor As Long
    bInheritHandle As Long
End Type

Enum RegHive
    'HKEY_CLASSES_ROOT = &H80000000
    HK_CR = &H80000000
    HKEY_CURRENT_USER = &H80000001
    HK_CU = &H80000001
    HKEY_LOCAL_MACHINE = &H80000002
    HK_LM = &H80000002
    HKEY_USERS = &H80000003
    HK_US = &H80000003
    HKEY_CURRENT_CONFIG = &H80000005
    HK_CC = &H80000005
    HKEY_DYN_DATA = &H80000006
    HK_DD = &H80000006
End Enum

Enum RegType
    REG_SZ = 1 'Unicode nul terminated string
    REG_BINARY = 3 'Free form binary
    REG_DWORD = 4 '32-bit number
End Enum

Const ERROR_SUCCESS = 0
Const KEY_WRITE = &H20006
Const APP_EXCEL = "Excel"
Const APP_WORD = "Word"
Const APP_PP = "PowerPoint"

Public Function CreateRegKey(hKey As RegHive, strPath As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CreateRegKey"
    
    Dim heKey As Long
    Dim secattr As SECURITY_ATTRIBUTES  ' security settings for the key
    Dim subkey As String        ' name of the subkey to create or open
    Dim neworused As Long       ' receives flag for if the key was created or opened
    Dim stringbuffer As String  ' the string to put into the registry
    Dim retval As Long          ' return value

    ' Set the name of the new key and the default security settings
    secattr.nLength = Len(secattr)
    secattr.lpSecurityDescriptor = 0
    secattr.bInheritHandle = 1
    
     retval = RegCreateKeyEx(hKey, strPath, 0, "", 0, KEY_WRITE, _
        secattr, heKey, neworused)
    If retval = 0 Then
        retval = RegCloseKey(hKey)
        Exit Function
    End If
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Public Function CreateRegKey2(hKey As RegHive, strPath As String) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CreateRegKey"
    CreateRegKey2 = 0
    
    Dim heKey As Long
    Dim secattr As SECURITY_ATTRIBUTES  ' security settings for the key
    Dim subkey As String        ' name of the subkey to create or open
    Dim neworused As Long       ' receives flag for if the key was created or opened
    Dim stringbuffer As String  ' the string to put into the registry
    Dim retval As Long          ' return value

    ' Set the name of the new key and the default security settings
    secattr.nLength = Len(secattr)
    secattr.lpSecurityDescriptor = 0
    secattr.bInheritHandle = 1
    
    retval = RegCreateKeyEx(hKey, strPath, 0, "", 0, KEY_WRITE, _
        secattr, heKey, neworused)
    If retval = ERROR_SUCCESS Then
        CreateRegKey2 = heKey
        Exit Function
    End If

FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    CreateRegKey2 = 0
    GoTo FinalExit
End Function


Public Function GetRegLong(ByVal hKey As RegHive, ByVal strPath As String, ByVal strValue As String) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetRegLong"
    
    Dim lRegResult As Long
    Dim lValueType As Long
    Dim lBuffer As Long
    Dim lDataBufferSize As Long
    Dim hCurKey As Long

    GetRegLong = 0
    lRegResult = RegOpenKey(hKey, strPath, hCurKey)
    lDataBufferSize = 4 '4 bytes = 32 bits = long
    
    lRegResult = RegQueryValueEx(hCurKey, strValue, 0, REG_DWORD, lBuffer, lDataBufferSize)
    If lRegResult = ERROR_SUCCESS Then
        GetRegLong = lBuffer
    End If
        lRegResult = RegCloseKey(hCurKey)
        Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Public Function SaveRegLong(ByVal hKey As RegHive, ByVal strPath As String, ByVal strValue As String, ByVal lData As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SaveRegLong"
    
    Const NumofByte = 4
    Dim hCurKey As Long
    Dim lRegResult As Long
    
    lRegResult = RegCreateKey(hKey, strPath, hCurKey)
    lRegResult = RegSetValueEx(hCurKey, strValue, 0&, REG_DWORD, lData, NumofByte)
    If lRegResult = ERROR_SUCCESS Then
        lRegResult = RegCloseKey(hCurKey)
        Exit Function
    End If
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
End Function


Public Function GiveAccessToMacroProject(application As String, sVersion As String, oldvalue As Long) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SaveRegLong"
    GiveAccessToMacroProject = False
    
    Const OfficePath = "Software\Policies\Microsoft\Office\"
    Const security = "\Security"
    Const AccessVBOM = "AccessVBOM"
    Const AccessVBOMValue = 1
    Dim subpath As String
    Dim RegistryValue As Long

    subpath = OfficePath & sVersion & "\" & application & security
    CreateRegKey HKEY_CURRENT_USER, subpath
    RegistryValue = GetRegLong(HKEY_CURRENT_USER, subpath, AccessVBOM)
    oldvalue = RegistryValue
    SaveRegLong HKEY_CURRENT_USER, subpath, AccessVBOM, AccessVBOMValue
    GiveAccessToMacroProject = True
    Exit Function
    
HandleErrors:
    GiveAccessToMacroProject = False
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Public Function SetDefaultRegValue(application As String, sVersion As String, sValue As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SaveRegLong"
    
    Const OfficePath = "Software\Policies\Microsoft\Office\"
    Const security = "\Security"
    Const AccessVBOM = "AccessVBOM"
    Dim subpath As String
    
    subpath = OfficePath & sVersion & "\" & application & security
    SaveRegLong HKEY_CURRENT_USER, subpath, AccessVBOM, sValue
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
End Function
Public Function DeleteRegValue(application As String, sVersion As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SaveRegLong"
    
    Const OfficePath = "Software\Policies\Microsoft\Office\"
    Const security = "\Security"
    Const AccessVBOM = "AccessVBOM"
    Dim subpath As String
    Dim retval As Long
    Dim hKey As Long
    
    subpath = OfficePath & sVersion & "\" & application & security
    retval = RegOpenKeyEx(HKEY_CURRENT_USER, subpath, 0, KEY_WRITE, hKey)
    If retval = ERROR_SUCCESS Then
        retval = RegDeleteValue(hKey, AccessVBOM)
        retval = RegCloseKey(hKey)
        Exit Function
    End If
 
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Public Function CheckForAccesToWordVBProject1(wrd As Word.application, RestoreValue As Long) As Boolean
    On Error Resume Next
    CheckForAccesToWordVBProject1 = True
    RestoreValue = -1
    If val(wrd.Version) < 10# Then Exit Function
    
    Set myProject = wrd.ActiveDocument.VBProject
    If Err.Number <> 0 Then
        Dim RegValue As Long
        If GiveAccessToMacroProject(APP_WORD, wrd.Version, RegValue) Then
            CheckForAccesToWordVBProject1 = True
            RestoreValue = RegValue
        Else
            CheckForAccesToWordVBProject1 = False
        End If
    End If
    
End Function
Public Function CheckForAccesToWordVBProject(wrd As Word.application) As Boolean
    On Error Resume Next
    CheckForAccesToWordVBProject = True
    If val(wrd.Version) < 10# Then Exit Function
    
    Set myProject = wrd.ActiveDocument.VBProject
    If Err.Number <> 0 Then
        CheckForAccesToWordVBProject = False
    End If
    
End Function
Public Function CheckForAccesToExcelVBProject1(xl As Excel.application, RestoreValue As Long) As Boolean
    On Error Resume Next
    CheckForAccesToExcelVBProject1 = True
    RestoreValue = -1
    If val(xl.Version) < 10# Then Exit Function
    
    Dim displayAlerts As Boolean
    displayAlerts = xl.displayAlerts
    xl.displayAlerts = False
    Set myProject = xl.ActiveWorkbook.VBProject
    If Err.Number <> 0 Then
        Dim RegValue As Long
        If GiveAccessToMacroProject(APP_EXCEL, xl.Version, RegValue) Then
            CheckForAccesToExcelVBProject1 = True
            RestoreValue = RegValue
        Else
            CheckForAccesToExcelVBProject1 = False
        End If
    End If
    xl.displayAlerts = displayAlerts

End Function
Public Function CheckForAccesToExcelVBProject(xl As Excel.application) As Boolean
    On Error Resume Next
    CheckForAccesToExcelVBProject = True
    If val(xl.Version) < 10# Then Exit Function
    
    Dim displayAlerts As Boolean
    displayAlerts = xl.displayAlerts
    xl.displayAlerts = False
    Set myProject = xl.ActiveWorkbook.VBProject
    If Err.Number <> 0 Then
        CheckForAccesToExcelVBProject = False
    End If
    xl.displayAlerts = displayAlerts

End Function
Public Function CheckForAccesToPPVBProject1(pp As PowerPoint.application, pres As PowerPoint.Presentation, RestoreValue As Long) As Boolean
    On Error Resume Next
    CheckForAccesToPPVBProject1 = True
    RestoreValue = -1
    If val(pp.Version) < 10# Then Exit Function

    Set myProject = pres.VBProject
    If Err.Number <> 0 Then
        Dim RegValue As Long
        If GiveAccessToMacroProject(APP_PP, pp.Version, RegValue) Then
            CheckForAccesToPPVBProject1 = True
            RestoreValue = RegValue
        Else
            CheckForAccesToPPVBProject1 = False
        End If
    End If
End Function

Public Function CheckForAccesToPPVBProject(pp As PowerPoint.application, pres As PowerPoint.Presentation) As Boolean
    On Error Resume Next
    CheckForAccesToPPVBProject = True
    If val(pp.Version) < 10# Then Exit Function

    Set myProject = pres.VBProject
    If Err.Number <> 0 Then
        CheckForAccesToPPVBProject = False
    End If
End Function
