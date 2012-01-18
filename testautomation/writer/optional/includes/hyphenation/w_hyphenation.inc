'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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
