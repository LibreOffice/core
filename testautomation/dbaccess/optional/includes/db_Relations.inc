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
'* short description : Base Level 1 Create Relations between Test Tables
'*
'\***********************************************************************
function tRelation( sFileName, sPWD, sRelTable1, sRelTable2 )
	'/// create an 1:1 and 1:n relation between two test tables
    Dim i as integer			    'counter
    Dim j as integer			    'counter
    Dim ix as integer			    'counter
    Dim iNoDS as integer                'number of data source in listbox
    Dim iNoTab as integer               'number of tables in grid
    Dim DSOK as boolean                 'datasource present check      
    
    call fOpenDatabase(sFileName, sPWD)
    
    call fOpenRelationDesign
    
    '/// searching and adding of the two needed testtables    
        
    bDelete_Container() 'delete all disturbing table container
    sleep(1)
        
    '/// insert add tables dialog if not present    
    Kontext "AddTables"
        if (AddTables.exists(1) = false) then
            printlog "open add table dialog"
            Kontext "RelationDesign"
            sleep(1)
            RelationDesign.UseMenu
            sleep(1)
            hMenuSelectNr (4)
            sleep(1)
            hMenuSelectNr (2)
            sleep(1)
        endif
        sleep(1)
    'call fChooseTableInAddTableDialog(sRelTable1)
        
        '/// searching and adding the two needed testtables
    Kontext "AddTables"
        dim sRelTable as string         'buffer variable
        dim breltablefound as integer   'control if both needed table are found
        breltablefound = 0
        
        for j = 1 to 2
            if j = 1 then 
                sRelTable = sRelTable1 'first table
            else 
                sRelTable = sRelTable2  'second table
            endif
            ix = TableName.getItemCount                
            for i = 1 to ix
                TableName.Select i
                TableName.TypeKeys "<ADD>"  'adding test table
                if TableName.getItemCount > ix then
                    ix = TableName.getItemCount
                endif
                if TableName.getSeltext = sRelTable then
                    printlog "table found: " + sRelTable
                    breltablefound = breltablefound +1
                    i = ix
                endif
            next i
                sleep(1)
                AddTable.Click
                sleep(1)                        
        next j
        CloseDlg.Click
        
        if breltablefound <> 2 then     'two needed tables not found if variable <> 2
            warnlog "proper tables: " + sRelTable1 + " and/or " + sRelTable2 + " not found - test abort"
            call fCloseRelationDesign
            call fCloseDatabase
            exit function
        else
            printlog "both needed tables found and added"
        endif
        sleep(1)
        
    Kontext "RelationDesign"        
   '/// open relation property menue
        sleep(1)
        RelationDesign.UseMenu
        sleep(1)
        hMenuSelectNr (4)
        sleep(1)
        hMenuSelectNr (1)
        sleep(1)
    Kontext "RelationProperties"
    '/// create 1:1 relation: open relation design and choose the ID fields from each table/listbox
        sleep(1)
        printlog "open relation design and choose the ID fields from each table/listbox"
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<MOD2 DOWN>",TRUE
        sleep(1)
        RelationProperties.TypeKeys "<DOWN>",TRUE
       ' RelationProperties.TypeKeys "<RETURN>",TRUE
        sleep(1)
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<MOD2 DOWN>",TRUE
        sleep(1)
        RelationProperties.TypeKeys "<DOWN>",TRUE
       ' RelationProperties.TypeKeys "<RETURN>",TRUE
        sleep(1)
        RelationProperties.OK
        printlog "1:1 relation between test tables created"
        sleep(1)
        
    Kontext "RelationDesign"
    '/// delete relation connector
        dim k as integer    'injurance that routine do not loop
        dim l as integer    'buffer for hMenuItemGetCount
        k = 0
        Do
           '/// checkout wich kontext has two entries (only relation connector has)
            Kontext "RelationDesign"
                 RelationDesign.TypeKeys "<TAB>",TRUE
                 sleep(1)
                 RelationDesign.TypeKeys "<SHIFT F10>",TRUE
                 sleep(1)
                 try
                 	l = hMenuItemGetCount
                 catch
                 endcatch	
                 MenuSelect 0
                 sleep(1)
                 k = k + 1
                 'printlog "k = " + k
                 'printlog "hMenuItemGetCount = " + l
        Loop Until k = 10 or l = 2
        if k = 10 then
           warnlog "abort deleting relation: searching for relation connector went wrong"
        endif
        printlog "relation connector deleted"
        RelationDesign.TypeKeys "<SHIFT F10>",TRUE
        hMenuSelectNr (1)
        sleep(1)
        
        '/// create 1:n relation 
        Kontext "RelationDesign"
        '/// open relation property menue
            printlog "open relation property menue for 1:n relation"
            sleep(1)
            RelationDesign.UseMenu
            sleep(1)
            hMenuSelectNr (4)
            sleep(1)
            hMenuSelectNr (1)
            sleep(1)
        Kontext "RelationProperties"
            '/// open relation design and change listbox content
            printlog "open relation design and choose proper fields from table/listbox"
            RelationProperties.TypeKeys "<TAB>",TRUE
            RelationProperties.TypeKeys "<TAB>",TRUE
            RelationProperties.TypeKeys "<MOD2 DOWN>",TRUE
            sleep(1)
            RelationProperties.TypeKeys "<DOWN>",TRUE
            RelationProperties.TypeKeys "<DOWN>",TRUE
            sleep(1)
            RelationProperties.TypeKeys "<TAB>",TRUE
            RelationProperties.TypeKeys "<MOD2 DOWN>",TRUE
            sleep(1)
            RelationProperties.TypeKeys "<DOWN>",TRUE
            RelationProperties.TypeKeys "<RETURN>",TRUE
            sleep(1)
            RelationProperties.OK
            printlog "1:n relation between test tables created"
            sleep (1)
            
            bDelete_Container() 'delete all table container to leave place clear
            sleep (1)
              
        fCloseRelationDesign(true)'close relation design and save it"
            

        call fCloseDatabase
end function
'-------------------------------------------------------------------------
function tDoubleRelation( sFileName, sPWD, sRelTable1, sRelTable2, sRelTable3 )
	'/// create an 1:1 relation, close and save relation dialog,
    '/// reopen the dialog and add an 1:n relation with third test table
    Dim i as integer			    'counter
    Dim j as integer			    'counter
    Dim ix as integer			    'counter
    Dim iNoDS as integer                'number of data source in listbox
    Dim iNoTab as integer               'number of tables in grid
    Dim DSOK as boolean                 'datasource present check      
    
    
    call fOpenDatabase(sFileName, sPWD)
    
    call fOpenRelationDesign
        
    '/// searching and adding of the two needed testtables    
        
    bDelete_Container() 'init
        
    '/// insert add tables dialog if not present    
    Kontext "AddTables"
        if (AddTables.exists(1) = false) then
            '/// open add table dialog
            printlog "open add table dialog"
            Kontext "RelationDesign"
            sleep(1)
            RelationDesign.UseMenu
            sleep(1)
            hMenuSelectNr (4)
            sleep(1)            
            hMenuSelectNr (2)
            sleep(1)
        endif
        sleep(1)
        
    '/// add the needed testtables
    Kontext "AddTables"
        dim sRelTable as string         'buffer for tablenames
        dim breltablefound as integer   'control if both needed table are found
        breltablefound = 0
        for j = 1 to 2
            if j = 1 then 
                sRelTable = sRelTable1 'first table
            else 
                sRelTable = sRelTable2  'second table
            endif
            ix = TableName.getItemCount                
            for i = 1 to ix
                TableName.Select i
                TableName.TypeKeys "<ADD>"
                if TableName.getItemCount > ix then
                    ix = TableName.getItemCount
                endif
                if TableName.getSeltext = sRelTable then
                    printlog "table found: " + sRelTable
                    breltablefound = breltablefound +1
                    i = ix
                endif
            next i
            sleep(1)
            AddTable.Click
            sleep(1)
        next j
        CloseDlg.Click
        
        if breltablefound <> 2 then     'two needed tables not found if variable <> 2
            warnlog "proper tables: " + sRelTable1 + " and/or " + sRelTable2 + " not found - test abort"
            call fCloseRelationDesign
            call fCloseDatabase
            exit function
        else
            printlog "both tables found and added"
        endif
        sleep(1)
    
    '/// create 1:1 relation
    Kontext "RelationDesign"
    '/// open relation property menue
        printlog "open relation property menue"
        sleep(1)
        RelationDesign.UseMenu
        sleep(1)
        hMenuSelectNr (4)
        sleep(1)
        hMenuSelectNr (1)
        sleep(1)
    Kontext "RelationProperties"
        '/// open relation design and choose proper fields from table/listbox
        printlog "open relation design and choose proper fields from table/listbox"
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<MOD1 DOWN>",TRUE
        sleep(1)        
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<MOD1 DOWN>",TRUE        
        sleep(1)
        RelationProperties.OK
        printlog "1:1 relation between test tables created"
        
    '/// close and save relation design and save design
    call fCloseRelationDesign(true)
        
'/// Second Part: adding 1:n relation  
        printlog ""
        printlog "Second Part: adding 1:n relation"
    
    call fOpenRelationDesign
        
    '/// insert add tables dialog if not present    
    Kontext "AddTables"
        if (AddTables.exists(1) = false) then
            '/// open add table dialog
            printlog "open add table dialog"
            Kontext "RelationDesign"
            sleep(1)
            RelationDesign.UseMenu
            sleep(1)
            hMenuSelectNr (4)
            sleep(1)
            hMenuSelectNr (2)
            sleep(1)
        endif
        sleep(1)
    
    '/// searching and adding proper table
    Kontext "AddTables"
        breltablefound = 0
        for j = 1 to 1
            sRelTable = sRelTable3 'third table
            ix = TableName.getItemCount                
            for i = 1 to ix
                TableName.Select i
                TableName.TypeKeys "<ADD>"
                if TableName.getItemCount > ix then
                    ix = TableName.getItemCount
                endif
                if TableName.getSeltext = sRelTable then
                    printlog "table found: " + sRelTable
                    breltablefound = breltablefound +1
                    i = ix
                endif
            next i
            sleep(1)
            AddTable.Click
            sleep(1)
        next j
        CloseDlg.Click
        
        if breltablefound <> 1 then     'needed table not found if variable <> 1
            warnlog "proper table: " + sRelTable3 + " not found - test abort"
            call fCloseRelationDesign
            call fCloseDatabase
            exit function
        else
            printlog "third table found and added"
        endif
        sleep(1)
    
    
    '/// add 1:n relation
    Kontext "RelationDesign"
    '/// open relation property menue
        printlog "open relation property menue"
        sleep(1)
        RelationDesign.UseMenu
        sleep(1)
        hMenuSelectNr (4)
        sleep(1)
        hMenuSelectNr (1)        
        sleep(1)
    Kontext "RelationProperties"
    '/// open relation design and choose the proper fields from table/listbox
        printlog "open relation design and choose the proper fields from table/listbox"
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<MOD1 DOWN>",TRUE
        RelationProperties.TypeKeys "<MOD1 DOWN>",TRUE        
        sleep(1)
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<MOD1 DOWN>",TRUE
        RelationProperties.TypeKeys "<MOD1 DOWN>",TRUE        
        sleep(1)
        RelationProperties.TypeKeys "<TAB>",TRUE
        RelationProperties.TypeKeys "<MOD1 DOWN>",TRUE        
        sleep(1)
        RelationProperties.OK
        
        Kontext "MessageBox"
        	if MessageBox.exists(1) then
        		MessageBox.OK
        		Kontext "RelationProperties"
        		RelationProperties.OK
        	endif
        printlog "1:n relation between test tables created" 
        sleep(1)
        
        bDelete_Container() 'delete all table container to leave place clear
        sleep (1)

    
    call fCloseRelationDesign   'close relation design and save init design"
        
    call fCloseDatabase
            
end function


function bDelete_Container() as boolean
    dim bcontainer_exist as boolean
    bcontainer_exist = true
    '/// delete container if present
    Do
        sleep(2)
        Kontext "RelationDesign"
        RelationDesign.MouseDown(2,2)
        RelationDesign.MouseUp(2,2)
        sleep(1)
        RelationDesign.TypeKeys "<DELETE>",TRUE
        sleep(2)
        Kontext "MessageBox"
            '/// container present if opening context menue possible
            if MessageBox.exists(2) then
                MessageBox.Yes
            else
                bcontainer_exist = false
            endif
    Loop Until bcontainer_exist = false
    printlog "deleted container to init relation design"
end function
