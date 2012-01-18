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
'*  short description : Remane files dialog
'*
'\******************************************************************************

testcase tFileRename

    dim cPath as string
        cPath = convertpath( gOfficePath & "user/work/" )
        
    dim cFile as string
        cFile = "tFileRename.odt"
        
    dim iCurrentFile as integer
    dim cCurrentFile as string
    dim iFileCount as integer
    dim bFileFound as boolean : bFileFound = false

    ' Issue #i88446
    Printlog( "Rename file dialog" )
    
    gApplication = "WRITER"
    hNewDocument()
    DocumentWriter.typeKeys( "framework/optional/filedlg_dialogtest.bas::tFileRename" )
    
    hFileSaveAsKill( cPath & cFile )
    FileClose
    FileOpen
    kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then

        DateiName.setText( cPath )
        Oeffnen.click()
        iFileCount = DateiAuswahl.getItemCount()
        printlog( iFileCount & " files listed" )
        DateiAuswahl.typeKeys( "<HOME>" )

        for iCurrentFile = 1 to iFileCount

            wait( 100 )
            DateiAuswahl.typeKeys( "<SPACE>" )
            wait( 100 )
            
            cCurrentFile = DateiAuswahl.getSelText()
            printlog( " * " & cCurrentFile )
            
            if ( cCurrentFile = cFile ) then
                printlog( "File found" )
                DateiAuswahl.openContextMenu()
                hMenuSelectNr( 2 )
                DateiAuswahl.typeKeys( "<ESCAPE>" )
                kontext "OeffnenDlg"
                OeffnenDlg.cancel()
                exit for
            endif
            
            wait( 100 )
            DateiAuswahl.typeKeys( "<DOWN>" )
            wait( 100 )
            
        next iCurrentFile
    endif
    
    if ( FileExists( cPath & cFile ) ) then kill( cPath & cFile )
    

endcase

