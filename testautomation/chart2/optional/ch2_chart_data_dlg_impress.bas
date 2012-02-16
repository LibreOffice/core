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
'* short description : Resource Test for the Chart Data Dialogue in Impress.
'*
'\************************************************************************************************

sub main
    use "chart2\optional\includes\ch2_datadialogue.inc"
    
    Call hStatusIn("Chart2", "ch2_chart_data_dlg_impress.bas")
        printlog "--------------------------------------------"
        printlog "C h a r t   D a t a   D i a l o g Updatetest"
        printlog "--------------------------------------------"

        printLog "-----------  tChartDataDialogue  -----------"
        call tChartDataDialogueButtons
        call tChartDataDialogueFieldsUndo
    Call hStatusOut  
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
     use "global\system\includes\master.inc"
     use "global\system\includes\gvariabl.inc"
     Call GetUseFiles 
     gApplication = "IMPRESS"
end sub

