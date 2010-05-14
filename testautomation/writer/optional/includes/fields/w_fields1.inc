'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_fields1.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:26:27 $
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
'* short description : Fields general test
'*
'\***********************************************************************

sub w_fields1

    Call tFields_1           'Insert / Fields / Date
    Call tFields_2           'Insert / Fields / Time
    Call tFields_3           'Insert / Fields / Page Number
    Call tFields_4           'Insert / Fields / Page Count
    Call tFields_5           'Insert / Fields / Subject
    Call tFields_6           'Insert / Fields / Title
    Call tFields_7           'Insert / Fields / Author

end sub

testcase tFields_1
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

    Dim sDate , sDate1 , sDate2 as String
    
    sDate  = Date
    sDate1 = Left (sDate , 6)
    sDate2 = Right(sDate , 2)
    
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
    EditFields
    Kontext "FeldbefehlBearbeitenDokument"
    Auswahl.Select 2
    Sleep 2
    FeldbefehlBearbeitenDokument.OK
    
    '/// Check if the date is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    if GetClipboard <> sDate1 then
        Warnlog "Fixed date is wrong ! Hope you don't run this case in the midnight"
    end if
    
    Call wTypeKeys "<Down><Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    if GetClipboard <> sDate2 then
        Warnlog "Date is wrong ! Hope you don't run this case in the midnight"
    end if
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFields_2
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

    Dim sTime() , sTime1 , sTime2 as String
    Dim sTemp                   as String
    
    printlog "Insert / Fields / Time"
    '/// <b> Default should be hh/mm/ss pm </b>
    
    Call hNewDocument
    
    sTime = Time
    sTemp = Right(sTime , 2)  'It should get am or pm
    
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
    Sleep 2
    FeldbefehlBearbeitenDokument.OK
    
    '/// Check if the time is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    sTime1 = GetClipboard
    if Len(sTime1)       < 11    then Warnlog "Fixed time is wrong !"
    if Right(sTime1 , 2) <> sTemp then Warnlog "Fixed time is wrong ! , Hope you don't run this case in the midnight"
    
    Call wTypeKeys "<Down><Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    sTime2 = GetClipboard
    if Len(sTime2)       <> 11    then Warnlog "Time is wrong !"
    if Right(sTime2 , 2) <> sTemp then Warnlog "Time is wrong ! , Hope you don't run this case in the midnight"
    
    printlog "Stop for a while ..."
    '/// Stop for  a while , check if the time is still correct
    Sleep 5
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    if GetClipboard <> sTime1 then Warnlog "Fixed time is wrong !"
    
    Call wTypeKeys "<Down><Home>"
    Call wTypeKeys "<Shift Right>"
    EditCopy
    if GetClipboard = sTime2  then Warnlog "Time is wrong !"
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFields_3
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

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

testcase tFields_4
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

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

testcase tFields_5
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

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

testcase tFields_6
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

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

testcase tFields_7
qaerrorlog "#i93906# - Testcase outcommented due to bug. 'Copied field-content not pastable outside Office'"
goto endsub

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
