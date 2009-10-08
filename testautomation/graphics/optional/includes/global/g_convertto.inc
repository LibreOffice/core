'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_convertto.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:39 $
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
    Call hGrafikEinfuegen ConvertPath  ( gTesttoolPath + "global\input\graf_inp\enter.bmp" )
    sleep 3
    ContextConvertIntoPolygon
    Kontext "UmwandelnInPolygon"
    Farbanzahl.ToMax
    MaxAnzahl = Farbanzahl.GetText
    Farbanzahl.More 1
    If Maxanzahl < Farbanzahl.GetText Then Warnlog "- Maximum value could be raised."
        MinAnzahl = Farbanzahl.GetText
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
endcase

testcase tdContextConvertIntoBitmap
    sleep 3
    sleep 3
    kontext "Active"
    if Active.Exists Then
        Active.OK
    end if
    sleep 5
    Kontext "GrafikEinfuegenDlg"
    sleep 2
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
        sleep 1
        Printlog "- Convert into bitmap works"
    catch
        Warnlog "- Convert into bitmap does not work"
    endcatch
    sleep 2
endcase

testcase tiAendernUmwandelnInKurve
    Printlog "  ----- ContextConvertIntoCurve testen"


    sleep 2
    Kontext "DocumentImpress"
    sleep 2
    sleep 2
    sleep 3
    gMouseClick 30,15

    try
        Warnlog "  - Convert into curve, seperate letters should not be selectable anymore"
    catch
        Printlog "  - Text converted into a curve"
    endcatch

endcase

testcase tiAendernUmwandelnInPolygonGrafik
    Printlog "- Context/ConvertIntoPolygon"



    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText ConvertPath ( gTesttoolPath + "global\input\graf_inp\desp.bmp" )
    Oeffnen.Click
    sleep 3
    Kontext "DocumentImpress"
    EditSelectAll
    sleep 1

    Kontext "UmwandelnInPolygon"
    Printlog "- Testing vectorize dialog"
    if Farbanzahl.GetText <> "32" Then Warnlog "- Maximum value for color depth should be 32, but it is: " + Farbanzahl.GetText
        sleep 1
        if Farbanzahl.GetText <> "8" Then Warnlog "- Minimum value for color depth should be 8 but it is: " + Farbanzahl.GetText
            sleep 2
            sleep 2
            if Punktreduktion.GetText <> "32 Pixel" Then Warnlog "- Maximum value should be 32, but it is: " + Punktreduktion.GetText
                sleep 1
                sleep 1
                if Punktreduktion.GetText <> "0 Pixel" Then Warnlog "- Minimum value should be 0, but it is: " + Punktreduktion.GetText
                    if Not Kachelgroesse.IsEnabled Then
                        Printlog "- Control disabled."
                    else
                        Warnlog "- Control should be disabled if Fill is not checked."
                    end if
                    sleep 2
                    sleep 5
endcase

testcase tiAendernUmwandelnIn3D
    Printlog "- Convert into 3D"
    Call hNewDocument
    sleep 2
    gMouseclick 35,35
    sleep 1
    try
        Printlog "- Convert into 3D works"
    catch
        Warnlog "- Convert into 3D does not work"
    endcatch
endcase

testcase tiAendernUmwandelnIn3DRotationskoerper
    Printlog "- ConvertInto3DRotationobject"
    Dim PosX
    Dim PosY

    Kontext "DocumentImpress"
    sleep 1
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    PosX = PositionX.GetText
    PosY = PositionY.GetText
    TabPositionAndSize.OK
    sleep 1
    Kontext "DocumentImpress"
    sleep 2
    ContextPositionAndSize
    Kontext
    Kontext "TabPositionAndSize"
    if PositionX.GetText = PosX and PositionY.GetText = PosY Then Warnlog "- No change in position or dimension, converting seemd not to work"
        TabPositionAndSize.OK
        sleep 1
endcase

testcase tiAendernUmwandelnInMetaFile
    Printlog "- ConvertIntoMetaFile"

    sleep 3
    sleep 1
    kontext "Active"
    sleep 1
    if Active.Exists Then
        Active.OK
    end if
    sleep 1
    Kontext "GrafikEinfuegenDlg"
    sleep 2
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\purzel.gif")
    sleep 2
    Oeffnen.Click
    sleep 5
    Kontext "DocumentImpress"
    try
        sleep 2
        Printlog "- Convert into Metafile does work"
    catch
        Warnlog "- Convert into Metafile does not work"
    endcatch
    sleep 2
endcase

