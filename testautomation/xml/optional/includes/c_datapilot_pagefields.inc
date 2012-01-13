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

***********************
'*
' #1 subPageFields
' #1 tDatapilotPagefields
'*
'\*************************************************************************************************
sub subPageFields
  printLog Chr(13) + "---------  datapilot pagefields  ----------"

     printlog "Testing Page '- all -'"
     call tDatapilotPagefields("- all -")
     printlog "Testing Page 'Audio'"
     call tDatapilotPagefields("Audio")
     printlog "Testing Page 'DVD'"
     call tDatapilotPagefields("DVD")
     printlog "Testing Page 'Video'"
     call tDatapilotPagefields("Video")

end sub

'************************************************************
testcase tDatapilotPagefields ( sCurrentPage as STRING )
    dim bTableSelectedPageShouldExist as BOOLEAN
    dim iIndex as INTEGER
    dim sComfirmAttribute as STRING
    dim sWarnAttribute as STRING
    select case sCurrentPage
           case ("- all -") : bTableSelectedPageShouldExist = "false"
                              iIndex = 1
           case ("Audio")   : bTableSelectedPageShouldExist = "true"
                              iIndex = 2
                              sComfirmAttribute = "OK, value for attribute 'table:selected-page' is 'Audio'"
                              sWarnAttribute = "Unexpected value for attibute 'table:selected-page' -> Check this out!" 
           case ("DVD")     : bTableSelectedPageShouldExist = "true"
                              iIndex = 3
                              sComfirmAttribute = "OK, value for attribute 'table:selected-page' is 'DVD'"
                              sWarnAttribute = "Unexpected value for attibute 'table:selected-page' -> Check this out!" 
           case ("Video")   : bTableSelectedPageShouldExist = "true"
                              iIndex = 4
                              sComfirmAttribute = "OK, value for attribute 'table:selected-page' is 'Video'"
                              sWarnAttribute = "Unexpected value for attibute 'table:selected-page' -> Check this out!" 
    end select

dim sInputFile as STRING
    sInputFile = ( gTesttoolPath & ConvertPath( "xml\optional\input\calc\ooo10\pagefields_datapilot" & iIndex & ".sxc" ))
    
dim sOutputFile as STRING
    sOutputFile = ( gOfficePath & ConvertPath( "user\work\datapilotPagefields" ) & iIndex & ".ods" )
    
dim sUnpackedStorageDir as STRING
    sUnpackedStorageDir = ( gOfficePath & ConvertPath( "user\work\datapilotPagefields" ) & iIndex )
    if (dir(sUnpackedStorageDir) <> "") then
        rmDir (sUnpackedStorageDir)
        if (dir(sUnpackedStorageDir) <> "") then
            warnlog "Directory has not been deleted. Test ends here."
            goto endsub
        endif
    endif
    
dim sContentXML as STRING
    sContentXML = ( gOfficePath & ConvertPath( "user\work\datapilotPagefields" ) & iIndex & ConvertPath( "\content.xml" ))
    
dim iSaxSeekIndex as INTEGER
    iSaxSeekIndex = 1
    
dim bContinueSeek as BOOLEAN
    bContinueSeek = TRUE
    
dim iAttributeIndex as INTEGER
    
    printlog "Open test document"
    call hFileOpen ( sInputFile )       
    printlog "Save in current default application format (ODF)"
    if hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) = FALSE then
       warnlog "Saving " & sOutputFile & " failed! -> Exiting test!"
       call hCloseDocument
       goto endsub
    else
       printlog "CLose the document."
       call hCloseDocument              
       printlog "Unpack storage of test document."
       UnpackStorage( sOutputFile , sUnpackedStorageDir )
       printlog "Read the test documents DOM."       
       SAXReadFile( sContentXML )
       sleep(2)       
       printlog "Set the DOM pointer on element 'table:data-pilot-table'"
       printlog "|-/"
       SAXSeekElement( "/" )
       printlog "  |-office:document-content"
       SAXSeekElement( "office:document-content" )
       printlog "    |-office:body"
       SAXSeekElement( "office:body" )
       printlog "      |-office:spreadsheet"
       SAXSeekElement( "office:spreadsheet" )
       printlog "        |-table:data-pilot-tables"
       SAXSeekElement( "table:data-pilot-tables" )
       printlog "          |-table:data-pilot-table"
       SAXSeekElement( "table:data-pilot-table" )              
       printlog "Seek in (all) elements 'table:data-pilot-field'"
       while SAXHasElement( "table:data-pilot-field" , iSaxSeekIndex ) AND bContinueSeek
             printlog "            |-table:data-pilot-field"
             SAXSeekElement( "table:data-pilot-field" , iSaxSeekIndex )             
             printlog ".. for attribute 'table:selected-page'"
             for iAttributeIndex = 1 to SAXGetAttributeCount
                if SaxGetAttributeName ( iAttributeIndex ) = "table:selected-page" then                
                printlog "When found, check if it should exist (unexpected for page selection '- all -')"
                   bContinueSeek = FALSE
                   if bTableSelectedPageShouldExist then
                      printlog "Bingo, found attribute 'table:selected-page'"
                      'Check correct index of expected node
                      if iSaxSeekIndex <> 1 then
                           warnlog "Found 'table:selected-page' on an unexpected Node -> Check this out!"
                           qaErrorLog "TODO: Check if change of file format was announced!"
                           qaErrorLog "TODO: Modify test if change of file format was intended!"
                       endif
                       'Check correct attribute index
                       if iAttributeIndex <> 3 then
                           warnlog "Found 'table:selected-page' on unexpected attribute position -> Check this out!"
                           qaErrorLog "TODO: Check if change of file format was announced!"
                           qaErrorLog "TODO: Modify test if change of file format was intended!"
                       endif                       
                       printlog "Now check for expected value of 'table:selected-page'!"
                       if SAXGetAttributeValue( "table:selected-page" ) = sCurrentPage then
                           printlog sComfirmAttribute
                       else
                           warnlog sWarnAttribute
                       endif
                   else
                       warnlog "OOPS, attribute 'table:selected-page' shouldn't exist for '- all -' pages."
                   endif
                endif
             next iAttributeIndex
             iSaxSeekIndex = iSaxSeekIndex + 1
             SAXSeekElement( 0 )
       wend
    endif
    ' Throw warning if testcase didn't succeed at all!
    if bContinueSeek AND bTableSelectedPageShouldExist then
        warnlog "OOPS, attribute 'table:selected-page' wasn't found though it should exist -> Check this out!"
    endif
   'Release DOM
    SAXRelease
endcase

