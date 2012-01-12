Attribute VB_Name = "ApplicationSpecific"
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

'*** Do NOT add any new categories - use those listed below or else the results spreadsheet and
'*** issues list will be out of sync
Public Const GlbPowerPoint = True

'** Issue Categories
Public Const CID_INFORMATION_REFS = 0
Public Const CID_ACTION_SETTINGS = 1
Public Const CID_CONTENT_AND_DOCUMENT_PROPERTIES = 2
Public Const CID_FIELDS = 3
Public Const CID_FORMAT = 4
Public Const CID_NOTES_AND_HANDOUTS = 5
Public Const CID_OBJECTS_GRAPHICS_TEXTBOXES = 6
Public Const CID_PORTABILITY = 7
Public Const CID_VBA_MACROS = 8
Public Const CTOTAL_CATEGORIES = 8

'** PP - XML Issue and SubIssue strings
Public Const CSTR_ISSUE_OBJECTS_GRAPHICS_AND_TEXTBOXES = "ObjectsGraphicsAndTextboxes"

Public Const CSTR_SUBISSUE_COMMENT = "Comment"
Public Const CSTR_SUBISSUE_MOVIE = "Movie"
Public Const CSTR_SUBISSUE_BACKGROUND = "Background"
Public Const CSTR_SUBISSUE_NUMBERING = "Numbering"
Public Const CSTR_SUBISSUE_HYPERLINK = "Hyperlink"
Public Const CSTR_SUBISSUE_HYPERLINK_SPLIT = "HyperlinkSplit"
Public Const CSTR_SUBISSUE_TEMPLATE = "Template"
Public Const CSTR_SUBISSUE_TABSTOP = "Tabstop"
Public Const CSTR_SUBISSUE_FONTS = "Fonts"

'** END PP - XML Issue and SubIssue strings

Public Const CAPPNAME_WORD = "Word"
'Public Const CAPPNAME_WORD_DOC = ".doc"
'Public Const CAPPNAME_WORD_DOT = ".dot"

Public Const CAPPNAME_EXCEL = "Excel"
'Public Const CAPPNAME_EXCEL_DOC = ".xls"
'Public Const CAPPNAME_EXCEL_DOT = ".xlt"

Public Const CAPPNAME_POWERPOINT = "PowerPoint"
Public Const CAPPNAME_PP_DOC = ".ppt"
Public Const CAPPNAME_PP_DOT = ".pot"

'Public Const CAPP_DOCPROP_LOCATION = "Document"
'Public Const CAPP_DOCPROP_LOCATION = "Workbook"
Public CAPP_DOCPROP_LOCATION As String


'Public Const CAPP_XMLDOCPROP_LOCATION = "Document"
'Public Const CAPP_XMLDOCPROP_LOCATION = "Workbook"
Public Const CAPP_XMLDOCPROP_LOCATION = "Presentation"

Public Const CTHIS_DOCUMENT = "ThisDocument"
Public Const CTOPLEVEL_PROJECT = "Project"

Const CSTART_DIR = 1
Const CSTORE_TO_DIR = 2
Const CRESULTS_FILE = 3
Const CRESULTS_TEMPALTE = 4
Const COVERWRITE_FILE = 5
Const CNEW_RESULTS_FILE = 6
Const CDOCUMENT = 7
Const CTEMPLATE = 8
Const CINCLUDE_SUBDIRS = 9
Const CLOG_FILE = 10
Const CDEBUG_LEVEL = 11

Public Function getAppSpecificDocExt() As String
    getAppSpecificDocExt = CAPPNAME_PP_DOC
End Function
Public Function getAppSpecificTemplateExt() As String
    getAppSpecificTemplateExt = CAPPNAME_PP_DOT
End Function

Public Function getAppSpecificPath() As String
    getAppSpecificPath = ActivePresentation.path
End Function
Public Function getAppSpecificApplicationName() As String
    getAppSpecificApplicationName = CAPPNAME_POWERPOINT
End Function

Public Function getAppSpecificCustomDocProperties(currDoc As Presentation) As DocumentProperties
    Set getAppSpecificCustomDocProperties = currDoc.CustomDocumentProperties
End Function
Public Function getAppSpecificCommentBuiltInDocProperty(currDoc As Presentation) As DocumentProperty
    Set getAppSpecificCommentBuiltInDocProperty = currDoc.BuiltInDocumentProperties("Comments")
End Function

Public Function getAppSpecificVBProject(currDoc as Presentation) As VBProject
    Set getAppSpecificVBProject = currDoc.VBProject
End Function

Public Function getAppSpecificOLEClassType(aShape As Shape) As String
    getAppSpecificOLEClassType = aShape.OLEFormat.ProgID
End Function

' Workaround as it does not seem to be possible to shut down PP
' from VB app
Public Sub QuitPowerPoint()
    Dim I As Integer
    On Error Resume Next
    With Application.Presentations
    For I = .count To 1 Step -1
        .item(I).Close
    Next
    End With
    Application.Quit
End Sub

Public Sub SetAppToMinimized()
    Application.WindowState = ppWindowMinimized
End Sub

Public Sub LocalizeResources()
    On Error GoTo ErrorHandler
    Dim ppStrings As StringDataManager
    Set ppStrings = New StringDataManager
    Dim aPres As Presentation
    Set aPres = Presentations("_OOoDocAnalysisPPTDriver.ppt")
    ppStrings.InitStringData (GetResourceDataFileName(aPres.path))
    LoadCommonStrings ppStrings
    LoadPPStrings ppStrings
    LoadResultsStrings ppStrings
    Set ppStrings = Nothing
    
    SetPPDriverText
FinalExit:
    Exit Sub
ErrorHandler:
    WriteDebug "Presentation_Open : " & Err.Number & " : " & Err.Description
    GoTo FinalExit
End Sub

Sub SetPPDriverText()
    On Error Resume Next
    CAPP_DOCPROP_LOCATION = RID_STR_COMMON_RESULTS_LOCATION_TYPE_PRESENTATION
    ActivePresentation.Slides.item(1).Shapes.item("RID_STR_DVR_PP_TXT2").OLEFormat.Object.Text = RID_STR_DVR_PP_TXT2
    ActivePresentation.Slides.item(1).Shapes.item("RID_STR_DVR_PP_TXT3").OLEFormat.Object.Text = RID_STR_DVR_PP_TXT3
    ActivePresentation.Slides.item(1).Shapes.item("RID_STR_DVR_PP_TXT4").OLEFormat.Object.Text = RID_STR_DVR_PP_TXT4
    ActivePresentation.Slides.item(1).Shapes.item("RID_STR_DVR_PP_TXT5").OLEFormat.Object.Text = RID_STR_DVR_PP_TXT5
    ActivePresentation.Slides.item(1).Shapes.item("RID_STR_DVR_PP_TXT6").OLEFormat.Object.Text = RID_STR_DVR_PP_TXT6
    ActivePresentation.Slides.item(1).Shapes.item("RID_STR_DVR_PP_TXT7").OLEFormat.Object.Text = RID_STR_DVR_PP_TXT7
    ActivePresentation.Slides.item(1).Shapes.item("RID_STR_DVR_PP_TXT8").OLEFormat.Object.Text = RID_STR_DVR_PP_TXT8
End Sub


