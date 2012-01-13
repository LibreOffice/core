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

*****

function hTestLibraryListBox( cNodeList() ) as boolean

    const CFN = "hTestLibraryListBox::"
    const EXACT_MATCH = 1

    dim iEntry as integer
    dim cEntry as string
    dim irc as integer

    hTestLibraryListBox() = true
    
    if ( bibliothek.getItemCount() <> IDOCS ) then
        warnlog( CFN & "Incorrect number of items in listbox" )
        printlog( CFN & "Expected: " & bibliothek.getItemCount() )
        printlog( CFN & "Found...: " & listcount( cNodeList() ) )
        hTestLibraryListBox() = false
    endif
        
    for iEntry = 3 to IDOCS
    
        Bibliothek.select( iEntry )
        cEntry = Bibliothek.getSelText()
        
        if ( hCompareSubStrings( cNodeList( iEntry ) , cEntry ) <> EXACT_MATCH ) then
            warnlog( CFN & "Comparision failed" )
            printlog( CFN & "Expected: " & cNodeList( iEntry ) & "<>" & cEntry )
            hTestLibraryListBox() = false
        else
            printlog( CFN & "Comparision succeeded:" & cEntry )
        endif
        
    next iEntry
    
end function
