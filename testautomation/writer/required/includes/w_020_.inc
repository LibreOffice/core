'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_020_.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:42:36 $
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
'* short description: Testcases for Toolbars. 
'*
'\******************************************************************

sub w_020_
    printLog Chr(13) + "--------- Toolbars (w_020_.inc) ----------"

'   ** Toolbar
    Call t_TB_Insert
    Call t_TB_InsertFields
    Call t_TB_ShowDrawFunctions
    Call t_TB_Form
    Call t_TB_EditAutotext
    Call t_TB_Graphics
    Call t_TB_DirektCursor
    Call t_TB_Find
    Call t_TB_Rest

end sub

'-----------------------------------------------------------

testcase t_TB_Insert
   qaErrorLog "#158646# - Inserting Indexes causes crash."
   goto endsub
   
    printlog "- Toolbox ( normal ) : All within Insert"

    Call hNewDocument
    printlog " Open new document"
    printlog "- TearOff Insert-Toolbox"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    
    if gApplication <> "HTMLDOKUMENT" then
        Kontext "Insertbar"
        'Insertbar.Move ( 20, 20 )
        printlog "  - Frame"
        printlog " Click 'Frame' and Insert a Frame"
        Rahmen.Click
        Call gMouseMove(40, 40, 60, 60)
        Call wTypeKeys ("<Delete>")
        printlog " Hit 'Delete' to remove the frame"
    end if

    printlog "  - Graphic"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    wait 500
	if gUSeSysDlg = False then
        printlog " Click 'Insert Graphics' in Toolbox"        
        if Grafik.IsEnabled then
            Grafik.Click
            Kontext "GrafikEinfuegenDlg"
            if GrafikEinfuegenDlg.Exists then
                Dateiname.SetText ( ConvertPath ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" ) )
                printlog " Select graphic from file '..\writer\required\input\graphics\jolink.jpg'"
                Oeffnen.Click
            else
                Warnlog "Dialog 'Insert Graphic' not up!"
            end if
            Sleep 1
            Kontext "Active"
            if Active.Exists then
                Warnlog Active.Gettext
                Active.Ok
                Kontext "GrafikEinfuegenDlg"
                GrafikEinfuegenDlg.Cancel
            end if
            Call wTypeKeys ("<Delete>")
            printlog " Hit 'Delete' to remove graphic"
        else
            Warnlog "Button 'Graphic' on Insertbar is not enabled!"
        end if
    end if

    Sleep 2
    printlog "  - Table"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    printlog " Click 'Insert Table' in Toolbox"
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

    if gApplication <> "HTMLDOKUMENT" then
        printlog "  - Footnote"
        Kontext "Insertbar"
        if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
        if Fussnote.IsEnabled then
            Fussnote.Click
            printlog " Click 'Insert Footnote Directly' in Toolbox"
            Call wTypeKeys ("<PageUp>")
        else
            Warnlog "Button 'Footnote' on Insertbar is not enabled!"
        end if
        
        Sleep 2
        printlog "  - Endnote"
        Kontext "Insertbar"
        if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
        printlog " Click 'Insert Endnote Directly' in Toolbox"
        if Endnote.IsEnabled then
            Endnote.CLick
            Call wTypeKeys ("<PageUp>")
        else
            Warnlog "Button 'Endnote' on Insertbar is not enabled!"
        end if

        
        printlog "  - Index Marker"
        Call wTypeKeys ("Index Marker <Shift Home>")
        Kontext "Insertbar"
        if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
        printlog " Click 'Insert Index Marker' in Toolbox"
        if Verzeichnis.IsEnabled then
            Verzeichnis.CLick
            Kontext "VerzeichniseintragEinfuegen"
            VerzeichniseintragEinfuegen.Close
            printlog " Close upcoming 'Insert Index Entry' dialog"
        else
            Warnlog "Button 'Index Marker' on Insertbar is not enabled!"
        end if            
    end if

    printlog "  - Special Character"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if Sonderzeichen.IsEnabled then
        Sonderzeichen.CLick
        Kontext "Sonderzeichen"
        printlog " Click 'Insert Special Character' in Toolbox"
        Sonderzeichen.Cancel
        printlog " Cancel upcoming 'Special Character' dialog"
    else
        Warnlog "Button 'Special Character' on Insertbar is not enabled!"
    end if

    printlog "  - Insert Section"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if BereichEinfuegen.IsEnabled then
        BereichEinfuegen.Click
        printlog " Click 'Insert Section' in Toolbox"
        Kontext
        Active.Setpage TabBereiche
        Kontext "TabBereiche"
        TabBereiche.Ok
        printlog " Close upcoming 'Section' dialog"
    else
        Warnlog "Button 'Insert Section' on Insertbar is not enabled!"
    end if

    printlog "  - Bookmark"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    if Textmarke.IsEnabled then
        Textmarke.CLick
        printlog " Click 'Insert Bookmark' in Toolbox"
        Kontext "TextmarkeEinfuegen"
        Textmarken.SetText "Bookmark"
        printlog " Enter a bookmark name and close dialog"
        TextmarkeEinfuegen.OK
    else
        Warnlog "Button 'Bookmark' on Insertbar is not enabled!"
    end if

    'Only with closing the toolbox the dialog is accessible for TT
    Kontext "Insertbar"           
    if Insertbar.Exists then Insertbar.Close
    printlog " Close toolbar"

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase t_TB_InsertFields
    Dim Ende, i as Integer
    printlog "- Toolbox ( normal ) : Fields"

	if gPlatgroup = "unx" then
		QAErrorlog "Doesn't run on unix"
		goto endsub
	end if	
    Call hNewDocument
    printlog " Open new document"
    printlog "- TearOff Insert Toolbox"
    printlog " Tear off toolbar and open insert toolbar"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    printlog " Click 'Insert Fields' in insertbar"    
    Sleep 1
    Feldbefehle.OpenMenu    
    wait 500    
    if gApplication = "HTMLDOKUMENT" then
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
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase t_TB_ShowDrawFunctions
    printlog "- Toolbox ( normal ) : Draw Functions"
    if gApplication = "HTMLDOKUMENT" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    Call hNewDocument
    printlog " Open new document"
    ViewZoom
    Kontext "Massstab"
    Optimal.Check
    Massstab.OK

    Kontext "DrawBar"
    if Not DrawBar.Exists then Call hToolbarSelect("Drawing", true)
    printlog " Open 'Draw functions' toolbar"
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

    Kontext "Drawbar"
    Drawbar.Close
    printlog " Close toolbar 'Draw Functions'"
    ViewZoom
    Kontext "Massstab"
    Vergroesserung100.Check
    Massstab.OK

    Call hCloseDocument
    printlog " Close active document"

endcase

'-----------------------------------------------------------

testcase t_TB_Form
    printlog "- Toolbox ( normal ) : Formular"

    Call hNewDocument
    printlog " Open new document"
    Sleep 1
    Kontext "FormControls"
    if Not FormControls.Exists then Call hToolbarSelect("Controls", true)

    try
        printlog " Tear off 'Form' Toolbox"
        Kontext "FormControls"
        if Not FormControls.Exists then
            Warnlog "Toolbar 'FormControls' is not opened!"
            Call hCloseDocument
            goto endsub
        end sub
        wait 500
        Pushbutton.Click
        printlog " Insert a pushbutton from toolbox"
        Call hMalZeichnenMitSelektion ( 35, 20, 30, 25 )
        Sleep 2
        Kontext "FormControls"
        if NOT ControlProperties.IsEnabled then
            if gPlatform = "osx" then
                warnlog "#i82427#-Mac OS X (Aqua): Drawing function does not draw anything"
            else
                warnlog "Control properties button disabled!"
            end if
        else
            ControlProperties.Click
            sleep(2)
        end if
        '///+ Open Control Properties"
        printlog "   - Control Properties"
        Call sMenufunktionen("-Pushbutton-")
        
        Kontext "FormControls"
        wait 500
        Radiobutton.Click
        printlog " Insert a Radiobutton from toolbox"
        Call hMalZeichnenMitSelektion ( 32, 20, 42, 25 )
        '///+ Open Control Properties"
        Call sMenufunktionen("-Radiobutton-")
        
        Kontext "FormControls"
        wait 500
        Checkbox.Click
        printlog " Insert a Checkbox from toolbox"
        Call hMalZeichnenMitSelektion ( 45, 20, 55, 25 )
        '///+ Open Control Properties"
        Call sMenufunktionen("-Checkbox-")

        Kontext "FormControls"
        if Ucase(gApplication) <> "HTMLDOKUMENT" then
            wait 500
            Label.Click
            printlog " Insert a textbox from toolbox"
            Call hMalZeichnenMitSelektion ( 57, 20, 68, 25 )
            '///+ Open Control Properties"
            Call sMenufunktionen("-Textbox-")
        end if

        Sleep 3
        Kontext "FormControls"
        printlog " Insert an Edit Field from toolbox"
        Edit.Click
        Wait 500
        Call hMalZeichnenMitSelektion ( 32, 30, 42, 35 )
        '///+ Open Control Properties"
        Call sMenufunktionen("-Edit-")

        Kontext "FormControls"
        Listbox.Click
        Wait 500
        printlog " Insert a listbox"
        Call hMalZeichnenMitSelektion ( 45, 30, 55, 35 )
        '///+ Step through pages of upcoming Autopilot"
        Call sAutopilot_ListboxCombobox("Listbox")
        '///+ Open Control Properties"
        Call sMenufunktionen("-Listbox-")
        
        Kontext "FormControls"
        if Ucase(gApplication) <> "HTMLDOKUMENT" then
            Combobox.Click
            printlog " Insert a Combobox from toolbox"
            Wait 500
            Call hMalZeichnenMitSelektion ( 57, 30, 68, 35 )
            '///+ Step through pages of upcoming Autopilot"
            Call sAutopilot_ListboxCombobox("Combo")
            '///+ Open Control Properties"
            Call sMenufunktionen("-Combobox-")
        end if
        
        printlog " Insert a Formatted Field from toolbox"
        Kontext "FormControls"
        if Ucase(gApplication) <> "HTMLDOKUMENT" then
            FormattedField.Click
            Wait 500
            Call hMalZeichnenMitSelektion ( 45, 50, 55, 55 )
            '///+ Open Control Properties"
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

    Call hNewDocument
    printlog " Open new document"
    Kontext "Insertbar"
    if Not Insertbar.Exists then Call hToolbarSelect("Insert", true)
    Sleep 2
    Autotext.Click
    printlog " Click 'Autotext' in Toolbar"
    wait 500
    Kontext "Active"
    if Active.Exists then
        Active.Gettext
        Active.Ok
    end if
    Sleep 5
    Kontext "Autotext"
    printlog " Close upcoming 'Autotext' dialog"
    Autotext.Close
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase t_TB_Find
    printlog "- Toolbox ( normal ) : Find & Replace"
    Call hNewDocument
    printlog " Open new document"
    Kontext "Standardbar"
    if Not Standardbar.Exists then Call hToolbarSelect("Standard", true)
    printlog " Click 'Find' in Toolbar"
    FindAndReplace.Click
    Kontext "FindAndReplace"
    printlog " Close 'Find&Replace' dialog"
    FindAndReplace.Close
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase t_TB_Graphics
    printlog "- Toolbox ( normal ) : Graphic"

    Call hNewDocument
    printlog " Open new document"
    Call hGrafikEinfuegen ( gTesttoolpath + "writer\required\input\graphics\jolink.jpg" )
    printlog " Insert graphic from file ..\writer\required\input\graphics\jolink.jpg"

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

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase t_TB_DirektCursor
    printlog "- Toolbox ( normal ) : Direct Cursor"
    if gApplication = "HTMLDOKUMENT" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    Call hNewDocument
    printlog " Open new document"
    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    printlog " Click 'Direct Cursor' in Toolbar"
    Direktcursor.Click
    Sleep 1
    Call gMouseDoubleClick(50, 70)
    printlog " Point cursor somewhere in document"
    Call wTypeKeys ("Direct Cursor On")
    printlog " Insert some words in document"

    Kontext "Toolbar"
    if Not Toolbar.Exists then Call hToolbarSelect("Tools", true)
    printlog " Click 'Direct Cursor' in Toolbar"
    Direktcursor.Click
    Call gMouseDoubleClick (80, 20)
    printlog " Insert some text in document"
    Call wTypeKeys ("Direct Cursor Off")

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase t_TB_Rest
    printlog "- Toolbox ( normal ) : Rest"

    Call hNewDocument
    printlog " Open new document"
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
    if gApplication <> "HTMLDOKUMENT" then
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
    Call hCloseDocument
    printlog " Close active document"
endcase
