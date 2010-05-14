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

'///      Dateiname.settext Convertpath (gTesttoolPath + "global\input\graf_inp\stabler.tif") ///'

testcase tiInsertSlide

    '/// open application ///'
    Call  hNewDocument
    '/// Insert->Slide ///'
    InsertSlide
    WaitSlot (2000)
    hTypekeys "<Pagedown>"
    WaitSlot (2000) 'sleep 2
    '/// close application ///'
    Call  hCloseDocument
endcase

testcase tiInsertDuplicateSlide
    '/// open application ///'
    Call hNewDocument
    '/// create rectangle ///'
    Call hRechteckErstellen ( 30, 40, 40, 50 )
    '/// Insert->Duplicate Slide ///'
    InsertDuplicateSlide
    WaitSlot (2000)
    '/// close application ///'
    Call  hCloseDocument
endcase

testcase tiInsertField
    '/// open application ///'
    Call hNewDocument

    '/// Insert->Fields->Time (fixed) ///'
    InsertFieldsTimeFix
    WaitSlot (1000)
    printlog "OK   Time Fix"
    '/// delete it with EditSelectAll + <Delete> ///'
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    '/// Insert->Fields->Date (fixed) ///'
    InsertFieldsDateFix
    WaitSlot (1000)
    printlog "OK   Date Fix"
    '/// delete it with EditSelectAll + <Delete> ///'
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    '/// Insert->Fields->Time (variable) ///'
    InsertFieldsTimeVariable
    WaitSlot (1000)
    printlog "OK   Time Variabel"
    '/// delete it with EditSelectAll + <Delete> ///'
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    '/// Insert->Fields->Date (variable) ///'
    InsertFieldsDateVariable
    WaitSlot (1000)
    printlog "OK   Date Variabel"
    '/// delete it with EditSelectAll + <Delete> ///'
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    '/// Insert->Fields->Author ///'
    InsertFieldsAuthorDraw
    WaitSlot (1000)
    printlog "OK   Author"
    '/// delete it with EditSelectAll + <Delete> ///'
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    '/// Insert->Fields->Page Numbers///'
    InsertFieldsPageNumberDraw
    WaitSlot (1000)
    printlog "OK   Page number"
    '/// delete it with EditSelectAll + <Delete> ///'
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    '/// Insert->Fields->Filename ///'
    InsertFieldsFileName
    WaitSlot (1000) 'sleep 1
    printlog "OK   File name"
    '/// delete it with EditSelectAll + <Delete> ///'
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 2
    '/// close application ///'
    Call  hCloseDocument
endcase

testcase tiInsertSpecialCharacter
    '/// open application ///'
    Call hNewDocument

    '/// insert text ///'
    hTextrahmenErstellen ("This is a testtext",30,40,60,50)
    sleep 2
    '/// Insert->SpecialCharacter ///'
    InsertSpecialCharacterDraw
    WaitSlot (2000)
    Kontext "Sonderzeichen"
        Call DialogTest (Sonderzeichen)
        '/// cancel dialog 'Special Characters' ///'
        Sonderzeichen.Cancel
        sleep 2
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertHyperlink
    '/// open application ///'
    Call hNewDocument
    '/// Insert->Hyperlink ///'
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
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertGraphic
    '/// open application ///'
    Call  hNewDocument
    '/// Insert->Graphics ///'
    InsertGraphicsFromFile
    WaitSlot (2000) '
    try
        Kontext "GrafikEinfuegenDlg"
            '/// if checkbox 'link' is available, check it; if not available: ERROR ///'
            if Link.exists then
                Link.Check
                else
                Warnlog  "Linking grafik doesn't work :-("
            end if
            '/// if checkbox 'Preview' is available, check it; if not available: ERROR ///'
            if Preview.exists then
                Preview.Check
            else
                Warnlog "Preview of graphic doesn't work :-("
            end if
            DialogTest (GrafikEinfuegenDlg)

            '/// use file: "global\input\graf_inp\stabler.tif") ///'
            Dateiname.settext Convertpath (gTesttoolPath + "global\input\graf_inp\stabler.tif")
            '/// click 'open' ///'
            Oeffnen.click
    catch
        Warnlog "Insert graphic doesn't work :-("
    endcatch

    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertObjectSound
    goto endsub ' disabled for final, because always wrong (TZ 01/2002)
    'TODO: TBO: enhance!
    '/// open application ///'
    Call hNewDocument
    try
        '/// Insert->Object->Sound ///'
        InsertObjectSound
        WaitSlot (1000)
        Kontext "OeffnenDlg"
            '       Call Dialogtest (OeffnenDlg) ' just be sure to check one pth and one open dialog : TZ 28.11.201

            '/// cancel file open dialog ///'
            OeffnenDlg.Cancel
    catch
        printlog "'Insert -> Object -> Sound' not available. TestDevelopmentInProgress (TDIP) ;-)"
    endcatch
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertObjectVideo
    goto endsub
    'TODO: TBO: enhance!
    '/// open application ///'
    Call hNewDocument
    try
        '/// Insert->Object->Video ///'
        InsertObjectVideo
        Kontext "OeffnenDlg"
            '      Call Dialogtest (OeffnenDlg)
            WaitSlot (1000)
            '/// cancel file open dialog ///'
            OeffnenDlg.Cancel
    catch
        printlog "'Insert -> Object -> Video' not available. (TDIP) ;-)"
    endcatch
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertChart
    '/// open application ///'
    Call hNewDocument
    '/// Insert->Chart ///'
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
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertObjectOLEObjects
    '/// open application ///'
    hNewDocument
    '/// Insert->Object->OLEObjekts ///'
    InsertObjectOLEObject
    Kontext "OLEObjektEinfuegen"
        '   Call Dialogtest ( OLEObjektEinfuegen )
        '   NeuErstellen.Check ' is default value
        Call DialogTest (OLEObjektEinfuegen, 1)
        '/// check 'Create from file' ///'
        AusDateiErstellen.Check
        Call DialogTest (OLEObjektEinfuegen, 2)
        '/// click 'Search...' ///'
        Durchsuchen.click
        Kontext "OeffnenDlG"
            '/// cancel file open dialog ///'
            OeffnenDLG.Cancel
    Kontext "OLEObjektEinfuegen"
        '/// cancel dialog 'insert OLE Object' ///'
        OLEObjektEinfuegen.Cancel
        sleep 1
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertSpreadsheet
    if gtSYSName = "Linux" then
        printlog "Linux = wont test tiInsertSpreadsheet"
    goto endsub
endif

'/// open application ///'
    Call hNewDocument
    WaitSlot (2000)
    '/// Insert->Spreadsheet ///'
    InsertSpreadsheetDraw
    WaitSlot (2000)
    Kontext "Messagebox"
        if Messagebox.Exists (5) then
            Warnlog Messagebox.GetText
            Messagebox.OK
        end if
        sleep 2
        '/// click somewhere out of the object, to deselect it ///'
        gMouseClick 1,1
        sleep 1
        '/// select and delete object with keys: <Tab><Delete> ///'
        hTypekeys "<Tab><Delete>"
        sleep 2
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertFormula
    '/// open application ///'
    Call hNewDocument
    '/// Insert->Object->Formula ///'
    InsertObjectFormulaDraw
    WaitSlot (2000)
    Kontext "Messagebox"
        if Messagebox.Exists then
            Warnlog Messagebox.GetText
            Messagebox.OK
            sleep 1
        end if
        '/// click somewhere out of the object, to deselect it ///'
        gMouseClick 1,1
        sleep 1
        '/// select and delete object with keys: <Tab><Delete> ///'
        hTypekeys "<Tab><Delete>"
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertFloatingFrame
    '/// open application ///'
    Call hNewDocument
    '/// Insert->Floating Frame ///'
    InsertFloatingFrame
    WaitSlot (2000)
    Kontext "TabEigenschaften"
        Dialogtest (TabEigenschaften)
        '/// click '...' ///'
        Oeffnen.Click
        Kontext "OeffnenDlg"
            sleep 1
            '/// cancel file open dialog ///'
            OeffnenDlg.Cancel
    Kontext "TabEigenschaften"
        '/// cancel dialog 'Floating Frame Properties' ///'
        TabEigenschaften.Cancel
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertFile
    '/// open application ///'
    Call  hNewDocument
    WaitSlot (1000)
    '/// Insert->File ///'
    InsertFileDraw
    WaitSlot (1000)
    Kontext "OeffnenDLG"
        '   	Call Dialogtest ( OeffnenDLG )
        '/// cancel file open dialog ///'
        OeffnenDLG.Cancel
    '/// close application ///'
    Call  hCloseDocument
endcase

testcase tiInsertPlugin
    '/// open application ///'
    call hNewDocument
    '/// Insert->Object->PlugIn ///'
    InsertObjectPlugIn
    Kontext "PluginEinfuegen"
        if PluginEinfuegen.exists (5) then
            call Dialogtest (PluginEinfuegen)
            '/// click 'browse' ///'
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
                    '/// cancel file open dialog ///'
                    OeffnenDLG.Cancel
                end if
    Kontext "PluginEinfuegen"
        '/// cancel dialog 'Insert Plug-in' ///'
        if PluginEinfuegen.exists (5) then PluginEinfuegen.Cancel
        else
            warnlog "Insert Plugin does not work :-("
        end if
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertScan
    goto endsub
    '/// DISABLED because not able to make automatic :-( ///'
    '/// open application ///'
    Call hNewDocument
    '/// Insert->Scan->Request ///'
    InsertScanRequest ' as long as there is no scanner available, nothing happens
    WaitSlot (1000)
    '/// Insert->Scan->SelectSource ///'
    InsertScanSelectSource
    WaitSlot (1000)
    printlog "Not testable, not translatable, just callable, because of systemdialog :-("
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiInsertSnappointLine
    '/// open application ///'
    Call  hNewDocument
    '/// DRAW   : Insert->Snap Point/Line  ///'
    '/// + IMPRESS: available via Kontext menu: Insert->Snap Point/Line  ///'
    InsertSnapPointLine
    Kontext "NeuesFangobjekt"
        DialogTest ( NeuesFangobjekt )
        '/// Cancel dialog 'New Snap Object' ///'
        NeuesFangobjekt.Cancel
        sleep 2
    '/// close application ///'
    Call  hCloseDocument
endcase

testcase tdInsertLayer
    '/// open application ///'
    Call  hNewDocument
    WaitSlot (1000)
    '/// View->Layer ///'
    ViewLayer
    '/// Insert->Layer ///'
    InsertLayer
    Kontext "EbeneEinfuegenDlg"
        DialogTest ( EbeneEinfuegenDlg )
        '/// cancel dialog 'Insert Layer' ///'
        EbeneEinfuegenDlg.Cancel
    '/// close application ///'
    Call  hCloseDocument
endcase

