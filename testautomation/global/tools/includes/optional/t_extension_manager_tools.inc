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
'*  short description : Tools to ease working with the extension manager
'*
'\******************************************************************************

function hExtensionAddGUI( _path as string, _flags as string ) as integer

    '///<h3>Install an extension using the OpenOffice.org Extension Manager UI</h3>
    '///<p>This function is intended for use with the new Extension Manager UI
    '///+ and replaces an older function with the same name. Please note that 
    '///+ the interface has changed significantly. <br>This has become necessary
    '///+ because the Extension Manager can turn up with a really huge number
    '///+ of different dialogs, warnings, errormessages etc. <br>
    '///+ As this function is designed to handle the most common installation 
    '///+ scenarios it needs quite a number of differnt options.<br>
    '///+ Please have a look at the usage sample:<br><br></p>
    '///+ <p align="center"><i>hExtensionAddGUI( sMyExtension, 
    '///+ &quot;InstallForAll,BrokenDeps,DenyUpdate&quot; )</i<p><br>

    
    '///<u>Input:</u><br>
    '///<ol>
    '///+<li>Path to extension (String)</li>
    '///<ul>
    '///+<li>The path has to be fully qualified</li>
    '///+<li>The path may be platform specific</li>
    '///</ul>
    
    '///+<li>Flags (String), defaults underlined</li>
    '///<ul>
    '///+<li>The string is non optional but may be empty. Allowed flags are:</li>
    '///<ul>
    '///+<li>InstallForAll | <u>InstallForUser</u><br>Used when running office with administrator rights</li>
    '///+<li>AllowUpdate | DenyUpdate | <u>NoUpdate</u><br>Reinstall already installed extension/update</li>
    '///+<li>AcceptLicense | DenyLicense | <u>NoLicense</u><br>How to handle possible license dialog</li>
    '///+<li>BrokenDeps<br>Close exactly one broken dependencies warning</li> 
    '///+<li>UseSlot<br>Use the File Open slot to load the extension (faster)</li>   
    '///</ul>
    '///+<li>It is recommended to use the comma as delimiter between flags</li>
    '///</ul>
    '///</ol>

    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Installation status (Integer)</li>
    '///<ul>
    '///+<li>&gt; 0 = Installation completed with no errors, number of installed extensions</li>
    '///+<li>-1 = The requested extension does not exist</li>
    '///+<li>-2 = The Add-button could not be accessed</li>
    '///+<li>-3 = The Extension Manager did not open</li>
    '///+<li>-4 = Unknown messagebox before the file Open dialog exists</li>
    '///+<li>-5 = Broken dependency warning displayed</li>
    '///+<li>-6 = The File Open dialog did not pop up</li>
    '///+<li>-7 = Unknown and unhandled messagebox. function exit</li>
    '///</ul>
    '///</ol>

    dim flags as string   : flags  = lcase( _flags )
    dim path as string    : path   = convertpath( _path )
    dim bLogs as boolean  : bLogs  = FALSE

    const CFN = "hExtensionAddGUI()::"

    '///<u>Description</u>
    '///<ul>

    ' set defaults if string is empty    
    if ( flags = "" ) then flags = "installforuser,noupdate,nolicense"
    if ( instr( flags , "verbose" ) <> 0 ) then bLogs = TRUE 
    
    if ( bLogs ) then printlog( CFN & "Flags: " & flags )
    
    '///+<li>Verify that the requested extension exists (filesystem level)</li>
    if ( not FileExists( path ) ) then
        printlog( CFN & "Requested extension does not exist" )
        printlog( CFN & path )
        hExtensionAddGUI() = -1
        exit function
    endif
    
    
    if ( bLogs ) then 
        printlog( "" )
        printlog( "********** Installing extension begin **********" )
    endif
    
    '///+<li>Open the Extension Manager - optionally using the slot (CWS oxtsysint01)</li>
    if ( instr( flags , "useslot" ) <> 0 ) then
        hFileOpen( path )
    else
        ToolsPackageManager
        kontext "PackageManager"
        
        if ( PackageManager.exists( 2 ) ) then
    
            '///+<li>Verify that the &quot;Add..&quot; button is available</li>
            if ( add.exists() and add.isEnabled() ) then
            
                '///+<li>Click the &quot;Add...&quot; button</li>
                add.click()
            else
                printlog( CFN & "Add button is missing or disabled" )
                hExtensionAddGUI() = -2
                exit function
            endif
            
            '///+<li>Test for the dreaded &quot;The office workdirectory does not exist&quot; warning, close it</li>
            kontext "Active"
            if ( Active.exists( 1 ) ) then 
                if ( Active.getButtonCount() = 1 ) then
                    if ( bLogs ) then printlog( Active.getText() )
                    active.ok()
                else
                    printlog( CFN & "Unexpected/unknown messagebox" )
                    printlog( Active.getText() ) 
                    hExtensionAddGUI() = -4
                    exit function
                endif
            endif
            
            '///+<li>Enter the extension name into the file picker, open the file</lI>
            kontext "OeffnenDlg"
            if ( OeffnenDlg.exists( 2 ) ) then
                DateiName.setText( path )
                Oeffnen.click()
            else
                printlog( CFN & "The File Open dialog did not open" )
                hExtensionAddGUI() = -6
                exit function
            endif
            
        else
            printlog( CFN & "Extension Manager is not open" )
            hExtensionAddGUI() = -3
            exit function
        endif
    endif
    
    '///+<li>Test for the installation target dialog that comes up as soon as the
    '///+ user has administrator rights or works on a userspace installation. 
    '///+ Handle the dialog as specified by the function flags</li>
    if ( instr( flags, "installfor" ) <> 0 ) then
        kontext "Active"
        if ( Active.exists( 1 ) ) then 
            if ( Active.getButtonCount() = 3 ) then
            
                if ( bLogs ) then 
                    printlog( CFN & "Installation target dialog found" )
                    printlog( Active.getText() )
                endif
                
                if ( instr( flags , "installforall" ) <> 0 ) then
                    printlog( CFN & "Installing for all users" )
                    Active.no()
                else
                    printlog( CFN & "Installing for user only" )
                    Active.yes()
                endif
            else
                if ( bLogs ) then 
                    printlog( CFN & "Unexpected/unknown dialog" )
                    printlog( Active.getText() )
                endif
            endif
        else
            if ( bLogs ) then printlog( CFN & "Skipping handling of installation target" )
        endif
    else
        if ( bLogs ) then printlog( CFN & "Not handling userspace installations" )
    endif

    '///+<li>Test for the broken dependencies exception, close it with ok.</li>
    if ( instr( flags , "brokendeps" ) <> 0 ) then
        if ( bLogs ) then printlog( CFN & "Testing for dependencies messagebox" )
        kontext "UnsatisfiedDependencies"
        if ( UnsatisfiedDependencies.exists( 1 ) ) then
            printlog( CFN & "Closing Unsatisfied Dependencies dialog." )
            UnsatisfiedDependencies.ok()
            hExtensionAddGUI() = -5
        else
            printlog( CFN & "No unsatisfied dependencies dialog" )
        endif
    else
        if ( bLogs ) then printlog( CFN & "Skipping handling of broken dependencies dialog" )
    endif
    
    '///+<li>Test for the extension update dialog which pops up if an extension
    '///+ is already installed. Handle as specified by flags</li>
    if ( instr( flags, "update" ) ) then
        if ( bLogs ) then printlog( CFN & "Testing for version message/update" )
        kontext "Active"
        if ( Active.exists( 1 ) ) then
        
            if ( bLogs ) then 
                printlog( CFN & "Found update dialog" )
                printlog( Active.getText() )
            endif
            
            if ( Active.getButtonCount() = 2 ) then
                if ( instr( flags, "denyupdate" ) <> 0 ) then
                    printlog( CFN & "Denying update" )
                    Active.cancel()                    
                else
                    printlog( CFN & "Allowing update" )
                    Active.ok()
                endif
            else
                printlog( CFN & "Unexpected/unknown dialog displayed" )
                printlog( Active.getText() )
                hExtensionAddGUI() = -7
                exit function
            endif
        else
            if ( instr( flags , "noupdate" ) <> 0 ) then
                printlog( CFN & "No update dialog present. Good" )
            else
                printlog( CFN & "Update messagebox is missing" )
            endif
        endif
    else
        if ( bLogs ) then printlog( CFN & "Skipping handling of update dialog" )
    endif
    
    '///+<li>Test for the Software License Agreement dialog. Handle as specified by
    '///+ flags</li>
    if ( instr( flags, "license" ) <> 0 ) then
        if ( bLogs ) then printlog( CFN & "Testing software license dialog" )
        kontext "ExtensionSoftwareLicenseAgreement"
        if ( ExtensionSoftwareLicenseAgreement.exists( 5 ) ) then
            if ( bLogs ) then printlog( CFN & "Software license dialog found" )
            if ( instr( flags , "denylicense" ) <> 0 ) then
                printlog( CFN & "Cancelling software license dialog" )
                ExtensionSoftwareLicenseAgreement.cancel()
            else
                printlog( CFN & "Accepting software license" )
                do while ( not accept.isEnabled() ) 
                    ScrollDown.click()
                    WaitSlot()
                loop
                accept.click()
            endif
        else
            if ( instr( flags , "nolicense" ) <> 0 ) then
                printlog( CFN & "No license dialog displayed. Good." )
            else
                warnlog( CFN & "Expected license dialog is missing" )
            endif
        endif
    else
        if ( bLogs ) then printlog( CFN & "Skipping handling of license dialog" )
    endif
    
    '///+<li>Retrieve the number of installed extensions</li>
    kontext "PackageManager"
    wait( 500 )
    hExtensionAddGUI() = BrowsePackages.getItemCount()
    
    '///+<li>Close the Extension Manager</li>
    hCloseDialog( PackageManager , "close" )
    
    if ( bLogs ) then 
        printlog( "**********  Installing extension end  **********" )
        printlog( "" )
    endif
    '///</ul>    
    
end function
    

'*******************************************************************************

function hExtensionRemoveGUI( cExtensionName as string ) as integer


    '///<h3>Remove an extension via Extension Manager </h3>

    '///<u>Input value(s):</u><br>
    '///<ol>
    '///+<li>UI Name of the extension (string)</li>
    '///</ol>


    '///<u>Return Value:</u><br>

    '///<ol>
    '///+<li>Errorcode (integer)</li>
    '///<ul>
    '///+<li>0 = No errors, extension was removed</li>
    '///+<li>1 = Failure to open Extension Manager (fatal)</li>
    '///+<li>2 = Cannot delete found extension, remove-button is disabled (fatal)</li>
    '///+<li>3 = The extension was not found (non-fatal)</li>
    '///</ul>
    '///</ol>
    
    const CFN = "hExtensionRemoveGUI(): "
    
    printlog( "Removing extension by name: " & cExtensionName )

    ToolsPackageManager
    kontext "PackageManager"
    if ( PackageManager.exists( 2 ) ) then
        try
            BrowsePackages.select( cExtensionName )
            if ( Remove.exists() ) then
                if ( Remove.isEnabled() ) then
                    Remove.click()
                    
                    kontext "Active"
                    if ( Active.exists( 3 ) ) then
                        printlog( Active.getText() )
                        Active.OK()
                        WaitSlot()
                    else
                        warnlog( CFN & "Expected confirmation dialog is missing" )
                    endif
                    hExtensionRemoveGUI() = 0
                else
                    hExtensionRemoveGUI() = 2
                endif
            else
                hExtensionRemoveGUI() = 4
            endif
        catch
            hExtensionRemoveGUI() = 3
        endcatch
        
        kontext "PackageManager"
        hCloseDialog( PackageManager , "close" )
    else
        hExtensionRemoveGUI() = 1
    endif

end function

'*******************************************************************************

function sExtensionCLI(sCommand as string, sExtensionName as string, optional sExtensionPath as string) as string
    '/// Add/remove an extension with the command line tool 'unopkg'///'
    '/// INPUT: sCommand: string of command from "add remove list reinstall" ///'
   '/// INPUT: sExtensionName: name of the extension ///'
   '/// INPUT: optional sExtensionPath: path to the extension ///'
   '/// RETURN: currently nothing ///'
    dim sLokalExtensionPath as string
    dim sCommands as string
    dim sUnoPkg as string
    dim i, a, b as integer
    dim args as string
    Dim sFile as string
    Dim sEnv as string
    Dim sContent(5) as string
    Dim sPlatformProgramPath as string
    
    if isMissing(sExtensionPath) then
        sLokalExtensionPath = ""
    else
        sLokalExtensionPath = sExtensionPath
    endif
    
    sCommands = "add remove list reinstall"
    
    a = len(sAppExe)
    if a > 12 then
        b = inStr(a-12, sAppExe, "soffice")
        sUnoPkg = left(sAppExe, b-1) + "unopkg" + mid(sAppExe, b+len("soffice"))
        'printlog sUnoPkg
        'sUnoPkg = convertToUrl(sUnoPkg)
    else
        qaErrorLog ("Need to think about another solution..." + sAppExe)
    endif
    args = sCommand+" "+sLokalExtensionPath + sExtensionName
    printlog "Executing: "+sUnopkg+" "+args
    if gPlatGroup <> "unx" then
        shell(sUnoPkg,2,args)
    else
        sFile = ConvertPath (gOfficePath + "user/work/uno.sh")
        if gPlatform = lcase("osx") then
            sPlatformProgramPath = "MacOS"
        else
            sPlatformProgramPath = "program"
        end if
        sEnv = convertToURL(convertPath(gNetzOfficePath + sPlatformProgramPath + "/fundamentalrc")
        listAppend(sContent(), "export URE_BOOTSTRAP=" + sEnv) 
        listAppend(sContent(), sUnoPkg + " " + args)
        listWrite(sContent(), sFile)
        shell("bash",1,sFile)
    endif
end function

'*******************************************************************************

function hExtensionGetItemList( cItemList() as string ) as integer

    '///<h3>Get the list of all items in the extensions list</h3>
    '///<i>The array contains the list of all items in the extension manager GUI,
    '///+ including all components of the extensions. Consider this when defining
    '///+ the size of the array to be passed to this function as problems here are
    '///+ hard to debug.<br>
    '///+ Starting point is any document, the function will return to the 
    '///+ calling document on completion</i><br><br>
    
    '///<u>Input:</u>
    '///<ol>
    '///+<li>Array for the list items (string)</li>
    '///</ol>
    
    
    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Number of items (integer)</li>
    '///<ul>
    '///+<li>0 on any error</li>
    '///+<li>2 if no extensions exist (My Macros/OpenOffice.org macros nodes present</li>
    '///+<li>&gt; 2 if any changes to the default exist</li>
    '///</ul>
    '///</ol>
    
    dim iItemCount as integer
    dim iCurrentExtension as integer
    
    printlog( "Retrieving extension list" )

    ToolsPackageManager
    kontext "PackageManager"
    if ( PackageManager.exists( 2 ) ) then

        iItemCount = BrowsePackages.getItemCount()

        for iCurrentExtension = 1 to iItemCount
            cItemList( iCurrentExtension ) = BrowsePackages.getItemText( iCurrentExtension , 1 )
            printlog( " * " & cItemList( iCurrentExtension )
        next iCurrentExtension

        hExtensionGetItemList() = iItemCount
        cItemList( 0 ) = iItemCount
        hCloseDialog( PackageManager , "close" )

    else
    
        hExtensionGetItemList() = 0
        
    endif    

end function

'*******************************************************************************

function hSelectExtensionID( iPos as integer ) as string

    '///<h3>Select an item by index in the list of available extensions</h3>
    '///<i>This function needs the Extension Manager to be open. It will only
    '///+ select extensions but not their components.</i><br><br>
        
    '///<u>Input:</u>
    '///<ol>
    '///+<li>Absolute position of the extension to be selected (Integer)</li>
    '///</ol>
    
    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Name of the selected extension (String)</li>
    '///+<li>Empty string on index out of range or Extension Manager not open</li>
    '///</ol>
    
    printlog( "Selecting extension at pos. " & iPos )

    ToolsPackageManager
    kontext "PackageManager"
    if ( PackageManager.exists( 2 ) ) then
        try
            BrowsePackages.select( iPos )
            hSelectExtensionID() = BrowsePackages.getItemText( iPos , 1 )
        catch
            hSelectExtensionID() = ""
        endcatch
        hCloseDialog( PackageManager , "close" )
    else
        hSelectExtensionID() = ""
    endif
    

end function

'*******************************************************************************

function hSelectExtensionName( cName as string ) as integer

    '///<h3>Select an item by name in the list of available extensions</h3>
    '///<i>This function needs the Extension Manager to be open. It will only
    '///+ select extensions but not their components.</i><br><br>
    
    '///<u>Input:</u>
    '///<ol>
    '///+<li>Name of the extension (String)</li>
    '///<ul>
    '///+<li>Name of any item in the treelist, even those of top nodes</li>
    '///</ul>
    '///</ol>
    
    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Absolute position of the selected extension (Integer)</li>
    '///<ul>
    '///+<li>0 = Extension was not found</li>
    '///+<li>&gt; 0 = Absolute position of the extension/node</li>
    '///+<li>-1 = Extension Manager did not open</li>
    '///</ul>
    '///</ol>

    printlog( "Selecting extension by display name: " & cName )
    
    ToolsPackageManager
    kontext "PackageManager"
    if ( PackageManager.exists( 2 ) ) then
        try
            BrowsePackages.select( cName )
            hSelectExtensionName() = BrowsePackages.getSelIndex()
        catch
            hSelectExtensionName() = 0
        endcatch
        hCloseDialog( PackageManager , "close" )
    else
        hSelectExtensionName() = -1
    endif


end function


'*******************************************************************************

function hSelectOptionsItem( cName as string, iIndex as integer ) as integer

    '///<h3>Select an item in Tools/Options and verify</h3>
     '///<i>You need to open the Tools/Options dialog before using this function,
     '///+ it will not close the dialog either. No warnlogs are printed so evaluation
     '///+ of the return value is mandatory</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>Name of the extension node (string)</li>
    '///<ul>
    '///+<li>Name of the module</li>
    '///+<li>Name of the leaf</li>
    '///+<li>Node must be valid</li>
    '///</ul>

    '///+<li>Position of the node (absolute) (integer)</li>
    '///<ul>
    '///+<li>All nodes are expanded</li>
    '///+<li>Position must be valid (&gt; 0 and &le; number of nodes in list)</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (integer)</li>
    '///<ul>
    '///+<li>0 = Success</li>
    '///+<li>1 = Partial success - node is at wrong position</li>
    '///+<li>2 = Node does not exist</li>    
    '///+<li>3 = Dialog not open</li>
    '///+<li>4 = Index out of range (incorrect call to function)</li>
    '///</ul>
    '///</ol>

    const CFN = "hSelectOptionsItem::"
    printlog( CFN & "Enter with option (Name).: " & cName  )
    printlog( CFN & "Enter with option (Index): " & iIndex )
        
    dim brc as boolean ' a multi purpose boolean returnvalue
    dim irc as integer ' a multi purpose integer returnvalue
    dim crc as string  ' a multi purpose string  returnvalue

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Make sure we are on the Tools/Options dialog</li>
    kontext "OptionenDlg"
    if ( not OptionenDlg.exists( 2 ) ) then
        warnlog( CFN & "Tools/Options is not open, aborting" )
        hSelectOptionsItem() = 3
        exit function
    endif
    
    '///+<li>Expand all nodes on &quot;OptionsListe&quot;</li>
    irc = hExpandAllNodes( OptionsListe )
    if ( irc < iIndex ) then
        warnlog( CFN & "Index out of range, quitting" )
        hSelectOptionsItem() = 4
        exit function
    endif        
    
    '///+<li>Search for the node at the given index, handle errors, exit function</li>
    crc = hSelectNode( OptionsListe , iIndex )
    
    ' if the name of the node at given position is ok ...
    if ( crc = cName ) then
        printlog( CFN & "Exit: The node was found: " & crc )
        hSelectOptionsItem() = 0
        exit function 

    ' if the name is not ok, try to find the node by name
    else 
        printlog( CFN & "Node not found at expected position, retrying" )
        irc = hSelectNodeByName( OptionsListe , cName )

        ' if the node is found it is just at the wrong position - bad but not deadly
        if ( irc > 0 ) then
            qaerrorlog( CFN & "Exit: Node <" & cName & "> found at pos: " & irc )
            hSelectOptionsItem() = 1
            exit function

        ' if the node was not found at all this is really bad.
        else
            warnlog( CFN & "Exit: Node does not exist: " & cName )
            hSelectOptionsItem() = 3
            exit function            
        endif
    endif
    
    '///</ul>

end function


'*******************************************************************************

function hIsExtensionAlreadyInstalled( cName as string ) as boolean

    '///<h3>Check if an extension is already installed</h3>
    '///<i>The function opents the Extension Manager, finds (or not) the extension
    '///+ and returns an appropriate return value. The Extension Manager is
    '///+ closed again at the end.</i><br><br>
    
    '///<u>Input:</u>
    '///<ol>
    '///+<li>Name of the extension (String)</li>
    '///</ol>
    
    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>is the extension allredy installed (boolean)</li>
    '///<ul>
    '///+<li>FALSE = Extension is not installed</li>
    '///+<li>FALSE = Extension Manager did not open</li>
    '///+<li>TRUE = Extension is installed</li>
    '///</ul>
    '///</ol>
    
    printlog( "Checking if extension is already installed: " & cname )

    dim iCountExtensions as integer
    dim i as integer
    
    hIsExtensionAlreadyInstalled() = FALSE
    
    ToolsPackageManager
    kontext "PackageManager"
    
    iCountExtensions = BrowsePackages.getItemCount()
    
    for i = 1 to iCountExtensions
        if(Instr(BrowsePackages.getItemText(i,1),cname) <> 0) then
            hIsExtensionAlreadyInstalled() = TRUE    
        endif
    next
    
    hCloseDialog( PackageManager , "close" )
        
end function


'*******************************************************************************

function hDisableUserExtensionGUI( extension_name as string ) as integer

    '///<h3>Disable a userspace extension by name via GUI</h3>
    '///<i>This function tries to disable an Extension specified by name. There
    '///+ are several reasons why this might fail so it is required to evaluate the
    '///+ returnvalue as the function will not print any warnings. If the function 
    '///+ fails this is most likely caused by a) the extension already being
    '///+ disabled or b) the extension belonging to the shared layer.<br>
    '///+ The Extension Manager needs to be open when function is called.</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>Name of the extension to be disabled (string)</li>
    '///<ul>
    '///+<li>Only extensions from the user layer can be disabled</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (integer)</li>
    '///<ul>
    '///+<li>0 = Extension was found and disabled</li>
    '///+<li>1 = Extension does not exist</li>
    '///+<li>2 = Extension could not be disabled</li>
    '///+<li>3 = Extension Manager did not open</li>
    '///</ul>
    
    printlog( "Disabling user extension" )

    ToolsPackageManager
    kontext "PackageManager"
    if ( PackageManager.exists( 2 ) ) then
        try
            BrowsePackages.select( extension_name )
            if ( Disable.exists() and Disable.isEnabled() ) then
                hDisableUserExtensionGUI() = 0
            else
                hDisableUserExtensionGUI() = 2
            endif
            hCloseDialog( PackageManager , "close" )
        catch
            hDisableUserExtensionGUI() = 1
        endcatch
    else
        hDisableUserExtensionGUI() = 3
    endif

end function

'*******************************************************************************

function hGetExtensionCount() as integer

    printlog( "Getting number of installed extensions." )

    ToolsPackageManager
    kontext "PackageManager"
    if ( PackageManager.exists( 2 ) ) then
        try
            hGetExtensionCount() = BrowsePackages.getItemCount()
            hCloseDialog( PackageManager , "close" )  
        catch
            hGetExtensionCount() = -2
        endcatch
    else
        hGetExtensionCount() = -1
    endif


end function


