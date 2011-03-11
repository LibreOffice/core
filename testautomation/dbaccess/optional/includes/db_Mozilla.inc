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
