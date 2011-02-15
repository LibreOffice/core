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
sub id_004

    printLog Chr(13) + "---------    id_004    ----------"

    call tiInsertSlide
    call tiInsertDuplicateSlide
    ' v expand slide
    '  v summary slide
    call tiInsertField
    call tiInsertSpecialCharacter
    call tiInsertHyperlink
    call tiInsertScan
    call tiInsertGraphic
    call tiInsertObjectSound
    call tiInsertObjectVideo
    call tiInsertObjectSound
    call tiInsertObjectVideo
    call tiInsertChart
    call tiInsertObjectOLEObjects
    call tiInsertSpreadsheet
    call tiInsertFormula
    call tiInsertFloatingFrame
    call tiInsertFile
    call tiInsertPlugin
    call tiInsertSnappointLine
    call tdInsertLayer ' IMPRESS: Edit->Layer->Insert
end sub
'------------------------------------------------------------------------------
testcase tiInsertSlide

    printlog "Dateiname.settext Convertpath (gTesttoolPath + global\input\graf_inp\stabler.tif) "
    printlog "open application"
    Call  hNewDocument
    printlog "Insert->Slide"
    InsertSlide
    WaitSlot (2000)
    hTypekeys "<Pagedown>"
    WaitSlot (2000) 'sleep 2
    printlog "close application"
    Call  hCloseDocument
    
endcase 'tiInsertSlide
'--------------------------------------------------------
testcase tiInsertDuplicateSlide

    printlog "open application"
    Call hNewDocument
    printlog "create rectangle"
    Call hRechteckErstellen ( 30, 40, 40, 50 )
    printlog "Insert->Duplicate Slide"
    InsertDuplicateSlide
    WaitSlot (2000)
    printlog "close application"
    Call  hCloseDocument
    
endcase 'tiInsertDuplicateSlide
'--------------------------------------------------------
testcase tiInsertField

    printlog "open application"
    Call hNewDocument

    printlog "Insert->Fields->Time (fixed)"
    InsertFieldsTimeFix
    WaitSlot (1000)
    printlog "OK   Time Fix"
    printlog "delete it with EditSelectAll + <Delete> "
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    printlog "Insert->Fields->Date (fixed) "
    InsertFieldsDateFix
    WaitSlot (1000)
    printlog "OK   Date Fix"
    printlog "delete it with EditSelectAll + <Delete> "
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    printlog "Insert->Fields->Time (variable) "
    InsertFieldsTimeVariable
    WaitSlot (1000)
    printlog "OK   Time Variabel"
    printlog "delete it with EditSelectAll + <Delete> "
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    printlog "Insert->Fields->Date (variable) "
    InsertFieldsDateVariable
    WaitSlot (1000)
    printlog "OK   Date Variabel"
    printlog "delete it with EditSelectAll + <Delete> "
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    printlog "Insert->Fields->Author "
    InsertFieldsAuthorDraw
    WaitSlot (1000)
    printlog "OK   Author"
    printlog "delete it with EditSelectAll + <Delete> "
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    printlog "Insert->Fields->Page Numbers"
    InsertFieldsPageNumberDraw
    WaitSlot (1000)
    printlog "OK   Page number"
    printlog "delete it with EditSelectAll + <Delete> "
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 1

    printlog "Insert->Fields->Filename "
    InsertFieldsFileName
    WaitSlot (1000) 'sleep 1
    printlog "OK   File name"
    printlog "delete it with EditSelectAll + <Delete> "
    EditSelectAll
    hTypekeys "<Delete>"
    sleep 2
    printlog "close application "
    Call  hCloseDocument
    
endcase 'tiInsertField
'--------------------------------------------------------
testcase tiInsertSpecialCharacter

    printlog "open application "
    Call hNewDocument

    printlog "insert text "
    hTextrahmenErstellen ("This is a testtext",30,40,60,50)
    sleep 2
    printlog "Insert->SpecialCharacter "
    InsertSpecialCharacterDraw
    WaitSlot (2000)
    Kontext "Sonderzeichen"
    Call DialogTest (Sonderzeichen)
    printlog "cancel dialog 'Special Characters' "
    Sonderzeichen.Cancel
    sleep 2
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertSpecialCharacter
'--------------------------------------------------------
testcase tiInsertHyperlink

    printlog "open application "
    Call hNewDocument
    printlog "Insert->Hyperlink "
    InsertHyperlink

    kontext "HyperlinkDialog"
    if ( HyperlinkDialog.exists( 2 ) ) then

        Kontext "TabHyperlinkInternet"
        
        Auswahl.MouseDown 50, 5
        Auswahl.MouseUp 50, 5
        Auswahl.typekeys "<PAGEDOWN><PAGEUP>"
        Auswahl.typekeys "<TAB>"

        printlog "Workaround to get rid of a Focusing-problem..."
        NameText.Typekeys "alal <RETURN>"
        NameText.Typekeys "<MOD1 A><DELETE>"
        TabHyperlinkInternet.Typekeys "<TAB>", 6
        TabHyperlinkInternet.Typekeys "<LEFT>", 3
        printlog "End of workaround..."

        Internet.Check
        ZielUrl.Settext "http://www.nowhere.com"

        Uebernehmen.Click()

        kontext "HyperlinkDialog"
        HyperlinkDialog.Close()
        
    else
        warnlog "Failed to open hyperlink ??!!"
    end if
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertHyperlink
'--------------------------------------------------------
testcase tiInsertGraphic

    printlog "open application "
    Call  hNewDocument
    printlog "Insert->Graphics "
    InsertGraphicsFromFile
    WaitSlot (2000) '
    try
        Kontext "GrafikEinfuegenDlg"
        printlog "if checkbox 'link' is available, check it; if not available: ERROR "
        if Link.exists then
            Link.Check
            else
            Warnlog  "Linking grafik doesn't work :-("
        end if
        printlog "if checkbox 'Preview' is available, check it; if not available: ERROR "
        if Preview.exists then
            Preview.Check
        else
            Warnlog "Preview of graphic doesn't work :-("
        end if
        DialogTest (GrafikEinfuegenDlg)
        printlog " use file: global\input\graf_inp\stabler.tif) "
        Dateiname.settext Convertpath (gTesttoolPath + "global\input\graf_inp\stabler.tif")
        printlog "click 'open' "
        Oeffnen.click
    catch
        Warnlog "Insert graphic doesn't work :-("
    endcatch
    WaitSlot (2000)
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertGraphic
'--------------------------------------------------------
testcase tiInsertObjectSound

    goto endsub ' disabled for final, because always wrong (TZ 01/2002)
    'TODO: TBO: enhance!
    printlog "open application "
    Call hNewDocument
    try
        printlog " Insert->Object->Sound "
        InsertObjectSound
        WaitSlot (1000)
        Kontext "OeffnenDlg"
        'Call Dialogtest (OeffnenDlg) ' just be sure to check one pth and one open dialog : TZ 28.11.201
        printlog "cancel file open dialog "
        OeffnenDlg.Cancel
    catch
        printlog "'Insert -> Object -> Sound' not available. TestDevelopmentInProgress (TDIP) ;-)"
    endcatch
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertObjectSound
'--------------------------------------------------------
testcase tiInsertObjectVideo

    goto endsub
    'TODO: TBO: enhance!
    printlog " open application "
    Call hNewDocument
    try
        printlog "Insert->Object->Video "
        InsertObjectVideo
        Kontext "OeffnenDlg"
        'Call Dialogtest (OeffnenDlg)
        WaitSlot (1000)
        printlog "cancel file open dialog "
        OeffnenDlg.Cancel
    catch
        printlog "'Insert -> Object -> Video' not available. (TDIP) ;-)"
    endcatch
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertObjectVideo
'--------------------------------------------------------
testcase tiInsertChart

    printlog "open application "
    Call hNewDocument
    printlog "Insert->Chart "
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
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertChart
'--------------------------------------------------------
testcase tiInsertObjectOLEObjects

    printlog "open application "
    hNewDocument
    printlog "Insert->Object->OLEObjekts "
    InsertObjectOLEObject
    Kontext "OLEObjektEinfuegen"
    'Call Dialogtest ( OLEObjektEinfuegen )
    'NeuErstellen.Check ' is default value
    Call DialogTest (OLEObjektEinfuegen, 1)
    printlog "check 'Create from file' "
    AusDateiErstellen.Check
    Call DialogTest (OLEObjektEinfuegen, 2)
    printlog "click 'Search...' "
    Durchsuchen.click
    Kontext "OeffnenDlG"
    printlog "cancel file open dialog "
    OeffnenDLG.Cancel
    Kontext "OLEObjektEinfuegen"
    printlog "cancel dialog 'insert OLE Object' "
    OLEObjektEinfuegen.Cancel
    sleep 1
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertObjectOLEObjects
'--------------------------------------------------------
testcase tiInsertSpreadsheet

    if gtSYSName = "Linux" then
        printlog "Linux = wont test tiInsertSpreadsheet"
        goto endsub
    endif

    printlog "open application "
    Call hNewDocument
    WaitSlot (2000)
    printlog "Insert->Spreadsheet "
    InsertSpreadsheetDraw
    WaitSlot (2000)
    Kontext "Messagebox"
    if Messagebox.Exists (5) then
        Warnlog Messagebox.GetText
        Messagebox.OK
    end if
    sleep 2
    printlog "click somewhere out of the object, to deselect it "
    gMouseClick 1,1
    sleep 1
    printlog "select and delete object with keys: <Tab><Delete> "
    hTypekeys "<Tab><Delete>"
    sleep 2
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertSpreadsheet
'--------------------------------------------------------
testcase tiInsertFormula

    printlog "open application "
    Call hNewDocument
    printlog "Insert->Object->Formula "
    InsertObjectFormulaDraw
    WaitSlot (2000)
    Kontext "Messagebox"
    if Messagebox.Exists then
        Warnlog Messagebox.GetText
        Messagebox.OK
        sleep 1
    end if
    printlog "click somewhere out of the object, to deselect it "
    gMouseClick 1,1
    sleep 1
    printlog "select and delete object with keys: <Tab><Delete> "
    hTypekeys "<Tab><Delete>"
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertFormula
'--------------------------------------------------------
testcase tiInsertFloatingFrame

    printlog "open application "
    Call hNewDocument
    printlog "Insert->Floating Frame "
    InsertFloatingFrame
    WaitSlot (2000)
    Kontext "TabEigenschaften"
    Dialogtest (TabEigenschaften)
    printlog "click '...' "
    Oeffnen.Click
    Kontext "OeffnenDlg"
    sleep 1
    printlog "cancel file open dialog "
    OeffnenDlg.Cancel
    Kontext "TabEigenschaften"
    printlog "cancel dialog 'Floating Frame Properties' "
    TabEigenschaften.Cancel
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertFloatingFrame
'--------------------------------------------------------
testcase tiInsertFile

    printlog "open application "
    Call  hNewDocument
    WaitSlot (1000)
    printlog "Insert->File "
    InsertFileDraw
    WaitSlot (1000)
    Kontext "OeffnenDLG"
    'Call Dialogtest ( OeffnenDLG )
    printlog "cancel file open dialog "
    OeffnenDLG.Cancel
    printlog "close application "
    Call  hCloseDocument
    
endcase 'tiInsertFile
'--------------------------------------------------------
testcase tiInsertPlugin

    printlog "open application "
    call hNewDocument
    printlog "Insert->Object->PlugIn "
    InsertObjectPlugIn
    Kontext "PluginEinfuegen"
    if PluginEinfuegen.exists (5) then
        call Dialogtest (PluginEinfuegen)
        printlog "click 'browse' "
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
            printlog "cancel file open dialog "
            OeffnenDLG.Cancel
        end if
    Kontext "PluginEinfuegen"
    printlog "cancel dialog 'Insert Plug-in' "
    if PluginEinfuegen.exists (5) then PluginEinfuegen.Cancel
        else
        warnlog "Insert Plugin does not work :-("
    end if
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertPlugin
'--------------------------------------------------------
testcase tiInsertScan

    goto endsub
    printlog "DISABLED because not able to make automatic :-( "
    printlog "open application "
    Call hNewDocument
    printlog "Insert->Scan->Request "
    InsertScanRequest ' as long as there is no scanner available, nothing happens
    WaitSlot (1000)
    printlog " Insert->Scan->SelectSource "
    InsertScanSelectSource
    WaitSlot (1000)
    printlog "Not testable, not translatable, just callable, because of systemdialog :-("
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiInsertScan
'--------------------------------------------------------
testcase tiInsertSnappointLine

    printlog "open application "
    Call  hNewDocument
    printlog "DRAW  : Insert->Snap Point/Line  "
    printlog "+ IMPRESS: available via Kontext menu: Insert->Snap Point/Line  "
    InsertSnapPointLine
    Kontext "NeuesFangobjekt"
    DialogTest ( NeuesFangobjekt )
    printlog "Cancel dialog 'New Snap Object' "
    NeuesFangobjekt.Cancel
    sleep 2
    printlog "close application "
    Call  hCloseDocument
    
endcase 'tiInsertSnappointLine
'--------------------------------------------------------
testcase tdInsertLayer

    printlog "open application "
    Call  hNewDocument
    WaitSlot (1000)
    printlog "View->Layer "
    ViewLayer
    printlog "Insert->Layer "
    InsertLayer
    Kontext "EbeneEinfuegenDlg"
    DialogTest ( EbeneEinfuegenDlg )
    printlog "cancel dialog 'Insert Layer' "
    EbeneEinfuegenDlg.Cancel
    printlog "close application "
    Call  hCloseDocument
    
endcase 'tdInsertLayer

