'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_tool2.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-18 08:36:08 $
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
'*                                                                **
'* owner : helge.delfs@sun.com                                    **
'*                                                                **
'* short description : Description of file                        **
'*                                                                **
'*******************************************************************
'*                                                                **
' #1 OptionenAufrufen
' #1 CheckUberpruefen
' #1 UnCheckUeberpruefen
' #1 WortErgaenzen
'*                                                                **
'\******************************************************************

sub OptionenAufrufen(Seite as string)
    Dim i as integer, GotItAlready as Boolean
    Kontext
    ToolsAutoCorrect
    For i = 1 to 5
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                if i = 1 then
                    Warnlog Active.Gettext + " Bug#90025"
                    GotItAlready = True
                endif
                Active.Ok
            endif
        endif
    next i
    Kontext
    Select case Seite
        case "TabErsetzung"   : active.SetPage TabErsetzung
        case "TabAusnahmen"   : active.SetPage TabAusnahmen
        case "TabOptionen"    : active.SetPage TabOptionen
        case "TabTypografisch": active.SetPage TabTypografisch
    end select
    for i = 1 to 5
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                if i = 1 and GotItAlready = False then Warnlog Active.Gettext + " Bug#90025"
                Active.Ok
            endif
        endif
    next i
    Kontext Seite
end sub



sub CheckUberpruefen ( Pruefung$ )
    Call wDokSchreiben Pruefung$
    wait 100
    Call wDokSchreiben "<Home><Shift End>"
    EditCopy
    if GetClipboardText = Pruefung$ then Warnlog "Text has not been replaced"
    Call wDokSchreiben "<Home><Shift End><Delete>"
end sub

sub UnCheckUeberpruefen ( Pruefung$ )
    Call wDokSchreiben Pruefung$
    wait 100
    Call wDokSchreiben "<Home><Shift End>"
    EditCopy
    if GetClipboardText <> Pruefung$ then Warnlog "Text has been replaced"
    Call wDokSchreiben "<Home><Shift End><Delete>"
end sub


sub WortErgaenzen(Tastaturbefehl as string)
    Call wDokSchreiben "Die"
    Wait 500
    Call wDokSchreiben Tastaturbefehl
    Call wDokSchreiben "<End><Mod1 Shift Left>"
    EditCopy
    if GetClipBoardtext = "Dies" then Warnlog "Word 'Dies' has been completed!"
    if Tastaturbefehl = "<Return>" then Call wDokSchreiben "<Return>"

    Call wDokSchreiben "auto"
    Wait 500
    Call wDokSchreiben Tastaturbefehl
    Call wDokSchreiben "<End><Mod1 Shift Left>"
    EditCopy
    if GetClipBoardtext <> "automatischen" then Warnlog "Word 'automatischen' has not been completed!"
    if Tastaturbefehl = "<Return>" then Call wDokSchreiben "<Return>"

    Call wDokSchreiben "Wor"
    Wait 500
    Call wDokSchreiben Tastaturbefehl
    Call wDokSchreiben "<End><Mod1 Shift Left>"
    EditCopy
    if GetClipBoardtext <> "Worterkennung" then Warnlog "Word 'Worterkennung' has not been completed!"
    if Tastaturbefehl = "<Return>" then Call wDokSchreiben "<Return>"

end sub

' -----------------------------------------------------------------------------

function wCreateAutotextCategory(vCategoryName as string) as boolean
    Dim i as integer, bCategoryExist as boolean    
    '/// This function creates an autotext-category
    '/// requires CategoryName to create
    '/// returns true if category could be created
    '/// Attention: Autotext-Dialog leaves open after creating category
    Kontext "Autotext"
    if Not Autotext.Exists then EditAutotext
    
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorlog Active.Gettext
            Active.Ok
        endif
    endif
    
    Kontext "Autotext"
    try
        Bereiche.Click        
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                QAErrorlog Active.Gettext
                Active.Ok
            endif
        endif
    catch
       Warnlog "Button 'Categories' is disabled!"
       goto NotPassed
    endcatch
    
    wait 500
    Kontext "BereicheBearbeitenAutotext"
    if Not BereicheBearbeitenAutotext.Exists then goto NotPassed
    
    Liste.TypeKeys "<Home>"
    For i = 1 to Liste.GetItemCount
        if Liste.GetText = vCategoryName then
            QAErrorlog "Category " & vCategoryName & " already existing"
            bCategoryExist = true
            exit for
        endif
    next i
    
    if bCategoryExist = false then
		for i = 1 to Pfad.GetItemCount        
			Pfad.Select i
			Bereich.Settext vCategoryName
			try
            	Neu.Click
				exit for
			catch
				if i = Pfad.GetItemCount then
					Warnlog "Unable to create new category"
					BereicheBearbeitenAutotext.Close
					goto NotPassed				
				endif
			endcatch
		next i
        
        if Liste.GetText <> vCategoryName then
            Warnlog "New category is not selected after creation"        
            try
                Liste.Select vCategoryName
                wCreateAutotextCategory = true
            catch
                goto NotPassed
            endcatch
        else
            wCreateAutotextCategory = true
        endif
    else
        wCreateAutotextCategory = true
    endif
    
    Sleep 1
    BereicheBearbeitenAutotext.Ok      

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorlog Active.Gettext
            Active.Ok
        endif
    endif    
    
    NotPassed:

end function

' -----------------------------------------------------------------------------

function wSelectAutotextCategory(vCategoryName as string) as boolean
    Dim i as integer    
    '/// This function selects an autotext-category
    '/// requires CategoryName to select
    '/// returns true if category could be selected
    '/// Attention: Autotext-Dialog leaves open after selecting category
    Kontext "Autotext"
    if Not Autotext.Exists then EditAutotext

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorlog Active.Gettext
            Active.Ok
        endif
    endif    
    '/// close all categories
    Kontext "Autotext"
    Liste.Select 1
    for i=1 to 10
        Liste.TypeKeys "-"
        Liste.TypeKeys "<Down>"
        wait 500
    next i

    Liste.TypeKeys "<Home>"
    for i=1 to 10
        if Liste.Gettext <>  vCategoryName then
            Liste.TypeKeys "<Down>"
            wait 500
        endif
    next i    
    
    if Liste.Gettext =  vCategoryName then wSelectAutotextCategory = true

end function    

' -----------------------------------------------------------------------------

function wDeleteAutotextCategory(vCategoryName as string) as boolean
    Dim i as integer    
    '/// This function deletes an autotext-category
    '/// requires CategoryName to delete
    '/// returns true if category could be deleted
    '/// Attention: Autotext-Dialog leaves open after deleting category
    Kontext "Autotext"
    if Not Autotext.Exists then EditAutotext

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorlog Active.Gettext
            Active.Ok
        endif
    endif
    
    Kontext "Autotext"
    try
        Bereiche.Click
        
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                QAErrorlog Active.Gettext
                Active.Ok
            endif
        endif

    catch
       Warnlog "Button 'Categories' is disabled!"
       goto NotPassed
    endcatch
    
    wait 500
    Kontext "BereicheBearbeitenAutotext"
    if Not BereicheBearbeitenAutotext.Exists then goto NotPassed
    
    Liste.TypeKeys "<Home>"
    for i=1 to 10
        if Liste.Gettext <>  vCategoryName then
            Liste.TypeKeys "<Down>"
            wait 500
        endif
    next i    
    
    if Liste.Gettext = vCategoryName then    
        try
            Loeschen.Click    
        catch
            goto NotPassed
            BereicheBearbeitenAutotext.Close
        endcatch           
    endif
    BereicheBearbeitenAutotext.Ok
    
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
                wDeleteAutotextCategory = true
                Sleep 1
            catch
                Active.Ok
            endcatch
        endif
    endif

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorlog Active.Gettext
            Active.Ok
        endif
    endif

    NotPassed:

end function

'----------------------------------------------------------------------

function wDeleteAutotext(vAutotextName as string) as boolean
    '/// This function deletes an autotext
    '/// requires AutotextName to delete
    '/// returns true if Autotext could be deleted
    '/// Attention: Autotext-Dialog leaves open after deleting autotext
    
    Dim j as integer, sAll as integer
    Kontext "Autotext"
    if Not Autotext.Exists then EditAutotext

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorlog Active.Gettext
            Active.Ok
        endif
    endif
    
    Kontext "Autotext"            
    Liste.Select 1
    for j=1 to 10
        Liste.TypeKeys "-"
        Liste.TypeKeys "<Down>"
        wait 500
    next j

    Liste.Select 1
    Liste.TypeKeys "+"
    sAll = Liste.GetItemCount
    For j = 1 to sAll
        Liste.TypeKeys "<Down>"        
        if Liste.Gettext = vAutotextName then
            try
                Menue.Click
                wait 500
                Call hMenuSelectNr ( 3 )
                Kontext "Active"
                if Active.Exists and Active.GetRT = 304 then
                    Active.yes
                    wDeleteAutotext = true
                else
                    Warnlog "No messages to confirm deleting the Autotext!"
                endif
                exit for
            catch
                Warnlog "Menu Autotext->'Delete' disabled ! Test failed !"
            endcatch
        endif
        Liste.TypeKeys "+"
        sAll = Liste.GetItemCount
    next j

end function
