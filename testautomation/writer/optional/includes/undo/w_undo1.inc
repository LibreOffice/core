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
'* short description : Writer Undo/Redo - Test
'*
'\***********************************************************************

testcase tEditUndoRedo
    '/// Edit / Undo - Redo ///
    PrintLog "- Edit / Undo - Redo"
    Printlog "  - Set Undobuffer to 2"
    '/// Call Options and Set Number of steps to 2 ///
    ToolsOptions
    Call hToolsOptions("StarOffice","Memory")
    UndoSteps.SetText "2"
    '/// Close Optionsdialog ///
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// New document ///
    Call hNewDocument
    '/// Write any text in document and insert a table ///
    Call wTypeKeys "Clipboard"
    Call hTabelleEinfuegen
    Call wTypeKeys "<Down><Down>text"

    '/// Call Edit / Undo 2 times ///
    printlog "  - Undo 2 times"
    EditUndo
    Sleep 1
    EditUndo
    Sleep 1
    '/// try Undo more than two 2 times ///
    try
        EditUndo
        Sleep 1
        Warnlog "Possible to perform Undo more than 2 times!"
        EditRedo
    catch
        Sleep 1
    endcatch
    '/// Check if Undo has been worked correctly ///
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> "Clipboard" then Warnlog "Undo is not working correct"
    Call wTypeKeys "<End>"

    '/// select Redo 2 times ///
    printlog "  - Redo 2 times"
    EditRedo
    Sleep 1
    EditRedo
    Sleep 1
    '/// try Redo more than 2 times ///
    try
        EditRedo
        Warnlog "Able to perform Redo more than 2 times"
    catch
        Sleep 1
    endcatch
    EditCopy
    if GetClipboardText <> "text" then Warnlog "Redo is not working correct"
    '/// Close document ///
    Call hCloseDocument
    Sleep 2

    '/// Select Options again and try to set Undobuffer to 0 (min should be 1) ///
    Printlog "  - set Undobuffer to 0"
    ToolsOptions
    Call hToolsOptions("StarOffice","Memory")
    UndoSteps.ToMin
    if UndoSteps.GetText <> "1" then
        Warnlog "Undobuffer has not been set to 1!"
    else
        Printlog "  - Undobuffer can only set to 1!"
    end if
    '/// Close Optionsdialog ///
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    '/// New document ///
    Call hNewDocument
    Call wTypeKeys "Don't undo this<Return>Undo this"
    '/// Insert any text and try to Undo more than 1 time ///

    EditUndo
    try
        EditUndo
        Warnlog "More than one Undo could be performed!"
    catch
    endcatch
    Call wTypeKeys "<Shift Home>"
    Wait 500
    EditCopy
    if GetClipboardText <> "Undo " then Warnlog "The UndoStep couldn't performed correct!"

    '/// Close document ///
    Call hCloseDocument

    Printlog "  - Set Undobuffer to Default ( 20 )"
    '/// Open options again and set Number of steps for Undo to default (20) ///
    ToolsOptions
    Call hToolsOptions("StarOffice","Memory")
    UndoSteps.SetText "20"

    '/// Close Options ///
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
endcase

' -----------------------------------------------------------------------------------

testcase tEditUndoTOX
    QaErrorLog "tEditUndoTOX outcommented due to makeover."
    goto endsub

    Dim CompareThis as string, Compare1 as string, Compare2 as string, Compare3 as string
    Dim CompareWith1 as string, CompareWith2 as string, MenuEntryEnabled as boolean
    ' Strings depends on the language so this testcase has to be adapted for all languages
    Select Case iSprache
    Case 1:	Compare1 = "Undo: "
        Compare2 = "Insert index/table"
        Compare3 = "Remove index/table"
    Case 49:Compare1 = "R�ckg�ngig"
        Compare2 = "Einf�gen Verzeichnis"
        Compare3 = "Verzeichnis aufheben"
    Case else:
        QAErrorlog "Please adapt this testcase for this language!"
        Compare1 = "Undo: "
        Compare2 = "Insert index/table"
        Compare3 = "Remove index/table"
    end select
    CompareWith1 = Compare1 & Compare2
    CompareWith2 = Compare1 & Compare3

    '/// Open a new writerdocument
    Call hNewDocument

    '/// Insert an index in document
    printlog "Insert Index"
    InsertIndexes

    Kontext
    Active.Setpage TabVerzeichnisseVerzeichnis
    Kontext "TabVerzeichnisseVerzeichnis"
    Verzeichnistitel.Settext "MyUndoRedoIndex"
    TabVerzeichnisseVerzeichnis.Ok

    '/// Check if Edit/Undo menu shows 'Undo: Insert Index'
    Call hUseMenu()
    try
        MenuSelect(Menugetitemid(2))	' Opens the Edit - Menu
        MenuEntryEnabled = True
    catch
        MenuEntryEnabled = False
    endcatch
    if MenuEntryEnabled = True then
        CompareThis = hhEntferneTilde(MenuGetItemText(Menugetitemid(1)))
        Call MenuSelect(0)
        if CompareWith1 > "" then
            if CompareThis <> CompareWith1 then
                Warnlog "Edit / Undo entry is wrong: " & CompareThis
            else
                printlog " - Menu Edit / Undo is ok"
            end if
        else
            QAErrorLog "Use this string in this testcase: " & CompareThis
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Edit / Undo in menu is disabled !"
    end if

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Up><Home>"
    Wait 500
    '/// Check if Undo dropdown-Icon shows:	'Insert index'
    Select Case CheckUndoEntry ( 1, Compare2 )
    Case 0:	Warnlog " - Undo-dropdown is disabled!"
    Case 1:	Warnlog " - Entry in Undo-dropdown is wrong!"
    Case 2:	printlog " - Entry in Undo-dropdown is ok!"
    end select

    '/// Update index with context menu entry: Update index"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    ' Update Index/table in context menu
    printlog "Update Index"
    Call MenuSelect(Menugetitemid(7))

    '/// Check if Edit/Undo menu shows 'Undo: Insert Index' after index update
    Call hUseMenu()
    try
        MenuSelect(Menugetitemid(2))	' Opens the Edit - Menu
        MenuEntryEnabled = True
    catch
        MenuEntryEnabled = False
    endcatch
    if MenuEntryEnabled = True then
        CompareThis = hhEntferneTilde(MenuGetItemText(Menugetitemid(1)))
        Call MenuSelect(0)
        if CompareWith1 > "" then
            if CompareThis <> CompareWith1 then
                Warnlog "Edit / Undo entry is wrong: " & CompareThis
            else
                printlog " - Menu Edit / Undo is ok"
            end if
        else
            QAErrorLog "Use this string in this testcase: " & CompareThis
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Edit / Undo in menu is disabled !"
    end if

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Up><Home>"
    Wait 500
    '/// Check if Undo dropdown-Icon shows: 'Insert index' after index update
    Select Case CheckUndoEntry ( 1, Compare2 )
    Case 0:	Warnlog " - Undo-dropdown is disabled!"
    Case 1:	Warnlog " - Entry in Undo-dropdown is wrong!"
    Case 2:	printlog " - Entry in Undo-dropdown is ok!"
    end select

    '/// Delete index with context menu entry: Delete index"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    ' Delete Index/table in context menu
    printlog "Remove Index"
    Call MenuSelect(Menugetitemid(9))

    '/// Check if Edit/Undo menu shows 'Undo: Remove index' after deleting index
    Call hUseMenu()
    try
        MenuSelect(Menugetitemid(2))	' Opens the Edit - Menu
        MenuEntryEnabled = True
    catch
        MenuEntryEnabled = False
    endcatch
    if MenuEntryEnabled = True then
        CompareThis = hhEntferneTilde(MenuGetItemText(Menugetitemid(1)))
        Call MenuSelect(0)
        if CompareWith2 > "" then
            if CompareThis <> CompareWith2 then
                Warnlog "Edit / Undo entry is wrong: " & CompareThis
            else
                printlog " - Menu Edit / Undo is ok"
            end if
        else
            QAErrorLog "Use this string in this testcase: " & CompareThis
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Edit / Undo in menu is disabled !"
    end if

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Up><Home>"
    Wait 500
    '/// Check if Undo dropdown-Icon shows: 'Remove index' after deleting index
    Select Case CheckUndoEntry ( 1, Compare3 )
    Case 0:	Warnlog " - Undo-dropdown is disabled!"
    Case 1:	Warnlog " - Entry in Undo-dropdown is wrong!"
    Case 2:	printlog " - Entry in Undo-dropdown is ok!"
    end select

    printlog "Working undo"
    '/// Edit / Undo 1 time -> Index should be restored
    EditUndo
    Sleep 1
    Call wNavigatorAuswahl ( 10, 1 )
    wait 500
    InsertIndexes

    wait 500
    Kontext
    try
        Active.Setpage TabVerzeichnisseVerzeichnis
    catch
        Warnlog "Index dialog has not been opened!"
        Call hCloseDocument
        goto endsub
    endcatch

    Kontext "TabVerzeichnisseVerzeichnis"
    if Verzeichnistitel.Gettext = "MyUndoRedoIndex" then
        printlog " - Index has been restored"
        TabVerzeichnisseVerzeichnis.Cancel
        '/// Check if Undo dropdown-Icon shows: 'Insert index'
        Select Case CheckUndoEntry ( 1, Compare2 )
        Case 0:	Warnlog " - Undo-dropdown is disabled!"
        Case 1:	Warnlog " - Entry in Undo-dropdown is wrong!"
        Case 2:	printlog " - Entry in Undo-dropdown is ok!"
        end select

        '/// Check if Redo dropdown-Icon shows: 'Remove index' after deleting index
        Select Case CheckRedoEntry ( 1, Compare3 )
        Case 0:	Warnlog " - Redo-dropdown is disabled!"
        Case 1:	Warnlog " - Entry in Redo-dropdown is wrong!"
        Case 2:	printlog " - Entry in Redo-dropdown is ok!"
        end select

        '/// Edit / Undo one more time
        '/// Check if Undo dropdown-Icon is disabled
        Select Case CheckUndoEntry ( 1, Compare2 )
        Case 0:	printlog " - Undo-dropdown is disabled!"
        Case 1:	Warnlog " - Entry in Undo-dropdown is enabled!"
        Case 2:	Warnlog " - Entry in Undo-dropdown is enabled!"
        end select

        '/// Check if Redo dropdown-Icon shows: 'Insert index' after Undo 2 times
        Select Case CheckRedoEntry ( 1, Compare2 )
        Case 0:	Warnlog " - Redo-dropdown is disabled!"
        Case 1:	Warnlog " - Entry in Redo-dropdown is wrong!"
        Case 2:	printlog " - Entry in Redo-dropdown is ok!"
        end select

    else
        Warnlog "Index has not been removed!"
        TabVerzeichnisseVerzeichnis.Cancel
    end if

    Call hCloseDocument
endcase
