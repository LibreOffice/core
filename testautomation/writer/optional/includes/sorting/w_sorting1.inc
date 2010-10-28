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
'* short description : Sorting functions in Writer
'*
'\***********************************************************************

sub w_sorting

    Call tToolsSort1                         ' Text in one Column and 1 Key
    Call tToolsSort2                         ' Numbers in 1 Column and 1 Key
    Call tToolsSort3                         ' Text in several Columns and 3 Keys
    Call tToolsSort4                         ' Table with Header ( Text and Numbers mixed )
    Call tToolsSort5                         ' Table without Header ( Text and Numbers mixed )
    Call tToolsSort6                         ' Copy sorted table in another table

end sub


testcase tToolsSort1
    PrintLog "- Tools / Sort normal Text ( 1. Column )"    
    Printlog "   - alphanumeric ascending ( Key 1 )"
    Call hNewDocument
    Call wTypeKeys "Ohallo<Return>Ahallo<Return>Ghallo<Return>Zhallo<Return>Shallo"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Schluessel2.UnCheck
    Schluessel3.UnCheck
    Spalte1.SetText "1"
    Schluesseltyp1.Select 1
    Aufsteigend1.Check
    Sortieren.OK
    Call AufsteigendKontrollieren ( FALSE )

    Printlog "   - alphanumeric descending ( Key 1 )"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Spalte1.SetText "1"
    Schluesseltyp1.Select 1
    Absteigend1.Check
    Sortieren.OK
    AbsteigendKontrollieren ( FALSE )

    Printlog "   - alphanumeric ascending ( Key 2 )"
    Call wTypeKeys "<Mod1 a>"
    Call wTypeKeys "<Delete>"
    Sleep 1
    Call wTypeKeys "Ohallo<Return>Ahallo<Return>Ghallo<Return>Zhallo<Return>Shallo"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel2.Check
    Schluessel1.Uncheck
    Schluessel3.UnCheck
    Spalte2.SetText "1"
    Schluesseltyp2.Select 1
    Aufsteigend2.Check
    Sortieren.OK
    Call AufsteigendKontrollieren ( FALSE )

    Printlog "   - alphanumeric descending ( Key 2 )"
    Call wTypeKeys "<Mod1 a>"
    ToolsSort
    Kontext "Sortieren"
    Spalte2.SetText "1"
    Schluesseltyp2.Select 1
    Absteigend2.Check
    Sortieren.OK
    AbsteigendKontrollieren ( FALSE )

    Printlog "   - alphanumeric ascending ( Key 3 )"
    Call wTypeKeys "<Mod1 a>"
    Call wTypeKeys "<Delete>"
    Sleep 1
    Call wTypeKeys "Ohallo<Return>Ahallo<Return>Ghallo<Return>Zhallo<Return>Shallo"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel3.Check
    Schluessel1.UnCheck
    Schluessel2.UnCheck
    Spalte3.SetText "1"
    Schluesseltyp3.Select 1
    Aufsteigend3.Check
    Sortieren.OK
    Call AufsteigendKontrollieren ( FALSE )

    Printlog "   - alphanumeric descending ( Key 3 )"
    Call wTypeKeys "<Mod1 a>"
    ToolsSort
    Kontext "Sortieren"
    Schluessel3.Check
    Schluessel1.Uncheck
    Schluessel2.UnCheck
    Spalte3.SetText "1"
    Schluesseltyp3.Select 1
    Absteigend3.Check
    Sortieren.OK
    Call AbsteigendKontrollieren ( FALSE )

    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSort2
    PrintLog "- Tools / Sorting numbers ( 1. Column )"

    Printlog "   - numeric ascending ( Key 1 )"
    Call hNewDocument
    Call wTypeKeys "43<Return>54<Return>23<Return>65<Return>56"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Spalte1.SetText "1"
    Schluesseltyp1.Select 2
    Aufsteigend1.Check
    Sortieren.OK
    Call AufsteigendKontrollieren ( TRUE )

    Printlog "   - numeric descending ( Key 1 )"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Spalte1.SetText "1"
    Schluesseltyp1.Select 2
    Absteigend1.Check
    Sortieren.OK
    Call AbsteigendKontrollieren ( TRUE )

    Printlog "   - numeric ascending ( Key 2 )"
    Call wTypeKeys "<Mod1 a>"
    Call wTypeKeys "<Delete>"
    Sleep 1
    Call wTypeKeys "43<Return>54<Return>23<Return>65<Return>56"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Uncheck
    Schluessel2.Check
    Spalte2.SetText "1"
    Schluesseltyp2.Select 2
    Aufsteigend2.Check
    Sortieren.OK
    Call AufsteigendKontrollieren ( TRUE )
    
    Printlog "   - numeric descending ( Key 2 )"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Uncheck
    Schluessel2.Check
    Spalte2.SetText "1"
    Schluesseltyp2.Select 2
    Absteigend2.Check
    Sortieren.OK
    Call AbsteigendKontrollieren ( TRUE )

    Printlog "   - numeric ascending ( Key 3 )"
    Call wTypeKeys ( "<Mod1 a>" )
    Call wTypeKeys ( "<Delete>" )
    Sleep 1
    Call wTypeKeys "43<Return>54<Return>23<Return>65<Return>56"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Uncheck
    Schluessel2.UnCheck
    Schluessel3.Check
    Spalte3.SetText "1"
    Schluesseltyp3.Select 2
    Aufsteigend3.Check
    Sortieren.OK
    Call AufsteigendKontrollieren ( TRUE )

    Printlog "   - numeric descending ( Key 3 )"
    Call wTypeKeys ( "<Mod1 a>" )
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Uncheck
    Schluessel2.UnCheck
    Schluessel3.Check
    Spalte3.SetText "1"
    Schluesseltyp3.Select 2
    Absteigend3.Check
    Sortieren.OK
    Call AbsteigendKontrollieren ( TRUE )

    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSort3
    Dim i as Integer
    PrintLog "- Tools / Sorting text in several lines"
    for i=1 to 3
        select case i
            case 1: Printlog "  - Separator is a tabulator"
            case 2: Printlog "  - Separator is a semicolon"
                    Trennzeichen = ";"
            case 3: Printlog "  - Separator is a '|'"
                     Trennzeichen = "|"
        end select
        if gApplication = "WRITER" then
            hFileOpenLocally(  gTesttoolPath + "writer\optional\input\sorting\sort2.odt" )
        else
            hFileOpenLocally(  gTesttoolPath + "writer\optional\input\sorting\sort2.odm" )
        end if

        if i <> 1 then TrennzeichenSetzen ( Trennzeichen )

        Printlog "     - alphanumeric ascending after 1.Column, followed by 4.Column and 3. Column"
        Call wTypeKeys ( "<Mod1 a>" )
        ToolsSort
        Kontext "Sortieren"
        Schluessel1.Check
        Spalte1.SetText "1"
        Schluesseltyp1.Select 1
        Aufsteigend1.Check

        Schluessel2.Check
        Spalte2.SetText "4"
        Schluesseltyp2.Select 1
        Aufsteigend2.Check

        Schluessel3.Check
        Spalte3.SetText "3"
        Schluesseltyp3.Select 1
        Aufsteigend3.Check

        if i=1 then
            Tabulator.Check
        else
            Zeichen.Check
            ZeichenText.SetText Trennzeichen
        end if
        Sortieren.OK
        Call AufSortierungPruefen(i)
        Call UndoRichtigBeiSortierung

        Printlog "     - alphanumeric descending 1.Column, followed by 4. column and 3. column"
        ToolsSort
        Kontext "Sortieren"
        Schluessel1.Check
        Spalte1.SetText "1"
        Schluesseltyp1.Select 1

        Schluessel2.Check
        Spalte2.SetText "4"
        Schluesseltyp2.Select 1

        Schluessel3.Check
        Spalte3.SetText "3"
        Schluesseltyp3.Select 1

        Absteigend1.Check
        Absteigend2.Check
        Absteigend3.Check
        if i=1 then
            Tabulator.Check
        else
            Zeichen.Check
            ZeichenText.SetText Trennzeichen
        end if
        Sortieren.OK
        Call AbSortierungPruefen(i)
        Call UndoRichtigBeiSortierung
        Call hCloseDocument
    next i

   'Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSort4
    PrintLog "- Tools / Sorting in table with header"
    if gApplication = "WRITER" then
        hFileOpenLocally(  gTesttoolPath + "writer\optional\input\sorting\sort1.odt" )
    else
        hFileOpenLocally(  gTesttoolPath + "writer\optional\input\sorting\sort1.odm" )
    end if
    Call wTypeKeys "<Mod1 a>", 2

    Printlog "   - 3. column alphanum., 1. column num., 3. column alphanum. ascending"
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Spalte1.SetText "3"
    Schluesseltyp1.Select 1

    Schluessel2.Check
    Spalte2.SetText "1"
    Schluesseltyp2.Select 2

    Schluessel3.Check
    Spalte3.SetText "2"
    Schluesseltyp3.Select 1
    Aufsteigend1.Check
    Aufsteigend2.Check
    Aufsteigend3.Check
    if Tabulator.IsEnabled then Warnlog "Tabs is active"
    if Zeichen.IsEnabled then Warnlog "Char is active"
    Sortieren.OK

    Printlog "   - Check"
    Call wTypeKeys "<Mod1 a>", 3   ' Umstellung zur 5.0 Cursor steht nach Sortierung in der letzten Zelle, mit 3*Ctrl_A kommt man in die 1.
    wait 100
    EditCopy
    if GetClipboardText <> "1" then Warnlog "Header of the first column has been sorted"
    if ZelleRuntertesten("2") = FALSE then Warnlog "1/2 is wrong"
    if ZelleRuntertesten("4") = FALSE then Warnlog "1/3 is wrong"
    if ZelleRuntertesten("5") = FALSE then Warnlog "1/4 is wrong"
    if ZelleRuntertesten("3") = FALSE then Warnlog "1/5 is wrong"
    if ZelleRuntertesten("6") = FALSE then Warnlog "1/6 is wrong"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<up>", 5
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "a" then Warnlog "Header of the 2. column has not been sorted"
    if ZelleRuntertesten("a") = FALSE then Warnlog "2/2 is wrong"
    if ZelleRuntertesten("b") = FALSE then Warnlog "2/3 is wrong"
    if ZelleRuntertesten("c") = FALSE then Warnlog "2/4 is wrong"
    if ZelleRuntertesten("b") = FALSE then Warnlog "2/5 is wrong"
    if ZelleRuntertesten("c") = FALSE then Warnlog "2/6 is wrong"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<up>", 5
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "zz" then Warnlog "Header der 3. column has not been sorted"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/2 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/3 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/4 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/5 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/6 is wrong"

    Printlog "   - 3. column alphanum., 1. column num., 3. column alphanum. descending"
    Call wTypeKeys "<Left Right>"
    Call wTypeKeys "<Mod1 a>", 2
    Sleep 2
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Spalte1.SetText "3"
    Schluesseltyp1.Select 1

    Schluessel2.Check
    Spalte2.SetText "1"
    Schluesseltyp2.Select 2

    Schluessel3.Check
    Spalte3.SetText "2"
    Schluesseltyp3.Select 1
    Absteigend1.Check
    Absteigend2.Check
    Absteigend3.Check
    Sortieren.OK

    Printlog "   - check"
    Call wTypeKeys "<up><Left>", 6
    Call wTypeKeys "<Mod1 a>"
    wait 100
    EditCopy
    if GetClipboardText <> "1" then Warnlog "Header of the 1. column has been sorted"
    if ZelleRuntertesten("6") = FALSE then Warnlog "1/2 is wrong"
    if ZelleRuntertesten("3") = FALSE then Warnlog "1/3 is wrong"
    if ZelleRuntertesten("5") = FALSE then Warnlog "1/4 is wrong"
    if ZelleRuntertesten("4") = FALSE then Warnlog "1/5 is wrong"
    if ZelleRuntertesten("2") = FALSE then Warnlog "1/6 is wrong"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<up>", 5
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "a" then Warnlog "Header of the 2. column has not been sorted"
    if ZelleRuntertesten("c") = FALSE then Warnlog "2/2 is wrong"
    if ZelleRuntertesten("b") = FALSE then Warnlog "2/3 is wrong"
    if ZelleRuntertesten("c") = FALSE then Warnlog "2/4 is wrong"
    if ZelleRuntertesten("b") = FALSE then Warnlog "2/5 is wrong"
    if ZelleRuntertesten("a") = FALSE then Warnlog "2/6 is wrong"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<up>", 5
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "zz" then Warnlog "Header of the 3. column has not been sorted"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/2 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/3 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/4 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/5 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/6 is wrong"
    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSort5
    PrintLog "- Tools / Sorting in table without header"
    if gApplication = "WRITER" then
        hFileOpenLocally(  gTesttoolPath + "writer\optional\input\sorting\sort1.odt" )
    else
        hFileOpenLocally(  gTesttoolPath + "writer\optional\input\sorting\sort1.odm" )
    end if

    Call wNavigatorAuswahl (2, 2)
    Call wTypeKeys "<Mod1 a>", 2
    Printlog "   - 3. column alphanum., 1. column num., 2. column alphanum. ascending"
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Spalte1.SetText "3"
    Schluesseltyp1.Select 1

    Schluessel2.Check
    Spalte2.SetText "1"
    Schluesseltyp2.Select 2

    Schluessel3.Check
    Spalte3.SetText "2"
    Schluesseltyp3.Select 1
    Aufsteigend1.Check
    Aufsteigend2.Check
    Aufsteigend3.Check
    if Tabulator.IsEnabled then Warnlog "'Tabs' is active when table is selected"
    if Zeichen.IsEnabled then Warnlog "'Character' is active when table is selected"
    Sortieren.OK

    Printlog "   - check"
    Call wNavigatorAuswahl (2, 2)
    Call wTypeKeys "<Mod1 a>"
    wait 100
    EditCopy

    if GetClipboardText <> "2" then Warnlog "1/1 is wrong!"
    if ZelleRuntertesten("4") = FALSE then Warnlog "1/2 is wrong"
    if ZelleRuntertesten("5") = FALSE then Warnlog "1/3 is wrong"
    if ZelleRuntertesten("1") = FALSE then Warnlog "1/4 is wrong"
    if ZelleRuntertesten("3") = FALSE then Warnlog "1/5 is wrong"
    if ZelleRuntertesten("6") = FALSE then Warnlog "1/6 is wrong"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<up>", 5
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "a" then Warnlog "2/1 is wrong!"
    if ZelleRuntertesten("b") = FALSE then Warnlog "2/2 is wrong"
    if ZelleRuntertesten("c") = FALSE then Warnlog "2/3 is wrong"
    if ZelleRuntertesten("a") = FALSE then Warnlog "2/4 is wrong"
    if ZelleRuntertesten("b") = FALSE then Warnlog "2/5 is wrong"
    if ZelleRuntertesten("c") = FALSE then Warnlog "2/6 is wrong"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<up>", 5
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "xx" then Warnlog "3/1 is wrong!"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/2 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/3 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/4 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/5 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/6 is wrong"


    Printlog "   - 3. column alphanum., 1. column num., 2. column alphanum. descending"
    Call wNavigatorAuswahl (2, 2)
    Call wTypeKeys "<Mod1 a>", 2
    Sleep 2
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Spalte1.SetText "3"
    Schluesseltyp1.Select 1

    Schluessel2.Check
    Spalte2.SetText "1"
    Schluesseltyp2.Select 2

    Schluessel3.Check
    Spalte3.SetText "2"
    Schluesseltyp3.Select 1
    Absteigend1.Check
    Absteigend2.Check
    Absteigend3.Check
    Sortieren.OK

    Printlog "   - check"
    Call wNavigatorAuswahl (2, 2)
    Call wTypeKeys "<Mod1 a>"
    wait 100
    EditCopy
    if GetClipboardText <> "6" then Warnlog "1/1 is wrong!"
    if ZelleRuntertesten("3") = FALSE then Warnlog "1/2 is wrong"
    if ZelleRuntertesten("1") = FALSE then Warnlog "1/3 is wrong"
    if ZelleRuntertesten("5") = FALSE then Warnlog "1/4 is wrong"
    if ZelleRuntertesten("4") = FALSE then Warnlog "1/5 is wrong"
    if ZelleRuntertesten("2") = FALSE then Warnlog "1/6 is wrong"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<up>", 5
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "c" then Warnlog "2/1 is wrong!"
    if ZelleRuntertesten("b") = FALSE then Warnlog "2/2 is wrong"
    if ZelleRuntertesten("a") = FALSE then Warnlog "2/3 is wrong"
    if ZelleRuntertesten("c") = FALSE then Warnlog "2/4 is wrong"
    if ZelleRuntertesten("b") = FALSE then Warnlog "2/5 is wrong"
    if ZelleRuntertesten("a") = FALSE then Warnlog "2/6 is wrong"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<up>", 5
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "zz" then Warnlog "3/1 is wrong!"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/2 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/3 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/4 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/5 is wrong"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/6 is wrong"
    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSort6
    PrintLog "- Tools / Sort a sorted table and copy in another one"

    if gApplication = "WRITER" then
        hFileOpenLocally(  gTesttoolPath + "writer\optional\input\sorting\sort3.odt" )
    else
        hFileOpenLocally(  gTesttoolPath + "writer\optional\input\sorting\sort3.odm" )
    end if
    Call wTypeKeys "<Mod1 a>", 2

    Printlog "   - Sort 1. column numeric descending"
    ToolsSort
    Kontext "Sortieren"
    Schluessel1.Check
    Spalte1.SetText "1"
    Schluesseltyp1.Select 2
    Absteigend1.Check
    Absteigend2.Check
    Absteigend3.Check
    Sortieren.OK

    Printlog "   - check"
    Call wTypeKeys "<Mod1 a>", 3
    wait 100
    EditCopy
    if GetClipboardText <> "4" then Warnlog "Header of the 1. column has not been sorted"
    if ZelleRuntertesten("3") = FALSE then Warnlog "1/2 is wrong"
    if ZelleRuntertesten("2") = FALSE then Warnlog "1/3 is wrong"
    if ZelleRuntertesten("1") = FALSE then Warnlog "1/4 is wrong"
    Call wTypeKeys "<Tab>"
    Call wTypeKeys "<up>", 3
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "dd" then Warnlog "Header of the 2. column has not been sorted"
    if ZelleRuntertesten("cc") = FALSE then Warnlog "2/2 is wrong"
    if ZelleRuntertesten("bb") = FALSE then Warnlog "2/3 is wrong"
    if ZelleRuntertesten("aa") = FALSE then Warnlog "2/4 is wrong"
    Call wTypeKeys "<Tab>"
    Call wTypeKeys "<up>", 3
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "ww" then Warnlog "Header of the 3 column has not been sorted"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/2 is wrong"
    if ZelleRuntertesten("yy") = FALSE then Warnlog "3/3 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/4 is wrong"

    Printlog "   - copy table"
    Call wTypeKeys "<Left Right>"
    Call wTypeKeys "<Mod1 a>", 2
    Sleep 1
    EditCopy
    Sleep 1
    Printlog "   - insert table"
    Call wTypeKeys "<Down>", 10
    Call wTypeKeys "<Up>", 4
    EditPaste
    Sleep 1

    Printlog "   - check"
    Call wTypeKeys "<Mod1 a>"
    wait 100
    EditCopy
    if GetClipboardText <> "4" then Warnlog "Header of the 1. column has not been sorted"
    if ZelleRuntertesten("3") = FALSE then Warnlog "1/2 is wrong"
    if ZelleRuntertesten("2") = FALSE then Warnlog "1/3 is wrong"
    if ZelleRuntertesten("1") = FALSE then Warnlog "1/4 is wrong"
    Call wTypeKeys "<Tab>"
    Call wTypeKeys "<up>", 3
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "dd" then Warnlog "Header of the 2. column has not been sorted"
    if ZelleRuntertesten("cc") = FALSE then Warnlog "2/2 is wrong"
    if ZelleRuntertesten("bb") = FALSE then Warnlog "2/3 is wrong"
    if ZelleRuntertesten("aa") = FALSE then Warnlog "2/4 is wrong"
    Call wTypeKeys "<Tab>"
    Call wTypeKeys "<up>", 3
    Call wTypeKeys "<Mod1 a>"
    Wait 100
    EditCopy
    if GetClipboardText <> "ww" then Warnlog "Header of the 3 column has not been sorted"
    if ZelleRuntertesten("xx") = FALSE then Warnlog "3/2 is wrong"
    if ZelleRuntertesten("yy") = FALSE then Warnlog "3/3 is wrong"
    if ZelleRuntertesten("zz") = FALSE then Warnlog "3/4 is wrong"
    Call hCloseDocument
endcase

