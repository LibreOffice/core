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
'* short description : test the Main Wizard Application Window
'*
'************************************************************************
'*
' #1 tQueryWizard
' #1 tReportWizard
' #1 tTableWizard
' #1 tFormWizard
'*
'\***********************************************************************************
sub Wizards
    
    printlog "------------------ Wizards.inc ---------------------"
   
    call tQueryWizard  
    call tReportWizard
    call tFormWizard
    call tTableWizard

end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tQueryWizard

    hFileOpen( gOfficePath & "user/database/biblio.odb" )
        
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)
    
    call fStartQueryWizard
    
    sleep(5)
    
    Kontext "QueryWizard"
        Dialogtest(QueryWizard)
        sleep(1)
        
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
        Dialogtest(QueryWizard)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)            
        Dialogtest(QueryWizard)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        Dialogtest(QueryWizard)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        Dialogtest(QueryWizard)
        '/// click CANCEL
        printlog "click CANCEL"       
        CancelBtn.Click
    sleep(1)
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tReportWizard

    hFileOpen( gOfficePath & "user/database/biblio.odb" )
        
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)
    
    StartReportWizard
    
    sleep(5)
    
    Kontext "ReportWizard"
        Dialogtest(ReportWizard)
        sleep(1)
        Tables.select 1
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
        Dialogtest(ReportWizard)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)            
        Dialogtest(ReportWizard)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        Dialogtest(ReportWizard)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)    
        Dialogtest(ReportWizard)
        '/// click CANCEL
        printlog "click CANCEL"       
        CancelBtn.Click
    sleep(1)
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tTableWizard

    hFileOpen( gOfficePath & "user/database/biblio.odb" )
        
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)

    ViewTables

    sleep(1)    

    StartTableWizard
    
    sleep(5)
    
    Kontext "TableWizard"
        Dialogtest(TableWizard)
        sleep(1)        
        AddAll.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        Dialogtest(TableWizard)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        Dialogtest(TableWizard)        
        '/// click CANCEL
        printlog "click CANCEL"       
        CancelBtn.Click
    sleep(1)
    call fCloseDatabase

endcase
'-------------------------------------------------------------------------
testcase tFormWizard
        
    hFileOpen( gOfficePath & "user/database/biblio.odb" )
        
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)
    
    StartFormWizard
    
    sleep(5)
    
    Kontext "FormWizard"
        Dialogtest(FormWizard)
        sleep(1)
        TablesOrQueriesMaster.select 1
        '/// add the third field
        printlog "add the third field"
        AvailableMasterFields.select 3
        sleep(1)
        MasterMoveSelected.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        Dialogtest(FormWizard)        
        '/// click CANCEL
        printlog "click CANCEL"       
        CancelBtn.Click
    sleep(1)
    call fCloseDatabase

endcase
