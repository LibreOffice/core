'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_loadsave.inc,v $
'*
'* $Revision: 1.1.4.3 $
'*
'* last change: $Author: fredrikh $ $Date: 2008/10/06 18:36:45 $
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
'/************************************************************************
'*
'* owner : fredrik.haegg@sun.com
'*
'* short description : Checks the export to all available Filters from 
'*
'***********************************************************************
'
' #1 tExportAllReadableFormatsIntoODF
' #1 tExportAllReadableFormatsIntoThemselves
' #1 tExportTop5FormatsIntoTop5
'
'\***********************************************************************

testcase tExportAllReadableFormatsIntoODF

    Dim AvailableFilters( 35 ) as String
    Dim TemplateFilterFile as String
    Dim CurrentFile as String
    Dim ImportFileList ( 300 ) as String
    Dim ExportFileList ( 300 ) as String
    Dim WorkDirectory as String
    Dim ImportDir as String
    Dim SavedCorrectly as Boolean
    Dim i as Integer

    TemplateFilterFile = ConvertPath ( gTesttoolPath + "writer\optional\input\filterlist.txt")
    WorkDirectory = ( ConvertPath ( gOfficePath + "user\work\" + gPlatgroup + "\export\"))
    ImportDir = ConvertPath ( gTesttoolPath + "writer\optional\input\import\")

    if gSamePC = TRUE then ' delete export directory
        GetFileList ( WorkDirectory, "*.*", ExportFileList() )
        if KillFileList ( ExportFileList() ) <> TRUE then
            Warnlog "Couldn't delete all Files in Output-Directory, the followings are still there:"
            for i=1 to FileList(0)
                printlog "  <> " & FileList(i)
            next i
        end if
    end if
    app.Mkdir WorkDirectory

    Printlog " Read the directory with files we wish to import."
        GetFileList ( ImportDir, "*.*", ImportFileList() )
    printlog "  Done."

    Printlog " Choose a filter."
    For i = 1 to listCount(ImportFileList())
        currentfile = ImportFileList(i)
        try
            Call hFileOpen ( ImportFileList(i), false )

            Kontext "TextImport"
            if TextImport.Exists then
                TextImport.Ok
            end if

            Kontext "Filterauswahl"
            if Filterauswahl.Exists then
                Filterauswahl.Cancel
            end if

            Kontext "AsciiFilterOptionen"
            if AsciiFilterOptionen.Exists then AsciiFilterOptionen.Ok
            Kontext "SecurityWarning"
            if SecurityWarning.Exists(3) then SecurityWarning.Ok
            Kontext "DocumentWriter"
            if DocumentWriter.Exists(3) then
                Do until DocumentWriter.StatusIsProgress = false
                    wait 500
                Loop
            else
                Kontext "DocumentCalc"
                if DocumentCalc.Exists(3) then
                    Do until DocumentCalc.StatusIsProgress = false
                        wait 500
                    Loop
                else
                    Warnlog "Neither DocumentWriter or DocumentCalc existed? Some other dialogue in focus?"
                end if
            end if

            ' Check for macro alert
            Kontext "SecurityWarning"
            if SecurityWarning.Exists then
                if inStr(currentfile,"sw40") = false then
                    QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
                end if
                SecurityWarning.Cancel
            end if
            Sleep 1

            Kontext "AlienWarning"
            if AlienWarning.Exists then AlienWarning.Ok

            Printlog " Exporting file: " + ImportFileList(CurrentFile)

            Printlog " Save as ( ODF )"
            printlog (ConvertPath (WorkDirectory + "ExportedFile" + i))
            Call hFileSaveAsWithFilterKill (ConvertPath (WorkDirectory + "ExportedFile" + i), "writer8")

            Printlog " Close file."
            Call hCloseDocument

            Printlog " Open Saved File."
            Call hFileOpenWithFilter (ConvertPath (WorkDirectory + "ExportedFile" + i), "writer8", false)

            ' Check for macro alert
            Kontext "SecurityWarning"
            if SecurityWarning.Exists then
                if inStr(currentfile,"sw40")  = false then
                    QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
                end if
                SecurityWarning.Cancel
            end if
            Sleep 1

            printlog "   Close all open files."
            Do Until GetDocumentCount = 0
                Call hCloseDocument
            Loop
        catch
            Warnlog "Error with file: " + ImportFileList(i)
        endcatch
    next i 'File

endcase

'---------------------------------------------------------------------------------------------------------------------

testcase tExportAllReadableFormatsIntoThemselves

    Dim AvailableFilters( 35 ) as String
    Dim TemplateFilterFile as String
    Dim SavedCorrectly as Boolean
    Dim ImportFileList ( 300 ) as String
    Dim ExportFileList ( 300 ) as String
    'Used - below
    Dim sLoadFile as String
    Dim sSaveFile as String
    Dim sCurrentFile as String
    Dim sCurrentFilter as String
    Dim sWorkDirectory as String
    Dim sImportDir as String
    Dim iCurrentFileIndex as Integer
    Dim iCounter as integer
    Dim DocumentCount as integer

    TemplateFilterFile = ConvertPath ( gTesttoolPath + "writer\optional\input\filterlist.txt")
    sWorkDirectory = ( ConvertPath ( gOfficePath + "user\work\" + gPlatgroup + "\export\"))
    sImportDir = ConvertPath ( gTesttoolPath + "writer\optional\input\import\")

    if gSamePC = TRUE then ' delete export directory
        GetFileList ( sWorkDirectory, "*.*", ExportFileList() )
        if KillFileList ( ExportFileList() ) <> TRUE then
            Warnlog "Couldn't delete all Files in Output-Directory, the followings are still there:"
            for iCounter=1 to FileList(0)
                printlog "  <> " & FileList(i)
            next iCounter
        end if
    end if
    app.Mkdir sWorkDirectory

    For iCurrentFileIndex = 1 to 17
        select case iCurrentFileIndex
                    case 1 :   sCurrentFile = "xml2.odt"
                                    sCurrentFilter = "writer8"
                    case 2 :    sCurrentFile = "wpsfile.wps"
                                    sCurrentFilter = "Text (encoded)"
                    case 3 :    sCurrentFile = "dostext.txt"
                                    sCurrentFilter = "Text (encoded)"
                    case 4 :   sCurrentFile = "sw30.sdw"
                                     sCurrentFilter = "StarWriter 3.0"
                    case 5 :   sCurrentFile = "sw50.vor"
                                     sCurrentFilter = "StarWriter 5.0 Vorlage/Template"
                    case 6 :   sCurrentFile = "sw31.sdw"
                                     sCurrentFilter = "StarWriter 3.0"
                    case 7 :   sCurrentFile = "sw31.vor"
                                     sCurrentFilter = "StarWriter 3.0 Vorlage/Template"
                    case 8 :   sCurrentFile = "sw40.sdw"
                                     sCurrentFilter = "StarWriter 4.0"
                    case 9 :   sCurrentFile = "sw40_sp2.vor"
                                     sCurrentFilter = "StarWriter 4.0 Vorlage/Template"
                    case 10 : sCurrentFile = "sw50.sdw"
                                     sCurrentFilter = "StarWriter 5.0"
                    case 11 : sCurrentFile = "pocketword.psw"
                                     sCurrentFilter = "PocketWord File"
                    case 12 : sCurrentFile = "rtf.rtf"
                                     sCurrentFilter = "Rich Text Format"
                    case 13 : sCurrentFile = "sw60.sxw"
                                     sCurrentFilter = "StarOffice XML (Writer)"
                    case 14 : sCurrentFile = "winw97.doc"
                                     sCurrentFilter = "MS Word 97"
                    case 15 : sCurrentFile = "html/html.html"
                                     sCurrentFilter = "HTML (StarWriter)"
                    case 16 : sCurrentFile = "winword6.doc"
                                     sCurrentFilter = "MS Word 95"
                    case 17 : sCurrentFile = "wintext.txt"
                                     sCurrentFilter = "Text (encoded)"

'TODO: 18-23 not applyable. Can be opened, but saving in the format is not supported.

'                    case 18 : sCurrentFile = "unixtext.txt"
'                                     sCurrentFilter = "Text"
'                    case 19 : sCurrentFile = "sw2.sdw"
'                                     sCurrentFilter = "StarWriter 2.0"
'                    case 20 : sCurrentFile = "amipro3.sam"
'                                     sCurrentFilter = "Ami Pro 1.x-3.1 (W4W)"
'                    case 21 : sCurrentFile = "mactext.txt"
'                                     sCurrentFilter = "Mac Write 4.x 5.0 (W4W)"
'                    case 22 : sCurrentFile = "sw1.sdw"
'                                     sCurrentFilter = "StarWriter 1.0"
'                    case 23 : sCurrentFile = "swdoc.txt"
'                                     sCurrentFilter = "Text (encoded) (StarWriter/GlobalDocument)"
'                    case 24 : sCurrentFile = "hangul.hwp"
'                                     sCurrentFilter = "writer_MIZI_Hwp_97"
        end select

        printlog " - Export of File nr " + iCurrentFileIndex + " started."
        
        sLoadFile = sImportDir & sCurrentFile
        sSaveFile = sWorkDirectory & "filenr_" & iCurrentFileIndex

        try 

            ' Load In-file
            Call hFileOpenWithFilter(sLoadFile, sCurrentFilter, false)

            kontext "active"
            if active.exists(2) then 
                active.ok
                warnlog "Error with file " + sLoadFile + " as " + sSaveFile + "."
            end if

                Kontext "TextImport"
                if TextImport.Exists then
                    TextImport.Ok
                end if

                Kontext "Filterauswahl"
                if Filterauswahl.Exists then
                    Filterauswahl.Cancel
                end if

                Kontext "AsciiFilterOptionen"
                if AsciiFilterOptionen.Exists then AsciiFilterOptionen.Ok
                Kontext "SecurityWarning"
                if SecurityWarning.Exists(3) then SecurityWarning.Ok
                Kontext "DocumentWriter"
                if DocumentWriter.Exists(3) then
                    Do until DocumentWriter.StatusIsProgress = false
                        wait 500
                    Loop
                else
                    Kontext "DocumentCalc"
                    if DocumentCalc.Exists(3) then
                        Do until DocumentCalc.StatusIsProgress = false
                            wait 500
                        Loop
                    else
                        Warnlog "Neither DocumentWriter or DocumentCalc existed? Some other dialogue in focus?"
                    end if
                end if

                ' Check for macro alert
                Kontext "SecurityWarning"
                if SecurityWarning.Exists then
                    if inStr(currentfile,"sw40") = false then
                        QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
                    end if
                    SecurityWarning.Cancel
                end if
                Sleep 1

                Kontext "AlienWarning"
                if AlienWarning.Exists then AlienWarning.Ok

            ' Save Out-file
            Call hFileSaveAsWithFilterKill(sSaveFile, sCurrentFilter)
            
            kontext "active"
            if active.exists(2) then 
                active.ok
                warnlog "Error saving file " + sLoadFile + " as " + sSaveFile + "."
                goto Cleanup
            end if

            ' Load Out-file again
            Call hFileOpenWithFilter(sSaveFile, sCurrentFilter, false)

            ' Check for macro alert
            Kontext "SecurityWarning"
            if SecurityWarning.Exists then
                if inStr(currentfile,"sw40") > 0 then
                    QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
                end if
                SecurityWarning.Cancel
            end if
            Sleep 1

        catch
            Warnlog "Error with file: " + sLoadFile
        endcatch
        
        Cleanup:
        kontext "active"
        if active.exists(2) then 
            active.ok
            warnlog "Error with file " + sLoadFile + " as " + sSaveFile + "."
        end if

        printlog "   Close all open files."
        Do Until GetDocumentCount = 0
            Call hCloseDocument
        Loop

        printlog " - File nr " + iCurrentFileIndex + " completed."
    Next iCurrentFileIndex

endcase

'---------------------------------------------------------------------------------------------------------------------

testcase tExportTop5FormatsIntoTop5

    Dim ExportFileList(300) as String
    Dim sWorkDirectory as String
    Dim sImportDir as String
    Dim iCurrentLoadFileIndex as Integer
    Dim sCurrentLoadFile as String
    Dim sCurrentLoadFilter as String
    Dim iCurrentSaveFileIndex as Integer
    Dim sCurrentSaveFile as String
    Dim sCurrentSaveFilter as String
    Dim iCounter as Integer
    Dim DocumentCount as Integer
    Dim FileList as String

    sWorkDirectory = ( ConvertPath ( gOfficePath + "user\work\" + gPlatgroup + "\loadsavetop5\"))
    sImportDir = ConvertPath ( gTesttoolPath + "writer\optional\input\import\")

    if gSamePC = TRUE then ' delete export directory
        GetFileList ( sWorkDirectory, "*.*", ExportFileList() )
        if KillFileList ( ExportFileList() ) <> TRUE then
            Warnlog "Couldn't delete all Files in Output-Directory, the followings are still there:"
            for iCounter=1 to FileList(0)
                printlog "  <> " & FileList(i)
            next iCounter
        end if
    end if
    app.Mkdir sWorkDirectory

    '// Start of the LOAD-Loop
    For iCurrentLoadFileIndex = 1 to 5
        Select case iCurrentLoadFileIndex
                    case 1 :  sCurrentLoadFile = "xml2.odt"
                                    sCurrentLoadFilter = "writer8"
                    case 2 :  sCurrentLoadFile = "sw60.sxw"
                                    sCurrentLoadFilter = "StarOffice XML (Writer)"
                    case 3 :  sCurrentLoadFile = "winw97.doc"
                                    sCurrentLoadFilter = "MS Word 97"
                    case 4 :  sCurrentLoadFile = "rtf.rtf"
                                    sCurrentLoadFilter = "Rich Text Format"
                    case 5 :  sCurrentLoadFile = "html\html.html"
                                    sCurrentLoadFilter = "HTML (StarWriter)"
        End select

        printlog " - Export of File nr " + iCurrentLoadFileIndex + " started."

        call fLoadTheFile((sImportDir & sCurrentLoadFile), sCurrentLoadFilter)

        '///  Here comes the SAVE-Loop
        For iCurrentSaveFileIndex = 1 to 5
                Select case iCurrentSaveFileIndex
                            case 1 : sCurrentSaveFile = "SavedFile_l" + iCurrentLoadFileIndex + "_s1.odt"
                                           sCurrentSaveFilter = "writer8"
                            case 2 : sCurrentSaveFile = "SavedFile_l" + iCurrentLoadFileIndex + "_s2.sxw"
                                           sCurrentSaveFilter = "StarOffice XML (Writer)"
                            case 3 : sCurrentSaveFile = "SavedFile_l" + iCurrentLoadFileIndex + "_s3.doc"
                                           sCurrentSaveFilter = "MS Word 97"
                            case 4 : sCurrentSaveFile = "SavedFile_l" + iCurrentLoadFileIndex + "_s4.rtf"  
                                           sCurrentSaveFilter = "Rich Text Format"
                            case 5 : sCurrentSaveFile = "SavedFile_l" + iCurrentLoadFileIndex + "_s5.html"
                                           sCurrentSaveFilter = "HTML (StarWriter)"
                End select

                Call hFileSaveAsWithFilterKill((sWorkDirectory & sCurrentSaveFile), sCurrentSaveFilter)

                if hFileExists (sWorkDirectory & sCurrentSaveFile) then

                    'Check if the saved file can be loaded
                    call fLoadTheFile((sWorkDirectory & sCurrentSaveFile), sCurrentSaveFilter)

                    'Close the opened file
                    Do Until GetDocumentCount = 1
                        Call hCloseDocument
                    Loop
                end if

        Next iCurrentSaveFileIndex
        '///  Here ends the Save-Loop

        Cleanup:
        kontext "active"
        if active.exists(2) then
            active.ok
            warnlog "Error. Last file processed: " + sCurrentLoadFile + " as " + sCurrentSaveFile + "."
        end if

        printlog "   Close all open files."

        Do Until GetDocumentCount = 0
            Call hCloseDocument
        Loop

        printlog " - File nr " + iCurrentLoadFileIndex + " completed."
    Next iCurrentLoadFileIndex
    '// End of the LOAD-Loop.
endcase

'---------------------------------------------------------------------------------------------------------------------

function fLoadTheFile(sLoadFile as string, sCurrentFilter as string)

        try
            ' Load In-file
            Call hFileOpenWithFilter(sLoadFile, sCurrentFilter, false)

            kontext "active"
            if active.exists(2) then 
                active.ok
                warnlog "Error with file " + sLoadFile + " as " + sSaveFile + "."
            end if

                Kontext "TextImport"
                if TextImport.Exists then
                    TextImport.Ok
                end if

                Kontext "Filterauswahl"
                if Filterauswahl.Exists then
                    Filterauswahl.Cancel
                end if

                Kontext "AsciiFilterOptionen"
                if AsciiFilterOptionen.Exists then AsciiFilterOptionen.Ok
                Kontext "SecurityWarning"
                if SecurityWarning.Exists(3) then SecurityWarning.Ok
                Kontext "DocumentWriter"
                if DocumentWriter.Exists(3) then
                    Do until DocumentWriter.StatusIsProgress = false
                        wait 500
                    Loop
                else
                    Kontext "DocumentCalc"
                    if DocumentCalc.Exists(3) then
                        Do until DocumentCalc.StatusIsProgress = false
                            wait 500
                        Loop
                    else
                        Warnlog "Neither DocumentWriter or DocumentCalc existed? Some other dialogue in focus?"
                    end if
                end if

                ' Check for macro alert
                Kontext "SecurityWarning"
                if SecurityWarning.Exists then
                    if inStr(currentfile,"sw40") > 0 then
                        QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
                    end if
                    SecurityWarning.Cancel
                end if
                Sleep 1

                Kontext "AlienWarning"
                if AlienWarning.Exists then AlienWarning.Ok

        catch
            Warnlog "Error with file: " + sLoadFile
        endcatch
end function 'fLoadTheFile

'------------------------------------------------------------------------------------------------------------------------------------------------

