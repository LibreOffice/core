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
'* short description : Perform standard operations on all samples/templates etc.
'*
'\***********************************************************************

private const MAX_FILE_COUNT = 3000 ' the max number of templates or samples

' NOTE: Usually we have only about a 100 templates but in multi-language
'       installations there are more than 2100. You should not run these     
'       tests on multi language installations. Really.

testcase tShowAllObjectsAsPreview( cCategory as string )


    dim iObjectFolder as Integer   ' the current folder in loops
    dim iObjectFolders as integer  ' the number of template/sample-folders
    
    dim iObject as Integer         ' the current template/sample within loops
    dim iObjectCount( 20 ) as integer  ' number of templates/samples in the folders
   
  
    printlog( "" )
   
   
    FileNewFromTemplate
    if ( TemplateAndDocuments.exists( 1 ) ) then

   
        hSelectCategory( cCategory )
       
        iObjectFolders = FileList.getItemCount()
       
        for iObject = 1 to iObjectFolders
       
            '//<li>grab a folder below cCategory and open it</li>
            hSelectFileFolder( iObject , true )
          
            '<li>get the number of items</li>
            iObjectCount( iObject ) = FileList.getItemCount()
          
            '<li>step up one level to the root of cCategory</li>
            UpOneLevel.click()
          
        next iObject
       
        preview.click()

        printlog( "" )
        printlog( "Displaying documents in preview pane" )
        printlog( "" )

        for iObjectFolder = 1 to iObjectFolders
            
            kontext "TemplateAndDocuments"

            hSelectCategory( cCategory )

            hSelectFileFolder( iObjectFolder , false )

            for iObject = 1 to iObjectCount( iObjectFolder ) 
       
                printlog( "Object-Folder: " & iObjectFolder & " Entry: " & iObject )

                hSelectDocumentObject( iObject , 0 )
          
                'Problem: Cannot verify the content of preview window.
                WaitSlot( 5000 )
          

            next iObject
          
          
       next iObjectFolder
       
       Kontext "TemplateAndDocuments"
       TemplateAndDocuments.cancel()
       
    else
        warnlog( "Failed to open Templates And Documents dialog" )
    endif
   
endcase   


