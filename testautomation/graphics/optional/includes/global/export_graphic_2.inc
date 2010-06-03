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
'* Owner : wolfram.garten@sun.com
'*
'* short description : Graphics Export B-tests. (the usual suspects)
'*
'\******************************************************************************
testcase tPNG

    dim x as integer
    dim i as integer
    dim iWaitIndex as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "PNG - Portable Network Graphic (.png)"
    sExt = ".png"

    hFileOpen( gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString )

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "PNGOptionen"
        if PNGOptionen.Exists (2) then
            Kompression.ToMax
            Interlaced.UnCheck
            PNGOptionen.OK
            iWaitIndex = 0
            do while PNGOptionen.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif

    call hCloseDocument
endcase 'tPNG

'-------------------------------------------------------------------------------
testcase tSVM

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string
    dim bTemp as boolean
    dim sX as string
    dim sY as string
    dim sx1 as string
    dim sX2 as string
    dim sY2 as string
    dim sDocument as string

    sFilter = "SVM - StarView Metafile (.svm)"
    sExt = ".svm"

    sDocument = ConvertPath ( gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString)
    hFileOpen sDocument

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Original.Check
            if Breite.IsEnabled then
                warnlog " :-("
            endif
            if Hoehe.IsEnabled then
                warnlog " :-("
            endif
            Groesse.Check
            Breite.More
            Hoehe.Less
            SVMOptionen.Cancel
            sleep 5
            if ( dir(OutputGrafikTBO+sExt) = "") then ' inspired by bug #99932 Graphic is exported though cancel is pressed
                Printlog "ok :-)"
            else
                warnlog "dialog was canceled, but file got saved, too :-(  - i35177"
            endif
        else
            Warnlog "No  '" & sFilter & "' -Dialog!"
            i=5
        end if
        sleep 2
        Kontext "Active"
        if Active.Exists(2) then
            Warnlog " '" & sFilter & "'  has a problem"
            Active.OK
        end if
    end if
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Groesse.Check
            Breite.Less
            Hoehe.More
            SVMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif
    hCloseDocument ()
    hFileOpen (sDocument)
    sleep (10)

    call fMakeDocumentWritable

    hTypeKeys ("<escape><tab>")
    fGetSizeXY sx1, sY, TRUE
    if hCallExport (OutputGrafikTBO & "1" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Original.Check
            SVMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO & "1" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "1" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO & "1" & sExt )
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    hCloseDocument ()
    hNewDocument()
    hRechteckErstellen ( 10, 10, 30, 40 )
    if hCallExport (OutputGrafikTBO & "2" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Groesse.Check
            Breite.SetText "9"
            Hoehe.SetText "9"
            Groesse.Check 'Check to make settings go throught
            sx1 = Breite.GetText
            sY = Hoehe.GetText
            SVMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO & "2" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "2" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hFileOpen (OutputGrafikTBO & "2" & sExt)
        kontext "Filterauswahl"
        if Filterauswahl.exists then
            Warnlog "Error when loading the file. The Filter-dialogue came up. Test aborted."
            Filterauswahl.Cancel
            goto endsub
        endif
        kontext "DocumentDraw"
        DocumentDraw.TypeKeys ("<escape><tab>")
        ContextOriginalSize
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    if hCallExport (OutputGrafikTBO & "3" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Groesse.Check
            sX2 = Breite.GetText
            if (LiberalMeasurement(sx1, sX2)) <> TRUE then
                if (val(str(StrToDouble(sx1)+5)) >= StrToDouble(sX2) ) AND (val(str(StrToDouble ( sx1 )-5)) <= StrToDouble ( sX2 )) then
                    Printlog "Width was ok. Expected: " & sx1 & "' was: '" & sX2 & "'"
                else
                    warnLog "Width is different expected: '" & sx1 & "' is: '" & sX2 & "'"
                endif
            endif
            sY2 = Hoehe.GetText
            if (LiberalMeasurement(sY, sY2)) <> TRUE then
                if ( val(str(StrToDouble(sY)+5)) >= StrToDouble(sY2) ) AND (val(str(StrToDouble ( sY )-5)) <= StrToDouble ( sY2 )) then
                    Printlog "Height was ok. Expected: " & sY & "' was: '" & sY2 & "'"
                else
                    warnLog "Height is different expected: '" & sY & "' is: '" & sY2 & "'"
                endif
            endif
            SVMOptionen.Cancel
            sleep 5
        endif
    endif

    call hCloseDocument
    
endcase 'tSVM
'-------------------------------------------------------------------------------
testcase tPDF

    dim sPath as string
    dim sTemp as string
    dim iFileSize(12) as long
    dim i as integer

    '/// Export the file 12 times: ///'
    '///+ every compression level (3) and every range (3), check if button in toolbar uses last settings (6) (-> 12) ///'
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport_pdf."+ExtensionString ))
    sleep (10)

    '---------------------------------------------------------------------------------------------------------------------------------------------------------
    '###### all / Screen optimized ######
    '/// File->Export as PDF document
    printlog "File->Export as PDF document"
    '///+ range = All
    printlog " range = All"
    '///+ compression = Screen optimized
    printlog " compression = Screen optimized"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '---------------------------------------------------------   Output the first file ---------------------------------------------------------------

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 1, TRUE, FALSE, 1, 1)
    '/// Export using Button in toolbar
    printlog "Export using Button in toolbar"
    '///+ use the settings from the export before
    printlog " use the settings from the export before"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '-------------------------------------------------------   Output the second file -------------------------------------------------------------

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 2, TRUE, FALSE, 1, 1) ' button in toolbar -> last settings have to be used

    '###### range / Screen optimized ######
    '/// File->Export as PDF document
    printlog "File->Export as PDF document"
    '///+ range = Range
    printlog " range = Range"
    '///+ compression = Screen optimized
    printlog " compression = Screen optimized"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '---------------------------------------------------------  Output the third file  ---------------------------------------------------------------

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 3, TRUE, FALSE, 2, 1, "2")

    '/// Export using Button in toolbar
    printlog "Export using Button in toolbar"
    '///+ use the settings from the export before
    printlog " use the settings from the export before"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '---------------------------------------------------------  Output the fourth file  -------------------------------------------------------------

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 4, TRUE, FALSE, 2, 1) ' button in toolbar -> last settings have to be used

    '###### all / Print optimized ######
    '/// File->Export as PDF document
    printlog "File->Export as PDF document"
    '///+ range = all
    printlog " range = all"
    '///+ compression = Print optimized
    printlog " compression = Print optimized"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '------------------------------------------------------------  Output the fifth file  --------------------------------------------------------------

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 5, TRUE, FALSE, 1, 2)

    '/// Export using Button in toolbar
    printlog "Export using Button in toolbar"
    '///+ use the settings from the export before
    printlog " use the settings from the export before"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '-----------------------------------------------------------  Output the sixth file  ---------------------------------------------------------------

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 6, TRUE, FALSE, 1, 2) ' button in toolbar -> last settings have to be used

    '###### range / Print optimized ######
    '/// File->Export as PDF document
    printlog "File->Export as PDF document"
    '///+ range = Range
    printlog " range = Range"
    '///+ compression = Screen optimized
    printlog " compression = Screen optimized"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '---------------------------------------------------------  Output the seventh file  -------------------------------------------------------------

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 7, TRUE, FALSE, 2, 2, "2")

    '/// Export using Button in toolbar
    printlog "Export using Button in toolbar"
'    '///+ use the settings from the export before
    printlog " use the settings from the export before"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '------------------------------------------------------------  Output the eight file  ----------------------------------------------------------------

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 8, TRUE, FALSE, 2, 2) ' button in toolbar -> last settings have to be used

    '###### all / Press optimized ######
    '/// File->Export as PDF document ///'
    printlog "File->Export as PDF document ///'"
    '///+ range = all
    printlog " range = all"
    '///+ compression = Press optimized
    printlog " compression = Press optimized"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '------------------------------------------------------------  Output the ninth file  ----------------------------------------------------------------

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 9, TRUE, FALSE, 1, 3)
    '/// Export using Button in toolbar
    '///+ use the settings from the export before

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '------------------------------------------------------------  Output the tenth file  ----------------------------------------------------------------

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 10, TRUE, FALSE, 1, 3) ' button in toolbar -> last settings have to be used

    '###### range / Press optimized ######
    '/// File->Export as PDF document ///'
    printlog "File->Export as PDF document ///'"
    '///+ range = all
    printlog " range = all"
    '///+ compression = Press optimized
    printlog " compression = Press optimized"

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '-----------------------------------------------------------  Output the eleventh file  -------------------------------------------------------------

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 11, TRUE, FALSE, 2, 3, "2")
    '/// Export using Button in toolbar
    '///+ use the settings from the export before

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '-------------------------------------------------------------  Output the twelfth file  --------------------------------------------------------------

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 12, TRUE, FALSE, 2, 3) ' button in toolbar -> last settings have to be used

    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '-----------------------------------------------------------------------------------------------------------------------------------------------------------------

    for i=1 to 12
        sPath = OutputGrafikTBO
        sPath = sPath + i + ".pdf"
        iFileSize(i) = app.FileLen(sPath)
        Printlog (" - " + i + ": " + " " + iFileSize(i) + " " + sPath
        if (dir(sPath) <> "") then
            sleep(1)
            '/// the first line has to be '%PDF-1.4' ///'
            sTemp = fGetFileText(sPath, 8)
            if (sTemp <> "%PDF-1.4") then
                Warnlog "File doesn't start with '%PDF-1.4' it is: '" + sTemp + "'"
            endif
            '/// the last line has to be '%%EOF' ///'
            sTemp = left(fGetFileText(sPath, -6),5)
            if (sTemp <> "%%EOF") then
                Warnlog "File doesn't end with '%%EOF' it is: '" + sTemp + "'"
            endif
        else
            Warnlog "File was not exported to PDF :-( '" + sPath + "'"
        endif
    next i

    ' Check if the file size is the same when exporting via toolbar and file/export
    ' if the size different then the options are not used when exporting via the toolbar
    if (iFileSize(1) <> iFileSize(2)) then
        qaerrorlog "i100919: pdf export: different file size when exporting with file/export or button"
        'Warnlog ("Diffrence in filesize for file 1 and 2")
    endif
    ' print optimized
    if (iFileSize(5) <> iFileSize(6)) then
        qaerrorlog "i100919: pdf export: different file size when exporting with file/export or button"
        'Warnlog ("Difference in filesize for file 5 and 6")
    endif
    ' press optimized
    if (iFileSize(9) <> iFileSize(10)) then
        qaerrorlog "i100919: pdf export: different file size when exporting with file/export or button"
        'Warnlog ("Difference in filesize for file 9 and 10")
    endif

    

    if (iFileSize(1) > iFileSize(5)) then
    qaerrorlog "i100919: pdf export: different file size when exporting with file/export or button"
        'Warnlog ("Filesize for file 1 is greater then for file 5. The compression level doesn't work.")
    endif
    if (iFileSize(5) > iFileSize(9)) then
        Warnlog ("Filesize for file 5 is greater then for file 9. The compression level doesn't work.")
    endif

    call hCloseDocument()
endcase 'tPDF

'-------------------------------------------------------------------------------
testcase tPDF_Creator

    dim sPath as String
    dim sApp as String
    dim sTemp as String
    dim bTemp as String
    dim sTemp2 as String
    dim iFileSize(3) as long
    dim iCreatorOffset as integer
    dim i as integer
    dim sTextFilter as String
    dim sTextFiltername as String
    dim sExtension() as String

    sTemp = ConvertPath ( gTesttoolPath + "graphics\required\input\export_graphic3." + ExtensionString )
    printlog "sTemp=" & sTemp
    printlog "File load"
    hFileOpen (sTemp)
    sleep (30)
    sPath = OutputGrafikTBO
    sTemp = sPath
    sPath = sPath + "L.pdf"
    printlog "sPath : " & sPath
    sleep(1)
    printlog "Exporting File as pdf but with txt ending.."
    hExportAsPDFmulti (1, TRUE, sPath, TRUE, FALSE, 1, 1)
    name sPath as sTemp+"L.txt"
    sleep (30)
    iFileSize(3) = app.FileLen(sTemp+"L.txt")
    iCreatorOffset = 0
    sleep 5
    printlog "load file in writer as text"
    printlog "Load File.."
    hFileOpen(sTemp+"L.txt")

    printlog "If dialog 'AsciiFilterOptionen' comes up, say OK"
    printlog "Checking for FilterDialog.."
    Kontext "AsciiFilterOptionen"
    If AsciiFilterOptionen.Exists(3) then
        AsciiFilterOptionen.OK
    endif

    printlog "Selecting Filter.."
    Kontext "Filterauswahl"
    If Filterauswahl.Exists then
        Filter.Select ( hGetUIFiltername("Text" ))
        Filterauswahl.OK
    End If

    sApp = gApplication
    gApplication = "WRITER"
    sleep 5 
    printlog "Search in Doc for <</Creator<"
    bTemp = hFindeImDokument ("<</Creator<",true,true)
    if (bTemp = TRUE) then
        Printlog "OK, found."
    else
        warnlog " /Creator not found in pdf file :-("
    endif
    printlog "getting Clipboard Text.."
    hTypeKeys("<left><right><shift end>")
    editcopy
    sTemp = getClipboardText()
    printlog "sTemp from clipboard: " & sTemp
    sTemp = left(sTemp, inStr(sTemp, ">")-1)
    printlog "shortened sTemp: " & sTemp
    sTemp2 = ""
    for i = 0 to (len(sTemp)/4)-1
        sTemp2 = sTemp2 + chr("&H"+mid(sTemp,(i*4)+1, 4))
    next i
    printlog "sTemp2: " & sTemp2
    printlog sTemp + " -- " + len(sTemp) + " ::: '" + sTemp2 + "'"
    if (lCase(sapp) <> lCase(right(sTemp2, len(stemp2)-1))) then
        warnlog "Application is different from Creator; is: '" + lCase(sTemp2) + "' should: '" + lCase(sapp) + "'"
    endif
    printlog "iCreatorOffset: " & iCreatorOffset
    iCreatorOffset = iCreatorOffset + len(sTemp)
    printlog "iCreatorOffset + len(sTemp): " & iCreatorOffset
    printlog "Search for Producer.."
    bTemp = hFindeImDokument ("/Producer<")
    if (bTemp = TRUE) then
        Printlog "OK, found."
    else
        warnlog " /Producer not found in pdf file :-("
    endif
    printlog "getting clipboard content.."
    hTypeKeys("<left><right><shift end><shift down>")
    editcopy
    sTemp = getClipboardText()
    printlog "sTemp from clipboard: " & sTemp
    sTemp = left(sTemp, inStr(sTemp, ">")-1)
    printlog "shortened sTemp: " & sTemp

    sTemp2 = ""
    printlog "len(sTemp): " & ((len(sTemp)/4)-1)
    for i = 0 to (len(sTemp)/4)-1
        sTemp2 = sTemp2 + chr("&H"+mid(sTemp,(i*4)+1, 4))
        printlog "sTemp2: " & sTemp2
    next i
    printlog sTemp + " -- " + len(sTemp) + " ::: '" + sTemp2 + "'"
    if gOOO then
        iCreatorOffset = iCreatorOffset + len(sTemp)
        printlog "In case of OOO iCreatorOffset is: " & iCreatorOffset
    else
        if bAsianLan then
            iCreatorOffset = iCreatorOffset + 48
            if (len(sTemp) <> 48) then
                warnlog "Producer is different from 'StarSuite 8'; is: '" + sTemp2 + "'"
            endif
        else
    printlog "iCreatorOffset:" & iCreatorOffset
            iCreatorOffset = iCreatorOffset + 52
            printlog "iCreatorOffset + 52:" & iCreatorOffset
            if (len(sTemp) <> 92) then
                warnlog "Producer is different from ''; is: '" + sTemp2 + "'"
            endif
        endif
    endif
    
    gApplication = sApp
    call hCloseDocument
    call hCloseDocument

endcase 'tPDF_Creator
'-------------------------------------------------------------------------------
testcase tSVG

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string
    dim bTemp as boolean
    dim sTemp as string
    dim sTextFilter as string
    dim sExtension() as string
    dim sTextFiltername as string

    sFilter = "SVG - Scalable Vector Graphics (.svg)"
    sExt = ".svg"

    hFileOpen ( gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString )

    '------------------------\
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        '------------------------\
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
            '------------------------\
            if (iSprache <> 82) then
                hCloseDocument ()
                sleep 5
                sTextFilter = "Text"

                hFileOpenWithFilter (OutputGrafikTBO+sExt, sTextFiltername, FALSE)
                printlog "if dialog 'AsciiFilterOptionen' comes up, say OK"
                Kontext "AsciiFilterOptionen"
                '-------------------------------\
                If AsciiFilterOptionen.Exists(3) then
                    AsciiFilterOptionen.OK
                else
                    warnlog "No ASCII filter Options Dialog?"
                endif
                '-------------------------------/
                sTemp = gApplication
                gApplication = "WRITER"
                sleep 5 ' wait for document to be loaded
                bTemp = hFindeImDokument ("<!DOCTYPE svg PUBLIC " & chr(34)& "-//W3C//DTD SVG 1.1//EN" & chr(34)&" " & chr(34)&"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd" & chr(34)&">",true)
                '-------------------------------\
                if (bTemp = TRUE) then
                    Printlog "OK"
                else
                    warnlog "DOCTYPE not found in svg file :-("
                endif
                '-------------------------------/
                gApplication = sTemp
            else
                qaerrorlog "disabled for 82: #111017#"
            endif
            '--------------------------/
        else
            warnlog "File didn't get saved :-("
        endif
        '------------------------/
    endif
    '------------------------/

    call hCloseDocument
endcase 'tSVG

'-------------------------------------------------------------------------------
testcase tBMP
qaerrorlog
   dim x as integer
   dim i as integer
   dim bTemp as boolean
   dim sX as string
   dim sY as string
   dim sx1 as string
   dim sX2 as string
   dim sY2 as string
   dim sDocument as string

    '/// open the document
    sDocument = ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString)
    hFileOpen sDocument

   if hCallExport (OutputGrafikTBO , "BMP - Windows Bitmap (.bmp)" ) = TRUE then
      Kontext "BMPOptionen"
      if BMPOptionen.Exists (2) then
      '/// check if all properties have the right count, and depend on each other ///'
         '/// 'Color Resolution' listbox contains eight items ///'
         x = Farbaufloesung.GetItemCount
         if x <> 8 then warnlog "Color Resolution Count is wrong; should be:8, is:" + x
         for i = 1 to x
            Farbaufloesung.Select i
            sleep 1
            Printlog " - " + i + ": '" +Farbaufloesung.GetSelText + "'"
            '/// checkbox RLE coding has to be enabled only for '4 and 8 bit' color palettes ///'
            if ((i > 3) AND (i < 8)) then
               if RLEKodierung.IsEnabled <> TRUE then warnlog "'RLE coding' is not checkable :-("
            else
               if RLEKodierung.IsEnabled <> FALSE then warnlog "'RLE coding' is checkable :-("
            endif
         next i
         '/// if Mode 'original' is selected, DPI///'
         Original.Check
            if DPI.IsEnabled then warnlog " :-("
            if Breite.IsEnabled then warnlog " :-("
            if Hoehe.IsEnabled then warnlog " :-("
         Aufloesung.Check
            x = DPI.GetItemCount
            '/// 'DPI' listbox contains eight items ///'
            if x <> 4 then warnlog "'DPI' Count is wrong; should be:4, is:" + x
            for i = 1 to x
               DPI.Select i
               Printlog " - " + i + ": '" +DPI.GetSelText + "'"
            next i
            if Breite.IsEnabled then warnlog " :-("
            if Hoehe.IsEnabled then warnlog " :-("
         Groesse.Check
            if DPI.IsEnabled then warnlog " :-("
            Breite.More
            Hoehe.Less
         Sleep 1
         '/// leave dialog with cancel -> there has to be no file created! ///'
         BMPOptionen.Cancel
         sleep 5
         if ( dir(OutputGrafikTBO+".bmp") = "") then ' inspired by bug #99932 Graphic is exported though cancel is pressed
            Printlog "ok :-)"
         else
            warnlog "Dialog was canceled, but file got saved, too :-(  - i35177"
         endif
      else
         Warnlog "No BMP-Option-Dialog!"
         i=5
      end if
      sleep 2
      Kontext "Active"
      if Active.Exists(2) then
         Warnlog "BMP-Exportfilter has a problem"
         Active.OK
      end if
   end if
   Printlog "'/// now save it realy and load the file afterwards ///'"
   if hCallExport (OutputGrafikTBO , "BMP - Windows Bitmap (.bmp)" ) = TRUE then
      Kontext "BMPOptionen"
      if BMPOptionen.Exists (2) then
         Farbaufloesung.Select 7
         RLEKodierung.Check
         Aufloesung.Check
         DPI.Select 3
         BMPOptionen.OK
         sleep 5
      endif
      if ( dir(OutputGrafikTBO+".bmp") <> "") then
         Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+".bmp" + "'"
      else
         warnlog "File didn't get saved :-("
      endif
      hCloseDocument ()
      sleep 5
      hNewDocument()
      sleep 5
      Call hGrafikEinfuegen ( OutputGrafikTBO+".bmp" )
   endif
   hCloseDocument ()
   Printlog "'/// now save a SELECTION in ORIGINAL SIZE and load the file afterwards ///'"
   
   hFileOpen (sDocument)
   kontext "Filterauswahl"
   if Filterauswahl.exists(10) then
      Warnlog "Error when loading the file. The Filter-dialogue came up. Test aborted."
      Filterauswahl.Cancel
      goto endsub
   endif

   ' check if the document is writable
   call fMakeDocumentWritable
    
   hTypeKeys ("<escape><tab>")
   fGetSizeXY sx1, sy, TRUE
   if hCallExport (OutputGrafikTBO + "1" , "BMP - Windows Bitmap (.bmp)", TRUE ) = TRUE then
      Kontext "BMPOptionen"
      if BMPOptionen.Exists (2) then
         Original.Check
         BMPOptionen.OK
         sleep 5
      endif
      if ( dir(OutputGrafikTBO + "1"+".bmp") <> "") then
         Printlog "Ok :-) Saved as: '" + OutputGrafikTBO + "1"+".bmp" + "'"
      else
         warnlog "File didn't get saved :-("
      endif
    hCloseDocument ()
      sleep 5
      hNewDocument()
      sleep 5
      Call hGrafikEinfuegen ( OutputGrafikTBO + "1"+".bmp" )
      bTemp = FALSE
      fGetSizeXY sx1, sY, bTemp
      if (bTemp = FALSE) then
         warnlog "Selected original size NOT OK :-("
      endif
   endif
   hCloseDocument ()
   
   Printlog "'/// now CREATE a rectangle, select it, save it in SIZE and load the file afterwards ///'"
   hNewDocument()
   hTypeKeys "<TAB>"
   gMouseClick 50, 50
   hRechteckErstellen ( 10, 10, 30, 40 )
   if hCallExport (OutputGrafikTBO + "2" , "BMP - Windows Bitmap (.bmp)", TRUE ) = TRUE then
      Kontext "BMPOptionen"
      if BMPOptionen.Exists (2) then
         Groesse.Check
            Breite.SetText "9"
            Hoehe.SetText "9"
            Groesse.Check  'Press "Size" one more time in order to make the change go through"
            sx1 = Breite.GetText
            sY = Hoehe.GetText
         BMPOptionen.OK
         sleep 5
      endif
      if ( dir(OutputGrafikTBO + "2"+".bmp") <> "") then
         Printlog "Ok :-) Saved as: '" + OutputGrafikTBO + "2"+".bmp" + "'"
      else
         warnlog "File didn't get saved :-("
      endif
      hCloseDocument ()
      sleep 5
      hFileOpen (OutputGrafikTBO + "2"+".bmp")
      kontext "Filterauswahl"
      if Filterauswahl.exists then
         Warnlog "Error when loading the file. The Filter-dialogue came up. Test aborted."
         Filterauswahl.Cancel
         goto endsub
      endif
      kontext "DocumentDraw"
      DocumentDraw.TypeKeys ("<escape><tab>")
      
      ContextOriginalSize
      bTemp = FALSE
      fGetSizeXY sx1, sY, bTemp
      if (bTemp = FALSE) then
         warnlog "Selected original size NOT OK :-("
      endif
      
   endif
   if hCallExport (OutputGrafikTBO + "3" , "BMP - Windows Bitmap (.bmp)", TRUE ) = TRUE then
      Kontext "BMPOptionen"
      if BMPOptionen.Exists (2) then
         Groesse.Check
            sX2 = Breite.GetText
            if (LiberalMeasurement(sx1, sX2)) <> TRUE then
               if (val(str(StrToDouble(sx1)+5)) >= StrToDouble(sX2) ) AND (val(str(StrToDouble ( sx1 )-5)) <= StrToDouble ( sX2 )) then
                  Printlog "Width was ok. Expected: " + sx1 + "' was: '" + sX2 + "'"
               else
                  warnLog "Width is different expected: '" + sx1 + "' is: '" + sX2 + "'"
               endif
            endif
            sY2 = Hoehe.GetText
            if (LiberalMeasurement(sY, sY2)) <> TRUE then
               if ( val(str(StrToDouble(sY)+5)) >= StrToDouble(sY2) ) AND (val(str(StrToDouble ( sY )-5)) <= StrToDouble ( sY2 )) then
                  Printlog "Height was ok. Expected: " + sY + "' was: '" + sY2 + "'"
               else
                  warnLog "Height is different expected: '" + sY + "' is: '" + sY2 + "'"
               endif
            endif
         BMPOptionen.Cancel
         sleep 5
       endif
   endif

   call hCloseDocument

endcase 'tBMP
'-------------------------------------------------------------------------------
testcase tEMF

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string
    dim bTemp as boolean
    dim sX as string
    dim sY as string
    dim sx1 as string
    dim sX2 as string
    dim sY2 as string
    dim sDocument as string

    sFilter = "EMF - Enhanced Metafile (.emf)"
    sExt = ".emf"

    sDocument = gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString
    hFileOpen sDocument

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Original.Check
            if Breite.IsEnabled then
                warnlog " :-("
            endif
            if Hoehe.IsEnabled then
                warnlog " :-("
            endif
            Groesse.Check
            Breite.More
            Hoehe.Less
            SVMOptionen.Cancel
            sleep 5
            if ( dir(OutputGrafikTBO+sExt) = "") then ' inspired by bug #99932 Graphic is exported though cancel is pressed
                Printlog "ok :-)"
            else
                warnlog "Dialog was canceled, but file got saved, too :-(  - i35177"
            endif
        else
            Warnlog "No  '" & sFilter & "' -Dialog!"
            i=5
        end if
        sleep 2
        Kontext "Active"
        if Active.Exists(2) then
            Warnlog " '" & sFilter & "'  has a problem"
            Active.OK
        end if
    end if
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Groesse.Check
            Breite.Less
            Hoehe.More
            SVMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) saved as: '" & OutputGrafikTBO+sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif
    hCloseDocument ()
    hFileOpen (sDocument)
    kontext "Filterauswahl"
    if Filterauswahl.Exists(10) then
        Warnlog "Error when loading the file. The Filter-dialogue came up. Test aborted."
        Filterauswahl.Cancel
        goto endsub
    endif
   printlog "making doc editable if it is readonly"   
    call fMakeDocumentWritable
    hTypeKeys ("<escape><tab>")
    fGetSizeXY sx1, sY, TRUE
    if hCallExport (OutputGrafikTBO & "1" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Original.Check
            SVMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO & "1" & sExt) <> "") then
            Printlog "Ok :-) saved as: '" & OutputGrafikTBO & "1" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO & "1" & sExt )
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    hCloseDocument ()
    hNewDocument()
    hRechteckErstellen ( 10, 10, 30, 40 )
    if hCallExport (OutputGrafikTBO & "2" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Groesse.Check
            Breite.SetText "9"
            Hoehe.SetText "9"
            Groesse.Check   'Press "Size" one more time to make the change go through"
            sx1 = Breite.GetText
            sY = Hoehe.GetText
            SVMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO & "2" & sExt) <> "") then
            Printlog "Ok :-) saved as: '" & OutputGrafikTBO & "2" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hFileOpen (OutputGrafikTBO & "2" & sExt)
        kontext "DocumentDraw"
        DocumentDraw.TypeKeys ("<escape><tab>")
        ContextOriginalSize
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif

        if hCallExport (OutputGrafikTBO & "3" , sFilter, TRUE ) = TRUE then
            Kontext "SVMOptionen"
            if SVMOptionen.Exists (2) then
                Groesse.Check
                sX2 = Breite.GetText
                if (LiberalMeasurement(sx1, sX2)) <> TRUE then
                    if (val(str(StrToDouble(sx1)+5)) >= StrToDouble(sX2) ) AND (val(str(StrToDouble ( sx1 )-5)) <= StrToDouble ( sX2 )) then
                        Printlog "Width was ok. Expected: " & sx1 & "' was: '" & sX2 & "'"
                    else
                        warnLog "Width is different expected: '" & sx1 & "' is: '" & sX2 & "'"
                    endif
                endif
                sY2 = Hoehe.GetText
                if (LiberalMeasurement(sY, sY2)) <> TRUE then
                    if ( val(str(StrToDouble(sY)+5)) >= StrToDouble(sY2) ) AND (val(str(StrToDouble ( sY )-5)) <= StrToDouble ( sY2 )) then
                        Printlog "Height was ok. Expected: " & sY & "' was: '" & sY2 & "'"
                    else
                        warnLog "Height is different expected: '" & sY & "' is: '" & sY2 & "'"
                    endif
                endif
                SVMOptionen.Cancel
                sleep 5
            endif
        endif
    endif

    call hCloseDocument
endcase 'tEMF

'-------------------------------------------------------------------------------
testcase tMET

    dim x as integer
    dim i as integer
    dim iWaitIndex as integer
    dim sFilter, sExt as string
    dim bTemp as boolean
    dim sX as string
    dim sY as string
    dim sx1 as string
    dim sX2 as string
    dim sY2 as string
    dim sTemp as string
    dim sDocument as string

    sFilter = "MET - OS/2 Metafile (.met)"
    sExt = ".met"

    sDocument = gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString
    hFileOpen sDocument

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "METOptionen"
        if METOptionen.Exists (2) then
            Original.Check
            if Breite.IsEnabled then
                warnlog " :-("
            endif
            if Hoehe.IsEnabled then
                warnlog " :-("
            endif
            Groesse.Check
            sTemp = Breite.getText
            Breite.More
            if (sTemp = Breite.getText) then
                qaErrorLog "Width didn't change on pressing button 'More' #112225#"
                Breite.setText "10"
                Hoehe.setText "10"
            endif
            sTemp = Hoehe.getText
            Hoehe.Less
            if (sTemp = Hoehe.getText) then
                warnlog "Height didn't change on pressing button 'Less'"
            endif
            METOptionen.Cancel
            sleep 5
            if ( dir(OutputGrafikTBO+sExt) = "") then ' inspired by bug #99932 Graphic is exported though cancel is pressed
                Printlog "ok :-)"
            else
                warnlog "Dialog was canceled, but file got saved, too :-(  - i35177"
            endif
        else
            Warnlog "No  '" & sFilter & "' -Dialog!"
            i=5
        end if
        sleep 2
        Kontext "Active"
        if Active.Exists(2) then
            Warnlog " '" & sFilter & "'  has a problem"
            Active.OK
        end if
    end if
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "METOptionen"
        if METOptionen.Exists (2) then
            Groesse.Check
            sTemp = Breite.getText
            Breite.Less
            if (sTemp = Breite.getText) then
                warnlog "Width didn't change on pressing button 'Less'"
            endif
            sTemp = Hoehe.getText
            Hoehe.More
            if (sTemp = Hoehe.getText) then
                qaErrorLog "Height didn't change on pressing button 'More' #112225#"
                Breite.setText "10"
                Hoehe.setText "10"
            endif
            METOptionen.OK
            iWaitIndex = 0
            do while METOptionen.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif
    hCloseDocument ()
    hFileOpen (sDocument)
    sleep (10)
   
    printlog "making doc editable if it is readonly"
    call fMakeDocumentWritable

    hTypeKeys ("<escape><tab>")
    fGetSizeXY sx1, sY, TRUE
    if hCallExport (OutputGrafikTBO & "1" , sFilter, TRUE ) = TRUE then
        Kontext "METOptionen"
        if METOptionen.Exists (2) then
            Original.Check
            METOptionen.OK
            iWaitIndex = 0
            do while METOptionen.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO & "1" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "1" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO & "1" & sExt )
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    hCloseDocument ()
    hNewDocument()
    hRechteckErstellen ( 10, 10, 30, 40 )
    if hCallExport (OutputGrafikTBO & "2" , sFilter, TRUE ) = TRUE then
        Kontext "METOptionen"
        if METOptionen.Exists (2) then
            Groesse.Check
            Breite.SetText "9"
            Hoehe.SetText "9"
            Groesse.Check  ' Click "Size" one more time to make the changes go through.
            sx1 = Breite.GetText
            sY = Hoehe.GetText
            METOptionen.OK
            iWaitIndex = 0
            do while METOptionen.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO & "2" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "2" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hFileOpen (OutputGrafikTBO & "2" & sExt)
        kontext "DocumentDraw"
        DocumentDraw.TypeKeys ("<escape><tab>")
        ContextOriginalSize
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    if hCallExport (OutputGrafikTBO & "3" , sFilter, TRUE ) = TRUE then
        Kontext "METOptionen"
        if METOptionen.Exists (2) then
            Groesse.Check
            sX2 = Breite.GetText
            if (LiberalMeasurement(sx1, sX2)) <> TRUE then
                if (val(str(StrToDouble(sx1)+5)) >= StrToDouble(sX2) ) AND (val(str(StrToDouble ( sx1 )-5)) <= StrToDouble ( sX2 )) then
                    Printlog "Width was ok. Expected: " & sx1 & "' was: '" & sX2 & "'"
                else
                    warnLog "Width is different expected: '" & sx1 & "' is: '" & sX2 & "'"
                endif
            endif
            sY2 = Hoehe.GetText
            if (LiberalMeasurement(sY, sY2)) <> TRUE then
                if ( val(str(StrToDouble(sY)+5)) >= StrToDouble(sY2) ) AND (val(str(StrToDouble ( sY )-5)) <= StrToDouble ( sY2 )) then
                    Printlog "Height was ok. Expected: " & sY & "' was: '" & sY2 & "'"
                else
                    warnLog "Height is different expected: '" & sY & "' is: '" & sY2 & "'"
                endif
            endif
            METOptionen.Cancel
            sleep 5
        endif
    endif

    call hCloseDocument
endcase 'tMET

'-------------------------------------------------------------------------------
testcase tSWF

    dim sFilter as string
    dim sExt as string
    dim x as integer
    dim i as integer

    sFilter = "Macromedia Flash (SWF) (.swf)"
    sExt = ".swf"

    hFileOpen( gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString )

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
            sleep 5
        else
            warnlog "File didn't get saved :-("
        endif
    endif

    call hCloseDocument
endcase 'tSWF

'------------------------------------------------------------------------------
testcase tWMF

    dim x as integer
    dim i as integer
    dim iWaitIndex as integer
    dim sFilter as string
    dim sExt as string
    dim bTemp as boolean
    dim sX as string
    dim sY as string
    dim sx1 as string
    dim sX2 as string
    dim sY2 as string
    dim sDocument as string

    sFilter = "WMF - Windows Metafile (.wmf)"
    sExt = ".wmf"

    sDocument = gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString
    hFileOpen sDocument

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Original.Check
            if Breite.IsEnabled then
                warnlog " :-("
            endif
            if Hoehe.IsEnabled then
                warnlog " :-("
            endif
            Groesse.Check
            Breite.More
            Hoehe.Less
            SVMOptionen.Cancel
            sleep 5
            if ( dir(OutputGrafikTBO+sExt) = "") then ' inspired by bug #99932 Graphic is exported though cancel is pressed
                Printlog "ok :-)"
            else
                warnlog "Dialog was canceled, but file got saved, too :-(  - i35177"
            endif
        else
            Warnlog "No  '" & sFilter & "' -Dialog!"
            i=5
        end if
        sleep 2
        Kontext "Active"
        if Active.Exists(2) then
            Warnlog " '" & sFilter & "'  has a problem"
            Active.OK
        end if
    end if
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Groesse.Check
            Breite.Less
            Hoehe.More
            SVMOptionen.OK
            iWaitIndex = 0
            do while SVMOptionen.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif
    hCloseDocument ()
    hFileOpen (sDocument)
    sleep (10)
   
    printlog "making doc editable if it is readonly"
    call fMakeDocumentWritable

    hTypeKeys ("<escape><tab>")
    fGetSizeXY sx1, sY, TRUE
    if hCallExport (OutputGrafikTBO & "1" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Original.Check
            SVMOptionen.OK
            iWaitIndex = 0
            do while SVMOptionen.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO & "1" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "1" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO & "1" & sExt )
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    hCloseDocument ()
    hNewDocument()
    hRechteckErstellen ( 10, 10, 30, 40 )
    if hCallExport (OutputGrafikTBO & "2" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Groesse.Check
            Breite.SetText "9"
            Hoehe.SetText "9"
            Groesse.Check 'Check "Size" to make changes really go through.
            sx1 = Breite.GetText
            sY = Hoehe.GetText
            SVMOptionen.OK
            iWaitIndex = 0
            do while SVMOptionen.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO & "2" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "2" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hFileOpen (OutputGrafikTBO & "2" & sExt)
        kontext "DocumentDraw"
        DocumentDraw.TypeKeys ("<escape><tab>")
        ContextOriginalSize
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    if hCallExport (OutputGrafikTBO & "3" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptionen"
        if SVMOptionen.Exists (2) then
            Groesse.Check
            sX2 = Breite.GetText
            if (LiberalMeasurement(sx1, sX2)) <> TRUE then
                if (val(str(StrToDouble(sx1)+5)) >= StrToDouble(sX2) ) AND (val(str(StrToDouble ( sx1 )-5)) <= StrToDouble ( sX2 )) then
                    Printlog "Width was ok. Expected: " & sx1 & "' was: '" & sX2 & "'"
                else
                    warnLog "Width is different expected: '" & sx1 & "' is: '" & sX2 & "'"
                endif
            endif
            sY2 = Hoehe.GetText
            if (LiberalMeasurement(sY, sY2)) <> TRUE then
                if ( val(str(StrToDouble(sY)+5)) >= StrToDouble(sY2) ) AND (val(str(StrToDouble ( sY )-5)) <= StrToDouble ( sY2 )) then
                    Printlog "Height was ok. Expected: " & sY & "' was: '" & sY2 & "'"
                else
                    warnLog "Height is different expected: '" & sY & "' is: '" & sY2 & "'"
                endif
            endif
            SVMOptionen.Cancel
            sleep 5
        endif
    endif

    call hCloseDocument
endcase 'tWMF

'-------------------------------------------------------------------------
