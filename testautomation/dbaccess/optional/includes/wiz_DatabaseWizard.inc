'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wiz_DatabaseWizard.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 07:43:42 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : marc.neumann@sun.com
'*
'* short description : Database Wizard
'*
'\***********************************************************************
sub wiz_DatabaseWizard
    call tNewDatabase
    call tOpenDatabase
    call tOpenNoneDatabaseDocument
    call tODBC
    call tEvolution
    call tMozilla
    call tJDBC
    call tAdabas
    call tdBase
    call tMySQLODBC
    call tMySQLJDBC
    call tSpreadsheet
    call tText
    call tOracleJDBC
end sub
'-------------------------------------------------------------------------
testcase tNewDatabase
        
    Kontext "DocumentWriter"
        if (DocumentWriter.exists(1)) then        
            DocumentWriter.UseMenu
        else
            Kontext "DocumentBackground"
            DocumentBackground.UseMenu
        endif  
        hMenuSelectNr(1)
        hMenuSelectNr(1)
        hMenuSelectNr(5)
        
    sleep(10)
    
    Kontext "DatabaseWizard"    
        CreateNewDatabase.Check
        sleep(1)
        NextBtn.Click
        sleep(1)        
        FinishBtn.Click
        sleep(1)
	                  
    Kontext "SpeichernDlg"
        if ( Dateiname.getSelText() = "" ) then
            warnlog "#i58413# Default filename is missing"
            Dim sFileName as String
            sFileName = ConvertPath(gOfficePath + "user/work/TTDB1.odb")
            if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
                app.kill(ConvertPath(sFileName))
            endif
            Dateiname.setText(sFileName)
        endif            
        Speichern.click 
        sleep(10)
        
    Kontext "MessageBox" 
        if MessageBox.exists then
            MessageBox.yes
        end if
        
    sleep(5)

    Kontext "ContainerView"
    
    ViewTables
    
    sleep(1)
    
    call fCloseDatabase(true)
    
endcase
'-------------------------------------------------------------------------
testcase tOpenDatabase
        
    Kontext "DocumentWriter"
        if (DocumentWriter.exists(1)) then        
            DocumentWriter.UseMenu
        else
            Kontext "DocumentBackground"
            DocumentBackground.UseMenu
        endif  
        hMenuSelectNr(1)
        hMenuSelectNr(1)
        hMenuSelectNr(5)
    
    sleep(5)
        
    Kontext "DatabaseWizard"    
        OpenExistingDoc.Check
        sleep(1)
        OpenBtn.Click
        sleep(1)        
                      
    Kontext "GeneralFileDialog"
        printlog "open database from: " + Convertpath(gofficePath + "user/database/biblio.odb")
        Dateiname.setText(Convertpath(gofficePath + "user/database/biblio.odb"))            
        Oeffnen.click 
        sleep(5)
        
    Kontext "ContainerView"
    
    ViewTables
    
    sleep(1)
    
    call fCloseDatabase(false)
    
endcase   
'-------------------------------------------------------------------------
testcase tOpenNoneDatabaseDocument
        
    Kontext "DocumentWriter"
        if (DocumentWriter.exists(1)) then        
            DocumentWriter.UseMenu
        else
            Kontext "DocumentBackground"
            DocumentBackground.UseMenu
        endif  
        hMenuSelectNr(1)
        hMenuSelectNr(1)
        hMenuSelectNr(5)
    
    sleep(5)
        
    Kontext "DatabaseWizard"    
        OpenExistingDoc.Check
        sleep(1)
        OpenBtn.Click
        sleep(1)        
                      
    Kontext "GeneralFileDialog"
        printlog "open spreadsheet file from: " + Convertpath(gTesttoolPath + "dbaccess/optional/input/spreadsheet_datasource/TT_Query1.ods")
        Dateiname.setText(Convertpath(gTesttoolPath + "dbaccess/optional/input/spreadsheet_datasource/TT_Query1.ods"))            
        Oeffnen.click 
        sleep(5)
        
    Kontext "DocumentCalc"
    if (DocumentCalc.exists()) then
		warnlog "the spreadsheet should not appear"
        call hCloseDocument()
    else
        Kontext "MessageBox"
        if(MessageBox.exists(1)) then
        	printlog "messagebox appear ->> OK"
        	MessageBox.OK
            Kontext "DatabaseWizard"	
    	        DatabaseWizard.Cancel
        else
        	warnlog "there should be a message box about the fact that this is no database"
        	Kontext "DatabaseWizard"	
	            DatabaseWizard.Cancel
        endif
    endif
    
endcase
'-------------------------------------------------------------------------
testcase tODBC
qaerrorlog "not yet implemented"
endcase
'-------------------------------------------------------------------------
testcase tEvolution

    if gPlatgroup <> "lin" then
        printlog "Evolution does only exists under linux."
        goto endsub    
    end if   

    call fCreateEvolutionAddressbookDatasource(gOfficePath + "user/work/tt_evolution.odb")
    call fOpendatabase(gOfficePath + "user/work/tt_evolution.odb")
    
    Kontext "ContainerView"
        ViewTables
    
    call fCloseDatabase(true)   
    
endcase
'-------------------------------------------------------------------------
testcase tMozilla

    call fCreateMozillaAddressbookDatasource(gOfficePath + "user/work/tt_mozilla.odb")
    call fOpendatabase(gOfficePath + "user/work/tt_mozilla.odb")
    
    Kontext "ContainerView"
        ViewTables
    
    call fCloseDatabase(true)    

endcase
'-------------------------------------------------------------------------
testcase tJDBC
qaerrorlog "not yet implemented"
endcase
'-------------------------------------------------------------------------
testcase tAdabas

    qaerrorlog "not yet implemented"
    goto endsub

    qaerrorlog "not yet implemented"
    goto endsub

    if gPlatform = "x86" then
        printlog "Adabas doesn't exists under x86."
        goto endsub    
    elseif gOOO then
        printlog "Adabas doesn't exists under Openoffice.org."
        goto endsub    
    end if
    
    dim aDatabaseProperties(6) as string
    aDatabaseProperties() = tools_dbtools_fgetAdabasDatabaseProperties()

    ' if and only if no properties are defined in the environment file the test is stopped
    if(aDatabaseProperties(1) = "no") then
        qaerrorlog "No database properties from Adabas defiened. The Test is stopped here."
        goto endsub
    endif        
    
	call fCreateAdabasDatasource( sFileName, aDatabaseProperties(2), aDatabaseProperties(3), aDatabaseProperties(4))
    call fOpendatabase(gOfficePath + "user/work/tt_adabas.odb","testuser")
    
    Kontext "ContainerView"
        ViewTables
    
    call fCloseDatabase(true)    
    
endcase
'-------------------------------------------------------------------------
testcase tdBase

    call fCreateDbaseDatasource(gOfficePath + "user/work/tt_dbase.odb",gOfficePath + "user/work/")
    call fOpendatabase(gOfficePath + "user/work/tt_dbase.odb")
    
    Kontext "ContainerView"
        ViewTables
    
    call fCloseDatabase(true)
    
endcase
'-------------------------------------------------------------------------
testcase tMySQLODBC
qaerrorlog "not yet implemented"
endcase
'-------------------------------------------------------------------------
testcase tMySQLJDBC
qaerrorlog "not yet implemented"
endcase
'-------------------------------------------------------------------------
testcase tSpreadsheet
qaerrorlog "not yet implemented"
endcase
'-------------------------------------------------------------------------
testcase tText

    call fCreateTextDatasource(gOfficePath + "user/work/tt_text.odb",gOfficePath + "user/work/")
    call fOpendatabase(gOfficePath + "user/work/tt_text.odb")
    
    Kontext "ContainerView"
        ViewTables
    
    call fCloseDatabase(true)    
    
endcase
'-------------------------------------------------------------------------
testcase tOracleJDBC
qaerrorlog "not yet implemented"
endcase
'-------------------------------------------------------------------------
