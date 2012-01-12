Attribute VB_Name = "CommonMigrationAnalyser"
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


'***********************************************
'**** APPLICATION COMMON ANALYSIS FUNCTIONS ****
'***********************************************

'** Common - XML Issue and SubIssue strings
'For preparation - need access to some Word/ Excel or PP consts
Public Const CSTR_ISSUE_OBJECTS_GRAPHICS_AND_FRAMES = "ObjectsGraphicsAndFrames"
Public Const CSTR_SUBISSUE_OBJECT_IN_HEADER_FOOTER = "ObjectInHeaderFooter"

Public Const CSTR_ISSUE_INFORMATION = "Information"
Public Const CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES = "ContentAndDocumentProperties"
Public Const CSTR_ISSUE_FORMAT = "Format"
Public Const CSTR_ISSUE_PORTABILITY = "Portability"
Public Const CSTR_ISSUE_VBA_MACROS = "VBAMacros"

Public Const CSTR_SUBISSUE_DOCUMENT_PARTS_PROTECTION = "DocumentPartsProtection"
Public Const CSTR_SUBISSUE_EXTERNAL_REFERENCES_IN_MACRO = "ExternalReferencesInMacro"
Public Const CSTR_SUBISSUE_EXTERNAL_REFERENCES_IN_MACRO_COUNT = "ExternalReferencesInMacroCount"
Public Const CSTR_SUBISSUE_GRADIENT = "Gradient"
Public Const CSTR_SUBISSUE_INVALID_PASSWORD_ENTERED = "InvalidPasswordEntered"
Public Const CSTR_SUBISSUE_LINE = "Line"
Public Const CSTR_SUBISSUE_MACRO_PASSWORD_PROTECTION = "PasswordProtected"
Public Const CSTR_SUBISSUE_OLD_WORKBOOK_VERSION = "OldWorkbookVersion"
Public Const CSTR_SUBISSUE_OLE_EMBEDDED = "EmbeddedOLEObject"
Public Const CSTR_SUBISSUE_OLE_LINKED = "LinkedOLEObject"
Public Const CSTR_SUBISSUE_OLE_CONTROL = "OLEControl"
Public Const CSTR_SUBISSUE_OLE_FIELD_LINK = "OLEFieldLink"
Public Const CSTR_SUBISSUE_OLE_UNKNOWN = "UnknownType"
Public Const CSTR_SUBISSUE_PASSWORDS_PROTECTION = "PasswordProtection"
Public Const CSTR_SUBISSUE_PROPERTIES = "Properties"
Public Const CSTR_SUBISSUE_REFERENCES = "References"
Public Const CSTR_SUBISSUE_TRANSPARENCY = "Transparency"
Public Const CSTR_SUBISSUE_VBA_MACROS_NUMLINES = "NumberOfLines"
Public Const CSTR_SUBISSUE_VBA_MACROS_USERFORMS_COUNT = "UserFormsCount"
Public Const CSTR_SUBISSUE_VBA_MACROS_USERFORMS_CONTROL_COUNT = "UserFormsControlCount"
Public Const CSTR_SUBISSUE_VBA_MACROS_USERFORMS_CONTROLTYPE_COUNT = "UserFormsControlTypeCount"
Public Const CSTR_SUBISSUE_VBA_MACROS_UNIQUE_MODULE_COUNT = "UniqueModuleCount"
Public Const CSTR_SUBISSUE_VBA_MACROS_UNIQUE_LINE_COUNT = "UniqueLineCount"
'** END Common - XML Issue and SubIssue strings

'Macro classification bounds
Public Const CMACRO_LINECOUNT_MEDIUM_LBOUND = 50

'Don't localize folder name
Public Const CSTR_COMMON_PREPARATION_FOLDER = "prepared"


Public Enum EnumDocOverallMacroClass
    enMacroNone = 0
    enMacroSimple = 1
    enMacroMedium = 2
    enMacroComplex = 3
End Enum
Public Enum EnumDocOverallIssueClass
    enNone = 0
    enMinor = 1
    enComplex = 2
End Enum

Sub EmptyCollection(docAnalysis As DocumentAnalysis, coll As Collection)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "EmptyCollection"
    Dim Num As Long
    For Num = 1 To coll.count    ' Remove name from the collection.
        coll.Remove 1    ' Default collection numeric indexes
    Next    ' begin at 1.
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Public Function Analyze_Macros(docAnalysis As DocumentAnalysis, _
                               userFormTypesDict As Scripting.Dictionary, _
                               currDoc As Object)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Macros"
    Dim macroDetails As String
    Dim cmpDetails As String
    Dim myProject As VBProject
    Dim myComponent As VBComponent
    Dim numLines As Long
    Dim myIssue As IssueInfo
    Dim wrd As Object
    Dim bUserFormWithEmptyCodeModule As Boolean
     
    On Error Resume Next
    Set myProject = getAppSpecificVBProject(currDoc)
    If Err.Number <> 0 Then
        ' Failed to get access to VBProject
        WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & _
            RID_STR_COMMON_ATTRIBUTE_UNABLE_TO_ACCESS_VBPROJECT & ":" & _
            RID_STR_COMMON_ATTRIBUTE_FURTHER_MACRO_ANALYSIS_NOT_POSSIBLE
        
        GoTo FinalExit
    End If
    
    On Error GoTo HandleErrors
    If myProject.Protection = vbext_pp_locked Then
        Set myIssue = New IssueInfo
        With myIssue
            .IssueID = CID_VBA_MACROS
            .IssueType = RID_STR_COMMON_ISSUE_VBA_MACROS
            .SubType = RID_STR_COMMON_SUBISSUE_MACRO_PASSWORD_PROTECTION
            .Location = .CLocationDocument
            
            .IssueTypeXML = CSTR_ISSUE_VBA_MACROS
            .SubTypeXML = CSTR_SUBISSUE_MACRO_PASSWORD_PROTECTION
            .locationXML = .CXMLLocationDocument
            
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_VBPROJECT_PASSWORD
            .Values.Add RID_STR_COMMON_ATTRIBUTE_FURTHER_MACRO_ANALYSIS_NOT_POSSIBLE
        End With
        docAnalysis.IssuesCountArray(CID_VBA_MACROS) = _
            docAnalysis.IssuesCountArray(CID_VBA_MACROS) + 1
        docAnalysis.Issues.Add myIssue
        docAnalysis.MacroIssuesCount = docAnalysis.MacroIssuesCount + 1
        
        docAnalysis.HasMacros = True
        GoTo FinalExit
    End If

    Dim myContolDict As Scripting.Dictionary
    For Each myComponent In myProject.VBComponents
    
        bUserFormWithEmptyCodeModule = False
        If CheckEmptyProject(docAnalysis, myProject, myComponent) Then
            If myComponent.Type <> vbext_ct_MSForm Then
                GoTo FOREACH_CONTINUE
            Else
                bUserFormWithEmptyCodeModule = True
            End If
        End If
        
        Analyze_MacrosForPortabilityIssues docAnalysis, myProject, myComponent
        
        Set myIssue = New IssueInfo
        With myIssue
            .IssueID = CID_VBA_MACROS
            .IssueType = RID_STR_COMMON_ISSUE_VBA_MACROS
            .SubType = RID_STR_COMMON_SUBISSUE_PROPERTIES
            .Location = .CLocationDocument
            
            .IssueTypeXML = CSTR_ISSUE_VBA_MACROS
            .SubTypeXML = CSTR_SUBISSUE_PROPERTIES
            .locationXML = .CXMLLocationDocument
            
            .SubLocation = VBComponentType(myComponent)
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_PROJECT
            .Values.Add myProject.name
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_COMPONENT
            .Values.Add myComponent.name
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_PROCEDURES
            .Values.Add VBNumFuncs(docAnalysis, myComponent.CodeModule), RID_STR_COMMON_ATTRIBUTE_PROCEDURES
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NUMBER_OF_LINES
            numLines = VBNumLines(docAnalysis, myComponent.CodeModule)
            .Values.Add numLines, RID_STR_COMMON_ATTRIBUTE_NUMBER_OF_LINES
            
            If bUserFormWithEmptyCodeModule Then
                .Attributes.Add RID_STR_COMMON_ATTRIBUTE_SIGNATURE
                .Values.Add RID_STR_COMMON_NA, RID_STR_COMMON_ATTRIBUTE_SIGNATURE
            Else
                .Attributes.Add RID_STR_COMMON_ATTRIBUTE_SIGNATURE
                .Values.Add MD5HashString( _
                    myComponent.CodeModule.Lines(1, myComponent.CodeModule.CountOfLines)), _
                    RID_STR_COMMON_ATTRIBUTE_SIGNATURE
            End If
            
            docAnalysis.MacroTotalNumLines = numLines + docAnalysis.MacroTotalNumLines
        End With
        
        ' User Forms - control details
        If (myComponent.Type = vbext_ct_MSForm) And Not bUserFormWithEmptyCodeModule Then
            myIssue.Attributes.Add RID_STR_COMMON_ATTRIBUTE_CONTROLS
            myIssue.Values.Add myComponent.Designer.Controls.count, RID_STR_COMMON_ATTRIBUTE_CONTROLS
            docAnalysis.MacroNumUserForms = 1 + docAnalysis.MacroNumUserForms
            docAnalysis.MacroNumUserFormControls = myComponent.Designer.Controls.count + docAnalysis.MacroNumUserFormControls
            
            Dim myControl As Control
            Dim controlTypes As String
            Dim myType As String
            
            Set myContolDict = New Scripting.Dictionary
            
            For Each myControl In myComponent.Designer.Controls
                myType = TypeName(myControl)
                If myContolDict.Exists(myType) Then
                   myContolDict.item(myType) = myContolDict.item(myType) + 1
                Else
                   myContolDict.Add myType, 1
                End If
                If userFormTypesDict.Exists(myType) Then
                   userFormTypesDict.item(myType) = userFormTypesDict.item(myType) + 1
                Else
                   userFormTypesDict.Add myType, 1
                End If
            Next
            
            If myComponent.Designer.Controls.count > 0 Then
                Dim count As Long
                Dim vKeyArray As Variant
                Dim vItemArray As Variant
                
                vKeyArray = myContolDict.Keys
                vItemArray = myContolDict.Items
                
                controlTypes = ""
                For count = 0 To myContolDict.count - 1
                    controlTypes = controlTypes & vKeyArray(count) & " " & CInt(vItemArray(count)) & " "
                Next count
                myIssue.Attributes.Add RID_STR_COMMON_ATTRIBUTE_USERFORM_TYPE
                myIssue.Values.Add controlTypes, RID_STR_COMMON_ATTRIBUTE_USERFORM_TYPE
                
                myIssue.Attributes.Add RID_STR_COMMON_ATTRIBUTE_USERFORM_TYPES_COUNT
                myIssue.Values.Add myContolDict.count, RID_STR_COMMON_ATTRIBUTE_USERFORM_TYPES_COUNT
            
                docAnalysis.MacroNumUserFormControlTypes = myContolDict.count + docAnalysis.MacroNumUserFormControlTypes
            End If
            Set myContolDict = Nothing
        End If
        
        'Check for occurence of " Me " in Form and Class Modules
        If myComponent.Type = vbext_ct_MSForm Or _
            myComponent.Type = vbext_ct_ClassModule Then
         
            Dim strFind As String
            strFind = ""
            count = 0
            strFind = VBFindLines(docAnalysis, myComponent.CodeModule, "Me", count, bWholeWord:=True)
'            If (strFind <> "") Then MsgBox strFind

            If count > 0 Then
                myIssue.Attributes.Add RID_STR_COMMON_ATTRIBUTE_CLASS_ME_COUNT
                myIssue.Values.Add count, RID_STR_COMMON_ATTRIBUTE_CLASS_ME_COUNT
            End If
        End If
        
        docAnalysis.IssuesCountArray(CID_VBA_MACROS) = _
            docAnalysis.IssuesCountArray(CID_VBA_MACROS) + 1
        docAnalysis.Issues.Add myIssue
        docAnalysis.MacroIssuesCount = docAnalysis.MacroIssuesCount + 1
        
        Set myIssue = Nothing
        
FOREACH_CONTINUE:
        'No equiv to C continue in VB
    Next myComponent 'End - For Each myComponent
    
    If docAnalysis.IssuesCountArray(CID_VBA_MACROS) > 0 Then
        Analyze_VBEReferences docAnalysis, currDoc
        docAnalysis.HasMacros = True
    End If
    
FinalExit:
    docAnalysis.MacroOverallClass = ClassifyDocOverallMacroClass(docAnalysis)
    
    Set myProject = Nothing
    Set myIssue = Nothing
    Set myContolDict = Nothing
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function CheckOnlyEmptyProject(docAnalysis As DocumentAnalysis, currDoc As Object) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckOnlyEmptyProject"
    Dim myProject As VBProject
    Set myProject = getAppSpecificVBProject(currDoc)
    Dim myVBComponent As VBComponent

    For Each myVBComponent In myProject.VBComponents
        If Not CheckEmptyProject(docAnalysis, myProject, myVBComponent) Then
            CheckOnlyEmptyProject = False
            GoTo FinalExit
        End If
    Next myVBComponent
    
    CheckOnlyEmptyProject = True

FinalExit:
    Set myProject = Nothing
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Sub Analyze_VBEReferences(docAnalysis As DocumentAnalysis, currDoc As Object)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_VBEReferences"
    'References
    Dim Ref As Reference
    Dim fso As Scripting.FileSystemObject
    Dim myVBProject As VBProject
    Dim myVBComponent As VBComponent
    
    Set fso = New Scripting.FileSystemObject
    
    If CheckOnlyEmptyProject(docAnalysis, currDoc) Then
        Exit Sub
    End If
    Set myVBProject = getAppSpecificVBProject(currDoc)
    
    For Each Ref In myVBProject.References
        Analyze_VBEReferenceSingle docAnalysis, Ref, fso
    Next Ref
    
FinalExit:
    Set myVBProject = Nothing
    Set fso = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_VBEReferenceSingle(docAnalysis As DocumentAnalysis, Ref As Reference, fso As Scripting.FileSystemObject)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_VBEReferenceSingle"
    'References
    Dim myIssue As IssueInfo
    Dim bBadRef As Boolean
    
    Set myIssue = New IssueInfo
    With myIssue
        .IssueID = CID_INFORMATION_REFS
        .IssueType = RID_STR_COMMON_ISSUE_INFORMATION
        .SubType = RID_STR_COMMON_SUBISSUE_REFERENCES
        .Location = .CLocationDocument
            
        .IssueTypeXML = CSTR_ISSUE_INFORMATION
        .SubTypeXML = CSTR_SUBISSUE_REFERENCES
        .locationXML = .CXMLLocationDocument
            
        If Ref.GUID = "" Then
            bBadRef = True
        Else
            bBadRef = False
        End If
        If Not bBadRef Then
            .SubLocation = LCase(fso.GetFileName(Ref.FullPath))
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
            .Values.Add Ref.name, RID_STR_COMMON_ATTRIBUTE_NAME
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_DESCRIPTION
            .Values.Add Ref.Description, RID_STR_COMMON_ATTRIBUTE_DESCRIPTION
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_FILE
            .Values.Add LCase(fso.GetFileName(Ref.FullPath)), RID_STR_COMMON_ATTRIBUTE_FILE
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_PATH
            .Values.Add LCase(Ref.FullPath), RID_STR_COMMON_ATTRIBUTE_PATH
        Else
            .SubLocation = RID_STR_COMMON_NA
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
            .Values.Add RID_STR_COMMON_ATTRIBUTE_MISSING, RID_STR_COMMON_ATTRIBUTE_NAME
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_DESCRIPTION
            .Values.Add RID_STR_COMMON_ATTRIBUTE_CHECK_DOCUMENT_REFERENCES, RID_STR_COMMON_ATTRIBUTE_DESCRIPTION
        End If

        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_MAJOR
        .Values.Add IIf(Not bBadRef, Ref.Major, ""), RID_STR_COMMON_ATTRIBUTE_MAJOR
        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_MINOR
        .Values.Add IIf(Not bBadRef, Ref.Minor, ""), RID_STR_COMMON_ATTRIBUTE_MINOR
        
        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_TYPE
        .Values.Add IIf(Ref.Type = vbext_rk_Project, RID_STR_COMMON_ATTRIBUTE_PROJECT, RID_STR_COMMON_ATTRIBUTE_TYPELIB), RID_STR_COMMON_ATTRIBUTE_TYPE
    
        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_BUILTIN
        .Values.Add IIf(Ref.BuiltIn, RID_STR_COMMON_ATTRIBUTE_BUILTIN, RID_STR_COMMON_ATTRIBUTE_CUSTOM), RID_STR_COMMON_ATTRIBUTE_BUILTIN
        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_ISBROKEN
        .Values.Add IIf(bBadRef, RID_STR_COMMON_ATTRIBUTE_BROKEN, RID_STR_COMMON_ATTRIBUTE_INTACT), RID_STR_COMMON_ATTRIBUTE_ISBROKEN
        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_GUID
        .Values.Add IIf(Ref.Type = vbext_rk_TypeLib, Ref.GUID, ""), RID_STR_COMMON_ATTRIBUTE_GUID
    End With
    
    docAnalysis.References.Add myIssue
     
FinalExit:
    Set myIssue = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebugLevelTwo currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_MacrosForPortabilityIssues(docAnalysis As DocumentAnalysis, myProject As VBProject, myComponent As VBComponent)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_MacrosForPortabilityIssues"
    Dim myIssue As IssueInfo
    Dim count As Long
    
    ' Code Modules
    Dim strFind As String
    strFind = VBFindLines(docAnalysis, myComponent.CodeModule, "CreateObject", count, bWholeWord:=True) & _
        VBFindLines(docAnalysis, myComponent.CodeModule, "GetObject", count, bWholeWord:=True) & _
        VBFindLines(docAnalysis, myComponent.CodeModule, "ADODB.", count, True, True) & _
        VBFindLines(docAnalysis, myComponent.CodeModule, "Word.", count, True, True) & _
        VBFindLines(docAnalysis, myComponent.CodeModule, "Excel.", count, True, True) & _
        VBFindLines(docAnalysis, myComponent.CodeModule, "PowerPoint.", count, True, True) & _
        VBFindLines(docAnalysis, myComponent.CodeModule, "Access.", count, True, True) & _
        VBFindLines(docAnalysis, myComponent.CodeModule, "Declare Function ", count, False) & _
        VBFindLines(docAnalysis, myComponent.CodeModule, "Declare Sub ", count, False)

    
    If (strFind <> "") And (myComponent.Type <> vbext_ct_Document) Then
        Set myIssue = New IssueInfo
        With myIssue
            .IssueID = CID_PORTABILITY
            .IssueType = RID_STR_COMMON_ISSUE_PORTABILITY
            .SubType = RID_STR_COMMON_SUBISSUE_EXTERNAL_REFERENCES_IN_MACROS
            .Location = .CLocationDocument

            .IssueTypeXML = CSTR_ISSUE_PORTABILITY
            .SubTypeXML = CSTR_SUBISSUE_EXTERNAL_REFERENCES_IN_MACRO
            .locationXML = .CXMLLocationDocument

            .SubLocation = VBComponentType(myComponent)

            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_PROJECT
            .Values.Add myProject.name
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_COMPONENT
            .Values.Add myComponent.name
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NON_PORTABLE_EXTERNAL_REFERENCES
            .Values.Add RID_STR_COMMON_ATTRIBUTE_INCLUDING & vbLf & Left(strFind, Len(strFind) - 1)
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NON_PORTABLE_EXTERNAL_REFERENCES_COUNT
            .Values.Add count, RID_STR_COMMON_ATTRIBUTE_NON_PORTABLE_EXTERNAL_REFERENCES_COUNT
        End With
        docAnalysis.IssuesCountArray(CID_PORTABILITY) = _
            docAnalysis.IssuesCountArray(CID_PORTABILITY) + 1
        docAnalysis.Issues.Add myIssue
        docAnalysis.MacroNumExternalRefs = count + docAnalysis.MacroNumExternalRefs
        docAnalysis.MacroIssuesCount = docAnalysis.MacroIssuesCount + 1
    End If
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub
    
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
Resume FinalExit
End Sub

'Find Lines in  code module containing strFind and return list of them
Function VBFindLines(docAnalysis As DocumentAnalysis, vbcm As CodeModule, strFind As String, _
    count As Long, _
    Optional bInProcedure As Boolean = True, _
    Optional bUsingNew As Boolean = False, _
    Optional bWholeWord As Boolean = False, _
    Optional bMatchCase As Boolean = False) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "VBFindLines"
    Dim lngStartLine As Long
    Dim lngStartCol As Long
    Dim lngEndLine As Long
    Dim lngEndCol As Long
    Dim strLine As String
    lngStartLine = 1
    lngStartCol = 1
    lngEndLine = vbcm.CountOfLines
    Dim tmpString As String
    If (vbcm.CountOfLines = 0) Then
        Exit Function
    End If
    tmpString = vbcm.Lines(vbcm.CountOfLines, 1)
    lngEndCol = Len(vbcm.Lines(vbcm.CountOfLines, 1))
    Dim lngType As Long
    Dim strProc As String
    Dim retStr As String
        
    ' Search
    Do While vbcm.Find(strFind, lngStartLine, _
        lngStartCol, lngEndLine, lngEndCol, bWholeWord, bMatchCase)
        
        'Ignore any lines using this func
        If InStr(1, vbcm.Lines(lngStartLine, 1), "VBFindLines") <> 0 Then
            GoTo CONTINUE_LOOP
        End If
        
        If bInProcedure Then
            If bUsingNew Then
                If InStr(1, vbcm.Lines(lngStartLine, 1), "New") <> 0 Then
                    strProc = vbcm.ProcOfLine(lngStartLine, lngType)
                Else
                    strProc = ""
                End If
            Else
                strProc = vbcm.ProcOfLine(lngStartLine, lngType)
            End If
            If strProc = "" Then GoTo CONTINUE_LOOP
            
            VBFindLines = VBFindLines & "[" & strProc & " ( ) - " & lngStartLine & " ]" & _
                vbLf & vbcm.Lines(lngStartLine, 1) & vbLf
        Else
            strProc = vbcm.Lines(lngStartLine, 1)
            If strProc = "" Then GoTo CONTINUE_LOOP
            
            'Can be External refs, Const, Type or variable declarations
            If InStr(1, vbcm.Lines(lngStartLine, 1), "Declare Function") <> 0 Then
            VBFindLines = VBFindLines & "[" & RID_STR_COMMON_DEC_TO_EXTERNAL_LIBRARY & " - " & lngStartLine & " ]" & _
                vbLf & strProc & vbLf
            Else
                VBFindLines = VBFindLines & "[" & RID_STR_COMMON_VB_COMPONENT_MODULE & " " & strFind & _
                    " - " & lngStartLine & " ]" & vbLf
            End If
        End If
        count = count + 1
        
CONTINUE_LOOP:
        'Reset Params to search for next hit
        lngStartLine = lngEndLine + 1
        lngStartCol = 1
        lngEndLine = vbcm.CountOfLines
        lngEndCol = Len(vbcm.Lines(vbcm.CountOfLines, 1))
        
        If lngStartLine >= lngEndLine Then Exit Function
        
    Loop 'End - Do While vbcm.Find
    VBFindLines = VBFindLines
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function
Function VBNumLines(docAnalysis As DocumentAnalysis, vbcm As CodeModule) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "VBNumLines"
    Dim cLines As Long
    Dim lngType As Long
    Dim strProc As String
    
    'Issue: Just give line count in module to be in sync with Macro Analysis and Migration Wizard
    VBNumLines = vbcm.CountOfLines
    
    'For cLines = 1 To vbcm.CountOfLines
    '    strProc = vbcm.ProcOfLine(cLines, lngType)
    '    If strProc <> "" Then
    '        VBNumLines = VBNumLines - _
    '            (vbcm.ProcBodyLine(strProc, lngType) - vbcm.ProcStartLine(strProc, lngType))
    '        cLines = cLines + vbcm.ProcCountLines(strProc, lngType) - 1
    '    End If
    'Next
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function
Function VBNumFuncs(docAnalysis As DocumentAnalysis, vbcm As CodeModule) As Long
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "VBNumFuncs"
    Dim cLines As Long
    Dim lngType As Long
    Dim strProc As String
    
    For cLines = 1 To vbcm.CountOfLines
        strProc = vbcm.ProcOfLine(cLines, lngType)
        If strProc <> "" Then
            VBNumFuncs = VBNumFuncs + 1
            cLines = cLines + vbcm.ProcCountLines(strProc, lngType) - 1
        End If
    Next
    Exit Function
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Function VBComponentType(vbc As VBComponent) As String
    Select Case vbc.Type
        Case vbext_ct_StdModule
            VBComponentType = RID_STR_COMMON_VB_COMPONENT_STANDARD
        Case vbext_ct_ClassModule
            VBComponentType = RID_STR_COMMON_VB_COMPONENT_CLASS
        Case vbext_ct_MSForm
            VBComponentType = RID_STR_COMMON_VB_COMPONENT_USER_FORM
        Case vbext_ct_Document
            VBComponentType = RID_STR_COMMON_VB_COMPONENT_DOCUMENT
        Case 11 'vbext_ct_ActiveX Designer
            VBComponentType = RID_STR_COMMON_VB_COMPONENT_ACTIVEX_DESIGNER
        Case Else
            VBComponentType = RID_STR_COMMON_UNKNOWN
    End Select
End Function

Function CheckEmptyProject(docAnalysis As DocumentAnalysis, myProject As VBProject, myComponent As VBComponent) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckEmptyProject"
    Dim bEmptyProject As Boolean
    
    'Bug: Can have empty project with different name from default, would be picked up
    ' as not empty.
    'bEmptyProject = _
    '        (StrComp(myProject.name, CTOPLEVEL_PROJECT) = 0) And _
    '        (VBNumFuncs(docAnalysis, myComponent.CodeModule) = 0) And _
    '        (VBNumLines(docAnalysis, myComponent.CodeModule) < 3)
    
    ' Code Modules
    Dim strFind As String
    Dim count As Long
    'Check for:
    'Public Const myFoo ....
    'Public Declare Function ....
    'Public myVar As ...
    strFind = VBFindLines(docAnalysis, myComponent.CodeModule, "Public", _
        count, bInProcedure:=False, bWholeWord:=True, bMatchCase:=True)
        
    bEmptyProject = _
            (VBNumFuncs(docAnalysis, myComponent.CodeModule) = 0) And _
            (VBNumLines(docAnalysis, myComponent.CodeModule) < 3) And _
            (strFind = "")
            
    CheckEmptyProject = IIf(bEmptyProject, True, False)
    Exit Function


HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Function getCustomDocPropTypeAsString(propType As MsoDocProperties)
    Dim Str As String

    Select Case propType
    Case msoPropertyTypeBoolean
        Str = RID_STR_COMMON_YES_OR_NO
    Case msoPropertyTypeDate
        Str = RID_STR_COMMON_DATE
    Case msoPropertyTypeFloat
        Str = RID_STR_COMMON_NUMBER
    Case msoPropertyTypeNumber
        Str = RID_STR_COMMON_NUMBER
    Case msoPropertyTypeString
        Str = RID_STR_COMMON_TEXT
    Case Else
        Str = "Unknown"
    End Select
    
    getCustomDocPropTypeAsString = Str
End Function

Sub HandleProtectedDocInvalidPassword(docAnalysis As DocumentAnalysis, strError As String, fso As FileSystemObject)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "HandleProtectedDocInvalidPassword"
    Dim f As File
    Set f = fso.GetFile(docAnalysis.name)

    docAnalysis.Application = RID_STR_COMMON_PASSWORD_SKIPDOC

    On Error Resume Next
    docAnalysis.PageCount = 0
    docAnalysis.Created = f.DateCreated
    docAnalysis.Modified = f.DateLastModified
    docAnalysis.Accessed = f.DateLastAccessed
    docAnalysis.Printed = DateValue("01/01/1900")
    docAnalysis.SavedBy = RID_STR_COMMON_NA
    docAnalysis.Revision = 0
    docAnalysis.Template = RID_STR_COMMON_NA
    On Error GoTo HandleErrors

    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo

    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_STR_COMMON_SUBISSUE_INVALID_PASSWORD_ENTERED
        .Location = .CLocationDocument

        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_INVALID_PASSWORD_ENTERED
        .locationXML = .CXMLLocationDocument

        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_PASSWORD
        .Values.Add strError

        docAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                docAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
    End With

    docAnalysis.Issues.Add myIssue

FinalExit:
    Set myIssue = Nothing
    Set f = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_OLEEmbeddedSingleShape(docAnalysis As DocumentAnalysis, aShape As Shape, mySubLocation As Variant)

    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_OLEEmbeddedSingleShape"
    Dim myIssue As IssueInfo
    Dim bOleObject As Boolean
    Dim TypeAsString As String
    Dim XMLTypeAsString As String
    Dim objName As String
    
    bOleObject = (aShape.Type = msoEmbeddedOLEObject) Or _
                    (aShape.Type = msoLinkedOLEObject) Or _
                    (aShape.Type = msoOLEControlObject)
                    
    If Not bOleObject Then Exit Sub
            
    aShape.Select
    Select Case aShape.Type
        Case msoEmbeddedOLEObject
            TypeAsString = RID_STR_COMMON_OLE_EMBEDDED
            XMLTypeAsString = CSTR_SUBISSUE_OLE_EMBEDDED
        Case msoLinkedOLEObject
            TypeAsString = RID_STR_COMMON_OLE_LINKED
            XMLTypeAsString = CSTR_SUBISSUE_OLE_LINKED
        Case msoOLEControlObject
            TypeAsString = RID_STR_COMMON_OLE_CONTROL
            XMLTypeAsString = CSTR_SUBISSUE_OLE_CONTROL
        Case Else
            TypeAsString = RID_STR_COMMON_OLE_UNKNOWN
            XMLTypeAsString = CSTR_SUBISSUE_OLE_UNKNOWN
    End Select
    
    Dim appStr As String
    appStr = getAppSpecificApplicationName
        
    Set myIssue = New IssueInfo
    With myIssue
        .IssueID = CID_PORTABILITY
        .IssueType = RID_STR_COMMON_ISSUE_PORTABILITY
        .SubType = TypeAsString
        .Location = .CLocationPage
        .SubLocation = mySubLocation
        
        .IssueTypeXML = CSTR_ISSUE_PORTABILITY
        .SubTypeXML = XMLTypeAsString
        .locationXML = .CXMLLocationPage
        
        .Line = aShape.top
        .column = aShape.Left

        If aShape.name <> "" Then
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
            .Values.Add aShape.name
        End If
        
        If aShape.Type = msoEmbeddedOLEObject Or _
           aShape.Type = msoOLEControlObject Then
            Dim objType As String
            On Error Resume Next
            
            objType = getAppSpecificOLEClassType(aShape)
            
            If objType = "" Then GoTo FinalExit
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_OBJECT_TYPE
            .Values.Add objType
                        
            If aShape.Type = msoOLEControlObject Then
                docAnalysis.MacroNumOLEControls = 1 + docAnalysis.MacroNumOLEControls
            End If
            
            If appStr = CAPPNAME_POWERPOINT Then
            '#114127: Too many open windows
            'Checking for OLEFormat.Object is Nothing or IsEmpty still causes problem
                If objType <> "Equation.3" Then
                    objName = aShape.OLEFormat.Object.name
                    If Err.Number = 0 Then
                        If aShape.name <> objName Then
                            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_OBJECT_NAME
                            .Values.Add objName
                       End If
                    End If
                End If
            Else
                If Not (aShape.OLEFormat.Object) Is Nothing Then
                    objName = aShape.OLEFormat.Object.name
                    If Err.Number = 0 Then
                        If aShape.name <> objName Then
                            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_OBJECT_NAME
                            .Values.Add objName
                        End If
                    End If
                End If
            End If
            
            On Error GoTo HandleErrors
        End If
        
        If aShape.Type = msoLinkedOLEObject Then
            If appStr <> CAPPNAME_WORD Then
                On Error Resume Next
                Dim path As String
                path = aShape.OLEFormat.Object.SourceFullName
                If Err.Number = 0 Then
                    .Attributes.Add RID_STR_COMMON_ATTRIBUTE_SOURCE
                    .Values.Add path
                End If
                On Error GoTo HandleErrors
            Else
                .Attributes.Add RID_STR_COMMON_ATTRIBUTE_SOURCE
                .Values.Add aShape.LinkFormat.SourceFullName
            End If
        End If
        
        docAnalysis.IssuesCountArray(CID_PORTABILITY) = _
            docAnalysis.IssuesCountArray(CID_PORTABILITY) + 1
    End With
    docAnalysis.Issues.Add myIssue
            
FinalExit:
    Set myIssue = Nothing
    Exit Sub
     
HandleErrors:
    WriteDebugLevelTwo currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_Lines(docAnalysis As DocumentAnalysis, myShape As Shape, mySubLocation As Variant)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Lines"

    If myShape.Line.Style = msoLineSingle Or _
       myShape.Line.Style = msoLineStyleMixed Then Exit Sub

    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo

    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_RESXLS_COST_LineStyle
        .Location = .CLocationPage
        .SubLocation = mySubLocation
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_LINE
        .locationXML = .CXMLLocationPage
        
        .Line = myShape.top
        .column = myShape.Left

        If myShape.name <> "" Then
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
            .Values.Add myShape.name
        End If

        AddIssueDetailsNote myIssue, 0, RID_STR_COMMON_SUBISSUE_LINE_NOTE

        docAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                docAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
    End With
       
    docAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_Transparency(docAnalysis As DocumentAnalysis, myShape As Shape, mySubLocation As Variant)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Transparency"

    If Not myShape.Type = msoPicture Then Exit Sub

    Dim bHasTransparentBkg
    bHasTransparentBkg = False

    On Error Resume Next
    If myShape.PictureFormat.TransparentBackground = msoTrue Then
        If Error.Number = 0 Then
            bHasTransparentBkg = True
        End If
    End If
    
    On Error GoTo HandleErrors
    If Not bHasTransparentBkg Then Exit Sub

    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    
    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_RESXLS_COST_Transparent
        .Location = .CLocationSlide
        .SubLocation = mySubLocation
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_TRANSPARENCY
        .locationXML = .CXMLLocationPage
        
        .Line = myShape.top
        .column = myShape.Left

        If myShape.name <> "" Then
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
            .Values.Add myShape.name
        End If

        AddIssueDetailsNote myIssue, 0, RID_STR_COMMON_SUBISSUE_TRANSPARENCY_NOTE

        docAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                docAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
    End With
       
    docAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_Gradients(docAnalysis As DocumentAnalysis, myShape As Shape, mySubLocation As Variant)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Gradients"

    If myShape.Fill.Type <> msoFillGradient Then Exit Sub
    
    Dim bUsesPresetGradient, bUsesFromCorner, bUsesFromCenter
    bUsesPresetGradient = False
    bUsesFromCorner = False
    bUsesFromCenter = False

    On Error Resume Next
    If myShape.Fill.PresetGradientType <> msoPresetGradientMixed Then
        If Error.Number = 0 Then
            bUsesPresetGradient = True
        End If
    End If
    If myShape.Fill.GradientStyle <> msoGradientFromCorner Then
        If Error.Number = 0 Then
            bUsesFromCorner = True
        End If
    End If
    If myShape.Fill.GradientStyle <> msoGradientFromCenter Then
        If Error.Number = 0 Then
            bUsesFromCenter = True
        End If
    End If
    
    On Error GoTo HandleErrors
    If Not bUsesPresetGradient And Not bUsesFromCorner _
       And Not bUsesFromCenter Then Exit Sub

    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    
    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_RESXLS_COST_GradientStyle
        .Location = .CLocationSlide
        .SubLocation = mySubLocation
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_GRADIENT
        .locationXML = .CXMLLocationSlide
        
        .Line = myShape.top
        .column = myShape.Left

        If myShape.name <> "" Then
            .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
            .Values.Add myShape.name
        End If

        If bUsesPresetGradient Then
            AddIssueDetailsNote myIssue, 0, RID_STR_COMMON_SUBISSUE_GRADIENT_PRESET_NOTE
        ElseIf bUsesFromCorner Then
            AddIssueDetailsNote myIssue, 0, RID_STR_COMMON_SUBISSUE_GRADIENT_CORNER_NOTE
        Else
            AddIssueDetailsNote myIssue, 0, RID_STR_COMMON_SUBISSUE_GRADIENT_CENTER_NOTE
        End If

        docAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                docAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
    End With
       
    docAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Private Function CreateFullPath(newPath As String, fso As FileSystemObject)
    'We don't want to create 'c:\'
    If (Len(newPath) < 4) Then
        Exit Function
    End If

    'Create parent folder first
    If (Not fso.FolderExists(fso.GetParentFolderName(newPath))) Then
        CreateFullPath fso.GetParentFolderName(newPath), fso
    End If

    If (Not fso.FolderExists(newPath)) Then
        fso.CreateFolder (newPath)
    End If
End Function

Function GetPreparedFullPath(sourceDocPath As String, startDir As String, storeToDir As String, _
    fso As FileSystemObject) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "GetPreparedFullPath"
    GetPreparedFullPath = ""

    Dim preparedPath As String
        
    preparedPath = Right(sourceDocPath, Len(sourceDocPath) - Len(startDir))
    If Left(preparedPath, 1) = "\" Then
        preparedPath = Right(preparedPath, Len(preparedPath) - 1)
    End If
    
    'Allow for root folder C:\
    If Right(storeToDir, 1) <> "\" Then
        preparedPath = storeToDir & "\" & CSTR_COMMON_PREPARATION_FOLDER & "\" & preparedPath
    Else
        preparedPath = storeToDir & CSTR_COMMON_PREPARATION_FOLDER & "\" & preparedPath
    End If
    
    'Debug: MsgBox "Preppath: " & preparedPath
    CreateFullPath fso.GetParentFolderName(preparedPath), fso

    'Only set if folder to save to exists or has been created, otherwise return ""
    GetPreparedFullPath = preparedPath
    
FinalExit:
    Exit Function
     
HandleErrors:
    WriteDebugLevelTwo currentFunctionName & " : " & sourceDocPath & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function ClassifyDocOverallMacroClass(docAnalysis As DocumentAnalysis) As EnumDocOverallMacroClass
    ClassifyDocOverallMacroClass = enMacroNone
    
    If Not docAnalysis.HasMacros Then Exit Function
    
    If (docAnalysis.MacroTotalNumLines >= CMACRO_LINECOUNT_MEDIUM_LBOUND) Then
        If (docAnalysis.MacroNumExternalRefs > 0) Or _
            (docAnalysis.MacroNumOLEControls > 0 Or docAnalysis.MacroNumFieldsUsingMacros > 0) Or _
            docAnalysis.MacroNumUserForms > 0 Then
            ClassifyDocOverallMacroClass = enMacroComplex
        Else
            ClassifyDocOverallMacroClass = enMacroMedium
        End If
    Else
        ClassifyDocOverallMacroClass = enMacroSimple
    End If
    
End Function

