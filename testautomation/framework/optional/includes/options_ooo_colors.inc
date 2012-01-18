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
'* short description : Tools->Options: OpenOffice.org Colors
'*
'\******************************************************************************

testcase tOOoColors

    printlog( "OOo color palette" )

    dim myColor(4)      as String
    dim oldColor(4)     as String
    dim currentColor(4) as String
    dim chColor(4)      as String
    
    const DEFAULT_COLOR_COUNT = 100
    const CHANGED_COLOR_COUNT = DEFAULT_COLOR_COUNT +1
    const USER_COLOR = "TT-Test-Color"
    
    myColor(1) = USER_COLOR
    myColor(2) = "255"
    myColor(3) = "245"
    myColor(4) = "200"

    printlog( "Check if all settings are saved in configuration ( StarOffice / Colors )" )
    printlog( "Open Tools / Options / StarOffice / Colors" )
    ToolsOptions
    
    Kontext "ExtrasOptionenDlg"
    if ( ExtrasOptionenDlg.exists( 1 ) ) then
    
        hToolsOptions ( "StarOffice", "Colors" )
        
        printlog( "Delete the userdefined color if it exists" )
        if ( Farbe.GetItemCount() <> DEFAULT_COLOR_COUNT + 1 ) then
            call DeleteColor( USER_COLOR )
        endif
    
        printlog( "Check the number of currently registered colors - cancel test on error" )
        Kontext "TabFarben"
        if ( Farbe.GetItemCount() <> DEFAULT_COLOR_COUNT ) then
            warnlog "The number of colors has changed, stopping test; Expected: " & DEFAULT_COLOR_COUNT & "; Found...: " & Farbe.getItemCount() 
            printlog "Close the Tools/Options dialog with OK" 
            Kontext "ExtrasOptionenDlg"
            ExtrasOptionenDlg.OK
            WaitSlot( 2000 )
            goto endsub
        else
            printlog( "Number of colors is ok: " & DEFAULT_COLOR_COUNT )
        endif        
        
        printlog( "Jump to the last color in the list." )
        Kontext "TabFarben"
        Farbe.Select( Farbe.GetItemCount() )
        
        printlog( "Get the settings for the current selected color" )
        call getColorRGB(oldColor()) '(204,204,255)

        printlog( "Change settings for last color and press 'modify'" )
        call ModifyColorRGB_PGUP( DEFAULT_COLOR_COUNT ) '(255,255,255)
        
        printlog( "Get the changed colors (Sun 4,255,255,255)" )
        call GetColorRGB(chColor())
        
        printlog( "Change the settings again and save it as TT-Test (adding to the list)" )
        call CreateNewColor(myColor()) '(255,245,200)
        
        printlog( "Check the number of currently registered colors" )
        Kontext "TabFarben"
        if ( Farbe.getItemCount() <> CHANGED_COLOR_COUNT ) then
            warnlog( "The number of colors has changed" )
            printlog( "Expected: " & CHANGED_COLOR_COUNT )
            printlog( "Found...: " & Farbe.getItemCount() )
        else
            printlog( "Number of colors is ok: " & DEFAULT_COLOR_COUNT )
        endif        

        printlog( "Close the Tools/Options dialog with OK" )
        Kontext "ExtrasOptionenDlg"
        hCloseDialog( ExtrasOptionenDlg, "ok" )
    else
        warnlog( "Tools/Options dialog not open" )
    endif
    
    printlog( "Exit and restart the office" )
    ExitRestartTheOffice
    
    printlog( "Check the changes" )
    ToolsOptions
    
    Kontext "ExtrasOptionenDlg"
    if ( ExtrasOptionenDlg.exists( 1 ) ) then
    
        hToolsOptions ( "StarOffice", "Colors" )
        
        printlog( "Check the number of colors (Plus one color)" )
        Kontext "TabFarben"
        if ( Farbe.getItemCount() <> CHANGED_COLOR_COUNT ) then
            warnlog( "The number of colors has changed" )
            printlog( "Expected: " & CHANGED_COLOR_COUNT )
            printlog( "Found...: " & Farbe.getItemCount() )
        else
            printlog( "Number of colors is ok: " & DEFAULT_COLOR_COUNT )
        endif        
        
        printlog( "Select the changed color and check the changes" )
        Kontext "TabFarben"   
        Farbe.Select( DEFAULT_COLOR_COUNT )
        call compareTwoColorsRGB(chColor())
        
        printlog( "Select the new color ( TT-Test ) and check the name and the settings" )
        Kontext "TabFarben"
        Farbe.Select( CHANGED_COLOR_COUNT )
        call CompareTwoColorsRGB(myColor())
    
        printlog( "Delete the new userdefined color" )
        call deleteColor( USER_COLOR )

        printlog( "Check the number of colors (One removed)" )
        Kontext "TabFarben"
        if ( Farbe.getItemCount() <> DEFAULT_COLOR_COUNT ) then
            warnlog( "The number of colors has changed" )
            printlog( "Expected: " & DEFAULT_COLOR_COUNT )
            printlog( "Found...: " & Farbe.getItemCount() )
        else
            printlog( "Number of colors is ok: " & DEFAULT_COLOR_COUNT )
        endif        
        
        printlog( "reset the modified color to default" )
        Kontext "TabFarben"
        Farbe.Select( DEFAULT_COLOR_COUNT )
        call ModifyColorRGB(oldColor())
        
        printlog( "Close options dialog with OK" )
        Kontext "ExtrasOptionenDlg"
        hCloseDialog( ExtrasOptionenDlg, "ok" )
    else
        warnlog( "Tools/Options dialog not open" )
    endif

    printlog( "Reopen options dialog and check the reset" )
    ToolsOptions
    
    Kontext "ExtrasOptionenDlg"
    if ( ExtrasOptionenDlg.exists( 1 ) ) then
    
        hToolsOptions ( "StarOffice", "Colors" )
        
        printlog( "Check if the number of colors in the list has been restored" )
        Kontext "TabFarben"
        if ( Farbe.getItemCount() <> DEFAULT_COLOR_COUNT ) then
            warnlog( "The number of colors has changed" )
            printlog( "Expected: " & DEFAULT_COLOR_COUNT )
            printlog( "Found...: " & Farbe.getItemCount() )
        else
            printlog( "Number of colors is ok: " & DEFAULT_COLOR_COUNT )
        endif        
 
        
        printlog( "Check if the last color has been reset to defaults." )
        Kontext "TabFarben"
        Farbe.Select( DEFAULT_COLOR_COUNT )
        call compareTwoColorsRGB(oldColor())

        printlog( "Close options dialog with OK" )
        Kontext "ExtrasOptionenDlg"
        hCloseDialog( ExtrasOptionenDlg, "ok" )
    else
        warnlog( "Tools/Options dialog not open" )
    endif
    
endcase
