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
'* short description : Test of the insert graphic function - 1
'*
'************************************************************************
'*
' #1 tInsertGraphic_1        'Test default directory of inserting graphic -1
' #1 tInsertGraphic_2        'Test default directory of inserting graphic -2
' #1 tInsertGraphic_3        'Call up dialog Insert / Graphic via Menu ?Insert - Graphics - From File...
' #1 tInsertGraphic_4        'Call up dialog Insert / Graphic via button from Insert-Toolbox of the Toolbar
' #1 tInsertGraphic_5        'Insert a graphic with open button
' #1 tInsertGraphic_6        'Insert a graphic with double click (Return)
' #1 Tinsertgraphic_7        'Default setting in insert/graphic dialog
' #1 tInsertGraphic_8        'Test preselect Filter (Format <All>)
' #1 tInsertGraphic_9        'Test preselect Filter excepte for (Format <All>)
' #1 tInsertGraphic_10       'Insert linked graphics and break the links via Menu Edit - Link
' #1 tInsertGraphic_11       'Insert graphics from clipboard -- Edit/Paste
' #1 tInsertGraphic_12       'Insert graphics from clipboard -- Edit/Paste special
' #1 fInsertGraphics         'This function will insert a graphic with Link checked
'*
'\***********************************************************************

testcase tInsertGraphic_1
    
    Dim iJump     as Integer
    Dim sPathName as String
    Dim i as integer
    
    PrintLog "- Test default dirtory of inserting graphic -1"
    '/// Test default directory of inserting graphic -1
    select case iSprache
        case 01: iJump =  5
        case 31: iJump =  1
        case 33: iJump = 14
        case 34: iJump = 12
        case 36: iJump = 10
        case 39: iJump = 5
        case 46: iJump = 11
        case 49: iJump = 11
        case 55: iJump =  9
        case 81: iJump = 16
        case 82: iJump =  4
        case 86: iJump =  7
        case 88: iJump =  5
        case else : QAErrorLog "The test does not support the language " + iSprache
        Goto endsub
    end select
    
    Call hNewDocument
    
    '/// Tools / Options / StarOffice / Paths
    ToolsOptions
    call hToolsOptions ("StarOffice", "Paths")
        
    Typ.TypeKeys "<HOME><SPACE>"
    Sleep 1
    if iJump-1 > 0 then Typ.TypeKeys "<DOWN>", iJump-1
    Sleep 1
    Bearbeiten.Click
    
    Kontext "OeffnenDlg"
    '/// Set a new path to Graphic
    Pfad.SetText (ConvertPath (gTesttoolpath + "writer\optional\input\insertgraphic\"))
    Sleep 1
    Oeffnen.Click
    Sleep 1
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Sleep 1
    
    '/// Insert / Graphic / From file
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    
    '/// Input the jpg file's name which exists in the graphic dirtory , then press open button
    Dateiname.Settext "Bughunter.jpg"
    Sleep 1
    
    Oeffnen.Click
    Sleep 2
    
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.OK
            QAErrorLog "#112220#The graphic directory in Tools/options doesn't work"
        end if
    end if
    
    Kontext "GrafikEinfuegenDlg"
    if GrafikEinfuegenDlg.Exists then GrafikEinfuegenDlg.Cancel
    
    '/// Reset to default path
    ToolsOptions
    call hToolsOptions ("StarOffice", "Paths")
    Typ.TypeKeys "<HOME>"
    Sleep 1
    if iJump-1 > 0 then Typ.TypeKeys "<DOWN>", iJump-1
    Sleep 1
    Standard.Click

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_2

  PrintLog "- Test default dirtory of inserting graphic -2"
 '/// Test default dirtory of inserting graphic -2

  Call hNewDocument

  '/// Input the 1st picture which exists in the dirtory1(full path) ,
  '/// +  then press open button
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   Dateiname.Settext ConvertPath(gTesttoolpath + "writer\optional\input\insertgraphic\Bughunter.jpg")
   Sleep 1
   Oeffnen.Click
   Sleep 2

   Kontext "Active"
   if Active.Exists then
      if Active.GetRT = 304 then
         Active.OK
         Sleep 1
         Warnlog "1 - the file isn't in the dirtory " + ConvertPath(gTesttoolpath + "writer\optional\input\insertgraphic\")
      end if

      Kontext "GrafikEinfuegenDlg"
      GrafikEinfuegenDlg.Cancel
   end if

   Call wTypeKeys "<ESCAPE>"

  '/// Input the 2nd picture which exists in the dirtory2(only file name) ,
  '/// + then press open button
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   Dateiname.Settext "jolink.jpg"
   Sleep 1
   Oeffnen.Click
   Sleep 2

   Kontext "Active"
   if Active.Exists then
      if Active.GetRT = 304 then
         Active.OK
         Sleep 1
      else
         Warnlog "2 - Should get the warning messagebox"
      end if

      Kontext "GrafikEinfuegenDlg"
      GrafikEinfuegenDlg.Cancel
   end if

   Call wTypeKeys "<ESCAPE>"

  '/// Input the 3rd picture which exists in the dirtory2(full path) ,
  '/// + then press open button
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   Dateiname.Settext ConvertPath(gTesttoolpath + "writer\optional\input\graphics\bild.jpg")
   Sleep 1
   Oeffnen.Click
   Sleep 2

   Kontext "Active"
   if Active.Exists then
      if Active.GetRT = 304 then
         Active.OK
         Sleep 1
         Warnlog "3 - the file isn't in the dirtory " + ConvertPath(gTesttoolpath + "writer\optional\input\grafik")
      end if

      Kontext "GrafikEinfuegenDlg"
      GrafikEinfuegenDlg.Cancel
   end if

   Call wTypeKeys "<ESCAPE>"

  '/// Input the 2nd picture which exists in the dirtory2(only file name) again,
  '/// + then press open button
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   Dateiname.Settext "jolink.jpg"
   Sleep 1
   Oeffnen.Click
   Sleep 2

   Kontext "Active"
   if Active.Exists then
      if Active.GetRT = 304 then
         Active.OK
         Sleep 1
         Warnlog "2 - the file isn't in the dirtory " + ConvertPath(gTesttoolpath + "writer\optional\input\grafik")
      end if

      Kontext "GrafikEinfuegenDlg"
      GrafikEinfuegenDlg.Cancel
   end if

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_3

  PrintLog "- Call up dialog Insert / Graphic via Menu ?Insert - Graphics - From File..."
 '/// Call up dialog Insert / Graphic via Menu ?Insert - Graphics - From File

  Call hNewDocument

  try
     InsertGraphicsFromFile
     Kontext "GrafikEinfuegenDlg"

     if GrafikEinfuegenDlg.Exists then
        GrafikEinfuegenDlg.Cancel
     else
        Warnlog "The dialogue can't be opened from Menu Insert - Graphics - From File..."
     end if
  catch
     Warnlog "Something wrong when calling up dialogue from Menu Insert - Graphics - From File..."
  endcatch

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_X
    Dim sShortcut as string
    PrintLog "- Call up dialog Insert / Picture via Alt+I+U+F ..."
    '/// Call up dialog Insert / Graphic via Alt+I+G+F
 
    Select case iSprache
        case 01: sShortcut = "<MOD2 I U F>"
        case else
            QAErrorlog "Please add shortcut for Insert->Picture"
            goto endsub
    end select
    
    Call hNewDocument
    
    try
        Call wTypeKeys sShortcut
        Kontext "GrafikEinfuegenDlg"
    
        if GrafikEinfuegenDlg.Exists then
            GrafikEinfuegenDlg.Cancel
        else
            Warnlog "The dialogue can't be opened from Menu Insert - Picture - From File..."
        end if
    catch
        Warnlog "Something wrong when calling up dialogue from Menu Insert - Picture - From File..."
    endcatch
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_4

  PrintLog "- Call up dialog Insert / Graphic via button from Insert-Toolbox of the Toolbar "
 '/// Call up dialog Insert / Graphic via button from Insert-Toolbox of the Toolbar

  Call hNewDocument

  Call hToolbarSelect("INSERT", true )
  Sleep 2
  Grafik.Click
  Sleep 3

  Kontext "GrafikEinfuegenDlg"

  if GrafikEinfuegenDlg.Exists then
     GrafikEinfuegenDlg.Cancel
  else
     Warnlog "The dialogue can't be opened from Insert-Toolbox of the Toolbar"
  end if

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_5

  PrintLog "- Insert a graphic with open button"
 '/// Insert a graphic with open button

  Call hNewDocument

  '/// Insert a Graphic , Insert/Graphics/From file
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   Dateiname.Settext (ConvertPath (gTesttoolpath + "writer\optional\input\insertgraphic") )
   Sleep 1
   Oeffnen.Click

   if NOT GrafikEinfuegenDlg.Exists then  Warnlog "The insert graphic dialog should be still there!"

   Dateiname.Settext ("boy.gif")

  '/// check if open button can insert a graphic or not
   Oeffnen.Click
   Sleep 2

   if GrafikEinfuegenDlg.Exists then  Warnlog "The insert graphic dialog should NOT be there!"

   try
      fFormatGraphic("TabZusaetze")
      Kontext "TabZusaetze"
      TabZusaetze.Cancel
   catch
      Warnlog "The graphic is NOT inserted correctly !"
   endcatch

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_6

  PrintLog "- Insert a graphic with double click (Return)"
 '/// Insert a graphic with double click (Return)

  Call hNewDocument

  '/// Insert a Graphic , Insert/Graphics/From file
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   Dateiname.Settext (ConvertPath (gTesttoolpath + "writer\optional\input\insertgraphic") )
   Sleep 1
   Oeffnen.Click

   if NOT GrafikEinfuegenDlg.Exists then  Warnlog "The insert graphic dialog should be still there!"

   Dateiname.Settext ("boy.gif")

  '/// check if double click (return) can insert a graphic or not
  '/// [Note : the first is CVS directory]
   DateiAuswahl.Select 2
   Sleep 1

   DateiAuswahl.TypeKeys "<Return>"
   Sleep 2

   if GrafikEinfuegenDlg.Exists then  Warnlog "The insert graphic dialog should NOT be there!"

   try
      fFormatGraphic("TabZusaetze")
      Kontext "TabZusaetze"
      TabZusaetze.Cancel
   catch
      Warnlog "The graphic is NOT inserted correctly !"
   endcatch

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_7

  Dim iFileType   as Integer
  Dim sStyle      as String

  iFileType = 1

  select case iSprache
      case 01   : sStyle = "Graphics"
      case 31   : sStyle = "Afbeeldingen"
      case 33   : sStyle = "Image"
      case 34   : sStyle = "Imagen"
      case 39   : sStyle = "Immagine"
      case 46   : sStyle = "Grafik"
      case 49   : sStyle = "Grafik"
      case 55   : sStyle = "Figuras"
      case 81   : sStyle = "図"
      case 82   : sStyle = "그래픽"
      case 86   : sStyle = "图形"
      case 88   : sStyle = "圖形"
      case else : QAErrorLog "Now, this testcase does not support for the language " +iSprache
                  exit sub
  end select

  PrintLog "- Default setting in insert/graphic dialog"
 '/// Default setting in insert/graphic dialog

  Call hNewDocument

  '/// Insert a Graphic , Insert/Graphics/From file
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
    if Stil.GetItemText(Stil.GetSelIndex) <> sStyle    then Warnlog "The default setting for Style is wrong ! should be "+ sStyle + " but get " + Stil.GetItemText(Stil.GetSelIndex)
    if Dateityp.GetSelIndex               <> iFileType then Warnlog "The default setting for file type is wrong ! should be "+ Dateityp.GetItemText(iFileType) + " but get " + Dateityp.GetItemText(Dateityp.GetSelIndex)
   GrafikEinfuegenDlg.Cancel

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_8

	Dim iFileType      as Integer
	Dim iFileNumber as Integer
	
	iFileType = 1  'filter (Format <All>)	
	iFileNumber = 4
	
	PrintLog "- Test preselect Filter (Format <All>)"
	'/// Test preselect Filter (Format <All>)
	
	Call hNewDocument
	
	'/// Insert a Graphic , Insert/Graphics/From file
	InsertGraphicsFromFile
	
	'/// Select filter (Format <All>)
	Kontext "GrafikEinfuegenDlg"
	Dateiname.Settext (ConvertPath (gTesttoolpath + "writer\optional\input\insertgraphic") )
	Sleep 1
	Oeffnen.Click
	
	Dateityp.Select iFileType
	Sleep 1
	
	'/// Check if Filter (Format <All>) works well
	if DateiAuswahl.GetItemCount <> iFileNumber then Warnlog "Filter (Format <All>) does NOT work well"
	GrafikEinfuegenDlg.Cancel
	
	Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_9

	Dim iFileType      as Integer
	Dim iFileNumber as Integer
	
	iFileType   = 2  'another filter except for (Format <All>) -- bmp	
	iFileNumber = 1
	
	PrintLog "- Test preselect Filter excepte for (Format <All>)"
	'/// Test preselect Filter excepte for (Format <All>)
	
	Call hNewDocument
	
	'/// Insert a Graphic , Insert/Graphics/From file
	InsertGraphicsFromFile
	
	'/// Select filter 'BMP'
	Kontext "GrafikEinfuegenDlg"
	Dateiname.Settext (ConvertPath (gTesttoolpath + "writer\optional\input\insertgraphic") )
	Sleep 1
	Oeffnen.Click
	
	Dateityp.Select iFileType
	Sleep 1
	
	'/// Check if Filter BMP works well
	if DateiAuswahl.GetItemCount <> iFileNumber then Warnlog "Filter BMP does NOT work well"
	GrafikEinfuegenDlg.Cancel
	
	Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_10

    Dim sTestFile as String
    dim sOutFile as string

    sTestFile = ConvertPath(gOfficepath + "user\work\tInsertGraphic_5.sxw")
    sOutFile  = gOfficepath + "user\work\tInsertGraphic_5.odt"

    PrintLog "- Insert linked graphics and break the links via Menu Edit - Link"
    '/// Insert linked graphics and break the links via Menu Edit - Link

    Call hNewDocument

    Call wTypeKeys "<Return>" , 2

    '/// Insert a jpg  Graphic
    Call fInsertGraphics("Bughunter.jpg")

    '/// + and set name as TEST1
    Call fFormatGraphic("TabZusaetze")
     sleep(1)
     Kontext "TabZusaetze"
     ObjektName.SetText "TEST1"
    TabZusaetze.OK
    sleep(1)

    Kontext "DocumentWriter"
    Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<Return>" , 2
    sleep(2)

    '/// Insert a gif  Graphic  (animated)
    Call fInsertGraphics("boy.gif")

    '/// + and set name as TEST2
    Call fFormatGraphic("TabZusaetze")
     sleep(1)
     Kontext "TabZusaetze"
     ObjektName.SetText "TEST2"
    TabZusaetze.OK
    sleep(1)

    Kontext "DocumentWriter"
    Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<Return>" , 2
    sleep(2)

    '/// Insert a bmp Graphic
    Call fInsertGraphics("game.bmp")

    '/// + and set name as TEST3
    Call fFormatGraphic("TabZusaetze")
     sleep(2)
     Kontext "TabZusaetze"
     ObjektName.SetText "TEST3"
    TabZusaetze.OK
    sleep(1)

    Kontext "DocumentWriter"
    Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<Return>" , 2
    sleep(2)

    '/// Edit / Links
    EditLinksWriter
    Kontext "VerknuepfungenBearbeiten"

    '/// Select all graphics
    Liste.TypeKeys "<Shift End>"
    sleep(1)

    '/// Remove link
     Loesen.Click
     sleep(1)
     Kontext "Active"
     if Active.Exists(1) then
        Active.YES
     else
        warnlog "Messagebox should appear!"
     end if
     sleep(2)

    '/// Close the edit/link dialogue
    Kontext "VerknuepfungenBearbeiten"
    VerknuepfungenBearbeiten.Close
    sleep(1)

    '/// Check if the inserted graphics are still there
    Kontext "DocumentWriter"
    Call wTypeKeys ( "<Shift F4>" )
    sleep(1)
    fFormatGraphic("TabZusaetze")
    sleep(1)
    Kontext "TabZusaetze"
    if ObjektName.GetText <> "TEST1" then  warnlog "The first graphic isn't right!"
    TabZusaetze.OK
    sleep(1)

    Kontext "DocumentWriter"
    Call wTypeKeys "<Tab>"
    Call fFormatGraphic("TabZusaetze")
    sleep(1)
    Kontext "TabZusaetze"
    if ObjektName.GetText <> "TEST2" then  Warnlog "The second graphic isn't right!"
    TabZusaetze.OK
    sleep(1)

    Kontext "DocumentWriter"
    Call wTypeKeys "<Tab>"
    Call fFormatGraphic("TabZusaetze")
    sleep(1)
    Kontext "TabZusaetze"
    if ObjektName.GetText <> "TEST3" then  Warnlog "The third graphic isn't right!"
    TabZusaetze.OK
    sleep(1)

    Kontext "DocumentWriter"
    Call wTypeKeys "<ESCAPE><Home>"
    PrintLog "- Save and Reload the file"

    '/// Save the file and close the file
    Call hFileSaveAsWithFilterKill( sOutFile, "writer8")
    sleep(2)
    Call hCloseDocument
    sleep(2)

    '/// Reload the file
    hFileOpen( sOutFile )
    sleep(2)
    

    '/// Check if the inserted graphics are still there
    Kontext "DocumentWriter"
    Call wTypeKeys ( "<Shift F4>" )
    sleep(1)
    if fFormatGraphic("TabZusaetze") = false then
        Warnlog "#i46300#linked JPEG pictures will disappear when doc is modified."

        kontext "TabZusatze"
        if ( TabZusaetze.exists() ) then TabZusaetze.close()
        kontext "DocumentWriter"
        Call hCloseDocument
        goto endsub
    end if
    sleep(1)
    Kontext "TabZusaetze"
    if ObjektName.GetText <> "TEST1" then  Warnlog "The first graphic isn't right!"
    TabZusaetze.OK
    sleep(1)

    Kontext "DocumentWriter"
    Call wTypeKeys "<Tab>"
    sleep(1)
    Call fFormatGraphic("TabZusaetze")
    sleep(1)
    Kontext "TabZusaetze"
    if ObjektName.GetText <> "TEST2" then  Warnlog "The second graphic isn't right!"
    TabZusaetze.OK
    sleep(2)

    Kontext "DocumentWriter"
    Call wTypeKeys "<Tab>"
    sleep(1)
    Call fFormatGraphic("TabZusaetze")
    sleep(1)
    Kontext "TabZusaetze"
    if ObjektName.GetText <> "TEST3" then  Warnlog "The third graphic isn't right!"
    TabZusaetze.OK
    sleep(1)

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_11

  Dim sGraphicName  as String
  Dim sWidth as String
  Dim sHeight as String
  Dim sLeft as String
  Dim sUp as String
  Dim iFromLeft as Integer
  Dim iFromUp as Integer
  Dim sURL as String

  sGraphicName = "TEST1"
  sWidth       = "5"+ gSeperator + "00" + gMeasurementUnit
  sHeight      = "4"+ gSeperator + "00" + gMeasurementUnit
  sLeft        = "2"+ gSeperator + "00" + gMeasurementUnit
  sUp          = "3"+ gSeperator + "00" + gMeasurementUnit

  iFromLeft = 4
  iFromUp   = 4

  sURL      = "http://www.sun.com/"

  PrintLog "- Insert graphics from clipboard -- Edit/Paste"
 '/// Insert graphics from clipboard -- Edit/Paste

  Call hNewDocument

  '/// Insert a jpg  Graphic and do some configuration
   fInsertGraphics("Bughunter.jpg")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   Kontext "TabZusaetze"
   ObjektName.SetText sGraphicName
   TabZusaetze.OK
   sleep(2)
   fFormatGraphic("TabType")
   Kontext "TabType"
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   Horizontal.Select    iFromLeft
   wait 500
   Vertical.Select      iFromUp
   wait 500
   HorizontalBy.SetText sLeft
   wait 500
   VerticalBy.SetText   sUp
   wait 500
   TabType.OK
   sleep(1)
   Call fFormatGraphic("TabHyperlinkRahmen")
   Kontext "TabHyperlinkRahmen"
   Sleep 1
   URL.SetText  sURL
   wait 500
   TabHyperlinkRahmen.OK

   '/// copy graphic to clipboard
   EditCopy
   Sleep 1

  Call hCloseDocument

  Call hNewDocument

   '/// paste the graphic from clipboard
   EditPaste
   Sleep 1

  '/// Check if the pasted graphic is correct
   Call fFormatGraphic("TabZusaetze")
   Sleep 1
   Kontext "TabZusaetze"
   if ObjektName.GetText <> sGraphicName   then Warnlog "The graphic's name is not correct ! "
   TabZusaetze.Cancel
   sleep(1)
   Call fFormatGraphic("TabType")
   Sleep 1
   Kontext "TabType"
   if Width.GetText          <> sWidth     then Warnlog "The graphic's width is not correct ! "
   wait 500
   if Height.GetText         <> sHeight    then Warnlog "The graphic's height is not correct ! "
   wait 500
   if Horizontal.GetSelIndex <> iFromLeft  then Warnlog "The graphic's Horizontal option is not correct ! "
   wait 500
   if Vertical.GetSelIndex   <> iFromUp    then Warnlog "The graphic's vertical option is not correct ! "
   wait 500
   if HorizontalBy.GetText   <> sLeft      then Warnlog "The graphic's Horizontal is not correct ! "
   wait 500
   if VerticalBy.GetText     <> sUp        then Warnlog "The graphic's vertical is not correct ! "
   wait 500
   TabType.Cancel

   Call fFormatGraphic("TabHyperlinkRahmen")
   Sleep 1
   Kontext "TabHyperlinkRahmen"
   if URL.GetText <>  sURL  then Warnlog "The graphic's URL is not correct ! "
   wait 500
   TabHyperlinkRahmen.Cancel
   sleep(1)
  Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_12

    Dim sGraphicName as String
    Dim sOriginalGraphicName as String
    Dim sWidth as String
    Dim sHeight as String
    Dim sLeft as String
    Dim sUp as String
    Dim sOriginalWidth as String
    Dim sOriginalHeight as String
    Dim sOriginalLeft as String
    Dim sOriginalUp as String
    Dim iFromLeft as Integer
    Dim iFromUp as Integer
    Dim iOriginalFromLeft as Integer
    Dim iOriginalFromUp as Integer
    Dim sURL as String
    Dim i as integer
    
    sGraphicName = "TEST1"
    sWidth       = "5"+ gSeperator + "00" + gMeasurementUnit
    sHeight      = "4"+ gSeperator + "00" + gMeasurementUnit
    sLeft        = "2"+ gSeperator + "00" + gMeasurementUnit
    sUp          = "3"+ gSeperator + "00" + gMeasurementUnit
    
    iFromLeft = 4
    iFromUp   = 4
    
    sURL      = "http://www.sun.com/"
    
    PrintLog "- Insert graphics from clipboard -- Edit/Paste special"
    '/// Insert graphics from clipboard -- Edit/Paste special
    
    Call hNewDocument
    
    '/// Insert a jpg  Graphic and do some configuration
    fInsertGraphics("Bughunter.jpg")
    
    fFormatGraphic("TabZusaetze")
    Sleep 1
    sOriginalGraphicName = ObjektName.GetText
    wait 500
    ObjektName.SetText sGraphicName
    wait 500
    TabZusaetze.OK
    
    fFormatGraphic("TabType")
    KeepRatio.UnCheck
    
    sOriginalWidth    =  Width.GetText
    sOriginalHeight   =  Height.GetText
    iOriginalFromLeft =  Horizontal.GetSelIndex
    iOriginalFromUp   =  Vertical.GetSelIndex
    sOriginalLeft     =  HorizontalBy.GetText
    sOriginalUp       =  VerticalBy.GetText
    
    Sleep 1
    Width.SetText        sWidth
    wait 500
    Height.SetText       sHeight
    wait 500
    Horizontal.Select    iFromLeft
    wait 500
    Vertical.Select      iFromUp
    wait 500
    HorizontalBy.SetText sLeft
    wait 500
    VerticalBy.SetText   sUp
    wait 500
    TabType.OK
    
    fFormatGraphic("TabHyperlinkRahmen")
    Sleep 1
    URL.SetText  sURL
    wait 500
    TabHyperlinkRahmen.OK
    
    '/// copy graphic to clipboard
    EditCopy
    Sleep 1
    
    '/// delete the graphic
    Call wTypeKeys "<Delete>"
    Sleep 1
    
    '/// 1. Edit/Paste special the graphic from
    '/// +  Graphic [StarOffice Writer]
    EditPasteSpecialWriter
    Sleep 1
    Kontext "InhaltEinfuegen"
    if Auswahl.GetItemCount < 4 then
        QAErrorlog "#i47320#Only " & Auswahl.GetItemCount & " graphic formats available"
        InhaltEinfuegen.Cancel
    else
        Auswahl.Select 1
        PrintLog "- " & Auswahl.GetSelText
        InhaltEinfuegen.OK
        Sleep 1

        '/// Check if the pasted graphic properties are correct
        FormatGraphics
        Kontext
        Active.Setpage TabZusaetze
        Kontext "TabZusaetze"
        Sleep 1
        if ObjektName.GetText     <> sGraphicName  then Warnlog "The graphic's name is not correct ! "

        Kontext
        Active.Setpage TabType
        Kontext "TabType"
        Sleep 1
        if Width.GetText          <> sWidth        then Warnlog "The graphic's width is not correct ! "
        wait 500
        if Height.GetText         <> sHeight       then Warnlog "The graphic's height is not correct ! "
        wait 500
        if Horizontal.GetSelIndex <> iFromLeft     then Warnlog "The graphic's Horizontal option is not correct ! "
        wait 500
        if Vertical.GetSelIndex   <> iFromUp       then Warnlog "The graphic's vertical option is not correct ! "
        wait 500
        if HorizontalBy.GetText   <> sLeft         then Warnlog "The graphic's Horizontal is not correct ! "
        wait 500
        if VerticalBy.GetText     <> sUp           then Warnlog "The graphic's vertical is not correct ! "
        wait 500

        Kontext
        Active.Setpage TabHyperlinkRahmen
        Kontext "TabHyperlinkRahmen"
        Sleep 1
        if URL.GetText <> sURL then Warnlog "The graphic's URL is not correct ! "
        wait 500
        TabHyperlinkRahmen.Ok

        'delete the graphic
        Call wTypeKeys "<Delete>"
        Sleep 1
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

function fInsertGraphics (sFileName as String)
   'This function will insert a graphic with Link checked

   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   Dateiname.Settext (ConvertPath (gTesttoolpath + "writer\optional\input\insertgraphic\" + sFileName) )
   Sleep 1
   Link.Check
   Sleep 1
   Oeffnen.Click
   Sleep 2

end function
