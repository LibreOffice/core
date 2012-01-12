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


Private mAnalysis As DocumentAnalysis

'***ADDING-ISSUE: Use Following Skeleton as Guideline for Adding Issue
' For complete list of all RID_STR_... for Issues (IssueType), SubIssues (SubType) and Attributes refer to:
'   powerpoint_res.bas and common_res.bas
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
    Dim containsInvalidChar As Boolean
    containsInvalidChar = False
    Dim currentFunctionName As String
    currentFunctionName = "DoAnalyse"
    mAnalysis.name = fileName
    Dim aPres As Presentation
    mAnalysis.TotalIssueTypes = CTOTAL_CATEGORIES
    
    If InStr(fileName, "[") = 0 And InStr(fileName, "]") = 0 Then 'If fileName does not contain [ AND ]
        containsInvalidChar = False
    Else
        containsInvalidChar = True
    End If
    
    'Cannot Turn off any AutoExce macros before loading the Presentation
    'WordBasic.DisableAutoMacros 1
    'On Error GoTo HandleErrors
 
    On Error Resume Next ' Ignore errors on setting
    If containsInvalidChar = True Then
        GoTo HandleErrors
    End If
    Set aPres = Presentations.Open(fileName:=fileName, ReadOnly:=True)
    If Err.Number <> 0 Then
        mAnalysis.Application = RID_STR_COMMON_CANNOT_OPEN
        GoTo HandleErrors
    End If
    On Error GoTo HandleErrors
    
    'MsgBox "Window: " & PPViewType(aPres.Windows(1).viewType) & _
    '    " Pane: " & PPViewType(aPres.Windows(1).ActivePane.viewType)
        
    'Set Doc Properties
    SetDocProperties mAnalysis, aPres, fso

    Analyze_SlideIssues aPres
    Analyze_Macros mAnalysis, userFormTypesDict, aPres
    
    ' Doc Preparation only
    ' Save document with any fixed issues under <storeToDir>\prepared\<source doc name>
    If mAnalysis.PreparableIssuesCount > 0 And CheckDoPrepare Then
        Dim preparedFullPath As String
        preparedFullPath = GetPreparedFullPath(mAnalysis.name, startDir, storeToDir, fso)
        If preparedFullPath <> "" Then
            If fso.FileExists(preparedFullPath) Then
                fso.DeleteFile preparedFullPath, True
            End If
            If fso.FolderExists(fso.GetParentFolderName(preparedFullPath)) Then
                aPres.SaveAs preparedFullPath
            End If
        End If
    End If

FinalExit:
    If Not aPres Is Nothing Then 'If Not IsEmpty(aDoc) Then
        aPres.Saved = True
        aPres.Close
    End If
    Set aPres = Nothing
    Exit Sub

HandleErrors:
    If containsInvalidChar = False Then
        WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Else
        WriteDebug currentFunctionName & " : " & mAnalysis.name & ": The file name contains the invalid character [ or ].  Please change the file name and run analysis again."
    End If
    Resume FinalExit
End Sub

Sub SetDocProperties(docAnalysis As DocumentAnalysis, pres As Presentation, fso As FileSystemObject)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetDocProperties"
    Dim f As File
    Set f = fso.GetFile(docAnalysis.name)
    
    Const appPropertyAppName = 9
    Const appPropertyLastAuthor = 7
    Const appPropertyRevision = 8
    Const appPropertyTemplate = 6
    Const appPropertyTimeCreated = 11
    Const appPropertyTimeLastSaved = 12

    On Error Resume Next
    docAnalysis.PageCount = pres.Slides.count
    docAnalysis.Created = f.DateCreated
    docAnalysis.Modified = f.DateLastModified
    docAnalysis.Accessed = f.DateLastAccessed
    docAnalysis.Printed = DateValue("01/01/1900")

    On Error Resume Next 'Some apps may not support all props
    DocAnalysis.Application = getAppSpecificApplicationName & " " & Application.Version
    
    'docAnalysis.Application = pres.BuiltInDocumentProperties(appPropertyAppName)
    'If InStr(docAnalysis.Application, "Microsoft") = 1 Then
    '    docAnalysis.Application = Mid(docAnalysis.Application, Len("Microsoft") + 2)
    'End If
    'If InStr(Len(docAnalysis.Application) - 2, docAnalysis.Application, ".") = 0 Then
    '    docAnalysis.Application = docAnalysis.Application & " " & Application.Version
    'End If
  
    docAnalysis.SavedBy = _
        pres.BuiltInDocumentProperties(appPropertyLastAuthor)
    docAnalysis.Revision = _
        val(pres.BuiltInDocumentProperties(appPropertyRevision))
    docAnalysis.Template = _
        fso.GetFileName(pres.BuiltInDocumentProperties(appPropertyTemplate))
        
FinalExit:
    Set f = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Function PPViewType(viewType As PPViewType) As String

    Select Case viewType
        Case ppViewHandoutMaster
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_HANDOUT_MASTER
        Case ppViewNormal
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_NORMAL
        Case ppViewNotesMaster
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_NOTES_MASTER
        Case ppViewNotesPage
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_NOTES_PAGE
        Case ppViewOutline
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_OUTLINE
        Case ppViewSlide
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_SLIDE
        Case ppViewSlideMaster
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_SLIDE_MASTER
        Case ppViewSlideSorter
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_SLIDE_SORTER
        Case ppViewTitleMaster
            PPViewType = RID_STR_PP_ENUMERATION_VIEW_TITLE_MASTER
        Case Else
            PPViewType = RID_STR_PP_ENUMERATION_UNKNOWN
    End Select
End Function

Sub Analyze_SlideIssues(curPresentation As Presentation)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_SlideIssues"

    Dim mySlide As Slide
    Dim SlideNum As Integer

    SlideNum = 1
    For Each mySlide In curPresentation.Slides
        ActiveWindow.View.GotoSlide index:=SlideNum
        Analyze_ShapeIssues mySlide
        Analyze_Hyperlinks mySlide
        Analyze_Templates mySlide
        SlideNum = SlideNum + 1
    Next mySlide

    Analyze_TabStops curPresentation

    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub Analyze_TabStops(curPresentation As Presentation)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_TabStops"

    'Dim firstSlide As Slide
    'Dim firstShape As Shape
    Dim mySlide As Slide
    Dim myShape As Shape
    Dim bInitialized, bHasDifferentDefaults As Boolean
    Dim curDefault, lastDefault As Single

    bInitialized = False
    bHasDifferentDefaults = False

    For Each mySlide In curPresentation.Slides
        For Each myShape In mySlide.Shapes
            If myShape.HasTextFrame Then
                If myShape.TextFrame.HasText Then
                    curDefault = myShape.TextFrame.Ruler.TabStops.DefaultSpacing
                    If Not bInitialized Then
                        bInitialized = True
                        lastDefault = curDefault
                        'Set firstSlide = mySlide
                        'Set firstShape = myShape
                    End If
                    If curDefault <> lastDefault Then
                        bHasDifferentDefaults = True
                        Exit For
                    End If
                End If
            End If
        Next myShape
        If bHasDifferentDefaults Then Exit For
    Next mySlide

    If Not bHasDifferentDefaults Then Exit Sub

    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo

    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_RESXLS_COST_Tabstop
        .Location = .CLocationSlide
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_TABSTOP
        .locationXML = .CXMLLocationSlide
        
        .SubLocation = mySlide.name
        .Line = myShape.top
        .column = myShape.Left

        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
        .Values.Add myShape.name

        AddIssueDetailsNote myIssue, 0, RID_STR_PP_SUBISSUE_TABSTOP_NOTE

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

Sub Analyze_Fonts(curPresentation As Presentation)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Fonts"

    Dim myFont As Font
    Dim bHasEmbeddedFonts As Boolean
    
    bHasEmbeddedFonts = False
    For Each myFont In curPresentation.Fonts
        If myFont.Embedded Then
            bHasEmbeddedFonts = True
            Exit For
        End If
    Next

    If Not bHasEmbeddedFonts Then Exit Sub

    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo

    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_STR_PP_SUBISSUE_FONTS
        .Location = .CLocationSlide
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_FONTS
        .locationXML = .CXMLLocationSlide
        
        .SubLocation = mySlide.name
        .Line = myShape.top
        .column = myShape.Left

        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
        .Values.Add myShape.name

        AddIssueDetailsNote myIssue, 0, RID_STR_PP_SUBISSUE_FONTS_NOTE

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

Sub Analyze_Templates(mySlide As Slide)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Templates"

    If mySlide.Layout <> ppLayoutTitle Then Exit Sub

    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo

    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_RESXLS_COST_Template
        .Location = .CLocationSlide

        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_TEMPLATE
        .locationXML = .CXMLLocationSlide
        .SubLocation = mySlide.name

        '.Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
        '.Values.Add mySlide.name

        AddIssueDetailsNote myIssue, 0, RID_STR_PP_SUBISSUE_TEMPLATE_NOTE

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

Sub Analyze_Hyperlinks(mySlide As Slide)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Hyperlinks"

    Dim myIssue As IssueInfo
    Dim hl As Hyperlink
    Dim bHasMultipleFonts As Boolean
    Dim bHasMultipleLines As Boolean

    bHasMultipleFonts = False
    bHasMultipleLines = False
        
    For Each hl In mySlide.Hyperlinks
        If TypeName(hl.Parent.Parent) = "TextRange" Then
            Dim myTextRange As TextRange
            Dim currRun As TextRange
            Dim currLine As TextRange
            Dim first, last, noteCount As Long

            Set myTextRange = hl.Parent.Parent
            first = myTextRange.start
            last = first + myTextRange.Length - 1
            
            For Each currRun In myTextRange.Runs
                If (currRun.start > first And currRun.start < last) Then
                    bHasMultipleFonts = True
                    Exit For
                End If
            Next

            For Each currLine In myTextRange.Lines
                Dim lineEnd As Long
                lineEnd = currLine.start + currLine.Length - 1
                If (first <= lineEnd And last > lineEnd) Then
                    bHasMultipleLines = True
                    Exit For
                End If
            Next
        End If

        noteCount = 0
        
        If bHasMultipleFonts Then
            Set myIssue = New IssueInfo

            With myIssue
                .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
                .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
                .SubType = RID_RESXLS_COST_Hyperlink
                .Location = .CLocationSlide

                .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
                .SubTypeXML = CSTR_SUBISSUE_HYPERLINK
                .locationXML = .CXMLLocationSlide
                .SubLocation = mySlide.name

                .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
                .Values.Add myTextRange.Text

                AddIssueDetailsNote myIssue, 0, RID_STR_PP_SUBISSUE_HYPERLINK_NOTE

                mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                        mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
            End With
            mAnalysis.Issues.Add myIssue
            Set myIssue = Nothing
            bHasMultipleFonts = False
        End If
        If bHasMultipleLines Then
            Set myIssue = New IssueInfo

            With myIssue
                .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
                .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
                .SubType = RID_RESXLS_COST_HyperlinkSplit
                .Location = .CLocationSlide

                .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
                .SubTypeXML = CSTR_SUBISSUE_HYPERLINK_SPLIT
                .locationXML = .CXMLLocationSlide
                .SubLocation = mySlide.name

                .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
                .Values.Add myTextRange.Text

                AddIssueDetailsNote myIssue, 0, RID_STR_PP_SUBISSUE_HYPERLINK_SPLIT_NOTE

                mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                        mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
            End With
            mAnalysis.Issues.Add myIssue
            Set myIssue = Nothing
            bHasMultipleLines = False
        End If
    Next

FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_ShapeIssues(mySlide As Slide)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_ShapeIssues"
    Dim myShape As Shape
    
    For Each myShape In mySlide.Shapes
        'myShape.Select msoTrue
        Analyze_Movie mySlide, myShape
        Analyze_Comments mySlide, myShape
        Analyze_Background mySlide, myShape
        Analyze_Numbering mySlide, myShape
        'Analyze global issues
        Analyze_OLEEmbeddedSingleShape mAnalysis, myShape, mySlide.name
        Analyze_Lines mAnalysis, myShape, mySlide.name
        Analyze_Transparency mAnalysis, myShape, mySlide.name
        Analyze_Gradients mAnalysis, myShape, mySlide.name
    Next myShape
    
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub Analyze_Numbering(mySlide As Slide, myShape As Shape)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Numbering"

    If Not myShape.HasTextFrame Then Exit Sub
    If Not myShape.TextFrame.HasText Then Exit Sub
    Dim shapeText As TextRange

    Set shapeText = myShape.TextFrame.TextRange

    If shapeText.Paragraphs.count < 2 Then Exit Sub
    If Not (shapeText.ParagraphFormat.Bullet.Type = ppBulletMixed Or _
            shapeText.ParagraphFormat.Bullet.Type = ppBulletNumbered) Then Exit Sub

    ' OpenOffice has Problems when the numbering does not start with the first
    ' paragraph or when there are empty paragraphs which do not have a number.
    ' Because PowerPoint does not give us the length of each paragraph ( .Length
    ' does not work ), we have to compute the length ourself.

    Dim I As Long
    Dim lastType As PpBulletType
    Dim currType As PpBulletType
    Dim lastStart As Long
    Dim lastLength As Long
    Dim currStart As Long
    Dim bHasNumProblem As Boolean
    Dim bHasEmptyPar As Boolean

    bHasNumProblem = False
    bHasEmptyPar = False

    lastType = shapeText.Paragraphs(1, 0).ParagraphFormat.Bullet.Type
    lastStart = shapeText.Paragraphs(1, 0).start

    For I = 2 To shapeText.Paragraphs.count
        currType = shapeText.Paragraphs(I, 0).ParagraphFormat.Bullet.Type
        currStart = shapeText.Paragraphs(I, 0).start
        lastLength = currStart - lastStart - 1

        If currType <> lastType Then
            lastType = currType
            If currType = ppBulletNumbered Then
                bHasNumProblem = True
                Exit For
            End If
        End If
        If lastLength = 0 Then
            bHasEmptyPar = True
        Else
            If (bHasEmptyPar) Then
                bHasNumProblem = True
                Exit For
            End If
        End If
        lastStart = currStart
    Next I

    lastLength = shapeText.Length - lastStart
    If (lastLength <> 0) And bHasEmptyPar Then
        bHasNumProblem = True
    End If

    If Not bHasNumProblem Then Exit Sub

    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo

    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_RESXLS_COST_Numbering
        .Location = .CLocationSlide

        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_NUMBERING
        .locationXML = .CXMLLocationSlide

        .SubLocation = mySlide.name
        .Line = myShape.top
        .column = myShape.Left

        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
        .Values.Add myShape.name

        AddIssueDetailsNote myIssue, 0, RID_STR_PP_SUBISSUE_NUMBERING_NOTE

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

Sub Analyze_Background(mySlide As Slide, myShape As Shape)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Background"
    
    If myShape.Fill.Type <> msoFillBackground Then Exit Sub
    
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    Dim strCr As String
    strCr = "" & vbCr
    
    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_RESXLS_COST_Background
        .Location = .CLocationSlide
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_BACKGROUND
        .locationXML = .CXMLLocationSlide
        
        .SubLocation = mySlide.name
        .Line = myShape.top
        .column = myShape.Left

        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
        .Values.Add myShape.name

        AddIssueDetailsNote myIssue, 0, RID_STR_PP_SUBISSUE_BACKGROUND_NOTE

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

Sub Analyze_Comments(mySlide As Slide, myShape As Shape)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Comments"
    
    If myShape.Type <> msoComment Then Exit Sub
    
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    Dim strCr As String
    strCr = "" & vbCr
    
    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_STR_PP_SUBISSUE_COMMENT
        .Location = .CLocationSlide
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_COMMENT
        .locationXML = .CXMLLocationSlide
        
        .SubLocation = mySlide.name
        .Line = myShape.top
        .column = myShape.Left

        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
        .Values.Add myShape.name
        .Attributes.Add RID_STR_PP_ATTRIBUTE_CONTENT
        .Values.Add Replace(myShape.TextFrame.TextRange.Text, strCr, "")

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
Sub Analyze_Movie(mySlide As Slide, myShape As Shape)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Movie"
    
    If myShape.Type <> msoMedia Then Exit Sub
    If myShape.MediaType <> ppMediaTypeMovie Then Exit Sub
    
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    
    With myIssue
        .IssueID = CID_OBJECTS_GRAPHICS_TEXTBOXES
        .IssueType = RID_STR_PP_ISSUE_OBJECTS_GRAPHICS_AND_TEXTBOXES
        .SubType = RID_STR_PP_SUBISSUE_MOVIE
        .Location = .CLocationSlide
        
        .IssueTypeXML = CSTR_ISSUE_OBJECTS_GRAPHICS_AND_TEXTBOXES
        .SubTypeXML = CSTR_SUBISSUE_MOVIE
        .locationXML = .CXMLLocationSlide
        
        .SubLocation = mySlide.name
        .Line = myShape.top
        .column = myShape.Left

        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_NAME
        .Values.Add myShape.name
        .Attributes.Add RID_STR_COMMON_ATTRIBUTE_SOURCE
        .Values.Add myShape.LinkFormat.SourceFullName
        .Attributes.Add RID_STR_PP_ATTRIBUTE_PLAYONENTRY
        .Values.Add IIf(myShape.AnimationSettings.PlaySettings.PlayOnEntry, RID_STR_PP_TRUE, RID_STR_PP_FALSE)
        .Attributes.Add RID_STR_PP_ATTRIBUTE_LOOP
        .Values.Add IIf(myShape.AnimationSettings.PlaySettings.LoopUntilStopped, RID_STR_PP_TRUE, RID_STR_PP_FALSE)
        .Attributes.Add RID_STR_PP_ATTRIBUTE_REWIND
        .Values.Add IIf(myShape.AnimationSettings.PlaySettings.RewindMovie, RID_STR_PP_TRUE, RID_STR_PP_FALSE)

         mAnalysis.IssuesCountArray(CID_OBJECTS_GRAPHICS_TEXTBOXES) = _
                mAnalysis.IssuesCountArray(CID_OBJECTS_GRAPHICS_TEXTBOXES) + 1
    End With
       
    mAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Private Sub Class_Initialize()
    Set mAnalysis = New DocumentAnalysis
End Sub
Private Sub Class_Terminate()
    Set mAnalysis = Nothing
End Sub

Public Property Get Results() As DocumentAnalysis
    Set Results = mAnalysis
End Property

