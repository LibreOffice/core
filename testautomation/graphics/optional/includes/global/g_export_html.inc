'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_export_html.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-08-28 11:42:05 $
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
'* short description :
'*
'*********************************************************************
' #1 tHtmlExport
' #1 tHtmlExport_Webcast
'\********************************************************************

testcase tHtmlExport
    
    Dim Autor1 as string
    Dim Adresse1 as string
    Dim Liste(50) as string
    dim i as integer
    dim x as integer
    dim sTemp as string
    dim sFile as string

   '/// open 'tools->options' and set first/last name and email adress on the page 'StarOffice->UserData' ///'
   ToolsOptions
   hToolsOptions ("STAROFFICE","USERDATA")
   VorName.SetText "Garfield"
   ZuName.SetText  "Peanuts"
   EMail.SetText   "recumbent@sun.com"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   '/// create directory if not exists and delete the content, if exists user\\work\\html\\ ///'
   app.mkDir ((ConvertPath (gOfficePath + "user\work\html\"))
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\")), "*.*" , Liste() )
   if (KillFileList (Liste()) <> TRUE) then
      for i = 1 to ListCount (Liste())
         Warnlog "File Not deleted: " + i + ": '" + Liste(i) + "'"
      next i
   endif

   '/// open application ///'
   Call hNewDocument
   WaitSlot (3000)
'---------------------------------- simple document -----------------------------------------------
   '/// create a rectrangle ///'
   hTextrahmenErstellen ("This is a test text",10,10,40,20)
   WaitSlot (2000)
   '/// create another rectrangle ///'
   hRechteckErstellen (50,50,70,70)
   WaitSlot (3000)
'###################################################################################################
   Printlog "'///<b> test if no file is created, if dialog is canceld </b>///'"
   '/// call 'File->Export'///'
   FileExport
   Kontext "ExportierenDlg"
   WaitSlot (1000)
   gHTMLFilter = Dateityp.GetItemText(1)
   Dateityp.Select(gHTMLFilter)
   '/// set Filename to 'user\\work\\html\\expo0.html' ///'
   sFile = ConvertPath (gOfficePath + "user\work\html\expo0")
   Dateiname.SetText sFile
   WaitSlot (2000)
   '/// click button 'Save' ///'
   Speichern.Click
   WaitSlot (3000)
   Kontext "Active"
      if Active.Exists Then Active.Yes
'--------------------------------------------------------------------------
   '/// now we are on the dialog -1- 'Assign design' ///'
   Kontext "HTMLExport1"
   '/// click button 'Cancel' ///'
   HTMLExport1.Cancel
   WaitSlot (5000)
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\")), "*.*" , Liste() )
   WaitSlot (1000)
   Printlog "used file name: '" + sFile + "'"
   if (ListCount (Liste()) > 0) then
      warnlog "dialog for export was canceled, but file was created :-("
      for i = 1 to ListCount (Liste())
         Warnlog "File created: " + i + ": '" + Liste(i) + "'"
      next i
      KillFileList (Liste())
   end if
'###################################################################################################
   Printlog "'///<b> test if files are created </b>///'"
   '/// call 'File->Export'///'
   FileExport
   Kontext "ExportierenDlg"
   WaitSlot (1000)
   try
      '/// select Filetype 'HTML...' aka 'WebPage' ///'
      Dateityp.Select gHTMLFilter
   catch
      errorlog  " Filter not available :-( : '"+gHTMLFilter+"'"
      Dateityp.Select 1 '/// Selects the first entry - which should be html ///'
   endcatch
   WaitSlot (1000)
   '/// set Filename to 'user\\work\\html\\expo.html' ///'
   sFile = ConvertPath (gOfficePath + "user\work\html\expo.html")
   Dateiname.SetText sFile
   WaitSlot (2000)
   '/// click button 'Save' ///'
   Speichern.Click
   WaitSlot (3000)
   Kontext "Active"
      if Active.Exists Then Active.Yes
'--------------------------------------------------------------------------
   Printlog "'/// now we are on the dialog -1- 'Assign design' ///'"
   Kontext "HTMLExport1"
   '/// 'New design' should be checked ///'
   if (NeuesDesign.IsEnabled <> TRUE) then
      warnlog "NewDesign is not enabled by default :-("
   endif
   '/// click button 'Next >>' ///'
   Weiter.Click
   WaitSlot (1000)
   Printlog "'/// now we are on the dialog -2- 'Publication Type & Options' ///'"
   Kontext "HTMLExport2"
   '/// checkbox 'Standard HTML Format' is checked by default ///'
   if (Standard.IsChecked <> TRUE) then
      warnlog "Standard is not enabled by default :-("
      Standard.Check
   end if
   '/// checkbox 'Title Page' is checked by default ///'
   if Not TitelseiteErzeugen.IsChecked Then
       TitelSeiteErzeugen.Check
       Warnlog  "- 'create Titlepage' should be checked as default :-("
   end if
   if (gApplication = "IMPRESS") then
      '/// if applcation is impress: the checkbox 'Show notes' is enabled by default///'
      If (Notizenanzeigen.IsChecked <> TRUE) Then
          warnlog "- 'Show notes' checked as default :-("
          Notizenanzeigen.Check
      end if
   end if
   '/// click button 'Next >>' ///'
   Weiter.Click
   Printlog "'/// now we are on the dialog -3- 'png/gif/jpeg & resolution & sound' ///'"
   Kontext "HTMLExport3"
   '/// checkbox 'PNG' is checked by default ///'
   if (PNG.isChecked <> TRUE) then
      warnlog "Save Graphic as 'PNG' is not checked as default :-("
      PNG.Check
   end if
   '/// Listbox compression '75%' is selected by default ///'
   if (Kompressionswert.GetSelText <> "75%") then
      warnlog "'Compression' should be '75%', but is: '" + Kompressionswert.GetSelText + "'"
      Kompressionswert.Select "75%"
   end if
   '/// checkbox 'low monitor resolution' is checked by default ///'
   if (NiedrigeAufloesung.IsChecked <> TRUE)Then
      warnlog "Monitor resolution 'low' is not selected as default :-("
      NiedrigeAufloesung.Check
   end if
   '/// checkbox 'export sounds' is checked by default ///'
   if (KlaengeExportieren.IsChecked <> true) then
      warnlog "'Export Sounds' is not checked as default :-("
      KlaengeExportieren.Check
   end if
   '/// click button 'Next >>' ///'
   Weiter.Click
   WaitSlot (1000)
   Printlog "'/// now we are on the dialog -4- 'Information for the title page' ///'"
   Kontext "HTMLExport4"
   '/// the names you entered in the options have to appeare here! ///'
   sTemp = Autor.GetText
   Autor1=Autor.GetText
   if (sTemp <> "Garfield Peanuts") then
      warnlog "Value for 'Author' is other than expected, should 'Garfield Peanuts', is: '"+sTemp+"'"
   end if
   sTemp = EMail.GetText
   Adresse1=EMail.GetText
   if (sTemp <> "recumbent@sun.com") then
      warnlog "Value for 'Email' is other than expected, should be 'recumbent@sun.com', is: '"+sTemp+"'"
   end if
   '/// set text in field 'your homepage' ///'
   Homepage.SetText "mahler"
   AdditionalInformation.SetText "QualityApprooved"
   '/// click button 'Next >>' ///'
   Weiter.Click
   WaitSlot (2000)
   Printlog "'/// now we are on the dialog -5- 'select button style' ///'"
   Kontext "HTMLExport5"
   WaitSlot (5000)
   '/// checkbox 'Text only' should be checked by default ///'
   if (NurText.ischecked <> TRUE) then
      warnlog "'Text only' is not checked by default :-("
      NurText.check
   end if
   '/// click button 'Next >>' ///'
   Weiter.Click
   WaitSlot (1000)
   Printlog "'/// now we are on the dialog -6- 'select color scheme' ///'"
   Kontext "HTMLExport6"
   WaitSlot (2000)
   ' maybe check if colour dialog comes up ?! TBO
   'Text
   'Hyperlink
   'AktiverLink
   'GesichteterLink
   'Hintergrund
   '/// 'Apply color schemes from document' should be checked per default :-) ///'
   if (FarbenAusDok.IsChecked <> TRUE) Then
      warnlog "'Apply color schemes from document' should be checked per default :-("
      FarbenAusDok.Check
   end if
   WaitSlot (2000)
   '/// click button 'Create' ///'
   HTMLExport6.OK
   WaitSlot (5000)
   Printlog "'/// now we are on the dialog -7- 'Name HTML Design' ///'"
   Kontext "HTMLDesign"
   WaitSlot (1000)
   Designname.SetText "Lem's Standard"
   HTMLDesign.OK
   WaitSlot (3000)
    Kontext "active"
    if active.exists (5) then
      printlog " the name for this design already exists : '" + active.GetText+ "'"
        printlog " SAY YES!------------------"
        active.yes
    end if
   '/// check number and names of created files ///'
   Printlog "used file name: '" + sFile + "'"
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\")), "*.*" , Liste() )
   WaitSlot (1000)
   x =  ListCount(Liste())
'   Printlog "gApplication is: " + gApplication   ' For debugging - FHA
   if gApplication = "DRAW" then
      if (x <> "3") then
         Warnlog "- Number of files is not correct, it should be:3 but it is: '" + x + "' - Draw"
            for i=1 to x
               printlog "("+i+"/"+x+"): '"+Liste(i)+"'"
            next
      end if
   else
      if (x <> "4") then
         Warnlog "- Number of files is not correct, it should be:4 but it is: '" + x + "' - Impress"
            for i=1 to x
               printlog "("+i+"/"+x+"): '"+Liste(i)+"'"
            next
      end if
   end if
   '/// check if fileextension is common - .htm or .html for all files the same ///'
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\")), "*.htm" , Liste() )
   WaitSlot (1000)
   x =  ListCount(Liste())
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\")), "*.html" , Liste() )
   WaitSlot (1000)
   i =  ListCount(Liste())
   if ((x > 0) AND (i > 0)) Then
      Warnlog " fileextension is different between HTML-files :-( (htm/html)"
      Warnlog " htm : " + x
      Warnlog " html: " + i
   end if
'------------------------------------------------------------------------------------------------------------------------------
   Printlog "'///<b> check if design is created in dialog </b>///'"
   WaitSlot (3000)
   '/// File->Export ///'
   FileExport
   Kontext "ExportierenDlg"
   '/// Set Filename to "user\\work\\html\\expo.html" ///'
   Dateiname.SetText sFile
   try
      '/// select Filetype 'HTML...' aka 'WebPage' ///'
      Dateityp.Select gHTMLFilter
   catch
      errorlog " Filter not available :-( : '"+gHTMLFilter+"'"
   endcatch
   WaitSlot (1000)
   '/// click button 'save' ///'
   Speichern.Click
   WaitSlot (1000)
      Kontext "Active"
      ' active about overwriting existing file
      if Active.Exists Then Active.Yes
'--------------------------------------------------------------------------
   WaitSlot (1000)
      Kontext "HTMLExport1"
   WaitSlot (1000)
      try
         '/// now 'existing design' is checkable -> check it ///'
         ExistierendesDesign.Check
         WaitSlot (3000)
         '/// there has to be 1 design in the list, select and delete it ///'
         x = Designliste.GetItemCount
         if (x <> 1) then
            warnlog "there is an unexpected count on designs in the list, expected 1, but there are : '" + x + "'"
         end if
         Designliste.Select 1
         sTemp = Designliste.GetSelText
         if sTemp = "Lem's Standard" Then
            Printlog "- Created design exists in list and will be deleted now"
            WaitSlot (1000)
            DesignLoeschen.Click
            WaitSlot (2000)
         else
            Warnlog "- Created design does not appear in the list it is: '"+sTemp+"'"
         end if
      catch
         Warnlog "- List could not be accessed"
      endcatch
      WaitSlot (5000)
      HTMLExport1.OK
      kontext "Active"
      if Active.Exists then
         Active.Yes
      end if
   Call hCloseDocument
   Printlog "used file name: '" + sFile + "'"
   ' TBOTODO CHECK files contenet !
   
endcase 'tHtmlExport
'-------------------------------------------------------------------------------
testcase tHtmlExport_Webcast

    Dim iWaitIndex as integer
    Dim i as integer
    Dim a as integer
    Dim x as integer
    dim Liste ( 200 ) as String
    dim sFile as string

   '/// create directories if not exists and delete the content, if exists user\\work\\html\\[asp|perl]\\ ///'
   app.mkDir (ConvertPath (gOfficePath + "user\work\html\asp\"))
   app.mkDir (ConvertPath (gOfficePath + "user\work\html\perl\"))
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\asp\")), "*.*" , Liste() )
   WaitSlot (1000)
   if (KillFileList (Liste()) <> TRUE) then
      for i = 1 to ListCount (Liste())
         Warnlog "File Not deleted: " + i + ": '" + Liste(i) + "'"
      next i
   end if
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\perl\")), "*.*" , Liste() )
   WaitSlot (1000)
   if (KillFileList (Liste()) <> TRUE) then
      for i = 1 to ListCount (Liste())
         Warnlog "File Not deleted: " + i + ": '" + Liste(i) + "'"
      next i
   end if

   '/// File->Autopillot->Presentation... ///'----------------------------------
    FileAutopilotPresentation
    Kontext "Active"
    if Active.Exists(1) then
        WarnlogActive.GetText
        try
        Active.OK
        catch
        Active.No
        endcatch
        goto endsub
    end if
    '/// Page 1 of autopilot///'
    Kontext "AutopilotPraesentation1"
    '/// uncheck 'preview' ///'
    Preview.Uncheck
    '/// click on button 'create' ///'
    AutopilotPraesentation1.OK
    WaitSlot (3000)
    '************************************************************ Page 1
    Printlog "'///<b> create a presentation and save it for further references </b>///'"
    '/// start creating the presentation now ///'-------------------------------
    Printlog "- Create presentation"
    '/// select 'click to add title' with [TAB] and press [F2] to edit the placeholder title ///'
    hTypeKeys "<TAB><F2>"
    WaitSlot (2000)
    Select Case Ucase(gApplication)
        case "DRAW"         : hMouseClick DocumentDraw,50,50
        case "IMPRESS"      : hMouseClick DocumentImpress,50,50
    end select
    WaitSlot (3000)
    '/// Type 'This is a test text', select it. ///'
    hTextrahmenErstellen ("This is a test text",10,10,40,20)
    EditSelectAll
    Kontext "TextObjectbar"
    '/// click 'bold text' in object toolbar ///'
    Fett.Click
    WaitSlot (2000)
    '/// press [esacpe][tab] to get out of edit mode and select the next placeholder ///'
    hTypeKeys "<escape><TAB>"
    WaitSlot (1000)
    '/// SlideShow->Effects ///'
    SlideShowCustomAnimation
    WaitSlot (1000)
    Kontext "Tasks"
    WaitSlot (1000)
    EffectAdd.Click
    '/// Dialog 'Custom Animation' comes up ///'
    kontext
    '/// Switch to TabPage: Entrance ///'
    active.setPage(TabEntrance)
    Kontext "TabEntrance"
    if TabEntrance.Exists (5) then
          WaitSlot (1000)
          '/// select effect 'ABC v' :-) ///'
          Effects.Select(5)
          '/// select speed 'slow' -> first item in list ///'
          Speed.Select 1
          try
              TabEntrance.Ok
          catch
              errorlog "SlideShowEffects wasnt closed as it should have been"
          endcatch
          if TabEntrance.Exists then
              TabEntrance.Close
              warnlog "TabEntrance.Ok didn't seem to close the dialog."
          else
              printlog "TabEntrance was closed as it should"
          end if
          kontext "Tasks"
      end if
   WaitSlot (3000)
   '/// activate grafik insert dialog by typing [return] ///'
'Doesnt work, will work when accessability works better - FHA.

'   hTypeKEys "<return>"  ' i love accessability feartures .-)))))))))))))))))
      InsertGraphicsFromFile
      Kontext "GrafikEinfuegenDlg"
      if GrafikEinfuegenDlg.Exists <> TRUE then
          WaitSlot (1000)
      end if
      '/// insert grafik: global\\input\\graf_inp\\borabora.jpg ///'
      Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\borabora.jpg")
      WaitSlot (3000)
      Oeffnen.Click
   WaitSlot (2000)
   '/// select 'click to add an outline' with [TAB] and press [F2] to edit the placeholder ///'
   hTypeKeys "<TAB><F2>"
   '/// insert text and select the words by typing [strg a] ///'
   hTypeKeys "Mecury<RETURN>Venus<RETURN>Earth<RETURN>Mars<RETURN>Jupiter<RETURN>Saturn<RETURN>Uranus<RETURN>Neptune<RETURN>Pluto"
   EditSelectAll
   WaitSlot (1000)
   '/// format->character ///'
   FormatCharacter
      WaitSlot (2000)
      Kontext
      Active.SetPage TabFont
      Kontext "TabFont"
      '/// set font size to '18' on tabpage 'font' ///'
      try
         SizeWest.SetText "18"
      catch
         Size.SetText "18"
      endcatch
   '/// click button OK ///'
   TabFont.OK
   WaitSlot (1000)
   '/// click 'bold text' in object toolbar ///'
   Kontext "TextObjectbar"
   try
       usebindings
       Fett.Click 'culprint
   catch
       warnlog "Objectbar 'Text' not available :-( / didn't change automatical!"
       hTypeKeys "<escape><escape><escape><tab><tab><TAB>"
       WaitSlot (1000)
       hTypeKeys "<F2>"
       hTypeKeys "<HOME>"
       hTypeKeys "<SHIFT><END>"
       Kontext "TextObjectbar"
       try
           usebindings
           Fett.Click 'culprint
       catch
           warnlog "darn again TBO"
       endcatch
   endcatch
   WaitSlot (2000)
   '/// SlideShow-> CustomAnimation ///'
   SlideShowCustomAnimation
       WaitSlot (1000)
       Kontext "Tasks"
       WaitSlot (1000)
       EffectChange.Click
       kontext
       '/// Switch to TabPage: Entrance ///'
       active.setPage(TabEntrance)
       kontext "TabEntrance"
       Effects.TypeKeys "<DOWN>",8
       Speed.Select 2
       try
           TabEntrance.Ok
       catch
           warnlog "select another effect, assign is not possible"
       endcatch
'************************************************************ Page 2 Merkur
   '/// create a 2nd slide with: insert->slide ///'
   InsertSlide
   WaitSlot (2000)
   hTypekeys "<Pagedown>"
   WaitSlot (2000)
   '/// select 'click to add title' with [TAB] and press [F2] to edit the placeholder title ///'
   hTypeKeys "<TAB><F2>"
   WaitSlot (2000)
   '/// Type 'MERKUR', select it via Edit-Select All///'
   hTextrahmenErstellen ("MERKUR",10,10,60,30)
   EditSelectAll
   WaitSlot (2000)
   '/// click 'bold text' in object toolbar ///'
   Kontext "TextObjectbar"
   Fett.Click
   WaitSlot (1000)
   '/// press [esacpe][tab][return] to get out of edit mode and select the next placeholder and activate grafik insert dialog ///'
'   hTypeKeys "<escape><tab><return>"
   hTypeKeys "<escape><tab>"
   InsertGraphicsFromFile
      WaitSlot (4000)
      '/// insert grafik: global\\input\\graf_inp\\borabora.jpg ///'
      Kontext "GrafikEinfuegenDlg"
      Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\borabora.jpg")
      WaitSlot (3000)
   Oeffnen.Click
   WaitSlot (2000)
   '/// select 'click to add an outline' with [TAB] and press [F2] to edit the placeholder ///'
   hTypeKeys "<TAB><F2>"
   '/// insert text and select the words by typing [strg a] ///'
   WaitSlot (1000)
   hTypeKeys "Reciprocal Mass *5972000<SHIFT RETURN>Mass (Erde=1) 0,0558<SHIFT RETURN>Mass in kg 3,30310<SHIFT RETURN>"
   hTypeKeys "Equator-radius (km) 2440<SHIFT RETURN>Equator-radius (Erde=1)0,382<SHIFT RETURN>Middle density (kg/m^3) 5400<SHIFT RETURN>"
   hTypeKeys "Gravity acceleration at the equator (m/s^2) 3,78<SHIFT RETURN>Flightvelocity at the equator (km/s) 4,3<SHIFT RETURN>Sidereal rotationtime 58,65 Tage<SHIFT RETURN>"
   hTypeKeys "Bias of the equatorplane against the orbitplane 0><SHIFT RETURN>Time of circulation 88 days<SHIFT RETURN>Distance to the sun Sun (km) 58Mio.<SHIFT RETURN>Temperatur at the day 350C<SHIFT RETURN>"
   hTypeKeys "Temperatur at night -170C<SHIFT RETURN>Diameter (km) 4800"
   EditSelectAll
   WaitSlot (1000)
   '/// format->character ///'
   FormatCharacter
      WaitSlot (2000)
      Kontext
      Active.SetPage TabFont
      Kontext "TabFont"
      '/// set font size to '18' on tabpage 'font' ///'
      try
         SizeWest.SetText "16"
      catch
         Size.SetText "16"
      endcatch
   '/// click button OK ///'
   TabFont.OK
   WaitSlot (2000)
   '/// click 'bold text' in object toolbar ///'
   Kontext "TextObjectbar"
   try
      Fett.Click 'culprint
   catch
      warnlog "Objectbar 'Text' not available :-( / didn't change automatical!"
      hTypeKeys "<escape><escape><escape><tab><tab><TAB>"
      WaitSlot (1000)
      hTypeKeys "<F2>"
      hTypeKeys "<HOME>"
      hTypeKeys "<SHIFT><END>"
      Kontext "TextObjectbar"
      try
         Fett.Click 'culprint
      catch
        warnlog "darn again TBO"
      endcatch
   endcatch
   WaitSlot (2000)
   '/// save file with impress filter :  "user\\work\\webcast.odp" ///'
   sFile = ConvertPath (gOfficePath + "user\work\webcast")
   if (Dir (sFile + ".odp") <> "") then app.kill (sFile + ".odp")
   WaitSlot (2000)
   hFileSaveAsWithFilterKill ((sFile + ".odp") , "impress8" )
   Printlog "saved presentation: '" + sFile + ".odp"
'----------------------------------- Exportieren als ASP Web-Cast
   Printlog "'///<b> export as webcast ASP </b>///'"
   '/// File->SaveAs ///'
   FileExport
      WaitSlot (2000)
      Kontext "ExportierenDlg"
      try
        '/// select Filetype 'HTML...' aka 'WebPage' ///'
        gHTMLFilter = Dateityp.GetItemText(1)
        Dateityp.Select(gHTMLFilter)
      catch
         errorlog  " Filter not available :-( : '"+gHTMLFilter+"'"
         Dateityp.Select 1 '/// Selects the first entry - which should be html ///'
      endcatch
      '/// Set Filename to "user\\work\\webcst_asp.html" ///'
      sFile = ConvertPath (gOfficePath + "user\work\html\asp\webcast.html")
      Dateiname.SetText sFile
      '/// click button 'save' ///'
      Speichern.Click
      WaitSlot (5000)
      Kontext "Active"' ????????? TBO
         if Active.Exists (5) Then Active.Yes
      WaitSlot (3000)
      '/// now we are on the dialog -1- 'Assign design' ///'
      Kontext "HTMLExport1"
      WaitSlot (2000)
      '/// click button 'Next >>' ///'
      Weiter.Click
      WaitSlot (2000)
      '/// now we are on the dialog -2- 'Publication Type & Options' ///'
      Kontext "HTMLExport2"
      WaitSlot (2000)
      Printlog "'/// check 'WebCast' ///"
      WebCast.Check
      WaitSlot (2000)
      Printlog "'/// check 'Active Server Pages (ASP)'  ///"
      ASP.Check
      '/// click button 'Next >>' ///'
      Weiter.Click
      '/// now we are on the dialog -3- 'gif/jpeg & resolution & sound' ///'
      Kontext "HTMLExport3"
      WaitSlot (2000)
      '/// check Save graphics as 'GIF' ///'
      Gif.Check
      '/// check 'Medium (monitor) Resolution' ///'
      MittlereAufloesung.Check
      WaitSlot (1000)
      '/// click button 'Create' ///'
      HTMLExport3.OK
      WaitSlot (2000)
      '/// now we are on the dialog -7- 'Name HTML Design' ///'
      Kontext "HTMLDesign"
      '/// click button 'Do Not Save' ///'
   if HTMLDesign.Exists Then HTMLDesign.Cancel
   iWaitIndex = 0
   do while HTMLDesign.Exists AND iWaitIndex < 30
      WaitSlot (1000)
      iWaitIndex = iWaitIndex + 1
   loop
   Printlog "- Check number of safed files"
   Printlog "file saved: '" + sFile + "'"
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\asp\")), "*.*" , Liste() )
   WaitSlot (1000)
   x =  ListCount(Liste())
   if (x <> "11") Then
      Warnlog "- Number of files is not correct, it should be:11 but it is: '" + x + "'"
      for i=1 to x
         printlog "("+i+"/"+x+"): '"+Liste(i)+"'"
      next
   end if
'----------------------------------- Exportieren als ASP Web-Cast
   Printlog "'///<b> export as webcast PERL </b>///'"
   Kontext "DocumentImpress"
   WaitSlot (2000)
   '/// File->export ///'
   FileExport
      WaitSlot (2000)
      Kontext "ExportierenDlg"
         '/// select Filetype 'HTML...' aka 'WebPage' ///'
      try
         Dateityp.Select gHTMLFilter
      catch
         errorlog " Filter not available :-( : '"+gHTMLFilter+"'"
         Dateityp.Select 1 ' selects the first one just in case anything else is selected.
         printlog "Selected " + Dateityp.GetSelText + " manually."
      endcatch
      WaitSlot (1000)
      '/// Set Filename to "user\\work\\wbcst_p.html" ///'
      sFile = ConvertPath (gOfficePath + "user\work\html\perl\webcast")
      Dateiname.SetText sFile
      '/// click button 'save' ///'
      Speichern.Click
      WaitSlot (2000)
      Kontext "Active"      '/// click button 'save' ///'
         if Active.Exists Then Active.Yes
      WaitSlot (2000)
      '/// now we are on the dialog -1- 'Assign design' ///'
      Kontext "HTMLExport1"
      WaitSlot (2000)
      '/// click button 'Next >>' ///'
      Weiter.Click
      WaitSlot (2000)
      '/// now we are on the dialog -2- 'Publication Type & Options' ///'
      Kontext "HTMLExport2"
      WaitSlot (2000)
      Printlog "'/// check 'WebCast' ///"
      WebCast.Check
      WaitSlot (2000)
      Printlog "'/// check 'Perl'  ///"
      PERL.Check
      WaitSlot (1000)
      '/// 'URL for listeners' ///'
    printlog "URL for listeners compare to to TBO: '"+BeamerURL.GetText+"'"
      '/// set 'URL for presentation' to 'wbcst_prl.html' ///'
      PraesentationURL.SetText "wbcst_prl"
      WaitSlot (1000)
      '/// set 'URL for perl scrips' to '/WebCast/cgi-bin/' ///'
      ScriptURL.SetText "/WebCast/cgi-bin/"
    printlog "presentation url? : "+ (gOfficePath + "/WebCast/cgi-bin/wbcst_prl")
      WaitSlot (1000)           
      '/// click button 'Next >>' ///'
      Weiter.Click
      WaitSlot (1000)
      '/// now we are on the dialog -3- 'gif/jpeg & resolution & sound' ///'
      Kontext "HTMLExport3"
      WaitSlot (1000)
      '/// check Save graphics as 'GIF' ///'
      Gif.Check
      '/// check 'Medium (monitor) Resolution' ///'
      MittlereAufloesung.Check
      WaitSlot (3000)
      '/// click button 'Create' ///'
   HTMLExport3.OK
   WaitSlot (3000)
   Kontext "HTMLDesign"
   if HTMLDesign.Exists Then HTMLDesign.Cancel
   Printlog "- Presentation saved and exported into both Webcast formats. all files can be found here: " + gOfficePath + "output\impress\web-cast\"
   Printlog "file saved: '" + sFile + ".html'"
   WaitSlot (10000)
   Liste(0) = 0
   GetFileList ((ConvertPath (gOfficePath + "user\work\html\perl\")), "*.*" , Liste() )
   x =  ListCount(Liste())
   if (x <> "12") Then
      Warnlog "- Number of files is not correct, it should be:5 but it is: '" + x + "'"
      for i=1 to x
         printlog "("+i+"/"+x+"): '"+Liste(i)+"'"
      next
   end if
   Call hCloseDocument

   ' TBO: TODO FILECHECK IF EXISTS
endcase 'tHtmlExport_Webcast
