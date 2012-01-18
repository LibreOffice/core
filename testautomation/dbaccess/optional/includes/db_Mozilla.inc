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
'* short description : Address book mozilla
'*
'\***********************************************************************
sub db_Mozilla

    if(gPlatform = "osx") then
        ' on Mac platform the Mozilla address book is not available
        ' so I call only the testcases to satisfied the database
        tQueryAddressbook("")
        tSortAddressbook("")
    else
    
        ' Information for this test under
        ' http://wiki.services.openoffice.org/wiki/Database_Automatic_Testing#Testing_the_mozilla_Address_book

        if fCreateMozillaAddressbookDatasource(gOfficePath + ConvertPath("user/work/TT_Mozilla.odb")) then

            tQueryAddressbook(gOfficePath + ConvertPath("user/work/TT_Mozilla.odb"))
            tSortAddressbook(gOfficePath + ConvertPath("user/work/TT_Mozilla.odb"))
            else
            tQueryAddressbook("")
            tSortAddressbook("")
        endif
    endif
    
end sub
'-------------------------------------------------------------------------
testcase tQueryAddressbook( sFileName )
    
    if(gPlatform = "osx") then
        printlog "mozilla address book not available on Mac"
        goto endsub
    endif

    if ( not fOpenDatabase(sFileName) ) then        
        warnlog "Database can't be open"
        printlog "May be you find a solution under http://wiki.services.openoffice.org/wiki/Database_Automatic_Testing#Testing_the_mozilla_Address_book"
        goto endsub
    endif
    
    fOpenNewQueryDesign
        
    Kontext "AddTables"    
        TableName.Select 1        
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

    call fCloseQueryDesign()

    call fCloseDatabase()
    
endcase
'-------------------------------------------------------------------------
testcase tSortAddressbook( sFileName )
    
    if(gPlatform = "osx") then
        printlog "mozilla address book not available on Mac"
        goto endsub
    endif

    Dim sRecordCount as String
    
    '/// open the database file created in the bas file
    printlog "open the database file created in the bas file"
    
    if ( not fOpenDatabase(sFileName) ) then        
        warnlog "Database can't be open"
        goto endsub
    endif
    
    '/// open the first table
    printlog "open the first table"    
    Kontext "ContainerView"
        ViewTables
        TableTree.select(1)
        OpenTable ' uno-Slot .uno:DB/Open
    
    sleep(2)

    '/// count the records in the table
    printlog "count the records in the table"
    Kontext "TableView"
        LastRecord.Click
        sRecordCount = AllRecords.caption()
    
    '/// sort the table
    printlog "sort the table"
    Kontext "Toolbar"
        SortAscending.Click
        sleep(1)
    
    '/// count the table again and check if there are the same count of records then before sorting
    printlog "count the table again and check if there are the same count of records then before sorting"        
    Kontext "TableView"
        LastRecord.Click
        if (sRecordCount <> AllRecords.caption() ) then
            warnlog "issue i61611 occur. The records are double after sorting."
        endif

    '/// close the table
    printlog "close the table"    
    call fCloseTableView()
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase()

endcase
