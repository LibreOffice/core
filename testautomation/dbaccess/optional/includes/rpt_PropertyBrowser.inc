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
'* short description : Property Browser
'*
'\***********************************************************************************
sub rpt_PropertyBrowser

    printlog "------------------ rpt_PropertyBrowser.inc ---------------------"
    
    call tDefaultStartup    
    
end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tDefaultStartup

    dim iCount as Integer

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign
    
    sleep(1)
    
    '/// check if the property browser is open with the data tabpage
    printlog "check if the property browser is open with the content tab"
    
    Kontext "ReportDataProperties"
    if ( ReportDataProperties.isVisible() = true ) then
        printlog "The property browser start with the data tabpage."
    else
        warnlog "#i77774# The property browser does not start with the datatab page"
    endif
        
    '/// close the report designer
    printlog "close the report designer"
    call fCloseReportDesign
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
