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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Helper functions to ease usage of templates
'*
'\******************************************************************************

function hFindTemplate( sTemplateName as string ) as integer

    '///<H3>Find a template by name in FileNewFromTemplate</H3>
    '///<i>Starting point: Templates and Documents dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li> Name of the template to search for (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li> Index of the Template in the containing folder (integer)</li>
    '///<ul>
    '///+<li>1 ... n : Index of the template (Position in folder)</li>
    '///+<li>0 : No template found by given name</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    const CFN = "hFindTemplate::"
    
    dim brc as boolean
        brc = false
    dim irc as integer
        irc = 0
        
    dim iObjectFolder as integer
    dim iObjectFolders as integer
    
    dim iItemCount as integer
    dim iCurrentItem as integer
    dim cCurrentItem as string
    
    '///+<li>select the templates from the category list</li>
    hSelectCategory( "TEMPLATES" )
    
    '///+<li>run through every item in the list to find the template.</li>
    ' NOTE: If the name of the template is not unique, the function will find
    '       the first occurrence
    ' NOTE: As we do not know the name of "My Templates" (it is localized) we 
    '       need to search all folders.. 
    iObjectFolders = FileList.getItemCount()

    '///<ul>
    for iObjectFolder = 1 to iObjectFolders
    
        '///+<li>Select the (next) folder</li>
        hSelectFileFolder( iObjectFolder , true )

        '///+<li>Retrieve the number of items within the folder</li>
        iItemCount = FileList.getItemCount()
        
        '///+<li>For each item in the folder do:</li>
        '///<ul>
        for iCurrentItem = 1 to iItemCount
        
            '///+<li>Select the (next) item</li>
            FileList.select( iCurrentItem )

            '///+<li>Get the name of the item</li>
            cCurrentItem = FileList.getSelText()
        
            '///+<li>If this is the item we are searching for, exit</li>
            if ( cCurrentItem = sTemplateName ) then
                irc = iCurrentItem : if ( irc = 0 ) then irc = 1 ' strange hack
                brc = true
                exit for
            endif
            
        next iCurrentItem
        '///</ul>
        
        '///+<li>Exit the outer loop</li>
        if ( brc ) then
            exit for
        endif
        
        '///+<li>Click &quot;Up one level&quot;</li>
        UpOneLevel.click()
        
    next iObjectFolder
    '///</ul>
    
    if ( brc ) then
        printlog( CFN & "Template found: " & cCurrentItem )
    else
        printlog( CFN & "Template could not be found." )
    endif
    
    '///+<li>Return the index of the requested template</li>
    hFindTemplate() = irc
    '///</ul>
    
end function

'*******************************************************************************

function hSelectCategory( cCategory as string ) as boolean

    '///<h3>Select a category from the left pane of the templates dialog</h3>
    '///<i>Requires: Templates and Documets dialog must be open</i><br>
    '///<u>Input</u>: Category (string):
    '///<ul>
    '///+<li>NEWDOCUMENTS to select New Documents</li>
    '///+<li>TEMPLATES to select Templates</li>
    '///+<li>MYDOCUMENTS to select My Documents</li>
    '///+<li>SAMPLES to select Samples</li>
    '///</ul>
    '///<u>Returns</u>: Alwas TRUE, no errorhandling

    Kontext "TemplateAndDocuments"
    if ( TemplateAndDocuments.exists( 2 ) ) then
        Wait( 500 )
        Category.typeKeys( "<HOME>" )
        wait( 500 )
        
        if ( UCASE( cCategory ) = "NEWDOCUMENTS" ) then
        ' do nothing, the selection should be on this item
            
        elseif ( UCASE( cCategory ) = "TEMPLATES" ) then
            Category.typeKeys( "<DOWN>" )
            
        elseif ( UCASE( cCategory ) = "MYDOCUMENTS" ) then
            Category.typeKeys( "<DOWN>" )
            Category.typeKeys( "<DOWN>" )
            
        elseif ( UCASE( cCategory ) = "SAMPLES" ) then
            Category.typeKeys( "<DOWN>" )
            Category.typeKeys( "<DOWN>" )
            Category.typeKeys( "<DOWN>" )
        endif

        hSelectCategory() = true
    else
        warnlog( "TemplateAndDocuments dialog did not open" )
    endif

end function

'***************************************************************************

function hSelectFileFolder( iFolder as integer , bVerbose as boolean  ) as integer

    '///<h3>Select a folder on the templates dialog's right pane by index</h3>
    '///<i>Requires: Templates and Documents dialog must be open</i><br>
    '///<u>Input</u>: 
    '///<ol>
    '///+<li>Index of the folder to be selected on the categories-pane (integer)</li>
    '///<ul>
    '///+<li>Must be > 0</li>
    '///</ul>
    '///+<li>Turn printlog on for debugging purpose (boolean)</li>
    '///<ul>
    '///+<li>TRUE : Be verbose</li>
    '///+<li>FALSE : Run silent</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>: 
    '///<ol>
    '///+<li>Number of items in the selected folder (integer)</li>
    '///<ul>
    '///+<li>Must be > 0</li>
    '///</ul>
    '///</ol>

    '///<u>Description</u>:
    '///<ul>
    dim iItems as integer
    dim cFolder as string

    Kontext "TemplateAndDocuments"
    
    '///+<li>Select the entry with the given index</li>
    FileList.select( iFolder )
    
    '///+<li>Retrieve the name of the selected object</li>
    cFolder = FileList.getText()
    
    '///+<li>Press return</li>
    FileList.typeKeys( "<return>" )
    wait( 500 )
    
    '///+<li>Get the number of items in the current folder</li>
    iItems = FileList.getItemCount()
    
    '///+<li>Print a comment to the log if specified</li>
    if ( bVerbose ) then
        printlog( " * " & cFolder & " contains " & iItems & " items." )
    endif

    '///+<li>Return the number of items</li>
    hSelectFileFolder() = iItems
    '///</ul>

end function

'*******************************************************************************

function hGetFileFolderName( iFolder as integer ) as string

    '///<h3>Get the name of the currently selected folder on templates dialog</h3>
    '///<i>Requires: Templates and Documents dialog must be open</i><br>
    '///<u>Input</u>: Index of the desired folder<br>
    '///<u>Returns</u>: Name of the selected folder

    Kontext "TemplateAndDocuments"
    FileList.select( iFolder )
    WaitSlot()
    hGetFileFolderName() = FileList.getText()
    WaitSlot()

end function

'*******************************************************************************

function hSelectDocumentObject( iTitle as integer , iMode as integer ) as string

    const CFN = "hSelectDocumentObject::"
    
    '///<h3>Open or edit sample/template from the templates dialog</h3>
    '///<i>Requires: Templates and Documents dialog must be open</i><br>
    '///<u>Input</u>: 
    '///<ol>
    '///+<li>Index of the folder to be selected on the categories-pane (integer)</li>
    '///<ul>
    '///+<li>Valid positive index</li>
    '///</ul>
    '///+<li>Mode in which to open the template (integer)</li>
    '///<ul>
    '///+<li>0 = Do not open the object, just return its name</li>
    '///+<li>1 = Open a new document based on the selected Template</li>
    '///+<li>2 = edit the template (unsupported)</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ul>
    '///+<li>The name of the selected item (string)</li>
    '///</ul>
    '///<u>Description</u>:
    '///<ul>

    dim cTitle as string
    dim brc as boolean
    dim iObjectCount as integer

    Kontext "TemplateAndDocuments"

    '///+<li>Get the title of the selected object</li>
    cTitle = hGetFileFolderName( iTitle )
    
    '///+<li>Count the number of objects in the list</li>
    iObjectCount = FileList.getItemCount()
    printlog( CFN & "Title: " & cTitle )

    select case iMode
    case 0 ' do not load the document, return the title and exit the function
        hSelectDocumentObject() = cTitle
        exit function
    case 1 ' open new document based on the template
        WaitSlot()
        kontext "TemplateAndDocuments"
        FileList.typeKeys( "<return>" )
        WaitSlot( 5000 )

        try
            kontext "TemplateAndDocuments"
            if ( TemplateAndDocuments.exists() ) then
                '///+<li>If yes: Try to determine if it is a new folder</li>
                if ( hIsObjectAFolder( iObjectCount ) ) then
                    hSelectDocumentObject() = "Folder"
                    exit function
                endif

            endif
        catch
        endcatch
        
    case 2 : warnlog( "Unsupported option: Edit template" )
        
    end select

    hFileWait()
    hHandleActivesOnLoad( 2, false )
    brc = hHandleInitialDialogs()

    '///+<li>If all initial dialogs were handled correctly, return the title</li>
    if ( brc ) then
        hSelectDocumentObject() = cTitle
    else
        hSelectDocumentObject() = ""
    endif
    '///</ul>

end function

'*******************************************************************************

function hIsTemplateDialogClosed() as boolean

    const CFN = "hIsTemplateDialogClosed::"
    '///<h3>Test whether the Templates and Documents dialog is closed after executing an object</h3>
    '///<i>Requires: Templates and Documents dialog must be open</i><br>
    '///<u>Returns</u>: 
    '///<ul>
    '///+<li>TRUE if the Templates and Documents dialog cannot be found</li>
    '///+<li>FALSE if the selected object was a foder (Templates and Documents still open</li>
    '///</ul>
    
    dim iTry as integer : iTry = 0

    
    if ( WaitSlot( 2000 ) = WSFinished ) then
        kontext "TemplateAndDocuments"
        try
            if ( TemplateAndDocuments.exists() ) then
                printlog( CFN & "Dialog still open. Maybe we opened a folder" )
                hIsTemplateDialogClosed() = false
                exit function
            else
                printlog( CFN & "Regular object. Dialog is closed" )
                hIsTemplateDialogClosed() = true
                exit function
            endif
        catch
            warnlog( "Failure to query Templates and Documents dialog" )
            hIsTemplateDialogClosed() = true
        endcatch
    else
        warnlog( "Slot not finished within 2000 msec" )
        hIsTemplateDialogClosed() = true
    endif

end function

'*******************************************************************************

function hIsObjectAFolder( iObjects as integer ) as boolean

    '///<h3>Test whether the &quot;Chapters&quot; folder has been selected</h3>
    '///<i>E.g. the Chapters-folder belongs to a master document and must be skipped.
    '///+ To didentify this folder, the number of items is checked (here: 4) which 
    '///+ should be unique (all other folders have more items)</i><br>
    '///<i>Requires: Templates and Documents dialog must be open</i><br>
    '///<u>Input</u>. Number of objects in the folder (integer)<br>
    '///<u>Returns</u>: TRUE if number of items matches iObjects (boolean)<br>
    '///<u>Description</u>:
    '///<ul>
    
    ' NOTE: This function should only handle one folder called "Chapters"
    '       below "Text Documents". We do not want to load the chapters
    '       separately, they are a part of a Master-Document and will be
    '       loaded at another time.
    '       To find out whether we are in a new folder or not, the number
    '       of objects in the parent folder is compared to the number in the
    '       current. This is a hack with a good probability to work.
    
    const CFN = "hIsObjectAFolder::"
    dim iCurrentObjects as integer

    if ( iObjects < 1 ) then
        warnlog( CFN & "Invalid Objectcount passed to function: " & iObjects )
        warnlog( CFN & "Defaulting to 1 -> outcome is undefined" )
        iObjects = 1
    endif

    kontext "TemplateAndDocuments"
    '///+<li>Compare the objectcount. If different, this is another folder</li>
    '///+<li>If the number is unchanged, we have an unknown error -> crash</li>
    iCurrentObjects = Filelist.getItemCount()

    if ( iCurrentObjects <> iObjects ) then
        printlog( CFN & "Object appears to be a folder with " & iCurrentObjects & " items" )
        hIsObjectAFolder() = true
    else
        warnlog( CFN & "Unknown error: Objectcount is unchanged" )
        hIsObjectAFolder() = false
    endif

    '///</ul>

end function

'*******************************************************************************

function hDeleteUserTemplates() as integer

    ' Recommendation: Use outside of testcase

    '///<h3>Delete all files located in the user templates directory</h3>
    '///<i>Uses hDeleteFile tzo remove all files below gOfficePath\user\template</i><br>
    '///<u>Input</u>: Nothing<br>
    '///<u>Returns</u>: Number of deleted objects<br>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hDeleteUserTemplates::"
    
    dim iFileCount as integer
    dim aFileList( 200 ) as string
    dim sPath as string
    dim iCounter as integer
    
    '///+<li>Set the path to the user-templates (default location)</li>
    sPath = convertpath( gOfficePath & "user\template" )
    
    '///+<li>Load the list of files into an array</li>
    iFileCount = GetFileList( sPath, "*.*", aFileList() )
    printlog( CFN & "Found " & iFileCount & " file(s)." )
    
    '///+<li>Delete each file and print the result to the log</li>
    for iCounter = 1 to iFileCount
    
        hDeleteFile( aFileList( iCounter ) )
        
    next iCounter
    
    '///+<li>Return the number of files that was found in the templates directory</li>
    hDeleteUserTemplates() = iFileCount
    
    '///</ul>
    
end function
