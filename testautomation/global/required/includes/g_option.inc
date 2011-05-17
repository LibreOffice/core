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
'* short description : update and resouce test for all option pages (at tools/options)
'*
'\***********************************************************************

global sErrMes as String
global iSectionNumber as Integer

sub tToolsOptionsTest
    ' To prevent users from running just a subsection from this file, without THIS
    ' subroutine, in every subsection the value of 'iSectionNumber' is checked.
    ' The test will exit with warnlog, if iSectionNumber is 0
    sErrMes = ""
    select case uCase(gApplication)
       case "WRITER","MASTERDOCUMENT","HTML"   : iSectionNumber = 8
       case "IMPRESS","DRAW","MATH","CALC"     : iSectionNumber = 7
    end select
    ' common testcases are placed in this library
    ' these tab-sets are always available:
    Call tToolsOptionsStarOffice
    Call tToolsOptionsLoadSave
    Call tToolsOptionsLanguageSettings
    Call tToolsOptionsDataSources
    Call tToolsOptionsChart
    Call tToolsOptionsInternet
    Call tToolsOptionsImprovementProgram
end sub

'-------------------------------------------------------------------------

testcase tToolsOptionsStarOffice

    dim EchteAnzahl as Integer, i as Integer
    dim gefunden as Boolean, bSave as Boolean
    dim iMenuShould as integer
    '/// We have 12 tab pages in this category. If the <i>central configuration</i> feature is being used 13 (<i>Single SignOn</i>)
    '/// <i>Tools</i> / <i>Options</i> / <i>OpenOffice.org</i>
    PrintLog "- Tools/Options/StarOffice  ( Extras/Optionen/StarOffice )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    
    if ( gApplication = "BACKGROUND" ) then 
        ' Do not open any document
    else
        Call hNewDocument
    endif
    
    ToolsOptions
    ToPosInOptionlist ( 1 )
    Kontext "ExtrasOptionenDlg"
    EchteAnzahl = Optionsliste.GetItemCount - iSectionNumber
    iMenuShould = 14

    if EchteAnzahl <> iMenuShould then
        if ((gtplatform = "sol") OR (gtplatform = "x86") OR gOOO) AND EchteAnzahl = 12 then
            printLog "One Tabpage is missing. This is ok for a patched Solaris (sparc and x86) or OOo installation."
        else
            Warnlog "The number of entries is not " + iMenuShould + "! => the current number is " + EchteAnzahl
        endif
    endif
    
    Kontext "Active"
    try
        if Active.Exists then Active.OK
    catch
        if Active.Exists then Active.No
    endcatch
    
    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        select case i
        case 1 :
            Kontext "TabAllgemeines"
            '///+<ul><li>User data</li>
            printlog "  - Userdata / Benutzerdaten"
            if TabAllgemeines.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabAllgemeines )
            endif
        case 2 :
            Kontext "TabSonstigesAllgemein"
            '///+<li>General</li>
            printlog "  - General / Allgemein"
            if TabSonstigesAllgemein.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabSonstigesAllgemein )
            endif
        case 3 :
            Kontext "TabMemory"
            '///+<li>Memory</li>
            printlog "  - Memory / Arbeitsspeicher"
            if TabMemory.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabMemory )
            endif
        case 4 :
            Kontext "TabAnsichtAllgemein"
            '///+<li>View</li>
            printlog "  - View / Ansicht"
            if TabAnsichtAllgemein.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabAnsichtAllgemein )
            endif
        case 5 :
            Kontext "TabPrintStarOffice"
            '///+<li>Print</li>
            printlog "  - Print / Drucken"
            if TabPrintStarOffice.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabPrintStarOffice )
            endif
        case 6 :
            Kontext "TabPfade"
            '///+<li>Paths
            '///+ <ul><li>Click <i>edit</i> to all list entries and then close the file dialog</li></ul></li>
            printlog "  - Paths / Pfade"
            if TabPfade.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabPfade )
                Call DialogeFuerTypenKontrollieren
            endif
        case 7 :
            Kontext "TabFarben"
            '///+<li>Colors
            '///+<ul><li>Click on <i>Add</i></li>
            '///+<li>Click <i>OK</i> on message box</li>
            '///+<li>Click <i>cancel> on <i>Name</i> dialog</li>
            '///+<li>Click on <i>Edit</i></li>
            '///+<li>Click on <i>Close</i> in <i>Colors</i> dialog</li></ul></li>
            printlog "  - Color / Farben"
            if ( TabFarben.exists( 4 ) ) then
                if TabFarben.IsVisible then
                    gefunden = TRUE
                    Call DialogTest (TabFarben)
                    Farbe.Select(1)
                    Hinzufuegen.Click
                    Kontext "Active"
                    Active.OK
                    Kontext "NameDlg"
                    Call DialogTest (NameDlg)
                    NameDlg.Cancel
                    Kontext "TabFarben"
                    Bearbeiten.Click
                    WaitSlot( 3000 )
                    Kontext "FarbeDlg"
                    FarbeDlg.Cancel
                endif
            endif
        case 8 :
            Kontext "TabSchriftErsetzung"
            '///+<li>Font replacement</li>
            printlog "  - Font Replacement / Schriftersetzung"
            if TabSchriftErsetzung.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabSchriftErsetzung )
            endif
        case 9:
            Kontext "TabSecurity"
            '///+<li>Security</li>
            printlog "  - Security / Sicherheit"
            if TabSecurity.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabSecurity )
                MacroSecurity.click()

                kontext "TabSecurityLevel"
                if ( TabSecurityLevel.exists( 2 ) ) then
                    call DialogTest( TabSecurityLevel )
                    kontext
                    active.setpage TabTrustedSources
                    kontext "TabTrustedSources"
                    call DialogTest( TabTrustedSources )
                    TabTrustedSources.cancel
                else
                    warnlog( "Macro Security Options dialog is missing" )
                    qaerrorlog( "#i45295# or office built without mozilla support" )
                endif

                Kontext "TabSecurity"
                if ( TabSecurity.exists( 1 ) ) then
                    Options.click
                    Kontext "TabSecurityOptionsAndWarnings"
                    Call DialogTest(TabSecurityOptionsAndWarnings)
                    TabSecurityOptionsAndWarnings.cancel
                else
                    warnlog( "Tabsecurity did not open" )
                endif
                
            endif
        case 10:
            Kontext "TabAppearance"
            '///+<li>Appearance</li>
            printlog "  - Appearance / Aussehen"
            if TabAppearance.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabAppearance )
            endif
        case 11:
            Kontext "TabAccessibility"
            '///+<li>Accessibility</li>
            printlog "  - Accessibility"
            if TabAccessibility.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabAccessibility )
            endif
        case 12:
            Kontext "TabJava"
            '///+<li>Java</li></ul>
            printlog "  - Java"
            if TabJava.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabJava )
            endif
            if useJava.isChecked then
                Add.click
                Kontext "OeffnenDlg"
                if ( OeffnenDlg.exists( 1 ) ) then
                    OeffnenDlg.Cancel
                else
                    warnlog( "OeffnenDlg is missing" )
                endif
                Kontext "TabJava"
                Parameters.click
                Kontext "JavaStartParameter"
                Call DialogTest ( JavaStartParameter )
                JavaStartParameter.cancel
                Kontext "TabJava"
                ClassPath.click
                Kontext "JavaClassPath"
                Call DialogTest ( JavaClassPath )
                JavaClassPath.cancel
            else
                printlog "Use Java is disabled."
            endif
        case 13:
            kontext "TabOnlineUpdate"
            '///+<li>Online update</li>
            printlog "  - Online update"
            if TabOnlineUpdate.isVisible then
                gefunden = TRUE
                Call DialogTest( TabOnlineUpdate )
            endif
            'CheckNow.click
        case 14:
            Kontext "TabOOoImprovement"
            printlog( "  - OpenOffice.org Improvement Program" )
            if ( TabOOoImprovement.isVisible() ) then
                gefunden = TRUE
                Call DialogTest( TabOOoImprovement )
            endif
        end select
        if ( NOT gefunden ) then
            Warnlog "Error with the " & i & ".page, perhaps this page has been deleted or changed its position"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 13 then
                i=10
            else
                i=1-1
            endif
        endif
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    if ( OptionenDlg.exists( 1 ) ) then
        OptionenDlg.Cancel
    else
        warnlog( "Options Dialog not ready" )
    endif
    '/// Close the document
    if ( gApplication = "BACKGROUND" ) then
        ' We should be on the backing window
    else
        Call hCloseDocument()
    endif
    
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsLoadSave

    dim EchteAnzahl as Integer, i as Integer
    dim gefunden as Boolean
    PrintLog "- Tools/Options/Load Save  ( Extras/Optionen/Laden Speichern )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    if gApplication <> "BACKGROUND" then Call hNewDocument
    '/// <i>Tools</i> / <i>Options</i> / <i>Load/Save</i>
    ToolsOptions
    ToPosInOptionlist (2)
    EchteAnzahl = OptionTabPageZaehler (4, FALSE)
    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        select case i
        case 1 :
            Kontext "TabSpeichern"
            '///+<ul><li>General</li>
            printlog "  - General / Save"
            if TabSpeichern.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabSpeichern )
            endif
        case 2 :
            Kontext "TabMicrosoftOffice97"
            '///+<li>VBA settings</li>
            printlog "  - VBA Settings / VBA Eigenschaften"
            if TabMicrosoftOffice97.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabMicrosoftOffice97 )
            endif
        case 3 :
            Kontext "TabMicrosoftOffice2"
            '///+<li>Microsoft Office</li>
            printlog "  - Microsoft Office"
            if TabMicrosoftOffice2.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabMicrosoftOffice2 )
            endif
        case 4 :
            Kontext "TabHTML"
            '///+<li>HTML Compatibility</li></ul>
            printlog "  - HTML Compatibility / HTML Kompatibilitaet"
            if TabHTML.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabHTML )
            endif
        end select
        if gefunden=FALSE then
            Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 5 then
                i=11
            else
                i=1-1
            endif
        endif
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    endif
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsLanguageSettings

    dim EchteAnzahl as Integer, i as Integer, j as Integer
    dim gefunden as Boolean
    dim bAsianSup as Boolean

    PrintLog "- Tools/Options/Language Settings  ( Extras/Optionen/Spracheinstellungen )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    if gApplication <> "BACKGROUND" then Call hNewDocument
    '/// <i>Tools</i> / <i>Options</i> / <i>Language settings</i>
    ToolsOptions
    ToPosInOptionlist ( 3 )
    EchteAnzahl = Optionsliste.GetItemCount - iSectionNumber
    '/// We only have two tab pages in this section:
    '///+ <ul><li><i>Languages</i> and <i>Writing Aids</i></li>
    '///+<ol><li>In asian languages we have two additional tab pages:
    '///+ <ul><li><i>Japanese Find</i> and <i>Asian Layout</i></li></ul></li>
    '///+<li>In languages with CTL (<i>Complex Text Layout</i>) there is also one more:
    '///+<ul><li><i>Complex Text Layout</i></li></ul></li></ol></li></ul>
    select case (EchteAnzahl)
    case 2:
        printlog "Asian or Complex Text Layout support is not activated (on languages-page)!"
    case 3:
        printlog "Complex Text Layout support is activated (on languages-page)!"
    case 4:
        printlog "Asian support is activated (on languages-page)!"
    case 5:
        if (gAsianSup AND gCTLSup) then
            printlog "Asian and Complex Text Layout support is activated (on languages-page)!"
        else
            Warnlog "There are more than 4 pages!"
        endif
    case else
        Warnlog "There are more than 5 pages!"
    end select
    Kontext "Active"
    try
        if Active.Exists then Active.OK
    catch
        if Active.Exists then Active.No
    endcatch
    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        select case i
        case 1 :
            Kontext "TabSprachen"
            'Languages
            printlog "  - Languages / Sprachen"
            if TabSprachen.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabSprachen )
            endif
        case 2 :
            Kontext "TabLinguistik"
            printlog "  - Writing Aids / Linguistik"
            if TabLinguistik.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabLinguistik )
                '/// In <i>Writing aids</i>
                '///+<ul><li> On <i>modules<i> click <i>edit</i></li>
                SprachmoduleBearbeiten.Click
                Kontext "ModuleBearbeiten"
                Call DialogTest ( ModuleBearbeiten )
                ModuleBearbeiten.Close
                '///+<li> close dialog</li></ul>
                Kontext "TabLinguistik"
                '///+<ul><li>On <i>dictionaries</i> click <i>new</i></li>
                '///+<li>close dialog</li></ul>
                Neu.Click
                Kontext "BenutzerwoerterbuchAnlegen"
                Call DialogTest ( BenutzerwoerterbuchAnlegen )
                BenutzerwoerterbuchAnlegen.Cancel

                '///+<ul><li>On <i>dictionaries</i> click <i>edit</i></li>
                '///+<li>close dialog</li></ul>
                Kontext "TabLinguistik"
                for j=1 to 5
                    if Bearbeiten.IsEnabled then
                        Bearbeiten.Click
                        j=6
                    else
                        Benutzerwoerterbuch.TypeKeys "<Down>"
                    endif
                next j
                Kontext "BenutzerwoerterbuchBearbeiten"
                if BenutzerwoerterbuchBearbeiten.Exists then
                    Call DialogTest ( BenutzerwoerterbuchBearbeiten )
                    BenutzerwoerterbuchBearbeiten.Close
                else
                    warnlog "Edit the 'user-defined dictionary' is not possible => Bug?"
                endif
                '/// <i>Options</i>
                '///+<ul><li>On 9th, 10th and 11th entry click <i>edit</i></li>
                '///+<li>close dialog</li></ul>
                Kontext "TabLinguistik"
                Optionen.TypeKeys "<PageUp>", 3
                WaitSlot() ' similar effect as a NOOP
                Optionen.TypeKeys "<Down>", 6
                WaitSlot() ' similar effect as a NOOP
                OptionenBearbeiten.Click
                Kontext "HyphenationOptions"
                Call DialogTest ( HyphenationOptions )
                HyphenationOptions.Cancel
                Kontext "TabLinguistik"
                Optionen.TypeKeys "<Down>"
                OptionenBearbeiten.Click
                Kontext "HyphenationOptions"
                Call DialogTest ( HyphenationOptions )
                HyphenationOptions.Cancel
                Kontext "TabLinguistik"
                Optionen.TypeKeys "<Down>"
                OptionenBearbeiten.Click
                Kontext "HyphenationOptions"
                Call DialogTest ( HyphenationOptions )
                HyphenationOptions.Cancel
            endif
        case 3,5 :
            try
                Kontext "TabJapaneseFind"
                'Japanese find
                if TabJapaneseFind.IsVisible then
                    printlog "  - Japanese Find / Japanische Suche"
                    gefunden = TRUE
                    Call DialogTest ( TabJapaneseFind )
                endif
            catch
            endcatch
            try
                Kontext "TabComplexTextLayout"
                'Complex Text Layout
                if TabComplexTextLayout.IsVisible then
                    printlog "  - Complex Text Layout"
                    gefunden = TRUE
                    Call DialogTest ( TabComplexTextLayout )
                endif
            catch
            endcatch
        case 4 :
            Kontext "TabAsianLayoutOptions"
            'Asian layout
            printlog "  - Asian Layout / Asiatisches Layout"
            if TabAsianLayoutOptions.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabAsianLayoutOptions )
            endif
        end select
        if gefunden=FALSE then
            Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 5 then
                i=5
            else
                i=1-1
            endif
        endif
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    endif
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsInternet

    dim EchteAnzahl as Integer, i as Integer
    dim gefunden as Boolean

    PrintLog "- Tools/Options/Internet  ( Extras/Optionen/Internet )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    if gApplication <> "BACKGROUND" then Call hNewDocument
    '/// <i>Tools</i> / <i>Options</i> / <i>Internet</i>
    ToolsOptions
    '/// (The last section in Tools / Options is <i>Internet</i>)
    ToPosInOptionlist ( iSectionNumber)

    if gPlatgroup = "w95" OR gPlatform = lcase("osx") then
        '/// On MacOS X systems this section includes the tab pages:
        '///+<ol><li>Proxy</li>
        '///+<li>Search</li>
        '///+<li>E-Mai</li></ol>
        '/// On Win32 systems this section includes the tab pages:
        '///+<ol><li>Proxy</li>
        '///+<li>Search</li>
        '///+<li>Mozilla Plugin</li></ol>
        EchteAnzahl = OptionTabPageZaehler ( 3 , TRUE )
        if (EchteAnzahl > 3) then
			qaErrorLog ("Due to some installed extensions the tabpage count is not 3 it is: " + EchteAnzahl)
        else 
            if (EchteAnzahl < 3) then
                warnlog "Number of tabpages old :   3   new :  " + EchteAnzahl
            endif
        endif
    else
        '/// On Unix systems this section includes four tab pages.
        '///+<ol><li>Proxy</li>
        '///+<li>Search</li>
        '///+<li>E-Mail</li>
        '///+<li>Mozilla Plugin</li></ol>
        EchteAnzahl = OptionTabPageZaehler ( 4 , TRUE )
        if (EchteAnzahl > 4) then
			qaErrorLog ("Due to some installed extensions the tabpage count is not 3 it is: " + EchteAnzahl)
        else 
            if (EchteAnzahl < 4) then
               warnlog "Number of tabpages old :   3   new :  " + EchteAnzahl
            endif
        endif
    endif

    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"

        select case i
        case 1 :
            Kontext "TabProxyServer"
            'Proxy
            printlog "  - Proxy / Proxy"
            if TabProxyServer.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabProxyServer )
            endif
        case 2 :
            Kontext "Active"
            'Search
            printlog "  - Search / Suche"
            if Active.Exists then
                sErrMes = Active.GetText
                Warnlog "A messagebox exists => BUG  : " + sErrMes
                Active.OK
            endif
            Kontext "TabSuche"
            if TabSuche.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabSuche )
            endif
        case 3 :
            if gPlatgroup = "unx" then
                Kontext "TabEmail"
                'E-mail
                printlog "  - E-mail"
                if TabEmail.IsVisible then
                    gefunden = TRUE
                    Call DialogTest ( TabEmail )
                endif
            endif
            if gPlatgroup = "w95" then
                Kontext "TabMozillaPlugin"
                'Mozilla Plug-in (On Win32)
                printlog "  - Mozilla Plug-in"
                if TabMozillaPlugin.IsVisible then
                    gefunden = TRUE
                    Call DialogTest ( TabMozillaPlugin )
                endif
            endif
        case 4 :
            if gPlatgroup = "w95" then
                warnlog "On Win32 platform there should be only 3 tab pages in section 'Internet'!?"
            else
                Kontext "TabMozillaPlugin"
                printlog "  - Mozilla Plug-in"
                if TabMozillaPlugin.IsVisible then
                    gefunden = TRUE
                    Call DialogTest ( TabMozillaPlugin )
                endif
            endif
        case 5,6 :  Kontext "TabMediaWiki"
                    if TabMediaWiki.exists then
                        qaerrorlog "  - Wiki Publisher"
                        Call DialogTest ( TabMediaWiki )
                        gefunden = TRUE
                    endif
                    Kontext "TabWeblog"
                    if TabWeblog.exists then
                        qaerrorlog "  - Weblog Publisher"
                        Call DialogTest ( TabWeblog )
                        gefunden = TRUE
                    endif
        end select
        if gefunden=FALSE then
            Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 4 then
                i=11
            else
                i=1-1
            endif
        endif
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    endif
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsImprovementProgram

    printlog( "Update test for the tools/options dialog of the OOo Improvement Program" )

    if (gOOO AND NOT gOOoImprovementIsEnabled) then
        'In StarOffice this test should always work - but on OOo contributet builds the function is not available - only on OOo builds provided by Sun Microsystems
        qaErrorlog ("This build does not participate in the OOo Improvement Program - test exit")
        goto endsub
    endif

    dim cLogFile as string
        cLogFile = gOfficePath & "user\temp\feedback\Current.csv"
        cLogFile = convertpath( cLogFile )
        
    dim bImportDialogHasBeenDisplayed as boolean : bImportDialogHasBeenDisplayed = false

    if ( hDeleteFile( cLogFile ) ) then
    
        printlog( "Test environment is clean, no logfile present (anymore)" )

        ToolsOptions
        hToolsOptions( "StarOffice", "Improvement" )
        printlog( "Improvement Program tabpage" )

        call DialogTest( TabOOoImprovement )

        ParticipateNo.check()
        ParticipateYes.check()

        if ( ShowData.isEnabled() ) then

            ShowData.click()

            ' Two different IDs for practically the same dialog: FilterAuswahl and TextImport.
            ' It depends on the application which one comes up. We just handle them both.
            Kontext "FilterAuswahl"
            if ( FilterAuswahl.exists( 1 ) ) then
                printlog( "FilterAuswahl present" )
                call DialogTest( FilterAuswahl )
                FilterAuswahl.cancel()
                bImportDialogHasBeenDisplayed = true
            endif
            
            kontext "TextImport"
            if ( TextImport.exists() ) then
                printlog( "Text Import dialog present" )
                call DialogTest( TextImport )
                TextImport.cancel()
                bImportDialogHasBeenDisplayed = true
            endif
        else
            printlog( "Show data is disabled" )
        endif

        ' Weird messages may appear in any order. We warn about anything
        Kontext "Active"
        if ( Active.exists( 1 ) ) then
            printlog( "Messagebox: " & Active.getText() )
            Active.ok()
        else
            if ( NOT bImportDialogHasBeenDisplayed ) then
                warnlog( "#i116461# Show Data button enabled but no logged data found" )
            endif
        endif

        Kontext "Active"
        if ( Active.exists( 1 ) ) then
            warnlog( "#97342# Second message for missing " & cLogFile )
            Active.ok()
        endif

        Kontext "OptionenDlg"
        if OptionenDlg.Exists then
            OptionenDlg.cancel()
        endif

        if GetDocumentCount = 1 then
            Call hCloseDocument
        endif
    else
        ' hDeleteFile() returns FALSE if the file is still present for some weird reason
        ' We should warn about this because the test breaks at a later point then.
        warnlog( "Logfile present, it could not be deleted. Please check path and accessrights" )
        printlog( "Path: " & cLogFile )
    endif

endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsChart
    Dim EchteAnzahl as Integer, i as Integer

    PrintLog "- Tools/Options/Chart  ( Extras/Optionen/Diagramme )"
    if (0 = iSectionNumber) then 
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif    
'/// Create a new document    
    if gApplication <> "BACKGROUND" then 
        Call hNewDocument
    end if
'/// <i>Tools</i> / <i>Options</i> / <i>Chart</i>    
    ToolsOptions
    ToPosInOptionlist ( iSectionNumber -1 )
    EchteAnzahl = OptionTabPageZaehler ( 1, FALSE )
    Kontext "OptionenDlg"
    Optionsliste.TypeKeys "<Down>"
'/// Default colors
    printlog "  - Default Colors / Grundfarben"
    Kontext "TabGrundfarben"
    Call DialogTest ( TabGrundfarben )
'/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    if gApplication <> "BACKGROUND" then
'/// Close bthe document    
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsDataSources
   qaerrorlog "#i104508# crashes on quit or reload if this is run."
   goto endsub
   
   Dim EchteAnzahl as Integer, i as Integer

   PrintLog "- Tools/Options/Data Sources  ( Extras/Optionen/Datenbank )"
    if (0 = iSectionNumber) then 
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif   
    '/// Create a new document
    if gApplication <> "BACKGROUND" then    
        Call hNewDocument
    end if
    '/// <i>Tools</i> / <i>Options</i> / <i>Data source</i>
    ToolsOptions
    ToPosInOptionlist ( iSectionNumber -2)
    EchteAnzahl = OptionTabPageZaehler ( 2 , FALSE )
    Kontext "OptionenDlg"
    Optionsliste.TypeKeys "<Down>"
    printlog "  - Connections / Verbindungen"
    Kontext "TabConnections"
    '/// Connections
    Call DialogTest ( TabConnections )
    Kontext "OptionenDlg"
    Optionsliste.TypeKeys "<Down>"
    printlog "  - Databases / Datenbanken"   
    Kontext "TabRegisteredDatabase"
    '/// Databases
    Call DialogTest ( TabRegisteredDatabase )
    NewBtn.click
    Kontext "CreateDatabaseLink"
    call DialogTest( CreateDatabaseLink )
    Browse.click
    Kontext "OeffnenDlg"
    OeffnenDlg.cancel
    Kontext "CreateDatabaseLink"
    CreateDatabaseLink.cancel
    Kontext "OptionenDlg"
    '/// Close the options dialog
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then 
        Call hCloseDocument
    end if
endcase

