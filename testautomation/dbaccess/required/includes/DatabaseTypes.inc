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
'* short description : test the database types
'*
'************************************************************************
'*    
' #1 tMySQLODBCProperties
' #1 tMySQLJDBCProperties
' #1 tAdabasProperties
' #1 tOracleJDBCProperties
' #1 tJDBCProperties
' #1 tODBCProperties
' #1 tdBaseProperties
' #1 tTextProperties
' #1 tSpreadsheetProperties
' #1 tMozillaProperties
' #1 tLDAPProperties
' #1 tEvolutionProperties
' #1 tCheckHSQLDB
'*
'\***********************************************************************************
sub DatabaseTypes

    printlog "------------------ DatabaseTypes.inc ---------------------"
    
    call tMySQLODBCProperties
    call tMySQLJDBCProperties
    call tAdabasProperties
    call tOracleJDBCProperties
    call tJDBCProperties
    call tODBCProperties
    call tdBaseProperties
    call tTextProperties
    call tSpreadsheetProperties
    call tMozillaProperties
    call tLDAPProperties
    call tEvolutionProperties
    call tCheckHSQLDB

end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tMySQLODBCProperties 
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms
    
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("sdbc:mysql:odbc:*"))
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        call fCheckConnectionTabPage("MySQLODBC")        
    
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        
    Kontext "TabMySQLODBC"
        '/// check the MySQLODBC page        
        call fCheckTabPage(TabMySQLODBC)
    
    Kontext "ConnectionType"        
        ConnectionType.OK
        
    call fCheckAdvancedDialog("MySQLODBC")        
        
    call fCloseDatabase()
    
endcase
'-------------------------------------------------------------------------
testcase tMySQLJDBCProperties 
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
    
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("sdbc:mysql:jdbc:*"))    
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("MySQLJDBC")
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        
    Kontext "TabMySQLJDBC"
        '/// check the MySQLJDBC page        
        call fCheckTabPage(TabMySQLJDBC)
    
    Kontext "ConnectionType"        
        ConnectionType.OK
        
    call fCheckAdvancedDialog("MySQLJDBC")
        
    call fCloseDatabase()
    
endcase
'-------------------------------------------------------------------------
testcase tAdabasProperties 

    if gPlatform = "x86" then
        printlog "Adabas is not available under x86."
        goto endsub    
    end if
    
    if gPlatform = "osx" then
        printlog "Adabas doesn't exists under mac osx."
        goto endsub    
    end if

    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    sleep(2)        
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("sdbc:adabas:*"))    
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("Adabas")
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        
    Kontext "TabAdabas"
        '/// check the Adabas page                
        call fCheckTabPage(TabAdabas)
    
    Kontext "ConnectionType"        
        ConnectionType.OK
    
    call fCheckAdvancedDialog("Adabas")
    
    call fCloseDatabase()
    
endcase
'-------------------------------------------------------------------------
testcase tOracleJDBCProperties
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("jdbc:oracle:thin:*"))    
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("OracleJDBC")
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        
    Kontext "TabOracleJDBC"
        '/// check the TabOracleJDBC page        
        call fCheckTabPage(TabOracleJDBC)
    
    Kontext "ConnectionType"        
        ConnectionType.Cancel
        
    call fCloseDatabase()    
    
endcase
'-------------------------------------------------------------------------
testcase tJDBCProperties 
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("jdbc:*"))    
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("JDBC")
        
    Kontext "ConnectionType"        
        ConnectionType.Cancel
        
    call fCloseDatabase()
    
endcase
'-------------------------------------------------------------------------
testcase tLDAPProperties 

    if gPlatform = "osx" then
        printlog "LDAP doesn't exists under mac osx."
        goto endsub    
    end if
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("sdbc:address:ldap:*"))
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("LDAP")
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        
    Kontext "TabLDAP"
        '/// check the LDAP page        
        call fChecktabPage(TabLDAP)
    
    Kontext "ConnectionType"        
        ConnectionType.Cancel
        
    call fCloseDatabase()  
    
endcase
'-------------------------------------------------------------------------
testcase tEvolutionProperties    

    if gPlatform <> "lin" then
        printlog "Evolution does only exists under linux."
        goto endsub    
    end if

    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        try
            if ( isDriverInstalled("sdbc:address:evolution:ldap")) then
                DataBaseType.select(hGetDatabaseDisplayName("sdbc:address:evolution:ldap"))
                Kontext "ConnectionType"        
                ConnectionType.OK
            else
                qaerrorlog "Evolution does not appear to be available on this system"
                Kontext "ConnectionType"
                ConnectionType.Cancel
            endif
        catch
            warnlog "failed to select DataBaseType Evolution - maybe not installed or activated in this build"
            TabGeneral.Cancel
            call fCloseDatabase
            goto endsub
        endcatch
        
    'call fCheckAdvancedDialog("Evolution")
        
    call fCloseDatabase()  
    
endcase
'-------------------------------------------------------------------------
testcase tMozillaProperties 

    if gPlatform = "osx" then
        printlog "LDAP doesn't exists under mac osx."
        goto endsub    
    end if
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        try
            if ( isDriverInstalled("sdbc:address:mozilla:")) then
                DataBaseType.select(hGetDatabaseDisplayName("sdbc:address:mozilla:"))
                Kontext "ConnectionType"
                ConnectionType.OK
            else
                qaerrorlog "Mozilla does not appear to be available on this system"
                Kontext "ConnectionType"
                ConnectionType.Cancel
            endif
        catch
            warnlog "failed to select DataBaseType Mozilla - maybe not installed or activated in this build"
            TabGeneral.Cancel
            call fCloseDatabase
            goto endsub
        endcatch
        

    'add sleep to give OOo time to react 
    sleep(2)    
        
    'call fCheckAdvancedDialog("Mozilla")
    
    'add sleep to give OOo time to react 
    sleep(2)
    
    call fCloseDatabase()  
    
endcase
'-------------------------------------------------------------------------
testcase tSpreadsheetProperties 
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("sdbc:calc:*"))    
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("CALC")
    
    Kontext "ConnectionType"        
        ConnectionType.Cancel
        
    'call fCheckAdvancedDialog("CALC")
        
    call fCloseDatabase()  
    
endcase
'-------------------------------------------------------------------------
testcase tTextProperties 
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("sdbc:flat:*"))
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("Text")
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        
    Kontext "TabTextBase"
        '/// check the Text page        
        call fCheckTabPage(TabTextBase)
    
    Kontext "ConnectionType"        
        ConnectionType.ok
        
    call fCheckAdvancedDialog("Text")
        
    call fCloseDatabase()  
    
endcase
'-------------------------------------------------------------------------
testcase tdBaseProperties
    
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("sdbc:dbase:*"))
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("dBase")
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        
    Kontext "TabdBase"
        '/// check the dBase page        
        call fCheckTabPage(TabdBase)
    
    Kontext "ConnectionType"        
        ConnectionType.OK
        
    call fCheckAdvancedDialog("dBase")
        
    call fCloseDatabase()  
    
endcase
'-------------------------------------------------------------------------
testcase tODBCProperties
 
    call fOpenDatabase (gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms    
            
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(2)
    
    Kontext "TabGeneral"    
        call DialogTest(TabGeneral)
        DataBaseType.select(hGetDatabaseDisplayName("sdbc:odbc:*"))    
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)        
        call fCheckConnectionTabPage("ODBC")
        
    Kontext "ConnectionType"        
        NextBtn.Click
        sleep(1)
        
    Kontext "TabODBC"
        '/// check the ODBC page        
        call fCheckTabPage(TabODBC)
    
    Kontext "ConnectionType"        
        ConnectionType.OK
        
    call fCheckAdvancedDialog("ODBC")        
        
    call fCloseDatabase()    
   
endcase
'-------------------------------------------------------------------------
testcase tCheckHSQLDB

    Dim sDatasourceFileName as String
    sDatasourceFileName = ConvertPath(gOfficePath + "user/work/hsqldb.odb")
    call fCreateHSQLDatasource(sDatasourceFileName)
    
    call fOpenDatabase(sDatasourceFileName) 

    Kontext "ContainerView"
        ViewTables
  
    call fCloseDatabase()

endcase
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
function fCheckAdvancedDialog(sType as string)
    
    printlog "+Check the Advanced Settings for the datasource type " + sType
    
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(3)    

    Kontext "Advanced"        
        
        if sType = "OracleJDBC" OR sType = "JDBC" OR sType = "ODBC" then
            TabControl.setPage AdvancedSettingsSpecial
            call fcheckControl(SQL92Check)
            call fcheckControl(AppendAlias)
            call fcheckControl(Ignoreprivileges)
            call fcheckControl(ParameterSubstiti)
            call fcheckControl(DisplayVersionColumns)
            call fcheckControl(BooleanComparison)
            call fcheckControl(EnableOuterJoin)
            call fcheckControl(CheckRequiredFields)
            
            TabControl.setPage TabGeneratedValues
            call fcheckControl(RetrieveGeneratedValues)
            call fcheckControl(AutoIncrementStatement)
            call fcheckControl(QueryGeneratedValues)
            
        elseif sType = "MySQLODBC" OR sType = "MySQLJDBC" then

            'call fcheckControl(SQL92Check)
            'call fcheckControl(AppendAlias)
            call fcheckControl(Ignoreprivileges)
            'call fcheckControl(ParameterSubstiti)
            call fcheckControl(DisplayVersionColumns)
            call fcheckControl(BooleanComparison)
            'call fcheckControl(EnableOuterJoin)
            call fcheckControl(CheckRequiredFields)
        
            
        elseif sType = "dBase" OR sType = "Text" then
            call fcheckControl(SQL92Check)
            'call fcheckControl(BooleanComparison)
            'call fcheckControl(AppendAlias)
        
        elseif sType = "CALC" OR sType = "Mozilla" OR sType = "Evolution" OR sType = "LDAP" then            
           ' call fcheckControl(BooleanComparison)
           ' call fcheckControl(AppendAlias)    
        
        elseif sType = "Adabas" then
                call fcheckControl(AppendAlias)        
                call fcheckControl(BooleanComparison)        
                call fcheckControl(SQL92Check)
                call fcheckControl(DisplayVersionColumns)
                call fcheckControl(CheckRequiredFields)
        else
            qaerrorlog "unknown database type"
        end if
    
    Kontext "Advanced"
        Advanced.Cancel
        
end function
'-------------------------------------------------------------------------
function fcheckControl(oControl as object)
        
        if oControl.exists() then
            printlog "|The control " + oControl.Name + " exists."
        else        
            warnlog "|The control " + oControl.Name + " doesn't exists."
        end if

end function
'-------------------------------------------------------------------------
function fCheckConnectionTabPage(sType as string)
    
    printlog "+check the connection page for the datasource type " + sType    

    Kontext "TabConnection"        
    
    call DialogTest(TabConnection)  
        
    if sType = "MySQLODBC" then        
        call fcheckControl(MySQLODBCDatasource)
        call fcheckControl(SelectPath)
        call fcheckControl(Username)        
        call fcheckControl(PasswordRequired)
        call fcheckControl(TestConnection)
    
    elseif sType = "MySQLJDBC" then        
        call fcheckControl(MySQLJDBCDatasource)
        call fcheckControl(Username)        
        call fcheckControl(PasswordRequired)
        call fcheckControl(TestConnection)
    
    elseif sType = "Adabas" then        
        call fcheckControl(AdabasDatasource)
        call fcheckControl(SelectPath)
        call fcheckControl(Username)        
        call fcheckControl(PasswordRequired)
        call fcheckControl(TestConnection)
    
    elseif sType = "OracleJDBC" then        
        call fcheckControl(OracleJDBCDatasource)        
        call fcheckControl(Username)        
        call fcheckControl(PasswordRequired)
        call fcheckControl(TestConnection)
        
    elseif sType = "JDBC" then        
        call fcheckControl(ConnectUrl)
        call fcheckControl(Username)        
        call fcheckControl(PasswordRequired)
        call fcheckControl(TestConnection)
        call fcheckControl(TestDriver)
        call fcheckControl(JDBCDriverClass)  
    
    elseif sType = "ODBC" then        
        call fcheckControl(OdbcDatasourceName)
        call fcheckControl(Username)        
        call fcheckControl(PasswordRequired)        
        call fcheckControl(TestConnection)
    
    elseif sType = "dBase" then        
        call fcheckControl(dBasePath)
        call fcheckControl(SelectPath)
        call fcheckControl(TestConnection)
    
    elseif sType = "CALC" then        
        call fcheckControl(SpreadsheetPath)
        call fcheckControl(SelectPath)
        call fcheckControl(TestConnection)
    
    elseif sType = "Text" then        
        call fcheckControl(TextPath)
        call fcheckControl(SelectPath)
        call fcheckControl(TestConnection)    
    
    elseif sType = "LDAP" then      
        call fcheckControl(HostName)
        call fcheckControl(TestConnection)        
        
    elseif sType = "Mozilla" OR sType = "Evolution" then
        call fcheckControl(TestConnection)    
    end if

end function
'-------------------------------------------------------------------------
function fCheckTabPage(oType as object)
        
    printlog "+check the " + oType.Name + " page."    
    Kontext oType.Name
    
    call DialogTest(oType)
    
    if oType.Name = "TabMySQLJDBC" OR oType.Name = "TabOracleJDBC" then
        call fcheckControl(CharSet)      
        call fcheckControl(HostName)
        call fcheckControl(PortNumber)
        call fcheckControl(DriverClass)
        call fcheckControl(TestClass)
        
    elseif oType.Name = "TabAdabas" then        
        call fcheckControl(CharSet)    
        call fcheckControl(HostName)        
        call fcheckControl(CacheSize)
        call fcheckControl(DataIncrement)
        call fcheckControl(CotrolUser)
        call fcheckControl(ControlPassword)
        call fcheckControl(ShutdownService)     
        call fcheckControl(Extended)        
    
    elseif oType.Name = "TabODBC" then   
        call fcheckControl(UseCatalogs) 
        call fcheckControl(Options)
        call fcheckControl(CharSet)
    
    elseif oType.Name = "TabdBase" then
        call fcheckControl(CharSet)
        call fcheckControl(DisplayInactiveRecords)  
        call fcheckControl(Indexes)
        
    elseif ( oType.Name = "TabLDAP" ) then
        call fcheckControl(BaseDN)
        call fcheckControl(PortNumber)
        call fcheckControl(Records)
    
    elseif ( oType.Name = "TabTextBase" ) then                
        call fcheckControl(TextContainsHeaders)
        call fcheckControl(FieldSeparator)
        call fcheckControl(TextSeparator)
        call fcheckControl(DecimalSeparator)
        call fcheckControl(ThousandsSeparator)
        call fcheckControl(PlainTextFiles)
        call fcheckControl(CVSFiles)
        call fcheckControl(Custom)
        call fcheckControl(CustomTxt)        
        call fcheckControl(CharSet)
    end if
    
end function
