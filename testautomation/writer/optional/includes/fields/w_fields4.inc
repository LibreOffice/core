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
'* short description : Doc Information Test
'*
'\***********************************************************************

sub w_fields4

    Call tDocInformationTitle
    Call tDocInformationSubject
    Call tDocInformationKeywords
    Call tDocInformationDescription
    Call tDocInformationUserDefined
    Call tDocInformationCreated
    Call tDocInformationModified
    Call tDocInformationModifiedSavefile
    Call tDocInformationPrinted
    Call tDocInformationRevisionnumber
    Call tDocInformationTotaleditingtime

end sub

'-----------------------------------------------------------------

testcase tDocInformationTitle

  Dim sTitle     as String
  Dim sTestFile  as String
  Dim sVarResult as String
  Dim sFixResult as String

  sTitle     = "Title Test"
  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw")
  sVarResult = sTitle
  sFixResult = "This is the title"

  printlog "Doc Information - Title"
 '/// <b> Doc Information - Title </b>

  Call hNewDocument

  '/// Open test file fields_docinfos.sxw
   hFileOpenLocally( sTestFile )
   

  '/// File / Properties / Description , input some
  '/// + new text in Title
   Call fFileProperties("TabBeschreibung")
     Titel.Settext sTitle
   TabBeschreibung.OK

  '/// Check if the title is changed in the document
  '/// It should be changed in Var
  '/// It should NOT be changed in Fix
   Call fFindWord ("Title")

   Call wTypeKeys "<End><Right>"
   Sleep 1
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sVarResult then
       Warnlog "Should get " & sVarResult & " but get " & fGetFieldContent()
   end if

   Call wTypeKeys "<End><Right>"
   Sleep 1
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sFixResult then
       Warnlog "Should get " & sFixResult & " but get " & fGetFieldContent()
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocInformationSubject

  Dim sSubject   as String
  Dim sTestFile  as String
  Dim sVarResult as String
  Dim sFixResult as String

  sSubject   = "Subject Test"
  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw")
  sVarResult = sSubject
  sFixResult = "This is the subject"

  printlog "Doc Information - Subject"
 '/// <b> Doc Information - Subject </b>

  Call hNewDocument

  '/// Open test file fields_docinfos.sxw
   hFileOpenLocally( sTestFile )
   

  '/// File / Properties / Description , input some
  '/// + text in Subject
   Call fFileProperties("TabBeschreibung")
     Thema.Settext sSubject
   TabBeschreibung.OK

  '/// Check if the Subject is changed in the document
  '/// It should be changed in Var
  '/// It should NOT be changed in Fix
   Call fFindWord ("Subject")

   Call wTypeKeys "<End><Right>"
   Sleep 1
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sVarResult then
       Warnlog "Should get " & sVarResult & " but get " & fGetFieldContent()
   end if

   Call wTypeKeys "<End><Right>"
   Sleep 1
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sFixResult then
       Warnlog "Should get " & sFixResult & " but get " & fGetFieldContent()
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocInformationKeywords

  Dim sKeywords  as String
  Dim sTestFile  as String
  Dim sVarResult as String
  Dim sFixResult as String

  sKeywords  = "Keywords Test"
  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw")
  sVarResult = sKeywords
  sFixResult = "This are keywords"

  printlog "Doc Information - Keywords"
 '/// <b> Doc Information - Keywords </b>

  Call hNewDocument

  '/// Open test file fields_docinfos.sxw
   hFileOpenLocally( sTestFile )
   

  '/// File / Properties / Description , input some
  '/// + text in Keywords
   Call fFileProperties("TabBeschreibung")
     Schluesselwoerter.Settext sKeywords
   TabBeschreibung.OK

  '/// Check if the Keywords is changed in the document
  '/// It should be changed in Var
  '/// It should NOT be changed in Fix
   Call fFindWord ("Keywords")

   Call wTypeKeys "<End><Right>"
   Sleep 1
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sVarResult then
       Warnlog "Should get " & sVarResult & " but get " & fGetFieldContent()
   end if

   Call wTypeKeys "<End><Right>"
   Sleep 1
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sFixResult then
       Warnlog "Should get " & sFixResult & " but get " & fGetFieldContent()
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocInformationDescription

  Dim sDescription as String
  Dim sTestFile    as String
  Dim sVarResult   as String
  Dim sFixResult   as String

  sDescription = "Description Test"
  sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw")
  sVarResult   = sDescription
  sFixResult   = "This is the description"

  printlog "Doc Information - Description"
 '/// <b> Doc Information - Description </b>

  Call hNewDocument

  '/// Open test file fields_docinfos.sxw
   hFileOpenLocally( sTestFile )
   

  '/// File / Properties / Description , input some
  '/// + text in Description
   Call fFileProperties("TabBeschreibung")
     Beschreibung.Settext sDescription
     Schluesselwoerter.Gettext  '<--This command is useless , but important ! :-)
   TabBeschreibung.OK

  '/// Check if the Description is changed in the document
  '/// It should be changed in Var
  '/// It should NOT be changed in Fix
   Call fFindWord ("Description")

   Call wTypeKeys "<End>"
   Call wTypeKeys "<Down>"  , 4
   Call wTypeKeys "<Right>" , 2
   Sleep 1
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sVarResult then
       Warnlog "Due to i31893 , should get " & sVarResult & " but get " & fGetFieldContent()
   end if

   Call wTypeKeys "<End><Right>"
   Sleep 1
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sFixResult then
       Warnlog "Should get " & sFixResult & " but get " & fGetFieldContent()
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocInformationUserDefined
    Warnlog "#i95523#Tabpage 'Custom properties' has no declaration"
    goto endsub
	Dim sTestFile as String
	Dim sVarResult() as String
	Dim sFixResult() as String
	Dim i as integer
	Dim j as Integer
	
	j = UBound(sVarResult)
	sTestFile = gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw"
	sVarResult = Array( _
                      "A1"     , _
                      "B2"     , _
                      "C3"     , _
                      "D4"     , _
                     )

     sFixResult = Array( _
                      "Lennon"     , _
                      "McCartney"  , _
                      "Harrison"   , _
                      "Star"       , _
                     )

	printlog "Doc Information - User Defined"
	
	
	Call hNewDocument
	
	'/// Open test file fields_docinfos.sxw
	hFileOpenLocally( sTestFile )
	
	'/// File / Properties / Description , input some
	'/// + text in Description
	Call fFileProperties("TabDokument")
	Info0.Settext sVarResult(0)
	Info1.Settext sVarResult(1)
	Info2.Settext sVarResult(2)
	Info3.Settext sVarResult(3)
	TabBenutzer.OK
	
	'/// Check if the User defined is changed in the document
	'/// It should be changed in Var
	'/// It should NOT be changed in Fix
	Call fFindWord ("Info fields")
	
	Call wTypeKeys "<End><Right><Down><Home>"
	Sleep 1
	
	for i= 0 to UBound(sVarResult)
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sVarResult(i) then
	Warnlog "Should get " & sVarResult(i) & " but get " & fGetFieldContent()
	end if
	
	Call wTypeKeys "<End><Right>"
	Sleep 1
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sFixResult(i) then
	Warnlog "Should get " & sFixResult(i) & " but get " & fGetFieldContent()
	end if
	
	Call wTypeKeys "<End><Down><Home><Left><Home>"
	next i
	
	Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocInformationCreated

	Dim sFirstName    as String
	Dim sLastName     as String
	Dim sFirstName1   as String
	Dim sLastName1    as String
	Dim sVarResult(3) as String
	Dim sFixResult(3) as String
        dim sTestFile as string
	Dim i as Integer
	
	sFirstName1 = "Test1" : sLastName1 = "Test2"
	sTestFile   = gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw"
	
	sVarResult(0) =  sFirstName1 + " " + sLastName1
	sVarResult(1) =  fGetDate1(Date)
	
	sFixResult(0) =  "Éric Savary"
	sFixResult(1) =  "07/20/2000"
	sFixResult(2) =  "11:41:21"
	
	printlog "Doc Information - Created"
	'/// <b> Doc Information - Created </b>
	
	Call hNewDocument
	
	'/// Open test file fields_docinfos.sxw
	hFileOpenLocally( sTestFile )
	
	
	Call fFindWord ("Created")
	Call wTypeKeys "<End><Right><Down><Down><Down><Home>"
	Sleep 1
	Call wTypeKeys "<Shift Right>"
	sVarResult(2)= fGetFieldContent()
	Call wTypeKeys "<MOD1 Home>" , 3
	
	'/// Change author to author :Test2 Test1
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	sFirstName = VorName.GetText
	sLastName  = ZuName.GetText
	VorName.SetText sFirstName1
	ZuName.SetText  sLastName1
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	'/// File / Properties / General , check "Apply user
	'/// + data" , click "delete" button
	Call fFileProperties("TabDokument")
	BenutzerdatenVerwenden.Check
	Sleep 1
	Loeschen.Click
	Sleep 1
	TabDokument.OK
	
	'/// Check if the Created field is changed in the document
	'/// It should be changed in Var
	'/// It should NOT be changed in Fix
	Call fFindWord ("Created")
	
	Call wTypeKeys "<End><Right><Down><Home>"
	Sleep 1
	
	for i = 0 to 1
		Call wTypeKeys "<Shift Right>"
		if fGetFieldContent() <> sVarResult(i) then
			Warnlog "Should get " & sVarResult(i) & " but get " & fGetFieldContent()
		end if
	
		Call wTypeKeys "<End><Right>"
		Sleep 1
		Call wTypeKeys "<Shift Right>"
		if fGetFieldContent() <> sFixResult(i) then
			Warnlog "Should get " & sFixResult(i) & " but get " & fGetFieldContent()
		end if
		Call wTypeKeys "<End><Down><Home><Left><Home>"
	next i
	
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() = sVarResult(2) then
		Warnlog "Should get " & sFixResult(2) & " but get " & fGetFieldContent()
	end if
	
	Call wTypeKeys "<End><Right>"
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sFixResult(2) then
		Warnlog "Should get " & sFixResult(2) & " but get " & fGetFieldContent()
	end if
	
	'Recover author to default
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	VorName.SetText sFirstName
	ZuName.SetText  sLastName
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocInformationModified

	Dim sTestFile     as String
	Dim sSaveasFile   as String
	Dim sFirstName    as String
	Dim sLastName     as String
	Dim sFirstName1   as String
	Dim sLastName1    as String
	Dim sVarResult(3) as String
	Dim sFixResult(3) as String
	Dim i             as Integer
	
	sFirstName1 = "Test1" : sLastName1 = "Test2"
	sTestFile   = gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw"
	sSaveasFile = ConvertPath (gOfficePath + "user\work\fields_docinfos1.sxw" )
	
	sVarResult(0) =  sFirstName1 + " " + sLastName1
	sVarResult(1) =  fGetDate1(Date)
	
	sFixResult(0) =  "Éric Savary"
	sFixResult(1) =  "08/18/2000"
	sFixResult(2) =  "11:40:11"
	
	printlog "Doc Information - Modified"
	'/// <b> Doc Information - Modified </b>
	
	if FileExists ( sSaveAsFile ) then app.kill ( sSaveAsFile )
	
	Call hNewDocument
	
	'/// Open test file fields_docinfos.sxw
	hFileOpenLocally( sTestFile )
	Call hFileSaveAsKill (sSaveAsFile)
	
	Call fFindWord ("Modified")
	Call wTypeKeys "<End><Right><Down><Down><Down><Home>"
	Sleep 1
	Call wTypeKeys "<Shift Right>"
	sVarResult(2)= fGetFieldContent()
	Call wTypeKeys "<MOD1 Home>" , 3
	
	'/// Change author to author : Test2 Test1
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	sFirstName = VorName.GetText
	sLastName  = ZuName.GetText
	VorName.SetText sFirstName1
	ZuName.SetText  sLastName1
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	'/// File / Properties / General , check "Apply user
	'/// + data" , click "delete" button
	Call fFileProperties("TabDokument")
	BenutzerdatenVerwenden.Check
	Sleep 1
	Loeschen.Click
	Sleep 1
	TabDokument.OK
	
	FileSave
	try
		Kontext "Active"
		Active.Yes
	catch
	endcatch
	Sleep 3
	
	'/// Check if the Modified field is changed in the document
	'/// It should be changed in Var
	'/// It should NOT be changed in Fix
	Call fFindWord ("Modified")
	
	Call wTypeKeys "<End><Right><Down><Home>"
	Sleep 1
	
	for i = 0 to 1
		Call wTypeKeys "<Shift Right>"
		if fGetFieldContent() <> sVarResult(i) then
			Warnlog "Should get " & sVarResult(i) & " but get " & fGetFieldContent()
		end if
	
		Call wTypeKeys "<End><Right>"
		Sleep 1
		Call wTypeKeys "<Shift Right>"
		if fGetFieldContent() <> sFixResult(i) then
			Warnlog "Should get " & sFixResult(i) & " but get " & fGetFieldContent()
		end if
		Call wTypeKeys "<End><Down><Home><Left><Home>"
	next i
	
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() = sVarResult(2) then
		Warnlog "Should get " & sFixResult(2) & " but get " & fGetFieldContent()
	end if
	
	Call wTypeKeys "<End><Right>"
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sFixResult(2) then
		Warnlog "Should get " & sFixResult(2) & " but get " & fGetFieldContent()
	end if
	
	'Recover author to default
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	VorName.SetText sFirstName
	ZuName.SetText  sLastName
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	FileClose
	Sleep 1
	Kontext "Active"
	if Active.Exists(2) then
		try
			Active.Yes
		catch
		endcatch
	end if
	
	if FileExists ( sSaveAsFile ) then app.kill ( sSaveAsFile )

endcase

'-----------------------------------------------------------------

testcase tDocInformationModifiedSavefile
	QaErrorLog "#i102792# - tDocInformationModifiedSavefile outcommented due to issue."
	goto endsub
	
	Dim sTestFile     as String
	Dim sSaveasFile   as String
	Dim sFirstName    as String
	Dim sLastName     as String
	Dim sFirstName1   as String
	Dim sLastName1    as String
	Dim sVarResult(3) as String
	Dim sFixResult(3) as String
	Dim i             as Integer
	
	sFirstName1 = "Test1" : sLastName1 = "Test2"
	sTestFile   = gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw"
	sSaveasFile = ConvertPath (gOfficePath + "user\work\fields_docinfos1.sxw" )
	
	sVarResult(0) =  sFirstName1 + " " + sLastName1
	sVarResult(1) =  fGetDate1(Date)
	
	sFixResult(0) =  "Éric Savary"
	sFixResult(1) =  "08/18/2000"
	sFixResult(2) =  "11:40:11"
	
	if FileExists ( sSaveAsFile ) then app.kill ( sSaveAsFile )
	
	printlog "Doc Information - Modified (save file)"
	'/// <b> Doc Information - Modified (save file)</b>
	
	Call hNewDocument
	
	'/// Open test file fields_docinfos.sxw
	hFileOpenLocally( sTestFile )
	
	Call fFindWord ("Modified")
	Call wTypeKeys "<End><Right><Down><Down><Down><Home>"
	Sleep 1
	Call wTypeKeys "<Shift Right>"
	sVarResult(2)= fGetFieldContent()
	Call wTypeKeys "<MOD1 Home>" , 3
	
	'/// Change author to author : Test2 Test1
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	sFirstName = VorName.GetText
	sLastName  = ZuName.GetText
	VorName.SetText sFirstName1
	ZuName.SetText  sLastName1
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	'/// Save the file as another file
	Call hFileSaveAsKill(sSaveAsFile)
	
	'/// Check if the Modified field is changed in the document
	'/// It should be changed in Var
	'/// It should NOT be changed in Fix
	Call fFindWord ("Modified")
	
	Call wTypeKeys "<End><Right><Down><Home>"
	Sleep 1
	
	for i = 0 to 1
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sVarResult(i) then
	QAErrorlog "#i41327# Should get " & sVarResult(i) & " but get " & fGetFieldContent()
	end if
	
	Call wTypeKeys "<End><Right>"
	Sleep 1
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sFixResult(i) then
	Warnlog "Should get " & sFixResult(i) & " but get " & fGetFieldContent()
	end if
	Call wTypeKeys "<End><Down><Home><Left><Home>"
	next i
	
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() = sVarResult(2) then
	QAErrorlog "#i41327# Should get " & sFixResult(2) & " but get " & fGetFieldContent()
	end if
	
	Call wTypeKeys "<End><Right>"
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sFixResult(2) then
	Warnlog "Should get " & sFixResult(2) & " but get " & fGetFieldContent()
	end if
	
	'Recover author to default
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	VorName.SetText sFirstName
	ZuName.SetText  sLastName
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	FileClose
	Sleep 1
	Kontext "Active"
	if Active.Exists(2) then
	try
	Active.Yes
	catch
	endcatch
	end if
	
	if FileExists ( sSaveAsFile ) then app.kill ( sSaveAsFile )

endcase

'-----------------------------------------------------------------

testcase tDocInformationPrinted

	Dim sTestFile     as String
	Dim sFirstName    as String
	Dim sLastName     as String
	Dim sFirstName1   as String
	Dim sLastName1    as String
	Dim sVarResult(3) as String
	Dim sFixResult(3) as String
	Dim i             as Integer
	
	sFirstName1 = "Test1" : sLastName1 = "Test2"
	sTestFile   = gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw"
	sVarResult(0) =  sFirstName1 + " " + sLastName1
	sVarResult(1) =  fGetDate1(Date)
	
	sFixResult(0) =  "Éric Savary"
	sFixResult(1) =  "08/11/2004"
	sFixResult(2) =  "11:46:06"
	
	printlog "Doc Information - Printed"
	'/// <b> Doc Information - Printed </b>
	
	Call hNewDocument
	
	'/// Open test file fields_docinfos.sxw
	hFileOpenLocally( sTestFile )
	
	Call fFindWord ("Last printed")
	Call wTypeKeys "<End><Right><Down><Down><Down><Home>"
	Call wTypeKeys "<Shift Right>"
	Sleep 1
	sVarResult(2)= fGetFieldContent()
	Call wTypeKeys "<MOD1 Home>" , 3
	
	'/// Change author to author : Test2 Test1
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	sFirstName = VorName.GetText
	sLastName  = ZuName.GetText
	VorName.SetText sFirstName1
	ZuName.SetText  sLastName1
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	'/// Print file
	FilePrint
	Kontext "Active"
	if Active.Exists then
	   	if Active.GetRT = 304 then
			Active.Ok
			QAErrorLog "No Default-Printer! The testcase isn't tested !"
			Kontext "Printing"
			Sleep 1
			Printing.Cancel
			goto NoTest
	   end if
	else
	   Kontext "Printing"
	   Sleep 1
	   Printing.OK
		kontext "active"
		if active.exists(5) then
			QaErrorLog "Error Printing..."
			Active.ok
		end if
	end if
	
	'/// Check if the Printed field is changed in the document
	'/// It should be changed in Var
	'/// It should NOT be changed in Fix
	Call fFindWord ("Last printed")
	
	Call wTypeKeys "<End><Right><Down><Home>"
	Sleep 1
	
	for i = 0 to 1
	   Call wTypeKeys "<Shift Right>"
	   if fGetFieldContent() <> sVarResult(i) then
		   Warnlog "Should get " & sVarResult(i) & " but get " & fGetFieldContent()
	   end if
	
	   Call wTypeKeys "<End><Right>"
	   Sleep 1
	   Call wTypeKeys "<Shift Right>"
	   if fGetFieldContent() <> sFixResult(i) then
		   Warnlog "Should get " & sFixResult(i) & " but get " & fGetFieldContent()
	   end if
	   Call wTypeKeys "<End><Down><Home><Left><Home>"
	next i
	
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() = sVarResult(2) then
	   Warnlog "Should NOT get " & sFixResult(2)
	end if
	
	Call wTypeKeys "<End><Right>"
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sFixResult(2) then
	   Warnlog "Should get " & sFixResult(2) & " but get " & fGetFieldContent()
	end if
	
	'Recover author to default
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	 VorName.SetText sFirstName
	 ZuName.SetText  sLastName
	 Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	NoTest:
	Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocInformationRevisionnumber

	Dim sTestFile   as String
	Dim sSaveasFile as String
	Dim sFirstName  as String
	Dim sLastName   as String
	Dim sFirstName1 as String
	Dim sLastName1  as String
	Dim sVarResult  as String
	Dim sFixResult  as String
	
	sFirstName1 = "Test1" : sLastName1 = "Test2"
	sVarResult  = "17"    : sFixResult = "4"

	sTestFile   = gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw"
	sSaveasFile = ConvertPath (gOfficePath + "user\work\tDocInformationRevisionnumber.sxw" )
	
	if FileExists ( sSaveAsFile ) then app.kill ( sSaveAsFile )
	
	printlog "Doc Information - Revision number"
	'/// <b> Doc Information - Revision number</b>
	
	Call hNewDocument
	
	'/// Open test file fields_docinfos.sxw
	hFileOpenLocally( sTestFile )
	
	'/// Change author to author : Test2 Test1
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	sFirstName = VorName.GetText
	sLastName  = ZuName.GetText
	VorName.SetText sFirstName1
	ZuName.SetText  sLastName1
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	'/// Save the file as another file
	Call hFileSaveAsKill(sSaveAsFile)
	
	'/// Press F9
	Call wTypeKeys "<F9>"
	
	'/// Save the file
	FileSave
	try
		Kontext "Active"
		Active.Yes
	catch
	endcatch
	Sleep 3
	
	'/// Check if the Revision number field is changed in the document
	'/// It should be changed in Var
	'/// It should NOT be changed in Fix
	Call fFindWord ("Version")
	
	Call wTypeKeys "<End><Right>"
	Sleep 1
	
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sVarResult then
		Warnlog "Should get " & sVarResult & " but get " & fGetFieldContent()
	end if
	
	Call wTypeKeys "<End><Right>"
	Sleep 1
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sFixResult then
		Warnlog "Should get " & sFixResult & " but get " & fGetFieldContent()
	end if
	
	'Recover author to default
	ToolsOptions
	Call hToolsOptions("STAROFFICE","USERDATA")
	VorName.SetText sFirstName
	ZuName.SetText  sLastName
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	FileClose
	Sleep 1
	Kontext "Active"
	if Active.Exists(2) then
		try
			Active.Yes
		catch
		endcatch
	end if
	
	if FileExists ( sSaveAsFile ) then app.kill ( sSaveAsFile )

endcase

'-----------------------------------------------------------------

testcase tDocInformationTotaleditingtime

	Dim sTestFile   as String
	Dim sSaveasFile as String
	Dim sVarResult  as String
	Dim sFixResult  as String
	
	sFixResult  = "00:29:05"
	sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\fields\fields_docinfos.sxw")
	sSaveasFile = ConvertPath (gOfficePath + "user\work\tDocInformationRevisionnumber.sxw" )

	if FileExists ( sSaveAsFile ) then app.kill ( sSaveAsFile )
	
	printlog "Doc Information - Tatal editing time"
	'/// <b> Doc Information - Tatal editing time </b>
	
	Call hNewDocument
	
	'/// Open test file fields_docinfos.sxw
	hFileOpenLocally( sTestFile )
	
	'/// Save the file as another file
	Call hFileSaveAsKill(sSaveAsFile)
	
	'/// Press F9
	Call wTypeKeys "<F9>"
	
	Call fFindWord ("Accessed")
	Call wTypeKeys "<End><Right>"
	Sleep 1
	Call wTypeKeys "<Shift Right>"
	sVarResult = fGetFieldContent()
	Call wTypeKeys "<MOD1 Home>" , 3
	
	'/// Save the file
	FileSave
	try
	   Kontext "Active"
	   Active.Yes
	catch
	endcatch
	Sleep 3
	
	'/// Check if the tatal editing time field is changed in the document
	'/// It should be changed in Var
	'/// It should NOT be changed in Fix
	Call fFindWord ("Accessed")
	
	Call wTypeKeys "<End><Right>"
	Sleep 1
	
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() = sVarResult then
	   Warnlog "Should NOT get " & sVarResult
	end if
	
	Call wTypeKeys "<End><Right>"
	Sleep 1
	Call wTypeKeys "<Shift Right>"
	if fGetFieldContent() <> sFixResult then
	   Warnlog "Should get " & sFixResult & " but get " & fGetFieldContent()
	end if
	
	FileClose
	Sleep 1
	Kontext "Active"
	if Active.Exists(2) then
	  try
		 Active.Yes
	  catch
	  endcatch
	end if
	
	if FileExists ( sSaveAsFile ) then app.kill ( sSaveAsFile )

endcase

'-----------------------------------------------------------------
