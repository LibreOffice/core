'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
'*
'* short description :
'*
'\*****************************************************************

sub d_003

    call tdViewPagePane()
    call tdViewSlide()

end sub

'*******************************************************************************

testcase tdViewPagePane

    printlog " open application "
    Call hNewDocument
    sleep 1
    kontext "pagepane"
    if (NOT pagepane.exists) then
        qaerrorlog "Pages Panel not visible on opening application. Opening now."
        ViewPagePane
    endif
    kontext "pagepane"
    sleep (2)
    try
        printlog " View->Page Pane "
        ViewPagePane
        sleep (2)
        if (pagepane.exists) then
            warnlog "View->Page Pane failed."
            ViewPagePane
        endif
    catch
        warnlog "View->Page Pane couldn't get executed"
    endcatch
    sleep 1
    if (NOT pagepane.exists) then
        ViewPagePane
        sleep (1)
    endif
    printlog " close application "
    Call hCloseDocument
    
endcase 'tdViewPagePane

'-------------------------------------------------------------------------------
testcase tdViewSlide

    printlog " open application "
    hNewDocument
    kontext "DocumentDrawImpress" ' special case :-)
    printlog " click the button on the bottom: 'Master View' (because it is not accessible via the menu :-() "
    ViewMasterPage
    sleep 1
    printlog " View->Slide "
    ViewPagePane
    Sleep 1
    printlog " close application "
    Call  hCloseDocument
   
endcase 'tdViewSlide
