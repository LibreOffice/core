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

'** Issue Categories
Public Const CID_INFORMATION_REFS = 0
Public Const CID_CHANGES_AND_REVIEWING = 1
Public Const CID_CHARTS_TABLES = 2
Public Const CID_CONTENT_AND_DOCUMENT_PROPERTIES = 3
Public Const CID_CONTROLS = 4
Public Const CID_FILTERS = 5
Public Const CID_FORMAT = 6
Public Const CID_FUNCTIONS = 7
Public Const CID_OBJECTS_GRAPHICS = 8
Public Const CID_PORTABILITY = 9
Public Const CID_VBA_MACROS = 10
Public Const CTOTAL_CATEGORIES = 10

'** Excel - XML Issue and SubIssue strings
Public Const CSTR_ISSUE_CHANGES_AND_REVIEWING = "ChangesAndReviewing"
Public Const CSTR_ISSUE_CHARTS_TABLES = "ChartsAndTables"
Public Const CSTR_ISSUE_CONTROLS = "Controls"
Public Const CSTR_ISSUE_FUNCTIONS = "Functions"
Public Const CSTR_ISSUE_OBJECTS_GRAPHICS = "ObjectsAndGraphics"

Public Const CSTR_SUBISSUE_ATTRIBUTES = "CellAttributes"
Public Const CSTR_SUBISSUE_DATEDIF = "DATEDIF"
Public Const CSTR_SUBISSUE_EMBEDDED_CHART = "EmbeddedChart"
Public Const CSTR_SUBISSUE_ERROR_TYPE = "ERROR.TYPE"
Public Const CSTR_SUBISSUE_EXTERNAL = "External"
Public Const CSTR_SUBISSUE_INFO = "INFO"
Public Const CSTR_SUBISSUE_MAX_ROWS_EXCEEDED = "MaximumRowsExceeded"
Public Const CSTR_SUBISSUE_MAX_SHEETS_EXCEEDED = "MaximumSheetsExceeded"
Public Const CSTR_SUBISSUE_PAGE_SETUP = "PageSetup"
Public Const CSTR_SUBISSUE_PASSWORD_PROTECTION = "PasswordProtection"
Public Const CSTR_SUBISSUE_PHONETIC = "PHONETIC"
Public Const CSTR_SUBISSUE_SHEET_CHART = "SheetChart"
Public Const CSTR_SUBISSUE_WORKBOOK_PROTECTION = "WorkbookProtection"
Public Const CSTR_SUBISSUE_ZOOM = "Zoom"

Public Const CSTR_SUBISSUE_CHART_COMPLEX = "ChartIssuesComplex"
Public Const CSTR_SUBISSUE_CHART_MINOR = "ChartIssuesMinor"
Public Const CSTR_SUBISSUE_CHART_PIVOT = "Pivot"
Public Const CSTR_SUBISSUE_INVALID_WORKSHEET_NAME = "InvalidWorksheetName"
Public Const CSTR_SUBISSUE_DB_QUERY = "DBQuery"

'** END Excel - XML Issue and SubIssue strings


Public Const CAPPNAME_WORD = "Word"
'Public Const CAPPNAME_WORD_DOC = ".doc"
'Public Const CAPPNAME_WORD_DOT = ".dot"

Public Const CAPPNAME_EXCEL = "Excel"
Public Const CAPPNAME_EXCEL_DOC = ".xls"
Public Const CAPPNAME_EXCEL_DOT = ".xlt"

Public Const CAPPNAME_POWERPOINT = "PowerPoint"
'Public Const CAPPNAME_PP_DOC = ".ppt"
'Public Const CAPPNAME_PP_DOT = ".pot"

'Public Const CAPP_DOCPROP_LOCATION = "Document"
Public CAPP_DOCPROP_LOCATION As String
'Public Const CAPP_DOCPROP_LOCATION = "Presentation"

'Public Const CAPP_XMLDOCPROP_LOCATION = "Document"
Public Const CAPP_XMLDOCPROP_LOCATION = "Workbook"
'Public Const CAPP_XMLDOCPROP_LOCATION = "Presentation"

Public Const CTHIS_DOCUMENT = "ThisWorkbook"
Public Const CTOPLEVEL_PROJECT = "VBAProject"

Public Function getAppSpecificDocExt() As String
    getAppSpecificDocExt = CAPPNAME_EXCEL_DOC
End Function
Public Function getAppSpecificTemplateExt() As String
    getAppSpecificTemplateExt = CAPPNAME_EXCEL_DOT
End Function

Public Function getAppSpecificPath() As String
    getAppSpecificPath = ActiveWorkbook.path
End Function
Public Function getAppSpecificApplicationName() As String
    getAppSpecificApplicationName = CAPPNAME_EXCEL
End Function

Public Function getAppSpecificCustomDocProperties(currDoc As Workbook) As DocumentProperties
    Set getAppSpecificCustomDocProperties = currDoc.CustomDocumentProperties
End Function
Public Function getAppSpecificCommentBuiltInDocProperty(currDoc As Workbook) As DocumentProperty
    Set getAppSpecificCommentBuiltInDocProperty = currDoc.BuiltinDocumentProperties("Comments")
End Function

Public Function getAppSpecificVBProject(currDoc As Workbook) As VBProject
    Set getAppSpecificVBProject = currDoc.VBProject
End Function

Public Function getAppSpecificOLEClassType(aShape As Shape) As String
    Dim objType As String
    
    If aShape.OLEFormat.ProgID = "" Then
        objType = aShape.OLEFormat.ClassType
    Else
        objType = aShape.OLEFormat.ProgID
    End If
        
    getAppSpecificOLEClassType = objType
End Function

Public Sub SetAppToMinimized()
    Application.WindowState = xlMinimized
    Application.Visible = False
End Sub

Public Sub LocalizeResources()
    Dim xlStrings As StringDataManager
    Set xlStrings = New StringDataManager
   
    xlStrings.InitStringData (GetResourceDataFileName(ThisWorkbook.path))
    LoadCommonStrings xlStrings
    LoadExcelStrings xlStrings
    LoadResultsStrings xlStrings
    Set xlStrings = Nothing
    
    SetWBDriverText
End Sub

Public Sub SetWBDriverText()
    On Error Resume Next
    CAPP_DOCPROP_LOCATION = RID_STR_COMMON_RESULTS_LOCATION_TYPE_DOCUMENT
    ThisWorkbook.Names("RID_STR_DVR_XL_EXCEL_DRIVER").RefersToRange.Cells(1, 1) = RID_STR_DVR_XL_EXCEL_DRIVER
    ThisWorkbook.Names("RID_STR_DVR_XL_ISSUES").RefersToRange.Cells(1, 1) = RID_STR_DVR_XL_ISSUES
    ThisWorkbook.Names("RID_STR_DVR_XL_PURPOSE").RefersToRange.Cells(1, 1) = RID_STR_DVR_XL_PURPOSE
    ThisWorkbook.Names("RID_STR_DVR_XL_READ_README").RefersToRange.Cells(1, 1) = RID_STR_DVR_XL_READ_README
    ThisWorkbook.Names("RID_STR_DVR_XL_THE_MACROS").RefersToRange.Cells(1, 1) = RID_STR_DVR_XL_THE_MACROS
    ThisWorkbook.Names("RID_STR_DVR_XL_THIS_DOC").RefersToRange.Cells(1, 1) = RID_STR_DVR_XL_THIS_DOC
    ThisWorkbook.Names("RID_STR_DVR_XL_TITLE").RefersToRange.Cells(1, 1) = RID_STR_DVR_XL_TITLE
End Sub

