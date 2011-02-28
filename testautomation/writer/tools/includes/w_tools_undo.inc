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
				Case 34:        vTypeString = "Escritura"
				Case 86:        vTypeString = "键入"
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
			end select
		' Overwrite
		case 2:
			CurrentUndoString() = Split(UndoStringFromUndoList, ":")
			Select Case iSprache
				Case 01: vTypeString = "Overwrite"
				Case 34: vTypeString = "Sobrescribir"
				Case 86: vTypeString = "覆盖"
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
			end select
		' Replace
		case 3:
			CurrentUndoString() = Split(UndoStringFromUndoList, ":")
			Select Case iSprache
				Case 01: vTypeString = "Replace"
				Case 34: vTypeString = "Reemplazar"
				Case 86: vTypeString = "替�?�"
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
			end select
		' Delete
		case 4:
			Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Delete multiple selection" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 34: if UndoStringFromUndoList <> "Eliminar selección múltiple" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 86: if UndoStringFromUndoList <> "删除 多项选择" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
			end select
		' Replace all
		case 5:
			Select Case vShould
				case "14 occurences of 'and'"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Replace: 14 occurrences of 'and'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				                Case 34: if UndoStringFromUndoList <> "Reemplazar: 6 ocurrencias de 'and'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入自动图文集" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select
			end select
		' Sort text
		case 6:
			Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Sort text" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 34: if UndoStringFromUndoList <> "Ordenar texto" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 86: if UndoStringFromUndoList <> "排�?文字" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt test for this language: " & UndoStringFromUndoList
			end select
		' Convert text -> table
		case 7:
			Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Convert text -> table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 34: if UndoStringFromUndoList <> "Convertir texto en tabla" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 86: if UndoStringFromUndoList <> "文字转�?��?表格" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
			end select

		'Paragraph
		case 8:
			Select Case vShould
				Case "New Paragraph"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "New Paragraph" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Párrafo nuevo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "新建段�?�" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "Delete Paragraph"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete paragraph" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar Párrafo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 段�?�" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select
			end select

		'Copy / Paste
		case 9:
			Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Paste clipboard" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 34: if UndoStringFromUndoList <> "Pegar del portapapeles" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 86: if UndoStringFromUndoList <> "粘贴剪贴�?�" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
			end select

		case 10:
			Select Case vShould
				Case "Accept"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Accept change: Insert 'This is a test'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Aplicar cambio: Insertar 'This is a test'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "接�?�修改：�?�入 'This is a test'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "Reject"
					Select Case iSprache
                                                Case 01: if UndoStringFromUndoList <> "Reject change: Insert 'This is a test'" then 
                                                         elseif UndoStringFromUndoList = "Reject change: $1" then 
                                                             QaErrorlog "#i97506# - Bug with Undo - Reject."
                                                         else
                                                             Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                         end if
                                                Case 34: if UndoStringFromUndoList = "Rechazar cambio: $1" then 
                                                             QaErrorlog "#i97506# - Bug with Undo - Reject."
                                                         else
                                                             Warnlog "Tell script-writer issue i97506 seems to be fixed, or that a string has to be set to: " & UndoStringFromUndoList
                                                         end if
                                                Case 86: if UndoStringFromUndoList = "拒�?修改：$1" then
                                                             QaErrorlog "#i97506# - Bug with Undo - Reject."
                                                         else
                                                             Warnlog "Tell script-writer issue i97506 seems to be fixed, or that a string has to be set to: " & UndoStringFromUndoList
                                                         end if
			                        Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertLineBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Typing: 1 line break(s)" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Escritura: 1 salto(s) de línea" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "键入: 1 �?�行符" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteLineBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete 1 line break(s)" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar 1 salto(s) de línea" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 1 �?�行符" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertColumnBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert column break" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar salto de columna" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入分�?符" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteColumnBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete paragraph" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar Párrafo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 段�?�" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertPageBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert page break" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar salto de página" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入分页符" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeletePageBreak"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete paragraph" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar Párrafo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 段�?�" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertField"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert field" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar campo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 字段" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ChangedField"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Field changed" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Cambio modificado" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "字段已�?被修改" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteField"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete field" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar campo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 字段" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertSpecialCharacter"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert special character" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar carácter especial" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 特殊字符" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteSpecialCharacter"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete '#'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar '#'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 '#'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ApplyStyles"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Apply Styles: AUndoHistory" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Aplicar estilos: AUndoHistory" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "应用样�?: AUndoHistory" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ChangeStyles"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Change style: AUndoHistory" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "Cambiar estilo: AUndoHistory" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "修改样�?：AUndoHistory" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DemoteOutline"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Promote/demote outline" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "Aumentar/Disminuir nivel" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�级/�?级外框" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "PromoteOutline"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Promote/demote outline" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "Aumentar/Disminuir nivel" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�级/�?级外框" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertSection"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert section" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "Insertar área" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入区域" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ModifySection"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Modify section" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Modificar área" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "修改区域" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteSection"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete section" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "Eliminar área" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除区域" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertHyperlink"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Hyperlink" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar hiperenlace" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入超链接" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertHyperlinkButton"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "insert URL button" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar botón URL" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 URL 按钮" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
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
						Case 34: if UndoStringFromUndoList <> "Insertar nota al pie" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入脚注" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
        					Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ModifyFootnote"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Modify footnote options" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Modificar opciones de nota al pie" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "修改脚注设置" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteFootnote"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete footnote" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar nota al pie" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 脚注" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertCaption"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert caption: 'This capt...aracters'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar título: 'This capt...aracters'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入标题: 'This capt...aracters'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertAutotext"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert AutoText" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar AutoTexto" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入自动图文集" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertBookmark"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert bookmark: I'm a bookmark" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar marcador: I'm a bookmark" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入书签: I'm a bookmark" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "RenameBookmark"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Rename bookmark: 'I'm a bookmark' -> 'a book'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList & " -> #i33498"
						Case 34: if UndoStringFromUndoList <> "marcador renombrado: 'I'm a bookmark' -> 'a book'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList & " -> #i33498"
						Case 86: if UndoStringFromUndoList <> "�?命�??书签: 'I'm a bookmark' -> 'a book'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList & " -> #i33498"
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteBookmark"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete bookmark: a book" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Borrar marcador: a book" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除书签: a book" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertComment"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert comment" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar nota" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 批注" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteComment"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete comment" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar nota" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 批注" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertScript"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert script" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar secuencia de comandos" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 脚本" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteScript"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete script" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar secuencia de comandos" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 脚本" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertIndexEntry"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert index entry" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar entrada de índice" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入索引�?�目" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteIndexEntry"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete 'IndexEntry'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar 'IndexEntry'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 'IndexEntry'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertBibliographyEntry"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert bibliography entry" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar entrada de bibliografía" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 文献目录�?�目" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteBibliographyEntry"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete bibliography entry" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar entrada de bibliografía" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 文献目录�?�目" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert frame" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar marco" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 框架" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "RenameFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Rename frame: 'MyFrame' -> 'MyRenamedFrame'" then QaErrorLog "#i94486# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if InStr(UndoStringFromUndoList, "renombrado") then QaErrorLog "#i94486# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 框架" then QaErrorLog "#i94486# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: " & UndoStringFromUndoList
					end select

				Case "DeleteFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete frame" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "Eliminar marco" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除框对象" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert table: 'Table1'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar tabla: 'Tabla1'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入表格: '表格1'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertRow"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Row" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar fila" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入行" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertColumn"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Column" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar columna" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入列" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteRow"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete row" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Borrar fila" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除行" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteColumn"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete column" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Borrar columna" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除列" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "FormatCell"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Format cell" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Formatear celda" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "格�?化�?�元格" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ApplyTableAttributes"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Apply table attributes" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Aplicar atributos de la tabla" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "应用表格属性" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "MergeTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Merge table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "unir tablas" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�并表格" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ConvertTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Convert text -> table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Convertir texto en tabla" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "文字转�?��?表格" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "SortTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Sort table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Ordenar tabla" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "排�?表格" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "SplitTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Split Table" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Dividir tabla" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "拆分表格" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteTable"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete table: 'AnotherTable'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList & " -> #i33497"
						Case 34: if UndoStringFromUndoList <> "Eliminar tabla 'AnotherTable'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList & " -> #i33497"
						Case 86: if UndoStringFromUndoList <> "删除 表格: 'AnotherTable'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList & " -> #i33497"
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertRuler"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert horizontal ruler" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar regla horizontal" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入水平标尺" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertGraphic"
					Select Case iSprache
                                                Case 01: if (UndoStringFromUndoList = "Insert graphics") OR (UndoStringFromUndoList = "Insert picture") then
                                                         else
                                                             Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                         end if
                                                Case 34: if (UndoStringFromUndoList = "Insertar gráficos") OR (UndoStringFromUndoList = "Insertar imagen") then
                                                         else
                                                             Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                         end if
					        Case 86: if (UndoStringFromUndoList = "�?�入 图形") OR (UndoStringFromUndoList = "�?�入 图片") then
                                                         else
                                                             Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                         end if
		              	                Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
				        end select

				Case "DeleteGraphic"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete graphics" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar imagen" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除图形" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ReplaceGraphic"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Replace graphics" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Reemplazar imagen" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "替�?�图形" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertObject"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert OLE-object" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar Objeto OLE" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 OLE 对象" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteObject"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete OLE-object" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar Objeto OLE" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 OLE 对象" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertFloatingFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert OLE-object" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar Objeto OLE" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 OLE 对象" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteFloatingFrame"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete OLE-object" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar Objeto OLE" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 OLE 对象" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "InsertDocument"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert file" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar archivo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入文件" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "APGS"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Asian Phonetic Guides Setting" then QaErrorLog "#i94534# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Aplicar símbolos fonéticos" then QaErrorLog "#i94534# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "加上拼音或注音符�?�" then QaErrorLog "#i94534# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "AutoCorrect"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "AutoCorrect" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "AutoCorrección" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "自动更正" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "AutoFormat"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "AutoFormat" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "Formateado automático" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "自动格�?" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

                                Case "FreeForm"
				    Select Case iSprache
                                        Case 01: if UndoStringFromUndoList <> "Insert Polyline with 3 corners" then
                                                     if UndoStringFromUndoList = "Insert Polyline with %2 corners" then
                                                         QaErrorlog "#i94530# - Bug with Undo-History for Polygon-Line-tool. " + UndoStringFromUndoList
                                                     else
                                                         Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                     end if
                                                 end if
                                        Case 34: if UndoStringFromUndoList <> "Insertar Polilínea con 3 puntos" then
                                                     if UndoStringFromUndoList = "Insertar Polilínea con %2 puntos" then
                                                         QaErrorlog "#i94530# - Bug with Undo-History for Polygon-Line-tool. " + UndoStringFromUndoList
                                                     else
                                                         Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                     end if
                                                 end if
                                        Case 86: if UndoStringFromUndoList <> "�?�入 有 3 个角的折线" then
                                                     if UndoStringFromUndoList = "�?�入 有 %2 个角的折线" then
                                                         QaErrorlog "#i94530# - Bug with Undo-History for Polygon-Line-tool. " + UndoStringFromUndoList
                                                     else
                                                         Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                     end if
                                                 end if
				        Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
			            end select

				Case "DrawGroup"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Group draw objects" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                                Case 34: if UndoStringFromUndoList <> "Agrupar objetos de dibujo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "组�?�绘图对象" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DrawUnGroup"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Ungroup drawing objects" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Resolver objeto de grupo" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�消组�?�绘图对象" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "RenameGroup"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Rename 2 Rectangles" then QaErrorLog "#i94486# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "" then QaErrorLog "#i94486# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "组�?�绘图对象" then QaErrorLog "#i94486# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "DeleteGroup"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete 2 Rectangles" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar 2 Rectángulos" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 2 矩形" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "Autocheck/Spellcheck"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Replace: 'Testp' -> 'Test'" then QaErrorLog "#i97506# - Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList = "Sobrescribir: $1" then
						         elseif InStr(UndoStringFromUndoList, "'Testp'") then 
							     QaErrorLog "#i97506# - Wrong text in Undo-List: " & UndoStringFromUndoList
							 end if
						Case 86: if UndoStringFromUndoList = "覆盖: $1" then
						             QaErrorLog "#i97506# - Wrong text in Undo-List: " & UndoStringFromUndoList
						         elseif InStr(UndoStringFromUndoList, "'Testp'") then 
							     QaErrorlog "#i97506# - Bug fixed: " & UndoStringFromUndoList
							 end if
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlInsert"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Insert Control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Insertar Campo de control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "�?�入 控制" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlMove"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Move Control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Desplazar Campo de control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "移动 控制" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlResize"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Resize Control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Modificar el tamaño de Campo de control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "更改大�? 控制" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlReplace"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Replace Control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Reemplazar campo de control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "替�?�控制" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlChange"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Set property 'Name'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Establecer propiedad 'Name'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "设置属性 'Name'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "ControlDelete"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Delete Control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "Eliminar Campo de control" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "删除 控制" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case "Header/Footer"
					Select Case iSprache
						Case 01: if UndoStringFromUndoList <> "Header/footer changed" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 34: if UndoStringFromUndoList <> "" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case 86: if UndoStringFromUndoList <> "" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
						Case else: QAErrorlog "Please adapt test for this language: "  & UndoStringFromUndoList
					end select

				Case else
					Warnlog "Wrong string for this function!"

			end select

        case 11:  'Delete ' ###'
            Select Case iSprache
				Case 01: if UndoStringFromUndoList <> "Delete ' ###'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
                                Case 34: if UndoStringFromUndoList <> "Eliminar 'g'" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case 86: if UndoStringFromUndoList <> "�?�入自动图文集" then Warnlog "Wrong text in Undo-List: " & UndoStringFromUndoList
				Case else: QAErrorlog "Please adapt tests for current language! (" & CurrentUndoString(0) & ")"
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
    Call wTypeKeys "<ESCAPE>"
end sub
