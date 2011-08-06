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
'* short description :
'*
'\***********************************************************************

sub w_205a_
    printLog Chr(13) + "---------   Level 1 - Tools - Menu (3)   ----------"

    Call tToolsOutlineNumbering1                ' Numerierungtabpage f�r Kapitelnumerierung
    Call tToolsOutlineNumbering2                ' Positionstabpage f�r Kapitelnumerierung
    Call tToolsOutlineNumbering3                ' Gesamtes f�r Kapitelnumerierung
    Call tToolsFootnoteEndnote                  ' Fu�- + Endnoten formatieren

end sub

' *******************************************************************

testcase tToolsOutlineNumbering1
    Dim VorlagenLst(11) as String : Dim Zaehler as Integer
    Dim i as integer
    PrintLog "- Tools / Outline Numbering: Part 1 - Numbering"

    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\number\numbering1.sxw" )
    
    
    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabKapitelnumerierung
    Kontext "TabKapitelnumerierung"

    PrintLog "   - adjust level"
    Ebene.Select 1
    ' I need to know the name of entry (none), depends on language, for checking the results
    Absatzvorlage.Select 1
    VorlagenLst(11)= Absatzvorlage.GetSelText
    Sleep 2
    ' Select next paragraph style
    Absatzvorlage.Select 2
    Nummer.Select 1
    if Vollstaendig.IsEnabled then Warnlog "On 1. Level 'Show Sublevels' is active -> Bug"
    BeginBei.SetText "7"
    Davor.SetText "Ha "
    Dahinter.SetText " aH"

    Ebene.Select 2
    Absatzvorlage.Select 3
    Nummer.Select 2
    Vollstaendig.SetText "0"
    BeginBei.SetText "4"
    Davor.SetText "Q "
    Dahinter.SetText " Q"

    Ebene.Select 3
    Absatzvorlage.Select 4
    Nummer.Select 3
    Vollstaendig.SetText "3"
    BeginBei.SetText "2"
    Davor.SetText "A "
    Dahinter.SetText " s"

    Ebene.Select 4
    Absatzvorlage.Select 5
    Nummer.Select 4
    Vollstaendig.SetText "1"
    Davor.SetText "Ups "
    Dahinter.SetText " spU"

    Ebene.Select 5
    Absatzvorlage.Select 6
    Nummer.Select 5
    Vollstaendig.SetText "0"
    Davor.SetText "*. "
    Dahinter.SetText " @�"

    Ebene.Select 6
    Absatzvorlage.Select 7
    Nummer.Select 6
    Vollstaendig.SetText "5"
    Davor.SetText "    "
    Sleep 1

    Ebene.Select 7
    Absatzvorlage.Select 8
    Nummer.Select 5
    Vollstaendig.SetText "9"
    Sleep 1

    Ebene.Select 8
    Absatzvorlage.Select 9
    Nummer.Select 4
    Vollstaendig.SetText "9"
    Sleep 1

    Ebene.Select 9
    Absatzvorlage.Select 10
    Nummer.Select 3
    Vollstaendig.SetText "9"
    Dahinter.SetText "Hallo dies ist ein langer Text"
    Sleep 1

    Ebene.Select 10
    Absatzvorlage.Select 11
    Nummer.Select 3
    Vollstaendig.SetText "9"
    Davor.SetText "Hallo dies ist ein langer Text"
    Sleep 1

    ' remember the paragraph style
    For i = 1 to 10
        Ebene.Select i
        VorlagenLst(i) = Absatzvorlage.GetSelText
    next i

    TabKapitelnumerierung.Ok

    printlog "- format document"
    call wTypeKeys "<Up>", 25

    for Zaehler = 1 to 10
        call wTypeKeys "<Shift End>"
        if wStyleSet ( VorlagenLst(Zaehler) ) = false then
            Warnlog "- Style '" & VorlagenLst(Zaehler) & "' could not be set!"
        end if
        call wTypeKeys "<Down>", 8
        call wTypeKeys "<Home>"
        wait 500
    next Zaehler

    printlog "- check properties"
    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabKapitelnumerierung
    Kontext "TabKapitelnumerierung"
    Ebene.Select 1
    if Absatzvorlage.GetSelText <> VorlagenLst(1) then Warnlog "Level 1: wrong Paragraph Style"
    if Nummer.GetSelIndex <> 1 then Warnlog "Level 1: wrong Character Style -> " + Nummer.GetSeltext + "instead of 1 !"
    if BeginBei.GetText <> "7" then Warnlog "7 is not selected at 'Start at' but " + BeginBei.Gettext
    if Davor.GetText <> "Ha " then Warnlog "Level 1: Seperator before not 'Ha ' but " + Davor.Gettext
    if Dahinter.GetText <> " aH" then Warnlog "Level 1: Seperator after not ' aH' but " + Dahinter.Gettext

    Ebene.Select 2
    if Absatzvorlage.GetSelText <> VorlagenLst(2) then Warnlog "Level 2: Paragraph Style not " + VorlagenLst(2) + " but " + Absatzvorlage.GetSelText
    if Nummer.GetSelIndex <> 2 then Warnlog "Level 2: wrong Character Style"
    if Vollstaendig.GetText <> "1" then Warnlog "Level 2: 'Show Sublevels' is not 1 but " + Vollstaendig.GetText
    if Davor.GetText <> "Q " then Warnlog "Level 2: Seperator before is wrong"
    if Dahinter.GetText <> " Q" then Warnlog "Level 2: Seperator after is wrong"

    Ebene.Select 3
    if Absatzvorlage.GetSelText <> VorlagenLst(3) then Warnlog "Level 3: Paragraph Style" + VorlagenLst(3) + " but " + Absatzvorlage.GetSelText
    if Nummer.GetSelIndex <> 3 then Warnlog "Level 3: wrong Character Style"
    if Vollstaendig.GetText <> "3" then Warnlog "Level 3: 'Show Sublevels' is not 3 but " + Vollstaendig.GetText
    if Davor.GetText <> "A " then Warnlog "Level 3: Seperator before is wrong"
    if Dahinter.GetText <> " s" then Warnlog "Level 3: Seperator after is wrong"

    Ebene.Select 4
    if Absatzvorlage.GetSelText <> VorlagenLst(4) then Warnlog "Level 4: Paragraph Style" + VorlagenLst(4) + " but " + Absatzvorlage.GetSelText
    if Nummer.GetSelIndex <> 4 then Warnlog "Level 4: wrong Character Style"
    if Vollstaendig.GetText <> "1" then Warnlog "Level 4: 'Show Sublevels' is not 1 but " + Vollstaendig.GetText
    if Davor.GetText <> "Ups " then Warnlog "Level 4: Seperator before is wrong"
    if Dahinter.GetText <> " spU" then Warnlog "Level 4: Seperator after is wrong"

    Ebene.Select 5
    if Absatzvorlage.GetSelText <> VorlagenLst(5) then Warnlog "Level 5: Paragraph Style" + VorlagenLst(5) + " but " + Absatzvorlage.GetSelText
    if Nummer.GetSelIndex <> 5 then Warnlog "Level 5: wrong Character Style"
    if Vollstaendig.GetText <> "1" then Warnlog "Level 5: 'Show Sublevels' is not 1 but " + Vollstaendig.GetText
    if Davor.GetText <> "*. " then Warnlog "Level 5: Seperator before is wrong"
    if Dahinter.GetText <> " @�" then Warnlog "Level 5: Seperator after is wrong"

    Ebene.Select 6
    if Absatzvorlage.GetSelText <> VorlagenLst(6) then Warnlog "Level 6: Paragraph Style" + VorlagenLst(6) + " but " + Absatzvorlage.GetSelText
    if Nummer.GetSelIndex <> 6 then Warnlog "Level 6: wrong Character Style"
    if Vollstaendig.GetText <> "5" then Warnlog "Level 6: 'Show Sublevels' is not 5 but " + Vollstaendig.GetText
    if Davor.GetText <> "    " then Warnlog "Level 6: Seperator before is wrong"

    Ebene.Select 7
    if Absatzvorlage.GetSelText <> VorlagenLst(7) then Warnlog "Level 7: Paragraph Style" + VorlagenLst(7) + " but " + Absatzvorlage.GetSelText
    if Nummer.GetSelIndex <> 5 then Warnlog "Level 7: wrong Character Style"
    if Vollstaendig.GetText <> "7" then Warnlog "Level 7: 'Show Sublevels' is not 7 but " + Vollstaendig.GetText

    Ebene.Select 8
    if Absatzvorlage.GetSelText <> VorlagenLst(8) then Warnlog "Level 8: Paragraph Style" + VorlagenLst(8) + " but " + Absatzvorlage.GetSelText
    if Nummer.GetSelIndex <> 4 then Warnlog "Level 8: wrong Character Style"
    if Vollstaendig.GetText <> "8" then Warnlog "Level 8: 'Show Sublevels' is not 8 but " + Vollstaendig.GetText

    Ebene.Select 9
    if Absatzvorlage.GetSelText <> VorlagenLst(9) then Warnlog "Level 9: Paragraph Style not " + VorlagenLst(9) + " but " + Absatzvorlage.GetSelText
    if Nummer.GetSelIndex <> 3 then Warnlog "Level 9: wrong Character Style"
    if Vollstaendig.GetText <> "9" then Warnlog "Level 9: 'Show Sublevels' is not 9 but " + Vollstaendig.GetText
    if Dahinter.GetText <> "Hallo dies ist ein langer Text" then Warnlog "Level 9: Seperator after is wrong"

    Ebene.Select 10
    if Absatzvorlage.GetSelText <> VorlagenLst(10) then 
        QAErrorlog "#125998#Level 10: Paragraph Style not " + VorlagenLst(10) + " but " + Absatzvorlage.GetSelText
    end if    
    if Nummer.GetSelIndex <> 3 then Warnlog "Level 10: wrong Character Style"
    if Vollstaendig.GetText <> "9" then Warnlog "Level 10: 'Show Sublevels' is not 9 but " + Vollstaendig.GetText
    if Davor.GetText <> "Hallo dies ist ein langer Text" then Warnlog "Level 10: Seperator before is wrong"
    TabKapitelnumerierung.Ok
    
    for i = 1 to 2
        Select Case i 
            Case 1
                printlog "- Save as sxw"
                Call hFileSaveAsKill ( gOfficePath + "user\work\tToolsOutlineNumbering1.sxw" )
            Case 2
                printlog "- Save as odt"
                Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\tToolsOutlineNumbering1.odt", "writer8")
        end select
    next i            
    Call hCloseDocument

    for i = 1 to 2
        Select Case i
            Case 1
                printlog "- Load sxw and check formattings"
                hFileOpen( gOfficePath & "user\work\tToolsOutlineNumbering1.sxw" )
            Case 2
                printlog "- Load odt and check formattings"
                hFileOpen( gOfficePath & "user\work\tToolsOutlineNumbering1.odt" )
        end select
        call wTypeKeys "<PageUp>", 8
        call wTypeKeys "<Up>", 10
    
        for Zaehler = 1 to 10
            call wTypeKeys "<Shift End>"
            if wStyleGet(VorlagenLst(Zaehler)) = false then Warnlog "Wrong Style for Outline Numbering " + Zaehler
            call wTypeKeys "<Down>", 8
            call wTypeKeys "<Home>"
        next Zaehler
    
        printlog "   - check saved properties"
        ToolsOutlineNumbering
        Kontext
        Active.SetPage TabKapitelnumerierung
        Kontext "TabKapitelnumerierung"
        Ebene.Select 1
        if Absatzvorlage.GetSelText <> VorlagenLst(1) then Warnlog "   - Level 1: wrong Paragraph Style"
        if Nummer.GetSelIndex <> 1 then Warnlog "   - Level 1: wrong Character Style -> " + Nummer.GetSeltext + "instead of 1 !"
        if BeginBei.GetText <> "7" then Warnlog "   - 7 is not selected at 'Start at' but " + BeginBei.Gettext
        if Davor.GetText <> "Ha " then Warnlog "   - Level 1: Seperator before not 'Ha ' but " + Davor.Gettext
        if Dahinter.GetText <> " aH" then Warnlog "   - Level 1: Seperator after not ' aH' but " + Dahinter.Gettext
    
        Ebene.Select 2
        if Absatzvorlage.GetSelText <> VorlagenLst(2) then Warnlog "   - Level 2: Paragraph Style not " + VorlagenLst(2) + " but " + Absatzvorlage.GetSelText
        if Nummer.GetSelIndex <> 2 then Warnlog "   - Level 2: wrong Character Style"
        if Vollstaendig.GetText <> "1" then Warnlog "   - Level 2: 'Show Sublevels' is not 1 but " + Vollstaendig.GetText
        if Davor.GetText <> "Q " then Warnlog "   - Level 2: Seperator before is wrong"
        if Dahinter.GetText <> " Q" then Warnlog "   - Level 2: Seperator after is wrong"
    
        Ebene.Select 3
        if Absatzvorlage.GetSelText <> VorlagenLst(3) then 
            QAErrorlog "   - #125998#Level 3: Paragraph Style not " + VorlagenLst(3) + " but " + Absatzvorlage.GetSelText
        end if
        if Nummer.GetSelIndex <> 3 then Warnlog "   - Level 3: wrong Character Style"
        if Vollstaendig.GetText <> "3" then Warnlog "   - Level 3: 'Show Sublevels' is not 3 but " + Vollstaendig.GetText
        if Davor.GetText <> "A " then Warnlog "   - Level 3: Seperator before is wrong"
        if Dahinter.GetText <> " s" then Warnlog "   - Level 3: Seperator after is wrong"
    
        Ebene.Select 4
        if Absatzvorlage.GetSelText <> VorlagenLst(4) then Warnlog "   - Level 4: Paragraph Style not " + VorlagenLst(4) + " but " + Absatzvorlage.GetSelText
        if Nummer.GetSelIndex <> 4 then Warnlog "   - Level 4: wrong Character Style"
        if Vollstaendig.GetText <> "1" then Warnlog "   - Level 4: 'Show Sublevels' is not 1 but " + Vollstaendig.GetText
        if Davor.GetText <> "Ups " then Warnlog "   - Level 4: Seperator before is wrong"
        if Dahinter.GetText <> " spU" then Warnlog "   - Level 4: Seperator after is wrong"
    
        Ebene.Select 5
        if Absatzvorlage.GetSelText <> VorlagenLst(5) then Warnlog "   - Level 5: Paragraph Style not " + VorlagenLst(5) + " but " + Absatzvorlage.GetSelText
        if Nummer.GetSelIndex <> 5 then Warnlog "   - Level 5: wrong Character Style"
        if Vollstaendig.GetText <> "1" then Warnlog "   - Level 5: 'Show Sublevels' is not 1 but " + Vollstaendig.GetText
        if Davor.GetText <> "*. " then Warnlog "   - Level 5: Seperator before is wrong"
        if Dahinter.GetText <> " @�" then Warnlog "   - Level 5: Seperator after is wrong"
    
        Ebene.Select 6
        if Absatzvorlage.GetSelText <> VorlagenLst(6) then Warnlog "   - Level 6: Paragraph Style not " + VorlagenLst(6) + " but " + Absatzvorlage.GetSelText
        if Nummer.GetSelIndex <> 6 then Warnlog "   - Level 6: wrong Character Style"
        if Vollstaendig.GetText <> "5" then Warnlog "   - Level 6: 'Show Sublevels' is not 5 but " + Vollstaendig.GetText
        if Davor.GetText <> "    " then Warnlog "   - Level 6: Seperator before is wrong"
    
        Ebene.Select 7
        if Absatzvorlage.GetSelText <> VorlagenLst(7) then Warnlog "   - Level 7: Paragraph Style not " + VorlagenLst(7) + " but " + Absatzvorlage.GetSelText
        if Nummer.GetSelIndex <> 5 then Warnlog "   - Level 7: wrong Character Style"
        if Vollstaendig.GetText <> "7" then Warnlog "   - Level 7: 'Show Sublevels' is not 7 but " + Vollstaendig.GetText
    
        Ebene.Select 8
        if Absatzvorlage.GetSelText <> VorlagenLst(8) then Warnlog "   - Level 8: Paragraph Style not " + VorlagenLst(8) + " but " + Absatzvorlage.GetSelText
        if Nummer.GetSelIndex <> 4 then Warnlog "   - Level 8: wrong Character Style"
        if Vollstaendig.GetText <> "8" then Warnlog "   - Level 8: 'Show Sublevels' is not 8 but " + Vollstaendig.GetText
    
        Ebene.Select 9
        if Absatzvorlage.GetSelText <> VorlagenLst(9) then Warnlog "   - Level 9: Paragraph Style not " + VorlagenLst(9) + " but " + Absatzvorlage.GetSelText
        if Nummer.GetSelIndex <> 3 then Warnlog "   - Level 9: wrong Character Style"
        if Vollstaendig.GetText <> "9" then Warnlog "   - Level 9: 'Show Sublevels' is not 9 but " + Vollstaendig.GetText
        if Dahinter.GetText <> "Hallo dies ist ein langer Text" then Warnlog "   - Level 9: Seperator after is wrong"
    
        Ebene.Select 10
        if Absatzvorlage.GetSelText <> VorlagenLst(10) then 
            QAErrorlog "   - #125998#Level 10: Paragraph Style not " + VorlagenLst(10) + " but " + Absatzvorlage.GetSelText
        end if    
        if Nummer.GetSelIndex <> 3 then Warnlog "   - Level 10: wrong Character Style"
        if Vollstaendig.GetText <> "9" then Warnlog "   - Level 10: 'Show Sublevels' is not 9 but " + Vollstaendig.GetText
        if Davor.GetText <> "Hallo dies ist ein langer Text" then Warnlog "   - Level 10: Seperator before is wrong"
        TabKapitelnumerierung.Cancel
    
        Call hCloseDocument
    next i
    
endcase

'--------------------------------------------------

testcase tToolsOutlineNumbering2
    Dim i as Integer
    Dim VorlagenLst (11) as String
    Dim V1 : Dim V2 :  Dim V3 : Dim V4 :  Dim V5 : Dim V6 :  Dim V7 : Dim V8 : Dim V9 : Dim V10
    Dim V11 : Dim V12 :  Dim V13 : Dim V14 :  Dim V15 : Dim V16

    PrintLog "- Tools / Outline Numbering: Part 2 - Tabpage Position"

    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\number\numbering1.sxw" )
    
    
    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabKapitelnumerierung
    Kontext "TabKapitelnumerierung"
    for i=1 to 10
        if iSprache = 01 then
            VorlagenLst (i) = Absatzvorlage.GetItemText ( i+1 )
        else
            VorlagenLst (i) = Absatzvorlage.GetItemText ( i+2 )
        end if
    next i
    Kontext
    Active.SetPage TabPositionKapitelnumerierung
    Kontext "TabPositionKapitelnumerierung"

    Level.Select 1
    NumberAlignment.Select 2
    if relativ.IsEnabled then Warnlog "'Relative' is enabled when selecting 'Level 1' -> Bug"
    IndentAt.More 5
    V1 = IndentAt.GetText
    AlignedAt.More 1
    V2 = AlignedAt.GetText

    Level.Select 3
    NumberAlignment.Select 3
    Relativ.Check
    IndentAt.Less 5
    V3 = IndentAt.GetText
    AbstandTextNumerierung.More 2
    V4 = AbstandTextNumerierung.GetText

    Level.Select 4
    NumberAlignment.Select 1
    Relativ.Check
    V5 = IndentAt.GetText
    V6 = AbstandTextNumerierung.GetText
    V7 = AlignedAt.GetText

    Level.Select 6
    NumberAlignment.Select 3
    Relativ.Check
    IndentAt.More 4
    V8 = IndentAt.GetText
    AbstandTextNumerierung.Less 3
    V9 = AbstandTextNumerierung.GetText
    AlignedAt.Less 3
    V10 = AlignedAt.GetText

    Level.Select 9
    NumberAlignment.Select 1
    Relativ.Check
    IndentAt.Less 2
    V11 = IndentAt.GetText
    AbstandTextNumerierung.More 2
    V12 = AbstandTextNumerierung.GetText
    AlignedAt.More 3
    V13 = AlignedAt.GetText

    Level.Select 10
    NumberAlignment.Select 3
    Relativ.Check
    IndentAt.More 4
    V14 = IndentAt.GetText
    AbstandTextNumerierung.Less 3
    V15 = AbstandTextNumerierung.GetText
    AlignedAt.Less 3
    V16 = AlignedAt.GetText
    TabPositionKapitelnumerierung.OK

    printlog "  - create Numbering"
    for i = 1 to 10
        call wTypeKeys "<Shift End>"
        if wStyleSet ( VorlagenLst(i) ) = false then
            Warnlog "- Style '" & VorlagenLst(i) & "' could not be set!"
        end if
        call wTypeKeys "<Down>", 8
        call wTypeKeys "<Home>"
    next i

    printlog "  - check Numbering-Position"
    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabPositionKapitelnumerierung
    Kontext "TabPositionKapitelnumerierung"
    Level.Select 1
    if NumberAlignment.GetSelIndex <> 2 then Warnlog "Level 1: Wrong Numbering Alignment:  Not = 2   But = "+ NumberAlignment.GetSelIndex
    if V1 <> IndentAt.GetText then Warnlog "Level 1: Wrong Indent:  Not = "+ V1+ "  But = "+ IndentAt.GetText
    if V2 <> AlignedAt.GetText then Warnlog "Level 1: Space to Text  Not = "+ V2+ "  But = "+ AlignedAt.GetText
    Level.Select 3
    if NumberAlignment.GetSelIndex <> 3 then Warnlog "Level 3: Wrong Numbering Alignment:  Not = 3   But = "+ NumberAlignment.GetSelIndex
    if V3 <> IndentAt.GetText then Warnlog "Level 3: Wrong Indent:  Not = "+ V3+ "  But = "+ IndentAt.GetText
    if V4 <> AbstandTextNumerierung.GetText then Warnlog "Level 3: Minimum Space numbering <> Text  Not = "+ V4+ "  But = "+ AbstandTextNumerierung.GetText
    Level.Select 4
    if NumberAlignment.GetSelIndex <> 1 then Warnlog "Level 4: Wrong Numbering Alignment:  Not = 1   But = "+ NumberAlignment.GetSelIndex
    if V5 <> IndentAt.GetText then Warnlog "Level 4: Wrong Indent:  Not = "+ V5+ "  But = "+ IndentAt.GetText
    if V6 <> AbstandTextNumerierung.GetText then Warnlog "Level 4: Minimum Space numbering <> Text  Not = "+ V6+ "  But = "+ AbstandTextNumerierung.GetText  
    if V7 <> AlignedAt.GetText then Warnlog "Level 4: Space to Text  Not = "+ V7+ "  But = "+ AlignedAt.GetText
    Level.Select 6
    if NumberAlignment.GetSelIndex <> 3 then Warnlog "Level 6: Wrong Numbering Alignment:  Not = 3   But = "+ NumberAlignment.GetSelIndex
    if V8 <> IndentAt.GetText then Warnlog "Level 6: Wrong Indent:  Not = "+ V8+ "  But = "+ IndentAt.GetText
    if V9 <> AbstandTextNumerierung.GetText then Warnlog "Level 6: Minimum Space numbering <> Text  Not = "+ V9+ "  But = "+ AbstandTextNumerierung.GetText
    if V10 <> AlignedAt.GetText then Warnlog "Level 6: Space to Text  Not = "+ V10+ "  But = "+ AlignedAt.GetText
    Level.Select 9
    if NumberAlignment.GetSelIndex <> 1 then Warnlog "Level 9: Wrong Numbering Alignment:  Not = 1   But = "+ NumberAlignment.GetSelIndex
    if V11 <> IndentAt.GetText then Warnlog "Level 9: Wrong Indent:  Not = "+ V11+ "  But = "+ IndentAt.GetText
    if V12 <> AbstandTextNumerierung.GetText then Warnlog "Level 9: Minimum Space numbering <> Text  Not = "+ V12+ "  But = "+ AbstandTextNumerierung.GetText
    if V13 <> AlignedAt.GetText then Warnlog "Level 9: Space to Text  Not = "+ V13+ "  But = "+ AlignedAt.GetText
    Level.Select 10
    if NumberAlignment.GetSelIndex <> 3 then Warnlog "Level 10: Wrong Numbering Alignment:  Not = 3   But = "+ NumberAlignment.GetSelIndex
    if V14 <> IndentAt.GetText then Warnlog "Level 10: Wrong Indent:  Not = "+ V14+ "  But = "+ IndentAt.GetText
    if V15 <> AbstandTextNumerierung.GetText then Warnlog "Level 10: Minimum Space numbering <> Text  Not = "+ V15+ "  But = "+ AbstandTextNumerierung.GetText
    if V16 <> AlignedAt.GetText then Warnlog "Level 10: Space to Text  Not = "+ V16+ "  But = "+ AlignedAt.GetText
    TabPositionKapitelnumerierung.Cancel

    printlog "  - check saved Numbering"
    Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\tToolsOutlineNumbering2.odt", "writer8" )
    Call hCloseDocument
    hFileOpen( gOfficePath & "user\work\tToolsOutlineNumbering2.odt" )

    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabPositionKapitelnumerierung
    Kontext "TabPositionKapitelnumerierung"
    Level.Select 1
    if NumberAlignment.GetSelIndex <> 2 then Warnlog "Level 1: Wrong Numbering Alignment:  Not = 2   But = "+ NumberAlignment.GetSelIndex
    if V1 <> IndentAt.GetText then Warnlog "Level 1: Wrong Indent:  Not = "+ V1+ "  But = "+ IndentAt.GetText
    if V2 <> AlignedAt.GetText then Warnlog "Level 1: Space to Text  Not = "+ V2+ "  But = "+ AlignedAt.GetText
    Level.Select 3
    if NumberAlignment.GetSelIndex <> 3 then Warnlog "Level 3: Wrong Numbering Alignment:  Not = 3   But = "+ NumberAlignment.GetSelIndex
    if V3 <> IndentAt.GetText then Warnlog "Level 3: Wrong Indent:  Not = "+ V3+ "  But = "+ IndentAt.GetText
    if V4 <> AbstandTextNumerierung.GetText then Warnlog "Level 3: Minimum Space numbering <> Text  Not = "+ V4+ "  But = "+ AbstandTextNumerierung.GetText
    Level.Select 4
    if NumberAlignment.GetSelIndex <> 1 then Warnlog "Level 4: Wrong Numbering Alignment:  Not = 1   But = "+ NumberAlignment.GetSelIndex
    if V5 <> IndentAt.GetText then Warnlog "Level 4: Wrong Indent:  Not = "+ V5+ "  But = "+ IndentAt.GetText
    if V6 <> AbstandTextNumerierung.GetText then Warnlog "Level 4: Minimum Space numbering <> Text  Not = "+ V6+ "  But = "+ AbstandTextNumerierung.GetText
    if V7 <> AlignedAt.GetText then Warnlog "Level 4: Space to Text  Not = "+ V7+ "  But = "+ AlignedAt.GetText
    Level.Select 6
    if NumberAlignment.GetSelIndex <> 3 then Warnlog "Level 6: Wrong Numbering Alignment:  Not = 3   But = "+ NumberAlignment.GetSelIndex
    if V8 <> IndentAt.GetText then Warnlog "Level 6: Wrong Indent:  Not = "+ V8+ "  But = "+ IndentAt.GetText
    if V9 <> AbstandTextNumerierung.GetText then Warnlog "Level 6: Minimum Space numbering <> Text  Not = "+ V9+ "  But = "+ AbstandTextNumerierung.GetText
    if V10 <> AlignedAt.GetText then Warnlog "Level 6: Space to Text  Not = "+ V10+ "  But = "+ AlignedAt.GetText
    Level.Select 9
    if NumberAlignment.GetSelIndex <> 1 then Warnlog "Level 9: Wrong Numbering Alignment:  Not = 1   But = "+ NumberAlignment.GetSelIndex
    if V11 <> IndentAt.GetText then Warnlog "Level 9: Wrong Indent:  Not = "+ V11+ "  But = "+ IndentAt.GetText
    if V12 <> AbstandTextNumerierung.GetText then Warnlog "Level 9: Minimum Space numbering <> Text  Not = "+ V12+ "  But = "+ AbstandTextNumerierung.GetText
    if V13 <> AlignedAt.GetText then Warnlog "Level 9: Space to Text  Not = "+ V13+ "  But = "+ AlignedAt.GetText
    Level.Select 10
    if NumberAlignment.GetSelIndex <> 3 then Warnlog "Level 10: Wrong Numbering Alignment:  Not = 3   But = "+ NumberAlignment.GetSelIndex
    if V14 <> IndentAt.GetText then Warnlog "Level 10: Wrong Indent:  Not = "+ V14+ "  But = "+ IndentAt.GetText
    if V15 <> AbstandTextNumerierung.GetText then Warnlog "Level 10: Minimum Space numbering <> Text  Not = "+ V15+ "  But = "+ AbstandTextNumerierung.GetText
    if V16 <> AlignedAt.GetText then Warnlog "Level 10: Space to Text  Not = "+ V16+ "  But = "+ AlignedAt.GetText
    TabPositionKapitelnumerierung.Cancel

    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsOutlineNumbering3
    Dim V1 : Dim V2 : Dim V3 : Dim V4 : Dim V5 : Dim V6 : Dim V7 : Dim V8 : Dim V9 : Dim V10

    PrintLog "- Tools / Outline Numbering: Part 3 - Total"

    Call hNewDocument
    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabKapitelnumerierung
    Kontext "TabKapitelnumerierung"

    PrintLog "   - Same Formatting from 1-10"
    Ebene.Select 11
    if Absatzvorlage.IsEnabled then Warnlog "Able to adjust Paragraph Style for all Levels -> Bug"
    Nummer.Select 4
    Davor.SetText "HoHo "
    Dahinter.SetText " HiHi "
    BeginBei.SetText "4"

    Ebene.Select 1
    Nummer.Select 3
    Davor.SetText "Anders 1 "
    Dahinter.SetText " 1 Anders"
    BeginBei.SetText "1"

    Ebene.Select 5
    Nummer.Select 2
    Davor.SetText "Anders 5 "
    Dahinter.SetText " 5 Anders"
    BeginBei.SetText "2"

    Kontext
    Active.SetPage TabPositionKapitelnumerierung
    Kontext "TabPositionKapitelnumerierung"

    Level.Select 1
    NumberingAlignment.Select 2
    if relativ.IsEnabled then Warnlog "Relative is enabled when selecting Level 1 -> Bug!"
    AlignedAt2.More 5
    V1 = AlignedAt2.GetText
    IndentAt2.More 1
    V2 = IndentAt2.GetText

    Level.Select 3
    NumberingAlignment.Select 3
    AlignedAt2.More 5
    V3 = AlignedAt2.GetText
    AbstandTextNumerierung2.More 2
    V4 = AbstandTextNumerierung2.GetText

    Level.Select 7
    NumberingAlignment.Select 1
    IndentAt2.More 4
    Sleep 1
    V5 = AlignedAt2.GetText
    V6 = AbstandTextNumerierung2.GetText
    V7 = IndentAt2.GetText

    Level.Select 8
    NumberingAlignment.Select 3
    AlignedAt2.More 1
    AbstandTextNumerierung2.Less 4
    IndentAt2.More 4
    V8 = AlignedAt2.GetText
    V9 = AbstandTextNumerierung2.GetText
    V10 = IndentAt2.GetText

    TabPositionKapitelnumerierung.OK
    Sleep 1

    PrintLog "   - check the Levels"
    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabKapitelnumerierung
    Kontext "TabKapitelnumerierung"
    printlog "        - Tabpage Numbering"
    Ebene.Select 1
    if Nummer.GetSelIndex <> 3 then Warnlog "Level 1: Wrong number selected, not  3 but " + Nummer.GetSelIndex
    if Davor.GetText <> "Anders 1 " then Warnlog "Level 1:Seperator before is wrong.. Not 'Anders 1 '   But " + Davor.GetText
    if Dahinter.GetText <> " 1 Anders" then Warnlog "Level 1: Seperator after text is wrong. Not ' Anders 1'   But " + Dahinter.GetText
    if BeginBei.GetText <> "1" then Warnlog "Level 1: Start at is not 1 but " + BeginnBei.GetText

    Ebene.Select 2
    if Nummer.GetSelIndex <> 4 then Warnlog "Level 2: Wrong number selected, not  4 but " + Nummer.GetSelIndex
    if Davor.GetText <> "HoHo " then Warnlog "Level 2:Seperator before is wrong.. Not 'Hoho '   But " + Davor.GetText
    if Dahinter.GetText <> " HiHi " then Warnlog "Level 2: Seperator after text is wrong. Not ' HiHi '   But " + Dahinter.GetText
    if BeginBei.GetText <> "4" then Warnlog "Level 2: Start at is not 4 but " + BeginnBei.GetText

    Ebene.Select 5
    if Nummer.GetSelIndex <> 2 then Warnlog "Level 5: Wrong number selected, not  2 but " + Nummer.GetSelIndex
    if Davor.GetText <> "Anders 5 " then Warnlog "Level 5:Seperator before is wrong.. Not 'Anders 5 '   But " + Davor.GetText
    if Dahinter.GetText <> " 5 Anders" then Warnlog "Level 5: Seperator after text is wrong. Not ' 5 Anders'   But " + Dahinter.GetText
    if BeginBei.GetText <> "2" then Warnlog "Level 1: Start at is not 1 but " + BeginnBei.GetText

    Ebene.Select 7
    if Nummer.GetSelIndex <> 4 then Warnlog "Level 7: Wrong number selected, not  4 but " + Nummer.GetSelIndex
    if Davor.GetText <> "HoHo " then Warnlog "Level 7:Seperator before is wrong.. Not 'Hoho '   But " + Davor.GetText
    if Dahinter.GetText <> " HiHi " then Warnlog "Level 7: Seperator after text is wrong. Not ' HiHi '   But " + Dahinter.GetText
    if BeginBei.GetText <> "4" then Warnlog "Level 7: Start at is not 4 but " + BeginnBei.GetText

    Ebene.Select 11
    if Nummer.GetSelIndex <> 0 then Warnlog "Level 1-10: Setting for 'Number' is not 'keine Selektion' but  " + Nummer.GetSelIndex
    if Davor.GetText <> "" then Warnlog "Level 1-10: Seperator before is not 'keine Selektion' but " + Davor.GetText
    if Dahinter.GetText <> "" then Warnlog "Level 1-10: Seperator after is not 'keine Selektion' but " + Dahinter.GetText
    if BeginBei.GetText <> "" then Warnlog "Level 1-10: Start at is not 'keine Selektion' but " + BeginBei.GetText

    Kontext
    Active.SetPage TabPositionKapitelnumerierung
    Kontext "TabPositionKapitelnumerierung"
    printlog "        - Tabpage Position"
    Level.Select 1
    if NumberingAlignment.GetSelIndex <> 2 then Warnlog "Level 1: Wrong Numbering Alignment:  Not = 2   But = "+ NumberingAlignment.GetSelIndex
    if V1 <> AlignedAt2.GetText then Warnlog "Level 1: Wrong Indent:  Not = "+ V1+ "  But = "+ AlignedAt2.GetText
    if V2 <> IndentAt2.GetText then Warnlog "Level 1: Space to Text  Not = "+ V2+ "  But = "+ IndentAt2.GetText
    Level.Select 3
    if NumberingAlignment.GetSelIndex <> 3 then Warnlog "Level 3: Wrong Numbering Alignment:  Not = 3   But = "+ NumberingAlignment.GetSelIndex
    if V3 <> AlignedAt2.GetText then Warnlog "Level 3: Wrong Indent:  Not = "+ V3+ "  But = "+ AlignedAt2.GetText
    if V4 <> AbstandTextNumerierung2.GetText then Warnlog "Level 3: Space to Text  Not = "+ V4+ "  But = "+ AbstandTextNumerierung2.GetText
    Level.Select 7
    if NumberingAlignment.GetSelIndex <> 1 then Warnlog "Level 7: Wrong Numbering Alignment:  Not = 1   But = "+ NumberingAlignment.GetSelIndex
    if V5 <> AlignedAt2.GetText then Warnlog "Level 7: Wrong Indent:  Not = "+ V5+ "  But = "+ AlignedAt2.GetText
    if V6 <> AbstandTextNumerierung2.GetText then Warnlog "Level 7: Space to Text  Not = "+ V6+ "  But = "+ AbstandTextNumerierung2.GetText
    if V7 <> IndentAt2.GetText then Warnlog "Level 7: Space to Text  Not = "+ V7+ "  But = "+ IndentAt2.GetText
    Level.Select 8
    if NumberingAlignment.GetSelIndex <> 3 then Warnlog "Level 8: Wrong Numbering Alignment:  Not = 3   But = "+ NumberingAlignment.GetSelIndex
    if V8 <> AlignedAt2.GetText then Warnlog "Level 8: Wrong Indent:  Not = "+ V8+ "  But = "+ AlignedAt2.GetText
    if V9 <> AbstandTextNumerierung2.GetText then Warnlog "Level 8: Space to Text  Not = "+ V9+ "  But = "+ AbstandTextNumerierung2.GetText
    if V10 <> IndentAt2.GetText then Warnlog "Level 8: Space to Text  Not = "+ V10+ "  But = "+ IndentAt2.GetText

    TabPositionKapitelnumerierung.Cancel

    printlog "  - check saved Numbering"
    Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\tToolsOutlineNumbering3.odt", "writer8" )
    Call hCloseDocument
    hFileOpen( gOfficePath & "user\work\tToolsOutlineNumbering3.odt" )
    ToolsOutlineNumbering
    Kontext
    Active.SetPage TabKapitelnumerierung
    Kontext "TabKapitelnumerierung"
    printlog "        - Tabpage Numbering"
    Ebene.Select 1
    if Nummer.GetSelIndex <> 3 then Warnlog "Level 1: Wrong number selected, not  3 but " + Nummer.GetSelIndex
    if Davor.GetText <> "Anders 1 " then Warnlog "Level 1:Seperator before is wrong.. Not 'Anders 1 '   But " + Davor.GetText
    if Dahinter.GetText <> " 1 Anders" then Warnlog "Level 1: Seperator after text is wrong. Not ' Anders 1'   But " + Dahinter.GetText
    if BeginBei.GetText <> "1" then Warnlog "Level 1: Start at is not 1 but " + BeginnBei.GetText

    Ebene.Select 2
    if Nummer.GetSelIndex <> 4 then Warnlog "Level 2: Wrong number selected, not  4 but " + Nummer.GetSelIndex
    if Davor.GetText <> "HoHo " then Warnlog "Level 2:Seperator before is wrong.. Not 'Hoho '   But " + Davor.GetText
    if Dahinter.GetText <> " HiHi " then Warnlog "Level 2: Seperator after text is wrong. Not ' HiHi '   But " + Dahinter.GetText
    if BeginBei.GetText <> "4" then Warnlog "Level 2: Start at is not 4 but " + BeginnBei.GetText

    Ebene.Select 5
    if Nummer.GetSelIndex <> 2 then Warnlog "Level 5: Wrong number selected, not  2 but " + Nummer.GetSelIndex
    if Davor.GetText <> "Anders 5 " then Warnlog "Level 5:Seperator before is wrong.. Not 'Anders 5 '   But " + Davor.GetText
    if Dahinter.GetText <> " 5 Anders" then Warnlog "Level 5: Seperator after text is wrong. Not ' 5 Anders'   But " + Dahinter.GetText
    if BeginBei.GetText <> "2" then Warnlog "Level 1: Start at is not 1 but " + BeginnBei.GetText

    Ebene.Select 7
    if Nummer.GetSelIndex <> 4 then Warnlog "Level 7: Wrong number selected, not  4 but " + Nummer.GetSelIndex
    if Davor.GetText <> "HoHo " then Warnlog "Level 7:Seperator before is wrong.. Not 'Hoho '   But " + Davor.GetText
    if Dahinter.GetText <> " HiHi " then Warnlog "Level 7: Seperator after text is wrong. Not ' HiHi '   But " + Dahinter.GetText
    if BeginBei.GetText <> "4" then Warnlog "Level 7: Start at is not 4 but " + BeginnBei.GetText

    Ebene.Select 11
    if Nummer.GetSelIndex <> 0 then Warnlog "Level 1-10: Setting for 'Number' is not 'keine Selektion' but  " + Nummer.GetSelIndex
    if Davor.GetText <> "" then Warnlog "Level 1-10: Seperator before is not 'keine Selektion' but " + Davor.GetText
    if Dahinter.GetText <> "" then Warnlog "Level 1-10: Seperator after is not 'keine Selektion' but " + Dahinter.GetText
    if BeginBei.GetText <> "" then Warnlog "Level 1-10: Start at is not 'keine Selektion' but " + BeginBei.GetText

    Kontext
    Active.SetPage TabPositionKapitelnumerierung
    Kontext "TabPositionKapitelnumerierung"
    printlog "        - Tabpage Position"
    Level.Select 1
    if NumberingAlignment.GetSelIndex <> 2 then Warnlog "Level 1: Wrong Numbering Alignment:  Not = 2   But = "+ NumberingAlignment.GetSelIndex
    if V1 <> AlignedAt2.GetText then Warnlog "Level 1: Wrong Indent:  Not = "+ V1+ "  But = "+ AlignedAt2.GetText
    if V2 <> IndentAt2.GetText then Warnlog "Level 1: Space to Text  Not = "+ V2+ "  But = "+ IndentAt2.GetText
    Level.Select 3
    if NumberingAlignment.GetSelIndex <> 3 then Warnlog "Level 3: Wrong Numbering Alignment:  Not = 3   But = "+ NumberingAlignment.GetSelIndex
    if V3 <> AlignedAt2.GetText then Warnlog "Level 3: Wrong Indent:  Not = "+ V3+ "  But = "+ AlignedAt2.GetText
    if V4 <> AbstandTextNumerierung2.GetText then Warnlog "Level 3: Space to Text  Not = "+ V4+ "  But = "+ AbstandTextNumerierung2.GetText
    Level.Select 7
    if NumberingAlignment.GetSelIndex <> 1 then Warnlog "Level 7: Wrong Numbering Alignment:  Not = 1   But = "+ NumberingAlignment.GetSelIndex
    if V5 <> AlignedAt2.GetText then Warnlog "Level 7: Wrong Indent:  Not = "+ V5+ "  But = "+ AlignedAt2.GetText
    if V6 <> AbstandTextNumerierung2.GetText then Warnlog "Level 7: Space to Text  Not = "+ V6+ "  But = "+ AbstandTextNumerierung2.GetText
    if V7 <> IndentAt2.GetText then Warnlog "Level 7: Space to Text  Not = "+ V7+ "  But = "+ IndentAt2.GetText
    Level.Select 8
    if NumberingAlignment.GetSelIndex <> 3 then Warnlog "Level 8: Wrong Numbering Alignment:  Not = 3   But = "+ NumberingAlignment.GetSelIndex
    if V8 <> AlignedAt2.GetText then Warnlog "Level 8: Wrong Indent:  Not = "+ V8+ "  But = "+ AlignedAt2.GetText
    if V9 <> AbstandTextNumerierung2.GetText then Warnlog "Level 8: Space to Text  Not = "+ V9+ "  But = "+ AbstandTextNumerierung2.GetText
    if V10 <> IndentAt2.GetText then Warnlog "Level 8: Space to Text  Not = "+ V10+ "  But = "+ IndentAt2.GetText

    TabPositionKapitelnumerierung.Cancel

    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsFootnoteEndnote
    PrintLog "- Tools / Format Footnote- and Endnote"

    Call hNewDocument
    ToolsFootnote
    Printlog "  - adjust Footnote-properties"
    Kontext
    Active.SetPage TabFussnoten
    Kontext "TabFussnoten"
    Numerierung.Select 2
    Pro.Select 1
    if BeginnBei.IsEnabled then Warnlog "On Numbering 'per page' 'Start at' is active. Bug!"
    Pro.Select 2
    if BeginnBei.IsEnabled then Warnlog "On Numbering 'per Chaper' 'Start at' is active. Bug!"
    Pro.Select 3
    BeginnBei.SetText "3"
    Davor.SetText "Vorher "
    Dahinter.SetText " Hinterher"
    Absatz.Select 4
    ZeichenvorlageTextbereich.Select 1
    ZeichenvorlageFussnotenbereich.Select 1
    SeitenEnde.Check
    FolgeSeite.SetText "Aha"
    Beginn.SetText "oHo"

    Printlog "  - adjust Endnote-properties"
    Kontext
    Active.SetPage TabEndnoten
    Kontext "TabEndnoten"
    Numerierung.Select 1
    BeginnBei.SetText "8"
    Absatz.Select 3
    Seite.Select 2
    Davor.SetText "Hihi "
    Dahinter.SetText " HaHa"
    ZeichenvorlageTextbereich.Select 4
    ZeichenvorlageEndnotenbereich.Select 5
    TabEndnoten.OK

    call wTypeKeys "Footnote bound to this text"
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    Fussnote.Check
    FussnoteEinfuegen.OK

    call wTypeKeys "Wassup<PageUp><Return>Followed by an endnote."
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    Endnote.Check
    FussnoteEinfuegen.OK

    call wTypeKeys "Wassup<PageUp><Return>And again a footnote"
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    Fussnote.Check
    FussnoteEinfuegen.OK
    InsertFieldsPageNumbers
    call wTypeKeys "+0"
    call wTypeKeys "<Shift Home>"
    Sleep 1
    try
        ToolsCalculate
    catch
        ' Workaround for bug.
        call wTypeKeys "<End>"
        call wTypeKeys "<Shift Home>"
        try
            ToolsCalculate
            Warnlog "Selecting text in footnote with <Shift Home> works only at the second time (bug#101047)"
        catch
            Warnlog "Tools/Calculate is disabled!"
        endcatch
    endcatch
    if GetClipboardText <> "1" then Warnlog "2. Footnote has not been inserted on 1. Page."

    call wTypeKeys "<PageUp><Return>Again followed by an endnote"
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    Endnote.Check
    FussnoteEinfuegen.OK
    InsertFieldsPageNumbers
    call wTypeKeys "+0"
    call wTypeKeys "<Shift Home>"
    Sleep 1
    try
        ToolsCalculate
    catch
        ' Workaround for bug.
        call wTypeKeys "<End>"
        call wTypeKeys "<Shift Home>"
        try
            ToolsCalculate
        catch
            Warnlog "Tools/Calculate is disabled!"
        endcatch
    endcatch
    Sleep 1
    ToolsCalculate
    if GetClipboardText <> "2" then Warnlog "2. Footnote has not been inserted on 2. page"

    call wTypeKeys "<PageUp><Return>That's it!"

    printlog "    - check properties"
    ToolsFootnote
    Kontext
    Active.SetPage TabFussnoten
    Kontext "TabFussnoten"
    if Numerierung.GetSelIndex <> 2 then Warnlog "Footnote: Numbering not the 2. entry selected, but " + Numerierung.GetSelIndex
    if Pro.GetSelIndex <> 3 then Warnlog "Footnote: 'Per document (3. entry) is not selected but " + Pro.GetSelIndex
    if BeginnBei.GetText <> "3" then Warnlog "Footnote: Start at is not 3 but " + BeginnBei.GetText
    if Davor.GetText <> "Vorher " then Warnlog "Footnote: Text 'Before' is wrong. Not 'Vorher ' but " +  Davor.GetText
    if Dahinter.GetText <> " Hinterher" then Warnlog "Footnote: Text 'After' is wrong. Not ' Hinterher' but " +  Dahinter.GetText
    if Absatz.GetSelIndex <> 4 then Warnlog "Footnote: Wrong entry selected in paragraph. Not 4 but " + Absatz.GetSelIndex
    if ZeichenvorlageTextbereich.GetSelIndex <> 1 then Warnlog "Footnote: Text area has been changed!"
    if ZeichenvorlageFussnotenbereich.GetSelIndex <> 1 then Warnlog "Footnote: Footnote area has been changed!"
    if NOT SeitenEnde.IsChecked then Warnlog "Footnote: End of page is not checked anymore."
    if FolgeSeite.GetText <> "Aha" then Warnlog "Footnote: End of footnote is not 'Aha' but " + FolgeSeite.GetText
    if Beginn.GetText <> "oHo" then Warnlog "Footnote: Start of next page is not 'oHo' but " + Beginn.GetText

    Kontext
    Active.SetPage TabEndnoten
    Kontext "TabEndnoten"
    if Numerierung.GetSelIndex <> 1 then Warnlog "Endnote: Numbering not 1. entry but " + Numerierung.GetSelIndex
    if BeginnBei.GetText <> "8" then Warnlog "Endnote: Strat at is not 8 but " + BeginnBei.GetText
    if Davor.GetText <> "Hihi " then Warnlog "Endnote: 'Before' is wrong. Not 'Hihi ' but " +  Davor.GetText
    if Dahinter.GetText <> " HaHa" then Warnlog "Endnote: Text after is wrong. Not ' HaHa' but " +  Dahinter.GetText
    if Absatz.GetSelIndex <> 3 then Warnlog "Endnote: Wrong entry selected in paragraph. Not 3 but " + Absatz.GetSelIndex
    if Seite.GetSelIndex <> 2 then Warnlog "Endnote: Wrong Page entry selected. Not 2 but " + Seite.GetSelIndex
    if ZeichenvorlageTextbereich.GetSelIndex <> 4 then Warnlog "Endnote: Text area has been changed!"
    if ZeichenvorlageEndnotenbereich.GetSelIndex <> 5 then Warnlog "Endnote: Endnote area has been changed!"

    TabEndnoten.Cancel

    printlog "  - Save and reload"
    Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\tToolsFootnoteEndnote.odt", "writer8" )
    Call hCloseDocument
    hFileOpen( gOfficePath & "user\work\tToolsFootnoteEndnote.odt" )
    printlog "    - check saved properties"
    ToolsFootnote
    Kontext
    Active.SetPage TabFussnoten
    Kontext "TabFussnoten"
    if Numerierung.GetSelIndex <> 2 then Warnlog "Footnote: Numbering not the 2. entry selected, but " + Numerierung.GetSelIndex
    if Pro.GetSelIndex <> 3 then Warnlog "Footnote: 'Per document (3. entry) is not selected but " + Pro.GetSelIndex
    if BeginnBei.GetText <> "3" then Warnlog "Footnote: Start at is not 3 but " + BeginnBei.GetText
    if Davor.GetText <> "Vorher " then Warnlog "Footnote: Text 'Before' is wrong. Not 'Vorher ' but " +  Davor.GetText
    if Dahinter.GetText <> " Hinterher" then Warnlog "Footnote: Text 'After' is wrong. Not ' Hinterher' but " +  Dahinter.GetText
    if Absatz.GetSelIndex <> 4 then Warnlog "Footnote: Wrong entry selected in paragraph. Not 4 but " + Absatz.GetSelIndex
    if ZeichenvorlageTextbereich.GetSelIndex <> 1 then Warnlog "Footnote: Text area has been changed!"
    if ZeichenvorlageFussnotenbereich.GetSelIndex <> 1 then Warnlog "Footnote: Footnote area has been changed!"
    if NOT SeitenEnde.IsChecked then Warnlog "Footnote: End of page is not checked anymore."
    if FolgeSeite.GetText <> "Aha" then Warnlog "Footnote: End of footnote is not 'Aha' but " + FolgeSeite.GetText
    if Beginn.GetText <> "oHo" then Warnlog "Footnote: Start of next page is not 'oHo' but " + Beginn.GetText

    Kontext
    Active.SetPage TabEndnoten
    Kontext "TabEndnoten"
    if Numerierung.GetSelIndex <> 1 then Warnlog "Endnote: Numbering not 1. entry but " + Numerierung.GetSelIndex
    if BeginnBei.GetText <> "8" then Warnlog "Endnote: Strat at is not 8 but " + BeginnBei.GetText
    if Davor.GetText <> "Hihi " then Warnlog "Endnote: 'Before' is wrong. Not 'Hihi ' but " +  Davor.GetText
    if Dahinter.GetText <> " HaHa" then Warnlog "Endnote: Text after is wrong. Not ' HaHa' but " +  Dahinter.GetText
    if Absatz.GetSelIndex <> 3 then Warnlog "Endnote: Wrong entry selected in paragraph. Not 3 but " + Absatz.GetSelIndex
    if Seite.GetSelIndex <> 2 then Warnlog "Endnote: Wrong Page entry selected. Not 2 but " + Seite.GetSelIndex
    if ZeichenvorlageTextbereich.GetSelIndex <> 4 then Warnlog "Endnote: Text area has been changed!"
    if ZeichenvorlageEndnotenbereich.GetSelIndex <> 5 then Warnlog "Endnote: Endnote area has been changed!"
    TabEndnoten.Cancel

    Call hCloseDocument
endcase
