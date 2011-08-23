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
'* short description : Writer Undo-History - Test ( Typing, Overwrite, Replace, Delete, Sort, Convert To table)
'*
'\***********************************************************************

sub w_undo_history_1

    ' a < 20 characters long string
    Call tUndoOverwrite		' Typing & Overwrite
    Call tUndoFindAndReplace		' Replace
    Call tUndoReplaceAll		' Replace all & Delete
    Call tUndoSort		' Sort
    Call tUndoTableConvert		' Text convert to table

end sub

testcase tUndoOverwrite

    '/// new document
    Call hNewDocument
    '/// <U><B>CHECK: Typing</B></U>
    printlog "- TYPING"
    '/// write a < 20 characters long string: If20CharactersThere
    Call wTypeKeys "If20CharactersThere"
    '/// string in Undo list has to be: Typing: If20CharactersThere
    Call CheckUndoStringInUndoList ( 1, "'If20Char...rsThere'" )

    '/// <U><B>CHECK: Overwrite</B></U>
    printlog "- OVERWRITE"
    '/// press 'Home' key
    Call wTypeKeys "<Home>"
    '/// press 'Ins' key
    Call wTypeKeys "<Insert>"
    '/// write a < 20 characters long string: IsThatDifferent4You
    Call wTypeKeys "IsThatDifferent4You"
    '/// string in Undo list has to be: Overwrite: IsThatDifferent4You
    Call CheckUndoStringInUndoList ( 2, "'IsThatDifferent4You'" )

    '/// CloseDocument
    Call hCloseDocument
endcase

testcase tUndoFindAndReplace

    '/// new Document
    '/// <U><B>CHECK: Replace</B></U>
    printlog "- REPLACE"
    Call hNewDocument
    '/// write a < 20 characters long string: If20CharactersThere
    Call wTypeKeys "If20CharactersThere"
    '/// Select text
    Call wTypeKeys "<Shift Home>"
    '/// Edit / Find and Replace
    EditSearchAndReplace
    Kontext "FindAndReplace"
    '/// Type 'ThisReplacementText' in 'Replace with' field
    ReplaceWith.Settext "ThisReplacementText"
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
    '/// string in Undo list has to be: Replace: 'IsThatDifferent4You' -> 'ThisReplacementText'
    Call CheckUndoStringInUndoList ( 3, "'If20CharactersThere' -> 'ThisReplacementText'" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoReplaceAll
    Dim CheckString as string

    '///  new document
    '/// <U><B>CHECK: Replace all</B></U>
    printlog "- REPLACE ALL"
    Call hNewDocument
    '/// Insert the dummy text and point cursor at the top of the document
    Call wBlindtextEinfuegen
    Call wTypeKeys "<Mod1 Home>"
    '/// Edit / Find and Replace
    EditSearchAndReplace
    Kontext "FindAndReplace"
    '/// Type 'and' in 'Search for' field
    SearchFor.Settext "and"
    '/// Type 'ThisReplacementText' in 'Replace with' field
    ReplaceWith.Settext "###"
    '/// Choose Button 'Replace' and close dialog
    ReplaceAll.Click
    wait 500
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then Active.Ok
    end if
    Kontext "FindAndReplace"
    FindAndReplace.Close

    Kontext "DocumentWriter"
    '/// string in Undo list has to be: Replace: '14 occurences of and'
    Call CheckUndoStringInUndoList ( 5, "14 occurences of 'and'" )

    '/// <U><B>CHECK: Delete</B></U>
    '///  Backspace 4 times to delete characters
    Call wTypeKeys "<Backspace>", 4
    '/// string in Undo list has to be: Delete: '###'
    printlog "- DELETE"
    Call CheckUndoStringInUndoList ( 11, "' ###'" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoSort

    '/// new document
    Call hNewDocument
    '/// CHECK: Text sort
    printlog "- TEXT SORT"
    '/// Enter some text like "This<Return>is a<Return>test<Return>"
    Call wTypeKeys "This"
    Call wTypeKeys "<Return>"
    Call wTypeKeys "is a"
    Call wTypeKeys "<Return>"
    Call wTypeKeys "test"
    Call wTypeKeys "<Return>"
    '/// Select all and Tools -> Sort
    Call wTypeKeys "<Mod1 A>"
    ToolsSort
    Kontext "Sortieren"
    Sortieren.Ok

    '/// string in Undo list has to be: "Sort text"
    Call CheckUndoStringInUndoList ( 6, "Sort text" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoTableConvert

    '/// new document
    Call hNewDocument
    '/// CHECK: Text convert to table
    printlog "TEXT CONVERT TO TABLE"
    '/// Enter some text like "This<Return>is a<Return>test<Return>"
    Call wTypeKeys "This"
    Call wTypeKeys "<Return>"
    Call wTypeKeys "is a"
    Call wTypeKeys "<Return>"
    Call wTypeKeys "test"
    Call wTypeKeys "<Return>"
    '/// Select all and Tools - Text <> Table
    Call wTypeKeys "<Mod1 A>"
    ToolsTextInTable
    Kontext "TextInTabelleUmwandeln"
    Absatz.Check
    TextInTabelleUmwandeln.Ok
    '/// string in Undo list has to be: "Convert text -> table"
    Call CheckUndoStringInUndoList ( 7, "Convert text -> table" )

    '/// Close document
    Call hCloseDocument
endcase
