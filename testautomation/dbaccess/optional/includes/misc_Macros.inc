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
'* short description : macros in databases
'*
'\***********************************************************************

sub misc_Macros

    printlog "------------------- misc_Macros.inc ------------------------"
       
    call tMacros
    
end sub
'-------------------------------------------------------------------------
testcase tMacros
   
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )

    dim sFileName as String
    sFileName = ConvertPath(gOfficePath + "/user/work/hsql_macros_in_subdocument.odb")
    app.FileCopy ConvertPath(gTesttoolPath + "/dbaccess/optional/input/hsql_datasource/hsql_macros_in_subdocument.odb"), ConvertPath(gOfficePath + "/user/work/hsql_macros_in_subdocument.odb")

    ' delete the backup file
    if fileexists(sFileName + "backup.odb") then
        app.kill sFileName + "backup.odb"
    endif
    
    FileOpen
    sleep (1)
    Kontext "OeffnenDlg"
    '/// open the given file
    printlog "open the file: " + sFileName
    Dateiname.SetText sFileName
    sleep (3)
    Oeffnen.Click
    sleep (3)
    
    printlog "check if the subdocument contains macros dialog appear"
    Kontext "MessageBox"
    if (MessageBox.exists(1)) then
        MessageBox.OK
    else
        warnlog "the subdocument contains macros dialog does not appear"
    endif
    
    MigrateMacros
    
    WaitSlot(10)
    
    Kontext "MacroMigration"
    if not (MacroMigration.exists(1)) then        
        warnlog "MacroMigration wizard does not appear"
    endif

    NextButton.click

    BackupLocation.settext sFileName + "backup.odb"
    
    NextButton.click
    sleep(20)
    MacroMigration.OK
    
    call fCloseDatabase
    
    printlog "open the database again to see if the message box appear again"

    FileOpen
    sleep (1)
    Kontext "OeffnenDlg"
    '/// open the given file
    printlog "open the file: " + sFileName
    Dateiname.SetText sFileName
    sleep (3)
    Oeffnen.Click
    sleep (3)
    
    Kontext "MessageBox"
    if (MessageBox.exists(1)) then
        warnlog "MessageBox about macros appear. Should not after migration."
    else
        'nothing
    endif
    
    call fCloseDatabase
    
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )
    
endcase    
