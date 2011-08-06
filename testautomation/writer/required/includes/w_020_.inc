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
'* short description: Testcases for Toolbars.
'*
'\******************************************************************

sub w_020_

    printLog Chr(13) + "--------- Toolbars (w_020_.inc) ----------"

    'Toolbar
    Call t_TB_Insert
    Call t_TB_InsertFields
    Call t_TB_ShowDrawFunctions
    Call t_TB_Form
    Call t_TB_EditAutotext
    Call t_TB_Graphics
    Call t_TB_DirektCursor
    Call t_TB_Find
    Call t_TB_Rest

    'Writer Web
    gApplication = "HTML"
    Call tHtmlDoc_TB_Insert
    Call tHtmlDoc_TB_InsertFields
    Call tHtmlDoc_TB_Form
    Call tHtmlDoc_TB_Rest
    gApplication = "WRITER"

    Call tUpdtFindBar()

end sub

'-----------------------------------------------------------

testcase t_TB_Insert

    ' JSK: This issue is solved since OOO300m9, enabling test again
    'qaErrorLog "#158646# - Inserting Indexes causes crash."
    'goto endsub

    printlog "- Toolbox ( normal ) : All within Insert"

    printlog " Open new document"
    Call hNewDocument
    printlog "- TearOff Insert-Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)

    if gApplication <> "HTML" then
        Kontext "Insertbar"
        printlog "  - Frame"
        printlog " Click 'Frame' and Insert a Frame"
        Rahmen.Click
        Call gMouseMove(40, 40, 60, 60)
        Call wTypeKeys ("<Delete>")
        printlog " Hit 'Delete' to remove the frame"
    end if

    printlog "  - Graphic"
    Kontext "Insertbar"
    if ( Insertbar.Exists ) then 
        ' all good
    else    
        Call hToolbarSelect("Insert", true)
    endif
    wait 500

    printlog " Click 'Insert Graphics' in Toolbox"
    if Grafik.IsEnabled then
        Grafik.Click
        Kontext "GrafikEinfuegenDlg"
        if GrafikEinfuegenDlg.Exists then
            printlog " Select graphic from file '..\writer\required\input\graphics\jolink.jpg'"
            Dateiname.SetText ( ConvertPath ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" ) )
            Oeffnen.Click
        else
            Warnlog "Dialog 'Insert Graphic' not up!"
        end if

        Kontext "Active"
        if Active.Exists( 1 ) then
            Warnlog Active.Gettext
            Active.Ok
            Kontext "GrafikEinfuegenDlg"
            GrafikEinfuegenDlg.Cancel
        end if
        printlog " Hit 'Delete' to remove graphic"
        Call wTypeKeys ("<Delete>")
    else
        Warnlog "Button 'Graphic' on Insertbar is not enabled!"
    end if

    printlog "  - Table"
    printlog " Click 'Insert Table' in Toolbox"
    Kontext "Insertbar"
    if ( Insertbar.Exists() ) then 
        ' No action required
    else
        Call hToolbarSelect("Insert", true)
    endif

    sleep( 2 ) ' Too fast for some machines. Using dirty fix.
    if Tabelle.IsEnabled then
        Tabelle.Click
        Kontext "TabelleEinfuegenWriter"
        TabelleEinfuegenWriter.OK
        printlog " Move cursor under table"
        Call wTypeKeys ("<Down>",2)
    else
        Warnlog "Button 'Table' on Insertbar is not enabled!"
    end if

    printlog "  - Document"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)

    if gApplication <> "HTML" then
        printlog "  - Footnote"
        printlog " Click 'Insert Footnote Directly' in Toolbox"
        Kontext "Insertbar"
        if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
        if Fussnote.IsEnabled then
            Fussnote.Click
            Call wTypeKeys ("<PageUp>")
        else
            Warnlog "Button 'Footnote' on Insertbar is not enabled!"
        end if

        Sleep 2
        printlog "  - Endnote"
        printlog " Click 'Insert Endnote Directly' in Toolbox"
        Kontext "Insertbar"
        if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
        if Endnote.IsEnabled then
            Endnote.Click
            Call wTypeKeys ("<PageUp>")
        else
            Warnlog "Button 'Endnote' on Insertbar is not enabled!"
        end if

        printlog "  - Index Marker"
        Call wTypeKeys ("Index Marker <Shift Home>")
        printlog " Click 'Insert Index Marker' in Toolbox"
        Kontext "Insertbar"
        if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
        if Verzeichnis.IsEnabled then
            Verzeichnis.Click
            printlog " Close upcoming 'Insert Index Entry' dialog"
            Kontext "VerzeichniseintragEinfuegen"
            VerzeichniseintragEinfuegen.Close
        else
            Warnlog "Button 'Index Marker' on Insertbar is not enabled!"
        end if
    end if

    printlog "  - Special Character"
    printlog " Click 'Insert Special Character' in Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if Sonderzeichen.IsEnabled then
        Sonderzeichen.Click
        printlog " Cancel upcoming 'Special Character' dialog"
        Kontext "Sonderzeichen"
        Sonderzeichen.Cancel
    else
        Warnlog "Button 'Special Character' on Insertbar is not enabled!"
    end if

    printlog "  - Insert Section"
    printlog " Click 'Insert Section' in Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if BereichEinfuegen.IsEnabled then
        BereichEinfuegen.Click
        printlog " Close upcoming 'Section' dialog"
        Kontext
        Active.Setpage TabBereiche
        Kontext "TabBereiche"
        TabBereiche.Ok
    else
        Warnlog "Button 'Insert Section' on Insertbar is not enabled!"
    end if

    printlog "  - Bookmark"
    printlog " Click 'Insert Bookmark' in Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if Textmarke.IsEnabled then
        Textmarke.Click
        printlog " Enter a bookmark name and close dialog"
        Kontext "TextmarkeEinfuegen"
        Textmarken.SetText "Bookmark"
        TextmarkeEinfuegen.OK
    else
        Warnlog "Button 'Bookmark' on Insertbar is not enabled!"
    end if

    'Only with closing the toolbox the dialog is accessible for TT
    printlog " Close toolbar"
    Kontext "Insertbar"
    if Insertbar.Exists then Insertbar.Close

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase t_TB_InsertFields
    Dim Ende as Integer
    Dim i as Integer
    printlog "- Toolbox ( normal ) : Fields"

    if gPlatgroup = "unx" then
        printlog( "Test not suitable for Unix(like) OS" )
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument
    printlog "- TearOff Insert Toolbox"
    printlog " Tear off toolbar and open insert toolbar"
    printlog " Click 'Insert Fields' in insertbar"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    Sleep 1
    Feldbefehle.OpenMenu
    wait 500
    if gApplication = "HTML" then
        Call hMenuSelectNr(6)
    else
        Call hMenuSelectNr(8)
    end if
    Kontext
    try
        Active.SetPage TabDokumentFeldbefehle
        Kontext "TabDokumentFeldbefehle"
        printlog " Cancel upcoming 'Fields' dialog"
        TabDokumentFeldbefehle.Cancel
    catch
        QAErrorlog "Dialog for Fields is not up !"
    endcatch
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase t_TB_ShowDrawFunctions
    printlog "- Toolbox ( normal ) : Draw Functions"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    ViewZoom
    Kontext "Massstab"
    Optimal.Check
    Massstab.OK

    printlog " Open 'Draw functions' toolbar"
    Kontext "DrawBar"
    if Not DrawBar.Exists then Call hToolbarSelect("Drawing", true)
    Sleep 5

    printlog "  - Draw Line"
    Kontext "Drawbar"
    printlog " Draw a Line"
    try
        Linie.Click
        Call gMouseMove(20, 20, 30, 30)
        Call wTypeKeys ("<Escape>")
    catch
        Warnlog " Line could not be created !"
    endcatch

    printlog "  - Draw Rectangle"
    Kontext "Drawbar"
    printlog " Draw a Rectangle"
    Rechteck.Click
    Call gMouseMove(31, 20, 40, 40)
    Call wTypeKeys ("<Escape>")

    printlog "  - Draw Ellipse"
    Kontext "Drawbar"
    printlog " Draw an Ellipse"
    Ellipse.Click
    Call gMouseMove(31, 30, 60, 50)
    Call wTypeKeys ("<Escape>")

    printlog "  - Draw Freeform"
    Kontext "Drawbar"
    Freihand.Click
    printlog " Draw a Freeform"
    Call gMouseMove ( 23, 20, 35, 31 )
    Call gMouseMove ( 35, 31, 40, 90 )
    Call gMouseMove ( 41, 91, 55 , 70 )
    Call gMouseClick( 55, 70 )
    Call wTypeKeys ("<Escape>")

    printlog "  - Draw Textobject"
    Kontext "Drawbar"
    printlog " Draw a Textobject"
    Textobjekt.Click
    Call gMouseMove ( 17, 17, 28, 25 )
    Call wTypeKeys ("Textobject")
    Call gMouseClick ( 50, 99 )
    Call wTypeKeys ("<Escape>")

    printlog "  - Draw Callouts"
    Kontext "Drawbar"
    printlog " Draw a Callout"
    Legende.Click
    Call gMouseMove (15, 20, 50, 60)
    Call gMouseClick( 50, 60 )
    Call wTypeKeys ("<Escape>")

    printlog " Close toolbar 'Draw Functions'"
    Kontext "Drawbar"
    Drawbar.Close
    ViewZoom
    Kontext "Massstab"
    Vergroesserung100.Check
    Massstab.OK

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase t_TB_Form
    printlog "- Toolbox ( normal ) : Formular"

    printlog " Open new document"
    Call hNewDocument
    
    Kontext "FormControls"
    if ( FormControls.Exists( 1 ) ) then 
        'do nothing
    else
        Call hToolbarSelect("Controls", true)
    endif

    try
        printlog " Tear off 'Form' Toolbox"
        Kontext "FormControls"
        if Not FormControls.Exists then
            Warnlog "Toolbar 'FormControls' is not opened!"
            Call hCloseDocument
            goto endsub
        end if
        wait 500
        Pushbutton.Click
        printlog " Insert a pushbutton from toolbox"
        Call hMalZeichnenMitSelektion ( 35, 20, 30, 25 )
        Sleep 2
        Kontext "FormControls"
        if NOT ControlProperties.IsEnabled then
            if gPlatform = "osx" then
                warnlog "Mac OS X (Aqua): Drawing function does not draw anything"
            else
                warnlog "Control properties button disabled!"
            end if
        else
            ControlProperties.Click
            sleep(2)
        end if
        printlog "+ Open Control Properties"
        printlog "   - Control Properties"
        Call sMenufunktionen("-Pushbutton-")

        Kontext "FormControls"
        wait 500
        Radiobutton.Click
        printlog " Insert a Radiobutton from toolbox"
        Call hMalZeichnenMitSelektion ( 32, 20, 42, 25 )
        printlog "+ Open Control Properties"
        Call sMenufunktionen("-Radiobutton-")

        Kontext "FormControls"
        wait 500
        Checkbox.Click
        printlog " Insert a Checkbox from toolbox"
        Call hMalZeichnenMitSelektion ( 45, 20, 55, 25 )
        printlog "+ Open Control Properties"
        Call sMenufunktionen("-Checkbox-")

        Kontext "FormControls"
        if gApplication <> "HTML" then
            wait 500
            Label.Click
            printlog " Insert a textbox from toolbox"
            Call hMalZeichnenMitSelektion ( 57, 20, 68, 25 )
            printlog "+ Open Control Properties"
            Call sMenufunktionen("-Textbox-")
        end if

        Sleep 3
        Kontext "FormControls"
        printlog " Insert an Edit Field from toolbox"
        Edit.Click
        Wait 500
        Call hMalZeichnenMitSelektion ( 32, 30, 42, 35 )
        printlog "+ Open Control Properties"
        Call sMenufunktionen("-Edit-")

        Kontext "FormControls"
        Listbox.Click
        Wait 500
        printlog " Insert a listbox"
        Call hMalZeichnenMitSelektion ( 45, 30, 55, 35 )
        printlog "+ Step through pages of upcoming Autopilot"
        Call sAutopilot_ListboxCombobox("Listbox")
        printlog "+ Open Control Properties"
        Call sMenufunktionen("-Listbox-")

        Kontext "FormControls"
        if ( gApplication <> "HTML" ) then
            Combobox.Click
            printlog " Insert a Combobox from toolbox"
            Wait 500
            Call hMalZeichnenMitSelektion ( 57, 30, 68, 35 )
            printlog "+ Step through pages of upcoming Autopilot"
            Call sAutopilot_ListboxCombobox("Combo")
            printlog "+ Open Control Properties"
            Call sMenufunktionen("-Combobox-")
        end if

        printlog " Insert a Formatted Field from toolbox"
        Kontext "FormControls"
        if ( gApplication <> "HTML" ) then
            FormattedField.Click
            Wait 500
            Call hMalZeichnenMitSelektion ( 45, 50, 55, 55 )
            printlog "+ Open Control Properties"
            Call sMenufunktionen("-Formatted Field-")
        end if
        Sleep 3
        Call gMouseClick ( 40, 80 )
        Kontext "FormControls"
        printlog " Click 'Design Mode' in Toolbox"
        wait 500
        SwitchControlDesignMode.Click
        Sleep 1
        printlog " Click 'Design Mode' in Toolbox"
        SwitchControlDesignMode.Click
        printlog "   - Design mode"
        Kontext "FormControls"
        FormControls.Close
    catch
        Exceptlog
    endcatch

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase t_TB_EditAutotext
    printlog "- Toolbox ( normal ) : Autotext"

    printlog " Open new document"
    Call hNewDocument
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    Sleep 2
    Autotext.Click
    printlog " Click 'Autotext' in Toolbar"

    Kontext "Active"
    if ( Active.Exists( 1 ) ) then
        Active.Gettext
        Active.Ok
    end if
    
    Kontext "Autotext"
    if ( Autotext.exists( 4 ) ) then
        printlog " Close upcoming 'Autotext' dialog"
        Autotext.Close
    else
        warnlog( "Dialog <Autotext> is not available" )
    endif
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase t_TB_Find
    printlog "- Toolbox ( normal ) : Find & Replace"
    printlog " Open new document"
    Call hNewDocument
    Kontext "Standardbar"
    if Not Standardbar.Exists then Call hToolbarSelect("Standard", true)
    printlog " Click 'Find' in Toolbar"
    FindAndReplace.Click
    Kontext "FindAndReplace"
    printlog " Close 'Find&Replace' dialog"
    FindAndReplace.Close
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase t_TB_Graphics
    printlog "- Toolbox ( normal ) : Graphic"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic from file ..\writer\required\input\graphics\jolink.jpg"
    Call hGrafikEinfuegen ( gTesttoolpath + "writer\required\input\graphics\jolink.jpg" )

    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    try
        printlog " Click 'Graphic' in Toolbar"
        GrafikenAusEin.Click
    catch
        Warnlog "Error on switching Graphic off!"
    endcatch
    Sleep 2
    try
        printlog " Click 'Graphic' in Toolbar"
        GrafikenAusEin.Click
    catch
        Warnlog "Error on switching Graphic on!"
    endcatch
    Sleep 2

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase t_TB_DirektCursor
    printlog "- Toolbox ( normal ) : Direct Cursor"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    printlog " Click 'Direct Cursor' in Toolbar"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    Direktcursor.Click
    Sleep 1
    printlog " Point cursor somewhere in document"
    Call gMouseDoubleClick(50, 70)
    printlog " Insert some words in document"
    Call wTypeKeys ("Direct Cursor On")

    printlog " Click 'Direct Cursor' in Toolbar"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    Direktcursor.Click
    printlog " Insert some text in document"
    Call gMouseDoubleClick (80, 20)
    Call wTypeKeys ("Direct Cursor Off")

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase t_TB_Rest
    printlog "- Toolbox ( normal ) : Rest"

    printlog " Open new document"
    Call hNewDocument
    printlog "  - Hyperlink"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    try
        printlog " Click 'Hyperlink' in Toolbar"
        Hyperlink.Click
        Sleep 2
        printlog " Click 'Hyperlink' in Toolbar"
        Hyperlink.Click
        Warnlog "Button 'Hyperlink' should have been removed!"
    catch
    endcatch

    printlog "  - OnlineLayout"
    if gApplication <> "HTML" then
        Kontext "Toolbar"
        if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
        try
            printlog " Click 'Online Layout' in Toolbar"
            OnlineLayout.Click
            Sleep 2
            printlog " Click 'Online Layout' in Toolbar"
            OnlineLayout.Click
        catch
            Warnlog "Button 'OnlineLayout' is missing or disabled!"
        endcatch
    end if

    printlog "  - Nonprinting Characters"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    try
        printlog " Click 'Nonprinting Characters' in Toolbar"
        SteuerzeichenEinAus.Click
        Sleep 2
        printlog " Click 'Nonprinting Characters' in Toolbar"
        SteuerzeichenEinAus.Click
    catch
        Warnlog "Button 'Nonprinting Characters' is missing or disabled!"
    endcatch

    printlog "  - Graphics on/off"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    try
        printlog " Click 'Graphics' in Toolbar "
        GrafikenAusEin.Click
        Sleep 2
        printlog " Click 'Graphics' in Toolbar "
        GrafikenAusEin.Click
    catch
        Warnlog "Button 'Graphics' is missing or disabled!"
    endcatch
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tHtmlDoc_TB_Insert

    ' JSK: This issue is solved since OOO300m9, enabling test again
    'qaErrorLog "#158646# - Inserting Indexes causes crash."
    'goto endsub

    printlog "- Toolbox ( normal ) : All within Insert"
    gApplication = "HTML"

    printlog " Open new document"
    Call hNewDocument
    printlog "- TearOff Insert-Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)

    printlog "  - Graphic"
    printlog " Click 'Insert Graphics' in Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    wait 500
    
    if Grafik.IsEnabled then
        Grafik.Click
        Kontext "GrafikEinfuegenDlg"
        if ( GrafikEinfuegenDlg.Exists( 1 ) ) then
            printlog " Select graphic from file '..\writer\required\input\graphics\jolink.jpg'"
            Dateiname.SetText ( ConvertPath ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" ) )
            Oeffnen.Click
        else
            Warnlog "Dialog 'Insert Graphic' not up!"
        end if

        Kontext "Active"
        if ( Active.Exists( 1 ) ) then
            Warnlog Active.Gettext
            Active.Ok
            Kontext "GrafikEinfuegenDlg"
            GrafikEinfuegenDlg.Cancel
        end if
        printlog " Hit 'Delete' to remove graphic"
        Call wTypeKeys ("<Delete>")
    else
        Warnlog "Button 'Graphic' on Insertbar is not enabled!"
    end if

    Sleep 2
    printlog "  - Table"
    printlog " Click 'Insert Table' in Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if Tabelle.IsEnabled then
        Tabelle.Click
        Kontext "TabelleEinfuegenWriter"
        TabelleEinfuegenWriter.OK
        printlog " Move cursor under table"
        Call wTypeKeys ("<Down>",2)
    else
        Warnlog "Button 'Table' on Insertbar is not enabled!"
    end if

    printlog "  - Document"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)

    printlog "  - Special Character"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if Sonderzeichen.IsEnabled then
        Sonderzeichen.Click
        Kontext "Sonderzeichen"
        printlog " Click 'Insert Special Character' in Toolbox"
        Sonderzeichen.Cancel
        printlog " Cancel upcoming 'Special Character' dialog"
    else
        Warnlog "Button 'Special Character' on Insertbar is not enabled!"
    end if

    printlog "  - Insert Section"
    printlog " Click 'Insert Section' in Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if BereichEinfuegen.IsEnabled then
        BereichEinfuegen.Click
        printlog " Close upcoming 'Section' dialog"
        Kontext
        Active.Setpage TabBereiche
        Kontext "TabBereiche"
        TabBereiche.Ok
    else
        Warnlog "Button 'Insert Section' on Insertbar is not enabled!"
    end if

    printlog "  - Bookmark"
    printlog " Click 'Insert Bookmark' in Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if Textmarke.IsEnabled then
        Textmarke.Click
        printlog " Enter a bookmark name and close dialog"
        Kontext "TextmarkeEinfuegen"
        Textmarken.SetText "Bookmark"
        TextmarkeEinfuegen.OK
    else
        Warnlog "Button 'Bookmark' on Insertbar is not enabled!"
    end if

    'Only with closing the toolbox the dialog is accessible for TT
    printlog " Close toolbar"
    Kontext "Insertbar"
    if Insertbar.Exists then Insertbar.Close

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDoc_TB_InsertFields
    Dim Ende as Integer
    Dim i as Integer
    printlog "- Toolbox ( normal ) : Fields"
    gApplication = "HTML"

    if gPlatgroup = "unx" then
        printlog( "Test not suitable for Unix(like) OS" )
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    printlog "- TearOff Insert Toolbox"
    printlog " Tear off toolbar and open insert toolbar"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    printlog " Click 'Insert Fields' in insertbar"
    Sleep 1
    Feldbefehle.OpenMenu
    wait 500
    Call hMenuSelectNr(6)
    Kontext
    try
        Active.SetPage TabDokumentFeldbefehle
        Kontext "TabDokumentFeldbefehle"
        printlog " Cancel upcoming 'Fields' dialog"
        TabDokumentFeldbefehle.Cancel
    catch
        QAErrorlog "Dialog for Fields is not up !"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------
testcase tHtmlDoc_TB_Form
    printlog "- Toolbox ( normal ) : Formular"
    gApplication = "HTML"

    printlog " Open new document"
    Call hNewDocument
    
    Kontext "FormControls"
    if ( FormControls.Exists( 1 ) ) then
        ' do nothing
    else
        Call hToolbarSelect("Controls", true)
    end if

    try
        printlog " Tear off 'Form' Toolbox"
        Kontext "FormControls"
        if Not FormControls.Exists then
            Warnlog "Toolbar 'FormControls' is not opened!"
            Call hCloseDocument
            goto endsub
        end if
        wait 500
        Pushbutton.Click
        printlog " Insert a pushbutton from toolbox"
        Call hMalZeichnenMitSelektion ( 35, 20, 30, 25 )
        Sleep 2
        Kontext "FormControls"
        if NOT ControlProperties.IsEnabled then
            if gPlatform = "osx" then
                warnlog "Mac OS X (Aqua): Drawing function does not draw anything"
            else
                warnlog "Control properties button disabled!"
            end if
        else
            ControlProperties.Click
            sleep(2)
        end if
        printlog "+ Open Control Properties"
        printlog "   - Control Properties"
        Call sMenufunktionen("-Pushbutton-")

        Kontext "FormControls"
        wait 500
        Radiobutton.Click
        printlog " Insert a Radiobutton from toolbox"
        Call hMalZeichnenMitSelektion ( 32, 20, 42, 25 )
        printlog "+ Open Control Properties"
        Call sMenufunktionen("-Radiobutton-")

        Kontext "FormControls"
        wait 500
        Checkbox.Click
        printlog " Insert a Checkbox from toolbox"
        Call hMalZeichnenMitSelektion ( 45, 20, 55, 25 )
        printlog "+ Open Control Properties"
        Call sMenufunktionen("-Checkbox-")

        Kontext "FormControls"
        printlog " Insert an Edit Field from toolbox"
        Edit.Click
        Wait 500
        Call hMalZeichnenMitSelektion ( 32, 30, 42, 35 )
        printlog "+ Open Control Properties"
        Call sMenufunktionen("-Edit-")

        Kontext "FormControls"
        Listbox.Click
        Wait 500
        printlog " Insert a listbox"
        Call hMalZeichnenMitSelektion ( 45, 30, 55, 35 )
        printlog "+ Step through pages of upcoming Autopilot"
        Call sAutopilot_ListboxCombobox("Listbox")
        printlog "+ Open Control Properties"
        Call sMenufunktionen("-Listbox-")

        printlog " Insert a Formatted Field from toolbox"
        Kontext "FormControls"
        Sleep 1
        Call gMouseClick ( 40, 80 )
        Kontext "FormControls"
        printlog " Click 'Design Mode' in Toolbox"
        wait 500
        SwitchControlDesignMode.Click
        Sleep 1
        printlog " Click 'Design Mode' in Toolbox"
        SwitchControlDesignMode.Click
        printlog "   - Design mode"
        Kontext "FormControls"
        FormControls.Close
    catch
        Exceptlog
    endcatch

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDoc_TB_Rest
    printlog "- Toolbox ( normal ) : Rest"

    printlog " Open new document"
    Call hNewDocument
    printlog "  - Hyperlink"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    try
        printlog " Click 'Hyperlink' in Toolbar"
        Hyperlink.Click
        Sleep 2
        printlog " Click 'Hyperlink' in Toolbar"
        Hyperlink.Click
        Warnlog "Button 'Hyperlink' should have been removed!"
    catch
    endcatch

    printlog "  - OnlineLayout"
    printlog "  - Nonprinting Characters"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    try
        printlog " Click 'Nonprinting Characters' in Toolbar"
        SteuerzeichenEinAus.Click
        Sleep 2
        printlog " Click 'Nonprinting Characters' in Toolbar"
        SteuerzeichenEinAus.Click
    catch
        Warnlog "Button 'Nonprinting Characters' is missing or disabled!"
    endcatch

    printlog "  - Graphics on/off"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    try
        printlog " Click 'Graphics' in Toolbar "
        GrafikenAusEin.Click
        Sleep 2
        printlog " Click 'Graphics' in Toolbar "
        GrafikenAusEin.Click
    catch
        Warnlog "Button 'Graphics' is missing or disabled!"
    endcatch
    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------
