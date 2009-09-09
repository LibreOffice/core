'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2009 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: ch2_lvl1a.inc,v $
'*
'* $Revision: 1.0 $
'*
'* last change: $Author: hde $ $Date: 2009-04-14 15:12:01 $
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
'* owner : oliver.craemer@sun.com
'*
'* short description : Chart2 functional tests
'*
'\************************************************************************

sub ch2_lvl1a

	Call tPlotOptions

end sub


testcase tPlotOptions
	
	Dim sOutputFile as string
	sOutputFile = convertpath(gOfficepath & "user\work\hiddenCells.ods")

	printlog "Load simple chart document"
	if fLoadVerySimpleChartAndSaveLocal() > 0 then
		warnlog "Loading test document seems to have failed -> Check this out!"
		goto endsub
	endif    
	printlog "Select chart using navigator"
	if fSelectFirstOLE = -1 then
		warnlog "It was not possible to select the chart object!"
		Call hCloseDocument
		goto endsub
	endif	

	printlog "Edit / Object / Edit"
	EditObjectEdit        

	printlog "Move to third object in Chart using TAB key (data series)"
	Kontext "DocumentChart"
	DocumentChart.TypeKeys "<TAB>" , 3        

	printlog "Invoke Format::ObjectProperties"
	FormatObjectProperties        

	printlog "Select tab page Options"
	Kontext
	Active.SetPage TabDataSeriesOptions
	Kontext "TabDataSeriesOptions"
	printlog "activate checkbox 'Include values from hidden cells'"
	try
		IncludeHiddenCells.Check
	catch
		Warnlog "checkbox hidden or disabled"
		Call hCloseDocument
		goto endsub
	endcatch
		
	printlog "Close and reopen data series dialog - checkbox should be still activated"
	TabDataSeriesOptions.Ok	
	FormatObjectProperties        
	printlog "Select tab page Options"
	Kontext
	Active.SetPage TabDataSeriesOptions
	Kontext "TabDataSeriesOptions"
	if Not IncludeHiddenCells.IsChecked then
		Warnlog "After closing and reopening of Data-Series dialog the checkbox isn't checked anymore"
		TabDataSeriesOptions.Cancel
		Kontext "DocumentCalc"
		DocumentCalc.TypeKeys "<Escape>"		
		call hCloseDocument
		goto endsub		
	endif

	TabDataSeriesOptions.Ok
	
	printlog "save and reopen file - checkbox still must be checked"
	if NOT hFileSaveAsKill (sOutputFile) then
		warnlog "Saving test document failed -> Aborting"
		Kontext "DocumentCalc"
		DocumentCalc.TypeKeys "<Escape>"		
		call hCloseDocument
		goto endsub
	endif    		
	Call hCloseDocument
	
    Call hFileOpen(sOutputFile)
	if fSelectFirstOLE = -1 then
		warnlog "It was not possible to select the chart object!"
		Call hCloseDocument
		goto endsub
	endif	


	EditObjectEdit        
	Kontext "DocumentChart"
	DocumentChart.TypeKeys "<TAB>" , 3        
	printlog "Invoke Format::ObjectProperties"
	FormatObjectProperties        	
	Kontext
	Active.SetPage TabDataSeriesOptions
	Kontext "TabDataSeriesOptions"
	if Not IncludeHiddenCells.IsChecked then
		Warnlog "After closing and reopening of Data-Series dialog the checkbox isn't checked anymore"
		TabDataSeriesOptions.Cancel
		Kontext "DocumentCalc"
		DocumentCalc.TypeKeys "<Escape>"		
		call hCloseDocument
		goto endsub		
	endif
	
	'/// Close dialog with Cancel-button
	TabDataSeriesOptions.Cancel
	'/// Close document
	Kontext "DocumentCalc"
	DocumentCalc.TypeKeys "<Escape>"

	Call hCloseDocument
endcase
