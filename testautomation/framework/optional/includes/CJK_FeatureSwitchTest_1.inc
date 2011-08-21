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
'* owner : hercule.li@oracle.com
'*
'* short description : CJK Feature Switch test
'*
'\***********************************************************************

testcase tSwitchLocation
    '/// Check if the switch "Asian Language support" exist -- tools/options/language settings/language
    ToolsOptions

    Call hToolsOptions("LANGUAGESETTINGS" , "LANGUAGES")

    try
        if Aktivieren.IsChecked = TRUE  then
            printLog "CJK language is checked"
        else
            printLog "CJK language isn't checked"
        end if
    catch
        warnlog "Something wrong in checkbox 'Asian Language Support'"
    endcatch

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest1
    '/// Check if checkbox "Snap to text grid" is enable or not -- Format/Paragraph tabpage "Alignment"
    call  hNewDocument

    '/// turn on "Asian Language support" , check if checkbox "Snap to text grid" is enable
    Call CheckAsianLanguageSupport("On")

    Kontext
    FormatParagraph
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"

    try
        if TextrasterFangen.IsChecked = TRUE  then
            printLog "snap to text grid is checked"
        else
            printLog "snap to text grid isn't checked"
        end if
    catch
        warnlog "Something wrong in checkbox 'snap to text grid'"
    endcatch

    TabAusrichtungAbsatz.Cancel

    '/// turn off "Asian Language support" , check if checkbox "Snap to text grid" isn't enable
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        Kontext
        FormatParagraph
        Active.SetPage TabAusrichtungAbsatz
        Kontext "TabAusrichtungAbsatz"

        try
            if TextrasterFangen.IsChecked = TRUE Or TextrasterFangen.IsChecked <> TRUE then
                warnlog "Something wrong in checkbox 'snap to text grid'"
            end if
        catch
        endcatch

        TabAusrichtungAbsatz.Cancel
    end if

    ' JSK I have no idea why the tabpage was closed a second time so i do not fix it
    ' but make closing the tabpage optional. I hate this.
    kontext "TabAusrichtungAbsatz"
    hCloseDialog( TabAusrichtungAbsatz, "cancel,optional" )

    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        Kontext
        FormatParagraph
        Active.SetPage TabAusrichtungAbsatz
        Kontext "TabAusrichtungAbsatz"

        try
            if TextrasterFangen.IsChecked = TRUE Or TextrasterFangen.IsChecked <> TRUE then
                warnlog "Something wrong in checkbox 'snap to text grid'"
            end if
        catch
        endcatch

        TabAusrichtungAbsatz.Cancel
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest2
    '/// Check if tabpage "Asian Typography" appear or not -- Format/Paragraph
    call  hNewDocument

    '/// turn on "Asian Language support" , check if tabpage "Asian Typography" appears
    Call CheckAsianLanguageSupport("On")

    Kontext
    FormatParagraph

    try
        Active.SetPage TabAsianTypography
        Kontext "TabAsianTypography"
        TabAsianTypography.Cancel
    catch
        Active.SetPage TabAusrichtungAbsatz
        Kontext "TabAusrichtungAbsatz"
        TabAusrichtungAbsatz.Cancel
        warnlog "Something wring in tabpage - Asian Typography!"
    endcatch

    '/// turn off "Asian Language support" , check if tabpage "Asian Typography" disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        Kontext
        FormatParagraph

        try
            Active.SetPage TabAsianTypography
            Kontext "TabAsianTypography"
            TabAsianTypography.Cancel
            warnlog "tabpage - Asian Typography should NOT appear!"
        catch
            Active.SetPage TabAusrichtungAbsatz
            Kontext "TabAusrichtungAbsatz"
            TabAusrichtungAbsatz.Cancel
        endcatch
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest3
    '/// Check if list box "Asian text font" appear or not  -- Format/Character
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if list box "Asian text font" appear
    Call CheckAsianLanguageSupport("On")

    Kontext
    FormatCharacter
    Kontext
    active.SetPage TabFont
    kontext "TabFont"
    try
        FontEast.Select 1
    catch
        warnlog "Something wrong in list box Asian text font when Asian Language support is on"
    endcatch
    TabFont.Cancel

    '/// turn off "Asian Language support" , Check if list box "Asian text font" disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        Kontext
        FormatCharacter
        Kontext
        active.SetPage TabFont
        kontext "TabFont"
        try
            FontEast.Select 1
            warnlog "Something wrong in list box Asian text font when Asian Language support is off"
        catch
        endcatch
        TabFont.Cancel
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest4
    '/// Check if list box "Emphasis mark" appear or not -- Format/Character
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if list box "Emphasis mark" appear
    Call CheckAsianLanguageSupport("On")

    Kontext
    FormatCharacter
    Kontext
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    try
        Emphasis.Select 1
    catch
        warnlog "Something wrong in list box Emphasis mark when Asian Language support is on"
    endcatch
    TabFontEffects.Cancel

    '/// turn off "Asian Language support" , Check if list box "Emphasis mark" disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        Kontext
        FormatCharacter
        Kontext
        active.SetPage TabFontEffects
        kontext "TabFontEffects"
        try
            Emphasis.Select 1
            warnlog "Something wrong in list box Emphasis mark when Asian Language support is off"
        catch
        endcatch
        TabFontEffects.Cancel
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest5
    '/// Check if tabpage "Searching in Japanese" appear or not -- tools/options/language settings/
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if tabpage "Searching in Japanese" appear
    Call CheckAsianLanguageSupport("On")

    ToolsOptions
    if hToolsOptions("LANGUAGESETTINGS","SEARCHINGINJAPANESE") <> TRUE then
        warnlog "Something wrong in tabpage Searching in Japanese when Asian Language support is on"
    end if

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    '/// turn on "Asian Language support" , Check if tabpage "Searching in Japanese" disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")
        ToolsOptions

        if hToolsOptions("LANGUAGESETTINGS","SEARCHINGINJAPANESE" , TRUE) = FALSE then
            printLog Chr(13) & "Yes, 'Searching Japanese'-page has been disabled after switching Asian Language Support 'off'."
        else
            warnlog "Something wrong in tabpage Searching in Japanese when Asian Language support is off"
        end if

        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.Cancel
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest6
    '/// Check if tabpage "Asian Layout" appear or not -- tools/options/language settings/
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if tabpage "Asian Layout" appear
    Call CheckAsianLanguageSupport("On")

    ToolsOptions
    if hToolsOptions("LANGUAGESETTINGS","ASIANLAYOUT") <> TRUE then
        warnlog "Something wrong in tabpage Asian Layout when Asian Language support is on"
    end if

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    '/// turn off "Asian Language support" , Check if tabpage "Asian Layout" disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")
        ToolsOptions

        if hToolsOptions("LANGUAGESETTINGS","ASIANLAYOUT", TRUE) = FALSE then
            printLog Chr(13) & "Yes, 'Asian Layout'-page has been disabled after switching 'Asian Language Support' off."
        else
            warnlog "Something wrong in tabpage Asian Layout when Asian Language support is off"
        end if

        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.Cancel
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest7
    '/// Check if menu format/Ruby is active or not
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if menu format/Ruby is actived
    Call CheckAsianLanguageSupport("On")

    try
        formatRuby
        Kontext "RubyDialog"
        DialogClose.Click
        Sleep 2
    catch
        warnlog "Something wrong in menu format/Ruby when Asian Language support is on"
    endcatch

    '/// turn off "Asian Language support" , Check if menu format/Ruby is inactived
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        try
            formatRuby
            Kontext "RubyDialog"
            DialogClose.Click
            Sleep 2
            '#i40913# 
            warnlog "Something wrong in menu format/Ruby when Asian Language support is off"
        catch
        endcatch
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest8
    '/// Check if the following submenus in format/change case is active or not
    '/// -  Half width
    '/// -  Full width
    '/// -  Hiragana
    '/// -  Katagana

    call  hNewDocument

    '/// turn on "Asian Language support" , Check if submenus in format/change case is actived
    Call CheckAsianLanguageSupport("On")

    try
        FormatChangeCaseHalfWidth
    catch
        warnlog "Something wrong in submenu format/change case -> Half width when Asian Language support is on"
    endcatch

    try
        FormatChangeCaseFullWidth
    catch
        warnlog "Something wrong in submenu format/change case -> Full width when Asian Language support is on"
    endcatch

    try
        FormatChangeCaseHiragana
    catch
        warnlog "Something wrong in submenu format/change case -> Hiragana when Asian Language support is on"
    endcatch

    try
        FormatChangeCaseKatagana
    catch
        warnlog "Something wrong in submenu format/change case -> Katagana when Asian Language support is on"
    endcatch

    '/// turn off "Asian Language support" , Check if submenus in format/change case is inactived
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        try
            FormatChangeCaseHalfWidth
            '#i43434# 
            warnlog "Something wrong in submenu format/change case -> Half width when Asian Language support is off"
        catch
        endcatch

        try
            FormatChangeCaseFullWidth
            '#i43434# 
            warnlog "Something wrong in submenu format/change case -> Full width when Asian Language support is off"
        catch
        endcatch

        try
            FormatChangeCaseHiragana
            '#i43434# 
            warnlog "Something wrong in submenu format/change case -> Hiragana when Asian Language support is off"
        catch
        endcatch

        try
            FormatChangeCaseKatagana
            '#i43434# 
            warnlog "Something wrong in submenu format/change case -> Katagana when Asian Language support is off"
        catch
        endcatch
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest9
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if list box text direction appear
    Call CheckAsianLanguageSupport("On")

    FormatPageWriter
    Kontext
    Active.SetPage TabSeite
    Kontext "TabSeite"

    try
        Textfluss.select 1
    catch
        warnlog "Something wrong in list box text direction when Asian Language support is on"
    endcatch
    TabSeite.Cancel

    '/// turn off "Asian Language support" , Check if list box text direction disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        FormatPageWriter
        Kontext
        Active.SetPage TabSeite
        Kontext "TabSeite"

        try
            Textfluss.select 1
            warnlog "Something wrong in list box text direction when Asian Language support is off"
        catch
        endcatch
        TabSeite.Cancel
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest10

    '/// Check if vertical text box and callout appear or not
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if vertical text box and callout appear
    Call CheckAsianLanguageSupport("On")
    sleep(1)

    Call hToolbarSelect("DRAWING", true)
    try
        VerticalText.Click
    catch
        warnlog "Something wrong in vertical text box when Asian Language support is on"
    endcatch
    try
        VerticalCallout.Click
    catch
        warnlog "Something wrong in vertical text callout when Asian Language support is on"
    endcatch

    '/// turn off "Asian Language support" , Check if vertical text box and callout disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")
        Kontext "Drawbar"
        try
            VerticalText.Click
            warnlog "Something wrong in vertical text box when Asian Language support is off"
        catch
        endcatch
        try
            VerticalCallout.Click
            warnlog "Something wrong in vertical text callout when Asian Language support is off"
        catch
        endcatch
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest11

    dim i as integer
    dim iCount as integer

    const iHopeNumber1 = 42
    const iHopeNumber2 = 23

    '/// Check if the number in listbox numbering  is right or not.
    '/// [if the number of the listbox numbering is changed, this case should be modified according]

    call  hNewDocument

    '/// turn on "Asian Language support" , Check if the number in listbox numbering  is right
    Call CheckAsianLanguageSupport("On")

    FormatNumberingBullets
    Kontext
    Active.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"

    iCount = Numerierung.GetItemCount
    if iCount = iHopeNumber1 then
        Numerierung.Select 15
        TabOptionenNumerierung.OK

        printLog Chr(13) & "Pls. go to the printer to find if you can get a paper"
        printLog Chr(13) & "which is printed like '-'.Otherwise , something wrong!"
        '/// --Pls. go to the printer to find if you can get a paper which is printed like '-'. Otherwise , something is wrong!

        FilePrint
        Kontext "Active"
        if Active.Exists( 1 ) then
            if Active.GetRT = 304 then
                Active.Ok
                QAErrorLog "No Default-Printer!"
                Kontext "Printing"
                hCloseDialog( Printing, "cancel,optional" )
            end if
        else
            Kontext "Printing"
            hCloseDialog( Printing, "cancel,optional" )
        endif
    else
        for i =1 to iCount
            printlog "(" & i & "/" & iCount & "): " & Numerierung.getItemText(i)
        next i
        'Referenze:
        TabOptionenNumerierung.Cancel
        qaerrorlog "the number of list box numbering is wrong when Asian Language support is on; expected: " + iHopeNumber1 + "; is: " + iCount
    end if

    'print warning if no printer is available
    kontext
    if active.exists(5) then
        qaErrorlog "print warning: '" + active.getText + "'"
        active.ok
    endif

    'print warning if no printer is available
    kontext
    if active.exists(5) then
        qaErrorlog "print warning: '" & active.getText & "'"
        active.ok
    endif

    '/// turn off "Asian Language support" , Check if the number in listbox numbering  is right
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")

        FormatNumberingBullets
        Kontext
        Active.SetPage TabOptionenNumerierung
        Kontext "TabOptionenNumerierung"

        iCount = Numerierung.GetItemCount
        if iCount <> iHopeNumber2 then
            for i =1 to iCount
                printlog "(" & i & "/" & iCount & "): " & Numerierung.getItemText(i)
            next i
            'Referenze:
            warnlog "the number of list box numbering is wrong when Asian Language support is off; expected: " & iHopeNumber2 & "; is: " & iCount
        end if
        TabOptionenNumerierung.Cancel
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tProlongedVowelsTest1
    '/// Check if the default setting of "Prolonged vowels (ka-/kaa) will be ACTIVATED.

    call  hNewDocument

    '/// turn on "Asian Language support" , Check if the default setting of "Prolonged vowels (ka-/kaa) is ACTIVATED.
    Call CheckAsianLanguageSupport("On")

    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","SEARCHINGINJAPANESE")

    if ( not ProlongedSoundmarks.IsChecked ) then
        warnlog "The default setting of 'Prolonged vowels (ka-/kaa)' isn't ACTIVATED."
    end if

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tCalcUIChangesTest_10

    '/// Check if vertical text box and callout appear or not

    call  hNewDocument

    '/// turn on "Asian Language support" , Check if vertical text box and callout appear
    Call CheckAsianLanguageSupport("On")
    sleep(1)
    Call hToolbarSelect("DRAWING", true)
    try
        VerticalText.Click
    catch
        warnlog "Something wrong in vertical text box when Asian Language support is on"
    endcatch
    try
        VerticalCallout.Click
    catch
        warnlog "Something wrong in vertical text callout when Asian Language support is on"
    endcatch

    '/// turn off "Asian Language support" , Check if vertical text box and callout disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")
        Kontext "Drawbar"
        try
            VerticalText.Click
            warnlog "Something wrong in vertical text box when Asian Language support is off"
        catch
        endcatch
        try
            VerticalCallout.Click
            warnlog "Something wrong in vertical text callout when Asian Language support is off"
        catch
        endcatch
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tImpressUIChangesTest_10

    '/// Check if vertical text box and callout and fit vertical text to frame appear or not
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if vertical text box appear
    Call CheckAsianLanguageSupport("On")
    sleep(1)
    Kontext "Drawbar"
    sleep(1)
    try
        VerticalText.Click
    catch
        warnlog "Something wrong in vertical text box when Asian Language support is on"
    endcatch

    '/// turn off "Asian Language support" , Check if vertical text box disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")
        sleep(1)
        Kontext "Drawbar"
        try
            VerticalText.Click
            warnlog "Something wrong in vertical text box when Asian Language support is off"
        catch
        endcatch
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tDrawUIChangesTest_10

    '/// Check if vertical text box and callout and fit vertical text to frame appear or not
    call  hNewDocument

    '/// turn on "Asian Language support" , Check if vertical text box appear
    Call CheckAsianLanguageSupport("On")
    Kontext "Drawbar"
    sleep(1)
    try
        VerticalText.Click
    catch
        warnlog "Something wrong in vertical text box when Asian Language support is on"
    endcatch

    '/// turn off "Asian Language support" , Check if vertical text box disappear
    if ( not hSystemHasAsianLocale() ) then
        Call CheckAsianLanguageSupport("Off")
        Kontext "Drawbar"
        try
            VerticalText.Click
            warnlog "Something wrong in vertical text box when Asian Language support is off"
        catch
        endcatch
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

function hSystemHasAsianLocale() as boolean

    select case ( iSystemSprache )
    case 81, 82, 86, 88 : hSystemHasAsianLocale() = true
    case else           : hSystemHasAsianLocale() = false
    end select

end function
