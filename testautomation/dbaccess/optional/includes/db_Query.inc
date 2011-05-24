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
'* short description : Query Test Library
'*
'\***********************************************************************
'TODO: testcase for distinct values
function db_Query( sFileName , optional sType , optional sPassword)
    
    if isMissing (sType) then
        sType = "1"
    else
        printlog sType    
        sType = UCase(sType)
        printlog sType
        select case sType
            case "TEXT", "CALC"
                sType = "1"
            case "ODBC", "JDBC"
                sType = "2"
            case "HSQLDB"
                sType = "2"
            case "MYSQL_JDBC"
            case "MYSQL_NATIVE"
                sType = "3"
            case else
                sType = "1"
        end select
    endif 

    call tQueryCriterion( sFileName , sType , sPassword)
    
    if sType <> "1" then 'run only with database from type >1  
        call tQueryJoin( sFileName , sType, sPassword )
        call tQueriesInQueries(sFileName)
    endif

    if sType = "3" then 'run only with database type MYSQL_JDBC
        call tCastQuery( sFileName , sPassword )
    endif
    
    call tSortQueryWithAlias(sFileName , sPassword)
    
end function
'-------------------------------------------------------------------------
function tQueryJoin( sFileName , optional sType, optional sPassword )
'TODO: check Join Type ( LEFT , INNER etc bug i17390)     
    Dim SelEntry as string
    Dim sTableName as string
    
    sTableName = "TT_Query2"

    if IsMissing(sPassword) then
        if ( not fOpenDatabase(sFileName) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    else
        if ( not fOpenDatabase(sFileName,sPassword) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    endif
    
    fOpenNewQueryDesign

    Kontext "AddTables"
    'TODO: add a test to click on the execute button, before the add table dialog is closed.
            dim ix as integer 
            dim i as integer
            ix = TableName.getItemCount                
            for i = 1 to ix
                TableName.Select i
                TableName.TypeKeys "<ADD>"
                'printlog "ix = " + ix
                'printlog "TableName.getItemCount = " + TableName.getItemCount 
                if TableName.getItemCount > ix then
                    ix = TableName.getItemCount
                    'printlog "TableName.getItemCount > ix"
                endif
                'printlog "TableName.getSeltext = " + TableName.getSeltext
                if TableName.getSeltext = sTableName then
                    i = ix
                endif
            next
            if TableName.getSeltext <> sTableName then
                warnlog "Can find the table " + sTableName + "." + _
                        "Make sure that the table exists when starting this test."
                DatabaseClose                        
                sleep(5)
                Kontext "Messagebox"
                    if Messagebox.Exists(3) then
                        if Messagebox.getRT = 304 then
                            Messagebox.No                        
                        end if                        
                    end if                
                call hCloseDocument
                exit function
            endif
            printlog "- Add " + sTableName +" table to query design"
            '/// Add database table to Query.
            AddTable.Click
            '######## add table TT_Query2 #########
            sTableName = "TT_Query3"
            Kontext "AddTables"
            'TODO: add a test to click on the execute button, before the add table dialog is closed.                
            ix = TableName.getItemCount                
            for i = 1 to ix
                TableName.Select i
                TableName.TypeKeys "<ADD>"
                if TableName.getItemCount > ix then
                    ix = TableName.getItemCount
            
                endif                
                if TableName.getSeltext = sTableName then
                    i = ix
                endif
            next

            if TableName.getSeltext <> sTableName then
                warnlog "Can find the table " + sTableName + "." + _
                        "Make sure that the table exists when starting this test."
                exit function
            endif

            printlog "- Add " + sTableName +" table to query design"
            '/// Add database table to Query.
            AddTable.Click
            
            sleep(2) 
            if AddTables.exists(2) then 
                CloseDlg.Click ' When the AddTables dialog still exists then close him
            endif
            sleep(1)        
    
        Kontext "QueryDesignTable"        
            QueryDesignTable.useMenu
            sleep(1)
            hMenuSelectNr(4)
            sleep(1)      
            hMenuSelectNr(1)                         
            sleep(1)
    
        Kontext "JoinProperties"
            JoinProperties.TypeKeys "<TAB>" , true
            sleep(1)
            JoinProperties.TypeKeys "<TAB>" , true
            sleep(1)
            JoinProperties.TypeKeys "<TAB>" , true
            sleep(1)
            JoinProperties.TypeKeys "<TAB>" , true
            sleep(1)
            JoinProperties.TypeKeys "<MOD1 DOWN>" , true
            sleep(1)
            JoinProperties.TypeKeys "<TAB>" , true
            sleep(1)
            JoinProperties.TypeKeys "<MOD1 DOWN>" , true
            sleep(1)
            JoinProperties.OK
        
        Kontext "QueryDesignCriterion"
            sleep(1)            
            QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
            mywait()
            QueryDesignCriterion.TypeKeys "<HOME>" , true
            mywait()                
            QueryDesignCriterion.TypeKeys "<DOWN>" , 2 , true
            
            QueryDesignCriterion.TypeKeys "<MOD1 DOWN>" , true
            
            QueryDesignCriterion.TypeKeys "<UP>" , 2 , true
            
            QueryDesignCriterion.TypeKeys "<MOD1 UP>" , 3 , true
            
            QueryDesignCriterion.TypeKeys "<DOWN>" , 5 , true
            mywait()
            QueryDesignCriterion.TypeKeys "<MOD1 DOWN>" , 2 , true
            mywait()
            sleep(1)
        Kontext "Toolbar"
            ExecuteBtn.Click
    
        Kontext "TableView"
            if TableView.Exists(3) then
                
                    DataWindow.TypeKeys "<MOD1 C>"
                    if GetClipboard <> "4" then
                        warnlog "Wrong result, maybe the join doesn't work."
                    endif
            endif
        sleep(1)
		call fCloseQueryDesign
        
        call fCloseDatabase
   
end function
'-------------------------------------------------------------------------
function tQueryCriterion( sFileName , optional sType , optional sPassword)
    
    Dim SelEntry as string
    Dim sTableName as string
    
    sTableName = "TT_Query1"

    if IsMissing(sPassword) then
        if ( not fOpenDatabase(sFileName) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    else
        if ( not fOpenDatabase(sFileName,sPassword) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    endif
    
    fOpenNewQueryDesign
    
        
    Kontext "AddTables"
        'TODO: add a test to click on the execute button, before the add table dialog is closed.
        dim ix as integer 
        dim i as integer
        ix = TableName.getItemCount                
        for i = 1 to ix
            TableName.Select i
            TableName.TypeKeys "<ADD>"
            'printlog "ix = " + ix
            'printlog "TableName.getItemCount = " + TableName.getItemCount 
            if TableName.getItemCount > ix then
                ix = TableName.getItemCount
                'printlog "TableName.getItemCount > ix"
            endif
            'printlog "TableName.getSeltext = " + TableName.getSeltext
            if TableName.getSeltext = sTableName then
                i = ix
            endif
        next
        if TableName.getSeltext <> sTableName then
            warnlog "Can find the table " + sTableName + "." + _
                    "Make sure that the table exists when starting this test."
            DatabaseClose                        
            sleep(5)
            Kontext "Messagebox"
                if Messagebox.Exists(3) then
                    if Messagebox.getRT = 304 then
                        Messagebox.No                        
                    end if                        
                end if                
            call hCloseDocument
            exit function
        endif
        printlog "- Add " + sTableName +" table to query design"
        '/// Add database table to Query.
        AddTable.Click
        sleep(2) 
        if AddTables.exists(2) then 
            CloseDlg.Click ' When the AddTables dialog still exists then close him
        endif
        sleep(1)        
    Kontext "QueryDesignCriterion"                           
        Field.Select(1)
        sleep(1)                
    Kontext "Toolbar"
        '/// Executing query
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if NOT DataWindow.Exists(3) then
            warnlog "Execution of a query failed!"
        end if
    ' #### don't comment a test, because they depends on each other ####
    if sType <> "1" then
        '/// start the 1. test #############################
        printlog "start the 1. test #############################"            
        call tTest1            
        '/// start the 2. test
        printlog "start the 2. test"            
        call tTest2
        '/// start the 3. test
        printlog "start the 3. test"            
        call tTest3  
        '/// start the 4. test
        printlog "start the 4. test"            
        call tTest4
        '/// start the 5. test
        printlog "start the 5. test"            
        call tTest5          
        '/// start the 6. test
        printlog "start the 6. test"            
        call tTest6
        '/// start the 7. test
        printlog "start the 7. test"            
        call tTest7            
        '/// start the 8. test
        printlog "start the 8. test"            
        call tTest8            
        ' main test end ###############################
        printlog "main test end ###############################"
    endif
    fCloseQueryDesign
    fCloseDatabase    
    
end function
'-------------------------------------------------------------------------
function tQueriesInQueries( sFileName , optional sType , optional sPassword)
    
    Dim SelEntry as string
    Dim sTableName as string
    
    sTableName = "TT_Query1"

    if IsMissing(sPassword) then
        if ( not fOpenDatabase(sFileName) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    else
        if ( not fOpenDatabase(sFileName,sPassword) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    endif
    
    '/// open new query design
    printlog "open new query design"
    call fOpenNewQueryDesign
    
    '/// choose table TT_Query1 from AddTable dialog
    printlog "choose table TT_Query1 from AddTable dialog"
    call fChooseTableInQueryAddTableDialog("TT_Query1")
    
    '/// insert fields name1 and name2
    printlog "insert fields name1 and name2"
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<MOD1 UP>" , 7, true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<MOD1 UP>" , 6, true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        sleep(1)
    
    '/// save the query as Query_test_1
    printlog "save the query as Query_test_1"
    call fSaveQueryDesign("Query_test_1")

    '/// close the query design
    printlog "close the query design"
    call fCloseQueryDesign()
    
    '/// open new query design
    printlog "open new query design"    
    call fOpenNewQueryDesign
    
    '/// choose query Query_test_1 from AddTable dialog
    printlog "choose query Query_test_1 from AddTable dialog"
    call fChooseQueryInQueryAddTableDialog("Query_test_1")
    
    '/// insert fields name1
    printlog "insert fields name1"
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<MOD1 UP>" , 2, true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        sleep(1)
    
    
    '/// save the query with the name Query_test_2
    printlog "save the query with the name Query_test_2"
    call fSaveQueryDesign("Query_test_2")

    '/// close the query design
    printlog "close the query design"
    call fCloseQueryDesign()    
    
    '/// close the query design
    printlog "close the query design"
    call fCloseDatabase(true) 
    
    '/// open the database again
    printlog "open the database again"
    if IsMissing(sPassword) then
        if ( not fOpenDatabase(sFileName) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    else
        if ( not fOpenDatabase(sFileName,sPassword) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    endif
    
    '/// execute the QueryInQuery "Query_test_2"
    printlog "execute the QueryInQuery Query_test_2"
    if ( fOpenQuery("Query_test_2") = false ) then
        warnlog "Can't open query Query_test_2"
        call fCloseDatabase()
        exit function        
    endif
    
    '/// close Query view
    printlog "close Query view"
    call fCloseQueryView()
    
    '/// close database
    printlog "close database"
    call fCloseDatabase()

end function
'-------------------------------------------------------------------------
sub mywait()
    'sleep(1)
end sub

sub tTest1
    '/// insert the <b>* as field</b>
    '/// take <b>count</b> from the <b>function</b> listbox
    '/// <b>Execute</b> the query
    '/// the <b>result</b> should be <b>11</b>      
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<DOWN>" , 5 , true
        mywait()
        QueryDesignCriterion.TypeKeys "<MOD1 DOWN>" , true
        mywait()
        sleep(1)
    Kontext "Toolbar"                
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if TableView.Exists(3) then            
                DataWindow.TypeKeys "<MOD1 C>"
                if GetClipboard <> "11" then 
                    warnlog "There should be 11 records but there are " + GetClipboard 
                else
                    printlog GetClipboard + " records available."
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
        sleep(2)
end sub

sub tTest2
    '/// add the field <b>'name1'</b> to the field list
    '/// set the field <b>invisible</b>
    '/// as criterion insert <b>LIKE 'F*'</b>
    '/// <b>execute</b> the query
    '/// the <b>result</b> should be <b>3</b>         
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<TAB>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<SHIFT PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<MOD1 DOWN>" , 2 , true
        mywait()
        QueryDesignCriterion.TypeKeys "<DOWN>" , 4 , true
        mywait()
        QueryDesignCriterion.TypeKeys "<SPACE>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<DOWN>" , 2 , true
        mywait()
        QueryDesignCriterion.TypeKeys "LIKE 'F*'" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        sleep(1)
    Kontext "Toolbar"                
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if TableView.Exists(3) then
            
                DataWindow.TypeKeys "<MOD1 C>"
                if GetClipboard <> "3" then 
                    warnlog "There should be 3 records but there are " + GetClipboard 
                else
                    printlog GetClipboard + " records available."
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
    sleep(2)
end sub

sub tTest3
    '/// add the field <b>'date'</b> to the field list
    '/// set the field <b>invisible</b>
    '/// as criterion insert <b>IS NULL</b>
    '/// b>execute</b> the query
    '/// the <b>result</b> should be <b>1</b>
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<TAB>" , 2 , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<SHIFT PAGEUP>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<MOD1 DOWN>" , 7 , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<DOWN>" , 4 , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<SPACE>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<DOWN>" , 2 , true
        mywait()                
        QueryDesignCriterion.TypeKeys "IS NULL" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        'TODO: check if the right localazied string appear              
        sleep(1)            
    Kontext "Toolbar"                
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if TableView.Exists(3) then
            
                DataWindow.TypeKeys "<MOD1 C>"
                if GetClipboard <> "1" then 
                    warnlog "There should be 1 records but there are " + GetClipboard 
                else
                    printlog GetClipboard + " records available."
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
    sleep(2)
end sub

sub tTest4
    '/// <b>change</b> the criterion for field <b>date</b> to <b>is not null</b>
    '/// <b>execute</b> the query
    '/// the <b>result</b> should be <b>2</b>     
    Kontext "QueryDesignCriterion"
                    
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<TAB>" , 2 , true
        mywait()                                
        QueryDesignCriterion.TypeKeys "<DOWN>" , 6 , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<SHIFT END>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "IS NOT NULL" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        'TODO: check if the right localazied string appear                
        sleep(1)
    Kontext "Toolbar"                
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if TableView.Exists(3) then
            
                DataWindow.TypeKeys "<MOD1 C>"
                if GetClipboard <> "2" then 
                    warnlog "There should be 2 records but there are " + GetClipboard 
                else
                    printlog GetClipboard + " records available."
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
    sleep(2)
end sub

sub tTest5
    '/// <b>remove</b> the fields <b>'name1' and 'date'</b>
    '/// <b>add</b> field <b>ID</b>
    '/// set <b>ID invisible</b>
    '/// as <b>criterion</b> insert <b>BETWEEN 4 AND 7</b>
    '/// <b>execute</b> the query
    '/// the <b>result</b> should be <b>4</b>     
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<TAB>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<SHIFT END>" , true        
        mywait()
        QueryDesignCriterion.TypeKeys "<DELETE>" , true        
        mywait()
        QueryDesignCriterion.TypeKeys "<SHIFT PAGEUP>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<MOD1 DOWN>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<DOWN>" , 6 , true
        mywait()
        QueryDesignCriterion.TypeKeys "<SHIFT END>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<DELETE>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "BETWEEN 4 AND 7" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<SHIFT END>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<DELETE>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        'TODO: check if the right localazied string appear                
        sleep(1)
    Kontext "Toolbar"                
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if TableView.Exists(3) then
            
                DataWindow.TypeKeys "<MOD1 C>"
                if GetClipboard <> "4" then 
                    warnlog "There should be 4 records but there are " + GetClipboard 
                else
                    printlog GetClipboard + " records available."
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
    sleep(2)
end sub

sub tTest6
    '/// <b>change the criterion</b> for field ID to <b>NOT BETWEEN 4 AND 7</b>
    '/// <b>execute</b> the query
    '/// the <b>result</b> should be <b>7</b>
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<TAB>" , true
        mywait()                                
        QueryDesignCriterion.TypeKeys "<DOWN>" , 6 , true
        mywait()
        QueryDesignCriterion.TypeKeys "<SHIFT END>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "NOT BETWEEN 4 AND 7" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        'TODO: check if the right localazied string appear                
        sleep(1)
    Kontext "Toolbar"                
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if TableView.Exists(3) then
            
                DataWindow.TypeKeys "<MOD1 C>"
                if GetClipboard <> "7" then 
                    warnlog "There should be 7 records but there are " + GetClipboard 
                else
                    printlog GetClipboard + " records available."
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
    sleep(2)
end sub

sub tTest7
    '/// remove the field ID
    '/// <b>add</b> field <b>name1</b>
    '/// set name1 <b>invisible</b>
    '/// as <b>criterion</b> insert <b>IN ('Heinz','Freddi')</b>
    '/// <b>execute</b> the query
    '/// the <b>result</b> should be <b>2</b>
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<TAB>" , true
        mywait()   
        QueryDesignCriterion.TypeKeys "<SHIFT END>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<DELETE>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<SHIFT PAGEUP>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<MOD1 DOWN>" , 2 , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<DOWN>" , 6 , true
        mywait()
        QueryDesignCriterion.TypeKeys "<SHIFT END>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<DELETE>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "IN ('Heinz','Freddi')" , true
        mywait()
        sleep(1)
        'TODO: check if the right localazied string appear                
        sleep(1)
    Kontext "Toolbar"                
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if TableView.Exists(3) then
            
                DataWindow.TypeKeys "<MOD1 C>"
                if GetClipboard <> "2" then 
                    warnlog "There should be 2 records but there are " + GetClipboard 
                else
                    printlog GetClipboard + " records available."
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
    sleep(2)
end sub

sub tTest8
    '/// change <b>criterion</b> to <b>NOT IN ('Heinz','Freddi')</b>
    '/// <b>execute</b> the query
    '/// the <b>result</b> should be <b>9</b>            
    Kontext "QueryDesignCriterion"
        sleep(1)            
        printlog "Go to the top left field."
        QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<HOME>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<TAB>" , true
        mywait()                
        QueryDesignCriterion.TypeKeys "<DOWN>" , 6 , true
        mywait()        
        QueryDesignCriterion.TypeKeys "<SHIFT END>" , true
        mywait()                                
        QueryDesignCriterion.TypeKeys "NOT IN ('Heinz','Freddi')" , true
        mywait()
        QueryDesignCriterion.TypeKeys "<RETURN>" , true
        mywait()
        'TODO: check if the right localazied string appear                
        sleep(1)
    Kontext "Toolbar"                
        printlog "- Executing query"
        ExecuteBtn.Click
        sleep(5)
    Kontext "TableView"
        if TableView.Exists(3) then
            
                DataWindow.TypeKeys "<MOD1 C>"
                if GetClipboard <> "9" then 
                    warnlog "There should be 9 records but there are " + GetClipboard 
                else
                    printlog GetClipboard + " records available."
                endif
        else
            warnlog "Execution of a query failed!"
        end if  
    sleep(2)
end sub
'--------------------------------------------------------------------
sub tCastQuery( sFileName , sPassword )

    fOpenDatabase(sFileName,sPassword)

    if ( fOpenNewQueryDesign ) then
        if ( fChooseTableInQueryAddTableDialog("TT_Query3") ) then
            Kontext "QueryDesignCriterion"
                sleep(1)
                QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
                sleep(1)
                QueryDesignCriterion.TypeKeys "cast(f_date_s as date)" , true
                sleep(1)
                QueryDesignCriterion.TypeKeys "<RETURN>" , true
                sleep(1)
            Kontext "Toolbar"
                printlog "- Executing query"
                ExecuteBtn.Click
                sleep(5)

            Kontext "TableView"
                    if TableView.Exists(3) then

                            DataWindow.TypeKeys "<MOD1 C>"
                            if GetClipboard <> "40219" then 
                                warnlog "the result should be 40219 but it is " + GetClipboard 
                            else
                                printlog "the result is OK"
                            endif
                    else
                        warnlog "Execution of a query failed!"
                    end if

            call fCloseQueryDesign

        endif
    endif
    call fCloseDatabase

end sub
'--------------------------------------------------------------------
function tSortQueryWithAlias(sFileName, optional sPassword) ' test for isse 27832

    if IsMissing(sPassword) then
        if ( not fOpenDatabase(sFileName) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    else
        if ( not fOpenDatabase(sFileName,sPassword) ) then        
            warnlog "Database can't be open"
            exit function
        endif
    endif
    
    if ( fOpenNewQueryDesign ) then
        if ( fChooseTableInQueryAddTableDialog("TT_Query1") ) then
            Kontext "QueryDesignCriterion"
                sleep(1)
                QueryDesignCriterion.TypeKeys "<MOD1 PAGEUP>" , true
                sleep(1)
                QueryDesignCriterion.TypeKeys "<MOD1 SHIFT DOWN>" , true
                sleep(1)
                QueryDesignCriterion.TypeKeys "<MOD1 SHIFT DOWN>" , true
                sleep(1)
                QueryDesignCriterion.TypeKeys "<DOWN>" , true
                sleep(1)
                QueryDesignCriterion.TypeKeys "test" , true
                sleep(1)
            Kontext "Toolbar"                
                printlog "- Executing query"
                ExecuteBtn.Click
                sleep(5)
                
            call fCloseQueryDesign
            
        endif
    endif
    call fCloseDatabase
    
end function	
'--------------------------------------------------------------------
function fChooseTableInQueryAddTableDialog(sTableName as string) as boolean
    
    
    Kontext "AddTables"
        if ( not AddTables.exists(1) ) then
            fChooseTableInQueryAddTableDialog = false    
            exit function            
        endif
        
    'TODO: add a test to click on the execute button, before the add table dialog is closed.
        dim ix as integer 
        dim i as integer
        ix = TableName.getItemCount                
        for i = 1 to ix
            TableName.Select i
            TableName.TypeKeys "<ADD>"
            'printlog "ix = " + ix
            'printlog "TableName.getItemCount = " + TableName.getItemCount 
            if TableName.getItemCount > ix then
                ix = TableName.getItemCount
                'printlog "TableName.getItemCount > ix"
            endif
            'printlog "TableName.getSeltext = " + TableName.getSeltext
            if TableName.getSeltext = sTableName then
                i = ix
            endif
        next
        if TableName.getSeltext <> sTableName then
            warnlog "Can find the table " + sTableName + "." + _
                    "Make sure that the table exists when starting this test."
            DatabaseClose                        
            sleep(5)
            Kontext "Messagebox"
                if Messagebox.Exists(3) then
                    if Messagebox.getRT = 304 then
                        Messagebox.No                        
                    end if                        
                end if                
            call hCloseDocument
            exit function
        endif
        printlog "- Add " + sTableName +" table to query design"
        '/// Add database table to Query.
        AddTable.Click
        sleep(2) 
        if AddTables.exists(2) then 
            CloseDlg.Click ' When the AddTables dialog still exists then close him
        endif
        sleep(1)
        fChooseTableInQueryAddTableDialog = true
        
end function
'--------------------------------------------------------------------
function fChooseQueryInQueryAddTableDialog(sQueryName as string) as boolean
    
    
    Kontext "AddTables"
        if ( not AddTables.exists(1) ) then
            fChooseQueryInQueryAddTableDialog = false    
            exit function            
        endif
        Queries.check
        
        
    'TODO: add a test to click on the execute button, before the add table dialog is closed.
        dim ix as integer 
        dim i as integer
        ix = QueryName.getItemCount                
        for i = 1 to ix
            QueryName.Select i
            QueryName.TypeKeys "<ADD>"
            'printlog "ix = " + ix
            'printlog "TableName.getItemCount = " + TableName.getItemCount 
            if QueryName.getItemCount > ix then
                ix = QueryName.getItemCount
                'printlog "TableName.getItemCount > ix"
            endif
            'printlog "TableName.getSeltext = " + TableName.getSeltext
            if QueryName.getSeltext = sQueryName then
                i = ix
            endif
        next
        if QueryName.getSeltext <> sQueryName then
            warnlog "Can find the table " + sTableName + "." + _
                    "Make sure that the table exists when starting this test."
            DatabaseClose                        
            sleep(5)
            Kontext "Messagebox"
                if Messagebox.Exists(3) then
                    if Messagebox.getRT = 304 then
                        Messagebox.No                        
                    end if                        
                end if                
            call hCloseDocument
            exit function
        endif
        printlog "- Add " + sQueryName +" table to query design"
        '/// Add database table to Query.
        AddTable.Click
        sleep(2) 
        if AddTables.exists(2) then 
            CloseDlg.Click ' When the AddTables dialog still exists then close him
        endif
        sleep(1)
        fChooseQueryInQueryAddTableDialog = true
        
end function
