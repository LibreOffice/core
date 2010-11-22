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

sub id_007

    printLog Chr(13) + "---------    id_007    ----------"

    call tdModifyFlipVertikal
    call tdModifyFlipHorizontal
    call tdContextConvertIntoCurve
    call tdContextConvertIntoPolygon
    call tdContextConvertIntoContour
    call tdContextConvertInto3D
    call tdContextConvertIntoRotationObject
    call tdContextConvertIntoBitmap
    call tdContextConvertIntoMetaFile
    call tdModifyArrange
    call tdModifyArrangeObjects
    call tdModifyAlignment
    call tdContextDistribution
    call tdContextDescriptionObject
    call tdContextNameObject
    call tdModifyConnectBreak
    call tdModifyShapes
    call tdModifyCombineSplit
end sub
'-------------------------------------------------------------------------------

testcase tdModifyFlipVertikal
    printlog "open application"
    Call hNewDocument            ' imp: contextmenue same SID!
    sleep 1
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    try
        printlog "imp: Kontextmenu: Flip->Vertically"
        ContextFlipVerticalDraw
        Printlog "- Flip-vertical is working"
    catch
   	    Warnlog "- Flip-Vertical does not work"
    endcatch
    sleep 1
    printlog "close application"
    Call hCloseDocument
endcase 'tdModifyFlipVertikal

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyFlipHorizontal
    printlog "open application"
    Call hNewDocument            ' imp: contextmenue same SID!
    WaitSlot (1000)
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    try
        printlog "imp: Kontextmenu: Flip->Horizontally"
        ContextFlipHorizontalDraw
        Printlog "- Flip-horizontal is working"
    catch
   	    Warnlog "- Flip-horizontal does not work"
    endcatch
    sleep 1
    printlog "close application"
    Call hCloseDocument
endcase 'tdModifyFlipHorizontal

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoCurve
    printlog "open application"
    Call hNewDocument
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    printlog "Modify->Convert->To Curve"
    ContextConvertIntoCurve
    WaitSlot (2000)
    printlog "close application"
    Call hCloseDocument
endcase 'tdContextConvertIntoCurve

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoPolygon
    dim iWaitIndex as integer

    printlog "open application"
    Call hNewDocument        
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    printlog "Check if the dialogue is there. If not - wait for maximum 10 seconds)"
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
    printlog "insert graphic: global\input\graf_inp\enter.bmp"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
    Oeffnen.Click
    sleep 3
    printlog "Modify->Convert->To Polygon"
    ContextConvertIntoPolygon
    Kontext "InPolygonUmwandeln"
    Call DialogTest (InPolygonUmwandeln)
    printlog "check checkbox: 'Fill holes'"
    LoecherFuellen.Check
    printlog "click more in number field: 'Number of colors'"
    Farbanzahl.More
    printlog "click more in number field: 'Point reductionn'"
    Punktreduktion.More
    printlog "click more in number field: 'Tile size'"
    Kachelgroesse.More
    printlog "click button 'Preview'"
    Vorschau.Click
    sleep 10
    printlog "cancel dialog 'Convert to Polygon'"
    InPolygonUmwandeln.Cancel
    sleep (2)
    printlog "close application"
    Call hCloseDocument
endcase 'tdContextConvertIntoPolygon

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoContour
    printlog "open application"
    Call hNewDocument
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    printlog "Modify->Convert->To Contour"
    ContextConvertIntoContour
    WaitSlot (1000)
    printlog "close application"
    Call hCloseDocument
endcase 'tdContextConvertIntoContour

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertInto3D
    printlog "open application"
    Call hNewDocument
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    printlog "Modify->Convert->To 3D"
    ContextConvertInto3D
    WaitSlot (1000)
    printlog "close application"
    Call hCloseDocument
endcase 'tdContextConvertInto3D

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoRotationObject
    printlog "open application"
    Call hNewDocument
    WaitSlot (1000)
    printlog "create rectangle"
    Call hRechteckErstellen (20,20,50,50)
     sleep 2
     printlog "Modify->Convert->To 3D Rotation Object"
     ContextConvertInto3DRotationObject
     WaitSlot (1000)
     printlog "close application"
     Call hCloseDocument
endcase 'tdContextConvertIntoRotationObject

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoBitmap
    printlog "open application"
    Call hNewDocument
    WaitSlot (3000)
    InsertGraphicsFromFile
    WaitSlot (3000)
    Kontext "GrafikEinfuegenDlg"
    sleep 2
    printlog "insert graphic: global\input\graf_inp\columbia.dxf"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\columbia.dxf")
    sleep 2
    Oeffnen.Click
    sleep 2
    try
        printlog "Modify->Convert->To Bitmap"
        ContextConvertIntoBitmap
        Printlog "- Convert into bitmap is working"
    catch
        Warnlog "- Convert into bitmap does not work"
    endcatch
printlog "close application"
    Call hCloseDocument
endcase 'tdContextConvertIntoBitmap

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoMetaFile
    printlog "open application"
    Call hNewDocument
    WaitSlot (3000)
    InsertGraphicsFromFile
    WaitSlot (1000)
    kontext "Messagebox"
    if Messagebox.Exists (5) Then Messagebox.OK
        sleep 1
        Kontext "GrafikEinfuegenDlg"
        printlog "insert graphic: global\input\graf_inp\desp.bmp"
        sleep 2
        Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
        sleep 2
        Preview.Click
        sleep 3
        Oeffnen.Click
        sleep 5
        try
            printlog "Modify->Convert->To Metafile"
            printlog "Modify->Convert->To Metafile"
            ContextConvertIntoMetafile
            Printlog "- convert into meta file does work"
        catch
            Warnlog "- convert into meta file does not work"
        endcatch
    printlog "close application"
    Call hCloseDocument
endcase 'tdContextConvertIntoMetaFile

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyArrange
    printlog "open application"
    Call hNewDocument
    printlog "create two rectangles"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30, 30, 50, 60 )
    printlog "Modify->Arrange->Bring to Front"
    FormatArrangeBringToFront
    WaitSlot (1000)
    printlog "Modify->Arrange->Bring Forward"
    ContextArrangeBringForward
    WaitSlot (1000)
    printlog "Modify->Arrange->Send Backward"
    ContextArrangeBringBackward
    WaitSlot (1000)
    printlog "Modify->Arrange->Send to Back"
    FormatArrangeSendToBack
    WaitSlot (1000)
    printlog "Edit->Select All"
    EditSelectAll
    printlog "Modify->Arrange->Reverse"
    ContextArrangeReverse
    WaitSlot (1000)
    printlog "close application"
    Call hCloseDocument
endcase 'tdModifyArrange

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyArrangeObjects
    printlog "open application"
    Call hNewDocument
    WaitSlot (1000)
    printlog "create two rectangles"
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30,30,50,50 )
    printlog "Modify->Arrange->In Front of Object"
    ContextArrangeInFrontOfObject
    printlog "click on the upper left rectangle"
    gMouseClick 11,11
    printlog "Modify->Arrange->Behind Object"
    ContextArrangeBehindObject
    printlog "click on the lower right rectangle"
    gMouseClick 45,45
    sleep 1
    printlog "close application"
    Call hCloseDocument
endcase 'tdModifyArrangeObjects

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyAlignment
    printlog "open application"
    Call hNewDocument
    WaitSlot (1000)
    printlog "create rectangle"
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    printlog "Modify->Alignment->Left"
    ContextAlignmentLeft
    WaitSlot (1000)
    printlog "Modify->Alignment->Centered"
    ContextAlignmentCentered
    WaitSlot (1000)
    printlog "Modify->Alignment->Right"
    ContextAlignmentRight
    WaitSlot (1000)
    printlog "Modify->Alignment->Top"
    ContextAlignmentTop
    WaitSlot (1000)
    printlog "Modify->Alignment->Center"
    ContextAlignmentBottom
    WaitSlot (1000)
    printlog "Modify->Alignment->Bottom"
    ContextAlignmentCenter
    WaitSlot (1000)
    printlog "close application"
    Call hCloseDocument
endcase 'tdModifyAlignment

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextDistribution
    printlog "open application"
    Call hNewDocument
    WaitSlot (3000)
    printlog "create 3 rectangles"
    Call sSelectEmptyLayout
    Call hRechteckErstellen (20,20,30,30)
    Call hRechteckErstellen (40,40,50,50)
    Call hRechteckErstellen (60,60,70,70)
    sleep 1
    printlog "select all with keyboard: <STRG>+ <A>"
    EditSelectAll
    sleep 1
    printlog "Modify->Distibution..."
    ContextDistribution
    Kontext "VerteilenDlg"
    sleep 1
    Call DialogTest (VerteilenDlg)
    sleep 1
    printlog "check radiobutton horizontal 'Left'"
    Links.Check
    printlog "check radiobutton 'Center'"
    MitteHorizontal.Check
    printlog "check radiobutton 'spacing'"
    AbstandHorizontal.Check
    printlog "check radiobutton 'right'"
    Rechts.Check
    printlog "check radiobutton 'none'"
    KeineHorizontal.Check
    printlog "check radiobutton vertical 'top'"
    Oben.Check
    printlog "check radiobutton 'center'"
    MitteVertikal.Check
    printlog "check radiobutton 'spacing'"
    AbstandVertikal.Check
    printlog "check radiobutton 'bottom'"
    Unten.Check
    printlog "check radiobutton 'none'"
    KeineVertikal.Check
    printlog "cancel dialog 'Distribution'"
    VerteilenDlg.Cancel
    sleep 2
    printlog "close application"
    Call hCloseDocument
endcase 'tdContextDistribution

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextDescriptionObject
    printlog "Open application"
    Call hNewDocument
    WaitSlot (1000)
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    ContextDescriptionObject
    Kontext "DescriptionObject"
    Call DialogTest (DescriptionObject)
    printlog "Cancel dialog 'DescriptionObject'"
    DescriptionObject.Cancel
    printlog "Close application"
    Call hCloseDocument
endcase 'tdContextDescriptionObject

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextNameObject
    printlog "open application"
    Call hNewDocument
    WaitSlot (1000)
    printlog "create two rectangles"
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30, 40, 50, 60 )
    sleep 1
    printlog "select both by spanning a selection with the mouse"
    gMouseMove 1,1,70,70
    sleep 1
    printlog "Modify->Group"
    FormatGroupGroup
    WaitSlot (1000)
    printlog "Modify->Name Object"
    ContextNameObject
    Kontext "NameDlgObject"
    Call DialogTest (NameDlgObject)
    printlog "cancel dialog 'name'"
    NameDlgObject.Cancel
    printlog "Modify->UnGroup"
    FormatUngroupDraw
    printlog "close application"
    Call hCloseDocument
endcase 'tdContextNameObject

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyConnectBreak
    printlog "open application"
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 1
    printlog "create two rectangles"
    Call hRechteckErstellen (10,10,30,30)
    Call hRechteckErstellen (35,35,50,50)
    sleep 1
    printlog "Edit->Select All"
    EditSelectAll
    printlog "Modify->Connect"
    ContextConnect
    sleep 1
    try
        printlog "Modify->Break"
        ContextBreak
    catch
        Warnlog "- Modify-Break does not work"
    endcatch
    sleep 1
    printlog "close application"
    Call hCloseDocument
endcase 'tdModifyConnectBreak

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyShapes
    printlog "open application"
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 1
    printlog "create two rectangles"
    gMouseClick 50,50
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    printlog "Edit->Select All"
    EditSelectAll
    sleep 1
    try
        printlog "Modify->Shapes->Merge"
        ModifyShapesMerge         ' 1
        WaitSlot (1000)    'sleep 1
        Printlog "- Modify-Shape merge is working"
    catch
        Warnlog "- Modify-shape merge is not working"
    endcatch
    printlog "select all and delete it"
    EditSelectAll
    sleep 1
    hTypeKeys "<DELETE>"
    sleep 1
    printlog "create two rectangles"
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    printlog "Edit->Select All"
    EditSelectAll
    sleep 1
    try
        printlog "Modify->Shapes->Substract"
        ModifyShapesSubstract     ' 2
        Printlog "- Modify-shape-substract is working"
    catch
        Warnlog "- Modify-shape substract is not working"
    endcatch
    sleep 1
    printlog "select all and delete it"
    EditSelectAll
    sleep 1
    hTypeKeys "<DELETE>"
    sleep 1
    printlog "create two rectangles"
    Call hRechteckErstellen (30,30,50,50)
    sleep 1
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    EditSelectall
    sleep 1
    try
        printlog "Modify->Shapes->Intersect"
        ModifyShapesIntersect      ' 3
        Printlog "- Modify-shape intersect is working"
    catch
        Warnlog "- Modify-Shape intersect is not working"
    endcatch
    printlog "close application"
    Call hCloseDocument
endcase 'tdModifyShapes

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyCombineSplit
    printlog "open application"
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 1
    printlog "create two rectangles"
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    printlog "Edit->Select All"
    EditSelectAll
    sleep 1
    try
        printlog "Modify->Shapes->Combine"
        ContextCombine
        Printlog "- Modify combine is working"
        printlog "Modify->Shapes->Split"
        ContextSplit
        Printlog "- Modify-split is working"
    catch
        Warnlog "- Modify-combine and split are not working"
    endcatch
    printlog "close application"
    Call hCloseDocument
endcase 'tdModifyCombineSplit

'------------------------------------------------------------------------------------------------------------------------------------------------