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
'* short description : Test of the Redlinig function - 2
'*
'************************************************************************
'*
' #1 tAcceptReject_1           'Test Default settings
' #1 tAcceptReject_2           'Test Accept in  Edit / Changes / Accept or Reject
' #1 tAcceptReject_3           'Test Reject in  Edit / Changes / Accept or Reject
' #1 tAcceptReject_4           'Test Accept All in  Edit / Changes / Accept or Reject
' #1 tAcceptReject_5           'Test Reject All in  Edit / Changes / Accept or Reject
' #1 tAcceptReject_6           'Test filter changes by author
' #1 tAcceptReject_7           'Test filter changes by time
' #1 tAcceptReject_8           'Test filter changes by action
' #1 tAcceptReject_9           'Test filter changes by comment
' #1 tAcceptReject_10          'Adding a comment to a change over the context menu
' #1 tAcceptReject_11          'Sort the list of changes  over context menu -- action
' #1 tAcceptReject_12          'Sort the list of changes  over context menu -- author
' #1 tAcceptReject_13          'Sort the list of changes  over context menu -- date
' #1 tAcceptReject_14          'Sort the list of changes  over context menu -- comment
' #1 tAcceptReject_15          'Sort the list of changes  over context menu -- document position
'*
'\***********************************************************************

testcase tAcceptReject_1

  '/// Test focus -- the focus must be set in the document on the change pointed in the dialog.
  printlog "Test focus -- the focus must be set in the document on the change pointed in the dialog"

   '/// New document
   Call hNewDocument

   'Insert some test sentences
   Call wTypeKeys "Redlining test => OK_1<Return><Return>"
   Call wTypeKeys "Redlining test => OK_2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Multi operations : Insert a new sentence , and delete some test text
   Call wTypeKeys "<Up>"
   Call wTypeKeys "Focus Test"
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Backspace>" , 3

   '/// Set the focus to the Start
   Call wTypeKeys "<Up>" , 3
   Call wTypeKeys "<Home>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Sleep 2
   Kontext "Redlining"
   Sleep 2

   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 2

   '/// Select the 1st -- insertion
   ListItem.TypeKeys "<Up>"
   Sleep 2

   Kontext "DocumentWriter"
   Call wTypeKeys "<Shift Home>"
   EditCopy
   Sleep 1
   if GetClipboardText <> "Focus Test" then Warnlog "The inserted focus location is incorrect! Getting " +GetClipboardText

   Kontext "Redlining"
   '/// Select the 2nd -- deletion
   ListItem.TypeKeys "<Down>"
   Sleep 2

   Kontext "DocumentWriter"
   Call wTypeKeys "<End>"
   Call wTypeKeys "<Shift Left>" , 4
   Sleep 1
   EditCopy
   Sleep 1
   if GetClipboardText <> "O" then Warnlog "The deleted focus location is incorrect! Getting " +GetClipboardText

   '/// Close Redlining dialog
   Kontext "Redlining"
   Redlining.Close

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_2

  '/// Test Accept in  Edit / Changes / Accept or Reject
  printlog "Test Accept in  Edit / Changes / Accept or Reject "

   '/// New document
   Call hNewDocument

   'Insert some test sentences
   Call wTypeKeys "Redlining test => OK_1<Return><Return>"
   Call wTypeKeys "Redlining test => OK_2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Multi operations : Insert a new sentence , and
   '/// + delete some test text
   Call wTypeKeys "<Up>"
   Call wTypeKeys "Focus Test"
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Backspace>" , 1
   Call wTypeKeys "<Up>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Sleep 2
   Kontext "Redlining"
   Sleep 2
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 2
   '/// + Select 'Accept'
   Akzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close
   Sleep 2

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up>" , 3
   Sleep 1
   Call wTypeKeys "<Down><Home><Shift End>"
   Sleep 1
   EditCopy
   Sleep 1
   if GetClipboardText <> "Focus Test" then Warnlog "The inserted text is incorrect! Getting" + GetClipboardText

   Call wTypeKeys "<Down><End>"
   Sleep 1
   Call wTypeKeys "<Shift Left>" , 4
   Sleep 1
   EditCopy
   Sleep 1
   if GetClipboardText <> " OK_" then Warnlog "The deleted text is incorrect! Getting" + GetClipboardText

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_3

  '/// Test Reject in  Edit / Changes / Accept or Reject
  printlog "Test Reject in  Edit / Changes / Accept or Reject "

   '/// New document
   Call hNewDocument

   'Insert some test sentences
   Call wTypeKeys "Redlining test => OK_1<Return><Return>"
   Call wTypeKeys "Redlining test => OK_2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Multi operations : Insert a new sentence , and delete some test text
   Call wTypeKeys "<Up>"
   Call wTypeKeys "Focus Test"
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Backspace>" , 3
   Call wTypeKeys "<Up>" , 3
   Call wTypeKeys "<Home>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"

   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 1

   ListItem.TypeKeys "<Up>"
   Sleep 2

   '/// + Select 'Reject'
   Ablehnen.Click
   Sleep 2

   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up>" , 3
   Call wTypeKeys "<Down><Home><Shift End>"

   try
      EditCopy
      Warnlog "The inserted text should be rejected , so EditCopy shouldn't be actived!"
   catch
   endcatch

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 4
   EditCopy
   if GetClipboardText <> "O" then Warnlog "The deleted text should be deleted and their location should be still reserved! Getting " + GetClipboardText

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_4

  '/// Test Accept All in  Edit / Changes / Accept or Reject
  printlog "Test Accept All in  Edit / Changes / Accept or Reject "

   '/// + New document
   Call hNewDocument

   'Insert some test sentences
   Call wTypeKeys "Redlining test => OK_1<Return><Return>"
   Call wTypeKeys "Redlining test => OK_2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Multi operations : Insert a new sentence , and delete some test text
   Call wTypeKeys "<Up>"
   Sleep 1
   Call wTypeKeys "Focus Test"
   Sleep 1
   Call wTypeKeys "<Down><End>"
   Sleep 1
   Call wTypeKeys "<Backspace>" , 3
   Sleep 2
   Call wTypeKeys "<Up>" , 3
   Sleep 2
   Call wTypeKeys "<Home>"
   Sleep 1

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   Sleep 1
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 1
   '/// + Select 'Accept All'
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close
   Sleep 1

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up>" , 3
   Sleep 1
   Call wTypeKeys "<Down><Home><Shift End>"
   Sleep 1
   EditCopy
   Sleep 1
   if GetClipboardText <> "Focus Test" then Warnlog "The inserted text is incorrect! Getting " + GetClipboardText

   Call wTypeKeys "<Down><End>"
   Sleep 2
   Call wTypeKeys "<Shift Left>" , 4
   Sleep 2
   EditCopy
   if GetClipboardText <> "=> O" then Warnlog "The deleted text is incorrect! Getting " + GetClipboardText

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_5

  '/// Test Reject all in  Edit / Changes / Accept or Reject
  printlog "Test Reject all in  Edit / Changes / Accept or Reject "

   '/// New document
   Call hNewDocument

   'Insert some test sentences
   Call wTypeKeys "Redlining test => OK_1<Return><Return>"
   Call wTypeKeys "Redlining test => OK_2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Multi operations : Insert a new sentence , and delete some test text
   Call wTypeKeys "<Up>"
   Call wTypeKeys "Focus Test"
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Backspace>" , 3
   wait 500
   Call wTypeKeys "<Up>" , 3
   wait 500
   Call wTypeKeys "<Home>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 1
   '/// + Select 'Reject All'
   AlleAblehnen.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up>" , 3
   wait 500
   Call wTypeKeys "<Down><Home><Shift End>"
   wait 500
   try
      EditCopy
      Warnlog "The inserted text should be rejected , so EditCopy shouldn't be actived!"
   catch
   endcatch

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 4
   EditCopy
   if GetClipboardText <> "OK_2" then Warnlog "The deleted text should not be deleted! Getting " + GetClipboardText

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_6

  Dim sFirstName  , sLastName  as String
  Dim sFirstName1 , sLastName1 as String
  Dim sFirstName2 , sLastName2 as String

  sFirstName1 = "Test1" : sLastName1 = "Test1"
  sFirstName2 = "Test2" : sLastName2 = "Test2"

  '/// Test filter changes by author
  printlog "Test filter changes by author"

   '/// New document
   Call hNewDocument

   'Insert some test sentences
   Call wTypeKeys "Redlining test => OK_1<Return>"
   Call wTypeKeys "Redlining test => OK_2"

   '/// Change author to author1 :Test1 Test1
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
   sFirstName = VorName.GetText
   sLastName  = ZuName.GetText
   VorName.SetText sFirstName1
   ZuName.SetText  sLastName1
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// author1 : delete some text
   Call wTypeKeys "<Up><End>"
   Sleep 1
   Call wTypeKeys "<Backspace>" , 3
   Sleep 1

   '/// Change author to author2 :Test2 Test2
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
   VorName.SetText sFirstName2
   ZuName.SetText  sLastName2
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   '/// author2 : delete some other text
   Call wTypeKeys "<Down><End>"
   Sleep 1
   Call wTypeKeys "<Backspace>" , 4
   Sleep 1

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'Filter' ///
   TabControl.SetPage TabFilter
   Sleep 1
   '/// + Check 'Author'
   Autor.Check
   Sleep 2
   '/// + Select Author2
   	try
   		AutoListe.Select 2
    catch
    	Warnlog "Change of author (User name) disregarded when changed during Office session"
		Redlining.Close
		Call hCloseDocument
		goto endsub
	endcatch
	Sleep 2
	'/// + Select tabpage 'List' ///
	TabControl.SetPage TabListe
	Sleep 2
	'/// + Select 'Accept All'///
	AlleAkzeptieren.Click
	Sleep 2

   '/// + Close Redlining dialog
   Redlining.Close
   Sleep 1

   '/// + Check if Filter works well
   Call wTypeKeys "<Up>" , 2
   Sleep 1
   Call wTypeKeys "<End>"
   Sleep 1
   Call wTypeKeys "<Shift Left>" , 6
   Sleep 1
   EditCopy
   Sleep 1
   if GetClipboardText <>  "> O" then Warnlog "The deleted text by author1 is incorrect! Getting " + GetClipboardText

   Call wTypeKeys "<Down><End>"
   Sleep 1
   Call wTypeKeys "<Shift Left>" , 6
   Sleep 1
   EditCopy
   Sleep 1
   if GetClipboardText <> "st => " then Warnlog "The deleted text by author2 is incorrect! Getting " + GetClipboardText

   'Change author to default
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
   VorName.SetText sFirstName
   ZuName.SetText  sLastName
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   '/// + Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_7

  Dim sTime as String

  '/// Test filter changes by date
  printlog "Test filter changes by date"

   '/// New document
   Call hNewDocument

   '/// Insert some test sentences
   Call wTypeKeys "Redlining test => OK_1<Return>"
   Call wTypeKeys "Redlining test => OK_2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// delete some text in first line
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Backspace>" , 3

   sTime = Str ( Format( Now  ) )
   sTime = Mid ( sTime , 12 , 5 )

   '/// wait 70 seconds
   Sleep 70

   '/// delete some text in second line
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Backspace>" , 2

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'Filter' ///
   TabControl.SetPage TabFilter
   Sleep 1
   '/// + Check 'Date'
   Datum.Check
   Sleep 1
   '/// + Select earlier than
   DatumListe.Select 1
   Sleep 1
   '/// + insert time
   Zeit1.SetText sTime
   Sleep 1
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 1
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// + Check if Filter works well
   Call wTypeKeys "<Up>" , 2
   Call wTypeKeys "<End>"
   Call wTypeKeys "<Shift Left>" , 4
   EditCopy
   if GetClipboardText <>  "=> O" then Warnlog "The deleted text in first line is incorrect!"

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 4
   EditCopy
   if GetClipboardText <> "OK" then Warnlog "The deleted text in second line is incorrect!"

   '/// + Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_8

	Dim iComparison1$,iComparison2$,iComparison3$,iComparison4$
	
	'/// Test filter changes by action
	printlog "Test filter changes by action"
	
	'/// New document
	Call hNewDocument
	
	'Insert some test sentences
	Call wTypeKeys "Redlining test => OK_1<Return>"
	Call wTypeKeys "Redlining test => OK_2"
	
	UseBindings
	'/// Edit / Changes / Record
	EditChangesRecord
	
	'/// Change the first line's attribute .
	Call wTypeKeys "<Up><Home><Shift End>"
	
	FormatCharacter
	Kontext
	Active.SetPage TabFont
	kontext "TabFont"
	
	if gAsianSup = True then
		FontWest.Select  5
		iComparison1$ = FontWest.GetSelText
		try
            StyleWest.Select 3
        catch
            StyleWest.Select StyleWest.GetItemCount
        endcatch
		iComparison2$ = StyleWest.GetSelText
		SizeWest.Select  5
		iComparison3$ = SizeWest.GetSelText
	else
		Font.Select  5
		iComparison1$ = Font.GetSelText
		try
            Style.Select 3
        catch
            Style.Select Style.GetItemCount
        endcatch
		iComparison2$ = Style.GetSelText
		Size.Select  5
		iComparison3$ = Size.GetSelText
	end if
	
	wait 100
	Kontext
	Active.Setpage TabHyperlinkZeichen
	Kontext "TabHyperlinkZeichen"
	UrlName.Settext "http://www.sun.com/"
	Sleep 1
	TargetFrame.Select 2
	Sleep 1
	iComparison4$ = TargetFrame.GetSelText
	Sleep 1
	TabHyperlinkZeichen.Ok
	
	'/// Delete some text in second line
	Call wTypeKeys "<Down><End>"
	Call wTypeKeys "<Backspace>" , 3
	
	'/// Edit / Changes / Accept or Reject
	EditChangesAcceptOrReject
	Kontext "Redlining"
	'/// + Select tabpage 'Filter' ///
	TabControl.SetPage TabFilter
	Sleep 1
	'/// + Check 'Action' , select format
	Aktion.Check
	Sleep 1
	Aktionsliste.Select 3
	Sleep 1
	'/// + Select tabpage 'List' ///
	TabControl.SetPage TabListe
	Sleep 1
	'/// + Select 'Accept All'///
	AlleAkzeptieren.Click
	Sleep 2
	'/// + Close Redlining dialog
	Redlining.Close
	
	'/// Check if Filter works well
	Call wTypeKeys "<Up><Home><Right>"
	FormatCharacter
	Kontext
	Active.SetPage TabFont
	Kontext "TabFont"
	if gAsianSup = True then
		if iComparison1$ <> FontWest.GetSelText  then Warnlog "Wrong Font in first line!"
		if iComparison2$ <> StyleWest.GetSelText then Warnlog "Wrong Fontstyle in first line!"
		if iComparison3$ <> SizeWest.GetSelText  then QAErrorlog "#108881#Wrong Fontsize in first line!"
	else
		if iComparison1$ <> Font.GetSelText  then Warnlog "Wrong Font in first line!"
		if iComparison2$ <> Style.GetSelText then Warnlog "Wrong Fontstyle in first line!"
		if iComparison3$ <> Size.GetSelText  then Warnlog "Wrong Fontsize in first line!"
	end if
	wait 100
	Kontext
	Active.Setpage TabHyperlinkZeichen
	Kontext "TabHyperlinkZeichen"
	if UrlName.Gettext <> "http://www.sun.com/"  then Warnlog "Wrong hyperlink has been added to text!"
	if iComparison4$   <> TargetFrame.GetSelText then Warnlog "Wrong targetFrame has been added to text!"
	TabHyperlinkZeichen.Cancel
	
	Call wTypeKeys "<Down><End>"
	Call wTypeKeys "<Shift Left>" , 4
	EditCopy
	if GetClipboardText <> "O" then Warnlog "The deleted text is incorrect!"
	
	'/// Close the document
	Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_9

  '/// Test filter changes by comments
  printlog "Test filter changes by comments"

   '/// New document
   Call hNewDocument

   '/// Insert some test sentences
   Call wTypeKeys "Redlining test => OK_1<Return>"
   Call wTypeKeys "Redlining test => OK_2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// delete some text in first line .
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Backspace>" , 3

   '/// insert comment1: Edit / Changes / Comment
   EditChangesComment
   Kontext "Kommentar"
    KommentarText.SetText "Comment1"
   Kommentar.OK

   '/// delete some text in second line .
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Backspace>" , 2

   '/// insert comment2: Edit / Changes / Comment
   EditChangesComment
   Kontext "Kommentar"
    KommentarText.SetText "Comment2"
   Kommentar.OK

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'Filter' ///
   TabControl.SetPage TabFilter
   Sleep 1
   '/// + Check 'comment' , and input comment1 in the relevant editbox
   Comment.Check
   CommentText.SetText "Comment1"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 1
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 1
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Shift Left>" , 4
   EditCopy
   if GetClipboardText <>  "=> O" then Warnlog "The deleted text in first line is incorrect!"

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 4
   EditCopy
   if GetClipboardText <> "OK" then Warnlog "The deleted text in second line is incorrect!"

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_10

  '/// Adding a comment to a change over the context menu.
  printlog "Adding a comment to a change over the context menu."

   '/// New document
   Call hNewDocument

   '/// Insert some test sentences
   Call wTypeKeys "Redlining test => O1<Return>"
   Call wTypeKeys "Redlining test => K2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// delete some text in first line .
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Backspace>"

   '/// delete some text in second line .
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Backspace>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"

   '/// + Select tabpage 'List'
   TabControl.SetPage TabListe
   Sleep 1

   '/// + insert comment with context menu
   '/// + insert comment for first line
   ListItem.TypeKeys "<Up>"
   ListItem.OpenContextMenu
   hMenuSelectNr(1)
   Sleep 1

   Kontext "NotizBearbeiten"
    Texteingabe.SetText "Comment1"
   NotizBearbeiten.OK

   '/// + insert comment for second line
   Kontext "Redlining"
   ListItem.TypeKeys "<Down>"
   ListItem.OpenContextMenu
   hMenuSelectNr(1)

   Kontext "NotizBearbeiten"
    Texteingabe.SetText "Comment2"
   NotizBearbeiten.OK

   Kontext "Redlining"

   '/// + Select tabpage 'Filter' ///
   TabControl.SetPage TabFilter
   Sleep 1
   '/// + Check 'comment' , and input comment1 in the relevant editbox
   Comment.Check
   CommentText.SetText "Comment1"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 1
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 1
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <>  " O" then Warnlog "The deleted text in first line is incorrect!"

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <> "K" then Warnlog "The deleted text in second line is incorrect!"

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_11

    '/// Sort the list of changes  over context menu -- action.
    '/// Action sort depends on not only StarOffice/StarSuite , but also system language.
    printlog "Sort the list of changes  over context menu -- action."
    
    '/// New document
    Call hNewDocument
    
    '/// Insert some test sentences
    Call wTypeKeys "Redlining_11_test<Return>"
    
    UseBindings
    '/// Edit / Changes / Record
    EditChangesRecord
    
    '/// Insert some test sentences
    Call wTypeKeys "A"
    
    '/// delete some text in first line .
    Call wTypeKeys "<Up><End>"
    Call wTypeKeys "<Backspace>"
    
    'This step to ensure the 1st list is deletion , the 2nd list is insertion
    Call wTypeKeys "<Home>"
    
    '/// Edit / Changes / Accept or Reject
    EditChangesAcceptOrReject
    Kontext "Redlining"
    
    '/// + Select tabpage 'List'
    TabControl.SetPage TabListe
    Sleep 1
    
    'Sort the list of changes  with their position
    ListItem.TypeKeys "<Up>"
    ListItem.OpenContextMenu
    Call hMenuSelectNr(2)
    Sleep 1
    Call hMenuSelectNr(5)
    Sleep 1
    
    '/// + Sort the list of changes  over context menu -- action
    ListItem.TypeKeys "<Up>"
    ListItem.OpenContextMenu
    Call hMenuSelectNr(2)
    Sleep 1
    Call hMenuSelectNr(1)
    Sleep 1   
    
    '/// + Click accept
    Akzeptieren.Click
    Sleep 1
    
    '/// + Close Redlining dialog
    Redlining.Close
    
    '/// Check if the configuration is all right
    Call wTypeKeys "<Up><End>"
    Call wTypeKeys "<Shift Left>"
    EditCopy
    if GetClipboardText <> "s" then Warnlog "The deleted text in first line is incorrect!"
    
    Call wTypeKeys "<Down><End>"
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> "A" then Warnlog "The inserted text in second line is incorrect!"
    
    Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_12

  Dim sFirstName  , sLastName  as String
  Dim sFirstName1 , sLastName1 as String
  Dim sFirstName2 , sLastName2 as String

  sFirstName1 = "ATest1" : sLastName1 = "ATest1"
  sFirstName2 = "BTest2" : sLastName2 = "BTest2"

  '/// Sort the list of changes  over context menu -- author.
  printlog "Sort the list of changes  over context menu -- author."

   '/// New document
   Call hNewDocument

   'Insert some test sentences
   Call wTypeKeys "Redlining test => O1<Return>"
   Call wTypeKeys "Redlining test => K2"

   '/// Change author to author1 :ATest1 ATest1
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
    sFirstName = VorName.GetText
    sLastName  = ZuName.GetText
    VorName.SetText sFirstName1
    ZuName.SetText  sLastName1
    Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// author1 : delete some text in 2nd line
   Call wTypeKeys "<Backspace>"

   '/// Change author to author2 :BTest2 BTest2
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
    VorName.SetText sFirstName2
    ZuName.SetText  sLastName2
    Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   '/// author2 : delete some text in 1st line
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Backspace>"
   Sleep 1
   Call wTypeKeys "<Home>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"

   '/// + Select tabpage 'List'
   TabControl.SetPage TabListe
   Sleep 1

   'Sort the list of changes  with their position
   ListItem.TypeKeys "<Up>"
   ListItem.OpenContextMenu
   hMenuSelectNr(2)
   Sleep 1
   hMenuSelectNr(5)
   Sleep 1

   '/// + Sort the list of changes  over context menu  - author
   ListItem.TypeKeys "<Up>"
   ListItem.OpenContextMenu
   hMenuSelectNr(2)
   Sleep 1
   hMenuSelectNr(2)
   Sleep 1

   '/// + Select author 1
   ListItem.TypeKeys "<Up>"
   Sleep 1

   '/// + Click accept
   Akzeptieren.Click
   Sleep 1

   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if Filter works well
   Call wTypeKeys "<Up>" , 2
   Call wTypeKeys "<End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <>  "O" then Warnlog "The deleted text by author1 is incorrect!"

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <> " K" then Warnlog "The deleted text by author2 is incorrect!"

   'Change author to default
   ToolsOptions
    Call hToolsOptions("STAROFFICE","USERDATA")
    VorName.SetText sFirstName
    ZuName.SetText  sLastName
    Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   '/// + Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_13

  '/// Sort the list of changes  over context menu -- date.
  printlog "Sort the list of changes  over context menu -- date."

   '/// New document
   Call hNewDocument

   '/// Insert some test sentences
   Call wTypeKeys "Redlining test => O1<Return>"
   Call wTypeKeys "Redlining test => K2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// delete some text in 2nd line .
   Call wTypeKeys "<Backspace>"

   '/// wait 70 seconds
   Sleep 70

   '/// delete some text in 1st line .
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Backspace>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"

   '/// + Select tabpage 'List'
   TabControl.SetPage TabListe
   Sleep 1

   'Sort the list of changes  with their position
   ListItem.TypeKeys "<Up>"
   ListItem.OpenContextMenu
   hMenuSelectNr(2)
   Sleep 1
   hMenuSelectNr(5)
   Sleep 1

   '/// + Sort the list of changes  over context menu -- date
   ListItem.TypeKeys "<Up>"
   ListItem.OpenContextMenu
   hMenuSelectNr(2)
   Sleep 1
   hMenuSelectNr(3)
   Sleep 1

   '/// + Select 2nd item (it should be the 2nd deletion operation)
   ListItem.TypeKeys "<Down>"

   '/// + Click accept
   Akzeptieren.Click
   Sleep 1

   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <> " O" then Warnlog "The deleted text in first line is incorrect!"

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <>  "K" then Warnlog "The deleted text in second line is incorrect!"

   '/// + Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_14

  '/// Sort the list of changes  over context menu -- comment.
  printlog "Sort the list of changes  over context menu -- comment"

   '/// New document
   Call hNewDocument

   '/// Insert some test sentences
   Call wTypeKeys "Redlining test => O1<Return>"
   Call wTypeKeys "Redlining test => K2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// delete some text in 2nd line .
   Call wTypeKeys "<Backspace>"

   '/// insert comment1: Edit / Changes / Comment
   EditChangesComment
    Kontext "Kommentar"
    KommentarText.SetText "Comment1"
   Kommentar.OK

   '/// delete some text in 1st line .
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Backspace>"

   '/// insert comment2: Edit / Changes / Comment
   EditChangesComment
    Kontext "Kommentar"
    KommentarText.SetText "Comment2"
   Kommentar.OK

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"

   '/// + Select tabpage 'List'
   TabControl.SetPage TabListe
   Sleep 1

   'Sort the list of changes  with their position
   ListItem.TypeKeys "<Up>"
   ListItem.OpenContextMenu
   hMenuSelectNr(2)
   Sleep 1
   hMenuSelectNr(5)
   Sleep 1

   '/// + Sort the list of changes  over context menu -- comment
   ListItem.TypeKeys "<Up>"
   ListItem.OpenContextMenu
   hMenuSelectNr(2)
   Sleep 1
   hMenuSelectNr(4)
   Sleep 1

   '/// + Select 2nd item
   ListItem.TypeKeys "<Down>"

   '/// + Click accept
   Akzeptieren.Click
   Sleep 1

   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <>  " O" then Warnlog "The deleted text in first line is incorrect!"

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <>  "K" then Warnlog "The deleted text in second line is incorrect!"

   '/// + Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tAcceptReject_15

  '/// Sort the list of changes  over context menu -- document position.
  printlog "Sort the list of changes  over context menu -- document position"

   '/// New document
   Call hNewDocument

   '/// Insert some test sentences
   Call wTypeKeys "Redlining test => O1<Return>"
   Call wTypeKeys "Redlining test => K2"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// delete some text in 2nd line .
   Call wTypeKeys "<Backspace>"

   '/// delete some text in 1st line .
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Backspace>"

   Call wTypeKeys "<Home>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"

   '/// + Select tabpage 'List'
   TabControl.SetPage TabListe
   Sleep 1

   '/// + Sort the list of changes  over context menu -- document position
   ListItem.TypeKeys "<Up>"
   ListItem.OpenContextMenu
   hMenuSelectNr(2)
   Sleep 1
   hMenuSelectNr(5)
   Sleep 1

   '/// + Select 2nd item
   ListItem.TypeKeys "<Down>"
   Sleep 1

   '/// + Click accept
   Akzeptieren.Click
   Sleep 1

   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the configuration is all right
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <>  "O" then Warnlog "The deleted text in first line is incorrect!"

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>" , 2
   EditCopy
   if GetClipboardText <>  " K" then Warnlog "The deleted text in second line is incorrect!"

   '/// Close the document
   Call hCloseDocument

endcase
