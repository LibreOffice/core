Attribute VB_Name = "modWizard"
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

Global Const WIZARD_NAME = "Analysis"

'Implementation details - not required for localisation
Public Const CWORD_DRIVER_FILE = "_OOoDocAnalysisWordDriver.doc"
Public Const CEXCEL_DRIVER_FILE = "_OOoDocAnalysisExcelDriver.xls"
Public Const CPP_DRIVER_FILE = "_OOoDocAnalysisPPTDriver.ppt"
Public Const CRESULTS_TEMPLATE_FILE = "results.xlt"
Public Const CISSUES_LIST_FILE = "issues.list"
Public Const CANALYSIS_INI_FILE = "analysis.ini"
Public Const CLAUNCH_DRIVERS_EXE = "LaunchDrivers.exe"
Public Const CMSO_KILL_EXE = "msokill.exe"
Public Const CRESOURCE_DLL = "Resources.dll"

' Preparation String ID's from DocAnalysisWizard.rc
Public Const RID_STR_ENG_TITLE_PREP_ID = 1030
Public Const RID_STR_ENG_SIDEBAR_ANALYZE_PREP_ID = 1074

Public Const RID_STR_ENG_INTRODUCTION_INTRO1_PREP_ID = 1131
Public Const RID_STR_ENG_INTRODUCTION_INTRO2_PREP_ID = 1132
Public Const RID_STR_ENG_INTRODUCTION_INTRO3_PREP_ID = 1134

Public Const RID_STR_ENG_DOCUMENTS_CHOOSE_DOCUMENTS_PREP_ID = 1230
Public Const RID_STR_ENG_DOCUMENTS_CHOOSE_DOC_TYPES_PREP_ID = 1236
Public Const RID_STR_ENG_DOCUMENTS_INCLUDE_SUBDIRECTORIES_PREP_ID = 1232

Public Const RID_STR_IGNORE_OLDER_CB_ID = 1231
Public Const RID_STR_IGNORE_OLDER_3_MONTHS_ID = 1233
Public Const RID_STR_IGNORE_OLDER_6_MONTHS_ID = 1234
Public Const RID_STR_IGNORE_OLDER_12_MONTHS_ID = 1235

Public Const RID_STR_ENG_RESULTS_CHOOSE_OPTIONS_PREP_ID = 1330
Public Const RID_STR_ENG_RESULTS_ANALYSIS_XLS_PREP_ID = 1332

Public Const RID_STR_ENG_ANALYZE_NUM_DOCS_PREP_ID = 1431
Public Const RID_STR_ENG_ANALYZE_SETUP_COMPLETE_PREP_ID = 1430
Public Const RID_STR_ENG_ANALYZE_IGNORED_DOCS_ID = 1435
Public Const RID_STR_ENG_ANALYZE_START_ID = 1413
Public Const RID_STR_ENG_ANALYZE_COMPLETED_ID = 1412
Public Const RID_STR_ENG_ANALYZE_VIEW_NOW_ID = 1414
Public Const RID_STR_ENG_ANALYZE_VIEW_LATER_ID = 1415
Public Const RID_STR_ENG_ANALYSE_NOT_RUN = 1416

Public Const RID_STR_ENG_OTHER_PLEASE_REFER_TO_README_PREP_ID = 1838
Public Const RID_STR_ENG_OTHER_XML_RESULTS_PREP_ID = 1845
Public Const RID_STR_ENG_OTHER_PREPARE_PROMPT_PREP_ID = 1846
Public Const RID_STR_ENG_OTHER_PREPARE_COMPLETED_PREP_ID = 1847

'Resource Strings Codes
' NOTE: to make a resource the default it must be the first string table inserted
' in the resource table - if it is not, just create several new string tables and
' copy what you want as default into the first new one you create, copy the others
' then delete the originals.
'
' To provide same string table for all English variants or all German variants
' I have added code to set LANG_BASE_ID dependent on current locale
' Refer to p.414 VBA in a Nutshell, Lomax
' I now have a single string table with each lang variant suitably offset:
' New locale - increase ofsets by 1000 - refer to DocAnalysisWizard.rc
'
'  English - eng - Start at 1000
'  German - ger - Start at 2000
'  BrazilianPortugese - por - Start at 4000
'  French - fre - Start at 5000
'  Italian - ita - Start at 6000
'  Spanish - spa - Start at 7000
'  Swedish - swe - Start at 8000


' String ID's must match those in DocAnalysisWizard.rc
Const LANG_BASE_ID = 1000
Const INTERNAL_RESOURCE_BASE_ID = LANG_BASE_ID + 800

' Setup Doc Preparation specific strings
#If PREPARATION Then
Global Const gBoolPreparation = True

Public Const TITLE_ID = RID_STR_ENG_TITLE_PREP_ID
Public Const CHK_SUBDIRS_ID = RID_STR_ENG_DOCUMENTS_INCLUDE_SUBDIRECTORIES_PREP_ID
Public Const SETUP_ANALYSIS_XLS_ID = RID_STR_ENG_RESULTS_ANALYSIS_XLS_PREP_ID
Public Const ANALYZE_TOTAL_NUM_DOCS_ID = RID_STR_ENG_ANALYZE_NUM_DOCS_PREP_ID
Public Const XML_RESULTS_ID = RID_STR_ENG_OTHER_XML_RESULTS_PREP_ID

#Else
Global Const gBoolPreparation = False

Public Const TITLE_ID = LANG_BASE_ID + 0
Public Const CHK_SUBDIRS_ID = LANG_BASE_ID + 202
Public Const SETUP_ANALYSIS_XLS_ID = LANG_BASE_ID + 302
Public Const ANALYZE_TOTAL_NUM_DOCS_ID = LANG_BASE_ID + 401
Public Const XML_RESULTS_ID = INTERNAL_RESOURCE_BASE_ID + 15
#End If

Public Const PRODUCTNAME_ID = LANG_BASE_ID + 1
Public Const LBL_STEPS_ID = LANG_BASE_ID + 40
Public Const INTRO1_ID = LANG_BASE_ID + 101

Public Const ANALYZE_DOCUMENTS_ID = LANG_BASE_ID + 402
Public Const ANALYZE_TEMPLATES_ID = LANG_BASE_ID + 403
Public Const ANALYZE_DOCUMENTS_XLS_ID = LANG_BASE_ID + 408
Public Const ANALYZE_DOCUMENTS_PPT_ID = LANG_BASE_ID + 409
Public Const RUNBTN_START_ID = LANG_BASE_ID + 404
Public Const PREPAREBTN_START_ID = LANG_BASE_ID + 411

Public Const README_FILE_ID = INTERNAL_RESOURCE_BASE_ID + 5 'Readme.doc
Public Const BROWSE_FOR_DOC_DIR_ID = INTERNAL_RESOURCE_BASE_ID + 6
Public Const BROWSE_FOR_RES_DIR_ID = INTERNAL_RESOURCE_BASE_ID + 7
Public Const RUNBTN_RUNNING_ID = INTERNAL_RESOURCE_BASE_ID + 10

Public Const PROGRESS_CAPTION = INTERNAL_RESOURCE_BASE_ID + 20
Public Const PROGRESS_ABORTING = INTERNAL_RESOURCE_BASE_ID + 21
Public Const PROGRESS_PATH_LABEL = INTERNAL_RESOURCE_BASE_ID + 22
Public Const PROGRESS_FILE_LABEL = INTERNAL_RESOURCE_BASE_ID + 23
Public Const PROGRESS_INFO_LABEL = INTERNAL_RESOURCE_BASE_ID + 24
Public Const PROGRESS_WAIT_LABEL = INTERNAL_RESOURCE_BASE_ID + 25

Public Const SEARCH_PATH_LABEL = PROGRESS_PATH_LABEL
Public Const SEARCH_CAPTION = INTERNAL_RESOURCE_BASE_ID + 26
Public Const SEARCH_INFO_LABEL = INTERNAL_RESOURCE_BASE_ID + 27
Public Const SEARCH_FOUND_LABEL = INTERNAL_RESOURCE_BASE_ID + 28

Public Const TERMINATE_CAPTION = INTERNAL_RESOURCE_BASE_ID + 30
Public Const TERMINATE_INFO = INTERNAL_RESOURCE_BASE_ID + 31
Public Const TERMINATE_YES = INTERNAL_RESOURCE_BASE_ID + 32
Public Const TERMINATE_NO = INTERNAL_RESOURCE_BASE_ID + 33

'Error Resource Strings Codes
Const ERROR_BASE_ID = LANG_BASE_ID + 900
Public Const ERR_MISSING_RESULTS_DOC = ERROR_BASE_ID + 0
Public Const ERR_NO_DOC_DIR = ERROR_BASE_ID + 1
Public Const ERR_NO_DOC_TYPES = ERROR_BASE_ID + 2
Public Const ERR_NO_RES_DIR = ERROR_BASE_ID + 3
Public Const ERR_CREATE_DIR = ERROR_BASE_ID + 4
Public Const ERR_MISSING_RESULTS_TEMPLATE = ERROR_BASE_ID + 5
Public Const ERR_MISSING_EXCEL_DRIVER = ERROR_BASE_ID + 6
Public Const ERR_EXCEL_DRIVER_CRASH = ERROR_BASE_ID + 7
Public Const ERR_MISSING_WORD_DRIVER = ERROR_BASE_ID + 8
Public Const ERR_WORD_DRIVER_CRASH = ERROR_BASE_ID + 9
Public Const ERR_MISSING_README = ERROR_BASE_ID + 10
Public Const ERR_MISSING_PP_DRIVER = ERROR_BASE_ID + 11
Public Const ERR_PP_DRIVER_CRASH = ERROR_BASE_ID + 12
Public Const ERR_SUPPORTED_VERSION = ERROR_BASE_ID + 13
Public Const ERR_ISSUES_VERSION_MISMATCH = ERROR_BASE_ID + 14
Public Const ERR_ISSUES_LIST_MISSING = ERROR_BASE_ID + 15
Public Const ERR_SUPPORTED_OSVERSION = ERROR_BASE_ID + 16
Public Const ERR_OPEN_RESULTS_SPREADSHEET = ERROR_BASE_ID + 17
Public Const ERR_EXCEL_OPEN = ERROR_BASE_ID + 18
Public Const ERR_NO_ACCESS_TO_VBPROJECT = ERROR_BASE_ID + 19
Public Const ERR_AUTOMATION_FAILURE = ERROR_BASE_ID + 20
Public Const ERR_NO_RESULTS_DIRECTORY = ERROR_BASE_ID + 21
Public Const ERR_CREATE_FILE = ERROR_BASE_ID + 22
Public Const ERR_XML_RESULTS_ONLY = ERROR_BASE_ID + 23
Public Const ERR_NOT_INSTALLED = ERROR_BASE_ID + 24
Public Const ERR_CDROM_NOT_ALLOWED = ERROR_BASE_ID + 25
Public Const ERR_CDROM_NOT_READY = ERROR_BASE_ID + 26
Public Const ERR_NO_WRITE_TO_READ_ONLY_FOLDER = ERROR_BASE_ID + 27
Public Const ERR_APPLICATION_IN_USE = ERROR_BASE_ID + 28
Public Const ERR_MISSING_IMPORTANT_FILE = ERROR_BASE_ID + 29


Private Const LOCALE_ILANGUAGE             As Long = &H1     'language id
Private Const LOCALE_SLANGUAGE             As Long = &H2     'localized name of language
Private Const LOCALE_SENGLANGUAGE          As Long = &H1001  'English name of language
Private Const LOCALE_SABBREVLANGNAME       As Long = &H3     'abbreviated language name
Private Const LOCALE_SCOUNTRY              As Long = &H6     'localized name of country
Private Const LOCALE_SENGCOUNTRY           As Long = &H1002  'English name of country
Private Const LOCALE_SABBREVCTRYNAME       As Long = &H7     'abbreviated country name
Private Const LOCALE_SISO639LANGNAME       As Long = &H59    'ISO abbreviated language name
Private Const LOCALE_SISO3166CTRYNAME      As Long = &H5A    'ISO abbreviated country name

Private Const LOCALE_JAPAN                 As Long = &H411
Private Const LOCALE_KOREA                 As Long = &H412
Private Const LOCALE_ZH_CN                 As Long = &H404
Private Const LOCALE_ZH_TW                 As Long = &H804

Private Const RES_PREFIX = ".\Resources\Resources.dll"

Declare Function GetLocaleInfo Lib "kernel32" Alias _
"GetLocaleInfoA" (ByVal Locale As Long, ByVal LCType As Long, ByVal lpLCData As String, _
ByVal cchData As Long) As Long

Declare Function WritePrivateProfileString& Lib "kernel32" Alias "WritePrivateProfileStringA" (ByVal AppName$, ByVal KeyName$, ByVal keydefault$, ByVal fileName$)
Declare Function LoadLibrary Lib "kernel32" Alias "LoadLibraryA" (ByVal lpLibFileName As String) As Long
Private Declare Function LoadString Lib "user32" Alias "LoadStringA" _
   (ByVal hInstance As Long, ByVal wID As Long, ByVal lpBuffer As String, _
    ByVal nBufferMax As Long) As Long

'WinHelp Commands
'Declare Function WinHelp Lib "user32" Alias "WinHelpA" (ByVal hWnd As Long, ByVal lpHelpFile As String, ByVal wCommand As Long, ByVal dwData As Long) As Long
'Public Const HELP_QUIT = &H2              '  Terminate help
'Public Const HELP_CONTENTS = &H3&         '  Display index/contents
'Public Const HELP_CONTEXT = &H1           '  Display topic in ulTopic
'Public Const HELP_INDEX = &H3             '  Display index

Public Const CBASE_RESOURCE_DIR = ".\resources"
Private mStrTrue As String
Private mLocaleDir As String
Private ghInst As Long


Function getLocaleDir() As String
    If mLocaleDir = "" Then
        getLocaleLangBaseIDandSetLocaleDir
    End If
    getLocaleDir = mLocaleDir
End Function

Public Function GetLocaleLanguage() As String
    Dim lReturn As Long
    Dim lLocID As Long
    Dim sData As String
    Dim lDataLen As Long

    lDataLen = 0
    lReturn = GetLocaleInfo(lLocID, LOCALE_SENGLANGUAGE, sData, lDataLen)
    sData = String(lReturn, 0) & vbNullChar
    lDataLen = lReturn
    lReturn = GetLocaleInfo(lLocID, LOCALE_SENGLANGUAGE, sData, lDataLen)

End Function

Function getLocaleLangBaseIDandSetLocaleDir() As Integer
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getLocaleLangBaseIDandSetLocaleDir"
    
    Dim baseID As Long
    Dim bUseLocale As Boolean
    Dim fso As FileSystemObject
    Set fso = New FileSystemObject

    Dim isoLangStr As String
    Dim isoCountryStr As String
    Dim langStr As String
        
    Dim userLCID As Long
    userLCID = GetUserDefaultLCID()
    Dim sysLCID As Long
    sysLCID = GetSystemDefaultLCID()
  
    isoLangStr = GetUserLocaleInfo(sysLCID, LOCALE_SISO639LANGNAME)
    isoCountryStr = GetUserLocaleInfo(sysLCID, LOCALE_SISO3166CTRYNAME)
    langStr = GetUserLocaleInfo(sysLCID, LOCALE_SENGLANGUAGE)
    
    baseID = 0
    mLocaleDir = ""
    
    If fso.FileExists(fso.GetAbsolutePathName("debug.ini")) Then
        Dim overrideLangStr As String
        overrideLangStr = ProfileGetItem("debug", "langoverride", "", fso.GetAbsolutePathName("debug.ini"))
        If overrideLangStr <> "" Then
            Debug.Print "Overriding language " & isoLangStr & " with " & overrideLangStr & "\n"
            isoLangStr = overrideLangStr
        End If
    End If
    
    'check for locale dirs in following order:
    '   CBASE_RESOURCE_DIR & "\" & isoLangStr
    '   CBASE_RESOURCE_DIR & "\" & isoLangStr & "-" & isoCountryStr
    '   CBASE_RESOURCE_DIR & "\" & "eng"
    'If fso.FolderExists(fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & isoLangStr)) Then
    '    mLocaleDir = CBASE_RESOURCE_DIR & "\" & isoLangStr
    '    baseID = getBaseID(isoLangStr)
    'ElseIf fso.FolderExists(fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & isoLangStr & "-" & isoCountryStr)) Then
    '    mLocaleDir = CBASE_RESOURCE_DIR & "\" & isoLangStr & "-" & isoCountryStr
    '    baseID = getBaseID(isoLangStr & "-" & isoCountryStr)
    'Else
        mLocaleDir = CBASE_RESOURCE_DIR
        baseID = 1000
    'End If
    
    getLocaleLangBaseIDandSetLocaleDir = CInt(baseID)
    
FinalExit:
    Set fso = Nothing

    Exit Function
    
HandleErrors:
    Debug.Print currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
'--------------------------------------------------------------------------
'this sub must be executed from the immediate window
'it will add the entry to VBADDIN.INI if it doesn't already exist
'so that the add-in is on available next time VB is loaded
'--------------------------------------------------------------------------
Sub AddToINI()
    Debug.Print WritePrivateProfileString("Add-Ins32", WIZARD_NAME & ".Wizard", "0", "VBADDIN.INI")
End Sub

Function GetResString(nRes As Integer) As String
    Dim sTmp As String
    Dim sRes As String * 1024
    Dim sRetStr As String
    Dim nRet As Long
  
    Do
        'sTmp = LoadResString(nRes)
        nRet = LoadString(ghInst, nRes, sRes, 1024)
        sTmp = Left$(sRes, nRet)
        
        If Right(sTmp, 1) = "_" Then
            sRetStr = sRetStr + VBA.Left(sTmp, Len(sTmp) - 1)
        Else
            sRetStr = sRetStr + sTmp
        End If
        nRes = nRes + 1
    Loop Until Right(sTmp, 1) <> "_"
    GetResString = sRetStr
  
End Function

Function GetField(sBuffer As String, sSep As String) As String
    Dim p As Integer
    
    p = InStr(sBuffer & sSep, sSep)
    GetField = VBA.Left(sBuffer, p - 1)
    sBuffer = Mid(sBuffer, p + Len(sSep))
  
End Function
' Parts of the following code are from:
' http://support.microsoft.com/default.aspx?scid=kb;en-us;232625&Product=vb6

Private Function GetCharSet(sCdpg As String) As Integer
   Select Case sCdpg
      Case "932" ' Japanese
         GetCharSet = 128
      Case "936" ' Simplified Chinese
         GetCharSet = 134
      Case "949" ' Korean
         GetCharSet = 129
      Case "950" ' Traditional Chinese
         GetCharSet = 136
      Case "1250" ' Eastern Europe
         GetCharSet = 238
      Case "1251" ' Russian
         GetCharSet = 204
      Case "1252" ' Western European Languages
         GetCharSet = 0
      Case "1253" ' Greek
         GetCharSet = 161
      Case "1254" ' Turkish
         GetCharSet = 162
      Case "1255" ' Hebrew
         GetCharSet = 177
      Case "1256" ' Arabic
         GetCharSet = 178
      Case "1257" ' Baltic
         GetCharSet = 186
      Case Else
         GetCharSet = 0
   End Select
End Function

Private Function StripNullTerminator(sCP As String)
   Dim posNull As Long
   posNull = InStr(sCP, Chr$(0))
   StripNullTerminator = Left$(sCP, posNull - 1)
End Function

Private Function GetResourceDataFileName() As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetResourceDataFileName"
    
    Dim fileName As String
    Dim fso As FileSystemObject
    Set fso = New FileSystemObject

    GetResourceDataFileName = fso.GetAbsolutePathName(RES_PREFIX)
    
    GoTo FinalExit

    ' use the following code when we have one resource file for each language
    Dim isoLangStr As String
    Dim isoCountryStr As String
    
    Dim userLCID As Long
    userLCID = GetUserDefaultLangID()
    Dim sysLCID As Long
    sysLCID = GetSystemDefaultLangID()

    isoLangStr = GetUserLocaleInfo(userLCID, LOCALE_SISO639LANGNAME)
    isoCountryStr = GetUserLocaleInfo(userLCID, LOCALE_SISO3166CTRYNAME)
    
    'check for locale data in following order:
    '  user language
    '   isoLangStr & "_" & isoCountryStr & ".dll"
    '   isoLangStr & ".dll"
    '  system language
    '   isoLangStr & "_" & isoCountryStr & ".dll"
    '   isoLangStr & ".dll"
    '   "en_US" & ".dll"
    
    fileName = fso.GetAbsolutePathName(RES_PREFIX & isoLangStr & "-" & isoCountryStr & ".dll")
    If fso.FileExists(fileName) Then
        GetResourceDataFileName = fileName
    Else
        fileName = fso.GetAbsolutePathName(RES_PREFIX & isoLangStr & ".dll")
        If fso.FileExists(fileName) Then
            GetResourceDataFileName = fileName
        Else
            isoLangStr = GetUserLocaleInfo(sysLCID, LOCALE_SISO639LANGNAME)
            isoCountryStr = GetUserLocaleInfo(sysLCID, LOCALE_SISO3166CTRYNAME)

            fileName = fso.GetAbsolutePathName(RES_PREFIX & isoLangStr & "-" & isoCountryStr & ".dll")
            If fso.FileExists(fileName) Then
                GetResourceDataFileName = fileName
            Else
                fileName = fso.GetAbsolutePathName(RES_PREFIX & isoLangStr & ".dll")
                If fso.FileExists(fileName) Then
                    GetResourceDataFileName = fileName
                Else
                    GetResourceDataFileName = fso.GetAbsolutePathName(RES_PREFIX & "en-US.dll")
                End If
            End If
        End If
    End If
FinalExit:
    Set fso = Nothing
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Sub LoadResStrings(frm As Form)
    Dim ctl As Control
    Dim obj As Object

    Dim LCID As Long, X As Long
    Dim sCodePage As String
    Dim nCharSet As Integer
    Dim currentFunctionName As String
    currentFunctionName = "LoadResStrings"

    On Error GoTo HandleErrors
    ghInst = LoadLibrary(GetResourceDataFileName())

    On Error Resume Next
    
    sCodePage = String$(16, " ")
    LCID = GetThreadLocale() 'Get Current locale
    
    X = GetLocaleInfo(LCID, LOCALE_IDEFAULTANSICODEPAGE, _
        sCodePage, Len(sCodePage))  'Get code page
    sCodePage = StripNullTerminator(sCodePage)
    nCharSet = GetCharSet(sCodePage)  'Convert code page to charset

    'set the form's caption
    If IsNumeric(frm.Tag) Then
        frm.Caption = LoadResString(CInt(frm.Tag))
    End If
    
    'set the controls' captions using the caption
    'property for menu items and the Tag property
    'for all other controls
    For Each ctl In frm.Controls
        Err = 0
        If (nCharSet <> 0) Then
            ctl.Font.Charset = nCharSet
        End If
        If TypeName(ctl) = "Menu" Then
            If IsNumeric(ctl.Caption) Then
                ctl.Caption = LoadResString(CInt(ctl.Caption))
            End If
        ElseIf TypeName(ctl) = "TabStrip" Then
            For Each obj In ctl.Tabs
                If IsNumeric(obj.Tag) Then
                    obj.Caption = LoadResString(CInt(obj.Tag))
                End If
                'check for a tooltip
                If IsNumeric(obj.ToolTipText) Then
                    If Err = 0 Then
                        obj.ToolTipText = LoadResString(CInt(obj.ToolTipText))
                    End If
                End If
            Next
        ElseIf TypeName(ctl) = "Toolbar" Then
            For Each obj In ctl.Buttons
                If IsNumeric(obj.Tag) Then
                    obj.ToolTipText = LoadResString(CInt(obj.Tag))
                End If
            Next
        ElseIf TypeName(ctl) = "ListView" Then
            For Each obj In ctl.ColumnHeaders
                If IsNumeric(obj.Tag) Then
                    obj.Text = LoadResString(CInt(obj.Tag))
                End If
            Next
        ElseIf TypeName(ctl) = "TextBox" Then
            If IsNumeric(ctl.Tag) Then
                ctl.Text = LoadResString(CInt(ctl.Tag))
            End If
        Else
            If IsNumeric(ctl.Tag) Then
                ctl.Caption = GetResString(CInt(ctl.Tag))
            End If
            'check for a tooltip
            If IsNumeric(ctl.ToolTipText) Then
                If Err = 0 Then
                    ctl.ToolTipText = LoadResString(CInt(ctl.ToolTipText))
                End If
            End If
        End If
    Next

FinalExit:
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
    
End Sub

'==================================================
'Purpose: Replace the sToken string(s) in
'         res file string for correct placement
'         of localized tokens
'
'Inputs:  sString = String to search and replace in
'         sToken = token to replace
'         sReplacement = String to replace token with
'
'Outputs: New string with token replaced throughout
'==================================================
Function ReplaceTopicTokens(sString As String, _
                            sToken As String, _
                            sReplacement As String) As String
    On Error Resume Next
    
    Dim p As Integer
    Dim sTmp As String
        
    sTmp = sString
    Do
        p = InStr(sTmp, sToken)
        If p Then
            sTmp = VBA.Left(sTmp, p - 1) + sReplacement + Mid(sTmp, p + Len(sToken))
        End If
    Loop While p
    
    
    ReplaceTopicTokens = sTmp
  
End Function
'==================================================
'Purpose: Replace the sToken1 and sToken2 strings in
'         res file string for correct placement
'         of localized tokens
'
'Inputs:  sString = String to search and replace in
'         sToken1 = 1st token to replace
'         sReplacement1 = 1st String to replace token with
'         sToken2 = 2nd token to replace
'         sReplacement2 = 2nd String to replace token with
'
'Outputs: New string with token replaced throughout
'==================================================
Function ReplaceTopic2Tokens(sString As String, _
                            sToken1 As String, _
                            sReplacement1 As String, _
                            sToken2 As String, _
                            sReplacement2 As String) As String
    On Error Resume Next
    
    ReplaceTopic2Tokens = _
        ReplaceTopicTokens(ReplaceTopicTokens(sString, sToken1, sReplacement1), _
        sToken2, sReplacement2)
End Function


Public Function GetResData(sResName As String, sResType As String) As String
    Dim sTemp As String
    Dim p As Integer
  
    sTemp = StrConv(LoadResData(sResName, sResType), vbUnicode)
    p = InStr(sTemp, vbNullChar)
    If p Then sTemp = VBA.Left$(sTemp, p - 1)
    GetResData = sTemp
End Function

Function AddToAddInCommandBar(VBInst As Object, sCaption As String, oBitmap As Object) As Object   'Office.CommandBarControl
    On Error GoTo AddToAddInCommandBarErr
    
    Dim c As Integer
    Dim cbMenuCommandBar As Object   'Office.CommandBarControl  'command bar object
    Dim cbMenu As Object
    
    'see if we can find the Add-Ins menu
    Set cbMenu = VBInst.CommandBars("Add-Ins")
    If cbMenu Is Nothing Then
        'not available so we fail
        Exit Function
    End If
    
    'add it to the command bar
    Set cbMenuCommandBar = cbMenu.Controls.add(1)
    c = cbMenu.Controls.count - 1
    If cbMenu.Controls(c).BeginGroup And _
        Not cbMenu.Controls(c - 1).BeginGroup Then
        'this s the first addin being added so it needs a separator
        cbMenuCommandBar.BeginGroup = True
    End If
    'set the caption
    cbMenuCommandBar.Caption = sCaption
    'undone:set the onaction (required at this point)
    cbMenuCommandBar.OnAction = "hello"
    'copy the icon to the clipboard
    Clipboard.SetData oBitmap
    'set the icon for the button
    cbMenuCommandBar.PasteFace
  
    Set AddToAddInCommandBar = cbMenuCommandBar
    
    Exit Function
AddToAddInCommandBarErr:
  
End Function

