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
'* short description : Addressbook Windows
'*
'\***********************************************************************
sub db_Windows

    if gPlatGroup = "w95" then
        if fCreateWindowsAddressbookDatasource(gOfficePath + ConvertPath("user/work/TT_Windows.odb")) then
            call tQueryAddressbook( gOfficePath + ConvertPath("user/work/TT_Windows.odb"))
        endif
    else
        printlog "This test works only under Windows"
    endif

end sub
'-------------------------------------------------------------------------
testcase tQueryAddressbook( sFileName )
    
    Dim SelEntry as string
    Dim sTableName as string
    
    sTableName = "TT_Addressbook"

    if ( not fOpenDatabase(sFileName) ) then        
        warnlog "Database can't be open"
        printlog "May be you find a solution under http://wiki.services.openoffice.org/wiki/Database_Automatic_Testing#Testing_the_windows_Address_book"
        goto endsub
    endif
    
    fOpenNewQueryDesign
        
    Kontext "AddTables"        
        TableName.Select 1
        AddTable.Click
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
