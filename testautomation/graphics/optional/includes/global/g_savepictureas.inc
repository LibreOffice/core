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
'/********************************************************************
'*
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Graphics ContextMenu -SavePictureAs -Test
'*
'*************************************************************************
'*
' #1 tContextSaveAsPicture
' #1 tContextSavePictureLinkAsPicture
'*
'\***********************************************************************

testcase tContextSaveAsPicture
    Dim Counter as integer
    Dim CounterShouldBe as integer
    Dim SavedOrgPosition as integer
    Dim i as integer
    Dim q as integer
    Dim iFileSize(50) as Long
    Dim FileList(50) as String
    Dim FileList2(50) as string
    Dim Filesize1 as Long
    Dim Filesize2 as Long
    Dim Filename, LastFilterName as String
    Dim Filetype as String
    Dim OrgFile as String
    Dim NewFile as String
    Dim OrgFileDir as String
    Dim NewFileDir as String
    Dim Testsize as Boolean

    OrgFileDir = ConvertPath ( gTesttoolPath + "graphics\required\input\SaveAsPicture\" )
    NewFileDir = ConvertPath ( gOfficePath + "user\work\SaveAsPicture\" )

    app.mkDir (NewFileDir)
    sleep 3
    FileList2(0) = 0
    GetFileList ((ConvertPath (NewFileDir)), "*.*" , FileList2() )
    sleep 1
    if (KillFileList (FileList2()) <> TRUE) then
        for i = 1 to ListCount2 (FileList2())
            Warnlog "File Not deleted: " + i + ": '" + FileList2(i) + "'"
        next i
    endif

    Call hNewDocument
    Call sSelectEmptyLayout
    setClipboard(" ")
    sleep 2
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    Counter = Dateityp.GetItemCount

    Pfad.SetText OrgFileDir
    Pfad.TypeKeys ("<Return>")
    Kontext "Active"
    if Active.Exists then
        warnlog "Error when going to the directory " + OrgFileDir + ". Active: " + Active.GetText
        Active.OK
    endif
    Kontext "GrafikEinfuegenDlg"

    if Dateityp.GetItemCount > 0 then
        Printlog "The ImportGraphic-Dialogue has " + Dateityp.GetItemCount + " File-types."
    else
        Warnlog "The ImportGraphic-Dialogue doesn't have any File-types listed"
    endif

    for i = 2 to Dateityp.GetItemCount 'Get the number of entries.
        Printlog " *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* "
        Pfad.SetText OrgFileDir
        Pfad.TypeKeys ("<Return>")
        Kontext "Active"
        if Active.Exists then
            warnlog "Error when going to the directory " + OrgFileDir + ". Active: " + Active.GetText
            Active.OK
        endif
        Kontext "GrafikEinfuegenDlg"

        if Dateityp.GetItemCount < 1 then
            Warnlog "Only one or none selectable fileformats."
        endif
        Dateityp.Select i
        Filetype = Dateityp.GetSelText
        Filetype = left(Right(Dateityp.GetSelText,4),3)

        DateiAuswahl.TypeKeys "<END>"
        DateiAuswahl.TypeKeys "<SPACE>"
        if DateiAuswahl.GetItemCount < 1 then
            warnlog "The file with the extension " + Filetype + " does not exist. Please inform FHA."
        else
            printlog "Selected: " + DateiAuswahl.GetSelText
            Filename = Dateiname.GetSelText
            OrgFile = ConvertPath (OrgFileDir + Dateiname.GetSelText)
            printlog "   File is: " + OrgFile

            iFileSize(i) = app.FileLen(OrgFile)
            printlog "   The filesize was: " + iFileSize(i)

            DateiAuswahl.TypeKeys "<DOWN>",true
            if DateiAuswahl.GetSelText <> Filename then
                warnlog "More than one file with the same extension. Contact Test-Administrator."
            endif

            if Link.IsChecked = true then
                Link.UnCheck
            endif

            Oeffnen.Click

            if (gApplication = "IMPRESS") then
                Kontext "DocumentImpress"
            else
                Kontext "DocumentDraw"
            endif
            EditSelectall

            hOpenContextMenu
            if (gApplication = "IMPRESS") then
                Kontext "DocumentImpress"
                DocumentImpress.OpenContextMenu true
                WaitSlot(1000)
                if hMenuFindSelect(27353, true, (MenuGetItemCount - 4), true) = false then
                    Warnlog "Context-Menu-entry `SaveAsGraphics` was not found. Therefore the test ends."
                    Call hCloseDocument
                    Goto Endsub
                endif
            else  'Should be Draw
                Kontext "DocumentDraw"
                DocumentDraw.OpenContextMenu true
                WaitSlot(1000)
                if hMenuFindSelect(27353, true, (MenuGetItemCount - 4), true) = false then
                    Warnlog "Context-Menu-entry `SaveAsGraphics` was not found. Therefore the test ends."
                    Call hCloseDocument
                    Goto Endsub
                endif
            endif
            sleep 1

            Kontext "ExportierenDlg"
            if ExportierenDlg.IsVisible then
                Printlog "   'Save As Picture' came up correctly when using the Context-menu."
            endif

            Kontext "ExportierenDlg" 'SaveAsDialogue

            Testsize = false
            SavedOrgPosition = Dateityp.GetSelIndex
            select case left(lcase(Dateityp.GetSelText),3) 'Filetype
                case "png" : if (Filetype = "png") OR (Filetype = "psd") OR (Filetype = "pcx") OR (Filetype = "xbm") OR (Filetype = "ras") OR (Filetype = "pbm") OR (Filetype = "pcd") OR (Filetype = "pgm") OR (Filetype = "bmp") OR (Filetype = "ppm") OR (Filetype = "sgf") OR (Filetype = "xpm") OR (Filetype = "tga") then
                    printlog "   " + Filetype + " will be saved as png, correct."
                    CounterShouldBe = 17
                else
                    warnlog "Png should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case "svm" : if (Filetype = "svm") OR (Filetype = "svg") OR (Filetype = "dxf") OR (Filetype = "eps") OR (Filetype = "sgv") then
                    printlog "   " + Filetype + " will be saved as Svm, correct."
                    CounterShouldBe = 17
                else
                    warnlog "Svm should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case "wmf" : if (Filetype = "wmf") OR (Filetype = "emf") then
                    printlog "   " + Filetype + " will be saved as Wmf, correct."
                    CounterShouldBe = 17
                if Filetype = "wmf" then Testsize = true
                else
                    warnlog "Wmf should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case "jpe" : if (Filetype = "jpg") OR Filetype = "jif" OR Filetype = "peg" then
                    printlog "   " + Filetype + " will be saved as Jpg, correct."
                    CounterShouldBe = 17
                    if Filetype = "jpg" then Testsize = true
                else
                    warnlog "Jpg should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case "tif" : if Filetype = "tif" OR Filetype = "iff" then
                    printlog "   " + Filetype + " will be saved as Tiff, correct."
                    CounterShouldBe = 17
                if Filetype = "tif" then Testsize = true
                else
                    warnlog "Tif should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case "sgf" : if (Filetype = "sgv") OR (Filetype = "sgf") then
                    printlog "   " + Filetype + " will be saved as Sgf, correct."
                    CounterShouldBe = 17
                if Filetype = "sgf" then Testsize = true
                else
                    warnlog "Sgf should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case "gif" : if (Filetype = "gif") then
                    printlog "   " + Filetype + " will be saved as Gif, correct."
                    CounterShouldBe = 17
                if Filetype = "gif" then Testsize = true
                else
                    warnlog "Gif should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case "met" : if (Filetype = "met") then
                    printlog "   " + Filetype + " will be saved as Met, correct."
                    CounterShouldBe = 17
                if Filetype = "met" then Testsize = true
                else
                    warnlog "Met should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case "pct" : if (Filetype = "pct") OR (Filetype = "ict") then
                    printlog "   " + Filetype + " will be saved as Pct, correct."
                    CounterShouldBe = 17
                if Filetype = "pct" then Testsize = true
                else
                    warnlog "Pct should not have been selected when we opened a '" + Filetype + "'-file."
                endif
                case else  : qaerrorlog "   The test has not yet been fitted for this format ( " + Filetype + "). Contact FHA."
                    printlog "   But will be saved as: " + left(lcase(Dateityp.GetSelText),3)
                    CounterShouldBe = 18
                    Testsize = true
            end select

    '..........................................................................................................................'

            Dateityp.TypeKeys "<HOME>"
            if (Dateityp.GetItemCount) = CounterShouldBe then  'Counter - 10
                for q = 1 to CounterShouldBe - 1
                    LastFilterName = Dateityp.GetSelIndex
                    Dateityp.TypeKeys "<DOWN>"
                        if Dateityp.GetSelIndex = LastFilterName then
                            warnlog "   Check the export-filters when trying to save a " + Filename + "-file. Exiting test."
                            goto endsub
                        endif
                next q
            else
                warnlog "Dateityp.GetItemCount  = " + (Dateityp.GetItemCount) + "  CounterShouldBe =  " + CounterShouldBe
            endif

    Dateityp.Select SavedOrgPosition

    Dateiname.SetText ( NewFileDir )
    Dateiname.TypeKeys ("<Return>")

    kontext "Active"
    if Active.Exists then  'Could not find directory.
        warnlog "Error when going to the directory " + NewFileDir + ". Active: " + Active.GetText
        warnlog "ABORTING TESTCASE. PLEASE INFORM TESTPROGRAMMER."
        Active.OK
        kontext "GrafikEinfuegenDlg"
        GrafikEinfuegenDlg.Close
        goto Endsub
    endif
    Kontext "ExportierenDlg"

    sleep 1
    Dateiname.SetText ConvertPath ( NewFileDir + "SaveAsPicture")
    NewFile = ConvertPath ( NewFileDir + "SaveAsPicture" + "." + left(lcase(Dateityp.GetSelText),3) )
    Dateiname.TypeKeys ("<Return>")
    Kontext "Active"
    if Active.Exists then  'either could not find directory, or overwrite.
        Active.Yes ' Overwrite?
    endif

    if Testsize = true then
        if NewFile <> "" then
            Filesize1 = iFileSize(i)
            printlog "   Saved file-size was: " + app.FileLen(NewFile)
            Filesize2 = app.FileLen(NewFile)
        endif
    endif

    if (Testsize = true) then
        if (Filesize1 < (Filesize2*0.95)) OR (Filesize1 > (Filesize2*1.05)) then
            warnlog "Format '" + right(NewFile,3) + "' was NOT saved properly. Org-Size: '" + Filesize1 + "' Saved-Size: '" + Filesize2 + "'."
        else
            printlog "   The Format '" + right(NewFile,3) + "' was saved correctly."
        endif
    endif
    if (gApplication = "IMPRESS") then
        Kontext "DocumentImpress"
    else
        Kontext "DocumentDraw"
    endif
    hTypeKeys "<DELETE>"
    call hCloseDocument
    call hNewDocument
    Call sSelectEmptyLayout
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    if GrafikEinfuegenDlg.exists(5) then
        printlog "   The Insertgraphics-dialogue showed up correctly."
        sleep (5)
    else
        warnlog "   The Insertgraphics-dialogue didn't show up."
    endif
endif
next i
GrafikEinfuegenDlg.Close

GetFileList (NewFileDir, "*.*" , FileList() )
sleep 1
if (KillFileList (FileList()) <> TRUE) then
for i = 1 to ListCount (FileList())
    Warnlog "File Not deleted: " + i + ": '" + FileList(i) + "'"
next i
endif
Printlog "The test will exit now."
Sleep 1
Call  hCloseDocument
endcase 'tContextSaveAsPicture

'--------------------------------------------------------------------------------------

'(Different behaviour = new testcase)

testcase tContextSavePictureLinkAsPicture
    Dim Counter as integer
    Dim CounterShouldBe as integer
    Dim SavedOrgPosition as integer
    Dim LastFilterName as String
    Dim i as integer
    Dim q as integer
    Dim iFileSize(50) as Long
    Dim FileList(50) as String
    Dim FileList2(50) as string
    Dim Filesize1 as Long
    Dim Filesize2 as Long
    Dim Filename as String
    Dim Filetype as String
    Dim SavedFileType as String
    Dim OrgFile as String
    Dim NewFile as String
    Dim OrgFileDir as String
    Dim NewFileDir as String
    Dim Testsize as Boolean
    Dim Found as Boolean

    OrgFileDir = ConvertPath ( gTesttoolPath + "graphics\required\input\SaveAsPicture\" )
    NewFileDir = ConvertPath ( gOfficePath + "user\work\SaveAsPicture\" )

    app.mkDir (NewFileDir)
    sleep 3
    FileList2(0) = 0
    GetFileList ((ConvertPath (NewFileDir)), "*.*" , FileList2() )
    sleep 1
    if (KillFileList (FileList2()) <> TRUE) then
        for i = 1 to ListCount2 (FileList2())
            Warnlog "File Not deleted: " + i + ": '" + FileList2(i) + "'"
        next i
    endif

    Call hNewDocument
    Call sSelectEmptyLayout
    setClipboard(" ")
    sleep 2
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    Counter = Dateityp.GetItemCount

    Pfad.SetText OrgFileDir
    Pfad.TypeKeys ("<Return>")
    Kontext "Active"
    if Active.Exists then
        warnlog "Error when going to the directory " + OrgFileDir + ". Active: " + Active.GetText
        Active.OK
    endif
    Kontext "GrafikEinfuegenDlg"

    if Dateityp.GetItemCount > 0 then
        Printlog "The ImportGraphic-Dialogue has " + Dateityp.GetItemCount + " File-types."
    else
        Warnlog "The ImportGraphic-Dialogue doesn't have any File-types listed"
    endif

    for i = 2 to Dateityp.GetItemCount 'Get the number of entries.
        Printlog " *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* "
        Pfad.SetText OrgFileDir
        Pfad.TypeKeys ("<Return>")
        Kontext "Active"
        if Active.Exists then
            warnlog "Error when going to the directory " + OrgFileDir + ". Active: " + Active.GetText
            Active.OK
        endif
        Kontext "GrafikEinfuegenDlg"

        if Dateityp.GetItemCount < 1 then
            Warnlog "Only one or none selectable fileformats."
        endif
        Dateityp.Select i
        Filetype = Dateityp.GetSelText
        printlog " orginal: " + Dateityp.GetSelText
        Filetype = left(Right(Dateityp.GetSelText,4),3)

        DateiAuswahl.TypeKeys "<END>"
        DateiAuswahl.TypeKeys "<SPACE>"
        if DateiAuswahl.GetItemCount < 1 then
            warnlog "The file with the extension " + Filetype + " does not exist. Please inform FHA."
        else
            printlog "   Selected: " + DateiAuswahl.GetSelText
            Filename = Dateiname.GetSelText
            OrgFile = ConvertPath (OrgFileDir + Dateiname.GetSelText)
            printlog "   File is: " + OrgFile

            iFileSize(i) = app.FileLen(OrgFile)
            printlog "   The filesize was: " + iFileSize(i)

            DateiAuswahl.TypeKeys "<DOWN>",true
            if DateiAuswahl.GetSelText <> Filename then
                warnlog "More than one file with the same extension. Contact Test-Administrator."
            endif

            if Link.IsChecked <> true then
                Link.Check
            endif
            Oeffnen.Click

            if (gApplication = "IMPRESS") then
                Kontext "DocumentImpress"
            else
                Kontext "DocumentDraw"
            endif
            EditSelectall

            hOpenContextMenu
            if (gApplication = "IMPRESS") then
                Kontext "DocumentImpress"
                DocumentImpress.OpenContextMenu true
                if hMenuFindSelect(27353, true, (MenuGetItemCount - 4), true) = false then
                    Warnlog "Context-Menu-entry `SaveAsGraphics` was not found. Therefore the test ends."
                    Call hCloseDocument
                    Goto Endsub
                endif
            else  'Should be Draw
                Kontext "DocumentDraw"
                DocumentDraw.OpenContextMenu true
                if hMenuFindSelect(27353, true, (MenuGetItemCount - 4), false) = false then
                    Warnlog "Context-Menu-entry `SaveAsGraphics` was not found. Therefore the test ends."
                    Call hCloseDocument
                    Goto Endsub
                endif
            endif
            sleep 1

            Kontext "ExportierenDlg"
            if ExportierenDlg.IsVisible then
                Printlog "   'Save As Picture' came up correctly when using the Context-menu."
            endif

            Kontext "ExportierenDlg" 'SaveAsDialogue

            Testsize = false
            Found = true
            SavedOrgPosition = Dateityp.GetSelIndex
            select case left(lcase(Dateityp.GetSelText),3) 'Filetype

            case "svg" : if (filetype = "svg") then
                printlog "   " + Filetype + " will be saved as svg, correct."
                SavedFileType = "svg"
                Testsize = true
                CounterShouldBe = 17
            else
                warnlog "svg should not have been selected when we opened a '" + Filetype + "'-file."
            endif
            
            case "png" : if (Filetype = "png") then
                printlog "   " + Filetype + " will be saved as png, correct."
                SavedFileType = "png"
                Testsize = true
                CounterShouldBe = 17
            else
                warnlog "Png should not have been selected when we opened a '" + Filetype + "'-file."
            endif
        case "pcx" : if(Filetype = "pcx") then
            printlog "   " + Filetype + " will be saved as pcx, correct."
            SavedFileType = "pcx"
            Testsize = true
            CounterShouldBe = 18
        else
            warnlog "Png should not have been selected when we opened a '" + Filetype + "'-file."
        endif
    case "xbm" : if (Filetype = "xbm") then
        printlog "   " + Filetype + " will be saved as xbm, correct."
        SavedFileType = "xbm"
        Testsize = true
        CounterShouldBe = 18
    else
        warnlog "xbm should not have been selected when we opened a '" + Filetype + "'-file."
    endif
case "xpm" : if (Filetype = "xpm") then
    printlog "   " + Filetype + " will be saved as Xpm, correct."
    SavedFileType = "xpm"
    Testsize = true
    CounterShouldBe = 17
else
    warnlog "xpm should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "svm" : if (Filetype = "svm") then
printlog "   " + Filetype + " will be saved as Svm, correct."
SavedFileType = "svm"
if (Filetype = "svm") then Testsize = true
    CounterShouldBe = 17
else
    warnlog "Svm should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "tga" : if (Filetype = "tga") then
printlog "   " + Filetype + " will be saved as Svm, correct."
SavedFileType = "tga"
if (Filetype = "tga") then Testsize = true
    CounterShouldBe = 18
else
    warnlog "Tga should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "bmp" : if (Filetype = "bmp") then
printlog "   " + Filetype + " will be saved as Bmp, correct."
SavedFileType = "bmp"
Testsize = true
CounterShouldBe = 17
else
warnlog "Bmp should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "ppm" : if (Filetype = "ppm") then
printlog "   " + Filetype + " will be saved as Bmp, correct."
SavedFileType = "ppm"
if Filetype = "ppm" then Testsize = true
CounterShouldBe = 17
else
warnlog "Ppm should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "eps" : if (Filetype = "eps") then
printlog "   " + Filetype + " will be saved as Eps, correct."
SavedFileType = "eps"
if Filetype = "eps" then Testsize = true
CounterShouldBe = 17
else
warnlog "Eps should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "pgm" : if (Filetype = "pgm") then
printlog "   " + Filetype + " will be saved as Pgm, correct."
SavedFileType = "pgm"
if Filetype = "pgm" then Testsize = true
CounterShouldBe = 17
else
warnlog "Pgm should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "dxf" : if (Filetype = "dxf") then
printlog "   " + Filetype + " will be saved as Dxf, correct."
SavedFileType = "dxf"
if Filetype = "dxf" then Testsize = true
CounterShouldBe = 18
else
warnlog "Dxf should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "pbm" : if (Filetype = "pbm") then
printlog "   " + Filetype + " will be saved as Pbm, correct."
SavedFileType = "pbm"
if Filetype = "pbm" then Testsize = true
CounterShouldBe = 17
else
warnlog "Pbm should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "ras" : if (Filetype = "ras") then
printlog "   " + Filetype + " will be saved as Ras, correct."
SavedFileType = "ras"
if Filetype = "ras" then Testsize = true
CounterShouldBe = 17
else
warnlog "Ras should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "wmf" : if (Filetype = "wmf") or (Filetype = "emf") then
printlog "   " + Filetype + " will be saved as Wmf, correct."
SavedFileType = "wmf"
if Filetype = "wmf" then Testsize = true
CounterShouldBe = 17
else
warnlog "Wmf should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "emf" : if (Filetype = "emf") then
printlog "   " + Filetype + " will be saved as Wmf, correct."
SavedFileType = "emf"
Testsize = true
CounterShouldBe = 17
else
qaerrorlog "Wmf should not have been selected when we opened a '" + Filetype + "'-file. #i95364#"
endif
case "jpe" : if (Filetype = "jpg") OR Filetype = "jif" OR Filetype = "peg" then
printlog "   " + Filetype + " will be saved as Jpg, correct."
SavedFileType = "jpg"
Testsize = true
CounterShouldBe = 17
else
warnlog "Jpg should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "tif" : if Filetype = "tif" OR Filetype = "iff"  then
printlog "   " + Filetype + " will be saved as Tiff, correct."
SavedFileType = "tif"
Testsize = true
CounterShouldBe = 17
else
warnlog "Tif should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "sgf" : if (Filetype = "sgf") OR (Filetype = "sgv") then  'sgf
printlog "   " + Filetype + " will be saved as Sgf, correct."
SavedFileType = "sgf"
Testsize = true
CounterShouldBe = 18
else
warnlog "Sgf should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "gif" : if (Filetype = "gif") then
SavedFileType = "gif"
printlog "   " + Filetype + " will be saved as Gif, correct."
if Filetype = "gif" then Testsize = true
CounterShouldBe = 17
else
warnlog "Gif should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "met" : if (Filetype = "met") then
printlog "   " + Filetype + " will be saved as Met, correct."
SavedFileType = "met"
if Filetype = "met" then Testsize = true
CounterShouldBe = 17
else
warnlog "Met should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "pct" : if (Filetype = "pct") OR (Filetype = "ict") then
printlog "   " + Filetype + " will be saved as Pct, correct."
SavedFileType = "pct"
if Filetype = "pct" then Testsize = true
CounterShouldBe = 17
else
warnlog "Pct should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "pcd" : if (Filetype = "pcd") then
printlog "   " + Filetype + " will be saved as Pcd, correct."
SavedFileType = "pcd"
if Filetype = "pcd" then Testsize = true
CounterShouldBe = 18
else
warnlog "Pcd should not have been selected when we opened a '" + Filetype + "'-file."
endif
case "psd" : if (Filetype = "psd") then
printlog "   " + Filetype + " will be saved as Psd, correct."
SavedFileType = "psd"
if Filetype = "psd" then Testsize = true
CounterShouldBe = 18
else
warnlog "Psd should not have been selected when we opened a '" + Filetype + "'-file."
endif
case else : qaerrorlog "   The test has not yet been fitted for this format ( " + Filetype + "). Contact FHA."
printlog "   But will be saved as: " + left(lcase(Dateityp.GetSelText),3)
CounterShouldBe = 18
Found = false
end select

Dateityp.TypeKeys "<HOME>"
if (Dateityp.GetItemCount) = CounterShouldBe then  'Counter - 10
for q = 1 to CounterShouldBe - 1
LastFilterName = Dateityp.GetSelIndex
Dateityp.TypeKeys "<DOWN>"
if Dateityp.GetSelIndex = LastFilterName then
warnlog "   Check the export-filters when trying to save a " + Filename + "-file. Exiting Test."
goto endsub
endif
next q
else
warnlog "Dateityp.GetItemCount  = " + (Dateityp.GetItemCount) + "  CounterShouldBe =  " + CounterShouldBe
endif
Dateityp.Select SavedOrgPosition

Dateiname.SetText ( NewFileDir )
Dateiname.TypeKeys ("<Return>")

kontext "Active"
if Active.Exists then  'Could not find directory.
warnlog "Error when going to the directory " + NewFileDir + ". Active: " + Active.GetText
warnlog "ABORTING TESTCASE. PLEASE INFORM THE TESTPROGRAMMER."
Active.OK
kontext "GrafikEinfuegenDlg"
GrafikEinfuegenDlg.Close
goto Endsub
endif
Kontext "ExportierenDlg"

sleep 1

if Found = false then
NewFile = ConvertPath ( NewFileDir + "SaveAsPicture"  + "." + left(lcase(Dateityp.GetSelText),3) )
else
NewFile = ConvertPath ( NewFileDir + "SaveAsPicture"  + "." + SavedFileType ' left(lcase(Dateityp.GetSelText),3) )
endif
Dateiname.SetText NewFile

Dateiname.TypeKeys ("<Return>")
Kontext "Active"
if Active.Exists then  'either could not find directory, or overwrite.
Active.Yes ' Overwrite?
endif

if Testsize = true then
if NewFile <> "" then
Filesize1 = iFileSize(i)
printlog "   Saved file-size was: " + app.FileLen(NewFile)
Filesize2 = app.FileLen(NewFile)
endif
endif

if (Testsize = true) AND (Filesize1 <> Filesize2) then
qaerrorlog "Format '" + right(NewFile,3) + "' was NOT saved properly. Org-Size: '" + Filesize1 + "' Saved-Size: '" + Filesize2 + "'.#i95364#"
else
printlog "   The Format '" + right(NewFile,3) + "' was saved correctly."
endif

if (gApplication = "IMPRESS") then
Kontext "DocumentImpress"
else
Kontext "DocumentDraw"
endif
hTypeKeys "<DELETE>"
call hCloseDocument
call hNewDocument
Call sSelectEmptyLayout
InsertGraphicsFromFile
Kontext "GrafikEinfuegenDlg"
if GrafikEinfuegenDlg.exists(5) then
printlog "   The Insertgraphics-dialogue showed up correctly."
sleep (5)
else
warnlog "   The Insertgraphics-dialogue didn't show up."
endif
endif
next i

GrafikEinfuegenDlg.Close

GetFileList (NewFileDir, "*.*" , FileList() )
sleep 1
if (KillFileList (FileList()) <> TRUE) then
for i = 1 to ListCount (FileList())
Warnlog "File Not deleted: " + i + ": '" + FileList(i) + "'"
next i
endif
Printlog "The test will exit now."
Sleep 1
Call  hCloseDocument

endcase 'tContextSaveAsPicture

'******************************************************************************************************************************'

testcase Mnemonickey
    qaerrorlog "Not yet ready."
    goto endsub

    if iSprache = 01 then
        ExportierenDlg.Close
        hOpenContextMenu
        TypeKeys ("<SHIFT S>")
        kontext "ExportierenDlg"
        if ExportierenDlg.IsVisible then
            Printlog "'Save As Picture' came up correctly when using the Context-menu."
        endif
    endif
endcase
