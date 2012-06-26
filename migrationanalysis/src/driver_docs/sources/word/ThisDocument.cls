'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
' This file incorporates work covered by the following license notice:
'
'   Licensed to the Apache Software Foundation (ASF) under one or more
'   contributor license agreements. See the NOTICE file distributed
'   with this work for additional information regarding copyright
'   ownership. The ASF licenses this file to you under the Apache
'   License, Version 2.0 (the "License"); you may not use this file
'   except in compliance with the License. You may obtain a copy of
'   the License at http://www.apache.org/licenses/LICENSE-2.0 .
'

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
