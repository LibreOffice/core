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
'*  short description : test names of labels
'*
'\******************************************************************************

testcase tLabelTypes()

    printlog( "Verify presence of labels and brands" )

    ' If you want to use this test for other languages, remove the following
    ' if() block and create the reference files by running this test once.
    ' Follow the instructions emitted by hManagaComparisionList() from the log.
    
    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for non-en_US languages" )
        goto endsub
    endif

    const NO_DELTA = 0
    const MAX_LABEL_COUNT = 1600
    const RELATIVE_PATH = "framework\optional\input\extras_formats\"
    
    dim iBrandCount as integer
    dim iCurrentBrand as integer
    dim cBrandName as string
    dim iCurrentType as integer    
    dim cBrandType as string
    
    dim al_UI_Labels( MAX_LABEL_COUNT ) as string : al_UI_Labels( 0 ) = "0"
        
    dim irc as integer
    
    dim sFile as string    : sFile    = "Labels_" & gISOLang & ".txt"
    dim sFileIn as string  : sFileIn  = convertpath( gTesttoolPath & RELATIVE_PATH & sFile )
    dim sFileOut as string : sFileOut = hGetWorkPath() & sFile
       
    printlog( "Open labels dialog, retrieve all label names" )
    hCreateLabels()

    kontext
    Active.SetPage TabEtiketten
    
    kontext "TabEtiketten"

    iBrandCount = Marke.getItemCount()

    for iCurrentBrand = 1 to iBrandCount

        Marke.select( iCurrentBrand )
        cBrandName = Marke.getSelText()

        for iCurrentType = 1 to Typ.getItemCount()

            Typ.select( iCurrentType )
            cBrandType = cBrandName & ":" & Typ.getSelText()
            ListAppend( al_UI_Labels() , cBrandType )

        next iCurrentType

    next iCurrentBrand

    printlog( "Close labels dialog" )
    TabEtiketten.Cancel()

    printlog( "Compare to reference (or create new reference, if none found)" )
    irc = hManageComparisionList( sFileIn, sFileOut, al_UI_Labels() )
    hListResultEvaluation( irc , NO_DELTA )
    
endcase
