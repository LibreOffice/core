Attribute VB_Name = "ApplicationSpecific"
'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: ApplicationSpecific.bas,v $
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


