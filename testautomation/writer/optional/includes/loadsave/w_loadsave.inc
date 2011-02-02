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
'* owner : helge.delfs@oracle.com
'*
'* short description : Checks the export to all available Filters from
'*
'\***********************************************************************

testcase tExportAllReadableFormatsIntoODF

    if iSprache <> 1 then
        QAErrorlog "#100970#test only support english language"
        goto endsub
    end if

    Dim AvailableFilters( 35 ) as String
    Dim ImportFileList ( 300 ) as String
    Dim ExportFileList ( 300 ) as String
    Dim WorkDirectory as String
    Dim ImportDir as String
    Dim SavedCorrectly as Boolean
	Dim sExportFilter as string
	Dim sExportFile as string
    Dim sExceptionFile as string
    Dim i as Integer

    WorkDirectory = ( ConvertPath ( gOfficePath + "user\work\" + gPlatgroup + "\export\"))
    ImportDir = ConvertPath ( gTesttoolPath + "writer\optional\input\import\")

    if gSamePC = TRUE then ' delete export directory
        GetFileList ( WorkDirectory, "*.*", ExportFileList() )
        if KillFileList ( ExportFileList() ) <> TRUE then
            Warnlog "Couldn't delete all Files in Output-Directory, the followings are still there:"
            for i=1 to FehlerListe(0)
                printlog "  <> " & FehlerListe(i)
            next i
            goto endsub
        end if
    end if
    app.Mkdir WorkDirectory

    Printlog "- Read the directory with files we wish to import."
    GetFileList ( ImportDir, "*.*", ImportFileList() )
    printlog "-  Done."

    ' Create local copies of all files to be imported
    hFileListCopyLocal( ImportFileList() )

    printlog "- Start loading files in list"
	For i = 1 to listCount(ImportFileList())
		printlog "- " & i & ". load: " & ImportFileList(i)
		sExportFile = ImportFileList(i)
        sExceptionFile  = DateiExtract ( sExportFile )
        printlog sExceptionFile
		if GetExtention ( sExportFile ) = "psw" then
            QAErrorlog "#i102221#Pocket Word filter puts General I/O Error"
        elseif LCase( sExceptionFile ) = "swdos.txt" then
            QAErrorlog "#i114062#General I/O error loading StarWriter DOS file"
		else
			Call hFileOpenLocally( ImportFileList(i) )
			printlog "-  done"
	
			Kontext "TextImport"
			if TextImport.Exists then
				TextImport.Ok
				printlog "-  Text import dialog passed"
			end if
	
			Kontext "Filterauswahl"
			if Filterauswahl.Exists then
				Filterauswahl.Cancel
				printlog "-  Filterdialog passed"
			end if
	
			Kontext "AsciiFilterOptionen"
			if AsciiFilterOptionen.Exists then
				AsciiFilterOptionen.Ok
				printlog "-  ASCII-Filter dialog passed"
			end if
	
			Kontext "SecurityWarning"
			if SecurityWarning.Exists(3) then
				SecurityWarning.Ok
				printlog "-  Security warning passed"
			end if
	
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
				if inStr(ImportFileList(i),"sw40") = false then
					QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
				end if
				SecurityWarning.Cancel
			end if
			Sleep 1
	
			Kontext "AlienWarning"
			if AlienWarning.Exists then
				AlienWarning.Ok
				printlog "-  Alien warning passed"
			end if
	
			' HTML-File can't be exported as odf this way
			if GetExtention(ImportFileList(i)) <> "html" then		
			
				sExportFile = WorkDirectory & "ExportedFile" & i & ".odf"
				Printlog "-  Save as : " & ConvertPath ( sExportFile )
				if hFileSaveAsWithFilterKill (sExportFile, "writer8") = true then
					printlog "-  done"
					printlog "-  close file"
					Call hCloseDocument
		
					Printlog "-  Open previous saved file"
					Call hFileOpenWithFilter (sExportFile, "writer8")
					printlog "-  done"
					' Check for macro alert
					Kontext "SecurityWarning"
					if SecurityWarning.Exists then
						if inStr(ImportFileList(i),"sw40")  = false then
							QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
						end if
						SecurityWarning.Cancel
					end if
					Sleep 1
				else
					Warnlog "Unable to save file: " & sExportFile
				end if
			else
				printlog "-  HTML-file is not exported this way"
			end if
	
			printlog "-  Close all open files."
			Do Until GetDocumentCount = 0
				Call hCloseDocument
			Loop		
		endif
    next i 'File

endcase

'---------------------------------------------------------------------------------------------------------------------

testcase tExportAllReadableFormatsIntoThemselves

    if iSprache <> 1 then
        QAErrorlog "#100970#test only support english language"
        goto endsub
    end if

    Dim AvailableFilters( 35 ) as String
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

    For iCurrentFileIndex = 1 to 7
        select case iCurrentFileIndex
        case 1 :   sCurrentFile = "xml2.odt"
            sCurrentFilter = "writer8"
        case 2 :   sCurrentFile = "wpsfile.wps"
            sCurrentFilter = "Text (encoded)"
        case 3 :   sCurrentFile = "dostext.txt"
            sCurrentFilter = "Text (encoded)"
        case 4 :  sCurrentFile = "html.html"
            sCurrentFilter = "HTML (StarWriter)"
        case 5 :  sCurrentFile = "rtf.rtf"
            sCurrentFilter = "Rich Text Format"
        case 6 :  sCurrentFile = "sw60.sxw"
            sCurrentFilter = "StarOffice XML (Writer)"
        case 7 :  sCurrentFile = "winw97.doc"
            sCurrentFilter = "MS Word 97"
        case 8 :  sCurrentFile = "winword6.doc"
            sCurrentFilter = "MS Word 95"
        case 9 :  sCurrentFile = "wintext.txt"
            sCurrentFilter = "Text (encoded)"
        end select

        sLoadFile = sImportDir & sCurrentFile
        sSaveFile = sWorkDirectory & "filenr_" & iCurrentFileIndex & "." & GetExtention ( sCurrentFile )
        printlog "- Export of File nr " + iCurrentFileIndex + " started"
        printlog " - Save file: " & sLoadFile
        printlog " - to file: " & sSaveFile


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

        Cleanup:
        kontext "active"
        if active.exists(2) then
            active.ok
            Select Case iCurrentFileIndex
            case 7: QAErrorlog "#i106758#Saving StarWriter 3 template corrupts file"
            case else
                warnlog "Error with file " + sLoadFile + " as " + sSaveFile + "."
            end select
        end if

        printlog "   Close all open files."
        hFileCloseAll()

        printlog " - File nr " + iCurrentFileIndex + " completed."
    Next iCurrentFileIndex

endcase

'---------------------------------------------------------------------------------------------------------------------

testcase tExportTop5FormatsIntoTop5

    if iSprache <> 1 then
        QAErrorlog "#100970#test only support english language"
        goto endsub
    end if

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
    For iCurrentLoadFileIndex = 1 to 4
        Select case iCurrentLoadFileIndex
        case 1 : sCurrentLoadFile = sImportDir & "xml2.odt"
            sCurrentLoadFilter = "writer8"
        case 2 : sCurrentLoadFile = sImportDir & "sw60.sxw"
            sCurrentLoadFilter = "StarOffice XML (Writer)"
        case 3 : sCurrentLoadFile = sImportDir & "winw97.doc"
            sCurrentLoadFilter = "MS Word 97"
        case 4 : sCurrentLoadFile = sImportDir & "rtf.rtf"
            sCurrentLoadFilter = "Rich Text Format"
        case 5 : sCurrentLoadFile = sImportDir & "html.html"
            sCurrentLoadFilter = "HTML (StarWriter)"
        End select

        printlog "- Export of File nr " + iCurrentLoadFileIndex + " started."
        printlog " - Save file: " & sCurrentLoadFile

        '///  Here comes the SAVE-Loop
        For iCurrentSaveFileIndex = 1 to 5
            if hFileOpenWithFilter((sCurrentLoadFile), sCurrentLoadFilter) = true then
                Select case iCurrentSaveFileIndex
                case 1 : sCurrentSaveFile = sWorkDirectory & "SavedFile_l" + iCurrentLoadFileIndex + "_s1.odt"
                    sCurrentSaveFilter = "writer8"
                case 2 : sCurrentSaveFile = sWorkDirectory & "SavedFile_l" + iCurrentLoadFileIndex + "_s2.sxw"
                    sCurrentSaveFilter = "StarOffice XML (Writer)"
                case 3 : sCurrentSaveFile = sWorkDirectory & "SavedFile_l" + iCurrentLoadFileIndex + "_s3.doc"
                    sCurrentSaveFilter = "MS Word 97"
                case 4 : sCurrentSaveFile = sWorkDirectory & "SavedFile_l" + iCurrentLoadFileIndex + "_s4.rtf"
                    sCurrentSaveFilter = "Rich Text Format"
                case 5 : sCurrentSaveFile = sWorkDirectory & "SavedFile_l" + iCurrentLoadFileIndex + "_s5.html"
                    sCurrentSaveFilter = "HTML (StarWriter)"
                End select

                printlog " - " & iCurrentSaveFileIndex & ". to file: " & sCurrentSaveFile
                if hFileSaveAsWithFilterKill(sCurrentSaveFile, sCurrentSaveFilter) = true then
                    printlog " - close all open documents"
                    hFileCloseAll()
                    printlog " - check if file exists"
                    if FileExists (sCurrentSaveFile) then
                        printlog "  - success"
                        printlog " - reload previously saved file"
                        if hFileOpenWithFilter(sCurrentSaveFile, sCurrentSaveFilter) = true then
                            printlog "  - success"
                        else
                            Warnlog "error loading file: " & sCurrentSaveFile
                        end if
                        hFileCloseAll()
                    else
                        Warnlog "Saved file doesn't exist"
                        'Close the opened file
                        hFileCloseAll()
                    end if
                else
                    Warnlog "Error saving file: " & sCurrentLoadFile
                    'Close the opened file
                    hFileCloseAll()
                end if
            else
                Warnlog "Error loading file: " & sCurrentLoadFile
                'Close the opened file
                hFileCloseAll()
            end if

        Next iCurrentSaveFileIndex
        printlog "- Export of File nr " + iCurrentLoadFileIndex + " completed."
    Next iCurrentLoadFileIndex
    '// End of the LOAD-Loop.
endcase
