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
'* short description : Test the functionality of Page Formatting / Page Styles - 3
'*
'************************************************************************
'*
' #1 tFormatPage_29      'check for same content on left/right pages in header
' #1 tFormatPage_30      'check for different content on left/right pages in header
' #1 tFormatPage_31      'check for same content on left/right pages in footer
' #1 tFormatPage_32      'check for different content on left/right pages in footer
' #1 tFormatPage_33      'cursor should jump to header when inserting a header via menu
' #1 tFormatPage_34      'cursor shouldn't jump to header when inserting a header via checking 'Header on'
' #1 tFormatPage_35      'cursor should jump to footer when inserting a footer via menu
' #1 tFormatPage_36      'cursor shouldn't jump to footer when inserting a footer via checking 'Footer on'
' #1 tFormatPage_37      'Header must be applied to the current page style when inserting via menu
' #1 tFormatPage_38      'Footer must be applied to the current page style when inserting via menu
' #1 tFormatPage_39      'Set multi column via controls in page dialog
' #1 tFormatPage_40      'Set multi column via spinfield in page dialog
' #1 tFormatPage_41      'create columns with different width
' #1 tFormatPage_42      'vary distance between columns
' #1 tFormatPage_43      'seperator line between columns - change height/position of seperator
' #1 tFormatPage_44      'seperator line between columns - different line formats
' #1 tFormatPage_45      'shouldn't exist spacing after remove separator line
' #1 tFormatPage_46      'Check if the settings are kept after reopening the file
' #1 tFormatPage_47      'Test Travel through the 5 columns preset boxes
'*
'\***********************************************************************

testcase tFormatPage_29

    Dim sTestFile as String  ,  sNewContent as String
    Dim sContentInHeaderInPage1 as String  ,  sContentInHeaderInPage2 as String

    sTestFile                = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testHeaderContent29.sxw")
    sContentInHeaderInPage1  = "Header1"
    sContentInHeaderInPage2  = "Header2"
    sNewContent              = "MyTest"

    PrintLog "- check for same content on left/right pages in header"
    '/// check for same content on left/right pages in header

    Call hNewDocument

    '/// Open test file testHeaderContent29.sxw
    '/// + (The test file include 2 pages , and the
    '/// + content in header are different )
    hFileOpenLocally( sTestFile )
    

    '/// formatPage , tabpage header , check 'same content left/right'
    fFormatPageWriter("TabKopfzeile")
    InhaltLinksRechtsGleich.Check
    Sleep 1
    TabKopfzeile.OK

    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sContentInHeaderInPage2
    Sleep 1
    SearchAll.Click
    Sleep 1

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then  Active.OK
    else
        warnlog "The content should be same after checking 'same content left/right' !"
    end if

    Kontext "FindAndReplace"
    FindAndReplace.Close

    '/// type some new content in the header in page1
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInHeaderInPage1
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Right>"
     Call wTypeKeys "<Backspace>" , 7
     Call wTypeKeys sNewContent
     Sleep 1

    '/// Check if the content in footer in page2 is changed
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInHeaderInPage1
       Sleep 1
       SearchAll.Click
       Sleep 1

       Kontext "Active"
       if Active.Exists then
           if Active.GetRT = 304 then  Active.OK
       else
            warnlog "The content should be same after changing content in header in 1st page!"
       end if

       Kontext "FindAndReplace"
     FindAndReplace.Close

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_30

    Dim sTestFile as String
    Dim sContentInHeader as String  ,  sNewContent as String
    Dim sPage2 as String

    sTestFile         = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testHeaderContent30.sxw")
    sContentInHeader  = "Header"
    sNewContent       = "MyTest"
    sPage2            = "Second"

    PrintLog "- check for different content on left/right pages in header"
    '/// check for different content on left/right pages in header

    Call hNewDocument

    '/// Open test file testHeaderContent30.sxw
    '/// + (The test file include 2 pages , and the
    '/// + content in header is same )
     hFileOpenLocally( sTestFile )
     

    '/// formatPage , tabpage header , Uncheck 'same content left/right'
     fFormatPageWriter("TabKopfzeile")
       InhaltLinksRechtsGleich.UnCheck
     TabKopfzeile.OK

    '/// type some new content in the header in page1
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInHeader
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Right>"
     Call wTypeKeys "<Backspace>" , 6
     Call wTypeKeys sNewContent
     Sleep 1

    '/// Check if the content in footer in page2 is changed
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInHeader
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Kontext "Navigator"
     if Not Navigator.Exists then ViewNavigator
     Kontext "NavigatorWriter"
     Sleep 1
     if Seitennummer.GetText <> "1" then warnlog "Something wrong in Header in 1st page!"
     fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------


testcase tFormatPage_31

    Dim sTestFile as String  ,  sNewContent as String
    Dim sContentInFooterInPage1 as String  ,  sContentInFooterInPage2 as String

    sTestFile                = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testFooterContent31.sxw")
    sContentInFooterInPage1  = "Footer1"
    sContentInFooterInPage2  = "Footer2"
    sNewContent              = "MyTest"

    PrintLog "- check for same content on left/right pages in footer"
    '/// check for same content on left/right pages in footer

    Call hNewDocument

    '/// Open test file testFooterContent31.sxw
    '/// + (The test file include 2 pages , and the content
    '/// + in footer are different )
     hFileOpenLocally( sTestFile )
     

    '/// + formatPage , tabpage footer , check 'same content left/right'
     fFormatPageWriter("TabFusszeile")
       InhaltLinksRechtsGleich.Check
       Sleep 1
     TabFusszeile.OK

     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInFooterInPage2
       Sleep 1
       SearchAll.Click
       Sleep 1

       Kontext "Active"
       if Active.Exists then
           if Active.GetRT = 304 then  Active.OK
       else
           warnlog "The content should be same after checking 'same content left/right' !"
       end if

       Kontext "FindAndReplace"
     FindAndReplace.Close

    '/// type some new content in the footer in page1
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInFooterInPage1
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Right>"
     Call wTypeKeys "<Backspace>" , 7
     Call wTypeKeys sNewContent
     Sleep 1

    '/// Check if the content in footer in page2 is changed
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInFooterInPage1
       Sleep 1
       SearchAll.Click
       Sleep 1

       Kontext "Active"
       if Active.Exists then
           if Active.GetRT = 304 then  Active.OK
       else
            warnlog "The content should be same after changing content in footer in 1st page!"
       end if

       Kontext "FindAndReplace"
     FindAndReplace.Close

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_32

    Dim sTestFile as String
    Dim sContentInFooter as String  ,  sNewContent as String
    Dim sPage2 as String

    sTestFile         = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testFooterContent32.sxw")
    sContentInFooter  = "Footer"
    sNewContent       = "MyTest"
    sPage2            = "Second"

    PrintLog "- check for different content on left/right pages in footer"
    '/// check for different content on left/right pages in footer

    Call hNewDocument

    '/// Open test file testFooterContent32.sxw
    '/// + (The test file include 2 pages , and the
    '/// + content in footer is same )
     hFileOpenLocally( sTestFile )
     

    '/// formatPage , tabpage footer , Uncheck 'same content left/right'
     fFormatPageWriter("TabFusszeile")
       InhaltLinksRechtsGleich.UnCheck
     TabFusszeile.OK

    '/// type some new content in the footer in page1
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInFooter
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Right>"
     Call wTypeKeys "<Backspace>" , 6
     Call wTypeKeys sNewContent
     Sleep 1

    '/// Check if the content in the footer in page2 is changed
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sContentInFooter
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Kontext "Navigator"
     if Not Navigator.Exists then ViewNavigator
     Kontext "NavigatorWriter"
     Sleep 1
     if Seitennummer.GetText <> "1" then warnlog "Something wrong in Footer in 1st page!"
     fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_33

    Dim sTest as String

    sTest = "Test"

    PrintLog "- cursor should jump to header when inserting a header via menu"
    '/// cursor should jump to header when inserting a header via menu

    Call hNewDocument

    '/// Input some test words
     Call wTypeKeys "ABC"

    '/// Insert a header via menu
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(9))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// type some test words -- it should be in the header
     Call wTypeKeys sTest

    '/// check if the text in header is right
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sTest
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close
     Sleep 1

     EditSelectAll
     EditCopy

     if GetClipboardText <> sTest then warnlog "The content in the header isn't right, should be " &sTest &  " but get " & GetClipboardText

    '/// Remove the header
     Kontext
     Kontext "DocumentWriter"
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(9))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

     Kontext "Active"
     if Active.Exists then
         if Active.GetRT = 304 then  Active.Yes
     else
          warnlog "There should be a warning message appear when turn off header !"
     end if
     Sleep 1

    '/// Check if the content in header is removed
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sTest
       Sleep 1
       SearchAll.Click
       Sleep 1

       Kontext "Active"
       if Active.Exists then
           if Active.GetRT = 304 then  Active.OK
       else
           warnlog "The content in header should be removed!"
       end if

       Kontext "FindAndReplace"
     FindAndReplace.Close

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_34

    Dim sTest as String

    sTest = "Test"

    PrintLog "- cursor shouldn't jump to header when inserting a header via checking 'Header on'"
    '/// cursor shouldn't jump to header when inserting a header via  checking 'Header on'

    Call hNewDocument

     Call wTypeKeys "ABC"

    '/// formatPage , tabpage header
     fFormatPageWriter("TabKopfzeile")

    '/// + turn on header (check Header on)
       KopfzeileMitAbstand.Check
     TabKopfzeile.OK
     Sleep 1

    '/// type some test words -- it should be in the header
     Call wTypeKeys sTest

    '/// check if the text in header is right
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
        WholeWordsOnly.UnCheck
       Sleep 1
       SearchFor.SetText sTest
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close
     Sleep 1

     EditSelectAll
     EditCopy

     if GetClipboardText <> "ABC"+sTest then warnlog "The content in the header isn't right, should be null"

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_35

    Dim sTest as String

    sTest = "Test"

    PrintLog "- cursor should jump to footer when inserting a footer via menu"
    '/// cursor should jump to footer when inserting a footer via menu

    Call hNewDocument

     Call wTypeKeys "ABC"

    '/// Insert a footer via menu
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(9))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// type some test words - it should be in the footer
     Call wTypeKeys sTest

    '/// check if the text in footer is right
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sTest
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close
     Sleep 1

     EditSelectAll
     EditCopy

     if GetClipboardText <> sTest then warnlog "The content in the footer isn't right, should be " &sTest &  " but get " & GetClipboardText

    '/// Remove the footer
     Kontext
     Kontext "DocumentWriter"
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(9))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

     Kontext "Active"
     if Active.Exists then
         if Active.GetRT = 304 then  Active.Yes
     else
          warnlog "There should be a warning message appear when turn off footer !"
     end if
     Sleep 1

    '/// Check if the content in footer is removed
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sTest
       Sleep 1
       SearchAll.Click
       Sleep 1

       Kontext "Active"
       if Active.Exists then
           if Active.GetRT = 304 then  Active.OK
       else
           warnlog "The content in footer should be removed!"
       end if

       Kontext "FindAndReplace"
     FindAndReplace.Close

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_36

    Dim sTest as String

    sTest = "Test"

    PrintLog "- cursor shouldn't jump to footer when inserting a footer via checking 'Footer on'"
    '/// cursor shouldn't jump to footer when inserting a footer via  checking 'Footer on'

    Call hNewDocument

     Call wTypeKeys "ABC"

    '/// formatPage , tabpage footer
     fFormatPageWriter("TabKopfzeile")

    '/// + turn on footer (check Footer on)
       KopfzeileMitAbstand.Check
     TabKopfzeile.OK
     Sleep 1

    '/// type some test words - it should be in the footer
     Call wTypeKeys sTest

    '/// check if the text in footer is right
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       WholeWordsOnly.UnCheck
       Sleep 1
       SearchFor.SetText sTest
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close
     Sleep 1

     EditSelectAll
     EditCopy

     if GetClipboardText <> "ABC"+sTest then warnlog "The content in the footer isn't right, should be null"

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_37

    Dim sHeader as String

    Select case iSprache
        case 01   : sHeader = "Header"
        case 31   : sHeader = "Koptekst"
        case 33   : sHeader = "En-tête"
        case 34   : sHeader = "Encabezamiento"
        case 39   : sHeader = "Riga d'intestazione"
        case 46   : sHeader = "Sidhuvud"
        case 49   : sHeader = "Kopfzeile"
        case 50   : sHeader = "Glava"
        case 55   : sHeader = "Cabeçalho"
        case 81   : sHeader = "ヘッダ"
        case 82   : sHeader = "머리글"
        case 86   : sHeader = "页眉"
        case 88   : sHeader = "頁首"
        case else : QAErrorLog "The test does not support the language " + iSprache
                    Goto endsub
    end select

    PrintLog "- Header must be applied to the current page style when inserting via menu"
    '/// Header must be applied to the current page style when inserting via menu

    Call hNewDocument

    '/// Insert a header via menu
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(9))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// Check if header can be seen in status bar
     Kontext "TableObjectbar"

     if Vorlage2.GetSelText <> sHeader then
         warnlog "The header can't be seen in status bar ! "
     end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_38

    Dim sFooter as String

    Select case iSprache
        case 01   : sFooter = "Footer"
        case 31   : sFooter = "Voettekst"
        case 33   : sFooter = "Pied de page"
        case 34   : sFooter = "Pie de página"
        case 39   : sFooter = "Piè di pagina"
        case 46   : sFooter = "Sidfot"
        case 49   : sFooter = "Fußzeile"
        case 50   : sFooter = "Noga"
        case 55   : sFooter = "Rodapé"
        case 81   : sFooter = "フッタ"
        case 82   : sFooter = "바닥글"
        case 86   : sFooter = "页脚"
        case 88   : sFooter = "頁尾"
        case else : QAErrorLog "The test does not support the language " + iSprache
                    Goto endsub
    end select

    PrintLog "- Footer must be applied to the current page style when inserting via menu"
    '/// Footer must be applied to the current page style when inserting via menu

    Call hNewDocument

    '/// Insert a footer via menu
     DocumentWriter.UseMenu
     MenuSelect(Menugetitemid(4))
     Sleep 2
     MenuSelect(Menugetitemid(10))
     Sleep 2
     MenuSelect(Menugetitemid(1))
     Sleep 2

    '/// Check if header can be seen in status bar
     Kontext "TableObjectbar"

     if Vorlage2.GetSelText <> sFooter then
         warnlog "The footer can't be seen in status bar ! "
     end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_39

    Dim sTestFile as String
    Dim sColumn as String

    sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testColumnInPage.sxw")
    sColumn    = "3"

    PrintLog "- Set multi column via controls in page dialog"
    '/// Set multi column via controls in page dialog

    Call hNewDocument

    '/// Open test file testColumnInPage.sxw
    '/// + (The test file includes 2 pages , some text
    '/// + is in 1st page and another is in 2nd page)
     hFileOpenLocally( sTestFile )
     

    '/// formatPage , tabpage columns , change column to 3 via controls
     fFormatPageWriter("TabSpalten")
       Anzahl.TypeKeys "<Up>", 2
       Sleep 1
     TabSpalten.OK

    '/// Check if the text in 1st page in the 1st column
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText "ABCD"
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Home>"
     Call wTypeKeys "<Shift End>"
     EditCopy
     if Len(GetClipboardText) = 26 then warnlog "the first column isn't right!"

    '/// Check if the text in 2nd page in the 2nd column
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText "This"
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Kontext "Navigator"
       if Not Navigator.Exists then ViewNavigator
       Kontext "NavigatorWriter"
       Sleep 1
       if Seitennummer.GetText <> "1" then warnlog "the second column isn't right!"
     fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_40

    Dim sTestFile as String
    Dim sColumn as String

    sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testColumnInPage.sxw")
    sColumn    = "3"

    PrintLog "- Set multi column via spinfield in page dialog"
    '/// Set multi column via spinfield in page dialog

    Call hNewDocument

    '/// Open test file testColumnInPage.sxw
    '/// + (The test file includes 2 pages , some text is
    '/// + in 1st page and another is in 2nd page)
     hFileOpenLocally( sTestFile )
     

    '/// formatPage , tabpage columns , change column to 3 via spinfield
     fFormatPageWriter("TabSpalten")
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
     TabSpalten.OK

    '/// Check if the text in 1st page in the 1st column
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText "ABCD"
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Home>"
     Call wTypeKeys "<Shift End>"
     EditCopy
     if Len(GetClipboardText) = 26 then warnlog "the first column isn't right!"

    '/// Check if the text in 2nd page in the 2nd column
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText "This"
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Kontext "Navigator"
       if Not Navigator.Exists then ViewNavigator
       Kontext "NavigatorWriter"
       Sleep 1
       if Seitennummer.GetText <> "1" then warnlog "the second column isn't right!"
     fCloseNavigator

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_41

    Dim sTestFile as String
    Dim sColumn as String
    Dim sWidthColumn1 as String
    Dim sWidthColumn2 as String
    Dim sWidthColumn3 as String

    sTestFile     = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testColumnInPage2.sxw")
    sColumn       = "3"
    sWidthColumn1 = "2" + gSeperator + "00"
    sWidthColumn2 = "4" + gSeperator + "00"

    PrintLog "- create columns with different width "
    '/// create columns with different width

    Call hNewDocument

    '/// Open test file testColumnInPage2.sxw
    '/// + (The test file includes 2 pages , some text is
    '/// + in 1st page and another is in 2nd page)
    hFileOpenLocally( sTestFile )
    

    '/// Create 3 with different columns . formatPage ,
    '/// + tabpage columns , set 3 columns
    Call fFormatPageWriter("TabSpalten")
    Anzahl.TypeKeys "<Up>", 2
    Sleep 1
    AutomBreite.Uncheck
    Sleep 1
    Spaltenbreite1.SetText sWidthColumn1
    wait 500
    Spaltenbreite2.SetText sWidthColumn2
    wait 500
    Art.Select 1
    wait 500
    sWidthColumn3 = Spaltenbreite3.GetText
    wait 500
    TabSpalten.OK

    '/// Check if columns' width is right
    fFormatPageWriter("TabSpalten")
    if Left$(Spaltenbreite1.GetText,4) <> sWidthColumn1 then Warnlog "the column1's width isn't set correctly !"
    if Left$(Spaltenbreite2.GetText,4) <> sWidthColumn2 then Warnlog "the column2's width isn't set correctly !"
    if Spaltenbreite3.GetText <> sWidthColumn3 then Warnlog "the column3's width isn't set correctly !"
    TabSpalten.Cancel

    'detailed check for column1
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText "ABCD"
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close

    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    if Len(GetClipboardText) > 8 then Warnlog "Something wrong in the column1"

    'detailed check for column2
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText "ZYXW"
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close

    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    if Len(GetClipboardText) > 14 then Warnlog "Something wrong in the column2"

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_42

    Dim sTestFile as String
    Dim sColumn as String
    Dim sWidthColumn1 as String
    Dim sWidthColumn2 as String
    Dim sWidthColumn3 as String
    Dim sSpacing1 as String
    Dim sSpacing2 as String

    sTestFile     = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testColumnInPage2.sxw")
    sColumn       = "3"

    sWidthColumn1 = "2" + gSeperator + "00" + gMeasurementUnit
    sWidthColumn2 = "4" + gSeperator + "00" + gMeasurementUnit
    sSpacing1     = "1" + gSeperator + "00" + gMeasurementUnit
    sSpacing2     = "2" + gSeperator + "00" + gMeasurementUnit

    PrintLog "- vary distance between columns "
    '/// vary distance between columns

    Call hNewDocument

    '/// Open test file testColumnInPage2.sxw
    '/// + (The test file includes 2 pages , some text is
    '/// + in 1st page and another is in 2nd page)
     hFileOpenLocally( sTestFile )
     

    '/// formatPage , tabpage columns , set 3 columns
     fFormatPageWriter("TabSpalten")
       Anzahl.TypeKeys "<Up>", 2
       Sleep 1
     TabSpalten.OK

    '/// vary distance between columns
     fFormatPageWriter("TabSpalten")
       AutomBreite.Uncheck
       Sleep 1
       Spaltenbreite1.SetText sWidthColumn1
       wait 500
       Spaltenbreite2.SetText sWidthColumn2
       wait 500
       Abstand12.SetText sSpacing1
       Sleep 2
       Abstand23.SetText sSpacing2
       Sleep 2
       Art.Select 1
       Sleep 1
       sWidthColumn2 = "3" + gSeperator + "00" + gMeasurementUnit
       sWidthColumn3 = Spaltenbreite3.GetText
       wait 500
     TabSpalten.OK

    '/// Check if columns' width and spacing are right
     fFormatPageWriter("TabSpalten")
       if Spaltenbreite1.GetText <> sWidthColumn1 then Warnlog "the column1's width isn't set correctly !"
       if Spaltenbreite2.GetText <> sWidthColumn2 then Warnlog "the column2's width isn't set correctly !"
       if Spaltenbreite3.GetText <> sWidthColumn3 then Warnlog "the column3's width isn't set correctly !"
       if Abstand12.GetText      <> sSpacing1     then Warnlog "the spacing1 isn't correct!"
       if Abstand23.GetText      <> sSpacing2     then Warnlog "the spacing1 isn't correct!"
     TabSpalten.Cancel

    'detailed check for column2
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText "ZYXW"
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Home>"
     Call wTypeKeys "<Shift End>"
     EditCopy
     if Len(GetClipboardText) > 11 then Warnlog "Somthing wrong in the column2"

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_43

    Dim sColumn as String
    Dim sHeight as String
    Dim iLine as Integer
    Dim iPosition as Integer

    sColumn        = "2"
    sHeight        = "50%"
    iLine          = 2
    iPosition      = 2

    PrintLog "- seperator line between columns - change height/position of seperator"
    '/// seperator line between columns - change height/position of seperator

    Call hNewDocument

    '/// formatPage , tabpage columns , set 2 columns
     fFormatPageWriter("TabSpalten")
       Anzahl.TypeKeys "<Up>"
       Sleep 1
     TabSpalten.OK

    '/// + Change height/position of seperator
     fFormatPageWriter("TabSpalten")
       Art.Select      iLine
       Sleep 1
       Hoehe.SetText   sHeight
       Sleep 1
       Position.Select iPosition
       Sleep 1
     TabSpalten.OK

    '/// Check if height/position of seperator is changed
     fFormatPageWriter("TabSpalten")
       if Hoehe.GetText        <> sHeight    then Warnlog "Height isn't correct!"
       Sleep 1
       if Position.GetSelIndex <> iPosition  then Warnlog "Position isn't correct!"
     TabSpalten.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_44

    Dim sColumn as String
    Dim i as Integer

    sColumn = "2"

    PrintLog "- seperator line between columns - different line formats "
    '/// seperator line between columns - different line formats

    Call hNewDocument

    '/// formatPage , tabpage columns , set 2 columns
     fFormatPageWriter("TabSpalten")
       Anzahl.TypeKeys "<Up>"
       Sleep 1
     TabSpalten.OK

    '/// + Change seperator line style

    '/// Check if height/position  of seperator is changed

     fFormatPageWriter("TabSpalten")
     for i = 1 to Art.GetItemCount
         if i > 1 then fFormatPageWriter("TabSpalten")
           Art.Select  i
           Sleep 1
         TabSpalten.OK

         fFormatPageWriter("TabSpalten")
           if Art.GetSelIndex <> i  then Warnlog "Seperator line isn't correct!"
         TabSpalten.Cancel
     next i

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_45

    Dim sTestFile as String
    Dim sTestText as String
    Dim iTestTextLength as Integer

    sTestFile     = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testSeparatorLine.sxw")
    sTestText     = "llll"

    PrintLog "- shouldn't exist spacing after remove separator line"
    '/// shouldn't exist spacing after remove separator line

    Call hNewDocument

    '/// Open test file testSeparatorLine.sxw
    '/// + (The test file includes 6 columns , some
    '/// + test text lllll... is in 2nd column
     hFileOpenLocally( sTestFile )
     

     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sTestText
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Home>"
     Call wTypeKeys "<Shift End>"
     EditCopy
     iTestTextLength = Len(GetClipboardText)

    '/// formatPage , tabpage columns, set Separator line to 5pt
     fFormatPageWriter("TabSpalten")
       Art.Select  Art.GetItemCount
       Sleep 1
     TabSpalten.OK

     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sTestText
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Home>"
     Call wTypeKeys "<Shift End>"
     EditCopy
     if iTestTextLength <= Len(GetClipboardText) then Warnlog "Seperator line isn't correct!"

    '/// + set separator line to null
     fFormatPageWriter("TabSpalten")
       Art.Select  1
       Sleep 1
     TabSpalten.OK

    '/// Check if column is covered to former
     Kontext
     EditSearchAndReplace
     Kontext "FindAndReplace"
       SearchFor.SetText sTestText
       Sleep 1
       SearchAll.Click
       Sleep 1
     FindAndReplace.Close

     Call wTypeKeys "<Home>"
     Call wTypeKeys "<Shift End>"
     EditCopy
     if iTestTextLength <> Len(GetClipboardText) then Warnlog "The column isn't covered to former! #112632"

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_46

    Dim sColumn as String
    Dim sWidthColumn1 as String
    Dim sWidthColumn2 as String
    Dim sWidthColumn3 as String
    Dim sSpacing1 as String
    Dim sSpacing2 as String
    Dim iLine as Integer
    Dim sLineHeight as String
    Dim iLinePosition as Integer

    sColumn       = "3"
    sWidthColumn1 = "2" + gSeperator + "00" + gMeasurementUnit
    sWidthColumn2 = "4" + gSeperator + "00" + gMeasurementUnit
    sSpacing1     = "0" + gSeperator + "30" + gMeasurementUnit
    sSpacing2     = "0" + gSeperator + "50" + gMeasurementUnit
    iLine         = 4
    sLineHeight   = "50%"
    iLinePosition = 2

    PrintLog "- Check if the settings are kept after reopening the file"
    '/// Check if the settings are kept after reopening the file

    Call hNewDocument

    '/// Format/Page / columns , do some configurations

    'set 3 columns
     fFormatPageWriter("TabSpalten")
       Anzahl.TypeKeys "<Up>", 2
       Sleep 1
     TabSpalten.OK

    'UnCheck autowidth , set different width to 3 columns
     fFormatPageWriter("TabSpalten")
       AutomBreite.Uncheck
       Sleep 1
       Abstand12.SetText      sSpacing1
       Sleep 1
       Abstand23.SetText      sSpacing2
       Sleep 1
       Spaltenbreite1.SetText sWidthColumn1
       wait 500
       Spaltenbreite2.SetText sWidthColumn2
       wait 500
       Art.Select             iLine
       wait 500
       Hoehe.SetText          sLineHeight
       Sleep 1
       Position.Select        iLinePosition
       Sleep 1
       sWidthColumn3 = Spaltenbreite3.GetText
       wait 500
     TabSpalten.OK

    '/// Save the file , close the file and reopen it
     Call hFileSaveAsWithFilterKill(gOfficepath + "user\work\tFormatPage_46.odt", "writer8")
     Call hCloseDocument

     Call hNewDocument
     hFileOpen( gOfficepath + "user\work\tFormatPage_46.odt")

    '/// Check if the setting are still kept
     fFormatPageWriter("TabSpalten")
       if AutomBreite.Ischecked  =  TRUE          then Warnlog "Autowidth shouldn't be checked !"
       if Abstand12.GetText      <> sSpacing1     then Warnlog "Spacing1 isn't right , should be " &sSpacing1 & " but get " &Abstand12.GetText
       if Abstand23.GetText      <> sSpacing2     then Warnlog "Spacing2 isn't right , should be " &sSpacing2 & " but get " &Abstand23.GetText
       if Spaltenbreite1.GetText <> sWidthColumn1 then Warnlog "column1 isn't right , should be " &sWidthColumn1 & " but get " &Spaltenbreite1.GetText
       if Spaltenbreite2.GetText <> sWidthColumn2 then Warnlog "column1 isn't right , should be " &sWidthColumn2 & " but get " &Spaltenbreite2.GetText
       if Spaltenbreite3.GetText <> sWidthColumn3 then Warnlog "column1 isn't right , should be " &sWidthColumn2 & " but get " &Spaltenbreite3.GetText
       if Art.GetSelIndex        <> iLine         then Warnlog "Line style isn't right !"
       if Hoehe.GetText          <> sLineHeight   then Warnlog "Height isn't correct!"
       if Position.GetSelIndex   <> iLinePosition then Warnlog "Position isn't correct!"
     TabSpalten.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_47

    Dim sWidth as String
    Dim sLeft as String  ,  sRight as String  ' set margin size
    Dim sWidth1 as String  ,  sWidth2 as String  ,  sWidth3 as String

    sWidth     = "20" + gSeperator + "00" + gMeasurementUnit
    sLeft      = "2"  + gSeperator + "00" + gMeasurementUnit
    sRight     = "3"  + gSeperator + "00" + gMeasurementUnit

    PrintLog "- Test Travel through the 5 columns preset boxes "
    '/// Test Travel through the 5 columns preset boxes

    Call hNewDocument

    'Set page's width to 20cm and left margin: 3cm , right margin : 2cm
     fFormatPageWriter("TabSeite")
       Breite.SetText  sWidth
       Wait 500
       Links.SetText   sLeft
       Wait 500
       Rechts.SetText  sRight
       Wait 500
     TabSeite.OK

    '/// formatPage , tabpage columns
     fFormatPageWriter("TabSpalten")

      '/// - Select fisrt box ==> only 1 column
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
       if Len(Spaltenbreite1.GetText) <> 0     then Warnlog "width1 shouldn't be writable when there is only 1 column !"
       Sleep 1

      '/// - Select second box ==> 2 columns ( 1st = 2rd )
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
       sWidth1     = "7" + gSeperator + "50" + gMeasurementUnit
       sWidth2     = "7" + gSeperator + "50" + gMeasurementUnit
       if Spaltenbreite1.GetText <> sWidth1 then Warnlog "2.width1 should be " &sWidth1 & " but get " &Spaltenbreite1.GetText
       if Spaltenbreite2.GetText <> sWidth2 then Warnlog "2.width2 should be " &sWidth2 & " but get " &Spaltenbreite2.GetText
       if AutomBreite.IsChecked  <> TRUE    then Warnlog "2.Autowidth should be checked !"

      '/// - Select third box ==> 3 columns ( 1st = 2rd = third )
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
       sWidth1     = "5" + gSeperator + "00" + gMeasurementUnit
       sWidth2     = "5" + gSeperator + "00" + gMeasurementUnit
       sWidth3     = "5" + gSeperator + "00" + gMeasurementUnit
       if Spaltenbreite1.GetText <> sWidth1 then Warnlog "3.width1 should be " &sWidth1 & " but get " &Spaltenbreite1.GetText
       if Spaltenbreite2.GetText <> sWidth2 then Warnlog "3.width2 should be " &sWidth2 & " but get " &Spaltenbreite2.GetText
       if Spaltenbreite3.GetText <> sWidth3 then Warnlog "3.width3 should be " &sWidth2 & " but get " &Spaltenbreite3.GetText
       if AutomBreite.IsChecked  <> TRUE    then Warnlog "3.Autowidth should be checked !"

      '/// - Select forth box ==> 2 columns ( 1st : 2rd = 2:1 )
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
       sWidth1     = "10" + gSeperator + "00" + gMeasurementUnit
       sWidth2     = "5"  + gSeperator + "00" + gMeasurementUnit
       if Spaltenbreite1.GetText <> sWidth1 then Warnlog "4.width1 should be " &sWidth1 & " but get " &Spaltenbreite1.GetText
       if Spaltenbreite2.GetText <> sWidth2 then Warnlog "4.width2 should be " &sWidth2 & " but get " &Spaltenbreite2.GetText
       if AutomBreite.IsChecked  =  TRUE    then Warnlog "4.Autowidth should be Unchecked !"

      '/// - Select fifth box ==> 2 columns ( 1st : 2rd = 1:2 )
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
       sWidth1     = "5"  + gSeperator + "00" + gMeasurementUnit
       sWidth2     = "10" + gSeperator + "00" + gMeasurementUnit
       if Spaltenbreite1.GetText <> sWidth1 then Warnlog "5.width1 should be " &sWidth1 & " but get " &Spaltenbreite1.GetText
       if Spaltenbreite2.GetText <> sWidth2 then Warnlog "5.width2 should be " &sWidth2 & " but get " &Spaltenbreite2.GetText
       if AutomBreite.IsChecked  =  TRUE    then Warnlog "5.Autowidth should be Unchecked !"

      '/// - Press right key again, will get the first box
       ColumnValueset.TypeKeys "<Right>"
       Sleep 1
       if Anzahl.GetText <> "1" then Warnlog "The focus should be in the leftest box !"
       Sleep 1

     TabSpalten.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

