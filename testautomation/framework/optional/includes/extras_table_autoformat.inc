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
'*  short description : Test the autoformat-feature for tables
'*
'\******************************************************************************

testcase tAutoformatTable( iApp as integer )

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for non-en_US languages" )
        goto endsub
    endif

    
    dim al_UI_formats( 1000 ) as string : al_UI_formats( 0 ) = "0"
    
    dim iFormatCount as integer
    dim iCurrentFormat as integer
    dim cCurrentFormat as string
    
    dim irc as integer
    
    dim sFileOut as string
    dim sFile as string
    dim sFileIn as string
        sFileIn = gTesttoolPath & "framework\optional\input\extras_formats\"
    
    dim oControl as object
            
    ' Build the filenames         
    select case iApp
    case 1 : sFile = "Tables_writer_" & gISOLang & ".txt"
    case 2 : sFile = "Tables_calc_" & gISOLang & ".txt"
    end select
    
    sFileIn = convertpath( sFileIn & sFile )
    sFileOut = hGetWorkPath() & sFile
    
    hCreateDocument()
    
    select case iApp
    case 1 : kontext "DocumentWriter"
             hTabelleEinfuegen()
    case 2 : kontext "DocumentCalc"
             DocumentCalc.TypeKeys( "<Shift Right>", 5 )
             DocumentCalc.TypeKeys( "<Shift Down>", 3 )
    end select
    
    FormatAutoformat
    
    select case iApp
    case 1 :    kontext "AutoformatTabelle"
                if ( AutoformatTabelle.exists( 2 ) ) then
                    oControl = FormatBox
                else
                    warnlog( "Unable to access Table Autoformat dialog (Writer)" )
                    goto endsub
                endif
                
    case 2 :    kontext "AutoformatCalc"
                if ( AutoformatCalc.exists( 2 ) ) then
                    oControl = FormatListe
                else
                    warnlog( "Unable to access Table Autoformat dialog (Calc)" )
                    goto endsub
                endif
    end select
    
    iFormatCount = oControl.getItemCount()
    
    for iCurrentFormat = 1 to iFormatCount
    
        oControl.select( iCurrentFormat )
        
        cCurrentFormat = oControl.getSelText()
        ListAppend( al_UI_formats() , cCurrentFormat )
        
    next iCurrentFormat
        
    select case iApp
    case 1 : AutoformatTabelle.ok()
    case 2 : AutoformatCalc.ok()
    end select
    
    hDestroyDocument()
    
    irc = hManageComparisionList( sFileIn, sFileOut, al_UI_formats() )
    hListResultEvaluation( irc , 0 )
    
endcase
        
    


