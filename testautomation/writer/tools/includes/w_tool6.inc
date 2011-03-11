'encoding UTF-8  Do not remove or change this line!
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
'* short description : Tools for writer tests.
'*
'\******************************************************************************

sub wOfficeRestart

    hFileCloseAll()
    ' Close the backing windows
    FileExit "SynchronMode", TRUE

    Call hStartTheOffice

    ' Open new document
    Call hNewDocument

end sub

'--------------------------------------------------------------
' end of temporaly functions
'--------------------------------------------------------------

function LiberalMeasurement ( sShould$, sActual$) as Boolean
    '/// Input : (1. Should, 2. Actual) as Number with or without MeasurementUnit 'NumericField' as String ///'
    '///+ if input has no MeasurementUnit i take it as 'cm' (was the default in old tests) ///'
    '///+ Output: Boolean are they likely the same?
    '/// NEEDED: mathematical proofment of iTolerance, by now just some guesses :-| ///'
    '/// reason for this function:///'
    '///+ because SO counts internaly in 'twip┤s' 'twentieth of a point' there are some rounding errors ///'
    '///+ there are also some rounding errors because of the internal representatio of floating point numbers in computers ///'
    '///+ now lets try to get rid of them and have a nicer output in tests... ///'
    dim iTolerance as Double

    LiberalMeasurement = False ' worst case

    if (sShould$ = sActual$) then
        LiberalMeasurement = True
    else
        ' check if measunit is the same !!
        if (GetMeasUnit(sShould$) <> GetMeasUnit(sActual$) ) then
            warnlog "in function LiberalMeasurement the measUnit is different, compare not possible yet :-("
        else
            ' set factor for liberality ;-)
            iTolerance = 0.04
            if ( (StrToDouble(sShould$) + iTolerance) >= StrToDouble(sActual$) ) AND ((StrToDouble ( sShould$ ) - iTolerance) <= StrToDouble ( sActual$ )) then
                LiberalMeasurement = True
            else
                LiberalMeasurement = False
            end if
        end if
    end if
end function

'--------------------------------------------------------------

function GetMeasUnit ( sWert$ ) as String
    '/// Input : Number with or without MeasurementUnit 'NumericField' as String ///'
    '///+ Output: Initials of MeasurementUnit as String or "" when only a number ///'

    '/// first lets check, if there is a number -> no unit there ///
    if isNumeric (right (sWert$, 1)) then
        GetMeasUnit = ""
    else
        '/// the only single character is '"' all others are two chars ///
        if (  StrComp (right (sWert$, 1), chr$(34) ) = 0 ) then
            GetMeasUnit = right (sWert$, 1)
        else
            GetMeasUnit = right (sWert$, 2)
        end if
    end if
end function

'--------------------------------------------------------------

function StrToDouble ( sWert$ ) as Double
    Dim sDummy$, test
    dim i, i1, i2 as integer
    dim a as integer
    dim b as integer
    dim c as double
    dim n as integer

    '/// Input : {'a[. ,]b[mm cm " pi pt]' with a, b as integer} as String ///'
    '///+ Output: a[. , ]b as double ///'

    ' get rid of measure unit, the only single character is '"' all others are two chars
    ' there was a problem, if there is NO meas.unit!!!
    if Len(sWert$) > Len(mUnit) then
        test = Left$(sWert$, (len(sWert$) - Len(mUnit)))
        StrToDouble = cDbl(rtrim(test))
        exit function
        if (isNumeric (sWert$) = FALSE) then
            if (  StrComp (right (sWert$, 1), chr$(34) ) = 0 ) then
                sDummy$ = Left$ ( sWert$, Len(sWert$)-1 )
            else
                sDummy$ = Left$ ( sWert$, Len(sWert$)-2 )
            end if
        else
            sDummy$ = sWert$
        end if
        ' get position of fraction mark
        i1 = instr (sDummy$, ",")  ' wrong output
        i2 = instr (sDummy$, ".")
        if i1 > i2 then i = i1 else i = i2
        ' vorkommateil
        try
            a = val (left$ (sDummy$,i-1))
        catch
        endcatch
        n = (len (sDummy$)-i)
        b = val (right$ (sDummy$, n) )
        c = b * 10 ^ -n
        StrToDouble = a + c
    else
        StrToDouble = 0
    end if
end function

'--------------------------------------------------------------

function wKillUpcomingActives(optional sDocument as string)
    ' primary used in import/export tests
    '/// to get any aktive killed & print the content of the aktive ///'
    ' to catch errors during loading document
    Kontext "Active"
    if Active.Exists then
        if Strcomp(Left(Active.GetText,42),"Saving in external formats may have caused") = 0 then
            printlog "info loss, messg!! OK??"
        end if
        try
            Active.Yes
        catch
            printLog Active.GetText
        endcatch
        sleep(2)
        Kontext "Active"
        if Active.Exists then
            if IsMissing(sDocument) then
                Warnlog "(1/2): " + Active.GetText ' Changed from Warn to print.
            else
                Select case sDocument
                Case "sw31.sdw", "sw31.vor"
                    warnlog "Saving sdw-file as sxw fails"
                Case else
                    Warnlog Active.Gettext
                end select
            end if
            try
                Active.Ok ' Read Error
            catch
                Active.No ' Style is different ...
            endcatch
        end if
    end if
    ' catching style after read error :-)
    sleep(2)
    Kontext "Active"
    if Active.Exists then
        WarnLog "(2/2): "+Active.GetText
        try
            Active.No ' Style is different ...
        catch
            Active.Ok ' maybe...
        endcatch
    end if
end function

'--------------------------------------------------------------

function wGetCharacterFont as String
    Dim wasAsianLan as boolean
    '/// Input : nothing ///'
    '///+ Output: Fontname at cursor position as String ///'
    FormatCharacter
    Kontext
    sleep (1)
    Active.SetPage TabFont
    Kontext "TabFont"
    sleep (1)
    try
        if gAsianSup = True then
            wGetCharacterFont = FontWest.GetSelText
        else
            wGetCharacterFont = Font.GetSelText
        end if
        TabFont.Cancel
        sleep (1)
    catch
        TabFont.Cancel
        wasAsianLan = ActiveDeactivateAsianSupport (True)
        FormatCharacter
        Kontext
        sleep (1)
        Active.SetPage TabFont
        Kontext "TabFont"
        sleep (1)
        wGetCharacterFont = FontWest.GetSelText
        TabFont.Cancel
        sleep (1)
        wasAsianLan = ActiveDeactivateAsianSupport (True)
    endcatch
end function

'--------------------------------------------------------------

function wGetCharacterFontSize as String
    Dim wasAsianLan as boolean
    '/// Input : nothing ///'
    '///+ Output: FontSize at cursor position as String ///'
    FormatCharacter
    Kontext
    sleep (1)
    Active.SetPage TabFont
    Kontext "TabFont"
    sleep (1)
    try
        if gAsianSup = True then
            wGetCharacterFontSize = SizeWest.GetSelText
        else
            wGetCharacterFontSize = Size.GetSelText
        end if
        TabFont.Cancel
        sleep (1)
    catch
        TabFont.Cancel
        wasAsianLan = ActiveDeactivateAsianSupport (True)
        FormatCharacter
        Kontext
        sleep (1)
        Active.SetPage TabFont
        Kontext "TabFont"
        sleep (1)
        wGetCharacterFontSize = SizeWest.GetSelText
        TabFont.Cancel
        sleep (1)
        wasAsianLan = ActiveDeactivateAsianSupport (True)
    endcatch
end function

'--------------------------------------------------------------

function ZeilenHoeheHolen as Double
    '/// input: nothing///'
    '///+ output: FormatRowHeight as double///'
    Dim zWert as Double : Dim Ausgabe$
    FormatRowHeight
    Wait 100
    Kontext "ZellenHoehe"
    Ausgabe$ = Hoehe.GetText
    zWert = ZahlAusSpinnfield ( Ausgabe$ )
    if zWert = 0.01 then
        ZeilenHoeheHolen = 0.00
    else
        ZeilenHoeheHolen = zWert
    end if
    ZellenHoehe.OK
end function

'--------------------------------------------------------------

sub ZeilenHoeheTesten ( Wert as Double )
    '/// input: FormatRowHeight as double///'
    '///+ output: warnlog, if not eaqual ///'
    '/// LiberalMeasurement enabled///'
    Dim zWert as Double
    FormatRowHeight
    Kontext "ZellenHoehe"
    zWert = ZahlAusSpinnfield ( Hoehe.GetText )
    if (LiberalMeasurement (Wert, zWert) <> TRUE) then QAErrorlog "#i94556# - Die Zeilenhöhe ist nicht "+ Wert + " sondern "+ zWert + "."
    ZellenHoehe.OK
end sub

'--------------------------------------------------------------

function SpaltenBreiteHolen as Double
    FormatColumnWidthWriter
    Kontext "SpaltenBreite"
    SpaltenBreiteHolen = ZahlAusSpinnfield (Breite.GetText )
    SpaltenBreite.OK
end function

'--------------------------------------------------------------

sub SpaltenBreiteTesten ( Wert as Double )
    Dim zWert as Double
    FormatColumnWidthWriter
    Kontext "SpaltenBreite"
    zWert = ZahlAusSpinnfield ( Breite.GetText )
    if Not Wert = zWert then
        Warnlog "Die Spaltenbreite ist nicht "+ Wert +" sondern "+ zWert
    end if
    SpaltenBreite.OK
end sub

'--------------------------------------------------------------

sub SeitenAbstaendeHolen ( ZweiWerte() as Double )
    FormatTable
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    ZweiWerte(1) = ZahlAusSpinnfield ( NachLinks.GetText )
    ZweiWerte(2) = ZahlAusSpinnfield ( NachRechts.GetText )
    TabTabelle.Cancel
end sub

'--------------------------------------------------------------

sub SeitenAbstaendeTesten ( WertLi as Double, WertRe as Double )
    '/// liberalMeasurement implemented ///'
    Dim zWert1 as Double : Dim zWert2 as Double
    FormatTable
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    zWert1 = ZahlAusSpinnfield ( NachLinks.GetText )
    zWert2 = ZahlAusSpinnfield ( NachRechts.GetText )

    if (LiberalMeasurement (zWert1, WertLi) <> TRUE) then Warnlog "Left distance not " + WertLi + "  but " + zWert1
    if (LiberalMeasurement (zWert2, WertRe) <> TRUE) then Warnlog "Right distance not " + WertRe + "  but " + zWert2
    wait 300
    TabTabelle.Cancel
end sub

'--------------------------------------------------------------

function ZahlAusSpinnfield ( sWert$ ) as Double
    ZahlAusSpinnfield = StrToDouble ( sWert$)
    printlog ZahlAusSpinnfield
end function

'--------------------------------------------------------------

sub ZeilenTesten ( Anzahl% )
    dim temp(10) as string
    Dim i as Integer
    Dim Dummy as Integer
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Up>", 5 ' Move out of table ????!!!!
    for i=2 to Anzahl% +4   ' might work, but if there are some more tables, it doesn't!!
        try
            Call wTypeKeys "<Down>"
            FormatRowHeight       'This is OK; but different evaluation is necessary
            Kontext "ZellenHoehe"
            ZellenHoehe.Cancel
            dummy = i
        catch
            i = Anzahl% +20
        endcatch
    next i
    if dummy <> Anzahl% then Warnlog "Die Tabelle hat wohl mehr Zeilen als erwartet: soll => "+ Anzahl% +" sind "+ dummy
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Up><Left>", 10
end sub

'--------------------------------------------------------------

sub SpaltenTesten ( Anzahl% )
    FormatColumnWidthWriter
    Kontext "SpaltenBreite"
    Spalte.More Anzahl%
    if Anzahl% <> Spalte.GetText then Warnlog "Table has mohl column then expected: should => "+ Anzahl% +" is "+ Spalte.GetText
    SpaltenBreite.Cancel
end sub

'--------------------------------------------------------------

sub TBOhTabelleEinfuegen (optional tName as string, optional tHeader as boolean, optional tRepeat as boolean, optional tSeperate as boolean, optional tBorder as boolean, optional tWidth as string, optional tHeight as string )
    '///   TBOhTabelleEinfuegen ("Garfield",0,1,0,1,"10",tHeight:="7") ///'
    ' maybe TODO: return of an array, that tells you the state of an existing / name table/ cause, you create a table in a tabl÷e :-)
    ' try with switching tabpage
    InsertTableWriter
    Sleep 2
    Kontext "TabelleEinfuegenWriter"
    wait 500

    ' Default had been changed. Not to rewrite the whole test I decided to changed the row-number
    Spalten.Settext "5"
    wait 500

    if (IsMissing (tName) <> True) then TabellenName.SetText tName
    if (IsMissing (tHeader) <> True) then
        if tHeader then
            Ueberschrift.Check
            if (IsMissing (tRepeat) <> True) then if tRepeat then UeberschriftWiederholen.Check else UeberschriftWiederholen.UnCheck
        else
            Ueberschrift.UnCheck
        end if
    end if

    if IsMissing (tSeperate) <> True then
        if tSeperate then
            TabelleNichtTrennen.Check
        else
            TabelleNichtTrennen.UnCheck
        end if
    end if

    if IsMissing(tBorder) <> True then
        if tBorder then
            Umrandung.check
        else
            Umrandung.check
        end if
    end if

    if IsMissing(tWidth) <> True then
        Spalten.SetText tWidth
    else
        Spalten.Settext "5"
    end if
    if IsMissing(tHeight) <> True then Zeilen.SetText tHeight

    TabelleEinfuegenWriter.OK
    Sleep 1

    Kontext "TableObjectbar"
    Sleep 1
    if TableObjectbar.NotExists then
        Kontext "TextObjectbar"
        TextObjectbar.SetNextToolBox
    end if

    Select Case gApplication
    Case "WRITER"
        Kontext "DocumentWriter"

    Case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"

    Case "HTML"
        Kontext "DocumentWriterWeb"

    end select
    Sleep 1
end sub

'--------------------------------------------------------------

function hGetTableName () as string
    hGetTableName = "" ' Worst Case
    TableTableProperties                ' get into existing table
    Sleep (1)
    try
        Kontext
        Active.SetPage TabTabelle
        Sleep (1)
        Kontext "TabTabelle"
        Sleep (1)
        hGetTableName = TabellenName.GetText
        TabTabelle.Cancel
    catch
        Kontext "TabelleEinfuegenWriter"
        if TabelleEinfuegenWriter.Exists then
            hGetTableName = TabellenName.Gettext
            TabelleEinfuegenWriter.Ok
        else
            hGetTableName = ""
        end if
    endcatch
    Sleep (1)
end function

'--------------------------------------------------------------

function dec(Ref as integer)
    ' ----------------------------------------
    ' to give this func a var as ref: call without ANNY brackets => 'dec Variable'
    ' opposite of this to call it via value ! WE DON'T WANT THIS !
    ' (would be 'dec (Variable)' or in declaration 'function dec (ByVal x)')
    Ref = Ref - 1
end function

'--------------------------------------------------------------

function inc(Ref as integer)
    'printlog "inc:"+ref
    Ref = Ref + 1
end function

'--------------------------------------------------------------

function hGetColumn() as integer
    try
        FormatColumnWidthWriter
    catch
        printlog "func1.inc->hGetColumn asks for table."
        hTypeKeys "<left>"
        try
            FormatColumnWidthWriter
        catch
            print "giving up to find a table: func1 hgetcolumn 2"
        endcatch
    endcatch
    Kontext "SpaltenBreite"
    if SpaltenBreite.exists then
        hGetColumn = Spalte.GetText
        SpaltenBreite.Cancel
    else
        hGetColumn = 0
    end if
end function

'--------------------------------------------------------------

function hNavigatorOpenWindows() as Integer
    hNavigatorOpenWindows = 0

    goto ENDE
    '/// Input: (); Output: 0: for the usual started SO first window / +1 for each other open Window ///'
    '///+                    -2 if no window is open :-) (i think then there is another problem ...)///'
    '///+ this fuunction depends on navigator-fuunction:  ///'
    '///+ NO nav avail in: Formular; NOT CATCHED UPTONOW ///'
    '///+ count classes - only windows in this class are visible in their nav ///'
    '///+ Spreadsheet ///'
    '///+ Presentation, Drawing///'
    '///+ Text Doc, HTML Doc, Labels, Business cards, AND ///'
    '///+ !Master Doc! in his nav are no windows countable!; NOT CATCHED UPTONOW ///'

    '/// usually it counts 2 windows; 1. the window, that results in starting office ///'
    '///+ 2. The entry 'Active Window', is always there (entries in navigator changes automatical ///'
    '///+    to the visible window ->  2 this is the minima! ///'
    '///+ so i give back a count of x-2 everything below 0 is an error !///'

    dim j as integer, WelcherEintrag as integer
    ' Navigator zur├�?cksetzen
    Kontext "NavigatorWriter"
    if NavigatorWriter.NotExists then ViewNavigator
    sleep (1)
    Kontext
    do while (Active.Exists)
        printlog Active.GetText
        try
            active.yes ' ....
        catch
            active.ok ' ...new since 638a5 7001 :-(
        endcatch
        Kontext
    loop
    Kontext "NavigatorWriter"
    sleep (3)
    hNavigatorOpenWindows = DokumentListe.GetItemCount - 2  ' couldn' see dokumentliste :-( reason:: active that prevents it :-(
    sleep (3)
    '   printlog " hnow: "+DokumentListe.GetItemCount +" "+DokumentListe.GetSelText
    if (DokumentListe.GetItemCount = 0) then print "waassss o ??!!?!??!?!"
    '   close Navigator
    ViewNavigator
    Kontext "DocumentWriter"
    ENDE:
end function

'--------------------------------------------------------------

sub hSetSpellHypLanguage
    dim sTrieit
    if bAsianLan then
        Printlog " to get it to work, ihave to change the default languge in the options! FOR CURRENT DOCUMENT ONLY!!!!!"
        ToolsOptions
        Call hToolsOptions ("LANGUAGESETTINGS","LANGUAGES")
        AktuellesDokument.Check
        if glLocale (4) = "" then
            warnlog "choose a spellbokk from the list below and insert it into the file <\testtool\input\impress\locale_1.txt> on position (4) (only enabled for asiann languages!) - '"+glLocale(4)+"'"
            Kontext "ExtrasOptionenDlg"
            ExtrasOptionenDlg.Cancel
            sTrieit = hFindSpellHypLanguage
            ToolsOptions
            Call hToolsOptions ("LANGUAGESETTINGS","LANGUAGES")
            AktuellesDokument.Check
            if sTrieit <> "" then
                Westlich.Select sTrieit
            else
                warnlog "SOrry no spellbook found :-("
            end if
        else
            Westlich.Select glLocale (4)
        end if
        printlog "selected: "+Westlich.GetSelText
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
    end if
end sub

'--------------------------------------------------------------

sub hFindSpellHypLanguage as string
    dim iListLength as integer
    dim i as integer

    if bAsianLan then
        ToolsOptions                                             ' take length of list
        Call hToolsOptions ("LANGUAGESETTINGS","WRITINGAIDS")
        SprachmoduleBearbeiten.click
        Kontext "ModuleBearbeiten"
        for i = 1 to Sprache.GetItemCount
            Sprache.Select i
            sleep 1
            Printlog " "+i+": '"+Sprache.GetSelText +"'"
            if i = 1 then hFindSpellHypLanguage = Sprache.GetSelText
        next i
        ModuleBearbeiten.Close
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
        sleep 1
    end if
end sub

'--------------------------------------------------------------

function wCheckRowHeight ( ShouldValue as string ) as boolean
    FormatRowHeight
    Kontext "ZellenHoehe"
    if Hoehe.Gettext <> ShouldValue then
        Warnlog "Height is not " + ShouldValue + " but " + Hoehe.Gettext
        wCheckRowHeight = False
    else
        wCheckRowHeight = True
    end if
    ZellenHoehe.Cancel
end function

'--------------------------------------------------------------

sub wSearchWriteableArea ()
    Dim PageDownNow as boolean, NowWriteable as boolean
    Dim i as integer
    ' This sup has not been properly worked
    ' Looks complicated but isn't
    ' Searches for messagebox, if found makes a PageDown in document 5 times
    ' if writeable then, the moves curors up 1000 times. Hope it helps
    ' Document is completely writeprotected the sup would loop endless
    NowWriteable = False
    PageDownNow = True
    Do until NowWriteable = True
        Call wTypeKeys " "
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Active.Ok
                if PageDownNow = True then
                    for i = 1 to 10
                        Call wTypeKeys "<Down>", 5
                        Call wTypeKeys " "
                        Kontext "Active"
                        if Active.Exists then
                            if Active.GetRT = 304 then
                                Active.Ok
                            end if
                        else
                            NowWriteable = True
                            i = 11
                        end if
                    next i
                else
                    for i = 1 to 1000
                        Call wTypeKeys "<Up>", 1
                        Call wTypeKeys " "
                        Kontext "Active"
                        if Active.Exists then
                            if Active.GetRT = 304 then
                                Active.Ok
                            end if
                        else
                            NowWriteable = True
                            i = 1002
                        end if
                    next i
                end if
            else
                NowWriteable = True
                Exit Do
            end if
        else
            NowWriteable = True
            Exit Do
        end if
    Loop
end sub

'---------------------------------------------------

function fInsertFrame (x1 as Integer , y1 as Integer , x2 as Integer , y2 as Integer)
    'This function will insert a frame with the coordinate

    hSetDocumentContext()

    Call hToolbarSelect("INSERT", true )
    Sleep 1
    Kontext "Insertbar"
    Rahmen.Click
    Sleep 1

    Call gMouseDown ( x1,y1 )
    Call gMouseMove ( x1,y1,x2,y2 )
    Call gMouseUp( x2,y2 )
    wait 500

end function
