'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_import.inc,v $
'*
'* $Revision: 1.3 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:23:09 $
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
'* short description : Functions for import
'*
'************************************************************************
'*
' #1 wLoadFile(sFileName as string)
' #1 wLoadFile2(Inputpfad as string)
' #1 wImportWithFilter
' #1 wImportWithoutFilter
'*
'\***********************************************************************

function wImportWithoutFilter ( sCurrentFilter as string, sCurrentFilterExt as string )
    Dim DocFileList( 30 ) as String
    Dim Inputpfad as String
    Dim i as Integer
    Dim x as Integer

    printlog " load all documents from  input\\writer\\loadsave\\import\\ without filter selection"
    Inputpfad = ConvertPath ( gTesttoolPath + "writer\loadsave\input\import\" )
    GetFileList ( Inputpfad, "*.*", DocFileList() )

    x = ListCount ( DocFileList() )
	for i=1 to x
		printlog "(" & i & "/" & x & "): " & DocFileList(i)
		Call wLoadFile(DocFileList(i), sCurrentFilterExt , sCurrentFilter)
		Sleep 2
	next i

end function

' ******************************************************************************

function wImportWithFilter ( sCurrentFilter as string, sCurrentFilterExt as string )
    Dim Documentpath as string
    Dim DocFile as string
    Dim DocFilter as String
    Dim TempFilter as string
    Dim i as Integer
    Dim sTemplate as String
    Dim sTemplates as String
    Dim sText as String

    Documentpath = ConvertPath ( gTesttoolPath + "writer\loadsave\input\import\" )

    select case iSprache
        case 01   : sTemplate = "Template"  	: sText = "Text"               	: sTemplates = "Templates"
        case 03   : sTemplate = "Modelos"   	: sText = "Text"               	: sTemplates = "Modelos"
        case 07   : sTemplate = "Шаблон"        : sText = "Текст"              	: sTemplates = "Шаблоны"
        case 31   : sTemplate = "Sjabloon"    	: sText = "Tekst"			   	: sTemplates = "Sjablonen"
        case 33   : sTemplate = "Modèle"   		: sText = "Texte"			   	: sTemplates = "Modèles"
        case 34   : sTemplate = "Plantilla" 	: sText = "Texto"           	: sTemplates= "Plantillas"
        case 36   : sTemplate = "-szövegsablon" : sText = "Szöveg"             	: sTemplates = "-sablonok"
        case 39   : sTemplate = "Modello"   	: sText = "Testo"	           	: sTemplates = "Modelli"
        case 46   : sTemplate = "mallar"    	: sText = "Text"               	: sTemplates = "mallar"
        case 49   : sTemplate = "Vorlage"   	: sText = "Text"               	: sTemplates = "Vorlagen"
        case 55   : sTemplate = "Modelo do"   	: sText = "Texto"              	: sTemplates = "Modelo do"
        case 81   : sTemplate = "ﾃ�?ﾌﾟﾚｰﾄ"      	: sText = "ﾃｷｽﾄ"		           	: sTemplates = "ﾃ�?ﾌﾟﾚｰﾄ"
        case 82   : sTemplate = "서식 파일" 		: sText = "텍스트"				: sTemplates = "서식 파일"
        case 86   : sTemplate = "样式"   		: sText = "文字"				   	: sTemplates = "样式"
        case 88   : sTemplate = "模�?�"   		: sText = "文本"				   	: sTemplates = "樣�?"
        case else :
			Warnlog "  - please add words vor: 'Vorlage' and 'Text' in import1.inc::wImportWithFilter"
			sTemplate = "Vorlage"   : sText = "Text (*.txt)"
    end select

	for i=1 to 25
		select case i
			'documents 1-25 get loaded with internal filters
			case 1  : DocFile = "xml2.odt" : DocFilter = gWriterFilter ' & " (*.odt)"
				if Instr(gWriterFilter,"(.odt)") then
					DocFilter = Left$(gWriterFilter,Len(gWriterFilter) - 6) + "(*.odt)"
				end if
			case 2  : DocFile = "sw60.sxw" : TempFilter = hGetFilter("680")
				if Instr(TempFilter,"(.sxw)") then
					DocFilter = Left$(TempFilter,Len(TempFilter) - 6) + "(*.sxw)"
				end if
			case 3  : 	DocFile = "sw50.sdw"      : DocFilter = "StarWriter 3.0 - 5.0 (*.sdw)"
			case 4  : 	DocFile = "sw50.vor"      
                Select Case iSprache
                	Case 7
                		DocFilter = "Шаблоны StarWriter 3.0 - 5.0 (*.vor)"
                    Case 36
                    	DocFilter = "StarWriter 3.0 - 5.0" & sTemplates & " (*.vor)"
                    Case 55, 03:
                        DocFilter = "Modelos do StarWriter 3.0 - 5.0 (*.vor)"
                    Case 82
                    	DocFilter = "StarWriter 3.0 - 5.0 서식 파일 (*.vor)"
                    Case else
                        DocFilter = "StarWriter 3.0 - 5.0 " & sTemplates & " (*.vor)"
                end select
			case 5  : 	DocFile = "sw40.sdw"      : DocFilter = "StarWriter 3.0 - 5.0 (*.sdw)"
			case 6  : 	DocFile = "sw40_sp2.vor"
                Select Case iSprache
                	Case 7
                		DocFilter = "Шаблоны StarWriter 3.0 - 5.0 (*.vor)"
                    Case 36
                    	DocFilter = "StarWriter 3.0 - 5.0" & sTemplates & " (*.vor)"
                    Case 55, 03:
                        DocFilter = "Modelos do StarWriter 3.0 - 5.0 (*.vor)"
                    Case 82
                    	DocFilter = "StarWriter 3.0 - 5.0 서식 파일 (*.vor)"
                    Case else
                        DocFilter = "StarWriter 3.0 - 5.0 " & sTemplates & " (*.vor)"
                end select
			case 7  : 	DocFile = "sw30.sdw"
                        DocFilter = "StarWriter 3.0 - 5.0 (*.sdw)"
                        QAErrorlog "#i76220#Saving sdw-file as sxw fails"
			case 8  : 	DocFile = "sw31.sdw"
                        DocFilter = "StarWriter 3.0 - 5.0 (*.sdw)"
                        QAErrorlog "#i76220#Saving sdw-file as sxw fails"
			case 9  : 	DocFile = "sw31.vor"
                        QAErrorlog "#i76220#Saving sdw-file as sxw fails"
                Select Case iSprache
                	Case 7
                		DocFilter = "Шаблоны StarWriter 3.0 - 5.0 (*.vor)"
                    Case 36
                    	DocFilter = "StarWriter 3.0 - 5.0" & sTemplates & " (*.vor)"
                    Case 55, 03:
                        DocFilter = "Modelos do StarWriter 3.0 - 5.0 (*.vor)"
                    Case 82
                    	DocFilter = "StarWriter 3.0 - 5.0 서식 파일 (*.vor)"
                    Case else
                        DocFilter = "StarWriter 3.0 - 5.0 " & sTemplates & " (*.vor)"
                end select
			case 10  : 	DocFile = "sw2.sdw"       : DocFilter = "StarWriter 2.0 (*.sdw)"
			case 11 : 	DocFile = "sw1.sdw"       : DocFilter = "StarWriter 1.0 (*.sdw)"
			case 12 : 	DocFile = "swdos.txt"     : DocFilter = "StarWriter DOS (*.txt)"
			case 13 : 	DocFile = "rft.rtf"       : DocFilter = "Rich Text Format (*.rtf)"
			case 14 : 	DocFile = "winword6.doc"
                        Select Case iSprache 
                            case 46
                                DocFilter = "Microsoft Word 6.0/95 (*.doc)"
                            case else
                                DocFilter = "Microsoft Word 6.0 / 95 (*.doc)"
                        end select
			case 15 : 	DocFile = "winword6.doc"
                        Select Case iSprache 
                            case 46
                                DocFilter = "Microsoft Word 6.0/95 (*.doc)"
                            case else
                                DocFilter = "Microsoft Word 6.0 / 95 (*.doc)"
                        end select
			case 16 : 	DocFile = "winw97.doc"    : DocFilter = "Microsoft Word 97/2000/XP (*.doc)"
			case 17 : 	DocFile = "wintext.txt"				
				if gPlatGroup <> "w95" then
					DocFilter = sText + " (*.txt)" ' + " WIN"
				else
					DocFilter = sText + " (*.txt)"
				end if
			
			case 18 : DocFile = "unixtext.txt"
				if gPlatgroup <> "unx" then
					DocFilter = sText + " (*.txt)" ' + " UNIX"
				else
					DocFilter = sText + " (*.txt)"
				end if
			
			case 19 : DocFile = "mactext.txt"
				if gPlatgroup <> "mac" then
					DocFilter = sText + " (*.txt)" ' + " MAC"
				else
					DocFilter = sText  + " MAC"
				end if
			
			case 20 : DocFile = "dostext.txt"
				if gPlatgroup <> "os2" then
					DocFilter = sText + " (*.txt)" '+ " DOS"   
				else
					DocFilter = sText  + " Ansi"
				end if

			printlog "   ' die Dokumente 20-32 will be loaded with default installed W4W-Filtern"
			case 21 : DocFile = "dostext.txt"		        : DocFilter = sText + " (*.txt)"
			case 22 : DocFile = "wpsfile.wps"		        : DocFilter = "WPS 2000/Office 1.0 (*.wps)"
			case 23 : DocFile = "hangul.hwp"     	        : DocFilter = "Hangul WP 97 (*.hwp)"
			case 24 : DocFile = "ichitaro.jtd"  	        : DocFilter = "Ichitaro 8/9/10/11 (*.jtd)"
			case 25 : DocFile = "ichitaro_template.jtt"  	
                Select Case iSprache
                	Case 7:         DocFilter = "Шаблон Ichitaro 8/9/10/11 (*.jtt)"
                    Case 34:        DocFilter = "Plantilla Ichitaro 8/9/10/11 (*.jtt)"
                    Case 36:        DocFilter = "Ichitaro 8/9/10/11-sablon (*.jtt)" 
                    Case 55, 03:    DocFilter = "Modelo Ichitaro 8/9/10/11 (*.jtt)"
                    Case else       DocFilter = "Ichitaro 8/9/10/11 " & sTemplate & " (*.jtt)"
                end select
			' case 21 : DocFile = "winword2.doc"  			: DocFilter = "Microsoft WinWord 2.x (*.doc)"
			' case 26 : DocFile = "amipro3.sam"    			: DocFilter = "Ami Pro 1.x-3.1 (*.sam)"
			
			' case 23 : DocFile = "filter\wordperf.wpd"  	: DocFilter = "WordPerfect (Win) 6.0 - 7.0"
			' case 24 : DocFile = "filter\excel4.xls"    		: DocFilter = "Microsoft Excel 4.x - 5.0 / 95"
			' case 25 : DocFile = "filter\excel5.xls"    		: DocFilter = "Microsoft Excel 4.x - 5.0 / 95"
			' case 26 : DocFile = "filter\excel5.xls"    		: DocFilter = "Microsoft Excel 4.x - 5.0 / 95"
			' case 27 : DocFile = "filter\gvr.wk3"       		: DocFilter = "Lotus 1-2-3" : printlog "v DOESN'T WORK IN STANDARD INSTALL ANNYMORE"
			' case 28 : DocFile = "wordstar.wsd"  				: DocFilter = "WordStar (Win) 1.x-2.0": printlog "v DOESN'T WORK IN STANDARD INSTALL ANNYMORE"
			' case 29 : DocFile = "filter\winword1.doc"  	: DocFilter = "Microsoft WinWord 1.x": printlog "v DOESN'T WORK IN STANDARD INSTALL ANNYMORE"
			' case 30 : DocFile = "filter\wordperf.wpd"  	: DocFilter = "WordPerfect (Win) 6.1" : printlog "v DOESN'T WORK IN STANDARD INSTALL ANNYMORE"
			' case 31 : DocFile = "filter\framemak.mif"  	: DocFilter = "Frame Maker MIF 5.0"  : printlog "v DOESN'T WORK IN STANDARD INSTALL ANNYMORE"' who changed it to wpd????!! TBO 02/07/2001
			' if a filter gets inserted, you have to change 'Ende' above !!
		end select		
		printlog "(" & i & "/" & 25 & "): "& Documentpath & DocFile & " -  " & DocFilter		
		Select Case DocFile 
            Case "wpsfile.wps"
			    if gPlatform = "sol" then
				    QAErrorlog "#125085#Loading wpsfile.wps crashes on Solaris SPARC [Stacktrace ID: 223014]"
                else	
				    Call wLoadFile2(Documentpath, DocFile, DocFilter, sCurrentFilterExt, sCurrentFilter, i)
                end if
            Case "hangul.hwp"
                QAErrorlog "#136537#Ichitaro, WPS2000 and HWP documents connot be opened without filter selection"
            Case else
			    Call wLoadFile2(Documentpath, DocFile, DocFilter, sCurrentFilterExt, sCurrentFilter, i)
		end select
	next i

end function

' ******************************************************************************

testcase wLoadFile(sFileName as string, sCurrentFilterExt as string, sCurrentFilter as string)
    Dim Outputdatei as string
    Dim i as integer
    Dim CheckBugDocExtension as string
    Dim Fehler as Boolean
    Dim CountDocu as integer
    Dim FileToLoad as string  
    Dim sCurrentFilterToSave as string

    printlog "- sFileName: " & sFileName
    printlog "- sCurrentFilter: " & sCurrentFilter
    printlog "- sCurrentFilterExt: " & sCurrentFilterExt

    FileToLoad = DateiExtract(sFileName)
    CheckBugDocExtension = GetExtention(FileToLoad)

	if CheckBugDocExtension = "wps" then 
		if gPlatform = "sol" then
			QAErrorlog "#125085#Loading wpsfile.wps crashes on Solaris SPARC [Stacktrace ID: 223014]"
			goto endsub
		end if
	end if

    Select Case CheckBugDocExtension
        case "jtt", "jtd", "hwp", "wps"
            QAErrorlog "#136537#Ichitaro, WPS2000 and HWP documents connot be opened without filter selection"
            exit sub
    end select
	
    Fehler = FALSE
    printlog " Open a new writer document"
    Call hNewDocument
    printlog " open file without filter selected"
    sleep 2
    Call hFileOpen ( sFileName )
    Select Case CheckBugDocExtension
        Case "sam"
            Sleep 20
        Case else
            Sleep 5        
    end select

    Kontext "Filterauswahl"
    if Filterauswahl.Exists then
        Select Case CheckBugDocExtension
            case "psw"
                QAErrorlog "i45865#XSLT-Filter are locked on Windows"
            case else
                Warnlog "  - Filter Selection dialog is up !"
        end select        
        Filterauswahl.Cancel
        if CheckbugDocExtension = "psw" then
            Kontext "Active"
            if Active.Exists then Active.Ok
        else
            Call hCloseDocument
        end if
    end if
    Sleep 5
    Kontext "AsciiFilterOptionen"
    if AsciiFilterOptionen.Exists then AsciiFilterOptionen.Ok    

    sleep (5)  ' cause winword6.doc is loaded tooo slow

    ' Check for macro alert
    Kontext "SecurityWarning"
    if SecurityWarning.Exists then
        if FileToLoad <> "sw40.sdw" then
            QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
        end if
        SecurityWarning.Cancel
    end if
    Sleep 1

    Kontext "TextImport"
    if TextImport.Exists then
        Warnlog "#i84264#Loading txt-file in writer brings CSV-Text Import dialog"
        TextImport.Cancel
        Call hCloseDocument
        exit sub
    end if
    
    kontext
    do while (active.exists)
        Call wKillUpcomingActives
        kontext
        sleep (1)
    loop

    printlog " check if loaded"
    if GetDocumentCount >= 1 then

        Outputdatei = DateiOhneExt(DateiExtract(sFileName))

        Kontext "AsciiFilterOptionen"
        if AsciiFilterOptionen.Exists then
            Warnlog "  - ASCII Filter Options at " + sFileName
            AsciiFilterOptionen.Ok
        end if

        ' There was a bug (b5013292) documents where empty after loading (Solaris only)
        SetClipBoard ""
        Kontext "DocumentWriter"
        Select Case Outputdatei
            Case  "sw31", "ichitaro_template.jtt"
                DocumentWriter.TypeKeys "<Shift F4>"
                DocumentWriter.TypeKeys "<Tab>"
                DocumentWriter.TypeKeys "<Return>"
                wait 500
        end select
        DocumentWriter.TypeKeys "<Mod1 Home>"
        Sleep 2
        DocumentWriter.TypeKeys "<Shift END>"
        wait 500
        DocumentWriter.TypeKeys "<Shift Down>", 5
        wait 500
        printlog " check if document isn't empty"
        try
            EditCopy
            if GetClipBoardText = "" then
                Warnlog "  - Document '" & FileToLoad & "' is empty after loading !"
            else
                printlog "   - Check for content => OK"
            end if
        catch
            Select Case FileToLoad
                case "rft.rtf"
                    QAErrorlog "#116176#Import rtf results in an empty document"
                case else
                    QAErrorlog "  - Document '" & FileToLoad & "' seems to be empty!"
            end select
        endcatch
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<Mod1 Home>"
        
        ' Point cursor to first page when write protected
        for i = 1 to 10
            Call wTypeKeys " "
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    Active.Ok
                    Call wTypeKeys "<DOWN>",5
                end if
            else
                i = 11
            end if
            wait 500
        next i

        Sleep 5
        printlog " modify document content"
        Call wTypeKeys "Writer Import test<Return>"
        Sleep 2

        Call wTypeKeys "<Return>", 5
        Sleep 1
        Call wSearchWriteableArea ()
        Call wTypeKeys "Writer Import test<Return>"
        wait 500
        printlog "   - Open and modify => OK "
        printlog " save file"
        if sCurrentFilter = "StarWriter 3.0 - 5.0" then
            sCurrentFilterToSave = "StarWriter 5.0 (.sdw)"
        else
            if Instr(gWriterFilter, "(" & sCurrentFilterExt & ")") <= 0 then
                sCurrentFilterToSave = sCurrentFilter & " (" & sCurrentFilterExt & ")"
            else
                sCurrentFilterToSave = sCurrentFilter
            end if
        end if
        printlog "   - Save " & sFileName & " as " & Outputdatei & sCurrentFilterExt
        if hFileSaveAsWithFilterKill ( gOfficePath + "user\work\" + Outputdatei + sCurrentFilterExt, sCurrentFilterToSave ) = true then
            Sleep 3
            printlog "   - save document => OK"
    
            FileClose
            Sleep 3
            Call wKillUpcomingActives()
            Sleep 3
            Kontext
            if active.exists then
                stemp= Active.GetText
                active.yes
                Sleep 3
            end if

            Kontext "AlienWarning"
            if AlienWarning.Exists then AlienWarning.Ok
            
            printlog "   - close document  => OK"
            printlog " open file again"
            Call hFileOpen ( gOfficePath + "user\work\" + Outputdatei + sCurrentFilterExt )
            Sleep 3

            ' Check for macro alert
            Kontext "SecurityWarning"
            if SecurityWarning.Exists then
                if FileToLoad <> "sw40.sdw" then
                    QAErrorlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
                end if
                SecurityWarning.Cancel
            end if
            Sleep 1

            if GetDocumentCount >= 1 then
                printlog "   - reopen document => OK"
                CountDocu = 0
                Do Until GetDocumentCount = 1
                    Call hCloseDocument
                    CountDocu = CountDocu + 1
                Loop
                if CountDocu > 1 then
                    QAErrorlog "#i42080#More than 1 document had to be closed!"
                end if
                Sleep 3
                printlog "   - close document again => OK"
            else
                Warnlog "  - Doc probably didn't get loaded !!!"
            end if
        else
            Warnlog "  - document couldn't be saved"
            Call wKillUpcomingActives()
            Do Until GetDocumentCount = 1
                Call hCloseDocument
            Loop
        end if
    else
        QAErrorlog "  - Doc probably didn't get loaded !!!"
    end if
    Sleep 2
    Call wKillUpcomingActives()
    Sleep 2

endcase

' ******************************************************************************

testcase wLoadFile2(Inputpfad as string, FileToLoad as string, Filter as string, sCurrentFilterExt as string, sCurrentFilter as string, DocCount as integer)
    Dim Fehler as boolean, CountDocu as integer, sCurrentFilterToSave as string
    printlog "- Inputpfad: " & Inputpfad
    printlog "- FileToLoad: " & FileToLoad
	if FileToLoad =  "ichitaro_template.jtt" then
		Select Case iSprache
			case 7
				Filter = "Шаблон Ichitaro 8/9/10/11 (*.jtt)"
            case 39
                Filter = "Modello di documento Ichitaro 8/9/10/11 (*.jtt)"
            case 46
                Filter = "Ichitaro 8/9/10/11 mall (*.jtt)"
            case 82
			    Filter = "이치타로 8/9/10/11 서식 파일 (*.jtt)"            
		end select        
	end if
    if Left$(FileToLoad, 4) =  "sw31" then
        QAErrorlog "#i76220#Saving sdw-file as sxw fails"
        goto endsub
    end if
    printlog "- Filter: " & Filter
    printlog "- sCurrentFilterExt: " & sCurrentFilterExt
    printlog "- sCurrentFilter: " & sCurrentFilter
    
    Fehler = FALSE
    printlog " open file with filter"
    Call hNewDocument
    Sleep 5
    Kontext
    wait 500
    FileOpen
    Kontext "OeffnenDlg"
    Dateiname.SetText ( ConvertPath ( Inputpfad + FileToLoad ) )
    try
        Dateityp.Select Filter
    catch
        warnlog "  - filter not available: " + Filter
		OeffnenDlg.Cancel
		wait 500
		Call hCloseDocument
		goto endsub
    endcatch
    Sleep 1
    Oeffnen.Click
    if FileToLoad = "amipro3.sam" then
        Sleep 20
    else
        Sleep 1
    end if
      
    Kontext "Active"
    if Active.Exists then
        QAErrorlog "Active: " + active.gettext
        try
            Active.Yes
        catch
            Active.Ok
        endcatch
    end if

    Kontext "SecurityWarning"
    if SecurityWarning.Exists then
        if FileToLoad <> "sw40.sdw" then
            Warnlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
        end if
        SecurityWarning.Cancel
    end if
    Sleep 1 
    
    IsItLoaded
    if GetDocumentCount < 1 then
        Warnlog "Document has not been loaded correctly!"
        goto endsub
    end if
    
    Kontext "Filterauswahl"
    printlog " catch filterselection"
    if Filterauswahl.Exists(3) then
        Sleep 1
        Filterauswahl.Cancel
        Call hCloseDocument
    end if

    Kontext "AsciiFilterOptionen"
    if AsciiFilterOptionen.Exists then AsciiFilterOptionen.Ok

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then ' MESSBOX
            Warnlog "  - active304: " + Active.Gettext
            Active.Ok
        end if
    end if

    Kontext "TextImport"
    if TextImport.Exists then
        Warnlog "#i84264#Loading txt-file in writer brings CSV-Text Import dialog"
        TextImport.Cancel
        Call hCloseDocument
        exit sub
    end if   
    
    kontext
    do while (active.exists)
        call wKillUpcomingActives
        kontext
        sleep (1)
    loop

    if GetDocumentCount >= 1 then
        printlog " - Open  => OK"

        printlog " write something in document"
        Kontext "AsciiFilterOptionen"
        if AsciiFilterOptionen.Exists then AsciiFilterOptionen.Ok

        Sleep 5
        ' There was a bug (b5013292) documents where empty after loading (Solaris only)
        SetClipBoard = ""
        Kontext "DocumentWriter"
        Select Case FileToLoad
            Case  "sw31.vor"
                DocumentWriter.TypeKeys "<Shift F4>"
                DocumentWriter.TypeKeys "<Tab>"
                DocumentWriter.TypeKeys "<Return>"
            Case "ichitaro_template.jtt"
                DocumentWriter.TypeKeys "<Shift F4>"
                wait 500
            Case else 
                DocumentWriter.TypeKeys "<Mod1 Home>"
                Sleep 2
                DocumentWriter.TypeKeys "<Shift END>"
                wait 500
                DocumentWriter.TypeKeys "<Shift Down>", 5
                wait 500
        end select
        try
            EditCopy
            printlog "  - Check for content => OK"
        catch
            Select Case FileToLoad
                case "rft.rtf"
                    QAErrorlog "#116176#Import rtf results in an empty document"
                case else
                    QAErrorlog "  - Document seems to be empty!"
            end select
        endcatch			
        
        Call wTypeKeys "<Escape>"
        Call wTypeKeys "<Mod1 End>"
        Call wTypeKeys "<Return>", 1
        Sleep 1
        printlog " check if writeable area!"
        Call wSearchWriteableArea ()
        Call wTypeKeys "Writer Import test"
        printlog "  - Able to modify Document  => OK"
        sleep (3)

        printlog " save with actual (SW5.0, SW6.0) filter & close window"
        printlog " use filename   user\\work\\import1.sdw"
        if sCurrentFilter = "StarWriter 3.0 - 5.0" then
            sCurrentFilterToSave = "StarWriter 5.0 (.sdw)"
        else
            if Instr(gWriterFilter, "(" & sCurrentFilterExt & ")") <= 0 then
                sCurrentFilterToSave = sCurrentFilter & " (" & sCurrentFilterExt & ")"
            else
                sCurrentFilterToSave = sCurrentFilter
            end if
        end if
        Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\import" & DocCount & sCurrentFilterExt, sCurrentFilterToSave )
        Kontext "AlienWarning"
        if AlienWarning.Exists then AlienWarning.Ok
        printlog "  - Save with " + sCurrentFilterToSave +" => OK"
        
        ' hCloseDocument Is NOT the right choice here, because:
        FileClose      ' active about caused information loss... YES
        Sleep 3
        Call wKillUpcomingActives(FileToLoad)
        Sleep 3
        printlog "  - Close document  => OK"

        printlog " open again automagical filter select & close"
        Call hFileOpen ( gOfficePath + "user\work\import" & DocCount & sCurrentFilterExt)
        ' Check for macro alert
        Kontext "SecurityWarning"
        if SecurityWarning.Exists then
            if FileToLoad <> "sw40.sdw" then
                Warnlog "Macro-Security Warning is up though document should have no macro! Macro disabled"
            end if
            SecurityWarning.Cancel
        end if
        Sleep 1
        
        Kontext
        
        printlog "  - Reload document  => OK"
        CountDocu = 0
        Do Until GetDocumentCount = 1
            Call hCloseDocument
            CountDocu = CountDocu + 1
        Loop
        if CountDocu > 1 then
            QAErrorlog "#i42080#More than 1 document had to be closed!"
        end if
        printlog " close document"
        printlog "  - Close document  => OK"
    else ' is named doc loaded
        Warnlog "  - Doc probably didn't get loaded !!!"
        if GetDocumentCount >= 1 then
            for i = 1 to GetDocumentCount - 1
                Call hCloseDocument
            next i
        end if
    end if
    Sleep 2
endcase
