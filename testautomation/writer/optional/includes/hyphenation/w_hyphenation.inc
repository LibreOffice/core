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
'* short description : hyphenation functionality test
'*
'************************************************************************
'*
' #1 tHyphenation_1
' #1 tHyphenation_2
' #1 tHyphenation_3
' #1 tHyphenation_4
'*
'\***********************************************************************

testcase tHyphenation_1

	Dim testFile as String
	Dim testWord as String
	Dim PartOfTestWord as String
	
	testFile             =  "tHyphenation.odt"
	testWord             =  "following"
	PartOfTestWord  =  "fol"
	
	printlog "Test Hyphenation  using ctrl and -"
	
	printlog "open a test file"
	hFileOpenLocally(  gTesttoolPath + "writer\optional\input\hyphenation\" + testFile )
	
		
	' we must assure the word 'following' is the first word in second line
	' otherwise the test will not work correctly	
	if fPrepareHyphenationDocument(testWord) = false then
		Warnlog "Unable to prepare document for hyphenation. Check testcase"
		Call hCloseDocument
	end if
	
	printlog "add '-' between 'fol' and 'lowing'"
	printlog "+ check if fol in the end of the line"
	
	Call wTypeKeys "<Home>"
	Call wTypeKeys "<Right>" , 3
	Call wTypeKeys "<Mod1 SUBTRACT>"
	Call wTypeKeys "<Mod1 Home>"
	Call wTypeKeys "<End>"
	Call wTypeKeys "<Delete>"
	
	Call wTypeKeys "<End><Left>"
	Call wTypeKeys "<Mod1 Shift Left>"
	EditCopy
	
	if GetClipboardText <> PartOfTestWord then
	 	Warnlog "The hyphenation does NOT work ! Found '" & GetClipboardText & "' and not '" & PartOfTestWord & "'"
	end if
	
	Call hCloseDocument

endcase	

'-------------------------------------------------------------------------

testcase tHyphenation_2

	Dim testFile as String
	Dim testWord as String
	Dim PartOfTestWord as String
	
	testFile       	=  "tHyphenation.odt"
	testWord       	=  "following"
	PartOfTestWord  =  "follow"
	
	printlog "Test Hyphenation using ctrl and -"
	
	printlog "open a test file"
	hFileOpenLocally(  gTesttoolPath + "writer\optional\input\hyphenation\" + testFile )
	
		
	' we must assure the word 'following' is the first word in second line
	' otherwise the test will not work correctly	
	if fPrepareHyphenationDocument(testWord) = false then
		Warnlog "Unable to prepare document for hyphenation. Check testcase"
		Call hCloseDocument
	end if
	
	printlog "add '-' between 'follow' and 'ing'"
	printlog "+ check if 'follow' in the end of the line"
	
	Call wTypeKeys "<Home>"
	Call wTypeKeys "<Right>" , 6
	Call wTypeKeys "<Mod1 SUBTRACT>"
	Call wTypeKeys "<Mod1 Home>"
	Call wTypeKeys "<End>"
	if gPlatgroup = "unx" then
		Call wTypeKeys "<Delete>", 6
	else
		Call wTypeKeys "<Delete>", 7
	end if
	
	Call wTypeKeys "<End><Left>"
	Call wTypeKeys "<Mod1 Shift Left>"
	EditCopy
	
	if GetClipboardText <> PartOfTestWord then
	 	Warnlog "The hyphenation does NOT work ! Found '" & GetClipboardText & "' and not '" & PartOfTestWord & "'"
	end if
	
	Call hCloseDocument

endcase	

'-------------------------------------------------------------------------

testcase tHyphenation_3

	Dim testFile as String
	Dim testWord as String
	Dim firstPartOfTestWord as String
	Dim secondPartOfTestWord as String
	Dim testWordInHyphenationDlg as String
	
	testFile                 =  "tHyphenation.odt"
	testWord                 =  "following"
	testWordInHyphenationDlg =  "fol=low-ing"
	firstPartOfTestWord      =  "fol"
	secondPartOfTestWord     =  "follow"
	
	printlog "Test Hyphenation  using Tools/Hyphenation ,test like fol-lowing"
	
	printlog "open a test file"
	hFileOpenLocally(  gTesttoolPath + "writer\optional\input\hyphenation\" + testFile )
	

	' we must assure the word 'following' is the first word in second line
	' otherwise the test will not work correctly	
	if fPrepareHyphenationDocument(testWord) = false then
		Warnlog "Unable to prepare document for hyphenation. Check testcase"
		Call hCloseDocument
	end if
	
	Call wTypeKeys "<Mod1 Home>"
	Call wTypeKeys "<End>"
	if gPlatgroup = "unx" then
		Call wTypeKeys "<Delete>", 6
	else
		Call wTypeKeys "<Delete>", 7
	end if
	
	printlog "Tools/Hyphenation"
	ToolsLanguageHyphenate
	Kontext "Hyphenation"
	
	if Not Hyphenation.Exists then	
		Warnlog "Unable to bring up Dialog 'Hyphenation'!"
		Call hCloseDocument
		goto endsub
	end if
	
	printlog "check if get 'fol=low-ing'"
	if Wort.Gettext <> testWordInHyphenationDlg then
		Warnlog "Expected " & testWordInHyphenationDlg & " but found " & Wort.Gettext 
		Hyphenation.Cancel
		Call hCloseDocument
		goto endsub
	end if
	
	if NOT Vor.IsEnabled then
		Warnlog "the 'Vor' button is Not enabled, or something wrong!"
		Hyphenation.Cancel
		Call hCloseDocument
		goto endsub
	end if
	
	printlog "add '-' between 'fol' and 'lowing' using left arrow key"
	Vor.Click
	Hyphenation.OK
	
	' Messagebox 'Start hypheantion from the beginning ?
	Kontext "Active"
	if Active.Exists then
		if Active.GetRT = 304 then
			try
				Active.Yes
			catch
				Active.ok
			endcatch
		else
			Warnlog "No messagebox asking for hyphenate at beginning of document"
		end if
	end if

	' End of hyphenation message
	if Active.Exists then
		if Active.GetRT = 304 then
			try
				Active.Ok
			catch
				Active.yes
			endcatch
		else
			Warning "No 'end of hyphenation' messagebox"
		end if
	end if
				
	printlog "Press 3 times 'space bar' in front of the line"
	Call wTypeKeys "<Mod1 Home>"
	Call wTypeKeys "<End>"
	Call wTypeKeys "<Left>"	
	Call wTypeKeys "<Shift Mod1 Left>"
	EditCopy
	
	printlog "Check if get 'fol'"
	if GetClipboardText <> firstPartOfTestWord then
		Warnlog "The hyphenation does NOT work ! Found '" & GetClipboardText & "' and not '" & PartOfTestWord & "'"
	end if
	
	Call hCloseDocument
	
endcase

'-------------------------------------------------------------------------

testcase tHyphenation_4

	Dim testFile as String
	Dim testWord as String
	Dim firstPartOfTestWord as string
	Dim secondPartOfTestWord as String
	Dim testWordInHyphenationDlg1 as String
	Dim testWordInHyphenationDlg2 as String
	
	testFile                  =  "tHyphenation.odt"
	testWord                  =  "following"
	firstPartOfTestWord       =  "fol"
	secondPartOfTestWord      =  "follow"
	testWordInHyphenationDlg1 =  "fol=low-ing"
	testWordInHyphenationDlg2 =  "fol-low=ing"
	
	printLog "- Test Hyphenation  using Tools/Hyphenation ,test like follow-ing"
	printlog "Test Hyphenation  using Tools/Hyphenation ,test like follow-ing"
	
	printlog "open a test file"
	hFileOpenLocally(  gTesttoolPath + "writer\optional\input\hyphenation\" + testFile )
	
	
	' we must assure the word 'following' is the first word in second line
	' otherwise the test will not work correctly	
	if fPrepareHyphenationDocument(testWord) = false then
		Warnlog "Unable to prepare document for hyphenation. Check testcase"
		Call hCloseDocument
	end if
	
	Call wTypeKeys "<Mod1 Home>"
	Call wTypeKeys "<End>"
	if gPlatgroup = "unx" then
		Call wTypeKeys "<Delete>", 6
	else
		Call wTypeKeys "<Delete>", 7
	end if
	
	printlog "Tools/Hyphenation"
	ToolsLanguageHyphenate
	Kontext "Hyphenation"
	
	if Not Hyphenation.Exists then
		Call hCloseDocument
		goto endsub
	end if
	
	printlog "check if get 'fol=low-ing'"
	if Wort.Gettext <> testWordInHyphenationDlg1 then
		Warnlog "Expected " & testWordInHyphenationDlg1 & " but found " & Wort.Gettext
		Hyphenation.Cancel
		Call hCloseDocument
		goto endsub
	end if
	
	printlog "click left arrow key"
	if NOT Vor.IsEnabled then
		Warnlog "the Vor button is Not enabled, or something wrong!"
		Hyphenation.Cancel
		Call hCloseDocument
		goto endsub
	end if
	Vor.Click
	
	printlog "check if get ''fol-low=ing'"
	if Wort.Gettext <> testWordInHyphenationDlg2 then
		Warnlog "Expected " & testWordInHyphenationDlg2 & " but found " & Wort.Gettext
		Call hCloseDocument
		goto endsub
	end if
	
	printlog "add '-' between 'follow' and 'ing' pressing right arrow key"
	if NOT Zurueck.IsEnabled then
		Warnlog "the 'Back' button is Not enabled, or something wrong!"
		Call hCloseDocument
		goto endsub	
	end if
	Zurueck.Click
	
	printlog "check if get 'fol-low=ing'"
	if Wort.Gettext <> testWordInHyphenationDlg1 then
		Warnlog "The test  word should be changed after clicking Zurueck!"
		Call hCloseDocument
		goto endsub
	end if
	
	Sleep 1
	
	printlog "close all dialogs"
	Hyphenation.OK
	
	' Messagebox 'Start hypheantion from the beginning ?
	Kontext "Active"
	if Active.Exists then
		if Active.GetRT = 304 then
			try
				Active.Yes
			catch
				Active.ok
			endcatch
		else
			Warnlog "No messagebox asking for hyphenate at beginning of document"
		end if
	end if

	' End of hyphenation message
	if Active.Exists then
		if Active.GetRT = 304 then
			try
				Active.Ok
			catch
				Active.yes
			endcatch
		else
			Warning "No 'end of hyphenation' messagebox"
		end if
	end if
	
	printlog "Press 3 times 'space bar' in front of the line"
	Call wTypeKeys "<Mod1 Home>"
	Call wTypeKeys "<End>"
	Call wTypeKeys "<Left>"	
	Call wTypeKeys "<Shift Mod1 Left>"
	EditCopy
	
	printlog "Check if get 'follow'"
	if GetClipboardText <> secondPartOfTestWord then
		Warnlog "The hyphenation does NOT work ! Found '" & GetClipboardText & "' and not '" & secondPartOfTestWord & "'"
	end if

	Call hCloseDocument
	
endcase
