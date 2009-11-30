'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: ThisWorkbook.cls,v $
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

Public xlStrings As StringDataManager

Private Sub Workbook_Open()
    Set xlStrings = New StringDataManager
   
    xlStrings.InitStringData (GetResourceDataFileName(ThisWorkbook.Path))
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

