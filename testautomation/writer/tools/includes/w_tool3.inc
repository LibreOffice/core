'*******************************************************************************
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
'/******************************************************************************
'*
'* owner : helge.delfs@oracle.com
'*
'* short description : Description of file
'*
'\******************************************************************************

sub InTabelleKontrollieren
    Call wTypeKeys ("<Home>")
    Call wTypeKeys ("<Tab><Shift End>")
    hUseAsyncSlot( "EditCopy" )
    if GetClipBoardText <> "Text" then
        Warnlog "Text has not been inserted in a table"
    else
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<Shift End>"
    end if
end sub

' ****
sub InTabelleKontrollierenExtra
    Call wTypeKeys ("<Home><Shift Tab><Shift End>")
    hUseAsyncSlot( "EditCopy" )
    if GetClipBoardText <> "Der" then
        Warnlog "Text has not been inserted in a table"
    else
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<Shift End>"
    end if
end sub

'***
sub InTextKontrollieren
    Call wTypeKeys ("<Home><Shift Mod1 Right>")
    hUseAsyncSlot( "EditCopy" )
    if Left$(GetClipBoardText,3) <> "Der" AND Left$(GetClipboardText,4) <> "Der " then
        Warnlog "Table has not been changed to text"
    else
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<Shift End>"
    end if
end sub

'***
sub AufsteigendKontrollieren ( Zahl as Boolean )
    Call wTypeKeys "<Home><Up>",6
    wait 500
    Call wTypeKeys ("<Home><Shift End>" )
    hUseAsyncSlot( "EditCopy" )
    if Zahl = TRUE then
        if GetClipboardText <> "23" then Warnlog "1. Number not correctly sorted"
    else
        if GetClipboardText <> "Ahallo" then Warnlog "1. Word is not correctly sorted"
    end if
    Call wTypeKeys ( "<Down><Down><Home><Shift End>" )
    hUseAsyncSlot( "EditCopy" )
    if Zahl = TRUE then
        if GetClipboardText <> "54" then Warnlog "3. Number not correctly sorted"
    else
        if GetClipboardText <> "Ohallo" then Warnlog "3. Word not correctly sorted"
    end if
    Call wTypeKeys ( "<Down><Down><Home><Shift End>" )
    hUseAsyncSlot( "EditCopy" )
    if Zahl = TRUE then
        if GetClipboardText <> "65" then Warnlog "Last number is not correctly sorted"
    else
        if GetClipboardText <> "Zhallo" then Warnlog "Last word is not correctly sorted"
    end if
end sub

'***
sub AbsteigendKontrollieren ( Zahl as Boolean )
    Call wTypeKeys "<Home><Up>",6
    wait 500
    Call wTypeKeys ( "<Home><Shift End>" )
    hUseAsyncSlot( "EditCopy" )
    if Zahl = TRUE then
        if GetClipboardText <> "65" then Warnlog "1. Number not correctly sorted"
    else
        if GetClipboardText <> "Zhallo" then Warnlog "1. word not correctly sorted"
    end if
    Call wTypeKeys ( "<Down><Down><Down><Home><Shift End>" )
    hUseAsyncSlot( "EditCopy" )
    if Zahl = TRUE then
        if GetClipboardText <> "43" then Warnlog "4th number not correctly sorted"
    else
        if GetClipboardText <> "Ghallo" then Warnlog "4th word not correctly sorted"
    end if
    Call wTypeKeys ( "<Down><Home><Shift End>" )
    hUseAsyncSlot( "EditCopy" )
    if Zahl = TRUE then
        if GetClipboardText <> "23" then Warnlog "Wrong sort: last number. Should => "+"Ohallo"+"   Is =>"+GetClipboardText
    else
        if GetClipboardText <> "Ahallo" then Warnlog "Wrong sort: last word. Should => "+"Ohallo"+"   Is =>"+GetClipboardText
    end if
end sub

'***
sub AufSortierungPruefen(i as integer)
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<up>",6
    Call wTypeKeys "<Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<Right><Shift Mod1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "AAA" then Warnlog "Entry in 1.Column/1.Row incorrect"
    if i <> 1 then
        Call wTypeKeys "<Mod1 Right>", 3
    else
        Call wTypeKeys "<Mod1 Right>"
    end if
    Call wTypeKeys "<Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if Left$(GetClipboardText,3) <> "CCC" then Warnlog "Entry in 2.Column/1.Row incorrect"
    Call wTypeKeys "<Down><Home>"
    Call wTypeKeys "<Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<Home><Shift Mod1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "AAA" then Warnlog "Entry in 1.Column/2.Row incorrect"
    if i <> 1 then
        Call wTypeKeys "<Mod1 Right>", 3
    else
        Call wTypeKeys "<Mod1 Right>"
    end if
    Call wTypeKeys "<Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<Right><Shift Mod1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "DDD" then Warnlog "Entry in 2.Column/2.Row incorrect"
    Call wTypeKeys "<Down><Home>"
    if i <> 1 then
        Call wTypeKeys "<Mod1 Right>", 10
    else
        Call wTypeKeys "<Mod1 Right>", 5
    end if
    Call wTypeKeys "<Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<Right><Shift Mod1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "AAA" then Warnlog "Entry in 4.Column/3.Row incorrect"
end sub

'***
sub AbSortierungPruefen(i as integer)
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<up>", 6
    Call wTypeKeys "<Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<Right><Shift Mod1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "CCC" then Warnlog "Entry in 1.Column/1.Row incorrect"
    Call wTypeKeys "<Down><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<Home><Shift Mod1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "CCC" then Warnlog "Entry in 1.Column/2.Row incorrect"
    if i <> 1 then
        Call wTypeKeys "<Mod1 Right>" , 4
    else
        Call wTypeKeys "<Mod1 Right>" , 2
    end if
    Call wTypeKeys "<Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<MOD1 Left>", 2
        Call wTypeKeys "<Shift MOD1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "BBB" then Warnlog "Entry in 2.Column/2.Row incorrect"
    Call wTypeKeys "<Left><Up>"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<Right><Shift Mod1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "EEE" then Warnlog "Entry in 2.Column/1.Row incorrect"
    if i <> 1 then
        Call wTypeKeys "<Mod1 Right>", 6
        Call wTypeKeys "<Down>", 3
        Call wTypeKeys "<End><Shift Mod1 Left>"
    else
        Call wTypeKeys "<Mod1 Right>", 3
        Call wTypeKeys "<Down>", 3
        Call wTypeKeys "<Shift Mod1 Right>"
    end if
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Trennzeichen then
        QaErrorLog "#i94661# - Marked the Separator-Sign, correcting now."
        Call wTypeKeys "<Right><Shift Mod1 Right>"
        EditCopy
    end if
    if Left$(GetClipboardText,3) <> "AAA" then Warnlog "Entry in 2.Column/1.Row incorrect"
end sub

'***
sub UndoRichtigBeiSortierung
    Dim Fehler as Boolean
    Call wTypeKeys "<Left Right>"
    wait 100
    EditUndo
    Sleep 1
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<up>", 6
    Call wTypeKeys "<Shift Mod1 Right>"
    EditCopy
    if Left$(GetClipboardText,3) <> "AAA" then Fehler = TRUE
    Call wTypeKeys "<Down><Home><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if Left$(GetClipboardText,3) <> "BBB" then Fehler = TRUE
    Call wTypeKeys "<Down><Home><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if Left$(GetClipboardText,3) <> "CCC" then Fehler = TRUE
    Call wTypeKeys "<Down><Home><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if Left$(GetClipboardText,3) <> "AAA" then Fehler = TRUE
    Call wTypeKeys "<Down><Home><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if Left$(GetClipboardText,3) <> "BBB" then Fehler = TRUE
    Call wTypeKeys "<Down><Home><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if Left$(GetClipboardText,3) <> "CCC" then Fehler = TRUE
    if Fehler = TRUE then
        Warnlog "Undo after sorting has not been functioned correctly"
        Call hCloseDocument
        hFileOpenLocally(  gTesttoolPath + "writer\optional\input\st1_5.sdw" )
    end if
    Call wTypeKeys "<End>"
    Call wTypeKeys "<Down>",6
    Call wTypeKeys "<Mod1 a>"
    wait 100
end sub

'***
sub TrennzeichenSetzen ( Trennstring as String )
    EditSearchAndReplace
    Kontext "FindAndReplace"
    if FindAndReplace.NotExists then EditSearchAndReplace
    Kontext "FindAndReplace"
    if Not RegularExpressions.IsVisible then More.Click
    Select Case Trennstring
    Case ";"
        RegularExpressions.Check
        SearchFor.SetText "\t"

    Case "|"
        RegularExpressions.Check
        SearchFor.SetText "\t"

    end select

    ReplaceWith.SetText TrennString
    ReplaceAll.Click

    Kontext
    if MessageBox.Exists then
        if InStr( MessageBox.GetText, "30" ) = 0 then
            MessageBox.OK
            Kontext "FindAndReplace"
            RegularExpressions.Check
            SearchFor.SetText "\t"
            ReplaceWith.SetText TrennString
            ReplaceAll.Click
            Sleep 1
            Kontext
            if MessageBox.Exists then
                if InStr( MessageBox.GetText, "30" ) = 0 then
                    MessageBox.OK
                else
                    Warnlog "Unable to insert Character. Following test doesn't work!"
                    MessageBox.OK
                end if
            end if
        else
            MessageBox.OK
        end if
    end if
    Kontext "FindAndReplace"
    More.Click
    if FindAndReplace.Exists then FindAndReplace.Close
end sub

'***
function ZelleRuntertesten ( Wert$ ) as Boolean
    Call wTypeKeys "<Down>"
    Call wTypeKeys "<Mod1 a>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText = Wert$ then
        ZelleRuntertesten = TRUE
    else
        ZelleRuntertesten = FALSE
    end if
end function

'***

sub wEinfuegenEintragAlle ( NaWieSollErheissen$ )
    Call wTypeKeys NaWieSollErheissen$
    Call wTypeKeys "<Mod1 Left><Shift Mod1 Right>"
    InsertIndexesEntry
    Kontext "VerzeichniseintragEinfuegen"
    Verzeichnis.Select 1
    VerzeichniseintragEinfuegen.OK

    Call wTypeKeys "<End> "
    Call wTypeKeys "<Left><Shift Mod1 Left>"
    InsertIndexesEntry
    Kontext "VerzeichniseintragEinfuegen"
    Verzeichnis.Select 2
    VerzeichniseintragEinfuegen.OK

    Call wTypeKeys "<End> "
    Call wTypeKeys "<Left><Left><Shift Mod1 Left>"
    InsertIndexesEntry
    Kontext "VerzeichniseintragEinfuegen"
    Verzeichnis.Select 3
    VerzeichniseintragEinfuegen.OK
    Call wTypeKeys "<End> "
end sub

'***

sub ReihenfolgePruefen
    Dim sDummy$

    printlog "     - Checking entry's on first page"
    Call wTypeKeys "<Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    sDummy$ = GetClipboardText
    if sDummy$ <> "Zhallo" then
        if sDummy$ <> "Ahallo" then
            Warnlog "Wrong Sort: Should -> Zhallo   Is -> "+ sDummy$
        else
            Warnlog "Sort infinitely is not in alphabetical order"
            Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
            wait 100
            EditCopy
            if GetClipboardText <> "Zhallo" then Warnlog "2. entry not Zhallo but "+GetClipboardText
        end if
    else
        Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
        wait 100
        EditCopy
        if GetClipboardText <> "Ahallo" then Warnlog "2. entry not Ahallo but "+GetClipboardText
    end if
    Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Xhallo" then Warnlog "3. entry not Xhallo but "+GetClipboardText
    Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Ghallo" then Warnlog "4. entry not Ghallo but "+GetClipboardText

    printlog "     - checking entrys on 3rd page"
    Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    sDummy$ = GetClipboardText
    if sDummy$ <> "Zhallo" then
        if sDummy$ <> "Ahallo" then
            Warnlog "Wrong Sort: Should -> Zhallo   Is -> "+ sDummy$
        else
            Warnlog "Sort infinitely is not in alphabetical order"
            Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
            wait 100
            EditCopy
            if GetClipboardText <> "Zhallo" then Warnlog "6. entry not Zhallo but "+GetClipboardText
        end if
    else
        Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
        wait 100
        EditCopy
        if GetClipboardText <> "Ahallo" then Warnlog "6. entry not Ahallo but "+GetClipboardText
    end if
    Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Xhallo" then Warnlog "7. entry not Xhallo but "+GetClipboardText
    Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Ghallo" then Warnlog "8. entry not Ghallo but "+GetClipboardText

    printlog "     - checking page entrys"
    Call wTypeKeys "<End><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "3" then Warnlog "8. Entry: Not page 3 but "+ GetClipboardText
    Call wTypeKeys "<End><Up><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "3" then Warnlog "7. Entry: Not page 3 but "+ GetClipboardText
    Call wTypeKeys "<End><Up><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "3" then Warnlog "6. entry: Not page 3 but "+ GetClipboardText
    Call wTypeKeys "<End><Up><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "3" then Warnlog "5. entry: Not page 3 but "+ GetClipboardText
    Call wTypeKeys "<End><Up><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "1" then Warnlog "4. entry: Not page 1 but "+ GetClipboardText
    Call wTypeKeys "<End><Up><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "1" then Warnlog "3. entry: Not page 1 but "+ GetClipboardText
    Call wTypeKeys "<End><Up><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "1" then Warnlog "2. entry: Not page 1 but "+ GetClipboardText
    Call wTypeKeys "<End><Up><Shift Mod1 Left>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "1" then Warnlog "1. entry: Not page 1 but "+ GetClipboardText
    Call wTypeKeys "<End><Up><Shift Mod1 Left>"
end sub

'***
sub ReihenfolgePruefen2
    printlog "     - checking entrys"
    Call wTypeKeys "<Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Ahallo" then Warnlog "1. entry not Ahallo but "+GetClipboardText
    Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Ghallo" then Warnlog "2. entry not Ghallo but "+GetClipboardText
    Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Xhallo" then Warnlog "3. entry not Xhallo but "+GetClipboardText
    Call wTypeKeys "<Home><Down><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Zhallo" then Warnlog "4. entry not Zhallo but "+GetClipboardText
    Call wTypeKeys "<Down><Mod1 Left><Shift Mod1 Right>"
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "Bhallo " then Warnlog "Number of entrys more then 4."

    printlog "     - checking page entrys"
    Call wTypeKeys "<Up>"
    Call wTypeKeys "<Shift Mod1 Right>", 3
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "1, 3" then Warnlog "4. entry: Not '1, 3' but "+ GetClipboardText
    Call wTypeKeys "<Up>"
    Call wTypeKeys "<Shift Mod1 Left>", 3
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "1, 3" then Warnlog "3. entry: Not '1, 3' but "+ GetClipboardText
    Call wTypeKeys "<Up>"
    Call wTypeKeys "<Shift Mod1 Right>", 3
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "1, 3" then Warnlog "2. entry: Not '1, 3' but "+ GetClipboardText
    Call wTypeKeys "<Up>"
    Call wTypeKeys "<Shift Mod1 Left>", 3
    hUseAsyncSlot( "EditCopy" )
    if GetClipboardText <> "1, 3" then Warnlog "1. entry: Not '1, 3' but "+ GetClipboardText
end sub
' ***
function SchriftFormatTesten ( sName$ ) as String
    SchriftFormatTesten = sName$
    FormatCharacter
    Kontext
    Active.SetPage TabFont
    Kontext "TabFont"
    if FontWest.GetSelText <> sName then SchriftFormatTesten = FontWest.GetSelText
    TabFont.OK
end function

' *******************************************************************

function ErmittleErgebnis ( Ergebnis as String ) as Boolean
    ErmittleErgebnis = TRUE
    Kontext "DocumentWriter"
    Call wTypeKeys "<End><Shift Home>"
    EditCopy
    Wait 500
    if GetClipboardText <> Ergebnis then ErmittleErgebnis = FALSE
end function

' *******************************************************************

sub VerzeichnisUeberNavigator(Welches as integer)
    dim j as integer
    Kontext "Navigator"
    if Navigator.NotExists then ViewNavigator
    Kontext "NavigatorWriter"
    if Auswahlliste.GetItemCount<11 then Inhaltsansicht.Click

    if Auswahlliste.GetItemCount >11 then
        For j= 1 to 11
            Auswahlliste.Select j
            Auswahlliste.TypeKeys "-"
        next j
    end if

    wait 500

    Auswahlliste.Select 10
    Auswahlliste.TypeKeys "+"
    Auswahlliste.Select 10 + Welches
    Auswahlliste.TypeKeys "<Return>"
    Kontext "DocumentWriter"
end sub

'**********************************************************

sub sZahlenformatfuerZelle
    Kontext "DocumentWriter"
    FormatNumberFormat
    Kontext "Zahlenformat"
    Kategorie.Select 3
    KategorieFormat.Select 1
    Zahlenformat.OK
    Kontext "DocumentWriter"
end sub

'**********************************************************

sub sNavigatorInBrauchbarenZustand
    dim j as integer
    Kontext "NavigatorWriter"
    if NavigatorWriter.NotExists then ViewNavigator
    if Auswahlliste.GetItemCount<11 then Inhaltsansicht.Click
    Wait 500
    if Auswahlliste.GetItemCount >11 then
        For j= 1 to 11
            Auswahlliste.Select j
            Auswahlliste.TypeKeys "-"
        next j
    end if

    Wait 500
    Auswahlliste.Select 2
    Auswahlliste.TypeKeys "+"
    Inhaltsansicht.Click
    Kontext "DocumentWriter"
end sub

'**********************************************************

sub sZellinhaltInZwischenablage(VergleichDas as string,Aktion as string)
    Call wTypeKeys "<Home><Shift End>"
    EditCopy
    if VergleichDas <> GetClipboardText then Warnlog "- Table has not been splitted correctly!(" + Aktion + ")"
end sub

'**********************************************************

sub sSeiteMitBrauchbarerTabelleEinrichten
    Call hTabelleEinfuegen
    FormatTable
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    Tabellenname.Settext "MeineTabelle"
    TabTabelle.OK

    Call wTypeKeys "�berschrift 1"
    Call wTypeKeys "<Down>"
    Call wTypeKeys "Text 1"
    Call wTypeKeys "<Down>"
    wait 500
    Call hTabelleEinfuegen
    wait 500
    Call wTypeKeys "�berschrift 2"
    Call wTypeKeys "<Down>"
    Call wTypeKeys "Text 2"
    Call wTypeKeys "<Up>",1
end sub

'**********************************************************

sub wOpenContextMenu ( optional FocusParam as boolean)
    if IsMissing(FocusParam) = True then
        Select Case gApplication
        Case "WRITER"
            Kontext "DocumentWriter"
            Call DocumentWriter.TypeKeys "<SHIFT F10>"  'OpenContextMenu

        Case "MASTERDOCUMENT"
            Kontext "DocumentMasterDoc"
            Call DocumentMasterDoc.TypeKeys "<SHIFT F10>"  'OpenContextMenu

        end select
    else
        if FocusParam = True then
            Select Case gApplication
            Case "WRITER"
                Kontext "DocumentWriter"
                Call DocumentWriter.OpenContextMenu true

            Case "MASTERDOCUMENT"
                Kontext "DocumentMasterDoc"
                Call DocumentMasterDoc.OpenContextMenu true

            end select
        else
            Select Case gApplication
            Case "WRITER"
                Kontext "DocumentWriter"
                Call DocumentWriter.OpenContextMenu

            Case "MASTERDOCUMENT"
                Kontext "DocumentMasterDoc"
                Call DocumentMasterDoc.OpenContextMenu
            end select
        end if
    end if
end sub
