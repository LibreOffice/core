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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : CJK Ruby Dialogue Proposal Test
'*
'\***********************************************************************

testcase tRubyDialog_1

    Dim sBaseText , sRubyText   as String
    Dim sBaseText1 , sRubyText1 as String

    sBaseText  = "BaseText"
    sRubyText  = "RubyText"

    sBaseText1 = "BaseText1"
    sRubyText1 = "RubyText1"

    '/// Check Apply and Close button in Ruby dialogue
    Call hNewDocument

    '/// Check close button.
    '/// open ruby dialogue , input base text and ruby text , then press close button
    '/// 1) check if the ruby dialogue is closed
    '/// 2) check if the base text and ruby text is recorded
    FormatRuby
    Kontext "RubyDialog"
    Sleep 3
    BasisText1.SetText  sBaseText
    Sleep 1
    RubyText1.SetText   sRubyText
    Sleep 1

    DialogClose.Click
    Sleep 1

    if RubyDialog.Exists then
        Warnlog "The Ruby dialogue is NOT closed !"
    end if

    FormatRuby
    Kontext "RubyDialog"
    Sleep 3
    if Len(BasisText1.GetText) <> 0 then Warnlog "The base text should not been inputed"
    if Len(RubyText1.SetText)  <> 0 then Warnlog "The ruby text should not been inputed"

    '/// Check Apply button -- Normally , we think it is RubyDialog.OK
    '/// open ruby dialogue , input base text and ruby text , then press close button
    '/// Press Apply button , then press close button
    '/// check if the base text and ruby text is recorded

    BasisText1.SetText  sBaseText1
    Sleep 1
    RubyText1.SetText   sRubyText1
    Sleep 1

    RubyDialog.OK
    Sleep 2

    DialogClose.Click
    Sleep 1

    EditSelectAll
    Sleep 1

    FormatRuby
    Kontext "RubyDialog"
    Sleep 3
    if BasisText1.GetText <> sBaseText1 then Warnlog "The base text should be "+sBaseText1 + " but get " + BasisText1.GetText
    if RubyText1.GetText  <> sRubyText1 then Warnlog "The ruby text should be "+sRubyText1 + " but get " + RubyText1.GetText

    DialogClose.Click
    Sleep 1

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tRubyDialog_2

    Dim sBaseText as String

    sBaseText = "test1 test2 test3 test4 test5 test6 test7 test8 test9"
    '/// Test scrollbar in Ruby dialog

    Call hNewDocument

    '/// when there is less basetext,  the scrollbar should appear
    FormatRuby
    Kontext "RubyDialog"
    if RubyDialog.HasScrollbar <> TRUE  then warnlog "The Scrollbar should appear when less basetext!"
    DialogClose.Click
    Sleep 1

    '/// when there is more basetext(here is >4),  the scrollbar should appear
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys sBaseText
    DocumentWriter.TypeKeys "<Shift Home>"

    FormatRuby
    Kontext "RubyDialog"
    if RubyDialog.HasScrollbar <> TRUE  then warnlog "The Scrollbar should appear when more basetext!"
    DialogClose.Click
    Sleep 1

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------
testcase tRubyDialog_3

    Dim sTestText              as String
    Dim sBaseText1, sBaseText2 as String

    sTestText  = "Test1 Test2"
    sBaseText1 = "Test1"
    sBaseText2 = "Test2"

    '/// The list in ruby dialogure will be updated as soon as the uses
    '/// selects new or further words of his text.
    '/// The update will be done once the user changes his focus
    '/// back to the Ruby-Dialogue from the text

    Call hNewDocument

    '/// input 2 test words, and mark the second
    DocumentWriter.TypeKeys sTestText
    DocumentWriter.TypeKeys "<Shift Left>" , 5

    '/// There should be only the second test word in basetext1 in ruby dialogue
    FormatRuby
    Kontext "RubyDialog"
    Sleep 2
    if BasisText1.GetText <> sBaseText2  then warnlog "The BaseText1 should be:" +sBaseText2 + " but get "+BasisText1.GetText

    '/// Mark 2 test words
    Kontext "DocumentWriter"
    try
        DocumentWriter.TypeKeys "<Shift Home>"
        Sleep 2
    catch
        warnlog "The ruby dialogue is MODE less dialogue , the document can be edited!"
    endcatch

    '/// Check if the 2 test words are all in the ruby dialogue when set forcus back to the dialogue
    Kontext "RubyDialog"
    Sleep 1
    'this step is meaningless, just let the focus back to ruby dialogue
    Adjust.Select 1
    Sleep 8
    if BasisText1.GetText <> sBaseText1  then warnlog "The BaseText1 should be:" +sBaseText1 + " but get "+BasisText1.GetText
    Sleep 1
    if BasisText2.GetText <> sBaseText2  then warnlog "The BaseText2 should be:" +sBaseText2 + " but get "+BasisText2.GetText

    DialogClose.Click
    Sleep 1

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tRubyDialog_4

    Dim sCharacterStyle  as String
    dim i as integer

    '/// Check default setting for Character Style of Ruby Text

    select case iSprache
    case 01   :  sCharacterStyle = "Rubies"
    case 33   :  sCharacterStyle = "Rubys"
    case 34   :  sCharacterStyle = "Rubys"
    case 39   :  sCharacterStyle = "Testo Ruby"
    case 46   :  sCharacterStyle = "Rubytext"
    case 49   :  sCharacterStyle = "Rubys"
    case 50   :  sCharacterStyle = "Rubys"
    case 55   :  sCharacterStyle = "Rubys"
    case 81   :  sCharacterStyle = "ふりがな"
    case 82   :  sCharacterStyle = "윗주 달기" ' not sure TBO
    case 86   :  sCharacterStyle = "拼音和注音符号"
    case 88   :  sCharacterStyle = "注音符號"
    case else :' QAErrorLog "Now, the test does not support for the language " +iSprache
        Goto endsub
    end select

    Call hNewDocument

    FormatRuby
    Kontext "RubyDialog"
    Sleep 3
    if CharacterStyle.GetSelText <> sCharacterStyle  then
        warnlog "Default setting for Character Style of Ruby Text isn't right, should be "+sCharacterStyle + " but get "+CharacterStyle.GetSelText
        for i = 1 to CharacterStyle.getItemCount
            printlog "("+i+"/"+CharacterStyle.getItemCount+"): '"+CharacterStyle.getItemText(i)+"'"
        next i
    endif
    DialogClose.Click
    Sleep 1

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tRubyDialog_5

    Dim sCharacterStyle as String
    Dim sStyleName as String
    Dim i as Integer
    dim iCount as integer
    dim bError as boolean
    dim bWileExit as boolean

    '/// Check stylist

    select case iSprache
    case 01   :  sCharacterStyle = "Rubies"
    case 33   :  sCharacterStyle = "Rubys"
    case 34   :  sCharacterStyle = "Rubys"
    case 39   :  sCharacterStyle = "Testo Ruby"
    case 46   :  sCharacterStyle = "Rubytext"
    case 49   :  sCharacterStyle = "Rubys"
    case 50   :  sCharacterStyle = "Rubys"
    case 55   :  sCharacterStyle = "Rubys"
    case 81   :  sCharacterStyle = "ふりがな"
    case 82   :  sCharacterStyle = "윗주 달기" ' not sure TBO was "덧말"
    case 86   :  sCharacterStyle = "拼音和注音符号"
    case 88   :  sCharacterStyle = "注音符號"
    case else :' QAErrorLog "Now, the test does not support for the language " +iSprache
        Goto endsub
    end select

    sStyleName = "ZZZ-Test-ZZZ"

    Call hNewDocument

    '/// Open ruby dialogue
    FormatRuby
    Kontext "RubyDialog"

    '/// Click stylist button
    CallStylist.Click
    Sleep 2

    Kontext "Stylist"
    if Stylist.NotExists then
        Warnlog "No Stylist with clicking Stylist button!"
    end if

    '/// click Character Style
    Zeichenvorlagen.Click

    ' needed to update the listbox count :-(
    Vorlagenliste.typeKeys ("<end><up><down>")
    iCount = Vorlagenliste.getItemCount
    printlog iCount
    i=1
    bWileExit = Vorlagenliste.getItemText (i) <> sCharacterStyle
    while ((i<iCount+1) AND (bWileExit))
        printlog "" + (i) + ": " + Vorlagenliste.getItemText (i)
        bWileExit = Vorlagenliste.getItemText (i) <> sCharacterStyle
        inc (i)
    wEnd
    if (i = iCount+1) then
        warnlog "Style (Rubies) not found: '" + sCharacterStyle + "' inside of entries: " + iCount
        bError = true
    else
        printlog "Style (Rubies) found: '" + sCharacterStyle + "' @ position: ("+(i)+"/"+icount+")"
        Vorlagenliste.select (i)
        bError = false
    endif

    if (NOT bError) then
        '/// find entry ruby and add a new entry
        AusSelektion.OpenMenu
        Sleep 1
        printlog "hMenuSelect 1"
        hMenuSelectNr(1)
        printlog "hMenuSelect 1 survived"
        Sleep 1

        Kontext "VorlageErzeugen"
        Sleep 1
        Vorlagenname.SetText sStyleName
        Sleep 1
        VorlageErzeugen.OK
        Sleep 2

        '/// check if the new entry is in the  Character Style list box in ruby dialogue
        Kontext "RubyDialog"

        ' needed to update the listbox count :-(
        CharacterStyle.typeKeys ("<end><up><down>")
        iCount = CharacterStyle.GetItemCount
        i=0
        bWileExit = true
        while ((i<(iCount+1)) AND (bWileExit))
            bWileExit = CharacterStyle.getItemText (i+1) <> sStyleName
            inc (i)
            printlog "" + (i) + ": " + CharacterStyle.getItemText (i)
        wEnd
        printlog CharacterStyle.GetItemCount
        if ((i = iCount) AND bWileExit) then
            warnlog "New created entry not found: '" + sStyleName + "' inside of entries: " + iCount
            bError = true
        else
            printlog "New created entry found: '" + sStyleName + "' @ position: ("+(i)+"/"+icount+")"
            CharacterStyle.select (i)
            bError = false
        endif

        if (NOT bError) then
            Kontext "Stylist"
            iCount = Vorlagenliste.getItemCount
            i=0
            while ((i<iCount) AND (Vorlagenliste.getItemText (i+1) <> sStyleName))
                inc (i)
                printlog "" + (i) + ": " + Vorlagenliste.getItemText (i)
            wEnd
            if (i = iCount) then
                warnlog "Style (Rubies) not found for deleting: '" + sStyleName + "' inside of entries: " + iCount
                bError = true
            else
                printlog "Style (Rubies) found, will delete it now: '" + sStyleName + "' @ position: ("+i+"/"+icount+")"
                Vorlagenliste.select (i+1)
                bError = false
                iCount = Vorlagenliste.getItemCount
                Vorlagenliste.TypeKeys "<Delete>"
                sleep(1)
                Kontext "Active"
                if active.exists(5) then
                    printlog "active about deleting; say yes to: '" + active.getText + "'"
                    Active.Yes
                else
                    warnlog "No warning, before deleting style."
                endif
                sleep 1
                Kontext "Stylist"
                i = Vorlagenliste.getItemCount
                if ( (iCount-1) <> i) then
                    warnlog "Style wasn't deleted; count is still the same: " + i + "/" + iCount
                else
                    printlog "Style was deleted; Count is OK: " + i + "/" + iCount
                endif
            endif
        endif
    endif

    Kontext "Stylist"
    Stylist.Close
    Sleep 1
    Kontext "RubyDialog"
    DialogClose.Click
    Sleep 1

    Call hCloseDocument
endcase


