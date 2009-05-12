'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: extras_preview.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : Perform standard operations on all samples/templates etc.
'*
'\***********************************************************************

private const MAX_FILE_COUNT = 3000 ' the max number of templates or samples

' NOTE: Usually we have only about a 100 templates but in multi-language
'       installations there are more than 2100. You should not run these     
'       tests on multi language installations. Really.

testcase tShowAllObjectsAsPreview( cCategory as string )

    '///<h1>Show documents in the preview pane of the teplates-dialog</h1>

    dim iObjectFolder as Integer   ' the current folder in loops
    dim iObjectFolders as integer  ' the number of template/sample-folders
    
    dim iObject as Integer         ' the current template/sample within loops
    dim iObjectCount( 20 ) as integer  ' number of templates/samples in the folders
   
  
    printlog( "" )
   
   
    '///<ul>
    '///+<li>Open File->New->Templates and documents</li>
    FileNewFromTemplate
    if ( TemplateAndDocuments.exists( 1 ) ) then

   
        '///+<li>find the category we want to work with (TEMPLATES/SAMPLES...)</li>
        hSelectCategory( cCategory )
       
        '///+<li>Count the folders in the root of cCategory</li>
        iObjectFolders = FileList.getItemCount()
       
        '///+</li>Now go through every folder and count the number of objects</li>
        for iObject = 1 to iObjectFolders
       
            '//<li>grab a folder below cCategory and open it</li>
            hSelectFileFolder( iObject , true )
          
            '<li>get the number of items</li>
            iObjectCount( iObject ) = FileList.getItemCount()
          
            '<li>step up one level to the root of cCategory</li>
            UpOneLevel.click()
          
        next iObject
       
        '///+<li>Click the preview button</li>
        preview.click()

        printlog( "" )
        printlog( "Displaying documents in preview pane" )
        printlog( "" )

        '///+<li>Step through all folders and templates/samples</li>
        '///<ol>   
        for iObjectFolder = 1 to iObjectFolders
            
            kontext "TemplateAndDocuments"

            '///+<li>Select the category (samples, templates ...)</li>
            hSelectCategory( cCategory )

            '///+<li>Select the current folder</li>
            hSelectFileFolder( iObjectFolder , false )

            for iObject = 1 to iObjectCount( iObjectFolder ) 
       
                printlog( "Object-Folder: " & iObjectFolder & " Entry: " & iObject )

                '///+<li>Select the desired template</li>
                hSelectDocumentObject( iObject , 0 )
          
                '///+<li>Wait for the preview to be loaded and displayed</li>
                'Problem: Cannot verify the content of preview window.
                WaitSlot( 5000 )
          
                '///+<li>Next document</li>

            next iObject
          
            '///</ol>
          
       next iObjectFolder
       
       '///</ul>
       Kontext "TemplateAndDocuments"
       TemplateAndDocuments.cancel()
       
    else
        warnlog( "Failed to open Templates And Documents dialog" )
    endif
   
endcase   


