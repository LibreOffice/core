'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: dbcreatetools.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 07:43:45 $
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
'* short description : Library with function for creating all possible databases
'*
'***************************************************************************************
'*
' #1 fCreateMySQL_ODBC_Datasource
' #1 fCreateMySQL_JDBC_Datasource
' #1 fCreateAdabasDatasource    
' #1 fCreateLocalAdabasDatasource    
' #1 fCreateJDBCDatasource     
' #1 fCreateODBCDatasource
' #1 fCreateDbaseDatasource
' #1 fCreateADODatasource	
' #1 fCreateTextDatasource	
' #1 fCreateSpreadsheetDatasource
' #1 fCreateLDAPAddressbookDatasource
' #1 fCreateEvolutionAddressbookDatasource
' #1 fCreateWindowsAddressbookDatasource
' #1 fCreateOutlookAddressbookDatasource
' #1 fCreateMozillaAddressbookDatasource
' #1 fRegisterDatabaseFile
' #1 fDeRegisterDatabaseFile
' #1 fSetJDBCDriverFiles    
' #1 getIdForDatabaseType
'*
'\***********************************************************************************
function fCreateMySQL_ODBC_Datasource(sFileName, sOdbcDSN, sUser,optional sRegister) as boolean
    '/// create a new MySQL database via ODBC file with the given filename 
    '///+ and the given URL 
    '/// <u>parameter:</u>
    '/// <b>sFileName:</b> the file name for the database file
    '/// <b>sOdbcDSN:</b> the name of the odbc datasource    
    '/// <b>sUser:</b> the user for the connection
    '/// <b>sPassword:</b> the password for the connection    
    '/// <b><i>optional</i> sRegister:</b> the name under which the database is be registered
    dim bOK as boolean
	bOK = FALSE

' delete the file
    if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
        app.kill(ConvertPath(sFileName))
    endif	

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
        
        sleep(4)    

    Kontext "DatabaseWizard"    
        ConnectToDatabase.Check
        sleep(1)        
        DatabaseType.select getIdForDatabaseType("MySQLODBC")
        sleep(1)
        NextBtn.Click
        sleep(1)
        MySQLODBC.Check
        NextBtn.Click
        
    Kontext "DatabaseWizard"    
                
        '/// add the URL
        printlog "add the URL"		
        MySQLODBCDSName.setText "sdbc:mysql:odbc:" + sOdbcDSN

    Kontext "DatabaseWizard"    
        NextBtn.Click

    Kontext "DatabaseWizard"
        '/// add the user name 
        printlog "add the user name"		
        UserName.setText sUser
        
        '/// check the password required checkbox
        printlog "check the password required checkbox"        
        PasswordRequired.Check
    
    Kontext "DatabaseWizard"    
        NextBtn.Click
        
    Kontext "DatabaseWizard"        
        FinishBtn.Click

    Kontext "SpeichernDlg"
        Dateiname.setText ConvertPath(sFileName)    
        Speichern.click

    sleep(5)
    
    if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
    else        
        printlog "register datasource with name " + sRegister        
        call fRegisterDatabaseFile(ConvertPath(sFilename),sRegister)
    end if  
    
	bOK = true
	if bOK = true then
		printlog "--- End of function - *** Succeed ***"
	else
    	warnlog "--- End of function - *** Failed *** Data source was not created"
	endif	

    call fCloseDatabase(true)
	
	fCreateMySQL_ODBC_Datasource = bOK
	
end function
'-------------------------------------------------------------------------
function fCreateMySQL_JDBC_Datasource(sFileName,sDatabaseName,sHost,sPort,sUser,optional sRegister) as boolean
    '/// create a new MySQL database via JDBC file with the given filename 
    '///+ and the given URL 
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b>sDatabaseName:</b> the name of the mysql database
    '/// <b>sHost:</b> the host where the mysql database is running
    '/// <b>sPort:</b> the port where the mysql database is running
    '/// <b>sUser:</b> the user for the connection        
    '/// <b><i>optional</i> sRegister:</b> the name under which the database is be registered 
    dim bOK as boolean
	bOK = FALSE

    ' delete the file
    if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
        app.kill(ConvertPath(sFileName))
    endif	

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
        
        sleep(4)    

    Kontext "DatabaseWizard"    
        ConnectToDatabase.Check
        sleep(1)        
        DatabaseType.select getIdForDatabaseType("MySQLJDBC") - 1
        sleep(1)
        NextBtn.Click
        sleep(1)
        MySQLJDBC.Check
        NextBtn.Click
        sleep(1)
	    MySQLDatabaseName.setText(sDatabaseName)
        MySQLHostName.setText(sHost)
        MySQLPort.setText(sPort)
        
        NextBtn.Click
        sleep(1)

        '/// add the user name 
        printlog "add the user name"		
        UserName.setText sUser
        
        '/// check the password required checkbox
        printlog "check the password required checkbox"        
        PasswordRequired.Check
    
    Kontext "DatabaseWizard"
        FinishBtn.Click
	
    Kontext "SpeichernDlg"
        Dateiname.setText ConvertPath(sFileName)    
        Speichern.click

        sleep(5)
        
    if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
    else        
        printlog "register datasource with name " + sRegister        
        call fRegisterDatabaseFile(ConvertPath(sFilename),sRegister)
    end if  
    
	bOK = true
	if bOK = true then
		printlog "--- End of function - *** Succeed ***"
	else
    	warnlog "--- End of function - *** Failed *** Data source was not created"
	endif	
	
	fCreateMySQL_JDBC_Datasource = bOK
    
	sleep(1)
    
    call fCloseDatabase(true)
        
end function
'-------------------------------------------------------------------------
function  fCreateAdabasDatasource(sFileName,sURL,sUser, optional sRegister) as boolean    
    '/// create a new adabas database file with the given filename 
    '///+ and the given URL
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b>sURL:</b> the URL for the adabas database
    '/// <b>sUser:</b> the user for connect    
    '/// <b><i>optional</i> sRegister:</b> the name under which the database is registered
	dim bOK as boolean
	bOK = FALSE
	
    try
    
        ' delete the file
        if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
            app.kill(ConvertPath(sFileName))
        endif
        
        
      
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
        
        sleep(4)
            
        Kontext "DatabaseWizard"    
            ConnectToDatabase.Check
            sleep(1)        
            DatabaseType.select getIdForDatabaseType("AdabasD") - 1
            sleep(1)
            NextBtn.Click
            sleep(1)        
            '/// insert the adabas database name
            printlog "insert the adabas database name"
            AdabasDBName.setText("sdbc:adabas:" + sURL)
            sleep(1)
            '/// click Next
            printlog "click Next"
            NextBtn.Click
            sleep(1)
            '/// insert the user
            printlog "insert the user"		
            UserName.setText sUser            
            PasswordRequired.Check
            sleep(1)
            '/// click Next
            printlog "click Next"
            NextBtn.Click
            sleep(1)
            '/// click finish
            printlog "click finish"
            FinishBtn.Click
            sleep(1)
        Kontext "SpeichernDlg"
            Dateiname.setText ConvertPath(sFileName)    
            Speichern.click 
            sleep(1)
        
        Kontext "MessageBox" 
            if MessageBox.exists then
                MessageBox.yes
            end if
            
        sleep(2)
            
        ' register the database
        if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister        
            call fRegisterDatabaseFile(ConvertPath(sFilename),sRegister)
        end if   
        
        bOK = true  
        printlog "*** Succeed *** Data source was created"
        
        fCreateAdabasDatasource = bOK
    
        sleep(1)
        call fCloseDatabase(true)
        
    catch
	    bOk = false
    	warnlog "*** Failed *** Data source was not created"
		
	endcatch
	
end function
'-------------------------------------------------------------------------
function  fCreateLocalAdabasDatasource(sFileName,sDatabaseName,sSuperUser,sSuperPassword, optional sRegister) as boolean    
    '/// create a new adabas database file with the given filename 
    '///+ and the given URL
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b>sURL:</b> the URL for the adabas database
    '/// <b>sUser:</b> the user for connect
    '/// <b>sPassword:</b> the password for connect
    '/// <b><i>optional</i> sRegister:</b> the name under which the database is registered
	dim bOK as boolean
	bOK = FALSE
	
    ' delete the file
    if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
        app.kill(ConvertPath(sFileName))
    endif
    
    'try

        '/// --- Create new Adabas data source ---
        printlog "--- Create new adabas data source ---"
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

        sleep(4)

        Kontext "DatabaseWizard"    
            ConnectToDatabase.Check
            sleep(1)        
            DatabaseType.select getIdForDatabaseType("AdabasD")
            sleep(1)
            NextBtn.Click
            sleep(1)


        Kontext "DatabaseWizard"    
            AdabasBrowse.Click
        	sleep(1)
        	
        Kontext "DataSourceSelection"
            Create.Click
			sleep(1)
			
        Kontext "CreateNewAdabasDatabase"
            DatabaseName.setText sDatabaseName
            Administrator.setText sSuperUser
            AdminPasswordBtn.Click
            sleep(1)
            
            
        Kontext "PasswordDLG"
            Password.setText sSuperPassword
            PasswordConfirm.setText sSuperPassword
            PasswordDLG.OK
        
        Kontext "CreateNewAdabasDatabase"
            CreateNewAdabasDatabase.OK
            sleep(10)

        Kontext "DataSourceSelection"
            DataSourceSelection.OK
        
        Kontext "DatabaseWizard"            
            '/// click Next
            printlog "click Next"
            NextBtn.Click
            sleep(1)            
            '/// click Next
            printlog "click Next"
            NextBtn.Click
            sleep(1)
            '/// click finish
            printlog "click finish"
            FinishBtn.Click
            sleep(1)
        Kontext "SpeichernDlg"
            Dateiname.setText ConvertPath(sFileName)    
            Speichern.click 
            
        sleep(2)
            
        ' register the database
        if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister        
            call fRegisterDatabaseFile(ConvertPath(sFilename),sRegister)
        end if   
        
        bOK = true  
        printlog "*** Succeed *** Data source was created"

    'catch
	    bOk = false
    	warnlog "*** Failed *** Data source was not created"
		
	'endcatch
	
    fCreateLocalAdabasDatasource = bOK
    
	sleep(1)
    call fCloseDatabase(true)    
        
end function
'-------------------------------------------------------------------------
function fCreateHSQLDatasource(sFileName, optional sRegister) as boolean
    '/// create a new hsql database file with the given filename    
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b><i>optional</i> sRegister:</b> the name under which the database is registered
	dim bOK as boolean
	bOK = FALSE
	
    ' delete the file
    if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
        app.kill(ConvertPath(sFileName))
    endif
    
    
    try    
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
            Dateiname.setText ConvertPath(sFileName) 
            Speichern.click 
            sleep(10)
            
        Kontext "MessageBox" 
            if MessageBox.exists then
                MessageBox.yes
            end if
            
        sleep(5)

        
        if ( IsMissing(sRegister) ) then
                printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister        
            call fRegisterDatabaseFile(ConvertPath(sFilename),sRegister)
        end if

        call fCloseDatabase(true)  
        
        bOK = true
    catch
        bOK = false
    endcatch
	
	if bOK = true then
		printlog "--- End of function - *** Succeed ***"
	else
    	warnlog "--- End of function - *** Failed *** Data source was not created"
	endif	
	
	fCreateHSQLDatasource = bOK
    
	sleep(1)
    
end function
'-------------------------------------------------------------------------
function fCreateJDBCDatasource(sFileName,sDriverClass,sURL,sClassPath,sUserName, optional sRegister) as boolean     
    'TODO:
        
    'ExitRestartTheOffice ' because the change the classpath wich is only used after a restart
    'Printlog "restart the office to make the classpath work"
    
    '/// create a new JDBC database file with the given filename 
    '///+ and the given URL 
    '/// <u>parameter:</u>
    '/// <b>sFileName:</b> the file name for the database file
    '/// <b>sDatabaseName:</b> the name of the mysql database
    '/// <b>sHost:</b> the host where the mysql database is running
    '/// <b>sPort:</b> the port where the mysql database is running
    '/// <b>sUser:</b> the user for the connection        
    '/// <b><i>optional</i> sRegister:</b> the name under which the database is be registered 
    dim bOK as boolean
	bOK = FALSE
	
    call fSetJDBCDriverFiles(sClassPath)
    
    ' delete the file
    if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
        app.kill(ConvertPath(sFileName))
    endif  
    
    FileOpen "FileName", "private:factory/sdatabase?Interactive" , "FrameName", "_blank" , "SynchronMode" ,True
	
    Kontext "SpeichernDlg"
        Dateiname.setText ConvertPath(sFileName)    
        Speichern.click
        sleep(1)
        
    Kontext "MessageBox" 
        if MessageBox.exists then
            MessageBox.yes
        end if
        
    sleep(5)
        
	Kontext "TabGeneral"	
		DatabaseType.Select "JDBC"
		sleep(1)
    
    Kontext "ConnectionType"    
        NextBtn.Click
        
    Kontext "TabConnection"    
                
        '/// add the URL
        printlog "add the URL"		
        ConnectURL.setText "jdbc:" + sURL
        
        '/// add the user name 
        printlog "add the user name"		
        UserName.setText sUserName
        
        '/// check the password required checkbox
        printlog "check the password required checkbox"        
        PasswordRequired.Check
    
        JDBCDriverClass.setText sDriverClass
        
    
    Kontext "ConnectionType"        
        CreateBtn.Click
	sleep(5)
        
    if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
    else        
        printlog "register datasource with name " + sRegister        
        call fRegisterDatabaseFile(ConvertPath(sFilename),sRegister)
    end if  
    
	bOK = true
	if bOK = true then
		printlog "--- End of function - *** Succeed ***"
	else
    	warnlog "--- End of function - *** Failed *** Data source was not created"
	endif	
	
	fCreateJDBCDatasource = bOK
    
	sleep(1)
    
    call fCloseDatabase(true)    
    
end function       
'-------------------------------------------------------------------------
function fCreateODBCDatasource(sDSName,sUser,sPassword) as boolean
    
    qaerrorlog "not implemented yet"
    fCreateODBCDatasource = false	

end function
'-------------------------------------------------------------------------
function  fCreateDbaseDatasource(sFileName,sURL, optional sRegister) as boolean
    '/// create a new dbase database file with the given filename 
    '///+ and the given URL as dbase path
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b>sURL:</b> the dbase path for the database
    '/// <b><i>optional</i> bRegister:</b> if true the register the database with the name tt_dBase 
    dim bOK as boolean
	bOK = FALSE
	
    ' delete the file
    if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
        app.kill(ConvertPath(sFileName))
    endif    
    
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
    
    sleep(4)
        
    Kontext "DatabaseWizard"    
        ConnectToDatabase.Check
        sleep(1)        
        DatabaseType.select getIdForDatabaseType("dBase", DatabaseType) - 1
        sleep(1)
        NextBtn.Click
        sleep(1)
        dBasePath.setText "sdbc:dbase:" + ConvertPath(sURL)
        sleep(1)
        FinishBtn.Click    
        
    sleep(1)
      
    Kontext "SpeichernDlg"
        Dateiname.setText ConvertPath(sFileName)    
        Speichern.click     
           
    sleep(2)
   
    if ( IsMissing(sRegister) ) then
        printlog "don't register datasource"
    else        
        printlog "register datasource with name " + sRegister
        call fRegisterDatabaseFile(sFileName,sRegister)
    end if    
        
	bOK = true
	'if bOK = true then
    '		printlog "--- End of function - *** Succeed ***"'
	'else
    '	warnlog "--- End of function - *** Failed *** Data source was not created"
	'endif	
	
	fCreatedBaseDatasource = bOK
    
	sleep(1)    
    
    call fClosedatabase(true)
    
end function
'-------------------------------------------------------------------------
function fCreateADODatasource(sFileName,sURL,optional sRegister) as boolean	
    '/// create a new ADO database file with the given filename 
    '///+ and the given URL as dbase path
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b>sURL:</b> the ADO url
    '/// <b><i>optional</i> sRegister:</b> if set, the database is registed under the given name 
    dim bOK as boolean
	bOK = FALSE
	
    ' delete the file
    if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
        app.kill(ConvertPath(sFileName))
    endif    
    
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
    
    sleep(4)
    Kontext "DatabaseWizard"    
        ConnectToDatabase.Check
        sleep(1)        
        DatabaseType.select getIdForDatabaseType("ADO", DatabaseType)
        sleep(1)
        NextBtn.Click
        sleep(1)
        '/// add the URL
        printlog "add the URL " + sURL		
        AdoURL.setText "sdbc:ado:" + sURL        
        sleep(1)
        FinishBtn.Click
        sleep(1)
    
    Kontext "SpeichernDlg"
        Dateiname.setText ConvertPath(sFileName)    
        Speichern.click
        
    sleep(5)
   
    if ( IsMissing(sRegister) ) then
        printlog "don't register datasource"
    else        
        printlog "register datasource with name " + sRegister
        call fRegisterDatabaseFile(sFileName,sRegister)
    end if    
        
	bOK = true
	if bOK = true then
		printlog "--- End of function - *** Succeed ***"
	else
    	warnlog "--- End of function - *** Failed *** Data source was not created"
	endif	
	
	fCreateADODatasource = bOK
    
	sleep(1)    
    
    call fClosedatabase(true)    
 
end function
'-------------------------------------------------------------------------
function fCreateTextDatasource(sFileName,sURL, optional sRegister) as boolean	
    '/// create a new text database file with the given filename 
    '///+ and the given URL as dbase path
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b>sURL:</b> the Text url
    '/// <b><i>optional</i> sRegister:</b> if set, the database is registed under the given name    
    Dim bOK as boolean		
	bOK = false	
	
    '/// --- Create new Text data source ---
    printlog "--- Create new " + sFileName + " data source ---"
	printlog "--- with the URL " + sURL + " ---"
    
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
    
    sleep(4)
        
    Kontext "DatabaseWizard"    
        ConnectToDatabase.Check
        sleep(1)        
        DatabaseType.select getIdForDatabaseType("Text", DatabaseType) - 1
        sleep(1)
        NextBtn.Click
        sleep(1)
        TextPath.setText "sdbc:flat:" + ConvertPath(sURL)
        sleep(1)
        FinishBtn.Click
        sleep(1)
	
    Kontext "SpeichernDlg"
        Dateiname.setText ConvertPath(sFileName)    
        Speichern.click
        sleep(1)
        
    Kontext "MessageBox" 
        if MessageBox.exists then
            MessageBox.yes
        end if
        
    sleep(5)

    if ( IsMissing(sRegister) ) then
        printlog "don't register datasource"
    else        
        printlog "register datasource with name " + sRegister
        call fRegisterDatabaseFile(sFileName,sRegister)
    end if    
     
        
	bOK = true
	if bOK = true then
		printlog "--- End of function - *** Succeed ***"
	else
    	warnlog "--- End of function - *** Failed *** Data source was not created"
	endif	
	
	fCreateTextDatasource = bOK
    
	sleep(1)    
    
    call fClosedatabase(true)    
  
end function
'-------------------------------------------------------------------------
function fCreateSpreadsheetDatasource(sFileName,sURL, optional sRegister) as boolean
    '/// create a new spreadsheet database file with the given filename 
    '///+ and the given URL as dbase path
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b>sURL:</b> the spreadsheet file name
    '/// <b><i>optional</i> bRegister:</b> if true the register the database with the given name 
    dim bOK as boolean
	bOK = FALSE
	
    ' delete the file
    if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
        app.kill(ConvertPath(sFileName))
    endif    
    
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
        
    sleep(4)
            
    Kontext "DatabaseWizard"    
        ConnectToDatabase.Check
        sleep(1)        
        DatabaseType.Select (getIdForDatabaseType("CALC", DatabaseType) - 1) ' -1 because in the wizard there is on datasource less because of mysql
        sleep(1)
        NextBtn.Click
        sleep(1)
        '/// add the URL
        printlog "add the URL"		
        SpreadsheetPath.setText "sdbc:calc:" + ConvertPath(sURL)
        sleep(1)
        '/// click finish
        printlog "click finish"
        FinishBtn.Click
        sleep(1)
    Kontext "SpeichernDlg"
        Dateiname.setText ConvertPath(sFileName)    
        Speichern.click 
        sleep(1)
    
    Kontext "MessageBox" 
        if MessageBox.exists then
            MessageBox.yes
        end if
        
    sleep(2)
   
    if ( IsMissing(sRegister) ) then
        printlog "don't register datasource"
    else        
        printlog "register datasource with name " + sRegister
        call fRegisterDatabaseFile(sFileName,sRegister)
    end if    
        
	bOK = true
	if bOK = true then
		printlog "--- End of function - *** Succeed ***"
	else
    	warnlog "--- End of function - *** Failed *** Data source was not created"
	endif	
	
	fCreateSpreadsheetDatasource = bOK
    
	sleep(1)    
    
    call fClosedatabase(true)    
 
end function
'-------------------------------------------------------------------------
function fCreateLDAPAddressbookDatasource(sFileName,sHostName,sBaseDN,sPortNr,optional sRegister) as boolean
    '/// create a new LDAP Addressbook database with the given filename     
    '/// <u>parameter:</u>
    '/// <b>sFileName:</b> the file name for the database file
    '/// <b><i>optional</i> sRegister:</b> if true then register the database
    dim bOK as boolean
	bOK = FALSE
'	try
    
        ' delete the file
        if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
            app.kill(ConvertPath(sFileName))
        endif
    
        
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
    
        sleep(4)
        
        Kontext "DatabaseWizard"    
            ConnectToDatabase.Check
            sleep(1)        
            DatabaseType.select getIdForDatabaseType("LDAP", DatabaseType)
            sleep(1)
            NextBtn.Click
            sleep(1)
            LDAPHostName.setText sHostName
            sleep(1)
            BaseDN.setText(sBaseDN)
            sleep(1)
            FinishBtn.Click
            sleep(1)
	
    Kontext "SpeichernDlg"
        Dateiname.setText ConvertPath(sFileName)    
        Speichern.click
        sleep(1)
        
    Kontext "MessageBox" 
        if MessageBox.exists then
            MessageBox.yes
        end if
        
      
        sleep(5)
   
        if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister
            call fRegisterDatabaseFile(sFileName,sRegister)
        end if
        printlog "*** Succeed *** Data source was created"            
        bOK = true
'	catch
'    	warnlog "*** Failed *** Data source was not created"
'        bOK = false
'    endcatch	
	
	fCreateLDAPAddressbookDatasource = bOK
    
	sleep(1)    
    
    call fClosedatabase(true)
    
end function
'-------------------------------------------------------------------------
function fCreateEvolutionAddressbookDatasource(sFileName, optional sRegister) as boolean
    '/// create a new Evolution Addressbook database with the given filename     
    '/// <u>parameter:</u> 
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b><i>optional</i> sRegister:</b> if true then register the database
    dim bOK as boolean
	bOK = FALSE
	try
    
        ' delete the file
        if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
            app.kill(ConvertPath(sFileName))
        endif
    
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
        
        sleep(4)
            
        Kontext "DatabaseWizard"    
            ConnectToDatabase.Check
            sleep(1)        
            DatabaseType.select getIdForDatabaseType("Evolution", DatabaseType)
            sleep(1)
            NextBtn.Click
            sleep(1)
            '/// click finish
            printlog "click finish"
            FinishBtn.Click
            sleep(1)
        Kontext "SpeichernDlg"
            Dateiname.setText ConvertPath(sFileName)    
            Speichern.click 
            sleep(1)
        
        Kontext "MessageBox" 
            if MessageBox.exists then
                MessageBox.yes
            end if
            
        sleep(2)
   
        if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister
            call fRegisterDatabaseFile(sFileName,sRegister)
        end if

        call fClosedatabase(true)
   
        printlog "*** Succeed *** Data source was created"            
        bOK = true
	catch
    	warnlog "*** Failed *** Data source was not created"
        bOK = false
    endcatch	
	
	fCreateEvolutionAddressbookDatasource = bOK
    
	sleep(1)
    
end function
'-------------------------------------------------------------------------
function fCreateKDEAddressbookDatasource(sFileName , optional sRegister) as boolean
    '/// create a new KDE Addressbook database with the given filename     
    '/// <u>parameter:</u>
    '/// <b>sFileName:</b> the file name for the database file
    '/// <b><i>optional</i> sRegister:</b> if true then register the database
    dim bOK as boolean
	bOK = FALSE
	try
    
        ' delete the file
        if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
            app.kill(ConvertPath(sFileName))
        endif
    
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
        
        sleep(4)
            
        Kontext "DatabaseWizard"    
            ConnectToDatabase.Check
            sleep(1)        
            DatabaseType.select getIdForDatabaseType("KDE", DatabaseType)
            sleep(1)
            NextBtn.Click
            sleep(1)
            '/// click finish
            printlog "click finish"
            FinishBtn.Click
            sleep(1)
        Kontext "SpeichernDlg"
            Dateiname.setText ConvertPath(sFileName)    
            Speichern.click 
            sleep(1)
        
        Kontext "MessageBox" 
            if MessageBox.exists then
                MessageBox.yes
            end if
            
        sleep(2)
   
        if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister
            call fRegisterDatabaseFile(sFileName,sRegister)
        end if

        call fClosedatabase(true)

        printlog "*** Succeed *** Data source was created"            
        bOK = true
	catch
    	warnlog "*** Failed *** Data source was not created"
        bOK = false
    endcatch	
	
	fCreateKDEAddressbookDatasource = bOK
    
	sleep(1)
    
end function
'-------------------------------------------------------------------------
function fCreateWindowsAddressbookDatasource(sFileName , optional sRegister) as boolean
    '/// create a new Windows Addressbook database with the given filename     
    '/// <u>parameter:</u>
    '/// <b>sFileName:</b> the file name for the database file
    '/// <b><i>optional</i> sRegister:</b> if true then register the database
    dim bOK as boolean
	bOK = FALSE
	try
    
        ' delete the file
        if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
            app.kill(ConvertPath(sFileName))
        endif
    
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
        
        sleep(4)
            
        Kontext "DatabaseWizard"    
            ConnectToDatabase.Check
            sleep(1)        
            DatabaseType.select getIdForDatabaseType("Windows", DatabaseType)
            sleep(1)
            NextBtn.Click
            sleep(1)
            '/// click finish
            printlog "click finish"
            FinishBtn.Click
            sleep(1)
        Kontext "SpeichernDlg"
            Dateiname.setText ConvertPath(sFileName)    
            Speichern.click 
            sleep(1)
        
        Kontext "MessageBox" 
            if MessageBox.exists then
                MessageBox.yes
            end if
            
        sleep(2)
   
        if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister
            call fRegisterDatabaseFile(sFileName,sRegister)
        end if

        call fClosedatabase(true)

        printlog "*** Succeed *** Data source was created"            
        bOK = true
	catch
    	warnlog "*** Failed *** Data source was not created"
        bOK = false
    endcatch	
	
	fCreateWindowsAddressbookDatasource = bOK
    
	sleep(1)
    
end function
'-------------------------------------------------------------------------
function fCreateOutlookAddressbookDatasource(sFileName, optional sRegister) as boolean
    '/// create a new Outlook Addressbook database with the given filename     
    '/// <u>parameter:</u>
    '/// <b>sFileName:</b> the file name for the database file
    '/// <b><i>optional</i> sRegister:</b> if true then register the database
    dim bOK as boolean
	bOK = FALSE
	try
    
        ' delete the file
        if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
            app.kill(ConvertPath(sFileName))
        endif
    
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
        
        sleep(4)
            
        Kontext "DatabaseWizard"    
            ConnectToDatabase.Check
            sleep(1)        
            DatabaseType.select getIdForDatabaseType("Outlook", DatabaseType)
            sleep(1)
            NextBtn.Click
            sleep(1)
            '/// click finish
            printlog "click finish"
            FinishBtn.Click
            sleep(1)
        Kontext "SpeichernDlg"
            Dateiname.setText ConvertPath(sFileName)    
            Speichern.click 
            sleep(1)
        
        Kontext "MessageBox" 
            if MessageBox.exists then
                MessageBox.yes
            end if
            
        sleep(2)
   
        if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister
            call fRegisterDatabaseFile(sFileName,sRegister)
        end if

        call fClosedatabase(true)

        printlog "*** Succeed *** Data source was created"            
        bOK = true
	catch
    	warnlog "*** Failed *** Data source was not created"
        bOK = false
    endcatch	
	
	fCreateOutlookAddressbookDatasource = bOK
    
	sleep(1)

end function
'-------------------------------------------------------------------------   
function fCreateMozillaAddressbookDatasource(sFileName, optional sRegister) as boolean
    '/// create a new mozilla addressbook database file with the given filename     
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file    
    '/// <b><i>optional</i> sRegister:</b> the name under which the database is registered

    dim bOK as boolean
	bOK = FALSE
	try    
        ' delete the file
        if ( app.Dir( ConvertPath(sFileName) ) ) <> "" then
            app.kill(ConvertPath(sFileName))
        endif
    
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
        
        sleep(4)
            
        Kontext "DatabaseWizard"    
            ConnectToDatabase.Check
            sleep(1)        
            DatabaseType.select getIdForDatabaseType("Mozilla", DatabaseType) - 1
            sleep(1)
            NextBtn.Click
            sleep(1)
            '/// click finish
            printlog "click finish"
            FinishBtn.Click
            sleep(1)
        Kontext "SpeichernDlg"
            Dateiname.setText ConvertPath(sFileName)    
            Speichern.click 
            sleep(1)
        
        Kontext "MessageBox" 
            if MessageBox.exists then
                MessageBox.yes
            end if
            
        sleep(2)
   
        if ( IsMissing(sRegister) ) then
            printlog "don't register datasource"
        else        
            printlog "register datasource with name " + sRegister
            call fRegisterDatabaseFile(sFileName,sRegister)
        end if
        printlog "*** Succeed *** Data source was created"            
        bOK = true

        call fClosedatabase(true)

	catch
    	warnlog "*** Failed *** Data source was not created"
        bOK = false
    endcatch	
	
	fCreateMozillaAddressbookDatasource = bOK
    
	sleep(1)  
    
end function
'--------------------------------------------------------------------
function fRegisterDatabaseFile( sFileName, sName)
    '/// register a database     
    '/// <u>parameter:</u>
    '/// <b>sFielName:</b> the file name for the database file
    '/// <b>sName:</b> The name under which the database is registered   
    
    dim i as integer
    
    ToolsOptions
    
    call hToolsOptions ( "Datasources", "Databases" )
    
    '/// remove registration if exists
    
    Kontext "TabRegisteredDatabase"
        for i = 1 to RegisteredDatabases.GetItemCount
            RegisteredDatabases.select i
            if RegisteredDatabases.getSeltext() = sName then
			    DeleteBtn.Click	
                Kontext "Active"
                    Active.yes
                exit for
            endif    
        next
        
    
    
    Kontext "TabRegisteredDatabase"
        NewBtn.Click
    sleep(1)
    Kontext "CreateDatabaseLink"
        DatabaseFile.setText(sFileName)    
        RegisteredName.setText(sName)
        CreateDatabaseLink.OK
        
    sleep(1)
    Kontext "OptionenDlg"
        OptionenDlg.OK

end function
'--------------------------------------------------------------------
function fDeRegisterDatabaseFile( sName)
    '/// deregister a database     
    '/// <u>parameter:</u>    
    '/// <b>sName:</b> The registered name which should be deregistered    
    dim i as integer
    
    ToolsOptions
    
    call hToolsOptions ( "Datasources", "Databases" )
    
    '/// remove registration for the database
    printlog "remove registration for the database " + sName
    
    Kontext "TabRegisteredDatabase"
        for i = 1 to RegisteredDatabases.GetItemCount
            RegisteredDatabases.select i
            if RegisteredDatabases.getSeltext() = sName then
			    DeleteBtn.Click	
                exit for
            endif    
        next
        
   try
    Kontext "Active"
        Active.yes
   catch
   
   endcatch 
    sleep(1)
    Kontext "OptionenDlg"
        OptionenDlg.OK

end function
'-------------------------------------------------------------------------
function fSetJDBCDriverFiles(sClassPath) as boolean    
    '/// set the java classpath     
    '/// <u>parameter:</u>    
    '/// <b>sClassPath:</b> The Classpath 
    
    Dim x_DriverInstall as boolean
    x_DriverInstall = FALSE

    call hNewDocument
    printlog "insert classpath: " + sClassPath
    ToolsOptions
    call hToolsOptions ( "StarOffice", "JAVA" )

    Kontext "TabJava"    
        ClassPath.Click
        sleep(1)
    Kontext "JavaClassPath"
        AddArchive.Click     
        sleep(1)
    Kontext "GeneralFileDialog"
        Dateiname.setText ConvertPath(sClassPath)    
        Auswaehlen.Click
        sleep(1)
    '/// if the class file allready exists a messages box pops up
    Kontext "MessageBox"
    	if ( MessageBox.exists(3) ) then
    		MessageBox.OK
		end if

    Kontext "JavaClassPath"
        JavaClassPath.OK    
        
    Kontext "MessageBox"
    	if ( MessageBox.exists(3) ) then
    		MessageBox.OK
		end if
        
    Kontext "OptionenDlg"
        OptionenDlg.OK
            
    call hCloseDocument
    
end function
'-------------------------------------------------------------------------
function getIdForDatabaseType(sType as String ) as Integer
    '/// get the index of the database type in the database type list box
    '///+ in the connection type dialog     
    '/// <u>parameter:</u>    
    '/// <b>sType:</b> The name of the database type in english 

    dim ii as integer

    if (sType = "AdabasD") then        
        ii = 4        
    elseif (sType = "dBase") then
        if gPlatform = "osx" then        
         ii = 5
        else    
         ii = 6
        endif
    elseif (sType = "Access") then    
        ii = 0
    elseif (sType = "Evolution") then        
        ii = 13
    elseif (sType = "JDBC") then
        if gPlatform = "osx" then        
         ii = 7
        else
         ii = 8
        endif
    elseif (sType = "LDAP") then             
        if gPlatGroup = "w95" then        
         ii = 12        
        else      
         ii = 11
        endif  
    elseif (sType = "Mozilla") then   
        if gPlatGroup = "w95" then        
         ii = 11
        elseif gPlatform = "osx" then        
         ii = 9
        else      
         ii = 10
        endif
    elseif (sType = "MySQLJDBC") then        
        ii = 2
    elseif (sType = "MySQLODBC") then
        ii = 1                    
    elseif (sType = "ODBC") then
        if gPlatform = "osx" then        
         ii = 8
        else
         ii = 9
        endif
    elseif (sType = "OracleJDBC") then        
        ii = 3                    
    elseif (sType = "CALC") then        
        if gPlatform = "osx" then        
         ii = 4
        else
         ii = 5
        endif
    elseif (sType = "Text") then
        if gPlatform = "osx" then        
         ii = 6
        else
         ii = 7
        endif 
    elseif (sType = "ADO") then
        ii = 9 ' only 9 because on windows +1 is added at the end
    elseif (sType = "Windows") then ' windows addressbook
        ii = 14 ' only 14 because on windows +1 is added at the end    
    elseif (sType = "KDE") then
        ii = 12
    else
        ii = 0
    endif

    ' when windows then +1 because the first entry 'MS Access' is only available under windows
    if gPlatGroup = "w95" then        
            ii = ii + 1    
    ' when x86 then -1 because the Adabas entry on the 4. position is missing
    elseif gPlatForm = "x86" then        
        if ii > 3 then
            ii = ii - 1
        endif
    endif
    
    getIdForDatabaseType = ii
    
end function


