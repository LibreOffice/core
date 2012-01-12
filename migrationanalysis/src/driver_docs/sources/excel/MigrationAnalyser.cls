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

Const CWORKBOOK_SHEETS_LIMIT = 256

'Class variables
Private Enum HFIssueType
    hfInline
    hfShape
    hfFrame
End Enum

Private Enum HFIssueLocation
    hfHeader
    hfFooter
End Enum

Private Type CellAtrributes
    LineStyle As Integer
    FillPattern As Integer
End Type
    
Private Type BadSheetNameChar
    BadChar As String
    Position As Integer
End Type

Private mAnalysis As DocumentAnalysis
Private mFileName As String

Const RID_STR_EXCEL_SUBISSUE_ERROR_TYPE = "ERROR.TYPE"
Const RID_STR_EXCEL_SUBISSUE_INFO = "INFO"
Const RID_STR_EXCEL_SUBISSUE_DATEDIF = "DATEDIF"
Const RID_STR_EXCEL_SUBISSUE_PHONETIC = "PHONETIC"
Const FontError = 94
Const CR_BADCHAR = "<TOKEN1>"
Const CR_BADCHARNUM = "<TOKEN2>"
Const DATA_SOURCE_EXCEL = 0
Const DATA_SOURCE_EXTERNAL = 1
Const DATA_SOURCE_MULTIPLE = 2
Const DATA_SOURCE_EXTERNAL_FILE = 3
Const C_MAX_CELL_RANGE_COUNT = 10000

Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)

'***ADDING-ISSUE: Use Following Skeleton as Guideline for Adding Issue
' For complete list of all RID_STR_... for Issues (IssueType), SubIssues (SubType) and Attributes refer to:
'   excel_res.bas and common_res.bas
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
    Dim currentFunctionName As String
    currentFunctionName = "DoAnalyse"
    'Dim secAutomation As MsoAutomationSecurity
    'secAutomation = Application.AutomationSecurity

    mAnalysis.name = fileName
    Dim aWB As Workbook
    mAnalysis.TotalIssueTypes = CTOTAL_CATEGORIES
    
    'Make Excel run as non interactively as possible
    Application.EnableEvents = False
    Application.DisplayAlerts = False
    Application.Interactive = False
    Application.AskToUpdateLinks = False
    Application.EnableAnimations = False
    Application.EnableSound = False
    
    'Only supported in Office XP and above
    'Application.AutomationSecurity = msoAutomationSecurityForceDisable
    'mFileName = fso.GetFileName(fileName)
    'WriteToLog "TmpDebug1", mFileName
    
    Dim myPassword As String

    myPassword = GetDefaultPassword

    If myPassword = "" Then
        myPassword = "xoxoxoxoxo"
    End If
    
    Set aWB = Workbooks.Open(fileName:=fileName, _
                             Password:=myPassword, _
                             WriteResPassword:=myPassword, _
                             UpdateLinks:=0)
    
    'Application.AutomationSecurity = secAutomation

    'Do Analysis
    Analyze_Password_Protection aWB
    Analyze_Workbook_Protection aWB
    
    'Set Doc Properties
    SetDocProperties mAnalysis, aWB, fso
    
    Analyze_SheetLimits aWB
    Analyze_SheetDisplay aWB
    Analyze_SheetIssues aWB
    Analyze_SheetCharts aWB
    Analyze_WorkbookVersion aWB
    Analyze_Macros mAnalysis, userFormTypesDict, aWB
    
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
                If IsOldVersion(aWB.FileFormat) Then
                    aWB.SaveAs fileName:=preparedFullPath, FileFormat:=xlExcel9795
                Else
                    aWB.SaveAs preparedFullPath
                End If
            End If
        End If
    End If

FinalExit:
    If Not aWB Is Nothing Then
        aWB.Close (False)
    End If
    
    Set aWB = Nothing
    
    Application.EnableEvents = True
    Application.DisplayAlerts = True
    Application.Interactive = True
    Application.AskToUpdateLinks = True
    Application.EnableAnimations = True
    Application.EnableSound = True
    
    'Debug - Call Sleep(5000)
    Exit Sub

HandleErrors:
    ' MsgBox currentFunctionName & " : " & fileName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    ' Handle Password error on Doc Open, Modify and Cancel
    If Err.Number = 1004 Then
        WriteDebug currentFunctionName & " : " & fileName & ": " & _
          "User entered Invalid Document Password - " & Err.Number & " " & Err.Description & " " & Err.Source
        HandleProtectedDocInvalidPassword mAnalysis, _
            "User entered Invalid Document Password, further analysis not possible", fso
        Resume FinalExit
    End If
    mAnalysis.Application = RID_STR_COMMON_CANNOT_OPEN
    WriteDebug currentFunctionName & " : " & fileName & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_SheetCharts(aWB As Workbook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_SheetCharts"
    
    Dim myChartSheet As Chart
    
    For Each myChartSheet In aWB.Charts
        SetChartIssueMinor myChartSheet, myChartSheet.name, False
        SetChartIssueComplex myChartSheet, myChartSheet.name
    Next myChartSheet
    
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub Analyze_EmbeddedCharts(mySheet As Worksheet)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_EmbeddedCharts"
    Dim BorderIssue As Boolean
            
    Dim index As Integer
    BorderIssue = False
    Dim chartcount As Integer
    Dim myChart As Chart
    
    chartcount = mySheet.ChartObjects.count
    
    For index = 1 To chartcount
        BorderIssue = False
        With mySheet.ChartObjects(index)
            If .Border.LineStyle <> xlLineStyleNone Then
                BorderIssue = True
            End If
            SetChartIssueMinor .Chart, mySheet.name, BorderIssue
            'If Not ((.ChartType = xlSurface) _
            '    And (.ChartType = xlSurfaceTopViewWireframe) _
            '    And (.ChartType = xlSurfaceTopView)) Then
                SetChartIssueComplex .Chart, mySheet.name
            'End If
        End With
    Next index
        
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Private Function getType(o As Variant) As Integer
    If (VarType(o) = vbString) Then
        Dim aDataSource As String
        aDataSource = o
        getType = DATA_SOURCE_EXCEL
        If (Len(aDataSource) > 0) Then
            Dim nBackslashPos As Long
            nBackslashPos = InStr(Trim(aDataSource), "\")
            If (nBackslashPos > 0 And nBackslashPos < 4) Then
                getType = DATA_SOURCE_EXTERNAL_FILE
            End If
        End If
    ElseIf (IsArray(o)) Then
        If (hasSecondDimension(o)) Then
            getType = DATA_SOURCE_MULTIPLE
        Else
            getType = DATA_SOURCE_EXTERNAL
        End If
    End If
End Function

Private Function hasSecondDimension(o2 As Variant) As Boolean
    On Error GoTo njet
    Dim temp As Integer
    temp = UBound(o2, 2)
    hasSecondDimension = True
    Exit Function
njet:
    hasSecondDimension = False
End Function

Private Sub Analyze_PivotTable(myIssue As IssueInfo, myPivotTable As PivotTable)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyse_PivotTable"
    
    Dim aPivotField As PivotField
    Dim aNoteCount As Long
    Dim bManualSort As Boolean
    Dim bCalculatedValues As Boolean
    Dim aSorting As XlSortOrder
    Dim nCount As Integer
    Dim nDataSource As Integer
    
    bManualSort = False
    bCalculatedValues = False

    For Each aPivotField In myPivotTable.PivotFields
        aSorting = xlAscending
        
        On Error Resume Next 'some fields don't have any property at all
        aSorting = aPivotField.AutoSortOrder
        On Error GoTo HandleErrors
        
        If (aSorting = xlManual) Then
            bManualSort = True
        End If

        nCount = 0
        
        On Error Resume Next 'some fields don't have any property at all
        nCount = aPivotField.CalculatedItems.count
        On Error GoTo HandleErrors
        
        If (nCount > 0) Then
            bCalculatedValues = True
        End If
    Next
    
    nCount = 0
    
    On Error Resume Next 'some fields don't have any property at all
    nCount = myPivotTable.CalculatedFields.count
    On Error GoTo HandleErrors
    
    If (nCount > 0) Then
        bCalculatedValues = True
    End If

    nDataSource = getType(myPivotTable.SourceData)
    
    aNoteCount = 0

    If (bManualSort) Then
        AddIssueDetailsNote myIssue, aNoteCount, RID_RESXLT_COST_PIVOT_ManSort_Comment
        aNoteCount = aNoteCount + 1
    End If

    If (nDataSource = DATA_SOURCE_EXTERNAL) Then
        AddIssueDetailsNote myIssue, aNoteCount, RID_RESXLT_COST_PIVOT_ExternData_Comment
        aNoteCount = aNoteCount + 1
    ElseIf (nDataSource = DATA_SOURCE_MULTIPLE) Then
        AddIssueDetailsNote myIssue, aNoteCount, RID_RESXLT_COST_PIVOT_MultConsRanges_Comment
        aNoteCount = aNoteCount + 1
    ElseIf (nDataSource = DATA_SOURCE_EXTERNAL_FILE) Then
        Dim noteString As String
        noteString = RID_RESXLT_COST_PIVOT_ExternData_Comment & "[" & _
                     myPivotTable.SourceData & "]"
        AddIssueDetailsNote myIssue, aNoteCount, noteString
        aNoteCount = aNoteCount + 1
    End If
    
    If (bCalculatedValues) Then
        AddIssueDetailsNote myIssue, aNoteCount, RID_RESXLT_COST_PIVOT_CalcVal_Comment
        aNoteCount = aNoteCount + 1
    End If

FinalExit:
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Private Sub SetChartIssueComplex(myChart As Chart, myName As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetChartIssueComplex"
    
    Dim myIssue As IssueInfo
    Dim bSeriesChartTypeChanged As Boolean
    Dim bDatasourceNotLinkedtoCell As Boolean
    Dim bDatasourceOnDifferentSheet As Boolean
    Dim bCategoryandValue As Boolean
    Dim bCLabelMorethanOneCell As Boolean
    Dim bOneColumnRow As Boolean
    Dim bDataTable As Boolean
    Dim bXAxes As Boolean
    Dim bseries As Boolean
    Dim bformat As Boolean
    Dim bpivot As Boolean
    
    
    Set myIssue = New IssueInfo
    bSeriesChartTypeChanged = False
    bDatasourceNotLinkedtoCell = False
    bDatasourceOnDifferentSheet = False
    bCategoryandValue = False
    bCLabelMorethanOneCell = False
    bOneColumnRow = False
    bDataTable = False
    bXAxes = False

    bformat = FormatIssueComplex(myChart, bDataTable, bXAxes)
    bseries = SeriesIssue(myChart, bSeriesChartTypeChanged, bDatasourceNotLinkedtoCell, bDatasourceOnDifferentSheet, bCategoryandValue, bCLabelMorethanOneCell, bOneColumnRow)
    bpivot = Not (myChart.PivotLayout Is Nothing)

    If (Not (bseries Or bformat Or bpivot)) Then
        GoTo FinalExit
    ElseIf bpivot Then
        With myIssue
            .IssueID = CID_CHARTS_TABLES
            .IssueType = RID_STR_EXCEL_ISSUE_CHARTS_AND_TABLES
            .SubType = RID_STR_EXCEL_SUBISSUE_PIVOT
            .Location = .CLocationSheet
            .SubLocation = myName

            .IssueTypeXML = CSTR_ISSUE_CHARTS_TABLES
            .SubTypeXML = CSTR_SUBISSUE_CHART_PIVOT
            .locationXML = .CXMLLocationSheet
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PIVOT_TABLE_NAME
            .Values.Add myChart.PivotLayout.PivotTable.name
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PIVOT_FIELDS_VISIBLE
            .Values.Add myChart.HasPivotFields
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PIVOT_FIELDS_NUM
            .Values.Add myChart.PivotLayout.PivotTable.PivotFields.count
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_TYPE
            .Values.Add getChartTypeAsString(myChart.ChartType)
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_CHARTNAME
            .Values.Add myChart.name
        End With
        
        AddIssueDetailsNote myIssue, 0, RID_RESXLT_COST_PIVOT_PivotChart_Comment
        mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) = _
            mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) + 1
        mAnalysis.Issues.Add myIssue
        
        GoTo FinalExit
    Else
        With myIssue
            Dim NoteIndex As Long
            NoteIndex = 0
            
            .IssueID = CID_CHARTS_TABLES
            .IssueType = RID_STR_EXCEL_ISSUE_CHARTS_AND_TABLES
            .SubType = RID_STR_EXCEL_SUBISSUE_CHART_COMPLEX
            .Location = .CLocationSheet
            .SubLocation = myName
        
            .IssueTypeXML = CSTR_ISSUE_CHARTS_TABLES
            .SubTypeXML = CSTR_SUBISSUE_CHART_COMPLEX
            .locationXML = .CXMLLocationSheet
            
            If bDataTable Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_DATATABLE
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_DATATABLE
                NoteIndex = NoteIndex + 1
            End If
            If bXAxes Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_XAXISCATEGORY
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_TIMESCALE
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_XAXISCATEGORY
                NoteIndex = NoteIndex + 1
            End If
            If bSeriesChartTypeChanged Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_SERIESCHARTTYPE
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_CHANGED
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_SERIESCHARTTYPE
                NoteIndex = NoteIndex + 1
            End If
            If bDatasourceNotLinkedtoCell Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_DATASOURCE
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_DATASOURCENOTLINKEDTOCELL
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_DATASOURCENOTLINKEDTOCELL
                NoteIndex = NoteIndex + 1
            End If
            If bDatasourceOnDifferentSheet Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_DATASOURCE
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_DATASOURCEONDIFFERENTSHEET
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_DATASOURCEONDIFFERENTSHEET
                NoteIndex = NoteIndex + 1
            End If
            If bCategoryandValue Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_CATEGORYANDDATA
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_SEPARATE
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_CATEGORYANDDATA
                NoteIndex = NoteIndex + 1
            End If
            If bCLabelMorethanOneCell Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_CATEGORYLABEL
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_CATEGORYLABELMORETHANONECELL
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_CATEGORYLABELMORETHANONECELL
                NoteIndex = NoteIndex + 1
            End If
            If bOneColumnRow Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_COLUMNBAR
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_ONECOLUMNROW
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_COLUMNBAR
                NoteIndex = NoteIndex + 1
            End If
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_TYPE
            .Values.Add getChartTypeAsString(myChart.ChartType)
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_CHARTNAME
            .Values.Add myChart.name
        End With
    
        mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) = _
            mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) + 1
        mAnalysis.Issues.Add myIssue
    End If
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Private Sub SetChartIssueMinor(myChart As Chart, myName As String, BorderIssue As Boolean)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetChartIssueMinor"

    Dim myIssue As IssueInfo
    Dim bUnsupportedType As Boolean
    Dim bTrendline As Boolean
    Dim bDatalabelWithLegend As Boolean
    Dim bLegendPosition As Boolean
    Dim bTitleFont As Boolean
    Dim bPiechartDirection As Boolean
    Dim bAxisInterval As Boolean
    
    
    Set myIssue = New IssueInfo
    bUnsupportedType = False
    bTrendline = False
    bDatalabelWithLegend = False
    bLegendPosition = False
    bTitleFont = False
    bPiechartDirection = False
    bAxisInterval = False
    

    If (Not FormatissueMinor(myChart, bUnsupportedType, bTrendline, bDatalabelWithLegend, bLegendPosition, bTitleFont, bPiechartDirection, bAxisInterval)) And (Not BorderIssue) Then
        GoTo FinalExit
    Else
        With myIssue
            Dim NoteIndex As Long
            NoteIndex = 0
            
            .IssueID = CID_CHARTS_TABLES
            .IssueType = RID_STR_EXCEL_ISSUE_CHARTS_AND_TABLES
            
            .SubType = RID_STR_EXCEL_SUBISSUE_CHART_MINOR
            .Location = .CLocationSheet
            .SubLocation = myName
            
            .IssueTypeXML = CSTR_ISSUE_CHARTS_TABLES
            .SubTypeXML = CSTR_SUBISSUE_CHART_PIVOT
            .locationXML = .CXMLLocationSheet
            
            If bUnsupportedType Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_UNSUPPORTEDTYPE
                .Values.Add getChartTypeAsString(myChart.ChartType)
                ' bubble chart
                If (myChart.ChartType = xlBubble Or myChart.ChartType = xlBubble3DEffect) Then
                    AddIssueDetailsNote myIssue, NoteIndex, RID_RESXLT_COST_CHART_Bubble_Comment
                ' bar of pie and pie of pie chart
                ElseIf (myChart.ChartType = xlPieOfPie Or myChart.ChartType = xlBarOfPie) Then
                    AddIssueDetailsNote myIssue, NoteIndex, RID_RESXLT_COST_CHART_BarOfPie_Comment
                ' Scatter chart
                ElseIf (myChart.ChartType = xlXYScatter Or myChart.ChartType = xlXYScatterLines _
                        Or myChart.ChartType = xlXYScatterLinesNoMarkers _
                        Or myChart.ChartType = xlXYScatterSmooth _
                        Or myChart.ChartType = xlXYScatterSmoothNoMarkers) Then
                    AddIssueDetailsNote myIssue, NoteIndex, RID_RESXLT_COST_CHART_Scattered_Comment
                ' radar chart
                ElseIf (myChart.ChartType = xlRadarMarkers Or myChart.ChartType = xlRadar) Then
                    AddIssueDetailsNote myIssue, NoteIndex, RID_RESXLT_COST_CHART_Radar_Comment
                ' radar filled chart
                ElseIf (myChart.ChartType = xlRadarFilled) Then
                    AddIssueDetailsNote myIssue, NoteIndex, RID_RESXLT_COST_CHART_FilledRadar_Comment
                ' surface chart
                ElseIf (myChart.ChartType = xlSurface Or myChart.ChartType = xlSurfaceTopView _
                        Or myChart.ChartType = xlSurfaceTopViewWireframe _
                        Or myChart.ChartType = xlSurfaceWireframe) Then
                    AddIssueDetailsNote myIssue, NoteIndex, RID_RESXLT_COST_CHART_Surface_Comment
                Else
                    AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_UNSUPPORTEDTYPE1
                    NoteIndex = NoteIndex + 1
                    AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_UNSUPPORTEDTYPE2
                End If
                NoteIndex = NoteIndex + 1
            End If
            If bTrendline Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_TRENDLINE
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_TRENDLINE
                NoteIndex = NoteIndex + 1
            End If
            If bDatalabelWithLegend Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_DATALABELWITHLEGEND
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_DATALABELWITHLEGEND
                NoteIndex = NoteIndex + 1
            End If
            If bLegendPosition Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_LEGENDPOSITION
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_NOTRIGHT
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_LEGENDPOSITION
                NoteIndex = NoteIndex + 1
            End If
            If bTitleFont Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_TITLEFONT
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_DIFFERENT
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_TITLEFONT
                NoteIndex = NoteIndex + 1
            End If
            If bPiechartDirection Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PIE
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_SLICES_IN_DIFFERENT_DIRECTION
            End If
            If BorderIssue Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_BORDER
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_BORDER
                NoteIndex = NoteIndex + 1
            End If
            If bAxisInterval Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_AXISINTERVAL
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_AUTO
                AddIssueDetailsNote myIssue, NoteIndex, RID_STR_EXCEL_NOTE_AXISINTERVAL
                NoteIndex = NoteIndex + 1
            End If
           .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_CHARTNAME
            .Values.Add myChart.name
        End With
    
        mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) = _
            mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) + 1
        mAnalysis.Issues.Add myIssue
    End If
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub SetChartIssue(myChart As Chart, myName As String, strSubType As String, _
    strXMLSubType As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetChartIssue"
    Dim myIssue As IssueInfo
    Dim bUnsupportedPosition As Boolean

    Set myIssue = New IssueInfo
    
    ' Common Settings
    With myIssue
        .IssueID = CID_CHARTS_TABLES
        .IssueType = RID_STR_EXCEL_ISSUE_CHARTS_AND_TABLES
        .SubType = strSubType
        .Location = .CLocationSheet
        .SubLocation = myName
        
        .IssueTypeXML = CSTR_ISSUE_CHARTS_TABLES
        .SubTypeXML = strXMLSubType
        .locationXML = .CXMLLocationSheet
            
        
        If myChart.HasTitle Then
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_TITLE
            .Values.Add myChart.chartTitle.Text
        End If
        .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_TYPE
        .Values.Add myChart.ChartType 'TBD - getChartTypeAsString() convert to String
        
        'Pie Chart
        If (myChart.ChartType = xlPie) Or _
            (myChart.ChartType = xlPieExploded) Or _
            (myChart.ChartType = xlPieOfPie) Or _
            (myChart.ChartType = xl3DPie) Or _
            (myChart.ChartType = xl3DPieExploded) Then
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PIE
            .Values.Add RID_STR_EXCEL_ATTRIBUTE_SLICES_IN_DIFFERENT_DIRECTION
        End If

        If Not myChart.PivotLayout Is Nothing Then
            'Pivot Chart
            .SubType = RID_STR_EXCEL_SUBISSUE_PIVOT & " " & strSubType
                        
            'Pivot Chart details
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PIVOT_TABLE_NAME
            .Values.Add myChart.PivotLayout.PivotTable.name
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PIVOT_FIELDS_VISIBLE
            .Values.Add myChart.HasPivotFields
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PIVOT_FIELDS_NUM
            .Values.Add myChart.PivotLayout.PivotTable.PivotFields.count
        End If
    End With
    
    mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) = _
        mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) + 1
    mAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Function getLineStyleAsString(myLineStyle As XlLineStyle) As String

    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getLineStyleAsString"

    Dim strVal As String
    
    Select Case myLineStyle
    Case xlContinuous
        strVal = RID_STR_EXCEL_ENUMERATION_LINE_STYLE_CONTINUOUS
    Case xlDash
        strVal = RID_STR_EXCEL_ENUMERATION_LINE_STYLE_DASH
    Case xlDashDot
        strVal = RID_STR_EXCEL_ENUMERATION_LINE_STYLE_DASHDOT
    Case xlDot
        strVal = RID_STR_EXCEL_ENUMERATION_LINE_STYLE_DOT
    Case xlDouble
        strVal = RID_STR_EXCEL_ENUMERATION_LINE_STYLE_DOUBLE
    Case xlSlantDashDot
        strVal = RID_STR_EXCEL_ENUMERATION_LINE_STYLE_SLANTDASHDOT
    Case xlLineStyleNone
        strVal = RID_STR_EXCEL_ENUMERATION_LINE_STYLE_LINESTYLENONE
    Case Else
        strVal = RID_STR_EXCEL_ENUMERATION_UNKNOWN
    End Select
    
    
    getLineStyleAsString = strVal
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Function getChartTypeAsString(myChartType As XlChartType) As String
    '*********************************************************
    '**** Localisation: ON HOLD ******************************
    '*********************************************************
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getChartTypeAsString"
    
    Dim strVal As String
    
    Select Case myChartType
    Case xl3DArea
        strVal = "3DArea"
    Case xl3DAreaStacked
        strVal = "3DAreaStacked"
    Case xl3DAreaStacked100
        strVal = "3DAreaStacked100"
    Case xl3DBarClustered
        strVal = "3DBarClustered"
    Case xl3DBarStacked
        strVal = "xl3DBarStacked"
    Case xl3DBarStacked100
        strVal = "xl3DBarStacked100"
    Case xl3DColumn
        strVal = "3DColumn"
    Case xl3DColumnClustered
        strVal = "xl3DColumnClustered"
    Case xl3DColumnStacked
        strVal = "xl3DColumnStacked"
    Case xl3DColumnStacked100
        strVal = "xl3DColumnStacked100"
    Case xl3DLine
        strVal = "3DLine"
    Case xl3DPie
        strVal = "3DPie"
    Case xl3DPieExploded
        strVal = "3DPieExploded"
    Case xlArea
        strVal = "Area"
    Case xlAreaStacked
        strVal = "AreaStacked"
    Case xlAreaStacked100
        strVal = "AreaStacked100"
    Case xlBarClustered
        strVal = "BarClustered"
    Case xlBarOfPie
        strVal = "BarOfPie"
    Case xlBarStacked
        strVal = "BarStacked"
    Case xlBarStacked100
        strVal = "BarStacked100"
    Case xlBubble
        strVal = "Bubble"
    Case xlBubble3DEffect
        strVal = "Bubble3DEffect"
    Case xlColumnClustered
        strVal = "ColumnClustered"
    Case xlColumnStacked
        strVal = "ColumnStacked"
    Case xlColumnStacked100
        strVal = "ColumnStacked100"
    Case xlConeBarClustered
        strVal = "ConeBarClustered"
    Case xlConeBarStacked
        strVal = "ConeBarStacked"
    Case xlConeBarStacked100
        strVal = "ConeBarStacked100"
    Case xlConeCol
        strVal = "ConeCol"
    Case xlConeColClustered
        strVal = "ConeColClustered"
    Case xlConeColStacked
        strVal = "ConeColStacked"
    Case xlConeColStacked100
        strVal = "ConeColStacked100"
    Case xlCylinderBarClustered
        strVal = "CylinderBarClustered"
    Case xlCylinderBarStacked
        strVal = "CylinderBarStacked"
    Case xlCylinderBarStacked100
        strVal = "CylinderBarStacked100"
    Case xlCylinderCol
        strVal = "CylinderCol"
    Case xlCylinderColClustered
        strVal = "CylinderColClustered"
    Case xlCylinderColStacked
        strVal = "CylinderColStacked"
    Case xlCylinderColStacked100
        strVal = "CylinderColStacked100"
    Case xlDoughnut
        strVal = "Doughnut"
    Case xlLine
        strVal = "Line"
    Case xlLineMarkers
        strVal = "LineMarkers"
    Case xlLineMarkersStacked
        strVal = "LineMarkersStacked"
    Case xlLineMarkersStacked100
        strVal = "LineMarkersStacked100"
    Case xlLineStacked
        strVal = "LineStacked"
    Case xlLineStacked100
        strVal = "LineStacked100"
    Case xlPie
        strVal = "Pie"
    Case xlPieExploded
        strVal = "PieExploded"
    Case xlPieOfPie
        strVal = "PieOfPie"
    Case xlPyramidBarClustered
        strVal = "PyramidBarClustered"
    Case xlPyramidBarStacked
        strVal = "PyramidBarStacked"
    Case xlPyramidBarStacked100
        strVal = "PyramidBarStacked100"
    Case xlPyramidCol
        strVal = "PyramidCol"
    Case xlPyramidColClustered
        strVal = "PyramidColClustered"
    Case xlPyramidColStacked
        strVal = "PyramidColStacked"
    Case xlPyramidColStacked100
        strVal = "PyramidColStacked100"
    Case xlRadar
        strVal = "Radar"
    Case xlRadarFilled
        strVal = "RadarFilled"
    Case xlRadarMarkers
        strVal = "RadarMarkers"
    Case xlStockHLC
        strVal = "StockHLC"
    Case xlStockOHLC
        strVal = "StockOHLC"
    Case xlStockVHLC
        strVal = "StockVHLC"
    Case xlStockVOHLC
        strVal = "StockVOHLC"
    Case xlSurface
        strVal = "Surface"
    Case xlSurfaceTopView
        strVal = "SurfaceTopView"
    Case xlSurfaceTopViewWireframe
        strVal = "SurfaceTopViewWireframe"
    Case xlSurfaceWireframe
        strVal = "SurfaceWireframe"
    Case xlXYScatter
        strVal = "XYScatter"
    Case xlXYScatterLines
        strVal = "XYScatterLines"
    Case xlXYScatterLinesNoMarkers
        strVal = "XYScatterLinesNoMarkers"
    Case xlXYScatterSmooth
        strVal = "XYScatterSmooth"
    Case xlXYScatterSmoothNoMarkers
        strVal = "XYScatterSmoothNoMarkers"
    Case Else
        strVal = RID_STR_EXCEL_ENUMERATION_UNKNOWN
    End Select

    getChartTypeAsString = strVal

    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Sub HandleZoomIssue(currentSheet)
    Dim myIssue As IssueInfo
    Dim currentFunctionName As String
    currentFunctionName = "HandleZoomIssue"

    On Error GoTo HandleErrors

    Set myIssue = New IssueInfo
    With myIssue
        .IssueID = CID_FORMAT
        .IssueType = RID_STR_EXCEL_ISSUE_FORMAT
        .SubType = RID_STR_EXCEL_SUBISSUE_ZOOM
        .Location = .CLocationSheet
        .SubLocation = currentSheet.name
        
        .IssueTypeXML = CSTR_ISSUE_FORMAT
        .SubTypeXML = CSTR_SUBISSUE_ZOOM
        .locationXML = .CXMLLocationSheet
        
        AddIssueDetailsNote myIssue, 0, RID_STR_EXCEL_NOTE_ZOOM
    End With
 
    mAnalysis.IssuesCountArray(CID_FORMAT) = _
        mAnalysis.IssuesCountArray(CID_FORMAT) + 1
    mAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_SheetDisplay(aWB As Workbook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_SheetDisplay"

    If aWB.Sheets.count = 1 Then Exit Sub

    Dim lastZoomVal As Integer
    Dim bInitZoom As Boolean
    Dim bZoomChanged As Boolean
    Dim ws As Object

    bInitZoom = True
    bZoomChanged = False

    For Each ws In aWB.Sheets
        ws.Activate
        
        On Error GoTo HandleErrors
        
        If bInitZoom Then
            lastZoomVal = ActiveWindow.Zoom
            bInitZoom = False
        ElseIf Not bZoomChanged Then
            If ActiveWindow.Zoom <> lastZoomVal Then
                bZoomChanged = True
                HandleZoomIssue ws
            End If
        End If
        If bZoomChanged Then Exit For
    Next ws
    
FinalExit:
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_SheetLimits(aWB As Workbook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_SheetLimits"
    Dim myIssue As IssueInfo

    If aWB.Sheets.count < CWORKBOOK_SHEETS_LIMIT + 1 Then Exit Sub
    
    Set myIssue = New IssueInfo
    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_STR_EXCEL_SUBISSUE_MAX_SHEETS_EXCEEDED
        .Location = .CLocationWorkBook
        .SubLocation = aWB.name
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_MAX_SHEETS_EXCEEDED
        .locationXML = .CXMLLocationWorkBook
        
        .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_NUMBER_OF_SHEETS
        .Values.Add aWB.Sheets.count
        
        AddIssueDetailsNote myIssue, 0, RID_STR_EXCEL_NOTE_SHEET_LIMITS_1 & CWORKBOOK_SHEETS_LIMIT
        AddIssueDetailsNote myIssue, 1, RID_STR_EXCEL_NOTE_SHEET_LIMITS_2 & CWORKBOOK_SHEETS_LIMIT
    End With
    mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
        mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
    mAnalysis.Issues.Add myIssue
    Set myIssue = Nothing
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_SheetIssues(aWB As Workbook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_SheetIssues"

    Dim myWrkSheet As Worksheet

    For Each myWrkSheet In aWB.Worksheets
        Analyze_OLEEmbedded myWrkSheet
        Analyze_CellInSheetIssues myWrkSheet
        Analyze_EmbeddedCharts myWrkSheet
        Analyze_SheetName myWrkSheet
        Analyze_QueryTables myWrkSheet
    Next myWrkSheet

    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub Analyze_SheetName(mySheet As Worksheet)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_SheetName"
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    
    Dim invalidCharacters As String
    invalidCharacters = InvalidSheetNameCharacters(mySheet.name)
    If Len(invalidCharacters) <> 0 Then
        With myIssue
            .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
            .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
            .SubType = RID_STR_EXCEL_SUBISSUE_INVALID_WORKSHEET_NAME
            .Location = .CLocationSheet
            .SubLocation = mySheet.name
            
            .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
            .SubTypeXML = CSTR_SUBISSUE_INVALID_WORKSHEET_NAME
            .locationXML = .CXMLLocationSheet
            
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_INVALIDCHARACTER
            .Values.Add invalidCharacters
            
            AddIssueDetailsNote myIssue, 0, RID_STR_EXCEL_NOTE_INVALIDWORKSHEETNAME
            
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

Function InvalidSheetNameCharacters(aName As String) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "InvalidSheetNameCharacters"
    
    Dim I As Integer
    Dim NameCount As Integer
    Dim newBadCharLine As String
    Dim invalidCharacterDetails As String
    Dim BadCharPosition As String
    Dim theBadChars As BadSheetNameChar
    NameCount = Len(aName)
    invalidCharacterDetails = ""
    For I = 1 To NameCount
        theBadChars.BadChar = Mid(aName, I, 1)
        theBadChars.Position = I
        BadCharPosition = CStr(theBadChars.Position)
        Select Case theBadChars.BadChar
        Case "[", "]", "{", "}", ".", "!", "%", "$", "^", ".", "&", "(", ")", _
            "-", "=", "+", "~", "#", "@", "'", ";", "<", ">", ",", "|", "`"
            newBadCharLine = ReplaceTopic2Tokens(RID_STR_EXCEL_ATTRIBUTE_BADCHARACTER, CR_BADCHAR, _
                theBadChars.BadChar, CR_BADCHARNUM, BadCharPosition)
            invalidCharacterDetails = invalidCharacterDetails + newBadCharLine + ", "
        Case Else
        End Select
    Next I
    If Len(invalidCharacterDetails) > 0 Then
        InvalidSheetNameCharacters = Left(invalidCharacterDetails, (Len(invalidCharacterDetails) - 2))
    Else
        InvalidSheetNameCharacters = ""
    End If
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    
End Function

Sub Analyze_QueryTables(mySheet As Worksheet)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_QueryTables"

    Dim aTable As QueryTable
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo

    For Each aTable In mySheet.QueryTables
        If (aTable.QueryType = xlADORecordset) Or _
           (aTable.QueryType = xlDAORecordSet) Or _
           (aTable.QueryType = xlODBCQuery) Or _
           (aTable.QueryType = xlOLEDBQuery) Then

            With myIssue
                .IssueID = CID_CHARTS_TABLES
                .IssueType = RID_STR_EXCEL_ISSUE_CHARTS_AND_TABLES
                .SubType = RID_RESXLS_COST_DB_Query
                .Location = .CLocationSheet
                .SubLocation = mySheet.name
                
                .IssueTypeXML = CSTR_ISSUE_CHARTS_TABLES
                .SubTypeXML = CSTR_SUBISSUE_DB_QUERY
                .locationXML = .CXMLLocationSheet
                
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_DB_QUERY
                .Values.Add aTable.Connection

                AddIssueDetailsNote myIssue, 0, RID_STR_EXCEL_NOTE_DB_QUERY

                mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) = _
                        mAnalysis.IssuesCountArray(CID_CHARTS_TABLES) + 1
            End With
            mAnalysis.Issues.Add myIssue
        End If
    Next aTable

FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_WorkbookVersion(aWB As Workbook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_WorkbookVersion"
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    Dim aProp As Variant
    
    If IsOldVersion(aWB.FileFormat) Then
        With myIssue
            .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
            .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
            .SubType = RID_STR_EXCEL_SUBISSUE_OLD_WORKBOOK_VERSION
            .Location = .CLocationWorkBook
        
            .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
            .SubTypeXML = CSTR_SUBISSUE_OLD_WORKBOOK_VERSION
            .locationXML = .CXMLLocationWorkBook
        
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_WORKBOOK_VERSION
            .Values.Add aWB.FileFormat

            AddIssueDetailsNote myIssue, 0, RID_STR_EXCEL_NOTE_OLDWORKBOOKVERSION

            mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) = _
                    mAnalysis.IssuesCountArray(CID_CONTENT_AND_DOCUMENT_PROPERTIES) + 1
        End With
        Call DoPreparation(mAnalysis, myIssue, RID_STR_EXCEL_NOTE_OLD_OLDWORKBOOKVERSION_PREPARABLE, aProp, aWB)
        
        mAnalysis.Issues.Add myIssue
    End If
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub
        
Function getRange(myRange As Range) As String
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "getRange"
    getRange = ""
    
    On Error Resume Next
    getRange = myRange.Address(RowAbsolute:=False, ColumnAbsolute:=False, ReferenceStyle:=xlA1)
    
FinalExit:
    Exit Function

HandleErrors:
    WriteDebug currentFunctionName & " : myRange.name " & myRange.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Sub Analyze_CellInSheetIssues(mySheet As Worksheet)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_CellInSheetIssues"
    Dim myCellRng As Range
 
    Set myCellRng = mySheet.UsedRange
    Call CheckAllCellFormatting(myCellRng, mySheet.name)
    Call CheckAllCellFunctions(myCellRng, mySheet.name)

FinalExit:
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub CheckAllCellFormatting(CurrRange As Range, myName As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckAllCellFormatting"

    Dim myCell As Range
    Dim myCellAttri As CellAtrributes
    Dim bCellIssue As Boolean
    Dim bCellIssueAll As Boolean
    Dim startTime As Single
    
    bCellIssue = False
    bCellIssueAll = False
    startTime = Timer

    For Each myCell In CurrRange
        bCellIssue = CheckCellFormatting(myCell, myCellAttri)
        bCellIssueAll = bCellIssueAll Or bCellIssue
        If (Timer - gExcelMaxRangeProcessTime > startTime) Then
            WriteDebug currentFunctionName & " : [" & myName & _
                           "]Too much time needed, abortet cell formatting check."
            Exit For
        End If
    Next

FinalExit:
    If bCellIssueAll Then
        ReportCellFormattingIssue myName, myCellAttri
    End If

    Exit Sub

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Function CheckLineFormatIssue(myRange As Range, edge As XlBordersIndex) As Boolean
    CheckLineFormatIssue = (myRange.Borders(edge).LineStyle <> xlContinuous) And _
        (myRange.Borders(edge).LineStyle <> xlDouble) And _
        (myRange.Borders(edge).LineStyle <> xlLineStyleNone)
End Function
 
Private Function CheckCellFormatting(myCell As Range, myCellAttri As CellAtrributes) As Boolean
    Dim currentFunctionName As String
    currentFunctionName = "CheckCellFormatting"
  
    On Error GoTo HandleErrors
    
    Dim bCellLineFormatIssue As Boolean
    
    CheckCellFormatting = False
    
    bCellLineFormatIssue = CheckLineFormatIssue(myCell, xlEdgeBottom) Or _
        CheckLineFormatIssue(myCell, xlEdgeLeft) Or _
        CheckLineFormatIssue(myCell, xlEdgeRight) Or _
        CheckLineFormatIssue(myCell, xlEdgeTop)
        
    CheckCellFormatting = bCellLineFormatIssue Or _
        (myCell.Interior.Pattern <> xlPatternSolid And myCell.Interior.Pattern <> xlPatternNone)
        
    If Not CheckCellFormatting Then Exit Function
    
            If bCellLineFormatIssue Then
                myCellAttri.LineStyle = myCellAttri.LineStyle + 1
            End If
            If (myCell.Interior.Pattern <> xlPatternSolid And myCell.Interior.Pattern <> xlPatternNone) Then
                myCellAttri.FillPattern = myCellAttri.FillPattern + 1
            End If
    
    Exit Function
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Private Sub ReportCellFormattingIssue(myName As String, myCellAttri As CellAtrributes)
    Dim currentFunctionName As String
    currentFunctionName = "ReportCellFormattingIssue"
  
    On Error GoTo HandleErrors
    
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
        
        With myIssue
            .IssueID = CID_FORMAT
            .IssueType = RID_STR_EXCEL_ISSUE_FORMAT
            .SubType = RID_STR_EXCEL_SUBISSUE_ATTRIBUTES
            .Location = .CLocationSheet
            
            .IssueTypeXML = CSTR_ISSUE_FORMAT
            .SubTypeXML = CSTR_SUBISSUE_ATTRIBUTES
            .locationXML = .CXMLLocationSheet
            
            .SubLocation = myName
            '.Line = myCell.row
            '.column = Chr(myCell.column + 65 - 1)
            
            Dim noteCount As Long
            noteCount = 0
                
            If myCellAttri.LineStyle > 0 Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_LINE_STYLE
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_DASHED_DOT
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_NUMBER_OF_CELLS
                .Values.Add myCellAttri.LineStyle
                AddIssueDetailsNote myIssue, noteCount, RID_STR_EXCEL_NOTE_CELL_ATTRIBUTES_3
                noteCount = noteCount + 1
            End If
            If myCellAttri.FillPattern > 0 Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_FILL_PATTERN
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_PATTERNED
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_NUMBER_OF_CELLS
                .Values.Add myCellAttri.FillPattern
                AddIssueDetailsNote myIssue, noteCount, RID_STR_EXCEL_NOTE_CELL_ATTRIBUTES_4
                noteCount = noteCount + 1
            End If
                
            
            mAnalysis.IssuesCountArray(CID_FORMAT) = _
                    mAnalysis.IssuesCountArray(CID_FORMAT) + 1
        End With
           
        mAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub CheckAllCellFunctions(CurrRange As Range, myName As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckAllCellFunctions"

    Dim myCell As Range
    Dim startTime As Single
    
    startTime = Timer

    For Each myCell In CurrRange
        Call CheckCellFunction(myCell, myName)
        If (Timer - gExcelMaxRangeProcessTime > startTime) Then
            WriteDebug currentFunctionName & " : [" & myName & _
                       "]Too much time needed, abortet cell functions check (xlCellTypeFormulas)."
            Exit For
        End If
    Next

FinalExit:
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub CheckCellFunction(myCell As Range, myName As String)
    Dim currentFunctionName As String
    currentFunctionName = "CheckCellFunction"
  
    On Error GoTo HandleErrors
    Dim bCellFunctionIssue As Boolean
    Dim bCellINFOFunctionIssue As Boolean
    Dim bCellERROR_TYPEFunctionIssue As Boolean
    Dim bCellExternalFunctionIssue As Boolean
    Dim bHasDateDifFunction As Boolean
    Dim bHasPhoneticFunction As Boolean
    Dim aFormularStr As String

    aFormularStr = myCell.FormulaR1C1

    If (aFormularStr = Null) Then Exit Sub
    If (aFormularStr = "") Then Exit Sub
    
    bCellINFOFunctionIssue = (InStr(aFormularStr, "INFO(") <> 0)
    bCellERROR_TYPEFunctionIssue = (InStr(aFormularStr, "ERROR.TYPE(") <> 0)
    bCellExternalFunctionIssue = (InStr(aFormularStr, ".xls!") <> 0)
    bHasDateDifFunction = (InStr(aFormularStr, "DATEDIF(") <> 0)
    bHasPhoneticFunction = (InStr(aFormularStr, "PHONETIC(") <> 0)
    
    bCellFunctionIssue = bCellINFOFunctionIssue Or bCellERROR_TYPEFunctionIssue _
                         Or bCellExternalFunctionIssue Or bHasDateDifFunction Or bHasPhoneticFunction
 
    If Not bCellFunctionIssue Then Exit Sub
    
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
        
    With myIssue
        .IssueID = CID_FUNCTIONS
        .IssueType = RID_STR_EXCEL_ISSUE_FUNCTIONS
        .Location = .CLocationSheet
        
        .IssueTypeXML = CSTR_ISSUE_FUNCTIONS
        .locationXML = .CXMLLocationSheet
        
        .SubLocation = myName
        .Line = myCell.row
        .column = Chr(myCell.column + 65 - 1)
        
        Dim noteCount As Long
        noteCount = 0
        If bCellINFOFunctionIssue Then
            .SubTypeXML = CSTR_SUBISSUE_INFO
            .SubType = RID_STR_EXCEL_SUBISSUE_INFO
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_FUNCTION_STRING
            .Values.Add myCell.FormulaR1C1
            AddIssueDetailsNote myIssue, noteCount, RID_STR_EXCEL_NOTE_CELL_FUNCTIONS_1
            noteCount = noteCount + 1
        End If
        If bCellERROR_TYPEFunctionIssue Then
            .SubTypeXML = CSTR_SUBISSUE_ERROR_TYPE
            .SubType = RID_STR_EXCEL_SUBISSUE_ERROR_TYPE
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_FUNCTION_STRING
            .Values.Add myCell.FormulaR1C1
            AddIssueDetailsNote myIssue, noteCount, RID_STR_EXCEL_NOTE_CELL_FUNCTIONS_2
            noteCount = noteCount + 1
        End If
        If bCellExternalFunctionIssue Then
            .SubTypeXML = CSTR_SUBISSUE_EXTERNAL
            .SubType = RID_STR_EXCEL_SUBISSUE_EXTERNAL
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_FUNCTION_STRING
            .Values.Add myCell.FormulaR1C1
            AddIssueDetailsNote myIssue, noteCount, RID_STR_EXCEL_NOTE_CELL_FUNCTIONS_3
            noteCount = noteCount + 1
        End If
        If bHasDateDifFunction Then
            .SubTypeXML = CSTR_SUBISSUE_DATEDIF
            .SubType = RID_STR_EXCEL_SUBISSUE_DATEDIF
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_FUNCTION_STRING
            .Values.Add myCell.FormulaR1C1
            AddIssueDetailsNote myIssue, noteCount, RID_STR_EXCEL_NOTE_CELL_FUNCTIONS_DATEDIF
            noteCount = noteCount + 1
        End If
        If bHasPhoneticFunction Then
            .SubTypeXML = CSTR_SUBISSUE_PHONETIC
            .SubType = RID_STR_EXCEL_SUBISSUE_PHONETIC
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_FUNCTION_STRING
            .Values.Add myCell.FormulaR1C1
            AddIssueDetailsNote myIssue, noteCount, RID_STR_EXCEL_NOTE_CELL_FUNCTIONS_PHONETIC
            noteCount = noteCount + 1
        End If

        mAnalysis.IssuesCountArray(CID_FUNCTIONS) = _
                mAnalysis.IssuesCountArray(CID_FUNCTIONS) + 1
    End With
       
    mAnalysis.Issues.Add myIssue
    
FinalExit:
    Set myIssue = Nothing
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub Analyze_Password_Protection(aWB As Workbook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Password_Protection"
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    
    If aWB.HasPassword Or aWB.WriteReserved Then
        With myIssue
            .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
            .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
            .SubType = RID_STR_COMMON_SUBISSUE_PASSWORDS_PROTECTION
            
            .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
            .SubTypeXML = CSTR_SUBISSUE_PASSWORD_PROTECTION
            .locationXML = .CLocationWorkBook
            
            .Location = .CLocationWorkBook
            
            If aWB.HasPassword Then
                .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PASSWORD_TO_OPEN
                .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
            End If
            If aWB.WriteReserved Then
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PASSWORD_TO_MODIFY
            .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
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

Sub SetDocProperties(docAnalysis As DocumentAnalysis, wb As Workbook, fso As FileSystemObject)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "SetProperties"
    Dim f As File
    Set f = fso.GetFile(docAnalysis.name)
    
    Const appPropertyAppName = 9
    Const appPropertyLastAuthor = 7
    Const appPropertyRevision = 8
    Const appPropertyTemplate = 6
    Const appPropertyTimeCreated = 11
    Const appPropertyTimeLastSaved = 12

    On Error Resume Next
    docAnalysis.PageCount = wb.Sheets.count
    docAnalysis.Created = f.DateCreated
    docAnalysis.Modified = f.DateLastModified
    docAnalysis.Accessed = f.DateLastAccessed
    docAnalysis.Printed = DateValue("01/01/1900")
    On Error GoTo HandleErrors

    On Error Resume Next 'Some apps may not support all props
    docAnalysis.Application = getAppSpecificApplicationName & " " & Application.Version
    'docAnalysis.Application = wb.BuiltinDocumentProperties(appPropertyAppName)
    'If InStr(docAnalysis.Application, "Microsoft") = 1 Then
    '    docAnalysis.Application = Mid(docAnalysis.Application, Len("Microsoft") + 2)
    'End If
    'If InStr(Len(docAnalysis.Application) - 2, docAnalysis.Application, ".") = 0 Then
    '    docAnalysis.Application = docAnalysis.Application & " " & Application.Version
    'End If
    
    docAnalysis.SavedBy = _
        wb.BuiltinDocumentProperties(appPropertyLastAuthor)
    docAnalysis.Revision = _
        val(wb.BuiltinDocumentProperties(appPropertyRevision))
    docAnalysis.Template = _
        fso.GetFileName(wb.BuiltinDocumentProperties(appPropertyTemplate))
    docAnalysis.Modified = _
        wb.BuiltinDocumentProperties(appPropertyTimeLastSaved)

FinalExit:
    Set f = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub Analyze_OLEEmbedded(wrkSheet As Worksheet)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_OLEEmbedded"

    ' Handle Shapes
    Dim aShape As Shape
    For Each aShape In wrkSheet.Shapes
        Analyze_OLEEmbeddedSingleShape mAnalysis, aShape, wrkSheet.name
        Analyze_Lines mAnalysis, aShape, wrkSheet.name
        Analyze_Transparency mAnalysis, aShape, wrkSheet.name
        Analyze_Gradients mAnalysis, aShape, wrkSheet.name
    Next aShape
    
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Sub Analyze_Workbook_Protection(aWB As Workbook)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Analyze_Workbook_Protection"
    Dim myIssue As IssueInfo
    Set myIssue = New IssueInfo
    Dim bProtectSharing As Boolean
    Dim bProtectStructure As Boolean
    Dim bProtectWindows As Boolean
    
    bProtectSharing = False
    bProtectStructure = False
    bProtectWindows = False
    
    If Not WorkbookProtectTest(aWB, bProtectSharing, bProtectStructure, bProtectWindows) Then
        GoTo FinalExit
    End If
    
    Set myIssue = New IssueInfo
    With myIssue
        .IssueID = CID_CONTENT_AND_DOCUMENT_PROPERTIES
        .IssueType = RID_STR_COMMON_ISSUE_CONTENT_AND_DOCUMENT_PROPERTIES
        .SubType = RID_STR_EXCEL_SUBISSUE_WORKBOOK_PROTECTION
        .Location = .CLocationWorkBook
        
        .IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES
        .SubTypeXML = CSTR_SUBISSUE_WORKBOOK_PROTECTION
        .locationXML = .CXMLLocationWorkBook
            
        If bProtectSharing Then
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PROTECT_TYPE_SHARING
            .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
        End If
        If bProtectStructure Then
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PROTECT_TYPE_STRUCTURE
            .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
        End If
        If bProtectWindows Then
            .Attributes.Add RID_STR_EXCEL_ATTRIBUTE_PROTECT_TYPE_WINDOWS
            .Values.Add RID_STR_EXCEL_ATTRIBUTE_SET
        End If
        
        AddIssueDetailsNote myIssue, 0, RID_STR_EXCEL_NOTE_PASSWORD_TO_OPEN
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

Private Function WorkbookProtectTest(aWB As Workbook, bProtectSharing As Boolean, _
    bProtectStructure As Boolean, bProtectWindows As Boolean) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WorkbookProtectTest"
    
    WorkbookProtectTest = False

    On Error Resume Next 'Simulate Try Catch
    aWB.UnprotectSharing sharingPassword:=" "
    If Err.Number = 1004 Then
        bProtectSharing = True
    ElseIf Err.Number <> 0 Then
        Resume HandleErrors
    End If
    On Error GoTo HandleErrors
    
    On Error Resume Next 'Simulate Try Catch
    aWB.Unprotect Password:=""
    If Err.Number = 1004 Then
        If aWB.ProtectStructure = True Then
            bProtectStructure = True
        End If
        If aWB.ProtectWindows = True Then
            bProtectWindows = True
        End If
    End If
    
    If bProtectSharing Or bProtectStructure Or bProtectWindows Then
        WorkbookProtectTest = True
    End If
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit

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
Private Function FormatIssueComplex(myChart As Chart, bDataTable As Boolean, bXAxes As Boolean) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "FormatIssueComplex"
    
    bXAxes = False

    If myChart.HasDataTable Then
        bDataTable = True
    End If
    If Not (IsPie(myChart) Or myChart.ChartType = xlDoughnut Or myChart.ChartType = xlBubble3DEffect) Then
        If myChart.HasAxis(1) Then
            If myChart.Axes(1).CategoryType = xlTimeScale Or myChart.Axes(1).CategoryType = xlAutomaticScale Then
                bXAxes = True
            End If
        End If
    End If
    If bDataTable Or bXAxes Then
        FormatIssueComplex = True
    End If
    Exit Function
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Private Function IsAreaChart(myChart As Chart) As Boolean

    If (myChart.ChartType = xlArea Or myChart.ChartType = xl3DArea Or _
        myChart.ChartType = xlAreaStacked Or _
        myChart.ChartType = xl3DAreaStacked Or _
        myChart.ChartType = xlAreaStacked100 Or _
        myChart.ChartType = xl3DAreaStacked100) _
    Then
        IsAreaChart = True
    Else
        IsAreaChart = False
    End If

End Function

Private Function FormatissueMinor(myChart As Chart, bUnsupportedType As Boolean, bTrendline As Boolean, bDatalabelWithLegend As Boolean, bLegendPosition As Boolean, bTitleFont As Boolean, bPiechartDirection As Boolean, bAxisInterval As Boolean) As Boolean
On Error GoTo HandleErrors
Dim currentFunctionName As String
currentFunctionName = "FormatissueMinor"

Dim ctype As Integer
Dim fsize As Integer
Dim se As Series
Dim dl As DataLabel

    FormatissueMinor = False
    ctype = myChart.ChartType
    
    If (ctype = xlBubble Or ctype = xlPieOfPie Or ctype = xl3DPieExploded _
        Or ctype = xlRadarFilled Or ctype = xlBubble3DEffect _
        Or ctype = xlRadarMarkers Or ctype = xlRadar Or ctype = xlBarOfPie _
        Or ctype = xlXYScatter Or ctype = xlXYScatterLines Or ctype = xlXYScatterLinesNoMarkers _
        Or ctype = xlXYScatterSmooth Or ctype = xlXYScatterSmoothNoMarkers _
        Or ctype = xlSurface Or ctype = xlSurfaceTopView Or ctype = xlSurfaceTopViewWireframe _
        Or ctype = xlSurfaceWireframe) Then
        bUnsupportedType = True
    End If
    
    For Each se In myChart.SeriesCollection
        On Error Resume Next ' may not have trendlines property
        If se.Trendlines.count <> 0 Then
            If Err.Number = 0 Then
            bTrendline = True
            End If
        End If
        If se.HasDataLabels Then
            If Err.Number = 0 Then
                If (IsAreaChart(myChart)) Then
                    For Each dl In se.DataLabels
                        If dl.ShowLegendKey = True Then
                            bDatalabelWithLegend = True
                            Exit For
                        End If
                    Next dl
                Else
                    Dim pt As Point
                    For Each pt In se.Points
                        If pt.HasDataLabel Then
                            If pt.DataLabel.ShowLegendKey Then
                                bDatalabelWithLegend = True
                                Exit For
                            End If
                        End If
                    Next pt
                End If
            End If
        End If
        On Error GoTo HandleErrors
        If bTrendline And bDatalabelWithLegend Then
           Exit For
        End If
    Next se
    
    If myChart.HasLegend Then
        Dim legPos As Long
        On Error Resume Next 'If legend moved accessing position will fail
        legPos = myChart.Legend.Position
        
        If (Err.Number <> 0) Or (legPos <> xlLegendPositionRight) Then
            bLegendPosition = True
        End If
        On Error GoTo HandleErrors
    End If
    
    If IsPie(myChart) Then
        bPiechartDirection = True
    ElseIf myChart.ChartType <> xlDoughnut And myChart.ChartType <> xlBubble3DEffect Then
        If myChart.HasAxis(xlValue, xlPrimary) Then
            With myChart.Axes(xlValue, xlPrimary)
                If .MajorUnitIsAuto And .MaximumScaleIsAuto And .MinimumScaleIsAuto And .MinorUnitIsAuto Then
                    bAxisInterval = True
                End If
            End With
        End If
    End If
    
    On Error Resume Next 'If title has mixed font size accessing Font.Size will fail - Title mixed font issue
    If myChart.HasTitle Then
        fsize = myChart.chartTitle.Font.Size
        If Err.Number = FontError Then
            bTitleFont = True
        End If
    End If
    
    On Error GoTo HandleErrors
    If bUnsupportedType Or bTrendline Or bDatalabelWithLegend Or bLegendPosition Or bTitleFont Or bPiechartDirection Or bAxisInterval Then
        FormatissueMinor = True
    End If

FinalExit:
    
    Set se = Nothing
    Set dl = Nothing
    Exit Function

HandleErrors:

    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit

End Function

Private Function SeriesIssue(myChart As Chart, bSeriesChartTypeChanged As Boolean, bDatasourceNotLinkedtoCell As Boolean, bDatasourceOnDifferentSheet As Boolean, bCategoryandValue As Boolean, bCLabelMorethanOneCell As Boolean, bOneColumnRow As Boolean) As Boolean
On Error GoTo HandleErrors
Dim currentFunctionName As String
currentFunctionName = "SeriesIssue"
SeriesIssue = False

Dim Num As Integer
Dim I As Integer
Dim i2 As Integer
Dim formula As String
Dim p1 As Integer, p2 As Integer
Dim b1 As Integer, b2 As Integer
Dim comma1 As Integer, comma2 As Integer
Dim starty As Integer
Dim ctype As Integer
Dim temp As Integer
Dim myarray() As String
Dim Values(3), sh
Dim chartseries As Series
Dim b As Boolean
Dim bmorecolumns As Boolean
Dim c As Boolean

bmorecolumns = False
Num = myChart.SeriesCollection.count

If (Num = 0) Then Exit Function

ctype = myChart.SeriesCollection(1).ChartType
I = 0
sh = ""

ReDim Preserve myarray(Num, 3)

If IsPie(myChart) And Num > 1 Then  'if pie chart has more than one series,set series number to 1
        bmorecolumns = True
        Num = 1
End If
For Each chartseries In myChart.SeriesCollection
    On Error Resume Next
    formula = chartseries.formula
    If Err.Number <> 0 Then
        GoTo FinalExit
    End If
    If Not bSeriesChartTypeChanged Then  'check if the chart type changed
        temp = chartseries.ChartType
        If temp <> ctype Then
            bSeriesChartTypeChanged = True
        End If
    End If
    
    'get each part of the formula, if it is a single range, set the value to the array
    p1 = InStr(1, formula, "(")
    comma1 = InStr(1, formula, ",")
    Values(0) = Mid(formula, p1 + 1, comma1 - p1 - 1)
    
    If Mid(formula, comma1 + 1, 1) = "(" Then
'       Multiple ranges
        bDatasourceNotLinkedtoCell = True
        GoTo FinalExit
    Else
        If Mid(formula, comma1 + 1, 1) = "{" Then
'           Literal Array
            bDatasourceNotLinkedtoCell = True
            GoTo FinalExit
        Else
'          A single range
            comma2 = InStr(comma1 + 1, formula, ",")
            Values(1) = Mid(formula, comma1 + 1, comma2 - comma1 - 1)
            starty = comma2
        End If
    End If
    
    If Mid(formula, starty + 1, 1) = "(" Then
'       Multiple ranges
        bDatasourceNotLinkedtoCell = True
        GoTo FinalExit
    Else
        If Mid(formula, starty + 1, 1) = "{" Then
'           Literal Array
            bDatasourceNotLinkedtoCell = True
            GoTo FinalExit
        Else
'          A single range
            comma1 = starty
            comma2 = InStr(comma1 + 1, formula, ",")
            Values(2) = Mid(formula, comma1 + 1, comma2 - comma1 - 1)
        End If
    End If
    
    If SheetCheck(sh, Values) Then      'check if data from different sheet
        bDatasourceOnDifferentSheet = True
        GoTo FinalExit
    End If

    For i2 = 0 To 2   'set data to myarray, if it is range, assign the range address, else null
        If IsRange(Values(i2)) Then
            myarray(I, i2) = Range(Values(i2)).Address
        'ElseIf (Not IsRange(values(i2))) And values(i2) <> "" Then
        '    bDatasourceNotLinkedtoCell = True
        '    myarray(i, i2) = ""
        Else
            bDatasourceNotLinkedtoCell = True
            myarray(I, i2) = ""
        End If
    Next i2
    
    I = I + 1
    If bmorecolumns Then 'if it is pie chart, exit
        Exit For
    End If
Next chartseries


c = DataCheck(myarray, Num, bCategoryandValue, bCLabelMorethanOneCell, bOneColumnRow) 'check data values and category of the chart
    
FinalExit:
If bSeriesChartTypeChanged Or bDatasourceNotLinkedtoCell Or bDatasourceOnDifferentSheet Or bCategoryandValue Or bCLabelMorethanOneCell Or bOneColumnRow Then
    SeriesIssue = True
End If

Last:
    Set chartseries = Nothing
    Exit Function

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume Last
End Function

Private Function DataCheck(myarray() As String, Num As Integer, bCategoryandValue As Boolean, bCLabelMorethanOneCell As Boolean, bOneColumnRow As Boolean)
On Error GoTo HandleErrors
Dim currentFunctionName As String
currentFunctionName = "DataCheck"

Dim s1() As String
Dim v1() As String
Dim v2() As String
Dim c1() As String
Dim c2() As String
Dim bs1isrange As Boolean
Dim bc1isrange As Boolean
Dim bc2isrange As Boolean
Dim j As Integer
Dim I As Integer
Dim btemp1 As Boolean
Dim btemp2 As Boolean


bs1isrange = True
bc1isrange = True
bc2isrange = True

If myarray(0, 1) = "" Then
    bs1isrange = False
Else
    s1 = SplitRange(myarray(0, 1))
    If UBound(s1) < 4 Then
        bOneColumnRow = True
        GoTo FinalExit
    End If
    If (Asclong(s1(0)) <> Asclong(s1(2))) And (Asclong(s1(1)) <> Asclong(s1(3))) Then
        bCLabelMorethanOneCell = True
        GoTo FinalExit
    End If
    
End If

If myarray(0, 0) = "" Then
    ReDim c1(2)
    bc1isrange = False
    c1(0) = ""
    c1(1) = ""
Else
    If InStr(1, myarray(0, 0), ":") <> 0 Then
        bCLabelMorethanOneCell = True
        GoTo FinalExit
    End If
    c1 = SplitRange(myarray(0, 0))
End If
v1 = SplitRange(myarray(0, 2))

If bs1isrange Then
    btemp1 = s1(0) = s1(2) And s1(1) = v1(1) And s1(3) = v1(3) And Asclong(v1(0)) >= Asclong(s1(0)) + 1 'category beside first column
    btemp2 = s1(1) = s1(3) And s1(0) = v1(0) And s1(2) = v1(2) And Asclong(v1(1)) >= Asclong(s1(1)) + 1 'category beside first row
    If (Not btemp1) And (Not btemp2) Then
        bCategoryandValue = True
        GoTo FinalExit
    End If
End If
If bc1isrange Then
    btemp1 = v1(0) = v1(2) And c1(0) = v1(0) And Asclong(c1(1)) <= Asclong(v1(1)) - 1 'data label beside row
    btemp2 = v1(1) = v1(3) And c1(1) = v1(1) And Asclong(c1(0)) <= Asclong(v1(0)) - 1 'data label beside column
    If (Not btemp1) And (Not btemp2) Then
        bCategoryandValue = True
        GoTo FinalExit
    End If
End If
For I = 1 To Num - 1
    If myarray(I, 0) = "" Then
        ReDim c2(2)
        c2(0) = ""
        c2(1) = ""
        bc2isrange = False
    Else
        If InStr(1, myarray(0, 1), ":") = 0 Then
            bCLabelMorethanOneCell = True
            GoTo FinalExit
        End If
        c2 = SplitRange(myarray(I, 0))
    End If
    v2 = SplitRange(myarray(I, 2))
    If bc2isrange Then
        btemp1 = v1(0) = v1(2) And c2(0) = v2(0) And Asclong(c2(1)) <= Asclong(v2(1)) - 1 'data label beside row
        btemp2 = v2(1) = v2(3) And c2(1) = v2(1) And Asclong(c2(0)) <= Asclong(v2(0)) - 1 'data label beside column
        If (Not btemp1) And (Not btemp2) Then
            bCategoryandValue = True
            GoTo FinalExit
            'break
        End If
    End If
    If bc1isrange And bc2isrange Then
        'series data beside last series data in column and data label beside last series data label
        btemp1 = v2(0) = v2(2) And Asclong(c2(0)) = Asclong(c1(0)) + 1 And c2(1) = c1(1) And Asclong(v2(0)) = Asclong(v1(0)) + 1 And v1(1) = v2(1) And v1(3) = v2(3)
        'series data beside last series data in row and data label beside laast series data label
        btemp2 = v2(1) = v2(3) And c1(0) = c2(0) And Asclong(c2(1)) = Asclong(c1(1)) + 1 And Asclong(v2(1)) = Asclong(v1(1)) + 1 And v1(0) = v2(0) And v1(2) = v2(2)
        If (Not btemp1) And (Not btemp2) Then
            bCategoryandValue = True
            GoTo FinalExit
        End If
    ElseIf Not bc2isrange Then
        btemp1 = v2(0) = v2(2) And Asclong(v2(0)) = Asclong(v1(0)) + 1 And v1(1) = v2(1) And v1(3) = v2(3) 'series data beside last series data  in column
        btemp2 = v2(1) = v2(3) And Asclong(v2(1)) = Asclong(v1(1)) + 1 And v1(0) = v2(0) And v1(2) = v2(2) 'series data beside last series data in row
        If (Not btemp1) And (Not btemp2) Then
            bCategoryandValue = True
            GoTo FinalExit
        End If
    End If
        For j = 0 To 1
            c1(j) = c2(j)
        Next j
        For j = 0 To 3
            v1(j) = v2(j)
        Next j
        bc1isrange = bc2isrange
        bc2isrange = True
        
Next I
FinalExit:
Exit Function
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function
Private Function SplitRange(a As String) As String()
On Error GoTo HandleErrors
Dim currentFunctionName As String
currentFunctionName = "SplitRange"

Dim c1 As Integer, c2 As Integer, c3 As Integer
Dim start As Integer
Dim l As Integer
Dim rearray() As String

start = 2
If a <> "" Then
    l = InStr(1, a, ":")
    If l = 0 Then
        ReDim rearray(2)
        c1 = InStr(start, a, "$")
        rearray(0) = Mid(a, start, c1 - start)
        rearray(1) = Mid(a, c1 + 1, Len(a) - c1)
    Else
        ReDim rearray(4)
        c1 = InStr(start, a, "$")
        rearray(0) = Mid(a, start, c1 - start)
        c2 = InStr(c1 + 1, a, "$")
        rearray(1) = Mid(a, c1 + 1, c2 - c1 - 2)
        c3 = InStr(c2 + 1, a, "$")
        rearray(2) = Mid(a, c2 + 1, c3 - c2 - 1)
        rearray(3) = Mid(a, c3 + 1, Len(a) - c3)
    End If
Else
    ReDim rearray(4)
    rearray(0) = ""
    rearray(1) = ""
    rearray(2) = ""
    rearray(3) = ""
End If
SplitRange = rearray

Exit Function
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function
Private Function Asclong(s As String) As Integer
On Error GoTo HandleErrors
Dim currentFunctionName As String
currentFunctionName = "Asclong"
Asclong = 0

Dim l As Integer
Dim I As Integer
Dim m As String

l = Len(s)

For I = 1 To l
    m = Mid(s, I, 1)
    Asclong = Asclong + Asc(m)
Next I
Exit Function

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function
Private Function SheetCheck(sh As Variant, Values() As Variant) As Boolean
On Error GoTo HandleErrors
Dim currentFunctionName As String
currentFunctionName = "SheetCheck"
SheetCheck = False

Dim c1 As Integer
Dim I As Integer

Dim temp

For I = 0 To 2
    If IsRange(Values(I)) Then
        c1 = InStr(1, Values(I), "!")
        If sh = "" Then
            sh = Mid(Values(I), 1, c1 - 1)
            temp = Mid(Values(I), 1, c1 - 1)
        Else
            temp = Mid(Values(I), 1, c1 - 1)
        End If
        If temp <> sh Then
            SheetCheck = True
            Exit Function
        End If
    End If
Next I
Exit Function

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function
Private Function IsRange(Ref) As Boolean
On Error GoTo HandleErrors
Dim currentFunctionName As String
currentFunctionName = "IsRange"

Dim x As Range

On Error Resume Next
Set x = Range(Ref)
If Err = 0 Then
    IsRange = True
Else
    IsRange = False
End If
FinalExit:
    Set x = Nothing
    Exit Function

HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function
Private Function IsPie(myChart As Chart) As Boolean
On Error GoTo HandleErrors
Dim currentFunctionName As String
currentFunctionName = "IsPie"
Dim ctype As Integer
    IsPie = False
    
    ctype = myChart.ChartType
    If (ctype = xlPie) Or _
        (ctype = xlPieExploded) Or _
        (ctype = xlPieOfPie) Or _
        (ctype = xl3DPie) Or _
        (ctype = xl3DPieExploded) Then
        
        IsPie = True
    End If
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & mAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
End Function

Private Function IsOldVersion(aFormat As XlFileFormat) As Boolean
    Dim theResult As Boolean
    Dim currentFunctionName As String
    currentFunctionName = "IsOldVersion"
    
    Select Case aFormat
    Case xlExcel2, xlExcel2FarEast, xlExcel3, xlExcel4, xlExcel4Workbook, xlExcel5, xlExcel7
        theResult = True
    Case xlExcel9795, xlWorkbookNormal
        theResult = False
    Case Else
        WriteDebug currentFunctionName & " : " & mAnalysis.name & ": The version of this spreadsheet is not recognised"
    End Select
    
    IsOldVersion = theResult
End Function


