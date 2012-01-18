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
'*  short description : Verify that all filters are listed in the filter listbox
'*
'\******************************************************************************

testcase tVerifyFilterNames

    printlog( "Verify filternames listed in the file open dialog" )
    
    if ( gIsoLang <> "en-US" ) then
        qaerrorlog( "No testing for non-US languages" )
        goto endsub
    endif
    
    const MAX_FILTERCOUNT = 300
    dim asFilterNames( MAX_FILTERCOUNT ) as string
    dim iFilterCount as integer
    dim iCurrentItem as integer
    
    dim cFile as string
    dim irc as integer


    ' For Windows and Solaris Sparc there exists additional commercial filters
    ' that are available for StarOffice only.
    cFile = gProductName & "_Filternames_"
    cFile = hStringReplaceChar( cFile, " ", "_" )

    if ( not gOOo ) then
        select case gtSysName
            case "Windows"       : cFile = cFile & "add_" & gIsoLang & ".txt"
            case "Solaris SPARC" : cFile = cFile & "add_" & gIsoLang & ".txt"
            case "WinXP"         : cFile = cFile & "add_" & gIsoLang & ".txt"
            case "Mac OS X"      : cFile = cFile & "add_" & gIsoLang & ".txt"
            case "Linux"         : cFile = cFile & gIsoLang & ".txt"
            case "Solaris x86"   : cFile = cFile & gIsoLang & ".txt"
        end select
    else
        cFile = cFile & gIsoLang & ".txt"
    endif
    
    dim cFileIn as string
        cFileIn  = gTesttoolPath & "framework\optional\input\filternames\" & cFile
        cFileIn = convertpath( cFileIn )
        
    dim cFileOut as string
        cFileOut = hGetWorkPath() & cFile

    printlog( "Create a new document" )
    hCreateDocument()
    
    printlog( "Open the File Open dialog" )
    FileOpen
    Kontext "OeffnenDlg"

    printlog( "Read the filterlist (en_US only), compare against a reference" )
    if ( OeffnenDlg.exists( 1 ) ) then    
    
        iFilterCount = DateiTyp.getItemCount()
        if ( iFilterCount > MAX_FILTERCOUNT ) then
            warnlog( "Too many filters in filterlist for this test to handle" )
            goto endsub
        endif
        
        printlog( "Reading in " & iFilterCount & " filternames" )

        asFilterNames( 0 ) = iFilterCount
        
        for iCurrentItem = 1 to iFilterCount         
        
            kontext "OeffnenDlg"
            try
                asFilterNames( iCurrentItem ) = DateiTyp.getItemText( iCurrentItem )
            catch
                printlog( "Problem encountered at pos: " & iCurrentItem )
            endcatch
            
        next iCurrentItem
        
        OeffnenDlg.cancel()
        
        irc = hManageComparisionList( cFileIn, cFileOut, asFilterNames() )
        hListResultEvaluation( irc , 0 )

    else
        warnlog( "File Open dialog not present." )
    endif

    printlog( "Close the document" )    
    hDestroyDocument()
         
endcase

