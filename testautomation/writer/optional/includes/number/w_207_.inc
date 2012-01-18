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
'* short description : Testing Number and Bullets
'*
'\***********************************************************************

sub w_207_

    Call tToolsNumbering12
    Call tToolsNumbering13

end sub

'------------------------------------------------------------------------------------------------------------------------

testcase tToolsNumbering12

    printlog "Import / Export Regression Test"
    '/// Import / Export Regression-Test
    Dim DocFileList( 30 ) as String, sCurrentFilter as string
    Dim sSourcePath as String, sCurrentFilterExt as string
    Dim iCurrentDocument as Integer
    Dim iDocumentType as Integer
    Dim ExportFileName as string

    ReDim DocFileList( 30 ) as String
    
    '/// load all documents from  "sun_writer\optional\input\number\regression"
    sSourcePath = convertpath( gTesttoolPath & "writer\optional\input\number\regression\" )
    GetFileList ( sSourcePath, "*.*", DocFileList() )
    hListPrint( DocFileList(), "Files to be loaded" ) 

    printlog( "Start loading and exporting files" )
    for iCurrentDocument = 1 to ListCount ( DocFileList() )
        printlog( "File: " & DocFileList( iCurrentDocument ) )
		For iDocumentType = 1 to 3
			Select Case iDocumentType
				Case 1
					sCurrentFilter = "writer8" 'gWriterFilter
                    sCurrentFilterExt = "odt" 'WriterFilterExtension(0)
				Case 2
					sCurrentFilter = "StarOffice XML (Writer)" 'hGetUIFiltername("StarOffice XML (Writer)") & " (.sxw)"
					sCurrentFilterExt = "sxw"
                Case 3
					sCurrentFilter = "StarWriter 5.0" 'StarWriter 5.0 (.sdw)
					sCurrentFilterExt = "sdw"
            end select

            printlog( "Open file: " & DocFileList( iCurrentDocument ) )
			hFileOpenLocally(  DocFileList( iCurrentDocument ) )

            printlog( "Make a copy if the file is write protected" )
            

            printlog( "Test for unexpected dialog: Filter Selection" )
			Kontext "Filterauswahl"
			if ( Filterauswahl.Exists( 1 ) ) then
				Warnlog "  - Filter Selection dialog is up !"
				Filterauswahl.Cancel
				hFileCloseAll()
				goto endsub
			endif

            printlog( "Test for unexpected dialog: ASCII Filter Options" )
			Kontext "AsciiFilterOptionen"
			if ( AsciiFilterOptionen.Exists() ) then 
				AsciiFilterOptionen.Ok
				hFileCloseAll()
				goto endsub
		    endif
			
			printlog( "Allow macro execution (if general setting permit" )
			hAllowMacroExecution()

			'/// check if loaded
			if ( GetDocumentCount <= 2 ) then
				ExportFileName = ConvertPath ( gOfficePath & "user\work\" & _
				                 DateiOhneExt(DateiExtract ( DocFileList( iCurrentDocument )  ) ) & _
				                 "." & sCurrentFilterExt )
				hDeleteFile( ExportFileName ) ' There should never be a reason to overwrite 

                ' Binary export filter was removed
                if iDocumentType < 3 then
                    printlog "  - export as: " & ExportFileName
                    Call hFileSaveAsWithFilter( ExportFileName, sCurrentFilter )
                    Call hCloseDocument

                    printlog "  - load previous saved document"
                    hFileOpen(  ExportFileName )
                endif

				printlog "  - close document"
				hFileCloseAll()
			else
            	Warnlog "  - Doc probably didn't get loaded !!!"
			end if
		next iDocumentType
    next iCurrentDocument
    
endcase

'------------------------------------------------------------------------------------------------------------------------

testcase tToolsNumbering13

    Dim i as integer, iCounter as integer
    '/// #i73790 Regression testing 
    '/// This testcase loads a Worddocument that lost all formattings
    printlog "#i73790 regression testing"
    '/// Load document 'writer\\optional\\input\\number\\regression\\73790.doc'
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\number\regression\73790.doc" )
    
    '/// Document has Heading1-Heading3 formatted, be sure it is
    call wTypeKeys ("<Mod1 Home>")
    '/// Point cursor to top of document with <STRG HOME>
    Select Case iSprache
		case 01 : iCounter = 2
		case 03 : iCounter = 1
		case 07 : iCounter = 2
		case 31 : iCounter = 1
		case 33 : iCounter = 3
		case 34 : iCounter = 2
		case 36 : iCounter = 2
		case 39 : iCounter = 2
		case 45 : iCounter = 1
		case 46 : iCounter = 2
		case 48 : iCounter = 2
		case 49 : iCounter = 3
		case 55 : iCounter = 3
		case 81 : iCounter = 3
		case 82 : iCounter = 1
		case 86 : iCounter = 1
		case 88 : iCounter = 3
		case else : iCounter = 2
	end select
	For i = 1 to 3
        call wTypeKeys ("<Shift End>")
        '/// Select first to third entry and check it i formatted with 'Heading1','Heading2','Heading3'
        Kontext "Textobjectbar"
        if Vorlage.GetSelIndex <> ( i + iCounter ) then
            Warnlog "Heading " & i & " not " & ( i + iCounter ) & " but " & Vorlage.GetSelIndex
        end if
        call wTypeKeys ( "<Home>" )
        call wTypeKeys ( "<Down>" )
    next i

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
