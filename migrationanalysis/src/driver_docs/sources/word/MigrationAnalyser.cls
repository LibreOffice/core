VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "MigrationAnalyser"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
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

'Class variables
Private Enum HFIssueType
    hfInline
    hfShape
    hfFrame
End Enum

Private Enum HFIssueLocation
    hfHeader
    hffooter
End Enum


Private Type ShapeInfo
        top As Single
        Height As Single
End Type

Private Type FrameInfo
        Height As Single
        VerticalPosition As Single
End Type

Private mAnalysis As DocumentAnalysis
Private mOdd As Boolean
Private mbFormFieldErrorLogged As Boolean
Private mbRefFormFieldErrorLogged As Boolean

'***ADDING-ISSUE: Use Following Skeleton as Guideline for Adding Issue
' For complete list of all RID_STR_... for Issues (IssueType), SubIssues (SubType) and Attributes refer to:
'   word_res.bas and common_res.bas
'
' For complete list of all CID_... for Issue Categories(IssueID) and
' CSTR_... for XML Issues (IssueTypeXML) and XML SubIssues (SubTypeXML) refer to:
'   ApplicationSpecific.bas and CommonMigrationAnalyser.bas
'
' You should not have to add any new Issue Categories or matching IssueTypes, only new SubIssues
Sub Analyze_SKELETON()
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_SKELETON"
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    
    With myIssue
        .IssueID = CID_VBA_MACROS 'Issue Category
        .IssueType = RID_STR_COMMON_ISSUE_VBA_MACROS 'Issue String
        .SubType = RID_STR_COMMON_SUBISSUE_PROPERTIES 'SubIssue String
        .Location = .CLocationDocument 'Location string
        
        .IssueTypeXML = CSTR_ISSUE_VBA_MACROS 'Non localised XML Issue String
        .SubTypeXML = CSTR_SUBISSUE_PROPERTIES 'Non localised XML SubIssue String
        .locationXML = .CXMLLocationDocument 'Non localised XML location
        
        .SubLocation = 0 'if not set will default to RID_STR_NOT_AVAILABLE_SHORTHAND
        .Line = 0 'if not set will default to RID_STR_NOT_AVAILABLE_SHORTHAND
        .column = 0 'if not set will default to RID_STR_NOT_AVAILABLE_SHORTHAND
            
        ' Add as many Attribute Value pairs as needed
        ' Note: following must always be true - Attributes.Count = Values.Count
        .Attributes.Add "AAA"
        .Values.Add "foobar"

        ' Use AddIssueDetailsNote to add notes to the Issue Details if required
        ' Public Sub AddIssueDetailsNote(myIssue As IssueInfo, noteNum As Long, noteStr As String, _
        '   Optional preStr As String = RID_STR_COMMON_NOTE_PRE)
        ' Where preStr is prepended to the output, with "Note" as the default
         AddIssueDetailsNote myIssue, 0, RID_STR_COMMON_NOTE_DOCUMENT_PROPERTIES_LOST
        
         'Only put this in if you have a preparation function added for this issue in CommonPreparation
         'or Preparation - NUll can be replaced with any variant if you want to pass info to the Prepare fnc
         Call DoPreparation(mAnalysis, myIssue, "", Null, Null)

         mAnalysis.IssuesCountArray(CID_VBA_MACROS) = _
                mAnalysis.IssuesCountArray(CID_VBA_MACROS) + 1
    End With
       
    mAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub DoAnalyse(fileName As String, userFormTypesDict As Scripting.Dictionary, _
    startDir As String, storeToDir As String, fso As FileSystemObject)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "DoAnalyse"
    mAnalysis.name = fileName
    Dim aDoc As Document
    Dim bUnprotectError As Boolean
    mAnalysis.TotalIssueTypes = CTOTAL_CATEGORIES
    mbFormFieldErrorLogged = False
    mbRefFormFieldErrorLogged = False
    
    'Turn off any AutoExce macros before loading the Word doc
    On Error Resume Next ' Ignore errors on setting
    WordBasic.DisableAutoMacros 1
    On Error GoTo HandleErrors
    
    Dim myPassword As String
    myPassword = GetDefaultPassword
    
    'Always skip password protected documents
    'If IsSkipPasswordDocs() Then
    Dim aPass As String
    If myPassword <> "" Then
        aPass = myPassword
    Else
        aPass = "xoxoxoxoxo"
    End If
    
    On Error Resume Next
    Set aDoc = Documents.Open(fileName, False, False, False, _
                            aPass, aPass, False, aPass, aPass, wdOpenFormatAuto, _
                            msoEncodingAutoDetect, False)
    If Err.Number = 5408 Then
        ' if password protected, try open readonly next
        Set aDoc = Documents.Open(fileName, False, True, False, _
                    aPass, aPass, False, aPass, aPass, wdOpenFormatAuto, _
                    msoEncodingAutoDetect, False)
    End If
    If Err.Number = 5408 Then
        HandleProtectedDocInvalidPassword mAnalysis, _
            "User entered Invalid Document Password, further analysis not possible", fso
        Analyze_Password_Protection True, False
        GoTo FinalExit
    ElseIf (Err.Number <> 0) Then
        GoTo HandleErrors
    End If

    On Error GoTo HandleErrors

    If aDoc Is Nothing Then GoTo FinalExit
 
    'Do Analysis
    Analyze_Password_Protection aDoc.HasPassword, aDoc.WriteReserved
    Analyze_Document_Protection aDoc
        
    If aDoc.ProtectionType <> wdNoProtection Then
        If myPassword <> "" Then
            aDoc.Unprotect (myPassword)
        Else
            aDoc.Unprotect
        End If
    End If
    
    'Set Doc Properties
    SetDocProperties mAnalysis, aDoc, fso
    
ContinueFromUnprotectError:

    Analyze_Tables_TablesInTables aDoc
    Analyze_Tables_Borders aDoc
    Analyze_TOA aDoc
    If Not bUnprotectError Then
        Analyze_FieldAndFormFieldIssues aDoc
    End If
    Analyze_OLEEmbedded aDoc
    Analyze_MailMerge_DataSource aDoc
    Analyze_Macros mAnalysis, userFormTypesDict, aDoc
    'Analyze_Numbering aDoc, mAnalysis
    'Analyze_NumberingTabs aDoc, mAnalysis

    ' Doc Preparation only
    ' Save document with any prepared issues under <storeToDir>\prepared\<source doc name>
    If mAnalysis.PreparableIssuesCount > 0 And CheckDoPrepare Then
        Dim preparedFullPath As String
        preparedFullPath = GetPreparedFullPath(mAnalysis.name, startDir, storeToDir, fso)
        If preparedFullPath <> "" Then
            If fso.FileExists(preparedFullPath) Then
                fso.DeleteFile preparedFullPath, True
            End If
            If fso.FolderExists(fso.GetParentFolderName(preparedFullPath)) Then
                    aDoc.SaveAs preparedFullPath
            End If
        End If
    End If
        
    'DebugMacroInfo
    
FinalExit:


    If Not aDoc Is Nothing Then 'If Not IsEmpty(aDoc) Then
        aDoc.Close (False)
    End If
    Set aDoc = Nothing
    
    Exit Sub

HandleErrors:
    ' MsgBox currentFunctionName & " : " & fileName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    ' Handle Password error on Doc Open, Modify and Cancel
    If Err.Number = 5408 Or Err.Number = 4198 Then
        WriteDebug currentFunctionName & " : " & fileName & ": " & _
            "User entered Invalid Document Password - " & Err.Number & " " & Err.Description & " " & Err.Source
        HandleProtectedDocInvalidPassword mAnalysis, _
            "User entered Invalid Document Password, further analysis not possible", fso
        Resume FinalExit
    ElseIf Err.Number = 5485 Then
        ' Handle Password error on Unprotect Doc
        WriteDebug currentFunctionName & " : " & fileName & ": " & _
            "User entered Invalid Document Part Password, Analysis of doc will continue but will skip analysis of:" & _
            "Forms, Comments, Headers & Footers and Table cell spanning issues - " & Err.Number & " " & Err.Description & " " & Err.Source
        HandleProtectedDocInvalidPassword mAnalysis, _
            "User entered Invalid Document Part Password, Analysis of doc will continue but will skip analysis of:" & vbLf & _
            "Forms, Comments, Headers & Footers and Table cell spanning issues", fso
        bUnprotectError = True
        'wdAllowOnlyComments, wdAllowOnlyFormFields, wdAllowOnlyRevisions
        Resume ContinueFromUnprotectError
    End If
    mAnalysis.Application = RID_STR_COMMON_CANNOT_OPEN
    WriteDebug currentFunctionName & " : " & fileName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub DebugMacroInfo()
    MsgBox "TotalNumLines: " & mAnalysis.MacroTotalNumLines & vbLf & _
    "NumUserForms: " & mAnalysis.MacroNumUserForms & vbLf & _
    "NumUserFormControls: " & mAnalysis.MacroNumUserFormControls & vbLf & _
    "NumUserFormControlTypes: " & mAnalysis.MacroNumUserFormControlTypes & vbLf & _
    "NumExternalRefs: " & mAnalysis.MacroNumExternalRefs & vbLf & _
    "MacroNumFieldsUsingMacros: " & mAnalysis.MacroNumFieldsUsingMacros & vbLf & _
    "NumOLEControls: " & mAnalysis.MacroNumOLEControls & vbLf & _
    "MacroOverallClass: " & getDocOverallMacroClassAsString(mAnalysis.MacroOverallClass)
End Sub

Sub SetDocProperties(docAnalysis As DocumentAnalysis, doc As Document, fso As FileSystemObject)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetProperties"
    Dim f As File
    Set f = fso.GetFile(docAnalysis.name)
    
    docAnalysis.PageCount = doc.ComputeStatistics(wdStatisticPages)
    docAnalysis.Accessed = f.DateLastAccessed
    
    On Error Resume Next 'Some apps may not support all props
    docAnalysis.Application = getAppSpecificApplicationName & " " & Application.Version
    'docAnalysis.Application = doc.BuiltinDocumentProperties(wdPropertyAppName)
    'If InStr(docAnalysis.Application, "Microsoft") = 1 Then
    '    docAnalysis.Application = Mid(docAnalysis.Application, Len("Microsoft") + 2)
    'End If
    'If InStr(Len(docAnalysis.Application) - 2, docAnalysis.Application, ".") = 0 Then
    '    docAnalysis.Application = docAnalysis.Application & " " & Application.Version
    'End If

    docAnalysis.Created = _
        doc.BuiltInDocumentProperties(wdPropertyTimeCreated)
    docAnalysis.Modified = _
        doc.BuiltInDocumentProperties(wdPropertyTimeLastSaved)
    docAnalysis.Printed = _
        doc.BuiltInDocumentProperties(wdPropertyTimeLastPrinted)
    docAnalysis.SavedBy = _
        doc.BuiltInDocumentProperties(wdPropertyLastAuthor)
    docAnalysis.Revision = _
        val(doc.BuiltInDocumentProperties(wdPropertyRevision))
    docAnalysis.Template = _
        fso.GetFileName(doc.BuiltInDocumentProperties(wdPropertyTemplate))

FinalExit:
    Set f = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

'Limitation: Detect first level table in tables, does not detect further nesting
'Can do so if required
Sub Analyze_Tables_TablesInTables(currDoc As Document)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Tables_TablesInTables"
    Dim myTopTable As Table
    Dim myInnerTable As Table
    Dim myIssue As IssueInfo
    
    For Each myTopTable In currDoc.Tables
        For Each myInnerTable In myTopTable.Tables
            Dim logString As String
            Dim myRng As Range
            Dim startpage As Long
            Dim startRow As Long
            Dim StartColumn As Long
            Dim details As String
            
            Set myIssue = New IssueInfo
            Set myRng = myInnerTable.Range
            myRng.start = myRng.End
            startpage = myRng.Information(wdActiveEndPageNumber)
            startRow = myRng.Information(wdStartOfRangeRowNumber)
            StartColumn = myRng.Information(wdStartOfRangeColumnNumber)
            
            With myIssue
                .IssueID = CID_TABLES
                .IssueType = RID_STR_WORD_ISSUE_TABLES
                .SubType = RID_STR_WORD_SUBISSUE_NESTED_TABLES
                .Location = .CLocationPage
                .SubLocation = startpage

                .IssueTypeXML = CSTR_ISSUE_TABLES
                .SubTypeXML = CSTR_SUBISSUE_NESTED_TABLES
                .locationXML = .CXMLLocationPage
                
                .Attributes.Add RID_STR_WORD_ATTRIBUTE_OUTER_TABLE
                .Values.Add myTopTable.Rows.count & "x" & myTopTable.Columns.count
                .Attributes.Add RID_STR_WORD_ATTRIBUTE_INNER_TABLE
                .Values.Add myInnerTable.Rows.count & "x" & myInnerTable.Columns.count
                .Attributes.Add RID_STR_WORD_ATTRIBUTE_START_ROW
                .Values.Add startRow
                .Attributes.Add RID_STR_WORD_ATTRIBUTE_START_COL
                .Values.Add StartColumn
                AddIssueDetailsNote myIssue, 0, RID_STR_WORD_NOTE_NESTED_TABLE_WILL_BE_LOST
                
                mAnalysis.IssuesCountArray(CID_TABLES) = _
                    mAnalysis.IssuesCountArray(CID_TABLES) + 1
            End With
        
            mAnalysis.Issues.Add myIssue
            Set myIssue = Nothing
            Set myRng = Nothing
        Next
    Next
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub Analyze_Document_Protection(currDoc As Document)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Document_Protection"
    If currDoc.ProtectionType = wdNoProtection Then
        Exit Sub
    End If
    
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    
    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_STR_COMMON_SUBISSUE_DOCUMENT_PARTS_PROTECTION
        .Location = .CLocationDocument
            
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_DOCUMENT_PARTS_PROTECTION
        .locationXML = .CXMLLocationDocument
        
        .Attributes.Add RID_STR_WORD_ATTRIBUTE_PROTECTION
        Select Case currDoc.ProtectionType
            Case wdAllowOnlyComments
                .Values.Add RID_STR_WORD_ATTRIBUTE_ALLOW_ONLY_COMMENTS
            Case wdAllowOnlyFormFields
                .Values.Add RID_STR_WORD_ATTRIBUTE_ALLOW_ONLY_FORM_FIELDS
            Case wdAllowOnlyRevisions
                .Values.Add RID_STR_WORD_ATTRIBUTE_ALLOW_ONLY_REVISIONS
            Case Else
                .Values.Add RID_STR_COMMON_ATTRIBUTE_UNKNOWN
        End Select
        
         mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
    End With
       
    mAnalysis.Issues.Add myIssue
FinalExit:
    Set myIssue = Nothing
    Exit Sub
     
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_Password_Protection(bHasPassword As Boolean, bWriteReserved As Boolean)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Password_Protection"
    Dim myIssue As IssueInfo

    If bHasPassword Or bWriteReserved Then
        Set myIssue = New IssueInfo

        With myIssue
            .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
            .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
            .SubType = RID_STR_COMMON_SUBISSUE_PASSWORDS_PROTECTION
            .Location = .CLocationDocument
                
            .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
            .SubTypeXML = CSTR_SUBISSUE_PASSWORDS_PROTECTION
            .locationXML = .CXMLLocationDocument
                
            If bHasPassword Then
                .Attributes.Add RID_STR_WORD_ATTRIBUTE_PASSWORD_TO_OPEN
                .Values.Add RID_STR_WORD_ATTRIBUTE_SET
            End If
            If bWriteReserved Then
                .Attributes.Add RID_STR_WORD_ATTRIBUTE_PASSWORD_TO_MODIFY
                .Values.Add RID_STR_WORD_ATTRIBUTE_SET
            End If

            mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                    mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
        End With

        mAnalysis.Issues.Add myIssue
    End If
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_OLEEmbedded(currDoc As Document)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_OLEEmbedded"

    ' Handle Inline Shapes
    Dim aILShape As InlineShape
    For Each aILShape In currDoc.InlineShapes
        Analyze_OLEEmbeddedSingleInlineShape aILShape
    Next aILShape
    
    ' Handle Shapes
    Dim aShape As Shape
    For Each aShape In currDoc.Shapes
        Analyze_OLEEmbeddedSingleShape mAnalysis, aShape, _
            Selection.Information(wdActiveEndPageNumber)
        Analyze_Lines mAnalysis, aShape, _
            Selection.Information(wdActiveEndPageNumber)
        Analyze_Transparency mAnalysis, aShape, _
            Selection.Information(wdActiveEndPageNumber)
        Analyze_Gradients mAnalysis, aShape, _
            Selection.Information(wdActiveEndPageNumber)
    Next aShape
    
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub


'WdInlineShapeType constants:
'wdInlineShapeEmbeddedOLEObject, wdInlineShapeHorizontalLine, wdInlineShapeLinkedOLEObject,
'wdInlineShapeLinkedPicture, wdInlineShapeLinkedPictureHorizontalLine, wdInlineShapeOLEControlObject,
'wdInlineShapeOWSAnchor, wdInlineShapePicture, wdInlineShapePictureBullet,
'wdInlineShapePictureHorizontalLine, wdInlineShapeScriptAnchor

Sub Analyze_OLEEmbeddedSingleInlineShape(aILShape As InlineShape)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_OLEEmbeddedSingleInlineShape"
    Dim myIssue As IssueInfo
    Dim bOleObject As Boolean
    Dim TypeAsString As String
    Dim XMLTypeAsString As String
    Dim objName As String
        
    bOleObject = (aILShape.Type = wdInlineShapeEmbeddedOLEObject) Or _
                    (aILShape.Type = wdInlineShapeLinkedOLEObject) Or _
                    (aILShape.Type = wdInlineShapeOLEControlObject)
                    
    If Not bOleObject Then Exit Sub
    
    aILShape.Select
    Select Case aILShape.Type
        Case wdInlineShapeOLEControlObject
            TypeAsString = RID_STR_COMMON_OLE_CONTROL
            XMLTypeAsString = CSTR_SUBISSUE_OLE_CONTROL
        Case wdInlineShapeEmbeddedOLEObject
            TypeAsString = RID_STR_COMMON_OLE_EMBEDDED
            XMLTypeAsString = CSTR_SUBISSUE_OLE_EMBEDDED
        Case wdInlineShapeLinkedOLEObject
            TypeAsString = RID_STR_COMMON_OLE_LINKED
            XMLTypeAsString = CSTR_SUBISSUE_OLE_LINKED
        Case Else
            TypeAsString = RID_STR_COMMON_OLE_UNKNOWN
            XMLTypeAsString = CSTR_SUBISSUE_OLE_UNKNOWN
    End Select
    
    Set myIssue = New IssueInfo
    With myIssue
        .IssueID = CID_PORTABILITY
        .IssueType = RID_STR_COMMON_ISSUE_PORTABILITY
        .SubType = TypeAsString
        .Location = .CLocationPage
        .SubLocation = Selection.Information(wdActiveEndPageNumber)
        
        .IssueTypeXML = CSTR_ISSUE_PORTABILITY
        .SubTypeXML = XMLTypeAsString
        .locationXML = .CXMLLocationPage
                
        .Line = Selection.Information(wdFirstCharacterLineNumber)
        .column = Selection.Information(wdFirstCharacterColumnNumber)
        
        DoEvents
        If aILShape.Type = wdInlineShapeEmbeddedOLEObject Or _
           aILShape.Type = wdInlineShapeOLEControlObject Then
            
            'If Object is invalid can get automation server hanging
            Dim tmpStr As String
            On Error Resume Next
            tmpStr = aILShape.OLEFormat.Object
            If Err.Number = 0 Then
                .Attributes.Add RID_STR_COMMON_ATTRIBUTE_OBJECT_TYPE
                .Values.Add aILShape.OLEFormat.ProgID
            Else
                Err.Clear
                tmpStr = aILShape.OLEFormat.ClassType
                If Err.Number = 0 Then
                    .Attributes.Add RID_STR_COMMON_ATTRIBUTE_OBJECT_TYPE
                    .Values.Add aILShape.OLEFormat.ClassType
                Else
                    .Attributes.Add RID_STR_COMMON_ATTRIBUTE_OBJECT_TYPE
                    .Values.Add RID_STR_COMMON_NA
                End If
            End If

            If aILShape.Type = wdInlineShapeOLEControlObject Then
                mAnalysis.MacroNumOLEControls = 1 + mAnalysis.MacroNumOLEControls
            End If
            
            objName = aILShape.OLEFormat.Object.name
            If Err.Number = 0 Then
                .Attributes.Add RID_STR_COMMON_ATTRIBUTE_OBJECT_NAME
                .Values.Add objName
            End If
            On Error GoTo HandleErrors
        End If
        If aILShape.Type = wdInlineShapeLinkedOLEObject Then
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_SOURCE
            .Values.Add aILShape.LinkFormat.SourceFullName
        End If
                        
        mAnalysis.IssuesCountArray(CID_PORTABILITY) = _
            mAnalysis.IssuesCountArray(CID_PORTABILITY) + 1
    End With
    
    mAnalysis.Issues.Add myIssue
        
FinalExit:
    Set myIssue = Nothing
    Exit Sub
     
HandleErrors:
    WriteDebugLevelTwo currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

'Appears to be picked up by other OLE analysis code - the Shapes are actually field codes
'So I get double reporting if I use this as well.
Sub Analyze_OLEFields(myField As Field)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_OLEFields"
    Dim myIssue As IssueInfo
    Dim bOleObject As Boolean
    Dim TypeAsString As String
    Dim XMLTypeAsString As String

    bOleObject = (myField.Type = wdFieldOCX)
                    
    If Not bOleObject Then Exit Sub
    
    myField.Select
    Select Case myField.Type
        Case wdFieldLink
            TypeAsString = RID_STR_COMMON_OLE_FIELD_LINK
            XMLTypeAsString = CSTR_SUBISSUE_OLE_FIELD_LINK
        Case Else
            TypeAsString = RID_STR_COMMON_OLE_UNKNOWN
            XMLTypeAsString = CSTR_SUBISSUE_OLE_UNKNOWN
    End Select
    Set myIssue = New IssueInfo
    With myIssue
        .IssueID = CID_PORTABILITY
        .IssueType = RID_STR_COMMON_ISSUE_PORTABILITY
        .SubType = TypeAsString
        .Location = .CLocationPage
        .SubLocation = Selection.Information(wdActiveEndPageNumber)
        
        .IssueTypeXML = CSTR_ISSUE_PORTABILITY
        .SubTypeXML = XMLTypeAsString
        .locationXML = .CXMLLocationPage
        
        .Line = Selection.Information(wdFirstCharacterLineNumber)
        .column = Selection.Information(wdFirstCharacterColumnNumber)
        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_OBJECT_TYPE
        .Values.Add myField.OLEFormat.ClassType
        
        If myField.Type = wdFieldLink Then
            .Attributes.Add RID_STR_WORD_ATTRIBUTE_LINK
            .Values.Add myField.LinkFormat.SourceFullName
        End If
        mAnalysis.IssuesCountArray(CID_PORTABILITY) = _
            mAnalysis.IssuesCountArray(CID_PORTABILITY) + 1
    End With
    mAnalysis.Issues.Add myIssue
    
    Set myIssue = Nothing
    
    Exit Sub
    
HandleErrors:
    Set myIssue = Nothing
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub Analyze_MailMergeField(myField As Field)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_MailMergeField"
    Dim myIssue As IssueInfo
    Dim TypeAsString As String
    Dim bProblemMailMergeField As Boolean
    
    bProblemMailMergeField = _
        (myField.Type = wdFieldFillIn) Or _
        (myField.Type = wdFieldAsk) Or _
        (myField.Type = wdFieldMergeRec) Or _
        (myField.Type = wdFieldMergeField) Or _
        (myField.Type = wdFieldNext) Or _
        (myField.Type = wdFieldRevisionNum) Or _
        (myField.Type = wdFieldSequence) Or _
        (myField.Type = wdFieldAutoNum) Or _
        (myField.Type = wdFieldAutoNumOutline) Or _
        (myField.Type = wdFieldAutoNumLegal)
    
    If bProblemMailMergeField Then
    'Some of the following are numbering fields and need to be broken out into a seperate function. See migration guide.
       
        Select Case myField.Type
        Case wdFieldFillIn
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_FILL_IN
        Case wdFieldAsk
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_ASK
        Case wdFieldMergeRec
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_MERGE_RECORDS
        Case wdFieldMergeField
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_MERGE_FIELDS
        Case wdFieldNext
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_NEXT
        Case wdFieldRevisionNum
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_REVISION_NUMBER
        Case wdFieldSequence
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_SEQUENCE
        Case wdFieldAutoNum
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_AUTO_NUMBER
        Case wdFieldAutoNumOutline
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_AUTO_NUMBER_OUTLINE
        Case wdFieldAutoNumLegal
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_AUTO_NUMBER_LEGAL
        Case Else
            TypeAsString = RID_STR_WORD_ENUMERATION_MAILMERGE_FIELD_NAME_NOT_KNOWN
        End Select
        
        Set myIssue = New IssueInfo
        myField.Select
        With myIssue
            .IssueID = CID_FIELDS
            .IssueType = RID_STR_WORD_ISSUE_FIELDS
            .SubType = RID_STR_WORD_SUBISSUE_MAILMERGE_FIELD
            .Location = .CLocationPage
            
            .IssueTypeXML = CSTR_ISSUE_FIELDS
            .SubTypeXML = CSTR_SUBISSUE_MAILMERGE_FIELD
            .locationXML = .CXMLLocationPage
            
            .SubLocation = Selection.Information(wdActiveEndPageNumber)
            .Line = Selection.Information(wdFirstCharacterLineNumber)
            .column = Selection.Information(wdFirstCharacterColumnNumber)
            
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
            .Values.Add TypeAsString
            If myField.Code.Text <> "" Then
                .Attributes.Add RID_STR_WORD_ATTRIBUTE_TEXT
                .Values.Add myField.Code.Text
            End If
            
            mAnalysis.IssuesCountArray(CID_FIELDS) = _
                mAnalysis.IssuesCountArray(CID_FIELDS) + 1
        End With
        mAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
    End If
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

'Get field DS Info
Sub Analyze_MailMerge_DataSource(currDoc As Document)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_MailMerge_DataSource"
    ' There may be no mail merge in the document
    If (currDoc.MailMerge.DataSource.Type = wdNoMergeInfo) Then
        Exit Sub
    End If
    
    'Dim issue As SimpleAnalysisInfo
    If (currDoc.MailMerge.DataSource.Type <> wdNoMergeInfo) Then
        Dim myIssue As IssueInfo
        Set myIssue = New IssueInfo
        With myIssue
            .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
            .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
            .SubType = RID_STR_WORD_SUBISSUE_MAILMERGE_DATASOURCE
            .Location = .CLocationDocument
            
            .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
            .SubTypeXML = CSTR_SUBISSUE_MAILMERGE_DATASOURCE
            .locationXML = .CXMLLocationDocument
            
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
            .Values.Add currDoc.MailMerge.DataSource.name
            .Attributes.Add RID_STR_WORD_ATTRIBUTE_DATASOURCE
            .Values.Add currDoc.MailMerge.DataSource.Type
            
            mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
        End With
        
        mAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
   End If
   Exit Sub
   
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Function getFormFieldTypeAsString(fieldType As WdFieldType)
    Dim Str As String
    
    Select Case fieldType
    Case wdFieldFormCheckBox
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_CHECK_BOX
    Case wdFieldFormDropDown
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_DROP_DOWN
    Case wdFieldFormTextInput
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_TEXT
    Case Else
        Str = RID_STR_WORD_ENUMERATION_UNKNOWN
    End Select
    
    getFormFieldTypeAsString = Str
End Function
Function getTextFormFieldTypeAsString(fieldType As WdTextFormFieldType)
    Dim Str As String
    
    Select Case fieldType
    Case wdCalculationText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_CALCULATION
    Case wdCurrentDateText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_CURRENT_DATE
    Case wdCurrentTimeText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_CURRENT_TIME
    Case wdDateText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_DATE
    Case wdNumberText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_NUMBER
    Case wdRegularText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_REGULAR
    Case Else
        Str = RID_STR_WORD_ENUMERATION_UNKNOWN
    End Select
    
    getTextFormFieldTypeAsString = Str
End Function
Function getTextFormFieldDefaultAsString(fieldType As WdTextFormFieldType)
    Dim Str As String
    
    Select Case fieldType
    Case wdCalculationText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_DEFAULT_EXPRESSION
    Case wdCurrentDateText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_DEFAULT_DATE
    Case wdCurrentTimeText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_DEFAULT_TIME
    Case wdDateText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_DEFAULT_DATE
    Case wdNumberText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_DEFAULT_NUMBER
    Case wdRegularText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_DEFAULT_TEXT
    Case Else
        Str = RID_STR_WORD_ENUMERATION_UNKNOWN
    End Select
    
    getTextFormFieldDefaultAsString = Str
End Function
Function getTextFormFieldFormatAsString(fieldType As WdTextFormFieldType)
    Dim Str As String
    
    Select Case fieldType
    Case wdCalculationText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_FORMAT_NUMBER
    Case wdCurrentDateText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_FORMAT_DATE
    Case wdCurrentTimeText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_FORMAT_TIME
    Case wdDateText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_FORMAT_DATE
    Case wdNumberText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_FORMAT_NUMBER
    Case wdRegularText
        Str = RID_STR_WORD_ENUMERATION_FORM_FIELD_FORMAT_TEXT
    Case Else
        Str = RID_STR_WORD_ENUMERATION_UNKNOWN
    End Select
    
    getTextFormFieldFormatAsString = Str
End Function

Sub Analyze_FieldAndFormFieldIssues(currDoc As Document)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_FormFields"
    Dim myIssue As IssueInfo

    'Analysze all Fields in doc
    Dim myField As Field

    For Each myField In currDoc.Fields
        'Analyze Mail Merge Fields
        Analyze_MailMergeField myField
        
        'Analyze TOA Fields
        Analyze_TOAField myField
    Next myField

    'Analyze FormField doc issues
    If currDoc.FormFields.count = 0 Then GoTo FinalExit
    
    If (currDoc.FormFields.Shaded) Then
        Set myIssue = New IssueInfo
        With myIssue
            .IssueID = CID_FIELDS
            .IssueType = RID_STR_WORD_ISSUE_FIELDS
            .SubType = RID_STR_WORD_SUBISSUE_APPEARANCE
            .Location = .CLocationDocument
            
            .IssueTypeXML = CSTR_ISSUE_FIELDS
            .SubTypeXML = CSTR_SUBISSUE_APPEARANCE
            .locationXML = .CXMLLocationDocument
            
            .Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_GREYED
            .Values.Add RID_STR_WORD_TRUE
            mAnalysis.IssuesCountArray(CID_FIELDS) = _
                mAnalysis.IssuesCountArray(CID_FIELDS) + 1
        End With
        mAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
    End If

    'Analyse all FormFields in doc
    Dim myFormField As FormField
    
    For Each myFormField In currDoc.FormFields
        Analyze_FormFieldIssue myFormField
    Next myFormField
    
FinalExit:
    Set myIssue = Nothing
    Set myFormField = Nothing
    Exit Sub
    
HandleErrors:
    
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_FormFieldIssue(myFormField As FormField)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_FormFieldIssue"
    Dim myIssue As IssueInfo
    Dim bCheckBoxIssues As Boolean
    Dim bFormFieldIssues As Boolean
    
    bCheckBoxIssues = False
    If (myFormField.Type = wdFieldFormCheckBox) Then
        If myFormField.CheckBox.AutoSize Then
            bCheckBoxIssues = True
        End If
    End If
    
    bFormFieldIssues = bCheckBoxIssues
        
    If Not bFormFieldIssues Then GoTo FinalExit
        
    myFormField.Select
    Set myIssue = New IssueInfo
    With myIssue
        .IssueID = CID_FIELDS
        .IssueType = RID_STR_WORD_ISSUE_FIELDS
        .SubType = RID_STR_WORD_SUBISSUE_FORM_FIELD
        .Location = .CLocationPage

        .IssueTypeXML = CSTR_ISSUE_FIELDS
        .SubTypeXML = CSTR_SUBISSUE_FORM_FIELD
        .locationXML = .CXMLLocationPage
            
        .SubLocation = Selection.Information(wdActiveEndPageNumber)
        .Line = Selection.Information(wdFirstCharacterLineNumber)
        .column = Selection.Information(wdFirstCharacterColumnNumber)
        myIssue.Attributes.Add RID_STR_COMMON_ATTRIBUTE_TYPE
        myIssue.Values.Add getFormFieldTypeAsString(myFormField.Type)
    End With

    'Checkbox Issues
    If (myFormField.Type = wdFieldFormCheckBox) Then
       'AutoSize CheckBoxes
        If myFormField.CheckBox.AutoSize Then
            myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_AUTOSIZE
            myIssue.Values.Add RID_STR_WORD_TRUE
        End If
    End If

    'TextInput Issues
    If myFormField.Type = wdFieldFormTextInput Then
        myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_TEXT_FORM_FIELD_TYPE
        myIssue.Values.Add getTextFormFieldTypeAsString(myFormField.TextInput.Type)
        Dim bLostType As Boolean
        bLostType = False
        If (myFormField.TextInput.Type = wdCalculationText) Or _
            (myFormField.TextInput.Type = wdCurrentDateText) Or _
            (myFormField.TextInput.Type = wdCurrentTimeText) Then
            AddIssueDetailsNote myIssue, 0, getTextFormFieldTypeAsString(myFormField.TextInput.Type) & _
            " " & RID_STR_WORD_NOTE_FORM_FIELD_TYPE_LOST
            bLostType = True
        End If
        
        If (myFormField.TextInput.Format <> "") Then
            myIssue.Attributes.Add getTextFormFieldFormatAsString(myFormField.TextInput.Type)
            myIssue.Values.Add myFormField.TextInput.Format
        End If
       
       'Default text
        If (myFormField.TextInput.Default <> "") Then
            myIssue.Attributes.Add getTextFormFieldDefaultAsString(myFormField.TextInput.Type)
            myIssue.Values.Add myFormField.TextInput.Default
        End If

       'Maximum text
        If (myFormField.TextInput.Width <> 0) Then
            myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_MAX_LENGTH
            myIssue.Values.Add myFormField.TextInput.Width
        End If
        
        'Fill-in disabled
        If (myFormField.Enabled = False) And (Not bLostType) Then
            myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_FILLIN_ENABLED
            myIssue.Values.Add RID_STR_WORD_FALSE
        End If
    End If

    'Help Key(F1)
    If (myFormField.OwnHelp And myFormField.HelpText <> "") Then
        myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_HELP_KEY_F1_OWN_TEXT
        myIssue.Values.Add myFormField.HelpText
    ElseIf ((Not myFormField.OwnHelp) And myFormField.HelpText <> "") Then
        myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_HELP_KEY_F1_AUTO_TEXT
        myIssue.Values.Add myFormField.HelpText
    End If

    'StatusHelp
    If (myFormField.OwnStatus And myFormField.StatusText <> "") Then
        myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_STATUS_BAR_HELP_OWN_TEXT
        myIssue.Values.Add myFormField.StatusText
    ElseIf ((Not myFormField.OwnStatus) And myFormField.StatusText <> "") Then
        myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_STATUS_BAR_HELP_AUTO_TEXT
        myIssue.Values.Add myFormField.StatusText
    End If

    'Macros
    If (myFormField.EntryMacro <> "") Then
        myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_ENTRY_MACRO
        myIssue.Values.Add myFormField.EntryMacro
    End If
    If (myFormField.ExitMacro <> "") Then
        myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_EXIT_MACRO
        myIssue.Values.Add myFormField.ExitMacro
    End If
    If (myFormField.EntryMacro <> "") Or (myFormField.ExitMacro <> "") Then
        mAnalysis.MacroNumFieldsUsingMacros = 1 + mAnalysis.MacroNumFieldsUsingMacros
    End If
               
    'LockedField
    If (myFormField.Enabled = False) And (myFormField.Type <> wdFieldFormTextInput) Then
        myIssue.Attributes.Add RID_STR_WORD_ATTRIBUTE_FORM_FIELD_LOCKED
        myIssue.Values.Add RID_STR_WORD_TRUE
    End If

    mAnalysis.IssuesCountArray(CID_FIELDS) = _
        mAnalysis.IssuesCountArray(CID_FIELDS) + 1
        
    mAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub
    
HandleErrors:
    'Log first occurence for this doc
    If Not mbFormFieldErrorLogged Then
        WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
        mbFormFieldErrorLogged = True
    End If
    Resume FinalExit
End Sub


Sub Analyze_TOA(currDoc As Document)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_TOA"
   
    Dim toa As TableOfAuthorities
    Dim myIssue As IssueInfo
    Dim myRng As Range

    For Each toa In currDoc.TablesOfAuthorities
        Set myRng = toa.Range
        myRng.start = myRng.End
        Set myIssue = New IssueInfo
        myRng.Select

        Dim TabLeaderAsString As String
        Select Case toa.TabLeader
            Case wdTabLeaderDashes
                TabLeaderAsString = RID_STR_WORD_ENUMERATION_INDEX_LEADER_DASHES
            Case wdTabLeaderDots
                TabLeaderAsString = RID_STR_WORD_ENUMERATION_INDEX_LEADER_DOTS
            Case wdTabLeaderHeavy
                TabLeaderAsString = RID_STR_WORD_ENUMERATION_INDEX_LEADER_HEAVY
            Case wdTabLeaderLines
                TabLeaderAsString = RID_STR_WORD_ENUMERATION_INDEX_LEADER_LINES
            Case wdTabLeaderMiddleDot
                TabLeaderAsString = RID_STR_WORD_ENUMERATION_INDEX_LEADER_MIDDLEDOT
            Case wdTabLeaderSpaces
                TabLeaderAsString = RID_STR_WORD_ENUMERATION_INDEX_LEADER_SPACES
            Case Else
                TabLeaderAsString = RID_STR_WORD_ENUMERATION_UNKNOWN
        End Select
        
        Dim FormatAsString As String
        Select Case currDoc.TablesOfAuthorities.Format
            Case wdTOAClassic
                FormatAsString = RID_STR_WORD_ENUMERATION_INDEX_TABLES_CLASSIC
            Case wdTOADistinctive
                FormatAsString = RID_STR_WORD_ENUMERATION_INDEX_TABLES_DISTINCTIVE
            Case wdTOAFormal
                FormatAsString = RID_STR_WORD_ENUMERATION_INDEX_TABLES_FORMAL
            Case wdTOASimple
                FormatAsString = RID_STR_WORD_ENUMERATION_INDEX_TABLES_SIMPLE
            Case wdTOATemplate
                FormatAsString = RID_STR_WORD_ENUMERATION_INDEX_TABLES_FROM_TEMPLATE
            Case Else
                FormatAsString = RID_STR_WORD_ENUMERATION_UNKNOWN
        End Select

        With myIssue
            .IssueID = CID_INDEX_AND_REFERENCES
            .IssueType = RID_STR_WORD_ISSUE_INDEX_AND_REFERENCES
            .SubType = RID_STR_WORD_SUBISSUE_TABLE_OF_AUTHORITIES
            .Location = .CLocationPage

            .IssueTypeXML = CSTR_ISSUE_INDEX_AND_REFERENCES
            .SubTypeXML = CSTR_SUBISSUE_TABLE_OF_AUTHORITIES
            .locationXML = .CXMLLocationPage

            .SubLocation = myRng.Information(wdActiveEndPageNumber)
            .Attributes.Add RID_STR_WORD_ATTRIBUTE_LEADER
            .Values.Add TabLeaderAsString
            
            AddIssueDetailsNote myIssue, 0, RID_STR_WORD_NOTE_TOA_MIGRATE_AS_PLAIN_TEXT
            
            mAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) = _
                mAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) + 1
        End With
        
        mAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
        Set myRng = Nothing
    Next
FinalExit:
    Set myIssue = Nothing
    Set myRng = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_TOAField(myField As Field)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_TOAField"
   
    Dim toa As TableOfAuthorities
    Dim myIssue As IssueInfo
    
    If myField.Type = wdFieldTOAEntry Then
        Set myIssue = New IssueInfo
        myField.Select
        
        With myIssue
            .IssueID = CID_FIELDS
            .IssueType = RID_STR_WORD_ISSUE_FIELDS
            .SubType = RID_STR_WORD_SUBISSUE_TABLE_OF_AUTHORITIES_FIELD
            .Location = .CLocationPage
            
            .IssueTypeXML = CSTR_ISSUE_FIELDS
            .SubTypeXML = CSTR_SUBISSUE_TABLE_OF_AUTHORITIES_FIELD
            .locationXML = .CXMLLocationPage
            
            .SubLocation = Selection.Information(wdActiveEndPageNumber)
            .Line = Selection.Information(wdFirstCharacterLineNumber)
            .column = Selection.Information(wdFirstCharacterColumnNumber)
            
            .Attributes.Add RID_STR_WORD_ATTRIBUTE_FIELD_TEXT
            .Values.Add myField.Code.Text
            
            AddIssueDetailsNote myIssue, 0, RID_STR_WORD_NOTE_TOA_FIELD_LOST_ON_ROUNDTRIP
            
            mAnalysis.IssuesCountArray(CID_FIELDS) = _
                mAnalysis.IssuesCountArray(CID_FIELDS) + 1
        End With
        
        mAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
    End If
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_Tables_Borders(currDoc As Document)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Tables_Borders"
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    Dim aTable As Table
    Dim invalidBorders As String
    
    For Each aTable In currDoc.Tables
        invalidBorders = GetInvalidBorder(aTable)
        If invalidBorders <> "" Then
            aTable.Range.Select
            Set myIssue = New IssueInfo
            With myIssue
                .IssueID = CID_TABLES
                .IssueType = RID_STR_WORD_ISSUE_TABLES
                .SubType = RID_STR_WORD_SUBISSUE_BORDER_STYLES
                .Location = .CLocationPage
        
                .IssueTypeXML = CSTR_ISSUE_TABLES
                .SubTypeXML = CSTR_SUBISSUE_BORDER_STYLES
                .locationXML = .CXMLLocationPage
        
                .SubLocation = Selection.Information(wdActiveEndPageNumber)
                .Line = Selection.Information(wdFirstCharacterLineNumber)
                .column = Selection.Information(wdFirstCharacterColumnNumber)
                
                .Attributes.Add RID_STR_WORD_ATTRIBUTE_BORDERS_NOT_DISPLAYING
                .Values.Add invalidBorders

                AddIssueDetailsNote myIssue, 0, RID_STR_WORD_NOTE_TABLE_BORDER
    
                mAnalysis.IssuesCountArray(CID_TABLES) = mAnalysis.IssuesCountArray(CID_TABLES) + 1
            End With
    
            mAnalysis.Issues.Add myIssue
            Set myIssue = Nothing
        End If
    Next aTable
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
Function GetInvalidBorder(aTable As Table) As String

    Dim theResult As String
    theResult = ""
    
    If IsInvalidBorderStyle(aTable.Borders(wdBorderTop).LineStyle) Then
        theResult = theResult + "Top, "
    End If
    If IsInvalidBorderStyle(aTable.Borders(wdBorderBottom).LineStyle) Then
            theResult = theResult + "Bottom, "
    End If
    If IsInvalidBorderStyle(aTable.Borders(wdBorderDiagonalDown).LineStyle) Then
            theResult = theResult + "Down Diagonal, "
    End If
    If IsInvalidBorderStyle(aTable.Borders(wdBorderDiagonalUp).LineStyle) Then
            theResult = theResult + "Up Diagonal, "

    End If
    If IsInvalidBorderStyle(aTable.Borders(wdBorderHorizontal).LineStyle) Then
            theResult = theResult + "Horizontal, "
    End If
    If IsInvalidBorderStyle(aTable.Borders(wdBorderLeft).LineStyle) Then
            theResult = theResult + "Left, "
    End If
    If IsInvalidBorderStyle(aTable.Borders(wdBorderRight).LineStyle) Then
            theResult = theResult + "Right, "
    End If
    If IsInvalidBorderStyle(aTable.Borders(wdBorderVertical).LineStyle) Then
            theResult = theResult + "Vertical, "
    End If
    
    If theResult <> "" Then
        theResult = Left(theResult, (Len(theResult) - 2)) + "."
    End If
    
    GetInvalidBorder = theResult
End Function

Function IsInvalidBorderStyle(aStyle As WdLineStyle) As Boolean

    Dim IsInvalid As Boolean
    
    Select Case aStyle
    Case wdLineStyleDot, wdLineStyleDashSmallGap, wdLineStyleDashLargeGap, wdLineStyleDashDot, _
        wdLineStyleDashDotDot, wdLineStyleTriple, wdLineStyleThinThickThinSmallGap, wdLineStyleThinThickMedGap, _
        wdLineStyleThickThinMedGap, wdLineStyleThinThickThinMedGap, wdLineStyleThinThickLargeGap, _
        wdLineStyleThickThinLargeGap, wdLineStyleThinThickThinLargeGap, wdLineStyleSingleWavy, _
        wdLineStyleDoubleWavy, wdLineStyleDashDotStroked, wdLineStyleEmboss3D, wdLineStyleEngrave3D
        IsInvalid = True
    Case Else
        IsInvalid = False
    End Select
    
    IsInvalidBorderStyle = IsInvalid
    
End Function

Private Sub Class_Initialize()
    Set mAnalysis = New DocumentAnalysis
End Sub
Private Sub Class_Terminate()
    Set mAnalysis = Nothing
End Sub

Public Property Get Results() As DocumentAnalysis
    Set Results = mAnalysis
End Property

Sub Analyze_NumberingTabs(currDoc As Document, docAnalysis As DocumentAnalysis)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_NumberingTabs"

    Dim tb As TabStop
    Dim customTabPos As Single
    Dim tabs As Integer
    Dim listLvl As Long
    Dim tp As Single
    Dim bHasAlignmentProblem As Boolean
    Dim bHasTooManyTabs As Boolean
    Dim myIssue As IssueInfo
    Dim p As Object

    bHasAlignmentProblem = False
    bHasTooManyTabs = False

    For Each p In currDoc.ListParagraphs
        tabs = 0
        For Each tb In p.TabStops
            If tb.customTab Then
                tabs = tabs + 1
                customTabPos = tb.Position
            End If
        Next

        If tabs = 1 Then
            listLvl = p.Range.ListFormat.ListLevelNumber
            tp = p.Range.ListFormat.ListTemplate.ListLevels.item(listLvl).TabPosition
            If (p.Range.ListFormat.ListTemplate.ListLevels.item(listLvl).Alignment <> _
                wdListLevelAlignLeft) Then
                ' ERROR: alignment problem
                bHasAlignmentProblem = True
            End If

            If tp <> customTabPos Then
                p.Range.InsertBefore ("XXXXX")
            End If
            'OK - at least heuristically
        Else
            'ERROR: too many tabs
            bHasTooManyTabs = True
        End If
    Next

    If (bHasAlignmentProblem) Then
        Set myIssue = New IssueInfo

        With myIssue
            .IssueID = CID_INDEX_AND_REFERENCES
            .IssueType = RID_STR_WORD_ISSUE_INDEX_AND_REFERENCES
            .SubType = RID_STR_WORD_SUBISSUE_NUMBERING_TAB_ALIGNMENT
            .Location = .CLocationDocument 'Location string

            .IssueTypeXML = CSTR_ISSUE_INDEX_AND_REFERENCES
            .SubTypeXML = CSTR_SUBISSUE_NUMBERING_TAB_ALIGNMENT
            .locationXML = .CXMLLocationDocument

            AddIssueDetailsNote myIssue, 0, RID_STR_WORD_NOTE_NUMBERING_TAB_ALIGNMENT

            docAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) = _
                    docAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) + 1
        End With
        docAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
    End If

    If (bHasTooManyTabs) Then
        Set myIssue = New IssueInfo

        With myIssue
            .IssueID = CID_INDEX_AND_REFERENCES
            .IssueType = RID_STR_WORD_ISSUE_INDEX_AND_REFERENCES
            .SubType = RID_STR_WORD_SUBISSUE_NUMBERING_TAB_OVERFLOW
            .Location = .CLocationDocument 'Location string

            .IssueTypeXML = CSTR_ISSUE_INDEX_AND_REFERENCES
            .SubTypeXML = CSTR_SUBISSUE_NUMBERING_TAB_OVERFLOW
            .locationXML = .CXMLLocationDocument

            AddIssueDetailsNote myIssue, 0, RID_STR_WORD_NOTE_NUMBERING_TAB_OVERFLOW

            docAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) = _
                    docAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) + 1
        End With
        docAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
    End If

FinalExit:
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Set myIssue = Nothing
    Resume FinalExit
End Sub

Sub Analyze_Numbering(currDoc As Document, docAnalysis As DocumentAnalysis)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Numbering"

    Dim myIssue As IssueInfo
    Dim nFormatProblems As Integer
    Dim nAlignmentProblems As Integer
    nFormatProblems = 0
    nAlignmentProblems = 0

    Dim lt As ListTemplate
    Dim lvl As ListLevel
    Dim I, l_, p1, p2, v1, v2 As Integer
    Dim display_levels As Integer
    Dim fmt, prefix, postfix, res As String
    
    For Each lt In currDoc.ListTemplates
        l_ = 0
        For Each lvl In lt.ListLevels
            l_ = l_ + 1
            'Selection.TypeText Text:="List Number Format " + lvl.NumberFormat
            'Apply Heuristic
            fmt = lvl.NumberFormat
            p1 = InStr(fmt, "%")
            p2 = InStrRev(fmt, "%")
            v1 = val(Mid(fmt, p1 + 1, 1))
            v2 = val(Mid(fmt, p2 + 1, 1))
            display_levels = v2 - v1 + 1
            prefix = Mid(fmt, 1, p1 - 1)
            postfix = Mid(fmt, p2 + 2)
            'Check Heuristic
            res = prefix
            For I = 2 To display_levels
                res = "%" + Trim(Str(l_ - I + 1)) + "." + res
            Next
            res = res + "%" + Trim(Str(l_)) + postfix
            If (StrComp(res, fmt) <> 0) Then
                nFormatProblems = nFormatProblems + 1
                'Selection.TypeText Text:="Label Problem: NumberFormat=" + fmt + " Heuristic=" + res
            End If
            
            'check alignment
            If (lvl.NumberPosition <> wdListLevelAlignLeft) Then
                nAlignmentProblems = nAlignmentProblems + 1
                'Selection.TypeText Text:="Number alignment problem"
            End If
        Next
    Next

    If (nFormatProblems > 0) Then
        Set myIssue = New IssueInfo

        With myIssue
            .IssueID = CID_INDEX_AND_REFERENCES
            .IssueType = RID_STR_WORD_ISSUE_INDEX_AND_REFERENCES
            .SubType = RID_STR_WORD_SUBISSUE_NUMBERING_FORMAT
            .Location = .CLocationDocument 'Location string

            .IssueTypeXML = CSTR_ISSUE_INDEX_AND_REFERENCES
            .SubTypeXML = CSTR_SUBISSUE_NUMBERING_FORMAT
            .locationXML = .CXMLLocationDocument

            .Attributes.Add RID_STR_WORD_ATTRIBUTE_COUNT
            .Values.Add nFormatProblems

            AddIssueDetailsNote myIssue, 0, RID_STR_WORD_NOTE_NUMBERING_FORMAT

            docAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) = _
                    docAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) + 1
        End With
        docAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
    End If

    If (nAlignmentProblems > 0) Then
        Set myIssue = New IssueInfo

        With myIssue
            .IssueID = CID_INDEX_AND_REFERENCES
            .IssueType = RID_STR_WORD_ISSUE_INDEX_AND_REFERENCES
            .SubType = RID_STR_WORD_SUBISSUE_NUMBERING_ALIGNMENT
            .Location = .CLocationDocument 'Location string

            .IssueTypeXML = CSTR_ISSUE_INDEX_AND_REFERENCES
            .SubTypeXML = CSTR_SUBISSUE_NUMBERING_ALIGNMENT
            .locationXML = .CXMLLocationDocument

            .Attributes.Add RID_STR_WORD_ATTRIBUTE_COUNT
            .Values.Add nAlignmentProblems

            AddIssueDetailsNote myIssue, 0, RID_STR_WORD_NOTE_NUMBERING_ALIGNMENT

            docAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) = _
                    docAnalysis.IssuesCountArray(CID_INDEX_AND_REFERENCES) + 1
        End With
        docAnalysis.Issues.Add myIssue
        Set myIssue = Nothing
    End If

FinalExit:
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Set myIssue = Nothing
    Resume FinalExit
End Sub

