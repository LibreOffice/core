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
'* short description : Resource Test - Window Menu
'*
'************************************************************************
'*
' #1 tWindowNewWindow
' #1 tWindowSplit
' #1 tWindowFreeze
'*
'\***********************************************************************

sub c_upd_windowmenu

    Printlog Chr(13) + "--------- Window Menu (c_upd_windowmenu.inc) ---------"
    
    call tWindowNewWindow
    call tWindowSplit
    call tWindowFreeze
    

end sub

'-----------------------------------------------------------

testcase tWindowNewWindow
'///<u><b>Window – New Window</b></u>

    Dim iDocumentcount as integer
    Dim iDocumentclose as integer

    '/// Verify that there is no document open
    printlog " Verify that there is no document open"
    iDocumentcount = getDocumentCount
    If iDocumentcount >0 then
        warnlog "There are " & iDocumentcount & " documents open, but there should be none. Closing these documents"
        For iDocumentclose = 1 to iDocumentcount
            Call hCloseDocument
        Next iDocumentclose
    end if
    '/// Opening new spreadsheet document for getting defined starting environment
    printlog " Opening new spreadsheet document for getting defined starting environment"
    Call hNewDocument
    '/// Open new window by 'Window – New Window'
    printlog " Open new window by 'Window – New Window'"
    WindowNewWindow
    '/// Count amount of windows (must be 2)
    printlog " Count amount of windows (must be 2)"
    If getDocumentCount <>2 then
        warnlog "There should be 2 windows, but there are " & getDocumentCount
    end if
    '/// Close new window by 'Window – Close Window'
    printlog " Close new window by 'Window – Close Window'"
    WindowCloseWindow
    '/// Count amount of windows (must be one)
    printlog " Count amount of windows (must be one)"
    If getDocumentCount <>1 then
        warnlog "There should be 1 window, but there are " & getDocumentCount
    end if
    '/// Close starting document
    printlog " Close starting document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tWindowSplit
'///<u><b>Window – Split</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Select cell E15
    printlog " Select cell E15"
    Call fCalcSelectRange ("E15")
    '/// Split window by 'Window – Split'
    printlog " Split window by 'Window – Split'"
    WindowSplit
    '/// Unsplit window by 'Window - Split'
    printlog " Unsplit window by 'Window - Split'"
    WindowSplit
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tWindowFreeze
'///<u><b>Window – Freeze</b></u>

'/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Select cell E15
    printlog " Select cell E15"
    Call fCalcSelectRange ("E15")
    '/// Split window by 'Window – Freeze'
    printlog " Split window by 'Window – Freeze'"
    WindowFreeze
    '/// Unsplit window by 'Window - Freeze'
    printlog " Unsplit window by 'Window - Freeze'"
    WindowFreeze
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------


