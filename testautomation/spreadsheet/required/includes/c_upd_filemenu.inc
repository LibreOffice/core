'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: c_upd_filemenu.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-07-31 19:05:11 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/***********************************************************************
'*
'* owner : oliver.craemer@sun.com
'*
'* short description : Resource Test - File Menu
'*
'************************************************************************
'*
' #1 tFileNewSpreadsheet     
' #1 tFileNewSpreadsheetFromTemplate
' #1 tFileOpen 
' #1 tFileOpenCSV
' #1 tFileOpenDBF
' #1 tFileRecentDocuments
' #1 tFileClose
' #1 tFileSave
' #0 tFileSaveAsDBF
' #1 tFileVersions
' #1 tFilePagePreview
' #1 tFilePrint
'*
'\***********************************************************************

sub c_upd_filemenu

    Printlog Chr(13) + "--------- FILE Menu (c_upd_filemenu.inc) ---------"
    
    Call tFileNewSpreadsheet
    Call tFileNewSpreadsheetFromTemplate
    Call tFileOpen
    Call tFileOpenCSV
    Call tFileOpenDBF
    Call tFileRecentDocuments
    Call tFileClose
    Call tFileSave
    Call tFileSaveAsDBF
    Call tFileVersions
    Call tFilePagePreview
    Call tFilePrint

end sub

'-----------------------------------------------------------

testcase tFileNewSpreadsheet
    
    '/// Opening new spreadsheet document for getting defined starting environment
    Printlog " Opening new spreadsheet document for getting defined starting environment"
    Call hNewDocument
    '/// Open new document via Menu FILE-NEW-SPREADSHEET
    Printlog " Open new document via Menu FILE-NEW-SPREADSHEET"
    Kontext "DocumentCalc"
    DocumentCalc.UseMenu 'Selecting Menubar
    hMenuSelectNr(1) 'Using first entry on Menubar (FILE)
    hMenuSelectNr(1) 'Using first entry on menu (NEW)
    hMenuSelectNr(2) 'Using second entry on menu (SPREADSHEET) 
    Kontext "DocumentCalc"
    Call DialogTest (DocumentCalc)
    '/// Close document
    Printlog " Close document"
    Call hCloseDocument
    '/// Close starting document
    Printlog " Close starting document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFileNewSpreadsheetFromTemplate

    '/// Opening new spreadsheet document for getting defined starting environment
    Printlog " Opening new spreadsheet document for getting defined starting environment"
    Call hNewDocument
    '/// Open new document via Menu FILE-NEW-TEMPLATES AND DOCUMENTS
    Printlog " Open new document via Menu FILE-NEW-FROM TEMPLATES AND DOCUMENTS"
    Kontext "DocumentCalc"
    DocumentCalc.UseMenu 'Selecting Menubar
    hMenuSelectNr(1) 'Using first entry on Menubar (FILE)
    hMenuSelectNr(1) 'Using first entry on menu (NEW)
    hMenuSelectNr(12) 'Using second entry on menu (TEMPLATES AND DOCUMENTS) 
    Kontext "TemplateAndDocuments"
    Call DialogTest (TemplateAndDocuments)
    '/// Cancel dialog
    Printlog " Cancel dialog"
    TemplateAndDocuments.Cancel
    '/// Opening TemplateAndDocuments-Dialog via slot FileNewFromTemplate
    Printlog " Opening TemplateAndDocuments-Dialog via slot FileNewFromTemplate"
    Kontext "DocumentCalc"
    FileNewFromTemplate
    Kontext "TemplateAndDocuments"
    Call DialogTest (TemplateAndDocuments)
    '/// Cancel dialog
    Printlog " Cancel dialog"
    TemplateAndDocuments.Cancel
    '/// Close starting document
    Printlog " Close starting document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFileOpen

    '/// Opening new spreadsheet document for getting defined starting environment
    Printlog " Opening new spreadsheet document for getting defined starting environment"
    Call hNewDocument
    '/// Open File-Open-dialog
    Printlog " Open File-Open-dialog"
    FileOpen
    Kontext "OeffnenDlg"
    Call DialogTest ( OeffnenDlg )
    '/// Click "Up one level"
    Printlog " Click Up one level"
    UebergeordneterOrdner.Click
    '/// Click "Default directory"
    Printlog " Click Default directory"
    Standard.Click
    '/// Click "New Folder"
    Printlog " Click New Folder"
    NeuerOrdner.Click
    Kontext "NeuerOrdner"
    Call DialogTest ( NeuerOrdner )
    '/// Cancel New Folder
    Printlog " Cancel New Folder"
    NeuerOrdner.Cancel
    Kontext "OeffnenDlg"
    '/// Check "Read only"
    Printlog " Check Read only"
    NurLesen.check
    '/// Close File-Open-dialog with Cancel
    Printlog " Close File-Open-dialog with Cancel"
    OeffnenDlg.Cancel
    '/// Close starting document
    Printlog " Close starting document"
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------

testcase tFileOpenCSV

    dim sTestFile as string
    
    sTestFile = convertpath(gTesttoolPath & "spreadsheet/required/input/tFileOpen_CSV.csv"
    '/// Opening new spreadsheet document for getting defined starting environment
    Printlog " Opening new spreadsheet document for getting defined starting environment"
    Call hNewDocument
    '/// Open File-Open-dialog
    Printlog " Open File-Open-dialog"
    FileOpen
    Kontext "OeffnenDlg"
    '/// Enter filename  spreadsheet/required/input/tFileOpen_CSV.csv
    Printlog " Enter filename  spreadsheet/required/input/tFileOpen_CSV.csv"
    Dateiname.settext (sTestFile)
    '/// Click on Open
    Printlog " Click on Open"
    Oeffnen.Click
    '/// Check existence of CSV import dialog
    Printlog " Check existence of CSV import dialog"
    Kontext "TextImport"
    if Not TextImport.exists(2) then
        'Verify if the file will be loaded in Writer instead
        Kontext "ASCIIFilterOptionen"
        if ASCIIFilterOptionen.exists(2) then
            warnlog "Filterdetection for CSV is broken. Textfilter is used instead"
            ASCIIFilterOptionen.Cancel
            Call hCloseDocument
            goto endsub
        else
            Kontext "DocumentWriter"
            if DocumentWriter.exists(2) then
                warnlog "CSV import has failed, file is loaded in Writer"
                'Close Writer document
                Call hCloseDocument
                'Close starting document
                Call hCloseDocument
                goto endsub
            else
                Kontext
                if Active.exists then 
                    Active.no
                    Call hCloseDocument
                    goto endsub
                end if
            end if
        end if
    end if
    Call Dialogtest (TextImport)
    TextImport.ok
    Printlog " - CSV import dialog is in function"
    Kontext "DocumentCalc"
    '/// Close document
    printlog " Close document"
    Call hCloseDocument
        
endcase

'-----------------------------------------------------------

testcase tFileOpenDBF

    dim sTestFile as string
    
    sTestFile = convertpath(gTesttoolPath & "spreadsheet/required/input/tFileOpen_DBF.dbf"
    '/// Opening new spreadsheet document for getting defined starting environment
    Printlog " Opening new spreadsheet document for getting defined starting environment"
    Call hNewDocument
    '/// Open File-Open-dialog
    Printlog " Open File-Open-dialog"
    FileOpen
    Kontext "OeffnenDlg"
    '/// Enter filename  spreadsheet/required/input/tFileOpen_DBF.dbf
    Printlog " Enter filename  spreadsheet/required/input/tFileOpen_DBF.dbf"
    Dateiname.settext (sTestFile)
    '/// Click on Open
    Printlog " Click on Open"
    Oeffnen.Click
    '/// Check if there is no Text document opened
    Printlog " Check if there is no Text document opened"
    Kontext "DocumentWriter"
    if DocumentWriter.exists(2) then
        warnlog "Filter detection is broken. Text file opened in writer although DBF filter was selected."
        Call hCloseDocument
        '/// Close starting document
        Printlog " Close starting document"
        Call hCloseDocument
        goto endsub
    end if
    '/// Check existence of import dialog
    Printlog " Check existence of import dialog"
    Kontext "ExportCalc"
    Call Dialogtest (ExportCalc)
    if ExportCalc.exists(2) then
        ExportCalc.ok
        Printlog " - Import dialog is in function"
        Kontext "DocumentCalc"
        Call hCloseDocument
    else
        warnlog "DBF Import failed"
        '/// Close starting document
        Printlog " Close starting document"
        Call hCloseDocument
    end if
        
endcase

'-----------------------------------------------------------

testcase tFileRecentDocuments
    
    dim sTestFile as string
      
    '/// Open new Spreadsheet document
    Printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter some text
    Printlog " Enter some text"
    Kontext "DocumentCalc"
    DocumentCalc.Typekeys "The first doc!<RETURN>"
    Printlog " Copy Test File spreadsheet/required/input/recentdocument.ods locally"
    app.FileCopy convertpath(gTesttoolpath & "spreadsheet\required\input\recentdocument.ods"),gOfficePath + ConvertPath("user/work/recentdocument.ods")
    Printlog " Load local copied testfile recentdocument.ods"
    sTestFile = convertpath(gOfficePath & "user/work/recentdocument.ods")
    Call hFileOpen(sTestFile)
    '/// Close the document.
    Printlog " Close the document."
    '/// Now an item in <i>File / Recent Documents</i> should exist
    Call hCloseDocument
    '/// Reopen Document using the menu <i>File / Recent Documents</i>
    Printlog " Reopen Document using the menu File / Recent Documents"
    Kontext "DocumentCalc"
    DocumentCalc.UseMenu
    hMenuSelectNr(1)
    hMenuSelectNr(3)
    hMenuSelectNr(1)
    '/// Check that cell A1 has value "Recent document"
    Printlog " Check that cell A1 has value Recent document"    
    Kontext "DocumentCalc"
    if fCalcGetCellValue ("A1") = "Recent document" then
        Printlog " - Loading most recent Document seems to work"
    else
        'Sometimes the recent doc is loaded in background (#i80533#)
         if fCalcGetCellValue ("A1") = "The first doc!" then
            QAErrorLog "Recent document is loaded in background (#i80533#)"
            Kontext "DocumentCalc"
            DocumentCalc.UseMenu 
            hMenuSelectNr(8) 'Open Window menu
            hMenuSelectNr(hMenuItemGetCount) 'Select last loaded document
            'Check that cell A1 has value "Recent document"
            Call fCalcCompareCellValue ("A1","Recent document") 
        else
            warnlog "Loading most recent Document seems to fail"
        end if
    end if
    '/// Close recent document
    Printlog " Close recent document"
    Call hCloseDocument   
    '/// Close starting document
    Printlog " Close starting document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileClose

    '/// Open new Spreadsheet document
    Printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter some text
    Printlog " Enter some text"
    Kontext "DocumentCalc"
    DocumentCalc.Typekeys "Test <RETURN>"
    '/// Close document via File / Close
    Printlog " Close document via File / Close"
    FileClose
    '/// Close Messagebox via Cancel
    Printlog " Close Messagebox via Cancel"
    Kontext "Active"
    Active.Cancel
    Kontext "DocumentCalc"
    FileClose
    '/// Close Messagebox via Save
    Printlog " Close Messagebox via Save"
    Kontext "Active"
    Active.Yes
    '///+ Check if Save Dialog exists
    Printlog " Check if Save Dialog exists"
    Kontext "SpeichernDlg"
    if SpeichernDlg.Exists then
        SpeichernDlg.Cancel
    else
        Kontext "Active"
        if Active.Exists then
            Warnlog Active.Gettext
            Active.Ok
        else
            Warnlog "Dialog to Save document isn't up!"
        end if
    end if
    Kontext "DocumentCalc"
    FileClose
    '/// Close Messagebox via Discard
    Printlog " Close Messagebox via Discard"
    Kontext "Active"
    Active.No

endcase

'-----------------------------------------------------------

testcase tFileSave

    '/// Opening new spreadsheet document
    Printlog " Opening new spreadsheet document"
    Call hNewDocument
    '/// Enter some text
    Printlog " Enter some text"
    Kontext "DocumentCalc"
    DocumentCalc.Typekeys "Test <RETURN>"
    '///Open SAVE dialog via File-Save
    Printlog "Open SAVE dialog via File-Save"
    FileSave
    Kontext "SpeichernDlg"
    Call DialogTest (SpeichernDlg)
    '/// Click "Up one level"
    Printlog " Click Up one level"
    UebergeordneterOrdner.Click
    '/// Click "Default directory"
    Printlog " Click Default directory"
    Standard.Click
    '/// Click "New Folder"
    Printlog " Click New Folder"
    NeuerOrdner.Click
    Kontext "NeuerOrdner"
    Call DialogTest ( NeuerOrdner )
    '/// Cancel New Folder
    Printlog " Cancel New Folder"
    NeuerOrdner.Cancel
    Kontext "SpeichernDlg"
    '/// Enable save with passwort
    Printlog " Enable save with passwort"
    Passwort.Check
    '/// Enter filename tFileSave
    Printlog " Enter filename tFileSave"
    Dateiname.settext "tFileSave"
    '/// Click on SAVE button
    Printlog " Click on SAVE button"
    Speichern.Click
    Kontext "Passwort"    
    Call DialogTest (Passwort)
    '/// Set password
    PasswortName.SetText "OpenSource"
    '/// Now enter wrong confirmation
    PasswortBestaetigen.SetText "CloseSource"
    '/// Try to leave dialog with OK
    Passwort.Ok    
    '/// MsgBox warning for wrong password confirmation should be visible
    Kontext
    if Active.exists(4) then
        if Active.GetRT = 304 then
            '/// Confirm warning
            Printlog " - Wrong passwort has been detected successfully."
            Active.OK
        end if
    else
        warnlog "There is no messagebox because of a wrong password. Aborting!"
        Call hCloseDocument
        goto endsub        
    end if
    Kontext "Passwort"
    '/// Leave dialog with Cancel
    Passwort.Cancel
    Kontext "DocumentCalc"
    '/// Open SAVE dialog via File-Save again
    Printlog " Open SAVE dialog via File-Save again"
    FileSave
    Kontext "SpeichernDlg"
    '/// Close dialog with Cancel
    SpeichernDlg.Cancel
    '/// Close document
    Printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFileSaveAsDBF

    Dim UIFilter as string
    Dim bStatus as boolean

    Printlog " Open new Spreadsheet document"
    Call hNewDocument
    Printlog " Fill two cells (10, Test)"
    Kontext "DocumentCalc"
    DocumentCalc.Typekeys "10 <TAB> Test <RETURN>"
    Printlog " Open 'SaveAs' dialog"
    FileSaveAs
    Kontext "SpeichernDlg"
    Printlog " Enter filename 'dbf-Test_update'"
    DateiName.SetText "dbf-Test_update"
    Printlog " Choose file typ dBase"
    UIFilter = hGetUIFiltername ( "dBase" )
    bStatus = hSelectUIFilter ( UIFilter )
    if ( not bStatus ) then
        warnlog "Filter not found"
        SpeichernDlg.Cancel
        Call hCloseDocument
        goto endsub
    end if
    Printlog " Click on Save button"
    Speichern.click
    Printlog " Commit overwrite warning if file already exists"
    Kontext
    if Active.exists(2) then
        Active.Yes
    end if
    Printlog " Check alien file format warning"
    Kontext "AlienWarning"
    if AlienWarning.exists(2) then
    Printlog "  Commit if exists"
        AlienWarning.OK
    else 
        warnlog "Alien file format warning is missing -> Check this out!"
    end if
    Printlog " Check existence of export encoding dialog"
    Kontext "ExportCalc"
    Call DialogTest (ExportCalc)
    Printlog " Accept dialog by OK"
    ExportCalc.OK
    Kontext
    if Active.exists(2) then
        printlog " "
        warnlog "The following warning exists. This could be #i87325:"
        Printlog Active.GetText
        Printlog " "
        Active.OK
    end if
    Printlog " Close document"
    Call hCloseDocument 

endcase

'-----------------------------------------------------------

testcase tFileVersions

    '/// Open new Spreadsheet document
    Printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter some text
    Printlog " Enter some text"
    Kontext "DocumentCalc"
    DocumentCalc.Typekeys "Test <RETURN>"
    '/// Save document as "user/work/version.ods"
    Printlog " Save document as 'user/work/version.ods'"
    Call hFileSaveAsKill convertpath(gOfficePath & "user/work/version.ods")
    '/// Open File-Versions
    Printlog " Open File-Versions"
    FileVersions
    Kontext "Versionen"
    Call DialogTest (Versionen)
    '/// Click 'Save New Versions' button
    Printlog " Click 'Save New Versions' button"
    Speichern.click
    Kontext "VersionskommentarEingeben"
    '/// Check existence of comments dialog
    Call DialogTest(VersionskommentarEingeben)
    '/// Cancel comments dialog
    Printlog " Cancel comments dialog"
    VersionskommentarEingeben.Cancel
    Kontext "Versionen"
    '/// Close versions dialog
    Printlog " Close versions dialog"
    Versionen.close
    '/// Close document
    Printlog " Close document"    
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFilePagePreview

    '/// Open new Spreadsheet document
    Printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter some text
    Printlog " Enter some text"
    Kontext "DocumentCalc"
    DocumentCalc.Typekeys "Test <RETURN>"
    '/// Open File-PagePreview
    Printlog " Open File-PagePreview"
    FilePageView
    Kontext "Previewbar"
    Call DialogTest (Previewbar)
'    Printlog " Switch to fullscreen mode"
'    GanzerBildschirm.Click
'    Kontext "FullScreenbar"
'    printlog " Disable fullscreen mode"
'    FullScreen.click
'    Kontext "Previewbar"
    '/// Enable and disable Margins (available only since cws_pagemargins)
    printlog " Enable and disable Margins (available only since cws_pagemargins)"
    try
        Margins.Click
    catch
        qaErrorLog "Pagemargins does not exists. Maybe version older then cws_pagemargins"
    endcatch
    '/// Close page preview with button in toolbar
    Printlog " Close page preview with button in toolbar"
    Seitenansicht.Click
    '/// Close document
    Printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFilePrint

    '/// Open new Spreadsheet document
    Printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter some text
    Printlog " Enter some text"
    Kontext "DocumentCalc"
    DocumentCalc.Typekeys "Test <RETURN>"
    '/// Open File-Print
    Printlog " Open File-Print"
    FilePrint
    '/// Look for warning regarding missing printer
    '/// -> If configuration is missing only this MsgBox is tested
    Kontext
    if Active.Exists(2) then
        if Active.GetRT = 304 then
            Printlog Active.GetText
            QAErrorLog "No printer is configured for this system"
        else
            Printlog Active.GetText
            warnlog "There is an unexpected warning!"
        end if
        '/// Close messagebox with OK
        Printlog " Close messagebox with OK"
        Active.OK
        '/// Cancel Print dialog
        Printlog " Cancel Print dialog"
        Kontext "DruckenDLG"
        DruckenDLG.Cancel
        '/// Close document
        Printlog " Close document"
        Call hCloseDocument
        goto endsub
    end if
    Kontext "DruckenDLG"
    '/// Check existence of print dilaog
    Call DialogTest ( DruckenDLG )
    '/// Invoke the print options dialog
    Printlog " Invoke the print options dialog"
    Kontext "DruckenDLG"
    Zusaetze.Click
    Kontext "TabPrintCalcOptions"
    '/// Check existence of print options dilaog
    Call DialogTest ( TabPrintCalcOptions )
    '/// Cancel print options dialog
    Printlog " Cancel print options dialog"
    TabPrintCalcOptions.Cancel
    '/// Check option 'Print to file'
    Printlog " Check option 'Print to file'"         
    Kontext "DruckenDLG"
    AusdruckInDatei.check
    '/// Commit print dialog
    Printlog " Commit print dialog"
    DruckenDLG.OK
    '/// (Save file dialog should be visible now)
    Kontext "SpeichernDLG"
    '/// Check existence of save dilaog
    Call DialogTest ( SpeichernDLG )
    '/// Cancel save dialog
    Printlog " Cancel save dialog"
    SpeichernDLG.Cancel
    '/// Close document
    Printlog " Close document"
    Call hCloseDocument
endcase
