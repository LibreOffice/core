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
'* short description : Test the functionality of Page Formatting / Page Styles - 2
'*
'************************************************************************
'*
' #1 tFormatPage_16      'AutoFit height - Check (Header,change font size)
' #1 tFormatPage_17      'AutoFit height - Check (Header,multiline content)
' #1 tFormatPage_18      'AutoFit height - UnCheck (Header,change font size)
' #1 tFormatPage_19      'AutoFit height - UnCheck (Header,multiline content)
' #1 tFormatPage_20      'AutoFit height - Check (Footer,change font size)
' #1 tFormatPage_21      'AutoFit height - Check (Footer,multiline content)
' #1 tFormatPage_22      'AutoFit height - UnCheck (Footer,change font size)
' #1 tFormatPage_23      'AutoFit height - UnCheck (Footer,multiline content)
' #1 tFormatPage_24      'warning message should appear when turning off header -1
' #1 tFormatPage_25      'warning message should appear when turning off header -2
' #1 tFormatPage_26      'warning message should appear when turning off footer -1
' #1 tFormatPage_27      'warning message should appear when turning off footer -2
' #1 tFormatPage_28      'Header/footer is limited to 80% of height of page text area
'*
'\***********************************************************************

testcase tFormatPage_16

    Dim sTestFile as String
    Dim sDummy as String

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testForAutoFit.sxw")

    PrintLog "- AutoFit height - Check (Header,change font size)"
    '/// AutoFit height - Check (Header,change font size)

    Call hNewDocument

    '/// Open test file testForAutoFit.sxw
    hFileOpenLocally( sTestFile )
    

    '/// Open Navigator and enter as page number 2
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    Seitennummer.TypeKeys "<Return>"
    '/// Close Navigator
    ViewNavigator
    
    '/// Step 3 times up in document
    Call wTypeKeys "<Up>" , 3
    Call wTypeKeys "<Home><Shift End>"

    '/// Remember number in line
    EditCopy
    sDummy =  GetClipboardText

    '/// Insert a header
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(9))
    Sleep 2
    MenuSelect(Menugetitemid(3))
    Sleep 2

    '/// formatPage , tabpage header
    fFormatPageWriter("TabKopfzeile")

    '/// AutoFit Height should be checked
    if Hoehedynamisch.IsChecked <> TRUE then
        warnlog "AutoFit Height should be checked!"
        Hoehedynamisch.Check
    end if
    TabKopfzeile.OK

    '/// Input some test words and select them
    Call wTypeKeys "Test"
    Call wTypeKeys "<End><Shift Home>"

    '/// Set the bigger fonts to selection in the header
    fFormatCharacter("TabFont")
    try
       SizeWest.Select SizeWest.GetItemCount-2
    catch
       Size.Select Size.GetItemCount-2
    endcatch
    TabFont.OK

    '/// Check if AutoFit Height works well
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close

    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then warnlog "Something wrong in AutoFit Height!"
    Call fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_17

    Dim sTestFile as String
    Dim sDummy1 as String
    Dim sDummy2 as String

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testForAutoFit.sxw")

    PrintLog "- AutoFit height - Check (Header,multiline content)"
    '/// AutoFit height - Check (Header,multiline content)

    Call hNewDocument

    '/// Open test file testForAutoFit.sxw
     hFileOpenLocally( sTestFile )
     

    '/// Open Navigator and enter as page number 2
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    Seitennummer.TypeKeys "<Return>"
    '/// Close Navigator
    ViewNavigator

    '/// Step 3 times up in document
    Call wTypeKeys "<Up>" , 3
    Call wTypeKeys "<Home><Shift End>"
    
    '/// Remember number in line
    EditCopy
    sDummy1 =  GetClipboardText
    
    Call wTypeKeys "<Up>"
    Call wTypeKeys "<Home><Shift End>"
    
    EditCopy
    sDummy2 =  GetClipboardText
    
    '/// Insert a header
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(9))
    Sleep 2
    MenuSelect(Menugetitemid(3))
    Sleep 2
    
    '/// formatPage , tabpage header
    fFormatPageWriter("TabKopfzeile")
    
    '/// AutoFit Height should be checked
    if Hoehedynamisch.IsChecked <> TRUE then
       warnlog "AutoFit Height should be checked!"
       Hoehedynamisch.Check
    end if
    TabKopfzeile.OK
    
    '/// Input some test words
    Call wTypeKeys "Test1"
    Call wTypeKeys "<Return>"
    Call wTypeKeys "Test2"
    
    '/// Check if AutoFit Height works well
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy1
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then warnlog "Something wrong in AutoFit Height --1 !"
    fCloseNavigator
    
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy2
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then warnlog "Something wrong in AutoFit Height --2 !"
    fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_18

    Dim sTestFile as String
    Dim sDummy as String

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testForAutoFit.sxw")

    PrintLog "- AutoFit height - UnCheck (Header,change font size)"
    '/// AutoFit height - UnCheck (Header,change font size)

    Call hNewDocument

    '/// Open test file testForAutoFit.sxw
     hFileOpenLocally( sTestFile )
     

    '/// Open Navigator and enter as page number 2
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    Seitennummer.TypeKeys "<Return>"
    '/// Close Navigator
    ViewNavigator

    '/// Step 3 times up in document
    Call wTypeKeys "<Up>" , 3
    Call wTypeKeys "<Home><Shift End>"
    
    '/// Remember number in line
    EditCopy
    sDummy =  GetClipboardText

    '/// Insert a header
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(9))
    Sleep 2
    MenuSelect(Menugetitemid(3))
    Sleep 2
    
    '/// formatPage , tabpage header ,UnCheck AutoFit Height
    fFormatPageWriter("TabKopfzeile")
    Hoehedynamisch.UnCheck
    TabKopfzeile.OK
    
    '/// Input some test words
    Call wTypeKeys "Test"
    Call wTypeKeys "<End><Shift Home>"
    
    '/// Set the bigger fonts to selection in the header
    fFormatCharacter("TabFont")
    try
        SizeWest.Select  SizeWest.GetItemCount-2
    catch
        Size.Select      Size.GetItemCount-2
    endcatch
    TabFont.OK
    
    '/// Check if AutoFit Height works well
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then warnlog "Something wrong in AutoFit Height!"
    fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_19

    Dim sTestFile as String
    Dim sDummy as String

    sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testForAutoFit.sxw")

    PrintLog "- AutoFit height - UnCheck (Header,multiline content)"
    '/// AutoFit height - UnCheck (Header,multiline content)

    Call hNewDocument

    '/// Open test file testForAutoFit.sxw
     hFileOpenLocally( sTestFile )
     

    '/// Open Navigator and enter as page number 2
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    Seitennummer.TypeKeys "<Return>"
    '/// Close Navigator
    ViewNavigator

    '/// Step 3 times up in document
    Call wTypeKeys "<Up>" , 3
    Call wTypeKeys "<Home><Shift End>"
    
    '/// Remember number in line
    EditCopy
    sDummy =  GetClipboardText

    '/// Insert a header
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(9))
    Sleep 2
    MenuSelect(Menugetitemid(3))
    Sleep 2
    
    '/// formatPage , tabpage header , Uncheck AutoFit Height
    fFormatPageWriter("TabKopfzeile")
    Hoehedynamisch.UnCheck
    TabKopfzeile.OK
    
    '/// input some test words
    Call wTypeKeys "Test1"
    Call wTypeKeys "<Return>"
    Call wTypeKeys "Test2"
    
    '/// Check if AutoFit Height works well
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close

    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then warnlog "Something wrong in AutoFit Height!"
    fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_20

    Dim sTestFile as String
    Dim sDummy as String

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testForAutoFit.sxw")

    PrintLog "- AutoFit height - Check (Footer,change font size)"
    '/// AutoFit height - Check (Footer,change font size)

    Call hNewDocument

    '/// Open test file testForAutoFit.sxw
    hFileOpenLocally( sTestFile )
    

    '/// Open Navigator and enter as page number 2
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    Seitennummer.TypeKeys "<Return>"
    '/// Close Navigator
    ViewNavigator

    '/// Step 3 times up in document
    Call wTypeKeys "<Up>" , 3
    Call wTypeKeys "<Home><Shift End>"
    
    '/// Remember number in line
    EditCopy
    sDummy =  GetClipboardText

    '/// Insert a footer
    DocumentWriter.UseMenu
    Sleep 2
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(10))
    Sleep 2
    Call hMenuFindSelect (".uno:InsertPageFooter?PageStyle:string=Default&On:bool=true", true, 3, true)   'Find "Insert:Footer:Default" and call the slot.
'    MenuSelect(Menugetitemid(3))
    Sleep 2

    '/// formatPage , tabpage footer
    fFormatPageWriter("TabFusszeile")

    '/// AutoFit Height should be checked
    if Hoehedynamisch.IsChecked <> TRUE then
        warnlog "AutoFit Height should be checked!"
        Hoehedynamisch.Check
    end if
    TabFusszeile.OK

    Call wTypeKeys "Test"
    Call wTypeKeys "<End><Shift Home>"

    '/// Set the bigger fonts to selection in the footer
    fFormatCharacter("TabFont")
    try
        SizeWest.Select SizeWest.GetItemCount-2
    catch
        Size.Select Size.GetItemCount-2
    endcatch
    TabFont.OK

    '/// Check if AutoFit Height works well
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close

    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then warnlog "Something wrong in AutoFit Height!"
    fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_21

    Dim sTestFile as String
    Dim sDummy1 as String
    Dim sDummy2 as String

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testForAutoFit.sxw")

    PrintLog "- AutoFit height - Check (Footer,multiline content)"
    '/// AutoFit height - Check (Footer,multiline content)

    '/// Open test file testForAutoFit.sxw
     hFileOpenLocally( sTestFile )
     

    '/// Open Navigator and enter as page number 2
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    Seitennummer.TypeKeys "<Return>"
    '/// Close Navigator
    ViewNavigator

    '/// Step 3 times up in document
    Call wTypeKeys "<Up>" , 3
    Call wTypeKeys "<Home><Shift End>"
    
    '/// Remember number in line
    EditCopy
    sDummy1 = GetClipboardText

    '/// Again 1 step up
    Call wTypeKeys "<Up>"
    Call wTypeKeys "<Home><Shift End>"

    '/// Remember number in current line
    EditCopy
    sDummy2 = GetClipboardText

    printlog " Insert a footer"
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(10))
    Sleep 2
    Call hMenuFindSelect (".uno:InsertPageFooter?PageStyle:string=Default&On:bool=true", true, 3, true)   'Find "Insert:Footer:Default" and call the slot.
    Sleep 2
    
    '/// formatPage , tabpage footer
    fFormatPageWriter("TabFusszeile")
    
    '/// AutoFit Height should be checked
    if Hoehedynamisch.IsChecked <> TRUE then
        warnlog "AutoFit Height should be checked!"
        Hoehedynamisch.Check
    end if
    TabFusszeile.OK
    
    '/// input some test words
    Call wTypeKeys "Test1"
    Call wTypeKeys "<Return>"
    Call wTypeKeys "Test2"
    
    '/// Check if AutoFit Height works well
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy1
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then warnlog "Something wrong in AutoFit Height --1 !"
    fCloseNavigator
    
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy2
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then warnlog "Something wrong in AutoFit Height --2 !"
    fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_22

    Dim sTestFile as String
    Dim sDummy as String

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testForAutoFit.sxw")

    PrintLog "- AutoFit height - UnCheck (Footer,change font size)"
    '/// AutoFit height - UnCheck (Footer,change font size)

    Call hNewDocument

    '/// Open test file testForAutoFit.sxw
    hFileOpenLocally( sTestFile )
    

    '/// Open Navigator and enter as page number 2
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    Seitennummer.TypeKeys "<Return>"
    '/// Close Navigator
    ViewNavigator

    '/// Step 3 times up in document
    Call wTypeKeys "<Up>" , 3
    Call wTypeKeys "<Home><Shift End>"
    
    '/// Remember number in line
    EditCopy
    sDummy =  GetClipboardText

    '/// Insert a footer
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(10))
    Sleep 2
    Call hMenuFindSelect (".uno:InsertPageFooter?PageStyle:string=Default&On:bool=true", true, 3, true)   'Find "Insert:Footer:Default" and call the slot.
'    MenuSelect(Menugetitemid(3))
    Sleep 2

    '/// formatPage , tabpage footer ,UnCheck AutoFit Height
    fFormatPageWriter("TabFusszeile")
    Hoehedynamisch.UnCheck
    TabFusszeile.OK
    
    '/// input some test words
    Call wTypeKeys "Test"
    Call wTypeKeys "<End><Shift Home>"
    
    '/// Set the bigger fonts to selection in the footer
    fFormatCharacter("TabFont")
    try
        SizeWest.Select SizeWest.GetItemCount-2
    catch
        Size.Select Size.GetItemCount-2
    endcatch
    TabFont.OK
    
    '/// Check if AutoFit Height works well
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then warnlog "Something wrong in AutoFit Height!"
    fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_23

    Dim sTestFile as String
    Dim sDummy as String

    sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testForAutoFit.sxw")

    PrintLog "- AutoFit height - UnCheck (Footer,multiline content)"
    '/// AutoFit height - UnCheck (Footer,multiline content)

    '/// Open test file testForAutoFit.sxw
    hFileOpenLocally( sTestFile )
    

    '/// Open Navigator and enter as page number 2
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    Seitennummer.TypeKeys "<Return>"
    '/// Close Navigator
    ViewNavigator

    '/// Step 3 times up in document
    Call wTypeKeys "<Up>" , 3
    Call wTypeKeys "<Home><Shift End>"
    
    '/// Remember number in line
    EditCopy
    sDummy =  GetClipboardText

    '/// Insert a footer
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(10))
    Sleep 2
    Call hMenuFindSelect (".uno:InsertPageFooter?PageStyle:string=Default&On:bool=true", true, 3, true)   'Find "Insert:Footer:Default" and call the slot.
'    MenuSelect(Menugetitemid(3))
    Sleep 2
    
    '/// formatPage , tabpage footer ,Uncheck AutoFit Height
    fFormatPageWriter("TabFusszeile")
    Hoehedynamisch.UnCheck
    TabFusszeile.OK
    
    '/// input some test words
    Call wTypeKeys "Test1"
    Call wTypeKeys "<Return>"
    Call wTypeKeys "Test2"
    
    '/// Check if AutoFit Height works well
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sDummy
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then warnlog "Something wrong in AutoFit Height!"
    fCloseNavigator
    
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_24

    PrintLog "- warning message should appear when turning off header -1"
    '/// warning message should appear when turning off header -1

    Call hNewDocument

    '/// Insert a header
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(9))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// formatPage , tabpage header
     fFormatPageWriter("TabKopfzeile")

    '/// + turn off header (uncheck Header on) ,
    '/// + a warning message should appear
       KopfzeileMitAbstand.UnCheck

       Kontext "Active"
       if Active.Exists then
           if Active.GetRT = 304 then  Active.No
       else
            warnlog "There should be a warning message appear when turn off header !"
       end if

       Kontext "TabKopfzeile"
     TabKopfzeile.Cancel

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_25

    PrintLog "- warning message should appear when turning off header -2"
    '/// warning message should appear when turning off header -2

    Call hNewDocument

    '/// Insert a header
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(9))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// turn off header from menu
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(9))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

     Kontext "Active"
     if Active.Exists then
         if Active.GetRT = 304 then  Active.No
     else
          warnlog "There should be a warning message appear when turn off header !"
     end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_26

    PrintLog "- warning message should appear when turning off footer -1"
    '/// warning message should appear when turning off footer -1

    Call hNewDocument

    '/// Insert a footer
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(10))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// formatPage , tabpage footer
     fFormatPageWriter("TabKopfzeile")

    '/// + turn off header (uncheck Header on) ,
    '/// + a warning message should appear
       KopfzeileMitAbstand.UnCheck

       Kontext "Active"
       if Active.Exists then
           if Active.GetRT = 304 then  Active.No
       else
            warnlog "There should be a warning message appear when turn off footer !"
       end if

       Kontext "TabKopfzeile"
     TabKopfzeile.Cancel

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_27

    PrintLog "- warning message should appear when turning off footer -2"
    '/// warning message should appear when turning off footer -2

    Call hNewDocument

    '/// Insert a footer
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(10))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// + turn off footer from menu ,
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
      Sleep 2
     MenuSelect(Menugetitemid(10))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// + the warning message should appear
     Kontext "Active"
     if Active.Exists then
         if Active.GetRT = 304 then  Active.No
     else
          warnlog "There should be a warning message appear when turn off footer !"
     end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_28

    Dim iFormat as Integer
    Dim sHeightInPage as String  ,  sHeight as String

    iFormat        = 2   'A4 paper
    sHeightInPage  = "20" + gSeperator + "00" + gMeasurementUnit
    sHeight        = "18" + gSeperator + "00" + gMeasurementUnit

    PrintLog "- Header/footer is limited to 80% of height of page text area "
   '/// Header/footer is limited to 80% of height of page text area

    Call hNewDocument

   '/// 1. Header is limited to 80% of height of page text area
    fFormatPageWriter("TabSeite")
      Papierformat.Select iFormat
      Wait 500
      Hoehe.SetText   sHeightInPage
      Wait 500
    TabSeite.OK

    '/// formatPage , tabpage header ,
    '/// + turn on header , and set header height more
    '/// + than 80% of page height
     fFormatPageWriter("TabKopfzeile")
       KopfzeileMitAbstand.Check
       Sleep 2
       Hoehe.SetText  sHeight
       Sleep 1
     TabKopfzeile.OK

    '/// Check if Header is limited to 80% of height of page text area
     fFormatPageWriter("TabKopfzeile")
       if StrToDouble(Hoehe.GetText) >= 16 then
           warnlog "Header height is > 16cm ! Get " & StrToDouble(Hoehe.GetText)
       end if
     TabKopfzeile.Cancel
    Call hCloseDocument

    Call hNewDocument

    '/// 2. Footer is limited to 80% of height of page text area
     fFormatPageWriter("TabSeite")
       Papierformat.Select iFormat
       Wait 500
       Hoehe.SetText   sHeightInPage
       Wait 500
     TabSeite.OK

    '/// formatPage , tabpage footer ,
    '/// + turn on footer , and set footer height more
    '/// + than 80% of page height
     fFormatPageWriter("TabFusszeile")
       FusszeileMitAbstand.Check
       Sleep 2
       Hoehe.SetText  sHeight
       Sleep 1
     TabFusszeile.OK

    '/// Check if footer is limited to 80% of height of page text area
     fFormatPageWriter("TabFusszeile")
       if StrToDouble(Hoehe.GetText) >= 16 then
           warnlog "Footer height is > 16cm ! Get " & StrToDouble(Hoehe.GetText)
       end if
     TabFusszeile.Cancel

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------
