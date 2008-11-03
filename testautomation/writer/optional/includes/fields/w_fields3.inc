'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_fields3.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:26:51 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : fredrik.haegg@sun.com
'*
'* short description : Fields - References Test
'*
'\***********************************************************************

sub w_fields3

    Call tFieldsReferences       'Insert/Cross-reference
    Call tSetReferences_1        'Set Reference - Cursor at any position
    Call tSetReferences_2        'Set Reference - Select some text
    Call tInsertReference_1      'Insert Reference - Page
    Call tInsertReference_2      'Insert Reference - Chapter
    Call tInsertReference_3      'Insert Reference - Reference
    Call tInsertReference_4      'Insert Reference - Above/Below
    Call tInsertReference_5      'Insert Reference - As Page Style
    Call tInsertReference_6      'Insert Footnote
    Call tInsertReference_7      'Insert Endnote
    Call tInsertReference_8_1    'Insert Caption - Above
    Call tInsertReference_8_2    'Insert Caption - Below
    Call tInsertBookmark_1       'Insert Bookmark - Page
    Call tInsertBookmark_2       'Insert Bookmark - Chapter
    Call tInsertBookmark_3       'Insert Bookmark - Reference
    Call tInsertBookmark_4       'Insert Bookmark - Above/Below
    Call tInsertBookmark_5       'Insert Bookmark - As Page Style

end sub

testcase tFieldsReferences

  printlog "Insert/Cross-reference"
 '/// <b> Insert/Cross-reference </b>

 '/// Open a new document
  Call hNewDocument

  '/// Insert/Cross-reference
   InsertReferenceField
   Sleep 2
   Kontext "TabReferenzen"
   if TabReferenzen.Exists then
       TabReferenzen.Close
   else
       Warnlog "Tabpage References should be as the actived tabpage!"
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tSetReferences_1

  Dim  sReference  as String
  Dim  sTestFile   as String

  sReference = "myTest"
  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\fields\pageTest.sxw")

  printlog "Set Reference - Cursor at any position"
 '/// <b> Set Reference - Cursor at any position </b>

  Call hNewDocument

  '/// Open test file pageTest.sxw
   Call hFileOpen(sTestFile)
   Call sMakeReadOnlyDocumentEditable

  '/// Set curson to the 2nd page
   Call wTypeKeys "<MOD1 Home>"
   Call fFindWord ("Sec")
   Call wTypeKeys "<End><Return>"

  '/// Insert/Cross-reference , choose "Set Reference"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 1
     Sleep 1
     Namefeld.SetText  sReference
     Einfuegen.Click
   TabReferenzen.Close

   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 2
     Sleep 1
     if Namefeld.GetText <> sReference then
         Warnlog "Reference can't be set correctly!"
     end if
   TabReferenzen.Close

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tSetReferences_2

  Dim  sReference  as String

  sReference = "myTest"

  printlog "Set Reference - Select some text"
 '/// <b> Set Reference - Select some text </b>

 '/// Open a new document
  Call hNewDocument

  '/// Input some text and select them
   Call wTypeKeys "ABCD"
   Call wTypeKeys "<Shift Home>"

  '/// Insert/Cross-reference , choose "Set Reference"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 1
     Sleep 1
     Namefeld.SetText  sReference
     Einfuegen.Click
   TabReferenzen.Close

   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 2
     Sleep 1
     if Namefeld.GetText <> sReference then
         Warnlog "Reference can't be set correctly!"
     end if
   TabReferenzen.Close

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_1

  Dim  sReference  as String
  Dim  sTestFile   as String

  sReference = "myTest"
  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\fields\pageTest.sxw")

  printlog "Insert Reference - Page"
 '/// <b> Insert Reference - Page </b>

  Call hNewDocument

  '/// Open test file pageTest.sxw
   Call hFileOpen(sTestFile)
   Call sMakeReadOnlyDocumentEditable

  '/// Set curson to the 2nd page
   Call wTypeKeys "<MOD1 Home>"
   Call fFindWord ("Sec")
   Call wTypeKeys "<End><Return>"

  '/// Insert/Cross-reference , choose "Set Reference"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 1
     Sleep 1
     Namefeld.SetText  sReference
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<Return>" , 2

  '/// Insert/Cross-reference , choose "Insert Reference" and "Page"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 2
     Sleep 1
     Auswahl.Select  sReference
     Sleep 1
     Formatliste.Select 1
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent( ) <> "2" then
       Warnlog "Should get 2 , but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_2

  Dim  sReference  as String
  Dim  sTestFile   as String

  sReference = "myTest"
  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\fields\chapterTest.sxw")

  printlog "Insert Reference - Chapter"
 '/// <b> Insert Reference - Chapter </b>

  Call hNewDocument

  '/// Open test file chapterTest.sxw
   Call hFileOpen(sTestFile)
   Call sMakeReadOnlyDocumentEditable

  '/// Set curson to the 2nd line
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Down><Home>"

  '/// Insert/Cross-reference , choose "Set Reference"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 1
     Sleep 1
     Namefeld.SetText  sReference
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<End>"
   Call wTypeKeys "<Return>" , 2

  '/// Insert/Cross-reference , choose "Insert Reference" and "Chapter"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 2
     Sleep 1
     Auswahl.Select  sReference
     Sleep 1
     Formatliste.Select 2
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent( ) <> "2" then
       Warnlog "#i40625# Should get 2 , but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_3

  Dim  sReference      as String
  Dim  sReferenceName  as String

  sReference     = "est !"
  sReferenceName = "myTest"

  printlog "Insert Reference - Reference"
 '/// <b> Insert Reference - Reference</b>

 '/// Open a new document
  Call hNewDocument

  '/// Input some thing in the document and press return
   Call wTypeKeys "This is a test !"
   Call wTypeKeys "<Return>"

  '/// Select some text
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Shift Left>" , 5

  '/// Insert/Cross-reference , choose "Set Reference"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 1
     Sleep 1
     Namefeld.SetText  sReferenceName
     Einfuegen.Click
   TabReferenzen.Close

  '/// set focus to 2nd line
   Call wTypeKeys "<Down>"

  '/// Insert/Cross-reference , choose "Insert Reference" and "Reference"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 2
     Sleep 1
     Auswahl.Select  sReferenceName
     Sleep 1
     Formatliste.Select 3
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent() <> sReference then
       Warnlog "Should get " & sReference & " but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_4
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

  Dim  sReference      as String
  Dim  sAbove , sBelow as String

  sReference = "myTest"
  sAbove = "above" : sBelow = "below"

  printlog "Insert Reference - Above/Below"
 '/// <b> Insert Reference - Above/Below</b>

 '/// Open a new document
  Call hNewDocument

  '/// Press Return first , then input some text in
  '/// + the document , press return again
   Call wTypeKeys "<Return>"
   Call wTypeKeys "This is a test !"
   Call wTypeKeys "<Return>"

  '/// Insert a reference before "This is ..."
   Call wTypeKeys "<Up><Home>"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 1
     Sleep 1
     Namefeld.SetText  sReference
     Einfuegen.Click
   TabReferenzen.Close

  '/// set focus to 3rd line
   Call wTypeKeys "<Down>"

  '/// Insert/Cross-reference , choose "Insert Reference" and "above/below"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 2
     Sleep 1
     Auswahl.Select  sReference
     Sleep 1
     Formatliste.Select 4
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent() <> sAbove then 'Check systemlanguage / Office-Language - if other than 01 then ignore/QaErrorLog!
       Warnlog "Should get " & sAbove & " but get " & fGetFieldContent()
   end if

  '/// set focus to 1st line
   Call wTypeKeys "<MOD1 Home>"

  '/// Insert/Cross-reference , choose "Insert Reference" and "above/below"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 2
     Sleep 1
     Auswahl.Select  sReference
     Sleep 1
     Formatliste.Select 4
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent() <> sBelow then
       Warnlog "Should get " & sBelow & " but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_5

  Dim  sReference  as String
  Dim  sTestSytle  as String

  sReference = "myTest"
  sTestSytle = "A"

  printlog "Insert Reference - As Page Style"
 '/// <b> Insert Reference - As Page Style </b>

 '/// Open a new document
  Call hNewDocument

  '/// Fomrat/page / page , set Format as "A , B , C ..."
   Call fFormatPageWriter("TabSeite")
     Numerierung.Select 1
   TabSeite.OK

  '/// Input some thing in the document , press return
   Call wTypeKeys "This is a test !"
   Call wTypeKeys "<Return>"

  '/// Insert a reference before "This is ..."
   Call wTypeKeys "<Up><Home>"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 1
     Sleep 1
     Namefeld.SetText  sReference
     Einfuegen.Click
   TabReferenzen.Close

  '/// set focus to 2nd line
   Call wTypeKeys "<Down>"

  '/// Insert/Cross-reference , choose "Insert Reference" and "As Page Style"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 2
     Sleep 1
     Auswahl.Select  sReference
     Sleep 1
     Formatliste.Select 5
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent() <> sTestSytle then
       Warnlog "Should get " & sTestSytle & " but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_6

  Dim  sFootnote   as String

  sFootnote = "Test"

  printlog "Insert Footnote"
 '/// <b> Insert Footnote </b>

 '/// Open a new document
  Call hNewDocument

  '/// Insert/Footnote
   InsertFootnote
     Kontext "FussnoteEinfuegen"
    'check footnote
     Fussnote.Check
     Zeichen.Check
     ZeichenText.SetText sFootnote
   FussnoteEinfuegen.OK

  '/// Check if footnote is inserted correctly
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   if fGetFieldContent() <> sFootnote then
       Warnlog "Should get " & sFootnote & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_7

  Dim  sEndnote   as String

  sEndnote = "Test"

  printlog "Insert Endnote"
 '/// <b> Insert Endnote </b>

 '/// Open a new document
  Call hNewDocument

  '/// Insert Endnote
   InsertFootnote
     Kontext "FussnoteEinfuegen"
    'check endnote
     Endnote.Check
     Zeichen.Check
     ZeichenText.SetText sEndnote
   FussnoteEinfuegen.OK

  '/// Check if footnote is inserted correctly
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   if fGetFieldContent() <> sEndnote then
       Warnlog "Should get " & sEndnote & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_8_1

  Dim sCategory , sCaption , sSeparator as String

  sCategory = "A" : sCaption = "B" : sSeparator = "*"

  printlog "Insert Caption - Above"
 '/// <b> Insert Caption - Above </b>

 '/// Open a new document
  Call hNewDocument

  '/// Insert a frame
   Call wInsertFrame

  '/// Insert / Caption , set Category as A , set Caption as B , Set Seperator as *
   InsertCaption
   Kontext "Beschriftung"
     Kategorie.SetText  sCategory
     Separator.SetText  sSeparator
     BeschriftungsText.SetText sCaption
     Position.Select 1
   Beschriftung.OK
   Sleep 1

  '/// Check if the insertion is correct
   Call wTypeKeys "<Return>"
   Call wTypeKeys "<Up><Home><Shift End>"
   EditCopy

   if fGetFieldContent() <> sCategory & " 1" & sSeparator & sCaption  then
       Warnlog "Should get " & sCategory & " 1" & sSeparator & sCaption & " but get " & fGetFieldContent()
   end if

   Call wTypeKeys "<Down>"
   Call wTypeKeys "<Home><Shift End>"
   try
       EditCopy
       Warnlog "Should NOT get anything !"
   catch
   endcatch

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertReference_8_2

  Dim sCategory , sCaption , sSeparator as String

  sCategory = "A" : sCaption = "B" : sSeparator = "*"

  printlog "Insert Caption - Below"
 '/// <b> Insert Caption - Below </b>

 '/// Open a new document
  Call hNewDocument

  '/// Insert a frame
   Call wInsertFrame

  '/// Insert / Caption , set Category as A , set Caption as B,
  '/// + select position as Above
   InsertCaption
   Kontext "Beschriftung"
     Kategorie.SetText  sCategory
     Separator.SetText  sSeparator
     BeschriftungsText.SetText sCaption
     Position.Select 2
   Beschriftung.OK
   Sleep 1

  '/// Check if the insertion is correct
   Call wTypeKeys "<Return>"
   Call wTypeKeys "<Up><Home><Shift End>"
   try
       EditCopy
       Warnlog "Should NOT get anything !"
   catch
   endcatch

   Call wTypeKeys "<Down>"
   Call wTypeKeys "<Home><Shift End>"
   EditCopy
   if fGetFieldContent() <> sCategory & " 1" & sSeparator & sCaption  then
       Warnlog "Should get " & sCategory & " 1" & sSeparator & sCaption & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertBookmark_1

  Dim  sBookmark   as String
  Dim  sTestFile   as String

  sBookmark = "myTest"
  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\fields\pageTest.sxw")

  printlog "Insert Bookmark - Page"
 '/// <b> Insert Bookmark - Page </b>

  Call hNewDocument

  '/// Open test file pageTest.sxw
   Call hFileOpen(sTestFile)
   Call sMakeReadOnlyDocumentEditable

  '/// Set curson to the 2nd page
   Call wTypeKeys "<MOD1 Home>"
   Call fFindWord ("Sec")
   Call wTypeKeys "<Home>"

  '/// Insert a bookmark
   InsertBookmark
     Kontext "TextmarkeEinfuegen"
     Textmarken.SetText sBookmark
   TextmarkeEinfuegen.OK

   Call wTypeKeys "<End>"
   Call wTypeKeys "<Return>" , 2

  '/// Insert/Cross-reference , choose "Bookmarks" and "Page"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 5
     Sleep 1
     Auswahl.Select sBookmark
     Sleep 1
     Formatliste.Select 1
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent( ) <> "2" then
       Warnlog "Should get 2 , but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertBookmark_2

  Dim  sBookmark   as String
  Dim  sTestFile   as String

  sBookmark = "myTest"
  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\fields\chapterTest.sxw")

  printlog "Insert Bookmark - Chapter"
 '/// <b> Insert Bookmark - Chapter </b>

  Call hNewDocument

  '/// Open test file chapterTest.sxw
   Call hFileOpen(sTestFile)
   Call sMakeReadOnlyDocumentEditable

  '/// Set curson to the 2nd line
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Down><Home>"

  '/// Insert a bookmark
   InsertBookmark
     Kontext "TextmarkeEinfuegen"
     Textmarken.SetText sBookmark
   TextmarkeEinfuegen.OK

   Call wTypeKeys "<End>"
   Call wTypeKeys "<Return>" , 2

  '/// Insert/Cross-reference , choose "Bookmarks" and "Chapter"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 5
     Sleep 1
     Auswahl.Select sBookmark
     Sleep 1
     Formatliste.Select 2
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent( ) <> "2" then
       Warnlog "#i40625# Should get 2 , but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertBookmark_3

  Dim  sReference   as String
  Dim  sBookmark    as String

  sReference  = "est !"
  sBookmark   = "myTest"

  printlog "Insert Bookmkar - Reference"
 '/// <b> Insert Bookmkar - Reference</b>

 '/// Open a new document
  Call hNewDocument

  '/// Input some thing in the document and press return
   Call wTypeKeys "This is a test !"
   Call wTypeKeys "<Return>"

  '/// Select some text
   Call wTypeKeys "<Up><End>"
   Call wTypeKeys "<Shift Left>" , 5

  '/// Insert a bookmark
   InsertBookmark
     Kontext "TextmarkeEinfuegen"
     Textmarken.SetText sBookmark
   TextmarkeEinfuegen.OK

  '/// set focus to 2nd line
   Call wTypeKeys "<Down>"

  '/// Insert/Cross-reference , choose "Bookmark" and "Reference"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 5
     Sleep 1
     Auswahl.Select sBookmark
     Sleep 1
     Formatliste.Select 3
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent() <> sReference then
       Warnlog "Should get " & sReference & " but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertBookmark_4
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

  Dim  sBookmark       as String
  Dim  sAbove , sBelow as String

  sBookmark = "myTest"
  sAbove = "above" : sBelow = "below"

  printlog "Insert Bookmark - Above/Below"
 '/// <b> Insert Bookmark - Above/Below</b>

 '/// Open a new document
  Call hNewDocument

  '/// Press Return first , then input some thing in
  '/// + the document , press return again
   Call wTypeKeys "<Return>"
   Call wTypeKeys "This is a test !"
   Call wTypeKeys "<Return>"

  '/// Insert a Bookmark before "This is ..."
   Call wTypeKeys "<Up><Home>"
   InsertBookmark
     Kontext "TextmarkeEinfuegen"
     Textmarken.SetText sBookmark
   TextmarkeEinfuegen.OK

  '/// set focus to 3rd line
   Call wTypeKeys "<Down>"

  '/// Insert/Cross-reference , choose "Bookmarks" and "above/below"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 5
     Sleep 1
     Auswahl.Select sBookmark
     Sleep 1
     Formatliste.Select 4
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent() <> sAbove then
       Warnlog "Should get " & sAbove & " but get " & fGetFieldContent()
   end if

  '/// set focus to 1st line
   Call wTypeKeys "<MOD1 Home>"

  '/// Insert/Cross-reference , choose "Bookmark" and "above/below"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 5
     Sleep 1
     Auswahl.Select sBookmark
     Sleep 1
     Formatliste.Select 4
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent() <> sBelow then
       Warnlog "Should get " & sBelow & " but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertBookmark_5

  Dim  sBookmark   as String
  Dim  sTestSytle  as String

  sBookmark  = "myTest"
  sTestSytle = "A"

  printlog "Insert Bookmark - As Page Style"
 '/// <b> Insert Bookmark - As Page Style </b>

 '/// Open a new document
  Call hNewDocument

  '/// Fomrat/page / page , set Format as "A , B , C ..."
   Call fFormatPageWriter("TabSeite")
     Numerierung.Select 1
   TabSeite.OK

  '/// Input some thing in the document , press return
   Call wTypeKeys "This is a test !"
   Call wTypeKeys "<Return>"

  '/// Insert a Bookmark before "This is ..."
   Call wTypeKeys "<Up><Home>"
   InsertBookmark
     Kontext "TextmarkeEinfuegen"
     Textmarken.SetText sBookmark
   TextmarkeEinfuegen.OK

  '/// set focus to 2nd line
   Call wTypeKeys "<Down>"

  '/// Insert/Cross-reference , choose "Bookmark" and "As Page Style"
   InsertReferenceField
   Kontext "TabReferenzen"
     Feldtyp.Select 5
     Sleep 1
     Auswahl.Select sBookmark
     Sleep 1
     Formatliste.Select 5
     Sleep 1
     Einfuegen.Click
   TabReferenzen.Close

   Call wTypeKeys "<F9>"
   Sleep 2
   Call wTypeKeys "<Shift Home>"

   if fGetFieldContent() <> sTestSytle then
       Warnlog "Should get " & sTestSytle & " but get " & fGetFieldContent()
   end if

 '/// Close the document
  Call hCloseDocument

endcase

'-----------------------------------------------------------------

