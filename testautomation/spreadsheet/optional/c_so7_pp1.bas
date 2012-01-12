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
'* short description : Test bug fixes made for StarOffice 7 PP1 - srx645_m22s1-5  
'*
'\***********************************************************************

sub main

PrintLog "---------------------------------------------------------------------"
Printlog "-----                    Calc SO7 PP1 Bug Fix Check             -----"
PrintLog "---------------------------------------------------------------------"

   use "spreadsheet\optional\includes\so7pp1\c_so7_pp1_iz.inc"
   use "spreadsheet\optional\includes\so7pp1\c_so7_pp1_ibis.inc"
   use "spreadsheet\tools\includes\c_select_tools.inc"
   
   Call hStatusIn("spreadsheet", "c_so7_pp1.bas", "Test bug fixes made for StarOffice 7 PP1")
 
        Printlog Chr(13) + "--------- Izzuezilla Bugs fixed? ---------"

        call tIZ19381
        call tIZ21036
          
        Printlog Chr(13) + "--------- IBIS Bugs fixed? ---------"

        call tIBIS111099
        call tIBIS111158
        call tIBIS111711

    call hStatusOut       

end sub

sub LoadIncludeFiles

   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   gApplication   = "CALC"
   Call GetUseFiles

End Sub

