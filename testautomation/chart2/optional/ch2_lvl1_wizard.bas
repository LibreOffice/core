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
'* short description : Level 1 functional test for Chart2 Wizard
'*
'\************************************************************************************************

sub main
    use "chart2\tools\ch_tools_common.inc"
    use "chart2\tools\ch_tools_chart_type.inc"
    use "chart2\tools\ch_tools_titles.inc"
    use "chart2\tools\ch_tools_axes.inc"
    use "chart2\tools\ch_tools_grids.inc"
    use "chart2\tools\ch_tools_legend.inc"
    use "chart2\tools\ch_tools_data_labels.inc"
    use "chart2\tools\ch_tools_statistics.inc"
    use "chart2\tools\ch_tools_tab_pages.inc"
    use "chart2\tools\ch_tools_select.inc"    
    use "chart2\optional\includes\wizard\ch2_lvl1_wizard.inc"
	use "chart2\optional\includes\wizard\ch2_lvl1_wizard2.inc"
    
    Call hStatusIn ( "Chart2", "ch2_lvl1_wizard.bas" )
    Call tCreateNew3DChart
    Call tCreateNewLineChart
	Call tCreateNewBubbleChart
	Call tCreateNewFilledNetChart
    Call hStatusOut		
	
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_set_standard_controls.inc"
    Call GetUseFiles     
    gApplication = "CALC"
end sub

