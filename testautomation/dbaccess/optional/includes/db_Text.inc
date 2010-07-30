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
'* short description : test for a text database
'*
'\***********************************************************************

sub db_Text(sFilename as string)

    call tCountCharacterSet(sFilename)
    call tOpenUTF8File(sFilename)

end sub
'-------------------------------------------------------------------------
testcase tCountCharacterSet(sFilename as string)

	'/// open the text database create in the bas file    
    call fOpendatabase(sFileName)

	'/// open the properties dialog of the open database
    printlog "open the properties dialog of the open database"            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(1)

    '/// switch to the Text Tabpage    
    printlog "switch to the Text Tabpage"    
    Kontext "DatabaseProperties"        
        TabControl.setPage TabTextBase
    
    '/// check if there are 54 character sets
    printlog "check if there are 54 character sets"   
    Kontext "TabTextBase"
        dim i as integer
        i = CharSet.getItemCount()
        if i <> 54 then
            warnlog "The count of the charsets is wrong. It have to be 54 but it is:" + i
        else
            printlog "The count of the charsets is correct.There are 54 character sets."
        endif       
    '/// close the properties dialog with OK
    printlog "close the properties dialog with OK"    
    Kontext "DatabaseProperties"
        DatabaseProperties.OK
    
    '/// close the database
    printlog "close the database"    
    call fCloseDatabase()

endcase
'-------------------------------------------------------------------------
testcase tOpenUTF8File(sFilename as string)
    
    '/// copy the file TesttoolPath + dbaccess/optional/input/text_datasource/text_database.txt" to your local wotk directory
    printlog "copy the file TesttoolPath + dbaccess/optional/input/text_datasource/text_database.txt to your local wotk directory"
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/text_datasource/text_database.txt"),gOfficePath + ConvertPath("user/work/text_database.txt")

    '/// open the text database create in the bas file
    call fOpendatabase(sFileName)
    
    '/// open the properties dialog of the open database
    printlog "open the properties dialog of the open database"
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(1)

    '/// switch to the Text Tabpage
    printlog "switch to the Text Tabpage"
    Kontext "DatabaseProperties"        
        TabControl.setPage TabTextBase
   
    '/// set UTF-8 as character set
    printlog "set UTF-8 as character set"    
    Kontext "TabTextBase"
        dim iCount as integer
        dim i as integer
        iCount = CharSet.getItemCount()
        for i = 1 to iCount 
            CharSet.select(i)            
            if (Instr(CharSet.getSelText(),"(UTF-8)") <> 0 ) then
                i = iCount
            endif
        next

    '/// close the properties dialog with OK
    printlog "close the properties dialog with OK"    
    Kontext "DatabaseProperties"
        DatabaseProperties.OK
    
    '/// save and close the database
    call fCloseDatabase(true)
    
    '/// open the text database again
    call fOpendatabase(sFileName)    
        
    '/// open the table text_database
    printlog "open the table text_Database"    
    call fOpenTable("text_database")
    
    '/// check if the first row contains the string öäü
    printlog "check if the first row contains the string öäü"
    kontext "TableView"
        TableView.TypeKeys("<MOD1 C>",true)
        if ( getClipboard = "1;öäü") then
        	printlog "The table is load with an UTF-8 locale -> OK"
        else
        	warnlog "The table is not load ed with an UTF-8 locale -> FAILED"
		endif
    
    '/// close the table
    printlog "close the table"    
    call fCloseTableView
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase()

endcase
