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
'* short description : test forms
'*
'************************************************************************
'*
' #1 tNewFormDesign
'*
'\***********************************************************************************
sub Forms

    printlog "------------------ Forms.inc ---------------------"
    
    call tNewFormDesign
         
end sub
'-------------------------------------------------------------------------
testcase tNewFormDesign

    '/// open biblio database
    printlog "open biblio database"    
    hFileOpen( gOfficePath & "user/database/biblio.odb" )
        
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)

    '/// view forms
    printlog "view forms"

    ViewForms
    
    sleep(1)

    '/// open a new form design
    printlog "open a new form design"
    
    NewFormDesign

    sleep(5)

    '/// close the form design
    printlog "close the form design"
    
    Kontext "DocumentWriter"
        DocumentWriter.UseMenu        
        hMenuSelectNr(8) ' the window menu
        hMenuSelectNr(2) ' the Close Window        
    ' if messages box appear because of unsaved record click no in the dialog
    Kontext "Messagebox"
        if Messagebox.Exists(3) then
            Messagebox.No
        end if
        
    sleep(1)        

    '/// close the database
    printlog "close the database"
    
    call fCloseDatabase
    
endcase
