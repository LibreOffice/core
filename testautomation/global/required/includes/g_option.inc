'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_option.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-07-31 19:25:49 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : update and resouce test for all option pages (at tools/options)
'*
'\***********************************************************************

global sErrMes as String
global iSectionNumber as Integer

sub tToolsOptionsTest

    dim sPreviousApplication as string

    sPreviousApplication = gApplication
    ' subs and functions has been placed there
    use "global/tools/includes/required/t_option2.inc"

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

    ' these tabpages only exist in the respective application
    select case uCase(gApplication)
    case "WRITER"  : Call tToolsOptionsTextdocument
    case "HTML"    : Call tToolsOptionsHTMLDocument
    case "CALC"    : Call tToolsOptionsSpreadsheet
    case "IMPRESS" : Call tToolsOptionsPresentation
    case "DRAW"    : Call tToolsOptionsDrawing
    case "MATH"    : Call tToolsOptionsFormula
    end select

    gApplication = sPreviousApplication
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
                if Protect.isEnabled then
                    Protect.click()
                    Kontext "PasswordDLG"
                    call DialogTest( PasswordDLG )
                    PasswordDLG.cancel()
                else
                    select case uCase(gApplication)
                    case "WRITER","MASTERDOCUMENT"
                        WarnLog "Protect-Button is disabled."
                    case "MATH","IMPRESS","DRAW","HTML","CALC"
                        PrintLog "Protect-Button is disabled."
                    end select
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
                Kontext "SilbentrennungOptionen"
                Call DialogTest ( SilbentrennungOptionen )
                SilbentrennungOptionen.Cancel
                Kontext "TabLinguistik"
                Optionen.TypeKeys "<Down>"
                OptionenBearbeiten.Click
                Kontext "SilbentrennungOptionen"
                Call DialogTest ( SilbentrennungOptionen )
                SilbentrennungOptionen.Cancel
                Kontext "TabLinguistik"
                Optionen.TypeKeys "<Down>"
                OptionenBearbeiten.Click
                Kontext "SilbentrennungOptionen"
                Call DialogTest ( SilbentrennungOptionen )
                SilbentrennungOptionen.Cancel
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
        EchteAnzahl = OptionTabPageZaehler ( 3 , FALSE )
    else
        '/// On Unix systems this section includes four tab pages.
        '///+<ol><li>Proxy</li>
        '///+<li>Search</li>
        '///+<li>E-Mail</li>
        '///+<li>Mozilla Plugin</li></ol>
        EchteAnzahl = OptionTabPageZaehler ( 4 , FALSE )
    endif

    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        Kontext "ProxyWarnung"
        if ProxyWarnung.Exists(1) then
            NichtMehrAnzeigen.Check
            ProxyWarnung.OK
        endif

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

    dim cLogFile as string
    cLogFile = gOfficePath & "user\temp\feedback\Current.csv"
    cLogFile = convertpath( cLogFile )

    if ( hDeleteFile( cLogFile ) ) then
        printlog( "Test environment is clean, no logfile present (anymore)" )
    endif

    ToolsOptions
    hToolsOptions( "StarOffice", "Improvement" )
    printlog( "Improvement Program tabpage" )

    call DialogTest( TabOOoImprovement )

    ParticipateNo.check()
    ParticipateYes.check()

    if ( ShowData.isEnabled() ) then

        ShowData.click()

        Kontext "TextImport"
        if ( TextImport.exists( 1 ) ) then
            printlog( "Text import dialog" )

            call DialogTest( TextImport )
            TextImport.cancel()
        else
            qaerrorlog( "#i97340# Show Data button enabled but no logged data found" )
        endif

        Kontext "Active"
        if ( Active.exists( 1 ) ) then
            printlog( "Messagebox: " & Active.getText() )
            Active.ok()
        else
            warnlog( "No warning for missing file: " & cLogFile )
        endif

        Kontext "Active"
        if ( Active.exists( 1 ) ) then
            warnlog( "#97342# Second message for missing " & cLogFile )
            Active.ok()
        endif
    else
        printlog( "Show data is disabled" )
    endif

    Kontext "OptionenDlg"
    OptionenDlg.cancel()

endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsTextdocument
  dim EchteAnzahl as Integer, i as Integer
  dim gefunden as Boolean
    
     PrintLog "- Tools/Options/Text document  ( Extras/Optionen/Textdokument )"
    if (0 = iSectionNumber) then 
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
'/// Create a new document    
    if gApplication <> "BACKGROUND" then   
        Call hNewDocument
    end if
 '/// <i>Tools</i> / <i>Options</i> / <i>Text document</i>
   ToolsOptions
   Kontext "Active"
   if Active.Exists (3) then
      if Active.GetText <> sErrMes then Warnlog "A messagebox exists => BUG : " + Active.GetText
      Active.OK
   end if
   ToPosInOptionlist ( 4 )
   EchteAnzahl = Optionsliste.GetItemCount - iSectionNumber
   if EchteAnzahl = 10 AND ( gAsianSup = TRUE OR gCTLSup = TRUE ) then
      Warnlog "There are only 10 pages in 'text document'-section, but asian support or CTL support are activated! => a section is missing?"
   else
      printlog "Asian support and CTL support are not activated ( on languages-page )!"
   end if
   if bDebugVersion = TRUE  then EchteAnzahl = EchteAnzahl - 1
   if gAsianSup     = FALSE then EchteAnzahl = EchteAnzahl + 1
   if gCTLSup       = FALSE then EchteAnzahl = EchteAnzahl + 1
   for i=1 to EchteAnzahl
      gefunden = FALSE
      Kontext "OptionenDlg"
      Optionsliste.TypeKeys "<Down>"
      if gAsianSup = FALSE AND gCTLSup = FALSE AND i=6 then i=i+2
      if gAsianSup = FALSE AND gCTLSup = TRUE  AND i=6 then i=i+1
      if gAsianSup = TRUE  AND gCTLSup = FALSE AND i=7 then i=i+1
      select case i
       case 1 : Kontext "TabLaden"
                '///+<ul><li>General</li>
                printlog "  - General / Allgemein"
                if TabLaden.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabLaden )
                end if
       case 2 : Kontext "TabInhalteWriter"
                '///+<li>View</li>
                printlog "  - View / Ansicht"
                if TabInhalteWriter.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabInhalteWriter )
                end if
       case 3 : Kontext "TabDirektCursor"
                '///+<li>Formating aids</li>
                printlog "  - Formating aids / Formatierungshilfen"
                if TabDirektCursor.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabDirektCursor )
                end if
       case 4 : Kontext "TabRaster"
                '///+<li>Grid</li>
                printlog "  - Grid / Raster"
                if TabRaster.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabRaster )
                end if
       case 5 : Kontext "TabGrundschriften"
                '///+<li>Default fonts</li>
                printlog "  - Default Fonts / Grundschriften"
                if TabGrundschriften.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabGrundschriften )
                end if
       case 6 : Kontext "TabGrundschriftenAsian"
                '///+<li>Default fonts asian</li>
                printlog "  - Default Fonts Asian / Grundschriften Asiatisch"
                if TabGrundschriftenAsian.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabGrundschriftenAsian )
                end if
       case 7 : Kontext "TabGrundschriftenCTL"
                '///+<li>Default fonts CTL</li>
                printlog "  - Default Fonts CTL / Grundschriften CTL"
                if TabGrundschriftenCTL.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabGrundschriftenCTL )
                end if
       case 8 : Kontext "TabDruckenWriter"
                '///+<li>Print</li>
                printlog "  - Print / Drucken"
                if TabDruckenWriter.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabDruckenWriter )
                end if
       case 9 : Kontext "TabTabelleOptionen"
                '///+<li>Table</li>
                printlog "  - Table / Tabellen"
                if TabTabelleOptionen.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabTabelleOptionen )
                end if
       case 10: Kontext "TabAenderungenWriter"
                '///+<li>Changes</li>
                printlog "  - Changes / Aenderungen"
                if TabAenderungenWriter.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabAenderungenWriter )
                end if
       case 11: Kontext "TabCompatibility"
                '///+<li>Compatibility</li>
                printlog "  - Compatibility"
                if TabCompatibility.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabCompatibility )
                end if
       case 12: Kontext "TabAutoCaption"
                '///+<li>AutoCaption</li>
                printlog "  - AutoCaption"
                if TabAutoCaption.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabAutoCaption )
                end if
       case 13: Kontext "TabMailMergeEMail"
                '///+<li>Mail Merge E-mail</li></ul>
                printlog "  - Mail Merge E-mail"
                if TabMailMergeEMail.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabMailMergeEMail )
                   ServerAuthentication.click
                   kontext "ServerAuthentication"
                   Call DialogTest ( ServerAuthentication )
                   ServerAuthentication.cancel
                   Kontext "TabMailMergeEMail"
                   TestSettings.click
                   
                   ' in case java is not enabled
                   sleep( 2 )
                   try   
                       Kontext "Messagebox"
                       if ( MessageBox.exists()) then            
                           sleep(2)                
                           warnlog(Messagebox.GetText())                
                           try
                   Messagebox.Cancel()
                               sleep( 2 )
               catch
                           Messagebox.OK() ' if "OK" works, we are in an undefined state
                   warnlog( "" )
                           endcatch                
                       end if             
           catch
               warnlog( "A messagebox was displayed but coul not be closed." )
                   endcatch
                   
                   Kontext "TestaccountSettings"
                   Call DialogTest ( TestaccountSettings )
                   TestaccountSettings.cancel
                end if
      end select
      if gefunden=FALSE then
         Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
         EchteAnzahl = EchteAnzahl - 1
         if EchteAnzahl < 12 then
            i=13
         else
            i=1-1
         end if
      end if
   next i
'/// Close the options dialog
   Kontext "OptionenDlg"
   OptionenDlg.Cancel
'/// Close the document  
    if gApplication <> "BACKGROUND" then 
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsHTMLDocument
  dim EchteAnzahl as Integer, i as Integer
  dim gefunden as Boolean
    
   PrintLog "- Tools/Options/HTML-Document  ( Extras/Optionen/HTML-Dokument )"
    if (0 = iSectionNumber) then 
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
'/// Create a new document    
    if gApplication <> "BACKGROUND" then    
        Call hNewDocument
    end if
 '/// <i>Tools</i> / <i>options</i> / <i>HTML Document</i>
   ToolsOptions
   ToPosInOptionlist ( 5 )
   i = 6
   if bDebugVersion = TRUE  then i = i + 1  ' here exists the special debug-tabpage, too
   EchteAnzahl = OptionTabPageZaehler (i)
   if bDebugVersion = TRUE  then EchteAnzahl = EchteAnzahl - 1
   for i=1 to EchteAnzahl
      gefunden = FALSE
      Kontext "OptionenDlg"
      Optionsliste.TypeKeys "<Down>"
      select case i
       case 1 : Kontext "TabInhalteHTML"
                '/// View
                printlog "  - View / Ansicht"
                if TabInhalteHTML.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabInhalteHTML )
                end if
       case 2 : Kontext "TabDirektCursor"
                '/// Formating aids
                printlog "  - Formatting aids / Formatierungshilfen"
                if TabDirektCursor.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabDirektCursor )
                end if
       case 3 : Kontext "TabRaster"
                '/// Grid
                printlog "  - Grid / Raster"
                if TabRaster.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabRaster )
                end if
       case 4 : Kontext "TabDruckenHTML"
                '/// Print
                printlog "  - Print / Drucken"
                if TabDruckenHTML.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabDruckenHTML )
                end if
       case 5 : Kontext "TabTabelleOptionen"
                '/// Table
                printlog "  - Table / Tabelle"
                if TabTabelleOptionen.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabTabelleOptionen )
                end if
       case 6 : Kontext "TabHintergrund"
                '/// Background
                printlog "  - Background / Hintergrund"
                Sleep 1
                if TabHintergrund.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabHintergrund )
                end if
      end select
      if gefunden=FALSE then
         Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
         EchteAnzahl = EchteAnzahl - 1
         if EchteAnzahl < 6 then
            i=11
         else
            i=1-1
         end if
      end if
   next i
'/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
'/// Close the document    
    if gApplication <> "BACKGROUND" then 
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsSpreadsheet
  dim EchteAnzahl as Integer, i as Integer
  dim gefunden as Boolean
    
    PrintLog "- Tools/Options/Spreadsheet  ( Extras/Optionen/Tabellendokument )"
    if (0 = iSectionNumber) then 
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
'/// Create a new document
    if gApplication <> "BACKGROUND" then 
        Call hNewDocument
    end if
 '/// <i>Tools</i> / <i>Options</i> / <i>Spreadsheet</i>
   ToolsOptions
   ToPosInOptionlist ( 4 )
   EchteAnzahl = OptionTabPageZaehler ( 7, FALSE )
   for i=1 to EchteAnzahl
      gefunden = FALSE
      Kontext "OptionenDlg"
      Optionsliste.TypeKeys "<Down>"
      select case i
       case 1 : Kontext "TabLayoutCalc"
                '/// General
                printlog "  - General / Allgemein"
                if TabLayoutCalc.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabLayoutCalc )
                end if
       case 2 : Kontext "TabInhaltCalc"
                '/// View
                printlog "  - View / Ansicht"
                if TabInhaltCalc.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabInhaltCalc )
                end if
       case 3 : Kontext "TabBerechnen"
                '/// Calculate
                printlog "  - Calculate / Berechnen"
                if TabBerechnen.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabBerechnen )
                end if
       case 4 : Kontext "TabSortierlisten"
                '/// Sort lists
                printlog "  - Sort Lists / Sortierliste"
                if TabSortierlisten.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabSortierlisten )
                end if
       case 5 : Kontext "TabAenderungenCalc"
                '/// Changes
                printlog "  - Changes / Aenderungen"
                if TabAenderungenCalc.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabAenderungenCalc )
                end if
       case 6 : Kontext "TabRaster"
                '/// Grid
                printlog "  - Grid / Raster"
                if TabRaster.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabRaster )
                end if
       case 7 : Kontext "TabPrintCalcOptions"
                '/// Print
                printlog "  - Print / Drucken"
                if TabPrintCalcOptions.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabPrintCalcOptions )
                end if
      end select
      if gefunden=FALSE then
         Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
         EchteAnzahl = EchteAnzahl - 1
         if EchteAnzahl < 7 then
            i=11
         else
            i=1-1
         end if
      end if
   next i
'/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
'/// Close the document
    if gApplication <> "BACKGROUND" then 
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsPresentation
  dim EchteAnzahl as Integer, i as Integer
  dim gefunden as Boolean
  dim iTemp as integer
    
   PrintLog "- Tools/Options/Presentation  ( Extras/Optionen/Praesentation )"
    if (0 = iSectionNumber) then 
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
'/// Create a new document
    if gApplication <> "BACKGROUND" then 
        Call hNewDocument
    end if
   kontext "DocumentImpress"
   if DocumentImpress.exists(5) then
       printlog "  - An application of type Presentation is available."
       '--- workaround #i48383#
       DocumentImpress.MouseDown (10,10)
       DocumentImpress.MouseUp (10,10)
   else
       qaErrorLog "  - No application of type Presentation is available"
   end if
 '/// <i>Tools</i> / <i>Options</i> / <i>Presentation</i>
   ToolsOptions
   ToPosInOptionlist ( 4 )
   EchteAnzahl = OptionTabPageZaehler ( 4, FALSE )
   for i=1 to EchteAnzahl
      gefunden = FALSE
      Kontext "OptionenDlg"
      Optionsliste.TypeKeys "<Down>"
      select case i
       case 1 : Kontext "TabSonstigesDraw"
                '/// General
                printlog "  - General / Allgemein"
                if TabSonstigesDraw.exists then
                    if TabSonstigesDraw.IsVisible then
                       gefunden = TRUE
                       Call DialogTest ( TabSonstigesDraw )
                    end if
                else
                    ' Trying to get root cause on some systems it seems the backing window is used. #i48383#
                    Kontext "OptionenDlg"
                    OptionenDlg.SnapShot convertPath(gOfficePath + "user/work/"+gApplication+".bmp")
                    qaErrorLog "#i48383# Trying to get root cause for wrong window; check picture: " + convertPath(gOfficePath + "user/work/"+gApplication+".bmp")
                    for iTemp = 1 to Optionsliste.getItemCount
                        qaErrorLog "" + iTemp + ": '" + Optionsliste.getItemText(iTemp) + "'"
                    next iTemp
                    gefunden = TRUE
                end if
       case 2 : Kontext "TabInhalteDraw"
                '/// View
                printlog "  - View / Ansicht"
                if TabInhalteDraw.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabInhalteDraw )
                end if
       case 3 : Kontext "TabRaster"
                '/// Grid
                printlog "  - Grid / Raster"
                if TabRaster.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabRaster )
                end if
       case 4 : Kontext "TabDruckenDraw"
                '/// Print
                printlog "  - Print / Drucken"
                if TabDruckenDraw.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabDruckenDraw )
                end if
      end select
      if gefunden=FALSE then
         Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
         EchteAnzahl = EchteAnzahl - 1
         if EchteAnzahl < 4 then
            i=11
         else
            i=1-1
         end if
      end if
   next i
'/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
'/// Close the document    
    if gApplication <> "BACKGROUND" then 
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsDrawing
  Dim EchteAnzahl as Integer, i as Integer
  Dim gefunden as Boolean
    
   PrintLog "- Tools/Options/Drawing  ( Extras/Optionen/Zeichnung )"
    if (0 = iSectionNumber) then 
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    end if
'/// Create a new document
    if gApplication <> "BACKGROUND" then 
        Call hNewDocument
    end if
   Kontext "DocumentDraw"
   if DocumentDraw.exists(5) then
       printlog "  - An application of type Draw is available."
       '--- workaround #i48383#
       DocumentDraw.MouseDown (10,10)
       DocumentDraw.MouseUp (10,10)
   else
       qaErrorLog "  - No application of type Draw is available"
   end if
'/// <i>Tools</i> / <i>Options</i> / <i>Drawing</i>
   ToolsOptions
   ToPosInOptionlist ( 4 )
   EchteAnzahl = OptionTabPageZaehler ( 4, FALSE )
   for i=1 to EchteAnzahl
      gefunden = FALSE
      Kontext "OptionenDlg"
      Optionsliste.TypeKeys "<Down>"
      select case i
       case 1 : Kontext "TabSonstigesDraw"
                '/// General
                printlog "  - General / Allgemein"
                if TabSonstigesDraw.exists then
                    if TabSonstigesDraw.IsVisible then
                       gefunden = TRUE
                       Call DialogTest ( TabSonstigesDraw )
                    end if
                else
                    ' Trying to get root cause on some systems it seems the backing window is used. #i48383#
                    Kontext "OptionenDlg"
                    qaErrorLog "#i48383# wrong focus on opening options"
                    gefunden = TRUE
                end if
       case 2 : Kontext "TabInhalteDraw"
                '/// View
                printlog "  - View / Ansicht"
                if TabInhalteDraw.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabInhalteDraw )
                end if
       case 3 : Kontext "Active"
                if Active.Exists then
                   Warnlog "A messagebox exists => BUG  : " + Active.GetText
                   try
                      Active.No
                   catch
                      Active.OK
                   endcatch
                end if
                Kontext "TabRaster"
                '/// Grid
                printlog "  - Grid / Raster"
                if TabRaster.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabRaster )
                end if
       case 4 : Kontext "TabDruckenDraw"
                '/// Print
                printlog "  - Print / Drucken"
                if TabDruckenDraw.IsVisible then
                   gefunden = TRUE
                   Call DialogTest ( TabDruckenDraw )
                end if
      end select
      if gefunden=FALSE then
         Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
         EchteAnzahl = EchteAnzahl - 1
         if EchteAnzahl < 4 then
            i=11
         else
            i=1-1
         end if
      end if
   next i
'/// Close the options dialog
    Kontext "OptionenDlg"
     OptionenDlg.Cancel
'/// Close the document
     if gApplication <> "BACKGROUND" then 
    Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsFormula
  dim EchteAnzahl as Integer, i as Integer
    
   PrintLog "- Tools/Options/Formula  ( Extras/Optionen/Formel )"
    if (0 = iSectionNumber) then 
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
'/// Create a new document
    if gApplication <> "BACKGROUND" then 
        Call hNewDocument
    end if
'/// <i>Tools</i> / <i>Options</i> / <i>Formula</i>    
   ToolsOptions
   ToPosInOptionlist ( 4 )
   EchteAnzahl = OptionTabPageZaehler ( 1, FALSE )
   Kontext "OptionenDlg"
   '/// Print
   Optionsliste.TypeKeys "<Down>"
   printlog "  - Print / Drucken"
   Kontext "TabDruckenMath"
   Call DialogTest ( TabDruckenMath )
'/// Close the options dialog
   Kontext "OptionenDlg"
   OptionenDlg.Cancel
'/// Close the document   
    if gApplication <> "BACKGROUND" then 
        Call hCloseDocument
    end if
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

