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
'* Owner : oliver.craemer@oracle.com
'*
'* short description : Resource-test for the Chart Data Dialogue.
'*
'*******************************************************************
'*
' #1 tChartDataDialogueButtons
' #1 tChartDataDialogueFieldsUndo
'*
'\*******************************************************************

testcase tChartDataDialogueButtons
    '/// Create new impress document
    printlog "Create new impress document"   
    call hNewDocument
        Kontext "DocumentImpress"
        '/// Insert a new Chart-object, via the Menu: 'Insert - Chart'.
        Printlog "   Insert a new Chart-object, via the Menu: 'Insert - Chart'."
        InsertChart
        WaitSlot()
        Kontext "DocumentChart"
        '/// Open context menu
        printlog "Open context menu" 
        DocumentChart.OpenContextMenu
        '/// Select the third entry
        printlog "Select the third entry"
        hMenuSelectNr (6)
        sleep (1)
        Kontext "Diagramdata"
        call DialogTest ( Diagramdata )
        '/// Check if the diagram data dialogue came up
        printlog "Check if the diagram data dialogue came up"
        if Diagramdata.Exists then 
            printlog "   Diagramdata came up just as expected."
        else
            warnlog "   Diagramdata didnt come up as expected."
            call hCloseDocument
            goto endsub
        end if
        '/// Check if the 'Insert Row'-button is enabled.
        printlog "Check if the 'Insert Row'-button is enabled."
        if InsertRow.isEnabled then 
            printlog "   InsertRow was correctly enabled."
        else
            warnlog "   InsertRow wasnt enabled, but should have been."
        end if
        '/// Check if the 'Swap Columns'-button is disabled.
        printlog "Check if the 'Swap Columns'-button is disabled."
        if SwapColumns.isEnabled then 
            warnlog "   SwapColumns was enabled, but should have been disabled."
        else
            printlog "   SwapColumns was correctly disabled."
        end if
        '/// Check if the 'Swap Rows'-button is enabled.
        printlog "Check if the 'Swap Rows'-button is enabled."
        if SwapRows.isEnabled then 
            printlog "   SwapRows was correctly enabled."
        else
            warnlog "   SwapRows wasnt enabled, but should have been."
        end if
        '/// Check if the 'Delete Row'-button is enabled.
        printlog "Check if the 'Delete Row'-button is enabled."
        if DeleteRow.isEnabled then 
            printlog "   DeleteRow was correctly enabled."
        else
            warnlog "   DeleteRow wasnt enabled, but should have been."
        end if
        '/// Check if the 'Delete Column'-button is enabled.
        printlog "Check if the 'Delete Column'-button is enabled."
        if DeleteColumn.isEnabled then 
            warnlog "   DeleteColumn was enabled, but should have been disabled."
        else
            printlog "   DeleteColumn was correctly disabled."
        end if
        printlog "Check if the 'Insert Text Column'-button is enabled."
        if InsertTextColumn.isEnabled then
            printlog "   Insert Text Column was correctly enabled."
        else
            warnlog "   Insert Text Column wasnt enabled, but should have been."
        end if
        '/// Press 'TAB' to enter the Data-fields, followed by 'SHIFT TAB' end up in the first field.
        printlog "Press 'TAB' to enter the Data-fields, followed by 'SHIFT TAB' end up in the first field."
        Kontext "Diagramdata"
        Diagramdata.TypeKeys "<TAB>", true
        Diagramdata.TypeKeys "<SHIFT TAB>", true
        '/// Set the text in the marked field to '111'. Then move to the next field by pressing 'TAB'.
        printlog "   Set the text in the marked field to '111'. Then move to the next field by pressing 'TAB'."
        Tables.TypeKeys "111", true
        Diagramdata.TypeKeys "<TAB>", true
        '/// Set the text in the marked field to '222'. Then move to the next field by pressing 'TAB'.
        printlog "   Set the text in the marked field to '222'. Then move to the next field by pressing 'TAB'."
        Tables.TypeKeys "222", true
        Diagramdata.TypeKeys "<TAB>", true
        '/// Set the text in the marked field to '333'. Then move to the next field by pressing 'TAB'. ///'
        printlog "   Set the text in the marked field to '333'. Then move back to the first field by pressing 'SHIFT TAB' twice."
        Tables.TypeKeys "333", true
        'Do one more TAB, then three DOWNs, and check that the SwapColumn and SwapRow Icons are disabled.
        Tables.TypeKeys "<TAB>", true
        Tables.TypeKeys "<DOWN>", 3, true
        if SwapColumns.isEnabled then 
            warnlog "   SwapColumns was enabled, but should have been disabled."
        else
            printlog "   SwapColumns was correctly disabled."
        end if
        if SwapRows.isEnabled then 
            warnlog "   SwapRows should have been disabled, but was enabled."
        else
            printlog "   SwapRows was correctly disabled."
        end if
        '/// Return to the upper left corner.
        printlog "Return to the upper left corner."
        Tables.TypeKeys "<SHIFT TAB>", 3, true
        Tables.TypeKeys "<UP>", 3, true
        '/// Click the button 'Insert Row'.
        printlog "   Click the button 'Insert Row'."
        InsertRow.Click
        '/// Click the second button of the six in the upper left corner: Insert Column 
        printlog "   Click the button 'Insert Column'."
        InsertColumn.Click
        '/// Check if a new Column got inserted. And if it's unnamed.
        Tables.TypeKeys "<TAB>", true
        Tables.TypeKeys "<MOD1 C>", true
        if GetClipboard <> "" then 
            warnlog "   Inserting a new column in the DataTable didn't seem to work."
        endif
        Diagramdata.TypeKeys "<TAB>", 5, true
        if DataSeriesLabel.GetText <> "" then 
            warnlog "   Error when inserting a new Column: the Label for the Column doesn't appear to be clear. Was:" +DataSeriesLabel.GetText
        endif
        Tables.TypeKeys "<TAB>", true
        '/// Click the third button of the six in the upper left corner: 'Delete Row'.  ///'
        printlog "   Click the button 'Delete Row'."
        DeleteRow.Click
        Tables.TypeKeys "<SHIFT TAB>", true
        Tables.TypeKeys "<MOD1 C>", true
        if GetClipboard <> "" then
            warnlog "   Inserting a new column in the DataTable didn't seem to work."
        endif
        '/// Press 'TAB' three times to enter the column we wish to delete. ///'
        printlog "   Press 'TAB' three times to enter the column we wish to delete."
        Tables.TypeKeys "<TAB>", 3, true
        '/// Click the button 'Delete Column'.
        printlog "   Click the button 'Delete Column'."
        DeleteColumn.Click
        '/// Check if the Column got deleted.
        printlog "   Check if the Column got deleted."
        Tables.TypeKeys "<TAB>", 2, true
        Tables.TypeKeys "<MOD1 C>", true
        if GetClipboard <> "" then 
            warnlog "   Error when deleting a Column."
        end if
        Tables.TypeKeys "<SHIFT TAB>", 3, true
        Tables.TypeKeys "<MOD1 C>", true
        if GetClipboard = "2,4" OR GetClipboard = "2.4" then 
            printlog "   DeleteColumn worked as expected."                                
        else
            warnlog "   Error when deleting a Column."
        end if
        '/// Click the button 'Swap Columns'.
        printlog "   Click the button 'Swap Columns'."
        SwapColumns.Click
        Tables.TypeKeys "<SHIFT TAB>", true
        Tables.TypeKeys "<MOD1 C>", true
        if GetClipboard = "8,8" OR GetClipboard = "8.8" then 
            printlog "   SwapColumns worked as expected."
        else
            warnlog "   Error when Swapping a Column."
        end if
        '/// Click the button 'Swap Rows'.
        printlog "   Click the button 'Swap Rows'."
        SwapRows.Click
        '/// Check the result
        Tables.TypeKeys "<SHIFT TAB>", 4, true
        Tables.TypeKeys "<MOD1 C>", true
        if GetClipboard = "1,5" OR GetClipboard = "1.5" then 
            printlog "   SwapRows worked as expected."
        else
            warnlog "   Error when Swapping a Row."
        end if
        printlog "Insert a text column"
        InsertTextColumn.Click
        '/// Close the dialogue
        printlog "   Close the dialogue."
        Diagramdata.Close
        '/// End of test. Close the document.
        printlog "   End of test. Closing the document."
    Call hCloseDocument
endcase
'
'---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
'
testcase tChartDataDialogueFieldsUndo
    printlog "   tChartDataDialogueFieldsUndo starts"   
    Call hNewDocument
        Kontext "DocumentImpress"
        '/// Insert a new Chart-object, via the Menu: 'Insert - Chart'.
        printlog "   Insert a new Chart-object, via the Menu: 'Insert - Chart'."
        InsertChart
        WaitSlot()
        Kontext "DocumentChart"
        '/// Open context menu
        printlog "Open context menu"
        DocumentChart.OpenContextMenu
        '/// Select third entry
        printlog "Select third entry"
        hMenuSelectNr (6)
        sleep (1)
        Kontext "Diagramdata"
        call DialogTest ( Diagramdata )
        '/// Check if the dialogue came up.
        printlog "Check if the dialogue came up."
        if Diagramdata.Exists then 
            printlog "   Diagramdata came up just as expected."
        else
            warnlog "   Diagramdata didnt come up as expected. Please contact the test-administrator."
            call hCloseDocument
            goto endsub
        end if
        '/// Type the word 'Great' in the Label-textfield (for the Column).
        printlog "   Typing the word 'Great' in the Label-textfield (for the Column), and checking that it remained changed after leaving the field."
        DataSeriesLabel.SetText "Great"
        '/// Press 'TAB', followed by 'SHIFT TAB'. To jump from and back to the field. (and deselect and select it again).
        printlog "   Press 'TAB', followed by 'SHIFT TAB'. To jump from and back to the field. (and deselect and select it again)"
        Diagramdata.TypeKeys "<TAB>"
        Diagramdata.TypeKeys "<SHIFT TAB>"
        '/// Compare the written text with what the field now contains.
        printlog "   Compare the written text with what the field now contains."
        if DataSeriesLabel.GetText <> "Great" then 
            warnlog "   The field 'DataSeriesLabel' didn't get set. Should be 'Great', but was '" + DataSeriesLabel.GetText + "'."
        endif                                    
        '/// Inside the Tables: Press the 'TAB'-key seven times.
        printlog "   Inside the Tables: Press the 'TAB'-key seven times."
        Tables.TypeKeys "<TAB>", 7
        '/// Inside the Tables: Check if you can type the numbers '774433', followed by 'TAB', and '774411'.
        printlog "   Inside the Tables: Check if you can type the numbers '774433', followed by 'TAB', and '774411'."
        try
            Tables.TypeKeys "774433", true
            Tables.TypeKeys "<TAB>", true
            Tables.TypeKeys "774411", true
            Tables.TypeKeys "<SHIFT TAB>", true
            Tables.TypeKeys "<MOD1 C>", true
            if GetClipboard <> "774433" then 
                warnlog "   Writing the value '774433' in the DataTable didn't seem to work."
            endif
            Tables.TypeKeys "<TAB>", true
            Tables.TypeKeys "<MOD1 C>", true
            if GetClipboard <> "774411" then 
                warnlog "   Writing the value '774411' in the DataTable didn't seem to work."
            endif
        catch
            warnlog "   Either we're not at the right position, or the Table-fields doesn't become editable onto selection."
        endcatch
        '/// Close the dialogue
        printlog "   Close the dialogue."
        Diagramdata.Close
        '/// Try if we can Undo the changes with 'Edit - Undo'.
        printlog "Try if we can Undo the changes with 'Edit - Undo'."
        Kontext "DocumentImpress"
        DatabaseUndo
        '/// And then we enter the dialogue again, to check if the values has returned to what they were before we changed them.
        printlog "And then we enter the dialogue again, to check if the values has returned to what they were before we changed them."
        '/// Followed by one press on "RETURN" / "ENTER" to enter the Chart-Data-dialogue  ///'
        printlog "   Click the button 'Return', to enter the Chart-Data-dialogue."
        DocumentImpress.TypeKeys "<RETURN>"
        Kontext "DocumentChart"
        '/// Open context menu
        printlog "Open context menu"        
        DocumentChart.OpenContextMenu
        '/// Select third entry
        printlog "Select third entry"
        hMenuSelectNr (6)        
        sleep (1)
        Kontext "Diagramdata"
        '/// Inside the Tables: Press the 'TAB'-key 14 times.
        printlog "   Inside the Tables: Press the 'TAB'-key 14 times."
        Tables.TypeKeys "<TAB>", 14
        '/// Inside the Tables: Check if the values has returned to what they were before changing them.
        printlog "   Inside the Tables: Check if the values has returned to what they were before changing them."
        Tables.TypeKeys "<MOD1 C>", true
        if GetClipboard <> "774433" then
            if GetClipboard = "9,02" OR GetClipboard = "9.02" then
                printlog "   Edit-Undo worked fine."
            else
                warnlog "   Edit-Undo didn't seem to work on the values in the Dialogue."
            end if
        else
            warnlog "   Edit-Undo didn't work on the values in the Dialogue."
        end if
        '/// Check if the Label we earlier named 'Great', now has changed back.
        printlog "   Check if the Label we earlier named 'Great', now has changed back."
        if DataSeriesLabel.GetText = "Great" then 
            warnlog "   The field 'DataSeriesLabel' didn't change back."
        endif
        '/// Close the dialogue
        printlog "   Close the dialogue."
        Diagramdata.Close
        '/// End of test. Close the document.
        printlog "   End of test. Closing the document."
    Call hCloseDocument
endcase

