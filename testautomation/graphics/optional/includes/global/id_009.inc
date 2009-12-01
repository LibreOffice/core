'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_009.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:41 $
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
'* Owner : wolfram.garten@sun.com
'*
'* short description : Testcases to test the Help-Menu.
'*
'***********************************************************************************
' #1 tmHelpHelpAgent
' #1 tmHelpTips
' #1 tmHelpExtendedTips
' #1 tmHelpAboutStarOffice
' #1 tmHelpContents
' #1 tCheckIfTheHelpExists
'\**********************************************************************************
'
testcase tmHelpHelpAgent

    Call hNewDocument

    hTBOtypeInDoc

    HelpHelpAgent         ' it's just a switch
    sleep 2
    HelpHelpAgent

    Call hCloseDocument
endcase

'...---....---.-.-.-.-.....---......--.-.-.-.....----..-........................---.......

testcase tmHelpTips
    Call hNewDocument
    hTBOtypeInDoc

    HelpTips
    Sleep 2
    HelpTips

    Call hCloseDocument
endcase

'...---....---.-.-.-.-.....---......--.-.-.-.....----..-........................---.......

testcase tmHelpExtendedTips
    Call hNewDocument
    hTBOtypeInDoc

    HelpEntendedHelp
    Sleep (2)
    HelpEntendedHelp

    Call hCloseDocument
endcase

'...---....---.-.-.-.-.....---......--.-.-.-.....----..-........................---.......

testcase tmHelpAboutStarOffice
    Call hNewDocument
    hTBOtypeInDoc

    HelpAboutStarOffice
    Kontext "UeberStarMath"
    DialogTest (UeberStarMath)
    UeberStarMath.OK

    Call hCloseDocument
endcase

'...---....---.-.-.-.-.....---......--.-.-.-.....----..-........................---.......

testcase tmHelpContents
    goto endsub '"#i84486# - tmHelpContents outcommented due to crash."
    dim i as integer

    Call hNewDocument
    HelpContents
    sleep(8)
    kontext "StarOfficeHelp"
    if Not StarOfficeHelp.Exists then
        Warnlog "Help is not up!"
    else
        Printlog "HelpAbout: '" + HelpAbout.GetItemCount +"'"
        '################ left half ################
        TabControl.SetPage ContentPage
        Printlog "SearchContent: '" + SearchContent.GetItemCount + "'"
        TabControl.SetPage IndexPage
        Printlog "SearchIndex: '" + SearchIndex.GetItemCount + "'"
        sleep 5
        DisplayIndex.Click
        sleep 5
        TabControl.SetPage FindPage
        Printlog "SearchFind: '" + SearchFind.GetItemCount + "'"
        if SearchFind.GetSelText = "" then
            if FindButton.IsEnabled then
                warnlog "   The Find-Button should have been inactive, but was active."
            endif
        else
            warnlog "   The Search-Text-Field shouldn't contain any text. But contained: " + SearchFind.GetSelText
        endif
        SearchFind.SetText "Doobbidedooo"
        FindButton.Click
        kontext
        if (active.exists (2) )then
            Printlog "active came up: '" + active.gettext + "'"
            active.ok
        endif
        kontext "StarOfficeHelp"
        FindFullWords.Check
        FindInHeadingsOnly.Check
        Printlog "Result: '" + Result.GetItemCount + "'"
        DisplayFind.Click
        TabControl.SetPage BookmarksPage
        Printlog "Bookmarks: '" + Bookmarks.GetItemCount + "'"
        DisplayBookmarks.Click
        '################ right half ################
        '################ toolbar ################
        Kontext "TB_Help"
        Index.Click
        sleep 1
        Index.Click
        sleep 1
        GoToStart.Click
        sleep 1
        Backward.Click
        sleep 1
        Forward.Click
        sleep 1
        PrintButton.Click
        sleep (1)

        kontext "Active"
        if Active.Exists( 2 ) then
            qaerrorlog "No default printer defined: " & Active.GetText
            Active.Ok
        end if

        kontext "DruckenDLG"
        if DruckenDLG.Exists then
            DruckenDLG.cancel
        else
            warnlog "the Print-Dialogue didnt appear."
        end if
        Kontext "TB_Help"
        sleep 1
        SetBookmarks.Click
        sleep 1
        Kontext "AddBookmark"
        Printlog "Bookmarkname: '" + Bookmarkname.GetText + "'"
        AddBookmark.Cancel
        sleep 1
        '################ help display ################
        kontext "HelpContent"
        HelpContent.OpenContextMenu

        sleep 1
        Printlog " i: " + hMenuItemGetCount
        hMenuClose()
        '################ right scroolbar ################
        kontext "HelpContent"
        if HelpContentUP.IsVisible then
            HelpContentUP.Click
            sleep 1
        endif
        if HelpContentNAVIGATION.IsVisible then
            HelpContentNAVIGATION.Click
            sleep 1
        endif
        kontext "NavigationsFenster"
        NavigationsFenster.Close
        sleep 1
        kontext "HelpContent"
        if HelpContentDOWN.IsVisible then
            HelpContentDOWN.Click
            sleep 1
        endif
        kontext "StarOfficeHelp"
        Printlog "trying to close the help now"
        try
            StarOfficeHelp.TypeKeys "<Mod1 F4>" ' strg F4   supported since bug  #103586#
        catch
            Warnlog "failed to close the help window :-("
        endcatch
        kontext "StarOfficeHelp"
        if StarOfficeHelp.Exists then
            warnlog "Help still up!"
        endif
    endif
    Call hCloseDocument
endcase

'...---....---.-.-.-.-.....---......--.-.-.-.....----..-........................---.......

testcase tCheckIfTheHelpExists
    Call hNewDocument
    HelpContents
    kontext "HelpContent"
    sleep (5)
    HelpContent.TypeKeys "<MOD1 A>"
    sleep (1)
    HelpContent.TypeKeys "<MOD1 C>"
    if GetClipBoard = "" then
        Warnlog "   No content in the Help-Content -view."
    else
        Printlog "   The Help-Content -view contained content. Good."
    endif
    kontext "StarOfficeHelp"
    try
        StarOfficeHelp.TypeKeys "<MOD1 F4>"
    catch
        Warnlog "   Failed to close the help window :-("
    endcatch
    kontext "StarOfficeHelp"
    if StarOfficeHelp.Exists then
        warnlog "Help was still visible!"
    endif
    hTypeKeys "."
    Call hCloseDocument
endcase 'tCheckIfTheHelpExists
