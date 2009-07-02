'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_207_.inc,v $
'*
'* $Revision: 1.3 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:19:53 $
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
'* short description : Testing Number and Bullets
'*
'\***********************************************************************

sub w_207_

    Call tToolsNumbering12
    Call tToolsNumbering13

end sub

'------------------------------------------------------------------------------------------------------------------------

testcase tToolsNumbering12
   QaErrorLog "Testcase outcommented due to incompability with new Filtername-behaviour."
   goto endsub

    printlog "Import / Export Regression Test"
    '/// Import / Export Regression-Test
    Dim DocFileList( 30 ) as String, sCurrentFilter as string
    Dim sSourcePath as String, sCurrentFilterExt as string
    Dim i as Integer
    Dim k as Integer
    Dim x as Integer
    Dim ExportFileName as string

    ReDim DocFileList( 30 ) as String
    '/// load all documents from  "sun_writer\\optional\\input\\number\\"
    sSourcePath = ConvertPath ( gTesttoolPath + "writer\optional\input\number\regression\" )
    GetFileList ( sSourcePath, "*.*", DocFileList() )

        x = ListCount ( DocFileList() )
        for i = 1 to x
            printlog " - (" & i & "/" & x & "): " & DocFileList(i)
			For k = 1 to 3
				Select Case k
					Case 1
						sCurrentFilter = "writer8" 'gWriterFilter
                        sCurrentFilterExt = "odt" 'WriterFilterExtension(0)
					Case 2
						sCurrentFilter = "StarOffice XML (Writer)" 'hGetUIFiltername("StarOffice XML (Writer)") & " (.sxw)"
						sCurrentFilterExt = "sxw"
                    Case 3
						sCurrentFilter = "StarWriter 5.0/GlobalDocument" 'StarWriter 5.0 (.sdw)
						sCurrentFilterExt = "sdw"
                end select

				'/// Open a new writer document
				Call hNewDocument
				'/// open file
				sleep 2
				Call hFileOpen ( DocFileList(i) )
                Call sMakeReadOnlyDocumentEditable

				Kontext "Filterauswahl"
				if Filterauswahl.Exists then
					Warnlog "  - Filter Selection dialog is up !"
					Filterauswahl.Cancel
					Call hCloseDocument
				end if
				Sleep 5
				Kontext "AsciiFilterOptionen"
				if AsciiFilterOptionen.Exists then AsciiFilterOptionen.Ok
				sleep (5)
				' Check for macro alert
				Kontext "SecurityWarning"
				if SecurityWarning.Exists then
					SecurityWarning.Cancel
				end if

				'/// check if loaded
				if GetDocumentCount >= 1 then
					ExportFileName = ConvertPath ( gOfficePath + "user\work\" + DateiOhneExt(DateiExtract ( DocFileList(i)  ) ) &  "." & sCurrentFilterExt )
					printlog "  - export as: " & ExportFileName
					Call hFileSaveAsWithFilterKill ( ExportFileName, sCurrentFilter )
					Sleep (2)
					Call hCloseDocument
					printlog "  - load previous saved document"
					Call hFileOpen ( ExportFileName )
					Sleep (2)
					printlog "  - close document"
					Call hCloseDocument
				else
                	Warnlog "  - Doc probably didn't get loaded !!!"
				end if
			next k
            Sleep 2
        next i

endcase

'------------------------------------------------------------------------------------------------------------------------

testcase tToolsNumbering13

    Dim i as integer, iCounter as integer
    '/// #i73790 Regression testing 
    '/// This testcase loads a Worddocument that lost all formattings
    printlog "#i73790 regression testing"
    '/// Load document 'writer\\optional\\input\\number\\regression\\73790.doc'
    Call hFileOpen ( gTesttoolPath + "writer\optional\input\number\regression\73790.doc" )
    Call sMakeReadOnlyDocumentEditable
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
