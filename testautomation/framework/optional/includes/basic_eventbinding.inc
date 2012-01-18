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
'* short description : Function test of Basic IDE Assign Componenet
'*
'\************************************************************************


testcase tAssignComponent
    dim j,b,i,a,x,y as integer
    dim sTemp as string
    dim sTemp2 as string
    dim sDialogName as string
  
    ' make sure we start from backingwindow
    b = getDocumentCount
    for j = 1 to b
        printlog "closing: " + j
        hCloseDocument()
    next j
    ' make sure we only have one document window
    hNewDocument()
    sleep (5)
    
    ToolsMacrosOrganizeDialogs
    Kontext "TabDialogs"
    if TabDialogs.exists(5) then
        ModuleList.typeKeys("<home>")
        sTemp = ModuleList.getSelText
        sTemp2 = ""
        while (NOT newDialog.isEnabled) AND (sTemp <> sTemp2)
            ModuleList.typeKeys("<down>")
            sTemp2 = sTemp
            sTemp = ModuleList.getSelText
        wend
        if sTemp = sTemp2 then
            warnlog "Couldn't find a way to enable the 'New...' button - aborting"
            goto endsub
        endif
        printlog ModuleList.getSelText
        newDialog.click
        kontext "NeuerDialog"
        if NeuerDialog.exists(5) then
            ' save dialog name, for deleting it later
            sDialogName = Dialogname.getText
            printlog sDialogName
            NeuerDialog.OK
            Kontext "TabDialogs"
            if TabDialogs.exists(5) then
                printlog ModuleList.getSelText
                edit.click
                Kontext "BasicIDE"
                if DialogWindow.exists(5) then ' you can't use exists on BasicIDE; check for a control
                    DialogWindow.TypeKeys("tab")
                    DialogWindow.TypeKeys("tab",true)
                    try
                        ContextProperties
                    catch
                        qaErrorLog "#i70901# .typeKeys doesn't work in DialogWindow"
                        ' works better, but also not always
                        DialogWindow.mouseDown( 10 , 10 )
                        DialogWindow.mouseMove( 90 , 90 )
                        DialogWindow.mouseUp( 90 , 90 )
                        try
                            ContextProperties
                        catch
                            qaErrorLog "Even Workaround doesn't work! - exiting"
                            goto endsub
                        endcatch
                    endcatch
                    kontext "ControlPropertiesDialog"
                    'printlog TabEventsControl.exists(5)
                    if NOT ControlPropertiesDialog.exists(5) then
                        qaErrorLog "#i70901# can't use exists on tabpage parent"
                    endif
                    'printlog TabGeneralControl.exists(5)
                    ' doesn't work...
'                    if ControlPropertiesDialog.exists(5) then
                    kontext "TabGeneralControl"
                    if TabGeneralControl.exists(5) then
                        kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabcontrol.setPage (TabEventsControl)
                        Kontext "TabEventsControl"
                        if TabEventsControl.exists(5) then
                            PBKeyTyped.click
                            Kontext "AssignMacro"
                            if AssignMacro.exists(5) then
                                Component.click
                                Kontext "AssingComponent"
                                if AssingComponent.exists(5) then
                                    DialogTest(AssingComponent)
                                    ComponentMethodName.setText ("vnd.sun.star.script:Library1.Module1.TestDialogComponent?language=Basic&location=document")
                                    AssingComponent.OK
                                    Kontext "AssignMacro"
                                    if AssignMacro.exists(5) then
                                        ' doesn't work, because featureset is not implemted on component EditListBox... expected for xmas 2006
                                        'printlog eventListBox.getItemText
                                        AssignMacro.OK
                                        Kontext "TabEventsControl"
                                        printlog evtKeyTyped.getText
                                        Kontext "ControlPropertiesDialog"
                                        ContextProperties
                                    endif
                                else
                                    warnlog "couldn't open dialog 'Assign Component'"
                                endif
                            else
                                warnlog "couldn't open dialog 'Assign Macro'"
                            endif
                        else
                            warnlog "couldn't switch to tab page 'Events''"
                        endif
                    else
                        warnlog "couldn't open flyer 'Properties: Dialog'"
                    endif
                else
                    warnlog "couldn't open Basic IDE Dialog editor"
                endif
            else
                warnlog "couldn't open dialog 'New Dialog'"
            endif
        else
            warnlog "couldn't open dialog 'New Dialog'"
        endif
    else
        warnlog "couldn't open dialog 'OOo Basic Macro Organizer'"
    endif
    
    'delete dialog!
    ToolsMacrosOrganizeDialogs
    Kontext "TabDialogs"
    if TabDialogs.exists(5) then
        ModuleList.typeKeys("<home>")
        sTemp = ModuleList.getSelText
        sTemp2 = ""
        while (sTemp <> sDialogName) AND (sTemp <> sTemp2)
            ModuleList.typeKeys("<down>")
            sTemp2 = sTemp
            sTemp = ModuleList.getSelText
        wend
        if sTemp = sTemp2 then
            warnlog "Couldn't find the created dialog - aborting"
            goto endsub
        endif
        printlog ModuleList.getSelText
        delete.click
        kontext
        if active.exists(5) then
            printlog active.getText
            active.yes
        else
            warnlog "couldn't delete dialog '"+sDialogName+"'"
        endif
        Kontext "TabDialogs"
        TabDialogs.cancel
    else
        warnlog "couldn't open dialog 'OOo Basic Macro Organizer'"
    endif
        
'Sub Main
'	oSimpleComponent = CreateUnoService( "my_module.MyService1" )
'	oActiveFrame = StarDesktop.getActiveFrame()
'	oSimpleComponent.createDialog("vnd.sun.star.script:Standard.Dialog1?location=application", oActiveFrame )
'End Sub
    hCloseDocument()
    hCloseDocument()
endcase

testcase tAssignComponentFunction
    dim x,i,a as integer
    dim sTemp as string
    dim sPath as string
        sPath = gTesttoolPath & "framework\optional\input/eventbinding/"
    dim sExtension as string
        sExtension = "DialogComponent.oxt"
    
    sExtensionCLI("list","")
    'sExtensionCLI("add","DialogComponent.oxt",convertPath(gTestToolPath + "framework\optional\input/eventbinding/") )
    hExtensionAddGUI( sPath & sExtension, "" )
    hFileOpen(convertPath(gTestToolPath + "framework\optional\input/eventbinding/DialogComponent.odt"))
    kontext "SecurityWarning"
    if SecurityWarning.exists(5) then
        SecurityWarning.ok
    endif
    ToolsMacrosRunMacro
    Kontext "ScriptSelector"
    if ScriptSelector.exists(10) then
        x = LibraryTreeList.getItemCount
        'There have to show up 3 libraries, else the document isn't loaded
        if x <> 3 then
            printlog "Try again..."
            ScriptSelector.cancel
            sleep 10
            ToolsMacrosRunMacro
            Kontext "ScriptSelector"
            x = LibraryTreeList.getItemCount
        endif
        'Look for library 'DialogComponent' and 'DIALOGCOMPONENT'
        for i = 1 to x
            sTemp = LibraryTreeList.getItemText(i)
            'printlog "("+i+"/"+x+"): '" + sTemp + "'"
            if (instr(stemp,"DialogComponent.odt") OR instr(stemp,"DIALOGCOMPONENT.ODT")) then
                a = i
                printlog "Found library ("+i+"/"+x+"): '" + sTemp + "'"
            endif
        next i
        if 0 <> a then
            'Select the library and unfold it
            LibraryTreeList.select(a)
            LibraryTreeList.typeKeys("+")
            'Go to 'Standard' and unfold
            LibraryTreeList.typeKeys("<down>")
            LibraryTreeList.typeKeys("+")
            'Go to 'Global'
            LibraryTreeList.typeKeys("<down>")

            'Select macro name 'TestDialogComponent'
            x = ScriptList.getItemCount
            for i = 1 to x
                sTemp = ScriptList.getItemText(i)
                'printlog "("+i+"/"+x+"): '" + sTemp + "'"
                if "TestDialogComponent" = sTemp then
                    a = i
                    printlog "Found macro ("+i+"/"+x+"): '" + sTemp + "'"
                endif
            next i
            if (0 <> a) then
                'Select the macro
                ScriptList.select(a)
                'Start the macro by pressing the button 'run'
                qaErrorLog ("#i68892# The test can't go on, because no help ids are useable in BASIC right now.")
                ScriptSelector.cancel
                'ScriptSelector.ok
        '       sleep... at least 60 seconds
                call sleep(10)
            else
                warnlog "Macro not found: 'TestDialogComponent'"
            endif
        else
            warnlog "Library not found: 'DialogComponent' or 'DIALOGCOMPONENT'"
        endif
    else
        warnlog "'Run macro dialog' didn't came up"
    endif
    'printlog resetapplication
    
   ' vnd.sun.star.UNO:doit1
   ' vnd.sun.star.UNO:doit2
   ' vnd.sun.star.UNO:doit3
   ' vnd.sun.star.UNO:handleEventWithArguments
   ' vnd.sun.star.UNO:handleEvent
   ' vnd.sun.star.UNO:copyText
    
    sExtensionCLI("remove","DialogComponent.oxt" )
    'hExtension RemoveGUI( sExtension )
    hCloseDocument()
endcase
