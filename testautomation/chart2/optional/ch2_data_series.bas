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
'*
'/************************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Function test for Data Series functions in chart
'*
'\************************************************************************************************

sub main
    use "chart2\tools\ch_tools_common.inc"
    use "chart2\tools\ch_tools_chart_type.inc"
    use "chart2\tools\ch_tools_select.inc"
    use "chart2\optional\includes\ch2_lvl1_data_series1.inc"
    
    Call hStatusIn("Chart2", "ch2_data_series.bas")

        printlog "----------------------------------------"
        printlog "   C h a r t 2   D a t a   S e r i e s  "        
        printlog "----------------------------------------"
        printLog Chr(13) + "-----------  Options  ------------"
        Call tPlotMissingValuesBar
        Call tPlotMissingValuesLine        
   Call hStatusOut  
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global\tools\includes\optional\t_set_standard_controls.inc"         
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles 
    gApplication = "CALC"
end sub

