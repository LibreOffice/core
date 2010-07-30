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
'* owner : marc.neumann@oracle.com
'*
'* short description : report builder include file
'*
'\***********************************************************************************
sub rpt_Main

    printlog "------------------ ReportBuilder01.inc ---------------------"
    
    if ( tools_reporttools_InstallExtension() = 0 ) then        
        call tMainMenuBar
        call tExecuteReport
        call tEditReport
        call tConditionalFormatingDialog
        call tPageNumber
        call tDateTime
        call tFormatPage
        call tGroupingSorting
        call tReportNavigator
        call tReportHeaderFooter
        call tPageHeaderFooter   
        call tFunctionWizard
    else
        warnlog "report extension could not installed"    
    endif
    
end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tMainMenuBar

    dim iCount as Integer

    '/// open Bibliography database
    
    call hFileOpen(gOfficePath + ConvertPath("user/database/biblio.odb"))
        
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)
    
    '/// open the report designer
    call fOpenNewReportDesign
    
    
        
    '/// check the menu for the right entries

 'file menu    
    Kontext "ReportDesign"
    if ( ReportDesign.exists( 20 ) ) then
        ReportDesign.UseMenu            
        '/// check if there are 10 items in the file menu.
        printlog "check if there are 10 items in the file menu."                
        hMenuSelectNr(1) ' the file menu        
        iCount = hMenuItemGetCount()        
        if ( lcase( gPlatform ) = "osx" ) then
            if ( iCount <> 9 ) then
                warnlog( "File menu: 9 Items expected, found " & iCount )
            endif
        else
            if (iCount <> 10 ) then
                warnlog "There should be 10 items in the file menu but there are " + iCount + " items."
            end if
        endif

        '/// check if there are 12 items in the FILE/NEW menu.
        printlog "check if there are 12 items in the FILE/NEW menu."
        hMenuSelectNr(1) ' FILE/NEW menu        
        iCount = hMenuItemGetCount()        
        if (iCount <> 12) then
            warnlog "There should 12 items in the FILE/NEW menu but there are " + iCount + " items."
        end if        
        call hMenuClose() ' close the FILE menu
        
        '/// check the FILE/AUTOPILOTS
        printlog "check the FILE/AUTOPILOTS"
        ReportDesign.UseMenu        
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(4) ' the autopilots menu        
        iCount = hMenuItemGetCount()
        if (iCount <> 8) then
            warnlog "There should 8 items in the FILE/AUTOPILOTS menu but there are " + iCount + " items."
        endif
        call hMenuClose()

 'edit menu        
        
        '/// check if there are 11 items in the edit menu.
        printlog "check if there are 11 items in the edit menu."
        ReportDesign.UseMenu 
        hMenuSelectNr(2)        
        iCount = hMenuItemGetCount()
        if (iCount <> 11) then
            warnlog "There should 11 items in the edit menu but there are " + iCount + " items."
        end if
        call hMenuClose()

 'view menu        
        
        '/// check if there are 11 items in the view menu.
        printlog "check if there are 11 items in the view menu."
        ReportDesign.UseMenu 
        hMenuSelectNr(3)        
        iCount = hMenuItemGetCount()
        if (iCount <> 11) then
            warnlog "There should 11 items in the view menu but there are " + iCount + " items."
        end if
        call hMenuClose()

 'insert menu        
        
        '/// check if there are 3 items in the insert menu.
        printlog "check if there are 3 items in the insert menu."
        ReportDesign.UseMenu 
        hMenuSelectNr(4)        
        iCount = hMenuItemGetCount()
        if (iCount <> 3) then
            warnlog "There should 3 items in the insert menu but there are " + iCount + " items."
        end if
        call hMenuClose()

 'format menu        
        
        '/// check if there are 7 items in the format menu.
        printlog "check if there are 7 items in the format menu."
        ReportDesign.UseMenu 
        hMenuSelectNr(5)        
        iCount = hMenuItemGetCount()
        if (iCount <> 7) then
            warnlog "There should 7 items in the format menu but there are " + iCount + " items."
        end if
        call hMenuClose()
        
        '/// check if there are 6 items in the format/arange menu.
        printlog "check if there are 6 items in the format/arange menu."        
        ReportDesign.UseMenu 
        hMenuSelectNr(5)
        hMenuSelectNr(4)             
        iCount = hMenuItemGetCount()
        if (iCount <> 6) then
            warnlog "There should 6 items in the format/arrange menu but there are " + iCount + " items."
        end if
        call hMenuClose()
        
        '/// check if there are 6 items in the format/alignment menu.
        printlog "check if there are 6 items in the format/alignment menu."        
        ReportDesign.UseMenu 
        hMenuSelectNr(5)
        hMenuSelectNr(5)             
        iCount = hMenuItemGetCount()
        if (iCount <> 6) then
            warnlog "There should 6 items in the format/alignment menu but there are " + iCount + " items."
        end if
        call hMenuClose()
        
        '/// check if there are 4 items in the format/object resiing menu.
        printlog "check if there are 4 items in the format/object resiing menu."                
        ReportDesign.UseMenu 
        hMenuSelectNr(5)
        hMenuSelectNr(6)             
        iCount = hMenuItemGetCount()
        if (iCount <> 4) then
            warnlog "There should 4 items in the format/alignment menu but there are " + iCount + " items."
        end if
        call hMenuClose()
        

 'tools menu        
        
        '/// check if there are 4 items in the tools menu.
        printlog "check if there are 4 items in the tools menu."
        ReportDesign.UseMenu 
        hMenuSelectNr(6)        
        iCount = hMenuItemGetCount()
        if ( lcase( gPlatform ) = "osx" ) then
            if ( iCount <> 3 ) then
		warnlog( "Tools menu: 3 items expected, found " & iCount )
            endif
        else
            if (iCount <> 4) then
                warnlog "There should 4 items in the tools menu but there are " + iCount + " items."
            end if
        endif
        call hMenuClose()

 'help menu        
        
        '/// check if there are 9 items in the help menu.
        printlog "check if there are 9 items in the help menu."
        ReportDesign.UseMenu 
        hMenuSelectNr(8)        
        iCount = hMenuItemGetCount()
        if ( lcase( gPlatform ) = "osx" ) then iCount = iCount + 1 ' dirty, but check is fuzzy anyway.
        if (gOOO) then
            if (iCount <> 9) then
                warnlog "There should 9 items in the help menu but there are " + iCount + " items."
            end if
        else
            if (iCount <> 9 AND iCount <> 8 ) then
                warnlog "There should 8 or 9 items in the help menu but there are " + iCount + " items."
            end if
        endif
        call hMenuClose()
        
        
        '/// close the report designer
        call fCloseReportDesign
     
    else
        warnlog "The Report Designer did not open within expected timeframe"
    endif
    '/// close the database
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tConditionalFormatingDialog

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign

    Kontext "ReportDesign"
        ReportDesign.Maximize()    
    
    '/// insert a data control
    printlog "insert a data control"
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    
    Kontext "ReportDesign"    
            ReportDesign.MouseDown ( 30, 10 )            
            ReportDesign.MouseMove ( 40, 20 )            
            ReportDesign.MouseUp ( 40, 20 )            
    sleep(1)                        
                            
    '/// open the conditional formating dialog via the menu    
    printlog "open the conditional formating dialog via the menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(5)
        hMenuSelectNr(1) 
        
    '/// check if the conditional formating dialog appear
    printlog "check if the conditional formating dialog appear"
    sleep(10)
    Kontext "ConditionalFormating"        
        if ( not ConditionalFormating.exists(3)) then
            warnlog "Condditional Formating Dialog doesn't appear"
            call fCloseReportDesign    
            call fCloseDatabase
            goto endsub
        else
            printlog "Conditional Formating Dialog appear"
            call DialogTest(ConditionalFormating)
            ConditionalFormating.Cancel
        endif
    
    '/// check if there is one condition
    '/// add 2 more conditions and check if the dialog grows
    '/// add one more condition and check if the scrollbar gets enabled
    '/// check if the right condition is displayed
    '/// remove the second condition and check if the scrollbar gets disabled
    '/// remove 2 conditions and check if the dialog skrink
    
    '/// close the report designer
    call fCloseReportDesign
    '/// close the database
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tExecuteReport

    '/// open Bibliography database    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    call fOpenNewReportDesign

    '/// select the first table in the content list box
    printlog "select the first table in the content list box"
    Kontext "ReportDataProperties"   
        Content.select 1
        Content.typeKeys("<RETURN>",true) ' important to leave the listbox
    
    Kontext "ReportDesign"
        ReportDesign.TypeKeys("<MOD1 TAB>",true)
        ReportDesign.TypeKeys("<MOD1 TAB>",true)
        
    Kontext "ReportAddField"
        ReportAddFieldList.select 1    
        ReportAddField.TypeKeys("<RETURN>",true)        
        
    'close the Add Field dialog to get the focus back to the design
    call fCloseAddFieldDialog()
        
    '/// execute the report
    printlog "execute the report"    
    Kontext "ReportDesign"
        ReportDesign.UseMenu            
        hMenuSelectNr(2)
        hMenuSelectNr(11)
       
    sleep(10)
    
    '/// ckeck if the report is created
    printlog "ckeck if the report is created"
    Kontext "DocumentWriter"
        if (DocumentWriter.exists(10)) then
            call fCloseReportView
        else
            warnlog "No report is created."
        endif
        
    sleep(1)
    '/// close the report design
    printlog "close the report design"
    call fCloseReportDesign
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase
    
endcase        
'-------------------------------------------------------------------------
testcase tPageNumber

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign

    '/// insert a data control
    printlog "insert a data control"
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    
    Kontext "ReportDesign"    
            ReportDesign.MouseDown ( 30, 20 )
            ReportDesign.MouseMove ( 40, 30 )            
            ReportDesign.MouseUp ( 40, 30 )            
    sleep(1)                        
                            
    '/// open page number dialog via the menu
    printlog "open page number dialog via the menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(1) 
        
    '/// check if the page number dialog appear
    printlog "check if the page number dialog appear"
    Kontext "ReportPageNumber"
        if ( not ReportPageNumber.exists(3)) then
            warnlog "Page Number Dialog doesn't appear"
            call fCloseReportDesign    
            call fCloseDatabase
            goto endsub
        else   
            printlog "Page Number Dialog appear"
            call DialogTest(ReportPageNumber)
            ReportPageNumber.Cancel
        endif
        
    '/// close the report designer
    call fCloseReportDesign
    '/// close the database
    call fCloseDatabase

endcase    
'-------------------------------------------------------------------------
testcase tDateTime

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign

    '/// insert a data control
    printlog "insert a data control"
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    
    Kontext "ReportDesign"    
            ReportDesign.MouseDown ( 30, 20 )
            ReportDesign.MouseMove ( 40, 30 )            
            ReportDesign.MouseUp ( 40, 30 )            
    sleep(1)                        
                            
    '/// open date time dialog via the menu
    printlog "open date time dialog via the menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(2) 
        
    '/// check if the date time dialog appear
    printlog "check if the date time dialog appear"    
    Kontext "ReportDateTime"
        if ( not ReportDateTime.exists(3)) then
            warnlog "Date Time Dialog doesn't appear"
            call fCloseReportDesign    
            call fCloseDatabase
            goto endsub
        else   
            printlog "Date Time Dialog appear"
            call DialogTest(ReportDateTime)
            ReportDateTime.Cancel
        endif
    
    
    '/// close the report designer
    call fCloseReportDesign
    '/// close the database
    call fCloseDatabase

endcase
'-------------------------------------------------------------------------
testcase tGroupingSorting

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign

    '/// insert a data control
    printlog "insert a data control"
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    
    Kontext "ReportDesign"    
            ReportDesign.MouseDown ( 30, 50 )            
            ReportDesign.MouseMove ( 40, 60 )            
            ReportDesign.MouseUp ( 40, 60 )            
    sleep(1)                        
                            
    '/// open date time dialog via the menu
    printlog "open date time dialog via the menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(3)
        hMenuSelectNr(4) 
        
    '/// check if the sorting and grouping dialog appear
    printlog "check if the sorting and grouping dialog appear"    
    Kontext "ReportSortingGrouping"
        if ( not ReportSortingGrouping.exists(3)) then
            warnlog "Sorting and Grouping Dialog doesn't appear"
            call fCloseReportDesign    
            call fCloseDatabase
            goto endsub
        else   
            printlog "Sorting and Grouping Dialog appear"
            call DialogTest(ReportSortingGrouping)
            ReportSortingGrouping.Close
        endif
    
    
    '/// close the report designer
    call fCloseReportDesign
    '/// close the database
    call fCloseDatabase

endcase
'-------------------------------------------------------------------------
testcase tReportNavigator

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign

    '/// insert a data control
    printlog "insert a data control"
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    
    Kontext "ReportDesign"    
            ReportDesign.MouseDown ( 30, 50 )            
            ReportDesign.MouseMove ( 40, 60 )            
            ReportDesign.MouseUp ( 40, 60 )            
    sleep(1)                        
                            
    '/// open report navigator via the menu
    printlog "open report navigator via the menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(3)
        hMenuSelectNr(5) 
        
    '/// check if the report navigator appear
    printlog "check if the report navigator appear"    
    Kontext "ReportNavigator"
        if ( not ReportNavigator.exists(3)) then
            warnlog "report navigator doesn't appear"
            call fCloseReportDesign    
            call fCloseDatabase
            goto endsub
        else   
            printlog "report navigator appear"
            call DialogTest(ReportNavigator)
            ReportNavigator.Close
        endif
    
    
    '/// close the report designer
    call fCloseReportDesign
    '/// close the database
    call fCloseDatabase

endcase
'-------------------------------------------------------------------------
testcase tReportHeaderFooter

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign
                        
    '/// open report navigator
    printlog "open report navigator"
    call fOpenReportNavigator        
        
    '/// check if the report navigator appear
    printlog "check if the report navigator appear"    
    Kontext "ReportNavigator"
        if ( not ReportNavigator.exists(3)) then
            warnlog "report navigator doesn't appear"
            call fCloseReportDesign    
            call fCloseDatabase
            goto endsub        
        endif
        '/// check if there are 6 entries in the report navigator
        printlog "check if there are 6 entries in the report navigator"
        if (NavigatorTree.getItemCount <> 6) then
            warnlog "the count of items in the report navigator is not correct." + _
            "it should 6 but it is " + NavigatorTree.getItemCount
        endif
    
    'close the report navigator to get the focus back into the document    
    call fCloseReportNavigator
        
    '/// turn on the report header via the edit menu
    printlog "turn on the report header via the edit menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(2)
        hMenuSelectNr(9)

    'open the navigator again    
    call fOpenReportNavigator
        
    '/// check if report header appear in the report navigator (8 entries)
    printlog "check if report header appear in the report navigator (8 entries)"
    Kontext "ReportNavigator"        
        if (NavigatorTree.getItemCount <> 8) then
            warnlog "the report header is not diaplyed in the report navigator."
        endif

    'close the report navigator to get the focus back into the document        
    call fCloseReportNavigator    
        
    '/// turn off the report header via the edit menu
    printlog "turn off the report header via the edit menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(2)
        hMenuSelectNr(9)
    
    'open the navigator again
    call fOpenReportNavigator
        
    '/// check if report header disappear in the report navigator (6 entries)
    printlog "check if report header disappear in the report navigator (6 entries)"
    Kontext "ReportNavigator"        
        if (NavigatorTree.getItemCount <> 6) then
            warnlog "the report header is not diaplyed in the report navigator."
        endif        

    'close the report navigator
    call fCloseReportNavigator
            
    '/// close the report designer
    printlog "close the report designer"
    call fCloseReportDesign
    '/// close the database
    printlog "close the database"
    call fCloseDatabase

endcase
'-------------------------------------------------------------------------
testcase tPageHeaderFooter

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign
                        
    '/// open report navigator
    call fOpenReportNavigator     
        
    '/// check if the report navigator appear
    printlog "check if the report navigator appear"    
    Kontext "ReportNavigator"
        if ( not ReportNavigator.exists(3)) then
            warnlog "report navigator doesn't appear"
            call fCloseReportDesign    
            call fCloseDatabase
            goto endsub        
        endif
        '/// check if there are 6 entries in the report navigator
        printlog "check if there are 6 entries in the report navigator"
        if (NavigatorTree.getItemCount <> 6) then
            warnlog "the count of items in the report navigator is not correct." + _
            "it should 6 but it is " + NavigatorTree.getItemCount
        endif
    
    'close the report navigator to get the focus back into the document        
    call fCloseReportNavigator
        
    '/// turn off the page header via the edit menu
    printlog "turn off the page header via the edit menu"    
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(2)
        hMenuSelectNr(8)
    
    'open the report navigator        
    call fOpenReportNavigator
        
    '/// check if page header disappear in the report navigator (4 entries)
    printlog "check if page header disappear in the report navigator (4 entries)"    
    Kontext "ReportNavigator"        
        if (NavigatorTree.getItemCount <> 4) then
            warnlog "the page header is not displayed in the report navigator."
        endif

    'close the report navigator to get the focus back into the document        
    call fCloseReportNavigator        
        
    '/// turn on the page header via the edit menu
    printlog "turn on the page header via the edit menu"    
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(2)
        hMenuSelectNr(8)
    
    'open the report navigator        
    call fOpenReportNavigator    
        
    '/// check if page header appear in the report navigator (6 entries)
    printlog "check if page header appear in the report navigator (6 entries)"
    Kontext "ReportNavigator"        
        if (NavigatorTree.getItemCount <> 6) then
            warnlog "the page header is displayed in the report navigator."
        endif        

    'close the report navigator        
    call fCloseReportNavigator    
        
    '/// close the report designer
    printlog "close the report designer"
    call fCloseReportDesign
    '/// close the database
    printlog "close the database"
    call fCloseDatabase

endcase
'-------------------------------------------------------------------------
testcase tEditReport

    '/// open Bibliography database    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    call fOpenNewReportDesign

    '/// select the first table in the content list box
    printlog "select the first table in the content list box"
    Kontext "ReportDataProperties"   
        Content.select 1
        Content.typeKeys("<RETURN>",true) ' important to leave the listbox
    
    Kontext "ReportDesign"
        ReportDesign.TypeKeys("<MOD1 TAB>",true)
        ReportDesign.TypeKeys("<MOD1 TAB>",true)
        
    Kontext "ReportAddField"
        ReportAddFieldList.select 1    
        ReportAddField.TypeKeys("<RETURN>",true)        
        
    'close the Add Field dialog to get the focus back to the design
    call fCloseAddFieldDialog()
        
    '/// save the report
    printlog "save the report"        
    Kontext "ReportDesign"
        ReportDesign.UseMenu            
        hMenuSelectNr(1)
        hMenuSelectNr(6)
       
    sleep(10)
    
    '/// insert a name into the save dialog
    printlog "insert a name into the save dialog"

    Kontext "FormSaveDialog" ' report and forms use the same dialog 
        FormName.setText("Report1")
        SaveBtn.Click

    '/// close the report design
    printlog "close the report design"
    call fCloseReportDesign
        
    '/// open the report again in the edit mode
    printlog "open the report again in the edit mode"
    call fOpenReportInDesign("Report1")
    
    Kontext "ReportDesign"
        if (ReportDesign.exists(10)) then
            call fCloseReportDesign
        else
            warnlog "ReportDesign not open"
        endif    
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tFormatPage
    
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
        
    printlog "open the report designer"
    call fOpenNewReportDesign
    
    printlog "open format/page dialog via the menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(5)
        hMenuSelectNr(3) 
     
    printlog "check if the format page dialog appear"    
    Kontext "TabSeite"
        if ( not TabSeite.exists(3)) then
            warnlog "format page dialog doesn't appear"
            call fCloseReportDesign    
            call fCloseDatabase
            goto endsub
        else   
            printlog "format page dialog appear"
            call DialogTest(TabSeite)
            TabSeite.Cancel
        endif    
    
    '/// close the report designer
    call fCloseReportDesign
    '/// close the database
    call fCloseDatabase

endcase
'-------------------------------------------------------------------------
testcase tFunctionWizard
    
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
        
    printlog "open the report designer"
    call fOpenNewReportDesign

    printlog "select the first table in the content list box"
    Kontext "ReportDataProperties"   
        Content.select 1
        Content.typeKeys("<RETURN>",true) ' important to leave the listbox
    
    Kontext "ReportDesign"
        ReportDesign.TypeKeys("<MOD1 TAB>",true)
        ReportDesign.TypeKeys("<MOD1 TAB>",true)
        
    Kontext "ReportAddField"
        ReportAddFieldList.select 1    
        ReportAddField.TypeKeys("<RETURN>",true)        
        
    'close the Add Field dialog to get the focus back to the design
    call fCloseAddFieldDialog()
    
    'press 2 time tab to select the edit field
    Kontext "ReportDesign"
        ReportDesign.TypeKeys("<TAB>",true)
        ReportDesign.TypeKeys("<TAB>",true)
    
    sleep(1)
    
    printlog "click on the ... button behind the datafield property in the property browser"    
    Kontext "ReportDataProperties"
        OpenFormularWizard.Click
    
    printlog "check if the function wizard appear"    
    Kontext "FunctionWizard"
        if(FunctionWizard.exists(5)) then
            printlog "The function wizard appear -> OK"
            FunctionWizard.close
        else
            warnlog "The function wizard does not appear -> FAILED"
        endif
    
    sleep(1)        
        
    printlog "close the report designer"
    call fCloseReportDesign
    printlog "close the database"
    call fCloseDatabase

endcase
