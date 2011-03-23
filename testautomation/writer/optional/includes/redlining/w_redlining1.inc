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
'* short description : Test of the Redlinig function - 1
'*
'************************************************************************
'*
' #1 tRedlining_1      'Test Default settings
' #1 tRedlining_2      'Inserted text : from Typing
' #1 tRedlining_3      'Inserted text : from Clipboard
' #1 tRedlining_4      'Inserted text : from another file
' #1 tRedlining_5      'Inserted text : from Autotext
' #1 tRedlining_6      'Deleteed text
' #1 tRedlining_7      'sorted text
' #1 tRedlining_8      'Searched and replaced text
' #1 tRedlining_9      'text attributs
' #1 tRedlining_10     'Inserted Section
' #1 tRedlining_11     'Inserted Table
' #1 tRedlining_12     'Inserted field (Date)
' #1 tRedlining_13     'Inserted footnotes
' #1 tRedlining_14     'Inserted headers
' #1 tRedlining_15     'Inserted footers
' #1 tRedlining_16     'moved paragraphs/chapters
'*
'\***********************************************************************

testcase tRedlining_1

  Dim iInsertionAttr as Integer
  Dim iInsertionColor as Integer
  Dim iDeletionAttr as Integer
  Dim iDeletionColor as Integer
  Dim iChangedAttr as Integer
  Dim iChangedColor as Integer
  Dim iLineChangedMark as Integer
  Dim iLineChangedColor as Integer

  iInsertionAttr   = 4  :  iInsertionColor   = 2
  iDeletionAttr    = 4  :  iDeletionColor    = 2
  iChangedAttr     = 2  :  iChangedColor     = 2
  iLineChangedMark = 2  :  iLineChangedColor = 1

  Call hNewDocument

  PrintLog "- Test Default settings"
  '/// Test Default settings

  '/// The default setting for Redlining is Off
   Call wTypeKeys "Redlining test"
   Call wTypeKeys "<Backspace>",2
   Sleep 1
   Call wTypeKeys "<End>"
   Call wTypeKeys "<Shift Left>" , 4
   EditCopy
   if GetClipboardText <> "g te" then Warnlog "The default setting for Redlining is NO Off"

  '/// Tools / Options / Textdocument / Changes
   ToolsOptions
   Call hToolsOptions("WRITER","CHANGES")

  '/// Check default settings

  '/// 1) Insertion Attributes , Insertion color
   if EinfuegenAttribute.GetSelIndex <> iInsertionAttr  then
      warnlog "The default settings for Insertion Attribute isn't correct, should be " +  EinfuegenAttribute.GetItemText(iInsertionAttr) + " but get " +EinfuegenAttribute.GetItemText(EinfuegenAttribute.GetSelIndex)
   end if
   if EinfuegenFarbe.GetSelIndex <> iInsertionColor  then
      warnlog "The default settings for Insertion color isn't correct, should be " +  EinfuegenFarbe.GetItemText(iInsertionColor) + " but get " +EinfuegenFarbe.GetItemText(EinfuegenFarbe.GetSelIndex)
   end if

  '/// 2) Deletion Attribute , Deletion color
   if LoeschenAttribute.GetSelIndex <> iDeletionAttr  then
      warnlog "The default settings for Deletion Attribute isn't correct, should be " +  LoeschenAttribute.GetItemText(iDeletionAttr) + " but get " +LoeschenAttribute.GetItemText(LoeschenAttribute.GetSelIndex)
   end if
   if LoeschenFarbe.GetSelIndex <> iDeletionColor  then
      warnlog "The default settings for Deletion color isn't correct, should be " +  LoeschenFarbe.GetItemText(iDeletionColor) + " but get " +LoeschenFarbe.GetItemText(LoeschenFarbe.GetSelIndex)
   end if

  '/// 3) Changed Attribute  , Changed  color
   if AendernAttribute.GetSelIndex <> iChangedAttr  then
      warnlog "The default settings for Changed Attribute isn't correct, should be " +  AendernAttribute.GetItemText(iChangedAttr) + " but get " +AendernAttribute.GetItemText(AendernAttribute.GetSelIndex)
   end if
   if AendernFarbe.GetSelIndex <> iChangedColor  then
      warnlog "The default settings for Changed color isn't correct, should be " +  AendernFarbe.GetItemText(iChangedColor) + " but get " +AendernFarbe.GetItemText(AendernFarbe.GetSelIndex)
   end if

  '/// 4) Lines changed Mark  , Lines changed color
   if ZeilenMarkierung.GetSelIndex <> iLineChangedMark  then
      warnlog "The default settings for Lines Changed Mark isn't correct, should be " +  ZeilenMarkierung.GetItemText(iLineChangedMark) + " but get " +ZeilenMarkierung.GetItemText(ZeilenMarkierung.GetSelIndex)
   end if
   if ZeilenFarbe.GetSelIndex <> iLineChangedColor  then
      warnlog "The default settings for Lines Changed color isn't correct, should be " +  ZeilenFarbe.GetItemText(iLineChangedColor) + " but get " +ZeilenFarbe.GetItemText(ZeilenFarbe.GetSelIndex)
   end if

  '/// Close the tools/option dialogue
    Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tRedlining_2

  '/// Test Inserted text : from Typing
  printlog "Inserted text : from Typing"

   '/// New document
   Call hNewDocument

   '/// Insert any sentence into document
   Call wTypeKeys "Redlining test => OK_1<Return>"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Insert some new sentence
   Call wTypeKeys "Redlining test => OK_2<Return>"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the changes have been accepted
   Call wTypeKeys "<Up>", 2
   Call wTypeKeys "<Down>"
   Call wTypeKeys "<End>"
   Call wTypeKeys "<Shift Left>"
   EditCopy
   Sleep 1
   if GetClipboardText <> "2" then Warnlog "The 2nd line is incorrect!"

   '/// Close the document
   Call hCloseDocument

endcase

'--------------------------------------------------------------------------

testcase tRedlining_3

  '/// Test Inserted text : from Clipboard
  printlog "Inserted text : from Clipboard"

   '/// New document
   Call hNewDocument

   '/// Insert any sentence into document , and copy it to clipboard
   Call wTypeKeys "Redlining test => OK_2<Return>"
   EditSelectAll
   EditCopy
   Sleep 1

   '/// Insert a new sentence into document
   Call wTypeKeys "Redlining test => OK_1<Return>"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// copy the clipboard into the document
   EditPaste
   Sleep 1

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the changes have been accepted
   Call wTypeKeys "<Up>", 2
   Call wTypeKeys "<Down>"
   Call wTypeKeys "<End>"
   Call wTypeKeys "<Shift Left>"
   EditCopy
   Sleep 1
   if GetClipboardText <> "2" then Warnlog "The 2nd Line is incorrect!"

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tRedlining_4

  '/// Test Inserted text : from another file
  printlog "Inserted text : from another file"

   '/// New document
   Call hNewDocument

   '/// Insert a sentence into document
   Call wTypeKeys "Redlining test => OK_1<Return>"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// insert a file - Insert/File
   InsertFileWriter
   Kontext "OeffnenDlg"
   Dateiname.Settext Convertpath (gTesttoolpath + "writer\optional\input\redlining\tRedlining_4.sxw")
   Oeffnen.Click
   Sleep 2

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the changes have been accepted
   Call wTypeKeys "<Up>", 2
   Call wTypeKeys "<Down>"
   Call wTypeKeys "<End>"
   Call wTypeKeys "<Shift Left>"
   EditCopy
   Sleep 1
   if GetClipboardText <> "2" then Warnlog "i35222 - The 2nd Line is incorrect!"

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tRedlining_5

    Dim j as Integer
    Dim iAll as Integer

    '/// Test Inserted text : from Autotext
    printlog "Inserted text : from Autotext"

    '/// New document
    Call hNewDocument

    'insert a category
    Call wCreateAutotextCategory("1-MyTest")
    
    Kontext "Autotext"
    Autotext.Close

    'insert a autotext
    '-----------------------------------------------------
    Call wTypeKeys "Redlining test => OK_2<Return>"
    Sleep 1
    EditSelectAll
    Sleep 1

    EditAutotext
    Sleep 2
    Kontext "Autotext"
    Sleep 1
    Liste.Select 1   '1-MyTest'
    Sleep 1

    'Insert a name for the Autotext and a Shortcut
    NameText.Settext "AutotextForRedlininggTest"
    Sleep 1
    Kuerzel.Settext "AFT"

    'Select 'New' in Menubutton 'Autotext'
    Sleep 2
    Menue.Click
    Sleep 2
    Call hMenuSelectNr ( 1 )
    Sleep 2

    Autotext.Close
    '-----------------------------------------------------

    '/// Insert a sentence into document
    Kontext "DocumentWriter"
    EditSelectAll
    Call wTypeKeys "Redlining test => OK_1<Return>"

    UseBindings
    '/// Edit / Changes / Record
    EditChangesRecord

    '/// Insert autotext
    Call wTypeKeys "AFT"
    Call wTypeKeys "<F3>"
    Sleep 1

    '/// Edit / Changes / Accept or Reject
    EditChangesAcceptOrReject
    Kontext "Redlining"
    '/// Select tabpage 'List' ///
    TabControl.SetPage TabListe
    '/// Select 'Accept All'///
    AlleAkzeptieren.Click
    Sleep 2
    '/// Close Redlining dialog
    Redlining.Close

    '/// Check if the changes have been accepted
    Call wTypeKeys "<Up>", 2
    Call wTypeKeys "<Down>"
    Call wTypeKeys "<End>"
    Call wTypeKeys "<Shift Left>"
    EditCopy
    Sleep 1
    if GetClipboardText <> "2" then Warnlog "The 2nd Line is incorrect!"
    
    ' Delete the new category
    ' ------------------------------------------------
    EditAutotext
    Sleep 2
    Kontext "Autotext"
    Sleep 1
    Bereiche.Click
    Sleep 5
    Kontext "BereicheBearbeitenAutoText"
    Liste.Select 1
    Sleep 2
    Loeschen.Click
    Sleep 5
    BereicheBearbeitenAutoText.OK
    Sleep 2
    Kontext "Active"
    if Active.Exists then Active.Yes
    Kontext "Autotext"
    Autotext.Close
    '-------------------------------------------------
    
    '/// Close the document
    Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tRedlining_6

  '/// Test Deleted text
  printlog "Deleted text"

   '/// New document
   Call hNewDocument

   '/// Insert a sentence into document
   Call wTypeKeys "Redlining test => OK_1"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Delete the last 2 characters
   Call wTypeKeys "<Backspace>",2
   Sleep 1

   '/// Check if the changes is OK => The characters are deleted ,
   '/// + but their location exists
   Call wTypeKeys "<End>"
   Sleep 1
   Call wTypeKeys "<Shift Left>" , 2
   Sleep 1

   Editcopy
   if Len(GetClipboardText)>0 then Warnlog "Something is wrong in deleting text before accepting the changes!"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Sleep 2
   Kontext "Redlining"
   Sleep 1
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 2
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the changes have been accepted => The
   '/// + characters are deleted , and their location doesn't exist
   Call wTypeKeys "<End>"
   Sleep 1
   Call wTypeKeys "<Shift Left>" , 2
   Sleep 1

   EditCopy
   Sleep 1
   if GetClipboardText <> "OK" then Warnlog "Something is wrong in deleting text after accepting the changes! Getting " + GetClipboardText

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tRedlining_7

    '/// Test sorted text
    printlog "sorted text"
    
    '/// New document
    Call hNewDocument
    
    '/// Insert some test text
    Call wTypeKeys "XYZ<Return>"
    Call wTypeKeys "ABC"
    
    UseBindings
    '/// Edit / Changes / Record
    EditChangesRecord
    
    '/// Select Tools / Sort , Language (English USA) , Alphanumeric
    EditSelectAll
    ToolsSort
    Kontext "Sortieren"
    Schluesseltyp1.Select 1
    Aufsteigend1.Check
    Sortieren.OK
    
    '/// Check if the changes is OK => The characters are changed ,
    '/// + but their location exists
    Call wTypeKeys "<Down>" , 4
    Call wTypeKeys "<End>"
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if Len(GetClipboardText)>0 then Warnlog "Something is wrong in sorting text before accepting the changes!"
    
    Call wTypeKeys "<Up>" , 4
    Call wTypeKeys "<End>"
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> "ABC" then Warnlog "Should get ABC in the first line, but get " +GetClipboardText + "  before accepting the changes!"
    
    '/// Edit / Changes / Accept or Reject
    EditChangesAcceptOrReject
    Sleep 2
    Kontext "Redlining"
    '/// + Select tabpage 'List' ///
    TabControl.SetPage TabListe
    Sleep 2
    '/// + Select 'Accept All'///
    AlleAkzeptieren.Click
    Sleep 2
    '/// + Close Redlining dialog
    Redlining.Close
    
    '/// Check if the changes have been accepted => The characters
    '/// + are changed , and their location doesn't exist
    Call wTypeKeys "<Down>" , 4
    Call wTypeKeys "<End><Backspace>"
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if Len(GetClipboardText) <> 3 then Warnlog "Something is wrong in sorting text after accepting the changes!"
    
    Call wTypeKeys "<Up>" , 4
    Call wTypeKeys "<End>"
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> "ABC" then Warnlog "Should get ABC in the first line, but get " +GetClipboardText + "  after accepting the changes!"
    
    '/// Close the document
    Call hCloseDocument
    
endcase

'----------------------------------------------------------------------------

testcase tRedlining_8

  '/// Test searched and replaced text
  printlog "searched and replaced text"

   '/// New document
   Call hNewDocument

   '/// Insert a sentence into document
   Call wTypeKeys "Redlining test => OK_1"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Select Edit / Find & Replace
   EditSearchAndReplace
   Kontext "FindAndReplace"

   '/// + Set OK_1 in search for editbox
   SearchFor.SetText "OK_1"
   Sleep 2

   '/// + set OK_2 in replace with editbox
   ReplaceWith.SetText "OK_2"
   Sleep 2

   '/// + Replace all and close the dialogue
   ReplaceAll.Click
   sleep 2
   Kontext "Active"
   if Active.Exists then
      if Active.GetRT = 304 then
         Active.OK
         Sleep 1
      else
         Warnlog "Messagebox, how often replaced, not up !"
      end if
   end if

   Kontext "FindAndReplace"
   FindAndReplace.close

   '/// Check if the changes is OK => The characters are changed ,
   '/// + but their location exists
   Call wTypeKeys "<End>"
   wait 500
   Call wTypeKeys "<Shift Left>"
   wait 500
   Editcopy
   wait 500
   if GetClipboardText <> "2" then Warnlog "Something is wrong in changing text before accepting the changes!"

   Call wTypeKeys "<Left>" , 3
   Sleep 1
   Call wTypeKeys "<Shift Left>" , 4
   Sleep 1
   Editcopy
   wait 500
   if Len(GetClipboardText) > 0 then Warnlog "Something is wrong in changing text before accepting the changes!"

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Sleep 2
   Kontext "Redlining"
   Sleep 2
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 2
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the changes have been accepted => The characters
   '/// + are changed , and their location doesn't exist
   Call wTypeKeys "<End>"
   Call wTypeKeys "<Shift Left>"

   EditCopy
   Sleep 1
   if GetClipboardText <> "2" then Warnlog "Something is wrong in changing text after accepting the changes!"

   Call wTypeKeys "<Left>" , 3
   Sleep 1
   Call wTypeKeys "<Shift Left>" , 4
   Sleep 1
   Editcopy
   if Len(GetClipboardText) <> 4 then Warnlog "Something is wrong in changing text after accepting the changes!"

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tRedlining_9

	Dim iComparison1$,iComparison2$,iComparison3$,iComparison4$
	
	'/// Test text attributs
	printlog "Test text attributs"
	
	'/// New document
	Call hNewDocument
	
	'/// Insert a sentence into document
	Call wTypeKeys "Redlining test => OK_1<Home>"
	Call wTypeKeys "<Shift Right>" , 9
	Sleep 1
	
	UseBindings
	'/// Edit / Changes / Record
	EditChangesRecord
	
	'/// Select Format / Character , changes some font Attributes
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
	wait 500
	TabHyperlinkZeichen.Ok
	
	Sleep 1
	Call wTypeKeys "<Left>"  , 2
	Sleep 1
	
	'/// Check if the Attribute changes id OK before accepting the changes
	printlog "Check if the Attribute changes id OK before accepting the changes"
	FormatCharacter
	Kontext
	Active.SetPage TabFont
	Kontext "TabFont"
	if gAsianSup = True then
		if iComparison1$ <> FontWest.GetSelText  then Warnlog "Wrong Font in Autotext!"
		if iComparison2$ <> StyleWest.GetSelText then Warnlog "Wrong Fontstyle in Autotext!"
		if iComparison3$ <> SizeWest.GetSelText  then QAErrorlog "#108881#Wrong Fontsize in Autotext!"
	else
		if iComparison1$ <> Font.GetSelText  then Warnlog "Wrong Font in Autotext!"
		if iComparison2$ <> Style.GetSelText then Warnlog "Wrong Fontstyle in Autotext!"
		if iComparison3$ <> Size.GetSelText  then Warnlog "Wrong Fontsize in Autotext!"
	end if
	wait 100
	Kontext
	Active.Setpage TabHyperlinkZeichen
	Kontext "TabHyperlinkZeichen"
	if UrlName.Gettext <> "http://www.sun.com/"  then Warnlog "Wrong hyperlink has been added to text!"
	if iComparison4$   <> TargetFrame.GetSelText then Warnlog "Wrong targetFrame has been added to text!"
	TabHyperlinkZeichen.Cancel
	
	'/// Edit / Changes / Accept or Reject
	EditChangesAcceptOrReject
	Kontext "Redlining"
	'/// + Select tabpage 'List' ///
	TabControl.SetPage TabListe
	'/// + Select 'Accept All'///
	AlleAkzeptieren.Click
	Sleep 2
	'/// + Close Redlining dialog
	Redlining.Close
	
	'/// Check if the Attribute changes id OK after accepting the changes
	printlog "Check if the Attribute changes id OK after accepting the changes"
	FormatCharacter
	Kontext
	Active.SetPage TabFont
	Kontext "TabFont"
	if gAsianSup = True then
		if iComparison1$ <> FontWest.GetSelText  then Warnlog "Wrong Font in Autotext!"
		if iComparison2$ <> StyleWest.GetSelText then Warnlog "Wrong Fontstyle in Autotext!"
		if iComparison3$ <> SizeWest.GetSelText  then QAErrorlog "#108881#Wrong Fontsize in Autotext!"
	else
		if iComparison1$ <> Font.GetSelText  then Warnlog "Wrong Font in Autotext!"
		if iComparison2$ <> Style.GetSelText then Warnlog "Wrong Fontstyle in Autotext!"
		if iComparison3$ <> Size.GetSelText  then Warnlog "Wrong Fontsize in Autotext!"
	end if
	wait 100
	Kontext
	Active.Setpage TabHyperlinkZeichen
	Kontext "TabHyperlinkZeichen"
	if UrlName.Gettext <> "http://www.sun.com/"   then Warnlog "Wrong hyperlink has been added to text!"
	if iComparison4$   <> TargetFrame.GetSelText  then Warnlog "Wrong targetFrame has been added to text!"
	TabHyperlinkZeichen.Cancel
	
	'/// Close the document
	Call hCloseDocument
endcase

'----------------------------------------------------------------------------

testcase tRedlining_10

  Dim sSectionName as String

  sSectionName = "testForRedlining"

  '/// Test inserted sections
  printlog "inserted sections"

   '/// New document
   Call hNewDocument

   '/// Insert a sentence into document
   Call wTypeKeys "Redlining test => OK_1<Shift Home>"

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Insert a section in the testing sentence , insert / section
   InsertSection
   wait 500
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"
   Bereichsliste.Settext sSectionName
   TabBereiche.OK
   wait 500

   '/// Check if the section is inserted correctly before accepting the changes , format / sections
   FormatSections
   Kontext "BereicheBearbeiten"
   if BereichsName.GetText <> sSectionName then warnlog "the section is NOt inserted correctly before accepting the changes "
   BereicheBearbeiten.Cancel

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the section is inserted correctly after accepting the changes , format / sections
   FormatSections
   Kontext "BereicheBearbeiten"
   if BereichsName.GetText <> sSectionName then warnlog "the section is NOt inserted correctly after accepting the changes "
   BereicheBearbeiten.Cancel

   '/// Close the document
   Call hCloseDocument
endcase

'----------------------------------------------------------------------------

testcase tRedlining_11

  '/// Test inserted tables
  printlog "inserted tables"

   '/// New document
   Call hNewDocument

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Insert a table
   Call hTabelleEinfuegen

   '/// Check if the table is inserted correctly before accepting the changes
   Call wOpenContextMenu
   hMenuSelectNr(10)

   try
      Kontext
      Active.Setpage TabTabelle
      Kontext "TabTabelle"
      if Right(TabellenName.GetText , 1) <> "1"  then warnlog "the table is NOt inserted correctly before accepting the changes "
      TabTabelle.Cancel
   catch
      Warnlog "Something wrong in inserting table  before accepting the changes!"
   endcatch

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   '/// Check if the table is inserted correctly after accepting the changes
   Call wOpenContextMenu
   hMenuSelectNr(10)

   try
      Kontext
      Active.Setpage TabTabelle
      Kontext "TabTabelle"
      if Right(TabellenName.GetText , 1) <> "1"  then warnlog "the table is NOt inserted correctly after accepting the changes "
      TabTabelle.Cancel
   catch
      Warnlog "Something wrong in inserting table  after accepting the changes!"
   endcatch

   '/// Close the document
   Call hCloseDocument
endcase

'----------------------------------------------------------------------------

testcase tRedlining_12

  '/// Test inserted field (date)
  printlog "inserted field (date)"

   '/// New document
   Call hNewDocument

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Insert a date , insert / field / date
   InsertFieldsDate

   Call wTypeKeys "<Home>"
   Sleep 1

   '/// Check if the date is inserted correctly before accepting the changes
   try
      EditFields
      Kontext "FeldbefehlBearbeitenDokument"
      FeldbefehlBearbeitenDokument.Cancel
   catch
      Warnlog "Something wrong in inserting date  before accepting the changes!"
   endcatch

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close
   Sleep 1

   Call wTypeKeys "<Home>"

   '/// Check if the date is inserted correctly after accepting the changes
   try
      EditFields
      Kontext "FeldbefehlBearbeitenDokument"
      FeldbefehlBearbeitenDokument.Cancel
   catch
      Warnlog "Something wrong in inserting date after accepting the changes!"
   endcatch

   '/// Close the document
   Call hCloseDocument
endcase

'----------------------------------------------------------------------------

testcase tRedlining_13

  '/// Test inserted footnotes
  printlog "inserted footnotes"

   '/// New document
   Call hNewDocument

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   '/// Insert a footnote , insert / footnotes
   InsertFootnote
   Kontext "FussnoteEinfuegen"
   Sleep 1
   FussnoteEinfuegen.OK

   Call wTypeKeys "<PageUp><Home>"

   '/// Check if the footnote is inserted correctly before
   '/// + accepting the changes
   try
      EditFootnote
      Kontext "FussnoteBearbeiten"
      FussnoteBearbeiten.Cancel
   catch
      Warnlog "Something wrong in inserting footnote before accepting the changes!"
   endcatch

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Kontext "Redlining"
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close

   Call wTypeKeys "<Home>"

   '/// Check if the footnote is inserted correctly after accepting the changes
   try
      EditFootnote
      Kontext "FussnoteBearbeiten"
      FussnoteBearbeiten.Cancel
   catch
      Warnlog "Something wrong in inserting footnote after accepting the changes!"
   endcatch

   '/// Close the document
   Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tRedlining_14

  '/// Test inserted header
  printlog "inserted header"

  '/// New document
  Call hNewDocument

  '/// Edit / Changes / Record
  EditChangesRecord

  '/// Insert a header , insert / header
  DocumentWriter.UseMenu
  MenuSelect(Menugetitemid(4))
  Sleep 2
  MenuSelect(Menugetitemid(9))
  Sleep 2
  MenuSelect(Menugetitemid(1))
  Sleep 2

  '/// input some test words in the header
  Call wTypeKeys "Header Test"

  '/// Check if the header is inserted correctly before
  '/// + accepting the changes
  FormatPageWriter
  Kontext
  Active.SetPage TabKopfzeile
  Kontext "TabKopfzeile"
  if Not KopfzeileMitAbstand.IsChecked then Warnlog "The header is NOT inserted correctly before accepting the changes!"
  TabKopfzeile.Cancel

  '/// Edit / Changes / Accept or Reject
  EditChangesAcceptOrReject
  Kontext "Redlining"
  '/// + Select tabpage 'List' ///
  TabControl.SetPage TabListe
  '/// + Select 'Accept All'///
  AlleAkzeptieren.Click
  Sleep 2
  '/// + Close Redlining dialog
  Redlining.Close

  '/// Check if the header is inserted correctly after accepting the changes
  FormatPageWriter
  Kontext
  Active.SetPage TabKopfzeile
  Kontext "TabKopfzeile"
  if Not KopfzeileMitAbstand.IsChecked then Warnlog "The header is NOT inserted correctly after accepting the changes!"
  TabKopfzeile.Cancel

  '/// Close the document
  Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tRedlining_15

  '/// Test inserted footer
  printlog "inserted footer"

  '/// New document
  Call hNewDocument

  '/// Edit / Changes / Record
  EditChangesRecord

  '/// Insert a footer , insert / footer
  DocumentWriter.UseMenu
  MenuSelect(Menugetitemid(4))
  Sleep 2
  MenuSelect(Menugetitemid(10))
  Sleep 2
  MenuSelect(Menugetitemid(1))
  Sleep 2

  Call wTypeKeys "Footer Test"

  '/// Check if the footer is inserted correctly before
  '/// + accepting the changes
  FormatPageWriter
  Kontext
  Active.SetPage TabFusszeile
  Kontext "TabFusszeile"
  if Not FusszeileMitAbstand.IsChecked then Warnlog "The footer is NOT inserted correctly before accepting the changes!"
  TabFusszeile.Cancel

  '/// Edit / Changes / Accept or Reject
  EditChangesAcceptOrReject
  Kontext "Redlining"
  '/// + Select tabpage 'List' ///
  TabControl.SetPage TabListe
  '/// + Select 'Accept All'///
  AlleAkzeptieren.Click
  Sleep 2
  '/// + Close Redlining dialog
  Redlining.Close

  '/// Check if the footer is inserted correctly after accepting the changes
  FormatPageWriter
  Kontext
  Active.SetPage TabFusszeile
  Kontext "TabFusszeile"
  if Not FusszeileMitAbstand.IsChecked then Warnlog "The footer is NOT inserted correctly after accepting the changes!"
  TabFusszeile.Cancel

  '/// Close the document
  Call hCloseDocument

endcase

'----------------------------------------------------------------------------

testcase tRedlining_16

  Dim iComparison1$ , iComparison2$

  '/// Test moved paragraphs/chapters
  printlog "moved paragraphs/chapters"

   '/// New document
   Call hNewDocument

   '/// Insert a paragraph ( Open the test file )
   '/// + There are 3 lines
   '/// + Redlining test => OK_1  <--  No numbering
   '/// + Redlining test => OK_2  <--  Numbering style : list 2
   '/// + Redlining test => OK_3  <--  Numbering style : list 3

   hFileOpenLocally(  gTesttoolPath + "writer\optional\input\redlining\tRedlining_15.sxw" )
   

   Call wTypeKeys "<Up>" , 3

   Call wTypeKeys "<Down>"
   FormatParagraph
   Kontext
   Active.SetPage TabNumerierungAbsatz
   Kontext "TabNumerierungAbsatz"
   iComparison1$ = Vorlage.GetSelText
   TabNumerierungAbsatz.Cancel

   Call wTypeKeys "<Down>"
   FormatParagraph
   Kontext
   Active.SetPage TabNumerierungAbsatz
   Kontext "TabNumerierungAbsatz"
   iComparison2$ = Vorlage.GetSelText
   TabNumerierungAbsatz.Cancel

   UseBindings
   '/// Edit / Changes / Record
   EditChangesRecord

   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Shift Down Right>"
   '/// Cut the selection
   EditCut

   Call wTypeKeys "<Up>" , 3
   Call wTypeKeys "<End><Return>"
   '/// paste under the first line
   EditPaste

   Call wTypeKeys "<Up><Up><End><Delete>"

   '/// Check if the paragraph is moved correctly before
   '/// + accepting the changes
   Call wTypeKeys "<Up>" , 3
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>"
   try
       EditCopy
   catch
       Warnlog "Bullets cannot be removed by Del key"
       Call wTypeKeys "<Down><End>"
       Call wTypeKeys "<Shift Left>"
       EditCopy
   endcatch

   if GetClipboardText <> "3" then Warnlog "A.The paragraph is moved incorrect before accepting the changes - 2nd line"

   FormatParagraph
   Kontext
   Active.SetPage TabNumerierungAbsatz
   Kontext "TabNumerierungAbsatz"
   if iComparison2$ <> Vorlage.GetSelText then Warnlog "B.The paragraph is moved incorrect before accepting the changes - 2nd line"
   TabNumerierungAbsatz.Cancel

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>"
   EditCopy
   if GetClipboardText <> "2" then Warnlog "A. The paragraph is moved incorrect before accepting the changes - 3rd line"

   FormatParagraph
   Kontext
   Active.SetPage TabNumerierungAbsatz
   Kontext "TabNumerierungAbsatz"
   if iComparison1$ <> Vorlage.GetSelText then Warnlog "B . The paragraph is moved incorrect before accepting the changes - 3rd line"
   TabNumerierungAbsatz.Cancel

   '/// Edit / Changes / Accept or Reject
   EditChangesAcceptOrReject
   Sleep 2
   Kontext "Redlining"
   Sleep 2
   '/// + Select tabpage 'List' ///
   TabControl.SetPage TabListe
   Sleep 2
   '/// + Select 'Accept All'///
   AlleAkzeptieren.Click
   Sleep 2
   '/// + Close Redlining dialog
   Redlining.Close
   Sleep 2

   '/// Check if the footnote is inserted correctly after
   '/// + accepting the changes
   Call wTypeKeys "<Up>" , 3
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>"
   try
       EditCopy
   catch
       Warnlog "Bullets cannot be removed by Del key"
       Call wTypeKeys "<Down><End>"
       Call wTypeKeys "<Shift Left>"
       EditCopy
   endcatch
   if GetClipboardText <> "3" then Warnlog "A.The paragraph is moved incorrect after accepting the changes - 2nd line"

   FormatParagraph
   Kontext
   Active.SetPage TabNumerierungAbsatz
   Kontext "TabNumerierungAbsatz"
   if iComparison2$ <> Vorlage.GetSelText then Warnlog "B.The paragraph is moved incorrect after accepting the changes - 2nd line"
   TabNumerierungAbsatz.Cancel

   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Shift Left>"
   EditCopy
   if GetClipboardText <> "2" then Warnlog "A. The paragraph is moved incorrect after accepting the changes - 3rd line"

   FormatParagraph
   Kontext
   Active.SetPage TabNumerierungAbsatz
   Kontext "TabNumerierungAbsatz"
   if iComparison1$ <> Vorlage.GetSelText then Warnlog "The paragraph is moved incorrect after accepting the changes - 3rd line"
   TabNumerierungAbsatz.Cancel

    PrintLog "Close active documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

