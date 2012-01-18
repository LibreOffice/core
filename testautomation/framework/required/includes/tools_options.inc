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
'*  short description : Verify that the applications appear in tools/options
'*
'\******************************************************************************

testcase tUpdtOptionItems

    printlog( "Verify that all applications are listed in tools/options" )
        
    const DOCTYPES = 7
    dim iDocType as integer
    dim cDocType as string
    dim iAppItems as integer
    dim iNodes as integer
    
    dim iAppCount( DOCTYPES ) as integer
        iAppCount( 1 ) = 8 ' writer
        iAppCount( 2 ) = 7 ' calc
        iAppCount( 3 ) = 7 ' impress
        iAppCount( 4 ) = 7 ' draw
        iAppCount( 5 ) = 7 ' math
        iAppCount( 6 ) = 8 ' masterdoc
        iAppCount( 7 ) = 8 ' html
        
    for iDocType = 1 to DOCTYPES
    
        printlog( "" )
        printlog( hNumericDocType( iDocType ) )
        hCreateDocument()
        
        ToolsOptions
        
        kontext "ExtrasOptionenDlg"
        if ( ExtrasOptionenDlg.exists( 2 ) ) then

            iAppItems = Optionsliste.getItemCount()
            Optionsliste.typeKeys( "<HOME>" )
            for iNodes = 1 to iAppItems
                Optionsliste.typeKeys( "-" )
                Optionsliste.typeKeys( "<DOWN>" )
            next iNodes

            iAppItems = Optionsliste.getItemCount()
            if ( iAppItems <> iAppCount( iDocType ) ) then
                warnlog( "#i68068# The number of top nodes in Tools/Options is incorrect." )
                printlog( "Found...: " & iAppItems )
                printlog( "Expected: " & iAppCount( iDocType ) )
            else
                printlog( "The number of top nodes is correct." )
            endif

            hCloseDialog( ExtrasOptionenDlg, "Cancel" )
        else
            warnlog( "Dialog <Tools/Options> did not open " )
        endif

        hDestroyDocument()
        
    next iDocType

endcase

