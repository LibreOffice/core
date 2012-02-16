'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

