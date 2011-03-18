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
'\******************************************************************************
sub id_005

    printLog Chr(13) + "---------    id_005    ----------"

    call tiFormatDefault
    call tiFormatLine
    call tdFormatArea
    call tiFormatText
    call tiFormatPositionAndSize
    call tiFormatCharacter
    call tiFormatControlForm
    '       ^ Form
    call tiFormatDimensions
    call tiFormatConnector
    call tiFormat3D_Effects
    call tiFormatNumberingBullets
    call tiFormatCaseCharacter
    call tiFormatParagraph
    call tiFormatPage
    call tiFormatStylesAndFormatting
    call tiFormatStylesSlideDesign
    call tiFormatFontwork
    call tiFormatGroup
    printlog " format->group is also modify->group "

end sub
'------------------------------------------------------------------------------
testcase tiFormatDefault

    printlog "open application"
    Call hNewDocument
    printlog "create rectangle"
    gMouseClick 50,50
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    printlog "Format->Default"
    FormatStandardDraw
    printlog "close application"
    Call hCloseDocument
    
endcase 'tiFormatDefault
'---------------------------------------------------------------------------------------
testcase tiFormatLine

    printlog "open application"
    hNewDocument
    printlog "create rectangle"
    gMouseClick 50,50
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    printlog "Format->Line"
    FormatLine
    printlog "switch to tabpage 'Line'"
    Kontext
    Messagebox.SetPage TabLinie
    kontext "TabLinie"
    Call DialogTest ( TabLinie )

    printlog "switch to tabpage 'Line Styles'"
    Kontext
    Messagebox.SetPage TabLinienstile
    kontext "TabLinienstile"
    Call DialogTest ( TabLinienstile )
    printlog "click 'add...'"
    Hinzufuegen.click
    Kontext "NameDLG"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'name'"
    NameDlg.Cancel

    kontext "TabLinienstile"
    printlog "click 'modify...'"
    Aendern.Click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'name'"
    NameDlg.Cancel
    kontext "TabLinienstile"
    printlog "click 'delete...'"
    Loeschen.Click
    printlog "say NO to messagebox"
    Kontext "Messagebox"
    Messagebox.no
    
    kontext "TabLinienstile"
    printlog "click 'load line styles'"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    printlog "cancel dialog 'open'"
    OeffnenDLG.Cancel
    kontext "TabLinienstile"
    printlog "click 'save line styles'"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    printlog "cancel dialog 'save'"
    SpeichernDLG.Cancel
    printlog "switch to tabpage 'Arrow Styles'"
    Kontext
    Messagebox.SetPage TabLinienenden
    kontext "TabLinienenden"
    Call DialogTest ( TabLinienenden )
    printlog "click 'add...'"
    Hinzufuegen.Click
    Kontext "NameDLG"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'name'"
    NameDlg.Cancel

    kontext "TabLinienenden"
    printlog "click 'modify...'"
    Aendern.Click
    Kontext "Messagebox"
    try
        printlog "say OK to messagebox"
        Messagebox.OK
    catch
        'print "TabLinienenden"
    endcatch
    
    kontext "NameDlg"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'name'"
    NameDlg.Cancel
    
    kontext "TabLinienenden"
    printlog "click 'delete...'"
    Loeschen.Click
    Kontext "Messagebox"
    printlog "say NO to messagebox"
    Messagebox.no
    
    kontext "TabLinienenden"
    printlog "click 'load arrow styles'"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    printlog "cancel dialog 'open'"
    OeffnenDLG.Cancel
    kontext "TabLinienenden"
    printlog "click 'save arrow styles'"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    printlog "cancel dialog 'save'"
    SpeichernDlg.Cancel
    kontext "TabLinienenden"
    printlog "cancel dialog 'line'"
    TabLinienenden.cancel
    printlog "close application"
    Call hCloseDocument
    
endcase 'tiFormatLine
'---------------------------------------------------------------------------------------
testcase tdFormatArea

    printlog "open application"
    Call hNewDocument
    printlog "create rectangle"
    gMouseClick 50,50
    Call hRechteckErstellen (15,15,65,65)
    gMouseClick 30,30
    printlog "Format->Area"
    FormatArea
    WaitSlot (1000)
    Kontext
    printlog "switch to tabpage 'Area'"
    Messagebox.SetPage TabArea
    Kontext "TabArea"
    Call DialogTest ( TabArea )
    Kontext
    printlog "switch to tabpage 'Shadow'"
    Messagebox.SetPage TabSchatten
    kontext "TabSchatten"
    Anzeigen.Check
    Call DialogTest ( TabSchatten )
    printlog "switch to tabpage 'Transparency'"
    Kontext
    printlog "switch to tabpage 'Colors'"
    Messagebox.SetPage TabFarben
    kontext "TabFarben"
    printlog "select the 1st 'color' in the listbox"
    Farbe.select 1
    printlog "select the 1st 'color model' in the listbox"
    Farbmodell.Select 1
    Call DialogTest ( TabFarben,1 )
    printlog "select the 2nd 'color model' in the listbox"
    Farbmodell.Select 2
    Call DialogTest ( TabFarben,2 )
    
    printlog "click button 'Add'"
    Hinzufuegen.click
    Kontext "Messagebox"
    printlog "say OK to the messagebox: the name already exists"
    Messagebox.OK
    kontext "NameDlg"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'Name'"
    NameDlg.Cancel
    kontext "TabFarben"
    printlog "click button 'Delete'"
    Loeschen.click
    Kontext "Messagebox"
    printlog "say NO to the messagebox: realy delete?"
    Messagebox.no

    kontext "TabFarben"
    sleep 1
    printlog "click button 'Save color List'"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    printlog "cancel dialog 'save as'"
    SpeichernDlg.Cancel
    sleep 1
    
    kontext "TabFarben"
    printlog "click button 'Load color List'"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    printlog "cancel dialog 'open'"
    OeffnenDLG.Cancel
    Kontext "TabFarben"

    Kontext
    printlog "switch to tabpage 'Gradients'"
    Messagebox.SetPage TabFarbverlaeufe
    kontext "TabFarbverlaeufe"
    Call DialogTest ( TabFarbverlaeufe )
    
    printlog "click button 'Add'"
    Hinzufuegen.click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'Name'"
    NameDlg.Cancel
    
    kontext "TabFarbverlaeufe"
    printlog "click button 'Mofify'"
    Aendern.Click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'Name'"
    NameDlg.Cancel

    kontext "TabFarbverlaeufe"
    printlog "click button 'Delete'"
    loeschen.click
    try
        kontext "Messagebox"
        printlog "say NO to the messagebox: realy delete?"
        Messagebox.no
    catch
        warnlog "nobody cares about deleting a gradient :-("
    endcatch

    kontext "TabFarbverlaeufe"
    printlog "click button 'Load gradients List'"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    printlog "cancel dialog 'open'"
    OeffnenDLG.Cancel
    kontext "TabFarbverlaeufe"
    printlog "click button 'Save gradients List'"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    printlog "cancel dialog 'save as'"
    SpeichernDLG.Cancel

    Kontext
    printlog "switch to tabpage 'Hatching' "
    Messagebox.SetPage TabSchraffuren
    kontext "TabSchraffuren"
    Call DialogTest ( TabSchraffuren)
    
    printlog "click button 'Add'"
    Hinzufuegen.click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'Name'"
    NameDlg.Cancel

    kontext "TabSchraffuren"
    printlog "click button 'Modify'"
    Aendern.Click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'Name'"
    NameDlg.Cancel

    kontext "TabSchraffuren"
    printlog "click button 'Delete'"
    Loeschen.click
    kontext "Messagebox"
    printlog "say NO to the messagebox: realy delete?"
    Messagebox.no

    kontext "TabSchraffuren"
    printlog "click button 'Load hatches List'"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    printlog "cancel dialog 'open'"
    OeffnenDLG.Cancel
    kontext "TabSchraffuren"
    printlog "click button 'Save hatches List'"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    printlog "cancel dialog 'save as'"
    SpeichernDLG.Cancel

    Kontext
    printlog "switch to tabpage 'Bitmaps'"
    Messagebox.SetPage TabBitmap
    kontext "TabBitmap"
    Call DialogTest ( TabBitmap )
    printlog "click button 'Reset'"
    zurueck.click
    sleep 1
    printlog "click button 'Add'"
    hinzufuegen.click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    printlog "cancel dialog 'Name'"
    NameDlg.cancel

    kontext "TabBitmap"
    try
        printlog "click button 'Modify'"
        Aendern.Click
        Kontext "NameDlg"
        Call DialogTest ( NameDlg )
        printlog "cancel dialog 'Name'"
        NameDlg.cancel
    catch
        WarnLog "Control is disabled - modify bitmap"
    endcatch
    kontext "TabBitmap"
    printlog "click button 'Import'"
    Import.Click
    try
        Kontext "GrafikEinfuegenDlg"
        Call DialogTest ( GrafikEinfuegenDlg )
        Kontext "GrafikEinfuegenDlg"
        printlog "cancel dialog 'Import'"
        GrafikEinfuegenDlg.Cancel
    catch
        Warnlog "Insert graphic does not work"
    endcatch
    
    kontext "TabBitmap"
    printlog "click button 'Delete'"
    loeschen.click
    kontext "Messagebox"
    printlog "say NO to the messagebox: realy delete?"
    Messagebox.no
    
    kontext "TabBitmap"
    printlog "click button 'Load Bitmap List'"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    printlog "cancel dialog 'open'"
    OeffnenDLG.Cancel
    kontext "TabBitmap"
    printlog "click button 'Save Bitmap List'"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    printlog "cancel dialog 'save as'"
    SpeichernDLG.Cancel
    kontext "TabBitmap"
    printlog "cancel dialog 'Area'"
    TabBitmap.Cancel
    printlog "close application"
    Call hCloseDocument
    
endcase 'tdFormatArea
'---------------------------------------------------------------------------------------
testcase tiFormatText
    
    printlog "open application"
    Call  hNewDocument
    printlog "Format->Text"
    FormatTextDraw
    Kontext
    printlog "switch to tabpage 'Text'"
    Messagebox.SetPage TabText
    Kontext "TabText"
    DialogTest ( TabText )
    Kontext
    printlog "switch to tabpage 'Text Animation'"
    Messagebox.SetPage TabLauftext
    Kontext "TabLauftext"
    DialogTest ( TabLauftext )
    printlog "cancel dialog 'text'"
    TabLauftext.Cancel
    printlog "close application"
    Call hCloseDocument

endcase 'tdFormatArea
'---------------------------------------------------------------------------------------
testcase tiFormatPositionAndSize

    printlog "open application"
    Call  hNewDocument
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    printlog "Format->Position And Size"
    ContextPositionAndSize
    Kontext
    printlog "switch to tabpage 'Position'"
    Messagebox.setpage TabPositionAndSize
    Kontext "TabPositionAndSize"
    call Dialogtest ( TabPositionAndSize )
    printlog "Type <right> two times in Position : Base Point"
    kontext "PositionPosition"
    PositionPosition.TypeKeys ("<right>", 2)
    printlog "Type <down> two times in Size : Base Point"
    kontext "SizePosition"
    SizePosition.TypeKeys ("<down>", 2)
    Kontext
    printlog "switch to tabpage 'Rotation'"
    Messagebox.setPage TabDrehung
    Kontext "TabDrehung"
    call Dialogtest ( TabDrehung )
    Kontext
    printlog "switch to tabpage 'Slant & Corner Radius'"
    Messagebox.setpage TabSchraegstellen
    Kontext "TabSchraegstellen"
    call Dialogtest ( TabSchraegstellen )
    printlog "cancel dialog 'Position and Size'"
    TabSchraegstellen.cancel
    printlog "close application"
    Call hCloseDocument
    
endcase 'tiFormatPositionAndSize
'---------------------------------------------------------------------------------------
testcase tiFormatCharacter
    
    printlog "open application"
    Call  hNewDocument
    printlog "Format->Character"
    FormatCharacter
    WaitSlot (1000)
    Kontext
    printlog "switch to tabpage 'Font'"
    Messagebox.SetPage TabFont
    kontext "TabFont"
    sleep 1
    Call DialogTest ( TabFont )
    Kontext
    printlog "switch to tabpage 'Font Effects'"
    Messagebox.SetPage TabFontEffects
    kontext "TabFontEffects"
    sleep 1
    Call DialogTest ( TabFontEffects )
    sleep 1
    Kontext
    printlog "switch to tabpage 'Position'"
    Messagebox.SetPage TabFontPosition
    Kontext "TabFontPosition"
    sleep 1
    Call DialogTest ( TabFontPosition )
    sleep 2
    printlog "cancel dialog 'Character'"
    TabFontPosition.Cancel
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormatCharacter
'---------------------------------------------------------------------------------------
testcase tiFormatControlForm

    printlog "testcase: check if controls are available"
    printlog "open new document"
    Call  hNewDocument
 
    'click in the document to get the focus into the document
    if ( UCase(gApplication) = "DRAW" ) then
        Kontext "DocumentDraw"
        DocumentDraw.MouseDown(50,50)
        DocumentDraw.MouseUp(50,50)
    else 'Impress
        Kontext "DocumentImpress"
        DocumentImpress.MouseDown(50,50)
        DocumentImpress.MouseUp(50,50)
    endif
    
    printlog "open the form controls toolbar"
    call hToolbarSelect("FormControls",true)
    
    kontext "FormControls"    
    printlog "insert a PushButton"
    Pushbutton.Click
    Sleep 1
    gMouseMove (50, 20,70, 40)
    
    printlog "open the control properties dialog"
    FormatControl
    
    Kontext "ControlPropertiesDialog"
    WaitSlot (1000)
    printlog "close the control properties dialog"
    ControlPropertiesDialog.Close
    
    printlog "open the form properties dialog"
    FormatForm
    Kontext "ControlPropertiesDialog"
    WaitSlot (1000)
    printlog "close the form properties dialog"
    ControlPropertiesDialog.Close
    
    printlog "close the form control toolbar"
    call hToolbarSelect("FormControls",false)
    
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormatControlForm
'---------------------------------------------------------------------------------------
testcase tiFormatDimensions

    printlog "open application"
    Call  hNewDocument
    printlog "Format->Dimensions"
    FormatDimensioning
    Kontext "Bemassung"
    DialogTest ( Bemassung )
    printlog "cancel dialog 'Dimensioning'"
    Bemassung.Cancel
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormatDimensions
'---------------------------------------------------------------------------------------
testcase tiFormatConnector

    printlog "open application"
    Call  hNewDocument
    printlog "Format->Connector"
    FormatConnector
    Kontext "Verbinder"
    DialogTest ( Verbinder )
    printlog "cancel dialog 'Connector'"
    Verbinder.Cancel
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormatConnector
'---------------------------------------------------------------------------------------
testcase tiFormat3D_Effects

    printlog "open application"
    Call  hNewDocument
    printlog "Format->3D_Effects"
    Format3D_Effects
    Kontext "Drei_D_Effekte"
    Call DialogTest ( Drei_D_Effekte,1 )
    printlog "click button 'Geometry'"
    Geometrie.Click
    Call DialogTest ( Drei_D_Effekte,2 )
    printlog "click button 'Shading'"
    Darstellung.Click
    Call DialogTest ( Drei_D_Effekte,3 )
    printlog "click button 'Illumination'"
    Beleuchtung.Click
    Call DialogTest ( Drei_D_Effekte,4 )
    printlog "click button 'Textures'"
    Texturen.Click
    Call DialogTest ( Drei_D_Effekte,5 )
    printlog "click button 'Material'"
    Material.Click
    Call DialogTest ( Drei_D_Effekte,6 )
    Kontext "Drei_D_Effekte"
    printlog "close dialog '3D Effects'"
    Drei_D_Effekte.Close
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormat3D_Effects
'---------------------------------------------------------------------------------------
testcase tiFormatNumberingBullets

    printlog "open application"
    Call hNewDocument
    WaitSlot (2000)
    printlog "Format->Numbering/Bullets"
    FormatNumberingBulletsDraw
    WaitSlot (2000)
    printlog "switch to tabpage 'Bullets'"
    Kontext
    Messagebox.SetPage TabBullet
    Kontext "TabBullet"
    Call DialogTest ( TabBullet )
    Kontext
    printlog "switch to tabpage 'Numbering Type'"
    Messagebox.SetPage TabNumerierungsart
    Kontext "TabNumerierungsart"
    Call DialogTest ( TabNumerierungsart )
    Kontext
    printlog "switch to tabpage 'Graphics'"
    Messagebox.SetPage TabGrafiken
    Kontext "TabGrafiken"
    Call DialogTest ( TabGrafiken )
    Kontext
    printlog "switch to tabpage 'Position'"
    Messagebox.SetPage TabPositionNumerierung
    Kontext "TabPositionNumerierung"
    Call DialogTest ( TabPositionNumerierung )
    Kontext
    printlog "switch to tabpage 'Customize'"
    Messagebox.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    Call DialogTest ( TabOptionenNumerierung )
    printlog "select 7th entry from the top in the listbox 'Numbering'"
    Numerierung.Select 9 ' last one always ? -> graphics
    printlog "click button graphics 'select'"
    TabOptionenNumerierung.MouseDown 50,60
    TabOptionenNumerierung.MouseUp 50,60
    Auswahl.TypeKeys "<SPACE>"
    printlog "select 'From file'"
    hMenuSelectNr (1)
    sleep 3
    printlog "dialog 'link' with an grayed out & selected link checkbox"
    Kontext "OeffnenDlg"
    printlog "cancel dialog 'link'"
    OeffnenDlg.Cancel
    sleep 1
    sleep 1
    Kontext
    printlog "switch to tabpage 'Customize'"
    Messagebox.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    sleep 1
    try
        printlog "click button graphics 'select'"
        Auswahl.TypeKeys "<SPACE>"
        printlog "select 'Gallery'"
        hMenuSelectNr (2)
        printlog "select 3rd element from the top 'blusquare.gif'"
        hMenuSelectNr (3)
        Sleep 2
    catch
        warnlog "couldn't do something :-) (1)"
        Exceptlog
        Call hMenuClose
    endcatch
    printlog "cancel dialog 'Numbering/Bullets'"
    TabOptionenNumerierung.Cancel
    sleep 1
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormatNumberingBullets
'---------------------------------------------------------------------------------------
testcase tiFormatCaseCharacter

    printlog "open application"
    Call  hNewDocument
    printlog "create textbox with text"
    Call hTextrahmenErstellen ("testit",20,20,50,30)
    sleep 1
    printlog "move curser 1 time to the left"
    hTypeKeys "<left>"
    
    printlog "Format->Case/Characters->Uppercase"
    FormatChangeCaseUpper
    WaitSlot (1000)
    printlog "Format->Case/Characters->Lowercase"
    FormatChangeCaseLower
    WaitSlot (2000)
    if bAsianLan then
        if not gAsianSup then
            qaerrorlog "This is an asian language-office, but asian support was disabled in a previous test?"
        end if
        printlog "if asian language (81/82/86/88):"
        try
           printlog "+ Format->Case/Characters->Half-width"
            FormatChangeCaseHalfWidth
        catch
            Warnlog "Format / Change Case / Half Width does not work."
        endcatch
        WaitSlot (1000)
        try
           printlog "+ Format->Case/Characters->Full-width"
            FormatChangeCaseFullWidth
        catch
            Warnlog "Format / Change Case / Full Width does not work!"
        endcatch
        sleep 1
        try
           printlog "+ Format->Case/Characters->Hiragana"
            FormatChangeCaseHiragana
        catch
            Warnlog "Format / Change Case / Hiragana does not work."
        endcatch
        sleep 1
        try
           printlog "+ Format->Case/Characters->Katatana"
            FormatChangeCaseKatagana
        catch
            Warnlog "Format / Change Case / Katagana does not work."
        endcatch
    end if
    printlog "close application"
    Call hCloseDocument
 
endcase 'tiFormatCaseCharacter
'---------------------------------------------------------------------------------------
testcase tiFormatParagraph
    
    printlog "open application"
    Call  hNewDocument
    printlog "Format->Paragraph"
    FormatParagraph
    Kontext
    printlog "switch to tabpage 'Indents & Spacing'"
    Messagebox.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    Call DialogTest ( TabEinzuegeUndAbstaende )
    Kontext
    printlog "switch to tabpage 'Alignment'"
    Messagebox.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    Call DialogTest ( TabAusrichtungAbsatz )
    Kontext
    printlog "switch to tabpage 'Tabs'"
    Messagebox.SetPage TabTabulator
    kontext "TabTabulator"
    Call DialogTest ( TabTabulator )
    printlog "cancel dialog 'Paragraph'"
    TabTabulator.Cancel
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormatParagraph
'---------------------------------------------------------------------------------------
testcase tiFormatPage

    printlog "open application"
    Call hNewDocument
    printlog "Format->Page"
    FormatSlideDraw
    kontext
    if Messagebox.exists (5) then
    printlog "switch to tabpage 'Page'"
    Messagebox.SetPage TabSeite
    Kontext "TabSeite"
    if TabSeite.exists (5) then
     Call Dialogtest (TabSeite)
    else
     warnlog "nope :-(1"
    endif
    sleep 1
    kontext
    printlog "switch to tabpage 'Background'"
    Messagebox.SetPage TabArea
    sleep 1
    kontext
    if messagebox.GetRT = 304 then
         printlog "active about pagesize != printersettings, will say NO: " + Messagebox.GetText
         try
            Messagebox.No
         catch
           warnlog messagebox.getText
           Messagebox.ok ' should be Error loading BASIC of document ##?
           kontext
           if messagebox.GetRT = 304 then
              try
                 warnlog messagebox.getText
                 Messagebox.ok
              catch
                 printlog "not expected state."
              endcatch
            endif
         endcatch
     endif
      sleep 1
      kontext
      Messagebox.SetPage TabArea
      Kontext "TabArea"
      if TabArea.exists (5) then
         Call Dialogtest (TabArea)
      endif
      sleep 1
      printlog "cancel dialog 'Page Setup'"
      TabArea.Cancel
    else
        warnlog "FormatPage doesn't come up with dialog :-("
    endif
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormatParagraph
'---------------------------------------------------------------------------------------
testcase tiFormatStylesAndFormatting

    Dim sTemp as String
    dim sSettings(20,3) ' Control_name; control_type; value
    dim i as integer
    dim abctemp

    printlog "Open Application"
    
    WaitSlot (10000)
    printlog "Open new document"
    Call  hNewDocument
    printlog "Create a Textframe with content"
    sleep 5

    hTextrahmenErstellen ("I love Wednesdays...",20,20,80,40)
    sleep 1
    printlog "Checking if TextObjectBar is up"
    Kontext "TextObjectbar"
    if TextObjectbar.Exists Then
    	printlog "TextObjectbar.Exists = " + TextObjectbar.Exists
    else
        ViewToolbarsTextFormatting
    endif
    FormatStylist
    printlog "Open Stylist with: Format -> Stylist"
    FormatStylist
    WaitSlot (1000)
    Kontext "Stylist"
    if (Stylist.NotExists) then
       qaErrorLog "There is no stylist open, trying again now"
       FormatStylist
    end if
    WaitSlot (1000)
    Vorlagenliste.TypeKeys "<End>"
    Vorlagenliste.TypeKeys "<Up>"
    Vorlagenliste.TypeKeys "<Up>"
    sleep 1
    printlog "open context menu, select first entry from top: 'New'"
    Vorlagenliste.OpenContextMenu
    sleep 1
    hMenuSelectNr (1)
    sleep 1
    
    Kontext
    if Messagebox.exists (5) then
        printlog "switch to tabpage Organizer"
        try
            Messagebox.SetPage TabVerwalten
            Kontext "TabVerwalten"
            TabVerwalten.TypeKeys "<TAB>"
            VorlagenName.setText("1Test")
            sTemp = VorlagenName.getText
            VerknuepftMit.getSelText
            Bereich.getSelText
            printlog "close dialog 'Graphics Styles'"
            TabVerwalten.OK
        catch
            warnlog "Under Gnome we have a focus problem here."
        endcatch
    end if
    sleep 1
    Kontext "Stylist"
    Vorlagenliste.TypeKeys "<Home>"  'to go to the style we've created ourselves.
    sleep 1
    Vorlagenliste.OpenContextMenu
    sleep 1
    hMenuSelectNr (2) 'modify...
    sleep 1
    Kontext
    if Messagebox.exists (5) then
        printlog "switch to tabpage Organizer"
        try
            Messagebox.SetPage TabVerwalten
            Kontext "TabVerwalten"
            VorlagenName.setText("2Test")
            printlog "close dialog 'Graphics Styles'"
            TabVerwalten.OK
        catch
            warnlog "Under Gnome we have a focus problem here."
        endcatch
    end if
    
    sleep 3
    Kontext "Stylist"
    printlog " Delete the style we created."
    Vorlagenliste.TypeKeys "<Home>"  'to go to the style we've created ourselves.
    sleep 1
    try
       Vorlagenliste.TypeKeys "<Delete>" 'To delete the style.
       Kontext "Active" 'do you really wish to delete?
       Active.YES
       sleep 2
    catch
       Warnlog "Couldnt delete the new Style, or maybe wrong position?"
    endcatch
    Kontext "Stylist"
    if (Stylist.NotExists) then
       ErrorLog "There was no Stylist open, should be."
    else
        printlog "Close Stylist"
        if lcase(gPlatform) = "osx" then
            hTypekeys "<mod1 t>"
        else
            hTypekeys "<F11>"
        endif
        Kontext "Stylist"
        if (Stylist.Exists) then
            ErrorLog "The Stylist should be closed now."
        endif
    endif
    Call hCloseDocument
    
endcase 'tiFormatStylesAndFormatting
'---------------------------------------------------------------------------------------
testcase tiFormatFontwork

    printlog "open application"
    Call hNewDocument
    printlog "create a textframe with text"
    Call hTextrahmenErstellen ("Flightplanning via www.aua.com is hard!",20,20,50,30)
    sleep 1
    printlog "Format->Fontwork"
    FormatFontwork
    Kontext "FontWork"
    if FontWork.exists (5) then
    DialogTest ( FontWork )
    sleep 1
    printlog "close dialog 'Fontwork'"
    FontWork.Close
    else
        warnlog "FontWork didn't came up :-("
    endif
    printlog "close application"
    Call hCloseDocument
   
endcase 'tiFormatFontwork
'---------------------------------------------------------------------------------------
testcase tiFormatGroup

    printlog "open application"
    Call hNewDocument
    Call sSelectEmptyLayout
    printlog "create 2 rectangles"
    hRechteckErstellen ( 10, 10, 20, 20 )
    hRechteckErstellen ( 30, 30, 40, 40 )
    printlog "select both objects with keys [strg]+[a]"
    EditSelectAll
    printlog "DRAW: in Modify menu"
    printlog "+ Impress in Format menu"
    printlog "+ Format->Group / Modify->Group"
    FormatGroupDraw
    WaitSlot (1000)
    printlog "Format->Edit Group / Modify->Enter Group"
    FormatEditGroupDraw
    WaitSlot (1000)
    printlog "Format->Exit Group / Modify->Exit Group"
    FormatExitGroupDraw
    WaitSlot (1000)
    printlog "Format->Ungroup Group / Modify->Ungroup Group"
    FormatUngroupDraw
    WaitSlot (1000)
    printlog "close application"
   Call hCloseDocument
   
endcase 'tiFormatGroup
'---------------------------------------------------------------------------------------
testcase tiFormatStylesSlideDesign

    ' create recktanglr; click outside ?
    printlog "open application"
   Call hNewDocument
    WaitSlot (3000)
    printlog "Format->Styles->Slide Design"
    FormatModifyLayout ' is OK : Format->Styles->Slide Design; 27064; SID_PRESENTATION_LAYOUT
    WaitSlot (1000)
    Kontext "Seitenvorlage"
    Call DialogTest ( Seitenvorlage )
    printlog "check the checkboxes: ExchangeBackgroundPages and DeleteUnusedBackgrounds"
      HintergrundseiteAustauschen.check
      DeleteUnusedBackgrounds.check
    printlog "click button 'Load...'"
      Laden.Click
      sleep (10)
    Kontext "Neu"
         Zusaetze.click
         sleep 1
      kontext "Neu"
    printlog "check checkbox 'Preview'"
    try
         Vorschau.check
    catch
      printlog "Preview wasn't checkable :-( hopfully now:"
         Zusaetze.click
         sleep 1
         Vorschau.check
      printlog "... OK :-)"
    endcatch
    printlog "cancel dialog 'Load Slide Design'"
      Neu.cancel
    Kontext "Seitenvorlage"
    printlog "cancel dialog 'Slide Design'"
    Seitenvorlage.Cancel
    sleep 2
    printlog "close application"
   Call hCloseDocument
   
endcase 'tiFormatStylesSlideDesign
'---------------------------------------------------------------------------------------
