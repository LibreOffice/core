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
'* short description : Test the AutoCorrect/AutoFormat/Options
'*
'\***********************************************************************

sub w_autocorrect1

    printLog Chr(13) + "- Test the AutoCorrect/AutoFormat Options features"

    Call tReplaceDashes_1                'Replace all of dashes    
    '## Tab page Options in Tools/Autocorrect|Autoformat ##
    Call tToolsAutocorrectOptions_1      'Use Replacement table
    Call tToolsAutocorrectOptions_2      'Correct TWo INitial CApital
    Call tToolsAutocorrectOptions_3      'Capitalize first letter of every setence
    Call tToolsAutocorrectOptions_4_1    'Automatic bold
    Call tToolsAutocorrectOptions_4_2    'Automatic underline
    Call tToolsAutocorrectOptions_5      'URL Recognition
    Call tToolsAutocorrectOptions_6      'Replace dashes
    Call tToolsAutocorrectOptions_7     'Ignore double spaces
    Call tToolsAutocorrectOptions_8     'Apply numbering
    Call tToolsAutocorrectOptions_9     'Apply Border
    Call tToolsAutocorrectOptions_10     'Create Table
    Call tToolsAutocorrectOptions_11     'Remove Blank Paragraphs
    Call tToolsAutocorrectOptions_12     'Replace bullets with •
    Call tToolsAutocorrectOptions_13     'Replace standard quotes with custom quotes

end sub

'-------------------------------------------------------------------------

testcase tReplaceDashes_1

   Dim testWord as string, testResult as String
   Dim i as Integer
   Dim j as Integer

   printLog "Replace dashes test"
  '/// Replace dashes test
  '/// - "Bla - Bla"           One short line between two words (with Blank before and after)
  '/// - "Bla--Bla"            Dashes lines between two words (without Blanks)
  '/// - "Helen and --Tom"     Dashes Lines before a Word (one Blank before, none after)
  '/// - "(Bla--Bla)"          Testing  Text in brackets ( )
  '/// - "[Bla--Bla]"          Testing  Text in brackets [ ]
  '/// - "{Bla--Bla}"          Testing  Text in brackets { }

   Call hNewDocument

   For i = 1 to 6
       Select Case i
            Case 1 : testWord = "Bla - Bla"         : testResult = "Bla – Bla"
            Case 2 : testWord = "Bla--Bla"          : testResult = "Bla—Bla"
            Case 3 : testWord = "Helen and --Tom"   : testResult = "Helen and –Tom"
            Case 4 : testWord = "(Bla--Bla)"        : testResult = "(Bla—Bla)"
            Case 5 : testWord = "[Bla--Bla]"        : testResult = "[Bla—Bla]"
            Case 6 : testWord = "{Bla--Bla}"        : testResult = "{Bla—Bla}"
        end select

        printlog "- Replace dashes " & testword & " => " & testResult
        Call wTypeKeys testWord
        Call wTypeKeys " "
        Sleep 1
        Call wTypeKeys "<Left><Shift Home>"
        EditCopy
        if GetClipboardText <> testResult then Warnlog "Something wrong in the test for " + testWord(i)
        Call wTypeKeys "<DELETE>"
    next i

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_1
   
    printLog "Test 'Use Replacement table'"
    '/// Test 'Use Replacement table'
    Call hNewDocument
    '/// Insert new test word in replacement table 
    '/// Tools/AutoCorrect / Replace , insert new test words in new and replace ,
    '/// + and press button New.
    ToolsAutoCorrect
    Kontext
    Active.SetPage TabErsetzung
    Kontext "TabErsetzung"
    Kuerzel.SetText "Aabbcc"
    ErsetzenDurch.SetText "Aaccbb"
    Sleep 1
    try
        Neu.Click
    catch
        Warnlog "Seems settings are already made. Please remove entries from replacement table"
    endcatch
    Sleep 1
    TabErsetzung.OK
    
    printlog "- M:Y  T:Y (default)"
    ToolsAutoCorrect
    Kontext
    Active.SetPage TabOptionenAutokorrektur
    Kontext "TabOptionenAutokorrektur"
    if Einstellungen.IsChecked(1,2) = FALSE then
        Warnlog "First Checkbox not checked by default"
        Einstellungen.Check(1,2)
    endif
    if Einstellungen.IsChecked(1,3) = FALSE then
        Warnlog "Second Checkbox not checked by default"
        Einstellungen.Check(1,3)
    endif
    TabOptionenAutokorrektur.Ok
    
    Call wTypeKeys "Aabbcc"
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> "Aaccbb" then Warnlog "Something wrong in Use Replacement table when M:Y  T:Y"
    
    Call wTypeKeys "<DELETE>"
    
    printlog "- M:Y  T:N"
    Call fAutocorrectOptions("UseReplacementTable", 1, 0)
    
    Call wTypeKeys "Aabbcc"
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    
    EditCopy
    if GetClipboardText <> "Aabbcc" then
        Warnlog "The content should NOT be changed before Autoformat! M:Y  T:N"
    else    
        Call wTypeKeys "<ESCAPE>"
        FormatAutoformatApply
        Call wTypeKeys "<Home><Shift End>"
        GetClipboardText = ""
        EditCopy
        if GetClipboardText <> "Aaccbb" then
            Warnlog "The content should be changed after Autoformat!M:Y  T:N"
        end if
    end if
    
    Call wTypeKeys "<DELETE>"
    
    printlog "- M:N  T:Y"
    Call fAutocorrectOptions("UseReplacementTable", 0, 1)
    
    Call wTypeKeys "Aabbcc"
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    GetClipboardText = ""
    EditCopy
    if GetClipboardText <> "Aaccbb" then Warnlog "Something wrong in Use Replacement table when M:N  T:Y"
    
    Call wTypeKeys "<DELETE>"
    
    printlog "- M:N  T:N"
    Call fAutocorrectOptions("UseReplacementTable", 0, 0)
    
    Call wTypeKeys "Aabbcc"
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> "Aabbcc" then
        Warnlog "The content should NOT be changed before Autoformat! M:N  T:N"
        Call hCloseDocument
        goto endsub
    else    
        Call wTypeKeys "<ESCAPE>"
        FormatAutoformatApply
        Call wTypeKeys "<Home><Shift End>"
        GetClipboardText = ""
        EditCopy
        if GetClipboardText <> "Aabbcc" then
            Warnlog "The content should NOT be changed after Autoformat! M:N  T:N"
        end if
    end if        

    printlog "- Recover to the default  M:Y  T:Y"
     Call fAutocorrectOptions("UseReplacementTable", 1, 1)

    'remove the test words which you insert in the replacement table
    Dim i as Integer
    ToolsAutoCorrect
    Kontext
    Active.SetPage TabErsetzung
    Kontext "TabErsetzung"
    for i =1 to Liste.GetItemCount
        if Kuerzel.GetText = "Aabbcc" then
            i = Liste.GetItemCount + 1
            Loeschen.Click
        else
            Liste.Typekeys "<Down>"
        end if
    next i
    TabErsetzung.OK
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_2

    Dim testContent as String
    Dim hopeResult as String
    Dim hopeResultBeforeAutoformat as String
    Dim hopeResultAfterAutoformat as String

    printLog "Test 'Correct TWo INitial CApital'  "
    '/// Test 'Correct TWo INitial CApital'
    Call hNewDocument

    '/// M:Y  T:Y (default)
    testContent = "TEst"
    hopeResult  = "Test"
    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResult then Warnlog "Something wrong in Correct TWo INitial CApital when M:Y  T:Y"

    Call wTypeKeys "<DELETE>"

    '/// M:Y  T:N
    testContent                 = "TEst"
    hopeResultBeforeAutoformat  = "TEst"
    hopeResultAfterAutoformat   = "Test"

    Call fAutocorrectOptions("CorrectTWoINitialCApitals",1,0)

    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"

    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then
    Warnlog "The content should NOT be changed before Autoformat! M:Y  T:N"
    goto GOING1
    end if

    Call wTypeKeys "<ESCAPE>"
    FormatAutoformatApply
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then
    Warnlog "The content should be changed after Autoformat!M:Y  T:N"
    end if

    GOING1:
    Call wTypeKeys "<DELETE>"

    '/// M:N  T:Y
    testContent  = "TEst"
    hopeResult   = "Test"

    Call fAutocorrectOptions("CorrectTWoINitialCApitals",0,1)

    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResult then Warnlog "Something wrong in Correct TWo INitial CApital when M:N  T:Y"

    Call wTypeKeys "<DELETE>"

    '/// M:N  T:N
    testContent                 = "TEst"
    hopeResultBeforeAutoformat  = "TEst"
    hopeResultAfterAutoformat   = "TEst"

    Call fAutocorrectOptions("CorrectTWoINitialCApitals",0,0)

    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then
        Warnlog "The content should NOT be changed before Autoformat! M:N  T:N"
        Call hCloseDocument
    end if

    Call wTypeKeys "<ESCAPE>"
    FormatAutoformatApply
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then
        Warnlog "The content should NOT be changed after Autoformat! M:N  T:N"
    end if

    '/// Recover to the default  M:Y  T:Y
    Call fAutocorrectOptions("CorrectTWoINitialCApitals",1,1)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_3

    Dim testContent as String
    Dim hopeResult as String
    Dim hopeResultBeforeAutoformat as String
    Dim hopeResultAfterAutoformat as String

    printLog "Test 'Capitalize first letter of every setence'"
    '/// Test 'Capitalize first letter of every setence'
    Call hNewDocument

    '/// M:Y  T:Y (default)
     testContent = "the test for StarOffice automation. who is the author?"
     hopeResult  = "The test for StarOffice automation. Who is the author?"
     Call wTypeKeys testContent
     Call wTypeKeys "<Shift Home>"
     EditCopy
     if GetClipboardText <> hopeResult then Warnlog "Something wrong in Capitalize first letter of every setence when M:Y  T:Y"

     Call wTypeKeys "<DELETE>"

    '/// M:Y  T:N
     testContent                 = "the test for StarOffice automation. who is the author?"
     hopeResultBeforeAutoformat  = "the test for StarOffice automation. who is the author?"
     hopeResultAfterAutoformat   = "The test for StarOffice automation. Who is the author?"

     Call fAutocorrectOptions("CapitalizeFirstLetter",1,0)
     Call wTypeKeys testContent
     Call wTypeKeys "<Shift Home>"
     EditCopy
     if GetClipboardText <> hopeResultBeforeAutoformat then
        Warnlog "The content should NOT be changed before Autoformat! M:Y  T:N"
        goto GOING1
     end if

     Call wTypeKeys "<ESCAPE>"
     FormatAutoformatApply
     Call wTypeKeys "<Shift End>"
     EditCopy
     if GetClipboardText <> hopeResultAfterAutoformat then
        Warnlog "The content should be changed after Autoformat!M:Y  T:N"
     end if

GOING1:
     Call wTypeKeys "<DELETE>"

    '/// M:N  T:Y
     testContent  = "the test for StarOffice automation. who is the author?"
     hopeResult   = "The test for StarOffice automation. Who is the author?"

     Call fAutocorrectOptions("CapitalizeFirstLetter",0,1)
     Call wTypeKeys testContent
     Call wTypeKeys "<Shift Home>"
     EditCopy
     if GetClipboardText <> hopeResult then Warnlog "Something wrong in Capitalize first letter of every setence when M:N  T:Y"

     Call wTypeKeys "<DELETE>"

    '/// test 'Capitalize first letter of every setence' , M:N  T:N
     testContent                 = "the test for StarOffice automation. who is the author?"
     hopeResultBeforeAutoformat  = "the test for StarOffice automation. who is the author?"
     hopeResultAfterAutoformat   = "the test for StarOffice automation. who is the author?"

     Call fAutocorrectOptions("CapitalizeFirstLetter",0,0)

     Call wTypeKeys testContent
     Call wTypeKeys "<Shift Home>"
     EditCopy
     if GetClipboardText <> hopeResultBeforeAutoformat then
        Warnlog "The content should NOT be changed before Autoformat! M:N  T:N"
        goto testend
     end if

     Call wTypeKeys "<ESCAPE>"
     FormatAutoformatApply
     Call wTypeKeys "<Shift End>"
     EditCopy
     if GetClipboardText <> hopeResultAfterAutoformat then
        Warnlog "The content should NOT be changed after Autoformat! M:N  T:N"
     end if

testend:

    '/// Recover to the default
     Call fAutocorrectOptions("CapitalizeFirstLetter",1,1)

    Call hCloseDocument


endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_4_1

    Dim testContent as String
    Dim hopeResult as String
    Dim hopeResultBeforeAutoformat as String
    Dim hopeResultAfterAutoformat as String
    Dim getStyle as String
    Dim hopeStyle as String

    printLog "Test 'Automatic bold and underline' --- bold  "
    '/// Test 'Automatic bold'
    Call hNewDocument

    '/// M:Y  T:Y (default)
     testContent = "*Test*"
     hopeResult  = "Test"

     Call wTypeKeys testContent
     Call wTypeKeys "<Shift Home>"
     EditCopy
     if GetClipboardText <> hopeResult then Warnlog "Something wrong in Automatic bold when M:Y  T:Y"

     fFormatCharacter("TabFont")

    try
        getStyle = Style.GetSelText
        Style.Select 3
        hopeStyle = Style.GetSelText
    catch
        getStyle = StyleWest.GetSelText
        StyleWest.Select 3
        hopeStyle = StyleWest.GetSelText
    endcatch
    Sleep 1

    if getStyle <> hopeStyle then Warnlog "The style is NOT bold style when M:Y  T:Y"

    try
        Style.Select 1
    catch
        StyleWest.Select 1
    endcatch
    Sleep 1

    TabFont.OK
    Sleep 1

    Call wTypeKeys "<DELETE>"

    '/// M:Y  T:N
    testContent                 = "*Test*"
    hopeResultBeforeAutoformat  = "*Test*"
    hopeResultAfterAutoformat   = "Test"

    Call fAutocorrectOptions("AutomaticBoldAndUnderline",1,0)

    Call wTypeKeys testContent
    Call wTypeKeys "<Shift Home>"

    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then
    Warnlog "The content should NOT be changed before Autoformat! M:Y  T:N"
    goto GOING1
    end if

    fFormatCharacter("TabFont")

    try
        getStyle = Style.GetSelText
        Style.Select 1
        hopeStyle = Style.GetSelText
    catch
        getStyle = StyleWest.GetSelText
        StyleWest.Select 1
        hopeStyle = StyleWest.GetSelText
    endcatch
    Sleep 1

    if getStyle <> hopeStyle then Warnlog "The style should be standard style before autoformat!"
    TabFont.Cancel

    Call wTypeKeys "<ESCAPE>"
    FormatAutoformatApply
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then
        Warnlog "The content should be changed after Autoformat!M:Y  T:N"
    end if

    Call fFormatCharacter("TabFont")

    try
        getStyle = Style.GetSelText
        Style.Select 3
        hopeStyle = Style.GetSelText
    catch
        getStyle = StyleWest.GetSelText
        StyleWest.Select 3
        hopeStyle = StyleWest.GetSelText
    endcatch
    Sleep 1

    if getStyle <> hopeStyle then Warnlog "The style is NOT bold style when M:Y  T:Y"

    try
        Style.Select 1
    catch
        StyleWest.Select 1
    endcatch
    Sleep 1

    TabFont.OK
    Sleep 1

    GOING1:

    Call wTypeKeys "<DELETE>"

    '/// M:N  T:Y
    testContent = "*Test*"
    hopeResult  = "Test"

    Call fAutocorrectOptions("AutomaticBoldAndUnderline",0,1)

    Call wTypeKeys testContent
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResult then Warnlog "Something wrong in Automatic bold when M:Y  T:Y"

    Call fFormatCharacter("TabFont")

    try
        getStyle = Style.GetSelText
        Style.Select 3
        hopeStyle = Style.GetSelText
    catch
        getStyle = StyleWest.GetSelText
        StyleWest.Select 3
        hopeStyle = StyleWest.GetSelText
    endcatch
    Sleep 1

    if getStyle <> hopeStyle then Warnlog "The style is NOT bold style when M:Y  T:Y"
    try
        Style.Select 1
    catch
        StyleWest.Select 1
    endcatch
    TabFont.OK
    Sleep 1

    Call wTypeKeys "<DELETE>"

    '/// M:N  T:N
    testContent                 = "*Test*"
    hopeResultBeforeAutoformat  = "*Test*"
    hopeResultAfterAutoformat   = "*Test*"

    Call fAutocorrectOptions("AutomaticBoldAndUnderline",0,0)

    Call wTypeKeys testContent
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then
        Warnlog "The content should NOT be changed before Autoformat! M:N  T:N"
        goto testend
    end if

    Call fFormatCharacter("TabFont")

    try
        getStyle = Style.GetSelText
        Style.Select 1
        hopeStyle = Style.GetSelText
    catch
        getStyle = StyleWest.GetSelText
        StyleWest.Select 1
        hopeStyle = StyleWest.GetSelText
    endcatch
    Sleep 1

    if getStyle <> hopeStyle then Warnlog "The style should be standard style before autoformat!"
    TabFont.Cancel

    Call wTypeKeys "<ESCAPE>"
    FormatAutoformatApply
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then
        Warnlog "The content should NOT be changed after Autoformat! M:N  T:N"
    end if

    Call fFormatCharacter("TabFont")

    try
        getStyle = Style.GetSelText
        Style.Select 1
        hopeStyle = Style.GetSelText
    catch
        getStyle = StyleWest.GetSelText
        StyleWest.Select 1
        hopeStyle = StyleWest.GetSelText
    endcatch
    Sleep 1

    if getStyle <> hopeStyle then Warnlog "The style should be standard style after autoformat!"
    TabFont.OK
    Sleep 1

    testend:
    '/// Recover to the default  M:Y  T:Y
    Call fAutocorrectOptions("AutomaticBoldAndUnderline",1,1)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_4_2

    Dim testContent as String
    Dim hopeResult as String
    Dim hopeResultBeforeAutoformat as String
    Dim hopeResultAfterAutoformat as String
    Dim getUnderlineStatus as Integer
    Dim hopeUnderlineStatus as Integer  ' 1 = without  , 2 = single

    printLog "Test 'Automatic bold and underline' --- underline  "
    '/// Test 'Automatic underline'
    Call hNewDocument

    '/// M:Y  T:Y (default)
    testContent = "_Test_"
    hopeResult  = "Test"

    Call wTypeKeys testContent
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResult then Warnlog "Something wrong in Automatic underline when M:Y  T:Y"

    Call fFormatCharacter("TabFontEffects")

    getUnderlineStatus  = Underline.GetSelIndex
    Underline.Select 2
    hopeUnderlineStatus = Underline.GetSelIndex

    if getUnderlineStatus <> hopeUnderlineStatus then Warnlog "The content is NOT underlined when M:Y  T:Y"
    Underline.Select 1
    TabFontEffects.OK
    Sleep 1

    Call wTypeKeys "<DELETE>"

    '/// M:Y  T:N
    testContent                 = "_Test_"
    hopeResultBeforeAutoformat  = "_Test_"
    hopeResultAfterAutoformat   = "Test"

    Call fAutocorrectOptions("AutomaticBoldAndUnderline",1,0)

    Call wTypeKeys testContent
    Call wTypeKeys "<Shift Home>"

    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then
        Warnlog "The content should NOT be changed before Autoformat! M:Y  T:N"
        goto GOING1
    end if

    Call fFormatCharacter("TabFontEffects")

    getUnderlineStatus  = Underline.GetSelIndex
    Underline.Select 1
    Sleep 1
    hopeUnderlineStatus = Underline.GetSelIndex

    if getUnderlineStatus <> hopeUnderlineStatus then Warnlog "The content should NOT be underlined when M:Y  T:N before autoformat!"
    TabFontEffects.OK
    Sleep 1

    Call wTypeKeys "<ESCAPE>"
    FormatAutoformatApply
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then
        Warnlog "The content should be changed after Autoformat!M:Y  T:N"
    end if

    Call fFormatCharacter("TabFontEffects")
    getUnderlineStatus  = Underline.GetSelIndex
    Underline.Select 2
    hopeUnderlineStatus = Underline.GetSelIndex

    if getUnderlineStatus <> hopeUnderlineStatus then Warnlog "The content should be underlined when M:Y  T:N"
    Underline.Select 1
    TabFontEffects.OK
    Sleep 1
    GOING1:
    Call wTypeKeys "<DELETE>"

    '/// M:N  T:Y
    testContent = "_Test_"
    hopeResult  = "Test"

    Call fAutocorrectOptions("AutomaticBoldAndUnderline",0,1)

    Call wTypeKeys testContent
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResult then Warnlog "Something wrong in Automatic bold when M:N  T:Y"

    Call fFormatCharacter("TabFontEffects")
    getUnderlineStatus  = Underline.GetSelIndex
    Underline.Select 2
    Sleep 1
    hopeUnderlineStatus = Underline.GetSelIndex

    if getUnderlineStatus <> hopeUnderlineStatus then Warnlog "The content is NOT underlined when M:N  T:Y"
    Underline.Select 1
    Sleep 1
    TabFontEffects.OK
    Sleep 1

    Call wTypeKeys "<DELETE>"

    '/// M:N  T:N
    testContent                 = "_Test_"
    hopeResultBeforeAutoformat  = "_Test_"
    hopeResultAfterAutoformat   = "_Test_"

    Call fAutocorrectOptions("AutomaticBoldAndUnderline",0,0)

    Call wTypeKeys testContent
    Call wTypeKeys "<Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then
        Warnlog "The content should NOT be changed before Autoformat! M:N  T:N"
        goto testend
    end if

    Call fFormatCharacter("TabFontEffects")
    getUnderlineStatus  = Underline.GetSelIndex
    Underline.Select 1
    hopeUnderlineStatus = Underline.GetSelIndex

    if getUnderlineStatus <> hopeUnderlineStatus then Warnlog "The content should NOT be underlined before autoformat!M:N  T:N "
    TabFontEffects.OK

    Call wTypeKeys "<ESCAPE>"
    FormatAutoformatApply
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then
    Warnlog "The content should NOT be changed after Autoformat! M:N  T:N"
    end if

    Call fFormatCharacter("TabFontEffects")
    getUnderlineStatus  = Underline.GetSelIndex
    Underline.Select 1
    Sleep 1
    hopeUnderlineStatus = Underline.GetSelIndex

    if getUnderlineStatus <> hopeUnderlineStatus then Warnlog "The content should NOT be underlined after autoformat! M:N  T:N"
    TabFontEffects.OK
    Sleep 1

    testend:
    '/// Recover to the default  M:Y  T:Y
    Call fAutocorrectOptions("AutomaticBoldAndUnderline",1,1)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_5

    Dim testContent as String
    Dim hopeResult as String
    Dim hopeResultBeforeAutoformat as String
    Dim hopeResultAfterAutoformat as String

    printLog "URL Recognition"
    '/// URL Recognition
    Call hNewDocument

    '/// M:Y  T:Y (default)
    testContent = "WWW.sun.com"
    hopeResult  = "http://WWW.sun.com/"

    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<Home>"

    Call fFormatCharacter("TabHyperlinkZeichen")
    if URLName.GetText <> hopeResult then Warnlog "The URL is NOT appeared when M:Y  T:Y"
    URLName.SetText ""
    TabHyperlinkZeichen.OK

    Call hCloseDocument

    Call hNewDocument
    '/// M:Y  T:N
    testContent                 = "WWW.sun.com"
    hopeResultBeforeAutoformat  = ""
    hopeResultAfterAutoformat   = "http://WWW.sun.com/"

    Call fAutocorrectOptions("URLRecognition",1,0)

    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<Home>"

    Call fFormatCharacter("TabHyperlinkZeichen")
    if URLName.GetText <> hopeResultBeforeAutoformat then Warnlog "The URL should NOT appeare when M:Y  T:N  before autoformat!"
    URLName.SetText ""
    TabHyperlinkZeichen.OK

    FormatAutoformatApply

    Call fFormatCharacter("TabHyperlinkZeichen")
    if URLName.GetText <> hopeResultAfterAutoformat then Warnlog "The URL should  appeare when M:Y  T:N  after autoformat!"
    URLName.SetText ""
    TabHyperlinkZeichen.OK

    Call hCloseDocument

    Call hNewDocument
    '/// M:N  T:Y
    testContent = "WWW.sun.com"
    hopeResult  = "http://WWW.sun.com/"

    Call fAutocorrectOptions("URLRecognition",0,1)

    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<Home>"

    Call fFormatCharacter("TabHyperlinkZeichen")
    if URLName.GetText <> hopeResult then Warnlog "The URL is NOT appeared when M:N  T:Y"
    URLName.SetText ""
    TabHyperlinkZeichen.OK

    Call hCloseDocument

    Call hNewDocument
    '/// M:N  T:N
    testContent                 = "WWW.sun.com"
    hopeResultBeforeAutoformat  = ""
    hopeResultAfterAutoformat   = ""

    Call fAutocorrectOptions("URLRecognition",0,0)

    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<Home>"

    Call fFormatCharacter("TabHyperlinkZeichen")
    if URLName.GetText <> hopeResultBeforeAutoformat then Warnlog "The URL should NOT appeare when M:N  T:N  before autoformat!"
    URLName.SetText ""
    TabHyperlinkZeichen.OK

    FormatAutoformatApply

    Call fFormatCharacter("TabHyperlinkZeichen")
    if URLName.GetText <> hopeResultAfterAutoformat then Warnlog "The URL should NOT appeare when M:N  T:N  after autoformat!"
    URLName.SetText ""
    TabHyperlinkZeichen.OK

    '/// Recover to the default  M:Y  T:Y
    Call fAutocorrectOptions("URLRecognition",1,1)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_6

    Dim testContent as String
    Dim hopeResult as String
    Dim hopeResultBeforeAutoformat
    Dim hopeResultAfterAutoformat as String

    printLog "Replace dashes"
    '/// Replace dashes
    Call hNewDocument

    '///  M:Y  T:Y (default)
    testContent = "Bla - Bla"
    hopeResult  = "Bla – Bla"
    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResult then Warnlog "Something wrong in Replace dashes M:Y  T:Y"

    Call wTypeKeys "<DELETE>"

    '///  M:Y  T:N
    testContent                 = "Bla - Bla"
    hopeResultBeforeAutoformat  = "Bla - Bla"
    hopeResultAfterAutoformat   = "Bla – Bla"

    Call fAutocorrectOptions("ReplaceDashes",1,0)
    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then
        Warnlog "The content should NOT be changed before Autoformat! M:Y  T:N"
        goto GOING1
    end if

    Call wTypeKeys "<ESCAPE>"
    FormatAutoformatApply
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then
        Warnlog "The content should be changed after Autoformat!M:Y  T:N"
    end if

    GOING1:
    Call wTypeKeys "<DELETE>"

    '///  M:N  T:Y
    testContent  = "Bla - Bla"
    hopeResult   = "Bla – Bla"

    Call fAutocorrectOptions("ReplaceDashes",0,1)
    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResult then Warnlog "Something wrong in Replace dashes  when M:N  T:Y"

    Call wTypeKeys "<DELETE>"

    '///  M:N  T:N
    testContent                 = "Bla - Bla"
    hopeResultBeforeAutoformat  = "Bla - Bla"
    hopeResultAfterAutoformat   = "Bla - Bla"

    Call fAutocorrectOptions("ReplaceDashes",0,0)

    Call wTypeKeys testContent
    Call wTypeKeys " "
    Call wTypeKeys "<BACKSPACE><Shift Home>"
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then
        Warnlog "The content should NOT be changed before Autoformat! M:N  T:N"
        goto testend
    end if

    Call wTypeKeys "<ESCAPE>"
    FormatAutoformatApply
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then
        Warnlog "The content should NOT be changed after Autoformat! M:N  T:N"
    end if

    testend:

    '/// Recover to the default
    Call fAutocorrectOptions("ReplaceDashes",1,1)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_7

    Dim testContent as String
    Dim hopeResult as String

    printLog "Ignore double spaces"
    '/// Ignore double spaces
    Call hNewDocument

    '/// + T:N (default)
     testContent = "Two  Space"
     hopeResult  = "Two  Space"
     Call wTypeKeys testContent
     Call wTypeKeys "<Shift Home>"
     EditCopy
     if GetClipboardText <> hopeResult then Warnlog "Should NOT ignore spaces T:N"

     Call wTypeKeys "<DELETE>"

    '/// + T:Y
     testContent = "Two  Space"
     hopeResult  = "Two Space"

     Call fAutocorrectOptions("IgnoreDoubleSpaces",1,1)
     Call wTypeKeys testContent
     Call wTypeKeys "<Shift Home>"
     EditCopy

     if GetClipboardText <> hopeResult then Warnlog "Should ignore spaces T:Y"

     Call wTypeKeys "<ESCAPE>"

    '/// Recover to the default
     Call fAutocorrectOptions("IgnoreDoubleSpaces",0,0)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_8

    Dim firstLine as String
    Dim testContent as String
    Dim hopeResult as String

    printLog "Apply numbering"
    '/// Apply numbering

    Call hNewDocument

    '/// + T:Y (default)

    firstLine   = "1. FirstLine"
    testContent = "Test1"    
    hopeResult  = "2. Test1"

    Call wTypeKeys firstLine
    Call wTypeKeys "<Return>"
    Call wTypeKeys testContent
    Call wTypeKeys "<Return>"
    Call wTypeKeys "<Up><End>"
    Call wTypeKeys "<Mod1 Shift Up>"
    EditCopy
    printlog GetClipBoardtext
    if GetClipboardText <> hopeResult then Warnlog "Should Apply numbering when T:Y. Not " & hopeResult & " but " & GetClipboardtext

    EditSelectAll
    Call wTypeKeys "<DELETE>"
    Call wTypeKeys "<BACKSPACE>",3

    '/// + T:N
    testContent = "Test1"
    hopeResult  = "Test1"

    Call fAutocorrectOptions("ApplyNumbering",0,0)
    Call wTypeKeys firstLine
    Call wTypeKeys "<Return>"
    Call wTypeKeys testContent
    Call wTypeKeys "<Return>"
    Call wTypeKeys "<Up><End>"
    Call wTypeKeys "<Mod1 Shift Up>"
    EditCopy

    if GetClipboardText <> hopeResult then Warnlog "Should NOT Apply numbering when T:N. Not " & hopeResult & " but " & GetClipboardtext

    '/// Recover to the default
    Call fAutocorrectOptions("ApplyNumbering",1,1)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_9

    Dim testList() as String
    Dim testContent as String
    Dim hopeResult as String
    Dim i as integer
    Dim bottom as String

    bottom = "0" + gSeperator + "10" + gMeasurementUnit

    testList   =  Array( "---", "___", "===", "***", "~~~" , "###" )

    '/// Apply border test , including
    '/// + '---' , '___", "===", "***" , "~~~", "###"
    printLog "Apply border"

    Call hNewDocument

    For i = 0 to 5

        printlog testlist(i)
        printlog "- T:Y (default)"
        testContent = testlist(i)

        Call wTypeKeys testContent
        Call wTypeKeys "<Return><Up><Home><Shift End>"
        try
            EditCopy
            Warnlog "Apply border is NOT correct when T:Y " +testlist(i)
        catch
        endcatch

        Call fFormatParagraph("TabUmrandung")
        try
            Unten.SetText  bottom
        catch
            Warnlog "Apply border is NOT correct when T:Y " +testlist(i)
        endcatch
        TabUmrandung.Cancel

        Call wTypeKeys "<Down>"

        printlog "- T:N"
        testContent = testlist(i)
        hopeResult  = testlist(i)

        Call fAutocorrectOptions("ApplyBorder",0,0)
        Call wTypeKeys testContent
        Call wTypeKeys "<Return><Up><Home><Shift End>"
        EditCopy

        if GetClipboardText <> hopeResult then Warnlog "Something wrong  when T:N "+testlist(i)

        Call fFormatParagraph("TabUmrandung")
        try
            Unten.SetText  bottom
            Warnlog "Apply border is NOT correct when T:N "+testlist(i)
        catch
        endcatch
        TabUmrandung.Cancel

        printlog "- Recover to the default"
        Call fAutocorrectOptions("ApplyBorder",1,1)

    next i

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_10

    Dim testContent as String
    Dim hopeResult as String

    printLog "Create Table"
    '/// Create Table
    Call hNewDocument

    '/// + T:Y (default)
    testContent = "+---+"
    Call wTypeKeys testContent
    Call wTypeKeys "<Return><Up><Home><Shift End>"
    try
        EditCopy
        Warnlog "Something wrong in create table T:Y"
    catch
    endcatch

    try
        FormatRowDelete
        Sleep 1
    catch
        Warnlog "Can't delete table ,T:Y"
    endcatch

    '/// + T:N
    testContent = "+---+"
    hopeResult  = "+---+"

    Call fAutocorrectOptions("CreateTable",0,0)
    Call wTypeKeys testContent
    Call wTypeKeys "<Return><Up><Home><Shift End>"
    EditCopy

    if GetClipboardText <> hopeResult then Warnlog "Something wrong  when T:N"

    try
        FormatRowDelete
        Sleep 1
        Warnlog "Should NOT generate table when T:N"
    catch
    endcatch

    '/// Recover to the default
    Call fAutocorrectOptions("CreateTable",1,1)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_11

    Dim testContent1 as String
    Dim testContent2 as String
    Dim hopeResultAfterAutoformat1 as String
    Dim hopeResultAfterAutoformat2 as String

    testContent1                 = "ABCD1234"
    testContent2                 = "EFGH5678"

    hopeResultAfterAutoformat1   = "ABCD1234"
    hopeResultAfterAutoformat2   = "EFGH5678"

    printLog "Remove Blank Paragraphs"
    '/// Remove Blank Paragraphs

    Call hNewDocument

    '///  M:N (default)
    Call wTypeKeys testContent1
    Call wTypeKeys "<Return>",2
    Call wTypeKeys testContent2

    FormatAutoformatApply

    Call wTypeKeys "<Up><Up><Home><Shift End>"
    wait 200
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat1 then Warnlog "the first line's content is NOT correct when M:N"

    Call wTypeKeys "<Down><Home><Shift End>"
    try
        EditCopy
        Warnlog "the second line's content is NOT correct when M:N"
    catch
    endcatch

    Call wTypeKeys "<Down><Home><Shift End>"
    wait 200
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat2 then Warnlog "the third line's content is NOT correct when M:N"

    Call hCloseDocument

    Call hNewDocument

    '///  M:Y
    Call fAutocorrectOptions("RemoveBlankParagraphs",1,1)

    Call wTypeKeys testContent1
    Call wTypeKeys "<Return>",2
    Call wTypeKeys testContent2

    FormatAutoformatApply

    Call wTypeKeys "<Up><Up><Home><Shift End>"
    wait 200
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat1 then Warnlog "the first line's content is NOT correct when M:Y"

    Call wTypeKeys "<Down><Home><Shift End>"
    wait 200
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat2 then Warnlog "the second line's content is NOT correct when M:Y"

    '/// Recover to the default
    Call fAutocorrectOptions("RemoveBlankParagraphs",0,0)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_12

    Dim testContent as String
    Dim hopeResultBeforeAutoformat as String
    Dim hopeResultAfterAutoformat as String

    printLog "Replace bullets with •"
    '/// Replace bullets with •

    Call hNewDocument

    '///  M:Y (default)
    printLog "M:Y (default)"
    testContent                 = "+ Hello"
    hopeResultBeforeAutoformat  = "+ Hello"
    hopeResultAfterAutoformat   = "•	Hello"

    Call wTypeKeys testContent
    wait 100
    Call wTypeKeys "<Shift Home>"
    Sleep 1
    GetClipboardText = ""
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then Warnlog "should NOT change before autoformat "

    FormatAutoformatApply

    Call wTypeKeys "<Home><Shift End>"
    Sleep 1
    GetClipboardText = ""
    EditCopy
    Sleep 1
    if GetClipboardText <> hopeResultAfterAutoformat then Warnlog "should  change after autoformat "

    Call hCloseDocument

    Call hNewDocument

    '///  M:N
    printLog "M:N"
    testContent                 = "+ Hello"
    hopeResultBeforeAutoformat  = "+ Hello"
    hopeResultAfterAutoformat   = "+ Hello"

    Call fAutocorrectOptions("ReplaceBullets",0,0)

    Call wTypeKeys testContent
    wait 100
    Call wTypeKeys "<Shift Home>"
    Sleep 1
    GetClipboardText = ""
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then Warnlog "Should NOT change before autoformat "

    FormatAutoformatApply
    Sleep 1
    Call wTypeKeys "<Home><Shift End>"
    Sleep 1
    GetClipboardText = ""
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then Warnlog "Should  NOT change after autoformat "

    '/// Recover to the default
    Call fAutocorrectOptions("ReplaceBullets",1,1)

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectOptions_13

    Dim testContent as String
    Dim hopeResultBeforeAutoformat as String
    Dim hopeResultAfterAutoformat as String

    printLog "Replace standard quotes with custom quotes"
    '/// Replace standard quotes with custom quotes

    Call hNewDocument

    '///  M:Y (default)
    testContent                 = "'"
    hopeResultBeforeAutoformat  = "'"
    hopeResultAfterAutoformat   = "["

    ToolsAutoCorrect
    Kontext
    Active.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    SingleQuotesReplace.UnCheck
    SingleQuotesStart.Click
    Kontext "Sonderzeichen"
    Schriftliste.TypeKeys "<PageUp><Home>"
    Schriftliste.TypeKeys "<Right>", 59   ' is [
    Schriftliste.TypeKeys "<Space>"
    Sonderzeichen.OK

    Kontext "TabLocalizedOptions"
    TabLocalizedOptions.OK

    Call wTypeKeys (testContent + "Hello")
    wait 100
    Call wTypeKeys "<Home><Shift Right>"
    Sleep 1
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then Warnlog "Should NOT change before autoformat "

    FormatAutoformatApply

    Call wTypeKeys "<Home><Shift Right>"
    Sleep 1
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then Warnlog "Should  change after autoformat "

    Call hCloseDocument

    Call hNewDocument

    '///  M:N
    testContent                 = "'"
    hopeResultBeforeAutoformat  = "'"
    hopeResultAfterAutoformat   = "'"

    Call fAutocorrectOptions("ReplaceQuotes",0,0)

    Call wTypeKeys (testContent + "Hello")
    wait 100
    Call wTypeKeys "<Home><Shift Right>"
    Sleep 1
    EditCopy
    if GetClipboardText <> hopeResultBeforeAutoformat then Warnlog "Should NOT change before autoformat "

    FormatAutoformatApply

    Call wTypeKeys "<Home><Shift Right>"
    Sleep 1
    EditCopy
    if GetClipboardText <> hopeResultAfterAutoformat then Warnlog "Should  NOT change after autoformat "

    '/// Recover to the default
    Call fAutocorrectOptions("ReplaceQuotes",1,1)

    ToolsAutoCorrect
    Kontext
    Active.Setpage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    SingleQuotesReplace.Check
    SingleQuotesDefault.Click
    wait 100
    TabLocalizedOptions.OK

    Call hCloseDocument

endcase
