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
