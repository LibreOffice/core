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
'* short description : Writer Undo-History - Test
'*
'\***********************************************************************

sub w_undo_history_2

    ' a > 20 characters long string
    Call tUndoOverwriteLong			' Typing & Overwrite
    Call tUndoFindAndReplaceLong			' Replace
    Call tUndoDeleteMultiSelection			' Delete a multi-selection
    Call tUndoParagraph			' Paragraphs
    Call tUndoClipboard		' Copy/Paste
    Call tUndoChanges		' Changes accept an reject
    Call tUndoLineBreak		' Manual breaks - line break
    Call tUndoColumnBreak		' Manual breaks - column break
    Call tUndoPageBreak		' Manual breaks - page break
    Call tUndoFields		' Fields (Insert/Modify / Delete)
    Call tUndoSpecialCharacter		' Special Character (Insert/ Delete)
    Call tUndoStyles		' Styles (Apply/Modify/Find&Replace/Promote/Demote)

end sub

testcase tUndoOverwriteLong

    '/// <B>Typing & overwrite a > 20 characters long string</B>
    '/// new document
    Call hNewDocument
    '/// <U><B>CHECK: Typing</B></U>
    printlog "- TYPING"
    '/// write a > 20 characters long string: ThisSentenceHasMoreThan20Characters
    Call wTypeKeys ( "ThisSentenceHasMoreThan20Characters" )
    '/// string in Undo list has to be: Typing: ThisSente...aracters
    Call CheckUndoStringInUndoList ( 1, "'ThisSent...racters'" )

    '/// <U><B>CHECK: Overwrite</B></U>
    printlog "- OVERWRITE"
    '/// press 'Home' key
    Call wTypeKeys "<Home>"
    '/// press 'Ins' key
    Call wTypeKeys "<Insert>"
    '/// write a > 20 characters long string: ThePreviousSentenceHasBeenOverwritten
    Call wTypeKeys ( "ThePreviousSentenceHasBeenOverwritten" )
    '/// string in Undo list has to be: Overwrite: ThePrevio...rwritten
    Call CheckUndoStringInUndoList ( 2, "'ThePrevio...rwritten'" )

    '/// CloseDocument
    Call hCloseDocument
endcase


testcase tUndoFindAndReplaceLong

    '/// <B>Replace a > 20 characters long string</B>
    '/// new Document
    '/// <U><B>CHECK: Replace</B></U>
    printlog "- REPLACE"
    Call hNewDocument
    '/// write a > 20 characters long string: ThePreviousSentenceHasBeenOverwritten
    Call wTypeKeys ( "ThePreviousSentenceHasBeenOverwritten" )
    '/// Select text
    Call wTypeKeys ( "<Shift Home>" )
    '/// Edit / Find and Replace
    EditSearchAndReplace
    Kontext "FindAndReplace"
    '/// Type 'ThisIsAboutALongTextThatHadToBeReplaced' in 'Replace with' field
    ReplaceWith.Settext "ThisIsAboutALongTextThatHadToBeReplaced"
    '/// Choose Button 'Replace' and close dialog
    ReplaceBtn.Click
    wait 500
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
            catch
                Active.Ok
            endcatch
        end if
    end if
    Kontext "FindAndReplace"
    FindAndReplace.Close
    Kontext "DocumentWriter"
    '/// string in Undo list has to be: Replace: 'ThePrevio...rwritten' -> 'ThisIsAbo...Replaced'
    Call CheckUndoStringInUndoList ( 3, "'ThePrevio...rwritten' -> 'ThisIsAbo...Replaced'" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoDeleteMultiSelection

    '/// <B>Delete a multi-selection</B>
    printlog "- Delete a multi-selection"
    '/// new Document
    Call hNewDocument
    '/// write down: the deletion of a multi-selection
    Call wTypeKeys ( "the deletion of a multi-selection" )
    '/// with Shift + F8 enter multi-selection mode
    Call wTypeKeys ( "<SHIFT F8>" )
    '/// multi-select "the ", " a" and "multi-"
    Call wTypeKeys ( "<MOD1 Left>" )
    Call wTypeKeys ( "<MOD1 SHIFT Left>", 2)
    Call wTypeKeys ( "<Left>" )
    Call wTypeKeys ( "<SHIFT Left>", 2)
    Call wTypeKeys ( "<Left>" )
    Call wTypeKeys ( "<MOD1 Left>", 2)
    Call wTypeKeys ( "<Left>" )
    Call wTypeKeys ( "<MOD1 SHIFT Left>")
    '/// Delete the multi-selection
    Call wTypeKeys ( "<DELETE>" )
    '/// string in Undo-list has to be: 'Delete: multi-selection'"
    Call CheckUndoStringInUndoList ( 4, "Delete multiple selection" )
    '/// Close document
    Call hCloseDocument

endcase


testcase tUndoParagraph

    '/// <B>Insert and delete a paragraph</B>
    printlog "- Insert Paragraph"
    '/// new document
    Call hNewDocument
    '/// press return to insert a new paragraph
    Call wTypeKeys ( "<Return>" )
    '///  string in Undo-list has to be: "New paragraph"
    Call CheckUndoStringInUndoList ( 8, "New Paragraph" )
    printlog "- Delete Paragraph"
    '/// press Backspace to delete paragraph
    Call wTypeKeys ( "<Backspace>" )
    '///  string in Undo-list has to be: "Delete paragraph"
    Call CheckUndoStringInUndoList ( 8, "Delete Paragraph" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoClipboard
    Dim i as integer

    '/// <B>Copy / Paste</B>
    printlog "- Copy/Paste"
    '/// new document
    Call hNewDocument
    '/// Insert some text in document and copy it
    Call wTypeKeys ( "CopyAndPasteInUndoList" )
    Call wTypeKeys ( "<Shift Home>" )
    EditCopy
    Call wTypeKeys ( "<End><Return>" )
    '/// Paste the text in all available formats
    For i = 1 to 4
        Sleep 2
        try
            EditPasteSpecialWriter
        catch
            i=5
        endcatch
        Kontext "InhaltEinfuegen"
        try
            Auswahl.Select i
            printlog " - Paste as: " + Auswahl.GetSeltext
            wait 500
        catch
            Warnlog "Not 4 but " & Auswahl.GetItemCount & " Clipboard-formats"
        endcatch
        InhaltEinfuegen.Ok
        '///  string in Undo-list has to be: "Paste clipboard"
        Call CheckUndoStringInUndoList ( 9 )
        Call wTypeKeys ( "<Return>" )
    next i
    '/// close document
    Call hCloseDocument
endcase


testcase tUndoChanges

    '/// <B>Changes accept and reject</B>
    printlog "- Changes accept and reject"
    '/// new document
    Call hNewDocument
    UseBindings
    '/// Select Edit / Changes / Record
    EditChangesRecord
    '/// Insert a string in document
    Call wTypeKeys ( "This is a test" )
    '/// Select Edit / Changes / Accept or Reject
    EditChangesAcceptOrReject
    '/// Dialog 'Accept or reject changes' has to come up ///
    Kontext "Redlining"
    if Redlining.Exists then
        '/// + Select tabpage 'List'
        TabControl.SetPage TabListe
        '/// + Select 'Accept All'
        Akzeptieren.Click
        Sleep 2
        '/// + Close Redlining dialog
        Redlining.Close
    else
        Warnlog "Redlining dialog not up!"
        Call hCloseDocument
        goto endsub
    end if
    '///  string in Undo-list has to be: "Accept change: Insert 'this is a test'")
    Call CheckUndoStringInUndoList ( 10, "Accept" )

    '/// Insert a line break in document
    Call wTypeKeys ( "<Return>" )
    '/// Insert a string in document
    Call wTypeKeys ( "This is a test" )
    '/// Select Edit / Changes / Accept or Reject
    EditChangesAcceptOrReject
    '/// Dialog 'Accept or reject changes' has to come up ///
    Kontext "Redlining"
    if Redlining.Exists then
        '/// + Select tabpage 'List'
        TabControl.SetPage TabListe
        '/// + Select 'Accept All'
        wait 500
        ListItem.TypeKeys "<Up>"
        wait 500
        Ablehnen.Click
        Sleep 2
        '/// + Close Redlining dialog
        Redlining.Close
    else
        Warnlog "Redlining dialog not up!"
        Call hCloseDocument
        goto endsub
    end if

    '///  string in Undo-list has to be: "Reject change: Insert 'this is a test'")
    Call CheckUndoStringInUndoList ( 10, "Reject" )

    Call hCloseDocument
endcase


testcase tUndoLineBreak

    '/// <B>Manual breaks (line breaks)</B>
    printlog "Manual breaks (line breaks)"
    printlog "- Insert line break"
    '/// new document
    Call hNewDocument
    '/// Insert a line break (Shift return)
    Call wTypeKeys ( "<SHIFT RETURN>" )
    '///  string in Undo-list has to be: "Insert line break")
    Call CheckUndoStringInUndoList ( 10, "InsertLineBreak" )

    '/// Delete line break
    printlog "- Delete line break"
    Call wTypeKeys ( "<SHIFT RETURN><Backspace>" )
    '///  string in Undo-list has to be: "Delete line break")
    Call CheckUndoStringInUndoList ( 10, "DeleteLineBreak" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoColumnBreak

    '/// <B>Column breaks</B>
    printlog "- Column breaks"
    '/// new document
    Call hNewDocument
    '/// Format / Columns
    FormatColumns
    Kontext "FormatColumn"
    '/// set number of columns to 2
    Columns.Settext 2
    FormatColumn.Ok
    '/// Insert a column break (Strg+Shift+Return)
    Call wTypeKeys ( "<Mod1 Shift Return>" )
    printlog "Insert a column break"
    '///  string in Undo-list has to be: "Insert column break")
    Call CheckUndoStringInUndoList ( 10, "InsertColumnBreak" )

    '/// Delete a column break (Strg+Shift+Return+Backspace)
    printlog "Delete a column break"
    Call wTypeKeys ( "<Mod1 Shift Return><Backspace>" )
    '///  string in Undo-list has to be: "Delete column break")
    Call CheckUndoStringInUndoList ( 10, "DeleteColumnBreak" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoPageBreak

    '/// <B>Page breaks</B>
    printlog "- Page breaks"
    '/// new document
    Call hNewDocument
    '/// Format / Columns
    InsertManualBreak
    Kontext "UmbruchEinfuegen"
    printlog "Insert a page break"
    Seitenumbruch.Check
    UmbruchEinfuegen.Ok
    '///  string in Undo-list has to be: "Insert page break")
    Call CheckUndoStringInUndoList ( 10, "InsertPageBreak" )

    printlog "Delete a page break"
    InsertManualBreak
    Kontext "UmbruchEinfuegen"
    '/// Insert a page break
    Seitenumbruch.Check
    UmbruchEinfuegen.Ok
    Call wTypeKeys ( "<Backspace>" )
    '///  string in Undo-list has to be: "Delete page break")
    Call CheckUndoStringInUndoList ( 10, "DeletePageBreak" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoFields

    '/// <B>Fields (Insert/Modify / Delete)</B>
    printlog "- Fields (Insert)"
    '/// new document
    Call hNewDocument
    '/// Insert Field 'Date'
    InsertFieldsDate
    '///  string in Undo-list has to be: "Insert field")
    Call CheckUndoStringInUndoList ( 10, "InsertField" )

    printlog "- Fields (Modify)"
    '/// Press Home-key
    Call wTypeKeys ( "<Home>" )
    '/// Edit fields and select 4th entry in Format-list
    EditFields
    Kontext "FeldbefehlBearbeitenDokument"
    Zahlenformat.Select 4
    FeldbefehlBearbeitenDokument.Ok
    '///  string in Undo-list has to be: "Changed field")
    Call CheckUndoStringInUndoList ( 10, "ChangedField" )

    printlog "- Fields (Delete)"
    '/// Press delete-key
    Call wTypeKeys ( "<Delete>" )
    '///  string in Undo-list has to be: "Delete field")
    Call CheckUndoStringInUndoList ( 10, "DeleteField" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoSpecialCharacter

    '/// <B>Special Character (Insert / Delete )</B>
    printlog "- Special Character (Insert)"
    '/// new document
    Call hNewDocument
    '/// Insert a special Character
    InsertSpecialCharacterWriter
    Kontext "Sonderzeichen"
    SchriftListe.TypeKeys "<Home>"
    SchriftListe.TypeKeys "<Right>", 3
    Sonderzeichen.Ok
    '///  string in Undo-list has to be: "Insert special character")
    Call CheckUndoStringInUndoList ( 10, "InsertSpecialCharacter" )

    printlog "- Special Character (Delete)"
    '/// Delete special character with <Backspace>
    Call wTypeKeys ("<Backspace>" )
    Call CheckUndoStringInUndoList ( 10, "DeleteSpecialCharacter" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoStyles
    Dim i as integer, StyleText as string, UndoText as string
    Dim NewStyle as string

    '/// <B>Styles (Apply/Modify/Find&Replace/Promote/Demote)</B>
    printlog "- Styles (Apply)"
    '/// new document
    Call hNewDocument
    '/// type some text
    Call wTypeKeys ( "I have the new style" )
    '/// Select text
    Call wTypeKeys ( "<Shift Home>" )
    '/// Apply Style to a newly created style
    Call wStyleCreate ( "AUndoHistory", "Paragraph" )
    '///  string in Undo-list has to be: "Apply styles: AUndoHistory")
    Call CheckUndoStringInUndoList ( 10, "ApplyStyles" )

    '/// Modify Style through Stylist
    if wStyleSelect ( "AUndoHistory" ) = false then
        Warnlog "Defined style has not been found in Stylist->Test aborted"
        Call hCloseDocument
        goto endsub
    end if

    Vorlagenliste.OpenContextMenu
    Call hMenuSelectNr(2)
    Kontext
    Active.Setpage TabFontEffects
    Kontext "TabFontEffects"
    FontColor.Select FontColor.GetItemCount
    TabFontEffects.Ok
    Sleep 1
    '///  string in Undo-list has to be: "Change styles: AUndoHistory")
    Call CheckUndoStringInUndoList ( 10, "ChangeStyles" )

    printlog "Styles (Replace)"
    '/// Edit / Search and Replace
    EditSearchAndReplace
    kontext "FindAndReplace"
    if SearchForStyles.IsVisible = False then More.Click
    SearchForStyles.Check
    Sleep 1
    ReplaceWithStyle.Select 4
    wait 500
    ReplaceBtn.Click
    NewStyle = ReplaceWithStyle.GetSelText
    Select Case iSprache
    Case 01: StyleText = "Replace style: AUndoHistory -> " & NewStyle
    Case 34: StyleText = "Reemplazar estilo: AUndoHistory -> " & NewStyle
    Case 86: StyleText = "替换样式: AUndoHistory -> " & NewStyle
    Case else: QAErrorlog "Please adapt test for this language"
    end select
    wait 500
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
            catch
                Active.Ok
            endcatch
        end if
    end if
    kontext "FindAndReplace"
    SearchForStyles.UnCheck
    FindAndReplace.Close

    '///  string in Undo-list has to be: "Replace Styles: 'AUndoHistory -> ...")
    Kontext "StandardBar"
    try
        Undo.OpenMenu
        Kontext "UndoRedoBox"
        UndoText = UndoRedoList.GetItemText(1)
        if UndoText <> StyleText then
            Warnlog "Wrong text in Undo-List: " & UndoText
        end if
        Kontext "StandardBar"
        Undo.Click
    catch
        Warnlog "Something wrong checking Undo-List!"
    endcatch

    printlog "Styles (Promote)"
    '/// point cursor to beginning of document
    Call wTypeKeys ( "<Mod1 Home>" )
    ' To promote style it has to be in the heading group
    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabKapitelnumerierung
    Kontext "TabKapitelnumerierung"
    Absatzvorlage.Select NewStyle
    TabKapitelnumerierung.Ok

    Call wNavigatorAuswahl ( 1, 1, true )
    Kontext "NavigatorWriter"
    '/// Select Promote in Navigator
    try
        Senken.Click
        '///  string in Undo-list has to be: "Demote Outline")
        Call CheckUndoStringInUndoList ( 10, "DemoteOutline" )
    catch
        Warnlog "Unable to demote outline"
    endcatch

    '/// Select Demote in Navigator
    printlog "Styles (Demote)"
    Kontext "NavigatorWriter"
    try
        Senken.Click
        '///  string in Undo-list has to be: "Promote Outline")
        Call CheckUndoStringInUndoList ( 10, "PromoteOutline" )
    catch
        Warnlog "Unable to promote outline"
    endcatch

    '/// CloseDocument
    Call hCloseDocument
endcase
