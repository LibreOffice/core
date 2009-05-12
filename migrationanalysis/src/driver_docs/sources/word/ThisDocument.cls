'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: ThisDocument.cls,v $
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

Public wrdStrings As StringDataManager

Private Sub Document_Open()
    Set wrdStrings = New stringDataManager
   
    wrdStrings.InitStringData (GetResourceDataFileName(ThisDocument.Path))
    LoadCommonStrings wrdStrings
    LoadWordStrings wrdStrings
    LoadResultsStrings wrdStrings
    SetWordDriverText
    Set wrdString = Nothing
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
