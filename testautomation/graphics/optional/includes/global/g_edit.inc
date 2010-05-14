'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_edit.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:40 $
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
' #1 tEditDuplicate
' #1 tEditFields
' #1 tEditDeleteSlide
' #1 tEditLinks
' #1 tdEditDeleteLayer
' #1 tEditObjectEdit
'\*************************************************************************************

'   sDatei = gTesttoolPath + "graphics\required\input\leer.sx"+left(gApplication,1)
'   sDatei = gTesttoolPath + "graphics\required\input\leer.sx"+left(gApplication,1)
'      Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")

testcase tEditDuplicate
    dim Ueber_Text_1
    dim Zaehler

    Kontext "Duplizieren"
    Ueber_Text_1 = AnzahlAnKopien.GetText
    sleep 1
    For Zaehler = 1 to 8
        sleep 1
        Kontext "Duplizieren"

    Case 1:	XAchse.SetText "0,0"
        YAchse.SetText "0,45"
        Hoehe.SetText  "-0,1"
        Breite.SetText "-0,1"
        Drehwinkel.SetText "3"
        Anfang.Select Int((Anfang.GetItemCount * Rnd)+1)
    Case 2,6:	XAchse.SetText "0,6"
        YAchse.SetText "0,0"
        Hoehe.SetText  "0,15"
        Breite.SetText "0,01"
        Drehwinkel.SetText "353"
    Case 3,7:	XAchse.SetText "0,0"
        YAchse.SetText "-0,4"
        Hoehe.SetText  "-0,1"
        Breite.SetText "0,06"
        Drehwinkel.SetText "11"
    Case 4,8:	XAchse.SetText "-0,6"
        YAchse.SetText "0,0"
        Hoehe.SetText  "0,05"
        Breite.SetText "-0,08"
        Drehwinkel.SetText "347"
    Case 5:	XAchse.SetText "0,1"
        YAchse.SetText "0,48"
        Hoehe.SetText  "-0,05"
        Breite.SetText "-0,05"
        Drehwinkel.SetText "355"
    End Select
    Ende.Select Int((Ende.GetItemCount * Rnd)+1)
    sleep 1
Next Zaehler
sleep 2
endcase

testcase tEditFields
    Call hNewDocument
    sleep 3
    sleep 1
    sleep 2
    sleep 2
    Kontext "FeldbefehlBearbeitenDraw"
    sleep 2
    If	FeldtypFix.Ischecked then
    Printlog "OK   FieldtypeFix checkable"
else
    warnlog "FieldtypeFix not checkable"
end if
sleep 1
If	FeldtypVariabel.Ischecked then
Printlog "OK   FieldtypeVariabel enabled"
else
warnlog "FieldtypeVariabel not enabled"
end if
'   For Zaehler = 1 to FeldtypFormat.GetItemCount
'    FeldTypFormat.Select Zaehler
'    printlog "OK   Format ", Zaehler, " = ", FeldtypFormat.GetSelText
'   Next Zaehler
sleep 2
sleep 2
endcase

testcase tEditDeleteSlide
    dim Ueber_Text_1 as string
    dim Ueber_Text_2 as string
    dim Ueber_Text_3 as string
    sleep 3
    sleep 1
    kontext
    Messagebox.SetPage TabPositionAndSize
    kontext "TabPositionAndSize"
    TabPositionAndSize.OK
    sleep 2
    hTypekeys "<Pagedown>"
    sleep 2
    sleep 1
    sleep 1
    Ueber_Text_2 = "OK   Page was closed"
    Ueber_Text_3 = "Page was not closed"
    sleep 1
endcase

testcase tEditLinks
    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
    Oeffnen.Click
    sleep 3
    Kontext "VerknuepfungenBearbeiten"
    sleep 1
    'Aendern							'kann man noch den Dialog aufrufen...
    Loesen.Click
    sleep 1
    Kontext
    Messagebox.Yes
    sleep 1
    try
        Kontext "VerknuepfungenBearbeiten"
        Loesen.Click
        sleep 1
        Kontext
        Messagebox.Yes
        VerknuepfungenBearbeiten.Close
        warnlog "Break link does not work"
    catch
        printlog "OK   Link broken"
    endcatch
endcase

testcase tdEditDeleteLayer
    dim Ueber_Text_1 as string
    dim Ueber_Text_2 as string
    dim Ueber_Text_3 as string
    sleep 2
    sleep 1
    ViewLayer
    kontext
    active.SetPage TabPositionAndSize
    kontext "TabPositionAndSize"
    Ueber_Text_1 = PositionX.GetText
    TabPositionAndSize.OK
    Kontext "EbeneEinfuegenDlg"
    EbeneEinfuegenDlg.OK
    sleep 1
    sleep 1
    Kontext
    Active.Yes
    sleep 1
    Ueber_Text_3 = "Layer was not deleted"
    Call Position_Vergleichen (Ueber_Text_1,Ueber_Text_2,Ueber_Text_3)
    sleep 1
endcase

testcase tEditObjectEdit
    Dim Schrieb  as string

    Schrieb = gOLEWriter
    if Schrieb = "" then
        warnlog " the OLE is not defined :-((( gOLEWriter: " + iSprache
    else

        Call hNewDocument
        Kontext "OLEObjektEinfuegen"
        sleep 3
        if ObjektTyp.GetItemCount = 0 Then
            Warnlog "- No available Ole-entry in the list, maybe an error in the Install.ini?"
            sleep 1
            OleObjektEinfuegen.Cancel
            sleep 1
        else

            ObjektTyp.Select Schrieb
            sleep 1
            OLEObjektEinfuegen.OK
            sleep 3                                 ' Changed from Sleep 1 to Sleep 3
            gMouseMove  10,10,90,90
            sleep 1
            EditSelectAll

            try
                sleep 2
            catch
                Warnlog "-  The EditObjectEdit did not work"
            endcatch


            gMouseClick 90,90
        end if
        sleep 1
    endif
endcase
