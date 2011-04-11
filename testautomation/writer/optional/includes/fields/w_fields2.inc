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
'* short description : Fields general test
'*
'\***********************************************************************

sub w_fields2

    Call tDialogNonModal
    Call tDocumentAuthor
    Call tDocumentAuthorFixed
    Call tDocumentChapter
    Call tDocumentDate
    Call tDocumentFilename
    Call tDocumentPage
    Call tDocumentSender
    Call tDocumentSenderFixed
    Call tDocumentStatistics
    Call tDocumentTemplate
    Call tDocumentTime

end sub

'-----------------------------------------------------------------

testcase tDialogNonModal

  Dim sTest as String

  sTest = "This is a Test"

  printlog "NON modal"
 '/// <b> NON modal </b>

  Call hNewDocument

  '/// Insert / Fields / Other / Document , test if
  '/// + the dialog is NON modal
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
       try
           Call wTypeKeys sTest
       catch
           Warnlog "Something is wrong in dialog document!"
       endcatch
   Kontext "TabDokumentFeldbefehle"
   TabDokumentFeldbefehle.Close

   Call wTypeKeys "<Shift Home>"
   EditCopy
   if GetClipboard <> sTest then
       Warnlog "The text in document isn't corrrect!"
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentAuthor
    if iSprache <> 1 then
        QaErrorLog "Currently no any other language than English. Aborting."
        exit sub
    end if

  Dim sFirstName  as String
  Dim sLastName   as String
  Dim sFirstName1 as String
  Dim sLastName1  as String
  Dim sFirstName2 as String
  Dim sLastName2  as String
  Dim sInitial1   as String
  Dim sInitial2   as String

  sFirstName1 = "ABC" : sLastName1 = "DEF"
  sFirstName2 = "XYZ" : sLastName2 = "OPQ"
  sInitial1   = "AD"  : sInitial2  = "XO"

  printlog "Insert/Fields/Other/Document / Author "
 '/// <b> Insert/Fields/Other/Document / Author </b>

  Call hNewDocument

  '/// Change author to author1 :ABC DEF
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     sFirstName = VorName.GetText
     sLastName  = ZuName.GetText
     Kuerzel.SetText ""
     VorName.SetText sFirstName1
     ZuName.SetText  sLastName1
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  '/// Insert/Fields/Other/Document / Author (Name, Initial)
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
     Feldtyp.Select fGetType("Author")
     Sleep 1
     Auswahl.Select 1
     Sleep 1
     Einfuegen.Click
     sleep 1
     Call wTypeKeys "<Return>"
     Kontext "TabDokumentFeldbefehle"
     Auswahl.Select 2
     Sleep 1
     Einfuegen.Click
   TabDokumentFeldbefehle.Close

  '/// Check if the name is inserted correctly
   Call wTypeKeys "<MOD1 Home><Shift End>"
   EditCopy
   if GetClipboard <> (sFirstName1 + " " + sLastName1) then
       Warnlog "Should get " & sFirstName1 & " " & sLastName1 & " but get " & GetClipboard
   end if

  '/// Check if the Initial is inserted correctly
   Call wTypeKeys "<Down>"
   Call wTypeKeys "<End><Shift Home>"
   EditCopy
   if GetClipboard <> sInitial1 then
       Warnlog "Should get " & sInitial1 & " but get " & GetClipboard
   end if

  '/// Change author to author1 :XYZ OPQ
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     VorName.SetText sFirstName2
     ZuName.SetText  sLastName2
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep 2

  '/// Press F9
   Call wTypeKeys "<F9>"
   Sleep 3

  '/// Check if the name is changed
   Call wTypeKeys "<MOD1 Home><Shift End>"
   EditCopy
   if GetClipboard <> (sFirstName2 + " " + sLastName2) then
       Warnlog "Should get " & sFirstName2 & " " & sLastName2 & " but get " & GetClipboard
   end if

  '/// Check if the Initial is changed
   Call wTypeKeys "<Down><End><Shift Home>"
   EditCopy
   if GetClipboard <> sInitial2 then
       Warnlog "Should get " & sInitial2 & " but get " & GetClipboard
   end if

  'Change author to default
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     VorName.SetText sFirstName
     ZuName.SetText  sLastName
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentAuthorFixed
    if iSprache <> 1 then
        QaErrorLog "Currently no any other language than English. Aborting."
        exit sub
    end if

  Dim sFirstName  as String
  Dim sLastName   as String
  Dim sFirstName1 as String
  Dim sLastName1  as String
  Dim sFirstName2 as String
  Dim sLastName2  as String
  Dim sInitial1   as String

  sFirstName1 = "ABC" : sLastName1 = "DEF"
  sFirstName2 = "XYZ" : sLastName2 = "OPQ"
  sInitial1   = "AD"

  printlog "Insert/Fields/Other/Document / Author - Fixed"
 '/// <b> Insert/Fields/Other/Document / Author - Fixed</b>

  Call hNewDocument

  '/// Change author to author1 :ABC DEF
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     sFirstName = VorName.GetText
     sLastName  = ZuName.GetText
     Kuerzel.SetText ""
     VorName.SetText sFirstName1
     ZuName.SetText  sLastName1
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  '/// Insert/Fields/Other/Document / Author (Name, Initial)
  '/// + check 'Fixed content'
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
     Feldtyp.Select fGetType("Author")
     Sleep 1
     Auswahl.Select 1
     Sleep 1
     InhaltFix.Check
     Einfuegen.Click
     sleep 1
     Call wTypeKeys "<Return>"
     Kontext "TabDokumentFeldbefehle"
     Auswahl.Select 2
     Sleep 1
     InhaltFix.Check
     Einfuegen.Click
   TabDokumentFeldbefehle.Close

  '/// Check if the name is inserted correctly
   Call wTypeKeys "<MOD1 Home><Shift End>"
   EditCopy
   if GetClipboard <> (sFirstName1 + " " + sLastName1) then
       Warnlog "Should get " & sFirstName1 & " " & sLastName1 & " but get " & GetClipboard
   end if

  '/// Check if the Initial is inserted correctly
   Call wTypeKeys "<Down>"
   Call wTypeKeys "<End><Shift Home>"
   EditCopy
   if GetClipboard <> sInitial1 then
       Warnlog "Should get " & sInitial1 & " but get " & GetClipboard
   end if

  '/// Change author to author1 :XYZ OPQ
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     VorName.SetText sFirstName2
     ZuName.SetText  sLastName2
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep 2

  '/// Press F9
   Call wTypeKeys "<F9>"
   Sleep 3

  '/// Check if the name is changed
   Call wTypeKeys "<MOD1 Home><Shift End>"
   EditCopy
   if GetClipboard <> (sFirstName1 + " " + sLastName1) then
       Warnlog "Still should get " & sFirstName1 & " " & sLastName1 & " but get " & GetClipboard
   end if

  '/// Check if the Initial is changed
   Call wTypeKeys "<Down><End><Shift Home>"
   EditCopy
   if GetClipboard <> sInitial1 then
       Warnlog "Still should get " & sInitial1 & " but get " & GetClipboard
   end if

  'Change author to default
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     VorName.SetText sFirstName
     ZuName.SetText  sLastName
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentChapter

  Dim sTestFile  as String
  Dim sChapter() as String
  Dim i          as Integer

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\fields\chapterTest.sxw")

  sChapter  = Array( _
                    "My House"             , _
                    "Before2After"         , _
                    "Before2AfterMy House" , _
                    "2"                    , _
                   )

  printlog "Insert/Fields/Other/Document / Chapter"
 '/// <b> Insert/Fields/Other/Document / Chapter </b>

  Call hNewDocument

  '/// Open test file chapterTest.sxw
   hFileOpenLocally( sTestFile )


   Call wTypeKeys "<MOD1 End><Return>"

  '/// Insert/Fields/Other/Document / Chapter
  '/// Insert Chapter name, Chapter number,
  '/// Chapter number and name , Chapter number without separator
  '/// Check if the insertion is correct
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
     Feldtyp.Select fGetType("Chapter")
     for i = 0 to UBound(sChapter)
         Kontext "TabDokumentFeldbefehle"
         Formatliste.Select i+1
         Sleep 1
         Einfuegen.Click
         sleep 1
         Call wTypeKeys "<F9>"
         Sleep 2
         Call wTypeKeys "<Shift Home>"
         Sleep 1
         if fGetFieldContent() <> sChapter(i) then
             QaErrorLog i &"#94997# Should get " & sChapter(i) & " but get " & fGetFieldContent()
         end if
         Call wTypeKeys "<Delete>"
     next i
     Kontext "TabDokumentFeldbefehle"
   TabDokumentFeldbefehle.Close

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentDate

    Dim sDate as String

    printlog "Insert/Fields/Other/Document / Date"
    '/// <b> Insert/Fields/Other/Document / Date </b>

    Call hNewDocument

    '/// Insert/Fields/Other/Document / Date
    '/// Check if the insertion is correct
    Call fInsertFieldsOther("TabDokumentFeldbefehle")
    Feldtyp.Select fGetType("Date")
    Auswahl.Select 1
    Einfuegen.Click
    Kontext "TabDokumentFeldbefehle"
    TabDokumentFeldbefehle.Close

    Call wTypeKeys "<Shift Left>"
    EditCopy
    Sleep 1
    if GetClipboard = "" then
        Warnlog "Field returns an empty string"
    end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentFilename

    if ( bAsianLan ) then
        QaErrorLog "No testing with Asian language. Aborting."
        goto endsub
    end if

    Dim  sTestFile     as String
    Dim  sFilename(4)  as String
    Dim  i             as Integer

    sTestFile = convertpath( gTesttoolPath & "writer\optional\input\fields\pageTest.sxw" )

    sFilename(0) = "pageTest.sxw"
    sFilename(1) = "pageTest"
    sFilename(2) = hGetWorkPath()
    sFilename(3) = hFileGetLocalPath( sTestFile )

    printlog "Insert/Fields/Other/Document / File name"

    Call hNewDocument

    '/// Open test file pageTest.sxw
    hFileOpenLocally( sTestFile )

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<End><Return>"

    printlog "Insert/Fields/Other/Document / File name"
    printlog "Insert File name, File name without extension , Path , Path/File name"
    printlog "Check if the insertion is correct"
    Call fInsertFieldsOther("TabDokumentFeldbefehle")
    Feldtyp.Select fGetType("File name")
    for i = 0 to 3
        Kontext "TabDokumentFeldbefehle"
        Formatliste.Select i+1
        Sleep 1
        Einfuegen.Click
        sleep 1
        Call wTypeKeys "<F9>"
        Sleep 2
        Call wTypeKeys "<Shift LEFT>"  'HOME
        Sleep 1
        if fGetFieldContent() <> sFilename(i) then
                Warnlog i &" - Should get " & sFilename(i) & " but get " & fGetFieldContent()
        end if
        Call wTypeKeys "<Delete>"
    next i
    Kontext "TabDokumentFeldbefehle"
    TabDokumentFeldbefehle.Close

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentPage

  Dim sTestFile as String
  Dim sPage()   as String
  Dim i         as Integer

  sPage = Array( _
                 "2"         , _
                 "1"         , _
                 "3"         , _
                )

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\fields\pageTest.sxw")

  printlog "Insert/Fields/Other/Document / Page"
 '/// <b> Insert/Fields/Other/Document / Page </b>

  Call hNewDocument

  '/// Open test file pageTest.sxw which includes 3 pages
   hFileOpenLocally( sTestFile )


  '/// Go to the 2nd page
   Call wTypeKeys "<MOD1 Home>"
   Call fFindWord ("Sec")
   Call wTypeKeys "<End><Return>"

  '/// Insert/Fields/Other/Document / Page
  '/// Insert Page numbers, Privious page , Next page
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
     Feldtyp.Select fGetType("Page")
     for i = 0 to UBound(sPage)
         Kontext "TabDokumentFeldbefehle"
         Auswahl.Select i+1
         Sleep 1
         Einfuegen.Click
         sleep 1
         Call wTypeKeys "<F9>"
         Sleep 2
         Call wTypeKeys "<Shift Home>"
         Sleep 1
         EditCopy
         if GetClipboard <> sPage(i) then
             Warnlog i &" - Should get " & sPage(i) & " but get " & GetClipboard
         end if
         Call wTypeKeys "<Delete>"
     next i
     Kontext "TabDokumentFeldbefehle"
   TabDokumentFeldbefehle.Close

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentSender
    if iSprache <> 1 then
        QaErrorLog "Currently no testing with other languages than English. (ID's are not the same) Aborting."
        exit sub
    end if

  Dim aUserData(14) as String
  Dim aUserData1() as String
  Dim aUserData2() as String

  Dim i as Integer

  aUserData1 = Array( _
                      "Com1"       , _
                      "ABC"        , _
                      "DEF"        , _
                      "AD"         , _
                      "Str1"       , _
                      "Country1"   , _
                      "123"        , _
                      "City1"      , _
                      "Title1"     , _
                      "Posi1"      , _
                      "123"        , _
                      "465"        , _
                      "789"        , _
                      "AD@a.a"     , _
                      "State1"     , _
                     )
  aUserData2 = Array( _
                      "Com2"       , _
                      "XYZ"        , _
                      "OPQ"        , _
                      "XO"         , _
                      "Str2"       , _
                      "Country2"   , _
                      "321"        , _
                      "City2"      , _
                      "Title2"     , _
                      "Posi2"      , _
                      "222"        , _
                      "654"        , _
                      "987"        , _
                      "XO@b.b"     , _
                      "State2"     , _
                     )

  printlog "Insert/Fields/Other/Document / Sender"
 '/// <b> Insert/Fields/Other/Document / Sender </b>

  Call hNewDocument

  '/// Change user data
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     aUserData(0) = Firma.GetText
     aUserData(1) = VorName.GetText
     aUserData(2) = ZuName.GetText
     aUserData(3) = Strasse.GetText
     aUserData(4) = City.GetText
     aUserData(5) = State.GetText
     aUserData(6) = Zip.GetText
     aUserData(7) = Land.GetText
     aUserData(8) = Titel.GetText
     aUserData(9) = Position.GetText
     aUserData(10) = TelPriv.GetText
     aUserData(11) = TelGe.GetText
     aUserData(12) = Fax.GetText
     aUserData(13) = EMail.GetText

     Kuerzel.SetText   ""
     Firma.SetText     aUserData1(0)
     VorName.SetText   aUserData1(1)
     ZuName.SetText    aUserData1(2)
     Strasse.SetText   aUserData1(4)
     City.SetText      aUserData1(7)
     State.SetText     aUserData1(14)
     Zip.SetText       aUserData1(6)
     Land.SetText      aUserData1(5)
     Titel.SetText     aUserData1(8)
     Position.SetText  aUserData1(9)
     TelPriv.SetText   aUserData1(10)
     TelGe.SetText     aUserData1(11)
     Fax.SetText       aUserData1(12)
     EMail.SetText     aUserData1(13)

     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  '/// Insert/Fields/Other/Document / Sender
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
     Feldtyp.Select fGetType("Sender")
     for i= 0 to UBound(aUserData1)
         Kontext "TabDokumentFeldbefehle"
         Auswahl.Select i+1
         Sleep 1
         Einfuegen.Click
         sleep 1
         Call wTypeKeys "<Return>"
     next i
     Kontext "TabDokumentFeldbefehle"
   TabDokumentFeldbefehle.Close

  '/// Check if the configuration is inserted correctly
   Call wTypeKeys "<MOD1 Home>"
   for i= 0 to UBound(aUserData1)
       Call wTypeKeys "<Shift End>"
       if fGetFieldContent() <> aUserData1(i) then
           Warnlog "i32872 1.Should get " & aUserData1(i)& " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Home>"
   next i

  '/// Change author to author1 :XYZ OPQ
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     Kuerzel.SetText   ""
     Firma.SetText     aUserData2(0)
     VorName.SetText   aUserData2(1)
     ZuName.SetText    aUserData2(2)
     Strasse.SetText   aUserData2(4)
     City.SetText      aUserData2(7)
     State.SetText     aUserData2(14)
     Zip.SetText       aUserData2(6)
     Land.SetText      aUserData2(5)
     Titel.SetText     aUserData2(8)
     Position.SetText  aUserData2(9)
     TelPriv.SetText   aUserData2(10)
     TelGe.SetText     aUserData2(11)
     Fax.SetText       aUserData2(12)
     EMail.SetText     aUserData2(13)

     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep 2

  '/// Press F9
   Call wTypeKeys "<F9>"
   Sleep 3

  '/// Check if the configuration is changed
   Call wTypeKeys "<MOD1 Home>"
   for i= 0 to UBound(aUserData1)
       Call wTypeKeys "<Shift End>"
       if fGetFieldContent() <> aUserData2(i) then
           Warnlog "i32872  2.Should get " & aUserData2(i)& " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Home>"
   next i

  'Change author to default
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     Firma.SetText     aUserData(0)
     VorName.SetText   aUserData(1)
     ZuName.SetText    aUserData(2)
     Strasse.SetText   aUserData(3)
     City.SetText      aUserData(4)
     State.SetText     aUserData(5)
     Zip.SetText       aUserData(6)
     Land.SetText      aUserData(7)
     Titel.SetText     aUserData(8)
     Position.SetText  aUserData(9)
     TelPriv.SetText   aUserData(10)
     TelGe.SetText     aUserData(11)
     Fax.SetText       aUserData(12)
     EMail.SetText     aUserData(13)

     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentSenderFixed
    if iSprache <> 1 then
        QaErrorLog "Currently no testing with other languages than English. (ID's are not the same) Aborting."
        exit sub
    end if

  Dim aUserData(16) as String
  Dim aUserData1()  as String
  Dim aUserData2()  as String

  Dim i as Integer

  aUserData1 = Array( _
                      "Com1"       , _
                      "ABC"        , _
                      "DEF"        , _
                      "AD"         , _
                      "Str1"       , _
                      "Country1"   , _
                      "123"        , _
                      "City1"      , _
                      "Title1"     , _
                      "Posi1"      , _
                      "123"        , _
                      "465"        , _
                      "789"        , _
                      "AD@a.a"     , _
                      "State1"     , _
                     )

  aUserData2 = Array( _
                      "Com2"       , _
                      "XYZ"        , _
                      "OPQ"        , _
                      "XO"         , _
                      "Str2"       , _
                      "Country2"   , _
                      "321"        , _
                      "City2"      , _
                      "Title2"     , _
                      "Posi2"      , _
                      "222"        , _
                      "654"        , _
                      "987"        , _
                      "XO@b.b"     , _
                      "State2"     , _
                     )


  printlog "Insert/Fields/Other/Document / Sender - Fixed"
 '/// <b> Insert/Fields/Other/Document / Sender - Fixed</b>

  Call hNewDocument

  '/// Change user data
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     aUserData(0) = Firma.GetText
     aUserData(1) = VorName.GetText
     aUserData(2) = ZuName.GetText
     aUserData(3) = Strasse.GetText
     aUserData(4) = City.GetText
     aUserData(5) = State.GetText
     aUserData(6) = Zip.GetText
     aUserData(7) = Land.GetText
     aUserData(8) = Titel.GetText
     aUserData(9) = Position.GetText
     aUserData(10) = TelPriv.GetText
     aUserData(11) = TelGe.GetText
     aUserData(12) = Fax.GetText
     aUserData(13) = EMail.GetText

     Kuerzel.SetText   ""
     Firma.SetText     aUserData1(0)
     VorName.SetText   aUserData1(1)
     ZuName.SetText    aUserData1(2)
     Strasse.SetText   aUserData1(4)
     City.SetText      aUserData1(7)
     State.SetText     aUserData1(14)
     Zip.SetText       aUserData1(6)
     Land.SetText      aUserData1(5)
     Titel.SetText     aUserData1(8)
     Position.SetText  aUserData1(9)
     TelPriv.SetText   aUserData1(10)
     TelGe.SetText     aUserData1(11)
     Fax.SetText       aUserData1(12)
     EMail.SetText     aUserData1(13)

     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  '/// Insert/Fields/Other/Document / Sender , check 'Fixed content'
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
     Feldtyp.Select fGetType("Sender")
     for i= 0 to UBound(aUserData1)
         Kontext "TabDokumentFeldbefehle"
         Auswahl.Select i+1
         Sleep 1
         InhaltFix.Check
         Einfuegen.Click
         sleep 1
         Call wTypeKeys "<Return>"
     next i
     Kontext "TabDokumentFeldbefehle"
   TabDokumentFeldbefehle.Close

  '/// Check if the configuration is inserted correctly
   Call wTypeKeys "<MOD1 Home>"
   for i= 0 to UBound(aUserData1)
       Call wTypeKeys "<Shift End>"
       if fGetFieldContent() <> aUserData1(i) then
           Warnlog "i32872  1.Should get " & aUserData1(i)& " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Home>"
   next i

  '/// Change author to author1 :XYZ OPQ
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     Kuerzel.SetText   ""
     Firma.SetText     aUserData2(0)
     VorName.SetText   aUserData2(1)
     ZuName.SetText    aUserData2(2)
     Strasse.SetText   aUserData2(4)
     City.SetText      aUserData2(7)
     State.SetText     aUserData2(14)
     Zip.SetText       aUserData2(6)
     Land.SetText      aUserData2(5)
     Titel.SetText     aUserData2(8)
     Position.SetText  aUserData2(9)
     TelPriv.SetText   aUserData2(10)
     TelGe.SetText     aUserData2(11)
     Fax.SetText       aUserData2(12)
     EMail.SetText     aUserData2(13)

     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep 2

  '/// Press F9
   Call wTypeKeys "<F9>"
   Sleep 3

  '/// Check if the configuration is changed
   Call wTypeKeys "<MOD1 Home>"
   for i= 0 to UBound(aUserData1)
       Call wTypeKeys "<Shift End>"
       if fGetFieldContent() <> aUserData1(i) then
           Warnlog "i32872  2.Should get " & aUserData1(i)& " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Home>"
   next i

  'Change author to default
   ToolsOptions
   Call hToolsOptions("STAROFFICE","USERDATA")
     Firma.SetText     aUserData(0)
     VorName.SetText   aUserData(1)
     ZuName.SetText    aUserData(2)
     Strasse.SetText   aUserData(3)
     City.SetText      aUserData(4)
     State.SetText     aUserData(5)
     Zip.SetText       aUserData(6)
     Land.SetText      aUserData(7)
     Titel.SetText     aUserData(8)
     Position.SetText  aUserData(9)
     TelPriv.SetText   aUserData(10)
     TelGe.SetText     aUserData(11)
     Fax.SetText       aUserData(12)
     EMail.SetText     aUserData(13)

     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentStatistics

  Dim sTestFile     as String
  Dim sStatistics() as String
  Dim sType as string
  Dim i             as Integer

  sStatistics = Array( "2", "2", "3", "13", "2", "2", "1" )
  sTestFile= Convertpath (gTesttoolpath + "writer\optional\input\fields\statisticsTest.sxw")

  printlog "Insert/Fields/Other/Document / Statistics"
 '/// <b> Insert/Fields/Other/Document / Statistics </b>

  '/// Open test file statisticsTest.sxw which includes
  '/// 2 tables , 2 pictures , 22 paragraph , 1 object ,
  '/// 2 words , 12 characters and 2 pages
   hFileOpenLocally( sTestFile )

   Call wTypeKeys "<MOD1 Home><Down>"

  '/// Insert/Fields/Other/Document / Statistics
  '/// Insert Pages, Paragraphs , Words , Characters , Tables, Graphics , Objects.
  '/// Check if the insertion is correct
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
     Feldtyp.Select fGetType("Statistics")
     sType = Auswahl.GetSelText
     for i = 0 to UBound(sStatistics)
         Kontext "TabDokumentFeldbefehle"
         Auswahl.Select i+1
         Sleep 1
         Einfuegen.Click
         sleep 1
         Call wTypeKeys "<F9>"
         Sleep 2
         Call wTypeKeys "<Shift Home>"
         Sleep 1
         EditCopy
         if GetClipboard <> sStatistics(i) then
             Warnlog "- " & sType & ": Should get " & sStatistics(i) & " but get " & GetClipboard
         end if
         Call wTypeKeys "<Delete>"
     next i
     Kontext "TabDokumentFeldbefehle"
   TabDokumentFeldbefehle.Close

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tDocumentTemplate

    Dim sTestFile     as String
    Dim sDirectory    as String
    Dim oDirectory    as String
    Dim sLanguage     as String
    Dim sFormat()     as String
    Dim sResult(6)    as String
    Dim i             as Integer

    sLanguage    = fGetDirLanguage()
    sTestFile = convertpath( gTesttoolpath + "writer\optional\input\fields\diploma.ott" )

    sResult(0)   = fGetCategory()
    sResult(1)   = "diploma.ott"
    sResult(2)   = "diploma"
    sResult(3)   = convertpath( gOfficePath & "user/work" )
    sResult(4)   = hGetWorkFile( sResult( 1 ) )
    sResult(5)   = fGetStyle()

    printlog "Insert/Fields/Other/Document / Template"
    '/// <b> Insert/Fields/Other/Document / Template </b>

    '/// Open test file diploma.ott
    hFileOpenLocally( sTestFile )

    Call wTypeKeys "<MOD1 Home>"

    '/// Insert/Fields/Other/Document / Template
    '/// Insert Category , File name , File name without extension ,
    '/// + Path , Path/File name , Style
    '/// Check if the insertion is correct
    Call fInsertFieldsOther("TabDokumentFeldbefehle")
    Feldtyp.Select fGetType("Templates")
    for i = 1 to Formatliste.GetItemCount
        Kontext "TabDokumentFeldbefehle"
        Formatliste.Select i
        Sleep 1
        Einfuegen.Click
        sleep 1
        Call wTypeKeys "<F9>"
        Sleep 2
        Call wTypeKeys "<Shift Left>"
        Sleep 1
        EditCopy
        if GetClipboard <> sResult(i-1) then
            select case i-1
                case 0
                    Warnlog "Field 'Templates->Category' is empty"
                case else
                    Warnlog " - Should get " & sResult(i-1) & " but get " & GetClipboard
            end select
        end if
        Call wTypeKeys "<Delete>"
    next i
    Kontext "TabDokumentFeldbefehle"
    TabDokumentFeldbefehle.Close

    hFileCloseAll()
endcase

'-----------------------------------------------------------------

testcase tDocumentTime

    Dim sTime1 as String
    Dim sTime2 as String

  printlog "Insert/Fields/Other/Document / Time"
 '/// <b> Insert/Fields/Other/Document / Time </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Document / Time
  '/// Insert a fixed time and a time .
  '/// Check if the insertion is correct
   Call fInsertFieldsOther("TabDokumentFeldbefehle")
     Feldtyp.Select fGetType("Time")
     Auswahl.Select 1  'Time (Fixed)
     Sleep 1
     Einfuegen.Click
     sleep 1
     Call wTypeKeys "<Return>"

     Kontext "TabDokumentFeldbefehle"
     Auswahl.Select 1  'Time
     Sleep 1
     Einfuegen.Click
     sleep 1
     Kontext "TabDokumentFeldbefehle"
   TabDokumentFeldbefehle.Close

   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   sTime1 = GetClipboard

   Call wTypeKeys "<Down><Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   sTime2 = GetClipboard

   Sleep 5
   Call wTypeKeys "<F9>"

   'Check fixed time
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if sTime1 <> GetClipboard then
       Warnlog "Time should NOT change !"
   end if

   'Check time
   Call wTypeKeys "<Down><Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if sTime2 <> GetClipboard then
       Warnlog "Time should change !"
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------
