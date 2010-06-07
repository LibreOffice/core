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
' #1 tiAendernUmwandelnInPolygon
' #1 tiModifyConvertToPolygon
' #1 tdContextConvertIntoBitmap
' #1 tiAendernUmwandelnInKurve
' #1 tiAendernUmwandelnInPolygonGrafik
' #1 tiAendernUmwandelnIn3D
' #1 tiAendernUmwandelnIn3DRotationskoerper
' #1 tiAendernUmwandelnInMetaFile
'\*************************************************************************************

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
      '/// compare width and heigth to control is it is still text ///'
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
   '/// insert graphic (input\\impress\\grafik\\sample.bmp)///'
   Call hGrafikEinfuegen ConvertPath  ( gTesttoolPath + "global\input\graf_inp\enter.bmp" )
   sleep 3
   ContextConvertIntoPolygon
      Kontext "UmwandelnInPolygon"
       Farbanzahl.ToMax
       MaxAnzahl = Farbanzahl.GetText
      Farbanzahl.More 1
      If Maxanzahl < Farbanzahl.GetText Then Warnlog "- Maximum value could be raised."
      Farbanzahl.ToMin									'/// set color to min ///'
      MinAnzahl = Farbanzahl.GetText
      Farbanzahl.Less 1									'/// try to set a value lower than minimum ///'
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

  Call hNewDocument									'/// new document  ///'
  sleep 3
  InsertGraphicsFromFile								'/// insert graphic  ///'
  sleep 3
  kontext "Active"
    if Active.Exists Then
       Active.OK
    end if
  sleep 5
  Kontext "GrafikEinfuegenDlg"
  sleep 2
'///  uses input\\impress\\grafik\\columbia.dxf ///
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
        ContextConvertIntoBitmap							'/// convert dxf file to bitmap  ///'
        sleep 1
        Printlog "- Convert into bitmap works"
   catch
        Warnlog "- Convert into bitmap does not work"
   endcatch
 sleep 2
  Call hCloseDocument									 '/// close document  ///'
  
endcase 'tdContextConvertIntoBitmap
'--------------------------------------------------------
testcase tiAendernUmwandelnInKurve

    Printlog "  ----- ContextConvertIntoCurve testen"

 Call hNewDocument							'/// New impress document///'
    Call sSelectEmptyLayout

  Call hTextrahmenErstellen ("I test,test,test,test only, if it works for you",20,20,60,30)	'/// create textbox///'
   sleep 2
   Kontext "DocumentImpress"
   gMouseDoubleClick  70,70					'/// deselect textbox ///'
   sleep 2
   EditSelectAll                                                '/// select textbox ///'
   sleep 2
   ContextConvertIntoCurve							'/// convert into curve ///'
   sleep 3
   gMouseClick 30,15

    hTypeKeys "<SHIFT RIGHT>",3					'/// try to select 3 letters ///'
     try
    EditCopy							'/// try edit copy to check if its still text ///'
        Warnlog "  - Convert into curve, seperate letters should not be selectable anymore"
     catch
        Printlog "  - Text converted into a curve"
     endcatch

 Call hCloseDocument								'/// close document ///'
 
endcase 'tiAendernUmwandelnInKurve
'--------------------------------------------------------
testcase tiAendernUmwandelnInPolygonGrafik

    Printlog "- Context/ConvertIntoPolygon"

 Call hNewDocument									'/// new document ///'
    Call sSelectEmptyLayout

  InsertGraphicsFromFile								'/// insert graphic (desp.bmp) ///'

  Kontext "GrafikEinfuegenDlg"
   '///use graphic : input\\impress\\grafik\\desp.bmp ///
   Dateiname.SetText ConvertPath ( gTesttoolPath + "global\input\graf_inp\desp.bmp" )
   Oeffnen.Click
   sleep 3
  Kontext "DocumentImpress"
  EditSelectAll
  sleep 1

  ContextConvertIntoPolygon								'///  convert into polygon ///'
  Kontext "UmwandelnInPolygon"
   Printlog "- Testing vectorize dialog"
   Farbanzahl.ToMax									'/// set number of colors to max  ///'
     if Farbanzahl.GetText <> "32" Then Warnlog "- Maximum value for color depth should be 32, but it is: " + Farbanzahl.GetText
   Farbanzahl.ToMin									'/// set number of colors to min  ///'
   sleep 1
     if Farbanzahl.GetText <> "8" Then Warnlog "- Minimum value for color depth should be 8 but it is: " + Farbanzahl.GetText
   sleep 2
   Punktreduktion.SetText "50"								'/// set point reduction to 50 ///'
   sleep 2
     if Punktreduktion.GetText <> "32 Pixel" Then Warnlog "- Maximum value should be 32, but it is: " + Punktreduktion.GetText
   sleep 1
   Punktreduktion.SetText "-50"								'/// set an out of range value  ///'
   sleep 1
     if Punktreduktion.GetText <> "0 Pixel" Then Warnlog "- Minimum value should be 0, but it is: " + Punktreduktion.GetText
   Loecherfuellen.Uncheck								'/// uncheck fill holes  ///'
    if Not Kachelgroesse.IsEnabled Then
       Printlog "- Control disabled."
    else
       Warnlog "- Control should be disabled if Fill is not checked."
    end if
   Vorschau.Click									'/// open preview  ///'
   sleep 2
   UmwandelnInPolygon.OK								'/// close dialog ///'
   sleep 5
  Call hCloseDocument									'/// close document  ///'
  
endcase 'tiAendernUmwandelnInPolygonGrafik
'--------------------------------------------------------
testcase tiAendernUmwandelnIn3D

    Printlog "- Convert into 3D"
 Call hNewDocument
 sleep 1								'/// new impress document ///'
  hRechteckErstellen (10,10,40,40)							'/// create rectangle  ///'
                    '/// convert into 3D ///'
   sleep 2
   gMouseclick 35,35
   sleep 1
   try
     ContextConvertInto3D								'/// try to access convert ito 3d a 2nd time, should be impossible using a 3d object as source ///'
     Printlog "- Convert into 3D works"
   catch
     Warnlog "- Convert into 3D does not work"
   endcatch
 Call hCloseDocument									'/// close document '///
 
endcase 'tiAendernUmwandelnIn3D
'--------------------------------------------------------
testcase tiAendernUmwandelnIn3DRotationskoerper

    Printlog "- ConvertInto3DRotationobject"
  Dim PosX
  Dim PosY

 Call hNewDocument									'/// new document '///
  Kontext "DocumentImpress"
  hRechteckErstellen (20,20,50,50)							'/// create rectangle  ///'
  sleep 1
  ContextPositionAndSize								'/// get position values for the rectangle ///'
  Kontext
  Active.SetPage TabPositionAndSize
  Kontext "TabPositionAndSize"
  PosX = PositionX.GetText
  PosY = PositionY.GetText
  TabPositionAndSize.OK
  sleep 1
  Kontext "DocumentImpress"
  ContextConvertInto3DRotationObject							'/// convert into 3d lathe object  ///'
  sleep 2
  ContextPositionAndSize
  Kontext
  Active.SetPage TabPositionAndSize								'/// control changes in position values to confirm convert action  ///'
  Kontext "TabPositionAndSize"
   if PositionX.GetText = PosX and PositionY.GetText = PosY Then Warnlog "- No change in position or dimension, converting seemd not to work"
  TabPositionAndSize.OK
  sleep 1
 Call hCloseDocument									'/// close document ///'
 
endcase 'tiAendernUmwandelnIn3DRotationskoerper
'--------------------------------------------------------
testcase tiAendernUmwandelnInMetaFile

    Printlog "- ConvertIntoMetaFile"
    
    Call hNewDocument									'/// new document ///'
    sleep 3
    InsertGraphicsFromFile								'/// insert graphic  ///'
    sleep 1
    kontext "Active"
    sleep 1
    if Active.Exists Then
       Active.OK
    end if
    sleep 1
    Kontext "GrafikEinfuegenDlg"
    sleep 2
    '///  uses input\\impress\\grafik\\desp.gif") ///
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\purzel.gif")
    sleep 2
    Oeffnen.Click
    sleep 5
    Kontext "DocumentImpress"
    try
        ContextConvertIntoMetafile							'/// convert bitmap into metafile  ///'
        sleep 2
        Printlog "- Convert into Metafile does work"
    catch
        Warnlog "- Convert into Metafile does not work"
    endcatch
    sleep 2
    Call hCloseDocument									'/// close document  ///'
    
endcase 'tiAendernUmwandelnInMetaFile

