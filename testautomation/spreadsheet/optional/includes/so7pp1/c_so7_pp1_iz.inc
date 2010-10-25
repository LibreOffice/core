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
'/************************************************************************
'**
'** owner : joerg.sievers@Sun.COM
'**
'** short description :  Test Issuezilla bug fixes
'**
'************************************************************************
' **
' #1 tIZ19381             ' I/O error when OLE object is thrown out of undo/redo buffer
' #1 tIZ21036             ' Crash after shifting cells with paste special
' **
'\***********************************************************************

testcase tIZ19381
    Dim sOutputFile as string
    Dim i as integer

    sOutputFile = convertpath(gOfficepath & "user/work/i19381.ods")
    printlog "IssueZilla Task 19381"

    '/// File / New / Spreadsheet
    call hNewDocument
    sleep(2)
    '/// Enter 1 [RETURN]
    '/// Enter 2 [RETURN]
    '/// Enter [UP]
    Kontext "DocumentCalc"
    DocumentCalc.typekeys "1<RETURN>2<RETURN><UP>"
    sleep(2)
    '/// Insert / Chart
    InsertChartCalc
    Kontext "ChartWizard"
    '/// On the chart wizard press OK button
    if ChartWizard.Exists(2) then
        printlog "Create chart"
        ChartWizard.OK
        sleep(2)
    else
        warnlog "Chart wizard did not occour! Exiting test case."
        call hCloseDocument
        goto endsub
    end if
    Kontext "DocumentCalc"
    '/// Type [ESCAPE]
    DocumentCalc.typekeys "<Escape>"
    sleep(2)
    '/// File Save As <i>gOfficepath</i>user/work/i19381.ods
    if NOT hFileSaveAsWithFilterKill (sOutputFile, "calc8") then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    sleep(2)
    '/// File / Close
    call hCloseDocument
    sleep(2)
    '/// Open the saved document
    if hFileOpen(sOutputFile) then
        Kontext "DocumentCalc"
        '/// Click into the chart
        call gMouseClick(95,95)
        Kontext "DocumentCalc"
        '/// Press [F5] to open the Navigator
        '/// Select the first Chart/OLE object in the navigator
        call fselectFirstOLE
        '/// Edit / Cut
        sleep(2)
        EditCut
        sleep(2)
        Kontext "DocumentCalc"
        '/// Type [ESCAPE] twice
        DocumentCalc.TypeKeys "<ESCAPE>" , 2
        '/// Edit / Paste
        EditPaste
        sleep(2)
        Kontext "DocumentCalc"
        '/// press [CTRL+S] to save the document again
        DocumentCalc.typekeys "<MOD1 S>"
        sleep(2)
        Kontext
        if Active.exists(2) then
            if gPlatGroup = "w95" then
                warnlog "REGRESSION! #i41751# - Saving after Cut&Paste of OLE fails on windows!"
            else
                warnlog "OOPS, found unexpected MsgBox -> Check this out!"
            end if
            Active.OK
            call hCloseDocument
            goto endsub
        end if
        Kontext "DocumentCalc"
        '/// Type [ESCAPE]
        DocumentCalc.TypeKeys "<ESCAPE>"
        '/// Go to cell E42
        call fCalcSelectRange("E42")
        for i = 1 to 21
            Kontext "DocumentCalc"
            '/// Enter 1 and [RETURN] 21 times
            '///+ as changes to get chart cut/paste out of the undo buffer
            DocumentCalc.typekeys "1<Return>"
        next i
        Kontext "DocumentCalc"
        '/// Type [CTRL + S] to save the document again
        DocumentCalc.typekeys "<MOD1 S>"
        '/// check for error message
        Kontext
        if Active.Exists(2) then
            if Active.GetRT = 304 then
                printlog Active.GetText
                warnlog "REGRESSION! Issue 19381 seems to occur again!"
                Active.OK
            end if
        else
            printlog "No message box is in the way!"
        end if
        '/// File / Close
        call hCloseDocument
    else
        warnlog "It was not possible to open the saved document again!"
    end if
endcase

'-------------------------------------------------------------------------

testcase tIZ21036

    printlog "IssueZilla Task 21036"

    '/// open new document
    call hNewDocument
    sleep(2)

    '/// select a whole row (e.g. A1:IV1)
    Kontext "DocumentCalc"
    DocumentCalc.typekeys "<SHIFT MOD1 RIGHT>"
    sleep(2)

    '/// cut selection
    try
        EditCut
    catch
        QAErrorLog "Is this a crash?? Please check it out!"
        goto endsub
    endcatch
    sleep(2)

    '/// go to A2
    Kontext "DocumentCalc"
    call fCalcSelectRange("A2")
    sleep(2)
    Kontext "DocumentCalc"
    '/// call edit->paste special
    EditPasteSpecialCalc
    sleep(2)

    '/// check shift cells->right
    Kontext "InhalteEinfuegenCalc"
    NachRechts.Check

    '/// OK
    InhalteEinfuegenCalc.OK

    '/// confirm MsgBox
    Kontext "Active"
    if Active.Exists(2) then
        if Active.GetRT = 304 then
            printlog Active.GetText
            Active.OK
            sleep(2)
        end if
    else
        warnlog "No message box came up!"
        goto endsub
    end if

    '/// check for crash
    try
        printlog "OK, there are still " & GetDocumentCount & " documents open"
    catch
        warnlog "Probably the office did crash -> #I21036#"
        goto endsub
    endcatch

    call hCloseDocument
endcase

