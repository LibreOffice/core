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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'*******************************************************************
'*
' #1 tFormatCharacter
' #1 tDialogCharacter
' #1 tSetFormatCharacter
' #1 tGetFormatCharacter
'*
'\*******************************************************************
testcase tFormatCharacter

   dim sTemp as string
   dim iTemp as integer

   printlog "open application "
   Call hNewDocument
   sleep 1

   printlog "create a textframe with content "
   hTextrahmenErstellen ("This is a check, a check, a check.",10,10,60,30)
   printlog "type with keys [strg]+[home] [shift]+[end] to select the inserted text "
   hTypeKeys "<MOD1 HOME><shift end>"
   sleep 1

   printlog "Format->Character "
   FormatCharacter
    sleep 1
    printlog "switch to tabpage 'Font' "
    Kontext
    Messagebox.SetPage TabFont
    Kontext "TabFont"
    sleep 1

    printlog "switch to tabpage 'Font Effects' "
    Kontext
    Messagebox.SetPage TabFontEffects
    kontext "TabFontEffects"
    sleep 1

    printlog "switch to tabpage 'Position' "
    Kontext
    Messagebox.SetPage TabFontPosition
    Kontext "TabFontPosition"
    sleep 1

    printlog "close dialog 'Character' with 'OK' "
    TabFontPosition.OK
    printlog "close application "
   Call hCloseDocument
   
endcase 'tFormatCharacter

'---------------------------------------------------------

testcase tDialogCharacter

    Dim i as integer
    Dim y as integer
    Dim z as integer
    Dim CountOfThem as integer
    Dim CountOfColors as integer
    Dim Xtext as string
    Dim Tcolor as string
    Dim TestColorString as string
    Dim ThisIsMyTextFromControl as string
    Dim Asiansupportsetting as string

    Printlog "DialogCharacter starts"

    Call hNewDocument

    sleep 3
    ToolsOptions
    hToolsOptions ( "LanguageSettings", "Languages" )

    printlog "Check in what state Asian support is "
    if Aktivieren.IsChecked then                                  
       Asiansupportsetting = TRUE
    else
        printlog "Activate if not already activated "
       Asiansupportsetting = FALSE
       Aktivieren.Check                                           
    end if

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Sleep (3)

    Kontext

    Call hTextrahmenErstellen ( "Some text in a box.", 20, 20, 50, 50 )

    FormatCharacter
    sleep 1
    Kontext '"Active"                 
    Active.SetPage TabFont           
    Kontext "TabFont"
    CountOfThem = FontWest.GetItemCount
    for y = 1 to CountOfThem
    FontWest.Select y
    Xtext = FontWest.GetSelText
'    printlog "This FontWest is: " & Xtext
    next y

    CountOfThem = StyleWest.GetItemCount
        for y = 1 to CountOfThem
        StyleWest.Select y
        Xtext = StyleWest.GetSelText
'        printlog "This StyleWest is: " & Xtext
    next y

    CountOfThem = SizeWest.GetItemCount
    for y = 1 to CountOfThem
        SizeWest.Select y
        Xtext = SizeWest.GetSelText
'        printlog "This SizeWest is: " & Xtext
    next y

    CountOfThem = LanguageWest.GetItemCount
    for y = 1 to CountOfThem
        LanguageWest.Select y
        Xtext = LanguageWest.GetSelText
'        printlog "This LanguageWest is: " & Xtext
    next y

    CountOfThem = FontEast.GetItemCount
    for y = 1 to CountOfThem
        FontEast.Select y
        Xtext = FontEast.GetSelText
'        printlog "This FontEast is: " & Xtext
    next y

    CountOfThem = StyleEast.GetItemCount
    for y = 1 to CountOfThem
        StyleEast.Select y
        Xtext = StyleEast.GetSelText
'        printlog "This StyleEast is: " & Xtext
    next y

    CountOfThem = SizeEast.GetItemCount
    for y = 1 to CountOfThem
        SizeEast.Select y
        Xtext = SizeEast.GetSelText
'        printlog "This SizeEast is: " & Xtext
    next y

    CountOfThem = LanguageEast.GetItemCount
    for y = 1 to CountOfThem
        LanguageEast.Select y
        Xtext = LanguageEast.GetSelText
'        printlog "This LanguageEast is: " & Xtext
    next y

    Kontext
    Active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    if NOT Outline.IsChecked then
        Outline.Check
    end if

    if NOT Shadow.IsChecked then
        Shadow.Check
    end if

    CountOfThem = Underline.GetItemCount
    CountOfColors = Color.GetItemCount

    for y = 2 to CountOfThem
        Underline.Select y
        Xtext = Underline.GetSelText
'        printlog "This Underline is: " & Xtext

        for i = 1 to CountOfColors
            Color.Select i
            TestColorString = Color.GetSelText
'            printlog "This Color is: " & TestColorString
        next i
    next y

    CountOfThem = StrikeThrough.GetItemCount
    for y = 2 to CountOfThem
        StrikeThrough.Select y
        IndividualWords.UnCheck
        Xtext = StrikeThrough.GetSelText
'        printlog "The StrikeThrough is: " & Xtext

        for i = 1 to CountOfColors
            Color.Select i
            TestColorString = Color.GetSelText
'            printlog "And the Color is: " & TestColorString
        next i
    next y

    IndividualWords.Check
    printlog "Individual Words = On"

    CountOfThem = StrikeThrough.GetItemCount
    for y = 2 to CountOfThem
        StrikeThrough.Select y
        Xtext = StrikeThrough.GetSelText
'        printlog "The StrikeThrough is: " & Xtext

        for i = 1 to CountOfColors
            Color.Select i
            TestColorString = Color.GetSelText
'            printlog "And the Color is: " & TestColorString
        next i
    next y

    CountOfThem = Emphasis.GetItemCount
    for y = 2 to CountOfThem
        Emphasis.Select y
        Xtext = Emphasis.GetSelText
'        printlog "The Emphasis is: " & Xtext

        for i = 1 to CountOfColors
            Color.Select i
            TestColorString = Color.GetSelText
'            printlog "And the Color is: " & TestColorString
        next i
    next y

    CountOfThem = Position.GetItemCount
    for y = 1 to CountOfThem
        Position.Select y
        Xtext = Position.GetSelText
'        printlog "The Position is: " & Xtext
        for i = 1 to CountOfColors
            Color.Select i
            TestColorString = Color.GetSelText
'            printlog "And the Color is: " & TestColorString
        next i
    next y

    CountOfThem = FontColor.GetItemCount
     for z = 1 to CountOfThem
        FontColor.Select z
        TestColorString = FontColor.GetSelText
'        printlog "The FontColor is: " & TestColorString
     next z

    CountOfThem = Relief.GetItemCount
    TestColorString = Color.GetSelText
    for y = 1 to CountOfThem
        Relief.Select y
        Xtext = Relief.GetSelText
        TestColorString = Color.GetSelText
'        printlog "The Relief is: " & Xtext
    next y

    Kontext
    printlog "Changes the active tab to Position"
    Active.SetPage TabFontPosition        
    Kontext "TabFontPosition"
    printlog "TabFontPosition"
    SuperScript.Check
    Super.Check
    printlog "Testing the Relative Font Size-Listbox"
    RelativeFontSize.ToMin         
    ThisIsMyTextFromControl = RelativeFontSize.GetText
'    printlog "The Relative Font Size minimum is: " & ThisIsMyTextFromControl
    for y = 1 to 99
        RelativeFontSize.TypeKeys "<UP>"
        ThisIsMyTextFromControl = RelativeFontSize.GetText
'        printlog "The Relative Font Size is: " & ThisIsMyTextFromControl
    next y

    Automatic.Check
    printlog "Testing the Scale With-Listbox"
    ScaleWith.ToMin          
    ThisIsMyTextFromControl = ScaleWith.GetText
    printlog "ScaleWith minimum is:" & ThisIsMyTextFromControl
    ' ScaleWith.TypeKeys "<TAB>"
    for y = 1 to 100
        ScaleWith.TypeKeys "<UP>"
        ThisIsMyTextFromControl = ScaleWith.GetText
'        printlog "ScaleWith is:" & ThisIsMyTextFromControl
    next y

    Normal.Check
    Subscript.Check

    printlog "Testing the Reliative Font Size-Listbox again but this time"
    RelativeFontSize.ToMin
    printlog "with Subscript and Normal Checked"
    ThisIsMyTextFromControl = RelativeFontSize.GetText      
    printlog "The Relative Font Size minimum is: " & ThisIsMyTextFromControl
    for y = 1 to 99
        RelativeFontSize.TypeKeys "<UP>"
        ThisIsMyTextFromControl = RelativeFontSize.GetText
'        printlog "The Relative Font Size is: " & ThisIsMyTextFromControl
    next y

    Automatic.Check

    printlog "Testing the ScaleWith-Listbox"
    ScaleWith.ToMin          
    ThisIsMyTextFromControl = ScaleWith.GetText
    printlog "The ScaleWith Size is: " & ThisIsMyTextFromControl
    for y = 1 to 100
        ScaleWith.TypeKeys "<UP>"
        ThisIsMyTextFromControl = ScaleWith.GetText
'        printlog "The ScaleWith Size is: " & ThisIsMyTextFromControl
    next y

    printlog "Testing the Spacing-ListBox"
    ThisIsMyTextFromControl = Spacing.GetSelText      
    printlog "The Spacing is first: " & ThisIsMyTextFromControl
    for y = 2 to 3
        Spacing.Select y
        ThisIsMyTextFromControl = Spacing.GetSelText
        printlog "The Spacing is: " & ThisIsMyTextFromControl
        printlog "Testing the Spacing By-Scrollbox"
        ThisIsMyTextFromControl = SpacingBy.GetText      
'        printlog "The SpacingBy Size minimum is: " & ThisIsMyTextFromControl
        SpacingBy.ToMin
        for z = 1 to 4
            SpacingBy.More
            ThisIsMyTextFromControl = SpacingBy.GetText
'            printlog "The SpacingBy Size is: " & ThisIsMyTextFromControl
        next z
        PairKerning.Check
    next y

    Kontext "TabFontPosition"
    TabFontPosition.Ok

    if Asiansupportsetting = FALSE then
       kontext
       ToolsOptions
       hToolsOptions ( "LanguageSettings", "Languages" )
       printlog "Deactivate if not already deactivated "
       Aktivieren.UnCheck                                    
       Printlog "Reseted the Asiansupport-value to it's original state"

       Kontext "ExtrasOptionenDlg"
       ExtrasOptionenDlg.OK
       Sleep (3)

'          ActiveDeactivateAsianSupport = FALSE
    end if

    Printlog "DialogCharacter Ends"
    Call hCloseDocument
    
endcase 'tDialogCharacter
'--------------------------------------------------------
testcase tSetFormatCharacter

    if iSprache = 48 then
        qaerrorlog "Test not adapted to polish, 48."
        goto endsub
    endif
    Dim i as integer
    dim iWest as integer
    dim iAsian as integer
    dim iComplex as integer
    dim bAsian as boolean
    dim bComplex as boolean
    dim sLanguage(500) as string
    dim sFile as string
    dim sTemp as string

   if (gApplication = "IMPRESS") then 
      ExtensionString = "odp"
   else
      ExtensionString = "odg"
   end if

    sFile = convertPath(gTesttoolpath + "graphics\required\input\spellb_" + iSprache + "."+ExtensionString)
    printlog "create document, if it doesn't exist, else skip this case :-) "
    if (dir(sFile)="") then
        printlog "open application "
        Call  hNewDocument

        printlog "activate all languages in options "
        printlog " Tools->Options "
        ToolsOptions
        printlog " select tabpage 'Languages' from set 'Language Settings' "
        hToolsOptions("LANGUAGESETTINGS","Languages")
        kontext "TabSprachen"
        printlog " save state and check checkbox 'Asian languages support' "
        printlog "if the language is Asian, skip this part "
        printlog "iSprache = " + iSprache
        if bAsianLan = FALSE then
           bAsian = Aktivieren.isEnabled
           Aktivieren.check
        endif
        printlog " save state and  check checkbox 'Complex text layout (CTL) support' "
        bComplex = ComplexScriptEnabled.isEnabled
        ComplexScriptEnabled.check
        printlog "close dialog 'Options' with OK "
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK

        printlog "get count of languages "
        printlog " Format->Character "
        FormatCharacter
        sleep 1
        Kontext
        printlog "switch to tabpage 'Font' "
        Messagebox.SetPage TabFont
        kontext "TabFont"
        sleep 1
        printlog " get count of languages for Western/Asian/CTL fonts "
        iWest    = LanguageWest.getItemCount
        iAsian   = LanguageEast.getItemCount
        iComplex = LanguageCTL.getItemCount
        if bAsianLan = True then
           if (iWest <> 144) then
              Warnlog "Western text font languages count wrong; expected: '113' is: '" + iWest + "'"
           else
              Printlog "Western text font languages count: " + iWest
           endif
           if (iAsian <> 8) then
              Warnlog "Asian text font languages count wrong; expected: '9' is: '" + iAsian + "'"
           else
              Printlog "Asian text font languages count  : " + iAsian
           endif
           if (iComplex <> 50) then
              Warnlog "CTL font languages count wrong; expected: '30' is: '" + iComplex + "'"
           else
              Printlog "CTL font languages count         : " + iComplex
           endif
        else
           if (iWest <> 113) then
              Warnlog "Western text font languages count wrong; expected: '113' is: '" + iWest + "'"
           else
              Printlog "Western text font languages count: " + iWest
           endif
           if (iAsian <> 9) then
              Warnlog "Asian text font languages count wrong; expected: '9' is: '" + iAsian + "'"
           else
              Printlog "Asian text font languages count  : " + iAsian
           endif
           if (iComplex <> 30) then
              Warnlog "CTL font languages count wrong; expected: '30' is: '" + iComplex + "'"
           else
              Printlog "CTL font languages count         : " + iComplex
           endif
        endif
	printlog "close dialog 'Character' with OK "
        TabFont.Ok

        WL_SD_TextEinfuegenDraw
        gMouseMove ( 1, 1, 100, 100 )

        printlog "print the language counts into the first line of the document "
        hTypeKeys(" "+iWest+" "+iAsian+" "+iComplex+"<return>")


        printlog "set all languages to '[None]' "
        printlog "Format->Character "
        FormatCharacter
        kontext "TabFont"
        printlog "select '[None]' (should be the first entry in every listbox) "
        LanguageWest.select 1
        LanguageEast.select 1
        LanguageCTL.select 1
        printlog "close dialog 'Character' with OK "
        TabFont.Ok

        printlog "print one line with language formated text for every language "
        for i = 1 to iWest
            printlog "Format->Character "
            FormatCharacter
                kontext "TabFont"
                printlog "select the next language "
                LanguageWest.select i
                sTemp = LanguageWest.getItemText (i)
                ListAppend(sLanguage(), sTemp)
                printlog "close dialog 'Character' with OK "
            TabFont.Ok
            printlog "type [return][number of language in list][Tab][Language name][Tab] "
            hTypeKeys(""+i+"<tab>"+sTemp+"<tab><return>")
        next i

        FormatCharacter
        kontext "TabFont"
        LanguageWest.select 1
        LanguageEast.select 1
        languageCTL.select 1
        TabFont.Ok

        for i = 1 to iAsian
            FormatCharacter
                Kontext
                kontext "TabFont"
                LanguageEast.select i
                sTemp = LanguageEast.getItemText (i)
                ListAppend(sLanguage(), sTemp)
            TabFont.Ok
            hTypeKeys(""+i+"<tab>"+sTemp+"<tab><return>")
        next i

        FormatCharacter
        kontext "TabFont"
        LanguageWest.select 1
        LanguageEast.select 1
        languageCTL.select 1
        TabFont.Ok

        for i = 1 to iComplex
            FormatCharacter
                Kontext
                kontext "TabFont"
                LanguageCTL.select i
                sTemp = LanguageCTL.getItemText (i)
                ListAppend(sLanguage(), sTemp)
            TabFont.Ok
            hTypeKeys(""+i+"<tab>"+sTemp+"<tab><return>")
        next i

        printlog "save the created document & the languages list if they don't exists "
        hFileSaveAs (sFile)
        qaErrorLog "Created file. Please check it and then commit it! " + sTemp
        sTemp = convertPath(gTesttoolpath + "graphics\required\input\spellb_" + iSprache + ".txt")
        if (dir(sTemp)="") then
            ListWrite(sLanguage(), sTemp,"utf8")
        endif

        printlog "restore the options "
        printlog "Tools->Options "
        ToolsOptions
        printlog "select tabpage 'Languages' from set 'Language Settings' "
        hToolsOptions("LANGUAGESETTINGS","Languages")
        kontext "TabSprachen"

        printlog "restore state of checkbox 'Asian languages support' "
        printlog "if the language is Asian, skip this part "
        if bAsianLan = FALSE then
           if (bAsian <> TRUE) then
               Aktivieren.UNcheck
           endif
        endif
        printlog "restore state of checkbox 'Complex text layout (CTL) support' "
        if (bComplex <> TRUE) then
            ComplexScriptEnabled.UNcheck
        endif
        printlog "close dialog 'Options' with OK "
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK

        printlog "close application "
        Call hCloseDocument
    endif
    
endcase 'tSetFormatCharacter
'--------------------------------------------------------
testcase tGetFormatCharacter

    qaerrorlog "tGetFormatCharacter outcommented due to too many qaerrorlogs"
    goto endsub
    Dim i as integer
    dim iWest(2) as integer
    dim iAsian(2) as integer
    dim iComplex(2) as integer
    dim bAsian as boolean
    dim bComplex as boolean
    dim sLanguage(500) as string
    dim sLanguageR(500) as string
    dim sTemp as string
    dim iTemp as integer
    dim sSpellBooks(30) as string

    if (gApplication = "IMPRESS") then 
       ExtensionString = "odp"
    else
       ExtensionString = "odg"
    end if
    
    printlog "open the english document with all languages "
    hDateiOeffnen (convertPath(gTesttoolpath + "graphics\required\input\spellb_" + iSprache + "."+ExtensionString)
    ListRead(sLanguageR(), convertPath(gTesttoolpath + "graphics\required\input\spellb_1.txt"),"utf8")

    printlog "activate all languages in options "
    printlog " Tools->Options "
    ToolsOptions
    printlog "select tabpage 'Languages' from set 'Language Settings' "
    hToolsOptions("LANGUAGESETTINGS","Languages")
    kontext "TabSprachen"
    printlog "save state and check checkbox 'Asian languages support' "
    bAsian = Aktivieren.isEnabled
    Aktivieren.check
    printlog "save state and  check checkbox 'Complex text layout (CTL) support' "
    bComplex = ComplexScriptEnabled.isEnabled
    ComplexScriptEnabled.check
    printlog "close dialog 'Options' with OK "
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    sleep (1)
    printlog "get count of languages "
    hTypeKeys("<tab><f2>")
    printlog "first how many are in the reference document (printed in the first line)"
    hTypeKeys("<mod1 home><shift end>")
    sleep (1)
    editcopy
    sTemp = getClipboard
    iTemp = inStr(2,sTemp, " ")
    iWest(1) = mid(sTemp, 2, iTemp-2)
    i = inStr(iTemp+1,sTemp, " ")
    iAsian(1) = mid(sTemp, iTemp+1, iTemp-i)
    iComplex(1) = right(sTemp, Len(sTemp) - i)
    sleep (1)
    printlog "Format->Character "
    FormatCharacter
    sleep 1
    Kontext
    printlog "switch to tabpage 'Font' "
    Messagebox.SetPage TabFont
    kontext "TabFont"
    sleep 1
    printlog "get count of languages for Western/Asian/CTL fonts "
    iWest(2)    = LanguageWest.getItemCount
    iAsian(2)   = LanguageEast.getItemCount
    iComplex(2) = LanguageCTL.getItemCount
    if (iWest(2) <> iWest(1)) then
        Warnlog "Western text font languages count wrong; expected: '" + iWest(1) + "' is: '" + iWest(2) + "'"
    else
        Printlog "Western text font languages count: " + iWest(2)
    endif
    if (iAsian(2) <> iAsian(1)) then
        Warnlog "Asian text font languages count wrong; expected: '" + iAsian(1) + "' is: '" + iAsian(2) + "'"
    else
        Printlog "Asian text font languages count  : " + iAsian(2)
    endif
    if (iComplex(2) <> iComplex(1)) then
        Warnlog "CTL font languages count wrong; expected: '" + iComplex(1) + "' is: '" + iComplex(2) + "'"
    else
        Printlog "CTL font languages count         : " + iComplex(2)
    endif
    printlog "close dialog 'Character' with OK "
    TabFont.Ok

    printlog "write the language for every line in the current language "
    hTypeKeys("<mod1 home><down><home>")
    for i = 1 to iWest(2)
        FormatCharacter
            Kontext
            kontext "TabFont"
            iTemp = LanguageWest.getSelIndex
            sTemp = LanguageWest.getSelText
            ListAppend(sLanguage(), sTemp)
        TabFont.Ok
        printlog "type [end][number of language in list][Tab][Language name][down][home] "
        hTypeKeys("<end>"+iTemp+"<tab>"+sTemp+"<down><home>")
    next i

    for i = 1 to iAsian(2)
        FormatCharacter
            Kontext
            kontext "TabFont"
            iTemp = LanguageEast.getSelIndex
            sTemp = LanguageEast.getSelText
            ListAppend(sLanguage(), sTemp)
        TabFont.Ok
        hTypeKeys("<end>"+iTemp+"<tab>"+sTemp+"<down><home>")
    next i

    for i = 1 to iComplex(2)
        FormatCharacter
            Kontext
            kontext "TabFont"
            iTemp = LanguageCTL.getSelIndex
            sTemp = LanguageCTL.getSelText
            ListAppend(sLanguage(), sTemp)
        TabFont.Ok
        hTypeKeys("<end>"+iTemp+"<tab>"+sTemp+"<down><home>")
    next i

    printlog "save the created document & the languages list "
    sTemp = convertPath(gOfficePath + "user\work\spellb_" + iSprache)
    hFileSaveAsKill (sTemp)
    sTemp = convertPath(gOfficePath + "user\work\spellb_" + iSprache + ".txt")
    if (dir(sTemp)<>"") then
        kill (sTemp)
    endif
    ListWrite(sLanguage(), sTemp,"utf8")

    for i = 1 to ListCount(sLanguageR())
        select case i
            case 1:printlog " * Western text font languages"
            case iWest(2)+1:printlog " * Asian text font languages"
            case iWest(2)+iAsian(2)+1:printlog " * CTL font languages"
        end select
        if (sLanguageR(i) <> sLanguage(i)) then
            if (instr(sLanguage(i), "[") > 0) then ' [None] - Entry ?
                qaErrorlog "#i15863# - " + i + " '" + sLanguageR(i) + "' : '" + sLanguage(i) + "'"
            else
                qaErrorlog "#112362# - " + i + " '" + sLanguageR(i) + "' : '" + sLanguage(i) + "'"
            endif
        endif
    next i

    printlog "close application "
    Call hCloseDocument
endcase 'tGetFormatCharacter
