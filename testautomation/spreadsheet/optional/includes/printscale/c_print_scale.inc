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
'* owner : oliver.craemer@oracle.com
'*
'* short description : Check correct print scaling settings
'*
'*************************************************************************************************
'*
' #1 subPrintScaling
' #1 tPrintScaling
'*
'\*************************************************************************************************
sub subPrintScaling
  printLog Chr(13) + "---------  Print scaling settings  ----------"

     call tPrintScaling

end sub
'*************************************************************************************************
testcase tPrintScaling
printlog "Check correct print scaling settings"
printlog " "

dim iIndex as INTEGER

dim sOutputFile as STRING
    sOutputFile = gOfficePath & ConvertPath( "user\work\printScaling.ods" )
    
dim sNameSheet1 as STRING
    sNameSheet1 = "Ten"
dim sNameSheet2 as STRING
    sNameSheet2 = "Moves"
dim sNameSheet3 as STRING
    sNameSheet3 = "Ahead"
    
dim sNameStyle1 as STRING
    sNameStyle1 = "1stStyle"
dim sNameStyle2 as STRING
    sNameStyle2 = "2ndStyle"
dim sNameStyle3 as STRING
    sNameStyle3 = "3rdStyle"
    
dim sPagesToPrint as STRING

    printlog "Open test document"
    Call hNewDocument
    sleep(2)
    
    printlog "Add minimal content to all sheets"
    for iIndex = 1 to 3
        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
            ViewToolbarsFormulaBar
        end if
        sleep(1)
        printlog "* Create simple data area on sheet" & iIndex
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        if iIndex = 1 then
            Bereich.TypeKeys "A1:B2"
            printlog "* Type 'A1:B2' in Name Box to select area"
        else
            Bereich.TypeKeys "A1:AF400"
            printlog "* Type 'A1:AF400' in Name Box to select area"
        end if
        sleep(1)
        printlog "* Press <RETURN>"
        Bereich.TypeKeys "<RETURN>"
        Kontext "DocumentCalc"
        printlog "* Fill area with '13' by typing 13<SHIFT MOD1 RETURN>"
        DocumentCalc.TypeKeys "=13<SHIFT MOD1 RETURN>"
        sleep(1)
        printlog "* Assign specific name for sheet to avoid language dependencies by Format - Sheet - Rename"
        FormatSheetRename
        Kontext "TabelleUmbenennen"
        select case iIndex
               case 1 : Tabellenname.SetText sNameSheet1
               case 2 : Tabellenname.SetText sNameSheet2
               case 3 : Tabellenname.SetText sNameSheet3
        end select
        TabelleUmbenennen.OK
        sleep(1)

        printlog "* Switch to next sheet by Edit - Sheet - Select"
        EditSheetSelect
        Kontext "SelectSheets"
        if iIndex = 3 then
            printlog "* At the end return to the first sheet"
            SheetSelectionBox.Select sNameSheet1
        else
            SheetSelectionBox.Select ( iIndex + 1 )
        end if
        SelectSheets.OK
        sleep(1)
    next iIndex
    
    printlog "Invoke Style catalog"
    FormatStylesCatalog
    Kontext "Vorlagenkatalog"
    printlog "Switch to Pagestyles"
    Vorlagen.select ( 2 )
    sleep(1)
    printlog "Create three styles using different page scaling features"
    for iIndex = 1 to 3
        Kontext "Vorlagenkatalog"
        printlog "* Click new"
        Neu.Click
        Kontext
        printlog "* (Page format dialog should appear)"
        printlog "* Select Page 'Organizer'"
        active.SetPage TabVerwalten
        printlog "* Close MsgBox when appeared"
        if Active.Exists(2) then
            if Active.GetRT = 304 then
                Printlog Active.GetText
                Active.Yes
            end if
        end if
        Kontext "TabVerwalten"
        printlog "* Assign name for each style"
        select case iIndex
               case 1 : VorlagenName.setText sNameStyle1
               case 2 : VorlagenName.setText sNameStyle2
               case 3 : VorlagenName.setText sNameStyle3
        end select
        Kontext
        printlog "* Select Page 'Sheet'"
        active.SetPage TabTabelleCalc
        Kontext "TabTabelleCalc"
        printlog "* For all three styles use different page scaling features"
        scalingMode.Select ( iIndex )
        select case iIndex
               case 1 : SkalierungProzent.setText "444%"
                        printlog "* Scale size to 444%"
                        printlog "* NOTE: Maximum scale is 400%! Entering 444% should be decreased to 400%."
               case 2 : ScalePageWidth.setText "3"
                        ScalePageHeight.setText "3"
                        printlog "* Scale output to 3 pages wide and 3 pages high"
               case 3 : AnzahlMaxSeiten.setText "4"
                        printlog "* Scale output to a total of 4 pages"
        end select
        printlog "* Leave page format dialog using OK"
        TabTabelleCalc.OK
    next iIndex
    printlog "Close style catalog using OK"
    Kontext "Vorlagenkatalog"
    Vorlagenkatalog.OK
    sleep(2)
    printlog "Assign the three styles created to the three sheets in document"
    for iIndex = 1 to 3
        printlog "* Select sheet" & iIndex & " by Edit - Sheet - Select"
        EditSheetSelect
        Kontext "SelectSheets"
        SheetSelectionBox.Select ( iIndex  )
        SelectSheets.OK
        sleep(1)
        printlog "* Open the stylist if it is not opened"
        Kontext "Gestalter"
        if NOT Gestalter.exists(2) then
            FormatStylist
        end if
        printlog "* Assign style" & iIndex & " using the stylist"
        Kontext "Gestalter"
        Seitenvorlagen.Click
        Kontext "Vorlagenliste"
        Vorlagenliste.TypeKeys "<HOME>"
        sleep (2)
        Vorlagenliste.Select ( iIndex )
        Vorlagenliste.TypeKeys "<RETURN>"
    next iIndex
    printlog "Close stylist"
    Kontext "Gestalter"
    if Gestalter.exists(2) then
        FormatStylist
    end if
        
    printlog "Save in current default format"
    if hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) = FALSE then
        warnlog "Saving " & sOutputFile & " failed! -> Exiting test!"
        call hCloseDocument
    else
        printlog "... and close"
        call hCloseDocument
        sleep(2)
        printlog "Reopen document"
        call hFileOpen ( sOutputFile )
        sleep(2)
        printlog "Invoke File - Print"
        FilePrint
        Kontext "Active"
        Sleep 1
        printlog "Check if a MsgBox pops up due to misconfigured printer settings"
        if Active.Exists(2) then
            if Active.GetRT = 304 then  
                qaErrorlog "OOPS, are you sure that a printer is configured at all?"
                try
                   Active.Ok
                catch
                   Active.Yes
                endcatch
            end if
        end if
        printlog "Check if print dialog exists"
        Kontext "Printing"
        if Printing.Exists(2) then
            Kontext "TabPrintGeneral"
            PrintPages.check
            printlog "Check for string '1-4' in pages textbox"
            sPagesToPrint = PageRange.getText
            if sPagesToPrint = "1-4" then
                printlog "    OK, expected number of pages for output"
            else
                warnlog "Unexpected number of pages for print output: " & sPagesToPrint & " -> Check this out!"
            end if
            Kontext "Printing"
            Printing.Cancel
        else
            qaErrorlog "OOPS, are you sure that a printer is configured at all?"
        end if
        printlog "For all sheets ..."
        for iIndex = 1 to 3
            printlog "* ... select currently desired one"
            EditSheetSelect
            Kontext "SelectSheets"
            SheetSelectionBox.Select ( iIndex  )
            SelectSheets.OK
            sleep(1)
            printlog "* Invoke Format - Page..."
            FormatPageCalc
            Kontext
            printlog "* Select Page 'Sheet'"
            active.SetPage TabTabelleCalc
            printlog "* Close MsgBox when appeared"
            if Active.Exists(2) then
                if Active.GetRT = 304 then
                    Printlog Active.GetText
                    Active.Yes
                end if
            end if
            Kontext "TabTabelleCalc"
            if scalingMode.GetSelIndex = iIndex then
                printlog "* Check if styles for print scaling have be appropriately assign"
                select case iIndex
                       case 1 
                           if SkalierungProzent.GetText <> "400%" then
                               warnlog "Unexpected scaling factor detected -> Check this out!"
                           end if
                           printlog "* Correct scaling mode selected"
                       case 2
                           if ScalePageWidth.GetText <> "3" then
                               warnlog "Unexpected page width detected -> Check this out!"
                           end if
                           if ScalePageHeight.GetText <> "3" then
                               warnlog "Unexpected page height detected -> Check this out!"
                           end if
                           printlog "* Correct scaling mode selected"
                       case 3 
                           if AnzahlMaxSeiten.GetText <> "4" then
                               warnlog "Unexpected page height detected -> Check this out!"
                           end if
                           printlog "* Correct scaling mode selected"
                end select
            else
                warnlog "Unexpected scaling mode found -> Check this out!"
            end if
            printlog "* Quit dialog"
            TabTabelleCalc.Cancel
        next iIndex
        printlog "Close document"
        call hCloseDocument
       
    end if
endcase

