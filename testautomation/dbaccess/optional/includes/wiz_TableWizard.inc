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
'* short description : Table Wizard
'*
'\***********************************************************************
sub wiz_TableWizard
    
    call tNewTable        
    call tStartFormWizardFromTableWizard
    call tCreateAllTables   

    call tCheckSampleTables
    call tChangeFieldName

end sub
'-------------------------------------------------------------------------
testcase tNewTable  

    if bAsianLan = true then
        qaerrorlog "#i62665# Table wizard doesn't work in CJK versions. So some test are left out."
        goto endsub
    endif

    if (iSprache = 7) then
        qaerrorlog "due to issue i94730 this testcase does not work under russian."
        goto endsub
    endif 
    
    'hold a table name
    Dim sTableName as String 

    '/// open Bibliography database
    printlog "open Bibliography database"
    if not fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb")) then
        warnlog "Database " + gOfficePath + ConvertPath("user/database/biblio.odb") + " could not be open."
        goto endsub
    end if

    '/// start the table wizard
    printlog "start the table wizard"
    if not fStartTableWizard() then
        warnlog "The table wizard doesn't start. TEST STOPPED"
        goto endsub
    endif

    Kontext "TableWizard"
        '/// add all fields from the displayed table
        printlog "add all fields from the displayed table"
        AddAll.click
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
             
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click

        'remember the table, because we want to delete it later
        sTableName = TableName.getText                    
        '/// click Finish
        printlog "click Finish"       
        FinishBtn.Click
    
    sleep(1)

    '/// close the open table
    printlog "close the open table"
    call fCloseTableView()
    
    '/// delete the created table
    printlog "delete the created table"
    call fDeleteTable sTableName

    '/// close the database
    printlog "close the database"
    call fClosedatabase()
    
endcase   
'-------------------------------------------------------------------------
testcase tCheckSampleTables

    dim iTablesCount as Integer
    dim iFieldsCount as Integer
    dim i as Integer
    dim ii as Integer        

    '/// open Bibliography database
    printlog "open Bibliography database"
    if not fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb")) then
        warnlog "Database " + gOfficePath + ConvertPath("user/database/biblio.odb") + " could not be open."
        goto endsub
    end if

    '/// start the table wizard
    printlog "start the table wizard"
    if not fStartTableWizard() then
        warnlog "The table wizard doesn't start. TEST STOPPED"
        goto endsub
    endif
    
    Kontext "TableWizard"        
        '/// check if there are 22 table in the business category
        printlog "check if there are 22 table in the business category"
        iTablesCount = Tables.GetItemCount()
        if ( iTablesCount <> 22 ) then
            warnlog "the number of tables in the business category should be 22 but it is : " + iTablesCount
        endif

        '/// check the count of all fields in all table in the business category
        printlog "check the count of all fields in all table in the business category"
        for i = 1 to iTablesCount 
            Tables.select i
            'printlog Tables.getseltext
            iFieldsCount = iFieldsCount + Fields.getItemCount
	            for ii = 1 to Fields.getItemCount
					Fields.select ii
	            	'printlog "          " + Fields.getSelText
	            next
        next

        if ( iFieldsCount <> 318 AND iFieldsCount <> 290 ) then
            warnlog "the number of all fields should be 318 or 290 but it is : " + iFieldsCount
        endif
    
    Kontext "TableWizard"
        '/// check the personal radio button
        Personal.Check            
        '/// check if there are 15 table in the personal category
        printlog "check if there are 15 table in the personal category"
        iTablesCount = Tables.GetItemCount()
        if ( iTablesCount <> 15 ) then
            warnlog "the number of tables in the personal category should be 15 but it is : " + iTablesCount
        endif

        '/// check the count of all fields in all table in the personal category
        printlog "check the count of all fields in all table in the personal category"
        for i = 1 to iTablesCount 
            Tables.select i
            'printlog Tables.getseltext
            iFieldsCount = iFieldsCount + Fields.getItemCount
	            for ii = 1 to Fields.getItemCount
					Fields.select ii
	            	'printlog "          " + Fields.getSelText
	            next
        next

        if ( iFieldsCount <> 480 AND iFieldsCount <> 508 ) then            
            warnlog "the number of all fields should be 480 or 508 but it is : " + iFieldsCount
        endif
        if (iFieldsCount = 480) then
            qaerrorlog "There have to be 508 fields. Please contact msc."
        endif
            
        '/// click CANCEL
        printlog "click CANCEL"       
        CancelBtn.Click
    
        '///close the database
        printlog "close the database"
        call fClosedatabase()
    
endcase   
'-------------------------------------------------------------------------
testcase tChangeFieldName

    if (iSprache = 7) then
        qaerrorlog "due to issue i94730 this testcase does not work under russian."
        goto endsub
    endif

    '/// open Bibliography database
    printlog "open Bibliography database"
    if not fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb")) then
        warnlog "Database " + gOfficePath + ConvertPath("user/database/biblio.odb") + " could not be open."
        goto endsub
    end if

    '/// delete the table t_wizard if it's allready exists
    printlog "delete the table t_wizard if it's allready exists"
    call fDeleteTable ("t_wizard")

    '/// start the table wizard
    printlog "start the table wizard"
    if not fStartTableWizard() then
        warnlog "The table wizard doesn't start. TEST STOPPED"
        goto endsub
    endif
    
    Kontext "TableWizard"
        '/// select the first field
        printlog "select the first field"
        Fields.Select 1
        '/// click the add button
        printlog "click the add button"
        Add.click        
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click

        '/// select the first field
        printlog "select the first field"		
        FieldNames.select 1
        '/// change the field name for the first field to field_test
        FieldName.setText("field_test")              
        '/// click the add fields button (the + sign)
        printlog "click the add fields button (the + sign)"                 
        FieldsAdd.Click()
        '/// select the second field
        printlog "select the second field"
        FieldNames.select 2
        '/// change the field name for the second field to field_2
        printlog "change the field name for the second field to field_2"
        FieldName.setText("field_2")
        '/// click on field move up button to move the second filed to the first position
        printlog "click on field move up button to move the second filed to the first position"        
        FieldsUp.click()        
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click

        '/// change table name to t_wizard
        printlog "change table name to t_wizard"
        TableName.settext "t_wizard"            
        '/// click Finish
        printlog "click Finish"
        FinishBtn.Click
    
    sleep(1)

    '/// close the table view
    printlog "close the table view"
    call fCloseTableView()

    '/// open the table t_wizard in the design view
    printlog "open the table t_wizard in the design view"
    call fOpenTableInDesign("t_wizard")

    Kontext "TableDesignTable"
        '/// check if the first field is field_2
        printlog "check if the first field is field_2"
        Dim s as String
        s = Fieldname.getText
        if s <> "field_2" then
            printlog "the field name of the first field is not 'field_2' it is '" + s + "'."
            warnlog "the renaming or moving of a field in the wizard doesn't work"
        else
            printlog "==>> the first field name is field_2 => OK"            
        endif
        '/// check the second field
        TableDesignTable.typeKeys("<DOWN>")
        s = Fieldname.getText
        if s <> "field_test" then
            printlog "the field name of the second field is not 'field_test' it is '" + s + "'."
            warnlog "the renaming or moving of a field in the wizard doesn't work"
        else
            printlog "==>> the second field name is field_test => OK"            
        endif
        
    '/// close the table design
    printlog "close the table design"
    call fCloseTableDesign

    '/// delete the table t_wizard we dont't need it anymore
    printlog "delete the table t_wizard we dont't need it anymore"
    call fDeleteTable ("t_wizard")

    '/// close the database
    printlog "close the database"
    call fClosedatabase()
    
endcase   
'-------------------------------------------------------------------------
testcase tStartFormWizardFromTableWizard
    
    if bAsianLan = true then
        qaerrorlog "#i62665# Table wizard doesn't work in CJK versions. So some test are left out."
        goto endsub
    endif
    
    if (iSprache = 7) then
        qaerrorlog "due to issue i94730 this testcase does not work under russian."
        goto endsub
    endif
    
    'hold a table name
    Dim sTableName as String 

    '/// open Bibliography database
    printlog "open Bibliography database"
    if not fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb")) then
        warnlog "Database " + gOfficePath + ConvertPath("user/database/biblio.odb") + " could not be open."
        goto endsub
    end if

    '/// start the table wizard
    printlog "start the table wizard"
    if not fStartTableWizard() then
        warnlog "The table wizard doesn't start. TEST STOPPED"
        goto endsub
    endif

    Kontext "TableWizard"
        '/// add all fields from the displayed table
        printlog "add all fields from the displayed table"
        AddAll.click
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
             
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click

        'remember the table, because we want to delete it later
        sTableName = TableName.getText  
                  
        CreateForm.Check

        '/// click Finish
        printlog "click Finish"       
        FinishBtn.Click
    
    sleep(10)
    
    '/// check if the form wizard appear
    printlog "check if the form wizard appear"
    Kontext "FormWizard"
        if FormWizard.exists(1) then
            printlog "The formwizard appear ==>> OK"
            CancelBtn.Click
        else
            warnlog "The Form Wizard doesn't appear."
        endif
    
    '/// delete the created table
    printlog "delete the created table"
    call fDeleteTable sTableName

    '/// close the database
    printlog "close the database"
    call fClosedatabase()
    
endcase
'-------------------------------------------------------------------------
testcase tCreateAllTables

    if bAsianLan = true then
        qaerrorlog "#i62665# Table wizard doesn't work in CJK versions. So some test are left out."
        goto endsub
    endif

    '/// create all tables form the business and personal category
    printlog "create all tables form the business and personal category"

    Dim i as integer 

    'copy the hsqldb database file to a local path    
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")

    '/// open a hsql database
    printlog "open a hsql database"
    call fOpendatabase(ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb"))
    sleep(5)  

    '/// check if there are 22 items in the business categoriy 
    if not fStartTableWizard() then
        warnlog "The table wizard doesn't start. TEST STOPPED"
        goto endsub
    endif    

    Kontext "TableWizard"
    dim iCount as integer
    iCount = Tables.GetItemCount
    if(iCount <> 22 ) then
        warnlog "#102019# there are not 22 table in the category business. There are " + iCount
        CancelBtn.Click
        call fCloseDatabase()
        goto endsub
    endif
    CancelBtn.Click        
    
    for i = 1 to 22

        '/// start the table wizard
        printlog "start the table wizard"
        if not fStartTableWizard() then
            warnlog "The table wizard doesn't start. TEST STOPPED"
            goto endsub
        endif    

        Kontext "TableWizard"
        Tables.select i
        dim sTableName as String
        sTableName = Tables.getSelText
        printlog "Create table " + sTableName 
        '/// add all fields
        printlog "   add all fields"
        AddAll.click
        '/// click Finish
        printlog "   click Finish"       
        FinishBtn.Click
        
        Kontext "MessageBox"
        if ( MessageBox.exists(3) ) then
            warnlog "error while creating table: " + sTableName
            MessageBox.OK
            Kontext "TableWizard"
                CancelBtn.Click
        else
            '/// close the open table
            printlog "   close the open table"
            call fCloseTableView()
        endif
    next
    
    '/// now create all tables from the personal tables
    printlog "now create all tables from the personal tables"
    
    if not fStartTableWizard() then
        warnlog "The table wizard doesn't start. TEST STOPPED"
        goto endsub
    endif  
    
    Kontext "TableWizard"            
        Personal.Check 
        iCount = Tables.GetItemCount
        if(iCount <> 15 ) then
            warnlog "#102019# there are not 15 table in the category business. There are " + iCount
            CancelBtn.Click
            call fCloseDatabase()
            goto endsub
        endif
        CancelBtn.Click
    
    for i = 1 to 15
        '/// start the table wizard
        printlog "start the table wizard"
        if not fStartTableWizard() then
            warnlog "The table wizard doesn't start. TEST STOPPED"
            goto endsub
        endif    
        Kontext "TableWizard"        
        Personal.Check 
        Tables.select i        
        sTableName = Tables.getSelText
        printlog "Create table " + sTableName
        '/// add all fields
        printlog "   add all fields"
        AddAll.click                            
        '/// click Finish
        printlog "   click Finish"       
        FinishBtn.Click

        Kontext "MessageBox"
        if ( MessageBox.exists(3) ) then
            warnlog "error while creating table: " + sTableName
            MessageBox.OK
            Kontext "TableWizard"
                CancelBtn.Click
        else
            '/// close the open table
            printlog "   close the open table"
            call fCloseTableView()
        endif
    next    
    sleep(1)

    '/// close the database
    printlog "close the database"
    call fClosedatabase()

endcase  
