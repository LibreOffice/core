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
'* short description : Test Drop Down List Box
'*
'************************************************************************
'*
' #1 tDropDownLB_1           'Check 'Input List'
' #1 tDropDownLB_2           'All input field should be initially empty
' #1 tDropDownLB_3           'Test Add button
' #1 tDropDownLB_4           'Test Return in Item editbox
' #1 tDropDownLB_5           'Test new item will always be added at the end of 'Items on list'
' #1 tDropDownLB_6           'The first entry in the list box is the shown default
' #1 tDropDownLB_7           'Test 'Remove' button
' #1 tDropDownLB_8           'Test 'Move Up' button
' #1 tDropDownLB_9           'Test 'Move Down' button
' #1 tDropDownLB_10          'Test 'Ctrl+Shift+F9'
' #1 tDropDownLB_11          'OK button should insert new value
' #1 tDropDownLB_12          'Test Next button
' #1 tDropDownLB_13          'Test Edit button
' #1 tDropDownLB_14          'Test Edit/Fields
' #1 tDropDownLB_15          'Test Right arrow button
' #1 tDropDownLB_16          'Test Left arrow button
' #1 tDropDownLB_17          'Import from Word document
'*
'\***********************************************************************

testcase tDropDownLB_1

    Dim iType as Integer
    iType = 2

    PrintLog "- Check 'Input List'"

    Call hNewDocument

    PrintLog "- Insert/Field/Other/ Functions"
    PrintLog "- Check if 'Input List' exists in Type area"
    Call fInsertFieldsOther("TabFunktionen")
    Feldtyp.Select iType
    Sleep 3
    if Not EntrysList.IsVisible then
        Warnlog "Maybe wrong type selected in list. Not 'Input list'"
    end if
    TabFunktionen.Close

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_2

  Dim  iType  as Integer

  iType = 2

  PrintLog "- All input field should be initially empty"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Check if all input field should be initially empty"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3
        if Len(NewListEntry.GetText) <> 0 then Warnlog "Item isn't empty!"
        if EntrysList.GetItemCount   <> 0 then Warnlog "Items on list isn't empty!"
        if Len(EntryName.GetText)    <> 0 then Warnlog "Name isn't empty!"
    TabFunktionen.Close

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_3

  Dim iType as Integer
  Dim sItem as String

  iType = 2  : sItem = "Entry 1"

  PrintLog "- Test Add button"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input text in 'Item' , then click 'Add' button"
   PrintLog "- Check if the text is inserted into 'Items on List'"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3
        NewListEntry.SetText sItem
        Sleep 1
        AddListEntry.Click
        Sleep 1
        if EntrysList.GetSelText(1) <> sItem then Warnlog "the text isn't inserted into 'Items on list' correctly!"
    TabFunktionen.Close

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_4

  Dim iType as Integer
  Dim sItem as String

  iType = 2  : sItem = "Entry 1"

  PrintLog "- Test Pressing return in Item editbox"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input text in 'Item' , then click 'Add' button"
   PrintLog "- Check if the text is inserted into 'Items on List'"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3
        NewListEntry.SetText sItem
        Sleep 1
        NewListEntry.Typekeys "<Return>"
        Sleep 1
        if EntrysList.GetSelText(1) <> sItem then Warnlog "the text isn't inserted into 'Items on list' correctly!"
    TabFunktionen.Close

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_5

  Dim iType as Integer
  Dim sItem() as String
  Dim i as Integer
  Dim j as Integer

  iType = 2
  sItem = Array( _
                   "Entry 1"    , _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   "Entry 4"    , _
                   "Entry 5"    , _
                   "Entry 6"    , _
                   )
  j = UBound(sItem)

  PrintLog "- Test new item will always be added at the end of 'Items on list'"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input some items"
   PrintLog "- Check if new item will always be added at the end of 'Items on list'"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        For i = 0 to j
            NewListEntry.SetText sItem(i)
            Sleep 1
            AddListEntry.Click
            Sleep 1
        next i

        For i = 0 to j
            try
                EntrysList.Select i+1
                Sleep 1
            catch
                Warnlog "There should be 6 itmes in the Items on list!"
            endcatch
            if EntrysList.GetSelText <> sItem(i) then Warnlog "Hope to get " & sItem(i) & " but get " & EntrysList.GetSelText
        next i
    TabFunktionen.Close

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_6

  Dim iType as Integer
  Dim sItem() as String
  Dim i as Integer
  Dim j as Integer

  iType = 2
  sItem = Array( _
                   "Entry 1"    , _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   "Entry 4"    , _
                   "Entry 5"    , _
                   "Entry 6"    , _
                   )
  j = UBound(sItem)

  PrintLog "- The first entry in the list box is the shown default"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input some items"
   PrintLog "- Set focus to the middle Item"
   PrintLog "- click insert button , then Close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        For i = 0 to j
            NewListEntry.SetText sItem(i)
            Sleep 1
            AddListEntry.Click
            Sleep 1
        next i
        EntrysList.Select 3
        Sleep 1
        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

    EditSelectAll
    EditCopy

    if GetClipboardText <>  sItem(0) then  Warnlog "should get " &sItem(0) & " but get " & GetClipboardText

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_7

  Dim iType as Integer
  Dim sItem() as String
  Dim i as Integer
  Dim j as Integer

  iType = 2
  sItem = Array( _
                   "Entry 1"    , _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   "Entry 4"    , _
                 )
  j = UBound(sItem)

  PrintLog "- Test Remove button"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input some items"
   PrintLog "- Set focus to the middle Item"
   PrintLog "- click remove button"
   PrintLog "- check if the selected item is removed"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        try
            RemoveEntry.Click
            Warnlog "The remove button should be only ebabled when an item is selected!"
        catch
        endcatch

        For i = 0 to j
            NewListEntry.SetText sItem(i)
            Sleep 1
            AddListEntry.Click
            Sleep 1
        next i

        EntrysList.Select 3
        Sleep 1
        RemoveEntry.Click
        Sleep 1

        EntrysList.Select 1
        Sleep 1
        if EntrysList.GetSelText <> sItem(0) then Warnlog "Hope to get " & sItem(0) & " but get " & EntrysList.GetSelText

        EntrysList.Select 2
        Sleep 1
        if EntrysList.GetSelText <> sItem(1) then Warnlog "Hope to get " & sItem(1) & " but get " & EntrysList.GetSelText

        EntrysList.Select 3
        Sleep 1
        if EntrysList.GetSelText <> sItem(3) then Warnlog "Hope to get " & sItem(3) & " but get " & EntrysList.GetSelText

        try
            EntrysList.Select 4
            Warnlog "There should be only 3 itmes in the Items on list !"
        catch
        endcatch

    TabFunktionen.Close

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_8

  Dim iType as Integer
  Dim sItem() as String
  Dim sNewItem() as String
  Dim i as Integer
  Dim j as Integer

  iType = 2
  sItem = Array( _
                   "Entry 1"    , _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   "Entry 4"    , _
                 )
  sNewItem = Array( _
                   "Entry 4"    , _
                   "Entry 1"    , _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   )

  j = UBound(sItem)

  PrintLog "- Test 'Move Up' button"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input some items"
   PrintLog "- Set focus to the last Item,"
   PrintLog "- click Move Up button third times"
   PrintLog "- check if the 4th item is the 1st one"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        For i = 0 to j
            NewListEntry.SetText sItem(i)
            Sleep 1
            AddListEntry.Click
            Sleep 1
        next i

        EntrysList.Select 4
        Sleep 1
        MoveUpEntry.Click
        Sleep 1
        MoveUpEntry.Click
        Sleep 1
        MoveUpEntry.Click
        Sleep 1

        For i = 0 to j
            EntrysList.Select i+1
            Sleep 1
            if EntrysList.GetSelText <> sNewItem(i) then Warnlog "Hope to get " & sItem(i) & " but get " & EntrysList.GetSelText
        next i
    TabFunktionen.Close

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_9

  Dim iType as Integer
  Dim sItem() as String
  Dim sNewItem() as String
  Dim i as Integer
  Dim j as Integer

  iType = 2
  sItem = Array( _
                   "Entry 1"    , _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   "Entry 4"    , _
                 )
  sNewItem = Array( _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   "Entry 4"    , _
                   "Entry 1"    , _
                   )
  j = UBound(sItem)

  PrintLog "- Test 'Move Down' button"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input some items"
   PrintLog "- Set focus to the 1st Item,"
   PrintLog "- click Move Down button third times"
   PrintLog "- check if the 1st item is the 4th one"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        For i = 0 to j
            NewListEntry.SetText sItem(i)
            Sleep 1
            AddListEntry.Click
            Sleep 1
        next i

        EntrysList.Select 1
        Sleep 1
        MoveDownEntry.Click
        Sleep 1
        MoveDownEntry.Click
        Sleep 1
        MoveDownEntry.Click
        Sleep 1

        For i = 0 to j
            EntrysList.Select i+1
            Sleep 1
            if EntrysList.GetSelText <> sNewItem(i) then Warnlog "Hope to get " & sItem(i) & " but get " & EntrysList.GetSelText
        next i
    TabFunktionen.Close

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_10

  Dim iType as Integer
  Dim sItem as String

  iType = 2
  sItem = "Entry 1"

  PrintLog "- Test 'Ctrl+Shift+F9'"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input 1 item"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3
        NewListEntry.SetText sItem
        Sleep 1
        AddListEntry.Click
        Sleep 1
        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus in front of the list"
    Call wTypeKeys "<MOD1 Home>"

   PrintLog "- Press Ctrl+Shift+F9"
    Call wTypeKeys "<MOD1 Shift F9>"

    Kontext "EditInputList"
    if EditInputList.Exists then
        EditInputList.Cancel
    else
        Warnlog "The dialog can't be actived ."
    end if

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_11

  Dim iType as Integer
  Dim sItem1 as String
  Dim sItem2 as String

  iType  = 2
  sItem1 = "Entry 1" : sItem2 = "Entry 2"

  PrintLog "- OK button should insert new value"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input 2 items"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3
        NewListEntry.SetText sItem1
        Sleep 1
        AddListEntry.Click
        Sleep 1
        NewListEntry.SetText sItem2
        Sleep 1
        AddListEntry.Click
        Sleep 1

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus in front of the list"
    Call wTypeKeys "<MOD1 Home>"

   PrintLog "- Press Ctrl+Shift+F9"
    Call wTypeKeys "<MOD1 Shift F9>"

   PrintLog "- select 2nd list and press OK"
    Kontext "EditInputList"
      AllListEntrys.Select 2
      Sleep 1
    EditInputList.OK

   PrintLog "- Check if the result is right"
    EditSelectAll
    EditCopy

    if GetClipboardText <>  sItem2 then  Warnlog "should get " &sItem2 & " but get " & GetClipboardText

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_12

  Dim iType as Integer
  Dim sItem1() as String
  Dim sItem2() as String
  Dim i as Integer
  Dim j as Integer

  iType = 2
  sItem1 = Array( _
                   "Entry 1"    , _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   "Entry 4"    , _
                   )
  sItem2 = Array( _
                   "Entry A"    , _
                   "Entry B"    , _
                   "Entry C"    , _
                   "Entry D"    , _
                   )

  j = UBound(sItem1)

  PrintLog "- Test Next button"
  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input some items"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")

        Feldtyp.Select iType
        Sleep 3

        For i = 0 to j
            NewListEntry.SetText sItem1(i)
            Sleep 1
            AddListEntry.Click
            Sleep 1
        next i

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus to another line"
    Call wTypeKeys "<End><Return><Return>"

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input some other items"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")

        Feldtyp.Select iType
        Sleep 3

        For i = 0 to j
            NewListEntry.SetText sItem2(i)
            Sleep 1
            AddListEntry.Click
            Sleep 1
        next i

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus in front of the 1st list."
    Call wTypeKeys "<MOD1 Home>"

   PrintLog "- Press Ctrl+Shift+F9"
    Call wTypeKeys "<MOD1 Shift F9>"

   PrintLog "- the 1st group items should be in edit area."
    Kontext "EditInputList"
      For i = 0 to j
          AllListEntrys.Select i+1
          Sleep 1
          if AllListEntrys.GetSelText <> sItem1(i) then Warnlog "Hope to get " & sItem1(i) & " but get " & AllListEntrys.GetSelText
      next i

     PrintLog "- Press next button , the 2nd group items should be in edit area."
      NextListEntry.click
      Sleep 1

      For i = 0 to j
          AllListEntrys.Select i+1
          Sleep 1
          if AllListEntrys.GetSelText <> sItem2(i) then Warnlog "Hope to get " & sItem2(i) & " but get " & AllListEntrys.GetSelText
      next i

    EditInputList.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_13

  Dim iType as Integer
  Dim sItem1 as String
  Dim sItem2 as String

  iType  = 2
  sItem1 = "Entry 1" : sItem2 = "Entry A"

  PrintLog "- Test Edit button"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input a item"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        NewListEntry.SetText sItem1
        Sleep 1
        AddListEntry.Click
        Sleep 1

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus to another line"
    Call wTypeKeys "<End><Return><Return>"

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input another item"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        NewListEntry.SetText sItem2
        Sleep 1
        AddListEntry.Click
        Sleep 1

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus in front of the 1st list"
    Call wTypeKeys "<MOD1 Home>"

   PrintLog "- Press Ctrl+Shift+F9"
    Call wTypeKeys "<MOD1 Shift F9>"

   PrintLog "- Press Edit button , the edit/field dialog should be up"
    Kontext "EditInputList"
      EditListEntry.click
      Sleep 1

      Kontext "FeldbefehlBearbeitenFunktionen"
      if FeldbefehlBearbeitenFunktionen.Exists then
          EntrysList.Select 1
          if EntrysList.GetSelText <> sItem1 then
              Warnlog "Hope to get " & sItem1 & " but get " & EntrysList.GetSelText & "->Bug#115731"
          end if
          FeldbefehlBearbeitenFunktionen.Cancel
      else
          Warnlog "The dialog can't be actived ."
      end if

    Kontext "EditInputList"
    EditInputList.cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_14

  Dim iType as Integer
  Dim sItem() as String
  Dim sEntryName as String
  Dim i as Integer
  Dim j as Integer

  iType = 2
  sEntryName = "entryTest"
  sItem = Array( _
                   "Entry 1"    , _
                   "Entry 2"    , _
                   "Entry 3"    , _
                   "Entry 4"    , _
                   "Entry 5"    , _
                   "Entry 6"    , _
                 )
  j = UBound(sItem)

  PrintLog "- Test Edit/Fields"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input some items"
   PrintLog "- set Entry name"
   PrintLog "- click insert button , then Close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        For i = 0 to j
            NewListEntry.SetText sItem(i)
            Sleep 1
            AddListEntry.Click
            Sleep 1
        next i

        EntryName.SetText sEntryName
        Einfuegen.Click
        Sleep 1

    TabFunktionen.Close

   PrintLog "- Set focus in front of the 1st list"
    Call wTypeKeys "<Home>"

   PrintLog "- Edit/Fields , check if the entry items' info is correct."
    EditFields
    Kontext "FeldbefehlBearbeitenFunktionen"
      For i = 0 to j
          EntrysList.Select i+1
          Sleep 1
          if EntrysList.GetSelText <> sItem(i) then Warnlog "Hope to get " & sItem(i) & " but get " & EntrysList.GetSelText
      next i
      if EntryName.GetText <> sEntryName then  Warnlog "Entry name should be " & sEntryName & " but get " & EntryName.GetText
    FeldbefehlBearbeitenFunktionen.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_15

  Dim iType as Integer
  Dim sItem1 as String
  Dim sItem2 as String

  iType  = 2
  sItem1 = "Entry 1" : sItem2 = "Entry A"

  PrintLog "- Test Right arrow button"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input a item"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        NewListEntry.SetText sItem1
        Sleep 1
        AddListEntry.Click
        Sleep 1

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus to another line"
    Call wTypeKeys "<End><Return><Return>"

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input another item"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        NewListEntry.SetText sItem2
        Sleep 1
        AddListEntry.Click
        Sleep 1

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus in front of the 1st list."
    Call wTypeKeys "<MOD1 Home>"

   PrintLog "- Edit/Fields"
    EditFields

   PrintLog "- click Right arrow button"
    Kontext "FeldbefehlBearbeitenFunktionen"
      EntrysList.Select 1
      if EntrysList.GetSelText <> sItem1 then
          Warnlog "1.Hope to get " & sItem1 & " but get " & EntrysList.GetSelText
      end if
      NaechsterFeldbefehl.Click
      Sleep 1
      EntrysList.Select 1
      if EntrysList.GetSelText <> sItem2 then
          Warnlog "2.Hope to get " & sItem2 & " but get " & EntrysList.GetSelText
      end if
    FeldbefehlBearbeitenFunktionen.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_16

  Dim iType as Integer
  Dim sItem1 as String
  Dim sItem2 as String

  iType  = 2
  sItem1 = "Entry 1" : sItem2 = "Entry A"

  PrintLog "- Test Left arrow button"

  PrintLog "- Open a new document"
   Call hNewDocument

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input a item"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        NewListEntry.SetText sItem1
        Sleep 1
        AddListEntry.Click
        Sleep 1

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus to another line."
    Call wTypeKeys "<End><Return><Return>"

   PrintLog "- Insert/Field/Other/ Functions"
   PrintLog "- select Input list"
   PrintLog "- Input another item"
   PrintLog "- click insert and close the dialog"
    Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select iType
        Sleep 3

        NewListEntry.SetText sItem2
        Sleep 1
        AddListEntry.Click
        Sleep 1

        Einfuegen.Click
        Sleep 1
    TabFunktionen.Close

   PrintLog "- Set focus in front of the 2nd list."
    Call wTypeKeys "<Home>"

   PrintLog "- Edit/Fields"
    EditFields

   PrintLog "- click Left arrow button."
    Kontext "FeldbefehlBearbeitenFunktionen"
      EntrysList.Select 1
      if EntrysList.GetSelText <> sItem2 then
          Warnlog "1.Hope to get " & sItem2 & " but get " & EntrysList.GetSelText
      end if
      VorigerFeldbefehl.Click
      Sleep 1
      EntrysList.Select 1
      if EntrysList.GetSelText <> sItem1 then
          Warnlog "2.Hope to get " & sItem1 & " but get " & EntrysList.GetSelText
      end if
    FeldbefehlBearbeitenFunktionen.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tDropDownLB_17

    Dim sTestFile as String
    Dim sItem1() as String 
    Dim sItem2() as String
    Dim i as Integer
    Dim j as Integer
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\dropdownLB\DropDownListBox.doc")
    
    sItem1 = Array( _
           "Entry 1"    , _
           "Entry 2"    , _
           "Entry 3"    , _
         )
    sItem2 = Array( _
           "Entry A"    , _
           "Entry B"    , _
           "Entry C"    , _
         )
       
    PrintLog "- Import from Word document"
    PrintLog "- Open a new document"
    Call hNewDocument
    PrintLog "- Open a word document  which includes 2 entries."
    hFileOpenLocally( sTestFile )
    

    Kontext "EditInputList"
    if EditInputList.Exists then EditInputList.Cancel

    PrintLog "- Set focus in front of first line."
    Call wTypeKeys "<MOD1 Home>"
    PrintLog "- Edit/Fields."
    try
        EditFields
    catch
        Warnlog "Can't import word document!"
        Call hCloseDocument
        goto endsub
    endcatch
    
    PrintLog "- click Left arrow button"
    Kontext "FeldbefehlBearbeitenFunktionen"
    if EntrysList.GetItemCount = 3 then
        For i = 1 to EntrysList.GetItemCount
            EntrysList.Select i
            Sleep 1
            if EntrysList.GetSelText <> sItem1(i-1) then Warnlog "Hope to get " & sItem1(i-1) & " but get " & EntrysList.GetSelText
        next i
    else
        warnlog "Not all entrys in field correctly imported!"
    end if

    FeldbefehlBearbeitenFunktionen.Cancel
    
    PrintLog "- Set focus in front of third line"
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Down>" , 2
    Call wTypeKeys "<Home>"
    
    PrintLog "- Edit/Fields"
    try
        EditFields
    catch
        Warnlog "Can't import word document!"
        Call hCloseDocument
        goto endsub
    endcatch
    
    PrintLog "- click Left arrow button"
    Kontext "FeldbefehlBearbeitenFunktionen"
    if EntrysList.GetItemCount = 3 then
        For i = 1 to EntrysList.GetItemCount
            EntrysList.Select i
            Sleep 1
            if EntrysList.GetSelText <> sItem2(i-1) then Warnlog "Hope to get " & sItem2(i-1) & " but get " & EntrysList.GetSelText
        next i
    else
        warnlog "Not all entrys in field correctly imported!"
    end if

    FeldbefehlBearbeitenFunktionen.Cancel
    
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    
endcase

'-------------------------------------------------------------------------------------------

