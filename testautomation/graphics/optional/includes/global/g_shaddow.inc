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
'* short description : Tests the shaddow-function on a picture
'*
'*******************************************************************
'*
' #1 tiShaddow
'*
'\*******************************************************************
testcase tiShaddow

   dim sFilter as string
   dim i as integer
   dim t as integer
   dim q as integer
   dim PosX as integer
   dim PosY as integer
   dim e as string
   dim sFileName as string
   dim ImageWidth as string
   dim ImageHeight as string
   dim shadowdistancevalue as string
   dim shadowcolorvalue as string
   dim shadowtransparencyvalue as string
   dim linetype as string
   dim linecolour as string
   dim linewidth as string
   dim cornerstyletype as string

    printlog "Set the file-extension we'll be using when opening the saved file "
    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

   printlog "Open application "
   Call hNewDocument
   Call sSelectEmptyLayout
   sleep 1

   printlog "Import picture "
   InsertGraphicsFromFile
      sleep 3
      kontext "Active"
      if Active.Exists Then
         Active.OK
      end if
      sleep 5
      Kontext "GrafikEinfuegenDlg"
      sleep 2
      Dateiname.SetText (ConvertPath(gOfficeBasisPath + "share\gallery\bigapple.gif"))
      sleep 2
      Oeffnen.Click
   printlog "Inserted file"
   sleep 2
   printlog "Select all"
   EditSelectAll
   sleep 3
   printlog "Check values and save them"
   FormatPositionAndSize
      kontext
      active.SetPage TabPositionAndSize
      kontext "TabPositionAndSize"
      ImageWidth = Width.GetText
      ImageHeight = Height.GetText
      printlog "Got values from position and size"
   printlog "Check the Protect Size-button. And close dialogue. "
      ProtectSize.Check
      TabPositionAndSize.Ok

   printlog "Add Shaddow via Toolbar-Button "
       Kontext "GraphicObjectbar"
       if GraphicObjectbar.Exists = FALSE then 
           ViewToolbarsPicture
       endif
       if schatten.GetState(2) <> 0 then  '0 = not pressed. 1 = pressed.
           warnlog "Shaddow-button shouldnt have been checked"
       end if
       sleep 1

    printlog "click button 'Shadow'"
    Schatten.Click
    printlog " Check and save Shaddow-values via context-menu "
    EditSelectAll
    hOpenContextMenu
    sleep(2)
    if hMenuFindSelect(10142, true, 4) = false then
         Warnlog "Context-Menu-entry `Area` was not found. Therefore the test ends."
         Call hCloseDocument
         Goto Endsub
    endif
    printlog " Select Contextmenu entry Area "
    sleep(2)
   kontext "TabArea"
   printlog "Switch to Shaddow-tabpage, do some changes, save them, close dialogue"
   Kontext
   active.SetPage TabSchatten
   kontext "TabSchatten"
      if Anzeigen.isVisible(5) AND Anzeigen.Exists(5) then
         if Anzeigen.GetState <> 1 then Warnlog "Anzeigen.GetState should be 1, but is: " + Anzeigen.GetState
         Anzeigen.Check
      else
         Warnlog "'Use Shadow' doesn't exist or isn't visible."
      endif

    printlog "Change Distance and Transparency-values "
    Entfernung.More ',2
    Transparenz.More ',2
    shadowdistancevalue = Entfernung.GetText
    shadowcolorvalue = Farbe.GetSelText
    shadowtransparencyvalue = Transparenz.GetText
    printlog "Got values from TabArea"
    TabSchatten.Ok

   printlog "Check if it's the same when going over FormatArea "
   FormatArea
   sleep 1
   kontext "TabArea"
   printlog "Switch to Shaddow-tabpage, do some changes, save them, close dialogue "
   Kontext
   active.SetPage TabSchatten
   kontext "TabSchatten"
      if Anzeigen.isVisible(5) AND Anzeigen.Exists(5) then
         if Anzeigen.GetState <> 1 then Warnlog "should be active"
         Anzeigen.Check
      else
         Warnlog "'Use Shadow' doesn't exist or isn't visible."
      endif
    if shadowdistancevalue <> Entfernung.GetText then 
        warnlog "Distance value changed. Should be: " + shadowdistancevalue + ". But is: " + Entfernung.gettext
    endif
    if shadowcolorvalue <> Farbe.GetSelText then
        warnlog "Color value changed. Should be: " + shadowcolorvalue + ". But is: " + Farbe.getSeltext
    endif
    if shadowtransparencyvalue <> Transparenz.GetText then 
        warnlog "Transparency value changed. Should be: " + shadowtransparencyvalue + ". But is: " + Transparenz.gettext
    endif
   TabSchatten.Ok

    printlog "Add Cornerstyle via FormatLine "
    FormatLine
    Kontext
    Messagebox.SetPage TabLinie
    kontext "TabLinie"
    Stil.Select 2    ' Select Continuously.
    linetype = Stil.GetItemText
    Farbe.Select 2 ' Select Blue
    linecolour = Farbe.GetItemText
    Breite.More 3 ' Set Width to 3
    linewidth = Breite.Gettext
    CornerStyle.Select 4 ' Select number four: Beveled
    cornerstyletype = CornerStyle.GetSelText
    printlog "Cornerstyle is: " + CornerStyle.GetSelText '+ cornerstyletype
      TabLinie.ok

   printlog "Check values via PositionAndSize via the Contextmenu "
   hOpenContextmenu
      sleep(2)
      if hMenuFindSelect(10087, true, 1) = false then
         Warnlog "Context-Menu-entry `Position and Size` was not found. Therefore the test ends."
         Call hCloseDocument
         Goto Endsub
      endif
    printlog "Select Position and Size"
    sleep(2)
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if ImageWidth <> Width.GetText then warnlog "Position X differs. Should be: " + ImageWidth + " But is: " + Width.GetText
    if ImageHeight <> Height.GetText then warnlog "Position Y differs. Should be: " + ImageHeight + " But is: " + Height.GetText
   TabPositionAndSize.OK

   printlog "Check Cornerstyle via FormatLine "
    FormatLine
    Kontext
    Messagebox.SetPage TabLinie
    kontext "TabLinie"
    if linetype <> Stil.GetItemText then warnlog "Linetype should be: " + linetype + " But is: " + Stil.GetItemText
    if linecolour <> Farbe.GetItemText then warnlog "Linecolour should be: " + linetype + " But is: " + Farbe.GetItemText
    if linewidth <> Breite.GetText then warnlog "Linewidth should be: " + linewidth + " But is: " + Breite.GetText
    if cornerstyletype <> CornerStyle.GetSelText then warnlog "CornerStyle should be: " + cornerstyletype + " But is: " + CornerStyle.GetSelText 'ItemText
    TabLinie.ok

    printlog "Check if the Shaddow-button is activated "
    Kontext "GraphicObjectbar"
    if Schatten.GetState(2) <> 1 then
        warnlog "Shaddow-button shouldnt have been checked"
    endif

   printlog "Save the file. Then close the office and reload the file "
   sFileName = ( ConvertPath(gOfficePath + "user\work\shaddow-test"))
   printlog "File-Save As with filter as: user\temp\shaddow-test "
   try
      FileSaveAs
         Kontext "SpeichernDlg"
         Dateiname.SetText sFileName
         printlog "Saving with filter: " + Dateityp.GetSelText
         Speichern.Click
         Kontext "Messagebox"
         if Messagebox.Exists(2) then Messagebox.Yes
         Kontext "AlienWarning"
            if AlienWarning.Exists(2) then AlienWarning.OK
      printlog "Saved as: " + sFileName
   catch
      warnlog "Error when saving file."
   endcatch

   printlog "Close the office-session "
   FileClose
      Kontext "Messagebox"
      if Messagebox.Exists(2) then Messagebox.Yes
   Sleep 3
   printlog "Open the saved file "
   try
      hFileOpen sFileName + "." + ExtensionString
      Sleep 3
      printlog "opened file successfully"
   catch
      warnlog "Error when opening file."
   endcatch

   printlog "Select all "
   EditSelectAll
   sleep 2

   printlog "Check if the Shaddow-button still is activated "
   kontext "GraphicObjectbar"
   if schatten.Getstate(2) <> 1 then
      warnlog "The Shadow-button should be activated"
   else
      Printlog "Shaddowbutton activated, good."
   endif

   printlog "Check the attributes for the picture "
   hOpenContextmenu
      sleep(2)
      if hMenuFindSelect(10087, true, 1) = false then
         Warnlog "Context-Menu-entry `Position and Size` was not found. Therefore the test ends."
         Call hCloseDocument
         Goto Endsub
      endif
      printlog "Position and Size"
      sleep(2)
      Kontext
      Active.SetPage TabPositionAndSize
      Kontext "TabPositionAndSize"
      if ImageWidth <> Width.GetText then warnlog "Position X differs. Should be: " + ImageWidth + " But is: " + Width.GetText
      if ImageHeight <> Height.GetText then warnlog "Position X differs. Should be: " + ImageHeight + " But is: " + Height.GetText
      TabPositionAndSize.OK

   FormatArea
      sleep 1
      kontext "TabArea"
      printlog "Switch to Shaddow-tabpage, do some changes, save them, close dialogue "
      Kontext
      active.SetPage TabSchatten
      kontext "TabSchatten"
         if Anzeigen.isVisible(5) AND Anzeigen.Exists(5) then
            if Anzeigen.GetState <> 1 then Warnlog "Shaddow-checkbox should be active"
            Anzeigen.Check
         else
            Warnlog "'Use Shadow' doesn't exist or isn't visible."
         endif
         if shadowdistancevalue <> Entfernung.GetText then warnlog "Distance value changed. Should be: " + shadowdistancevalue + ". But is: " + Entfernung.gettext
         if shadowcolorvalue <> Farbe.GetSelText then warnlog "Color value changed. Should be: " + shadowcolorvalue + ". But is: " + Farbe.gettext
         if shadowtransparencyvalue <> Transparenz.GetText then warnlog "Transparency value changed. Should be: " + shadowtransparencyvalue + ". But is: " + Transparenz.gettext

   printlog "Check Transparency"
      Kontext
      Active.SetPage TabTransparenz
      kontext "TabTransparenz"
         LineareTransparenz.Check
         shadowtransparencyvalue = MFLinTransparenz.GetText

   sleep 2
   Kontext
   active.SetPage TabSchatten
   kontext "TabSchatten"
      TabSchatten.Ok

   printlog "Check Cornerstyle via FormatLine "
   FormatLine
      Kontext
      Messagebox.SetPage TabLinie
         kontext "TabLinie"
         if linetype <> Stil.GetItemText then warnlog "Linetype should be: " + linetype + " But is: " + Stil.GetItemText
         if linecolour <> Farbe.GetItemText then warnlog "Linecolour should be: " + linetype + " But is: " + Farbe.GetItemText
         if linewidth <> Breite.GetText then warnlog "Linewidth should be: " + linewidth + " But is: " + Breite.GetText
         if cornerstyletype <> CornerStyle.GetSelText then warnlog "CornerStyle should be: " + cornerstyletype + " But is: " + CornerStyle.GetSelText 'ItemText
      TabLinie.ok

   printlog "If Impress: Save as Powerpoint-file. If Draw: Save as usual Draw-file. "
    if (gApplication = "IMPRESS") then
        ExtensionString = "ppt"
    else
        ExtensionString = "odg"
    end if

   sFileName = ( ConvertPath(gOfficePath + "user\work\shaddow-test"))

   FileSaveAs
      Kontext "SpeichernDlg"
      Dateiname.SetText sFileName
      if (gApplication = "IMPRESS") then
         Dateityp.Select 5  ' Powerpoint
         printlog "Trying to save with filter: " + Dateityp.GetSelText + sFilter(5)
      else
         Dateityp.Select 1
         printlog "Trying to save with filter: " + Dateityp.GetSelText + sFilter(1)
      endif
         Speichern.Click
         Kontext "Messagebox"
         if Messagebox.Exists(2) then Messagebox.Yes
         Kontext "AlienWarning"
         if AlienWarning.Exists(2) then AlienWarning.OK
         printlog "Saved as: " + sFileName
      Sleep 3

   printlog "Close the office and reload the file"
   FileClose
      Kontext "Messagebox"
      if Messagebox.Exists(2) then Messagebox.Yes
   Sleep 3
   printlog " Open the saved file "
   hFileOpen (sFileName + "." + ExtensionString)
   printlog "File opened: " + sFileName + "." + ExtensionString
   Sleep 3
   printlog "Select all"
   EditSelectAll

   printlog "Check the attributes for the picture"
   hOpenContextmenu
      sleep(2)
      if hMenuFindSelect(10087, true, 1) = false then
         Warnlog "Context-Menu-entry `Position and Size` was not found. Therefore the test ends."
         Call hCloseDocument
         Goto Endsub
      endif
      printlog "Select Position and Size"
      sleep(2)
      Kontext
      Active.SetPage TabPositionAndSize
      Kontext "TabPositionAndSize"
      if ImageWidth <> Width.GetText then warnlog "Position X differs. Should be: " + ImageWidth + " But is: " + Width.GetText
      if ImageHeight <> Height.GetText then warnlog "Position X differs. Should be: " + ImageHeight + " But is: " + Height.GetText
      TabPositionAndSize.OK

   printlog "Check if transparency has disappeared and if the cornerstyle has changed"
   FormatArea
      Kontext
      Messagebox.SetPage TabTransparenz
         kontext "TabTransparenz"
         if (gApplication = "DRAW") then
            if LineareTransparenz.Ischecked = FALSE then
               warnlog "Transparency should have been checked!"
               if MFLinTransparenz.GetText <> shadowtransparencyvalue then warnlog "Transparency-value should NOT have changed"
               KeineTransparenz.Check
            end if
         endif
         if (gApplication = "DRAW") then
            if LineareTransparenz.Ischecked = FALSE then
               warnlog "Transparency should have been checked!"
               if MFLinTransparenz.GetText <> shadowtransparencyvalue then warnlog "Transparency-value should NOT have changed"
               KeineTransparenz.Check
            end if
         endif
         if (gApplication = "IMPRESS") then
            if LineareTransparenz.Ischecked =TRUE then
                printlog "Transparency contained in newer ppt files, ok.!"
            if MFLinTransparenz.GetText <> shadowtransparencyvalue then warnlog "Transparency-value should NOT have changed"
               KeineTransparenz.Check
            end if
         endif

   TabTransparenz.Cancel

   printlog "Check Cornerstyle via FormatLine "
   FormatLine
      Kontext
      Messagebox.SetPage TabLinie
         kontext "TabLinie"
         if linetype <> Stil.GetItemText then warnlog "Linetype should be: " + linetype + " But is: " + Stil.GetItemText
         if linecolour <> Farbe.GetItemText then warnlog "Linecolour should be: " + linetype + " But is: " + Farbe.GetItemText
         if linewidth <> Breite.GetText then warnlog "Linewidth should be: " + linewidth + " But is: " + Breite.GetText
         if cornerstyletype <> CornerStyle.GetSelText then warnlog "CornerStyle should be: " + cornerstyletype + " But is: " + CornerStyle.GetSelText 'ItemText
      TabLinie.ok

   printlog "Remove the Shaddow, Transparency and Cornerstyle"
   FormatArea
      Kontext
      Messagebox.SetPage TabTransparenz
         kontext "TabTransparenz"
         KeineTransparenz.Check
      sleep 1
      kontext "TabArea"
         printlog "Switch to Shaddow-tabpage, do some changes, save them, close dialogue"
         Kontext
         active.SetPage TabSchatten
         kontext "TabSchatten"
         Anzeigen.UnCheck
     TabSchatten.Ok

   printlog "If Impress: Save as Powerpoint-file. If Draw: Save as usual Draw-file."
    if (gApplication = "IMPRESS") then
        ExtensionString = "ppt"
    else
        ExtensionString = "odg"
    end if
    sFileName = ( ConvertPath(gOfficePath + "user\work\shaddow-test"))

   FileSaveAs
      Kontext "SpeichernDlg"
      Dateiname.SetText sFileName
      if (gApplication = "IMPRESS") then
         Dateityp.Select 5  ' Powerpoint
         printlog "Trying to save with filter: " + Dateityp.GetSelText + sFilter(5)
      else
         Dateityp.Select 1
         printlog "Trying to save with filter: " + Dateityp.GetSelText + sFilter(1)
      endif
         Speichern.Click
         Kontext "Messagebox"
         if Messagebox.Exists(2) then Messagebox.Yes
         Kontext "AlienWarning"
         if AlienWarning.Exists(2) then AlienWarning.OK
         printlog "Saved as: " + sFileName
      Sleep 3

   printlog "Close the office-session"
   FileClose
      Kontext "Messagebox"
      if Messagebox.Exists(2) then Messagebox.Yes
   Sleep 3
   printlog "Open the saved file"
   hFileOpen (sFileName + "." + ExtensionString)
   printlog "File opened: " + sFileName + "." + ExtensionString
   Sleep 3
   printlog "Select all"
   EditSelectAll

   printlog "Check the attributes for the picture"
   hOpenContextmenu
      sleep(2)
      if hMenuFindSelect(10087, true, 1) = false then
         Warnlog "Context-Menu-entry `Position and Size` was not found. Therefore the test ends."
         Call hCloseDocument
         Goto Endsub
      endif
      printlog "Select Position and Size"
      sleep(2)
      Kontext
      Active.SetPage TabPositionAndSize
      Kontext "TabPositionAndSize"
      if ImageWidth <> Width.GetText then warnlog "Position X differs. Should be: " + ImageWidth + " But is: " + Width.GetText
      if ImageHeight <> Height.GetText then warnlog "Position X differs. Should be: " + ImageHeight + " But is: " + Height.GetText
      TabPositionAndSize.OK

   printlog "Check if transparency has disappeared "
   FormatArea
      Kontext
      Messagebox.SetPage TabTransparenz
         kontext "TabTransparenz"
         if LineareTransparenz.Ischecked <> FALSE then
            warnlog "Transparency should NOT have been checked!"
            if MFLinTransparenz.GetText <> shadowtransparencyvalue then warnlog "Transparency-value should NOT have changed"
            LineareTransparenz.UnCheck
         end if
   TabTransparenz.Cancel

   printlog "Check if the cornerstyle has changed via FormatLine "
   FormatLine
      Kontext
      Messagebox.SetPage TabLinie
         kontext "TabLinie"
         if linetype <> Stil.GetItemText then warnlog "Linetype should be: " + linetype + " But is: " + Stil.GetItemText
         if linecolour <> Farbe.GetItemText then warnlog "Linecolour should be: " + linetype + " But is: " + Farbe.GetItemText
         if linewidth <> Breite.GetText then warnlog "Linewidth should be: " + linewidth + " But is: " + Breite.GetText
         if cornerstyletype <> CornerStyle.GetSelText then warnlog "CornerStyle should be: " + cornerstyletype + " But is: " + CornerStyle.GetSelText 'ItemText
      TabLinie.ok

   printlog "Close application"
   Call hCloseDocument
   Printlog "Finished Shaddow-test for " + gApplication
   
endcase ' tiShaddow
