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
'* short description : Updatetest for Flexible Source Range Selection 
'*
'\***********************************************************************

sub main

    use "chart2\optional\includes\ch2_flexible_source_range_selection.inc"
    use "chart2\optional\includes\ch2_flexible_source_range_selection01.inc"

    Call hStatusIn("Chart2", "ch2_flexible_source_range_selection.bas")    
        printlog "----------------------------------------------"
        printlog "Updatetest for Flexible Source Range Selection"        
        printlog "----------------------------------------------"            
        printlog "------------ Chart in Spreadsheet ------------"            
        gApplication = "CALC"    
            Call tDataRangeWizardCalc
            Call tDataSeriesWizardCalc
            Call tDataRangeDialogCalc
            Call tDataSeriesDialogCalc    
        printlog "-------------- Chart in Writer ---------------"            
        gApplication = "WRITER"    
            Call tDataRangeWizardWriter
            Call tDataSeriesWizardWriter
            Call tDataRangeDialogWriter
            Call tDataSeriesDialogWriter
    Call hStatusOut
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles    
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"          

    use "chart2\tools\ch_tools_common.inc"
    use "chart2\tools\ch_tools_chart_type.inc"

    Call GetUseFiles
    gApplication = "CALC"    
end sub
