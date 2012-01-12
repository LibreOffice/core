Attribute VB_Name = "AnalysisDriver"
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

' Declare Public variables.
Public Type ShortItemId
   cb As Long
   abID As Byte
End Type

Public Type ITEMIDLIST
   mkid As ShortItemId
End Type

Public Declare Function FindWindow Lib "user32" Alias _
    "FindWindowA" (ByVal lpClassName As String, _
    ByVal lpWindowName As Long) As Long

Private Declare Function GetTickCount Lib "kernel32" () As Long

'This function saves the passed value to the file,
'under the section and key names specified.
'If the ini file, lpFileName, does not exist, it is created.
'If the section, lpSectionName, does not exist, it is created.
'If the key name, lpKeyName, does not exist, it is created.
'If the key name exists, it's value, lpString, is replaced.
Private Declare Function WritePrivateProfileString Lib "kernel32" _
   Alias "WritePrivateProfileStringA" _
  (ByVal lpSectionName As String, _
   ByVal lpKeyName As Any, _
   ByVal lpString As Any, _
   ByVal lpFileName As String) As Long

Private Declare Function GetPrivateProfileString Lib "kernel32" _
   Alias "GetPrivateProfileStringA" _
  (ByVal lpSectionName As String, _
   ByVal lpKeyName As Any, _
   ByVal lpDefault As String, _
   ByVal lpReturnedString As String, _
   ByVal nSize As Long, _
   ByVal lpFileName As String) As Long

Private Declare Function UrlEscape Lib "shlwapi" _
   Alias "UrlEscapeA" _
  (ByVal pszURL As String, _
   ByVal pszEscaped As String, _
   pcchEscaped As Long, _
   ByVal dwFlags As Long) As Long

Public Declare Function SHGetPathFromIDList Lib "shell32.dll" _
   (ByVal pidl As Long, ByVal pszPath As String) As Long

Public Declare Function SHGetSpecialFolderLocation Lib _
   "shell32.dll" (ByVal hwndOwner As Long, ByVal nFolder _
   As Long, pidl As ITEMIDLIST) As Long

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

Public Declare Function GetSystemDefaultLangID Lib "kernel32" () As Long
Public Declare Function GetUserDefaultLangID Lib "kernel32" () As Long

Public Declare Function GetLocaleInfo Lib "kernel32" _
   Alias "GetLocaleInfoA" _
  (ByVal Locale As Long, _
   ByVal LCType As Long, _
   ByVal lpLCData As String, _
   ByVal cchData As Long) As Long


Public Const CWIZARD = "analysis"

Const CROWOFFSET = 2
Const CDOCPROP_PAW_ROWOFFSET = 3
Private mDocPropRowOffset As Long

Const CNUMBERDOC_ALL = "All"
Const CTOTAL_DOCS_ANALYZED = "TotalDocsAnalysed"
Const CNUMDAYS_IN_MONTH = 30
Const CMAX_LIMIT = 10000

Const CISSUE_DETDOCNAME = 1
Const CISSUE_DETDOCAPPLICATION = CISSUE_DETDOCNAME + 1
Const CISSUE_DETTYPE = CISSUE_DETDOCAPPLICATION + 1
Const CISSUE_DETSUBTYPE = CISSUE_DETTYPE + 1
Const CISSUE_DETLOCATION = CISSUE_DETSUBTYPE + 1
Const CISSUE_DETSUBLOCATION = CISSUE_DETLOCATION + 1
Const CISSUE_DETLINE = CISSUE_DETSUBLOCATION + 1
Const CISSUE_DETCOLUMN = CISSUE_DETLINE + 1
Const CISSUE_DETATTRIBUTES = CISSUE_DETCOLUMN + 1
Const CISSUE_DETNAMEANDPATH = CISSUE_DETATTRIBUTES + 1

Const CREF_DETDOCNAME = 1
Const CREF_DETDOCAPPLICATION = CREF_DETDOCNAME + 1
Const CREF_DETREFERENCE = CREF_DETDOCAPPLICATION + 1
Const CREF_DETDESCRIPTION = CREF_DETREFERENCE + 1
Const CREF_DETLOCATION = CREF_DETDESCRIPTION + 1
Const CREF_DETATTRIBUTES = CREF_DETLOCATION + 1
Const CREF_DETNAMEANDPATH = CREF_DETATTRIBUTES + 1

Const CINPUT_DIR = "indir"
Const COUTPUT_DIR = "outdir"
Const CRESULTS_FILE = "resultsfile"
Const CLOG_FILE = "logfile"
Const CRESULTS_TEMPLATE = "resultstemplate"
Const CRESULTS_EXIST = "resultsexist"
Const COVERWRITE_FILE = "overwritefile"
Const CNEW_RESULTS_FILE = "newresultsfile"
Const CINCLUDE_SUBDIRS = "includesubdirs"
Const CDEBUG_LEVEL = "debuglevel"
Const COUTPUT_TYPE = "outputtype"
Const COUTPUT_TYPE_XLS = "xls"
Const COUTPUT_TYPE_XML = "xml"
Const COUTPUT_TYPE_BOTH = "both"
Const COVERVIEW_TITLE_LABEL = "OV_Document_Analysis_Overview_lbl"
Const CDEFAULT_PASSWORD = "defaultpassword"
Const CVERSION = "version"
Const CTITLE = "title"
Const CDOPREPARE = "prepare"
Const CISSUES_LIMIT = "issuesmonthlimit"
Const CSINGLE_FILE = "singlefile"
Const CFILE_LIST = "filelist"
Const CSTAT_FILE = "statfilename"
Const C_ABORT_ANALYSIS = "abortanalysis"
Const C_DOCS_LESS_3_MONTH = "DocumentsYoungerThan3Month"
Const C_DOCS_LESS_6_MONTH = "DocumentsYoungerThan6Month"
Const C_DOCS_LESS_12_MONTH = "DocumentsYoungerThan12Month"
Const C_DOCS_MORE_12_MONTH = "DocumentsOlderThan12Month"

Private Const C_ANALYSIS                    As String = "Analysis"
Private Const C_LAST_CHECKPOINT             As String = "LastCheckpoint"
Private Const C_NEXT_FILE                   As String = "NextFile"
Private Const C_MAX_CHECK_INI               As String = "FilesBeforeSave"
Private Const C_MAX_WAIT_BEFORE_WRITE_INI   As String = "SecondsBeforeSave"
Private Const C_MAX_RANGE_PROCESS_TIME_INI  As String = "ExcelMaxRangeProcessTime"
Private Const C_ERROR_HANDLING_DOC          As String = "_ERROR_HANDLING_DOC_"
Private Const C_MAX_CHECK                   As Long = 100
Private Const C_MAX_WAIT_BEFORE_WRITE       As Long = 300 ' sec
Private Const C_MAX_RANGE_PROCESS_TIME      As Integer = 30 'sec

Private Const C_STAT_STARTING As Integer = 1
Private Const C_STAT_DONE     As Integer = 2
Private Const C_STAT_FINISHED As Integer = 3

Private Type DocumentCount
    numDocsAnalyzed As Long
    numDocsAnalyzedWithIssues As Long
    numMinorIssues As Long
    numComplexIssues As Long
    numMacroIssues As Long
    numPreparableIssues As Long
    totalMacroCosts As Long
    totalDocIssuesCosts As Long
    totalPreparableIssuesCosts As Long
End Type

Private Type DocModificationDates
    lessThanThreemonths As Long
    threeToSixmonths As Long
    sixToTwelvemonths As Long
    greaterThanOneYear As Long
End Type

Private Type DocMacroClassifications
    None As Long
    Simple As Long
    Medium As Long
    complex As Long
End Type

Private Type DocIssueClassifications
    None As Long
    Minor As Long
    complex As Long
End Type

Const CCOST_COL_OFFSET = -1

Private mLogFilePath As String
Private mDocIndex As String
Private mDebugLevel As Long
Private mIniFilePath As String
Private mUserFormTypesDict As Scripting.Dictionary
Private mIssuesDict As Scripting.Dictionary
Private mMacroDict As Scripting.Dictionary
Private mPreparedIssuesDict As Scripting.Dictionary
Private mIssuesClassificationDict As Scripting.Dictionary
Private mIssuesCostDict As Scripting.Dictionary
Private mIssuesLimit As Date

Public Const CWORD_DRIVER_FILE = "_OOoDocAnalysisWordDriver.doc"
Public Const CEXCEL_DRIVER_FILE = "_OOoDocAnalysisExcelDriver.xls"
Public Const CPP_DRIVER_FILE = "_OOoDocAnalysisPPTDriver.ppt"
Public Const CWORD_DRIVER_FILE_TEMP = "~$OoDocAnalysisWordDriver.doc"
Public Const CEXCEL_DRIVER_FILE_TEMP = "~$OoDocAnalysisExcelDriver.xls"
Public Const CPP_DRIVER_FILE_TEMP = "~$OoDocAnalysisPPTDriver.ppt"

'Doc Properties Offsets - used in WriteDocProperties and GetPreparableFilesFromDocProps
Const CDOCINFONAME = 1
Const CDOCINFOAPPLICATION = CDOCINFONAME + 1

Const CDOCINFOISSUE_CLASS = CDOCINFOAPPLICATION + 1
Const CDOCINFOCOMPLEXISSUES = CDOCINFOISSUE_CLASS + 1
Const CDOCINFOMINORISSUES = CDOCINFOCOMPLEXISSUES + 1
Const CDOCINFOPREPAREDISSUES = CDOCINFOMINORISSUES + 1

Const CDOCINFOMACRO_CLASS = CDOCINFOPREPAREDISSUES + 1
Const CDOCINFOMACRO_USERFORMS = CDOCINFOMACRO_CLASS + 1
Const CDOCINFOMACRO_LINESOFCODE = CDOCINFOMACRO_USERFORMS + 1

Const CDOCINFODOCISSUECOSTS = CDOCINFOMACRO_LINESOFCODE + 1
Const CDOCINFOPREPARABLEISSUECOSTS = CDOCINFODOCISSUECOSTS + 1
Const CDOCINFOMACROISSUECOSTS = CDOCINFOPREPARABLEISSUECOSTS + 1

Const CDOCINFONUMBERPAGES = CDOCINFOMACROISSUECOSTS + 1
Const CDOCINFOCREATED = CDOCINFONUMBERPAGES + 1
Const CDOCINFOLASTMODIFIED = CDOCINFOCREATED + 1
Const CDOCINFOLASTACCESSED = CDOCINFOLASTMODIFIED + 1
Const CDOCINFOLASTPRINTED = CDOCINFOLASTACCESSED + 1
Const CDOCINFOLASTSAVEDBY = CDOCINFOLASTPRINTED + 1
Const CDOCINFOREVISION = CDOCINFOLASTSAVEDBY + 1
Const CDOCINFOTEMPLATE = CDOCINFOREVISION + 1
Const CDOCINFONAMEANDPATH = CDOCINFOTEMPLATE + 1

'Overview shapes
Const COV_DOC_MOD_DATES_CHART = "Chart 21"
Const COV_DOC_MACRO_CHART = "Chart 22"
Const COV_DOC_ANALYSIS_CHART = "Chart 23"

Const COV_DOC_MOD_DATES_COMMENT_TXB = "Text Box 25"
Const COV_DOC_MOD_DATES_LEGEND_TXB = "Text Box 12"

Const COV_DOC_MACRO_COMMENT_TXB = "Text Box 26"
Const COV_DOC_MACRO_LEGEND_TXB = "Text Box 16"

Const COV_DOC_ANALYSIS_COMMENT_TXB = "Text Box 27"
Const COV_DOC_ANALYSIS_LEGEND_DAW_TXB = "Text Box 28"
Const COV_DOC_ANALYSIS_LEGEND_PAW_TXB = "Text Box 18"

Const COV_HIGH_LEVEL_ANALYSIS_RANGE = "OV_High_Level_Analysis_Range"
Const COV_COST_RANGE = "OV_Cost_Range"

'Sheet labels
Const COV_HIGH_LEVEL_ANALYSIS_LBL = "OV_High_level_analysis_lbl"
Const COV_DP_PREPISSUES_COL_LBL = "DocProperties_PreparedIssues_Column"
Const COV_COSTS_PREPISSUE_COUNT_COL_LBL = "Costs_PreparedIssueCount_Column"
Const CDP_DAW_HIDDEN_COLS_LBL = "DP_DAW_HIDDEN_COLS_RANGE"
Const CDP_DAW_HIDDEN_COLS2_LBL = "DP_DAW_HIDDEN_COLS_RANGE2"
Const CDP_DAW_HIDDEN_ROW_LBL = "DP_DAW_HIDDEN_ROW_RANGE"

Const COV_DAW_SETUP_SHEETS_RUN_LBL = "OV_DAW_SETUP_SHEETS_RUN"
Const COV_PAW_SETUP_SHEETS_RUN_LBL = "OV_PAW_SETUP_SHEETS_RUN"
Const COV_Internal_Attributes_Cols_LBL = "OV_Internal_Attributes_Cols"

Const CR_STR = "<CR>"
Const CR_TOPIC = "<TOPIC>"
Const CR_PRODUCT = "<PRODUCT>"

Const CLEGEND_FONT_SIZE = 8
Const CCOMMENTS_FONT_SIZE = 10

Dim mTstart As Single
Dim mTend As Single
Public gExcelMaxRangeProcessTime As Integer

Sub AnalyseDirectory()
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AnalyseDirectory"
    
    Dim iniFilePath As String
    Dim startDir As String
    Dim fileList As String
    Dim storeToDir As String
    Dim resultsFile As String
    Dim resultsTemplate As String
    Dim statFileName As String
    Dim bOverwriteResultsFile As Boolean
    Dim bNewResultsFile As Boolean
    Dim outputType As String
    Dim singleFile As String
    Dim nTimeNeeded As Long
    Dim nIncrementFileCounter As Long
    Dim nMaxWaitBeforeWrite As Long
    Dim fso As Scripting.FileSystemObject
    Set fso = New Scripting.FileSystemObject

    SetAppToMinimized

    If InDocPreparation Then
        mDocPropRowOffset = CDOCPROP_PAW_ROWOFFSET
    Else
        mDocPropRowOffset = CROWOFFSET
    End If

    'Get Wizard input variables
    SetupWizardVariables fileList, storeToDir, resultsFile, _
        mLogFilePath, resultsTemplate, bOverwriteResultsFile, bNewResultsFile, _
        statFileName, mDebugLevel, outputType, singleFile
        
    startDir = ProfileGetItem("Analysis", CINPUT_DIR, "", mIniFilePath)

    nIncrementFileCounter = CLng(ProfileGetItem("Analysis", _
                            C_MAX_CHECK_INI, C_MAX_CHECK, mIniFilePath))
    nMaxWaitBeforeWrite = CLng(ProfileGetItem("Analysis", _
                          C_MAX_WAIT_BEFORE_WRITE_INI, C_MAX_WAIT_BEFORE_WRITE, mIniFilePath))
    gExcelMaxRangeProcessTime = CInt(ProfileGetItem("Analysis", _
                          C_MAX_RANGE_PROCESS_TIME_INI, C_MAX_RANGE_PROCESS_TIME, mIniFilePath))
    LocalizeResources

    'Setup File List
    'For Prepare - get list from results spreadsheet with docs analysis found as preparable
    'If no results spreadsheet then just try to prepare all the docs - run over full analysis list
    Dim myFiles As Collection
    Set myFiles = New Collection
    Dim sAnalysisOrPrep As String
    If InDocPreparation And CheckDoPrepare Then
        sAnalysisOrPrep = "Prepared"
        If fso.FileExists(storeToDir & "\" & resultsFile) Then
            If Not GetPrepareFilesToAnalyze(storeToDir & "\" & resultsFile, myFiles, fso) Then
                SetPrepareToNone
                WriteDebug currentFunctionName & ": No files to analyse!"
                GoTo FinalExit 'No files to prepare - exit
            End If
        Else
            If Not GetFilesToAnalyze(fileList, singleFile, myFiles) Then
                SetPrepareToNone
                WriteDebug currentFunctionName & ": No files to analyse! Filelist (" & fileList & ") empty?"
                GoTo FinalExit 'No files to prepare - exit
            End If
        End If
    Else
        sAnalysisOrPrep = "Analyzed"
        If Not GetFilesToAnalyze(fileList, singleFile, myFiles) Then
            WriteDebug currentFunctionName & ": No files to analyse! Filelist (" & fileList & ") empty?"
            GoTo FinalExit
        End If
    End If

    Dim index As Long
    Dim numFiles As Long
    Dim nextSave As Long
    Dim startIndex As Long
    Dim bResultsWaiting As Boolean
    Dim AnalysedDocs As Collection
    Dim startDate As Date
    Dim currentDate As Date

    Set AnalysedDocs = New Collection
    numFiles = myFiles.count
    bResultsWaiting = False

    If (singleFile <> "") Then
        ' No recovery handling for single file analysis and the value in the
        ' ini file should be used for bNewResultsFile
        startIndex = 1
    Else
        bNewResultsFile = bNewResultsFile And GetIndexValues(startIndex, nextSave, myFiles)
    End If

    startDate = Now()

    ' Analyse all files
    For index = startIndex To numFiles
        Set mIssuesClassificationDict = New Scripting.Dictionary
        mIssuesClassificationDict.CompareMode = TextCompare
        Set mIssuesCostDict = New Scripting.Dictionary
        'mIssuesCostDict.CompareMode = TextCompare
        
        Set mUserFormTypesDict = New Scripting.Dictionary
        Set mIssuesDict = New Scripting.Dictionary
        Set mMacroDict = New Scripting.Dictionary
        Set mPreparedIssuesDict = New Scripting.Dictionary
        
        'Write to Application log
        Dim myAnalyser As MigrationAnalyser
        Set myAnalyser = New MigrationAnalyser

        If (CheckForAbort) Then GoTo FinalExit

        'Log Analysis
        WriteToStatFile statFileName, C_STAT_STARTING, myFiles.item(index), fso
        WriteToLog "Analyzing", myFiles.item(index)
        WriteToIni C_NEXT_FILE, myFiles.item(index)
        mDocIndex = index
        
        'Do Analysis
        myAnalyser.DoAnalyse myFiles.item(index), mUserFormTypesDict, startDir, storeToDir, fso
        
        AnalysedDocs.Add myAnalyser.Results
        bResultsWaiting = True

        WriteToLog sAnalysisOrPrep, index & "of" & numFiles & _
            " " & getAppSpecificApplicationName & " Documents"
        WriteToLog "Analyzing", "Done"
        WriteToLog sAnalysisOrPrep & "Doc" & index, myFiles.item(index)
        Set myAnalyser = Nothing

        If (CheckForAbort) Then GoTo FinalExit

        'No need to output results spreadsheet, just doing prepare
        If CheckDoPrepare Then GoTo CONTINUE_FOR

        nTimeNeeded = val(DateDiff("s", startDate, Now()))
        If ((nTimeNeeded > nMaxWaitBeforeWrite) Or _
            (index >= nextSave)) Then
            If WriteResults(storeToDir, resultsFile, resultsTemplate, _
                            bOverwriteResultsFile, bNewResultsFile, _
                            outputType, AnalysedDocs, fso) Then
                nextSave = index + C_MAX_CHECK
                bResultsWaiting = False
                Set AnalysedDocs = New Collection
                WriteToIni C_LAST_CHECKPOINT, myFiles.item(index)
                startDate = Now()
            Else
                'write error
            End If
        End If
        WriteToStatFile statFileName, C_STAT_DONE, myFiles.item(index), fso
CONTINUE_FOR:
    Next index

    If (bResultsWaiting) Then
        If WriteResults(storeToDir, resultsFile, resultsTemplate, _
                        bOverwriteResultsFile, bNewResultsFile, _
                        outputType, AnalysedDocs, fso) Then
            WriteToIni C_LAST_CHECKPOINT, myFiles.item(index - 1)
        Else
            'write error
        End If
    End If
    WriteToStatFile statFileName, C_STAT_FINISHED, "", fso

FinalExit:

    Set fso = Nothing
    Set myFiles = Nothing
    Set mIssuesClassificationDict = Nothing
    Set mIssuesCostDict = Nothing
    Set mUserFormTypesDict = Nothing
    Set mIssuesDict = Nothing
    Set mMacroDict = Nothing
    Set mPreparedIssuesDict = Nothing
    
    Set AnalysedDocs = Nothing
    
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Function WriteResults(storeToDir As String, resultsFile As String, resultsTemplate As String, _
                      bOverwriteResultsFile As Boolean, bNewResultsFile As Boolean, _
                      outputType As String, AnalysedDocs As Collection, _
                      fso As FileSystemObject) As Boolean

    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteResults"
    
    If InDocPreparation Then
        If outputType = COUTPUT_TYPE_XML Or outputType = COUTPUT_TYPE_BOTH Then
            WriteXMLOutput storeToDir, resultsFile, _
                bOverwriteResultsFile, bNewResultsFile, AnalysedDocs, fso
        End If
    End If
    
    If outputType = COUTPUT_TYPE_XLS Or outputType = COUTPUT_TYPE_BOTH Then
        WriteXLSOutput storeToDir, resultsFile, fso.GetAbsolutePathName(resultsTemplate), _
                       bOverwriteResultsFile, bNewResultsFile, AnalysedDocs, fso
    End If
    
    WriteResults = True
    bNewResultsFile = False

FinalExit:
    Exit Function

HandleErrors:
    WriteResults = False
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function GetFilesToAnalyze_old(startDir As String, bIncludeSubdirs As Boolean, _
    myFiles As Collection) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetFilesToAnalyze"
    Dim fso As New FileSystemObject
    Dim theResultsFile As String
    theResultsFile = ProfileGetItem("Analysis", CINPUT_DIR, "c:\", mIniFilePath) & "\" & ProfileGetItem("Analysis", CRESULTS_FILE, "", mIniFilePath)
    
    GetFilesToAnalyze = False

    Dim searchTypes As Collection
    Set searchTypes = New Collection
    SetupSearchTypes searchTypes
    If searchTypes.count = 0 Then
        GoTo FinalExit
    End If
    
    Dim myDocFiles As CollectedFiles
    Set myDocFiles = New CollectedFiles
    With myDocFiles
        .BannedList.Add fso.GetAbsolutePathName(getAppSpecificPath & "\" & CWORD_DRIVER_FILE)
        .BannedList.Add fso.GetAbsolutePathName(getAppSpecificPath & "\" & CEXCEL_DRIVER_FILE)
        .BannedList.Add fso.GetAbsolutePathName(getAppSpecificPath & "\" & CPP_DRIVER_FILE)
        .BannedList.Add fso.GetAbsolutePathName(getAppSpecificPath & "\" & CWORD_DRIVER_FILE_TEMP)
        .BannedList.Add fso.GetAbsolutePathName(getAppSpecificPath & "\" & CEXCEL_DRIVER_FILE_TEMP)
        .BannedList.Add fso.GetAbsolutePathName(getAppSpecificPath & "\" & CPP_DRIVER_FILE_TEMP)
        .BannedList.Add theResultsFile
    End With
    myDocFiles.Search rootDir:=startDir, FileSpecs:=searchTypes, _
        IncludeSubdirs:=bIncludeSubdirs
    
    If getAppSpecificApplicationName = CAPPNAME_WORD Then
        Set myFiles = myDocFiles.WordFiles
    ElseIf getAppSpecificApplicationName = CAPPNAME_EXCEL Then
        Set myFiles = myDocFiles.ExcelFiles
    ElseIf getAppSpecificApplicationName = CAPPNAME_POWERPOINT Then
        Set myFiles = myDocFiles.PowerPointFiles
    Else
        WriteDebug currentFunctionName & " : invalid application " & getAppSpecificApplicationName
        GoTo FinalExit
    End If
    
    GetFilesToAnalyze = True

FinalExit:
    Set searchTypes = Nothing
    Set myDocFiles = Nothing
    
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
    
Function GetFilesToAnalyze(fileList As String, startFile As String, _
                           myFiles As Collection) As Boolean

    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetFilesToAnalyze"
    
    Dim fso As New FileSystemObject
    Dim fileContent As TextStream
    Dim fileName As String

    GetFilesToAnalyze = False
    
    If (startFile = "") Then
        If (fso.FileExists(fileList)) Then
            Set fileContent = fso.OpenTextFile(fileList, ForReading, False, TristateTrue)
            While (Not fileContent.AtEndOfStream)
                fileName = fileContent.ReadLine
                fileName = Trim(fileName)
                If (fileName <> "") Then
                    myFiles.Add (fileName)
                End If
            Wend
            fileContent.Close
        End If
    Else
        myFiles.Add (startFile)
    End If

    If (myFiles.count <> 0) Then GetFilesToAnalyze = True

FinalExit:
    Set fileContent = Nothing
    Set fso = Nothing
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function GetPrepareFilesToAnalyze(resultsFilePath As String, myFiles As Collection, _
    fso As FileSystemObject) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetPrepareFilesToAnalyze"

    GetPrepareFilesToAnalyze = False
    
    If Not fso.FileExists(resultsFilePath) Then
        WriteDebug currentFunctionName & ": results file does not exist : " & resultsFilePath
        GoTo FinalExit
    End If
    
    'Open results spreadsheet
    Dim xl As Excel.Application
    If getAppSpecificApplicationName = CAPPNAME_EXCEL Then
        Set xl = Application
        xl.Visible = True
    Else
        Set xl = GetExcelInstance
        xl.Visible = False
    End If
    Dim logWb As WorkBook
    Set logWb = xl.Workbooks.Open(resultsFilePath)
        
    Dim wsDocProp As Worksheet
    Set wsDocProp = logWb.Sheets(RID_STR_COMMON_RESULTS_SHEET_NAME_DOCPROP)

    Dim startRow As Long
    Dim endRow As Long
    startRow = mDocPropRowOffset + 1
    endRow = GetWorkbookNameValueAsLong(logWb, CTOTAL_DOCS_ANALYZED) + mDocPropRowOffset
    
    GetPreparableFilesFromDocProps wsDocProp, startRow, endRow, fso, myFiles
    
    GetPrepareFilesToAnalyze = (myFiles.count > 0)

FinalExit:
    Set wsDocProp = Nothing
    If Not logWb Is Nothing Then logWb.Close
    Set logWb = Nothing
    
    If getAppSpecificApplicationName <> CAPPNAME_EXCEL Then
        If Not xl Is Nothing Then
            If xl.Workbooks.count = 0 Then
                xl.Quit
            End If
        End If
    End If
    Set xl = Nothing
    
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function GetPreparableFilesFromDocProps(wsDocProp As Worksheet, startRow As Long, _
    endRow As Long, fso As FileSystemObject, myFiles As Collection) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetPreparableFilesFromDocProps"
    GetPreparableFilesFromDocProps = False
    
    Dim index As Long
    Dim fileName As String
    Dim fileExt As String
    Dim docExt As String
    Dim templateExt As String
    
    docExt = getAppSpecificDocExt
    templateExt = getAppSpecificTemplateExt
        
    For index = startRow To endRow
        If GetWorksheetCellValueAsLong(wsDocProp, index, CDOCINFOPREPAREDISSUES) > 0 Then
            fileName = GetWorksheetCellValueAsString(wsDocProp, index, CDOCINFONAME)
            fileExt = "." & fso.GetExtensionName(fileName)
            'Don't have to worry about search types - just looking at existing results
            'so just check both legal extensions for this application
            If fileExt = docExt Or fileExt = templateExt Then
                myFiles.Add GetWorksheetCellValueAsString(wsDocProp, index, CDOCINFONAMEANDPATH)
            End If
        End If
    Next index
    
    GetPreparableFilesFromDocProps = myFiles.count > 0
FinalExit:
    Exit Function
    
HandleErrors:
    GetPreparableFilesFromDocProps = False
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Sub OpenXLSResultFile(resultsFile As String, _
                      resultsTemplate As String, _
                      bNewResultsFile As Boolean, _
                      excelApp As Excel.Application, _
                      resultSheet As Excel.WorkBook)

    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "OpenXLSResultFile"

    If getAppSpecificApplicationName = CAPPNAME_EXCEL Then
        Set excelApp = Application
        excelApp.Visible = True
    Else
        Set excelApp = GetExcelInstance
        excelApp.Visible = False
    End If

    If bNewResultsFile Then
        Set resultSheet = excelApp.Workbooks.Add(Template:=resultsTemplate)
        Localize_WorkBook resultSheet
    Else
        Set resultSheet = excelApp.Workbooks.Open(resultsFile)
    End If

FinalExit:
    Exit Sub

HandleErrors:
    excelApp.DisplayAlerts = False
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub CloseXLSResultFile(excelApp As Excel.Application, _
                       resultSheet As Excel.WorkBook)

    On Error Resume Next

    If Not resultSheet Is Nothing Then resultSheet.Close
    Set resultSheet = Nothing

    If getAppSpecificApplicationName <> CAPPNAME_EXCEL Then
        If Not excelApp Is Nothing Then
            excelApp.Visible = True
            If excelApp.Workbooks.count = 0 Then
                excelApp.Quit
            End If
        End If
    End If
    Set excelApp = Nothing

    Exit Sub
End Sub

Sub WriteXLSOutput(storeToDir As String, resultsFile As String, resultsTemplate As String, _
    bOverwriteResultsFile As Boolean, bNewResultsFile As Boolean, AnalysedDocs As Collection, _
    fso As Scripting.FileSystemObject)

    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXLSOutput"

    Dim offsetDocPropRow As Long
    Dim offsetDocIssuesRow As Long
    Dim offsetDocIssueDetailsRow As Long
    Dim offsetDocRefDetailsRow As Long

    Const COVERVIEW_SHEET_IDX = 1
    Const CDOCLIST_SHEET_IDX = 2
    Const CISSUES_ANALYSED_SHEET = 3
    Const CISSUE_DETAILS_SHEET = 4
    Const CWORD_ISSUES_SHEET = 5
    Const CEXCEL_ISSUES_SHEET = 6
    Const CPOWERPOINT_ISSUES_SHEET = 7
    Const CREFERENCE_ISSUES_SHEET = 8

    'Begin writing stats to excel
    Dim xl As Excel.Application
    If getAppSpecificApplicationName = CAPPNAME_EXCEL Then
        Set xl = Application
        xl.Visible = True
    Else
        Set xl = GetExcelInstance
        xl.Visible = False
    End If

    Dim logWb As WorkBook

    If bNewResultsFile Then
        Set logWb = xl.Workbooks.Add(Template:=resultsTemplate)
        Localize_WorkBook logWb
    Else
        Set logWb = xl.Workbooks.Open(storeToDir & "\" & resultsFile)
    End If

    SetupAnalysisResultsVariables logWb, offsetDocPropRow, _
        offsetDocIssuesRow, offsetDocIssueDetailsRow, offsetDocRefDetailsRow

    ' Iterate through results and write info
    Dim aAnalysis As DocumentAnalysis
    Dim row As Long
    Dim docCounts As DocumentCount
    Dim templateCounts As DocumentCount

    Dim issuesRow As Long
    Dim issueDetailsRow As Long
    Dim refDetailsRow As Long

    Dim wsOverview As Worksheet
    Dim wsCosts As Worksheet
    Dim wsPgStats As Worksheet
    Dim wsIssues As Worksheet
    Dim wsIssueDetails As Worksheet
    Dim wsRefDetails As Worksheet

    Set wsOverview = logWb.Sheets(COVERVIEW_SHEET_IDX)
    Set wsPgStats = logWb.Sheets(CDOCLIST_SHEET_IDX)

    'Some localized names might be longer than 31 chars, excel doesn't
    'allow such names!
    On Error Resume Next
    wsOverview.name = RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW
    wsPgStats.name = RID_STR_COMMON_RESULTS_SHEET_NAME_DOCPROP
    On Error GoTo HandleErrors

    If InDocPreparation Then
        Set wsCosts = logWb.Sheets(CISSUES_ANALYSED_SHEET)
        Dim appName As String
        appName = getAppSpecificApplicationName
        Select Case appName
            Case "Word"
                Set wsIssues = logWb.Worksheets(CWORD_ISSUES_SHEET)
            Case "Excel"
                Set wsIssues = logWb.Worksheets(CEXCEL_ISSUES_SHEET)
            Case "PowerPoint"
                Set wsIssues = logWb.Worksheets(CPOWERPOINT_ISSUES_SHEET)
            Case Default
                Err.Raise Number:=-1, Description:="BadAppName"
        End Select
        Set wsIssueDetails = logWb.Sheets(CISSUE_DETAILS_SHEET)
        Set wsRefDetails = logWb.Sheets(CREFERENCE_ISSUES_SHEET)
        issuesRow = 1 + CROWOFFSET + offsetDocIssuesRow
        issueDetailsRow = 1 + CROWOFFSET + offsetDocIssueDetailsRow
        refDetailsRow = 1 + CROWOFFSET + offsetDocRefDetailsRow
        ' localize PAW worksheets
        Dim wsWordIssues As Worksheet
        Dim wsExcelIssues As Worksheet
        Dim wsPowerPointIssues As Worksheet
        Set wsWordIssues = logWb.Worksheets(CWORD_ISSUES_SHEET)
        Set wsExcelIssues = logWb.Worksheets(CEXCEL_ISSUES_SHEET)
        Set wsPowerPointIssues = logWb.Worksheets(CPOWERPOINT_ISSUES_SHEET)

        On Error Resume Next
        wsCosts.name = RID_STR_COMMON_RESULTS_SHEET_NAME_COSTS
        wsIssueDetails.name = RID_STR_COMMON_RESULTS_SHEET_NAME_DOCISSUE_DETAILS
        wsRefDetails.name = RID_STR_COMMON_RESULTS_SHEET_NAME_DOCREF_DETAILS
        wsWordIssues.name = RID_STR_COMMON_RESULTS_SHEET_NAME_DOCISSUES_WORD
        wsExcelIssues.name = RID_STR_COMMON_RESULTS_SHEET_NAME_DOCISSUES_EXCEL
        wsPowerPointIssues.name = RID_STR_COMMON_RESULTS_SHEET_NAME_DOCISSUES_POWERPOINT
        On Error GoTo HandleErrors
    End If

    Dim fileName As String
    Dim macroClasses As DocMacroClassifications
    Dim issueClasses As DocIssueClassifications

    For row = 1 To AnalysedDocs.count 'Need Row count - so not using Eor Each
        Set aAnalysis = AnalysedDocs.item(row)
        fileName = fso.GetFileName(aAnalysis.name)

        If InDocPreparation Then
            issuesRow = WriteDocIssues(wsIssues, issuesRow, aAnalysis, fileName)
            issueDetailsRow = _
                ProcessIssuesAndWriteDocIssueDetails(logWb, wsIssueDetails, issueDetailsRow, aAnalysis, fileName)
            refDetailsRow = _
                WriteDocRefDetails(wsRefDetails, refDetailsRow, aAnalysis, fileName)
            aAnalysis.MacroCosts = getMacroIssueCosts(logWb, aAnalysis)
            WriteDocProperties wsPgStats, row + offsetDocPropRow, aAnalysis, fileName
        Else
            ProcessIssuesForDAW logWb, aAnalysis, fileName
            WriteDocProperties wsPgStats, row + offsetDocPropRow, aAnalysis, fileName
        End If
        
        UpdateAllCounts aAnalysis, docCounts, templateCounts, macroClasses, issueClasses, fso
        
        Set aAnalysis = Nothing
    Next row
    
    ' We change the font used for text box shapes here for the japanese
    ' version, because office 2000 sometimes displays squares instead of
    ' chars
    Dim langStr As String
    Dim userLCID As Long
    Dim textSize As Long
    Dim fontName As String

    userLCID = GetUserDefaultLangID()
    langStr = GetUserLocaleInfo(userLCID, LOCALE_SISO639LANGNAME)

    If (langStr = "ja") Then
        WriteDebug currentFunctionName & " : Setting font to MS PGothic for 'ja' locale"
        fontName = "MS PGothic"
        textSize = 10
    Else
        fontName = "Arial"
        textSize = CLEGEND_FONT_SIZE
    End If

    'DAW - PAW switches
    If InDocPreparation Then
        SaveAnalysisResultsVariables logWb, issueDetailsRow - (1 + CROWOFFSET), _
            refDetailsRow - (1 + CROWOFFSET)

        WriteOverview logWb, docCounts, templateCounts, macroClasses, issueClasses

        SetupPAWResultsSpreadsheet logWb, fontName, textSize
        WriteIssueCounts logWb
    Else
        WriteOverview logWb, docCounts, templateCounts, macroClasses, issueClasses

        'StartTiming
        SetupDAWResultsSpreadsheet logWb, fontName, textSize
        'EndTiming "SetupDAWResultsSpreadsheet"
    End If

    SetupPrintRanges logWb, row, issuesRow, issueDetailsRow, refDetailsRow
    
    If resultsFile <> "" Then
       'Overwrite existing results file without prompting
       If bOverwriteResultsFile Or (Not bNewResultsFile) Then
           xl.DisplayAlerts = False
       End If
         
       logWb.SaveAs fileName:=storeToDir & "\" & resultsFile
       xl.DisplayAlerts = True
    End If

FinalExit:
    If Not xl Is Nothing Then
        xl.Visible = True
    End If
    
    Set wsOverview = Nothing
    Set wsPgStats = Nothing

    If InDocPreparation Then
        Set wsCosts = Nothing
        Set wsIssues = Nothing
        Set wsIssueDetails = Nothing
        Set wsRefDetails = Nothing
    End If
    
    If Not logWb Is Nothing Then logWb.Close
    Set logWb = Nothing
    
    If getAppSpecificApplicationName <> CAPPNAME_EXCEL Then
        If Not xl Is Nothing Then
            If xl.Workbooks.count = 0 Then
                xl.Quit
            End If
        End If
    End If
    Set xl = Nothing
    
    Exit Sub
    
HandleErrors:
    xl.DisplayAlerts = False

    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Public Sub StartTiming()
        mTstart = 0
        mTend = 0
        mTstart = GetTickCount()
End Sub
Public Sub EndTiming(what As String)
        mTend = GetTickCount()
        WriteDebug "Timing: " & what & ": " & (FormatNumber((mTend - mTstart) / 1000, 0) & " seconds")
        mTstart = 0
        mTend = 0
End Sub
Sub WriteIssueCounts(logWb As WorkBook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteIssueCounts"
    
    Dim Str As String
    Dim str1 As String
    Dim val1 As Long
    Dim count As Long
    Dim vKeyArray As Variant
    Dim vItemArray As Variant
    Dim vPrepKeyArray As Variant
    Dim vPrepItemArray As Variant
    
    vKeyArray = mIssuesDict.Keys
    vItemArray = mIssuesDict.Items
    
    vPrepKeyArray = mPreparedIssuesDict.Keys
    vPrepItemArray = mPreparedIssuesDict.Items
    
    'Write Issue Counts across all Documents
    For count = 0 To mIssuesDict.count - 1
        str1 = vKeyArray(count)
        val1 = CInt(vItemArray(count))
        logWb.Names(str1).RefersToRange.Cells(1, 1) = _
            logWb.Names(str1).RefersToRange.Cells(1, 1).value + vItemArray(count)
        'DEBUG: str = str & "Key: " & str1 & " Value: " & val1 & vbLf
    Next count
    
    'Write Prepared Issues Counts across all Documents
    For count = 0 To mPreparedIssuesDict.count - 1
        str1 = vPrepKeyArray(count)
        val1 = CInt(vPrepItemArray(count))
        AddVariantToWorkbookNameValue logWb, str1, vPrepItemArray(count)
        'DEBUG: str = str & "Key: " & str1 & " Value: " & val1 & vbLf
    Next count
    
    'User Form control type count across all analyzed documents of this type
    str1 = getAppSpecificApplicationName & "_" & _
        CSTR_ISSUE_VBA_MACROS & "_" & _
        CSTR_SUBISSUE_PROPERTIES & "_" & _
        CSTR_SUBISSUE_VBA_MACROS_USERFORMS_CONTROLTYPE_COUNT
    SetWorkbookNameValueToLong logWb, str1, mUserFormTypesDict.count

    'Add list of User Form controls and counts to ...USERFORMS_CONTROLTYPE_COUNT field
    If mUserFormTypesDict.count > 0 Then
        vKeyArray = mUserFormTypesDict.Keys
        vItemArray = mUserFormTypesDict.Items
        
        Str = RID_STR_COMMON_ATTRIBUTE_CONTROLS & ": "
        For count = 0 To mUserFormTypesDict.count - 1
            Str = Str & vbLf & vKeyArray(count) & " " & vItemArray(count)
        Next count
        WriteUserFromControlTypesComment logWb, str1, Str
    End If
    'DEBUG: MsgBox str & vbLf & mIssuesDict.count

    WriteUniqueModuleCount logWb

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : logging costs : " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub WriteUniqueModuleCount(logWb As WorkBook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteUniqueModuleCount"
    
    Dim strLabel As String
    Dim uniqueLineCount As Long
    Dim uniqueModuleCount As Long
    Dim count As Long
    Dim vItemArray As Variant
    
    vItemArray = mMacroDict.Items
    
    'Write Issues Costs
    uniqueLineCount = 0
    For count = 0 To mMacroDict.count - 1
        uniqueLineCount = uniqueLineCount + CInt(vItemArray(count))
    Next count
    uniqueModuleCount = mMacroDict.count
    
    
    strLabel = getAppSpecificApplicationName & "_" & _
        CSTR_ISSUE_VBA_MACROS & "_" & _
        CSTR_SUBISSUE_PROPERTIES & "_" & _
        CSTR_SUBISSUE_VBA_MACROS_UNIQUE_MODULE_COUNT
    SetWorkbookNameValueToLong logWb, strLabel, uniqueModuleCount

    strLabel = getAppSpecificApplicationName & "_" & _
        CSTR_ISSUE_VBA_MACROS & "_" & _
        CSTR_SUBISSUE_PROPERTIES & "_" & _
        CSTR_SUBISSUE_VBA_MACROS_UNIQUE_LINE_COUNT
    SetWorkbookNameValueToLong logWb, strLabel, uniqueLineCount

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : logging Unique Module/ Line Counts : " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteUserFromControlTypesComment(logWb As WorkBook, name As String, comment As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteUserFromControlTypesComment"

    On Error Resume Next 'Ignore error if trying to add comment again - would happen on append to results
    logWb.Names(name).RefersToRange.Cells(1, 1).AddComment
    
    On Error GoTo HandleErrors
    logWb.Names(name).RefersToRange.Cells(1, 1).comment.Text Text:=comment
    'Autosize not supported - Office 2000
    'logWb.Names(name).RefersToRange.Cells(1, 1).comment.AutoSize = True
    logWb.Names(name).RefersToRange.Cells(1, 1).comment.Visible = False

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : name : " & name & _
    " : comment : " & comment & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub UpdateAllCounts(aAnalysis As DocumentAnalysis, counts As DocumentCount, templateCounts As DocumentCount, _
                    macroClasses As DocMacroClassifications, issueClasses As DocIssueClassifications, _
    fso As FileSystemObject)
    Const CMODDATE_LESS3MONTHS = 91
    Const CMODDATE_LESS6MONTHS = 182
    Const CMODDATE_LESS12MONTHS = 365
    
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "UpdateAllCounts"
    'DocIssue Classification occurs in setDocOverallIssueClassification under
    '   ProcessIssuesAndWriteDocIssueDetails when all DocIssues are being traversed.
    'MacroClass for the Doc is setup at the end of the Analyze_Macros in DoAnalysis
    'Mod Dates are determined in SetDocProperties in DoAnalysis
    
    'DocMacroClassifications
    Select Case aAnalysis.MacroOverallClass
    Case enMacroComplex
        macroClasses.complex = macroClasses.complex + 1
    Case enMacroMedium
        macroClasses.Medium = macroClasses.Medium + 1
    Case enMacroSimple
        macroClasses.Simple = macroClasses.Simple + 1
    Case Else
        macroClasses.None = macroClasses.None + 1
    End Select
    
    'DocIssueClassifications
    aAnalysis.BelowIssuesLimit = True
    Select Case aAnalysis.DocOverallIssueClass
    Case enComplex
        issueClasses.complex = issueClasses.complex + 1
    Case enMinor
        issueClasses.Minor = issueClasses.Minor + 1
    Case Else
        issueClasses.None = issueClasses.None + 1
    End Select
    
    'DocumentCounts
    Dim extStr As String
    extStr = "." & LCase(fso.GetExtensionName(aAnalysis.name))
    If extStr = getAppSpecificDocExt Then
        UpdateDocCounts counts, aAnalysis
    ElseIf extStr = getAppSpecificTemplateExt Then
        UpdateDocCounts templateCounts, aAnalysis
    Else
        WriteDebug currentFunctionName & " : path " & aAnalysis.name & _
            ": unhandled file extesnion " & extStr & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    End If
    
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub UpdateDocCounts(counts As DocumentCount, aAnalysis As DocumentAnalysis)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "UpdateDocCounts"

    counts.numDocsAnalyzed = counts.numDocsAnalyzed + 1
    If aAnalysis.IssuesCount > 0 Then 'During Analysis incremented
        counts.numDocsAnalyzedWithIssues = counts.numDocsAnalyzedWithIssues + 1
        
        If aAnalysis.BelowIssuesLimit Then
            counts.numMinorIssues = _
                counts.numMinorIssues + aAnalysis.MinorIssuesCount
                'MinorIssuesCount incemented as all DocIssues are being traversed are being written out - ProcessIssuesAndWriteDocIssueDetails
            counts.numComplexIssues = counts.numComplexIssues + aAnalysis.ComplexIssuesCount 'Calculated
            counts.totalDocIssuesCosts = counts.totalDocIssuesCosts + _
                aAnalysis.DocIssuesCosts
            counts.totalPreparableIssuesCosts = counts.totalPreparableIssuesCosts + _
                aAnalysis.PreparableIssuesCosts
        End If
        
        counts.numMacroIssues = counts.numMacroIssues + aAnalysis.MacroIssuesCount 'During Analysis incremented
        counts.totalMacroCosts = counts.totalMacroCosts + aAnalysis.MacroCosts
    End If

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub


Sub WriteDocProperties(wsPgStats As Worksheet, row As Long, aAnalysis As DocumentAnalysis, _
    fileName As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteDocProperties"
    
    Dim rowIndex As Long
    rowIndex = row + mDocPropRowOffset
    
    If aAnalysis.Application = RID_STR_COMMON_CANNOT_OPEN Then
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFONAME, fileName
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOAPPLICATION, aAnalysis.Application
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFONAMEANDPATH, aAnalysis.name
            
        GoTo FinalExit
    End If
    
    If InDocPreparation Then
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFONAME, fileName
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOAPPLICATION, aAnalysis.Application
            
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFODOCISSUECOSTS, aAnalysis.DocIssuesCosts
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFOPREPARABLEISSUECOSTS, aAnalysis.PreparableIssuesCosts
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFOMACROISSUECOSTS, aAnalysis.MacroCosts
        
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOISSUE_CLASS, _
            getDocOverallIssueClassificationAsString(aAnalysis.DocOverallIssueClass)
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFOCOMPLEXISSUES, aAnalysis.ComplexIssuesCount
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFOMINORISSUES, aAnalysis.MinorIssuesCount
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFOPREPAREDISSUES, aAnalysis.PreparableIssuesCount
            
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOMACRO_CLASS, _
            getDocOverallMacroClassAsString(aAnalysis.MacroOverallClass)
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFOMACRO_USERFORMS, aAnalysis.MacroNumUserForms
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFOMACRO_LINESOFCODE, aAnalysis.MacroTotalNumLines
        
        SetWorksheetCellValueToLong wsPgStats, rowIndex, CDOCINFONUMBERPAGES, aAnalysis.PageCount
        SetWorksheetCellValueToVariant wsPgStats, rowIndex, CDOCINFOCREATED, CheckDate(aAnalysis.Created)
        SetWorksheetCellValueToVariant wsPgStats, rowIndex, CDOCINFOLASTMODIFIED, CheckDate(aAnalysis.Modified)
        SetWorksheetCellValueToVariant wsPgStats, rowIndex, CDOCINFOLASTACCESSED, CheckDate(aAnalysis.Accessed)
        SetWorksheetCellValueToVariant wsPgStats, rowIndex, CDOCINFOLASTPRINTED, CheckDate(aAnalysis.Printed)
            
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOLASTSAVEDBY, aAnalysis.SavedBy
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOREVISION, aAnalysis.Revision
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOTEMPLATE, aAnalysis.Template
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFONAMEANDPATH, aAnalysis.name
    Else
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFONAME, fileName
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOAPPLICATION, aAnalysis.Application
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOISSUE_CLASS, _
            getDocOverallIssueClassificationAsString(aAnalysis.DocOverallIssueClass)
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFOMACRO_CLASS, _
            getDocOverallMacroClassAsString(aAnalysis.MacroOverallClass)
        SetWorksheetCellValueToVariant wsPgStats, rowIndex, CDOCINFOLASTMODIFIED, CheckDate(aAnalysis.Modified)
        SetWorksheetCellValueToString wsPgStats, rowIndex, CDOCINFONAMEANDPATH, aAnalysis.name
    End If
    
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Function CheckDate(myDate As Date) As Variant
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckDate"
    
    Dim lowerNTDateLimit As Date
    If Not IsDate(myDate) Then
        CheckDate = RID_STR_COMMON_NA
        Exit Function
    End If
    
    lowerNTDateLimit = DateSerial(1980, 1, 1)
    CheckDate = IIf(myDate < lowerNTDateLimit, RID_STR_COMMON_NA, myDate)
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : date " & myDate & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function WriteDocIssues(wsIssues As Worksheet, row As Long, _
    aAnalysis As DocumentAnalysis, fileName As String) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteDocIssues"

    Const CNAME = 1
    Const CAPPLICATION = CNAME + 1
    Const CISSUE_COLUMNOFFSET = CAPPLICATION

    If aAnalysis.IssuesCount = 0 Then
        WriteDocIssues = row
        Exit Function
    End If
    SetWorksheetCellValueToString wsIssues, row, CNAME, fileName
    SetWorksheetCellValueToString wsIssues, row, CAPPLICATION, aAnalysis.Application
  
    Dim index As Integer
    For index = 1 To aAnalysis.TotalIssueTypes
        If aAnalysis.IssuesCountArray(index) > 0 Then
            SetWorksheetCellValueToString wsIssues, row, CISSUE_COLUMNOFFSET + index, aAnalysis.IssuesCountArray(index)
        End If
    Next index
    SetWorksheetCellValueToString wsIssues, row, CISSUE_COLUMNOFFSET + aAnalysis.TotalIssueTypes + 1, aAnalysis.name
    
    WriteDocIssues = row + 1
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Sub ProcessIssuesForDAW(logWb As WorkBook, aAnalysis As DocumentAnalysis, fileName As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "ProcessIssuesForDAW"
    
    Dim myIssue As IssueInfo
    Dim issueClass As EnumDocOverallIssueClass
            
    Dim index As Integer
    For index = 1 To aAnalysis.Issues.count
        Set myIssue = aAnalysis.Issues(index)
                
        If Not isMacroIssue(myIssue) Then
            issueClass = getDocIssueClassification(logWb, myIssue)
            CountDocIssuesForDoc issueClass, aAnalysis
            SetOverallDocIssueClassification issueClass, aAnalysis
        End If
                
        Set myIssue = Nothing
    Next index
        
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Function ProcessIssuesAndWriteDocIssueDetails(logWb As WorkBook, wsIssueDetails As Worksheet, DetailsRow As Long, _
    aAnalysis As DocumentAnalysis, fileName As String) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "ProcessIssueAndWriteDocIssueDetails"
    
    Dim myIssue As IssueInfo
    Dim rowIndex As Long
    Dim issueClass As EnumDocOverallIssueClass
    Dim issueCost As Long
    
    rowIndex = DetailsRow
        
    Dim index As Integer
    For index = 1 To aAnalysis.Issues.count
        Set myIssue = aAnalysis.Issues(index)
                
        ' Process Document Issues and Costs for the Document
        ' Will be output to List of Documents sheet by WriteDocProperties( )
        If Not isMacroIssue(myIssue) Then
            issueClass = getDocIssueClassification(logWb, myIssue)
            CountDocIssuesForDoc issueClass, aAnalysis
            SetOverallDocIssueClassification issueClass, aAnalysis
            issueCost = getDocIssueCost(logWb, aAnalysis, myIssue)
            aAnalysis.DocIssuesCosts = aAnalysis.DocIssuesCosts + issueCost
            If myIssue.Preparable Then
                aAnalysis.PreparableIssuesCosts = aAnalysis.PreparableIssuesCosts + issueCost
            End If
        End If
                
        'Collate Issue and Factor counts across all Documents
        'Will be output to the Issues Analyzed sheet by WriteIssueCounts( )
        CollateIssueAndFactorCountsAcrossAllDocs aAnalysis, myIssue, fileName
        
        OutputCommonIssueDetails wsIssueDetails, rowIndex, aAnalysis, myIssue, fileName
        OutputCommonIssueAttributes wsIssueDetails, rowIndex, myIssue
        rowIndex = rowIndex + 1
        Set myIssue = Nothing
    Next index
        
    ProcessIssuesAndWriteDocIssueDetails = rowIndex
    
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function getDocIssueCost(logWb As WorkBook, aAnalysis As DocumentAnalysis, myIssue As IssueInfo) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getDocIssueCost"
    
    Dim issueKey As String
    Dim ret As Long
    ret = 0
    
    issueKey = getAppSpecificApplicationName & "_" & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML
    
    ret = getIssueValueFromXLSorDict(logWb, aAnalysis, mIssuesCostDict, issueKey, 1, CCOST_COL_OFFSET)
    
FinalExit:
    getDocIssueCost = ret
    Exit Function
    
HandleErrors:
    ret = 0
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Function getMacroIssueCosts(logWb As WorkBook, aAnalysis As DocumentAnalysis) As Long
    'Error handling not required
    getMacroIssueCosts = getVBAMacroIssueCost(logWb, aAnalysis) '+ getMacroExtRefIssueCost(logWb, aAnalysis)
    'NOTE: Currently not counting External Refs as Macro Cost
    'could be added if porting off Windows
    
End Function

Function getVBAMacroIssueCost(logWb As WorkBook, aAnalysis As DocumentAnalysis) As Long
    Const CMACRO_ROW_OFFSET_UNIQUE_LINES_COST = 4
    Const CMACRO_ROW_OFFSET_USER_FORMS_COUNT_COST = 5
    Const CMACRO_ROW_OFFSET_USER_FORMS_CONTROL_COUNT_COST = 6
    Const CMACRO_ROW_OFFSET_USER_FORMS_CONTROL_TYPE_COUNT_COST = 7
    
    Const CMACRO_NUM_OF_LINES_FACTOR_KEY = "_UniqueLineCount"
    Const CMACRO_USER_FORMS_COUNT_FACTOR_KEY = "_UserFormsCount"
    Const CMACRO_USER_FORMS_CONTROL_COUNT_FACTOR_KEY = "_UserFormsControlCount"
    Const CMACRO_USER_FORMS_CONTROL_TYPE_COUNT_FACTOR_KEY = "_UserFormsControlTypeCount"
    
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getVBAMacroIssueCost"
    
    Dim baseIssueKey As String
    Dim ret As Long
    ret = 0
    
    If Not aAnalysis.HasMacros Then GoTo FinalExit
        
    'Fetch VBA Macro Cost Factors - if required
    baseIssueKey = getAppSpecificApplicationName & "_" & CSTR_ISSUE_VBA_MACROS & "_" & CSTR_SUBISSUE_PROPERTIES
    
    'Num Lines - Costing taken from "Lines in Unique Modules"
    If aAnalysis.MacroTotalNumLines > 0 Then
        ret = ret + aAnalysis.MacroTotalNumLines * _
            getValueFromXLSorDict(logWb, aAnalysis, mIssuesCostDict, _
                baseIssueKey & CMACRO_NUM_OF_LINES_FACTOR_KEY, baseIssueKey, _
                CMACRO_ROW_OFFSET_UNIQUE_LINES_COST, CCOST_COL_OFFSET)
    End If
    'User Forms Count
    If aAnalysis.MacroNumUserForms > 0 Then
        ret = ret + aAnalysis.MacroNumUserForms * _
            getValueFromXLSorDict(logWb, aAnalysis, mIssuesCostDict, _
                baseIssueKey & CMACRO_USER_FORMS_COUNT_FACTOR_KEY, baseIssueKey, _
                CMACRO_ROW_OFFSET_USER_FORMS_COUNT_COST, CCOST_COL_OFFSET)
    End If
    'User Forms Control Count
    If aAnalysis.MacroNumUserFormControls > 0 Then
        ret = ret + aAnalysis.MacroNumUserFormControls * _
            getValueFromXLSorDict(logWb, aAnalysis, mIssuesCostDict, _
                baseIssueKey & CMACRO_USER_FORMS_CONTROL_COUNT_FACTOR_KEY, baseIssueKey, _
                CMACRO_ROW_OFFSET_USER_FORMS_CONTROL_COUNT_COST, CCOST_COL_OFFSET)
    End If
    'User Forms Control Type Count
    If aAnalysis.MacroNumUserFormControlTypes > 0 Then
        ret = ret + aAnalysis.MacroNumUserFormControlTypes * getValueFromXLSorDict(logWb, aAnalysis, mIssuesCostDict, _
        baseIssueKey & CMACRO_USER_FORMS_CONTROL_TYPE_COUNT_FACTOR_KEY, baseIssueKey, CMACRO_ROW_OFFSET_USER_FORMS_CONTROL_TYPE_COUNT_COST, CCOST_COL_OFFSET)
    End If
    

FinalExit:
    getVBAMacroIssueCost = ret
    Exit Function
    
HandleErrors:
    ret = 0
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Function getMacroExtRefIssueCost(logWb As WorkBook, aAnalysis As DocumentAnalysis) As Long
    Const CMACRO_ROW_OFFSET_NUM_EXTERNAL_REFS_COST = 2
    Const CMACRO_NUM_EXTERNAL_REFS_FACTOR_KEY = "_ExternalRefs"
    
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getMacroExtRefIssueCost"
    Dim baseIssueKey As String
    Dim ret As Long
    ret = 0
    
    If aAnalysis.MacroNumExternalRefs <= 0 Then GoTo FinalExit
        
    'Fetch External Ref Cost Factors
    baseIssueKey = getAppSpecificApplicationName & "_" & CSTR_ISSUE_PORTABILITY & "_" & _
        CSTR_SUBISSUE_EXTERNAL_REFERENCES_IN_MACRO
    ret = ret + aAnalysis.MacroNumExternalRefs * _
        getValueFromXLSorDict(logWb, aAnalysis, mIssuesCostDict, _
            baseIssueKey & CMACRO_NUM_EXTERNAL_REFS_FACTOR_KEY, baseIssueKey, _
            CMACRO_ROW_OFFSET_NUM_EXTERNAL_REFS_COST, CCOST_COL_OFFSET)

FinalExit:
    getMacroExtRefIssueCost = ret
    Exit Function
    
HandleErrors:
    ret = 0
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Function getIssueValueFromXLSorDict(logWb As WorkBook, aAnalysis As DocumentAnalysis, dict As Scripting.Dictionary, _
    key As String, row As Long, column As Long) As Long
    'Error handling not required
    getIssueValueFromXLSorDict = getValueFromXLSorDict(logWb, aAnalysis, dict, key, key, row, column)
End Function

Function getValueFromXLSorDict(logWb As WorkBook, aAnalysis As DocumentAnalysis, dict As Scripting.Dictionary, _
    dictKey As String, xlsKey As String, row As Long, column As Long) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getValueFromXLSorDict"
    
    Dim ret As Long
    ret = 0
    
    If dict.Exists(dictKey) Then
        ret = dict.item(dictKey)
    Else
        On Error Resume Next
        ret = logWb.Names(xlsKey).RefersToRange.Cells(row, column).value
        'Log as error missing key
        If Err.Number <> 0 Then
            WriteDebug currentFunctionName & _
            " : Issue Cost Key - " & xlsKey & ": label missing from results.xlt Costs sheet, check sheet and add/ check spelling label" & Err.Number & " " & Err.Description & " " & Err.Source
            WriteDebug currentFunctionName & " : dictKey " & dictKey & " : xlsKey " & xlsKey & " : " & Err.Number & " " & Err.Description & " " & Err.Source
            ret = 0
        End If
        On Error GoTo HandleErrors
        dict.Add dictKey, ret
    End If

FinalExit:
    getValueFromXLSorDict = ret
    Exit Function
    
HandleErrors:
    ret = 0
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Function isMacroIssue(myIssue As IssueInfo)
    'Error handling not required
    isMacroIssue = False
    
    If myIssue.IssueTypeXML = CSTR_ISSUE_VBA_MACROS Or _
        (myIssue.IssueTypeXML = CSTR_ISSUE_PORTABILITY And _
            myIssue.SubTypeXML = CSTR_SUBISSUE_EXTERNAL_REFERENCES_IN_MACRO) Then
        isMacroIssue = True
    End If
End Function
Sub CountDocIssuesForDoc(issueClass As EnumDocOverallIssueClass, aAnalysis As DocumentAnalysis)
    'Error handling not required
    
    If issueClass = enMinor Then
        aAnalysis.MinorIssuesCount = aAnalysis.MinorIssuesCount + 1
    End If
    ' Macro issues are counted during analysis
    ' Complex issues is calculated from: mIssues.count - mMinorIssuesCount - mMacroIssuesCount
End Sub
Sub SetOverallDocIssueClassification(issueClass As EnumDocOverallIssueClass, aAnalysis As DocumentAnalysis)
    'Error handling not required
    
    If aAnalysis.DocOverallIssueClass = enComplex Then Exit Sub
    
    If issueClass = enComplex Then
        aAnalysis.DocOverallIssueClass = enComplex
    Else
        aAnalysis.DocOverallIssueClass = enMinor
    End If
End Sub
Function getDocIssueClassification(logWb As WorkBook, myIssue As IssueInfo) As EnumDocOverallIssueClass
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getDocIssueClassification"
    Dim issueKey As String
    Dim bRet As Boolean
    bRet = False
    getDocIssueClassification = enMinor
    
    issueKey = getAppSpecificApplicationName & "_" & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML
    If mIssuesClassificationDict.Exists(issueKey) Then
        bRet = mIssuesClassificationDict.item(issueKey)
    Else
        On Error Resume Next
        bRet = logWb.Names(issueKey).RefersToRange.Cells(1, 0).value
        'Log as error missing key
        If Err.Number <> 0 Then
            WriteDebug currentFunctionName & _
            " : Issue Cost Key - " & issueKey & ": label missing from results.xlt Costs sheet, check sheet and add/ check spelling label" & Err.Number & " " & Err.Description & " " & Err.Source
            bRet = False
        End If
        On Error GoTo HandleErrors
        mIssuesClassificationDict.Add issueKey, bRet
    End If


FinalExit:
    If bRet Then
        getDocIssueClassification = enComplex
    End If
    Exit Function
    
HandleErrors:
    bRet = False
    WriteDebug currentFunctionName & " : issueKey " & issueKey & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function getDocOverallIssueClassificationAsString(docIssueClass As EnumDocOverallIssueClass) As String
    Dim Str As String
    'Error handling not required
    
    Select Case docIssueClass
    Case enComplex
        Str = RID_STR_COMMON_ISSUE_CLASS_COMPLEX
    Case enMinor
        Str = RID_STR_COMMON_ISSUE_CLASS_MINOR
    Case Else
        Str = RID_STR_COMMON_ISSUE_CLASS_NONE
    End Select
    
    getDocOverallIssueClassificationAsString = Str
End Function

Public Function getDocOverallMacroClassAsString(docMacroClass As EnumDocOverallMacroClass) As String
    Dim Str As String
    'Error handling not required
    
    Select Case docMacroClass
    Case enMacroComplex
        Str = RID_STR_COMMON_MACRO_CLASS_COMPLEX
    Case enMacroMedium
        Str = RID_STR_COMMON_MACRO_CLASS_MEDIUM
    Case enMacroSimple
        Str = RID_STR_COMMON_MACRO_CLASS_SIMPLE
    Case Else
        Str = RID_STR_COMMON_MACRO_CLASS_NONE
    End Select
    
    getDocOverallMacroClassAsString = Str
End Function

Function WriteDocRefDetails(wsRefDetails As Worksheet, DetailsRow As Long, _
    aAnalysis As DocumentAnalysis, fileName As String) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteDocRefDetails"
    
    Dim myIssue As IssueInfo
    Dim rowIndex As Long
    rowIndex = DetailsRow
        
    Dim index As Integer
    
    'Output References for Docs with Macros
    If aAnalysis.HasMacros And (aAnalysis.References.count > 0) Then
        For index = 1 To aAnalysis.References.count
            Set myIssue = aAnalysis.References(index)
            OutputReferenceAttributes wsRefDetails, rowIndex, aAnalysis, myIssue, fileName
            rowIndex = rowIndex + 1
            Set myIssue = Nothing
        Next index
    End If
    
    WriteDocRefDetails = rowIndex

FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : path " & aAnalysis.name & ": " & _
    " : row " & DetailsRow & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Sub OutputReferenceAttributes(wsIssueDetails As Worksheet, rowIndex As Long, _
    aAnalysis As DocumentAnalysis, myIssue As IssueInfo, fileName As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "OutputReferenceAttributes"
    
    Dim strAttributes As String
    
    With myIssue
        SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETDOCNAME, fileName
        SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETDOCAPPLICATION, aAnalysis.Application
                
        strAttributes = .Values(RID_STR_COMMON_ATTRIBUTE_MAJOR) & "." & .Values(RID_STR_COMMON_ATTRIBUTE_MINOR)
        strAttributes = IIf(strAttributes = "0.0" Or strAttributes = ".", .Values(RID_STR_COMMON_ATTRIBUTE_NAME), _
            .Values(RID_STR_COMMON_ATTRIBUTE_NAME) & " " & .Values(RID_STR_COMMON_ATTRIBUTE_MAJOR) & _
            "." & .Values(RID_STR_COMMON_ATTRIBUTE_MINOR))
        SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETREFERENCE, strAttributes
        
        If .Values(RID_STR_COMMON_ATTRIBUTE_TYPE) = RID_STR_COMMON_ATTRIBUTE_PROJECT Then
            SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETDESCRIPTION, RID_STR_COMMON_ATTRIBUTE_PROJECT
        Else
            SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETDESCRIPTION, _
                IIf(.Values(RID_STR_COMMON_ATTRIBUTE_DESCRIPTION) <> "", .Values(RID_STR_COMMON_ATTRIBUTE_DESCRIPTION), RID_STR_COMMON_NA)
        End If
        
        
        If .Values(RID_STR_COMMON_ATTRIBUTE_ISBROKEN) <> RID_STR_COMMON_ATTRIBUTE_BROKEN Then
            SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETLOCATION, _
                .Values(RID_STR_COMMON_ATTRIBUTE_FILE)
        Else
            SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETLOCATION, _
                RID_STR_COMMON_NA
        End If
         
        'Reference Details
        strAttributes = RID_STR_COMMON_ATTRIBUTE_TYPE & ": " & .Values(RID_STR_COMMON_ATTRIBUTE_TYPE) & vbLf
        strAttributes = strAttributes & RID_STR_COMMON_ATTRIBUTE_PROPERTIES & ": " & _
            .Values(RID_STR_COMMON_ATTRIBUTE_BUILTIN) & " " & .Values(RID_STR_COMMON_ATTRIBUTE_ISBROKEN)
        strAttributes = IIf(.Values(RID_STR_COMMON_ATTRIBUTE_GUID) <> "", _
            strAttributes & vbLf & RID_STR_COMMON_ATTRIBUTE_GUID & ": " & .Values(RID_STR_COMMON_ATTRIBUTE_GUID), _
            strAttributes)
        SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETATTRIBUTES, strAttributes
        
        SetWorksheetCellValueToString wsIssueDetails, rowIndex, CREF_DETNAMEANDPATH, aAnalysis.name
    End With
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : path " & aAnalysis.name & ": " & _
    " : rowIndex " & rowIndex & ": " & _
    " : myIssue " & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub OutputCommonIssueAttributes(wsIssueDetails As Worksheet, rowIndex As Long, _
    myIssue As IssueInfo)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "OutputCommonIssueAttributes"
    
    Dim index As Integer
    Dim strAttributes As String
        
    strAttributes = ""
    For index = 1 To myIssue.Attributes.count
        strAttributes = strAttributes & myIssue.Attributes(index) & " - " & _
                            myIssue.Values(index)
        strAttributes = strAttributes & IIf(index <> myIssue.Attributes.count, vbLf, "")
    
    Next index
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETATTRIBUTES, strAttributes

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : rowIndex " & rowIndex & ": " & _
    " : myIssue " & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
'Store issue cost and factor costs across all documents
Sub CollateIssueAndFactorCountsAcrossAllDocs(aAnalysis As DocumentAnalysis, myIssue As IssueInfo, fileName As String)
    Const CSTR_USER_FORM = "User Form"
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CollateIssueAndFactorCountsAcrossAllDocs"
    
    'Don't want to cost ISSUE_INFORMATION issues
    If myIssue.IssueTypeXML = CSTR_ISSUE_INFORMATION Then Exit Sub
    
    Dim issueKey As String
    issueKey = getAppSpecificApplicationName & "_" & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML
    
    'Store costing metrics for Issue
    AddIssueAndOneToDict issueKey
    
    'Store prepeared issue for costing metrics
    If myIssue.Preparable Then
        AddPreparedIssueAndOneToDict issueKey & "_Prepared"
    End If

    'Additional costing Factors output for VB macros
    If (myIssue.IssueTypeXML = CSTR_ISSUE_VBA_MACROS) And _
        (myIssue.SubTypeXML <> CSTR_SUBISSUE_MACRO_PASSWORD_PROTECTION) Then
        
        'Unique Macro Module and Line count
        AddMacroModuleHashToMacroDict myIssue
        
        'Line count
        AddIssueAndValToDict issueKey & "_" & CSTR_SUBISSUE_VBA_MACROS_NUMLINES, myIssue, _
            RID_STR_COMMON_ATTRIBUTE_NUMBER_OF_LINES
        
        'User From info
        If myIssue.SubLocation = CSTR_USER_FORM Then
            AddIssueAndOneToDict issueKey & "_" & CSTR_SUBISSUE_VBA_MACROS_USERFORMS_COUNT
            
            AddIssueAndValToDict issueKey & "_" & CSTR_SUBISSUE_VBA_MACROS_USERFORMS_CONTROL_COUNT, myIssue, _
               RID_STR_COMMON_ATTRIBUTE_CONTROLS
        End If
    'Additional costing Factors output for External References
    ElseIf (myIssue.IssueTypeXML = CSTR_ISSUE_PORTABILITY And _
            myIssue.SubTypeXML = CSTR_SUBISSUE_EXTERNAL_REFERENCES_IN_MACRO) Then
    
        AddIssueAndValToDict issueKey & "_" & CSTR_SUBISSUE_EXTERNAL_REFERENCES_IN_MACRO_COUNT, myIssue, _
            RID_STR_COMMON_ATTRIBUTE_NON_PORTABLE_EXTERNAL_REFERENCES_COUNT
    End If
    
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : path " & aAnalysis.name & ": " & _
    " : myIssue " & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub OutputCommonIssueDetails(wsIssueDetails As Worksheet, rowIndex As Long, _
    aAnalysis As DocumentAnalysis, myIssue As IssueInfo, fileName As String)
    Const CSTR_USER_FORM = "User Form"
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "OutputCommonIssueDetails"
    
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETDOCNAME, fileName
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETDOCAPPLICATION, aAnalysis.Application
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETTYPE, myIssue.IssueType
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETSUBTYPE, myIssue.SubType
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETLOCATION, myIssue.Location
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETSUBLOCATION, _
        IIf(myIssue.SubLocation = "", RID_STR_COMMON_NA, myIssue.SubLocation)
    SetWorksheetCellValueToVariant wsIssueDetails, rowIndex, CISSUE_DETLINE, _
        IIf(myIssue.Line = -1, RID_STR_COMMON_NA, myIssue.Line)
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETCOLUMN, _
        IIf(myIssue.column = "", RID_STR_COMMON_NA, myIssue.column)
    SetWorksheetCellValueToString wsIssueDetails, rowIndex, CISSUE_DETNAMEANDPATH, aAnalysis.name


FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : path " & aAnalysis.name & ": " & _
    " : rowIndex " & rowIndex & ": " & _
    " : myIssue " & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub AddIssueAndBoolValToDict(issueKey As String, issue As IssueInfo, valKey As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AddIssueAndBoolValToDict"
        
    If mIssuesDict.Exists(issueKey) Then
        mIssuesDict.item(issueKey) = mIssuesDict.item(issueKey) + _
            IIf(issue.Values(valKey) > 0, 1, 0)
    Else
        mIssuesDict.Add issueKey, IIf(issue.Values(valKey) > 0, 1, 0)
    End If
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : issueKey " & issueKey & ": " & _
    " : valKey " & valKey & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub AddIssueAndValToDict(issueKey As String, issue As IssueInfo, valKey As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AddIssueAndValToDict"
        
    If mIssuesDict.Exists(issueKey) Then
        mIssuesDict.item(issueKey) = mIssuesDict.item(issueKey) + issue.Values(valKey)
    Else
        mIssuesDict.Add issueKey, issue.Values(valKey)
    End If
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : issueKey " & issueKey & ": " & _
    " : valKey " & valKey & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub AddMacroModuleHashToMacroDict(issue As IssueInfo)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    Dim issueKey As String
    Dim issueVal As String
    currentFunctionName = "AddMacroModuleHashToMacroDict"
        
    issueKey = issue.Values(RID_STR_COMMON_ATTRIBUTE_SIGNATURE)
    If issueKey = RID_STR_COMMON_NA Then Exit Sub
    
    If Not mMacroDict.Exists(issueKey) Then
        mMacroDict.Add issueKey, issue.Values(RID_STR_COMMON_ATTRIBUTE_NUMBER_OF_LINES)
    End If
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : issueKey " & issueKey & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub AddIssueAndOneToDict(key As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AddIssueAndOneToDict"

    If mIssuesDict.Exists(key) Then
        mIssuesDict.item(key) = mIssuesDict.item(key) + 1
    Else
        mIssuesDict.Add key, 1
    End If
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : key " & key & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub AddPreparedIssueAndOneToDict(key As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AddPreparedIssueAndOneToDict"

    If mPreparedIssuesDict.Exists(key) Then
        mPreparedIssuesDict.item(key) = mPreparedIssuesDict.item(key) + 1
    Else
        mPreparedIssuesDict.Add key, 1
    End If
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : key " & key & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Function GetExcelInstance() As Excel.Application
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetExcelInstance"
    
    Dim xl As Excel.Application
    On Error Resume Next
    'Try and get an existing instance
    Set xl = GetObject(, "Excel.Application")
    If Err.Number = 429 Then
        Set xl = CreateObject("Excel.Application")
    ElseIf Err.Number <> 0 Then
        Set xl = Nothing
        MsgBox "Error: " & Err.Description
        Exit Function
    End If
    Set GetExcelInstance = xl
    Set xl = Nothing
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Sub WriteOverview(logWb As WorkBook, DocCount As DocumentCount, templateCount As DocumentCount, _
                  macroClasses As DocMacroClassifications, issueClasses As DocIssueClassifications)
    Const COV_ISSUECLASS_COMPLEX = "MAW_ISSUECLASS_COMPLEX"
    Const COV_ISSUECLASS_MINOR = "MAW_ISSUECLASS_MINOR"
    Const COV_ISSUECLASS_NONE = "MAW_ISSUECLASS_NONE"
    
    Const COV_MACROCLASS_COMPLEX = "MAW_MACROCLASS_COMPLEX"
    Const COV_MACROCLASS_MEDIUM = "MAW_MACROCLASS_MEDIUM"
    Const COV_MACROCLASS_SIMPLE = "MAW_MACROCLASS_SIMPLE"
    Const COV_MACROCLASS_NONE = "MAW_MACROCLASS_NONE"
    
    Const COV_ISSUECOUNT_COMPLEX = "MAW_ISSUECOUNT_COMPLEX"
    Const COV_ISSUECOUNT_MINOR = "MAW_ISSUECOUNT_MINOR"
    
    Const COV_MODDATES_LESS3MONTHS = "MAW_MODDATES_LESS3MONTHS"
    Const COV_MODDATES_3TO6MONTHS = "MAW_MODDATES_3TO6MONTHS"
    Const COV_MODDATES_6TO12MONTHS = "MAW_MODDATES_6TO12MONTHS"
    Const COV_MODDATES_MORE12MONTHS = "MAW_MODDATES_MORE12MONTHS"
    
    Const COV_DOC_MIGRATION_COSTS = "Document_Migration_Costs"
    Const COV_DOC_PREPARABLE_COSTS = "Document_Migration_Preparable_Costs"
    Const COV_MACRO_MIGRATION_COSTS = "Macro_Migration_Costs"
 
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteOverview"
    
    Dim appName As String
    appName = getAppSpecificApplicationName
  
    'OV - Title
    SetWorkbookNameValueToString logWb, COVERVIEW_TITLE_LABEL, GetTitle
    SetWorkbookNameValueToVariant logWb, "AnalysisDate", Now
    SetWorkbookNameValueToString logWb, "AnalysisVersion", _
        RID_STR_COMMON_OV_VERSION_STR & ": " & GetTitle & " " & GetVersion
              
    'OV - Number of Documents Analyzed
    AddLongToWorkbookNameValue logWb, CNUMBERDOC_ALL & getAppSpecificDocExt, DocCount.numDocsAnalyzed
    AddLongToWorkbookNameValue logWb, CNUMBERDOC_ALL & getAppSpecificTemplateExt, templateCount.numDocsAnalyzed
    
    'OV - Documents with Document Migration Issues (excludes macro issues)
    AddLongToWorkbookNameValue logWb, appName & "_" & COV_ISSUECLASS_COMPLEX, issueClasses.complex
    AddLongToWorkbookNameValue logWb, appName & "_" & COV_ISSUECLASS_MINOR, issueClasses.Minor
    AddLongToWorkbookNameValue logWb, appName & "_" & COV_ISSUECLASS_NONE, issueClasses.None
    
    'OV - Documents with Macro Migration Issues
    AddLongToWorkbookNameValue logWb, appName & "_" & COV_MACROCLASS_COMPLEX, macroClasses.complex
    AddLongToWorkbookNameValue logWb, appName & "_" & COV_MACROCLASS_MEDIUM, macroClasses.Medium
    AddLongToWorkbookNameValue logWb, appName & "_" & COV_MACROCLASS_SIMPLE, macroClasses.Simple
    AddLongToWorkbookNameValue logWb, appName & "_" & COV_MACROCLASS_NONE, macroClasses.None
    
    'OV - Document Modification Dates
    Dim modDates As DocModificationDates
    Call GetDocModificationDates(modDates)

    SetWorkbookNameValueToLong logWb, COV_MODDATES_LESS3MONTHS, modDates.lessThanThreemonths
    SetWorkbookNameValueToLong logWb, COV_MODDATES_3TO6MONTHS, modDates.threeToSixmonths
    SetWorkbookNameValueToLong logWb, COV_MODDATES_6TO12MONTHS, modDates.sixToTwelvemonths
    SetWorkbookNameValueToLong logWb, COV_MODDATES_MORE12MONTHS, modDates.greaterThanOneYear


    If InDocPreparation Then
        'OV - Document Migration Issues(excludes macro issues)
        AddLongToWorkbookNameValue logWb, appName & "_" & COV_ISSUECOUNT_COMPLEX, _
            DocCount.numComplexIssues + templateCount.numComplexIssues
        AddLongToWorkbookNameValue logWb, appName & "_" & COV_ISSUECOUNT_MINOR, _
            DocCount.numMinorIssues + templateCount.numMinorIssues
    
        'OV - Document Migration Costs
        AddLongToWorkbookNameValue logWb, appName & "_" & COV_DOC_MIGRATION_COSTS, _
            DocCount.totalDocIssuesCosts + templateCount.totalDocIssuesCosts
        
        'OV - Document Migration Preparable Costs
        AddLongToWorkbookNameValue logWb, COV_DOC_PREPARABLE_COSTS, _
            DocCount.totalPreparableIssuesCosts + templateCount.totalPreparableIssuesCosts
        
        'OV - Macro Migration Costs
        AddLongToWorkbookNameValue logWb, appName & "_" & COV_MACRO_MIGRATION_COSTS, _
            DocCount.totalMacroCosts + templateCount.totalMacroCosts
    End If
    
    'OV - Internal Attributes
    AddLongToWorkbookNameValue logWb, appName & "_" & "TotalDocsAnalysedWithIssues", _
        DocCount.numDocsAnalyzedWithIssues + templateCount.numDocsAnalyzedWithIssues

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : Problem writing overview: " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetupDAWResultsSpreadsheet(logWb As WorkBook, fontName As String, fontSize As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetupDAWResultsSpreadsheet"
    Dim bSetupRun As Boolean
    bSetupRun = CBool(GetWorkbookNameValueAsLong(logWb, COV_DAW_SETUP_SHEETS_RUN_LBL))
    
    If bSetupRun Then Exit Sub
    
    'Setup Text Boxes
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MOD_DATES_COMMENT_TXB, _
        RID_STR_COMMON_OV_DOC_MOD_DATES_COMMENT_TITLE, RID_STR_COMMON_OV_DOC_MOD_DATES_COMMENT_BODY, _
        CCOMMENTS_FONT_SIZE, fontName
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MOD_DATES_LEGEND_TXB, _
        RID_STR_COMMON_OV_LEGEND_TITLE, RID_STR_COMMON_OV_DOC_MOD_DATES_LEGEND_BODY, fontSize, fontName
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MACRO_COMMENT_TXB, _
        RID_STR_COMMON_OV_DOC_MACRO_COMMENT_TITLE, RID_STR_COMMON_OV_DOC_MACRO_COMMENT_BODY, _
        CCOMMENTS_FONT_SIZE, fontName
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MACRO_LEGEND_TXB, _
        RID_STR_COMMON_OV_LEGEND_TITLE, RID_STR_COMMON_OV_DOC_MACRO_LEGEND_BODY, fontSize, fontName
    Dim monthLimit As Long
    monthLimit = GetIssuesLimitInDays / CNUMDAYS_IN_MONTH
    SetWorkbookNameValueToString logWb, COV_HIGH_LEVEL_ANALYSIS_LBL, _
        IIf(monthLimit <> CMAX_LIMIT, _
            ReplaceTopicTokens(RID_STR_COMMON_OV_HIGH_LEVEL_ANALYSIS_DAW, CR_TOPIC, CStr(monthLimit)), _
            RID_STR_COMMON_OV_HIGH_LEVEL_ANALYSIS_PAW_NO_LIMIT)
            
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_ANALYSIS_COMMENT_TXB, _
        RID_STR_COMMON_OV_DOC_ANALYSIS_COMMENT_TITLE, RID_STR_COMMON_OV_DOC_ANALYSIS_COMMENT_BODY, _
        CCOMMENTS_FONT_SIZE, fontName
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_ANALYSIS_LEGEND_DAW_TXB, _
        RID_STR_COMMON_OV_LEGEND_TITLE, RID_STR_COMMON_OV_DOC_ANALYSIS_DAW_LEGEND_BODY, fontSize, fontName

    'Setup Chart Titles
    SetupSheetChartTitles logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MOD_DATES_CHART, _
        RID_STR_COMMON_OV_DOC_MOD_DATES_CHART_TITLE
    SetupSheetChartTitles logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MACRO_CHART, _
        RID_STR_COMMON_OV_DOC_MACRO_CHART_TITLE
    SetupSheetChartTitles logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_ANALYSIS_CHART, _
        RID_STR_COMMON_OV_DOC_ANALYSIS_CHART_TITLE
        
    'Set selection to top cell of Overview
    logWb.Sheets(RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW).Range("A1").Select
    
    bSetupRun = True
    SetWorkbookNameValueToBoolean logWb, COV_DAW_SETUP_SHEETS_RUN_LBL, bSetupRun
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : Problem setting up spreadsheet for DAW: " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetupPAWResultsSpreadsheet(logWb As WorkBook, fontName As String, fontSize As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetupPAWResultsSpreadsheet"
    Dim bSetupRun As Boolean
    bSetupRun = CBool(GetWorkbookNameValueAsLong(logWb, COV_PAW_SETUP_SHEETS_RUN_LBL))
    
    If bSetupRun Then Exit Sub
    
    'Costs
    logWb.Names(COV_COSTS_PREPISSUE_COUNT_COL_LBL).RefersToRange.EntireColumn.Hidden = False
    
    'Setup Text Boxes
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MOD_DATES_LEGEND_TXB, _
        RID_STR_COMMON_OV_LEGEND_TITLE, RID_STR_COMMON_OV_DOC_MOD_DATES_LEGEND_BODY, fontSize, fontName
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MACRO_LEGEND_TXB, _
        RID_STR_COMMON_OV_LEGEND_TITLE, RID_STR_COMMON_OV_DOC_MACRO_LEGEND_BODY, fontSize, fontName
    SetWorkbookNameValueToString logWb, COV_HIGH_LEVEL_ANALYSIS_LBL, _
        RID_STR_COMMON_OV_HIGH_LEVEL_ANALYSIS_PAW_NO_LIMIT
    SetupSheetTextBox logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_ANALYSIS_LEGEND_PAW_TXB, _
        RID_STR_COMMON_OV_LEGEND_TITLE, RID_STR_COMMON_OV_DOC_ANALYSIS_PAW_LEGEND_BODY, fontSize, fontName
    
    'Setup Chart Titles
    SetupSheetChartTitles logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MOD_DATES_CHART, _
        RID_STR_COMMON_OV_DOC_MOD_DATES_CHART_TITLE
    SetupSheetChartTitles logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_MACRO_CHART, _
        RID_STR_COMMON_OV_DOC_MACRO_CHART_TITLE
    SetupSheetChartTitles logWb, RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW, COV_DOC_ANALYSIS_CHART, _
        RID_STR_COMMON_OV_DOC_ANALYSIS_CHART_TITLE
        
    'Set selection to top cell of Overview
    logWb.Sheets(RID_STR_COMMON_RESULTS_SHEET_NAME_OVERVIEW).Range("A1").Select

    bSetupRun = True
    SetWorkbookNameValueToBoolean logWb, COV_PAW_SETUP_SHEETS_RUN_LBL, bSetupRun
        
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : Problem setting up spreadsheet for PAW: " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetupPrintRanges(logWb As WorkBook, docPropRow As Long, appIssuesRow As Long, issueDetailsRow As Long, _
    refDetailsRow As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetupPrintRanges"
    
    'Set Print Ranges
    If InDocPreparation Then
    
        logWb.Worksheets(RID_STR_COMMON_RESULTS_SHEET_NAME_DOCPROP).PageSetup.PrintArea = "$A1:$U" & (docPropRow + mDocPropRowOffset)
        logWb.Worksheets(RID_STR_COMMON_RESULTS_SHEET_NAME_DOCISSUE_DETAILS).PageSetup.PrintArea = "$A1:$J" & issueDetailsRow
        logWb.Worksheets(RID_STR_COMMON_RESULTS_SHEET_NAME_DOCREF_DETAILS).PageSetup.PrintArea = "$A1:$G" & refDetailsRow
        If getAppSpecificApplicationName = CAPPNAME_WORD Then
            logWb.Worksheets(RID_STR_COMMON_RESULTS_SHEET_NAME_DOCISSUES_WORD).PageSetup.PrintArea = _
                "$A1:$N" & appIssuesRow
        ElseIf getAppSpecificApplicationName = CAPPNAME_EXCEL Then
            logWb.Worksheets(RID_STR_COMMON_RESULTS_SHEET_NAME_DOCISSUES_EXCEL).PageSetup.PrintArea = _
                "$A1:$M" & appIssuesRow
        Else
            logWb.Worksheets(RID_STR_COMMON_RESULTS_SHEET_NAME_DOCISSUES_POWERPOINT).PageSetup.PrintArea = _
                "$A1:$K" & appIssuesRow
        End If
    Else
        logWb.Worksheets(RID_STR_COMMON_RESULTS_SHEET_NAME_DOCPROP).PageSetup.PrintArea = "$A1:$U" & (docPropRow + mDocPropRowOffset)
    End If
    
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : Problem setting print ranges: " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetupSheetChartTitles(logWb As WorkBook, namedWorksheet As String, namedChart As String, _
    chartTitle As String)
    Const CCHART_TITLE_FONT_SIZE = 11
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetupSheetChartTitles"
    
    With logWb.Sheets(namedWorksheet).ChartObjects(namedChart).Chart
        .HasTitle = True
        .chartTitle.Characters.Text = chartTitle
        .chartTitle.Font.Size = CCHART_TITLE_FONT_SIZE
    End With

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
        " namedWorkSheet: " & namedWorksheet & _
        " namedChart: " & namedChart & _
        " chartTitle: " & chartTitle & _
        Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetupSheetTextBox(logWb As WorkBook, namedWorksheet As String, _
    textBoxName As String, textBoxTitle As String, textBoxBody As String, _
    textSize As Long, fontName As String)

    Const CMAX_INSERTABLE_STRING_LEN = 255
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetupSheetTextBox"
    
    Dim strTextBody As String
    Dim allText As String
    strTextBody = ReplaceTopic2Tokens(textBoxBody, CR_STR, Chr(10), CR_PRODUCT, RID_STR_COMMON_OV_PRODUCT_STR)
    
    'Setup Text Boxes
    logWb.Sheets(namedWorksheet).Activate
    logWb.Sheets(namedWorksheet).Shapes(textBoxName).Select
    
    '*** Workaround Excel bug:  213841 XL: Passed Strings Longer Than 255 Characters Are Truncated
    Dim I As Long
    logWb.Application.Selection.Text = ""
        
    logWb.Application.Selection.Characters.Text = textBoxTitle & Chr(10)
         
    With logWb.Application.Selection
      For I = 0 To Int(Len(strTextBody) / CMAX_INSERTABLE_STRING_LEN)
        .Characters(.Characters.count + 1).Text = Mid(strTextBody, _
            (I * CMAX_INSERTABLE_STRING_LEN) + 1, CMAX_INSERTABLE_STRING_LEN)
      Next
    End With

    'Highlight title only
    With logWb.Application.Selection.Characters(start:=1, Length:=Len(textBoxTitle)).Font
        .name = fontName
        .FontStyle = "Bold"
        .Size = textSize
    End With
    With logWb.Application.Selection.Characters(start:=Len(textBoxTitle) + 1, _
        Length:=Len(strTextBody) + 1).Font
        .name = fontName
        .FontStyle = "Regular"
        .Size = textSize
    End With

FinalExit:
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & _
        " namedWorkSheet: " & namedWorksheet & _
        " textBoxName: " & textBoxName & _
        " textBoxTitle: " & textBoxTitle & _
        " textBoxBody: " & textBoxBody & _
        " textSize: " & textSize & _
        Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Function GetWorkbookNameValueAsLong(logWb As WorkBook, name As String) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetWorkbookNameValueAsLong"

    GetWorkbookNameValueAsLong = logWb.Names(name).RefersToRange.Cells(1, 1).value

FinalExit:
    Exit Function
    
HandleErrors:
    GetWorkbookNameValueAsLong = 0
    WriteDebug currentFunctionName & " : name " & name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function GetWorksheetCellValueAsLong(logWs As Worksheet, row As Long, col As Long) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetWorksheetCellValueAsLong"

    GetWorksheetCellValueAsLong = logWs.Cells(row, col).value

FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : row " & row & _
    " : col " & col & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
        
Function GetWorksheetCellValueAsString(logWs As Worksheet, row As Long, col As Long) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetWorksheetCellValueToString"

    GetWorksheetCellValueAsString = logWs.Cells(row, col).value

FinalExit:
    Exit Function
    
HandleErrors:
    GetWorksheetCellValueAsString = ""
    
    WriteDebug currentFunctionName & _
    " : row " & row & _
    " : col " & col & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
        
Sub SetWorksheetCellValueToLong(logWs As Worksheet, row As Long, col As Long, val As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetWorksheetCellValueToLong"

    logWs.Cells(row, col) = val

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : row " & row & _
    " : col " & col & _
    " : val " & val & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub SetWorksheetCellValueToInteger(logWs As Worksheet, row As Long, col As Long, intVal As Integer)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetWorksheetCellValueToInteger"

    logWs.Cells(row, col) = intVal

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : row " & row & _
    " : col " & col & _
    " : intVal " & intVal & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetWorksheetCellValueToVariant(logWs As Worksheet, row As Long, col As Long, varVal As Variant)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetWorksheetCellValueToInteger"

    logWs.Cells(row, col) = varVal

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : row " & row & _
    " : col " & col & _
    " : varVal " & varVal & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetWorksheetCellValueToString(logWs As Worksheet, row As Long, col As Long, strVal As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetWorksheetCellValueToString"

    logWs.Cells(row, col) = strVal

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : row " & row & _
    " : col " & col & _
    " : strVal " & strVal & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetWorkbookNameValueToBoolean(logWb As WorkBook, name As String, bVal As Boolean)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetWorkbookNameValueToBoolean"

    logWb.Names(name).RefersToRange.Cells(1, 1) = bVal

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : name " & name & " : boolean value " & bVal & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetWorkbookNameValueToString(logWb As WorkBook, name As String, val As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetWorkbookNameValueToString"

    logWb.Names(name).RefersToRange.Cells(1, 1) = val

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : name " & name & " : value " & val & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetWorkbookNameValueToLong(logWb As WorkBook, name As String, val As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetWorkbookNameValueToLong"

    logWb.Names(name).RefersToRange.Cells(1, 1) = val

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : name " & name & " : value " & val & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetWorkbookNameValueToVariant(logWb As WorkBook, name As String, val As Variant)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetWorkbookNameValueToVariant"

    logWb.Names(name).RefersToRange.Cells(1, 1) = val

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : name " & name & " : value " & val & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub AddLongToWorkbookNameValue(logWb As WorkBook, name As String, val As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AddLongToWorkbookNameValue"

    logWb.Names(name).RefersToRange.Cells(1, 1) = logWb.Names(name).RefersToRange.Cells(1, 1).value + val

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : name " & name & " : value " & val & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub AddVariantToWorkbookNameValue(logWb As WorkBook, name As String, varVal As Variant)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AddVariantToWorkbookNameValue"

    logWb.Names(name).RefersToRange.Cells(1, 1) = logWb.Names(name).RefersToRange.Cells(1, 1).value + varVal

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : name " & name & " : value " & varVal & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SaveAnalysisResultsVariables(logWb As WorkBook, offsetDocIssueDetailsRow As Long, _
    offsetDocRefDetailsRow As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SaveAnalysisResultsVariables"
    
    'OV - Internal Attributes
    SetWorkbookNameValueToLong logWb, "TotalIssuesAnalysed", offsetDocIssueDetailsRow
    SetWorkbookNameValueToLong logWb, "TotalRefsAnalysed", offsetDocRefDetailsRow
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : offsetDocIssueDetailsRow " & offsetDocIssueDetailsRow & _
    " : offsetDocRefDetailsRow " & offsetDocRefDetailsRow & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetupAnalysisResultsVariables(logWb As WorkBook, _
    offsetDocPropRow As Long, offsetDocIssuesRow As Long, _
    offsetDocIssueDetailsRow As Long, offsetDocRefDetailsRow As Long)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetupAnalysisResultsVariables"
    
    offsetDocPropRow = GetWorkbookNameValueAsLong(logWb, CTOTAL_DOCS_ANALYZED)
    offsetDocIssueDetailsRow = GetWorkbookNameValueAsLong(logWb, "TotalIssuesAnalysed")
    offsetDocRefDetailsRow = GetWorkbookNameValueAsLong(logWb, "TotalRefsAnalysed")
    offsetDocIssuesRow = GetWorkbookNameValueAsLong(logWb, getAppSpecificApplicationName & "_" & "TotalDocsAnalysedWithIssues")
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : offsetDocPropRow " & offsetDocPropRow & _
    " : offsetDocIssueDetailsRow " & offsetDocIssueDetailsRow & _
    " : offsetDocRefDetailsRow " & offsetDocRefDetailsRow & _
    " : offsetDocIssuesRow " & offsetDocIssuesRow & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteToIni(key As String, value As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteToIni"
    
    If mIniFilePath = "" Then Exit Sub

    Call WritePrivateProfileString("Analysis", key, value, mIniFilePath)
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : key " & key & " : value " & value & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteToLog(key As String, value As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteToLog"
    
    If mLogFilePath = "" Then Exit Sub
    
    Dim sSection As String
    sSection = getAppSpecificApplicationName
        
    Call WritePrivateProfileString(sSection, key, value, mLogFilePath)
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : key " & key & " : value " & value & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub WriteDebug(value As String)
    On Error Resume Next 'Ignore errors in our error writing routines - could get circular dependency otherwise
    Static ErrCount As Long
    
    If mLogFilePath = "" Then Exit Sub
    
    Dim sSection As String
    sSection = getAppSpecificApplicationName & "Debug"
       
    If mDebugLevel > 0 Then
        Call WritePrivateProfileString(sSection, "Doc" & mDocIndex & "_debug" & ErrCount, value, mLogFilePath)
        ErrCount = ErrCount + 1
    Else
        Debug.Print
    End If
End Sub
Sub WriteDebugLevelTwo(value As String)
    On Error Resume Next 'Ignore errors in our error writing routines - could get circular dependency otherwise
    Static ErrCountTwo As Long
    
    If mLogFilePath = "" Then Exit Sub
    
    Dim sSection As String
    sSection = getAppSpecificApplicationName & "Debug"
       
    If mDebugLevel > 1 Then
        Call WritePrivateProfileString(sSection, "Doc" & mDocIndex & "_debug" & ErrCountTwo, "Level2: " & value, mLogFilePath)
        ErrCountTwo = ErrCountTwo + 1
    Else
        Debug.Print
    End If
End Sub

Public Function ProfileLoadDict(dict As Scripting.Dictionary, _
                                lpSectionName As String, _
                                inifile As String) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "ProfileLoadDict"
    Dim success As Long
    Dim c As Long
    Dim nSize As Long
    Dim KeyData As String
    Dim lpKeyName As String
    Dim ret As String
    
    ret = Space$(2048)
    nSize = Len(ret)
    success = GetPrivateProfileString( _
     lpSectionName, vbNullString, "", ret, nSize, inifile)
    
    If success Then
         ret = Left$(ret, success)
       
          Do Until ret = ""
             lpKeyName = StripNulls(ret)
             KeyData = ProfileGetItem( _
                 lpSectionName, lpKeyName, "", inifile)
            dict.Add lpKeyName, KeyData
          Loop
    End If
    ProfileLoadDict = dict.count
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : dict.Count " & dict.count & _
    " : lpSectionName " & lpSectionName & _
    " : inifile " & inifile & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Private Function StripNulls(startStrg As String) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "StripNulls"
    Dim pos As Long
    Dim item As String
    
    pos = InStr(1, startStrg, Chr$(0))
    
    If pos Then
    
       item = Mid$(startStrg, 1, pos - 1)
       startStrg = Mid$(startStrg, pos + 1, Len(startStrg))
       StripNulls = item
     
    End If

FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : startStrg " & startStrg & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Public Function ProfileGetItem(lpSectionName As String, _
                               lpKeyName As String, _
                               defaultValue As String, _
                               inifile As String) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "ProfileGetItem"
    
    Dim success As Long
    Dim nSize As Long
    Dim ret As String
    ret = Space$(2048)
    nSize = Len(ret)
    success = GetPrivateProfileString(lpSectionName, _
                                      lpKeyName, _
                                      defaultValue, _
                                      ret, _
                                      nSize, _
                                      inifile)
    If success Then
       ProfileGetItem = Left$(ret, success)
    Else
       ProfileGetItem = defaultValue
    End If
    
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : lpSectionName " & lpSectionName & _
    " : lpKeyName " & lpKeyName & _
    " : defaultValue " & defaultValue & _
    " : inifile " & inifile & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Public Function GetDefaultPassword() As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetDefaultPassword"
    
    Static myPassword As String
    
    If myPassword = "" Then
        myPassword = ProfileGetItem("Analysis", CDEFAULT_PASSWORD, "", mIniFilePath)
    End If

    GetDefaultPassword = myPassword
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Public Function GetVersion() As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetVersion"
    
    Static myVersion As String

    If myVersion = "" Then
        myVersion = ProfileGetItem("Analysis", CVERSION, "", mIniFilePath)
    End If

    GetVersion = myVersion
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Public Function GetTitle() As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetTitle"
    
    Static myTitle As String

    If myTitle = "" Then
        myTitle = ProfileGetItem("Analysis", CTITLE, RID_STR_COMMON_ANALYSIS_STR, mIniFilePath)
    End If

    GetTitle = myTitle
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Sub SetPrepareToNone()
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetPrepareToNone"

    Call WritePrivateProfileString("Analysis", CDOPREPARE, CStr(0), mIniFilePath)

FinalExit:
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Function CheckForAbort() As Boolean
    Dim currentFunctionName As String
    Dim bAbort As Boolean

    currentFunctionName = "CheckForAbort"
    bAbort = False

    On Error GoTo HandleErrors

    bAbort = CBool(ProfileGetItem("Analysis", C_ABORT_ANALYSIS, "false", mIniFilePath))
    
    'reset the flag
    If (bAbort) Then Call WriteToIni(C_ABORT_ANALYSIS, "false")

FinalExit:
    CheckForAbort = bAbort
    Exit Function

HandleErrors:
    WriteDebug currentFunctionName & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function CheckDoPrepare() As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckDoPrepare"
    
    Static bDoPrepare As Boolean
    Static myDoPrepare As String

    If myDoPrepare = "" Then
        bDoPrepare = CBool(ProfileGetItem("Analysis", _
            CDOPREPARE, "False", mIniFilePath))
        myDoPrepare = "OK"
    End If

    CheckDoPrepare = bDoPrepare
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function GetIssuesLimitInDays() As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    
    currentFunctionName = "GetIssuesLimitInDays"
    
    Static issuesLimit As Long
    Static myDoPrepare As String

    If issuesLimit = 0 Then
        issuesLimit = CLng(ProfileGetItem("Analysis", _
            CISSUES_LIMIT, CMAX_LIMIT, mIniFilePath)) * CNUMDAYS_IN_MONTH
    End If

    GetIssuesLimitInDays = issuesLimit
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Public Sub AddIssueDetailsNote(myIssue As IssueInfo, noteNum As Long, noteStr As String, _
    Optional preStr As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "AddIssueDetailsNote"
    
    If IsMissing(preStr) Then
        preStr = RID_STR_COMMON_NOTE_PRE
    End If
    myIssue.Attributes.Add preStr & "[" & noteNum & "]"
    myIssue.Values.Add noteStr

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : noteNum " & noteNum & " : noteStr " & noteStr & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Public Sub SetupWizardVariables( _
    fileList As String, storeToDir As String, resultsFile As String, _
    logFile As String, resultsTemplate As String, bOverwriteFile As Boolean, _
    bNewResultsFile As Boolean, statFileName As String, debugLevel As Long, _
    outputType As String, singleFile As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetupWizardVariables"
    
    If mIniFilePath = "" Then
        mIniFilePath = GetAppDataFolder & "\Sun\AnalysisWizard\" & CWIZARD & ".ini"
    End If
    
    statFileName = ProfileGetItem("Analysis", CSTAT_FILE, "", mIniFilePath)
    fileList = ProfileGetItem("Analysis", CFILE_LIST, "", mIniFilePath)
    storeToDir = ProfileGetItem("Analysis", COUTPUT_DIR, "", mIniFilePath)
    resultsFile = ProfileGetItem("Analysis", CRESULTS_FILE, "", mIniFilePath)
    logFile = ProfileGetItem("Analysis", CLOG_FILE, "", mIniFilePath)
    resultsTemplate = ProfileGetItem("Analysis", CRESULTS_TEMPLATE, "", mIniFilePath)
    bOverwriteFile = IIf(ProfileGetItem("Analysis", CRESULTS_EXIST, COVERWRITE_FILE, mIniFilePath) = COVERWRITE_FILE, _
        True, False)
    bNewResultsFile = CBool(ProfileGetItem("Analysis", CNEW_RESULTS_FILE, "True", mIniFilePath))
    debugLevel = CLng(ProfileGetItem("Analysis", CDEBUG_LEVEL, "1", mIniFilePath))
    outputType = ProfileGetItem("Analysis", COUTPUT_TYPE, COUTPUT_TYPE_XLS, mIniFilePath)
    singleFile = ProfileGetItem("Analysis", CSINGLE_FILE, "", mIniFilePath)
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & _
        ": mIniFilePath " & mIniFilePath & ": " & _
        Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Public Sub SetupSearchTypes(searchTypes As Collection)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetupSearchTypes"
    
    Dim bDocument As Boolean
    Dim bTemplate As Boolean

    bDocument = CBool(ProfileGetItem("Analysis", LCase("type" & getAppSpecificApplicationName & "doc"), "False", mIniFilePath))
    bTemplate = CBool(ProfileGetItem("Analysis", LCase("type" & getAppSpecificApplicationName & "dot"), "False", mIniFilePath))
    If bDocument = True Then searchTypes.Add "*" & getAppSpecificDocExt
    If bTemplate = True Then searchTypes.Add "*" & getAppSpecificTemplateExt
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & ": searchTypes.Count " & searchTypes.count & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteXMLHeader(out As TextStream)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXMLHeader"
    
    out.WriteLine "<?xml version=""1.0"" encoding=""ISO-8859-1""?>"
    out.WriteLine "<!DOCTYPE results SYSTEM 'analysis.dtd'>"

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub WriteXMLResultsStartTag(out As TextStream)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXMLResultsStartTag"
    
    out.WriteLine "<results generated-by=""" & IIf(InDocPreparation, "documentanalysis_preparation", "documentanalysis") & """"
    out.WriteLine " version=""" & GetVersion & """ timestamp=""" & Now & """"
    out.WriteLine " type=""" & getAppSpecificApplicationName & """ >"

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Sub WriteXMLResultsEndTag(out As TextStream)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXMLResultsEndTag"
    
    out.WriteLine "</results>"

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteXMLDocProperties(out As TextStream, aAnalysis As DocumentAnalysis)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXMLDocProperties"
    
    out.WriteLine "<document location=""" & EncodeXML(aAnalysis.name) & """"
    out.WriteLine " application=""" & aAnalysis.Application & """"
    out.WriteLine " issues-count=""" & (aAnalysis.IssuesCount) & """"
    out.WriteLine " pages=""" & aAnalysis.PageCount & """"
    out.WriteLine " created=""" & CheckDate(aAnalysis.Created) & """"
    out.WriteLine " modified=""" & CheckDate(aAnalysis.Modified) & """"
    out.WriteLine " accessed=""" & CheckDate(aAnalysis.Accessed) & """"
    out.WriteLine " printed=""" & CheckDate(aAnalysis.Printed) & """"
    out.WriteLine " last-save-by=""" & aAnalysis.SavedBy & """"
    out.WriteLine " revision=""" & aAnalysis.Revision & """"
    out.WriteLine " based-on-template=""" & EncodeXML(aAnalysis.Template) & """"
    out.WriteLine ">"

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteXMLDocPropertiesEndTag(out As TextStream)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXMLDocPropertiesEndTag"
    
    out.WriteLine "</document>"

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteXMLDocRefDetails(out As TextStream, aAnalysis As DocumentAnalysis)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXMLDocRefDetails"
    Dim myIssue As IssueInfo
    
    'Output References for Docs with Macros
    If aAnalysis.HasMacros And (aAnalysis.References.count > 0) Then
        out.WriteLine "<references>"
        For Each myIssue In aAnalysis.References
            OutputXMLReferenceAttributes out, aAnalysis, myIssue
        Next myIssue
        out.WriteLine "</references>"
    End If
    
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub OutputXMLReferenceAttributes(out As TextStream, aAnalysis As DocumentAnalysis, myIssue As IssueInfo)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "OutputXMLReferenceAttributes"
    Dim strAttributes As String
    
    With myIssue
        out.WriteLine "<reference"
        
        strAttributes = .Values("Major") & "." & .Values("Minor")
        strAttributes = IIf(strAttributes = "0.0" Or strAttributes = ".", .Values("Name"), _
            .Values("Name") & " " & .Values("Major") & "." & .Values("Minor"))
        out.WriteLine " name=""" & EncodeXML(strAttributes) & """"
        
        If .Values("Type") = "Project" Then
            strAttributes = "Project reference"
        Else
            strAttributes = IIf(.Values("Description") <> "", .Values("Description"), RID_STR_COMMON_NA)
        End If
        out.WriteLine " description=""" & EncodeXML(strAttributes) & """"
        If .Values("IsBroken") <> RID_STR_COMMON_ATTRIBUTE_BROKEN Then
            out.WriteLine " location=""" & .Values("File") & """"
        End If
        out.WriteLine " type=""" & .Values("Type") & """"
        strAttributes = IIf(.Values("GUID") <> "", .Values("GUID"), RID_STR_COMMON_NA)
        out.WriteLine " GUID=""" & strAttributes & """"
        out.WriteLine " is-broken=""" & .Values("IsBroken") & """"
        out.WriteLine " builtin=""" & .Values("BuiltIn") & """"
        
        out.WriteLine " />"
    End With

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & " : myIssue " & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteXMLDocIssueDetails(out As TextStream, aAnalysis As DocumentAnalysis)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXMLDocIssueDetails"
    
    Dim myIssue As IssueInfo
        
    If aAnalysis.Issues.count = 0 Then Exit Sub
    
    out.WriteLine "<issues>"
    For Each myIssue In aAnalysis.Issues
        OutputXMLCommonIssueDetails out, aAnalysis, myIssue
        OutputXMLCommonIssueAttributes out, myIssue
        out.WriteLine "</issue>"
    Next myIssue
    out.WriteLine "</issues>"
        
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub OutputXMLCommonIssueDetails(out As TextStream, aAnalysis As DocumentAnalysis, myIssue As IssueInfo)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "OutputXMLCommonIssueDetails"
    
    out.WriteLine "<issue category=""" & myIssue.IssueTypeXML & """"
    out.WriteLine " type=""" & myIssue.SubTypeXML & """"
    
    'NOTE: Dropping severity - now stored in results.xlt, do not want to open it to fetch this data
    'out.WriteLine " severity=""" & IIf(CheckForMinorIssue(aAnalysis, myIssue), "Minor", "Major") & """"
    out.WriteLine " prepared=""" & IIf((myIssue.Preparable), "True", "False") & """ >"
    
    out.WriteLine "<location type=""" & myIssue.locationXML & """ >"
    
    If myIssue.SubLocation <> "" Then
        out.WriteLine "<property name=""sublocation"" value=""" & myIssue.SubLocation & """ />"
    End If
    If myIssue.Line <> -1 Then
        out.WriteLine "<property name=""line"" value=""" & myIssue.Line & """ />"
    End If
    If myIssue.column <> "" Then
        out.WriteLine "<property name=""column"" value=""" & myIssue.column & """ />"
    End If
    out.WriteLine "</location>"
    
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & aAnalysis.name & " : myIssue " & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub OutputXMLCommonIssueAttributes(out As TextStream, myIssue As IssueInfo)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "OutputXMLCommonIssueAttributes"
    
    Dim index As Integer
    Dim valStr As String
    Dim attStr As String
    
    If myIssue.Attributes.count = 0 Then Exit Sub
    
    out.WriteLine "<details>"
    For index = 1 To myIssue.Attributes.count
        attStr = myIssue.Attributes(index)
        If InStr(attStr, RID_STR_COMMON_NOTE_PRE & "[") = 1 Then
            attStr = Right$(attStr, Len(attStr) - Len(RID_STR_COMMON_NOTE_PRE & "["))
            attStr = Left$(attStr, Len(attStr) - 1)
            out.WriteLine "<note index=""" & attStr & """ value=""" & EncodeXML(myIssue.Values(index)) & """ />"
        Else
            out.WriteLine "<property name=""" & EncodeXML(myIssue.Attributes(index)) & """ value=""" & EncodeXML(myIssue.Values(index)) & """ />"
        End If
    Next index
        
    out.WriteLine "</details>"
    
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : myIssue " & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub


Sub WriteXMLOutput(storeToDir As String, resultsFile As String, _
    bOverwriteResultsFile As Boolean, bNewResultsFile As Boolean, AnalysedDocs As Collection, _
    fso As Scripting.FileSystemObject)
    
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteXMLOutput"
    
    Dim xmlOutput As TextStream
    Dim xmlOrigOutput As TextStream
    Dim origOutput As String
    Dim analysis As DocumentAnalysis
    Dim outFilePath As String
    
    outFilePath = storeToDir & "\" & fso.GetBaseName(resultsFile) & "_" & _
        getAppSpecificApplicationName & ".xml"

    Set xmlOutput = fso.CreateTextFile(outFilePath, True)
    WriteXMLHeader xmlOutput
        
    'Set xmlOrigOutput = fso.OpenTextFile(outFilePath, ForReading)
    'Set xmlOutput = fso.OpenTextFile(outFilePath, ForWriting)
    
    WriteXMLResultsStartTag xmlOutput
    For Each analysis In AnalysedDocs
        WriteXMLDocProperties xmlOutput, analysis
        WriteXMLDocRefDetails xmlOutput, analysis
        WriteXMLDocIssueDetails xmlOutput, analysis
        WriteXMLDocPropertiesEndTag xmlOutput
    Next analysis
    WriteXMLResultsEndTag xmlOutput
    
FinalExit:
    xmlOutput.Close
    Set xmlOutput = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : path " & outFilePath & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Private Function EncodeUrl(ByVal sUrl As String) As String
    Const MAX_PATH                   As Long = 260
    Const ERROR_SUCCESS              As Long = 0
    Const URL_DONT_SIMPLIFY          As Long = &H8000000
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "EncodeUrl"
    
    Dim sUrlEsc As String
    Dim dwSize As Long
    Dim dwFlags As Long
    
    If Len(sUrl) > 0 Then
       
       sUrlEsc = Space$(MAX_PATH)
       dwSize = Len(sUrlEsc)
       dwFlags = URL_DONT_SIMPLIFY
       
       If UrlEscape(sUrl, _
                    sUrlEsc, _
                    dwSize, _
                    dwFlags) = ERROR_SUCCESS Then
                    
          EncodeUrl = Left$(sUrlEsc, dwSize)
       
       End If  'If UrlEscape
    End If 'If Len(sUrl) > 0

FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : sUrl " & sUrl & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Private Function EncodeXML(Str As String) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "EncodeXML"
    
    Str = Replace(Str, "^", "&#x5E;")
    Str = Replace(Str, "&", "&amp;")
    Str = Replace(Str, "`", "&apos;")
    Str = Replace(Str, "{", "&#x7B;")
    Str = Replace(Str, "}", "&#x7D;")
    Str = Replace(Str, "|", "&#x7C;")
    Str = Replace(Str, "]", "&#x5D;")
    Str = Replace(Str, "[", "&#x5B;")
    Str = Replace(Str, """", "&quot;")
    Str = Replace(Str, "<", "&lt;")
    Str = Replace(Str, ">", "&gt;")
    
    'str = Replace(str, "\", "&#x5C;")
    'str = Replace(str, "#", "&#x23;")
    'str = Replace(str, "?", "&#x3F;")
    'str = Replace(str, "/", "&#x2F;")

    EncodeXML = Str
    
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : string " & Str & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function


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
            sTmp = Left(sTmp, p - 1) + sReplacement + Mid(sTmp, p + Len(sToken))
        End If
    Loop While p > 0
    
    
    ReplaceTopicTokens = sTmp
  
End Function

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

'Language setting functions
Function GetResourceDataFileName(thisDir As String) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetResourceDataFileName"
    
    Dim fso As FileSystemObject
    Set fso = New FileSystemObject

    'A debug method - if a file called debug.dat exists load it.
    If fso.FileExists(fso.GetAbsolutePathName(thisDir & "\debug.dat")) Then
        GetResourceDataFileName = fso.GetAbsolutePathName(thisDir & "\debug.dat")
        GoTo FinalExit
    End If

    Dim isoLangStr As String
    Dim isoCountryStr As String
    Dim langDir As String
            
    langDir = thisDir & "\" & "lang"
    
    Dim userLCID As Long
    userLCID = GetUserDefaultLangID()
    Dim sysLCID As Long
    sysLCID = GetSystemDefaultLangID()

    isoLangStr = GetUserLocaleInfo(userLCID, LOCALE_SISO639LANGNAME)
    isoCountryStr = GetUserLocaleInfo(userLCID, LOCALE_SISO3166CTRYNAME)
    
    'check for locale data in following order:
    '  user language
    '   isoLangStr & "_" & isoCountryStr & ".dat"
    '   isoLangStr & ".dat"
    '  system language
    '   isoLangStr & "_" & isoCountryStr & ".dat"
    '   isoLangStr & ".dat"
    '   "en_US" & ".dat"
    
    If fso.FileExists(fso.GetAbsolutePathName(langDir & "\" & isoLangStr & "-" & isoCountryStr & ".dat")) Then
        GetResourceDataFileName = fso.GetAbsolutePathName(langDir & "\" & isoLangStr & "-" & isoCountryStr & ".dat")
    ElseIf fso.FileExists(fso.GetAbsolutePathName(langDir & "\" & isoLangStr & ".dat")) Then
        GetResourceDataFileName = fso.GetAbsolutePathName(langDir & "\" & isoLangStr & ".dat")
    Else
        isoLangStr = GetUserLocaleInfo(sysLCID, LOCALE_SISO639LANGNAME)
        isoCountryStr = GetUserLocaleInfo(sysLCID, LOCALE_SISO3166CTRYNAME)
    
        If fso.FileExists(fso.GetAbsolutePathName(langDir & "\" & isoLangStr & "-" & isoCountryStr & ".dat")) Then
            GetResourceDataFileName = fso.GetAbsolutePathName(langDir & "\" & isoLangStr & "-" & isoCountryStr & ".dat")
        ElseIf fso.FileExists(fso.GetAbsolutePathName(langDir & "\" & isoLangStr & ".dat")) Then
            GetResourceDataFileName = fso.GetAbsolutePathName(langDir & "\" & isoLangStr & ".dat")
        Else
            GetResourceDataFileName = fso.GetAbsolutePathName(langDir & "\" & "en-US.dat")
        End If
    End If
FinalExit:
    Set fso = Nothing
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Public Function GetUserLocaleInfo(ByVal dwLocaleID As Long, ByVal dwLCType As Long) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetUserLocaleInfo"
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
    
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
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

Sub WriteToStatFile(statFileName As String, statValue As Integer, _
                    currDocument As String, fso As Scripting.FileSystemObject)

    On Error Resume Next
    
    Dim fileCont As TextStream
    
    Set fileCont = fso.OpenTextFile(statFileName, ForWriting, True, TristateTrue)
    If (statValue = C_STAT_STARTING) Then
        fileCont.WriteLine ("analysing=" & currDocument)
    ElseIf (statValue = C_STAT_DONE) Then
        fileCont.WriteLine ("analysed=" & currDocument)
    ElseIf (statValue = C_STAT_FINISHED) Then
        fileCont.WriteLine ("finished")
    End If

    fileCont.Close
End Sub

' The function FindIndex looks for a document in the given document list
' starting at the position lastIndex in that list. If the document could
' not be found, the function starts searching from the beginning

Function FindIndex(myDocument As String, _
                   myDocList As Collection, _
                   lastIndex As Long) As Long

    Dim lastEntry As Long
    Dim curIndex As Long
    Dim curEntry As String
    Dim entryFound As Boolean
    
    entryFound = False
    lastEntry = myDocList.count
        
    If (lastIndex > lastEntry) Then lastIndex = lastEntry
    
    If (lastIndex > 1) Then
        curIndex = lastIndex
    Else
        curIndex = 1
    End If

    While Not entryFound And curIndex <= lastEntry
        curEntry = myDocList.item(curIndex)
        If (curEntry = myDocument) Then
            entryFound = True
        Else
            curIndex = curIndex + 1
        End If
    Wend

    If (Not entryFound) Then
        curIndex = 1
        While Not entryFound And curIndex < lastIndex
            curEntry = myDocList.item(curIndex)
            If (curEntry = myDocument) Then
                entryFound = True
            Else
                curIndex = curIndex + 1
            End If
        Wend
    End If
    
    If entryFound Then
        FindIndex = curIndex
    Else
        FindIndex = 0
    End If

End Function

' The sub GetIndexValues calulates the start index of the analysis and the index
' of the file after which the next intermediate reult will be written
Function GetIndexValues(startIndex As Long, nextCheck As Long, _
                        myFiles As Collection) As Boolean

    Dim lastCheckpoint As String
    Dim nextFile As String
    Dim newResultsFile As Boolean

    lastCheckpoint = ProfileGetItem(C_ANALYSIS, C_LAST_CHECKPOINT, "", mIniFilePath)
    nextFile = ProfileGetItem(C_ANALYSIS, C_NEXT_FILE, "", mIniFilePath)
    newResultsFile = True

    If (nextFile = "") Then
        ' No Analysis done yet
        startIndex = 1
        nextCheck = C_MAX_CHECK
    Else
        If (lastCheckpoint = "") Then
            startIndex = 1
        Else
            startIndex = FindIndex(lastCheckpoint, myFiles, 1) + 1
            If (startIndex > 0) Then newResultsFile = False
        End If

        nextCheck = FindIndex(nextFile, myFiles, startIndex - 1)
        
        If (nextCheck = 0) Then   ' Next file not in file list, restarting
            startIndex = 1
            nextCheck = C_MAX_CHECK
            newResultsFile = True
        ElseIf (nextCheck < startIndex) Then  'we are done?
            nextCheck = startIndex + C_MAX_CHECK
        ElseIf (nextCheck = startIndex) Then 'skip this one
            WriteToLog C_ERROR_HANDLING_DOC & nextCheck, nextFile
            startIndex = startIndex + 1
            nextCheck = startIndex + C_MAX_CHECK
        Else 'last time an error occured with that file, write before analysing
            nextCheck = nextCheck - 1
        End If
    End If
    GetIndexValues = newResultsFile
End Function

Private Sub GetDocModificationDates(docCounts As DocModificationDates)

    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetDocModificationDates"

    docCounts.lessThanThreemonths = CLng(ProfileGetItem("Analysis", C_DOCS_LESS_3_MONTH, "0", mIniFilePath))
    docCounts.threeToSixmonths = CLng(ProfileGetItem("Analysis", C_DOCS_LESS_6_MONTH, "0", mIniFilePath))
    docCounts.sixToTwelvemonths = CLng(ProfileGetItem("Analysis", C_DOCS_LESS_12_MONTH, "0", mIniFilePath))
    docCounts.greaterThanOneYear = CLng(ProfileGetItem("Analysis", C_DOCS_MORE_12_MONTH, "0", mIniFilePath))

FinalExit:
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
