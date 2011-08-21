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
'* short description : Test the table functionality in Writer
'*
'\***********************************************************************

sub w_204a_

    Call sTableName
    Call sTableAlignment
    Call sTableMergeCell
    Call TableSplit_copy_heading
    Call TableSplit_custom_heading
    Call TableSplit_custom_heading_apply_style
    Call TableSplit_no_heading
    Call sTableMerge
    Call sTablePagebreak
    Call sTableBorder
    Call sTableIndent
    Call sTableSort

end sub

' ----------------------------------------------------------------------

testcase sTableName
    dim temp(10) as string
    Call hNewDocument

    Call TBOhTabelleEinfuegen ("dummy" ,0,1,0,1,"1",tHeight:="1")  ' insert table
    Call wTypeKeys "<mod1 end><mod1 end>"
    temp (1) = "Garfield"
    Call TBOhTabelleEinfuegen (temp (1) ,0,1,0,1,"10",tHeight:="7")  ' insert table

    printlog "'///  Format table via Format->Table ///"
    Printlog "'///      change tablename ///"
    TableTableProperties                ' get into existing table
    Sleep (2)
    Kontext "TabelleEinfuegenWriter"
    sleep (2)
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"

    '  0. right table ?? ///'
    if (TabellenName.GetText <> temp (1) ) then warnlog " --- wrong table? schould be : "+temp (1)+ ", is : "+TabellenName.GetText

    '///  1. try: space included ///'
    temp (1) = "Gar field"
    try
        TabellenName.SetText temp (1)
    catch
        printlog " - couldn't insert : " + temp (1)
    endcatch
    if (TabellenName.GetText = temp (1) ) then warnlog " --- ups, could insert smth. with space? : "+TabellenName.GetText

    '///  2. try: dot included ///'
    temp (1) = "Gar.field"
    try
        TabellenName.SetText temp (1)
    catch
        printlog " - couldn't insert : "+temp (1)
    endcatch
    if (TabellenName.GetText = temp (1) ) then warnlog " --- ups, could insert smth. with dot? : "+TabellenName.GetText

    TabTabelle.cancel
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase sTableAlignment
    dim temp(10) as string
    dim PAGEWIDTH as double
    dim ac as integer, fc as integer
    dim i as integer
    dim sBug as string

    Call hNewDocument
    Call TBOhTabelleEinfuegen ("dummy" ,0,1,0,1,"1",tHeight:="1")  ' insert table
    Call wTypeKeys "<mod1 end><mod1 end>"
    temp (1) = "Garfield"
    Call TBOhTabelleEinfuegen (temp (1) ,0,1,0,1,"10",tHeight:="7")  ' insert table

    '   ' get text to init some constances for this test
    '   temp(2) = NachOben.GetText '----------------------------------------------------
    ' initialisation of some constantzen for this test  '------------------------------
    PAGEWIDTH = hGetPageWidth()
    printlog "--- Pagewidth is: " + PAGEWIDTH
    'MEASUNIT =  GetMeasUnit (temp(2))
    'DECSEP = GetDecimalSeperator(temp(2))

    Printlog "'///     set alignment of table ///"
    TableTableProperties
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    Printlog "'///      Left ///"
    Links.check
    sleep (1)
    if ((Breite.IsWritable AND NachRechts.IsWritable) <> TRUE) then warnlog "edit field not active "
    if (LiberalMeasurement(PAGEWIDTH ,(StrToDouble (Breite.GetText) + StrToDouble (NachRechts.GetText))) <> TRUE) then
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
            warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachRechts.GetText))+" should: " + PAGEWIDTH
        end if
    end if
    NachRechts.SetText "2"
    sleep (1)
    temp(1) = NachRechts.GetText
    TabTabelle.OK
    TableTableProperties
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    if (Links.Ischecked <> TRUE) then warnlog "links state changed"
    if ((Breite.IsWritable AND NachRechts.IsWritable) <> TRUE) then warnlog "edit field not active"
    if (temp(1) <> NachRechts.GetText) then warnlog "value changed"
    if (LiberalMeasurement(PAGEWIDTH ,(StrToDouble (Breite.GetText) + StrToDouble (NachRechts.GetText))) <> TRUE) then
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
            warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachRechts.GetText))+" should: "+PAGEWIDTH
        end if
    end if
    '--------------------------------
    NachRechts.SetText "0"
    sleep (1)
    Printlog "'///       FromLeft ///"
    VonLinks.check
    sleep (1)
    if ((Breite.IsWritable AND NachLinks.IsWritable) <> TRUE) then warnlog "edit field not active "
    if (LiberalMeasurement(PAGEWIDTH ,StrToDouble (Breite.GetText) + StrToDouble(NachLinks.GetText)) <> TRUE) then
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
            warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText))+" should: "+PAGEWIDTH
        end if
    end if
    NachLinks.SetText "2"
    sleep (1)
    temp(1) = NachLinks.GetText
    TabTabelle.OK
    sleep 1
    TableTableProperties
    Kontext "TabelleEinfuegenWriter"
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    if (VonLinks.Ischecked <> TRUE) then warnlog "VonLinks state changed"
    'sBug = NachLinks.GetText
    if ((Breite.IsEnabled AND NachLinks.IsEnabled) <> TRUE) then warnlog "edit field not active "
    'if (NachLinks.IsWritable <> TRUE) then warnlog "edit field not active "
    'if sBug <> NachLinks.GetText then warnlog "TESTOOL BUG; is writable loescht feld "
    if (temp(1) <> NachLinks.GetText) then warnlog "value changed"+" should: "+temp(1)+" is:  "+ NachLinks.GetText
    if (LiberalMeasurement(PAGEWIDTH ,StrToDouble (Breite.GetText) + StrToDouble(NachLinks.GetText)) <> TRUE) then
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
                warnlog "measurement value error  "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText))+" should: "+PAGEWIDTH
        end if
    end if
    '--------------------------------
    NachLinks.SetText "0"
    Printlog "'///     Right ///"
    rechts.check
    sleep (1)
    if ((Breite.IsWritable AND NachLinks.IsWritable) <> TRUE) then warnlog "edit field not active "
    if (LiberalMeasurement(PAGEWIDTH ,StrToDouble (Breite.GetText) + StrToDouble(NachLinks.GetText)) <> TRUE) then
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
             warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText))+" should: "+PAGEWIDTH
        end if
    end if
    NachLinks.SetText "2"
    sleep (1)
    temp(1) = NachLinks.GetText
    TabTabelle.OK
    TableTableProperties
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    if (rechts.Ischecked <> TRUE) then warnlog "rechts state changed"
    if ((Breite.IsWritable AND NachLinks.IsWritable) <> TRUE) then warnlog "edit field not active "
    if (temp(1) <> NachLinks.GetText) then warnlog "value changed"
    if (LiberalMeasurement(PAGEWIDTH ,StrToDouble (Breite.GetText) + StrToDouble(NachLinks.GetText)) <> TRUE) then 
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
            warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText))+" should: "+PAGEWIDTH
        end if
    end if    
    '--------------------------------
    NachLinks.SetText "0"
    Printlog "'///      center ///"
    Zentriert.check
    sleep (1)
    if ((Breite.IsWritable AND NachLinks.IsWritable) <> TRUE) then warnlog "edit field not active "
    if (LiberalMeasurement(PAGEWIDTH ,StrToDouble (Breite.GetText) + (2 * StrToDouble (NachLinks.GetText))) <> TRUE) then
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
            warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText))+" should: "+PAGEWIDTH
        end if
    end if    
    NachLinks.SetText "2"
    sleep (1)
    temp(1) = NachLinks.GetText
    TabTabelle.OK
    TableTableProperties
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    if (Zentriert.Ischecked <> TRUE) then warnlog "state changed"
    if ((Breite.IsWritable AND NachLinks.IsWritable) <> TRUE) then warnlog "edit field not active "
    if (temp(1) <> NachLinks.GetText) then warnlog "value changed"
    if (LiberalMeasurement(PAGEWIDTH ,StrToDouble (Breite.GetText) + (2 * StrToDouble (NachLinks.GetText))) <> TRUE) then 
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
            warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText))+" should: "+PAGEWIDTH
        end if
    end if    
    '--------------------------------
    NachLinks.SetText "0"
    Printlog "'///     manual ///"
    Manuell.check
    sleep (1)
    if ((Breite.IsWritable AND NachLinks.IsWritable AND NachRechts.IsWritable) <> TRUE) then warnlog "edit field not active "
    if (LiberalMeasurement(PAGEWIDTH ,StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText) + StrToDouble (NachRechts.GetText)) <> TRUE) then
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
            warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText)+ StrToDouble (NachRechts.GetText))+" should: "+PAGEWIDTH
        end if
    end if
    NachLinks.SetText "2"
    NachRechts.SetText "2"
    sleep (1)
    temp(1) = NachLinks.GetText
    temp(2) = NachRechts.GetText
    TabTabelle.OK
    TableTableProperties
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    if (Manuell.Ischecked <> TRUE) then warnlog "state changed"
    if ((Breite.IsWritable AND NachLinks.IsWritable) <> TRUE) then warnlog "edit field not active "
    if (temp(1) <> NachLinks.GetText) then warnlog "value changed"
    if (temp(2) <> NachRechts.GetText) then warnlog "value changed"
    if (LiberalMeasurement(PAGEWIDTH ,StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText) + StrToDouble (NachRechts.GetText)) <> TRUE) then
        if PAGEWIDTH < 0 then
            warnlog "Cdbl returns wrong values"
        else
             warnlog "measurement value error "+ "is: "+(StrToDouble (Breite.GetText) + StrToDouble (NachLinks.GetText)+ StrToDouble (NachRechts.GetText))+" should: "+PAGEWIDTH
        end if
    end if
    '--------------------------------
    NachLinks.SetText "0"
    NachRechts.SetText "0"
    TabTabelle.OK
    Printlog "'///    <font color=#FF0000>TODO TBO automatical ---------- MISSING yet </FONT> ///"
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase sTablePagebreak
    Dim temp(10) as string

    Call hNewDocument
    Call TBOhTabelleEinfuegen ("dummy" ,0,1,0,1,"1",tHeight:="1")  ' insert table
    Call wTypeKeys "<mod1 end><mod1 end>"
    temp (1) = "Garfield"
    Call TBOhTabelleEinfuegen (temp (1) ,0,1,0,1,"10",tHeight:="7")  ' insert table

    Printlog "'///     Check TextFlow: pagebreak ///"

    InsertFieldsPagenumbers          ' get pagenumber & leave it for next calculation
    Call wTypeKeys "<shift home>"
    ToolsCalculate
    temp(1) = val(GetClipboardText)

    TableTableProperties
    Kontext "TabelleEinfuegenWriter"
    Kontext
    Active.SetPage TabTextFlusstabelle
    Kontext "TabTextFlusstabelle"
    Umbruch.Check
    if ((Seite.IsChecked AND Davor.IsChecked) <> TRUE) then
        printlog "--- defaultvalue changed..."
        Seite.Check
        Davor.Check
    end if
    TabTextFlusstabelle.OK

    ToolsUpdateUpdateAll             ' update field & get pagenumber & clear
    Call wTypeKeys "<shift end>"
    ToolsCalculate
    temp(2) = val(GetClipboardText)
    if ((val(temp(1))+1) <> temp(2)) then
        warnlog "--- Table didn't move, that's not good "+ "from: "+val(temp(1))+" to: "+ temp(2) +" 1+1= "+(val(temp(1))+1)
    else
        printlog "    works , moved, ok!"
    end if

    Call wTypeKeys "<mod1 end><mod1 end><return>"

    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase sTableMergeCell
    dim temp(10) as string

    dim PAGEWIDTH as double
    dim ac as integer, fc as integer
    dim i as integer

    Call hNewDocument

    temp (1) = "Table1"
	printlog "Insert a table with 10 columns and 7 rows"
    Call TBOhTabelleEinfuegen (temp (1) ,0,1,0,1,"10",tHeight:="7")  ' insert table
    printlog "Point cursor out of the table"
    Call wTypeKeys "<MOD1 END>", 2

	printlog "Insert a table with 10 columns and 10 rows"
    temp (1) = "Table2"
    Call TBOhTabelleEinfuegen (temp (1) ,0,0,1,1,"10",tHeight:="10")  ' insert table

    Printlog "merging of cells & undo"
    temp (2) = hGetTableName()
    if (temp(2) <> temp(1)) then warnlog " --- wrong table? schould be : "+temp (1)+ ", is : "+temp(2)

    Printlog " - merging of cells"
    ac=0   ' ActionCount
    fc=100 ' FieldCount
    Call wTypeKeys "<down><shift right>"     ' 2 horizontal (0,1) & (1,1) (x,y)
    TableMergeCells : inc ac : dec fc '   FormatZelleVerbinden
    Call wTypeKeys "<down><shift down>"      ' 2 vertical (1,2) & (1,3)
    TableMergeCells : inc ac : dec fc
    Call wTypeKeys "<down><up><shift down><shift up>"      ' merge again
    TableMergeCells

    printlog "Go to top of table"
    Call wTypeKeys "<mod1 home>"
    printlog "Undo all cell mergers (3 times)"
    for i=1 to ac
        try
    		EditUndo
    		wait 500
    	catch
    		Warnlog "Edit / Undo disabled on Undo step: " & i
    	endcatch
    next i
    
    printlog "Close document"
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase sTableBorder
    dim temp(10) as string
    temp(1) = "bloed"

    dim PAGEWIDTH as double
    dim ac as integer, fc as integer
    dim i as integer
    Call hNewDocument

    ' set marks to find
    Call wTypeKeys "<mod1 end><mod1 end>"
    Call wTypeKeys "NULL<return>"
    Call wTypeKeys "Start<return>"
    Call wTypeKeys "End<return>"
    Call wTypeKeys "NULL<return><up><up>"

    Call TBOhTabelleEinfuegen (temp (1) ,0,1,0,1,"10",tHeight:="7")  ' insert table

    Printlog "'///       set border & undo ///"
    ' assign border to whole table
    Call wTypeKeys "<shift mod1 end>"
    TableTableProperties                ' get into existing table
    Sleep (1)
    Kontext "TabelleEinfuegenWriter"
    sleep (1)
    Kontext
    if active.exists (5) then
        Active.SetPage TabUmrandung
        Sleep (1)
        Kontext "TabUmrandung"
        Sleep (1)
        Vorgaben.Typekeys "<end>" ' set border
        StilFarbe.Select StilFarbe.GetItemCount

        'links.IsWriteable 'si checkable, but not now!...
        Position.TypeKeys "<home><right><right><right>" ' 3. from the left
        if (groesse.IsEnabled <> TRUE) then warnlog "position of shadow is not changeable "
        SchattenFarbe.Select SchattenFarbe.GetItemCount - 1
    else
        TableTableProperties                ' get into existing table
        Kontext "TabelleEinfuegenWriter"
        sleep (1)
        Kontext
        warnlog  "2.trie"
        if active.exists (5) then
            Active.SetPage TabUmrandung
            Sleep (1)
            Kontext "TabUmrandung"
            Sleep (1)
            Vorgaben.Typekeys "<end>" ' set border
            StilFarbe.Select StilFarbe.GetItemCount

            'links.IsWriteable 'si checkable, but not now!...
            Position.TypeKeys "<home><right><right><right>" ' 3. from the left
            if (groesse.IsEnabled <> TRUE) then warnlog "position of shadow is not changeable "
            SchattenFarbe.Select SchattenFarbe.GetItemCount - 1
        else
            warnlog "baeh!"
        end if
    end if
    Kontext
    Active.SetPage TabHintergrund
    Sleep (1)
    Kontext "TabHintergrund"
    Sleep (1)
    Hintergrundfarbe.TypeKeys "<home><down><right><down><right><down><right><down><right>"
    TabHintergrund.OK
    ' just an idea of checking this type of control
    i=0
    TableTableProperties                ' get into existing table
    Sleep (1)
    Kontext "TabelleEinfuegenWriter"
    sleep (1)
    Kontext
    Active.SetPage TabUmrandung
    Sleep (1)
    Kontext "TabUmrandung"
    Sleep (1)
    while (groesse.IsEnabled)
        Position.TypeKeys "<left>"
        inc i
    wend
    TabUmrandung.cancel
    if (i <> 3) then warnlog "wrong shadow selected should be 3, was: "+i

    ' undo & check where we are ///'
    EditUndo
    temp (2) = hGetTableName()
    if (temp(2) <> temp(1)) then warnlog " --- wrong table? schould be : "+temp (1)+ ", is : "+temp(2)
    EditUndo ' go one step further
    Call wTypeKeys "<up><shift end>"
    EditCopy
    temp(2) = GetClipboardText
    Call wTypeKeys "<down><shift home>"
    EditCopy
    temp(3) = GetClipboardText
    if ((temp(2) <> "Start") AND (temp(3) <> "End")) then
        warnlog "there is smth. wrong with the undo stuff !!!"
        print "whats wrong with the undo stuff"
    end if
    EditRedo ' and back again
    Call wTypeKeys "<up><mod1 home>" ' now we are in (0,0)
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase sTableIndent
    dim temp(10) as string
    dim ac as integer, fc as integer

    Call hNewDocument
    ' set marks to find
    Call wTypeKeys "NULL<return>"
    Call wTypeKeys "Start<return>"
    Call wTypeKeys "End<return>"
    Call wTypeKeys "NULL<return><up><up>"

    temp (1) = "Garfield"
    Call TBOhTabelleEinfuegen (temp (1) ,0,1,0,1,"10",tHeight:="7")  ' insert table

    Printlog "'///    tab, paragraph indent & UNDO ///"

    ac=0   ' ActionCount
    fc=100 ' FieldCount
    ' : inc ac : dec fc
    FormatParagraph
    Kontext
    Active.SetPage TabTabulator
    Sleep (1)
    Kontext "TabTabulator"
    Sleep (1)
    Position.SetText "1"
    TypZentriert.Check
    Fuellzeichen5.Check
    FuellzeichenZeichen.SetText "%"
    Neu.Click
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    Vonlinks.SetText "-1"
    VonRechts.SetText "-1"
    TabEinzuegeUndAbstaende.OK


    Printlog "'///     text attributes ///"
    Call wTypeKeys "John"  : inc ac
    Call wTypeKeys "<shift home>"
    FormatCharacter
    Kontext
    Active.SetPage TabFontEffects
    Sleep (1)
    Kontext "TabFontEffects"
    Sleep (1)
    FontColor.Select FontColor.GetItemCount - 1
    Kontext
    Active.SetPage TabFontPosition
    Sleep (1)
    Kontext "TabFontPosition"
    Sleep (1)
    Super.Check
    n90Degree.Check
    TabFontPosition.OK

    EditUndo
    EditUndo

    Printlog "'///      cell protection ///"
    Call wTypeKeys "<LEFT><UP>", 10
    Call wTypeKeys "<DOWN>", 3
    Call wTypeKeys "<down><shift down><shift right><Right><Left><Shift right><Shift left>"
    TableCellProtect
    Call wTypeKeys "<LEFT>"
    Call wTypeKeys "x"
    Kontext
    if (Active.Exists <> TRUE) then
        Call wTypeKeys "<LEFT><UP>", 10
        Call wTypeKeys "<DOWN>", 3
        Call wTypeKeys "<down><shift down><shift right><Right><Left><Shift right><Shift left>"
        try
            EditCopy
        catch
        endcatch
        if GetClipboardtext <> "" then
            QAErrorLog "#110549#Protection of cell does not work correct!"
        end if
        ' If protection fails, reselect table with Navigator!
        Call wNavigatorAuswahl(2,1)
    else
        Active.OK
        EditUndo
        Call wTypeKeys "x"
        Kontext
        if (Active.Exists = TRUE) then
            Warnlog "ReProtection of table does not work!" + Active.GetText
            active.OK
        end if

        EditUndo
        EditUndo
        'Printlog "undoCheck is here"
        temp (2) = hGetTableName()
        if (temp(2) <> temp(1)) then warnlog " --- wrong table? schould be : "+temp (1)+ ", is : "+temp(2)
        EditUndo ' go one step further
        Call wTypeKeys "<up><shift end>"
        EditCopy
        temp(2) = GetClipboardText
        Call wTypeKeys "<down><shift home>"
        EditCopy
        temp(3) = GetClipboardText
        if ((temp(2) <> "Start") AND (temp(3) <> "End")) then
            warnlog "there is smth. wrong with the undo stuff !!! is: "+temp(2)+" should: Start; is: "+temp(3)+" should: End;"
        end if
        EditRedo ' and back again
        Call wTypeKeys "<up><mod1 home>" ' now we are in (0,0)
    end if

    Printlog "'///      Set graphic in table as background ///"
    try
        TableTableProperties
    catch
        Warnlog "Dialog 'Insert Table' not up!"
        Call hCloseDocument
        goto endsub
    endcatch

    try
        Kontext
        Active.SetPage TabHintergrund
        Sleep (1)
        Kontext "TabHintergrund"
        Sleep (1)
        Fuer.Select(2) ' Zeile
        Als.Select(2)  ' Grafik
        Durchsuchen.Click
        Kontext "GrafikEinfuegenDlg"
        Dateiname.SetText convertPath(gTesttoolPath +"writer\optional\input\options\ga000907.gif")
        Oeffnen.Click
        Kontext "TabHintergrund"
        TabHintergrund.OK

        FormatColumnWidthWriter ' thre is a bigger test in the option test
        Kontext "SpaltenBreite"
        Spalte.SetText "2"
        Breite.SetText "2"
        SpaltenBreite.OK
        FormatRowHeight
        Kontext "ZellenHoehe"
        Hoehe.SetText "2"
        ZellenHoehe.OK
        EditUndo
        EditUndo

        EditUndo

        ' Printlog "undoCheck is here"
        sleep (3)
        temp (2) = hGetTableName()
        if (temp(2) <> temp(1)) then warnlog " --- wrong table? schould be : "+temp (1)+ ", is : "+temp(2)
        EditUndo ' go one step further
        Call wTypeKeys "<up><shift end>"
        EditCopy
        temp(2) = GetClipboardText
        Call wTypeKeys "<down><shift home>"
        EditCopy
        temp(3) = GetClipboardText
        if ((temp(2) <> "Start") AND (temp(3) <> "End")) then
            warnlog "there is smth. wrong with the undo stuff !!! is: "+temp(2)+" should: Start; is: "+temp(3)+" should: End;"
        end if
        EditRedo ' and back again
        Call wTypeKeys "<mod1 end>" ' go to end of doc
    catch
        Warnlog "Dialog 'Insert Table' not up!"
    endcatch
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase sTableSort
    dim temp(10) as string
    dim PAGEWIDTH as double
    dim ac as integer, fc as integer
    dim i as integer

    Call hNewDocument
    '   Kontext "DocumentWriter"
    Call TBOhTabelleEinfuegen ("dummy" ,0,1,0,1,"1",tHeight:="1")  ' insert table
    Call wTypeKeys "<mod1 end><mod1 end>"
    temp (1) = "Sort"
    Call TBOhTabelleEinfuegen (temp (1) ,0,1,0,1,"10",tHeight:="10")  ' insert table

    fc = 99
    randomize()
    for i=1 to fc
        Call wTypeKeys str(rnd()) + "<right>"
    next i
    Call wTypeKeys "<shift mod1 home>"
    ToolsSort '-----------------------------------------------------------11111111111111111
    Kontext "Sortieren"
    Spalten.Check  'direction 1
    for i = 1 to Schluesseltyp1.GetItemCount
        Schluesseltyp1.select i
        printlog "possible values for sorting :" + Schluesseltyp1.Getseltext
    next i
    Sortieren.OK
    Kontext
    if active.exists (5) then printlog "active1: "+active.gettext
    Call wTypeKeys "<shift mod1 end>",2
    try
        ToolsSort '-----------------------------------------------------------22222222222222222
    catch
        printlog "WorkAround only seen on linux, can't select immediately, cursor movement needed "
        Call wTypeKeys "<right><left>"
        Call wTypeKeys "<shift mod1 end>",2
        ToolsSort
    endcatch
    Kontext
    if active.exists (5) then
        printlog "active2: "+active.gettext
        active.ok
    end if
    Kontext "Sortieren"
    Zeilen.Check
    Sortieren.OK
    Kontext
    if active.exists (5) then
        printlog "active3: "+active.gettext
        active.ok
    end if

    'Schluessel1
    'Spalte1
    'Schluesseltyp1
    'Aufsteigend1
    'Absteigend1
    '
    'Schluessel2
    'Spalte2
    'Schluesseltyp2
    'Aufsteigend2
    'Absteigend2
    '
    'Schluessel3
    'Spalte3
    'Schluesseltyp3
    'Aufsteigend3
    'Absteigend3
    '
    'Tabulator
    'Zeichen
    'Zeichentext
    'Sonderzeichen
    'Sprache
    'ExakterVergleich
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase TableSplit_copy_heading
    printlog "'/// Splitting Tables & UNDO ///"
    printlog "'///     check 'copy heading' ///"
    dim temp(10) as string
    dim PAGEWIDTH as double
    dim ac as integer, fc as integer
    dim i as integer

    Call hNewDocument
    ' Kontext "DocumentWriter"
    Call TBOhTabelleEinfuegen ("dummy" ,0,1,0,1,"1",tHeight:="1")  ' insert table
    Call wTypeKeys "<mod1 end><mod1 end>"
    temp (1) = "Split"
    Call TBOhTabelleEinfuegen (temp (1) ,1,1,0,1,"10",tHeight:="10")  ' insert table

    Printlog "'/// Splitting Tables & UNDO ///"
    Printlog "'///     check 'copy heading' ///"
    temp(2) = "Jon"
    Call wTypeKeys temp(2)+"<down>"
    FormatSplitTable
    Kontext "TabelleAuftrennen"
    UeberschriftKopieren.Check      '<------------- this is the key
    TabelleAuftrennen.OK
    ' check where am i
    temp (6) = hGetTableName ()
    if (temp(6) <> (fLocaleString("LocaleTable")+"1")) then warnlog "Split table didn't work! is: "+temp(6)+"; should: "+(fLocaleString("LocaleTable")+"1")
'   if (temp(6) <> (fLocaleString("LocaleTable")+"2")) then warnlog "Split table didn't work! is: "+temp(6)+"; should: "+(fLocaleString("LocaleTable")+"2")

    Call wTypeKeys "<up>"  ' im in the content of the table
    Sleep 1
    ' -------------check if heading (style)
    Kontext "TextObjectbar"
    if (Vorlage.GetSelText <> fLocaleString("LocaleTableHeading")) then
        Warnlog "tabel heading did not work!is: " & Vorlage.GetSelText & ", should " & fLocaleString("LocaleTableHeading")
    end if
    '----------------------------------
    Call wTypeKeys "<end><shift home>" ' check heading in 'new' table (content)
    EditCopy
    temp(3) = GetClipboardText
    if (temp(2) <> temp(3)) then  warnlog "wrong heading! Is: "+temp(3)+" Should: "+temp(2)
    Call wTypeKeys "<up><up>"
    if (hGetTableName () <> (temp(1))) then warnlog "Split table didn't work?"
    Call wTypeKeys "<end><shift home>" ' check heading in 'old' table
    EditCopy
    temp(3) = GetClipboardText
    if (temp(2) <> temp(3)) then  warnlog "wrong heading! is: "+temp(3)+" should: "+temp(2)
    EditUndo
    EditUndo

    Printlog "'///     check 'custom heading (apply style)' ///"
    ' splitting headers doesn't work -> move down 1 line
    temp(2) = temp(1)
    Call wTypeKeys temp(2)
    ' set some style to verify
    FormatParagraph
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    Vonlinks.SetText "-1"
    VonRechts.SetText "-1"
    temp (4) = Vonlinks.GetText
    temp (5) = Vonrechts.GetText
    TabEinzuegeUndAbstaende.OK
    Call wTypeKeys "<down>",2
    FormatSplitTable
    Kontext "TabelleAuftrennen"
    EigeneMitVorlage.Check    '<------------- this is the key
    TabelleAuftrennen.OK
    ' check where am i
    temp (6) = hGetTableName ()
    if (temp(6) <> (fLocaleString("LocaleTable")+"1")) then
        warnlog "Split table didn't work! is: "+temp(6)+"; should: "+(fLocaleString("LocaleTable")+"1")
    end if
    ' -------------check if heading
    Kontext "TextObjectbar"
    if (Vorlage.GetSelText <> fLocaleString("LocaleTableHeading")) then
        Warnlog "Table heading did not work!"
        Warnlog "is: " & Vorlage.GetSelText & ", should " & fLocaleString("LocaleTableHeading")
        Call wTypeKeys "<Escape>"
    end if
    '----------------------------------

    FormatParagraph         ' check style
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    '         if ((Vonlinks.GetText <>temp (4) ) OR (Vonrechts.GetText <>temp (5) )) then warnlog "style didn't get inherited. is: "+Vonlinks.GetText+"; should: "+temp (4) +";; is: "+Vonrechts.GetText+"; should: "+temp (5) +";;"
    TabEinzuegeUndAbstaende.OK
    Call wTypeKeys "<up><up><up>"               ' goto old table
    if (hGetTableName () <> (temp(1))) then warnlog "Split table didn't work? "
    Call wTypeKeys "<end><shift home>" ' check heading in 'old' table
    EditCopy
    temp(3) = GetClipboardText
    if (temp(2) <> temp(3)) then  warnlog "wrong heading! is: "+temp(3)+"; should: "+temp(2)

    sleep (3)
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase TableSplit_custom_heading
    dim temp(10) as string
    dim PAGEWIDTH as double
    dim ac as integer, fc as integer
    dim i as integer

    Call hNewDocument
    Call TBOhTabelleEinfuegen ("dummy" ,0,1,0,1,"1",tHeight:="1")  ' insert table
    Call wTypeKeys "<mod1 end><mod1 end>"
    temp (1) = "Header"
    Call TBOhTabelleEinfuegen (temp (1) ,1,1,0,1,"10",tHeight:="10")  ' insert table
    Printlog "'///     check  'custom heading' ///"
    Call wTypeKeys "Header"
    FormatStylesCatalog ' set a style to verify
    Kontext "Vorlagenkatalog"
    Ansicht.TypeKeys "<up><up><up><up>"
    temp (4) = Ansicht.GetText
    Vorlagenkatalog.OK
    ' set some style to verify
    FormatParagraph
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    Vonlinks.SetText "-1"
    VonRechts.SetText "-1"
    temp (2) = Vonlinks.GetText
    temp (3) = Vonrechts.GetText
    TabEinzuegeUndAbstaende.OK
    Call wTypeKeys ("<down>")
    FormatSplitTable
    Kontext "TabelleAuftrennen"
    Eigene.Check                 '<------------- this is the key
    TabelleAuftrennen.OK
    ' check where am i
    temp (5) = hGetTableName ()
    if (temp(5) <> ( fLocaleString("LocaleTable") + "1" )) then warnlog "Split table didn't work! is: "+ temp(5) + "; should: "+(fLocaleString("LocaleTable") + "1")
    ' -------------check if NO heading (would be a custom style...))
    Call wTypeKeys "<Up>",2
    Call wTypeKeys "<mod1 a>"
    EditCopy
    temp(6) = GetClipboardText
    FormatParagraph         ' check style
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    if temp (2) <> Vonlinks.GetText then Warnlog "Indent 'Before text' has been changed!"
    if temp (3) <> Vonrechts.GetText then Warnlog "Indent 'After text' has been changed!"
    TabEinzuegeUndAbstaende.OK
    FormatStylesCatalog ' verify
    Kontext "Vorlagenkatalog"
    if Ansicht.Gettext <> temp(4) then Warnlog "Style in Header has been changed!"
    Vorlagenkatalog.Cancel
    if (hGetTableName () <> temp(1)) then warnlog "Split table didn't work? is: "+hGetTableName () +"; should: "+temp(1)
    if (temp(6) <> temp(1)) then  warnlog "wrong heading! is: " + temp(6) + " should: " + temp(1)
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase TableSplit_custom_heading_apply_style
    ' SHOULD BE EASIER TO WORK ON ODIE TOOOOOO
    dim temp(10) as string
    dim PAGEWIDTH as double
    dim ac as integer, fc as integer
    dim i as integer

    Call hNewDocument
    Call TBOhTabelleEinfuegen ("dummy" ,0,1,0,1,"1",tHeight:="1")  ' insert table
    Call wTypeKeys "<mod1 end><mod1 end>"
    temp (1) = "Split"
    TBOhTabelleEinfuegen (temp (1) ,1,1,0,1,"10",tHeight:="10")  ' insert table

    Printlog "'/// Splitting Tables & UNDO ///"
    Printlog "'///     check 'custom heading (apply style)' ///"
    temp(2) = temp(1)
    Call wTypeKeys temp(2)
    ' set some style to verify
    FormatParagraph
    Kontext "TabTabulator"
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    Vonlinks.SetText "-1"
    VonRechts.SetText "-1"
    temp (4) = Vonlinks.GetText
    temp (5) = Vonrechts.GetText
    TabEinzuegeUndAbstaende.OK
    Call wTypeKeys "<down>",2
    FormatSplitTable
    Kontext "TabelleAuftrennen"
    EigeneMitVorlage.Check    '<------------- this is the key
    TabelleAuftrennen.OK
    ' check where am i
    temp (6) = hGetTableName ()
    if (temp(6) <> (fLocaleString("LocaleTable")+"1")) then
        warnlog "Split table didn't work! is: "+temp(6)+"; should: "+(fLocaleString("LocaleTable")+"1")
    end if
    ' -------------check if heading
    Kontext "TextObjectbar"
    if (Vorlage.GetSelText <> fLocaleString("LocaleTableHeading")) then
        Warnlog "table heading did not work!"
        warnlog "is: " & Vorlage.GetSelText & ", should " & fLocaleString("LocaleTableHeading")
        'GetSelText bleibt in der box
        Call wTypeKeys "<Escape>"
    end if
    '----------------------------------
    FormatParagraph         ' check style
    Kontext "TabTabulator"
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    TabEinzuegeUndAbstaende.OK
    Call wTypeKeys "<up><up><up>"               ' goto old table
    if (hGetTableName () <> (temp(1))) then warnlog "Split table didn't work? "
    Call wTypeKeys "<end><shift home>" ' check heading in 'old' table
    EditCopy
    temp(3) = GetClipboardText
    if (temp(2) <> temp(3)) then  warnlog "wrong heading! is: "+temp(3)+"; should: "+temp(2)
    sleep (3)
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase TableSplit_no_heading
    ' SHOULD BE EASIER TO WORK ON ODIE TOOOOOO
    dim temp(10) as string
    dim PAGEWIDTH as double
    dim ac as integer, fc as integer
    dim i as integer

    Call hNewDocument
    Call TBOhTabelleEinfuegen ("dummy" ,0,1,0,1,"1",tHeight:="1")  ' insert table
    Call wTypeKeys "<mod1 end><mod1 end>"
    temp (1) = "Split"
    Call TBOhTabelleEinfuegen (temp (1) ,1,1,0,1,"10",tHeight:="10")  ' insert table

    Printlog "'/// Splitting Tables & UNDO ///"
    Printlog "'///     check 'no heading' ///"
    temp(2) = "Header"
    temp(3) = "Content"
    Call wTypeKeys ( temp(2) )
    Call wTypeKeys ( "<down>" )
    Call wTypeKeys ( temp(3) )
    Call wTypeKeys ( "<Home>" )
    ' set some style to verify
    FormatParagraph
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    Vonlinks.SetText "-1"
    VonRechts.SetText "-1"
    temp (4) = Vonlinks.GetText
    temp (5) = Vonrechts.GetText
    TabEinzuegeUndAbstaende.OK
    FormatSplitTable
    Kontext "TabelleAuftrennen"
    Keine.Check
    TabelleAuftrennen.OK
    ' check where am i
    temp (6) = hGetTableName ()
    if (temp(6) <> ( fLocaleString("LocaleTable") + "1" )) then warnlog "Split table didn't work! is: " + temp(6) + "; should: " + (fLocaleString("LocaleTable") + "1" )
    ' -------------check if NO heading (would be a custom style...))
    Kontext "TextObjectbar"
    Call wTypeKeys "<end><shift home>" ' check heading in 'new' table
    try
        EditCopy
        ' Should be 'Content'
        if (GetClipboardText   <> temp(3)) then  warnlog "wrong content! is: " + GetClipboardText + " should: " + temp(3)
    catch
        Warnlog "Unable to copy content! Maybe not selected!"
    endcatch
    FormatParagraph         ' check style
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Sleep (1)
    Kontext "TabEinzuegeUndAbstaende"
    Sleep (1)
    if ((Vonlinks.GetText <>temp (4) ) OR (Vonrechts.GetText <>temp (5) )) then warnlog "style didn't get inherited"
    TabEinzuegeUndAbstaende.OK
    Call wTypeKeys "<up><up>"               ' goto header
    if (hGetTableName () <> (temp(1))) then warnlog "Split table didn't work? "
    Call wTypeKeys "<Mod1 A>" ' check heading in 'old' table
    try
        EditCopy
    catch
        Warnlog "Unable to copy content! Maybe not selected!"
    endcatch
    if (GetClipboardText  <> temp(2)) then  warnlog "wrong heading! is: " + GetClipboardText + " should: " + temp(2)
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

testcase sTableMerge
    dim temp(10) as string

    Call hNewDocument
    Call wTypeKeys "<return><mod1 end><mod1 end>"
    Call wTypeKeys "<mod1 end><mod1 end>"

    temp(2) = "Arlene"
    temp(3) = "Pooky"
    temp(4) = "Nermal"

    Call TBOhTabelleEinfuegen (temp (2) ,0,1,0,1,"10",tHeight:="7")  ' insert table Arlene
    sleep (3)
    Call wTypeKeys "<mod1 end><mod1 end>"
    Call TBOhTabelleEinfuegen (temp (3) ,0,1,0,1,"10",tHeight:="7")  ' insert table Pooky
    sleep (3)
    Call wTypeKeys "<mod1 end><mod1 end>"
    Call TBOhTabelleEinfuegen (temp (4) ,0,1,0,1,"10",tHeight:="7")  ' insert table Nermal
    sleep (3)

    Printlog "'///  Join tables & UNDO///"
    Printlog "'///      insert 3 tables: 'Arlene' 'Pooky' 'Nermal' ///"

    ' usually lowwer to upper one...
    ' update formulars ....
    Printlog "'///     join Nermal with pooky -> Nermal has to be under Arlene -> undo ///"
    FormatMergeTables
    if (hGetTableName () <> temp(4)) then warnlog "Merge table didn't work? 1 "
    Call wTypeKeys "<mod1 home><up>"
    if (hGetTableName () <> temp(2)) then warnlog "Merge table didn't work? 1+ "
    EditUndo
    Call wTypeKeys "<up><mod1 home>"

    Printlog "'///     join Nermal with pooky -> choose lowerTable -> Nermal has to be under Arlene again -> undo  ///"
    FormatMergeTables
    Kontext "TabellenVerbinden"
    if TabellenVerbinden.exists (5) <> TRUE then
        warnlog "there is only one table around no question for which table should be connected "
    end if
    MitNachfolgenderTabelleVerbinden.Check
    TabellenVerbinden.OK
    temp (5) = hGetTableName ()
    if (temp (5) <> temp(3)) then warnlog "Merge table didn't work? 2 is:"+temp (5)+" should: "+temp(3)
    Call wTypeKeys "<up>"
    if (hGetTableName () <> temp(2)) then warnlog "Merge table didn't work? 2+ "
    EditUndo
    Call wTypeKeys "<up><mod1 home>"

    Printlog "'///     join Pooky with Arlene -> choose upperTable -> Pooky is now above Nermal -> undo  ///"
    FormatMergeTables
    Kontext "TabellenVerbinden"
    MitVorherigerTabelleVerbinden.Check
    TabellenVerbinden.OK
    temp (5) = hGetTableName ()
    if (temp (5) <> temp(3)) then warnlog "Merge table didn't work? 3 is:"+temp (5)+" should: "+temp(3)
    Call wTypeKeys "<up>"
    if (hGetTableName () <> temp(3)) then warnlog "Merge table didn't work? 3+ "
    EditUndo
    Call wTypeKeys "<up><mod1 home>"

    Printlog "'///     join Arlene with Pooky -> Pooky is above Nermal -> undo ///"
    FormatMergeTables
    Kontext "TabellenVerbinden"
    if TabellenVerbinden.exists then
        warnlog "tzhere shouldn't be a table connect dialog; yust one table to connect to available "
        TabellenVerbinden.cancel
    end if
    sleep (2)
    temp (5) = hGetTableName ()
    if (temp (5) <> temp(2)) then warnlog "Merge table didn't work? 4 is:"+temp (5)+" should: "+temp(2)
    sleep (2)
    Call wTypeKeys "<mod1 end><down>"
    sleep (3)
    if (hGetTableName () <> temp(4)) then warnlog "Merge table didn't work? 4+ "
    sleep (3)
    EditUndo
    sleep (3)
    Call hCloseDocument
endcase

' ----------------------------------------------------------------------

function hGetPageWidth() as double
    Dim tWidth as double
    try
        FormatPageWriter
    catch
        warnlog ":couldn't get menuentry: FormatPageWriter"
        exit function
    endcatch
    Kontext
    Active.SetPage TabSeite
    Kontext "TabSeite"
    printlog "Test: "+ Breite.GetText +" " +Links.GetText  +" " +Rechts.GetText
	tWidth = (StrToDouble(Breite.GetText) - ( StrToDouble(Links.GetText) + StrToDouble(Rechts.GetText)) )
    hGetPageWidth = tWidth
    TabSeite.cancel
end function

