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
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'**************************************************************************************
' #1 tiFormatLine
' #1 tLineConnect
'\*************************************************************************************
testcase tiFormatLine

    Dim ZaehlerStil
    Dim ZaehlerFarbe
    Dim ZaehlerTransp
    Dim i               'Variable fuer Stil (Anzahl)
    Dim j               'Variable fur Farbe (Anzahl)
    Dim k               'Variable fuer Transparenz (Anzahl)
    Dim l
    Dim ZaehlerStilLinks
    
    Call hNewDocument       '/// New impress document
    sleep 3
    Call hRechteckErstellen (10,10,60,60)     '/// create rectangle
    sleep 2
    FormatLine        '/// open Format line dialog
    Kontext
    Active.SetPage TabLinie '///Open TabLine Tabpage
    Kontext "TabLinie"
    sleep 2
    Stil.GetItemCount       '/// Get item count for style ///'
    Farbe.GetItemCount       '/// Get item count for color ///'
    Breite.More 1        '/// change value for ///'
    ZaehlerStil = Stil.GetItemCount
    ZaehlerFarbe = Farbe.GetItemCount
    
    for i=1 to ZaehlerStil      '/// Apply every style to the rectangle ///'
       wait 10
       Stil.Select i
       wait 10
       TabLinie.OK '/// Closing dialog with ok
    '    Kontext "DocumentImpress"
       FormatLine '///Open TabLine Tabpage
       Kontext
       Active.SetPage TabLinie
       Kontext "TabLinie"
    next i
    
    for j=1 to ZaehlerFarbe      '/// Apply every color to the rectangle///'
       PrintLog "-- " + Farbe.GetSelText
       wait 10
       Farbe.Select j
       wait 10
       TabLinie.OK
    '    Kontext "DocumentImpress"
       FormatLine
       Kontext
       Active.SetPage TabLinie
       Kontext "TabLinie"
    next j
    Breite.More 3        '/// change Breite ///'
    Breite.Less 2
    for k=1 to 6
       Transparenz.More 1      '/// change tranparence///'
    next k
    TabLinie.OK
    '---------------------------------------
    sleep 2
    EditSelectAll     '/// Select all objects in document ///'
    sleep 2
    hTypeKeys "<DELETE>"     '/// Delete objects ///'
    sleep 2
    FormatLine
    
    Kontext
    Active.SetPage TabLinie      '/// TabLine ///'
    Kontext "TabLinie"
    StilLinks.GetItemCount
    ZaehlerStilLinks=StilLinks.GetItemCount
    for l=1 to ZaehlerStilLinks      '/// Apply all line end styles///'
      wait 10
      if EndenSynchronisieren.IsChecked=False Then EndenSynchronisieren.Check '/// check synchronize ///'
      wait 10
      StilLinks.Select l
      if StilLinks.GetSelText<>StilRechts.GetSelText Then
         WarnLog "  Ends not synchronized. " + StilLinks.GetSelText + "   Right:  " + StilRechts.GetSelText   '/// check if style is automatically applied for both ends ///'
      else
         PrintLog "  Left:  " + StilLinks.GetSelText + "   Right:  " + StilRechts.GetSelText
      end if
       EndenSynchronisieren.UnCheck
    next l
    ZentriertLinks.Check
    if ZentriertRechts.isChecked = True Then PrintLog "  Centered right does work"
    i = CornerStyle.GetItemCount
    if i <> 4 then warnlog "CornerStyle should contain four options, but currently has: " + i
    for i = 1 to 4
    CornerStyle.Select i
    if CornerStyle.GetSelIndex <> i then warnlog "Corner Styles was: " + CornerStyle.GetSelIndex + ", should have been " + i
    Printlog "Corner Style nr: " + i + " = " + CornerStyle.GetSelText
    next i
    TabLinie.OK
    '------------------------------------------
    sleep 2
    FormatLine
    Kontext
    Active.SetPage TabLinienstile
    Kontext "TabLinienstile"
    Hinzufuegen.Click        '/// Add new line style ///'
    Kontext "NameDlg"
    Eingabefeld.SetText "Testlinie2"      '/// insert name of style (Testlinie2) ///'
    NameDlg.OK
    sleep 1
    Kontext
    Active.SetPage TabLinienstile
    Kontext "TabLinienstile"
    Loeschen.Click         '/// delete created style ///'
    Kontext "Active"
    Active.Yes
    Kontext
    Active.SetPage TabLinienstile
    Kontext "TabLinienstile"
    TypLinks.Select 2
    TypRechts.Select 2
    AnzahlLinks.SetText "20"
    AnzahlRechts.SetText "5"
    LaengeLinks.More 3
    LaengeRechts.More 3
    Abstand.SetText "0,1"
    if AnLinienbreite.IsChecked = True Then AnLinienbreite.Click   '/// all fields changed in TabLinienstile ///'
    PrintLog "  All controls could be manipulated."
    TypLinks.Select 1
    TypRechts.Select 1
    if LaengeLinks.IsEnabled And LaengeRechts.IsEnabled =True Then  '/// check if its possible to apply a length to a point ///'
     WarnLog "  A point where you can change its length is not a point anymore"
    else
     PrintLog "  Line style pint tested"    '/// All styles for points are changed  ///'
    end if
    sleep 1
    try
     Linienstil.Select 1
    catch
     if Linienstil.GetItemCount = 0 then     '/// check if style list is filled ///'
        Warnlog "- The list for line styles is empty"
        Hinzufuegen.Click      '/// new style ///'
        Kontext "NameDlg"
        NameDlg.OK
     end if
    endcatch
    Kontext
    Active.SetPage TabLinienstile
    Kontext "TabLinienstile"
    Aendern.Click         '/// change style ///'
    Kontext "NameDlg"
    Eingabefeld.SetText Eingabefeld.GetText + "1"     '/// new name for changed style ///'
    SetClipboard Eingabefeld.GetText      '/// put name into clipboard ///'
    NameDlg.OK
    Kontext
    Active.SetPage TabLinienstile
    Kontext "TabLinienstile"
    TabLinienstile.OK
    sleep 2
    FormatLine
    Kontext
    Active.SetPage TabLinienstile
    Kontext "TabLinienstile"
    try
    Aendern.Click
    catch
    Hinzufuegen.Click
    endcatch
    Kontext "NameDlg"
    if Eingabefeld.GetText <> GetClipboardText Then WarnLog "No changes for Line style"  '/// check if changed style is in list ///'
    NameDlg.Cancel
    sleep 2
    Kontext
    Active.SetPage TabLinienstile
    Kontext "TabLinienstile"
    sleep 1
    try
     Loeschen.Click           '/// delete changed style ///'
     Kontext "Active"
     Active.Yes
     sleep 3
    catch
    if Linienstile.GetItemCount = 0 Then
       Hinzufuegen.Click
       Kontext "NameDlg"
       sleep 1
       Eingabefeld.SetText "Delete"
       NameDlg.OK
    end if
    endcatch
    Kontext
    Active.SetPage TabLinienstile
    Kontext "TabLinienstile"
    sleep 1
    TabLinienstile.OK
    '--------------------------------------------
    sleep 2
    Call hRechteckErstellen (40,40,80,50)         '/// create rectangle ///'
    sleep 2
    FormatLine
    Kontext
    Active.SetPage TabLinienenden
    Kontext "TabLinienenden"
    sleep 2
    Kontext "Linienstil"
    if Linienstil.Exists then Aendern.Click        '/// change style ///'
    sleep 1
    Kontext
    Active.SetPage TabLinienenden
    Kontext "TabLinienenden"
    sleep 1
    Liste.GetItemCount
    Liste.Select 3
    Hinzufuegen.Click           '/// add style ///'
    Kontext "NameDlg"
    NameDlg.OK
    
    Kontext
    Active.SetPage TabLinienenden
    Kontext "TabLinienenden"
    Aendern.Click            '/// change style ///'
    Kontext "Active"
    Printlog Active.GetText
    Active.OK
    Kontext "NameDlg"
    Eingabefeld.SetText Eingabefeld.GetText + "1"
    PrintLog "  Name inserted"
    NameDlg.OK
    
    Kontext
    if active.GetRt=304 then
     active.ok
     Kontext "NameDlg"
     Eingabefeld.SetText Eingabefeld.GetText + "1"
     PrintLog "  Name inserted"
     NameDlg.OK
    endif
    if active.GetRt=304 then
     active.ok
     warnlog "still not a valid name :-("
    endif
    Kontext
    Active.SetPage TabLinienenden
    Kontext "TabLinienenden"
    Loeschen.Click           '/// delete style ///'
    Kontext "Active"
    Active.Yes
    
    Kontext
    Active.SetPage TabLinienenden
    Kontext "TabLinienenden"
    sleep 1
    Speichern.Click          '/// save style ///'
    sleep 1
    Kontext "SpeichernDlg"
    sleep 1
    SpeichernDlg.Cancel
    
    Kontext "TabLinienenden"
    TabLinienenden.Cancel
    sleep 2
    
    '///New part for line
    'EditSelectAll     '/// Select all objects in document ///'
    'sleep 2
    'hTypeKeys "<DELETE>"     '/// Delete objects ///'
    'sleep 2
    
    '/// Create line to test shadow tabpage in line dialog
    'kontext "Toolbar"
    'Kurven.Click
    'sleep 1
    'if (gApplication = "DRAW") then
    '    Kontext "DocumentDraw"
    '    gMouseMove (30,50, 40,60)
    '    sleep 1
    'else
    '    Kontext "DocumentImpress"
    '    gMouseMove (30,50, 40,60)
    '    sleep 1
    'endif
    'FormatLine
    'Kontext
    'Active.SetPage TabSchatten
    
   ' FormatLine.Cancel
    Call hCloseDocument          '/// close document ///'
    
endcase 'tiFormatLine
'-------------------------------------------------------------------------------'
testcase tLineConnect

    Dim value1 as integer
    Dim value2 as integer
    Dim value3 as integer
    Dim StatusBarText as string
    Dim i as integer
    Dim cname as string
    Dim cfirst as integer
    
    if bAsianLan then 
     QaErrorLog "tLineConnect ends because Asian languages are not fully supported."
     goto Endsub
    end if
    
    Call hNewDocument   '/// New Impress / Draw document ///'
    Call sSelectEmptyLayout
    sleep 3
    kontext "GraphicObjectbar"
    if GraphicObjectbar.Exists then
     if GraphicObjectbar.isDocked = False then
        GraphicObjectbar.Move 900,900
     endif
    endif
    kontext "Gluepointsobjectbar"
    if Gluepointsobjectbar.Exists then
     if Gluepointsobjectbar.isDocked = false then
        Gluepointsobjectbar.Move 900,900
     endif
    endif
    kontext "Optionsbar"
    if Optionsbar.Exists then
     if Optionsbar.isDocked = false then
        Optionsbar.Move 900,900
     end if
    endif
    
    '/// Create 2 lines ///'
    kontext "Toolbar"
    Kurven.Click
    sleep 1
    if (gApplication = "DRAW") then
     Kontext "DocumentDraw"
     gMouseMove (30,50, 40,60)
     sleep 1
     kontext "Toolbar"
     Kurven.Click
     Kontext "DocumentDraw"
     gMouseMove (70,50, 60,60)
    else
     Kontext "DocumentImpress"
     gMouseMove (30,50, 40,60)
     sleep 1
     kontext "Toolbar"
     Kurven.Click
     Kontext "DocumentImpress"
     gMouseMove (70,50, 60,60)
    endif
    
    printlog "'" + (gApplication) +"'"
    
    '/// Check the amount of objects ///'
    gMouseClick (10, 10)
    sleep 1
    EditSelectAll
    sleep 1
    if (gApplication = "IMPRESS") then
        StatusBarText = DocumentImpress.StatusGetText(DocumentImpress.StatusGetItemID(1))
    else
        kontext "DocumentDraw"
        StatusBarText = DocumentDraw.StatusGetText(DocumentDraw.StatusGetItemID(1))
    endif
    if bAsianLan then 
        value1 = left(StatusBarText,(InStr(StatusBarText, "2" ))) 
        if value1 <> 2 then
            warnlog "UH? There is supposed to be two objects visible, but was: " + value1 + " and the StatusBarText said: " + StatusBarText + "."
        end if
    else
        value1 = left(StatusBarText,1)
        printlog "StatusBarText was: " + StatusBarText
        if value1 <> 2 then
            warnlog "UH? There is supposed to be two objects visible, but we found " + value1 + " objects."
        end if
    end if
    
    '/// Connect the two objects selected ///'
    hOpenContextMenu
    sleep 1
    hMenuSelectNr(13)
    sleep 1
    '/// Check the amount of objects ///'
    gMouseClick (10, 10)
    sleep 1
    EditSelectAll
    sleep 1
    if (gApplication = "IMPRESS") then
     StatusBarText = DocumentImpress.StatusGetText(DocumentImpress.StatusGetItemID(1))
    else
     kontext "DocumentDraw"
     StatusBarText = DocumentDraw.StatusGetText(DocumentDraw.StatusGetItemID(1))
    endif
    value2 = left(StatusBarText,1)
    printlog "StatusBarText was: " + StatusBarText
    
    '/// if same as the first value = wrong ///'
    if value1 = value2 then
     warnlog "Expected to find one object, but found " + value2 + " instead."
    endif
    if value2 = "2" then
     warnlog "Connect was not successful. We should have one object, but have " + value2 + " instead."
    endif
    
    '/// unmark the objects, thereafter mark them again ///'
    gMouseClick (10, 10)
    sleep 1
    EditSelectAll
    sleep 1
    '/// Break them from eachother ///'
    hOpenContextMenu
    sleep 1
    hMenuSelectNr(11)
    sleep 1
    '/// unmark the objects, thereafter mark them again ///'
    gMouseClick (10, 10)
    sleep 1
    EditSelectAll
    sleep 1
    '/// Check the amount of objects ///'
    if (gApplication = "IMPRESS") then
     StatusBarText = DocumentImpress.StatusGetText(DocumentImpress.StatusGetItemID(1))
    else
     kontext "DocumentDraw"
     StatusBarText = DocumentDraw.StatusGetText(DocumentDraw.StatusGetItemID(1))
    endif
    value3 = left(StatusBarText,1)
    
    printlog "StatusBarText was: " + StatusBarText
    
    '/// if the same as any of the ones before: Wrong ///'
    if (value3 = value1 OR value3 = value2) then
     warnlog "Expected to find three objects, but found " + value3 + " instead."
    endif
    
    '/// if 3 objects, then everything is ok ///'
    if value3 = 3 then
     printlog "Found three objects. Means the test was successful."
    else
     warnlog "Wrong value, expected three objects, but found " + value3 + "."
    endif
    
    '/// Go through the different Corner-styles ///'
    
    hOpenContextMenu
    sleep 1
    hMenuSelectNr(2) 'Choose "Line"
    
    Kontext "TabLinie"
    if TabLinie.Exists(2) then
        cfirst = CornerStyle.GetSelIndex
    else
        sleep 1
    endif
    
    For i = 1 to CornerStyle.GetItemCount
      Kontext "TabLinie"
      if (i > 1) AND (cname <> CornerStyle.GetSelText) then
         warnlog "Cornerstylename wasnt saved. Should have been: " + cname + ". But was: " + CornerStyle.GetSelText
      endif
      CornerStyle.Select i
      cname = CornerStyle.GetSelText
      Printlog " Found and selected Corner-Style: " + cname
      TabLinie.Ok
      sleep 1
      hOpenContextMenu
      hMenuSelectNr(2) 'Choose "Line"
    Next i
    
    Kontext "TabLinie"
    CornerStyle.Select cfirst
    TabLinie.Ok
    
    '/// And a nice finish to make life a bit happier for anyone who looks at the test ///'
    kontext "Toolbar"
    Ellipsen.Click
    if (gApplication = "DRAW") then
    Kontext "DocumentDraw"
    gMouseMove (40,40, 50,30)
    sleep 1
    kontext "Toolbar"
    Ellipsen.Click
    Kontext "DocumentDraw"
    gMouseMove (51,40, 61,30)
    kontext "Toolbar"
    Ellipsen.Click
    Kontext "DocumentDraw"
    gMouseMove (48,43, 53,50)
    else
    Kontext "DocumentImpress"
    gMouseMove (40,40, 50,30)
    sleep 1
    kontext "Toolbar"
    Ellipsen.Click
    Kontext "DocumentImpress"
    gMouseMove (51,40, 61,30)
    kontext "Toolbar"
    Ellipsen.Click
    Kontext "DocumentImpress"
    gMouseMove (48,43, 53,50)
    endif
    gMouseClick (10, 10)
    '/// Close the document ///'
    hCloseDocument
    '/// Endcase ///'
    
endcase 'tiLineConnect
'-------------------------------------------------------------------------------'
