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
'* short description : Tests the clipboard-function with a rectangle
'*
'*******************************************************************
'*

' #1 MeasurementSetFirst
' #1 ResetMeasurement
' #1 SaveMeasurementSetFirst
' #1 Select_Copy
' #1 calctest
' #1 writertest
' #1 tClipboardFromDrawTest

' #1 Small_test_Draw
' #1 Small_test_Impress
' #1 Small_test_Calc
' #1 Small_test_Writer
' #1 Full_test_Writer
' #1 Full_test_Draw
' #1 Full_test_Impress
' #1 Full_test_Calc
' #1 clear_all_and_close
' #1 Make_LineObject_From_Application
' #1 Make_Rectangle_From_Application
' #1 Make_Make3dObject_From_Application
' #1 Make_TextSquare_From_Application
' #1 Make_And_Check_Formatted_Text_Line_From_Application
' #1 Make_ConnectorObject_From_Application
' #1 Make_Circle_From_Application
' #1 Make_CurveObject_From_Application
' #1 Make_Objectplugin_From_Application
'*
'\*******************************************************************

global i as integer
global sApplication as string
global ReferenceOld as string
global ReferenceNew as string
global ToApp as string
global ToAppCounter as integer
global FromApp as string
global FromApp2 as string
global CheckValue1 as integer
global Marker as integer
global WindowsSting (4) as string
global PositionPositionX as string
global PositionPositionY as string
global PositionWidth as string
global PositionHeight as string

global PositionKeepRatio as string
global PositionProtectPosition as string
global PositionProtectSize as string
global PositionFitWidthText as string
global PositionFitHeightText as string
global PositionAnchorPosition as string
global PositionAnchor as string
global PositionPositionPosition as string
global PositionSizePosition as string

global AreaNoFillValue as string
global AreaColorValue as string
global AreaColourListValue as string
global AreaGradientValue as string
global AreaGradientListValue as string
global AreaAutomaticValue as string
global AreaHatchingValue as string
global AreaHatchingListValue as string
global AreaBackgroundColourValue as string
global AreaBackgroundColourListValue as string
global AreaIncrementsValue as string
global AreaBitmapValue as string
global AreaBitmapListValue as string
global AreaOriginalListValue as string
global AreaRelativeValue as string
global AreaWidthValue as string
global AreaHeightValue as string
global AreaXOffsetValue as string
global AreaYOffsetValue as string
global AreaTileValue as string
global AreaStretchValue as string
global AreaRowValue as string
global AreaColumnValue as string
global AreaOffsetValue as string

global ShadowUseShadowsValue as string
global ShadowDistanceValue as string
global ShadowColorValue as string
global ShadowTransparencyValue as string

global TransparencyTransparencyValue as string
global TransparencyGradientType as string
global TransparencyMFZentrumX as string
global TransparencyMFZentrumY as string
global TransparencyMFWinkel as string
global TransparencyMFRand as string
global TransparencyMFStartwert as string
global TransparencyMFEndwert as string

global ColorName as string
global ColorColor as string
global ColorModel as string
global ColorR as string
global ColorG as string
global ColorB as string
global ColorC as string
global ColorM as string
global ColorY as string
global ColorK as string

global GradientsType as string
global GradientsCenterX as string
global GradientsCenterZ as string
global GradientsAngle as string
global GradientsBorder as string
global GradientsFromColor as string
global GradientsFromValue as string
global GradientsToColor as string
global GradientsToValue as string

global HatchingDistance as string
global HatchingAngle as string
global HatchingLineType as string
global HatchingLineColor as string
global HatchingLineTable as string

global BitmapForeground as string
global BitmapBackground as string
global BitmapTable as string

global Text1 as string
global Text2 as string
global Text3 as integer
global Text4 as string
global Text5 as string
global Text6 as string
global Text7 as string

'--------------------------- Tests for Writer ----------------------------------
sub writertest

    try
        call Make_And_Check_Formatted_Text_Line_From_Application
    catch
        warnlog "Something went wrong with testing writertest"
    endcatch

    try
        call Make_Rectangle_From_Application
        call Full_test_Draw
        call Full_test_Impress
        call Full_test_Writer
        call Full_test_Calc
    catch
        warnlog "something wrong with testing writertest"
    endcatch
end sub ' big one

'---------------------------- Tests for Calc -----------------------------------
sub calctest

    try
        call Make_Rectangle_From_Application
        call Full_test_Draw
        call Full_test_Impress
        call Full_test_Writer
        call Full_test_Calc
    catch
        warnlog "something wrong with calctest"
    endcatch
    printlog "currently no specific tests from Calc"
end sub

'-------------------------------------------------------------------------------
sub tClipboardFromDrawTest

    EnableQAErrors = false
    FromApp2 = gApplication
    printlog "gApplication = " + gApplication

    select case( gApplication )
    case "WRITER" : call writertest()
    case "CALC"   : call calctest()
    case else     : warnlog( "Unsupported gApplication provided: " & gApplication )
    end select

end sub

'------------------------------- "object"-dependant tests ----------------------

sub g_clipexport
    try
        call Make_Rectangle_From_Application
        call Full_test_Draw
        call Full_test_Impress
        call Full_test_Writer
        call Full_test_Calc
    catch
        warnlog "something wrong with testing Rectangle"
    endcatch

    try
        call Make_Make3dObject_From_Application
        call Full_test_Draw
        call Small_test_Impress
        call Small_test_Calc
        call Small_test_Writer
    catch
        warnlog "something wrong with testing 3dObject"
    endcatch

    'qaerrorlog "Make_CurveObject_From_Application needs an overview - FHA"
    try
        call Make_CurveObject_From_Application
        call Small_test_Draw
        call Small_test_Impress
        call Small_test_Calc
        call Small_test_Writer
    catch
        warnlog "something wrong with testing CurveObj"
    endcatch

    try
        call Make_ConnectorObject_From_Application
        call Small_test_Draw
        call Small_test_Impress
        call Small_test_Calc
        call Small_test_Writer
    catch
        warnlog "something wrong with testing ConnectorObject"
    endcatch

    try
        call Make_LineObject_From_Application
        call Small_test_Draw
        call Small_test_Impress
        '   call Small_test_Calc
        '   call Small_test_Writer
    catch
        warnlog "something wrong with testing LineObject"
    endcatch

    try
    catch
        warnlog "something wrong with testing ConnectorObject"
    endcatch

    try
        call Make_Circle_From_Application
        call Small_test_Draw
        call Small_test_Impress
        call Small_test_Calc
        call Small_test_Writer
    catch
        warnlog "something wrong with testing Circle"
    endcatch

    try
        call Make_TextSquare_From_Application
        call Small_test_Draw
        call Small_test_Impress
        call Small_test_Calc
        call Small_test_Writer

    catch
        warnlog "something wrong with testing TextSquare"
    endcatch

    call clear_all_and_close
    Printlog "tClipboardFromDrawTest finished"
end sub


'------------------------------- Objects ---------------------------------------
testcase Make_Make3dObject_From_Application

    printlog "--- 3D-Object --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call Make3dObject
    call Select_Copy
    Call Get_Position_Size_Attributes
    Call Get_Colour_Attributes
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Make_Objectplugin_From_Application

    goto endsub
    printlog "--- Objectplugin --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call Objectplugin
    call Select_Copy
    call Get_Position_Size_Attributes
    Call Get_Colour_Attributes
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Make_TextSquare_From_Application

    printlog "--------- TextSquare --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call MakeTextSquare
    call Select_Copy
    call Get_Position_Size_Attributes
    '   Call Get_Colour_Attributes
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Make_Rectangle_From_Application

    printlog "--------- Rectangle --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call MakeRectangle
    call Select_Copy
    call Get_Position_Size_Attributes
    Call Get_Colour_Attributes
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Make_Circle_From_Application

    printlog "--------- Ellipse Object --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call MakeCircle
    call Select_Copy
    call Get_Position_Size_Attributes
    Call Get_Colour_Attributes
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Make_CurveObject_From_Application

    printlog "--------- Curve-Object --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call MakeCurveObject
    call Select_Copy
    call Get_Position_Size_Attributes
    '   Call Get_Colour_Attributes
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Make_LineObject_From_Application

    printlog "--------- Line-Object --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call MakeLineObject
    call Select_Copy
    call Get_Position_Size_Attributes
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Make_ConnectorObject_From_Application

    printlog "--------- Connector-Object --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call MakeConnectorObject
    call Select_Copy
    Call Get_Position_Size_Attributes
    '   Call Get_Colour_Attributes
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Make_And_Check_Formatted_Text_Line_From_Application

    printlog "--------- Formatted Text --- from --- " + gApplication
    sleep 1
    call SaveMeasurementSetFirst
    sleep 1
    Call MakeFormattedTextLine 'inclusive get variable-info.
    call Select_Copy
    Call hCloseDocument
    'call New_Draw
    '   Call Check_Text_Formatting
    '   Call hCloseDocument
    Call New_Impress
    Call Check_Text_Formatting
    Call hCloseDocument
    sleep 10
    Call New_Calc
    Call Check_Text_Formatting
    Call hCloseDocument
    Call New_Writer
    Call Check_Text_Formatting
    Call hCloseDocument
endcase

'--------------------------- Part with the different kinds of tests. -----------
testcase Full_test_Draw

    call New_Draw
    call MeasurementSetFirst
    call Check_Position_Size_Attributes
    call Check_Colors_Borders_Attributes
    gApplication = FromApp2
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Small_test_Draw

    call New_Draw
    call MeasurementSetFirst
    call Check_Position_Size_Attributes
    gApplication = FromApp2
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Full_test_Impress

    call New_Impress
    call MeasurementSetFirst
    call Check_Position_Size_Attributes
    call Check_Colors_Borders_Attributes
    gApplication = FromApp2
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Small_test_Impress

    call New_Impress
    call MeasurementSetFirst
    call Check_Position_Size_Attributes
    gApplication = FromApp2
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Full_test_Calc

    call New_Calc
    call MeasurementSetFirst
    call Check_Position_Size_Attributes
    call Check_Colors_Borders_Attributes
    gApplication = FromApp2
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Small_test_Calc

    call New_Calc
    call MeasurementSetFirst
    call Check_Position_Size_Attributes
    gApplication = FromApp2
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Full_test_Writer

    call New_Writer
    call MeasurementSetFirst
    call Check_Position_Size_Attributes
    call Check_Colors_Borders_Attributes
    gApplication = FromApp2
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase Small_test_Writer

    call New_Writer
    call MeasurementSetFirst
    call Check_Position_Size_Attributes
    gApplication = FromApp2
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase clear_all_and_close

    goto endsub
    printlog "   --------------------------------------------------------------"
    printlog "   All tests done... clearing up everything and finishing"
    if gApplication = "WRITER" then
        DocumentWriter.TypeKeys "<DELETE>"
    else
        printlog "   gApplication is " + gApplication
        EditSelectAll
        EditDelete
    end if
    call ResetMeasurement
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
