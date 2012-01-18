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
'*
'* short description :
'*
'\***********************************************************************
testcase tTaskPane

    Dim sExtensionPath as String
    Dim iStatus as integer

    sExtensionPath = gTesttoolPath + "extensions/optional/input/extensions/TaskPaneComponent/TaskPaneComponent.oxt"

    iStatus = hExtensionAddGUI( sExtensionPath, "InstallForUser, NoLicense" )
    if(iStatus <= 0 ) then
        warnlog "extension could not installed. ErrorCode is " + iStatus
        goto endsub
    endif

    call hNewDocument

    kontext "DocumentCalc"

    DocumentCalc.useMenu

    hMenuSelectNr(3)
    if(hMenuItemIsChecked(11)) then
        ' do nothing. The pane is displayed
    else
        hMenuSelectNr(11)
    endif

    kontext "MSC_Special_Dialog"
    if ( MSC_Special_Dialog.exists(1) ) then
        printlog "task pane exists"
    else
        warnlog "task pane does not exists"
    endif
    
    call hCloseDocument

    iStatus = hExtensionRemoveGUI("TaskPaneComponent.oxt")
    if(iStatus <> 0 ) then
        warnlog "extension could not deinstalled. ErrorCode is " + iStatus
        goto endsub
    endif
    
endcase
