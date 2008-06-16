'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: i_us2_present.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:42 $
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
'/**************************************************************************
'*
'* Owner : wolfram.garten@sun.com
'*
'* short description : Impress User-Scenario: Beginner makes presentation.
'*
'***************************************************************************
' #1 i_us2_pres1
' #1 i_us2_pres2
'\**************************************************************************

testcase i_us2_pres1

    dim Result1, Result2 as integer
    dim iPictures as integer
    dim PresentationFile1 as string
    PresentationFile1 = (ConvertPath (gOfficePath + "user\work\PwrPres1.odp"))
    kontext "DocumentImpress"
    '/// New - Wizard - Presentation. ///'
    FileAutopilotPresentation
      printlog "   Called File-Autopilot-Presentation."
      '/// Create from Template ///'
      kontext "AutoPilotPraesentation1"
      FromTemplate.Check
      'This part is for language-indepencancy
      if gOOO = TRUE then
         TemplateRegion.Select (1)
         '/// Choose a Presentation ///'
         TemplateList.Select (2)
      else
         TemplateRegion.Select (1)
         if TemplateList.GetItemCount < 40 then
            TemplateRegion.Select (2)
            if TemplateList.GetItemCount < 40 then
               TemplateRegion.Select (3)
               if TemplateList.GetItemCount < 40 then
                  Warnlog "   No Templates selectable in the wizard. Please check."
                  goto endsub
               endif
            endif
         endif
         '/// Choose a Presentation ///'
         TemplateList.Select (4)
      endif
      '/// Next ///'
      Nextbutton.Click
      sleep (1)
      Backbutton.Click
      sleep (1)
      Nextbutton.Click

      printlog "   Switched to the second Wizard-page."
      sleep (1)
      '/// Presentations ///'
      kontext "AutoPilotPraesentation2"
      '/// Output: Screen ///'
      if gOOO = TRUE then  'OpenOffice.org
         Background.Select (2)
         if Backgroundchoice.GetItemCount < 2 then
            Background.Select (1)
            if Backgroundchoice.GetItemCount = 0 then
               Warnlog "   No Backgrounds selectable in the wizard. Please check."
               goto endsub
            endif
         endif
         Backgroundchoice.Select (2)
      else  'StarOffice
         Background.Select (3)
         if Backgroundchoice.GetItemCount < 10 then
            Background.Select (2)
            if Backgroundchoice.GetItemCount < 10 then
               Background.Select (3)
               if Backgroundchoice.GetItemCount < 10 then
                  Warnlog "   No Backgrounds selectable in the wizard. Please check."
                  goto endsub
               endif
            endif
         endif
         Backgroundchoice.Select (5)
      endif
      Screen.Check
      '/// Next ///'
      Nextbutton.Click

      printlog "   Switched to the third Wizard-page."
      sleep (1)
      kontext "AutoPilotPraesentation3"
      '/// Random Effect. Random Speed. Click through every choice. ///'
      Effect.Select (15)
      Speed.Select (3)
      Automatic.Check
      Timebutton.SetText "00:00:15"
      Break.SetText "00:00:20"
      Logo.Check
      '/// Next ///'
      Nextbutton.Click

      printlog "   Switched to the fourth Wizard-page."
      sleep (1)
      kontext "AutoPilotPraesentation4"
      '/// Fill in some company-name-subject-text ///'
      AddName.SetText "Lala1"
      AddTopic.SetText "Lala2"
      Askinformation.SetText "Lala3"
      '/// Next ///'
      Nextbutton.Click
      printlog "   Switched to the fifth Wizard-page."
      sleep (1)
      kontext "AutoPilotPraesentation5"
      Summary.Check
      '/// Create ///'
      WaitSlot (2000)
      AutoPilotPraesentation5.OK

   sleep (1)
   printlog "   Pressed 'Create'."
   '/// Add a Slide via Insert - Slide. ///'
   Kontext "DocumentImpress"
   hTypeKeys "What we will talk about"
   gMouseClick 1,1
   Kontext "DocumentImpress"
   DocumentImpress.UseMenu
      hMenuSelectNr (4)
      hMenuSelectNr (1)
   sleep (1)

   kontext "Slides"
   SlidesControl.TypeKeys "<HOME><RETURN>"
   kontext "DocumentImpress"
   DocumentImpress.TypeKeys "<TAB>Text we just wrote..."
   '/// Switch to the second slide. ///'
   kontext "Slides"
      SlidesControl.TypeKeys "<DOWN><RETURN>"
      SlidesControl.TypeKeys "<RETURN>"
   kontext "DocumentImpress"
   '/// Insert three lines with text, all with different formatting. ///'
      call hTextrahmenErstellen ("First line with text",20,50,70,50)
      call hTextrahmenErstellen ("Second line with text",20,60,70,60)
      call hTextrahmenErstellen ("Third line with text",20,70,70,70)
   '/// Go down to the eleventh slide. ///'
   kontext "Slides"
      SlidesControl.TypeKeys "<HOME>"
      SlidesControl.TypeKeys "<PAGEDOWN>", 10
      SlidesControl.TypeKeys "<RETURN>"
   '/// Insert Smiley, + two circles around the eyes, + two new pupils, ///'
   '/// placed at some interesteing place inside the circles. ///'
      Kontext "Toolbar"
      SymbolShapes.TearOff
      sleep (1)
      kontext "SymbolShapes"
      SymbolShapesSmiley.Click
      sleep (1)
      kontext "DrawingObjectbar"
         AreaFilling.TypeKeys "<HOME>"
         AreaFilling.TypeKeys "<DOWN>", 10    'Select (11) 'Lime-Green
         AreaFilling.TypeKeys "<RETURN>"
      sleep (1)
      gMouseMove (53,50,90,80)
      Kontext "Toolbar"
      Ellipsen.Click
      kontext "SymbolShapes"
      SymbolShapes.Close
      Kontext "DocumentImpress"
      gMouseMove (60,43,70,65)    'Right eye
      hTypeKeys "<Escape>"
      Kontext "Toolbar"
      Ellipsen.Click
      gMouseMove (65,57,69,63)    'Right pupil
      hTypeKeys "<Escape>"
      Kontext "Toolbar"
      Ellipsen.Click
      gMouseMove (70,50,82,65)    'Left eye
      hTypeKeys "<Escape>"
      Kontext "Toolbar"
      Ellipsen.Click
      gMouseMove (77,57,80,63)    'Left pupil
      hTypeKeys "<Escape>"
      sleep (1)

   '/// Insert new slide. ///'
   kontext "Slides"
      SlidesControl.TypeKeys "<PAGEDOWN>", 15  'To get to the last slide.
      SlidesControl.TypeKeys "<RETURN>"
   '/// Delete the two fields the stylist has. ///'
   kontext "DocumentImpress"
   gMouseClick 1,1
   EditSelectAll
   DocumentImpress.TypeKeys "<DELETE>"
   '/// Insert-Picture-From File. (some crazy picture which fits the theme) ///'
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   '/// Open graphic  ///'
   Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\ball.eps")
   Oeffnen.Click
   gMouseClick 1,1
   '/// Add a text above the picture. 'There is movement...' ///'
   call hTextrahmenErstellen ("There is movement...",20,20,70,30)

   '/// Add a new slide. ///'
   InsertSlide
   printlog "   Inserted new slide."
   kontext "DocumentImpress"
   EditSelectAll
   DocumentImpress.TypeKeys "<DELETE>"
   '/// Bla bla about everything... ///'
   call hTextrahmenErstellen ("Bla bla about everything...",20,20,70,30)

   '/// Add a new slide. ///'
   InsertSlide
   printlog "   Inserted new slide."
   kontext "DocumentImpress"
   EditSelectAll
   DocumentImpress.TypeKeys "<DELETE>"
   '/// Write text:  Q & A  ///'
   call hTextrahmenErstellen ("Q & A",20,20,70,30)
   DocumentImpress.TypeKeys "<SHIFT HOME>"
   sleep (1)

   '/// Add a new slide. ///'
   InsertSlide
   kontext "DocumentImpress"
   EditSelectAll
   DocumentImpress.TypeKeys "<DELETE>"
   '/// Thanks for listening, time for coffee... (ending) ///'
   call hTextrahmenErstellen ("Class dismissed...",20,20,70,30)

   '/// Save Document ///'
   call hFileSaveAsKill (PresentationFile1)
   printlog "OK   saved at ", PresentationFile1
   sleep (1)

   '/// Close Document ///'
   Call hCloseDocument
endcase

'00oo...//==---...---...---...---....---...---...---...---...---...---...--..--.--.-.-.-.-....---....

testcase i_us2_pres2
    qaerrorlog "not yet ready."
    goto endsub
   dim Result1, Result2 as integer
   dim iPictures as integer
   dim PresentationFile1, PresentationFile2, PresentationFileToBeLoaded as string
   PresentationFile1 = (ConvertPath (gTesttoolPath + "graphics\required\input\rightfont.odp"))
   PresentationFile2 = (ConvertPath (gOfficePath + "user\work\xxxxxx9.odp"))
   kontext "DocumentImpress"
   '/// New - Wizard - Presentation ///'
   FileAutopilotPresentation
      printlog "   Called File-Autopilot-Presentation."
      '/// Create from Template ///'
      kontext "AutoPilotPraesentation1"
      EmptyPresentation.Check
      AutoPilotPraesentation1.TypeKeys "<DOWN>"
      sleep (3)
      AutoPilotPraesentation1.TypeKeys "<DOWN>"
      if Existingpresentation.IsChecked(5) then
         printlog "   Pressing down worked fine"
      else
         warnlog "   Pressing down didnt work as expected."
      endif
      '/// Click the Open-button ///'
      ExistingOpen.Click
      '/// Check if the Open-dialogue is there ///'
      kontext "GeneralFileDialog"
      if GeneralFileDialog.Exists(10) = FALSE then
         warnlog "   Open-file-dialogue didnt show up."
      else
         GeneralFileDialog.Cancel
      endif
      sleep (3)
      kontext "AutoPilotPraesentation1"
      AutoPilotPraesentation1.TypeKeys "<SHIFT TAB>"
      AutoPilotPraesentation1.TypeKeys "<UP>"
      AutoPilotPraesentation1.TypeKeys "<DOWN>"
      AutoPilotPraesentation1.TypeKeys "<TAB><TAB>"
      sleep (1)
      Existinglist.TypeKeys "<HOME>"
      sleep (1)
      Existinglist.TypeKeys "<DOWN>"
      sleep (3)
      if Existinglist.GetSelText = "" then
         Warnlog "   No entries in the recently-opened-files-list. Check if it's correct."
      else
         PresentationFileToBeLoaded = Existinglist.GetSelText
         AutoPilotPraesentation1.OK
      endif

   kontext "DocumentImpress"
   if DocumentImpress.Exists(10) = FALSE then
      if DocumentImpress.StatusIsProgress = FALSE then
         sleep (10)
         if DocumentImpress.StatusIsProgress then
            warnlog "   Took over 20 seconds to load the document Document: " + PresentationFileToBeLoaded + ". Stalled?"
         endif
         goto endsub
      endif
   else
      printlog "   Document " + PresentationFileToBeLoaded + " seemed to have been loaded correctly."
   endif
   sleep (3)
   if DocumentImpress.StatusIsProgress then
      sleep (5)
   endif
   '/// Save Document ///'
   call hFileSaveAsKill (PresentationFile2)
   printlog "OK   saved at ", PresentationFile2
   sleep (1)

   '/// Close Document ///'
   Call hCloseDocument
endcase
