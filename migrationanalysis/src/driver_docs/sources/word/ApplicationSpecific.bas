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
Public Const CID_CHANGES_AND_REVIEWING = 1 'no issue logged
Public Const CID_CONTENT_AND_DOCUMENT_PROPERTIES = 2
Public Const CID_CONTROLS = 3 'no issue logged
Public Const CID_FIELDS = 4
Public Const CID_FORMAT = 5
Public Const CID_INDEX_AND_REFERENCES = 6
Public Const CID_OBJECTS_GRAPHICS_FRAMES = 7
Public Const CID_PORTABILITY = 8
Public Const CID_TABLES = 9
Public Const CID_VBA_MACROS = 10
Public Const CID_WRITING_AIDS = 11
Public Const CTOTAL_CATEGORIES = 11

'** Word - XML Issue and SubIssue strings
Public Const CSTR_ISSUE_CHANGES_AND_REVIEWING = "ChangesAndReviewing"
Public Const CSTR_ISSUE_CONTROLS = "Controls"
Public Const CSTR_ISSUE_FIELDS = "Fields"
Public Const CSTR_ISSUE_INDEX_AND_REFERENCES = "IndexesAndReferences"
Public Const CSTR_ISSUE_TABLES = "Tables"

Public Const CSTR_SUBISSUE_APPEARANCE = "Appearance"
Public Const CSTR_SUBISSUE_BORDER_STYLES = "BorderStyles"
Public Const CSTR_SUBISSUE_CELL_SPAN_PAGE = "CellSpanningPage"
Public Const CSTR_SUBISSUE_COMMENT = "Comment"
Public Const CSTR_SUBISSUE_CUSTOM_BULLET_LIST = "CustomBulletList"
Public Const CSTR_SUBISSUE_FORM_FIELD = "FormField"
Public Const CSTR_SUBISSUE_MAILMERGE_DATASOURCE = "MailMergeDatasource"
Public Const CSTR_SUBISSUE_MAILMERGE_FIELD = "MailMergeField"
Public Const CSTR_SUBISSUE_NESTED_TABLES = "NestedTables"
Public Const CSTR_SUBISSUE_TABLE_OF_AUTHORITIES = "TableOfAuthorities"
Public Const CSTR_SUBISSUE_TABLE_OF_AUTHORITIES_FIELD = "TableOfAuthoritiesField"
Public Const CSTR_SUBISSUE_TABLE_OF_CONTENTS = "TableOfContents"
'** END Word - XML Issue and SubIssue strings

Public Const CAPPNAME_WORD = "Word"
Public Const CAPPNAME_WORD_DOC = ".doc"
Public Const CAPPNAME_WORD_DOT = ".dot"

Public Const CAPPNAME_EXCEL = "Excel"
Public Const CAPPNAME_EXCEL_DOC = ".xls"
Public Const CAPPNAME_EXCEL_DOT = ".xlt"

Public Const CAPPNAME_POWERPOINT = "PowerPoint"
Public Const CAPPNAME_PP_DOC = ".ppt"
Public Const CAPPNAME_PP_DOT = ".pot"

Public CAPP_DOCPROP_LOCATION As String
'Public Const CAPP_DOCPROP_LOCATION = RID_STR_COMMON_RESULTS_LOCATION_TYPE_WORKBOOK
'Public Const CAPP_DOCPROP_LOCATION = RID_STR_COMMON_RESULTS_LOCATION_TYPE_PRESENTATION

Public Const CAPP_XMLDOCPROP_LOCATION = "Document"
'Public Const CAPP_XMLDOCPROP_LOCATION = "Workbook"
'Public Const CAPP_XMLDOCPROP_LOCATION = "Presentation"

Public Const CTHIS_DOCUMENT = "ThisDocument"
Public Const CTOPLEVEL_PROJECT = "Project"

Public Function getAppSpecificDocExt() As String
    getAppSpecificDocExt = CAPPNAME_WORD_DOC
End Function
Public Function getAppSpecificTemplateExt() As String
    getAppSpecificTemplateExt = CAPPNAME_WORD_DOT
End Function
Public Function getAppSpecificPath() As String
    getAppSpecificPath = ActiveDocument.path
End Function
Public Function getAppSpecificApplicationName() As String
    getAppSpecificApplicationName = CAPPNAME_WORD
End Function

Public Function getAppSpecificCustomDocProperties(currDoc As Document) As DocumentProperties
    Set getAppSpecificCustomDocProperties = currDoc.CustomDocumentProperties
End Function
Public Function getAppSpecificCommentBuiltInDocProperty(currDoc As Document) As DocumentProperty
    Set getAppSpecificCommentBuiltInDocProperty = currDoc.BuiltInDocumentProperties(wdPropertyComments)
End Function

Public Function getAppSpecificVBProject(currDoc As Document) As VBProject
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
    Application.WindowState = wdWindowStateMinimize
    Application.Visible = False
End Sub

Public Sub LocalizeResources()
    Dim wrdStrings As StringDataManager
    Set wrdStrings = New StringDataManager

    wrdStrings.InitStringData (GetResourceDataFileName(ThisDocument.path))
    LoadCommonStrings wrdStrings
    LoadWordStrings wrdStrings
    LoadResultsStrings wrdStrings
    Set wrdStrings = Nothing

    'SetWordDriverText
End Sub

' Set the loaded string variable into the fields in the document
Private Sub SetWordDriverText()
    On Error Resume Next
    CAPP_DOCPROP_LOCATION = RID_STR_COMMON_RESULTS_LOCATION_TYPE_DOCUMENT
    ThisDocument.FormFields.item("RID_STR_WDVR_SOANA").Result = RID_STR_WDVR_SOANA
    ThisDocument.FormFields.item("RID_STR_WDVR_INTRO").Result = RID_STR_WDVR_INTRO
    ThisDocument.FormFields.item("RID_STR_WDVR_TITLE").Result = RID_STR_WDVR_TITLE
    ThisDocument.FormFields.item("RID_STR_WDVR_PURPO").Result = RID_STR_WDVR_PURPO
    ThisDocument.FormFields.item("RID_STR_WDVR_PARA1").Result = RID_STR_WDVR_PARA1
    ThisDocument.FormFields.item("RID_STR_WDVR_ISSUE").Result = RID_STR_WDVR_ISSUE
    ThisDocument.FormFields.item("RID_STR_WDVR_PARA2").Result = RID_STR_WDVR_PARA2
End Sub

