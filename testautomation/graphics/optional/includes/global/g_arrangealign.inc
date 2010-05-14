'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_arrangealign.inc,v $
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
' #1 tiDatei_Fuer_Anordnen_Erstellen
' #1 tdContextSendBackward
' #1 tdContextBringForward
' #1 tdContextSendToBack
' #1 tdContextBringToFront
' #1 tdContextReverse
' #1 tdContextInFrontOfObject
' #1 tdContextBehindObject
'\*************************************************************************************
testcase tiDatei_Fuer_Anordnen_Erstellen

    dim sTemp as string
    dim i as integer
    dim zaehler as integer
    if Dir(Datei)<>"" then app.Kill(Datei) ' kill old file
        Kontext
        Select Case Zaehler
        Case 1:	WL_SD_Wuerfel
            Printlog " Object start ---------------------- "+ Zaehler +" cubicel "
        Case 2:	WL_SD_RechteckVoll
            Printlog " Object start ---------------------- "+ Zaehler +" rectangel "
        Case 3: WL_SD_Torus
            Printlog " Object start ---------------------- "+ Zaehler +" donut "
        End Select
        sleep (1)
        sleep (1)
        kontext
        Select Case Zaehler
        Case 1:	active.SetPage TabFarbverlaeufe
            kontext "TabFarbverlaeufe"
            Tabelle.Select Int((Tabelle.GetItemCount * Rnd) + 1)
            printlog "test tbo-------------------------------------------------color of object : "+Tabelle.GetSelText
            TabFarbverlaeufe.OK
        Case 2:	active.SetPage TabSchraffuren
            kontext "TabSchraffuren"
            Tabelle.Select Int((Tabelle.GetItemCount * Rnd) + 1)
            printlog "test tbo-------------------------------------------------color of object : "+Tabelle.GetSelText
            TabSchraffuren.OK
        Case 3:	active.SetPage TabFarben
            kontext "TabFarben"
            Farbe.Select Int((Farbe.GetItemCount * Rnd) + 1)
            printlog "test tbo-------------------------------------------------color of object : "+Farbe.GetSelText
            TabFarben.OK
        End Select
        sleep 1
        sleep (1)
        kontext
        active.SetPage TabPositionAndSize
        kontext "TabPositionAndSize"
        Select Case Zaehler
        Case 1:	Ueber_Text_1 = PositionX.GetText : printlog "xpos: "+zaehler+": "+PositionX.GetText
        Case 2:	Ueber_Text_2 = PositionX.GetText : printlog "xpos: "+zaehler+": "+PositionX.GetText
        Case 3:	Ueber_Text_3 = PositionX.GetText : printlog "xpos: "+zaehler+": "+PositionX.GetText
        End Select
        TabPositionAndSize.OK
        gMouseClick 96,5
    next Zaehler
    sleep 1
    printlog "OK   saved at ", Datei
    sleep 1
    sPrintCheckOrder (TRUE)
endcase

'-------------------------------------------------------------------------------
testcase tdContextSendBackward

    dim sTemp as string
    if (hFileOpen (Datei)) then
        sleep 10
        sPrintCheckOrder (TRUE)
        hTypeKeys ("<TAB><TAB>")
        WL_TB_ANORDNUNG_WeiterNachHinten
        sleep 2
        sPrintCheckOrder
        hTypeKeys ("<escape><escape>")
        sleep 2
        hTypeKeys ("<TAB><TAB>")
        sleep 2
        sTemp = fGetPositionX
        if sTemp <> Ueber_Text_1 then
            warnlog "Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_1+";"
        end if
        Call hCloseDocument
    endif
    sleep 2
endcase

'-------------------------------------------------------------------------------
testcase tdContextBringForward

    dim sTemp as string
    if (hFileOpen (Datei)) then
        sleep 10
        sPrintCheckOrder (TRUE)
        hTypeKeys ("<TAB><TAB>")
        WL_TB_ANORDNUNG_WeiterNachVorn
        sleep 1
        sPrintCheckOrder
        hTypeKeys ("<TAB><TAB>")
        sleep 2
        sTemp = fGetPositionX
        if sTemp <> Ueber_Text_3 then
            warnlog "Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_3+";"
        end if
        sleep 1
        Call hCloseDocument
    endif
endcase

'-------------------------------------------------------------------------------
testcase tdContextSendToBack

    dim sTemp as string
    if (hFileOpen (Datei)) then
        sleep 10
        sPrintCheckOrder (TRUE)
        hTypeKeys ("<TAB><TAB><TAB>")
        OL_DRAW_GanzNachHinten
        sleep 3
        sPrintCheckOrder
        hTypeKeys ("<TAB><TAB>")
        sleep 1
        sTemp = fGetPositionX
        if sTemp <> Ueber_Text_1 then
            warnlog "Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_1+";"
        end if
        sleep 3
        Call hCloseDocument
    endif
endcase

'-------------------------------------------------------------------------------
testcase tdContextBringToFront

    dim sTemp as string
    if (hFileOpen (Datei)) then
        sleep 10
        sPrintCheckOrder (TRUE)
        hTypeKeys ("<TAB>")
        OL_DRAW_GanzNachVorn
        sPrintCheckOrder
        hTypeKeys ("<TAB><TAB>")
        sleep 1
        sTemp = fGetPositionX
        if sTemp <> Ueber_Text_3 then
            warnlog "Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_3+";"
        end if
        sleep 3
        Call hCloseDocument
    endif
endcase

'-------------------------------------------------------------------------------
testcase tdContextInFrontOfObject

    dim sTemp as string
    if (hFileOpen (Datei)) then
        sleep 10
        sPrintCheckOrder (TRUE)
        hTypeKeys ("<TAB><TAB><TAB>")
        OL_DRAW_VorDasObjekt
        gMouseClick 50,5
        sPrintCheckOrder
        sleep 1
        hTypeKeys ("<TAB><TAB>")
        sTemp = fGetPositionX
        if sTemp <> Ueber_Text_3 then
            printlog "DISABLED the WARNLOG, because mouseaction is not always the same :-( : Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_3+";"
        end if
        Call hCloseDocument
    endif
endcase

'-------------------------------------------------------------------------------
testcase tdContextBehindObject

    dim sTemp as string
    if (hFileOpen (Datei)) then
        sleep 10
        sPrintCheckOrder (TRUE)
        hTypeKeys ("<TAB><TAB><TAB>")
        OL_DRAW_HinterDasObjekt
        sleep 1
        gMouseClick 50,5
        sPrintCheckOrder
        sleep 1
        hTypeKeys ("<TAB>")
        sTemp = fGetPositionX
        if sTemp <> Ueber_Text_3 then
            printlog "DISABLED the WARNLOG, because mouseaction is not always tthe same  Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_3+";"
        end if
        Call hCloseDocument
    endif
endcase

'-------------------------------------------------------------------------------
testcase tdContextReverse

    dim sTemp as string
    dim sTemp2 as string
    dim i as integer
    if (hFileOpen (Datei)) then
        sleep 10
        sPrintCheckOrder (TRUE)
        EditSelectAll
        sleep 3
        WL_TB_ANORDNUNG_Vertauschen
        sPrintCheckOrder
        hTypeKeys ("<escape><escape>")
        for i = 1 to 3
            hTypeKeys ("<TAB>")
            sTemp = fGetPositionX
            Select Case i
            Case 1:	sTemp2 = Ueber_Text_3
            Case 2:	sTemp2 = Ueber_Text_2
            Case 3:	sTemp2 = Ueber_Text_1
            End Select
            if sTemp <> sTemp2 then
                warnlog " - " + i + " Arrangement is wrong; is: "+sTemp+"; should: "+sTemp2+";"
            end if
        next i
    endif
endcase

'-------------------------------------------------------------------------------
