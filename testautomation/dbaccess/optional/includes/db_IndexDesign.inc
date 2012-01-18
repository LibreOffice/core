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
'* owner : marc.neumann@oracle.com
'*
'* short description : tests for indexes
'*
'\***********************************************************************

function tIndex(sFileName as String, sTableName as String)
    
    printlog sTableName
    printlog sFileName

    call fOpendatabase(sFileName)

    call fOpenTableInDesign(sTableName)
    
    Kontext "TableDesignTable"
        TableDesignTable.UseMenu        
        hMenuSelectNr(4) ' the tools menu
        hMenuSelectNr(1) ' the IndexDesign

    Kontext "DatabaseIndexes"
        if ( not DatabaseIndexes.exists(3) ) then
            warnlog "The index design doesn't exists"
        endif
        NewIndex.Click
        sleep(1)
        DatabaseIndexes.TypeKeys("<RETURN>")
        IndexFields.select 6
        DatabaseIndexesClose.Click
        
    Kontext "MessageBox"
        MessageBox.Yes
        
    '/// remove the created index again

    Kontext "TableDesignTable"
        TableDesignTable.UseMenu        
        hMenuSelectNr(4) ' the tools menu
        hMenuSelectNr(1) ' the IndexDesign

    Kontext "DatabaseIndexes"
        if ( not DatabaseIndexes.exists(3) ) then
            warnlog "The index design doesn't exists"
        endif
        IndexList.select 1
        DropIndex.Click
    Kontext "MessageBox"
        MessageBox.Yes
        sleep(1)

    Kontext "DatabaseIndexes"
        DatabaseIndexesClose.Click
        
    Kontext "MessageBox"
    	if MessageBox.exists(2) then
	        MessageBox.Yes
	    endif
                                
        
    call fCloseTableDesign()

    call fCloseDatabase()

end function
