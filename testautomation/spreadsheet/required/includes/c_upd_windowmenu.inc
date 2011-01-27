'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
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


