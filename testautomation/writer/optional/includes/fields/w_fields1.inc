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
'* short description : Fields general test
'*
'\***********************************************************************

sub w_fields1

    Call tInsertFieldsDate
    Call tInsertFieldsTime
    Call tInsertFieldsPageNumber
    Call tInsertFieldsPageCount
    Call tInsertFieldsSubject
    Call tInsertFieldsTitle
    Call tInsertFieldsAuthor

end sub

'-----------------------------------------------------------------

testcase tInsertFieldsDate

    Dim sDate as String
    sDate  = Date

    printlog "Insert / Fields / Date"
    '/// <b> Default should be mm/dd/yy </b>

    Call hNewDocument

    '/// Insert / Fields / Date , the 1st date is fixed
    InsertFieldsDate

    '/// Return
    Call wTypeKeys "<Return>"

    '/// Insert / Fields / Date
    InsertFieldsDate

    Call wTypeKeys "<Home>"

    '/// Set 2nd date only to be date , not fixed
    '/// In 2nd date , Edit / Fields , select date in 'select'
    '/// Select second format in 'Format'
    EditFields
    Kontext "FeldbefehlBearbeitenDokument"
    Auswahl.Select 2
    Zahlenformat.Select 2  'Should be the same as the value we get with 'Time'.
    '/// Close the dialog
    FeldbefehlBearbeitenDokument.OK

    '/// Set first date to the right format, but leave as unfixed
    '/// In 1nd date , Edit / Fields , select second format in 'Format'
    Call wTypeKeys "<MOD1 Home>"

    EditFields
    Kontext "FeldbefehlBearbeitenDokument"
    Zahlenformat.Select 2  'Should be the same as the value we get with 'Time'.
    FeldbefehlBearbeitenDokument.OK

    '/// Check if the date is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    if GetClipboard <> sDate then
        Warnlog "Fixed date is wrong ! Hope you don't run this case in the midnight"
    end if

    Call wTypeKeys "<Down><Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    if GetClipboard <> sDate then
        Warnlog "Date is wrong ! Hope you don't run this case in the midnight"
    end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertFieldsTime

    Dim sTime  as String
    Dim sTime1 as String
    Dim sTime2 as String
    Dim sTemp  as String
    
    printlog "Insert / Fields / Time"
    '/// <b> Default could be hh/mm/ss pm  or simply  hh/mm/ss , depending on the system the test is running on.</b>
    
    Call hNewDocument
    sTime = Time
    
    '/// Insert / Fields / Time , the 1st time is fixed
    InsertFieldsTime
    
    '/// Return
    Call wTypeKeys "<Return>"
    
    '/// Insert / Fields / Time
    InsertFieldsTime
    
    Call wTypeKeys "<Home>"
    
    '/// Set 2nd time only to be time , not fixed
    '/// In 2nd time , Edit / Fields , select Time in 'select'
    EditFields
    Kontext "FeldbefehlBearbeitenDokument"
    Auswahl.Select 2
    FeldbefehlBearbeitenDokument.OK
    
    '/// Check if the time is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    sTime1 = GetClipboard
    if NOT Len(sTime1) > 7 then Warnlog "Fixed time-format is wrong !"
    
    Call wTypeKeys "<Down><Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    sTime2 = GetClipboard
    if NOT Len(sTime2) > 7 then Warnlog "Time-format is wrong !"
    
    printlog "Stop for a while ..."

    '/// Stop for  a while , check if the time is still correct
    Sleep 2
    ' update fields
    Call wTypeKeys "<F9>"
    ' read field content
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    if GetClipboard <> sTime1 then Warnlog "Fixed time is wrong !"

    '/// Stop for  a while , check if the time is still correct
    Sleep 2
    ' update fields
    Call wTypeKeys "<F9>"
    ' read field content
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Down><Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    if GetClipboard = sTime2  then Warnlog "Time is wrong !"
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertFieldsPageNumber

  Dim sPage1 , sPage2 as String

  sPage1 = "1" : sPage2 = "2"

  printlog "Insert / Fields / Page Number"

  Call hNewDocument

  '/// In 1st page , Insert / Fields / Page Number
   InsertFieldsPageNumbers

  '/// Insert a page break
   InsertManualBreak
   Sleep 2
   kontext "UmbruchEinfuegen"
     Seitenumbruch.check
   UmbruchEinfuegen.OK

  '/// In 2nd page , Insert / Fields / Page Number
   InsertFieldsPageNumbers

   Call wTypeKeys "<MOD1 Home>"

  '/// Check if the page number is correct
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent( ) <> sPage1 then
       QAErrorLog "#i31214# The 1st page number isn't correct ! should be: '" + sPage1 + "', but was: '" + GetClipboard + "'."
   end if

   Call wTypeKeys "<MOD1 End>"
   Call wTypeKeys "<Shift Left>"
   EditCopy
   if GetClipboard <> sPage2 then
       Warnlog "The 2nd page number isn't correct !"
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertFieldsPageCount

  Dim sPage1 , sPage2  as  String

  sPage1 = "1" : sPage2 = "2"

  printlog "Insert / Fields / Page Count"

  Call hNewDocument

  '/// In 1st page , Insert / Fields / Page Count
   InsertFieldsPageCount

   Call wTypeKeys "<MOD1 Home>"

  '/// Check if the 1st page count is correct
   Call wTypeKeys "<Shift Right>"
   EditCopy
   if GetClipboard <> sPage1 then
       Warnlog "1-The 1st page count isn't correct !"
   end if
   Call wTypeKeys "<End>"

  '/// Insert a page break
   InsertManualBreak
   Sleep 2
   kontext "UmbruchEinfuegen"
     Seitenumbruch.check
   UmbruchEinfuegen.OK

  '/// In 2nd page , Insert / Fields / Page Count
   InsertFieldsPageCount

   Call wTypeKeys "<MOD1 Home>"

  '/// Check if the page count is correct
   Call wTypeKeys "<Shift Right>"
   'EditCopy
   if fGetFieldContent( ) <> sPage2 then
       Warnlog "2-The 1st page count isn't correct"
   end if

   Call wTypeKeys "<MOD1 End>"
   Call wTypeKeys "<Shift Left>"
   EditCopy
   if GetClipboard <> sPage2 then
       Warnlog "The 2nd page count isn't correct !"
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertFieldsSubject

  Dim sSubject  as  String
  sSubject = "This is a Subject"

  printlog "Insert / Fields / Subject"

  Call hNewDocument

  '/// File / Properties / Description , input some
  '/// + text in Subject
   Call fFileProperties("TabBeschreibung")
     Thema.Settext sSubject
   TabBeschreibung.OK

  '/// Insert / Fields / Subject
   InsertFieldsSubject

  '/// Check if the subject is correct
   Call wTypeKeys "<Shift Left>"
   EditCopy
   if GetClipboard <> sSubject then
       Warnlog "Subject isn't correct !  should be: '" + sSubject + "', but was: '" + GetClipboard + "'."
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertFieldsTitle

  Dim sTitle  as  String

  sTitle = "This is a Title"

  printlog "Insert / Fields / Title"

  Call hNewDocument

  '/// File / Properties / Description , input some
  '/// + text in Title
   Call fFileProperties("TabBeschreibung")
     Titel.Settext sTitle
   TabBeschreibung.OK

  '/// Insert / Fields / Title
   InsertFieldsTitle

  '/// Check if the title is correct
   Call wTypeKeys "<Shift Left>"
   EditCopy
   if GetClipboard <> sTitle then
       Warnlog "Title isn't correct ! should be: '" + sTitle + "', but was: '" + GetClipboard + "'."
   end if

 Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tInsertFieldsAuthor

  Dim sFirstName  , sLastName  as String
  Dim sFirstName1 , sLastName1 as String

  sFirstName1 = "Test1" : sLastName1 = "Test2"

  printlog "Insert / Fields / Author"

  Call hNewDocument

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

  '/// Insert / Fields / Author
   InsertFieldsAuthor

  '/// Check if the title is correct
   Call wTypeKeys "<Shift Left>"
   EditCopy
   if GetClipboard <> (sFirstName1 + " " + sLastName1) then
       Warnlog "Author isn't correct ! should be: '" + (sFirstName1 + " " + sLastName1) + "', but was: '" + GetClipboard + "'."
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
