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
