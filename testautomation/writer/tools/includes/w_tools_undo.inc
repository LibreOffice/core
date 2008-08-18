'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_tools_undo.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:44:47 $
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
'* owner : helge.delfs@sun.com
'*
'* short description : Tools for w_undo.bas
'*
'************************************************************************
'*
' #1 CheckUndoStringInUndoList
'*
'\***********************************************************************

sub CheckUndoStringInUndoList(vType as integer, optional vShould as string)
	'/// This function checks the string (localized) in Undo-List
	Dim CurrentUndoString(1) as string, vTypeString as string
	Dim UndoStringFromUndoList as string

	Kontext "StandardBar"
	Sleep 1
	if Undo.IsEnabled = true then
		Undo.OpenMenu
	else
		Warnlog "Unable to read from Undo-Box!"
		exit sub
	end if

	Sleep 1
	Kontext "UndoRedoBox"
	UndoStringFromUndoList = UndoRedoList.GetItemText(1)
	Select case vType
		' Typing
		case 1:
			CurrentUndoString() = Split(UndoStringFromUndoList, ":")
			Select Case iSprache
				Case 01:	vTypeString = "Typing"
				Case 49: 	vTypeString = "EinfÆgen"
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
			end select
		' Overwrite
		case 2:
			CurrentUndoString() = Split(UndoStringFromUndoList, ":")
			Select Case iSprache
				Case 01: vTypeString = "Overwrite"
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
			end select
		' Replace
		case 3:
			CurrentUndoString() = Split(UndoStringFromUndoList, ":")
			Select Case iSprache
				Case 01: vTypeString = "Replace"
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
			end select
		' Delete
		case 4:
			Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Delete ' ###'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
			end select
		' Replace all
		case 5:
			Select Case vShould
				case "14 occurences of 'and'"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Replace: 14 occurrences of 'and'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select
			end select
		' Sort text
		case 6:
			Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Sort text" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt test for this language: " & UndoStringFromUndoList
			end select
		' Convert text -> table
		case 7:
			Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Convert text -> table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
			end select

		'Paragraph
		case 8:
			Select Case vShould
				Case "New Paragraph"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "New Paragraph" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "Delete Paragraph"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete paragraph" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select
			end select

		'Copy / Paste
		case 9:
			Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Paste clipboard" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
			end select

		case 10:
			Select Case vShould
				Case "Accept"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Accept change: Insert 'This is a test'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "Reject"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Reject change: Insert 'This is a test'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertLineBreak"
					Select Case iSprache
						Case 01, 1: if UndoStringFromUndoList <> "Typing: 1 line break(s)" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteLineBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete 1 line break(s)" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertColumnBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert column break" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteColumnBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete paragraph" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertPageBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert page break" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeletePageBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete paragraph" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertField"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert field" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ChangedField"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Field changed" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteField"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete field" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertSpecialCharacter"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert special character" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteSpecialCharacter"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete '#'" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ApplyStyles"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Apply Styles: AUndoHistory" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ChangeStyles"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Change style: AUndoHistory" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DemoteOutline"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Demote outline" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "PromoteOutline"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Promote outline" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertSection"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert section" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ModifySection"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Modify section" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteSection"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete section" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertHyperlink"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Hyperlink" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertHyperlinkButton"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "insert URL button" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "HeaderFooter"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Header/footer changes" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertFootnote"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert footnote" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ModifyFootnote"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Modify footnote options" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteFootnote"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete footnote" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertCaption"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert caption: 'This capt...aracters'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertAutotext"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert AutoText" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertBookmark"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert bookmark: I'm a bookmark" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "RenameBookmark"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Rename bookmark: 'I'm a bookmark' -> 'a book'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList & " -> #i33498"
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteBookmark"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete bookmark: a book" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertNote"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert note" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteNote"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete note" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertScript"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert script" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteScript"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete script" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertIndexEntry"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert index entry" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteIndexEntry"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete 'IndexEntry'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select


				Case "InsertBibliographyEntry"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert bibliography entry" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteBibliographyEntry"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete bibliography entry" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert frame" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "RenameFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Rename frame: 'MyFrame' -> 'MyRenamedFrame'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete frame" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert table: 'Table1'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertRow"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Row" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertColumn"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Column" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteRow"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete row" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteColumn"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete column" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select


				Case "FormatCell"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Format cell" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ApplyTableAttributes"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Apply table attributes" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "MergeTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Merge table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ConvertTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Convert text -> table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "SortTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Sort table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "SplitTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Split Table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete table: 'AnotherTable'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList & " -> #i33497"
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertRuler"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert horizontal ruler" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertGraphic"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert pictures" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteGraphic"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete picture" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ReplaceGraphic"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Replace picture" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertObject"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert OLE-object" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteObject"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete OLE-object" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertFloatingFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert OLE-object" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteFloatingFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete OLE-object" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertDocument"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Document" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "APGS"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Asian Phonetic Guides Setting" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "AutoCorrect"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "AutoCorrect" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "AutoFormat"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "AutoFormat" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "FreeForm"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Polyline with 3 corners" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DrawGroup"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Group draw objects" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DrawUnGroup"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Ungroup group object 'MyGroup'" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "RenameGroup"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Rename 2 Rectangles" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteGroup"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete 2 Rectangles" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "Autocheck/Spellcheck"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Replace: 'Testp' -> 'teste'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlInsert"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlResize"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Resize Control" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlMove"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Move Control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlReplace"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Replace Control" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select


				Case "ControlChange"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Set property 'Name'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlDelete"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete control" then QAErrorlog "#i42852#Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "Header/Footer"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Header/footer changed" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select


				Case else
					Warnlog "Wrong string for this function!"


			end select


		case else:
			QAErrorlog "Wrong type!"

	end select

	if vTypeString > "" then
		if Trim(CurrentUndoString(0)) <> vTypeString then
			Warnlog "Wrong Type in Undo list. Not " & vTypeString & " but " & Trim(CurrentUndoString(0))
		else
			Printlog "-  Type in Undo-List is ok"
		end if
		try
			if Trim(CurrentUndoString(1)) <> vShould then
				Warnlog "Wrong String in Undo list. Not " & vShould & " but " & Trim(CurrentUndoString(1))
			else
				Printlog "-  String in Undo-List is ok"
			end if
		catch
			warnlog "Something wrong with type: " & vTypeString
		endcatch
	end if
	wait 500
    Kontext "StandardBar"
    Undo.Click
    
end sub
