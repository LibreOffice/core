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
'* short description : Test the functionality of Page Formatting / Page Styles - 4
'*
'************************************************************************
'*
' #1 tFormatPage_48      'Text flow direction test Right to Left (Vertical) - Paragraph
' #1 tFormatPage_49      'Text flow direction test Right to Left (Vertical) - Section
' #1 tFormatPage_50      'Text flow direction test Right to Left (Vertical) - Footnotes
' #1 tFormatPage_51      'Text flow direction test Right to Left (Vertical) - Endnotes
' #1 tFormatPage_52      'Text flow direction test Right to Left (Vertical) - Table
' #1 tFormatPage_53      'Text flow direction test Right to Left (Vertical) - Columns
' #1 tFormatPage_54      'Only when a border is set, "spacing to contents" gets a minimum value of 0.05cm
' #1 tFormatPage_55      'All "spacing to contents" spin boxes are limited to 5cm
' #1 tFormatPage_56      'Test Uncheck 'Synchronize'
' #1 tFormatPage_57      'Test Check 'Synchronize'
' #1 tFormatPage_58      'Test default shadow color
' #1 tFormatPage_59      'Test if the shadow settings is OK after closing the dialog
' #1 tFormatPage_60      'Test if the shadow settings is OK after reopening the file
'*
'\***********************************************************************

testcase tFormatPage_48

    Dim sTestText1 as string, sTestText2 as String
    Dim RecheckAsianSupport as boolean
    RecheckAsianSupport = gAsianSup

    Select Case iSystemSprache
        Case 81, 82, 86, 88
            printlog "No test with asian lovale, because checkbox 'Asian language support' is disabled by default!"
            goto endsub
    end select

    sTestText1    = "This is the test!"
    sTestText2    = "That isn't a test!"

    PrintLog "- Text flow direction test Right to Left (Vertical) - Paragraph"
    '/// Text flow direction test Right to Left (Vertical) - Paragraph

    Call hNewDocument

    '/// Set Asian language support enabled
    Call ActiveDeactivateCTLSupport(FALSE)
    Call ActiveDeactivateAsianSupport(TRUE)

    '/// Input some test words
    Call wTypeKeys sTestText1
    Call wTypeKeys "<Return>"
    Call wTypeKeys sTestText2
    Sleep 1

    '/// formatPage , tabpage page, select right to left (vertical)
    fFormatPageWriter("TabSeite")
    Textfluss.Select 2
    Sleep 1
    TabSeite.OK
    Sleep 2

    '/// Check if the direction of the test words is changed
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Down>", 4
    Sleep 1
    EditCopy
    if GetClipboardText <> "This" then
        Warnlog "Something wrong in first word - get " &GetClipboardText
    end if

    Call wTypeKeys "<Home><Left>"
    Call wTypeKeys "<Shift Down>"  , 4
    Sleep 1
    EditCopy
    if GetClipboardText <> "That" then
        Warnlog "Something wrong in second word - get " &GetClipboardText
    end if

    if RecheckAsianSupport = False then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_49

    Dim sTestText as String
    Dim RecheckAsianSupport as boolean
    RecheckAsianSupport = gAsianSup

    sTestText     = "This is the test!"

    PrintLog "- Text flow direction test Right to Left (Vertical) - Section"
    '/// Text flow direction test Right to Left (Vertical) - Section

    Call hNewDocument

    '/// Set Asian language support enabled
    Call ActiveDeactivateAsianSupport(TRUE)

    '/// Insert a section
    fInsertSection("TabBereiche")
    TabBereiche.OK

    '/// insert something in the section
    Call wTypeKeys "<Up>"
    Call wTypeKeys sTestText

    '/// formatPage , tabpage page, select right to left (vertical)
    fFormatPageWriter("TabSeite")
    Textfluss.Select 2
    Sleep 1
    TabSeite.OK
    Sleep 2

    '/// Check if the direction of the test words is changed
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift Down>" , 4
    EditCopy
    if GetClipboardText <> "This" then Warnlog "Something wrong in the section - get " &GetClipboardText

    if RecheckAsianSupport = False then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_50

    Dim RecheckAsianSupport as boolean
    RecheckAsianSupport = gAsianSup

    PrintLog "- Text flow direction test Right to Left (Vertical) - Footnotes"
    '/// Text flow direction test Right to Left (Vertical) - Footnotes

    Call hNewDocument

    '/// Set Asian language support enabled
    Call ActiveDeactivateAsianSupport(TRUE)

    '/// Insert a footnotes
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    Sleep 1
    Fussnote.Check
    Sleep 1
    FussnoteEinfuegen.OK
    Sleep 1

    Call wTypeKeys "<PageUp>"
    Sleep 1

    '/// formatPage , tabpage page, select right to left (vertical)
    Call fFormatPageWriter("TabSeite")
    Textfluss.Select 2
    Sleep 1
    TabSeite.OK
    Sleep 2

    '/// Check if the direction of the footnote is changed
    Call wTypeKeys "<Down>"
    try
        EditFootnote
        Kontext "FussnoteBearbeiten"
        FussnoteBearbeiten.Cancel
        Warnlog "Should NOT execute edit/footnote  when focus is under the footnote"
    catch
    endcatch

    Call wTypeKeys "<Up>"
    try
        EditFootnote
        Kontext "FussnoteBearbeiten"
        FussnoteBearbeiten.Cancel
    catch
        Warnlog "Should execute edit/footnote  when focus is up the footnote"
    endcatch

    if RecheckAsianSupport = False then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_51

    Dim RecheckAsianSupport as boolean
    RecheckAsianSupport = gAsianSup

    PrintLog "- Text flow direction test Right to Left (Vertical) - Endnotes"
    '/// Text flow direction test Right to Left (Vertical) - Endnotes

    Call hNewDocument

    '/// Set Asian language support enabled
    Call ActiveDeactivateAsianSupport(TRUE)

    '/// Insert a endnotes
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    Sleep 1
    Endnote.Check
    Sleep 1
    FussnoteEinfuegen.OK
    Sleep 1

    Call wTypeKeys "<PageUp>"
    Sleep 1

    '/// formatPage , tabpage page, select right to left (vertical)
    Call fFormatPageWriter("TabSeite")
    Textfluss.Select 2
    Sleep 1
    TabSeite.OK
    Sleep 2

    '/// Check if the direction of the endnote is changed
    Call wTypeKeys "<Down>"
    try
        EditFootnote
        Kontext "FussnoteBearbeiten"
        FussnoteBearbeiten.Cancel
        Warnlog "Should NOT execute edit/footnote when focus is under the endnote"
    catch
    endcatch

    Call wTypeKeys "<Up>"
    try
        EditFootnote
        Kontext "FussnoteBearbeiten"
        FussnoteBearbeiten.Cancel
    catch
        Warnlog "Should execute edit/footnote  when focus is on the endnote!"
    endcatch

    if RecheckAsianSupport = False then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_52

    Dim sTest1 as String  ,  sTest2 as String  ,  sTest3 as String
    Dim RecheckAsianSupport as boolean
    RecheckAsianSupport = gAsianSup

    sTest1 = "A1" : sTest2 = "B1" : sTest3 = "A2"

    PrintLog "- Text flow direction test Right to Left (Vertical) - Table"
    '/// Text flow direction test Right to Left (Vertical) - Table

    Call hNewDocument

    '/// Set Asian language support enabled
    Call ActiveDeactivateAsianSupport(TRUE)

    '/// Insert a table
    Call hTabelleEinfuegen

    '/// + input some string in the table , like
    '/// a1  b1
    '/// a2
    Call wTypeKeys sTest1
    wait 500
    Call wTypeKeys "<Right>"
    wait 500
    Call wTypeKeys sTest2
    wait 500
    Call wTypeKeys "<Down><Left>"   'so far, the focus should be in 2nd line of the table
    wait 500
    Call wTypeKeys sTest3
    Call wTypeKeys "<Up><Home>"     'so far, the focus should be in front of A1

    '/// formatPage , tabpage page, select right to left (vertical)
    Call fFormatPageWriter("TabSeite")
    Textfluss.Select 2
    Sleep 1
    TabSeite.OK
    Sleep 2

    '/// Check if the direction of the table is changed
    Call wTypeKeys "<Shift Down>" , 2
    Sleep 1
    EditCopy
    if GetClipboardText <> sTest1 then
        Warnlog "1st word in table is wrong!"
    end if

    Call wTypeKeys "<Down>"
    Call wTypeKeys "<Shift Down>" , 2
    Sleep 1
    EditCopy
    if GetClipboardText <> sTest2 then
        Warnlog "2nd word in table is wrong!"
    end if

    Call wTypeKeys "<Left><Up>"
    Call wTypeKeys "<Shift Home>"
    Sleep 1
    EditCopy
    if GetClipboardText <> sTest3 then
        Warnlog "2rd word in table is wrong!"
    end if

    if RecheckAsianSupport = False then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_53

    Dim sTest as String
    Dim sTestFile as String
    Dim RecheckAsianSupport as boolean
    RecheckAsianSupport = gAsianSup

    sTest         = "Test"
    sTestFile     = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\testColumnVertical.sxw")

    PrintLog "- Text flow direction test Right to Left (Vertical) - Column"
    '/// Text flow direction test Right to Left (Vertical) - Column

    Call hNewDocument

    '/// Set Asian language support enabled
    Call ActiveDeactivateAsianSupport(TRUE)

    '/// Open test file testColumnVertical.sxw
    '/// + There are 8 columns in the page , and a test
    '/// + word is in 3rd column
    hFileOpenLocally( sTestFile )
    

    '/// formatPage , tabpage page, select right to left (vertical)
    Call fFormatPageWriter("TabSeite")
    Textfluss.Select 2
    Sleep 1
    TabSeite.OK
    Sleep 2

    '/// Check if the direction of the column is changed
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sTest
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
    Sleep 1

    Call wTypeKeys "<Down>"
    Sleep 1

    Call wTypeKeys "<Shift Up>" , 4
    EditCopy
    if GetClipboardText <> sTest then
        Warnlog "the test word is wrong!"
    end if

    if RecheckAsianSupport = False then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_54

    Dim sSpacingToContentsNoBorder as String   'no border in the page
    Dim sSpacingToContentsYesBorder as String   'border exists  in the page

    sSpacingToContentsNoBorder  = "0" + gSeperator + "0" '+ gMeasurementUnit
    sSpacingToContentsYesBorder = "0" + gSeperator + "05" '+ gMeasurementUnit

    PrintLog "- Only when a border is set, 'spacing to contents' gets a minimum value of 0.05cm"
    '/// Only when a border is set, "spacing to contents" gets a minimum value of 0.05cm

    Call hNewDocument

    PrintLog "Check if 'spacing to contents' is 0.00cm when there is no border in the page"
    '/// format/Page , tabpage borders ,
    '/// + Check if "spacing to contents" is 0.00cm when
    '/// + there is no border in the page
     fFormatPageWriter("TabUmrandung")
       if InStr( Links.GetText, sSpacingToContentsNoBorder) = false then Warnlog "Default of left Spacing to contents is wrong !"
       if InStr( Rechts.GetText, sSpacingToContentsNoBorder) = false then Warnlog "Default of right Spacing to contents is wrong !"
       if InStr( Oben.GetText, sSpacingToContentsNoBorder) = false then Warnlog "Default of up Spacing to contents is wrong !"
       if InStr( Unten.GetText, sSpacingToContentsNoBorder) = false then Warnlog "Default of bottom Spacing to contents is wrong !"
     TabUmrandung.Cancel

    PrintLog "Check if 'spacing to contents' is 0.50cm when there are borders in the page"
    '/// format/Page , tabpage borders
    '/// + Check if "spacing to contents" is 0.05cm when
    '/// + there are borders in the page
     fFormatPageWriter("TabUmrandung")
        Vorgaben.TypeKeys "<Right>" , 2
        Sleep 2
     TabUmrandung.OK

     fFormatPageWriter("TabUmrandung")
       if InStr( Links.GetText, sSpacingToContentsYesBorder) = false then Warnlog "Default of left Spacing to contents is wrong !"
       if InStr( Rechts.GetText, sSpacingToContentsYesBorder) = false then Warnlog "Default of right Spacing to contents is wrong !"
       if InStr( Oben.GetText, sSpacingToContentsYesBorder) = false then Warnlog "Default of up Spacing to contents is wrong !"
       if InStr( Unten.GetText, sSpacingToContentsYesBorder) = false then Warnlog "Default of bottom Spacing to contents is wrong !"
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_55

    Dim sSpacingToContents   as String   'the max spacing to content
    Dim sSpacingToContentsL  as String
    Dim sSpacingToContentsR  as String
    Dim sSpacingToContentsU  as String
    Dim sSpacingToContentsB  as String

    sSpacingToContents  = "5" + gSeperator + "0" '+ gMeasurementUnit
    sSpacingToContentsL = "6" + gSeperator + "0" '+ gMeasurementUnit
    sSpacingToContentsR = "7" + gSeperator + "0" '+ gMeasurementUnit
    sSpacingToContentsU = "8" + gSeperator + "0" '+ gMeasurementUnit
    sSpacingToContentsB = "9" + gSeperator + "0" '+ gMeasurementUnit

    PrintLog "- All 'spacing to contents' spin boxes are limited to 5cm"
    '/// All "spacing to contents" spin boxes are limited to 5cm

    Call hNewDocument

    'set border exists in the page
     fFormatPageWriter("TabUmrandung")
        Vorgaben.TypeKeys "<Right>" , 2
        Sleep 2
     TabUmrandung.OK

    '/// format/Page , tabpage borders , set all spacing to content's size
    '/// + more than 5.00cm
     fFormatPageWriter("TabUmrandung")
       Synchronisieren.Uncheck
       Links.SetText  sSpacingToContentsL
       Rechts.SetText sSpacingToContentsR
       Oben.SetText   sSpacingToContentsU
       Unten.SetText  sSpacingToContentsB
     TabUmrandung.OK

    '/// Check if all "spacing to contents" is 5.00cm
     fFormatPageWriter("TabUmrandung")

       if InStr( Links.GetText, sSpacingToContents) = false then Warnlog "left Spacing to contents is wrong !"
       if InStr( Rechts.GetText, sSpacingToContents) = false then Warnlog "right Spacing to contents is wrong !"
       if InStr( Oben.GetText, sSpacingToContents) = false then Warnlog "up Spacing to contents is wrong !"
       if InStr( Unten.GetText, sSpacingToContents) = false then Warnlog "bottom Spacing to contents is wrong !"

     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_56

    Dim sSpacingToContentsL  as String
    Dim sSpacingToContentsR  as String
    Dim sSpacingToContentsU  as String
    Dim sSpacingToContentsB  as String

    sSpacingToContentsL = "0" + gSeperator + "5" '+ gMeasurementUnit
    sSpacingToContentsR = "0" + gSeperator + "6" '+ gMeasurementUnit
    sSpacingToContentsU = "0" + gSeperator + "7" '+ gMeasurementUnit
    sSpacingToContentsB = "0" + gSeperator + "8" '+ gMeasurementUnit

    PrintLog "- Test Uncheck 'Synchronize'"
    '///  Test Uncheck 'Synchronize'

    Call hNewDocument

    'set border exists in the page
     fFormatPageWriter("TabUmrandung")
        Vorgaben.TypeKeys "<Right>" , 2
        Sleep 2
     TabUmrandung.OK

    '/// format/Page , tabpage borders
    '/// + Uncheck synchronize
    '/// + Set all spacing to contents
     fFormatPageWriter("TabUmrandung")
       Synchronisieren.Uncheck
       Sleep 1
       Links.SetText  sSpacingToContentsL
       Rechts.SetText sSpacingToContentsR
       Oben.SetText   sSpacingToContentsU
       Unten.SetText  sSpacingToContentsB
     TabUmrandung.OK
     Sleep 1

    '/// Check if all "spacing to contents" is correct
     fFormatPageWriter("TabUmrandung")
       if InStr( Links.GetText, sSpacingToContentsL) = false then Warnlog "left Spacing to contents is wrong !"
       if InStr( Rechts.GetText, sSpacingToContentsR) = false then Warnlog "right Spacing to contents is wrong !"
       if InStr( Oben.GetText, sSpacingToContentsU) = false then Warnlog "up Spacing to contents is wrong !"
       if InStr( Unten.GetText, sSpacingToContentsB) = false then Warnlog "bottom Spacing to contents is wrong !"
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_57

    Dim sSpacingToContentsL  as String
    Dim sSpacingToContentsR  as String
    Dim sSpacingToContentsU  as String
    Dim sSpacingToContentsB  as String

    sSpacingToContentsL = "0" + gSeperator + "5" '+ gMeasurementUnit
    sSpacingToContentsR = "0" + gSeperator + "6" '+ gMeasurementUnit
    sSpacingToContentsU = "0" + gSeperator + "7" '+ gMeasurementUnit
    sSpacingToContentsB = "0" + gSeperator + "8" '+ gMeasurementUnit

    PrintLog "- Test Check 'Synchronize'"
    '///  Test Check 'Synchronize'

    Call hNewDocument

    'set border exists in the page
     fFormatPageWriter("TabUmrandung")
        Vorgaben.TypeKeys "<Right>" , 2
        Sleep 2
     TabUmrandung.OK

    '/// format/Page , tabpage borders
    '/// + Check synchronize
    '/// + Set all spacing to contents
     fFormatPageWriter("TabUmrandung")
       Synchronisieren.Check
       Sleep 1
       Links.SetText  sSpacingToContentsL
       Rechts.SetText sSpacingToContentsR
       Oben.SetText   sSpacingToContentsU
       Unten.SetText  sSpacingToContentsB
     TabUmrandung.OK
     Sleep 1

    '/// Check if all "spacing to contents" is correct
     fFormatPageWriter("TabUmrandung")
       if InStr( Links.GetText, sSpacingToContentsL) = true then Warnlog "left Spacing to contents is wrong !"
       if InStr( Rechts.GetText, sSpacingToContentsR) = true then Warnlog "right Spacing to contents is wrong !"
       if InStr( Oben.GetText, sSpacingToContentsU) = true then Warnlog "up Spacing to contents is wrong !"
       if InStr( Unten.GetText, sSpacingToContentsB) = false then Warnlog "bottom Spacing to contents is wrong !"
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_58

    Dim sDefaultShadowColor as String

    Select case iSprache
        case 01   : sDefaultShadowColor = "Gray"
        case 31   : sDefaultShadowColor = "Grijs"
        case 33   : sDefaultShadowColor = "Gris"
        case 34   : sDefaultShadowColor = "Gris"
        case 39   : sDefaultShadowColor = "Grigio"
        case 46   : sDefaultShadowColor = "Grått"
        case 49   : sDefaultShadowColor = "Grau"
        case 50   : sDefaultShadowColor = "Siva"
        case 55   : sDefaultShadowColor = "Cinza"
        case 81   : sDefaultShadowColor = "灰色"
        case 82   : sDefaultShadowColor = "회색"
        case 86   : sDefaultShadowColor = "灰色"
        case 88   : sDefaultShadowColor = "灰色"
        case else : QAErrorLog "The test does not support the language " + iSprache
                    Goto endsub
    end select

    PrintLog "- Test default shadow color"
    '///  Test default shadow color

    Call hNewDocument

    '/// format/Page , tabpage borders
    '/// + Check if the default shadow color is gray
     fFormatPageWriter("TabUmrandung")
        if SchattenFarbe.GetSelText <> sDefaultShadowColor then
            Warnlog "Shadow default color is wrong , get " &SchattenFarbe.GetSelText
        end if
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_59

    Dim iBorderColor as Integer  ,  iShadowColor as Integer
    Dim sSpacingToContentsL           as String
    Dim sSpacingToContentsR           as String
    Dim sSpacingToContentsU           as String
    Dim sSpacingToContentsB           as String
    Dim sShadowDistance               as String

    iBorderColor = 5 : iShadowColor = 4

    sSpacingToContentsL = "0" + gSeperator + "5" '+ gMeasurementUnit
    sSpacingToContentsR = "0" + gSeperator + "6" '+ gMeasurementUnit
    sSpacingToContentsU = "0" + gSeperator + "7" '+ gMeasurementUnit
    sSpacingToContentsB = "0" + gSeperator + "8" '+ gMeasurementUnit
    sShadowDistance     = "2" + gSeperator + "54" '+ gMeasurementUnit

    PrintLog "- Test if the shadow settings is OK after closing the dialog"
    '///  Test if the shadow settings is OK after closing the dialog

    Call hNewDocument

    '/// format/Page , tabpage borders , do some configuration
    '/// + and close the dialog
     fFormatPageWriter("TabUmrandung")
       Vorgaben.TypeKeys "<Right>" , 2
       Sleep 2
       Synchronisieren.UnCheck
       Sleep 1
       Links.SetText    sSpacingToContentsL
       Rechts.SetText   sSpacingToContentsR
       Oben.SetText     sSpacingToContentsU
       Unten.SetText    sSpacingToContentsB
       StilFarbe.Select iBorderColor            'set border color

       Position.TypeKeys "<Right>" , 4
       Sleep 2
       Groesse.SetText      sShadowDistance
       SchattenFarbe.Select iShadowColor        'set shadow color
       Sleep 1
     TabUmrandung.OK

    '/// format/Page , check if the configuration is all right
     fFormatPageWriter("TabUmrandung")
       if Synchronisieren.IsChecked = TRUE          then Warnlog "Synchronize should be Unchecked !"
       if InStr( Links.GetText, sSpacingToContentsL) = false then Warnlog "left Spacing to contents is wrong !"
       if InStr( Rechts.GetText, sSpacingToContentsR) = false then Warnlog "right Spacing to contents is wrong !"
       if InStr( Oben.GetText, sSpacingToContentsU) = false then Warnlog "up Spacing to contents is wrong !"
       if InStr( Unten.GetText, sSpacingToContentsB) = false then Warnlog "bottom Spacing to contents is wrong !"
       if StilFarbe.GetSelIndex <> iBorderColor     then Warnlog "border color is wrong !"

       if InStr( Groesse.GetText, sShadowDistance) = false then Warnlog "shadow distance is wrong !"
       if SchattenFarbe.GetSelindex <> iShadowColor then Warnlog "shadow color is wrong !"
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_60

    Dim iBorderColor as Integer  ,  iShadowColor as Integer
    Dim sSpacingToContentsL           as String
    Dim sSpacingToContentsR           as String
    Dim sSpacingToContentsU           as String
    Dim sSpacingToContentsB           as String
    Dim sShadowDistance               as String

    iBorderColor = 5 : iShadowColor = 4

    sSpacingToContentsL = "0" + gSeperator + "5" '+ gMeasurementUnit
    sSpacingToContentsR = "0" + gSeperator + "6" '+ gMeasurementUnit
    sSpacingToContentsU = "0" + gSeperator + "7" '+ gMeasurementUnit
    sSpacingToContentsB = "0" + gSeperator + "8" '+ gMeasurementUnit
    sShadowDistance     = "2" + gSeperator + "74" '+ gMeasurementUnit

    PrintLog "- Test if the shadow settings is OK after reopening the file"
    '///  Test if the shadow settings is OK after reopening the file

    Call hNewDocument

    '/// format/Page , tabpage borders , do some configuration
    '/// + and close the dialog
    fFormatPageWriter("TabUmrandung")
    Vorgaben.TypeKeys "<Right>" , 2
    Sleep 2
    Synchronisieren.UnCheck
    Sleep 1
    Links.SetText    sSpacingToContentsL
    Rechts.SetText   sSpacingToContentsR
    Oben.SetText     sSpacingToContentsU
    Unten.SetText    sSpacingToContentsB
    StilFarbe.Select iBorderColor            'set border color

    Position.TypeKeys "<Right>" , 4
    Sleep 2
    Groesse.SetText      sShadowDistance
    SchattenFarbe.Select iShadowColor        'set shadow color
    Sleep 1
    TabUmrandung.OK

    '/// Save the file , close the file and reopen it
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\tFormatPage_60.odt", "writer8")
    Call hCloseDocument

    Call hNewDocument
    hFileOpen( gOfficepath + "user\work\tFormatPage_60.odt")
    

    '/// check if the configuration is all right
    fFormatPageWriter("TabUmrandung")
    if Synchronisieren.IsChecked = TRUE          then Warnlog "Synchronize should be Unchecked !"
    if InStr( Links.GetText, sSpacingToContentsL) = false then Warnlog "left Spacing to contents is wrong !"
    if InStr( Rechts.GetText, sSpacingToContentsR) = false then Warnlog "right Spacing to contents is wrong !"
    if InStr( Oben.GetText, sSpacingToContentsU) = false then Warnlog "up Spacing to contents is wrong !"
    if InStr( Unten.GetText, sSpacingToContentsB) = false then Warnlog "bottom Spacing to contents is wrong !"
    if StilFarbe.GetSelIndex <> iBorderColor     then Warnlog "border color is wrong !"

       if InStr( Groesse.GetText, sShadowDistance) = false then Warnlog "shadow distance is wrong !"
    if SchattenFarbe.GetSelindex <> iShadowColor then Warnlog "shadow color is wrong !"
    TabUmrandung.Cancel

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'--------------------------------------------------
