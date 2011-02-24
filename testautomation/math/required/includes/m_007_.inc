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
'* short description : Test the Help Menu
'*
'\*****************************************************************

sub M_007_
   printlog Chr(13)+ "--  Help-Menu  m_007_ --"
  Call tmHelpHelpAgent
  Call tmHelpTips
  Call tmHelpExtendedTips
  Call tmHelpAboutStarOffice
  Call tmHelpContents 'wrn:1
end sub

'-----------------------------------------------------------
'*******************    I D dito   *************************
'-----------------------------------------------------------


testcase tmHelpHelpAgent
   '/// open application ///'
   Call hNewDocument

   SchreibenInMathdok "a over b"

   HelpHelpAgent ' it's just a switch
   sleep 2
   HelpHelpAgent

   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmHelpTips
   '/// open application ///'
   Call hNewDocument
   SchreibenInMathdok "a over b"

   HelpTips
   Sleep 2
   HelpTips

   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmHelpExtendedTips
   '/// open application ///'
   Call hNewDocument
   SchreibenInMathdok "a over b"

   HelpEntendedHelp
   Sleep (2)
   HelpEntendedHelp

   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmHelpAboutStarOffice
   '/// open application ///'
   Call hNewDocument
   SchreibenInMathdok "a over b"

   HelpAboutStarOffice
   Kontext "UeberStarMath"
   DialogTest (UeberStarMath)
   UeberStarMath.OK

   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmHelpContents
   dim i as integer

   '/// open application ///'
   Call hNewDocument
   '/// Help->Contents ///'
   sleep 1
   try
       HelpContents
   catch
       qaerrorlog "argh!"
   endcatch
   sleep(8)
   kontext "StarOfficeHelp"
   if Not StarOfficeHelp.Exists then
      Warnlog "Help is not up!"
   else
      '/// get the number of entries in the listbox 'select Help modul' ///'
      try
          Printlog "HelpAbout: '" + HelpAbout.GetItemCount +"'"
      catch
          Index.Click
          '#i55563# 
          warnlog "Used Index Button"
          Printlog "HelpAbout: '" + HelpAbout.GetItemCount +"'"
      endcatch
      try
         '################ left half ################
         '/// on the left half of the dialog: ///'
         '///+ change to the tabpage 'Contents' ///'
         TabControl.SetPage ContentPage
            '///+ get the number of entries in the listbox 'MAin help themes' ///'
            Printlog "SearchContent: '" + SearchContent.GetItemCount + "'"
         '///+ change to the tabpage 'Index' ///'
         TabControl.SetPage IndexPage
            '///+ get the number of entries in the listbox 'Search term' ///'
            Printlog "SearchIndex: '" + SearchIndex.GetItemCount + "'"
            sleep 5
            '///+ click on button 'Display' ///'
            DisplayIndex.Click
            sleep 5
         '///+ change to the tabpage 'Find' ///'
         TabControl.SetPage FindPage
            '///+ get the number of entries in the listbox 'Search term' ///'
            Printlog "SearchFind: '" + SearchFind.GetItemCount + "'"
            '///+ click on button 'Find' -> It has to be disabled #107880# ///'
            try
                FindButton.Click
                warnlog "Find button is enabled, but no text was enterd to search for: BUG!"
            catch
                printlog "Find without text entered to search for didn't work: OK"
            endcatch
            '///+ Enter 'recumbent' in textfield 'Search term'  ///'
            SearchFind.SetText "recumbent"
            '///+ click on button 'Find' ///'
            FindButton.Click
            kontext
            '///+ Messagebox comes up about: 'No topics found.' say OK ///'
            if (active.exists (2) )then
               Printlog "active came up: '" + active.gettext + "'"
               active.ok
            endif
            kontext "StarOfficeHelp"
            '///+ check checkbox 'Complete words only'  ///'
            FindFullWords.Check
            '///+ check checkbox 'Find in headings only'  ///'
            FindInHeadingsOnly.Check
            '///+ get the number of entries in the listbox 'List of headings' ///'
            Printlog "Result: '" + Result.GetItemCount + "'"
            '///+ click on button 'Display' ///'
            DisplayFind.Click
         '///+ change to the tabpage 'Bookmarks' ///'
         TabControl.SetPage BookmarksPage
            '///+ get the number of entries in the listbox 'Bookmark list' ///'
            Printlog "Bookmarks: '" + Bookmarks.GetItemCount + "'"
            '///+ click on button 'Display' ///'
            DisplayBookmarks.Click
         '################ right half ################
         '/// on the right half of teh dialog: ///'
         '################ toolbar ################
         Kontext "TB_Help"
            '///+ click on button 'Hide/Show Navigation Pane' ///'
            Index.Click
            sleep 1
            '///+ click on button 'Hide/Show Navigation Pane' ///'
            Index.Click
            '///+ click on button 'First Page' ///'
            GoToStart.Click
            '///+ click on button 'Previous Page' ///'
            Backward.Click
            '///+ click on button 'Next Page' ///'
            Forward.Click
            '///+ click on button 'Print ...' ///'
            PrintButton.Click
                  kontext
                  if active.exists(2) then
                      active.ok
                      qaerrorlog "There is no printer available - please install one on your system!"
                  endif
                  sleep 2
               kontext "Printing"
               '/// On the dialog 'Print' press the button 'Cancel' ///'
               Printing.cancel
            Kontext "TB_Help"
            '///+ click on button 'Add to Bookmarks ...' ///'
            SetBookmarks.Click
               Kontext "AddBookmark"
               '///+ on the dialog 'Add to Bookmarks ...' get the text from the editfield 'Bookmark' and press button 'Cancel' ///'
               Printlog "Bookmarkname: '" + Bookmarkname.GetText + "'"
               AddBookmark.Cancel
         '################ help display ################
         kontext "HelpContent"
            '///+ open the Context Menu of the Frame 'Help Content' and count the entries ///'
            HelpContent.OpenContextMenu
            Printlog " i: " + hMenuItemGetCount
            hMenuClose()
         '################ right scroolbar ################
         kontext "HelpContent"
            '///+ click on button 'Previous Page' ///'
            if HelpContentUP.IsVisible then
               HelpContentUP.Click
               kontext
               if active.exists(5) then
                   qaerrorlog "Messagebox comes up! ##"
                   printlog "active: '" + active.getText + "'"
                   active.no
               endif
            endif
            kontext "HelpContent"
            '///+ click on button 'Navigation' ///'
            if HelpContentNAVIGATION.IsVisible then
               HelpContentNAVIGATION.Click
            endif
               kontext "NavigationsFenster"
               '/// on the toolbox 'Navigation' press the window close button 'X' ///'
               NavigationsFenster.Close
            kontext "HelpContent"
            '///+ click on button 'Next Page' ///'
            if HelpContentDOWN.IsVisible then
               HelpContentDOWN.Click
               kontext
               if active.exists(5) then
                   qaerrorlog "Messagebox comes up! ##"
                   printlog "active: '" + active.getText + "'"
                   active.no
               endif
            endif
      catch
         warnlog "something is not working in the help-window :-("
      endcatch
         kontext "StarOfficeHelp"
'         StarOfficeHelp.TypeKeys "<Mod2 F4>" ' alt F4 ' doesn't work
'         StarOfficeHelp.TypeKeys "<Mod1 w>" ' strg w  ' doesn't work
         '/// close the help with the keys [strg]+[F4] ///'
      Printlog "trying to close the help now"
      try
         StarOfficeHelp.TypeKeys "<Mod1 F4>" ' strg F4   supported since bug  #103586#
      catch
         Warnlog "failed to close the help window :-("
      endcatch
'         kontext "HelpContent"
'         HelpContent.TypeKeys "<Mod1 w>" ' strg w ' doesn't work
'         HelpContent.TypeKeys "<Mod2 F4>" ' alt f4 ' doesn't work
'         HelpContent.TypeKeys "<Mod1 F4>" ' strg f4 supported since bug  #103586#
      kontext "StarOfficeHelp"
      if StarOfficeHelp.Exists then
         warnlog "Help still up!"
      endif
   endif
   '/// close application ///'
   Call hCloseDocument
endcase


