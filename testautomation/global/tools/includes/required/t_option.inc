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
'/******************************************************************************
'*
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : General option test (sub-routines for options tests)
'*
'\******************************************************************************

function hToolsOptions ( Applicationname as string, Tabpagename as string, OPTIONAL bSilent as Boolean ) as Boolean

    const CFN = "global::tools::includes::required::t_option.inc:"

    Dim bCorPage as Boolean
    Dim bFirst as Boolean
    
    Dim InitialNodeCount as Integer ' All nodes collapsed
    Dim CurrentNodeCount as Integer ' Number of nodes when a top node is expanded
    Dim iCurrentPage as Integer     ' Increment variable for stepping through the nodes
    Dim startpos as integer
    
    Applicationname = ucase( Applicationname )
    Tabpagename     = ucase( Tabpagename     )

    '///<i>hToolsOptions</i>: Routine to select the correct option page
    '///+<u>input</u>: The groupname as string, name of optionpage as string
    '///+<u>note</u>: The options dialog has to be opened
    '///+<u>OPTIONAL</u>: TRUE as an additional parameter if you want a printlog instead of a warning if a page has not been found.

    if IsMissing(bSilent) then
        bSilent = FALSE
    end if

    Kontext "Active"
    try
        if Active.Exists(1) then Active.OK
    catch
        if Active.Exists(1) then Active.No
    endcatch
    '///Group names list:
    '///+<ul><li>&quot;StarOffice&quot;</li>
    '///+<li>&quot;LoadSave&quot;</li>
    '///+<li>&quot;LanguageSettings&quot;</li>
    '///+<li>&quot;Internet&quot;</li>
    '///+<li>&quot;Textdocument&quot;</li>
    '///+<li>&quot;HTMLDocument&quot;</li>
    '///+<li>&quot;Spreadsheet&quot;</li>
    '///+<li>&quot;Presentation&quot;</li>
    '///+<li>&quot;Drawing&quot;</li>
    '///+<li>&quot;Formula&quot;</li>
    '///+<li>&quot;Chart&quot;</li>
    '///+<li>&quot;Datasource&quot;</li></ul>
    Kontext "ExtrasOptionenDlg"

    if ExtrasOptionenDlg.Exists then
        Optionsliste.TypeKeys "<HOME>"
        for iCurrentPage = 1 to 12+1
            Optionsliste.TypeKeys "-<DOWN>"
        next iCurrentPage
        'Optionsliste.TypeKeys "<HOME>"
        InitialNodeCount = Optionsliste.GetItemCount
        select case Applicationname
        case "STAROFFICE"        : startpos = 1
        case "LOADSAVE"          : startpos = 2
        case "LANGUAGESETTINGS"  : startpos = 3
        case "WRITER"            : startpos = 4
        case "MASTERDOCUMENT"    : startpos = 4
        case "HTML"              : startpos = 5
        case "CALC"              : startpos = 4
        case "IMPRESS"           : startpos = 4
        case "DRAW"              : startpos = 4
        case "MATH"              : startpos = 4
        case "DATASOURCES"       : startpos = InitialNodeCount -2
        case "CHART"             : startpos = InitialNodeCount -1
        case "INTERNET"          : startpos = InitialNodeCount ' always the last one
        case else
            warnlog( CFN & "Invalid Application name specified: " & Applicationname )
            printlog( "Valid options are:" )
            printlog( "gApplications: WRITER, CALC, MASTERDOCUMENT, DRAW, IMPRESS, HTML, MATH, CHART" )
            printlog( "additionally.: STAROFFICE, LOADSAVE, LANGUAGESETTINGS, DATASOURCES, INTERNET" )
            printlog( "All names must be uppercase" )
            exit function
        end select

        Optionsliste.select Startpos
        Optionsliste.typekeys "+"
        CurrentNodeCount = Optionsliste.GetItemCount
        bFirst = TRUE
        for iCurrentPage = 1 to CurrentNodeCount - InitialNodeCount
            bCorPage = FALSE
            Kontext "ExtrasOptionenDlg"
            Optionsliste.TypeKeys "<DOWN>"
            WaitSlot() ' Some sort of NOOP
            select case Applicationname
            case "STAROFFICE"
                '///Pages in group &quot;StarOffice&quot;are:
                '///+<ul><li>&quot;UserData&quot;</li>
                '///+<li>&quot;General&quot;</li>
                '///+<li>&quot;Memory&quot;</li>
                '///+<li>&quot;View&quot;</li>
                '///+<li>&quot;Print&quot;</li>
                '///+<li>&quot;Paths&quot;</li>
                '///+<li>&quot;Colors&quot;</li>
                '///+<li>&quot;FontReplacement&quot;</li>
                '///+<li>&quot;Security&quot;</li>
                '///+<li>&quot;Appearence&quot;</li>
                '///+<li>&quot;Accessibility&quot;</li>
                '///+<li>&quot;Java&quot;</li></ul>
                select case Tabpagename
                case "USERDATA"
                    Kontext "TabBenutzerdaten"
                    if TabBenutzerdaten.exists then bCorPage = TRUE
                case "GENERAL"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabSonstigesAllgemein"
                    If TabSonstigesAllgemein.exists(2) then bCorPage = TRUE
                case "MEMORY"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabMemory"
                    If TabMemory.exists(2) then bCorPage = TRUE
                case "VIEW"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 4 )
                    bFirst = FALSE
                    Kontext "TabAnsichtAllgemein"
                    If TabAnsichtAllgemein.exists(2) then bCorPage = TRUE
                case "PRINT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 5 )
                    bFirst = FALSE
                    Kontext "TabPrintStarOffice"
                    if TabPrintStarOffice.exists(2) then bCorPage = TRUE
                case "PATHS"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 6 )
                    bFirst = FALSE
                    Kontext "TabPfade"
                    If TabPfade.exists(2) then bCorPage = TRUE
                case "COLORS"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 7 )
                    bFirst = FALSE
                    Kontext "TabFarben"
                    If TabFarben.exists(4) then bCorPage = TRUE
                case "FONTREPLACEMENT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 8 )
                    bFirst = FALSE
                    Kontext "TabSchriftErsetzung"
                    If TabSchriftErsetzung.exists(2) then bCorPage = TRUE
                case "SECURITY"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 9 )
                    bFirst = FALSE
                    Kontext "TabSecurity"
                    If TabSecurity.exists(2) then bCorPage = TRUE
                case "APPEARANCE"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 10 )
                    bFirst = FALSE
                    Kontext "TabAppearance"
                    If TabAppearance.exists(2) then bCorPage = TRUE
                case "ACCESSIBILITY"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 11 )
                    bFirst = FALSE
                    Kontext "TabAccessibility"
                    If TabAccessibility.exists(2) then bCorPage = TRUE
                case "JAVA"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 12 )
                    bFirst = FALSE
                    Kontext "TabJava"
                    If TabJava.exists(2) then bCorPage = TRUE
                case "ONLINEUPDATE"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 13 )
                    bFirst = FALSE
                    Kontext "TabOnlineUpdate"
                    If TabOnlineUpdate.exists(2) then bCorPage = TRUE
                case "IMPROVEMENT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 14 )
                    bFirst = FALSE
                    Kontext "TabOOoImprovement"
                    If TabOOoImprovement.exists(2) then bCorPage = TRUE
                case else :  bCorPage = FALSE
                end select
            case "LOADSAVE"
                '///Pages in group &quot;LoadSave&quot;are:
                '///+<ul><li>&quot;General&quot;</li>
                '///+<li>&quot;VBAProperties&quot;</li>
                '///+<li>&quot;MicrosoftOffice&quot;</li>
                '///+<li>&quot;HTMLCompatibility&quot;</li></ul>
                select case Tabpagename
                case "GENERAL"
                    Kontext "TabSpeichern"
                    if TabSpeichern.exists then bCorPage = TRUE
                case "VBAPROPERTIES"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabMicrosoftOffice97"
                    If WinwordBasicLaden.exists(4) then bCorPage = TRUE
                case "MICROSOFTOFFICE"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabMicrosoftOffice2"
                    If Auswahl.exists(4) then bCorPage = TRUE
                case "HTMLCOMPATIBILITY"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 4 )
                    bFirst = FALSE
                    Kontext "TabHtml"
                    If TabHtml.exists(2) then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "LANGUAGESETTINGS"
                '///Pages in group &quot;LanguageSettings&quot;are:
                '///+<ul><li>&quot;Language&quot;</li>
                '///+<li>&quot;WritingAids&quot;</li>
                '///+<li>&quot;SearchingJapanese&quot;</li>
                '///+<li>&quot;AsianLayout&quot;</li></ul>
                '///+<li>&quot;Complex Text Layout&quot;</li></ul>
                Kontext "Active"
                if Active.Exists then Active.OK
                Kontext "ExtrasOptionenDlg"
                select case Tabpagename
                case "LANGUAGES"
                    Kontext "TabSprachen"
                    If TabSprachen.exists(2) then bCorPage = TRUE
                case "WRITINGAIDS"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabLinguistik"
                    If TabLinguistik.exists(2) then bCorPage = TRUE
                case "SEARCHINGINJAPANESE"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabJapaneseFind"
                    if TabJapaneseFind.exists(2) then bCorPage = TRUE
                case "ASIANLAYOUT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 4 )
                    bFirst = FALSE
                    Kontext "TabAsianLayoutOptions"
                    if TabAsianLayoutOptions.exists(2) then bCorPage = TRUE
                case "COMPLEXTEXTLAYOUT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 5 )
                    bFirst = FALSE
                    Kontext "TabComplexTextLayout"
                    if TabComplexTextLayout.exists(2) then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "INTERNET"
                '///Pages in group &quot;Internet&quot;are:
                '///+<ul><li>&quot;Proxy&quot;</li>
                '///+<li>&quot;Search&quot;</li></ul>
                Kontext "Active"
                if Active.Exists then Active.OK
                Kontext "ExtrasOptionenDlg"
                select case Tabpagename
                case "PROXY"
                    Kontext "TabProxyServer"
                    if TabProxyServer.exists(2) then bCorPage = TRUE
                case "SEARCH"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabSuche"
                    If TabSuche.exists(2) then bCorPage = TRUE
                case "EMAIL"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabEmail"
                    If TabEmail.exists(2) then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "WRITER"
                '///Pages in group &quot;TextDocument&quot;are:
                '///+<ul><li>&quot;General&quot;</li>
                '///+<li>&quot;View&quot;</li>
                '///+<li>&quot;FormattinAids&quot;</li>
                '///+<li>&quot;Grid&quot;</li>
                '///+<li>&quot;BasicFonts&quot;</li>
                '///+<li>&quot;BasicFontsAsian&quot;</li>
                '///+<li>&quot;BasicFontsCTL&quot;</li>
                '///+<li>&quot;Print&quot;</li>
                '///+<li>&quot;Table&quot;</li>
                '///+<li>&quot;Changes&quot;</li></ul>
                select case Tabpagename
                case "GENERAL"
                    Kontext "TabLaden"
                    If TabLaden.exists then bCorPage = TRUE
                case "VIEW"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabInhalteWriter"
                    If TabInhalteWriter.exists(2) then bCorPage = TRUE
                case "FORMATTINGAIDS"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabDirektCursor"
                    If TabDirektCursor.exists(2) then bCorPage = TRUE
                case "GRID"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 4 )
                    bFirst = FALSE
                    Kontext "TabRaster"
                    If TabRaster.exists(2) then bCorPage = TRUE
                case "BASICFONTS"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 5 )
                    bFirst = FALSE
                    Kontext "TabGrundschriften"
                    If TabGrundschriften.exists(2) then bCorPage = TRUE
                case "BASICFONTSASIAN"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 6 )
                    bFirst = FALSE
                    Kontext "TabGrundschriftenASIAN"
                    If TabGrundschriftenASIAN.exists(2) then bCorPage = TRUE
                case "BASICFONTSCTL"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 6 )
                    bFirst = FALSE
                    Kontext "TabGrundschriftenCTL"
                    If TabGrundschriftenCTL.exists(2) then bCorPage = TRUE
                case "PRINT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 6 )
                    bFirst = FALSE
                    Kontext "TabDruckenWriter"
                    If TabDruckenWriter.exists(2) then bCorPage = TRUE
                case "TABLE"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 7 )
                    bFirst = FALSE
                    Kontext "TabTabelleOptionen"
                    If TabTabelleOptionen.exists(2) then bCorPage = TRUE
                case "CHANGES"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 8 )
                    bFirst = FALSE
                    Kontext "TabAenderungenWriter"
                    If TabAenderungenWriter.exists(2) then bCorPage = TRUE
                case "COMPATIBILITY"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 9 )
                    bFirst = FALSE
                    Kontext "TabCompatibility"
                    If TabCompatibility.exists(2) then bCorPage = TRUE
                case "AUTOCAPTION"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 10 )
                    bFirst = FALSE
                    Kontext "TabAutoCaption"
                    If TabAutoCaption.exists(2) then bCorPage = TRUE
                case "MAILMERGEEMAIL"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 11 )
                    bFirst = FALSE
                    Kontext "TabMailMergeEMail"
                    If TabMailMergeEMail.exists(2) then bCorPage = TRUE

                case else : bCorPage = FALSE
                end select
            case "HTML"
                '///Pages in group &quot;HTMLDocument&quot;are:
                '///+<ul><li>&quot;General&quot;</li>
                '///+<li>&quot;View&quot;</li>
                '///+<li>&quot;Grid&quot;</li>
                '///+<li>&quot;Print&quot;</li>
                '///+<li>&quot;Table&quot;</li>
                '///+<li>&quot;Source&quot;</li>
                '///+<li>&quot;Background&quot;</li></ul>
                select case Tabpagename
                case "VIEW"
                    Kontext "TabInhalteHTML"
                    If TabInhalteHTML.exists then bCorPage = TRUE
                case "FORMATTINGAIDS"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabDirektCursor"
                    If TabDirektCursor.exists(2) then bCorPage = TRUE
                case "GRID"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabRaster"
                    If TabRaster.exists(2) then bCorPage = TRUE
                case "PRINT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 4 )
                    bFirst = FALSE
                    Kontext "TabDruckenHTML"
                    If TabDruckenHTML.exists(2) then bCorPage = TRUE
                case "TABLE"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 5 )
                    bFirst = FALSE
                    Kontext "TabTabelleOptionen"
                    If TabTabelleOptionen.exists(2) then bCorPage = TRUE
                case "BACKGROUND"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 6 )
                    bFirst = FALSE
                    Kontext "TabHintergrund"
                    If TabHintergrund.exists then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "CALC"
                '///Pages in group &quot;Spreadsheet&quot;are:
                '///+<ul><li>&quot;General&quot;</li>
                '///+<li>&quot;View&quot;</li>
                '///+<li>&quot;Calculate&quot;</li>
                '///+<li>&quot;Sortlists&quot;</li>
                '///+<li>&quot;Changes&quot;</li>
                '///+<li>&quot;Grid&quot;</li>
                '///+<li>&quot;Print&quot;</li></ul>
                select case Tabpagename
                case "GENERAL"
                    Kontext "TabLayoutCalc"
                    If TabLayoutCalc.exists(2) then bCorPage = TRUE
                case "VIEW"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabInhaltCalc"
                    If TabInhaltCalc.exists(2) then bCorPage = TRUE
                case "CALCULATE"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabBerechnen"
                    If TabBerechnen.exists(2) then bCorPage = TRUE
                case "SORTLISTS"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 4 )
                    bFirst = FALSE
                    Kontext "TabSortierlisten"
                    If TabSortierlisten.exists(2) then bCorPage = TRUE
                case "CHANGES"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 5 )
                    bFirst = FALSE
                    Kontext "TabAenderungenCalc"
                    If TabAenderungenCalc.exists(2) then bCorPage = TRUE
                case "GRID"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 6 )
                    bFirst = FALSE
                    Kontext "TabRaster"
                    If TabRaster.exists(2) then bCorPage = TRUE
                case "PRINT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 7 )
                    bFirst = FALSE
                    Kontext "TabPrintCalcOptions"
                    If TabPrintCalcOptions.exists(2) then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "IMPRESS"
                '///Pages in group &quot;Presentation&quot;are:
                '///+<ul><li>&quot;General&quot;</li>
                '///+<li>&quot;View&quot;</li>
                '///+<li>&quot;Grid&quot;</li>
                '///+<li>&quot;Print&quot;</li></ul>
                select case Tabpagename
                case "GENERAL"
                    Kontext "TabSonstigesDraw"
                    If TabSonstigesDraw.exists then bCorPage = TRUE
                case "VIEW"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabInhalteDraw"
                    If TabInhalteDraw.exists(2) then bCorPage = TRUE
                case "GRID"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabRaster"
                    If TabRaster.exists(2) then bCorPage = TRUE
                case "PRINT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 4 )
                    bFirst = FALSE
                    Kontext "TabDruckenDraw"
                    If TabDruckenDraw.exists(2) then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "DRAW"
                '///Pages in group &quot;Drawing&quot;are:
                '///+<ul><li>&quot;General&quot;</li>
                '///+<li>&quot;View&quot;</li>
                '///+<li>&quot;Grid&quot;</li>
                '///+<li>&quot;Print&quot;</li></ul>
                select case Tabpagename
                case "GENERAL"
                    Kontext "TabSonstigesDraw"
                    If TabSonstigesDraw.exists then bCorPage = TRUE
                case "VIEW"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 2 )
                    bFirst = FALSE
                    Kontext "TabInhalteDraw"
                    If TabInhalteDraw.exists(2) then bCorPage = TRUE
                case "GRID"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 3 )
                    bFirst = FALSE
                    Kontext "TabRaster"
                    If TabRaster.exists(2) then bCorPage = TRUE
                case "PRINT"
                    if bFirst = TRUE then Optionsliste.Select ( startpos + 4 )
                    bFirst = FALSE
                    Kontext "TabDruckenDraw"
                    If TabDruckenDraw.exists(2) then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "MATH"
                '///Page(s) in group &quot;Formula&quot;are:
                '///+<ul><li>&quot;Settings&quot;</li></ul>
                select case Tabpagename
                case "SETTINGS"
                    Kontext "TabDruckenMath"
                    If TabDruckenMath.exists then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "CHART"
                '///Pages in group &quot;Chart&quot;are:
                '///+<ul><li>&quot;DefaultColors&quot;</li></ul>
                select case Tabpagename
                case "DEFAULTCOLORS"
                    Kontext "TabGrundfarben"
                    If TabGrundfarben.exists(4) then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case "DATASOURCES"
                '///Pages in group &quot;Datasource&quot;are:
                '///+<ul><li>&quot;Connetions&quot;</li></ul>
                select case Tabpagename
                case "CONNECTIONS"
                    Kontext "TabConnections"
                    If TabConnections.exists(4) then bCorPage = TRUE
                case "DATABASES"
                    Kontext "TabRegisteredDatabase"
                    If TabRegisteredDatabase.exists(4) then bCorPage = TRUE
                case else : bCorPage = FALSE
                end select
            case else
                bCorPage = False
            end select
            If bCorPage = TRUE then Exit for
        next iCurrentPage
        if ( NOT bCorPage ) then
            if ( NOT bSilent ) then
                warnlog CFN & Applicationname + " / " + Tabpagename + " could not be found! Please review your test!"
            else
                printlog CFN & Applicationname + " / " + Tabpagename + " could not be found!"
            end if
        end if
    else
        bCorPage = FALSE
        warnlog CFN & "Please open the option-dialog with 'ToolsOptions' before you called this routine!"
    end if
    hToolsOptions = bCorPage
end function

