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
