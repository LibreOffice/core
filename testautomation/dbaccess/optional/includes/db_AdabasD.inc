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
'* short description : Create Adabas DS & Table & fill in Test
'*
'\***********************************************************************
testcase db_AdabasD

    printlog "------------------ db_AdabasD.inc ---------------------"

    if gPlatform = "x86" then
        printlog "No Adabas available under x86."
    elseif gOOO then
        printlog "No Adabas available under OpenOffice.org."
    else             
    
        ' **************************************************
        '   databases specific settings for Adabas D 11.02
        ' **************************************************
           
        Dim sFileName as string
        sFileName = gOfficePath + "user/work/TT_Adabas.odb"
        
        Dim sTableName as string
        sTableName = "tt_test_table"    'table name lenght restriction
                
        dim sCatalog as string
        sCatalog = " "              ' empty for adabas
        
        dim sSchema as string
        sSchema = "TESTTOOL"        ' case sensitive !
        
        dim sRelTable1 as string    'for relation test (tRelation1&2)
        sRelTable1 = "tt_rel1"
        
        dim sRelTable2 as string    'for relation test (tRelation1&2)
        sRelTable2 = "tt_rel2"
        
        dim sRelTable3 as string    'for relation test (tRelation2)
        sRelTable3 = "tt_rel3"
        
        Dim aFieldTypeContent(17,3) as string     'database specific data matrix
            
        aFieldTypeContent(1,1)="tt_boolean"     'name of fieldtype
        aFieldTypeContent(1,2)="boolean"        'number of fieldtype (listbox entry)    
        
        aFieldTypeContent(2,1)="tt_long_byte"
        aFieldTypeContent(2,2)="long byte"
             
        aFieldTypeContent(3,1)="tt_varchar_byte"
        aFieldTypeContent(3,2)="varchar() byte"
             
        aFieldTypeContent(4,1)="tt_char_byte"
        aFieldTypeContent(4,2)="char() byte"
             
        aFieldTypeContent(5,1)="tt_long"        ' does not work #69846
        aFieldTypeContent(5,2)="long"
             
        aFieldTypeContent(6,1)="tt_char"
        aFieldTypeContent(6,2)="char"
            
        aFieldTypeContent(7,1)="tt_decimal"     ' will probably work after #108512 is fixed
        aFieldTypeContent(7,2)="decimal"
            
        aFieldTypeContent(8,1)="tt_fixed"       ' will probably work after #108512 is fixed
        aFieldTypeContent(8,2)="fixed"
            
        aFieldTypeContent(9,1)="tt_integer"
        aFieldTypeContent(9,2)="integer"
            
        aFieldTypeContent(10,1)="tt_smallint"
        aFieldTypeContent(10,2)="smallint"
            
        aFieldTypeContent(11,1)="tt_float"
        aFieldTypeContent(11,2)="float"
            
        aFieldTypeContent(12,1)="tt_real"
        aFieldTypeContent(12,2)="real"
            
        aFieldTypeContent(13,1)="tt_double_precision"
        aFieldTypeContent(13,2)="double precision"
            
        aFieldTypeContent(14,1)="tt_varchar"
        aFieldTypeContent(14,2)="varchar"
        
        aFieldTypeContent(15,1)="tt_date"
        aFieldTypeContent(15,2)="date"
            
        aFieldTypeContent(16,1)="tt_time"
        aFieldTypeContent(16,2)="time"
            
        aFieldTypeContent(17,1)="tt_timestamp"
        aFieldTypeContent(17,2)="timestamp"
            
        Dim aFieldContent(1,4) as string
        aFieldContent(1,1)="<Space>"
        aFieldContent(1,2)="1"
        aFieldContent(1,3)="1"
        aFieldContent(1,4)="1"
        
        dim dbok as boolean
        dbok = false
        
        dim aDatabaseProperties(5) as string
        aDatabaseProperties() = tools_dbtools_fgetAdabasDatabaseProperties()
    
        ' if and only if no properties are defined in the environment file the test is stopped
        if(aDatabaseProperties(1) = "no") then
            qaerrorlog "No database properties from Adabas defiened. The Test is stopped here."
            goto endsub
        endif        
        
        dbok = fCreateAdabasDatasource( sFileName, aDatabaseProperties(2), aDatabaseProperties(3), aDatabaseProperties(4))
                                        
        if dbok = true then
            call fOpenDatabase(sFileName,"testtool")
            call fDeleteTable( sTableName )
            call fCreateTable( aFieldTypeContent(), sTableName )    
            call fInsertIntoTable( aFieldContent(), sTableName )
            call fCloseDatabase
            
                call tRelation( sFileName, sRelTable1, sRelTable2 )
                call tDoubleRelation( sFileName, aDatabaseProperties(5), sRelTable1, sRelTable2, sRelTable3 )
                call db_Query(sFileName,"adabas",aDatabaseProperties(5))  
            
        else
            warnlog "Data Source could not be created - beyond testcases stopped"
        endif  
    endif
endcase
