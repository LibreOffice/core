Attribute VB_Name = "Loader"
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

Public ppStrings As StringDataManager

Public Sub Presentation_Open()
    On Error GoTo ErrorHandler
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
