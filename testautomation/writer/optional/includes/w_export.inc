'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_export.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-18 14:50:47 $
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
'* short description : Checks the export to all available Filters from sdw-files
'*
'***********************************************************************
'
' #1 tExportFile
' #1 tExportListCompare
' #1 tExportFileToFile
'
'\***********************************************************************

testcase tExportListCompare
    Dim i as Integer
    Dim j as Integer
    Dim CurrentFilterFile as String
    Dim TemplateFilterFile as String
    Dim ExpectedFilters( 30 ) as String
	Dim AvailableFilters( 30 ) as String
	Dim UnknownFilters( 30 ) as String

    printlog " check export filters from 'saveAs dialog'"
    printlog " - compare available filters with file in \\writer\\input\\export\\filter\\exlst_{Language}.w95"
    printlog " - save available filters in writer\\input\\export\\filter\\act_{Language}.all"
    printlog " +   which will be used in the next testcase for saving."

    CurrentFilterFile = ConvertPath ( gOfficePath + "user\work\act_" & iSprache & ".all" )
    TemplateFilterFile = ConvertPath ( gTesttoolPath + "writer\loadsave\input\export\filter\exlst_" & iSprache & ".w95")

    ' read the 'Expected' List from file
    if Dir ( TemplateFilterFile ) <> "" then
        ListRead ( ExpectedFilters(), TemplateFilterFile, "utf8" )
        Printlog "- read expected filters from: " & TemplateFilterFile
    end if

    ' get Available list from 'save as' dialog & write ist to a file
    Call hNewDocument
    FileSaveAs
    Kontext "SpeichernDlg"
    for i=1 to DateiTyp.GetItemCount
        ListAppend ( AvailableFilters (), DateiTyp.GetItemText (i) )
    next i
    SpeichernDlg.Cancel

    ListWrite ( AvailableFilters(), CurrentFilterFile, "utf8" )
    printlog "- available filters written to: " + CurrentFilterFile

    ' compare both lists
    printlog "Comparing Lists - found:"
    if Val(ExpectedFilters(0)) > 0 then
        for i=1 to Val(AvailableFilters(0))
            for j=1 to Val(ExpectedFilters(0))
                if AvailableFilters(i) = ExpectedFilters(j) then
                    ListDelete ( ExpectedFilters(), j )
                    printlog " - " & AvailableFilters(i)
                    j=1000
                end if
            next j
            if j<>1001 then ListAppend ( UnknownFilters(), AvailableFilters(i) )
        next i
        if Val(UnknownFilters(0)) > 0 then
            QAErrorlog "The following Export-Filters are new in Filterlist:"
            for i=1 to Val(UnknownFilters(0))
                Printlog "" & UnknownFilters(i)
            next i
        end if
        if Val(ExpectedFilters(0)) > 0 then
            for i=15 to Val(ExpectedFilters(0))
                Select Case ExpectedFilters(i)
                    Case "DocBook (.xml)"
                        if gOOO = true then
                            Warnlog "The following Export-Filter is not installed: " & ExpectedFilters(i)
                        end if
                end select
            next i
        end if
    else
        ListWrite ( AvailableFilters(), TemplateFilterFile, "utf8" )
        printlog "The Have-To-List has no entry, so the list will be created!"
        printlog " ...written to"+TemplateFilterFile
    end if
    Call hCloseDocument
endcase

' **********************************************************************************************

sub tExportFile(sFileExtension as string)
    Dim pos as integer
    Dim FilterName as String
    Dim i as Integer
    Dim j as Integer
    Dim FileLst ( 10 ) as String
    Dim FilterLst( 30 ) as String
    Dim Exlist ( 100 ) as String

    FileLst(0) = 0

    if gSamePC = TRUE then ' delete export directory
        GetFileList ( ConvertPath ( gOfficePath + "user\work\" + gPlatgroup + "\export\"), "*.*", Exlist() )
        if KillFileList ( Exlist() ) <> TRUE then
            Warnlog "Couldn't delete all Files in Output-Export-Directory, the followings are still there:"
            for i=1 to Exlist(0)
                printlog "  <> " & Exlist(i)
            next i
        end if
    end if

	GetFileList ( ConvertPath ( gTesttoolPath + "writer\loadsave\input\export\" & sFileExtension & "\"), "*." & sFileExtension, FileLst() )
    for i = 1 to Val(FileLst(0))
        ' get to test filter list
        ListRead ( FilterLst(), ConvertPath ( gOfficePath + "user\work\act_" & iSprache & ".all" ), "utf8" )
        printlog "("  & i & "/" & Val(FileLst(0)) & "): " & FileLst(i)
        for j = 1 to Val(FilterLst(0))
            printlog "    " & i & ":" & "(" & j & "/"& Val(FilterLst(0)) & "): " & FilterLst(j)
			Call tExportFileToFile(FileLst(i), FilterLst(j), j)
        next j
    next i

end sub

' **********************************************************************************************

testcase tExportFileToFile(SourceFile as string, toFiltername as string, sCount as integer)
	Dim toExtension as string
    Dim ExportFileName as string
	Dim pos as integer
    Dim FilterExtension as string
    Dim CountDocu as integer
    Dim sVorlage as String
    Dim sText as String
    Dim sAddToFilter as string
    select case iSprache
        case 01 : sVorlage = "Template" 			:sText = "Text"  					:sAddToFilter = "Templates"
        case 07 : sVorlage = "Шаблон"          		:sText = "Текст"                    :sAddToFilter = "Шаблоны"
        case 03 : sVorlage = "Modelos"   			:sText ="Text"  					:sAddToFilter = "Modelli"
        case 31 : sVorlage = "Sjabloon"  			:sText = "Text"                     :sAddToFilter = "Sjablonen"
        case 33 : sVorlage = "Modèle"   			:sText = "Texte"  					:sAddToFilter = "Modèles"
        case 34 : sVorlage = "Plantilla"  			:sText = "Texto"  					:sAddToFilter = "Plantillas"
        case 36 : sVorlage = "-szövegsablon"   		:sText = "Szöveg"                   :sAddToFilter = "-sablonok"
        case 39 : sVorlage = "Modello"  			:sText = "Documenti di testo"		:sAddToFilter = "Modelli"
        case 46 : sVorlage = "Mallar"  				:sText = "Text"  					:sAddToFilter = "mallar"
        case 49 : sVorlage = "Vorlage"   			:sText = "Text"  					:sAddToFilter = "Vorlagen"
        case 55 : sVorlage = "Planilha"  			:sText = "Texto"  					:sAddToFilter = "Modelo do"
		case 81 : sVorlage = "テンプレート"			:sText = "テキスト"					:sAddToFilter = "テンプレート"
        case 82 : sVorlage = "서식 파일"				:sText = "텍스트"					:sAddToFilter = "서식 파일"
        case 86 : sVorlage = "模板"					:sText = "文本"						:sAddToFilter = "样式"
        case 88 : sVorlage = "範本"					:sText = "文字"						:sAddToFilter = "樣式"
        case else : Warnlog "Please add strings for 'Template' and 'Text'. Test exits!"
                    goto endsub
    end select    

    printlog "- SourceFile: " & SourceFile
    printlog "- toFilterName: " & toFiltername    
    pos = Instr(toFilterName, "(.")
    if pos > 0 then
        toExtension = Mid$(toFilterName, pos + 1, Len(toFiltername))
        toExtension = Left$(toExtension, len(toExtension) -1 )
        printlog "- saving file with extension: " & toExtension
        Select Case toExtension
            case ".rtf"
            case ".psw", ".pdb"
            case ".sdw", ".vor"
        end select
    else
        Warnlog "Unable to extract extension from filtername! Please adjust test !"
        goto endsub
    end if


	printlog " open file with automagical filter selection"
	printlog " - export"
	Call hFileOpen ( SourceFile )
	Sleep 5

    Kontext "SecurityWarning"
    if SecurityWarning.Exists then SecurityWarning.Ok

    Kontext "DocumentWriter"
    Do until DocumentWriter.StatusIsProgress = false
        wait 500
    Loop

	ExportFileName = ConvertPath ( gOfficePath + "user\work\" + DateiOhneExt(DateiExtract ( SourceFile  ) ) + "_" + sCount + toExtension )
	printlog " - target: " & ExportFileName
	printlog " save with to-test-filter"

	Call hFileSaveAsWithFilterKill ( ExportFileName, toFilterName ) ' automatic file extension is disbled here!

    printlog " Check for Alien Warning dialog"
    Kontext "AlienWarning"
    if AlienWarning.Exists then AlienWarning.Ok
   
	printlog " if 'text-encoded' filter is used, the 'ASCII Filter Options' come up, catch them!"
	Kontext "AsciiFilterOptionen"
	if AsciiFilterOptionen.Exists then
		printlog "       - ASCII Filter Options at filter:" + toFilterName
		AsciiFilterOptionen.Ok
	end if

    Kontext "DocumentWriter"
    Do until DocumentWriter.StatusIsProgress = false
        wait 500
    Loop        
    
	Sleep 2
	Kontext "Active"
	if Active.Exists then
		if Active.GetRT = 304 then
			printlog "- " & Active.GetText
            QAErrorlog "#i45865#XSLT-Filters are locked on windows"
			try
                Active.No
            catch
                Active.Ok
            endcatch
		end if
	end if

	FileClose
    wait 500
	Kontext "Active"
	if Active.Exists then
		if Active.GetRT = 304 then
			printlog "       - " & Active.GetText
			Active.Yes
		end if
	end if

	Sleep 2
    
    if hFileExists ( ExportFileName ) = true then    
        if toExtension = ".xml" then
            if gPlatForm = "sol" then
                QAErrorlog "#i70470#Conversion error loading XML-file"
                goto endsub
            end if
        end if
        printlog " - open without Filterselection"
        printlog " open saved doc with automagical filter selection"
        Select Case toExtension
            case ".pdb"
                QAErrorlog "#i44847#Writer can't open AportisDoc (.pdb) files"
                goto endsub
        end select

        Call hNewDocument
        Sleep 3
        Call hFileOpen ( ExportFileName )
       
        Kontext "SecurityWarning"
        if SecurityWarning.Exists then SecurityWarning.Ok
        
        Kontext "AsciiFilterOptionen"
        if AsciiFilterOptionen.Exists then
            if toExtension <> ".txt" then
                Select Case toExtension
                    case ".psw"
                        QAErrorlog "#i45865#XSLT-Filter are locked on Windows"
                    case else
                        Warnlog "ASCII Filter Options at " + ExportFileName
                end select
            end if
            AsciiFilterOptionen.Ok
        end if

        Kontext "DocumentWriter"
        Do until DocumentWriter.StatusIsProgress = false
            wait 500
        Loop        

        Sleep 2
        Kontext
        FileClose
    
        Kontext "Active"
        if Active.Exists then
            ' ... the normal save-messagebox
            if Active.GetRT = 304 then
                printlog "       - " & Active.GetText
                Active.No
            end if
        end if
    
        Sleep 2
        if toFilterName > "" then
            if GetDocumentCount = 1 then Call hNewDocument
            printlog " - open with Filterselection"
            ' Filtername in FileOpen dialog is not like in Save as dialog (*) is missing here
            ' so a * is added to the Filtername
            pos = Instr(toFilterName,"(.html)"
            if pos <> 0 then
                pos = Instr(toFilterName,"(.html;.htm)"
                if pos <> 0 then
                    toFilterName = Left$(toFilterName,Len(toFilterName) - 12) & "(*.html;*.htm)"
                else
                    toFilterName = Left$(toFilterName,Len(toFilterName) - 7) & "(*.html;*.htm)"
                end if
            else
                FilterExtension = "(*." + right$(toFilterName,4)
                toFilterName = Left$(toFilterName,Len(toFilterName) - 6) & FilterExtension
            end if
            printlog " open saved file with saved filter"
            printlog " In Importlist of File-Open-Dialog the StarWriter-Filter have been summarized"
            printlog " StarWriter 3.0 - 5.0"
            if Instr(toFilterName, "StarWriter") <= 0 then
                Select Case Left$(toFilterName,16)
                    Case "Microsoft Word 9"  '"Microsoft Word 95"
                        Select Case iSprache
                            Case 46                            
                                toFilterName = "Microsoft Word 6.0/95 (*.doc)"
                            Case else
                                toFilterName = "Microsoft Word 6.0 / 95 (*.doc)"
                        end select
                    Case "Microsoft Word 6"   '"Microsoft Word 6.0"
                        Select Case iSprache                    
                            Case 46                            
                                toFilterName = "Microsoft Word 6.0/95 (*.doc)"
                            Case else
                                toFilterName = "Microsoft Word 6.0 / 95 (*.doc)"
                        end select
                end select                
                Select Case toExtension
                   	Case ".txt"
                   		Select Case iSprache
                   			Case 82
                   				toFilterName = "텍스트 (*.txt)"
                   			Case 86
                   				toFilterName = "文字 (*.txt)"
                   			Case 88
                   				toFilterName = "編碼文字 (*.txt)"
						end select
				end select
            else
                Select Case toExtension
                	Case ".vor"
		            	Select Case iSprache
        		        	Case 07
                		    	toFilterName = "Шаблоны StarWriter 3.0 - 5.0 (*.vor)"
                        	Case 55, 03
                            	toFilterName = "Modelos do StarWriter 3.0 - 5.0 (*.vor)"
		                    Case else
        		                 toFilterName = "StarWriter 3.0 - 5.0 " & sAddToFilter & " (*.vor)" 
                		    end select
                   	Case else
                        Select Case iSprache
                            Case 36
                                toFilterName = "StarWriter 3.0 - 5.0" & FilterExtension
                            Case else
                                toFilterName = "StarWriter 3.0 - 5.0 " & FilterExtension
	                    end select
   	            end select    
            end if
            printlog " - " & toFilterName
            Call hFileOpenMitFilter ( ExportFileName, toFilterName, false )

            Kontext "SecurityWarning"
            if SecurityWarning.Exists then SecurityWarning.Ok

            Kontext "DocumentWriter"
            Do until DocumentWriter.StatusIsProgress = false
                wait 500
            Loop        

            CountDocu = 0
            Do Until GetDocumentCount = 1
                Call hCloseDocument
                CountDocu = CountDocu + 1
            Loop
            if CountDocu > 1 and toExtension <> ".html" then
                Select Case toExtension
                    case ".vor"
                        QAErrorlog "#i49778#StarWriter 3.0-5.0 Templates are loaded in a new task"
                    case else                
                        QAErrorlog "#i42080#More than 1 document had to be closed!"
                end select
            end if
            Sleep 2
        end if
    else
        QAErrorlog "File " &  ExportFileName & " not found"
        Kontext "AlienWarning"
        if AlienWarning.Exists then AlienWarning.Ok
    end if
endcase
