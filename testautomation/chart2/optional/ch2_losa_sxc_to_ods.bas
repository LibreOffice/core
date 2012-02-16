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
'* short description : Load/Save test for chart files - Input: SXC, Output:ODS
'*
'\************************************************************************

Global gsFileFormat as String
Global gsDirectoryInSourcePath as String
Global gsOutputFileCustomExtension as String

sub main       
    gsFileFormat = "calc8"
            
    'Important for determination of input file source path
    '-> gtesttoolpath & "chart2\optional\input\" & gsDirectoryInSourcePath &"\"
    gsDirectoryInSourcePath = "sxc"
    
    'Custom output file extension. Can be different to result from hGetFilternameExtension.
    'e.g. "_excel95.xls"
    gsOutputFileCustomExtension = ".ods"        

    Printlog "--------------------------------------------------"
    Printlog "---       Load charts in SXC - SaveAS ODS      ---"
    Printlog "--------------------------------------------------"

    use "chart2/optional/includes/loadsave/ch2_losa.inc"
    use "chart2/tools/ch_tools_common.inc"
    
    Call hStatusIn("Chart2", "ch2_losa_sxc_to_ods.bas")       
        Call subLoSaFromTo   
    Call hStatusOut                                 
end sub
'
'-------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication   = "CALC"
    Call GetUseFiles
end sub
