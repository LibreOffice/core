'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_204_.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-26 12:40:56 $
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
'* short description : Writer Table Test
'*
'*    Call tTableOption
'*    Call tTableInsert1
'*    Call tTableInsert2
'*    Call tTableInsert3
'*    Call tTableInsert4
'*    Call tTableInsert5
'*    Call tTableNumberFormat
'*    Call tTableDelete
'*    Call tTableText
'*    Call tTableCalculate
'*    Call tTableEdit
'*
'\***********************************************************************

testcase tTableOption
   '/// Check Tables' default Option ///
    printlog "/// Check Tables' default Option ///"

    Call hNewDocument

    '/// TOOLS->OPTIONS->StarOffice Writer->Table ///"
    '/// Number recognition should be OFF by default///"
    '/// two boxes below are checked but disabled ///"
    
    ToolsOptions
    Call hToolsOptions ("TextDocument","Table")
    if AutomatischeZahlenerkennung.IsChecked then 
        warnlog "The default for 'Number recognition' should be OFF => 114166 "
        AutomatischeZahlenerkennung.UnCheck
    else
        if AutomatischeAusrichtung.IsChecked then
            try
                AutomatischeAusrichtung.UnCheck
                warnlog "'Number format recognition' should be disabled => 114166 "
            catch
            endcatch
        else 
            warnlog "'Number format recognition' should be checked => 114166 "    
        end if        
        if Zahlenformaterkennung.IsChecked then   
            try
                Zahlenformaterkennung.UnCheck
                warnlog "'Alignment' should be disabled => 114166 "
            catch                 
            endcatch
        else
            warnlog "'Alignment' should be checked => 114166 "
        end if    
    end if        
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    Call hCloseDocument

endcase

' --------------------------------------------------------------------

testcase tTableInsert1
    dim i as integer

    Call hNewDocument
    Kontext "DocumentWriter"

    Printlog "'///  1. Insert Toolbox -> Insert Table ///"
    Call hToolbarSelect ( "Insert", True )
    Kontext "InsertBar"
    Tabelle.Click
    Kontext "TabelleEinfuegenWriter"
    TabellenName.SetText "one"
	Spalten.Settext "5"
    TabelleEinfuegenWriter.OK
    Call wDokSchreiben "one<down>"

    Printlog "'///+ ... play with number recognition in TOOLS->OPTIONS->TextDoc->Table ///"
    Printlog "'///+     ... Number recognition OFF ///"
    ToolsOptions
    Call hToolsOptions ("TextDocument","Table")
    AutomatischeZahlenerkennung.UnCheck
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    ' insert test row
    Call hToolbarSelect ( "Table", true )
	ZeileEinfuegen.Click
    Call wDokSchreiben "<down>11111<Tab>1"+cSep+"116111<Tab>1"+cDecSep+"116111<Tab>abcd<shift tab><shift tab><shift tab>"
    ' check format
    for i=1 to 4
        FormatNumberFormat
        Kontext "Zahlenformat"
        if (Kategorie.GetSelText <> fLocaleString("LocaleText")) then
            warnlog "wrong format; is: "+Kategorie.GetSelText+" should: "+fLocaleString("LocaleText") ' Text
        end if
        Zahlenformat.Cancel
        FormatParagraph
        Kontext
        Active.SetPage tabAusrichtungAbsatz
        Kontext "tabAusrichtungAbsatz"
        if (links.isChecked <> true) then warnlog "wrong alignment of tablefield: "+i
        TabAusrichtungAbsatz.Cancel
        Call wDokSchreiben "<Tab>"
    next i
    ' go back to start of table
    Call wDokSchreiben "<Mod1 Home><Down>"

    Printlog "'///+       ... Number recognition ON -                           ///"
    ToolsOptions
    Call hToolsOptions ("TextDocument","Table")
    AutomatischeZahlenerkennung.Check ' even with that, everything is a TEXT :-(????
    Zahlenformaterkennung.UnCheck ' much better with that! everything with numbers&decimalsep is number with 2 decimal places displayed, but all kept!!!
    AutomatischeAusrichtung.UnCheck ' just the allignment does change too!
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

	Call hToolbarSelect ( "Table", true )
    ZeileEinfuegen.Click
    Call wDokSchreiben "<down>11111<Tab>1"+cSep+"116111<Tab>1"+cDecSep+"116111<Tab>abcd<shift tab><shift tab><shift tab>"
    for i=1 to 4
        FormatNumberFormat
        Kontext "Zahlenformat"
        if (Kategorie.GetSelText <> fLocaleString("LocaleText")) then
            warnlog "wrong format; is: "+Kategorie.GetSelText+" should: "+fLocaleString("LocaleText") ' Text
        end if
        Zahlenformat.Cancel
        FormatParagraph
        Kontext
        Active.SetPage tabAusrichtungAbsatz
        Kontext "TabAusrichtungAbsatz"
        if (Links.IsChecked = false) then warnlog "wrong alignment of tablefield: "+i
        TabAusrichtungAbsatz.Cancel
        Call wDokSchreiben "<Tab>"
    next i
    Call wDokSchreiben "<Mod1 Home><Down>"

    Printlog "'///+       ... Number recognition ON - Number format recognition ///"
    ToolsOptions
    Call hToolsOptions ("TextDocument","Table")
    AutomatischeZahlenerkennung.Check ' even with that, everything is a TEXT :-(????
    Zahlenformaterkennung.Check ' much better with that! everything with numbers&decimalsep is number with 2 decimal places displayed, but all kept!!!
    AutomatischeAusrichtung.UnCheck ' just the allignment does change too!
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
	Call hToolbarSelect ( "Table", true )
    ZeileEinfuegen.Click
    Call wDokSchreiben "<down>11111<Tab>1"+cSep+"116111<Tab>1"+cDecSep+"116111<Tab>abcd<shift tab><shift tab><shift tab>"
    ' number, text, number, text
    for i=1 to 4
        FormatNumberFormat
        Kontext "Zahlenformat"
        Select Case i
            Case 1,2,3
                if (Kategorie.GetSelText <> fLocaleString("LocaleNumber")) then
                    warnlog "wrong format; "+ i +"; is: "+Kategorie.GetSelText+" should: "+fLocaleString("LocaleNumber") ' Number
                end if
            Case 4
                if (Kategorie.GetSelText <> fLocaleString("LocaleText")) then
                    warnlog "wrong format; "+ i +"; is: "+Kategorie.GetSelText+" should: "+fLocaleString("LocaleText") ' Text
                end if
        end select
        Zahlenformat.Cancel
        Call wDokSchreiben "<Tab>"
    next i
    Call wDokSchreiben "<Mod1 Home><Down>"

    Printlog "'///+       ... Number recognition ON - Number format recognition - Alignment ///"
    ToolsOptions
    Call hToolsOptions ("TextDocument","Table")
    AutomatischeZahlenerkennung.Check ' even with that, everything is a TEXT :-(????
    Zahlenformaterkennung.Check ' much better with that! everything with numbers&decimalsep is number with 2 decimal places displayed, but all kept!!!
    AutomatischeAusrichtung.Check ' just the allignment does change too!
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
	Call hToolbarSelect ( "Table", true )
    ZeileEinfuegen.Click
    Call wDokSchreiben "<down>11111<Tab>1"+cSep+"116111<Tab>1"+cDecSep+"116111<Tab>abcd<shift tab><shift tab><shift tab>"
    ' number-right, text-left, number-right, text-left
    for i=1 to 4
        Select Case i
            Case 1,2,3
                FormatNumberFormat
                Kontext "Zahlenformat"
                if (Kategorie.GetSelText <> fLocaleString("LocaleNumber")) then
                    warnlog "wrong format; "+ i +"; is: "+Kategorie.GetSelText+" should: "+fLocaleString("LocaleNumber") ' number
                end if
                Zahlenformat.Cancel
                formatparagraph
                Kontext
                Active.SetPage tabAusrichtungAbsatz
                Kontext "tabAusrichtungAbsatz"
                if (rechts.isChecked <> true) then warnlog "wrong alignment of tablefield: "+i +"; should: right"
                tabausrichtungabsatz.Cancel
            Case 4
                FormatNumberFormat
                Kontext "Zahlenformat"
                if (Kategorie.GetSelText <> fLocaleString("LocaleText")) then
                    warnlog "wrong format; is: "+ i +"; "+Kategorie.GetSelText+" should: "+fLocaleString("LocaleText") ' Text
                end if
                Zahlenformat.Cancel
                formatparagraph
                Kontext
                Active.SetPage TabAusrichtungAbsatz
                Kontext "TabAusrichtungAbsatz"
                if (links.isChecked <> true) then warnlog "wrong alignment of tablefield: "+i+"; should: left"
                TabAusrichtungAbsatz.Cancel
         end select
         Call wDokSchreiben "<Tab>"
     next i

     Call hCloseDocument

endcase

' --------------------------------------------------------------------

testcase tTableInsert2

    Call hNewDocument
    Printlog "'///  2. Insert Toolbox -> Insert Table -> Autoformat ///"
    ' using autoformat
    Call hToolbarSelect ( "Insert", True )
    Kontext "InsertBar"
    Tabelle.Click
    Kontext "TabelleEinfuegenWriter"
    Spalten.Settext "5"
	Zeilen.SetText "20"
    Autoformat.Click
    Kontext "AutoformatTabelle"
    AutoFormatTabelle.OK
    Kontext "TabelleEinfuegenWriter"
    TabellenName.SetText "two"
    TabelleEinfuegenWriter.OK
    Call wDokSchreiben "two"

    Call hCloseDocument

endcase

' --------------------------------------------------------------------
testcase tTableInsert3
    dim i as integer
    dim sTemp as string
    dim itemp(3) as integer

    Call hNewDocument
    Printlog "'///  3. Menu: Insert -> Table (Shortcut): {strg} + {F12} ///"
    Printlog "'///+ Test Options on 'Insert Table'                 ///"
    Printlog "'///+ Header ON                                 ///"
    ' table heading active
    ' Don't split table INactive
    ' table border unchecked
    Call wDokSchreiben "<Mod1 F12>"
    Kontext "TabelleEinfuegenWriter"

    TabellenName.SetText "three"
	Spalten.Settext "5"
    Ueberschrift.Check
    Umrandung.Uncheck
    TabelleEinfuegenWriter.OK
    Call wDokSchreiben "three"

    ' -------------check if heading 1
    ' if checked: heading; else: contents !! SystemLanguage dependent !! :-(
    Printlog "'///+ check if Header ON ///"
    Kontext "TextObjectbar"
    sleep (1)
    sTemp = Vorlage.GetSelText
    if (sTemp <> fLocaleString("LocaleTableHeading")) then
        Warnlog "--- table heading did not work!"
        warnlog "--- is:" + sTemp + ", should :" + fLocaleString("LocaleTableHeading")
    end if
    '----------------------------------
    Call wDokSchreiben "<down>"
      ' -------------check if splitting; it HAS TO split
    Printlog "'///+ check if Table gets splitted -  it HAS TO split ///"

    InsertFieldsPagenumbers          ' get pagenumber & leave it for next calculation
    Call wDokSchreiben "<Mod1 A>"
    ToolsCalculate
    itemp(1) = val(GetClipboardText)
	Call hToolbarSelect ( "Table", true )
    for i=1 to 60
        ZeileEinfuegen.Click ' inserted below the current row
        wait 200
    next i
    ToolsUpdateUpdateAll             ' update field & get pagenumber & clear
    Call wDokSchreiben "<Left><Right>" ' Seems there are focus problems on Solaris...workaround
    Call wDokSchreiben "<Mod1 A>"
    ToolsCalculate
    itemp(2) = val(GetClipboardText)
    if (itemp(1)+1 = itemp(2)) then warnlog "--- Table moved to seconde page"
    EditCut
    Call wDokSchreiben "got start3"

    Call wDokSchreiben "<Mod1 end>"       ' move to last cell in table
    InsertFieldsPagenumbers          ' get pagenumber & clear
    Call wDokSchreiben "<Mod1 A>"
    ToolsCalculate
    itemp(3) = val(GetClipboardText)
    if (itemp(2) <> itemp(3)) then
        printlog "--- Table splitted" + itemp(3)
    else
        QAErrorlog "put some more rows in!!!"
    end if

    Call hCloseDocument 	
endcase

' --------------------------------------------------------------------
testcase tTableInsert4
    dim i as integer
    dim stemp as string

    '/// Open a new writer document
    Call hNewDocument
    '/// Insert some dummy text
    if wBlindTextEinfuegen() = false then
        Call hCloseDocument
        goto endsub
    end if
    '/// Insert a table with parameters:
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"

    '/// - Set table name: four
    TabellenName.SetText "four"
    '/// - number of columns: 5
	Spalten.Settext "5"
    '/// - number of rows: 16
    Zeilen.Settext "16"
    '/// - Header: uncheck
    Ueberschrift.UnCheck
    '/// - Don't split table: check
    TabelleNichtTrennen.Check
    '/// - Border: check
    Umrandung.check
    TabelleEinfuegenWriter.OK

    '/// check if table has no header
    Printlog "check if Header OFF"
    Kontext "TextObjectbar"
    sleep (1)
    if wStyleGet(fLocaleString("LocaleTableHeading")) = true then
        Warnlog "Seems tableheader is activated though header is off"
    end if

    '/// Cursor leaves table with <Up> and <End>
    Printlog "check if Table gets splitted"
    Call wDokSchreiben "<Up><End><SPACE>"
    '/// Insert dummy text again 3 times
    Call wBlindtextEinfuegen
'    wait 500
    Call wBlindtextEinfuegen
'    wait 500
    Call wBlindtextEinfuegen
    '/// check if table has been moved to Page 2

    Call wDokSchreiben "<Mod1 End>"     ' jump to documents end
    Call wDokschreiben "<Up>"           ' move cursor into table
    Call wDokSchreiben "<Mod1 Home>"    ' move cursor to first cell

    '/// Insert / Fields / Pagenumbers to check on which page table is
    InsertFieldsPagenumbers
    Call wDokSchreiben "<shift home>"
    ToolsCalculate

    if (GetClipboardText = "2") or (GetClipboardText = "3") then
	        printlog "Table has been moved correctly"
        else
            warnlog " It seems table has been splitted"
    end if

	'/// <B>Table should split if table is bigger than one page</B>
    Call wDokSchreiben "<Up>"
    Call wDokSchreiben "<shift mod1 home>"
    Call wDokSchreiben "<delete>"
    Call wBlindtextEinfuegen
    Call wDokSchreiben "<mod1 End>"
    Call wDokSchreiben "<up>"
    Call wDokSchreiben "<mod1 Home>"

    Printlog "Table should split"
	Call hToolbarSelect ( "Table", true )
    for i=1 to 25
        ZeileEinfuegen.Click ' inserted below the current row
    next i
    Call wDokSchreiben "<shift end>"
    ToolsCalculate
	stemp = GetClipboardText
	Call wDokSchreiben "<ESCAPE>"
	Call wDokSchreiben "<mod1 End>"
	InsertFieldsPagenumbers
	Call wDokSchreiben "<shift home>"
	ToolsCalculate
    if (GetClipboardText <> stemp) then
	        printlog "Table has been splitted"
        else
            warnlog " It seems table has not been splitted"
    end if

    Call hCloseDocument
endcase

' --------------------------------------------------------------------
testcase tTableInsert5

    Call hNewDocument
    '///  Insert Text with delimeter & call Tools -> Text<->Table
    Printlog "'///  Insert Text with delimeter & call Tools -> Text<->Table ///"
    Call wDokSchreiben "five;2;3;4;5;<return>"
    Call wDokSchreiben "a;b;c;d;e;<return>"
    Call wDokSchreiben "A;B;C;D;E;"
    Call wDokSchreiben "<shift home>"
    Call wDokSchreiben "<shift up>"
    Call wDokSchreiben "<shift up>"

    ToolsTextInTable
    Kontext "TextInTabelleUmwandeln"
    Semikolon.Check

    TextInTabelleUmwandeln.OK
    Call wDokSchreiben "<end><Mod1 end><down><return>"

    Call hCloseDocument
endcase

' --------------------------------------------------------------------

testcase tTableNumberFormat

    Call hNewDocument
    Printlog "'/// Fill table with content: Tools->Options->TextDoc->Number formating/recognition check ALL ///"
    '///    use table 5x10; and try to get every entry from format->numberFormat->Categorie ///'

    Call TBOhTabelleEinfuegen ("six",0, 0, 0, 1,"5", "10" )
    Call wDokSchreiben "six<down>"

    ToolsOptions           ' set options for this test
    Call hToolsOptions ("TextDocument","Table")
    AutomatischeZahlenerkennung.Check
    Zahlenformaterkennung.Check
    AutomatischeAusrichtung.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog "Checking numberformater"
    '///+ - Currency    
    printlog "  - Currency"
    Call wDokSchreiben(fLocaleString("LocaleCurrencyValue"))
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    if (Kategorie.GetSelIndex <> 5) then
            warnlog "The value is not recognized as currency, it is " & Kategorie.GetSelText
        else
            printlog "    - Value is correctly recognized as currency"
    end if
    Zahlenformat.Cancel
    formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (rechts.isChecked <> true) then 
            warnlog "wrong alignment of number"
        else
            printlog "    - Currencyvalue is correctly alligned"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> fLocaleString("LocaleCurrencyValue")) then
            warnlog "The number is not correctly formated. It is: " & GetClipboardText & " but should be: " & fLocaleString("LocaleCurrencyValue")
        else
            printlog "    - Currencyvalue is correctly formated"
    end if
    
    '///+ - Unknown currency
    printlog "  - Unknown currency"
    Call wDokSchreiben("10.78 §")
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    if (Kategorie.GetSelIndex <> 11) then
            warnlog "The value is not recognized as string, it is " & Kategorie.GetSelText
        else
            printlog "    - Value is correctly recognized as string"
    end if
    Zahlenformat.Cancel
    formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (links.isChecked <> true) then 
            warnlog "wrong alignment of number"
        else
            printlog "    - Value is correctly alligned"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> "10.78 §") then
            warnlog "The number is not correctly formated: should be 10.78 §, but is " & GetClipboardText
        else
            printlog "    - Value is correctly formated"
    end if
    
    '///+ - Date
    printlog "  - Date"
    Call wDokSchreiben(fLocaleString("LocaleDateValue"))
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    if (Kategorie.GetSelIndex <> 6) then
            warnlog "The value is not recognized as date, it is " & Kategorie.GetSelText
        else
            printlog "    - Value is correctly recognized as date"
    end if
    Zahlenformat.Cancel
    Formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (rechts.isChecked <> true) then 
            warnlog "wrong alignment of number"
        else
            printlog "    - Datevalue is correctly alligned"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> fLocaleString("LocaleDateOutput")) then
            warnlog "The number is not correctly formated: should be " & fLocaleString("LocaleDateOutput") & ", but is " & GetClipboardText
        else
            printlog "    - Datevalue is correctly formated"
    end if
    
    '///+ - Time
    printlog "  - Time"
    Call wDokSchreiben(fLocaleString("LocaleTimeInput"))
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    if (Kategorie.GetSelIndex <> 7) then
            warnlog "The value is not recognized as time, it is " & Kategorie.GetSelText
        else
            printlog "    - Value is correctly recognized as date"
    end if
    Zahlenformat.Cancel
    Formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (rechts.isChecked <> true) then 
            warnlog "wrong alignment of number"
        else
            printlog "    - Timevalue is correctly alligned"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> fLocaleString("LocaleTimeValue")) then
            warnlog "The number is not correctly formated: should be " & fLocaleString("LocaleTimeValue") & ", but is " & GetClipboardText
        else
            printlog "    - Timevalue is correctly formated"
    end if
    
    '///+ - Percent
    printlog "  - Percent"
    Call wDokSchreiben(fLocaleString("LocalePercentValue"))
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    if (Kategorie.GetSelIndex <> 4) then
            warnlog "The value is not recognized as percent, it is " & Kategorie.GetSelText
        else
            printlog "    - Value is correctly recognized as percent"
    end if
    Zahlenformat.Cancel
    Formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (rechts.isChecked <> true) then 
            warnlog "wrong alignment of number"
        else
            printlog "    - Percentvalue is correctly alligned"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> fLocaleString("LocalePercentValue")) then
            warnlog "The number is not correctly formated: should be " & fLocaleString("LocalePercentValue") & ", but is " & GetClipboardText
        else
            printlog "    - Percentvalue is correctly formated"
    end if
    
    '///+ - Scientific
    printlog "  - Scientific"
    Call wDokSchreiben(fLocaleString("LocaleScientificValue"))
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    if (Kategorie.GetSelIndex <> 8) then
            warnlog "The value is not recognized as scientific, it is " & Kategorie.GetSelText
        else
            printlog "    - Value is correctly recognized as scientific"
    end if
    Zahlenformat.Cancel
    Formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (rechts.isChecked <> true) then 
            warnlog "wrong alignment of number"
        else
            printlog "    - Scientificvalue is correctly alligned"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> fLocaleString("LocaleScientificValue")) then
            warnlog "The number is not correctly formated: should be " & fLocaleString("LocaleScientificValue") & ", but is " & GetClipboardText
        else
            printlog "    - Scientificvalue is correctly formated"
    end if
    
    '///+ - Fraction
    printlog "  - Fraction"
    Call wDokSchreiben("58 11/4")
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    if (Kategorie.GetSelIndex <> 9) then
            warnlog "The value is not recognized as fraction, it is " & Kategorie.GetSelText
        else
            printlog "    - Value is correctly recognized as fraction"
    end if
    Zahlenformat.Cancel
    Formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (rechts.isChecked <> true) then 
            warnlog "wrong alignment of number"
        else
            printlog "    - Fractionvalue is correctly alligned"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> "60 3/4") then
            warnlog "The number is not correctly formated: should be 60 3/4, but is " & GetClipboardText
        else
            printlog "    - Fractionvalue is correctly formated"
    end if
    
    '///+ - Boolean
    printlog "  - Boolean"
    Call wDokSchreiben(fLocaleString("LocaleBooleanValue"))
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    
    if (Kategorie.GetSelIndex = 10) then
    		printlog "    - Value is correctly recognized as boolean"
    	elseif iSprache = 86 or iSprache = 82 or iSprache =88 then 
    			qaerrorlog "#i73416#Boolean value in chinese and korean not recognized"
    		else
    		warnlog "The value " & fLocaleString("LocaleBooleanValue") & "is not recognized as boolean, it is " & Kategorie.GetSelText 
     end if
    Zahlenformat.Cancel
    Formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (rechts.isChecked = true) then 
            printlog "    - Booleanvalue is correctly alligned"
        elseif iSprache = 86 or iSprache = 82 or iSprache = 88 then 
    			qaerrorlog "#i73416#Boolean value in chinese and korean not recognized"
            else
            warnlog "wrong alignment of number"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> fLocaleString("LocaleBooleanValue")) then
            warnlog "The number is not correctly formated: should be " & fLocaleString("LocaleBooleanValue") & ", but is " & GetClipboardText
        else
            printlog "    - Booleanvalue is correctly formated"
    end if
    
    '///+ - String
    printlog "  - String"
    Call wDokSchreiben("testtext")
    Call wDokSchreiben "<Tab><shift tab>"
    FormatNumberFormat
    Kontext "Zahlenformat"
    if (Kategorie.GetSelIndex <> 11) then
            warnlog "The value is not recognized as string, it is " & Kategorie.GetSelText
        else
            printlog "    - Value is correctly recognized as string"
    end if
    Zahlenformat.Cancel
    Formatparagraph
    Kontext
    Active.SetPage tabAusrichtungAbsatz
    Kontext "tabAusrichtungAbsatz"
    if (links.isChecked <> true) then 
            warnlog "wrong alignment of string"
        else
            printlog "    - String is correctly alligned"
    end if
    TabAusrichtungAbsatz.Cancel
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> "Testtext") then
            warnlog "The string is not correctly formated: should be Testtext, but is " & GetClipboardText
        else
            printlog "    - String is correctly formated"
    end if

    '/// finished the file, now some hardwired tests ///'
    '///+    check, if all decimals are saved, even if not displayed ///'
    Call wDokSchreiben "123" & cDecSep & "123456789"
    Call wDokSchreiben "<Tab><shift tab>"
    Call wDokSchreiben "<home><shift end>"
    wait 500
	EditCopy
    if (GetClipboardText <> "123" & cDecSep & "12") then warnlog "wrong reformating of text tablefield 1; should: " & "123" & cDecSep & "12 is: "+GetClipboardText
    EditUndo
    Call wDokSchreiben "<home><shift end>"
    EditCopy
    if (GetClipboardText <> "123" & cDecSep  & "123456789") then warnlog "wrong reformating of text tablefield 2; should: 123" & cDecSep &  "123456789 is: "+GetClipboardText

    Call hCloseDocument ()
endcase

' --------------------------------------------------------------------

testcase tTableDelete
    dim i as integer

    Call hNewDocument
    Printlog "'/// delete rows/columns in a table ///"

    Call TBOhTabelleEinfuegen ("seven",0, 0, 0, 1,"20", "20" )
    ' Point cursor to upper left cell
    Call wDokSchreiben "<Mod1 Home>", 2
    Call wDokSchreiben "1"
    For i = 2 to 20
        Call wDokSchreiben "<down><right>"
        Call wDokSchreiben CStr(i)
    next i
    Call wDokSchreiben "<Mod1 Home>", 2

    Printlog "'///   via keys {alt delete} {cursor} -> delete row/column ///"
    ' delete some rows/columns using shortcuts del 2rows 2cols
    Call wDokSchreiben "<mod2 delete><right>"
    Call wDokSchreiben "<right>"
    Call wDokSchreiben "<mod2 delete><left>"
    Call wDokSchreiben "<left>"
    Call wDokSchreiben "<mod2 delete><down>"
    Call wDokSchreiben "<down>"
    Call wDokSchreiben "<mod2 delete><up>"
    Call wDokSchreiben "<up>"
    Call SpaltenTesten ( 18 )
    Call wDokSchreiben "<Mod1 Home>", 2
    Call wDokSchreiben "<Mod1 A>"
    EditCopy
    if GetClipboardtext <> "2" then
        Warnlog "Deleting rows/columns via keys has not been worked as accepted! -> Test failed!"
        Call hCloseDocument
        exit sub
    end if

    Printlog "'///   via menue Format[Zeile|Spalte]Loeschen ///"
    for i = 1 to 17             ' delete some rows/columns
        FormatZeileLoeschen
        wait 500
        FormatSpalteLoeschen
        wait 500
    next i
    Call wDokSchreiben "<Mod1 A>"
    EditCopy
    if GetClipboardtext <> "20" then
        Warnlog "Deleting rows/columns via keys has not been worked as accepted! -> Test failed!"
        Call hCloseDocument
        exit sub
    end if

    Printlog "'///   via keys {alt insert} {cursor} -> insert row/column ///"
    Call wDokSchreiben "<ESCAPE>"
    Call wDokSchreiben "<mod2 insert><left>"
    Call wDokSchreiben "<mod2 insert><right>"
    Call wDokSchreiben "<mod2 insert><down>"
    Call wDokSchreiben "<mod2 insert><up>"
    Call SpaltenTesten ( 3 )
    Call wDokSchreiben "<Mod1 Home>", 2
    Call wDokSchreiben "<Shift End>"
    try
        EditCopy
        Warnlog "Maybe no columns have been inserted!"
    catch
        Call wDokSchreiben "<Mod1 Right>", 2
        Call wDokSchreiben "<Shift End>"
        try
            EditCopy
            if GetClipboardtext <> "20" then
                Warnlog "Maybe no columns have been inserted!"
            end if
        catch
            Warnlog "Maybe no columns have been inserted!"
        endcatch
    endcatch

    Printlog "'///   delete whole table ///"
    Call wDokSchreiben "<mod1 home>", 2  ' delete hole table
    Call wDokSchreiben "<Mod1 A>"
    try
        EditCut
        Call wDokSchreiben "<return>"
    catch
        Warnlog "Unable to delete table!"
    endcatch
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------

testcase tTableText
    dim i as integer

    Call hNewDocument
    Printlog "'/// +++ table to text ///"

    Call TBOhTabelleEinfuegen ("seven",0, 0, 0, 1,"10", "10" )

    for i = 1 to 99          ' fill every cell
        Call wDokSchreiben CStr(i)
        Call wDokSchreiben "<Tab>"
    next i

    ToolsTextInTable         ' convert with '+' seperator
    Kontext "TextInTabelleUmwandeln"
    Andere.Check
    AndereText.settext "+"
    TextInTabelleUmwandeln.OK

    Call wDokSchreiben "<home>"   ' calculate every row
    for i = 1 to 10
        Call wDokSchreiben "<shift end>"
        ToolsCalculate
        Call wDokSchreiben "<shift insert>"
        Call wDokSchreiben "+<delete>"
    next i
    Call wDokSchreiben "<backspace>0<shift home>"
    ToolsCalculate
    if (GetClipboardText <> "12645") then warnlog "wrong table converting table<->text "
    Call wDokSchreiben "<shift insert><return>"

    Call hCloseDocument ()
endcase

'-------------------------------------------------------------------------------------------

testcase tTableCalculate
    Printlog "'/// calculating in table ///"
    Call hNewDocument

    Call TBOhTabelleEinfuegen ()
    Call wDokSchreiben "1<Tab>2<Tab>3<Tab>4<Tab>5<Tab>6<Tab>7<Tab>8<Tab>"
    Call wDokSchreiben "<end><Mod1 end><down><return>"
    Call TBOhTabelleEinfuegen ("eight",0, 0, 0, 1,"5", "5" )

    '///'  should look like: <pre> ///
    '///+  a        b    c        d                 e ///
    '///+1 1,.1  2,.2   =3,.3  4,.4        =summe<:4,.4 ///
    '///+2 1        2   =3        4        =summe<:4 ///
    '///+3 =2,.1 =18,.9 =2     =-0,31      =32,.6926671... <1.check> ///
    '///+4 =14   =16    gone   =<2.check>  = ...table1 ?? ///
    '///+5 </pre> ///

    '/// this table gets generated one by one ///'

    Call wDokSchreiben "1"+cDecSep+"1<Tab>" ' insert input A1
    Call wDokSchreiben "2"+cDecSep+"2<Tab>"
    Call wDokSchreiben "="                  '
    Kontext "OL_SW_Rechenleiste"       '               C1
    Eingabefeld.SetText "<A1>+<B1>"
    Uebernehmen.click
    Call wDokSchreiben "<Tab>"
    Call wDokSchreiben "4"+cDecSep+"4<Tab>"
    Call wDokSchreiben "<Tab>"

    Call wDokSchreiben "1<Tab>"             '               A2
    Call wDokSchreiben "2<Tab>"
    Call wDokSchreiben "<Tab>"
    Call wDokSchreiben "4<shift tab>"
    Call wDokSchreiben "="                  '
    Kontext "OL_SW_Rechenleiste"       '               C2
    Eingabefeld.SetText "<D2>-<A2>"
    Uebernehmen.click
    Call wDokSchreiben "<Tab>"
    Call wDokSchreiben "<Tab>"

    ' automatic sum works only to next formula :-< ///'
	Call hToolbarSelect ( "Table", true )
    formeleingeben.click
    Kontext "OL_SW_Rechenleiste"
    Uebernehmen.click
    Call wDokSchreiben "<up>"

	Call hToolbarSelect ( "Table", true )
    formeleingeben.click
    Kontext "OL_SW_Rechenleiste"
    Uebernehmen.click
    Call wDokSchreiben "<down><Tab>"

    Call wDokSchreiben "="                  ' slip into calculation area
    Kontext "OL_SW_Rechenleiste"       '               A3
    Eingabefeld.SetText "sum <A1:A2>"
    Uebernehmen.click
    Call wDokSchreiben "<Tab>"
    Call wDokSchreiben "<F2>"                  '
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.SetText "sum <D2:B1>"
    Uebernehmen.click
    Call wDokSchreiben "<Tab>"
    Call wDokSchreiben "<F2>"                  '
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.SetText "sqrt <E2>"
    Uebernehmen.click
    Call wDokSchreiben "<Tab>"
    Call wDokSchreiben "<F2>"                  '
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.SetText "cos <E1>"
    Uebernehmen.click
    Call wDokSchreiben "<Tab>"

    Call wDokSchreiben "<F2>"                  ' E3 check this out :-)
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.SetText "sum <A2:D3>"
    Uebernehmen.click
    Call wDokSchreiben "<shift home>"
    EditCopy
    if (GetClipboardText <> "32" & cDecSep & "69") then warnlog "wrong calculation should: 32" & cDecSep & "69 is: "+GetClipboardText
    Call wDokSchreiben "<end><Tab>"

    Call wDokSchreiben "<F2>"                  ' A4
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.SetText "2+3*4"
    Uebernehmen.click
    Call wDokSchreiben "<shift home>"
    EditCopy
    if (GetClipboardText <> "14") then warnlog "wrong calculation should: "+"14"+" is: "+GetClipboardText
    Call wDokSchreiben "<end><Tab>"

    Call wDokSchreiben "<F2>"                  ' B4
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.SetText "2*2 pow 3"
    Uebernehmen.click
    Call wDokSchreiben "<shift home>"
    EditCopy
    if (GetClipboardText <> "16") then warnlog "wrong calculation should: "+"16"+" is: "+GetClipboardText
    Call wDokSchreiben "<end><Tab>"
    Call wDokSchreiben "<shift tab>"
    sleep (1)
    Call wDokSchreiben "<Tab>"
    sleep (1)

    '/// one row gets deleted///'
    FormatSpalteLoeschen ' grumel: have to find a cell :-(
    Call wDokSchreiben "<F2>"                  ' C4
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.SetText "sum <A1:D3>"
    Uebernehmen.click
    Call wDokSchreiben "<shift home>"
    EditCopy
    if (GetClipboardText <> "58" & cDecSep & "89") then warnlog "wrong calculation should: " & "58" & cDecSep & "89" & " is: " & GetClipboardText
    Call wDokSchreiben "<end><Tab>"
    Call wDokSchreiben "<F2>"                  ' C4
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.SetText "sum <"+fLocaleString("LocaleTable")+"1.B1:E1> / <A2:D2>"
    sleep (2)
    Uebernehmen.click
    Call wDokSchreiben "<shift home>"
    EditCopy
    if (GetClipboardText <> "1" & cDecSep & "27") then warnlog "wrong calculation should: 1" & cDecSep & "27 is: " & GetClipboardText
    Call wDokSchreiben "<end><Tab>"

    Call wDokSchreiben "<end><Mod1 end><down><return>"
    Call hCloseDocument ()
endcase

'-------------------------------------------------------------------------------------------

testcase tTableEdit
    dim itemp(5) as integer

    Call hNewDocument
    Printlog "'///  edit table ///"

    Call TBOhTabelleEinfuegen ("nine-dumy",0, 0, 0, 1,"5", "3" )
    Call wDokSchreiben "<end><Mod1 end><Mod1 end><return>"
    Call TBOhTabelleEinfuegen ("nine",0, 0, 0, 1,"5", "3" )
    Call wDokSchreiben "nine<Tab>"

    Printlog "'///    insert Frame & Graphic in heading ///"
    InsertFrame
    sleep (5)
    Kontext "tabtype"
    sleep (5)
    TabType.ok
    sleep (5)
    Call wDokSchreiben "<escape>"
    sleep (5)
    Call hGrafikEinfuegen (gTesttoolPath +"writer\optional\input\options\ga000907.gif")
    sleep (5)
    Call wDokSchreiben "<escape>"
    sleep (5)
    Call wDokSchreiben "<mod1 end><up><up>"
    sleep (5)

    Printlog "'///    insert Frame & Graphic in body ///"
    InsertFrame
    sleep (5)
    Kontext "tabtype"
    sleep (5)
    TabType.ok
    sleep (5)
    Call wDokSchreiben "<escape>"
    sleep (5)
    Call hGrafikEinfuegen (gTesttoolPath +"writer\optional\input\options\ga000907.gif")
    sleep (5)
    Call wDokSchreiben "<escape>"
    sleep (5)
    Call wDokSchreiben "<mod1 end><up><up><Tab>"

    Printlog "'///    insert Frame & Table in body ///"
    InsertFrame
    sleep (5)
    Kontext "tabtype"
    sleep (5)
    TabType.ok
    sleep (5)
    Call wDokSchreiben "<return>"
    sleep (5)
    try
        Call hTabelleEinfuegen ()
        Call wDokSchreiben "<mod1 end><mod1 end><up><up><Tab><Tab>"
    catch
        Warnlog "Hit <Enter>-Key should enable edit mode for selected frames! (srx642z or greater)"
        Call wDokSchreiben "<escape>"
        Call wDokSchreiben "<down>"
    endcatch
    sleep (5)

    Printlog "'///    insert PageBreak ///"
    '/// TBO: check test / insert dummy table      ' doesn't work, if the first 'stuff' in a document is the table ///'
    InsertFieldsPagenumbers
    Call wDokSchreiben "<shift home>"
    EditCopy
    ToolsCalculate
    sleep (5)
    itemp(1) = val(GetClipboardText)
    EditCut
    InsertManualBreak
    sleep (5)
    Kontext "UmbruchEinfuegen"
    Seitenumbruch.check
    UmbruchEinfuegen.OK
    sleep (5)
    InsertFieldsPagenumbers
    Call wDokSchreiben "<shift home>"
    EditCopy
    ToolsCalculate
    sleep (5)
    itemp(3) = val(GetClipboardText)
    if (itemp(1) <> itemp(3)) then
        printlog "--- Pagebreak works in Table, that's good :-)"
    else
        warnlog " Pagebreak doesn't work in Table, that's not good :-("+itemp(1) +" "+ itemp(3)
    end if

    sleep (5)
    Call wDokSchreiben "<end><Mod1 end><down><return>"

    Printlog "'///   the following may not be possible ///"
    Printlog "'///+      Insert table in footnote ///"
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    FussnoteEinfuegen.OK
    try
        InsertTableWriter
        warnlog "     Table in Footnote is possible, that's not good :-("
    catch
    endcatch

    Call wDokSchreiben "<end><Mod1 end><return>"
    Call hCloseDocument
endcase
