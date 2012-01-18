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
'* short description : check the internal file dialog ( extended tests )
'*
'\******************************************************************************

testcase tCJKFolders()

    
    if ( not hTestLocale() ) then
        warnlog( "Test requires UTF-8 locale" )
        goto endsub
    endif
    
    
    
    dim cStrangeFolder as string
    dim cStrangeFile as string
    dim iCounter as integer ' iterator
    dim iRandom  as long    ' random number.(needs long for cjk chars)
    dim brc as boolean
    
    printlog( CHR$(13) )
    printlog( "Check if CJK-files/folders are handled ok" )
    printlog( CHR$(13) )
    
    printlog( "Open a new document" )
    brc = hCreateDocument()
    
    printlog( CHR$(13) + "Names with CJK-chars" )
    printlog( "" )
    
    call randomize()
    
    for iCounter = 1 to 3
        
        iRandom = int( 19968 + ( 20911 * rnd ) )
        printlog( " * Using decimal char: " & iRandom )
        
        cStrangeFolder = hNameGen_append( iRandom )
        brc = hCreateDirctoryExpectSuccess( cStrangeFolder )
        
        cStrangeFile = hNameGen_append( iRandom )
        cStrangeFile = convertpath( cStrangeFolder & "\" & cStrangeFile )
        brc = hSaveLoadDelSuccess( cStrangeFile , TRUE )
        
        cStrangeFile = hNameGen_lead( iRandom , true )
        cStrangeFile = convertpath( cStrangeFolder & "\" & cStrangeFile )
        brc = hSaveLoadDelSuccess( cStrangeName , true )
        
        brc = hDeleteFileExpectSuccess( cStrangeFolder )
        
    next iCounter
    
    printlog( "Close the document" )
    brc = hDestroyDocument()
    
endcase

