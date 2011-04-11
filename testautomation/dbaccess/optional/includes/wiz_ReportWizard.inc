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
'* short description : Report Wizard Test
'*
'\***********************************************************************
sub wiz_ReportWizard
    
    call tNewReport

end sub
'-------------------------------------------------------------------------
testcase tNewReport

    qaerrorlog "#i92543# crash when closing report"
    goto endsub

    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
        
    Kontext "DATABASE"
    
    ViewReports
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)
    
    StartReportWizard
    
    sleep(5)
    
    Kontext "ReportWizard"        
        sleep(1)
        Tables.select 1
        '/// add the third field
        printlog "add the third field"
        AvailableFields.select 1
        sleep(1)
        Add.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)        
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        '/// click Finish
        printlog "click Finish"       
        FinishBtn.Click
    sleep(10)

    Kontext "DocumentWriter"    	 
    	DocumentWriter.TypeKeys "<MOD1 SHIFT I>" ,  true ' EDIT / SELECT TEXT
    	DocumentWriter.TypeKeys "<DOWN>" ,2,  true    	        
        DocumentWriter.TypeKeys "<SHIFT DOWN>" , true
        DocumentWriter.TypeKeys "<MOD1 C>" , true

	dim s as String 
	s = getClipboard	

    if left(s,10) = "Identifier" then
        printlog "Report Table Header contains " + left(s,10) + ". -> OK"
    else
        warnlog "Report Table Header contains " + left(s,10) + " instead of IDENTIFIER"
    endif

    'for windows a "new Line" are two characters
    'so I need to start at char 13 and not on 12   
    dim iFromCharacter as integer
    if gPlatGroup = "w95" then
        iFromCharacter = 13
    else
        iFromCharacter = 12
    endif

    if mid(s,iFromCharacter,5) = "GUR00" OR mid(s,iFromCharacter,5) = "BOR04" then
        printlog "1. record is " + mid(s,iFromCharacter,5) + ". -> OK"
    else
        warnlog  "1. record is " + mid(s,iFromCharacter,5) + " instead of GUR00 OR BOR04"
    endif

    call fCloseForm ' should be changed to a CloseReport

    call fCloseDatabase

endcase

