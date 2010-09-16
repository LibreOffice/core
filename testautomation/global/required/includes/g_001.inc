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
'*
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description : Global resource tests for the menu: File
'*
'*************************************************************************
'*
' #1 tFileExportAsPDF
' #1 tExportAsPDFButton
'*
'\************************************************************************

testcase tFileExportAsPDF
    dim sPDF as string
    dim sTemp as string

    sPDF = "PDF - Portable Document Format (.pdf)"

    '/// open application ///'
    Call hNewDocument

    '/// choose File->Export As PDF ... ///'
    FileExportAsPDF
    ' The file dialog has to come up
    Kontext "SpeichernDlg"
    if SpeichernDlg.exists(5) then
        '/// the selected file type should be: 'PDF - Portable Document Format (.pdf)' ///'
        sTemp = Dateityp.GetSelText
        if (sTemp <> sPDF) then
            Warnlog "filter for PDF export is missing :-( should: '" + sPDF + "'; is: '" + sTemp + "'"
        endif
        '///+ - set Textbox 'File name' to "abc" ///'
        Dateiname.SetText "abc"
        speichern.click
        kontext
        if active.exists(5) then
            ' catch active about already existing file name
            if active.getrt = 304 then
                active.yes
            endif
        endif
    else
        ' changed with SRC680m210 - first options, then file dialog...
        ' kept for testcase backwards compatibility
        'warnlog "Export dialog didn't come up."
    endif
    kontext
    '/// dialog 'PDF Options' comes up ///'
    if active.exists(5) then
        '/// select the tab page 'General' ///'
        active.setPage PDFOptions
        kontext "PDFOptions"
        if PDFOptions.exists(5) then
            dialogTest(PDFOptions)
        else
            warnlog "Tab page 'PDF Options - General' isn't available."
        endif
    else
        warnlog "Dialog 'PDF Options' didn't come up"
    endif
    kontext
    if active.exists(5) then
        '/// select the tab page 'Initial View' ///'
        active.setPage PDFOptionsInitialView
        kontext "PDFOptionsInitialView"
        if PDFOptionsInitialView.exists(5) then
            dialogTest(PDFOptionsInitialView)
        else
            warnlog "Tab page 'PDF Options - Initial View' isn't available."
        endif
    endif
    kontext
    if active.exists(5) then
        '/// select the tab page 'User Interface' ///'
        active.setPage PDFOptionsUserInterface
        kontext "PDFOptionsUserInterface"
        if PDFOptionsUserInterface.exists(5) then
            dialogTest(PDFOptionsUserInterface)
        else
            warnlog "Tab page 'PDF Options - User Interface' isn't available."
        endif
    endif
    kontext
    if active.exists(5) then
        '/// select the tab page 'General' ///'
        active.setPage PDFOptions
        kontext "PDFOptions"
        '/// close the dialog with 'Cancel' ///'
        if PDFOptions.exists(5) then
            PDFOptions.cancel
        else
            warnlog "Tab page 'PDF Options - General' isn't available."
        endif
    endif
    '/// close application ///'
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tExportAsPDFButton
    Dim sTemp as string
    '/// Create new document
    Call hNewDocument
    '/// Click the button 'Export Directly as PDF' on the standard toolbar.
    Kontext "Standardbar"
    ExportAsPDF.click
    '///  The 'Export as PDF' dialog has to come up, with the only 'File type' 'PDF - Portable Document Format'
    Kontext "SpeichernDlg"
    if SpeichernDlg.exists(1) then
        sTemp = Dateityp.GetSelText
        if InStr(sTemp, "PDF") = 0 then
            warnlog "Filter for PDF export seems to be wrong or is missing in selection."
        endif
        '/// Leave dialog with CANCEL button
        SpeichernDlg.Cancel
    else
        warnlog "SaveAsPDF dialog did not come up."
    endif
    '/// Close spreadsheet document.
    Call hCloseDocument
endcase

