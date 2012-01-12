Attribute VB_Name = "Utilities"
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

Public Const LOCALE_ILANGUAGE             As Long = &H1    'language id
Public Const LOCALE_SLANGUAGE             As Long = &H2    'localized name of lang
Public Const LOCALE_SENGLANGUAGE          As Long = &H1001 'English name of lang
Public Const LOCALE_SABBREVLANGNAME       As Long = &H3    'abbreviated lang name
Public Const LOCALE_SNATIVELANGNAME       As Long = &H4    'native name of lang
Public Const LOCALE_ICOUNTRY              As Long = &H5    'country code
Public Const LOCALE_SCOUNTRY              As Long = &H6    'localized name of country
Public Const LOCALE_SENGCOUNTRY           As Long = &H1002 'English name of country
Public Const LOCALE_SABBREVCTRYNAME       As Long = &H7    'abbreviated country name
Public Const LOCALE_SNATIVECTRYNAME       As Long = &H8    'native name of country
Public Const LOCALE_SINTLSYMBOL           As Long = &H15   'intl monetary symbol
Public Const LOCALE_IDEFAULTLANGUAGE      As Long = &H9    'def language id
Public Const LOCALE_IDEFAULTCOUNTRY       As Long = &HA    'def country code
Public Const LOCALE_IDEFAULTCODEPAGE      As Long = &HB    'def oem code page
Public Const LOCALE_IDEFAULTANSICODEPAGE  As Long = &H1004 'def ansi code page
Public Const LOCALE_IDEFAULTMACCODEPAGE   As Long = &H1011 'def mac code page

Public Const LOCALE_IMEASURE              As Long = &HD     '0 = metric, 1 = US
Public Const LOCALE_SSHORTDATE            As Long = &H1F    'short date format string

'#if(WINVER >=  &H0400)
Public Const LOCALE_SISO639LANGNAME       As Long = &H59   'ISO abbreviated language name
Public Const LOCALE_SISO3166CTRYNAME      As Long = &H5A   'ISO abbreviated country name
'#endif /* WINVER >= as long = &H0400 */

'#if(WINVER >=  &H0500)
Public Const LOCALE_SNATIVECURRNAME        As Long = &H1008 'native name of currency
Public Const LOCALE_IDEFAULTEBCDICCODEPAGE As Long = &H1012 'default ebcdic code page
Public Const LOCALE_SSORTNAME              As Long = &H1013 'sort name
'#endif /* WINVER >=  &H0500 */

Public Const CSTR_LOG_FILE_NAME = "analysis.log"

Public Declare Function GetThreadLocale Lib "kernel32" () As Long

Public Declare Function GetSystemDefaultLCID Lib "kernel32" () As Long
Public Declare Function GetUserDefaultLCID Lib "kernel32" () As Long
Public Declare Function GetUserDefaultLangID Lib "kernel32" () As Long
Public Declare Function GetSystemDefaultLangID Lib "kernel32" () As Long

Public Declare Function GetLocaleInfo Lib "kernel32" _
   Alias "GetLocaleInfoA" _
  (ByVal Locale As Long, _
   ByVal LCType As Long, _
   ByVal lpLCData As String, _
   ByVal cchData As Long) As Long

Private Const VER_PLATFORM_WIN32s = 0
Private Const VER_PLATFORM_WIN32_WINDOWS = 1
Private Const VER_PLATFORM_WIN32_NT = 2

Private Type OSVERSIONINFO
  OSVSize         As Long         'size, in bytes, of this data structure
  dwVerMajor      As Long         'ie NT 3.51, dwVerMajor = 3; NT 4.0, dwVerMajor = 4.
  dwVerMinor      As Long         'ie NT 3.51, dwVerMinor = 51; NT 4.0, dwVerMinor= 0.
  dwBuildNumber   As Long         'NT: build number of the OS
                                  'Win9x: build number of the OS in low-order word.
                                  '       High-order word contains major & minor ver nos.
  PlatformID      As Long         'Identifies the operating system platform.
  szCSDVersion    As String * 128 'NT: string, such as "Service Pack 3"
                                  'Win9x: string providing arbitrary additional information
End Type

Public Type RGB_WINVER
  PlatformID      As Long
  VersionName     As String
  VersionNo       As String
  ServicePack     As String
  BuildNo         As String
End Type

'defined As Any to support OSVERSIONINFO and OSVERSIONINFOEX
Private Declare Function GetVersionEx Lib "kernel32" Alias "GetVersionExA" _
  (lpVersionInformation As Any) As Long

Private Declare Function GetDesktopWindow Lib "user32" () As Long

Private Declare Function ShellExecute Lib "shell32" _
    Alias "ShellExecuteA" _
   (ByVal hWnd As Long, _
    ByVal lpOperation As String, _
    ByVal lpFile As String, _
    ByVal lpParameters As String, _
    ByVal lpDirectory As String, _
    ByVal nShowCmd As Long) As Long
    
Public Const SW_SHOWNORMAL As Long = 1
Public Const SW_SHOWMAXIMIZED As Long = 3
Public Const SW_SHOWDEFAULT As Long = 10
Public Const SE_ERR_NOASSOC As Long = 31

Public Const CNO_OPTIONAL_PARAM = "_NoOptionalParam_"
Private Declare Function WritePrivateProfileString Lib "kernel32" _
   Alias "WritePrivateProfileStringA" _
  (ByVal lpSectionName As String, _
   ByVal lpKeyName As Any, _
   ByVal lpString As Any, _
   ByVal lpFileName As String) As Long


Public Const HKEY_LOCAL_MACHINE  As Long = &H80000002
Public Const HKEY_CLASSES_ROOT = &H80000000
Private Const ERROR_MORE_DATA = 234
Private Const ERROR_SUCCESS As Long = 0
Private Const KEY_QUERY_VALUE As Long = &H1
Private Const KEY_ENUMERATE_SUB_KEYS As Long = &H8
Private Const KEY_NOTIFY As Long = &H10
Private Const STANDARD_RIGHTS_READ As Long = &H20000
Private Const SYNCHRONIZE As Long = &H100000
Private Const KEY_READ As Long = ((STANDARD_RIGHTS_READ Or _
                                   KEY_QUERY_VALUE Or _
                                   KEY_ENUMERATE_SUB_KEYS Or _
                                   KEY_NOTIFY) And _
                                   (Not SYNCHRONIZE))
                                   
Private Declare Function RegOpenKeyEx Lib "advapi32.dll" _
   Alias "RegOpenKeyExA" _
  (ByVal hKey As Long, _
   ByVal lpSubKey As String, _
   ByVal ulOptions As Long, _
   ByVal samDesired As Long, _
   phkResult As Long) As Long

Private Declare Function RegQueryValueEx Lib "advapi32.dll" _
   Alias "RegQueryValueExA" _
  (ByVal hKey As Long, _
   ByVal lpValueName As String, _
   ByVal lpReserved As Long, _
   lpType As Long, _
   lpData As Any, _
   lpcbData As Long) As Long

Private Declare Function RegCloseKey Lib "advapi32.dll" _
  (ByVal hKey As Long) As Long
  
Private Declare Function lstrlenW Lib "kernel32" _
  (ByVal lpString As Long) As Long

Private Type ShortItemId
   cb As Long
   abID As Byte
End Type

Private Type ITEMIDLIST
   mkid As ShortItemId
End Type

Private Declare Function SHGetPathFromIDList Lib "shell32.dll" _
   (ByVal pidl As Long, ByVal pszPath As String) As Long

Private Declare Function SHGetSpecialFolderLocation Lib _
   "shell32.dll" (ByVal hWndOwner As Long, ByVal nFolder _
   As Long, pidl As ITEMIDLIST) As Long


Public Function IsWin98Plus() As Boolean
    'returns True if running Windows 2000 or later
    Dim osv As OSVERSIONINFO
    
    osv.OSVSize = Len(osv)
    
    If GetVersionEx(osv) = 1 Then
    
       Select Case osv.PlatformID 'win 32
            Case VER_PLATFORM_WIN32s:
                IsWin98Plus = False
                Exit Function
            Case VER_PLATFORM_WIN32_NT: 'win nt, 2000, xp
                IsWin98Plus = True
                Exit Function
            Case VER_PLATFORM_WIN32_WINDOWS:
                Select Case osv.dwVerMinor
                    Case 0: 'win95
                        IsWin98Plus = False
                        Exit Function
                    Case 90:   'Windows ME
                        IsWin98Plus = True
                        Exit Function
                    Case 10:   ' Windows 98
                        If osv.dwBuildNumber >= 2222 Then 'second edition
                            IsWin98Plus = True
                            Exit Function
                        Else
                            IsWin98Plus = False
                            Exit Function
                        End If
                End Select
            Case Else
                IsWin98Plus = False
                Exit Function
      End Select
    
    End If

End Function

Public Function GetWinVersion(WIN As RGB_WINVER) As String

'returns a structure (RGB_WINVER)
'filled with OS information

  #If Win32 Then
  
   Dim osv As OSVERSIONINFO
   Dim pos As Integer
   Dim sVer As String
   Dim sBuild As String
   
   osv.OSVSize = Len(osv)
   
   If GetVersionEx(osv) = 1 Then
   
     'PlatformId contains a value representing the OS
      WIN.PlatformID = osv.PlatformID
     
      Select Case osv.PlatformID
         Case VER_PLATFORM_WIN32s:   WIN.VersionName = "Win32s"
         Case VER_PLATFORM_WIN32_NT: WIN.VersionName = "Windows NT"
         
         Select Case osv.dwVerMajor
            Case 4:  WIN.VersionName = "Windows NT"
            Case 5:
            Select Case osv.dwVerMinor
               Case 0:  WIN.VersionName = "Windows 2000"
               Case 1:  WIN.VersionName = "Windows XP"
            End Select
        End Select
                  
         Case VER_PLATFORM_WIN32_WINDOWS:
         
          'The dwVerMinor bit tells if its 95 or 98.
            Select Case osv.dwVerMinor
               Case 0:    WIN.VersionName = "Windows 95"
               Case 90:   WIN.VersionName = "Windows ME"
               Case Else: WIN.VersionName = "Windows 98"
            End Select
         
      End Select
   
   
     'Get the version number
      WIN.VersionNo = osv.dwVerMajor & "." & osv.dwVerMinor
  
     'Get the build
      WIN.BuildNo = (osv.dwBuildNumber And &HFFFF&)
       
     'Any additional info. In Win9x, this can be
     '"any arbitrary string" provided by the
     'manufacturer. In NT, this is the service pack.
      pos = InStr(osv.szCSDVersion, Chr$(0))
      If pos Then
         WIN.ServicePack = Left$(osv.szCSDVersion, pos - 1)
      End If

   End If
   
  #Else
  
    'can only return that this does not
    'support the 32 bit call, so must be Win3x
     WIN.VersionName = "Windows 3.x"
  #End If
  GetWinVersion = WIN.VersionName
  
End Function

Public Sub RunShellExecute(sTopic As String, _
                           sFile As Variant, _
                           sParams As Variant, _
                           sDirectory As Variant, _
                           nShowCmd As Long)

   Dim hWndDesk As Long
   Dim success As Long
  
  'the desktop will be the
  'default for error messages
   hWndDesk = GetDesktopWindow()
  
  'execute the passed operation
   success = ShellExecute(hWndDesk, sTopic, sFile, sParams, sDirectory, nShowCmd)

  'This is optional. Uncomment the three lines
  'below to have the "Open With.." dialog appear
  'when the ShellExecute API call fails
  If success = SE_ERR_NOASSOC Then
     Call Shell("rundll32.exe shell32.dll,OpenAs_RunDLL " & sFile, vbNormalFocus)
  End If
   
End Sub

Public Sub WriteToLog(key As String, value As String, _
    Optional path As String = CNO_OPTIONAL_PARAM, _
    Optional section As String = WIZARD_NAME)

    Static logFile As String

    If logFile = "" Then
        logFile = GetLogFilePath
    End If

    If path = "" Then
        Exit Sub
    End If

    If path = CNO_OPTIONAL_PARAM Then
        path = logFile
    End If
    Call WritePrivateProfileString(section, key, value, path)
End Sub

Public Sub WriteDebug(value As String)
    Static ErrCount As Long
    Static logFile As String
    Static debugLevel As Long
    
    If logFile = "" Then
        logFile = GetLogFilePath
    End If
    
    Dim sSection As String
    sSection = WIZARD_NAME & "Debug"
        
    Call WritePrivateProfileString(sSection, "Analysis" & "_debug" & ErrCount, _
        value, logFile)
    ErrCount = ErrCount + 1
End Sub

Public Function GetDebug(section As String, key As String) As String
    Static logFile As String
    
    If logFile = "" Then
        logFile = GetLogFilePath
    End If
    
    GetDebug = ProfileGetItem(section, key, "", logFile)
End Function

Public Function GetUserLocaleInfo(ByVal dwLocaleID As Long, ByVal dwLCType As Long) As String

   Dim sReturn As String
   Dim r As Long

  'call the function passing the Locale type
  'variable to retrieve the required size of
  'the string buffer needed
   r = GetLocaleInfo(dwLocaleID, dwLCType, sReturn, Len(sReturn))
    
  'if successful..
   If r Then
    
     'pad the buffer with spaces
      sReturn = Space$(r)
       
     'and call again passing the buffer
      r = GetLocaleInfo(dwLocaleID, dwLCType, sReturn, Len(sReturn))
     
     'if successful (r > 0)
      If r Then
      
        'r holds the size of the string
        'including the terminating null
         GetUserLocaleInfo = Left$(sReturn, r - 1)
      
      End If
   
   End If
    
End Function

Public Function GetRegistryInfo(sHive As String, sSubKey As String, sKey As String) As String
    GetRegistryInfo = ""
    Dim hKey As Long
    
    hKey = OpenRegKey(sHive, sSubKey)
    
    If hKey <> 0 Then
       GetRegistryInfo = GetRegValue(hKey, sKey)
    
      'the opened key must be closed
       Call RegCloseKey(hKey)
    End If
End Function


Private Function GetRegValue(hSubKey As Long, sKeyName As String) As String

   Dim lpValue As String   'value retrieved
   Dim lpcbData As Long    'length of retrieved string

  'if valid
   If hSubKey <> 0 Then
   
     'Pass an zero-length string to
     'obtain the required buffer size
     'required to return the result.
     'If the key passed exists, the call
     'will return error 234 (more data)
     'and lpcbData will indicate the
     'required buffer size (including
     'the terminating null).
      lpValue = ""
      lpcbData = 0
      If RegQueryValueEx(hSubKey, _
                         sKeyName, _
                         0&, _
                         0&, _
                         ByVal lpValue, _
                         lpcbData) = ERROR_MORE_DATA Then

         lpValue = Space$(lpcbData)
      
        'retrieve the desired value
         If RegQueryValueEx(hSubKey, _
                            sKeyName, _
                            0&, _
                            0&, _
                            ByVal lpValue, _
                            lpcbData) = ERROR_SUCCESS Then
                        
            GetRegValue = TrimNull(lpValue)
         
         End If  'If RegQueryValueEx (second call)
      End If  'If RegQueryValueEx (first call)
   End If  'If hSubKey

End Function

Private Function OpenRegKey(ByVal hKey As Long, _
                            ByVal lpSubKey As String) As Long
    Dim hSubKey As Long
    Dim retval As Long

    retval = RegOpenKeyEx(hKey, lpSubKey, _
                          0, KEY_READ, hSubKey)

    If retval = ERROR_SUCCESS Then
        OpenRegKey = hSubKey
    End If
End Function


Private Function TrimNull(startstr As String) As String

   TrimNull = Left$(startstr, lstrlenW(StrPtr(startstr)))
   
End Function

Function GetLogFilePath() As String

    Dim fso As New FileSystemObject
    Dim TempPath As String
    
    TempPath = fso.GetSpecialFolder(TemporaryFolder).path
    
    If (TempPath = "") Then
        TempPath = "."
    End If

    GetLogFilePath = fso.GetAbsolutePathName(TempPath & "\" & CSTR_LOG_FILE_NAME)
End Function
    
Function GetIniFilePath() As String

    Dim fso As New FileSystemObject
    Dim AppDataDir As String
    
    AppDataDir = GetAppDataFolder
    If (AppDataDir = "") Then
        AppDataDir = CBASE_RESOURCE_DIR
    Else
        If Not fso.FolderExists(AppDataDir) Then
            fso.CreateFolder (AppDataDir)
        End If
        AppDataDir = AppDataDir & "\Sun"
        If Not fso.FolderExists(AppDataDir) Then
            fso.CreateFolder (AppDataDir)
        End If
        AppDataDir = AppDataDir & "\AnalysisWizard"
        If Not fso.FolderExists(AppDataDir) Then
            fso.CreateFolder (AppDataDir)
        End If
    End If

    GetIniFilePath = fso.GetAbsolutePathName(AppDataDir & "\" & CANALYSIS_INI_FILE)
End Function

' This function returns the Application Data Folder Path
Function GetAppDataFolder() As String
   Dim idlstr As Long
   Dim sPath As String
   Dim IDL As ITEMIDLIST
   Const NOERROR = 0
   Const MAX_LENGTH = 260
   Const CSIDL_APPDATA = &H1A

   On Error GoTo Err_GetFolder

   ' Fill the idl structure with the specified folder item.
   idlstr = SHGetSpecialFolderLocation(0, CSIDL_APPDATA, IDL)

   If idlstr = NOERROR Then
       ' Get the path from the idl list, and return
       ' the folder with a slash at the end.
       sPath = Space$(MAX_LENGTH)
       idlstr = SHGetPathFromIDList(ByVal IDL.mkid.cb, ByVal sPath)
       If idlstr Then
           GetAppDataFolder = Left$(sPath, InStr(sPath, Chr$(0)) - 1)
       End If
   End If

Exit_GetFolder:
    Exit Function

Err_GetFolder:
   MsgBox "An Error was Encountered" & Chr(13) & Err.Description, _
      vbCritical Or vbOKOnly
   Resume Exit_GetFolder

End Function



