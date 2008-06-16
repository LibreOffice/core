'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: options_ooo_colors.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Tools->Options: OpenOffice.org Colors
'*
'\******************************************************************************


testcase tOOoColors
    dim USERCOLOR as String
    dim myColor(4)      as String
    dim oldColor(4)     as String
    dim currentColor(4) as String
    dim chColor(4)      as String
    dim iDefaultColorCount  as Integer
    dim iModifiedColorCount as Integer
    dim iTemp as integer
    dim i as integer
    
    USERCOLOR = "TT-Test" + iSprache
    myColor(1) = USERCOLOR
    myColor(2) = "255"
    myColor(3) = "245"
    myColor(4) = "200"
    
    printlog " testing Tools/Options -> colors"
    '///check if all settings are saved in configuration ( StarOffice / Colors )
    '///open Tools / Options / StarOffice / Colors
    printlog " - Save current settings and modify/add some colors"
    ToolsOptions
        hToolsOptions ( "StarOffice", "Colors" )
    
        '///determine the number of currently registered colors
        iDefaultColorCount = Farbe.GetItemCount()
        for i = 1 to iDefaultColorCount 
            printlog "("+i+"/"+iDefaultColorCount +"): '"+Farbe.getItemText(i)+"'"
        next i
        printlog " - Currently colors are listed: '" + iDefaultColorCount + "'"
        '///delete the userdefined color if it exists
        call DeleteColor( USERCOLOR )
        '///determine the number of currently registered colors
        Kontext "TabFarben"
        iDefaultColorCount = Farbe.GetItemCount
        printlog " - Colors after deleting: '" + iDefaultColorCount + "'"
        '///select the last color in the list
        printlog " - jump to the last color in the list."
        Kontext "TabFarben"
        Farbe.Select( iDefaultColorCount )
        '///get the settings for the current selected color 
        '(204,204,255)
        call getColorRGB(oldColor())
        '///change settings for last color and press 'modify' 
        '(255,255,255)
        call ModifyColorRGB_PGUP(iDefaultColorCount)
        '///get the changed colors (Sun 4,255,255,255)
        call GetColorRGB(chColor())
        '///change the settings again and save it as TT-Test (adding to the list)
        '(255,245,200)
        call CreateNewColor(myColor())
        '///get the current number of registered colors (should be 92)
        Kontext "TabFarben"
        iModifiedColorCount = Farbe.getItemCount
        printlog " - Currently colors are listed: '" + iModifiedColorCount + "'" 
        '///close the options dialog with OK
        printlog " - close extras/options"
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
    Sleep(3)
    
    '///exit and restart the office
    printlog " - exit/restart StarOffice"
    ExitRestartTheOffice
    '///check the changes
    printlog " - first check of changes (modifications)"
    '///open tools / options / StarOffice / Colors
    ToolsOptions
        hToolsOptions ( "StarOffice", "Colors" )
        '///check the number of colors ( is the new entry saved )
        '(92)
        iTemp = Farbe.getItemCount
        printlog "Current color count: '" + iTemp + "'"
        if (iTemp <> iModifiedColorCount) then 
            Warnlog "The number of registered colors is incorrect. Should be: '" + iModifiedColorCount + "' is: '"+ iTemp + "'" 
        end if
        '///select the changed color and check the changes
        Kontext "TabFarben"   
        Farbe.Select(iDefaultColorCount)
        call compareTwoColorsRGB(chColor())
        '///select the new color ( TT-Test ) and check the name and the settings
        Kontext "TabFarben"
        Farbe.Select(iModifiedColorCount)
        call CompareTwoColorsRGB(myColor())
    
        '///delete the new userdefined color
        call deleteColor( USERCOLOR )
        '///reset the modified color to default
        Kontext "TabFarben"
        Farbe.Select(iDefaultColorCount)
        call ModifyColorRGB(oldColor())
        '///close options dialog with OK
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
    Sleep(3)
    
    '///reopen options dialog and check the reset
    printlog " - Second check of changes (reset)"
    ToolsOptions
        hToolsOptions ( "StarOffice", "Colors" )
        '///check if the number of colors in the list has been restored
        iTemp = Farbe.getItemCount
        printlog "Current color count: '" + iTemp + "'"
        if (iTemp <> iDefaultColorCount)  then 
            Warnlog "The number of registered colors is incorrect. Should be: '" + iDefaultColorCount + "' is: '"+ iTemp + "'" 
        endif
        '///check if the last color has been reset to defaults.
        Kontext "TabFarben"
        Farbe.Select(iDefaultColorCount)
        call compareTwoColorsRGB(oldColor())
        '///close options dialog with OK
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
endcase
