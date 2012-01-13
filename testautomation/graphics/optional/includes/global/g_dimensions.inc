'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

*********
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

