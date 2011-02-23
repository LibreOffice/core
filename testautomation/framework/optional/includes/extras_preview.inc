'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : gregor.hartmann@oracle.com
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


