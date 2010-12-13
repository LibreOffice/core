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
'***********************************************************************************
'*
' #1 tdFormatDimensioning
'*
'\***********************************************************************************

Sub testFormatDimensioning

    call tdFormatDimensioning

End Sub

testcase tdFormatDimensioning
    Dim ZaehlerMetrik
    Dim i

    Call hNewDocument

    FormatDimensioning
    Kontext "Bemassung"
    LinienDistanz.ToMin
    PrintLog  LinienDistanz.GetText + " = minimum value"
    LinienDistanz.ToMax
    PrintLog  LinienDistanz.GetText + " = maximum value"

    HLUeberhang.ToMin
    PrintLog HLUeberhang.GetText + " = minimum value"
    HLUeberhang.ToMax
    PrintLog HLUeberhang.GetText + " = maximum value"

    HLDistanz.ToMin
    PrintLog HLDistanz.GetText + " = minimum value"
    HLDistanz.ToMax
    PrintLog HLDistanz.GetText + " = maximum value"

    LinkeHL.ToMin
    PrintLog LinkeHL.GetText + " = minimum value"
    LinkeHL.ToMax
    PrintLog LinkeHL.GetText + " = maximum valuet"

    RechteHL.ToMin
    PrintLog RechteHL.GetText + " = minimum value"
    RechteHL.ToMax
    PrintLog RechteHL.GetText + " = maximum value"

    MetrikList.GetItemCount
    ZaehlerMetrik=MetrikList.GetItemCount
    for i = 1 to ZaehlerMetrik
        MetrikList.Select i
        PrintLog MetrikList.GetSelText + " set"
    next i
    MetrikAnzeigen.Check
    AnzeigeUnterhalb.Check
    AnzeigeParallel.Check
    AutomatischHorizontal.Check
    AutomatischVertikal.Check
    sleep 2
    Bemassung.OK

    FormatDimensioning
    Kontext "Bemassung"
    MetrikAnzeigen.GetState
    AnzeigeUnterhalb.GetState
    AnzeigeParallel.GetState
    AutomatischHorizontal.GetState
    AutomatischVertikal.GetState
    if MetrikAnzeigen.GetState = 1 Then
        PrintLog "  show metric works"
    else
        WarnLog "  show metric does not work"
    end if
    if AnzeigeUnterhalb.GetState= 1 Then
        PrintLog "  AnzeigeUnterhalb does work"
    else
        WarnLog "  TriStateBox AnzeigeUnterhalb does not work"
    end if
    if AnzeigeParallel.GetState= 1 Then
        PrintLog "  AnzeigeParallel does work"
    else
        WarnLog "  TriStateBox AnzeigeParallel does not work properly"
    end if
    Bemassung.OK
    Call hCloseDocument
endcase

