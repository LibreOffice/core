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
'* short description : Graphics Export B-tests. (the usual suspects)
'*
'\******************************************************************************
testcase tPNG

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub

    dim x as integer
    dim i as integer
    dim iWaitIndex as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "PNG - Portable Network Graphic (.png)"
    sExt = ".png"

    hFileOpen( gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString )

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "PNGOptions"
        if PNGOptions.Exists (2) then
            Compression.ToMax
            Interlaced.UnCheck
            PNGOptions.OK
            iWaitIndex = 0
            do while PNGOptions.Exists AND iWaitIndex < 30
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

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub


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
    printlog "opening test file"
    sDocument = ConvertPath ( gTesttoolPath & "graphics\required\input\graphicexport." & ExtensionString)
    hFileOpen sDocument

    printlog "Trying export and canceling it.."
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "SVMOptions"
        if SVMOptions.Exists (2) then
            Exportwidth.More
            Exportheight.Less
            SVMOptions.Cancel
            sleep 5
            printlog "Checking if directory is still empty.."
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

    printlog "Doing real SVM export."
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "SVMOptions"
        if SVMOptions.Exists (2) then
            Exportwidth.Less
            Exportheight.More
            SVMOptions.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
    endif
    printlog "Closing TestDoc."
    hCloseDocument ()
    sleep 5
    printlog "Opening new doc and inserting exported file."
    hNewDocument()
    sleep 5
    Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    printlog "Closing doc with inserted file."
    hCloseDocument ()

    printlog "Loading testdoc."
    hFileOpen (sDocument)
    sleep (10)
    printlog "Making doc editable."
    call fMakeDocumentWritable
    printlog "Selecting part of doc."
    hTypeKeys ("<escape><tab>")
     printlog "getting size of part. "
    fGetSizeXY sx1, sY, TRUE
    printlog "exporting part of doc."
    if hCallExport (OutputGrafikTBO & "1" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptions"
        SVMOptions.OK
        sleep 5
        if ( dir(OutputGrafikTBO & "1" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "1" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        printlog "Closing testdoc."
        hCloseDocument ()
        sleep 5
        printlog "Opening new doc."
        hNewDocument()
        sleep 5
        printlog "Inserting exported part of Testdoc."
        Call hGrafikEinfuegen ( OutputGrafikTBO & "1" & sExt )
        bTemp = FALSE
        printlog "checking size of inserted file."
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    printlog "closing doc."
    hCloseDocument ()

    printlog "opening new doc."
    hNewDocument()
    printlog "inserting Rectangle."
    hRechteckErstellen ( 10, 10, 30, 40 )
    printlog "Exporting Rectangle"
    if hCallExport (OutputGrafikTBO & "2" , sFilter, TRUE ) = TRUE then
        Kontext "SVMOptions"
        if SVMOptions.Exists (2) then
            Exportwidth.SetText "9"
            if Exportmeasurement.GetSelText = "inches" then
                sx1 = (Exportwidth.GetText)+ """"
                printlog "sx1= " & sx1
            else
                sx1 = (Exportwidth.GetText)+ "cm"
            endif

            if Exportmeasurement.GetSelText = "inches" then
                sY = (Exportheight.GetText)+ """"
                printlog "sY= " & sY
            else
                sY = (Exportwidth.GetText)+ "cm"
            endif
            SVMOptions.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO & "2" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "2" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        printlog "Closing doc."
        hCloseDocument ()
        sleep 5
        printlog "Opening exported rectangle."
        hFileOpen (OutputGrafikTBO & "2" & sExt)
        kontext "Filterauswahl"
        if Filterauswahl.exists then
            Warnlog "Error when loading the file. The Filter-dialogue came up. Test aborted."
            Filterauswahl.Cancel
            goto endsub
        endif
        kontext "DocumentDraw"
        DocumentDraw.TypeKeys ("<escape><tab>")
        printlog "Selecting and checking size.."
        ContextOriginalSize
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif

    printlog "Closing doc."
    call hCloseDocument
    
endcase 'tSVM
'-------------------------------------------------------------------------------
testcase tPDF

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub


    dim sPath as string
    dim sTemp as string
    dim iFileSize(12) as long
    dim i as integer

    printlog "Export the file 12 times: "
    printlog "every compression level (3) and every range (3), check if button in toolbar uses last settings (6) (-> 12) "
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport_pdf."+ExtensionString ))
    sleep (10)

    '---------------------------------------------------------------------------------------------------------------------------------------------------------
    printlog "File->Export as PDF document"
    printlog " range = All"
    printlog " compression = Screen optimized"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "-------------------   Output the first file ---------------------"

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 1, TRUE, FALSE, 1, 1)
    printlog "Export using Button in toolbar"
    printlog " use the settings from the export before"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "---------------------   Output the second file -------------------"

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 2, TRUE, FALSE, 1, 1) ' button in toolbar -> last settings have to be used

   
    printlog "File->Export as PDF document"
    printlog " range = Range"
    printlog " compression = Screen optimized"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "----------------  Output the third file  --------------------"

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 3, TRUE, FALSE, 2, 1, "2")

    printlog "Export using Button in toolbar"
    printlog " use the settings from the export before"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "----------------------  Output the fourth file  -----------------"

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 4, TRUE, FALSE, 2, 1) ' button in toolbar -> last settings have to be used

    printlog "File->Export as PDF document"
    printlog " range = all"
    printlog " compression = Print optimized"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "-----------------------------  Output the fifth file  ------------"

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 5, TRUE, FALSE, 1, 2)

    printlog "Export using Button in toolbar"
    printlog " use the settings from the export before"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "----------------------  Output the sixth file  -------------------"

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 6, TRUE, FALSE, 1, 2) ' button in toolbar -> last settings have to be used

    printlog "File->Export as PDF document"
    printlog " range = Range"
    printlog " compression = Screen optimized"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "------------------  Output the seventh file  ---------------------"

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 7, TRUE, FALSE, 2, 2, "2")

    printlog "Export using Button in toolbar"
    printlog " use the settings from the export before"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "----------------------  Output the eight file  -------------------"

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 8, TRUE, FALSE, 2, 2) ' button in toolbar -> last settings have to be used

    printlog "File->Export as PDF document"
    printlog " range = all"
    printlog " compression = Press optimized"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "-------------------  Output the ninth file  ----------------------"

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 9, TRUE, FALSE, 1, 3)
    printlog "Export using Button in toolbar"
    printlog "use the settings from the export before"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "-------------------------  Output the tenth file  ----------------"

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 10, TRUE, FALSE, 1, 3) ' button in toolbar -> last settings have to be used

    printlog "File->Export as PDF document"
    printlog " range = all"
    printlog " compression = Press optimized"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "-----------------  Output the eleventh file  ---------------------"

    hExportAsPDFmulti (3, TRUE, OutputGrafikTBO + 11, TRUE, FALSE, 2, 3, "2")
    printlog "Export using Button in toolbar"
    printlog "use the settings from the export before"
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    printlog "------------------  Output the twelfth file  ---------------------"

    hExportAsPDFmulti (1, TRUE, OutputGrafikTBO + 12, TRUE, FALSE, 2, 3) ' button in toolbar -> last settings have to be used
    kontext "Standardbar"
    checkexppdfwaitmax10sec

    '---------------------------------------------------------------------------

    for i=1 to 12
        sPath = OutputGrafikTBO
        sPath = sPath + i + ".pdf"
        iFileSize(i) = app.FileLen(sPath)
        Printlog (" - " + i + ": " + " " + iFileSize(i) + " " + sPath
        if (dir(sPath) <> "") then
            sleep(1)
            printlog " the first line has to be %PDF-1.4"
            sTemp = fGetFileText(sPath, 8)
            if (sTemp <> "%PDF-1.4") then
                Warnlog "File doesn't start with '%PDF-1.4' it is: '" + sTemp + "'"
            endif
            printlog "the last line has to be '%%EOF' "
            sTemp = left(fGetFileText(sPath, -6),5)
            if (sTemp <> "%%EOF") then
                Warnlog "File doesn't end with '%%EOF' it is: '" + sTemp + "'"
            endif
        else
            Warnlog "File was not exported to PDF :-( '" + sPath + "'"
        endif
    next i

    printlog "Check if the file size is the same when exporting via toolbar and file/export"
    printlog "if the size is different then the options are not used when exporting via the toolbar"
    if (iFileSize(1) <> iFileSize(2)) then
        qaerrorlog "i100919: pdf export: different file size when exporting with file/export or button"
        'Warnlog ("Diffrence in filesize for file 1 and 2")
    endif
    if (iFileSize(5) <> iFileSize(6)) then
        qaerrorlog "i100919: pdf export: different file size when exporting with file/export or button"
        'Warnlog ("Difference in filesize for file 5 and 6")
    endif
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

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub

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

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub

   dim x as integer
   dim i as integer
   dim bTemp as boolean
   dim sX as string
   dim sY as string
   dim sx1 as string
   dim sX2 as string
   dim sY2 as string
   dim sDocument as string

    printlog "open the Test-document"
    sDocument = ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString)
    hFileOpen sDocument

    printlog "Trying to export and canceling.."
    if hCallExport (OutputGrafikTBO , "BMP - Windows Bitmap (.bmp)" ) = TRUE then
      Kontext "BMPOptions"
      if BMPOptions.Exists (2) then
         x = Colordepth.GetItemCount
         if x <> 7 then warnlog "Color Resolution Count is wrong; should be:7, is:" + x
         for i = 1 to x
            Colordepth.Select i
            sleep 1
            Printlog " - " + i + ": '" +Colordepth.GetSelText + "'"
         next i
         Sleep 1
         printlog "Leave dialog with cancel -> there has to be no file created!"
         BMPOptions.Cancel
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

   Printlog " now save it really and load the file afterwards"
   if hCallExport (OutputGrafikTBO , "BMP - Windows Bitmap (.bmp)" ) = TRUE then
      Kontext "BMPOptions"
      if BMPOptions.Exists (2) then
         Colordepth.Select 7
         Compression.Check
         BMPOptions.OK
         sleep 5
      endif
      if ( dir(OutputGrafikTBO+".bmp") <> "") then
         Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+".bmp" + "'"
      else
         warnlog "File didn't get saved :-("
      endif
      printlog "Closing doc."
      hCloseDocument ()
      sleep 5
      printlog "Opening new doc."
      hNewDocument()
      sleep 5
      printlog "Inserting exported file."
      Call hGrafikEinfuegen ( OutputGrafikTBO+".bmp" )
   endif
   printlog "Closing doc again."
   hCloseDocument ()
   Printlog "Now save a SELECTION in and load the file afterwards"
   
   hFileOpen (sDocument)
   kontext "Filterauswahl"
   if Filterauswahl.exists(10) then
      Warnlog "Error when loading the file. The Filter-dialogue came up. Test aborted."
      Filterauswahl.Cancel
      goto endsub
   endif

   printlog "check if the document is writable"
   call fMakeDocumentWritable
    
   hTypeKeys ("<escape><tab>")
   fGetSizeXY sx1, sy, TRUE
    if hCallExport (OutputGrafikTBO + "1" , "BMP - Windows Bitmap (.bmp)", TRUE ) = TRUE then
        Kontext "BMPOptions"
        if BMPOptions.Exists (2) then
             BMPOptions.OK
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
   
   Printlog "Now create a rectangle, select it, save it in SIZE and load the file afterwards"
   hNewDocument()
   hTypeKeys "<TAB>"
   gMouseClick 50, 50
   hRechteckErstellen ( 10, 10, 30, 40 )
   if hCallExport (OutputGrafikTBO + "2" , "BMP - Windows Bitmap (.bmp)", TRUE ) = TRUE then
      Kontext "BMPOptions"
      if BMPOptions.Exists (2) then
            Exportwidth.SetText "9"
            if Exportmeasurement.GetSelText = "inches" then
                sx1 = (Exportwidth.GetText)+ """"
                printlog "sx1= " & sx1
            else
                sx1 = (Exportwidth.GetText)+ "cm"
            endif

            if Exportmeasurement.GetSelText = "inches" then
                sY = (Exportheight.GetText)+ """"
                printlog "sY= " & sY
            else
                sY = (Exportwidth.GetText)+ "cm"
            endif
         BMPOptions.OK
         sleep 5
      endif
      if ( dir(OutputGrafikTBO + "2"+".bmp") <> "") then
         Printlog "Ok :-) Saved as: '" + OutputGrafikTBO + "2"+".bmp" + "'"
      else
         warnlog "File didn't get saved :-("
      endif
      printlog "Closing doc."
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
   call hCloseDocument

endcase 'tBMP
'-------------------------------------------------------------------------------
testcase tEMF

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub

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
        Kontext "EMFOptions"
        if EMFOptions.Exists (2) then
            Exportwidth.More
            Exportheight.Less
            EMFOptions.Cancel
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
        Kontext "EMFOptions"
        if EMFOptions.Exists (2) then
            Exportwidth.More
            Exportheight.Less
            EMFOptions.OK
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
   
    hNewDocument()
    hRechteckErstellen ( 10, 10, 30, 40 )
    if hCallExport (OutputGrafikTBO & "2" , sFilter, TRUE ) = TRUE then
        Kontext "EMFOptions"
        if EMFOptions.Exists (2) then
            Exportwidth.SetText "9"
            if Exportmeasurement.GetSelText = "inches" then
                sx1 = (Exportwidth.GetText)+ """"
                printlog "sx1= " & sx1
            else
                sx1 = (Exportwidth.GetText)+ "cm"
            endif
            if Exportmeasurement.GetSelText = "inches" then
                sY = (Exportheight.GetText)+ """"
                printlog "sY= " & sY
            else
                sY = (Exportwidth.GetText)+ "cm"
            endif
            EMFOptions.OK
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
            Kontext "EMFOptions"
            if EMFOptions.Exists (2) then
                sX2 = Exportwidth.GetText
                if (LiberalMeasurement(sx1, sX2)) <> TRUE then
                    if (val(str(StrToDouble(sx1)+5)) >= StrToDouble(sX2) ) AND (val(str(StrToDouble ( sx1 )-5)) <= StrToDouble ( sX2 )) then
                        Printlog "Width was ok. Expected: " & sx1 & "' was: '" & sX2 & "'"
                    else
                        warnLog "Width is different expected: '" & sx1 & "' is: '" & sX2 & "'"
                    endif
                endif
                sY2 = Exportheight.GetText
                if (LiberalMeasurement(sY, sY2)) <> TRUE then
                    if ( val(str(StrToDouble(sY)+5)) >= StrToDouble(sY2) ) AND (val(str(StrToDouble ( sY )-5)) <= StrToDouble ( sY2 )) then
                        Printlog "Height was ok. Expected: " & sY & "' was: '" & sY2 & "'"
                    else
                        warnLog "Height is different expected: '" & sY & "' is: '" & sY2 & "'"
                    endif
                endif
                EMFOptions.Cancel
                sleep 5
            endif
        endif
    endif

    call hCloseDocument
endcase 'tEMF

'-------------------------------------------------------------------------------
testcase tMET

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub

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
    printlog "loading Test File"
    hFileOpen sDocument

    printlog "----------1st: Trying to export and then canceling it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "METOptions"
        if METOptions.Exists (2) then
            sTemp = Exportwidth.getText
            Exportwidth.More
            if (sTemp = Exportwidth.getText) then
                qaErrorLog "Width didn't change on pressing button 'More' #112225#"
                Exportwidth.setText "10"
            endif
            sTemp = Exportheight.getText
            Exportheight.Less
            if (sTemp = Exportheight.getText) then
                warnlog "Height didn't change on pressing button 'Less'"
            endif
            printlog "Canceling export..."
            METOptions.Cancel
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
    printlog "----------2nd: doing real export.."
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "METOptions"
        if METOptions.Exists (2) then
            sTemp = Exportwidth.getText
            Exportwidth.Less
            if (sTemp = Exportwidth.getText) then
                warnlog "Width didn't change on pressing button 'Less'"
            endif
            sTemp = Exportheight.getText
            Exportheight.More
            if (sTemp = Exportheight.getText) then
                qaErrorLog "Height didn't change on pressing button 'More' #112225#"
                Exportwidth.setText "10"
            endif
            METOptions.OK
            iWaitIndex = 0
            do while METOptions.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        printlog "Closing test doc."
        hCloseDocument ()
        sleep 5
        printlog "----------3rd: Inserting export into new doc."
        printlog "Opening new doc."
        hNewDocument()
        sleep 5
        printlog "Inserting the exported file..."
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif
    printlog "Closing file with inserted graphic."
    hCloseDocument ()
    printlog "----------4th: Exporting part of test doc."
    printlog "Opening test file.."
    hFileOpen (sDocument)
    sleep (10)
    printlog "making doc editable if it is readonly"
    call fMakeDocumentWritable
    printlog "Selecting part of doc for export.."
    hTypeKeys ("<escape><tab>")
    fGetSizeXY sx1, sY, TRUE
    if hCallExport (OutputGrafikTBO & "1" , sFilter, TRUE ) = TRUE then
        Kontext "METOptions"
        if METOptions.Exists (2) then
            METOptions.OK
            iWaitIndex = 0
            do while METOptions.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO & "1" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "1" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        printlog "Closing testfile"
        hCloseDocument ()
        sleep 5
        printlog "----------5th: Inserting exported part into new doc."
        printlog "Opening new file.."
        hNewDocument()
        sleep 5
        printlog "inserting exported part."
        Call hGrafikEinfuegen ( OutputGrafikTBO & "1" & sExt )
        bTemp = FALSE
        printlog "Checking size."
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif
    printlog "Closing doc."
    hCloseDocument ()
    printlog "----------6th: exporting fresh rectangle."
    printlog "opening new file."
    hNewDocument()
    printlog "inserting rectangle."
    hRechteckErstellen ( 10, 10, 30, 40 )
    printlog "Exporting rectangle.."
    if hCallExport (OutputGrafikTBO & "2" , sFilter, TRUE ) = TRUE then
        Kontext "METOptions"
        if METOptions.Exists (2) then
            Exportwidth.SetText "9"
            if Exportmeasurement.GetSelText = "inches" then
                sx1 = (Exportwidth.GetText)+ """"
                printlog "sx1= " & sx1
            else
                sx1 = (Exportwidth.GetText)+ "cm"
            endif

            if Exportmeasurement.GetSelText = "inches" then
                sY = (Exportheight.GetText)+ """"
                printlog "sY= " & sY
            else
                sY = (Exportwidth.GetText)+ "cm"
            endif

            METOptions.OK
            iWaitIndex = 0
            do while METOptions.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO & "2" & sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO & "2" & sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        printlog "closing doc."
        hCloseDocument ()
        sleep 5
        printlog "----------7th:Opening exported file with rectangle"
        hFileOpen (OutputGrafikTBO & "2" & sExt)
        kontext "DocumentDraw"
        DocumentDraw.TypeKeys ("<escape><tab>")
        printlog "Checking file size."
        ContextOriginalSize
        bTemp = FALSE
        fGetSizeXY sx1, sY, bTemp
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif

    call hCloseDocument
endcase 'tMET

'-------------------------------------------------------------------------------
testcase tSWF

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub

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

    warnlog( "#i115138 - Loading test document freezes the office" )
    goto endsub

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
    printlog "Loading testdoc."
    hFileOpen sDocument

    printlog "Starting export as WMF and canceling.."
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "WMFOptions"
        if WMFOptions.Exists (2) then
            Exportwidth.Less
            Exportheight.More
            WMFOptions.Cancel
            sleep 5
            if ( dir(OutputGrafikTBO+sExt) = "") then ' inspired by bug #99932 Graphic is exported though cancel is pressed
                Printlog "ok :-), nothing exported due to canceling."
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
    printlog "Doing real export."
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "WMFOptions"
        if WMFOptions.Exists (2) then
            Exportwidth.Less
            Exportheight.More
            WMFOptions.OK
            iWaitIndex = 0
            do while WMFOptions.Exists AND iWaitIndex < 30
                sleep (1)
                iWaitIndex = iWaitIndex + 1
            loop
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" & OutputGrafikTBO+sExt & "'"
        else
            warnlog "File didn't get saved :-("
        endif
        Printlog "Closing test doc."
        hCloseDocument ()
        sleep 5
        printlog "Opening new odc."
        hNewDocument()
        sleep 5
        printlog "Inserting exported graphic."
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif
    printlog "Closing."
    hCloseDocument ()
    Printlog "Opening exported file."
    hFileOpen (sDocument)
    sleep (10)
   
    printlog "making doc editable if it is readonly"
    call fMakeDocumentWritable

    hTypeKeys ("<escape><tab>")
    fGetSizeXY sx1, sY, TRUE
    if hCallExport (OutputGrafikTBO & "1" , sFilter, TRUE ) = TRUE then
        Kontext "WMFOptions"
        if WMFOptions.Exists (2) then
            WMFOptions.OK
            iWaitIndex = 0
            do while WMFOptions.Exists AND iWaitIndex < 30
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
        Kontext "WMFOptions"
        if WMFOptions.Exists (2) then
            Exportwidth.SetText "9"
            if Exportmeasurement.GetSelText = "inches" then
                sx1 = (Exportwidth.GetText)+ """"
                printlog "sx1= " & sx1
            else
                sx1 = (Exportwidth.GetText)+ "cm"
            endif

            if Exportmeasurement.GetSelText = "inches" then
                sY = (Exportheight.GetText)+ """"
                printlog "sY= " & sY
            else
                sY = (Exportwidth.GetText)+ "cm"
            endif
            WMFOptions.OK
            iWaitIndex = 0
            do while WMFOptions.Exists AND iWaitIndex < 30
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

    call hCloseDocument
endcase 'tWMF

'-------------------------------------------------------------------------

function fGetFileText (sFilename as string, iCount as long) as string
    printlog "This function is for getting  the first or last n characters of a file"
    printlog "Input:filename number If the number greater 0 then get n characters from start."
    printlog "number smaller 0 get from end of file."
    printlog "Output:string with characters"

    dim iFile as integer ' filehandle
    dim iTem as integer  ' get 2 bytes of the file
    dim iTemByte(2) as integer ' move 1 byte from iTem in each item
    dim sTemp as string   ' string of file
    dim iSize as long  ' size in bytes of file
    dim i as long      ' runner :-)

    iFile = FreeFile
    Printlog "FreeFile: " + iFile
    if (dir (sFilename) <> "") then
        Printlog "FileLen: " + FileLen(sFile)
        Open sFilename For binary access read shared As #iFile
        Printlog "Loc: " + Loc(#iFile)  ' LONG! where am i in the file?

        iSize = Lof(#iFile) ' get size in bytes of file
            if (iSize > 65530) then '65536 = 64kB
                'Warnlog "fGetFileText: file '" + sFilename + "' might get problems on reading it? size is > 65530 Byte: '" + iSize + "'"
            else
                printlog "iSize: " + iSize
            endif

      sTemp = ""
      if (iCount >= 0) then   ' get bytes from file start
         get iFile,1,sTemp    ' get max 64kByte; but not the 1st 2 bytes :-(
         get iFile,1,iTem     ' get the first 2 bytes of the file
         iTemByte(2) = (iTem AND &H0000FF00) \ &H100   ' and seperate the bytes
         iTemByte(1) = (iTem AND &H000000FF)
         sTemp = chr(iTemByte(1)) + chr(iTemByte(2)) + sTemp  ' put them together
       else     ' get bytes from file end
         if ((iSize+iCount) > 0) then
               select case (iSize+iCount)
                  case 1: get iFile,1,sTemp ' take bytes from the end of the file
                          get iFile,1,iTem     ' get the first 2 bytes of the file
                          sTemp = chr(iTemByte(2)) + sTemp  ' put them together
                  case else: get iFile,(iSize+iCount)-1,sTemp ' take bytes from the end of the file
               end select
         else
            get iFile,1,sTemp ' take bytes from the end of the file
            get iFile,1,iTem     ' get the first 2 bytes of the file
            iTemByte(2) = (iTem AND &H0000FF00) \ &H100   ' and seperate the bytes
            iTemByte(1) = (iTem AND &H000000FF)
            sTemp = chr(iTemByte(1)) + chr(iTemByte(2)) + sTemp  ' put them together
         endif
       endif
      printlog "'"+left(sTemp,iSize)+"'"  ' gotcha!

      if (iSize-(Abs(iCount)) >= 0) then
         fGetFileText = left(sTemp,Abs(iCount))
      else
         'Warnlog "fGetFileText: file '" + sFilename + "' isn't as big as expected; will only return '" + iSize+ "' bytes fom: " + iCount
         fGetFileText = left(sTemp,iSize)
      endif

   ' debugging routine --------------------------------------
   '   iSize = Lof(#iFile)
   '   printlog "iSize: " + iSize
   '   sTemp = ""
   '   if iSize > 0 then
   '      printlog "iSize \ 2: " + (iSize \ 2)
   '      for i = 0 to ((iSize \ 2)-1)
   '         get iFile,(i*2)+1,iTem
   '         Printlog "i: " + i + ": 0x" + hex(iTem)
   '       iTemByte(2) = (iTem AND &H0000FF00) \ &H100
   '       iTemByte(1) = (iTem AND &H000000FF)
   '         sTemp = sTemp + chr(iTemByte(1)) + chr(iTemByte(2))
   '      next i
   '      if (iSize MOD 2) = 1 then
   '         get iFile,iSize,iTem
   '         Printlog "i: " + iSize + ": 0x" + hex(iTem)
   '       iTemByte(1) = (iTem AND &H000000FF)
   '         sTemp = sTemp + chr(iTemByte(1))
   '      endif
   '   endif
   '   printlog "'"+sTemp+"'"
   ' debugging routine --------------------------------------
      Close #iFile
   else  ' does file exist
      Warnlog "fGetFileText: file '" + sFilename + "' doesn't exist"
      fGetFileText = ""
   endif
end function
