'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: tools_customize.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 08:03:53 $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Update Test for the Tools Customize Dialog
'*
'\******************************************************************************

testcase tUpdtCustomize( cApp as string )

    warnlog( "#i106370# - Test disabled, cannot switch tabpage on Tools/Customize dialog" )
    goto endsub

    printlog( "Tools/Customize dialog" )
    const CLOSE_METHOD = 1 ' 1 = Cancel button

    if ( hCreateDocument() ) then    
        if ( hToolsCustomizeOpen() ) then
            hUpdtToolsCustomizeKeyboard()
            hUpdtToolsCustomizeMenu()
            hUpdtToolsCustomizeToolbars()
            hUpdtToolsCustomizeEvents()
            hToolsCustomizeClose( CLOSE_METHOD )
        else
            warnlog( "Tools/Customize dialog did not open" )
        endif
        hDestroyDocument
    else
        warnlog( "Failed to create initial document" )
    endif
    
endcase

'*******************************************************************************

function hUpdtToolsCustomizeMenu()

    '///<h3>Update test for the Tools/Customize - Menu-Tab</h3>
    '///<i>Starting point: Tools/Customize dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hUpdtToolsCustomizeMenu::"
    const ITEMNAME = "tUpdtCustomize"
    dim brc as boolean

    dim iItems as integer
    dim iCurrentItem as integer
    dim iMenuListItems as integer
    dim iEntriesListItems as integer    
    
    printlog( "" )
    printlog( "Menu" )

    '///+<li>Access the Menu-Tabpage</li>
    brc = hToolsCustomizeSelectTab( "Menu" )
    call DialogTest( TabCustomizeMenu )
    
    '///+<li>Click &quot;New...&quot; to add a new menu</li>
    printlog( CFN & "Click New..." )
    kontext "TabCustomizeMenu"
    BtnNew.click()
    
    '///+<li>Have a look at the MenuOrganiser, close it</li>
    Kontext "MenuOrganiser"
    printlog( CFN & "MenuOrganiser" )
    call DialogTest( MenuOrganiser )
    
    '///+<li>Click &quot;Down&quot;</li>
    printlog( CFN & "Down..." )
    ButtonDown.click()
    
    '///+<li>Click &quot;Up&quot;</li>
    printlog( CFN & "Up..." )
    ButtonUp.click()    
    
    '///+<li>Enter a name for a new menu</li>
    printlog( CFN & "Name the new menu" )
    MenuName.setText( "tUpdtCustomize" )
    
    'qaerrorlog( "#i60609# Undeclared listbox in menu organiser" )
    printlog( "TODO: Access the listbox in menuorganizer" )
    
    '///+<li>Close the dialog with OK</li>
    printlog( CFN & "Close the menu with ok" )
    MenuOrganiser.ok()
    
    '///+<li>Click the Menu List-Button</li>
    printlog( CFN & "Click the menu listbutton" )
    kontext "TabCustomizeMenu"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    '///+<li>Select the first entry (Move...) to open the Menu Organiser</li>
    printlog( CFN & "Select Move..." )
    hMenuSelectNr( 1 ) 
    
    '///+<li>Click the UP button</li>
    kontext "MenuOrganiser"
    printlog( CFN & "Click UP" )
    ButtonUp.click()
    
    '///+<li>Click the DOWN button</li>
    kontext "MenuOrganiser"
    printlog( CFN & "Click DOWN" )
    ButtonDown.click()

    '///+<li>Cancel the dialog</li>
    printlog( CFN & "Cancel the dialog" )
    MenuOrganiser.cancel()
    
    '///+<li>Click the Menu List-Button</li>
    printlog( CFN & "Click the menu listbutton" )
    kontext "TabCustomizeMenu"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    '///+<li>Select the second entry (Rename...) to open the Rename dialog</li>
    printlog( CFN & "Select Rename..." )
    hMenuSelectNr( 2 )

    '///+<li>Cancel the renaming dialog</li>
    printlog( CFN & "Cancel the renaming-dialog" )
    kontext "RenameMenu"
    RenameMenu.cancel()
    
    '///+<li>Click the Menu List-Button</li>
    printlog( CFN & "Click the menu listbutton" )
    kontext "TabCustomizeMenu"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    '///+<li>Select the third entry (Delete) to delete the new menu<br>
    '///+Note that there will be no warning as the menu is empty</li>
    printlog( CFN & "Select Delete" )
    hMenuSelectNr( 3 )
    
    '///+<li>Click the &quot;Add Commands...&quot; button</li>
    '///+<li>Check that the dialog is open, close it</li>
    kontext "TabCustomizeMenu"
    hUpdtToolsCustomizeScriptSelector( 3 )
    
    '///+<li>Check the number of entries in SaveIn-Listbox<br>
    printlog( CFN & "Check itemcount in SaveIn List" )
    kontext "TabCustomizeMenu"
    select case gApplication
    case "BACKGROUND"   :   brc = hToolsCustomizeTestSaveIn( 1 )
    case else           :   brc = hToolsCustomizeTestSaveIn( 2 )
    end select
    if ( not brc ) then
        warnlog( "Incorrect itemcount in listbox, see above" )
    endif
    
    '///+<li>Click &quot;Down&quot;</li>
    printlog( CFN & "Down..." )
    BtnDown.click()
    
    '///+<li>Click &quot;Up&quot;</li>
    printlog( CFN & "Up..." )
    BtnUp.click()
    
    '///+<li>Click the &quot;Modify&quot;-button and select the first entry (Add submenu...)</li>
    printlog( CFN & "Click the Modify-Button and select item 1 (Add submenu)" )
    kontext "TabCustomizeMenu"
    brc = hClickCommandButton( 1 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>Enter a name for the submenu, accept with OK</li>
    printlog( CFN & "Give the submenu a name, accept with OK" )
    kontext "CustomizeMenuName"
    call dialogtest( CustomizeMenuName )
    EingabeFeld.setText( ITEMNAME )
    CustomizeMenuName.ok()
    
    '///+<li>Find the submenu in the Entries-list and select it</li>
    printlog( CFN & "Find the new submenu in the list and select it" )
    WaitSlot()
    kontext "TabCustomizeMenu"
    iEntriesListItems = Entries.getItemCount()
    for iCurrentItem = 1 to iEntriesListItems
    
        Entries.select( iCurrentItem )
        if ( Entries.getSelText() = ITEMNAME ) then
            exit for
        endif
        
    next iCurrentItem
    
    
    '///+<li>Click the &quot;Modify&quot;-button and select the third entry (Rename...)</li>
    printlog( CFN & "Click the Modify-Button and select to rename the item" )
    kontext "TabCustomizeMenu"
    brc = hClickCommandButton( 3 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>Rename the the submenu, accept with ok</li>
    printlog( CFN & "Rename the item, accept with OK" )
    kontext "CustomizeMenuReName" 
    call dialogtest( CustomizeMenuReName )
    EingabeFeld.setText( ITEMNAME & "1" )
    CustomizeMenuReName.ok()
    
    '///+<li>Verify that the name has been updated</li>
    printlog( CFN & "Verify that the name has been updated" )
    kontext "TabCustomizeMenu"
    if ( Entries.getSelText() <> ( ITEMNAME & "1" ) ) then
        warnlog( "Names do not match" )
    endif
    
    '///+<li>Click the &quot;Modify&quot;-button and select the fourth entry (Delete)</li>
    printlog( CFN & "Click the Modify-Button and delete the current item" )
    kontext "TabCustomizeMenu"
    brc = hClickCommandButton( 4 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>Click the &quot;Modify&quot;-button and select the second entry (Begin a group)</li>
    printlog( CFN & "Click the Modify-Button and create a new group" )
    kontext "TabCustomizeMenu"
    brc = hClickCommandButton( 2 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>Click the &quot;Modify&quot;-button and select the fourth entry (Delete)</li>
    ' Note: The current index for the delete-function is at pos 2.
    printlog( CFN & "Click the Modify-Button and delete the new group" )
    kontext "TabCustomizeMenu"
    brc = hClickCommandButton( 2 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hUpdtToolsCustomizeToolbars()

    '///<h3>Update test for the Tools/Customize - Toolbar-Tab</h3>
    '///<i>Starting point: Tools/Customize dialog</i><br>

    '///<u>Input</u>:<br>
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Returns</u>:<br>
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Description</u>:<br>
    '///<ul>

    const CFN = "hUpdtToolsCustomizeToolbars::"
    dim brc as boolean
    
    dim iCurrentItem as integer
        
    printlog( "" )
    printlog( "Toolbars" )

    '///+<li>Access the toolbar tabpage</li>
    brc = hToolsCustomizeSelectTab( "Toolbars" )
    call DialogTest( TabCustomizeToolbars )
    
    '///+<li>Click &quot;New...&quot; to add a new toolbar</li>
    printlog( CFN & "Click New..." )
    kontext "TabCustomizeToolbars"
    BtnNew.click()
    
    '///+<li>Have a look at the New Toolbar dialog, close it</li>
    kontext "NewToolbar"
    if ( NewToolbar.exists( 2 ) ) then
        printlog( CFN & "NewToolbar" )
        call DialogTest( NewToolbar )
        
        '///+<li>Check that the SaveIn control has the correct number of items</li>
        select case gApplication
        case "BACKGROUND"   :   brc = hToolsCustomizeTestSaveIn( 1 )
        case else           :   brc = hToolsCustomizeTestSaveIn( 2 )
        end select
        if ( not brc ) then
            warnlog( "Incorrect itemcount in listbox, see above" )
        endif
        
        '///+<li>Name the toolbar (required to test the menubutton later)</li>
        printlog( CFN & "Name the new toolbar for further usage" )
        ToolbarName.setText( "tUpdtCustomize" )
        
        '///+<li>Close the dialog with OK</li>
        printlog( CFN & "Close NewToolbar dialog with OK" )
        NewToolbar.ok()
    else
        warnlog( CFN & "Could not access New Toolbar dialog" )
    endif
    
    '///+<li>Access the &quot;Toolbar&quot;-Button and rename the toolbar</li>
    printlog( CFN & "Rename the toolbar via Toolbar-Button" )
    kontext "TabCustomizeToolbars"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    printlog( "Select rename" )
    hMenuSelectNr( 1 )
    
    '///+<li>Cancel the renaming-dialog</li>
    printlog( CFN & "Cancel the dialog" )
    kontext "RenameToolbar"
    if ( RenameToolbar.exists( 2 ) ) then
        call dialogtest( RenameToolbar )
        RenameToolbar.Cancel()
    else
        warnlog( CFN & "Could not access Toolbar renaming dialog" )
    endif
    
    '///+<li>Access the &quot;Toolbar&quot;-Button and delete the toolbar<br>
    '///+Note that there will be no deletion warning</li>
    printlog( CFN & "Delete the toolbar via Toolbar-Button" )
    kontext "TabCustomizeToolbars"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    printlog( "Select delete" )
    hMenuSelectNr( 2 )
       
    '///+<li>Click the &quot;Add Commands...&quot; button</li>
    '///+<li>Check that the dialog is open, close it</li>
    kontext "TabCustomizeToolbars"
    hUpdtToolsCustomizeScriptSelector( 2 )
    
    '///+<li>Check that the SaveIn control has two items</li>
    kontext "TabCustomizeToolbars"
    select case gApplication
    case "BACKGROUND"   :   brc = hToolsCustomizeTestSaveIn( 1 )
    case else           :   brc = hToolsCustomizeTestSaveIn( 2 )
    end select
    if ( not brc ) then
        warnlog( "Incorrect itemcount in listbox, see above" )
    endif
    
    '///+<li>Click &quot;Down&quot;</li>
    printlog( CFN & "Down..." )
    if ( BtnDown.isEnabled() ) then
        BtnDown.click()
    else
        qaerrorlog( "BtnDown is not enabled" )
    endif
    
    '///+<li>Click &quot;Up&quot;</li>
    printlog( CFN & "Up..." )
    if ( BtnUp.isEnabled() ) then
        BtnUp.click()
    else
        qaerrorlog( "BtnUp is not enabled" )
    endif

    kontext "TabCustomizeToolbars"
    '///+<li>Add a new item (click &quot;Add...&quot;)</li>
    printlog( CFN & "Add command" )
    AddCommands.click()
    
    '///+<li>Accept the default item from the script selector (&quot;Add&quot;)</li>
    kontext "ScriptSelector"
    if ( ScriptSelector.exists( 5 ) ) then
    
        call dialogtest( ScriptSelector )
    
        ' This is a workaround that applies when - for some reason - the menu
        ' (menubutton: AddCommands) is not open. In this case the ScriptSelector
        ' will not open. Before this hack the office was left in an unstable state
        ' and the following tests would have failed. Now we get a  warning
        ' instead.
        
        printlog( CFN & "Add the default item" )
        try
            kontext "ScriptSelector"
            ScriptSelector.ok()
        catch
            qaerrorlog( "#i79207# Could not access ok button on ScriptSelector" )
            kontext "ScriptSelector"
            if ( ScriptSelector.exists( 2 ) ) then
                printlog( "Scriptselector is open" )
            endif
            
        endcatch
        
    else
        LibraryTreeList.typeKeys( "<HOME>" )
        for iCurrentItem = 1 to 5
            printlog( CFN & "LibraryTreeList: Moving down..." )
            LibraryTreeList.typeKeys( "<DOWN>" )
            if ( ScriptSelector.isEnabled() ) then
                exit for
            endif
        next iCurrentItem
    endif
    
    ' Note: The Customize dialog in the background is updated immediately. This
    '       means that the cancel-button is active but blocked for a moment
    WaitSlot( 2000 )
    
    '///+<li>Close the Script Selector</li>
    printlog( CFN & "Close the Script Selector, back to ToolsCustomize dialog" )
    ScriptSelector.cancel()
    
    ' make sure the dialog is really closed. At times it just refuses to do so??
    if ( ScriptSelector.exists( 1 ) ) then
        ScriptSelector.close()
    endif
    
    '///+<li>Click the &quot;Modify&quot; button</li>
    '///+<li>Select the first item (Rename)</li>
    kontext "TabCustomizeToolbars"
    printlog( CFN & "Click the Modify-button" )
    brc = hClickCommandButton( 1 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
    	exit function
    endif
    
    '///+<li>Rename the item</li>
    printlog( CFN & "Rename the item" )
    kontext "CustomizeToolbarsRename"
    if ( CustomizeToolbarsRename.exists( 2 ) ) then
        call dialogtest( CustomizeToolbarsRename )
        EingabeFeld.setText( "Renamed item" )
    
        '///+<li>Close the dialog (with OK)</li>
        printlog( CFN & "Close the dialog" )
        CustomizeToolbarsRename.ok()
    else
        warnlog( CFN & "CustomizeToolbarsRename could not be accessed" )
    endif
    
    '///+<li>Click the &quot;Modify&quot; button</li>
    '///+<li>Select the second item (Delete)</li>
    kontext "TabCustomizeToolbars"
    brc = hClickCommandButton( 2 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>Select the first Toolbar and click the &quot;Modify&quot; button</li>
    '///+<li>Select the third item (Restore)</li>
    kontext "TabCustomizeToolbars"
    waitslot
    printlog( CFN & "Click the Modify-button" )
    printlog( "******************** 1 ********************" )
    hDeselectSeparator()
    printlog( "******************** 2 ********************" )
    brc = hClickCommandButton( 3 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>Click the &quot;Modify&quot; button</li>
    '///+<li>Select the fourth item (Begin a group)</li>
    kontext "TabCustomizeToolbars"
    waitslot
    printlog( CFN & "Click the Modify-button" )
    brc = hClickCommandButton( 4 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>Click the &quot;Modify&quot; button</li>
    '///+<li>Select the second item (Delete the group)</li>
    kontext "TabCustomizeToolbars"
    WaitSlot
    printlog( CFN & "Click the Modify-button" )
    brc = hClickCommandButton( 1 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>Click the &quot;Modify&quot; button</li>
    '///+<li>Select the fifth item (Change Icon) to open the Change-Icon dialog</li>
    kontext "TabCustomizeToolbars"
    waitslot
    printlog( CFN & "Click the Modify-button" )
    printlog( "******************** 3 ********************" )
    hDeselectSeparator()
    printlog( "******************** 4 ********************" )
    brc = hClickCommandButton( 5 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    '///+<li>On the Change Icon dialog: Click to import an icon</li>
    printlog( CFN & "On the Change Icon dialog: Click to import an icon" )
    kontext "ChangeIcon"
    if ( ChangeIcon.exists( 2 ) ) then
        call dialogtest( ChangeIcon )
        import.click()
    
            '///+<li>Cancel the FileOpen-dialog</li>
        printlog( CFN & "Cancel the FileOpen-dialog" )
        kontext "OeffnenDlg"
        call dialogtest( OeffnenDlg )
        OeffnenDlg.cancel()
        
        '///+<li>Close the Change Icon dialog</li>
        printlog( CFN & "Cancel the Change Icon dialog" )
        kontext "ChangeIcon"
        ChangeIcon.ok()
    else
        warnlog( CFN & "Could not access Change Icon dialog" )
    endif
    
    '///+<li>Click the &quot;Modify&quot; button</li>
    '///+<li>Select the third item (Restore)</li>
    kontext "TabCustomizeToolbars"
    waitslot
    printlog( CFN & "Click the Modify-button" )
    brc = hClickCommandButton( 3 )
    if ( not brc ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    '///</ul>

end function


'*******************************************************************************

function hUpdtToolsCustomizeEvents()

    '///<h3>Update test for the Tools/Customize - Events-Tab</h3>
    '///<i>Starting point: Tools/Customize dialog</i><br>

    '///<u>Input</u>:<br>
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Returns</u>:<br>
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Description</u>:<br>
    '///<ul>

    const CFN = "hUpdtToolsCustomizeEvents::"
    dim brc as boolean

    printlog( "" )
    printlog( "Events" )
    
    '///+<li>Access the events tabpage</li>
    brc = hToolsCustomizeSelectTab( "events" )
    call DialogTest( TabCustomizeEvents )
    
    '///+<li>Click to assign a macro, close dialog with cancel</li>
    hUpdtToolsCustomizeScriptSelector( 1 )
    
    '///+<li>Check that the SaveIn control has two items</li>
    kontext "TabCustomizeEvents"
    select case gApplication    
    case "BACKGROUND"   :   brc = hToolsCustomizeTestSaveIn( 1 )
    case else           :   brc = hToolsCustomizeTestSaveIn( 2 )
    end select
    if ( not brc ) then
        warnlog( "Incorrect itemcount in listbox, see above" )
    endif
    '///</ul>

end function

'*******************************************************************************

function hUpdtToolsCustomizeKeyboard() as boolean

    '///<h3>Update test for the Tools/Customize - Keyboard-Tab</h3>
    '///<i>Starting point: Tools/Customize dialog</i><br>

    '///<u>Input</u>:<br>
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Returns</u>:<br>
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Description</u>:<br>
    '///<ul>

    const CFN = "hUpdtToolsCustomizeKeyboard::"
    dim brc as boolean
    
    printlog( "" )
    printlog( "Keyboard" )
   
    '///+<li>Access the keyboard tabpage</li>
    brc = hToolsCustomizeSelectTab( "keyboard" )
    if ( not brc and gApplication = "BACKGROUND" ) then
        qaerrorlog( "#i61765# TabTastatur is missing when called from Backing Window" )
        exit function
    endif
    
    call DialogTest( TabTastatur )
  
    '///+<li>Assign an item to *second* Entry in OpenOffice.org/Tastatur</li>
    ' no verification of functionality, this is an update test!
    StarOffice.check()
    hSelectNode( Tastatur , 2 )
    hSelectTopNode( bereich )
    waitslot

    ' Workaround: It might still happen that the currently selected accelerator
    '             is "fixed" so we need to find another one that can be modified.
    do while ( not Aendern.isEnabled() ) 
        printlog( "Moving selection down by one, Control is not enabled" )
        Tastatur.TypeKeys( "<DOWN>" )
    loop 

    Aendern.click()
        
    
    '///+<li>Reset</li>
    Zuruecksetzen.click()
    
    '///+<li>Assign an item to *second* Entry in OpenOffice.org/Tastatur</li>
    hSelectNode( Tastatur , 2 )
    waitslot

    ' Workaround: It might still happen that the currently selected accelerator
    '             is "fixed" so we need to find another one that can be modified.
    do while ( not Aendern.isEnabled() ) 
        printlog( "Moving selection down by one, Control is not enabled" )
        Tastatur.TypeKeys( "<DOWN>" )
    loop 

    Aendern.click()
       
    '///+<li>Delete</li>
    Loeschen.click()
    
    '///+<li>Click Save</li>
    Speichern.click()
    
    '///+<li>Close FileSave dialog</li>
    kontext "SpeichernDlg"
    SpeichernDlg.cancel()
    
    '///+<li>Click Load</li>
    kontext "TabTastatur"
    Laden.click()
    
    '///+<li>Close Fileopen dialog</li>
    kontext "OeffnenDlg"
    OeffnenDlg.cancel()   
    
    '///</ul>

end function

'*******************************************************************************

function hUpdtToolsCustomizeScriptSelector( iBtn as integer ) as boolean

    '///<h3>A brief look at the ScriptSelector via Tools/Customize</h3>
    '///<i>Starting point: Tools/Customize dialog (AddMacro/BtnNew)</i><br>
    '///<i>Uses: t_treelist_tools.inc</i><br>
    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Button identifier (integer)</li>
    '///<ul>
    '///+<li>1: Click TabCustomizeEvents::AssignMacro</li>
    '///+<li>2: Click TabCustomizeToolbars::AddCommands</li>
    '///+<li>3: Click TabMenu::AddCommands</li>
    '///</ul>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus</li>
    '///<ul>
    '///+<li>TRUE: All ok</li>
    '///+<li>FALSE: Any other error</li>
    '///</ul>    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hUpdtToolsCustomizeScriptSelector::"

    dim brc as boolean
        brc = true

    '///+<li>Select the correct button and context to execute</li>
    '///<ol>
    '///+<li>Events</li>
    '///+<li>Toolbars</li>
    '///+<li>Menu</li>
    '///</ol>
    '///+<li>Open the ScriptSelector</li>
    select case iBtn
    case 1 : kontext "TabCustomizeEvents"
             printlog( CFN & "Assign Macro... (1)" )
             AssignMacro.click()
    case 2 : kontext "TabCustomizeToolbars"
             printlog( CFN & "Add... (2)" )
             AddCommands.click()
    case 3 : kontext "TabCustomizeMenu"
             printlog( CFN & "Add... (3)" )
             AddCommands.click()
    case else
             warnlog( CFN & "Invalid function parameter" )
             brc = false
    end select
  
    '///+<li>Test the dialog (DialogTest)</li>
    if ( brc ) then
        kontext "ScriptSelector"
        call DialogTest( ScriptSelector )
        
        '///+<li>Cancel the ScriptSelector</li>
        kontext "ScriptSelector"
        ScriptSelector.cancel()
        
    endif
   
    '///+<li>Reset the context to the originating page</li>
    select case iBtn
    case 1 : kontext "TabCustomizeEvents"
    case 2 : kontext "TabCustomizeToolbars"
    case 3 : kontext "TabCustomizeMenu"
    end select    
    
    hUpdtToolsCustomizeScriptSelector() = brc
    '///</ul>
    
end function

'*******************************************************************************

function hToolsCustomizeTestSaveIn( iItems as integer ) as boolean

    '///<h3>Test number of entries in SaveIn Listbox (Tools/Customize)</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Expected number of items (integer)</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstate (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Correct number of items is present</li>
    '///+<li>FALSE: incorrect number of items is present</li>
    '///</ul>
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hToolsCustomizeTestSaveIn::"
    dim iPresentItems as integer

    '///+<li>Get the number of items from the listbox</li>
    iPresentItems = SaveIn.getItemCount()
    
    '///+<li>Compare the number to the expected itemcount</li>
    if ( iPresentItems <> iItems ) then
        printlog( CFN & "Incorrect itemcount in SaveIn: " )
        printlog( CFN & "Expected: " & iItems )
        printlog( CFN & "Found...: " & iPresentItems )
        hToolsCustomizeTestSaveIn() = false
    else
        printlog( CFN & "Correct itemcount in SaveIn: " & iItems )
        hToolsCustomizeTestSaveIn() = true
    endif
    '///</ul>

end function

'*******************************************************************************

function hOpenMenuButton( oControl as object ) as integer

    ' This function is very evil.
    ' It was written to hopefully workaround the infamous menubutton which is a button 
    ' that - when clicked - opens a menu. This feature is shaky and depends on 
    ' a lot of factors. There is a simple working implemantation in t_menu.inc 
    ' which has the drawback of being absolutely slow, costing the tools_customize
    ' test a lot of time waiting for the control (30% of testtime spent waiting).
    ' This approach tries to be dynamic: Machines that can do it the fast way
    ' use it automatically, machines that are too slow get two retries with the 
    ' slower approach.
    ' The menubutton is one of the last remaining places where none of the
    ' speed optmizations and enhancements apply, so neither WaitSlot() nor 
    ' synchronous slot execution help here.

    dim iClick as integer
    dim bUseSlowMethod as boolean : bUseSlowMethod = false
    
    const CFN = "hOpenMenuButton: "
    
    hOpenMenuButton() = -1 ' this is the general failure returnvalue
    
    for iClick = 1 to 3
    
        if ( bUseSlowMethod ) then 
            wait( 1000 )
            oControl.OpenMenu()
            wait( 3000 )
        else
            oControl.click()
        endif
            
        try
            hOpenMenuButton() = MenuGetItemCount
            printlog( CFN & "Success on " & iClick & ". attempt" )
            exit function
        catch
            qaerrorlog( CFN & "#i96753 - Failed to retrieve itemcount from Menu-/Command-button" )
            bUseSlowMethod = true
        endcatch
        
    next iClick
    
end function

'*******************************************************************************

function hClickCommandButton( iItemToClick as integer ) as boolean

    const CFN = "hClickCommandButton::"
    printlog( CFN & "Enter" )
    
    dim brc as boolean 'a multi purpose boolean returnvalue
    dim iMenuItems as integer

    iMenuItems = hOpenMenuButton( Command )
    
    ' exit on error
    if ( iMenuItems < 0 ) then
        warnlog( CFN & "Menu apparently not open, giving up." )
    	hClickCommandButton() = false
    	exit function
    endif        
    
    hMenuSelectNr( iItemToClick ) 
    wait( 500 )
    ' check for the rename dialog (menu)
    kontext "CustomizeMenuReName"
    if ( CustomizeMenuReName.exists() ) then
    	printlog( CFN & "Opened dialog: Rename Menu" )
    	hClickCommandButton() = true
    	exit function
    endif
    
    ' check for the rename dialog (toolbar)    
    kontext "CustomizeToolbarsRename"
    if ( CustomizeToolbarsRename.exists() ) then
    	printlog( CFN & "Opened dialog: Rename Toolbar" )
    	hClickCommandButton() = true
    	exit function
    endif

    printlog( CFN & "Exit" )
    hClickCommandButton() = true

end function
