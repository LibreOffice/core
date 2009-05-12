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

