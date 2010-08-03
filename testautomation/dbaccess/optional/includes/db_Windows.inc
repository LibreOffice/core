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
