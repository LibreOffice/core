'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_basic_ide_tools.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: obo $ $Date: 2008-07-25 07:44:37 $
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
' **
' ** owner : joerg.skottke@sun.com
' **
' ** short description : tools for the property-browser test
' **
'\******************************************************************************

function hShowMacroControls() as boolean

    '///<h3>Tear off the ToolsCollectionBar from the MacroBar</h3>
    '///<i>Starting point: Basic dialog editor is open and has the focus</i>
    '///<ul>
    const CFN = "hShowMacroControls::"   
    
    printlog( CFN & "Enter" )

    '///+<li>Test whether the ToolsCollectionBar is already open. If yes, exit directly</li>
    kontext "ToolsCollectionBar"
    if ( ToolsCollectionBar.exists( 3 ) ) then
        printlog( CFN & "Exit: ToolsCollectionBar is already open." )
        hShowMacroControls() = true
        exit function
    endif
    
    '///+<li>Verify that the Dialog Bar is available</li>
    Kontext "DialogBar"
    if ( DialogBar.exists( 5 ) ) then

        '///+<li>Verify that the controls-button is enabled</li>
        try        
            '///+<li>Click on the controls-button to open the ToolsCollectionBar</li>
            controls.click()
            
            '///+<li>Tear off the ToolsCollectionBar from the DialogBar</li>
            controls.tearOff()
            
            '///+<li>Verify that the ToolsCollectionBar is visible and can be accessed</li>
            kontext "ToolsCollectionBar"
            if ( ToolsCollectionBar.exists( 5 ) ) then
            
                '///+<li>Move it to the upper left corner at pos 50/50</li>
                ToolsCollectionBar.Move ( 50 , 50 )
                printlog( CFN & "Exit: ToolsCollectionBar is open and has been placed" )
                hShowMacroControls() = true
                exit function
            else
                printlog( CFN & "Exit: ToolsCollectionBar is not available. Aborting." )
                hShowMacroControls() = false
                exit function
            endif
        catch
            printlog( CFN & "Exit: The Controls-Button is not enabled. Aborting." )
            hShowMacroControls() = false
            exit function
        endcatch	
    else
        printlog( CFN & "Exit: The Dialog Bar is not available. Aborting." )
        hShowMacroControls() = false
        exit function
    endif
    '///</ul>    
        
end function

'*******************************************************************************

function hGetBasicWorkFile( cMajorID as string ) as string

    '///<h3>Retrieve a filename, depending on gApplication and Build-ID</h3>
    '///<i>Structure: work-directory/basic[.suffix]</i>
    '///<ul>
    
    const CFN = "hGetBasicWorkFile::"
    
    dim sFile as string
    
    '///+<li>Build a path and a filename (path from UNO, Suffix from global-module)</li>
    sFile = hGetWorkPath() & "basic" & hGetSuffix( cMajorId )
    sFile = ConvertPath( sFile )
    
    printlog( CFN & sFile & " for " & gApplication & " (" & cMajorID & ")" )
        
    '///+<li>Return the path as string</li>
    hGetBasicWorkFile() = convertpath( sFile )
    
    '///</ul>

end function

'*******************************************************************************

function hOpenBasicIDE() as boolean

    '///<h3>Open the Basic IDE for the last module listed in the Macro Organizer</h3>
    '///<i>Starting point: Basic Macro Organizer has focus</i>
    '///<ul>
    
    const CFN = "hOpenBasicIDE::"
    
    '///+<li>Select the last module in the treelist, verify that it can be edited</li>
    hSelectTheLastModule( true )
    
    '///+<li>Click the Edit-button</li>
    Bearbeiten.click()
    
    '///+<li>Maximize the Basic IDE (required to ensure proper operation of mousemovements)</li>
    sleep( 2 )
    kontext "BasicIde"
    BasicIde.maximize()
    
    '///+<li>Verify that the Basic IDE is open, return TRUE on success</li>
    if ( BasicIde.exists() ) then
       printlog( CFN & "Open Basic-IDE for existing module: success" )
       hOpenBasicIDE() = true
    else
       printlog( CFN & "Open Basic-IDE for existing module: failure" )
       hOpenBasicIDE() = false
    endif
    
    '///</ul>

end function

'*******************************************************************************

function hCloseBasicIDE() as boolean

    '///<h3>Close the BASIC-IDE with best effort verification</h3>
    '///<i>Starting point: Basic IDE is open and has the focus</i>
    '///<ol>
    '///+<li>File-menu</li>
    '///+<li>Slot FileClose</li>
    '///+<li>hCloseDocument()/hDestroyDocument()</li>
    '///</ol>
    '///<ul>
    
    use "global\tools\includes\optional\t_docfuncs.inc"
    
    const CFN = "hCloseBasicIDE::"
    dim brc as boolean

    '///+<li>Close the macro controls float</li>
    hCloseCollectionBar()

    '///+<li>Try to close the current document</li>
    brc = hDestroyDocument()
    if ( brc ) then
        hCloseBasicIDE() = true
    else
        hCloseBasicIDE() = false
        qaerrorlog( "#i54135# Failed to close BASIC IDE" )
    endif
    '///</ul>

end function

'*******************************************************************************

function hReOpenBasicDialog() as boolean

    '///<h3>Open BASIC-IDE after loading a workfile with documentbound macro</h3>
    '///<i>Starting point: Focus on document containing at least one macro</i><br>
    '///<i>IMPORTANT: Errorhandling is incomplete, test may end up in undefined state</i>
    '///<ul>
    
    dim brc as boolean

    '///+<li>open the macro dialog, select the last module</li>
    brc = hSelectTheLastModule( true )
    if ( not brc ) then
        warnlog( "Could not open the last module for editing." )
        hReOpenBasicDialog() = false
        exit function
    endif

    '///+<li>edit the module</li>
    Bearbeiten.Click()

    '///+<li>find the dialog window.</li>
    kontext "BasicIDE"
    BasicIde.maximize()

    brc = hFindFirstDialog() 
    if ( not brc ) then
        warnlog( "Could not find any dialog" )
        hReOpenBasicDialog() = false
    else
        hReOpenBasicDialog() = true
    endif
    
    '///+<li>Return TRUE on success, FALSE on failure</li>
    '///</ul>

end function

'*******************************************************************************

function hFindFirstDialog() as boolean

    '///<h3>Find the first dialog in the BASIC-IDE</h3>
    '///<i>Starting point: Basic IDE must be open and has the focus</i>
    '///<ul>
    
    dim iPosOverTabbar as integer
    dim brc as boolean
    const CFN = "hFindFirstDialog::"

    '///+<li>Set focus to Basic IDE, maximize it</li>
    kontext "BasicIde"
    BasicIde.maximize()
    

    brc = false

    '///+<li>move pointer over the tabbar and click until a dialog opens (hopefully)</li>
    for iPosOverTabbar = 1 to 20

        Tabbar.MouseDown ( 5 * iPosOverTabbar , 50 )
        Tabbar.MouseUp   ( 5 * iPosOverTabbar , 50 )

        if ( DialogWindow.Exists() ) then
            brc = true
            DialogWindow.typeKeys( "<up>", 5 )
            exit for
        endif

    next iPosOverTabbar
    
    '///+<li>Return TRUE on success, FALSE on failure</li>
    if ( brc ) then
        printlog( CFN & "Found dialog" )
    else
        printlog( CFN & "Could not find the first dialog via tabbar" )
    endif
    hFindFirstDialog() = brc
    '///</ul>

end function

'*******************************************************************************

function hSelectDialogPane() as boolean

    '///<h3>Select the dialog pane in the Basic dialog editor</h3>
    '///<i>Starting point: Basic dialog editor is open and has the focus</i>
    '///<ul>

    const CFN = "hSelectDialogPane::"
    printlog( CFN & "Enter" )
    
    '///+<li>Select background dialog by rubberband</li>
    kontext "BasicIDE"
    
    '///+<li>Set the mousepointer down at 90/90</li>
    DialogWindow.mouseDown( 90 , 90 )
    
    '///+<li>Move it to 10/10</li>
    DialogWindow.mouseMove( 10 , 10 )
    
    '///+<li>Release the mouse pointer at 10/10</li>
    DialogWindow.mouseUp  ( 10 , 10 )
    
    '///+<li>Return TRUE, currently no verification is implemented</li>
    printlog( CFN & "Exit: Selected dialog pane (not verified)" )
    hSelectDialogPane() = true
    '///</ul>

end function

'*******************************************************************************

function hCloseCollectionBar() as boolean

    '///<h3>Close the ToolsCollectionBar (Macro Controls Float)</h3>
    '///<i>Starting point: Basic Dialog editor is open, ToolsCollectionBar is open</i>
    '///<ul>
    
    const CFN = "hCloseCollectionBar::"
    const RETRIES = 3
    
    dim iTry as integer
        iTry = 0
    
    '///+<li>Try to close the ToolsCollectionBar max 3 times</li>
    kontext "ToolsCollectionBar"
    while ( ToolsCollectionBar.exists( 1 ) )
        iTry = iTry + 1
        printlog( CFN & "Closing ToolsCollectionBar..." )
        ToolsCollectionBar.close()
        
        if ( iTry = RETRIES ) then
            printlog( CFN & "Failed to close ToolsCollectionBar" )
            hCloseCollectionBar() = false
            exit function
        endif
    wend

    hCloseCollectionBar() = true
    '///</ul>
    
end function

'*******************************************************************************

function hDeleteMacro() as boolean

    '///<h3>Delete all text in the Basic IDE editor</h3>
    '///<i>Starting point: Basic IDE is open, has focus, code can be edited</i>
    '///<ul>
    
    use "global\tools\includes\optional\t_accels.inc"

    '///+<li>Verify all prerequisites are met (BasicIDE must be open)</li>
    kontext "BasicIDE"
    if ( not BasicIde.exists() ) then
        warnlog( "hDeleteMacro::BasicIDE is not visible/active. Aborting" )
        hDeleteMacro() = false
        exit function
    endif

    dim brc as boolean
    
    '///+<li>get the languagespecific accelerator for "Edit->Select All"</li> 
    dim cAccelerator as string
        cAccelerator = hGetAccel( "SelectAll" )
    
    printlog( "hDeleteMacro::Using Accel: " & cAccelerator )

    ' Common for all actions is that we delete the existing macro before
    ' inserting the new one. This fails if the accelerator for SelectAll
    ' is incorrect, we try to recover by doing this manually then.
    '///+<li>Delete the entire content of the Basic IDE editor</li>
    EditWindow.TypeKeys( cAccelerator )
    EditWindow.TypeKeys( "<Delete>" )

    ' Verify that the EditWindow is empty, if not use a method that
    ' should work for all languages/locales
    '///+<li>Verify that the editor window is indeed empty</li>
    brc = hIsEditWindowEmpty()
    if ( not brc ) then
        qaerrorlog( "#i52430# hDeleteMacro::Accelerator " & cAccelerator & " failed" )
        printlog( "hDeleteMacro::Trying alternative method" )
        EditWindow.TypeKeys( "<Mod1 Home>" )
        EditWindow.TypeKeys( "<Mod1 Shift End>" )
        EditWindow.TypeKeys( "<Delete>" )
        brc = hIsEditWindowEmpty()
    endif

    '///+<li>Return TRUE on success, FALSE on failure</li>
    hDeleteMacro() = brc
    '///</ul>

end function

'*******************************************************************************

function hIsEditWindowEmpty() as boolean

    '///<h3>Test if the Basic-EditWindow is empty</h3>
    '///<i>Starting point: Basic IDE editor is open and has the focus</i>
    '///<ul>
    
    '///+<li>Verify that prerequisites are met: Basic IDE must be open</li>
    kontext "BasicIDE"
    if ( not BasicIDE.exists() ) then
        warnlog( "hIsEditWindowEmpty::BasicIde is not open/visible" )
        hIsEditWindowEmpty() = false
        exit function
    endif

    '///+<li>Verify that the EditWindow is empty. EditCopy is disabled when empty</li>
    EditWindow.TypeKeys( "<Mod1 Home>" )
    EditWindow.TypeKeys( "<Mod1 Shift End>" )
    try
        EditCopy
        printlog( "EditCopy should be enabled for empty Basic IDE editor" )
    catch
        setClipboard( "" )
    endcatch

    if ( getClipBoardText() <> "" ) then
        hIsEditWindowEmpty() = false
        printlog( "hIsEditWindowEmpty::no - content follows:" )
        printlog( GetClipboardText )
    else
        hIsEditWindowEmpty() = true
        printlog( "hIsEditWindowEmpty::yes" )
    endif
    
    '///+<li>Return TRUE on success, FALSE on failure</li>
    '///</ul>

end function

'*******************************************************************************

function hRenameTab( cTabName as string ) as integer

    '///<h3>Rename the first tab in the Basic IDE Tabbar</h3>
    '///<i>Starting point: Basic IDE is open, at least one tab exists</i><br>
    '///Return values are:
    '///<ul>
    '///+<li>0 = Tab was correctly renamed</li>
    '///+<li>1 = Invalid name warning is displayed (handle outside of function)</li>
    '///+<li>2 = Unknown error / failed to rename the tab</li>
    '///+<li>3 = Unable to access context menu</li>
    '///</ul>
    '///Description
    '///<ul>
    
    const CFN = "hRenameTab::"
    
    dim cString as string
    dim iWait as integer
    dim iTry as integer
    dim brc as boolean
        brc = false

    cString = "<HOME><SHIFT END>" & cTabName & "<RETURN>" 
    kontext "basicide"
    
    '///+<li>Open the context menu of the tabbar</li>
    for iTry = 1 to 5
	    try
    	    kontext "basicide"
        	Tabbar.OpenContextMenu
            hMenuSelectNr( 3 )
            sleep( 2 )
            brc = true

        catch
            printlog( "No context menu, retrying..." )
        endcatch
    next iTry    
        '///+<li>Select to rename the tab</li>
    if ( not brc ) then
        warnlog( CFN & "Unable to access Context Menu" )
        hRenameTab() = 3 
        exit function       
    endif
    
    try
        '///+<li>Overwrite the old name of the tab</li>
        kontext "basicide"
        Tabbar.TypeKeys( cString , true )
        hRenameTab() = 0
        printlog( CFN & "Tab renamed: " & cTabName )
    catch
        '///+<li>Handle possible errors</li>
        hRenameTab() = 2
        warnlog(  CFN & "Failed to rename tab: " & cTabName )
        exit function
    endcatch
    
    kontext "active"
    for iWait = 1 to 3
        sleep( 1 )
        if ( active.exists() ) then
            hRenameTab() = 1
            printlog( CFN & "Invalid name for tab: Warning is displayed" )
            exit for
        endif
    next iWait
    '///+<li>Return 0, 1 or 2</li>
    '///</ul>
    
end function

'*******************************************************************************

function hGetTabNameFromOrganizer() as string

    '///<h3>Retrieve the name of a tab from the Basic Library Organizer</h3>
    '///<i>Starting point: Basic IDE is open and has the focus</i>
    '///<ul>

    const CFN = "hGetTabNameFromOrganizer::"
    dim cReturnString as string

    '///+<li>Open the Basic Library Organizer</li>
    '///+<li>Go to the Module-Tab</li>
    '///+<li>Get the name of the currently selected item from the Module-list</li>
    '///+<li>Close the Macro Library Organizer</li> 
    if ( hOpenBasicObjectOrganizer( 1 ) ) then
        cReturnString = Modulliste.GetSelText()
        TabModule.Close()
        printlog( CFN & "Returning Tab-Name: " & cReturnString )
    else
        cReturnString = "Failure: Macro Organizer not open"
        warnlog( CFN & cReturnString )
        
    endif
    
    '///+<li>Return the name from the Module-list</li>
    hGetTabNameFromOrganizer() = cReturnString
    '///</ul>
   
end function

'*******************************************************************************

function hNewDialog() as boolean

    '///<h3>Create a new BASIC dialog</h3>
    '///<i>Starting point: Basic IDE is open</i>
    '///<ul>

    const CFN = "hNewDialog::"

    '///+<li>Open the context menu for the tabbar</li>
    kontext "basicide"
    Tabbar.OpenContextMenu
    
    '///+<li>Select the first item (insert)</li>
    hMenuSelectNr( 1 )
    WaitSlot( 1000 ) ' sleep( 1 )
    
    '///+<li>Select the second item (new dialog)</li>
    hMenuSelectNr( 2 )
    WaitSlot( 2000 ) ' sleep( 2 )
    
    '///+<li>Verify that a new dialog is opened and has the focus</li>
    if ( DialogWindow.Exists() ) then 
        printlog( CFN & "New dialog is open" )
        hNewDialog() = true
		if ( DialogWindow.IsMax() = false ) then		
			DialogWindow.Maximize()
			Wait( 2000 )
		end if		
    else
        warnlog( CFN & "New dialog is not open" )
        hNewDialog() = false
    endif
    '///+<li>Return TRUE on success, FALSE on failure</li>
    '///</ul>

end function

'*******************************************************************************

function hNewModule() as boolean

    '///<h3>Create a new BASIC module</h3>
    '///<i>Starting point: Basic IDE is open</i>
    '///<ul>

    const CFN = "hNewModule::"

    '///+<li>Open the context menu for the tabbar</li>
    kontext "basicide"
    Tabbar.OpenContextMenu
    
    '///+<li>Select the first item (insert)</li>
    hMenuSelectNr( 1 )
    WaitSlot( 1000 ) ' sleep( 1 )
    
    '///+<li>Select the first item (new module)</li>
    hMenuSelectNr( 1 )
    WaitSlot( 2000 ) ' sleep( 2 )
    
    '///+<li>Verify that a new module is opened and has the focus</li>
    if ( EditWindow.Exists() ) then 
        printlog( CFN & "New module is open" )
        hNewModule() = true 
    else
        warnlog( CFN & "New module is not open" )
        hNewModule() = false
    endif
    '///+<li>Return TRUE on success, FALSE on failure</li>
    '///</ul>

end function        

'*******************************************************************************

function hHideModule() as boolean

    '///<h3>Hide a Basic Module from the IDE</h3>
    '///<i>Starting point: Basic IDE is open and has the focus</i>
    '///<ul>

    const CFN = "hHideModule::"
    
    kontext "basicide"
    
    '///+<li>Open the context menu of the tabbar</li>
    try
       tabbar.openContextMenu
       
       '///+<li>Select the fourth entry (Hide)</li>
       hMenuSelectNr( 4 )
       sleep( 1 )
       hHideModule() = true
       printlog( CFN & "Success" )
    catch
       hHideModule() = false 
       warnlog( CFN & "Failure" )
    endcatch
    '///+<li>Return TRUE on success, FALSE on any other condition (not verified)</li>
    '///</ul>
   
end function   

'*******************************************************************************

function hInitFormControls( cModule as string ) as boolean

    '///<h3>Get from a new document to the Macro Controls Float in one go</h3>
    '///<i>Starting point: Plain document</i><br>
    '///This function performs following steps:
    '///<ol>
    '///+<li>Open the Basic Organizer</li>
    '///+<li>Create a new module for the current document</li>
    '///+<li>Open a new basic dialog</li>
    '///+<li>Open the Macro controls float</li>
    '///</ol>
    '///Description:
    '///<ul>
    
    const CFN = "hInitFormControls::"    
    dim brc as boolean
    '///+<li>Create and edit a new module for the current document</li>
    brc = hInitBasicIde( cModule )
    if ( not brc ) then
        warnlog( "Could not create new module" )
        hCloseBasicIde()
        hInitFormControls() = false
        exit function
    endif     
    
    kontext "BasicIde"
    BasicIde.maximize()   

    '///+<li>Create a new dialog.</li>
    brc = hNewDialog()
    if ( not brc ) then
        warnlog( "Could not create Basic-Dialog" )
        hCloseBasicIde()
        hInitFormControls() = false
        exit function
    endif    
    
    '///+<li>Open the macro controls float.</li>
    brc = hShowMacroControls()
    if ( not brc ) then
        warnlog( CFN & "Could not tear off the macro controls float" )
        hCloseBasicIde()
        hInitFormControls() = false
        exit function            
    endif
    
    '///+<li>Return TRUE if all went well, FALSE on any other error</li>
    hInitFormControls() = true
    '///</ul>
    '///NOTE: The function will try to cleanup after itself in case of failure
    
end function

'*******************************************************************************

function hInitBasicIde( cModule as string ) as boolean

    '///<h3>Get from a new document to the Basic IDE in one go</h3>
    '///<i>Starting point: New, plain document</i>
    '///<ul>
    
    use "global\tools\includes\optional\t_basic_organizer_tools.inc"
    use "global\tools\includes\optional\t_listfuncs.inc"

    const CFN = "hInitBasicIde::"

    if ( cModule = "" ) then
        warnlog( CFN & "Invalid parameter passed to function" )
        hInitBasicIde() = false
        exit function
    endif        
    
    ' some multi-purpose return code 
    dim brc as boolean 
    
    '///+<li>Open the basic Organizer</li>
    brc = hOpenBasicOrganizerFromDoc()
    if ( not brc ) then
        warnlog( CFN & "Could not open the Basic Organizer, aborting" )
        hInitBasicIde() = false
        exit function
    endif

    '///+<li>Create a new module for the current document</li>
    brc = hCreateModuleForDoc( cModule )
    if ( not brc ) then
        warnlog( CFN & "The Basic-IDE is not open, aborting" )
        hInitBasicIde() = false
        
        kontext "Makro"
        if ( Makro.exists() ) then
            printlog( CFN & "Closed Macro Organizer" )
            Makro.cancel()
        endif
        exit function
    endif
    
    '///+<li>Maximize the BasicIDE</li>
    kontext "BasicIde"
    BasicIde.maximize()
    
    '///+<li>Verify that the Basic IDE is open, return TRUE on success, FASLE on failure</li>
    hInitBasicIde() = true
    '///</ul>
    '///NOTE: The function will try to to clean up after itself in case of any error    
    
end function 

'*******************************************************************************

function hInsertMacro( _id as integer ) as boolean

    '///<h3>Insert a macro into the Basic-IDE edit window by Index</h3>
    '///<i>Starting point: Basic IDE is open, has the focus</i>
    '///<ul>
    
    const CFN = "hInsertMacro::"
    dim brc as boolean
    
    if ( ( _id < 1 ) or ( _id > 3 ) ) then
        warnlog( CFN & "Invalid parameter passed to function: " & _id )
        hInsertMacro() = false
        exit function
    endif
    
    kontext "basicide"
    
    '///+<li>Delete the content of the edit window</li>
    brc = hDeleteMacro()
    if ( not brc ) then
        warnlog( CFN & "Content of Basic-IDE was not deleted" )
        hInsertMacro() = false
        exit function
    endif
        
    '///+<li>Insert a macro by index (passed as function parameter)</li>
    select case _id
    case 1 :
        ' this is a working macro that contains no errors
        EditWindow.TypeKeys( "'# TTMacro1: This is a short testscript for automated testing!<return><return>" )
        EditWindow.TypeKeys( "sub main<return><return>" )
        EditWindow.TypeKeys( "print " & Chr (34) + "hallo" & Chr (34) )
        EditWindow.TypeKeys( "<Return><Return>" )
        EditWindow.TypeKeys( "<Home>end sub<return>" )
        hInsertMacro() = true
        printlog( CFN & "Inserted: " & _id )
    case 2 :
        EditWindow.TypeKeys( "'# TTMacro2: a second module for automated testing!" )
        EditWindow.TypeKeys( "<Return><Return>" )
        hInsertMacro() = true
        printlog( CFN & "Inserted: " & _id )
    case 3 :
        EditWindow.TypeKeys( "'# TTMacro3: Bring up a messagebox<return><return>" )
        EditWindow.TypeKeys( "sub main<return><return>" )
        EditWindow.TypeKeys( "    msgbox " & Chr (34) + "TTMacro3" & Chr (34) )
        EditWindow.TypeKeys( "<Return><Return>" )
        EditWindow.TypeKeys( "<Home>end sub<return>" )
        hInsertMacro() = true  
        printlog( CFN & "Inserted: " & _id )
    end select
   
    '///</ul>
    
end function

'*******************************************************************************

function hTestMacro( _id as integer ) as integer

    '///<h3>Verify that the correct macro is visible in the IDE edit Window</h3>
    '///<i>Starting point: Basic IDE is open and has focus</i>
    '///<ul>
    
    const CFN = "hTestMacro::"

    kontext "basicide"

    '///+<li>Copy the first line in the editwindow to clipboard</li>  
    EditWindow.TypeKeys( "<Mod1 Home>" )
    EditWindow.TypeKeys( "<Home><Shift End>" ) 
    EditCopy
    
    '///+<li>Compare the string to a unique substring for the macro</li>
    select case _id
    case 1 :
        if ( Instr ( GetClipboardText, "TTMacro1" ) <> 0 ) then
            printlog( CFN & "The correct macro is displayed in the editwindow" )
            htestMacro() = 1
            exit function
        endif
    case 2 :
        if ( Instr ( GetClipboardText, "TTMacro2" ) <> 0 ) then
            printlog( CFN & "The correct macro is displayed in the editwindow" )
            htestMacro() = 2
            exit function
        endif
    case 3 :
        if ( Instr ( GetClipboardText, "TTMacro3" ) <> 0 ) then
            printlog( CFN & "The correct macro is displayed in the editwindow" )
            htestMacro() = 3
            exit function
        endif        
    end select
    
    hTestMacro() = 0
    '///</ul>
    
end function

'*******************************************************************************

function hCreateBasicWorkFiles() as string

    '///<H3>Create a macro, attach it to a file, save the file</H3>
    '///<i>Starting point: The first doc or any other plain document</i>
    '///<ul>
    const CFN = "hCreateBasicWorkFiles::"
    const CMACRO = "TestMacro"
    
    ' some returncodes
    dim brc as boolean
    dim irc as integer
    dim crc as string
    
    dim sFile as string
        sFile = hGetBasicWorkFile( "CURRENT" )
        
    '///+<li>Open a new document</li>
    call hCreateDocument()
    
    '///+<li>Change the document (to save it)</li>
    crc = hChangeDoc()
    printlog( CFN & crc )
    
    '///+<li>Open Basic-Organizer, select the document and create a module</li>
    brc = hInitBasicIde( CMACRO )
    if ( not brc ) then
        printlog( CFN & "Unable to create a module, aborting" )
        call hDestroyDocument()
        hCreateBasicWorkFiles() = ""
        exit function
    endif
    
    '///+<li>Insert a macro that triggers a messagebox</li>
    brc = hInsertMacro( 3 )
    if ( not brc ) then
        printlog( CFN & "Unable to insert macro, aborting" )
        hCloseBasicIde()
        call hDestroyDocument()
        hCreateBasicWorkFiles() = ""
        exit function
    endif
    
    '///+<li>Close the Basic-IDE</li>
    brc = hCloseBasicIde()
    
    '///+<li>Save the file without autoextension, overwrite existing</li>
    brc = hFileSaveAsKill( sFile )
    if ( not brc ) then
        printlog( CFN & "Unknown error while saving the file. Resetting" )
        call ExitRestartTheOffice
        hCreateBasicWorkFiles() = ""
        exit function
    endif
    
    '///+<li>Close the document</li>
    call hDestroyDocument()
    
    '///</ul>
    
end function


'*******************************************************************************

function hIDESelectTabByIndex( iIndex as integer ) as string

    use "global\tools\includes\optional\t_accels.inc"


    '///<h3>Select a tab within the BASIC-IDE by index</h3>
     '///<i>The BASIC-IDE has to be open and has the focus. This function
     '///+ selects the tab by sending CTRL+PAGEUP or CTRL+PAGEDOWN to the IDE using 
     '///+ the .typeKeys method. It is assumed that - when opening the IDE - the 
     '///+ first tab is active. When the specified index has been reached we 
     '///+ retrieve the name of the current tab by opening the context menu on the 
     '///+ tab and selecting &quot;Rename&quot;. The name (which should be 
     '///+ highlighted by default) is then copied to the clipboard and returned  
     '///+ to the calling function.</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>Index of the requested Tab (Integer)</li>
    '///<ul>
    '///+<li>Index must not be negative</li>
    '///+<li>0 does nothing (returning the name of the current tab)</li>
    '///+<li>Index must be &le; present number of tabs</li>
    '///+<li>If Index &gt; number of tabs, the last tab is selected</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Name of the selected tab (string)</li>
    '///<ul>
    '///+<li>Always evaluate the returnvalue</li>
    '///</ul>
    '///</ol>

    const CFN = "hIDESelectTabByIndex::"
    printlog( CFN & "Enter with option: " & iIndex )
    dim brc as boolean 'a multi purpose boolean returnvalue
    
    dim cTabName as string
    dim iCurrentTab as integer
    dim cAccel as string

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Set context to BASIC IDE</li>
    kontext "BasicIDE"
    
    '///+<li>Retrieve the accelerator to switch tabs</li>
    '///+<li>Switch tabs</li>
    select case iIndex
    case 0   :  cAccel = ""
    case 1   :  cAccel = hGetAccel( "IDE_SWITCH_TAB-" )
                for iTab = 1 to 20
                    BasicIDE.typeKeys( cAccel )
                next iTab
    case else:  cAccel = hGetAccel( "IDE_SWITCH_TAB+" )
                for iTab = 1 to iIndex
                    BasicIDE.typeKeys( cAccel )
                next iTab
    end select
    
    '///+<li>Select the Tabbar</li>
    Kontext "Tabbar"
    
    '///+<li>Open the context menu</li>
    hUseMenu()
    
    '///+<li>Select &quot;Rename&quot;</li>
    hSelectMenuNr( 3 )
    
    '///+<li>Copy the selected string to the clipboard</li>
    EditCopy
    
    '///+<li>Send ESC to the tabbar to deselect the tab</li>
    Kontext "Tabbar"
    Tabbar.typeKeys( "<ESCAPE>" )
    
    '///+<li>Copy the string from clipboard to a local variable</li>
    cTabName = getClipboardText
    
    '///+<li>Print exit message, set return value</li>
    printlog( CFN & "Exit with result: " & cTabName )
    hIDESelectTabByIndex() = cTabName
    '///</ul>

end function
