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
'*                                                                **
'* owner : helge.delfs@oracle.com                                    **
'*                                                                **
'* short description : Description of file                        **
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
                end if
                Active.Ok
            end if
        end if
    next i
    Kontext
    Select case Seite
    case "TabErsetzung"   : active.SetPage TabErsetzung
    case "TabAusnahmen"   : active.SetPage TabAusnahmen
    case "TabOptionen"    : active.SetPage TabOptionen
    case "TabLocalizedOptions": active.SetPage TabLocalizedOptions
    end select
    for i = 1 to 5
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                if i = 1 and GotItAlready = False then Warnlog Active.Gettext + " Bug#90025"
                Active.Ok
            end if
        end if
    next i
    Kontext Seite
end sub



sub CheckUberpruefen ( Pruefung$ )
    Call wTypeKeys Pruefung$
    wait 100
    Call wTypeKeys "<Home><Shift End>"
    EditCopy
    if GetClipboardText = Pruefung$ then Warnlog "Text has not been replaced"
    Call wTypeKeys "<Home><Shift End><Delete>"
end sub

sub UnCheckUeberpruefen ( Pruefung$ )
    Call wTypeKeys Pruefung$
    wait 100
    Call wTypeKeys "<Home><Shift End>"
    EditCopy
    if GetClipboardText <> Pruefung$ then Warnlog "Text has been replaced"
    Call wTypeKeys "<Home><Shift End><Delete>"
end sub


sub WortErgaenzen(Tastaturbefehl as string)
    Call wTypeKeys "Die"
    Wait 500
    Call wTypeKeys Tastaturbefehl
    Call wTypeKeys "<End><Mod1 Shift Left>"
    EditCopy
    if GetClipBoardtext = "Dies" then Warnlog "Word 'Dies' has been completed!"
    if Tastaturbefehl = "<Return>" then Call wTypeKeys "<Return>"

    Call wTypeKeys "auto"
    Wait 500
    Call wTypeKeys Tastaturbefehl
    Call wTypeKeys "<End><Mod1 Shift Left>"
    EditCopy
    if GetClipBoardtext <> "automatischen" then Warnlog "Word 'automatischen' has not been completed!"
    if Tastaturbefehl = "<Return>" then Call wTypeKeys "<Return>"

    Call wTypeKeys "Wor"
    Wait 500
    Call wTypeKeys Tastaturbefehl
    Call wTypeKeys "<End><Mod1 Shift Left>"
    EditCopy
    if GetClipBoardtext <> "Worterkennung" then Warnlog "Word 'Worterkennung' has not been completed!"
    if Tastaturbefehl = "<Return>" then Call wTypeKeys "<Return>"

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
        end if
    end if

    Kontext "Autotext"
    try
        Bereiche.Click
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                QAErrorlog Active.Gettext
                Active.Ok
            end if
        end if
    catch
        Warnlog "Button 'Categories' is disabled!"
        exit function
    endcatch

    Kontext "BereicheBearbeitenAutotext"
    if ( BereicheBearbeitenAutotext.Exists( 1 ) ) then

        Liste.TypeKeys "<Home>"
        For i = 1 to Liste.GetItemCount
            if Liste.GetText = vCategoryName then
                QAErrorlog "Category " & vCategoryName & " already existing"
                bCategoryExist = true
                exit for
            end if
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
                        exit function
                    end if
                endcatch
            next i

            if Liste.GetText <> vCategoryName then
                Warnlog "New category is not selected after creation"
                try
                    Liste.Select vCategoryName
                    wCreateAutotextCategory = true
                catch
                    exit function
                endcatch
            else
                wCreateAutotextCategory = true
            end if
        else
            wCreateAutotextCategory = true
        end if

        BereicheBearbeitenAutotext.Ok

        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                QAErrorlog Active.Gettext
                Active.Ok
            end if
        end if

    else
        warnlog( "Dialog <BereicheBearbeitenAutotext> is not available" )
    endif

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
        end if
    end if
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
        end if
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
        end if
    end if

    Kontext "Autotext"
    try
        Bereiche.Click

        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                QAErrorlog Active.Gettext
                Active.Ok
            end if
        end if

    catch
        Warnlog "Button 'Categories' is disabled!"
        exit function
    endcatch

    wait 500
    Kontext "BereicheBearbeitenAutotext"
    if Not BereicheBearbeitenAutotext.Exists then exit function

    Liste.TypeKeys "<Home>"
    for i=1 to 10
        if Liste.Gettext <>  vCategoryName then
            Liste.TypeKeys "<Down>"
            wait 500
        end if
    next i

    if Liste.Gettext = vCategoryName then
        try
            Loeschen.Click
        catch
            exit function
            BereicheBearbeitenAutotext.Close
        endcatch
    end if
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
        end if
    end if

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorlog Active.Gettext
            Active.Ok
        end if
    end if

end function

'----------------------------------------------------------------------

function wDeleteAutotext(vAutotextName as string) as boolean
    '/// This function deletes an autotext
    '/// requires AutotextName to delete
    '/// returns true if Autotext could be deleted
    '/// Attention: Autotext-Dialog leaves open after deleting autotext

    Dim j as integer
    Dim ik as integer
    Dim bDeleted as boolean
    Dim sAllGroups as integer
    Dim sAllInGroups as integer

    Kontext "Autotext"
    if Not Autotext.Exists then EditAutotext

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorlog Active.Gettext
            Active.Ok
        end if
    end if

    Kontext "Autotext"
    Liste.Select 1
    for j=1 to 10
        Liste.TypeKeys "-"
        Liste.TypeKeys "<Down>"
        wait 500
    next j

    bDeleted = false
    ' count all groups
    sAllGroups = Liste.GetItemCount
    ' travel through all groups
    For ik = 1 to sAllGroups
        Liste.Select ik
        Liste.TypeKeys "+"

        sAllInGroups = Liste.GetItemCount - sAllGroups
        For j = 1 to sAllInGroups
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
                    end if
                    bDeleted = true
                    exit for
                catch
                    Warnlog "Menu Autotext->'Delete' disabled ! Test failed !"
                endcatch
            end if
        next j
        if bDeleted = true then exit for
        ' close group
        Liste.Select ik
        Liste.TypeKeys "-"
    next ik

end function


sub wChangeDefaultView()

    ' #This code forces the writer to display one page only which is
    ' zoomed to pagewidth, filling the entire - previously maximized - window.
    ' This is required because the VCL Testtool might eventually draw objects
    ' outside the document window depending on display size

    Call hNewDocument
    ViewZoom
    Kontext "Massstab"
    Vergroesserung100.check
    Singlepage.check
    Massstab.Ok
    Call hCloseDocument

end sub
