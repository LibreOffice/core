'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Resource test for the printing dialog
'*
'\******************************************************************************

private const MSG1 = "Incorrect default setting: "

'*******************************************************************************

sub g_printing()

    call tPrintCommon()
    call tPrintGeneral()
    call tPrintApplication()
    call tPrintLayout()
    call tPrintOptions()
    
end sub

'*******************************************************************************

testcase tPrintCommon()

    printlog( "Common print options (main dialog)" )

    printlog( "Open new document" )
    hCreateDocument()
    
    printlog( "Open printing dialog" )
    FilePrint
    
    kontext "Printing"
    if ( Printing.exists( 2 ) ) then
        
        call DialogTest( Printing )
        
        if ( PreviewPageNumber.isEnabled() ) then 
            warnlog( "Page number should be disabled for empty document" )
        endif
        
        if ( ScrollBackward.isEnabled() ) then
            warnlog( "Scroll backward button should be disabled for empty document" )
        endif

        if ( ScrollForward.isEnabled() ) then
            warnlog( "Scroll forward button should be disabled for empty document" )
        endif
        
        kontext "Printing"
        printlog( "Close print dialog")
        Printing.cancel()
        
    else
        warnlog( "Printing dialog did not open" )
    endif
    
    printlog( "Close document" )
    hDestroyDocument()        
    
endcase

'*******************************************************************************

testcase tPrintGeneral()

    printlog( "General printing options - General tabpage" )
    
    dim iSlidesPerPage as integer
    dim iExpected as integer

    printlog( "Open new document" )
    hCreateDocument()
    
    printlog( "Open printing dialog" )
    FilePrint
    
    kontext "Printing"
    if ( Printing.exists( 2 ) ) then
    
        PrintingTabControl.setPage( TabPrintGeneral )
        
        kontext "TabPrintGeneral"
        call DialogTest( TabPrintGeneral )

        printlog( "PrinterList" )    
        if ( NOT PrinterList.isEnabled() ) then
            warnlog( "Printer List is disabled" )
        endif
        
        printlog( "Number of copies/Collate" )    
        if ( NumberOfCopies.getText <> "1" ) then
            warnlog( "Number of copies should be 1 by default, is " & NumberOfCopies.getText() )
        endif
        
        if ( Collate.isEnabled() ) then
            warnlog( "<Collate> should not be enabled for just one copy" )
        endif

        NumberOfCopies.setText( "2" )
        
        if ( Collate.isEnabled() ) then
            cb_test( Collate, true, "" )
        else
            warnlog( "Collate should be enabled if number of copies <> 1" )
        endif
        
        NumberOfCopies.setText( "1" )
        
        if ( Collate.isEnabled() ) then
            warnlog( "<Collate> should not be enabled for just one copy" )
        endif

        printlog( "Properties/Details" )
        Details.click() ' open
        Details.click() ' close
        
        if ( gPlatGroup = "unx" and lcase( gPlatform ) <> "osx" ) then
            Properties.click()
            kontext "TabSPAPaper"
            if ( TabSPAPaper.exists( 2 ) ) then
                TabSPAPaper.close()
            else
                warnlog( "Printer properties did not open." )
            endif
        else
            printlog( "Skipping printer properties on Windows and MacOS X (no SPAdmin)" )
        endif

        kontext "TabPrintGeneral"
        printlog( "General settings on the Tab-Page" )
        
        select case( gApplication )
        case "WRITER", "HTML", "MASTERDOCUMENT"

            lb_test( PrintNotes, 4, 1, "" )
            
            if ( NOT AllPages.isChecked() ) then
                warnlog( MSG1 & "<All pages> should be checked" )
            endif            
            
            if ( Pages.isEnabled() ) then
                Pages.check()
                PageRange.setText( "1-5,7" )
            else
                warnlog( "Pages setting disabled" )
            endif

            cb_test( PrintReverse, false, "" )
            'Selection.check()
            'AllPages.check()
            
        case "CALC"
        
            printlog( "Print ranges" )
            if ( NOT SelectedSheets.isChecked() ) then
                warnlog( MSG1 & "<Selected sheets> should be selected" )
            endif
            
            AllSheets.check()
            SelectedSheets.check()
            SelectedCells.check()
            
            if ( NOT PrintAllPages.isChecked() ) then
                warnlog( MSG1 & "<Thereof print all pages> should be selected" )
            endif
            
            PrintPages.check()
            PageRange.setText( "1-5,8" )
            PrintAllPages.check()
            
        case "IMPRESS", "DRAW"
        
            printlog( "Print ranges" )
            if ( NOT AllSlides.isChecked() ) then
                warnlog( MSG1 & "<All slides> should be selected" )
            endif
            
            Slides.check()
            PageRange.setText( "1-6,9" )
            
            Selection.check()
            
            
            if ( gApplication = "IMPRESS" ) then
            
                printlog( "General print options/layout (IMPRESS only)" )
            
                lb_test( PartsToPrint, 4, 1, "" )
                
                if ( SlidesPerPage.isEnabled() ) then
                    warnlog( "<Slides per page> should only be enabled for Handouts" )
                endif

                if ( ArrangeSlides.isEnabled() ) then
                    warnlog( "<Ararange slides> should only be enabled for Handouts" )
                endif
                
                PartsToPrint.select( 2 ) ' Should be "Handouts" to enable further controls
                
                if ( NOT SlidesPerPage.isEnabled() ) then
                    warnlog( "<Slides per Page> should be enabled for Handouts" )
                endif
                
                if ( NOT ArrangeSlides.isEnabled() ) then
                    warnlog( "<Arrange slides> should be enabled for Handouts" )
                endif

                lb_test( SlidesPerPage, 7, 0, "" )
                lb_test( ArrangeSlides, 2, 1, "" )
                
            endif
            
                        
        case "MATH"
        
            printlog( "Math does not have any special options on this tabpage" )
            
        case default
            warnlog( "Invalid gApplication: " & gApplication )
        end select

        kontext "Printing"
        printlog( "Close print dialog" )
        Printing.cancel()

    else
        warnlog( "Printing dialog did not open" )
    endif
    
    printlog( "Close document" )
    hDestroyDocument()        
    
endcase

'*******************************************************************************

testcase tPrintApplication()

    printlog( "Application specific printing options - Application tabpage" )

    printlog( "Open new document" )
    hCreateDocument()
    
    printlog( "Open printing dialog" )
    FilePrint
    
    kontext "Printing"
    if ( Printing.exists( 2 ) ) then

        PrintingTabControl.setPage( TabApplication )
        
        kontext "TabApplication" 
        call DialogTest( TabApplication )

        printlog( "Check defaults, modify and reset controls" )
        select case( gApplication )
        case "WRITER", "HTML", "MASTERDOCUMENT"
        
            cb_test( PrintPageBackground, true, "" )
            'cb_test( PrintGraphicsAndDiagrams, true, "" )
            cb_test( PrintPicturesAndObjects, true, "" )
            cb_test( PrintTextInBlack, false, "" )
            cb_test( PrintBlankPages, true, "" )

            if ( gApplication <> "HTML" ) then
                cb_test( PrintHiddenText, false, "" )
                cb_test( PrintTextPlaceholder, false, "" )
            endif
                
        case "CALC"
        
            cb_test( IncludeEmptyPages, false, "" )
            
        case "IMPRESS", "DRAW"
        
            cb_test( SlideName, false, "" )
            cb_test( DateAndTime, false, "" )
            if ( gApplication = "IMPRESS" ) then 
                cb_test( HiddenPages, false, "" )
            endif
            
            printlog( "Color options" )
            if ( NOT OriginalColors.isChecked() ) then
                warnlog( MSG1 & "<Original colors> should be checked" )
            endif
            
            GrayScale.check()
            BlackAndWhite.check()
            
            printlog( "Scaling options" )
            if ( NOT OriginalSize.isChecked() ) then
                warnlog( MSG1 & "<Original size> should be checked" )
            endif
            
            FitToPage.check()
            MultiplePages.check()
            TilePaper.check()
      
        case "MATH"
        
            cb_test( TitleRow, true, "" )
            cb_test( FormulaText, true, "" )
            cb_test( Borders, true, "" )
            
            if ( NOT OriginalSizeMath.isChecked() ) then
                warnlog( MSG1 & "<Original size> should be selected" )
            endif
            
            FitToPageMath.check()
            Scaling.check()
            ScalingFactor.setText( "1000" )
            
        case default
        
            warnlog( "Invalid gApplication: " & gApplication )
            
        end select
        
        kontext "Printing"
        printlog( "Close print dialog" )
        Printing.cancel()

    else
        warnlog( "Printing dialog did not open" )
    endif
    
    printlog( "Close document" )
    hDestroyDocument()
    
endcase

'*******************************************************************************

testcase tPrintLayout()

    printlog( "Layout printing options - Layout tabpage" )
    
    printlog( "Open new document" )
    hCreateDocument()
    
    printlog( "Open printing dialog" )
    FilePrint
    
    kontext "Printing"
    if ( Printing.exists( 2 ) ) then

        ' Math does not have a layout page as it does not have any layout options
        if ( gApplication <> "MATH" ) then    
        
            PrintingTabControl.setPage( TabLayout )
            
            kontext "TabLayout"
            call DialogTest( TabLayout )
            
            printlog( "Controls common for (almost) all applications" )
            if ( gApplication <> "CALC" ) then
                if ( NOT PagesPerSheet.isChecked()) then
                    warnlog( MSG1 & "<Pages per sheet> should be selected" )
                endif

                lb_test( NumberOfPages, 7, 1, "" )
                NumberOfPages.select( 7 )
               
                if ( NumberOfColumns.getText() <> "1" ) then
                    warnlog( MSG1 & "<Pages> has incorrect default: " & NumberOfColumns.getText() )
                endif
                
                if ( NumberOfRows.getText() <> "1" ) then
                    warnlog( MSG1 & "<by> has incorrect default: " & NumberOfRows.getText() )
                endif
                
                NumberOfColumns.typeKeys( "<UP>" )
                NumberOfRows.typeKeys( "<UP>" )
                DistanceBetweenPages.typeKeys( "<UP>" )
                MarginToSheetBorder.typeKeys( "<UP>" )
            
                lb_test( PaperOrientation, 3, 1, "" )
            else
                if ( PagesPerSheet.isVisible() ) then
                    warnlog( "<Pages per Sheet> should not be visible for spreadsheet documents" )
                endif
            endif
            
            lb_test( PrintOrder, 4, 1, "" )
            cb_test( BorderAroundPages, false, "" )
            
            select case( gApplication )
            case "WRITER", "HTML", "MASTERDOCUMENT"

                'if ( NOT PrintBrochureRTL.isEnabled() ) then
                '    warnlog( "<Brochure> listbox should be enabled" )
                'endif
                
                Brochure.check()
                
                'lb_test( PrintBrochureRTL, 2, 1, "#i105265#" )
            
            case "CALC"
            
                printlog( "Calc does not have any options in this section" )
                
            case "IMPRESS", "DRAW"

                if ( BrochurePageSides.isEnabled() ) then
                    warnlog( "<Page sides> listbox should be disabled" )
                endif
                Brochure.check()
                lb_test( BrochurePageSides, 3, 1, "" )

            case default
            
                warnlog( "Invalid gApplication: " & gApplication )
                
            end select
            
        else ' What we do if gApplication = MATH
            try
                PrintingTabControl.setPage( TabLayout )
                warnlog( "Math should not have a layout page as no layout options exist" )
            catch
                printlog( "Math has no layout page. Good." )
            endcatch
        endif
        
        kontext "Printing"
        printlog( "Close print dialog" )
        Printing.cancel()

    else
        warnlog( "Printing dialog did not open" )
    endif
    
    printlog( "Close document" )
    hDestroyDocument()
    
endcase

'*******************************************************************************

testcase tPrintOptions()

    printlog( "Extended printing options - Options tabpage" )
    
    printlog( "Open new document" )
    hCreateDocument()
    
    printlog( "Open printing dialog" )
    FilePrint
    
    kontext "Printing"
    if ( Printing.exists( 2 ) ) then
    
        PrintingTabControl.setPage( TabOptions )
        
        kontext "TabOptions"
        call DialogTest( TabOptions )
        
        printlog( "Controls common for all applications" )
        cb_test( PrintToFile, false, "" )
        cb_test( PrintAsSingleJobs, false, "" )        
        
        printlog( "Application specific controls" )    
        select case( gApplication )
        case "WRITER", "HTML", "MASTERDOCUMENT", "IMPRESS", "DRAW"

            cb_test( PaperTrayFromPrefs, false, "" )

        case "CALC", "MATH"
        
            printlog( "Calc does not have any program specific options." )
            
        case default
        
            warnlog( "Invalid gApplication: " & gApplication )
            
        end select
        
        kontext "Printing"
        printlog( "Close print dialog" )
        Printing.cancel()

    else
        warnlog( "Printing dialog did not open" )
    endif
    
    printlog( "Close document" )
    hDestroyDocument()
            
endcase

