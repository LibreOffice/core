'encoding UTF-8  Do not remove or change this line!
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

************
' #1 tInsertSlide
' #1 tInsertDuplicateSlide
' #1 tInsertLayer
'\*************************************************************************************

testcase tInsertSlide
    Call  hNewDocument
    InsertSlide
    sleep 2
    hTypekeys "<Pagedown>"
    sleep 2
    try
        EditDeleteSlide
        PrintLog "Slide 2 deleted"
    catch
        sleep 2
        WarnLog "No slide was inserted"
    endcatch
    hCloseDocument
endcase

testcase tInsertDuplicateSlide
    Call  hNewDocument
    sleep 2
    hRechteckErstellen ( 10, 10, 50, 50 )
    sleep 2
    InsertDuplicateSlide
    try
        EditSelectAll
        sleep 2
        hTypeKeys "<DELETE>"
        PrintLog "Slide duplicated"
    catch
        Warnlog " Slide not duplicated"
    endcatch
    try
        EditDeleteSlide
    catch
        WarnLog "Error when deleting. Slide is not duplicated ??"
    endcatch
    hCloseDocument
endcase

testcase tInsertLayer
    Call hNewDocument
    sleep 2
    ViewLayer
    InsertLayer
    Kontext "EbeneEinfuegenDlg"
    EbeneEinfuegenDlg.OK
    sleep 2
    try
        EditDeleteLayer
        Kontext "Active"
        Active.Yes
    catch
        WarnLog "There is no avaiable additional layer to delete"
    endcatch
    hCloseDocument
endcase
