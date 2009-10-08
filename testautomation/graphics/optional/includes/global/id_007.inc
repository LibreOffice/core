'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_007.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:41 $
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

    Call hNewDocument            ' imp: contextmenue same SID!
    sleep 1
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    try
        ContextFlipVerticalDraw
        Printlog "- Flip-vertical is working"
    catch
        Warnlog "- Flip-Vertical does not work"
    endcatch
    sleep 1
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyFlipHorizontal
    Call hNewDocument            ' imp: contextmenue same SID!
    WaitSlot (1000)
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    try
        ContextFlipHorizontalDraw
        Printlog "- Flip-horizontal is working"
    catch
        Warnlog "- Flip-horizontal does not work"
    endcatch
    sleep 1
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoCurve
    Call hNewDocument
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    ContextConvertIntoCurve
    WaitSlot (2000)
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoPolygon
    dim iWaitIndex as integer
    Call hNewDocument            ' imp: contextmenue same SID!
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
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
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
    Oeffnen.Click
    sleep 3
    ContextConvertIntoPolygon
    Kontext "InPolygonUmwandeln"
    Call DialogTest (InPolygonUmwandeln)
    LoecherFuellen.Check
    Farbanzahl.More
    Punktreduktion.More
    Kachelgroesse.More
    Vorschau.Click
    sleep 10
    InPolygonUmwandeln.Cancel
    sleep (2)
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoContour
    Call hNewDocument
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    ContextConvertIntoContour
    WaitSlot (1000)
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertInto3D
    Call hNewDocument
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    ContextConvertInto3D
    WaitSlot (1000)
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoRotationObject
    Call hNewDocument
    WaitSlot (1000)
    Call hRechteckErstellen (20,20,50,50)
    sleep 2
    ContextConvertInto3DRotationObject
    WaitSlot (1000)
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoBitmap
    Call hNewDocument
    WaitSlot (3000)
    InsertGraphicsFromFile
    WaitSlot (3000)
    Kontext "GrafikEinfuegenDlg"
    sleep 2
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\columbia.dxf")
    sleep 2
    Oeffnen.Click
    sleep 2
    try
        ContextConvertIntoBitmap
        Printlog "- Convert into bitmap is working"
    catch
        Warnlog "- Convert into bitmap does not work"
    endcatch
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextConvertIntoMetaFile
    Call hNewDocument
    WaitSlot (3000)
    InsertGraphicsFromFile
    WaitSlot (1000)
    kontext "Messagebox"
    if Messagebox.Exists (5) Then Messagebox.OK
        sleep 1
        Kontext "GrafikEinfuegenDlg"
        sleep 2
        Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
        sleep 2
        Preview.Click
        sleep 3
        Oeffnen.Click
        sleep 5
        try
            ContextConvertIntoMetafile
            Printlog "- convert into meta file does work"
        catch
            Warnlog "- convert into meta file does not work"
        endcatch
        Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyArrange
    Call hNewDocument
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30, 30, 50, 60 )
    FormatArrangeBringToFront
    WaitSlot (1000)
    ContextArrangeBringForward
    WaitSlot (1000)
    ContextArrangeBringBackward
    WaitSlot (1000)
    FormatArrangeSendToBack
    WaitSlot (1000)
    EditSelectAll
    ContextArrangeReverse
    WaitSlot (1000)
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyArrangeObjects
    Call hNewDocument
    WaitSlot (1000)
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30,30,50,50 )
    ContextArrangeInFrontOfObject
    gMouseClick 11,11
    ContextArrangeBehindObject
    gMouseClick 45,45
    sleep 1
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyAlignment
    Call hNewDocument
    WaitSlot (1000)
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    ContextAlignmentLeft
    WaitSlot (1000)
    ContextAlignmentCentered
    WaitSlot (1000)
    ContextAlignmentRight
    WaitSlot (1000)
    ContextAlignmentTop
    WaitSlot (1000)
    ContextAlignmentBottom
    WaitSlot (1000)
    ContextAlignmentCenter
    WaitSlot (1000)
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextDistribution
    Call hNewDocument
    WaitSlot (3000)
    Call hRechteckErstellen (20,20,30,30)
    Call hRechteckErstellen (40,40,50,50)
    Call hRechteckErstellen (60,60,70,70)
    sleep 1
    EditSelectAll
    sleep 1
    ContextDistribution
    Kontext "VerteilenDlg"
    sleep 1
    Call DialogTest (VerteilenDlg)
    sleep 1
    Links.Check
    MitteHorizontal.Check
    AbstandHorizontal.Check
    Rechts.Check
    KeineHorizontal.Check
    Oben.Check
    MitteVertikal.Check
    AbstandVertikal.Check
    Unten.Check
    KeineVertikal.Check
    VerteilenDlg.Cancel
    sleep 2
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextDescriptionObject
    Call hNewDocument
    WaitSlot (1000)
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    ContextDescriptionObject
    Kontext "DescriptionObject"
    Call DialogTest (DescriptionObject)
    DescriptionObject.Cancel
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdContextNameObject
    Call hNewDocument
    WaitSlot (1000)
    Call hRechteckErstellen ( 20, 20, 30, 50 )
    hTypeKeys("<escape>")
    Call hRechteckErstellen ( 30, 40, 50, 60 )
    sleep 1
    gMouseMove 1,1,95,95
    sleep 1
    FormatGroupGroup
    WaitSlot (1000)
    ContextNameObject
    Kontext "NameDlgObject"
    Call DialogTest (NameDlgObject)
    NameDlgObject.Cancel
    FormatUngroupDraw
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyConnectBreak
    Call hNewDocument
    sleep 1
    Call hRechteckErstellen (10,10,30,30)
    Call hRechteckErstellen (35,35,50,50)
    sleep 1
    EditSelectAll
    ContextConnect
    sleep 1
    try
        ContextBreak
    catch
        Warnlog "- Modify-Break does not work"
    endcatch
    sleep 1
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyShapes
    Call hNewDocument
    sleep 1
    gMouseClick 50,50
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    EditSelectAll
    sleep 1
    try
        ModifyShapesMerge         ' 1
        WaitSlot (1000)    'sleep 1
        Printlog "- Modify-Shape merge is working"
    catch
        Warnlog "- Modify-shape merge is not working"
    endcatch
    EditSelectAll
    sleep 1
    hTypeKeys "<DELETE>"
    sleep 1
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    EditSelectAll
    sleep 1
    try
        ModifyShapesSubstract     ' 2
        Printlog "- Modify-shape-substract is working"
    catch
        Warnlog "- Modify-shape substract is not working"
    endcatch
    sleep 1
    EditSelectAll
    sleep 1
    hTypeKeys "<DELETE>"
    sleep 1
    Call hRechteckErstellen (30,30,50,50)
    sleep 1
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    EditSelectall
    sleep 1
    try
        ModifyShapesIntersect      ' 3
        Printlog "- Modify-shape intersect is working"
    catch
        Warnlog "- Modify-Shape intersect is not working"
    endcatch
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------

testcase tdModifyCombineSplit
    Call hNewDocument
    sleep 1
    Call hRechteckErstellen (30,30,50,50)
    Call hRechteckErstellen (60,60,80,80)
    sleep 1
    EditSelectAll
    sleep 1
    try
        ContextCombine
        Printlog "- Modify combine is working"
        ContextSplit
        Printlog "- Modify-split is working"
    catch
        Warnlog "- Modify-combine and split are not working"
    endcatch
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------
