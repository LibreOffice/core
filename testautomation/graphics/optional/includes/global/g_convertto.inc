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
'**************************************************************************************

'  Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\columbia.dxf")
'   Call hGrafikEinfuegen ConvertPath  ( gTesttoolPath + "global\input\graf_inp\enter.bmp" )
'   Dateiname.SetText ConvertPath ( gTesttoolPath + "global\input\graf_inp\desp.bmp" )
'  Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\purzel.gif")

'   Call tdContextFlip
'   Call tdContextConvertIntoCurve
'   Call tdContextConvertInto3D
'   Call tdContextConvertInto3D_Rotationobject
'   Call tdContextConvertIntoMetaFile

testcase tiModifyConvertToPolygon
    Dim PosX as string
    Dim PosY as string
    Dim Maxanzahl as string
    Dim Minanzahl as string

    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 3
    hTextrahmenErstellen ("This is a text that will be transformed into a Polygon-object", 10,10,60,30)
    sleep 1
    hTypeKeys "<Escape>"
    sleep 1
    EditSelectAll
    sleep 1
    ContextConvertIntoPolygon
    sleep 1
    ContextPositionAndSize
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    PosX = Width.GetText
    PosY = Height.GetText
    TabPositionAndSize.OK
   sleep 1
   FormatCharacter
       Kontext
      Active.SetPage TabFont
      Kontext "TabFont"
      sleep 1
      try
         Size.SetText "96"
      catch
         if (gAsianSup OR gCTLSup) then
            try
               SizeWest.SetText "96"
            catch
               Warnlog "Asian or ctl support is enabled, but Size of font is not accessable :-("
            endcatch
         else
            Warnlog "Asian or ctl support is NOT enabled, but Size of font is not accessable :-("
            try
               SizeWest.SetText "96"
               qaErrorLog "SizeWest. works"
            catch
            endcatch
            try
               SizeEast.SetText "96"
               qaErrorLog "SizeEast. works"
            catch
            endcatch
            try
               Size.SetText "96"
               qaErrorLog "Size. works"
            catch
            endcatch
            try
               SizeCTL.SetText "96"
               qaErrorLog "SizeCTL. works"
            catch
            endcatch
         endif
      endcatch
      TabFont.OK
   sleep 1
   ContextPositionAndSize
      Kontext
      Active.SetPage TabPositionAndSize
      Kontext "TabPositionAndSize"
      printlog "compare width and heigth to control is it is still text"
      if ( Width.GetText = PosX or Height.GetText = PosY ) Then
           Printlog "- Text converted into polygon"
      else
         Warnlog "- Convert into polygon does not work."
      end if
      TabPositionAndSize.OK
   sleep 1
    Call hCloseDocument
   Printlog "-------------------------------------------------end"

   Call hNewDocument
   sleep 3
   printlog "insert graphic (input\\impress\\grafik\\sample.bmp)"
   Call hGrafikEinfuegen ConvertPath  ( gTesttoolPath + "global\input\graf_inp\enter.bmp" )
   sleep 3
   ContextConvertIntoPolygon
      Kontext "UmwandelnInPolygon"
       Farbanzahl.ToMax
       MaxAnzahl = Farbanzahl.GetText
      Farbanzahl.More 1
      If Maxanzahl < Farbanzahl.GetText Then Warnlog "- Maximum value could be raised."
      printlog "set color to min"
      Farbanzahl.ToMin								
      MinAnzahl = Farbanzahl.GetText
      printlog "try to set a value lower than minimum"
      Farbanzahl.Less 1									
      If Farbanzahl.GetText < Minanzahl then Warnlog "- Minimum value could be lowered."
      Farbanzahl.SetText "2"
      Vorschau.Click
      sleep 5
      UmwandelnInPolygon.OK
   sleep 5
    Printlog "- object vectorized."
    'TBO: verify nasty bug about asian/ctl support
      ToolsOptions
      call hToolsOptions ("Languagesettings", "Languages")
      printlog "Asian: '" + Aktivieren.IsChecked + "'"
      printlog "CTL  : '" + ComplexScriptEnabled.IsChecked + "'"
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '-----------------------------------------------
   Call hCloseDocument
   
endcase 'tiModifyConvertToPolygon
'--------------------------------------------------------
testcase tdContextConvertIntoBitmap

    printlog "new document"
    Call hNewDocument
    sleep 3
    printlog "insert graphic"
    InsertGraphicsFromFile
    sleep 3
    kontext "Active"
    if Active.Exists Then
        Active.OK
    end if
    sleep 5
    Kontext "GrafikEinfuegenDlg"
    sleep 2
printlog " uses input\impress\grafik\columbia.dxf"
  Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\columbia.dxf")
  sleep 2
  Oeffnen.Click
  sleep 2
  Kontext "Active"
   if Active.Exists Then
      Warnlog Active.GetText + ", Test will end here because of problems putting graphics into the document"
      Active.OK
      sleep 2
      Call hCloseDocument
      goto endsub
   end if
  Kontext "DocumentImpress"
   try
        printlog "convert dxf file to bitmap"
        ContextConvertIntoBitmap							
        sleep 1
        Printlog "- Convert into bitmap works"
   catch
        Warnlog "- Convert into bitmap does not work"
   endcatch
    sleep 2
    printlog "close document"
    Call hCloseDocument

endcase 'tdContextConvertIntoBitmap
'--------------------------------------------------------
testcase tiAendernUmwandelnInKurve

    Printlog "  ----- ContextConvertIntoCurve testen"
    printlog "New impress document"
    Call hNewDocument
    Call sSelectEmptyLayout

    printlog "create textbox"
    Call hTextrahmenErstellen ("I test,test,test,test only, if it works for you",20,20,60,30)
    sleep 2
    Kontext "DocumentImpress"
    printlog "deselect textbox "
    gMouseDoubleClick  70,70
    sleep 2
    printlog "select textbox "
    EditSelectAll
    sleep 2
    printlog "convert into curve "
    ContextConvertIntoCurve
    sleep 3
    gMouseClick 30,15

    printlog "try to select 3 letters "
    hTypeKeys "<SHIFT RIGHT>",3					
     try
     printlog "try edit copy to check if its still text "
    EditCopy							
        Warnlog "  - Convert into curve, seperate letters should not be selectable anymore"
     catch
        Printlog "  - Text converted into a curve"
     endcatch

     printlog "close document "
    Call hCloseDocument
 
endcase 'tiAendernUmwandelnInKurve
'--------------------------------------------------------
testcase tiAendernUmwandelnInPolygonGrafik

    Printlog "- Context/ConvertIntoPolygon"

    printlog "new document "
    Call hNewDocument
    Call sSelectEmptyLayout

    printlog "insert graphic (desp.bmp) "
  InsertGraphicsFromFile								

  Kontext "GrafikEinfuegenDlg"
  printlog "use graphic : input\impress\grafik\desp.bmp"
   Dateiname.SetText ConvertPath ( gTesttoolPath + "global\input\graf_inp\desp.bmp" )
   Oeffnen.Click
   sleep 3
  Kontext "DocumentImpress"
  EditSelectAll
  sleep 1

  printlog " convert into polygon "
  ContextConvertIntoPolygon								
  Kontext "UmwandelnInPolygon"
   Printlog "- Testing vectorize dialog"
   printlog "set number of colors to max"
   Farbanzahl.ToMax									
     if Farbanzahl.GetText <> "32" Then Warnlog "- Maximum value for color depth should be 32, but it is: " + Farbanzahl.GetText
     printlog "set number of colors to min"
    Farbanzahl.ToMin
   sleep 1
     if Farbanzahl.GetText <> "8" Then Warnlog "- Minimum value for color depth should be 8 but it is: " + Farbanzahl.GetText
   sleep 2
   printlog "set point reduction to 50 "
   Punktreduktion.SetText "50"								
   sleep 2
     if Punktreduktion.GetText <> "32 Pixel" Then Warnlog "- Maximum value should be 32, but it is: " + Punktreduktion.GetText
   sleep 1
   printlog "set an out of range value"
   Punktreduktion.SetText "-50"								
   sleep 1
     if Punktreduktion.GetText <> "0 Pixel" Then Warnlog "- Minimum value should be 0, but it is: " + Punktreduktion.GetText
     printlog "uncheck fill holes"
    Loecherfuellen.Uncheck
    if Not Kachelgroesse.IsEnabled Then
       Printlog "- Control disabled."
    else
       Warnlog "- Control should be disabled if Fill is not checked."
    end if
    printlog "open preview"
   Vorschau.Click									
   sleep 2
   printlog "close dialog "
   UmwandelnInPolygon.OK								
   sleep 5
   printlog "close document"
  Call hCloseDocument									
  
endcase 'tiAendernUmwandelnInPolygonGrafik
'--------------------------------------------------------
testcase tiAendernUmwandelnIn3D

    Printlog "- Convert into 3D"
    printlog "new impress document "
    Call hNewDocument
    sleep 1
    printlog "create rectangle"
    hRechteckErstellen (10,10,40,40)
                    printlog "convert into 3D "
   sleep 2
   gMouseclick 35,35
   sleep 1
   try
   printlog "try to access convert ito 3d a 2nd time, should be impossible using a 3d object as source "
     ContextConvertInto3D								
     Printlog "- Convert into 3D works"
   catch
     Warnlog "- Convert into 3D does not work"
   endcatch
   printlog "close document"
 Call hCloseDocument									
 
endcase 'tiAendernUmwandelnIn3D
'--------------------------------------------------------
testcase tiAendernUmwandelnIn3DRotationskoerper

    Printlog "- ConvertInto3DRotationobject"
  Dim PosX
  Dim PosY

  printlog "new document"
 Call hNewDocument									
  Kontext "DocumentImpress"
  printlog "create rectangle"
  hRechteckErstellen (20,20,50,50)							
  sleep 1
  printlog "get position values for the rectangle "
  ContextPositionAndSize								
  Kontext
  Active.SetPage TabPositionAndSize
  Kontext "TabPositionAndSize"
  PosX = PositionX.GetText
  PosY = PositionY.GetText
  TabPositionAndSize.OK
  sleep 1
  Kontext "DocumentImpress"
  printlog "convert into 3d lathe object"
  ContextConvertInto3DRotationObject							
  sleep 2
  ContextPositionAndSize
  Kontext
  printlog "control changes in position values to confirm convert action"
  Active.SetPage TabPositionAndSize								
  Kontext "TabPositionAndSize"
   if PositionX.GetText = PosX and PositionY.GetText = PosY Then Warnlog "- No change in position or dimension, converting seemd not to work"
  TabPositionAndSize.OK
  sleep 1
  printlog "close document "
 Call hCloseDocument									
 
endcase 'tiAendernUmwandelnIn3DRotationskoerper
'--------------------------------------------------------
testcase tiAendernUmwandelnInMetaFile

    Printlog "- ConvertIntoMetaFile"

    printlog "new document "
    Call hNewDocument									
    sleep 3
    printlog "insert graphic"
    InsertGraphicsFromFile								
    sleep 1
    kontext "Active"
    sleep 1
    if Active.Exists Then
       Active.OK
    end if
    sleep 1
    Kontext "GrafikEinfuegenDlg"
    sleep 2
    printlog " uses input\impress\grafik\desp.gif"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\purzel.gif")
    sleep 2
    Oeffnen.Click
    sleep 5
    Kontext "DocumentImpress"
    try
        printlog "convert bitmap into metafile"
        ContextConvertIntoMetafile
        sleep 2
        Printlog "- Convert into Metafile does work"
    catch
        Warnlog "- Convert into Metafile does not work"
    endcatch
    sleep 2
    printlog "close document"
    Call hCloseDocument									
endcase 'tiAendernUmwandelnInMetaFile