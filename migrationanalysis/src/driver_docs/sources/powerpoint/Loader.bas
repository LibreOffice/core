Attribute VB_Name = "Loader"
'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: Loader.bas,v $
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
