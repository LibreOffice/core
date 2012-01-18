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
'* short description : Query Autopilot Test
'* preconditions : Adabas has to be installed on the test machine
'* testdocuments : no
'* testpurpose : Test for the query wizard 
'*
'\***********************************************************************
sub wiz_QueryWizard



    if fCreateDbaseDatasource(gOfficePath + ConvertPath("user/work/tt_dbase-01.odb"),gOfficePath + ConvertPath("user/database/biblio")) then            
        call tQueryAutopilotMain
    else
        qaerrorlog "can't create dBase datasource. Stop Test"	
	endif
    
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")
    call tQueryAutopilotTest1
    call tQueryAutopilotTest2
        
end sub
'-------------------------------------------------------------------------
testcase tQueryAutopilotMain
       
	if not fOpendatabase(gOfficePath + ConvertPath("user/work/tt_dbase-01.odb")) then
        warnlog "Database " +gOfficePath + ConvertPath("user/work/tt_dbase-01.odb") + " could not be open."
        goto endsub
    end if
    
    if not fStartQueryWizard() then
        warnlog "The Query Wizard could not be start."
        goto endsub
    end if	

    Kontext "QueryWizard"
        '/// select the first table (biblio)
        printlog "select the first table (biblio)"
        Tables.select 1
        sleep(1)
        '/// add the third field
        printlog "add the third field"
        AvailableFields.select 3
        sleep(1)
        Add.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)            
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        '/// click FINISH
        printlog "click FINISH"
        FinishBtn.click
        sleep(2)
    
    Kontext "DatabaseBeamer"
        DatabaseBeamer.UseMenu
        call hMenuSelectnr(1)
        call hMenuSelectnr(4)
        'FileClose
        
    call fCloseDatabase(true)
       
endcase
'----------------------------------------------------------
testcase tQueryAutopilotTest1

  	if not fOpendatabase(gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")) then
        warnlog "Database " + gOfficePath + ConvertPath("user/work/TT_hsqldb.odb") + " could not be open."
        goto endsub
    end if
    
    if not fStartQueryWizard() then
        warnlog "The Query Wizard could not be start."
        goto endsub
    end if	

    Kontext "QueryWizard"
        '/// select the table TT_QueryAutopilot
        printlog "select the table TT_QueryAutopilot"        
        Tables.select 7
        sleep(1)
        '/// add the third field
        printlog "add the 1. and 4. field"
        AvailableFields.select 1
        sleep(1)
        Add.click
        sleep(1)
        AvailableFields.select 4
        sleep(1)
        Add.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)            
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        '/// click FINISH
        printlog "click FINISH"
        FinishBtn.click
        sleep(2)
    
    Kontext "DatabaseBeamer"
        FileClose
        
    call fClosedatabase(true)
    
endcase
'----------------------------------------------------------
testcase tQueryAutopilotTest2

  	if not fOpendatabase(gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")) then
        warnlog "Database " + gOfficePath + ConvertPath("user/work/TT_hsqldb.odb") + " could not be open."
        goto endsub
    end if
    
    if not fStartQueryWizard() then
        warnlog "The Query Wizard could not be start."
        goto endsub
    end if
	
    Kontext "QueryWizard"
        '/// select the table TT_QueryAutopilot
        printlog "select the table TT_QueryAutopilot"        
        Tables.select 7
        sleep(1)
        '/// add the third field
        printlog "add the 1. field"
        AvailableFields.select 1
        sleep(1)
        Add.click
        sleep(1)
        '/// click NEXT to Sorting page
        printlog "click NEXT to Sorting page"        
        NextBtn.click
        sleep(1)    
        '/// click NEXT to Search page
        printlog "click NEXT to Search page"        
        NextBtn.click
        sleep(1)            
        '/// click NEXT to summary query page
        printlog "click NEXT to summary query page"
        NextBtn.click
        sleep(1)
        SummaryQuery.Check
        sleep(3)
        ' this doesn't work        
        'AggregatFunction1.select 1
        'sleep(1)
        'AggregatField1.select 1
        'sleep(1)
        ' do this instead
        QueryWizard.TypeKeys "<SHIFT TAB>" , true
        sleep(1)
        QueryWizard.TypeKeys "<SHIFT TAB>" , true
        sleep(1)
        QueryWizard.TypeKeys "<DOWN>" , true
        sleep(1)
        QueryWizard.TypeKeys "<TAB>" , true
        sleep(1)
        QueryWizard.TypeKeys "<DOWN>" , true
        sleep(1)
        '----                
        '/// click NEXT to alias page
        printlog "click NEXT to group alias page"
        NextBtn.click
        sleep(1)
        '/// click NEXT to final page
        printlog "click NEXT to final page"
        NextBtn.click
        sleep(1)
        '/// click FINISH
        printlog "click FINISH"
        FinishBtn.click
        sleep(2)
    
    Kontext "DatabaseBeamer"
        FileClose
    
    call fClosedatabase(true)
    
endcase
