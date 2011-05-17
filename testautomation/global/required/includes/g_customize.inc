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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description : update and resouce test for tools/customize
'*
'************************************************************************
'*
' #1  tToolsCustomize
'*
'\***********************************************************************

testcase tToolsCustomize

    Dim iCountMsgBox as integer
    '/// open application ///'
    Call hNewDocument
    sleep 2
    '/// Tools->Customize... ///'
    ToolsCustomize
    sleep 3
    Kontext
    '/// switch to tabpage 'Menus' ///'
    try
        Active.SetPage TabCustomizeMenu             ' 1 ------------------
    catch
        warnlog "couldn't switch to tabpage 'Menus'"
    endcatch
    Kontext "TabCustomizeMenu"
    if TabCustomizeMenu.exists(5) then
        Call DialogTest ( TabCustomizeMenu )
        '/// Go in 'Menu' list-box one item down
        Menu.typeKeys("<down>")
        '/// Go in 'Entries' list-box one item down 
        Entries.typeKeys("<down>")
        sleep(2)
        '/// click button 'new' ///'
        BtnNew.Click
        sleep(1)
        '/// 'New Menu' dialog comes up ///'
        Kontext "MenuOrganiser"
        Call DialogTest ( MenuOrganiser )
        '/// Cancel dialog 'New Menu' ///'
        MenuOrganiser.cancel
        sleep(2)

        Kontext "TabCustomizeMenu"
        '/// In 'Menu' list-box select one item up
        Menu.Select(1)
        sleep(1)
        '/// Click on button 'Menu' ///'        
        MenuBtn.openMenu
        printlog "opened first one"
        sleep(1)
        '/// Select the first entry 'Move' ///'
        hMenuSelectNr(1)
        '/// Dialog 'Move Menu' comes up ///'
        Kontext "MenuOrganiser"
        Call DialogTest ( MenuOrganiser )
        '/// Click on the 'Up'-button ///'
        ButtonUp.click
        '/// Click on the 'Down'-button ///'
        ButtonDown.click
        '/// Cancel Dialog 'Move Menu' ///'
        MenuOrganiser.cancel
        
        Kontext "TabCustomizeMenu"
        '/// Click Button 'Add Commands' ///'
        AddCommands.click
        '/// Dialog 'Add Commands' comes up ///'
        Kontext "ScriptSelector"
        call DialogTest(ScriptSelector)
        LibraryTreeList.typeKeys ("<down>")
        ScriptList.typeKeys ("<down>")
        '/// Close Dialog 'Add Commands' ///'
        ScriptSelector.cancel
        
        Kontext "TabCustomizeMenu"
        '/// Click button 'Command' ///'
        sleep(1)
        Command.click
        printlog "opened second one"
        WaitSlot()
        '/// Select the first entry 'Add Submenu' ///'
        hMenuSelectNr(1)
        '/// Dialog 'Name' comes up ///'
        Kontext "CustomizeMenuName"
        call DialogTest (CustomizeMenuName)
        Eingabefeld.typeKeys ("A")
        '/// Cancel Dialog 'Name' ///'
        CustomizeMenuName.cancel

        WaitSlot()
        Kontext "TabCustomizeMenu"
        '/// Click button 'Command' ///'
		sleep(1)
        Command.click
        printlog "opened third one"
        WaitSlot()
        '/// Select the third entry 'Rename' ///'
        hMenuSelectNr(3)
        '/// Dialog 'Name' comes up ///'
        Kontext "CustomizeMenuReName"
        call DialogTest (CustomizeMenuReName)
        Eingabefeld.typeKeys ("A")
        '/// Cancel Dialog 'Name' ///'
        CustomizeMenuReName.cancel

        Kontext "TabCustomizeMenu"
        '/// click button 'downwards' ///'
        BtnDown.Click
        '/// click button 'upwards' ///'
        BtnUp.Click
        '/// click button 'Reset' ///'
        ResetBtn.Click
        kontext
        printlog "survived all menu comands"
        if Messagebox.exists (5) then
            if Messagebox.GetRT = 304 then
                printlog "Messagebox: Menu configuration will be reset...: '"+Messagebox.getText+"'"
                call DialogTest(Messagebox, 50)
                Messagebox.yes
            endif
        endif
    endif
    
        Kontext
        '/// switch to tabpage 'Toolbars' ///'
    try
        Messagebox.SetPage TabCustomizeToolbars    ' 4 ------------------
    catch
        warnlog "couldn't switch to tabpage 'Toolbars'"
    endcatch
    Kontext "TabCustomizeToolbars"
    if TabCustomizeToolbars.exists(5) then
        Call DialogTest ( TabCustomizeToolbars )
        Menu.typeKeys("<down>")
        ToolbarContents.typeKeys("<down>")
        SaveIn.typeKeys("<down>")
        '/// click button 'new' ///'
        BtnNew.Click
        waitslot
        '/// Dialog 'Name' comes up ///'
        Kontext "NewToolbar"
        Call DialogTest ( NewToolbar )
        ToolbarName.typeKeys("<down>")
        SaveIn.typeKeys("<down>")
        '/// Cancel dialog 'Name' ///'
        NewToolbar.cancel
        waitslot

        Kontext "TabCustomizeToolbars"
        '/// Click Button 'Add Commands...' ///'
        SaveIn.typeKeys("<up>")
        AddCommands.click
        '/// Dialog 'Add Commands' comes up ///'
        Kontext "ScriptSelector"
        call DialogTest (ScriptSelector)
        LibraryTreeList.typeKeys("<down>")
        ScriptList.typeKeys("<down>")
        '/// Close Dialog 'Add Commands' ///'
        ScriptSelector.cancel
        
        waitslot
        Kontext "TabCustomizeToolbars"
        sleep(1)
        '/// Click button 'Command' ///'
        Command.click
        waitslot
        '/// Select the firste entry 'Rename' ///'
        hMenuSelectNr(1)
        '/// Dialog 'Name' comes up ///'
        Kontext "CustomizeToolbarsReName"
        if NOT CustomizeToolbarsReName.exists(2) then
        Kontext "TabCustomizeToolbars"
        ToolbarContents.typeKeys("<down>")
        else
        CustomizeToolbarsReName.cancel()
        endif        

        Kontext "TabCustomizeToolbars"
        sleep(1)
        Command.click
        waitslot
        hMenuSelectNr(1)
        Kontext "CustomizeToolbarsReName"
        call DialogTest (CustomizeToolbarsReName)
        Eingabefeld.typeKeys ("A")
        '/// Cancel Dialog 'Name' ///'
        CustomizeToolbarsReName.close
        
        waitslot
        Kontext "TabCustomizeToolbars"
        '/// Click button 'Command' ///'
        sleep(1)
        Command.click
        waitslot
        '/// Select the fifth entry 'Change Icon' ///'
        hMenuSelectNr(5)
        '/// Dialog 'Change Icon' comes up ///'
        Kontext "ChangeIcon"
        call DialogTest (ChangeIcon)
        '/// Click button 'Import...' on dialog Change 'Icon' ///'
        Import.click
        '/// Dialog 'Open' comes up ///'
        Kontext "GeneralFileDialog"
        '/// Cancel dialog 'Open' ///'
        GeneralFileDialog.cancel
        '/// Cancel Dialog 'Change Icon' ///'
        Kontext "ChangeIcon"
        ChangeIcon.cancel
        
        Kontext "TabCustomizeToolbars"
        '/// click button 'downwards' ///'
        BtnDown.Click
        '/// click button 'upwards' ///'
        BtnUp.Click
        '/// click button 'Reset' ///'
        ResetBtn.Click
        kontext
        if Messagebox.exists (5) then
            if Messagebox.GetRT = 304 then
                printlog "Messagebox: Menu configuration will be reset...: '"+Messagebox.getText+"'"
                call DialogTest(Messagebox, 50)
                Messagebox.no
            endif
        endif

        WaitSlot()
        Kontext "TabCustomizeToolbars"
        '/// On Tab Page 'Customize Menus' click on Button 'Toolbar' ///'
        sleep(1)
        MenuBtn.openMenu
        WaitSlot()
        hMenuSelectNr(1)
        kontext
        sleep 3
        if Messagebox.exists (5) then
            if Messagebox.GetRT = 304 then
                printlog "Messagebox: Toolbar configuration will be reset...: '"+Messagebox.getText+"'"
                call DialogTest(Messagebox, 50)
                Messagebox.no
            endif
        endif
    endif    
        Kontext
        '/// switch to tabpage 'Events' ///'
    try
        Messagebox.SetPage TabCustomizeEvents      ' 5 ------------------
    catch
        warnlog "couldn't switch to tabpage 'Events'"
    endcatch
    Kontext "TabCustomizeEvents"
    if TabCustomizeEvents.exists(5) then
        Call DialogTest ( TabCustomizeEvents )
        SaveIn.typeKeys("<down>")
        EventListbox.typeKeys("<down>")
        WaitSlot()
        '/// click button 'Assign Macro' ///'
        AssignMacro.Click
        WaitSlot()
        ' If Java is not [correctly] installed a messagebox will be shown
        Kontext "Messagebox"
        if Messagebox.Exists(2) then            
            for iCountMsgBox = 1 to 20
                Kontext "Messagebox"
                if MessageBox.Exists(2) then
                    if MessageBox.GetRT = 304 then
                        if iCountMsgBox = 1 then
                            warnlog "Java runtime is defective. Detection failed!"
                        endif
                        '.getRT is need because otherwise the operation will be done
                        'on the opened tab control behind the message box
                        QAErrorLog "Dialog (" & iCountMsgBox & "): " & Messagebox.GetText
                        if iCountMsgBox > 15 then
                            warnlog "Now the message box occours " & iCountMsgBox & " instead of 15 times!"
                        endif
                        Messagebox.OK
                    endif
                else
                    exit for
                endif
            next iCountMsgBox
        endif        
        '/// Dialog 'MacroSelector' comes up ///'
        Kontext "ScriptSelector"
        call DialogTest (ScriptSelector)
        LibraryTreeList.typeKeys("<down>")
        ScriptList.typeKeys("<down>")
        '/// Cancel Dialog 'MacroSelector' ///'
        ScriptSelector.cancel
        '/// click button 'Remove Macro' ///'
        Kontext "TabCustomizeEvents"
        RemoveMacro.Click
        WaitSlot()
        '/// Press Button 'Reset' ///'
        ResetBtn.click
        '/// cancel dialog 'Customize' ///'
        Kontext "TabCustomizeEvents"
        TabCustomizeEvents.cancel
    endif
    
    '/// close application ///'
    Call hCloseDocument
endcase

