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
' **
' ** owner : gregor.hartmann@oracle.com
' **
' ** short description : tools for the property-browser test
' **
'\******************************************************************************

function hShowMacroControls() as boolean

    '///<h3>Tear off the ToolsCollectionBar from the MacroBar</h3>
    '///<i>Starting point: Basic dialog editor is open and has the focus</i>
    '///<ul>
    const CFN = "hShowMacroControls::"

    const MAX_WAIT = 5
    const SHORT_WAIT = 1

    const VERTICALLY_CENTERED = 50
    const HORIZONTALLY_CENTERED = 50
    
    if ( GVERBOSE ) then printlog( CFN & "Enter" )

    '///+<li>Test whether the ToolsCollectionBar is already open. If yes, exit directly</li>
    kontext "ToolsCollectionBar"
    if ( ToolsCollectionBar.exists( SHORT_WAIT ) ) then
        printlog( CFN & "ToolsCollectionBar is already open." )
        hShowMacroControls() = true
        exit function
    endif
    
    '///+<li>Verify that the Dialog Bar is available</li>
    Kontext "DialogBar"
    if ( DialogBar.exists( MAX_WAIT ) ) then

        '///+<li>Verify that the controls-button is enabled</li>
        try        
            '///+<li>Click on the controls-button to open the ToolsCollectionBar</li>
            controls.click()
            
            '///+<li>Tear off the ToolsCollectionBar from the DialogBar</li>
            controls.tearOff()
            
            '///+<li>Verify that the ToolsCollectionBar is visible and can be accessed</li>
            kontext "ToolsCollectionBar"
            if ( ToolsCollectionBar.exists( MAX_WAIT ) ) then
            
                '///+<li>Move it to the upper left corner at pos 50/50</li>
                ToolsCollectionBar.Move ( HORIZONTALLY_CENTERED , VERTICALLY_CENTERED )
                printlog( CFN & "ToolsCollectionBar is open and has been placed" )
                hShowMacroControls() = true
            else
                printlog( CFN & "ToolsCollectionBar is not available. Aborting." )
                hShowMacroControls() = false
            endif
        catch
            printlog( CFN & "The Controls-Button is not enabled. Aborting." )
            hShowMacroControls() = false
        endcatch	
    else
        printlog( CFN & "The Dialog Bar is not available. Aborting." )
        hShowMacroControls() = false
    endif
    '///</ul>    
        
end function

'*******************************************************************************

function hGetBasicWorkFile( cMajorID as string ) as string

    '///<h3>Retrieve a filename, depending on gApplication and Build-ID</h3>
    '///<i>Structure: work-directory/basic[.suffix]</i>
    const CFN = "hGetBasicWorkFile::"
    dim sFile as string : sFile = hGetWorkPath() & "basic" & hGetSuffix( cMajorId )
    printlog( CFN & sFile & " for " & gApplication & " (" & cMajorID & ")" )
    hGetBasicWorkFile() = sFile

end function

'*******************************************************************************

function hOpenBasicIDE() as boolean

    '///<h3>Open the Basic IDE for the last module listed in the Macro Organizer</h3>
    '///<i>Starting point: Basic Macro Organizer has focus</i>
    
    const CFN = "hOpenBasicIDE::"
    const BUTTON_TIMEOUT = -1
    const MAX_WAIT = 2
    
    if ( hSelectTheLastModule( true ) ) then
    
        if ( hClickButton( Bearbeiten) <> BUTTON_TIMEOUT ) then

            kontext "BASICIDE"
            if ( BasicIDE.exists( MAX_WAIT ) ) then
                printlog( "Basic IDE is open" )
                BasicIDE.maximize()
                hOpenBasicIDE() = true
            else
                warnlog( CFN & "Failed to open BASIC IDE" )
                hOpenBasicIDE() = false
            endif
        else
            warnlog( CFN & "Edit button is not enabled, cannot open BASIC IDE"
            hOpenBasicIDE() = false
        endif
    else
        warnlog( CFN & "Unable to select the last module" )
        hOpenBasicIDE() = false
    endif

end function

'*******************************************************************************

function hCloseBasicIDE() as boolean

    '///<h3>Close the BASIC-IDE</h3>
    hCloseCollectionBar()
    kontext "BasicIDE"
    hCloseBasicIDE() = hDestroyDocument()

end function

'*******************************************************************************

function hReOpenBasicDialog() as boolean

    '///<h3>Open BASIC-IDE after loading a workfile with documentbound macro</h3>
    '///<i>Starting point: Focus on document containing at least one macro</i><br>
    const CFN = "hReopenBasicDialog: "
    const BUTTON_TIMEOUT = -1
    
    if ( hSelectTheLastModule( true ) ) then

        if ( hClickButton( Bearbeiten ) <> BUTTON_TIMEOUT ) then

            kontext "BasicIDE"
            BasicIde.maximize()

            if ( hFindFirstDialog() ) then
                printlog( "BASIC IDE is open." )
                hReOpenBasicDialog() = true
            else
                warnlog( CFN & "Could not find any dialog" )
                hReOpenBasicDialog() = false
            endif
        else
            warnlog( CFN & "Unable to click Edit button, BASIC IDE not open" )
            hReOpenBasicDialog() = false
        endif
    else
        warnlog( CFN & "Could not open the last module for editing." )
        hReOpenBasicDialog() = false
    endif

end function

'*******************************************************************************

function hFindFirstDialog() as boolean

    '///<h3>Find the first dialog in the BASIC-IDE</h3>
    '///<i>Starting point: Basic IDE must be open and has the focus</i>
    '///<ul>
    
    const CFN = "hFindFirstDialog::"
    const REPEAT_COUNT = 5 ' number of times to send a keystroke to BASIC IDE
    const MAX_NUMBER_OF_RETRIES = 20
    const SELECT_TAB_RIGHT = "<MOD1 PAGEUP>"
    const SCROLL_UP_LEFT   = "<UP><LEFT>"

    dim iPosOverTabbar as integer
    hFindFirstDialog() = false

    '///+<li>Set focus to Basic IDE, maximize it</li>
    kontext "BasicIDE"
    BasicIde.maximize()


    '///+<li>move pointer over the tabbar and click until a dialog opens (hopefully)</li>
    for iPosOverTabbar = 1 to MAX_NUMBER_OF_RETRIES

        BASICIDE.typeKeys( SELECT_TAB_RIGHT )

        if ( DialogWindow.Exists() ) then
            printlog( "BASIC Dialog window found" )
            hFindFirstDialog() = true
            DialogWindow.typeKeys( SCROLL_UP_LEFT , REPEAT_COUNT )
            exit for
        endif

    next iPosOverTabbar
       '///</ul>

end function

'*******************************************************************************

function hSelectDialogPane() as boolean

    '///<h3>Select the dialog pane in the Basic dialog editor</h3>
    '///<i>Starting point: Basic dialog editor is open and has the focus</i>
    const CFN = "hSelectDialogPane::"
    const UP_LEFT    = 10
    const DOWN_RIGHT = 90

    if ( GVERBOSE ) then printlog( CFN & "Enter" )
    
    kontext "BasicIDE"
    DialogWindow.mouseDown( DOWN_RIGHT , DOWN_RIGHT )
    DialogWindow.mouseMove( UP_LEFT , UP_LEFT )
    DialogWindow.mouseUp  ( UP_LEFT , UP_LEFT )
    
    printlog( CFN & "Selected dialog pane (not verified)" )
    hSelectDialogPane() = true

end function

'*******************************************************************************

function hCloseCollectionBar() as boolean

    const CFN = "hCloseCollectionBar(): "

    kontext "ToolsCollectionBar"
    if ( ToolsCollectionBar.exists() ) then

        if ( hCloseDialog( ToolsCollectionBar, "close" ) ) then
            printlog( "Closed Macro Controls Float" )
            hCloseCollectionBar() = true
        else
            warnlog( CFN & "Failed to close Macro Controls Float" )
            hCloseCollectionBar() = false
        endif
    else
        printlog( "Macro Controls Float not open" )
        hCloseCollectionBar() = true
    endif

end function

'*******************************************************************************

function hDeleteMacro() as boolean

    '///<h3>Delete all text in the Basic IDE editor</h3>
    const CFN = "hDeleteMacro(): "
    
    kontext "BasicIDE"
    if ( BasicIDE.exists() ) then
        WaitSlot() : Wait( 100 ) : EditWindow.typeKeys( "<MOD1 HOME>" )
        WaitSlot() : Wait( 100 ) : EditWindow.typeKeys( "<MOD1 SHIFT END>" )
        WaitSlot() : Wait( 100 ) : EditWindow.typeKeys( "<DELETE>" )
        hDeleteMacro() = hIsEditWindowEmpty()
    else
        warnlog( CFN & "Basic IDE is not open" )
        hDeleteMacro() = false
    endif

end function

'*******************************************************************************

function hIsEditWindowEmpty() as boolean

    '///<h3>Test if the Basic-EditWindow is empty</h3>
    '///<i>Starting point: Basic IDE editor is open and has the focus</i>
    const CFN = "hIsEditWindowEmpty(): "

    kontext "BasicIDE"
    if ( BasicIDE.exists() ) then
        SetClipboard( "" )
        WaitSlot() : EditWindow.typeKeys( "<MOD1 HOME>" )
        WaitSlot() : EditWindow.typeKeys( "<MOD1 SHIFT END>" )
        hUseAsyncSlot( "EditCopy" )

        if ( GetClipBoardText() = "" ) then
            hIsEditWindowEmpty() = true
        else
            warnlog( CFN & "Edit window is not empty" )
            hIsEditWindowEmpty() = false
        endif
    else
        warnlog( CFN & "BasicIDE is not open" )
        hIsEditWindowEmpty() = false
    endif
    
end function

'*******************************************************************************

function hRenameTab( cTabName as string ) as integer

    const CFN = "hRenameTab(): "

    const MENUITEM_RENAME  = 3
    const MAX_WAIT         = 1000

    const RETVAL_SUCCESS   = 0
    const RETVAL_NO_IDE    = 1
    const RETVAL_NO_ACCESS = 2
    const RETVAL_NO_MENU   = 3

    dim sFullString as string : sFullString = "<HOME><SHIFT END>" & cTabName & "<RETURN>"

    kontext "BasicIDE"
    if ( BasicIDE.exists() ) then
        try
            hRenameTab() = RETVAL_NO_MENU
            Tabbar.openContextMenu()
            hMenuSelectNr( MENUITEM_RENAME )
            Wait( MAX_WAIT )

            ' At this point the string on the tab should be selected, print the 
            ' string to the log and overwrite it with the new name
            hRenameTab() = RETVAL_NO_ACCESS

            Tabbar.typeKeys( sFullString , true )
            hRenameTab() = RETVAL_SUCCESS
        catch
            warnlog( CFN & "Failed to rename the current module" )
        endcatch
    else
        warnlog( CFN & "BASIC IDE is not open" )
        hRenameTab() = RETVAL_NO_IDE
    endif

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
    const MAX_WAIT = 2000

    '///+<li>Open the context menu for the tabbar</li>
    kontext "basicide"
    Tabbar.OpenContextMenu
    
    '///+<li>Select the first item (insert)</li>
    hMenuSelectNr( 1 )
    WaitSlot( MAX_WAIT ) ' sleep( 1 )
    
    '///+<li>Select the second item (new dialog)</li>
    hMenuSelectNr( 2 )
    WaitSlot( MAX_WAIT ) ' sleep( 2 )
    
    '///+<li>Verify that a new dialog is opened and has the focus</li>
    if ( DialogWindow.Exists() ) then 
        printlog( CFN & "New dialog is open" )
        hNewDialog() = true
	if ( DialogWindow.IsMax() = false ) then		
            DialogWindow.Maximize()
            Wait( MAX_WAIT )
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
    const MAX_WAIT = 2000

    '///+<li>Open the context menu for the tabbar</li>
    kontext "basicide"
    Tabbar.OpenContextMenu
    
    '///+<li>Select the first item (insert)</li>
    hMenuSelectNr( 1 )
    WaitSlot( MAX_WAIT ) ' sleep( 1 )
    
    '///+<li>Select the first item (new module)</li>
    hMenuSelectNr( 1 )
    WaitSlot( MAX_WAIT ) ' sleep( 2 )
    
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
    const MENUENTRY_HIDE = 4
    
    kontext "basicide"
    
    '///+<li>Open the context menu of the tabbar</li>
    try
       tabbar.openContextMenu
       
       '///+<li>Select the fourth entry (Hide)</li>
       hMenuSelectNr( MENUENTRY_HIDE )
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
    hInitFormControls() = false
    '///+<li>Create and edit a new module for the current document</li>

    if ( not hInitBasicIde( cModule ) ) then
        warnlog( "Could not create new module" )
        hCloseBasicIde()
        exit function
    endif     
    
    kontext "BasicIde"
    BasicIde.maximize()   

    '///+<li>Create a new dialog.</li>
    if ( not hNewDialog() ) then
        warnlog( "Could not create Basic-Dialog" )
        hCloseBasicIde()
        exit function
    endif    
    
    '///+<li>Open the macro controls float.</li>
    if ( not hShowMacroControls() ) then
        warnlog( CFN & "Could not tear off the macro controls float" )
        hCloseBasicIde()
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

    const CFN = "hInitBasicIde::"

    ToolsMacro_uno

    kontext "Makro"
    if ( Makro.exists( 2 ) ) then

        if ( hCreateModuleForDoc( cModule ) ) then

            kontext "BasicIDE"
            if ( BasicIDE.exists( 2 ) ) then
                BasicIDE.maximize()
                hInitBasicIde() = true
            else
                warnlog( CFN & "Failed to open BASIC IDE" )
                hInitBasicIde() = false
            endif
        else
            warnlog( CFN & "Failed to create a module for the current document" )
            hInitBasicIde() = false
        endif
    else
        warnlog( CFN & "Failed to open macro organizer" )
        hInitBasicIde() = false
    endif
    
end function 

'*******************************************************************************

function hInsertMacro( _id as integer ) as boolean

    '///<h3>Insert a macro into the Basic-IDE edit window by Index</h3>
    '///<i>Starting point: Basic IDE is open, has the focus</i>
    '///<ul>
    
    const CFN = "hInsertMacro::"
    kontext "basicide"

    printlog( CFN & "This function is deprecated" )
    
    '///+<li>Delete the content of the edit window</li>
    if ( not hDeleteMacro() ) then
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
    qaerrorlog( CFN & "This function is deprecated, remove me!" )

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
    '///<i>Starting point: Any plain document or the backing window</i>
    '///<ul>
    const CFN = "hCreateBasicWorkFiles::"
    const CMACRO = "TestMacro"
    
    dim sFile as string : sFile = hGetBasicWorkFile( "CURRENT" )
        
    '///+<li>Open a new document</li>
    hCreateDocument()
    
    '///+<li>Change the document (to save it)</li>
    printlog( CFN & hChangeDoc() )
    
    '///+<li>Open Basic-Organizer, select the document and create a module</li>
    if ( not hInitBasicIde( CMACRO ) ) then
        printlog( CFN & "Unable to create a module, aborting" )
        hDestroyDocument()
        hCreateBasicWorkFiles() = ""
        exit function
    endif
    
    '///+<li>Insert a macro that triggers a messagebox</li>
    if ( not hInsertMacro( 3 ) ) then
        printlog( CFN & "Unable to insert macro, aborting" )
        hCloseBasicIde()
        hDestroyDocument()
        hCreateBasicWorkFiles() = ""
        exit function
    endif
    
    '///+<li>Close the Basic-IDE</li>
    hCloseBasicIde()
    
    '///+<li>Save the file without autoextension, overwrite existing</li>
    if ( not hFileSaveAsKill( sFile ) ) then
        printlog( CFN & "Unknown error while saving the file. Resetting" )
        ExitRestartTheOffice
        hCreateBasicWorkFiles() = ""
        exit function
    endif
    
    '///+<li>Close the document</li>
    hDestroyDocument()
    
    '///</ul>
    
end function


'*******************************************************************************

function hIDESelectTabByIndex( iIndex as integer ) as string

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

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Set context to BASIC IDE</li>
    kontext "BasicIDE"
    
    '///+<li>Retrieve the accelerator to switch tabs</li>
    '///+<li>Switch tabs</li>
    select case iIndex
    case 1   :
        for iTab = 1 to 20
            BasicIDE.typeKeys( "<MOD1 PAGEDOWN>" )
        next iTab
    case else:  
        for iTab = 1 to iIndex
            BasicIDE.typeKeys( "<MOD1 PAGEUP>" )
        next iTab
    end select
    
    '///+<li>Select the Tabbar</li>
    Kontext "Tabbar"
    
    '///+<li>Open the context menu</li>
    hUseMenu()
    
    '///+<li>Select &quot;Rename&quot;</li>
    hSelectMenuNr( 3 )
    
    '///+<li>Copy the selected string to the clipboard</li>
    hUseAsyncSlot( "EditCopy" )
    
    '///+<li>Send ESC to the tabbar to deselect the tab</li>
    Kontext "Tabbar"
    Tabbar.typeKeys( "<ESCAPE>" )
    
    '///+<li>Copy the string from clipboard to a local variable</li>
    cTabName = getClipboardText
    
    '///+<li>Print exit message, set return value</li>
    if ( GVERBOSE ) then printlog( CFN & "Exit with result: " & cTabName )
    hIDESelectTabByIndex() = cTabName
    '///</ul>

end function

