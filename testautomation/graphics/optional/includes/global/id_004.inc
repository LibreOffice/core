'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_004.inc,v $
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
' #1 tiInsertSlide
' #1 tiInsertDuplicateSlide
' #1 tiInsertField
' #1 tiInsertSpecialCharacter
' #1 tiInsertHyperlink
' #1 tiInsertGraphic
' #1 tiInsertObjectSound
' #1 tiInsertObjectVideo
' #1 tiInsertChart
' #1 tiInsertObjectOLEObjects
' #1 tiInsertSpreadsheet
' #1 tiInsertFormula
' #1 tiInsertFloatingFrame
' #1 tiInsertFile
' #1 tiInsertPlugin
' #1 tiInsertScan
' #1 tiInsertSnappointLine
' #1 tdInsertLayer
'\**********************************************************************************


testcase tiInsertSlide

    Call  hNewDocument
    InsertSlide
    WaitSlot (2000)
    hTypekeys "<Pagedown>"
    WaitSlot (2000) 'sleep 2
    Call  hCloseDocument
endcase

testcase tiInsertDuplicateSlide
    Call hNewDocument
    Call hRechteckErstellen ( 30, 40, 40, 50 )
    InsertDuplicateSlide
    WaitSlot (2000)
    Call  hCloseDocument
endcase

testcase tiInsertField
    Call hNewDocument

    InsertFieldsTimeFix
    WaitSlot (1000)
    printlog "OK   Time Fix"
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    InsertFieldsDateFix
    WaitSlot (1000)
    printlog "OK   Date Fix"
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    InsertFieldsTimeVariable
    WaitSlot (1000)
    printlog "OK   Time Variabel"
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    InsertFieldsDateVariable
    WaitSlot (1000)
    printlog "OK   Date Variabel"
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    InsertFieldsAuthorDraw
    WaitSlot (1000)
    printlog "OK   Author"
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    InsertFieldsPageNumberDraw
    WaitSlot (1000)
    printlog "OK   Page number"
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    InsertFieldsFileName
    WaitSlot (1000) 'sleep 1
    printlog "OK   File name"
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 2
    Call  hCloseDocument
endcase

testcase tiInsertSpecialCharacter
    Call hNewDocument

    hTextrahmenErstellen ("This is a testtext",30,40,60,50)
    sleep 2
    InsertSpecialCharacterDraw
    WaitSlot (2000)
    Kontext "Sonderzeichen"
    Call DialogTest (Sonderzeichen)
    Sonderzeichen.Cancel
    sleep 2
    Call hCloseDocument
endcase

testcase tiInsertHyperlink
    Call hNewDocument
    InsertHyperlink
    WaitSlot (5000)
    Kontext "Hyperlink"
    if Hyperlink.exists (5) then
        Auswahl.MouseDown 50, 5
        Auswahl.MouseUp 50, 5
        Auswahl.typekeys "<PAGEDOWN><PAGEUP>"
        Auswahl.typekeys "<TAB>"
        sleep 3
        Kontext "TabHyperlinkInternet"

        'Workaround to get rid of a Focusing-problem...
        NameText.Typekeys "alal <RETURN>"
        NameText.Typekeys "<MOD1 A><DELETE>"
        TabHyperlinkInternet.Typekeys "<TAB>", 6
        TabHyperlinkInternet.Typekeys "<LEFT>", 3
        'End of workaround...

        Internet.Check
        ZielUrl.Settext "http://mahler"
        Kontext "Hyperlink"
        Uebernehmen.Click
        Hyperlink.Close
    else
        warnlog "Failed to open hyperlink ??!!"
    end if
    Call hCloseDocument
endcase

testcase tiInsertGraphic
    Call  hNewDocument
    InsertGraphicsFromFile
    WaitSlot (2000) '
    try
        Kontext "GrafikEinfuegenDlg"
        if Link.exists then
            Link.Check
        else
            Warnlog  "Linking grafik doesn't work :-("
        end if
        if Preview.exists then
            Preview.Check
        else
            Warnlog "Preview of graphic doesn't work :-("
        end if
        DialogTest (GrafikEinfuegenDlg)

        Dateiname.settext Convertpath (gTesttoolPath + "global\input\graf_inp\stabler.tif")
        Oeffnen.click
    catch
        Warnlog "Insert graphic doesn't work :-("
    endcatch

    Call hCloseDocument
endcase

testcase tiInsertObjectSound
    goto endsub ' disabled for final, because always wrong (TZ 01/2002)
    'TODO: TBO: enhance!
    Call hNewDocument
    try
        InsertObjectSound
        WaitSlot (1000)
        Kontext "OeffnenDlg"
        '       Call Dialogtest (OeffnenDlg) ' just be sure to check one pth and one open dialog : TZ 28.11.201

        OeffnenDlg.Cancel
    catch
        printlog "'Insert -> Object -> Sound' not available. TestDevelopmentInProgress (TDIP) ;-)"
    endcatch
    Call hCloseDocument
endcase

testcase tiInsertObjectVideo
    goto endsub
    'TODO: TBO: enhance!
    Call hNewDocument
    try
        InsertObjectVideo
        Kontext "OeffnenDlg"
        '      Call Dialogtest (OeffnenDlg)
        WaitSlot (1000)
        OeffnenDlg.Cancel
    catch
        printlog "'Insert -> Object -> Video' not available. (TDIP) ;-)"
    endcatch
    Call hCloseDocument
endcase

testcase tiInsertChart
    Call hNewDocument
    InsertChart
    WaitSlot (2000)
    Kontext "Messagebox"
    if Messagebox.Exists then
        Warnlog Messagebox.GetText
        Messagebox.OK
        sleep 1
    end if
    gMouseClick 1,1
    sleep 2
    Call hCloseDocument
endcase

testcase tiInsertObjectOLEObjects
    hNewDocument
    InsertObjectOLEObject
    Kontext "OLEObjektEinfuegen"
    '   Call Dialogtest ( OLEObjektEinfuegen )
    '   NeuErstellen.Check ' is default value
    Call DialogTest (OLEObjektEinfuegen, 1)
    AusDateiErstellen.Check
    Call DialogTest (OLEObjektEinfuegen, 2)
    Durchsuchen.click
    Kontext "OeffnenDlG"
    OeffnenDLG.Cancel
    Kontext "OLEObjektEinfuegen"
    OLEObjektEinfuegen.Cancel
    sleep 1
    Call hCloseDocument
endcase

testcase tiInsertSpreadsheet
    if gtSYSName = "Linux" then
        printlog "Linux = wont test tiInsertSpreadsheet"
        goto endsub
    endif

    Call hNewDocument
    WaitSlot (2000)
    InsertSpreadsheetDraw
    WaitSlot (2000)
    Kontext "Messagebox"
    if Messagebox.Exists (5) then
        Warnlog Messagebox.GetText
        Messagebox.OK
    end if
    sleep 2
    gMouseClick 1,1
    sleep 1
    hTypekeys "<Tab><Delete>"
    sleep 2
    Call hCloseDocument
endcase

testcase tiInsertFormula
    Call hNewDocument
    InsertObjectFormulaDraw
    WaitSlot (2000)
    Kontext "Messagebox"
    if Messagebox.Exists then
        Warnlog Messagebox.GetText
        Messagebox.OK
        sleep 1
    end if
    gMouseClick 1,1
    sleep 1
    hTypekeys "<Tab><Delete>"
    Call hCloseDocument
endcase

testcase tiInsertFloatingFrame
    Call hNewDocument
    InsertFloatingFrame
    WaitSlot (2000)
    Kontext "TabEigenschaften"
    Dialogtest (TabEigenschaften)
    Oeffnen.Click
    Kontext "OeffnenDlg"
    sleep 1
    OeffnenDlg.Cancel
    Kontext "TabEigenschaften"
    TabEigenschaften.Cancel
    Call hCloseDocument
endcase

testcase tiInsertFile
    Call  hNewDocument
    WaitSlot (1000)
    InsertFileDraw
    WaitSlot (1000)
    Kontext "OeffnenDLG"
    '   	Call Dialogtest ( OeffnenDLG )
    OeffnenDLG.Cancel
    Call  hCloseDocument
endcase

testcase tiInsertPlugin
    call hNewDocument
    InsertObjectPlugIn
    Kontext "PluginEinfuegen"
    if PluginEinfuegen.exists (5) then
        call Dialogtest (PluginEinfuegen)
        Durchsuchen.Click
        sleep 1
        Kontext "Messagebox"
        if Messagebox.Exists (5) Then
            Warnlog Messagebox.GetText
            Messagebox.OK
        else
            printlog "No Messagebox :-)"
        end if
        Kontext "OeffnenDlG"
        if OeffnenDlG.exists (5) then
            sleep 1
            OeffnenDLG.Cancel
        end if
        Kontext "PluginEinfuegen"
        if PluginEinfuegen.exists (5) then PluginEinfuegen.Cancel
        else
            warnlog "Insert Plugin does not work :-("
        end if
        Call hCloseDocument
endcase

testcase tiInsertScan
    goto endsub
    Call hNewDocument
    InsertScanRequest ' as long as there is no scanner available, nothing happens
    WaitSlot (1000)
    InsertScanSelectSource
    WaitSlot (1000)
    printlog "Not testable, not translatable, just callable, because of systemdialog :-("
    Call hCloseDocument
endcase

testcase tiInsertSnappointLine
    Call  hNewDocument
    InsertSnapPointLine
    Kontext "NeuesFangobjekt"
    DialogTest ( NeuesFangobjekt )
    NeuesFangobjekt.Cancel
    sleep 2
    Call  hCloseDocument
endcase

testcase tdInsertLayer
    Call  hNewDocument
    WaitSlot (1000)
    ViewLayer
    InsertLayer
    Kontext "EbeneEinfuegenDlg"
    DialogTest ( EbeneEinfuegenDlg )
    EbeneEinfuegenDlg.Cancel
    Call  hCloseDocument
endcase

