'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_print.inc,v $
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
' #1 tFilePrint
'\*************************************************************************************

testcase tFilePrint
    Call hNewDocument

    printlog "Inserting testtext."
    hTextrahmenErstellen ("This is an automated print test with testtool for GRAPHICS ",90,90,80,10)
    hTextrahmenErstellen ("Version    : "+ gVersionsnummer + " / " + gLanguage,80,90,70,10)
    hTextrahmenErstellen ("Date / Time : "+  Date + " / " + Time,70,90,60,10)
    hTextrahmenErstellen ("Machine / User: " + gPCName + "   " + gUser,60,90,50,10)

    printlog "Opening print Dialog."
    FilePrint
    kontext
    if active.exists(2) then
        active.ok
        qaerrorlog "There is no printer available - please install one on your system!"
    endif
    sleep 2
    Kontext "DruckenDlg"


    ' Not yet active since this feature is not build into the master: i85355
    '   printlog "Checking options for impress..."
    '    if (gApplication = "IMPRESS") then
    '        if PrintContent.GetSelIndex <> 1 then
    '            warnlog "Not first entry selected!"
    '       else
    '            printlog " First entry selected."
    '        endif
    '
    '        if PrintContent.GetItemCount <> 4 then
    '            warnlog "Entry-number of PrintContent listbox is wrong!"
    '        else
    '            printlog "Entry-number of PrintContent listbox is 4."
    '        endif
    '
    '        PrintContent.Select 2
    '        if SlidesPerPage.IsEnabled then
    '            printlog "SlidesPerPage active."
    '        else
    '            warnlog "SlidesPerPage did not get active!"
    '        endif
    '
    '        VerticalOrder.Check
    '        printlog "Vertical checked."
    '        HorizontalOrder.Check
    '        printlog "Horizontal checked again."
    '
    '    else
    '        printlog "This is not impress, so no further print content testing."
    '    endif

    printlog "Opening Options dialog."
    Zusaetze.Click
    sleep 1
    Kontext "DruckerZusaetzeDraw"

    printlog "Checking brochure printing."
    if Prospekt.IsChecked then
        warnlog "Somebody forgot to uncheck the prospect printing!"
        Standard.Check
    else
        printlog "Brochure printing is checked."
    endif


    Seitenname.Check
    Datum.Check
    Zeit.Check
    AusgeblendeteSeiten.Check
    SeitengroesseAnpassen.Check
    AusDruckereinstellung.Check

    DruckerZusaetzeDraw.OK
    Kontext "DruckenDlg"
    Zusaetze.Click
    sleep 2
    Kontext "DruckerZusaetzeDraw"
    if NOT Seitenname.IsChecked Then
        warnlog "  - Page name not checked"
    else
        printlog "Page name checked."
    endif

    if NOT Datum.IsChecked Then
        warnlog "  - Date not checked"
    else
        printlog "Date is checked."
    endif

    if NOT Zeit.IsChecked Then
        warnlog "  - Time not checked"
    else
        printlog "Time is checked."
    endif

    if NOT AusgeblendeteSeiten.IsChecked Then
        warnlog "  - Hidden pages not checked"
    else
        printlog "Hidden pages checked."
    endif

    if NOT SeitengroesseAnpassen.IsChecked Then
        warnlog "  - Fit to page not checked"
    else
        printlog "Fit to page checked."
    endif

    if NOT AusDruckereinstellung.IsChecked Then
        warnlog "  - Paper tray from printer settings not checked"
    else
        printlog "Paper tray checked."
    endif

    AusDruckereinstellung.UnCheck
    Standard.Check
    DruckerZusaetzeDraw.OK
    Kontext "DruckenDlg"
    DruckenDlg.Cancel
    Sleep 5

    Call hCloseDocument
endcase

