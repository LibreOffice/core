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

*****

sub checkPage( sFile as string , bDisabled as boolean, optional iMiddleMouseButtonControl as integer)
    dim iMiddleMouseButton as integer
    dim iMiddleMouseButtonExtern as integer
    dim iTemp as integer

    ' depends on if defaults or chnges are checked;
    ' changes : 1
    ' defaults: 0
    if isMissing(iMiddleMouseButtonControl) then
        iMiddleMouseButtonExtern = 0
    else
        iMiddleMouseButtonExtern = iMiddleMouseButtonControl
    endif
    sFile = convertpath( sFile )

    printlog( " * Testing current settings against a reference list." )
    printlog( " * Using settings from file: " & sFile )

    ToolsOptions
    call hToolsOptions( "StarOffice" , "View" )

    ' Antialiasing settings only exist for Linux and Solaris
    if ( gPlatgroup() <> "w95" ) then
        if ( bDisabled ) then
            checkCheckBox( sFile , "*" , FontAntiAliasing)
            checkEntryField( sFile , "state_aapixel" , AAPixel)
        else
            checkEntryField( sFile , "*" , AAPixel)
            checkCheckBox( sFile , "*" , FontAntiAliasing)
        endif
    else
        if ( FontAntiAliasing.exists() ) then
            warnlog( "Antialiasing controls not expected on Windows" )
        endif
        if ( AAPixel.exists() ) then
            warnlog( "Antialiasing controls not expected on Windows" )
        endif
    endif

    checkEntryField( sFile , "*" , FontScale)
    checkListBox( sFile , "*" , IconScale )
    checkListBox( sFile , "*" , IconStyle )
    checkListBox( sFile , "*" , IconsInMenueAnzeigen )
    checkListBox( sFile , "*" , IconStyle )
    checkCheckBox( sFile , "*" , VorschauInSchriftlisten )
    checkCheckBox( sFile , "*" , SchriftenHistorie )

    if ( UseHardwareAcceleration.isEnabled ) then
        checkCheckBox( sFile , "*" , UseHardwareAcceleration )
    else
        printlog( "Hardware acceleration is not available on this system" )
    endif
    
    if ( UseAntiAliasing.isEnabled() ) then
        checkCheckBox( sFile , "*" , UseAntiAliasing )
    else
        printlog( "Antialiasing is not available on this system" )
    endif
    
    checkListBox( sFile , "*" , MousePositioning)
    ' needs to get handled differently on platforms!
    iMiddleMouseButton = MausAktion.getSelIndex
    if iMiddleMouseButtonExtern = 1 then
        checkListBox( sFile , "*" , MausAktion)
    else
        if gPlatGroup = "unx" then
            iTemp = 3 ' Paste clipboard
        else
            iTemp = 2 ' Automatic scrolling
        endif
        if Itemp = iMiddleMouseButton then
            printlog "     * Middle mouse button: ok"
        else
            warnlog "Middle mouse button: expected: '" + iTemp + "' '" _
                    + MausAktion.getItemText(iTemp) + "'; is: '" _
                    + iMiddleMouseButton + "' '" _
                    + MausAktion.getItemText(iMiddleMouseButton) + "'"
        endif
    endif
    
    if ( Transparency.isEnabled() ) then
        if ( Transparency.isChecked() ) then
            checkCheckBox( sFile , "*" , transparency )
            checkEntryField( sFile , "*", selectionopacity )
        else
            Transparency.check()
            checkEntryField( sFile , "*", selectionopacity, "i104150" )
            Transparency.unCheck()
        endif
    else
        printlog( "Transparency is not available on this system" )
    endif

    Kontext "OptionenDLG"
    OptionenDLG.OK
    WaitSlot()
    call exitRestartTheOffice
end sub

'*******************************************************************************

sub changePage( sFile as string , bInverted as boolean, optional iMiddleMouseButtonControl as integer )
    dim iMiddleMouseButtonExtern as integer

    ' depends on if defaults or chnges are checked;
    ' changes : 1
    ' defaults: 0
    if isMissing(iMiddleMouseButtonControl) then
        iMiddleMouseButtonExtern = 0
    else
        iMiddleMouseButtonExtern = iMiddleMouseButtonControl
    endif

    sFile = convertpath( sFile )
    printlog( " * Changing current settings according to the reference list." )
    printlog( " * Using settings from file: " & sFile )

    ToolsOptions
    call hToolsOptions( "StarOffice" , "View" )
    sleep( 2 )

    ' Antialiasing settings only exist for Linux and Solaris
    if ( gPlatGroup <> "w95" ) then
        if ( bInverted ) then
            setEntryField( sFile , "*" , AAPixel )
            setCheckBox( sFile , "*" , FontAntiAliasing )
        else
            setCheckBox( sFile , "*" , FontAntiAliasing )
            setEntryField( sFile , "*" , AAPixel )
        endif
    endif

    setEntryField( sFile , "*" , FontScale )
    setListBox( sFile , "*" , IconScale )
    setListBox( sFile , "*" , IconStyle )
    setListBox( sFile , "*" , IconsInMenueAnzeigen )
    setCheckBox( sFile , "*" , VorschauInSchriftlisten )
    setCheckBox( sFile , "*" , SchriftenHistorie )

    if ( UseHardwareAcceleration.isEnabled() ) then
        setCheckBox( sFile , "*" , UseHardwareAcceleration )
    else
        printlog( "Harware acceleration is not available on this system" )
    endif
    
    if ( UseAntiAliasing.isEnabled() ) then
        setCheckBox( sFile , "*" , useantialiasing )
    else
        printlog( "Antialiasing is not available on this system" )
    endif
    
    setListBox( sFile , "*" , MousePositioning )
    ' needs to get handled differently on platforms!
    if iMiddleMouseButtonExtern = 1 then
        setListBox( sFile , "*" , MausAktion )
    else
        if gPlatGroup = "unx" then
            MausAktion.select(3) ' Paste clipboard
        else
            MausAktion.select(2) ' Automatic scrolling
        endif
    endif
    
    if ( Transparency.isEnabled() ) then
        if ( Transparency.isChecked() ) then
            setEntryField( sFile , "*", selectionopacity )
            setCheckBox( sFile , "*" , transparency )
        else
            setCheckBox( sFile , "*" , transparency )
            setEntryField( sFile , "*", selectionopacity )
        endif        
    else
        printlog( "Transparency is not available on this system" )
    endif

    Kontext "OptionenDLG"
    OptionenDLG.OK
    WaitSlot()
    call exitRestartTheOffice
end sub


