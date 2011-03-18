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
'* short description : Helper Routines for Base tests.
'*
'***************************************************************************************
'*
' #1 fSelectDatasourceInBeamer
' #1 fConnectToDatasourceInBeamer
' #1 fSelectSchemaInTableFilter
' #1 fOpenNewDataBase
' #1 fOpenDataBase
' #1 fCloseDatabase
' #1 tools_dbtools_fgetMySQLDatabaseProperties
'*
'\***********************************************************************************

private const VERBOSE = true

'--------------------------------------------------------------------
function fSelectDatasourceInBeamer(sDSName1)
	'/// select a datasource in the beamer with the given name
    '/// <u>parameter:</u>
    '/// <b>sDSName1:</b> the name of the datasource which shall be seleced
            
	dim i as integer
	dim bfindDS as boolean
	dim iNoDS as integer    
    
    bfindDS = false
	Kontext "DatabaseBeamer"
    Kontext "DatabaseSelection"
		
    iNoDS = DatabaseSelection.getItemCount	
	for i = 1 to iNoDS
		DatabaseSelection.Select i
		if DatabaseSelection.getText = sDSName1 then
			i = iNoDS
            bfindDS = TRUE
		endif
	next i	
	
    fSelectDatasourceInBeamer = bfindDS
	
    sleep(1)
    
end function
'--------------------------------------------------------------------
function fConnectToDatasourceInBeamer(sDSName , sPWD, optional bNewDoc)
    
    if ( isMissing (bNewDoc) )then
        bNewDoc = false
    endif
    
    if (bNewDoc) then
        call hNewDocument()
        ViewCurrentDatabase
    endif
    
    sleep(1)
    dim bConnect as boolean 
    bConnect = false
    if fFindDatasource(sDSName) = true then
        DatabaseSelection.TypeKeys "<ADD>"
        DatabaseSelection.TypeKeys "<DOWN>"
        DatabaseSelection.TypeKeys "<DOWN>"
        DatabaseSelection.TypeKeys "<DOWN>"
        DatabaseSelection.TypeKeys "<ADD>"                
        Kontext "LoginDialog"
        if LoginDialog.Exists(3) then
            LoginPassword.SetText sPWD
            LoginDialog.OK
        end if
        bConnect = true    
    else
        warnlog "Could not found the datasource " + sDSName
    endif
    
    if (bNewDoc) then
        call hCloseDocument
    endif
    
    fConnectToDatasource = bConnect
    
end function
'--------------------------------------------------------------------
function fSelectSchemaInTableFilter(sSchema as String)
	'/// select the given schema in the table filter dialog in an open database
	'/// <u>parameter:</u>
	'/// <b>sSchema:</b> the name of the schema
    dim bOK as boolean
    bOK = false
    
    dim iRem as integer
	iRem = 0
    Kontext "DATABASE"
        Database.useMenu
        hMenuSelectNr 5
        hMenuSelectNr 3
    
     
    Kontext "TablesFilter"
        printlog Tables.getItemCount
        TablesFilter.OK
        
    fSelectSchemaInTableFilter = true
    
end function
'-------------------------------------------------------------------------
function fOpenNewDataBase ( sSaveFileName )    
    '/// open a database window and sabe it to the given name
    '/// <u>parameter:</u>
    '/// <b>sSaveFileName:</b> the file name for the database file
    FileOpen "FileName", "private:factory/sdatabase" , "FrameName", "_blank" , "SynchronMode" ,True
    
    sleep(2)
    
    Kontext "SpeichernDlg"
        Dateiname.setText Convertpath(gOfficePath + "/user/work/" + sSaveFileName)    
        Speichern.click
        sleep(1)
        
    Kontext "MessageBox" 
        if MessageBox.exists then
            MessageBox.yes
        end if
        
    sleep(5)

end function
'-------------------------------------------------------------------------
function fOpenDataBase ( sFileName , optional sPassword)    
    '/// open a database file with the given name and try to connect
    '/// <u>parameter:</u>
    '/// <b>sFileName:</b> the file name which shall be open
    '/// <b><i>optional</i>sFileName:</b> the password for connecting                

    Dim bOK as boolean
    bOK = true
    
    Dim DieDatei as String
    DieDatei = ConvertPath ( sFileName )

    if(VERBOSE) then
        printlog "Filename : " + sFileName
        if(NOT isMissing(sPassword)) then
            printlog "Password : " + sPassword
        endif
    endif

    '/// open the File Open dialog
    printlog "open the File Open dialog"
    FileOpen
    sleep (1)
    Kontext "OeffnenDlg"
    '/// open the given file    
    Dateiname.SetText DieDatei
    sleep (3)
    Oeffnen.Click
    sleep (3)
    
    'click on the main window to get the focus
    Kontext "DATABASE"    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50) 
    
    ViewTables
    sleep(1)
    
    Kontext "DATABASE"
    '/// if the password is given add the password    
    if ( Not IsMissing(sPassword) ) then
        printlog "set password : " + sPassword
        Kontext "LoginDialog"
        if LoginDialog.Exists then
         	LoginPassword.setText sPassword
         	LoginDialog.OK
            sleep(1)
        endif
    endif
    Kontext "MessageBox"
    if (MessageBox.exists(2)) then
        'an error occurr    
        bOK = false
        dim iNrTexts as integer
        Dim i as integer
        iNrtexts = MessageBox.getFixedtextCount
        for i = 1 to iNrtexts
            qaerrorlog MessageBox.getFixedtext(i)
        next
        MessageBox.OK
        ' try again to close the dialog if exists
        if (MessageBox.exists(2)) then 	       
    	    MessageBox.OK
    	endif
        call fCloseDatabase
    endif
    
    fOpenDataBase = bOK
    
end function
'-------------------------------------------------------------------------
function fCloseDatabase(optional bSave)
    '/// close the database
    '/// <u>parameter:</u>
    '/// <b><i>optional</i>bSave:</b> if true the the database is saved if false then the changes are lost
    sleep(1)
 	Kontext "DATABASE"
        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
        Database.useMenu
        hMenuSelectNr 1
        hMenuSelectNr 5    
        
    Kontext "Messagebox"
        if Messagebox.Exists(3) then
            if ( IsMissing( bSave ) ) then
                Messagebox.No
            else
                if bSave then
                    Messagebox.Yes
                else
                    Messagebox.No
                endif
            endif
        end if
        
    sleep(1)        
        
end function
'-------------------------------------------------------------------------
sub sCloseAllToolbars
    
    Kontext "FormControls"
    if FormControls.exists(1) then 
        FormControls.close()
    endif

    Kontext "MoreControls"
    if MoreControls.exists(1) then
        MoreControls.close()
    endif

    Kontext "FormDesignTools"
    if FormDesignTools.exists(1) then
        FormDesignTools.close()
    endif

end sub
'-------------------------------------------------------------------------
function tools_dbtools_fgetMySQLJDBCDatabaseProperties(sDBConfigFileName as string)
    '/// return the databaseserver properties from the environment directory
	'/// <u>parameter:</u>
	'/// <br>
    '/// <u>return:</u>
	'/// the properties as an array
    '/// entry 1 defined (yes or no)
    '/// entry 2 server name 
    '/// entry 3 database name
    '/// entry 4 database port
    '/// entry 5 database user name
    '/// entry 6 database user password
    '/// entry 7 jdbc driver file path

    tools_dbtools_fgetMySQLJDBCDatabaseProperties = fgetGenericDatabaseProperties( sDBConfigFileName, "mysql_jdbc" )

end function
'-------------------------------------------------------------------------
function tools_dbtools_fgetMySQLODBCDatabaseProperties()
    '/// return the databaseserver properties from the environment directory
	'/// <u>parameter:</u>
	'/// <br>
    '/// <u>return:</u>
	'/// the properties as an array
    '/// entry 1 defined (yes or no)
    '/// entry 2 odbc dsn name
    '/// entry 3 database user name
    '/// entry 4 database user password        

    dim i as integer
    dim b(10000) as String
    dim c
    c = hGetDataFileSection( gTesttoolPath + gPrivateEnvironmentLocation + "input/base.txt", b(), "mysql_odbc" , "" , "" )
    
    ' split the string at = and return the right part 
    for i = 1 to 4
        'printlog b(i)
        b(i) = Right(b(i),len(b(i))-Instr(b(i),"="))    
        'printlog b(i)        
    next
    
    tools_dbtools_fgetMySQLODBCDatabaseProperties = b() 

end function
'-------------------------------------------------------------------------
function tools_dbtools_fgetAdabasDatabaseProperties()
    '/// return the databaseserver properties from the environment directory
	'/// <u>parameter:</u>
	'/// <br>
    '/// <u>return:</u>
	'/// the properties as an array
    '/// entry 1 defined (yes or no)
    '/// entry 2 server name 
    '/// entry 3 database name    
    '/// entry 4 database user name
    '/// entry 5 database user password        

    dim i as integer
    dim b(10000) as String
    dim c
    c = hGetDataFileSection( gTesttoolPath + gPrivateEnvironmentLocation + "input/base.txt", b(), "adabas" , "" , "" )
    
    ' split the string at = and return the right part 
    for i = 1 to 5
        'printlog b(i)
        b(i) = Right(b(i),len(b(i))-Instr(b(i),"="))    
        'printlog b(i)        
    next
    
    tools_dbtools_fgetAdabasDatabaseProperties = b() 

end function
'-------------------------------------------------------------------------------
function fgetGenericDatabaseProperties(sDBConfigFileName as string, sType as String)

    dim i as integer
    dim b(10000) as String
    dim c
    c = hGetDataFileSection( sDBConfigFileName, b(), sType , "" , "" )

    ' split the string at = and return the right part
    for i = 1 to 7
        if(VERBOSE) then
            printlog "orig. string : " + b(i)
        endif
        b(i) = Right(b(i),len(b(i))-Instr(b(i),"="))
        if(VERBOSE) then
            printlog "string after the '=' : " + b(i)
        endif
    next




    fgetGenericDatabaseProperties = b()

end function
