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
'***********************************************************************************
' #1 tdModifyFlipVertikal
' #1 tdModifyFlipHorizontal
' #1 tdContextConvertIntoCurve
' #1 tdContextConvertIntoPolygon
' #1 tdContextConvertIntoContour
' #1 tdContextConvertInto3D
' #1 tdContextConvertIntoRotationObject
' #1 tdContextConvertIntoBitmap
' #1 tdContextConvertIntoMetaFile
' #1 tdModifyArrange
' #1 tdModifyArrangeObjects
' #1 tdModifyAlignment
' #1 tdContextDistribution
' #1 tdContextDescriptionObject
' #1 tdContextNameObject
' #1 tdModifyConnectBreak
' #1 tdModifyShapes
' #1 tdModifyCombineSplit
'\**********************************************************************************

testcase tdModifyFlipVertikal

    '/// open application ///'
    Call hNewDocument            ' imp: contextmenue same SID!
    sleep 1
    '/// create rectangle ///'
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    try
        '/// imp: Kontextmenu: Flip->Vertically ///'
        '///+ Modify->Flip->Vertically ///'
        ContextFlipVerticalDraw
        Printlog "- Flip-vertical is working"
    catch
   	    Warnlog "- Flip-Vertical does not work"
    endcatch
    sleep 1
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyFlipHorizontal
    '/// open application ///'
    Call hNewDocument            ' imp: contextmenue same SID!
    WaitSlot (1000)
    '/// create rectangle ///'
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    try
        '/// imp: Kontextmenu: Flip->Horizontally ///'
        '///+ Modify->Flip->Horizontally ///'
        ContextFlipHorizontalDraw
        Printlog "- Flip-horizontal is working"
    catch
   	    Warnlog "- Flip-horizontal does not work"
    endcatch
    sleep 1
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoCurve
    '/// open application ///'
    Call hNewDocument
    '/// create rectangle ///'
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    '/// Modify->Convert->To Curve ///'
    '///+ Modify->Convert->To Curve ///'
    ContextConvertIntoCurve
    WaitSlot (2000)
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoPolygon
    dim iWaitIndex as integer
    '/// open application ///'
    Call hNewDocument            ' imp: contextmenue same SID!
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
        '/// Check if the dialogue is there. If not - wait for maximum 10 seconds)
        iWaitIndex = 0
        do while NOT GrafikEinfuegenDlg.Exists AND iWaitIndex < 10
            sleep(1)
            iWaitIndex = iWaitIndex + 1
        loop
        if NOT GrafikEinfuegenDlg.Exists AND iWaitIndex = 10 then
            warnlog "Dialogue Insert Graphics didnt work. Ending testcase."
            Call hCloseDocument
            goto endsub
        end if
        '/// insert graphic: "global\input\graf_inp\enter.bmp" ///'
        Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
        Oeffnen.Click
        sleep 3
        '/// Modify->Convert->To Polygon ///'
        '/// Modify->Convert->To Polygon ///'
        ContextConvertIntoPolygon
        Kontext "InPolygonUmwandeln"
            Call DialogTest (InPolygonUmwandeln)
            '/// check checkbox: 'Fill holes' ///'
            LoecherFuellen.Check
            '/// click more in number field: 'Number of colors' ///'
            Farbanzahl.More
            '/// click more in number field: 'Point reductionn' ///'
            Punktreduktion.More
            '/// click more in number field: 'Tile size' ///'
            Kachelgroesse.More
            '/// click button 'Preview' ///'
            Vorschau.Click
            sleep 10
            '/// cancel dialog 'Convert to Polygon' ///'
            InPolygonUmwandeln.Cancel
            sleep (2)
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoContour
    '/// open application ///'
    Call hNewDocument
    '/// create rectangle ///'
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    '/// Modify->Convert->To Contour ///'
    '/// Modify->Convert->To Contour ///'
    ContextConvertIntoContour
    WaitSlot (1000)
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertInto3D
    '/// open application ///'
    Call hNewDocument
    '/// create rectangle ///'
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    '/// Modify->Convert->To 3D ///'
    '/// Modify->Convert->To 3D ///'
    ContextConvertInto3D
    WaitSlot (1000)
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoRotationObject
    '/// open application ///'
    Call hNewDocument
    WaitSlot (1000)
    '/// create rectangle ///'
    Call hRechteckErstellen (20,20,50,50)
     sleep 2
     '/// Modify->Convert->To 3D Rotation Object ///'
     '/// Modify->Convert->To 3D Rotation Object ///'
     ContextConvertInto3DRotationObject
     WaitSlot (1000)
     '/// close application ///'
     Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoBitmap
    '/// open application ///'
    Call hNewDocument
    WaitSlot (3000)
    InsertGraphicsFromFile
    WaitSlot (3000)
    Kontext "GrafikEinfuegenDlg"
        sleep 2
        '/// insert graphic: "global\input\graf_inp\columbia.dxf" ///'
        Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\columbia.dxf")
        sleep 2
        Oeffnen.Click
        sleep 2
        try
            '/// Modify->Convert->To Bitmap ///'
            ContextConvertIntoBitmap
            Printlog "- Convert into bitmap is working"
        catch
            Warnlog "- Convert into bitmap does not work"
        endcatch
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoMetaFile
    '/// open application ///'
    Call hNewDocument
    WaitSlot (3000)
    InsertGraphicsFromFile
    WaitSlot (1000)
    kontext "Messagebox"
        if Messagebox.Exists (5) Then Messagebox.OK
            sleep 1
            Kontext "GrafikEinfuegenDlg"
                '/// insert graphic: "global\input\graf_inp\desp.bmp" ///'
                sleep 2
                Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
                sleep 2
                Preview.Click
                sleep 3
                Oeffnen.Click
                sleep 5
            try
                '/// Modify->Convert->To Metafile ///'
                '/// Modify->Convert->To Metafile ///'
                ContextConvertIntoMetafile
                Printlog "- convert into meta file does work"
            catch
                Warnlog "- convert into meta file does not work"
            endcatch
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyArrange
    '/// open application ///'
    Call hNewDocument
    '/// create two rectangles ///'
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30, 30, 50, 60 )
    '/// Modify->Arrange->Bring to Front ///'
    '///+ Modify->Arrange->Bring to Front ///'
    FormatArrangeBringToFront
    WaitSlot (1000)
    '/// Modify->Arrange->Bring Forward ///'
    '///+ Modify->Arrange->Bring Forward ///'
    ContextArrangeBringForward
    WaitSlot (1000)
    '/// Modify->Arrange->Send Backward ///'
    '///+ Modify->Arrange->Send Backward ///'
    ContextArrangeBringBackward
    WaitSlot (1000)
    '/// Modify->Arrange->Send to Back ///'
    '///+ Modify->Arrange->Send to Back ///'
    FormatArrangeSendToBack
    WaitSlot (1000)
    '/// Edit->Select All ///'
    EditSelectAll
    '/// Modify->Arrange->Reverse ///'
    '///+ Modify->Arrange->Reverse ///'
    ContextArrangeReverse
    WaitSlot (1000)
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyArrangeObjects
    '/// open application ///'
    Call hNewDocument
    WaitSlot (1000)
    '/// create two rectangles ///'
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30,30,50,50 )
    '/// Modify->Arrange->In Front of Object ///'
    '///+ Modify->Arrange->In Front of Object ///'
    ContextArrangeInFrontOfObject
    '/// click on the upper left rectangle ///'
    gMouseClick 11,11
    '/// Modify->Arrange->Behind Object ///'
    '///+ Modify->Arrange->Behind Object ///'
    ContextArrangeBehindObject
    '/// click on the lower right rectangle ///'
    gMouseClick 45,45
    sleep 1
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyAlignment
    '/// open application ///'
    Call hNewDocument
    WaitSlot (1000)
    '/// create rectangle ///'
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    '/// Modify->Alignment->Left ///'
    '///+ Modify->Alignment->Left ///'
    ContextAlignmentLeft
    WaitSlot (1000)
    '/// Modify->Alignment->Centered ///'
    '///+ Modify->Alignment->Centered ///'
    ContextAlignmentCentered
    WaitSlot (1000)
    '/// Modify->Alignment->Right ///'
    '///+ Modify->Alignment->Right ///'
    ContextAlignmentRight
    WaitSlot (1000)
    '/// Modify->Alignment->Top ///'
    '///+ Modify->Alignment->Top ///'
    ContextAlignmentTop
    WaitSlot (1000)
    '/// Modify->Alignment->Center ///'
    '///+ Modify->Alignment->Center ///'
    ContextAlignmentBottom
    WaitSlot (1000)
    '/// Modify->Alignment->Bottom ///'
    '///+ Modify->Alignment->Bottom ///'
    ContextAlignmentCenter
    WaitSlot (1000)
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextDistribution
    '/// open application ///'
    Call hNewDocument
    WaitSlot (3000)
    '/// create 3 rectangles ///'
    Call sSelectEmptyLayout
    Call hRechteckErstellen (20,20,30,30)
    Call hRechteckErstellen (40,40,50,50)
    Call hRechteckErstellen (60,60,70,70)
    sleep 1
    '/// select all with keyboard: <STRG>+ <A> ///'
    EditSelectAll
    sleep 1
    '/// Modify->Distibution... ///'
    '///+ Modify->Distibution... ///'
    ContextDistribution
    Kontext "VerteilenDlg"
        sleep 1
        Call DialogTest (VerteilenDlg)
        sleep 1
        '/// check radiobutton horizontal 'Left' ///'
        Links.Check
        '///+ check radiobutton 'Center' ///'
        MitteHorizontal.Check
        '///+ check radiobutton 'spacing' ///'
        AbstandHorizontal.Check
        '///+ check radiobutton 'right' ///'
        Rechts.Check
        '///+ check radiobutton 'none' ///'
        KeineHorizontal.Check
        '/// check radiobutton vertical 'top' ///'
        Oben.Check
        '///+ check radiobutton 'center' ///'
        MitteVertikal.Check
        '///+ check radiobutton 'spacing' ///'
        AbstandVertikal.Check
        '///+ check radiobutton 'bottom' ///'
        Unten.Check
        '///+ check radiobutton 'none' ///'
        KeineVertikal.Check
        '/// cancel dialog 'Distribution' ///'
        VerteilenDlg.Cancel
        sleep 2
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextDescriptionObject
    '/// Open application ///'
    Call hNewDocument
    WaitSlot (1000)
    '/// create rectangle ///'
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    ContextDescriptionObject
    Kontext "DescriptionObject"
        Call DialogTest (DescriptionObject)
        '/// Cancel dialog 'DescriptionObject' ///'
        DescriptionObject.Cancel
    '/// Close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextNameObject
    '/// open application ///'
    Call hNewDocument
    WaitSlot (1000)
    '/// create two rectangles ///'
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30, 40, 50, 60 )
    sleep 1
    '/// select both by spanning a selection with the mouse ///'
    gMouseMove 1,1,70,70
    sleep 1
    '/// Modify->Group ///'
    '///+ Modify->Group ///'
    FormatGroupGroup
    WaitSlot (1000)
    '/// Modify->Name Object ///'
    '///+ Modify->Name Object ///'
    ContextNameObject
    Kontext "NameDlgObject"
        Call DialogTest (NameDlgObject)
        '/// cancel dialog 'name' ///'
        NameDlgObject.Cancel
        '/// Modify->UnGroup ///'
        '///+ Modify->UnGroup ///'
        FormatUngroupDraw
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyConnectBreak
    '/// open application ///'
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 1
    '/// create two rectangles ///'
    Call hRechteckErstellen (10,10,30,30)
    Call hRechteckErstellen (35,35,50,50)
    sleep 1
    '/// Edit->Select All ///'
    EditSelectAll
    '/// Modify->Connect ///'
    '///+ Modify->Connect ///'
    ContextConnect
    sleep 1
    try
        '/// Modify->Break ///'
        '///+ Modify->Break ///'
        ContextBreak
    catch
  	    Warnlog "- Modify-Break does not work"
    endcatch
    sleep 1
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyShapes
    '/// open application ///'
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 1
    '/// create two rectangles ///'
    gMouseClick 50,50
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    '/// Edit->Select All ///'
    EditSelectAll
    sleep 1
    try
        '/// Modify->Shapes->Merge ///'
        '///+ Modify->Shapes->Merge ///'
   	    ModifyShapesMerge         ' 1
        WaitSlot (1000)    'sleep 1
   	    Printlog "- Modify-Shape merge is working"
    catch
   	    Warnlog "- Modify-shape merge is not working"
    endcatch
    '/// select all and delete it ///'
    EditSelectAll
    sleep 1
    hTypeKeys "<DELETE>"
    sleep 1
    '/// create two rectangles ///'
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    '/// Edit->Select All ///'
    EditSelectAll
    sleep 1
    try
        '/// Modify->Shapes->Substract ///'
        '///+ Modify->Shapes->Substract ///'
   	    ModifyShapesSubstract     ' 2
   	    Printlog "- Modify-shape-substract is working"
    catch
   	    Warnlog "- Modify-shape substract is not working"
    endcatch
    sleep 1
    '/// select all and delete it ///'
    EditSelectAll
    sleep 1
    hTypeKeys "<DELETE>"
    sleep 1
    '/// create two rectangles ///'
    Call hRechteckErstellen (30,30,50,50)
    sleep 1
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    EditSelectall
    sleep 1
    try
        '/// Modify->Shapes->Intersect ///'
        '///+ Modify->Shapes->Intersect ///'
   	    ModifyShapesIntersect      ' 3
   	    Printlog "- Modify-shape intersect is working"
    catch
   	    Warnlog "- Modify-Shape intersect is not working"
    endcatch
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyCombineSplit
    '/// open application ///'
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 1
    '/// create two rectangles ///'
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    '/// Edit->Select All ///'
    EditSelectAll
    sleep 1
    try
        '/// Modify->Shapes->Combine ///'
        '///+ Modify->Shapes->Combine ///'
   	    ContextCombine
   	    Printlog "- Modify combine is working"
        '/// Modify->Shapes->Split ///'
        '///+ Modify->Shapes->Split ///'
   	    ContextSplit
   	    Printlog "- Modify-split is working"
    catch
   	    Warnlog "- Modify-combine and split are not working"
    endcatch
    '/// close application ///'
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------
