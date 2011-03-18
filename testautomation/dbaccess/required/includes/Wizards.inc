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
'* owner : marc.neumann@oracle.com
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
