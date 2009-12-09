'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: CJK_FeatureSwitchTest_1.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'* owner : hercule.li@sun.com
'*
'* short description : CJK Feature Switch test
'*
'\***********************************************************************

testcase tSwitchLocation
  ToolsOptions

  Call hToolsOptions("LANGUAGESETTINGS" , "LANGUAGES")

  try
    if Aktivieren.IsChecked = TRUE  then
      printLog "CJK language is checked"
    else
      printLog "CJK language isn't checked"
    end if
  catch
    warnlog "Something wrong in checkbox 'Asian Language Support'"
  endcatch

  Kontext "ExtrasOptionenDlg"
  ExtrasOptionenDlg.OK
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest1
  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

   Kontext
   FormatParagraph
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"

   try
    if TextrasterFangen.IsChecked = TRUE  then
      printLog "snap to text grid is checked"
    else
      printLog "snap to text grid isn't checked"
    end if
   catch
    warnlog "Something wrong in checkbox 'snap to text grid'"
   endcatch

   TabAusrichtungAbsatz.Cancel

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")

      Kontext
      FormatParagraph
      Active.SetPage TabAusrichtungAbsatz
      Kontext "TabAusrichtungAbsatz"

      try
        if TextrasterFangen.IsChecked = TRUE Or TextrasterFangen.IsChecked <> TRUE then
           warnlog "Something wrong in checkbox 'snap to text grid'"
        end if
      catch
      endcatch

     TabAusrichtungAbsatz.Cancel
  end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest2
  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

   Kontext
   FormatParagraph

   try
    Active.SetPage TabAsianTypography
    Kontext "TabAsianTypography"
    TabAsianTypography.Cancel
   catch
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    TabAusrichtungAbsatz.Cancel
    warnlog "Something wring in tabpage - Asian Typography!"
   endcatch

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")

      Kontext
      FormatParagraph

      try
       Active.SetPage TabAsianTypography
       Kontext "TabAsianTypography"
       TabAsianTypography.Cancel
       warnlog "tabpage - Asian Typography should NOT appear!"
      catch
       Active.SetPage TabAusrichtungAbsatz
       Kontext "TabAusrichtungAbsatz"
       TabAusrichtungAbsatz.Cancel
      endcatch
   end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest3
  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

   Kontext
   FormatCharacter
   Kontext
   active.SetPage TabFont
   kontext "TabFont"
     try
      FontEast.Select 1
     catch
      warnlog "Something wrong in list box Asian text font when Asian Language support is on"
     endcatch
   TabFont.Cancel

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")

      Kontext
      FormatCharacter
      Kontext
      active.SetPage TabFont
      kontext "TabFont"
        try
         FontEast.Select 1
         warnlog "Something wrong in list box Asian text font when Asian Language support is off"
        catch
        endcatch
      TabFont.Cancel
   end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest4
  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

   Kontext
   FormatCharacter
   Kontext
   active.SetPage TabFontEffects
   kontext "TabFontEffects"
     try
      Emphasis.Select 1
     catch
      warnlog "Something wrong in list box Emphasis mark when Asian Language support is on"
     endcatch
   TabFontEffects.Cancel

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")

      Kontext
      FormatCharacter
      Kontext
      active.SetPage TabFontEffects
      kontext "TabFontEffects"
        try
         Emphasis.Select 1
         warnlog "Something wrong in list box Emphasis mark when Asian Language support is off"
        catch
        endcatch
      TabFontEffects.Cancel
   end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest5
  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

  ToolsOptions
  if hToolsOptions("LANGUAGESETTINGS","SEARCHINGINJAPANESE") <> TRUE then
     warnlog "Something wrong in tabpage Searching in Japanese when Asian Language support is on"
  end if

  Kontext "ExtrasOptionenDlg"
  ExtrasOptionenDlg.Cancel

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")
     ToolsOptions

     if hToolsOptions("LANGUAGESETTINGS","SEARCHINGINJAPANESE" , TRUE) = FALSE then
        printLog Chr(13) + "Yes, 'Searching Japanese'-page has been disabled after switching Asian Language Support 'off'."
     else
        warnlog "Something wrong in tabpage Searching in Japanese when Asian Language support is off"
     end if

     Kontext "ExtrasOptionenDlg"
     ExtrasOptionenDlg.Cancel
  end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest6
  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

  ToolsOptions
  if hToolsOptions("LANGUAGESETTINGS","ASIANLAYOUT") <> TRUE then
     warnlog "Something wrong in tabpage Asian Layout when Asian Language support is on"
  end if

  Kontext "ExtrasOptionenDlg"
  ExtrasOptionenDlg.Cancel

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")
     ToolsOptions

     if hToolsOptions("LANGUAGESETTINGS","ASIANLAYOUT", TRUE) = FALSE then
        printLog Chr(13) + "Yes, 'Asian Layout'-page has been disabled after switching 'Asian Language Support' off."
     else
        warnlog "Something wrong in tabpage Asian Layout when Asian Language support is off"
     end if

     Kontext "ExtrasOptionenDlg"
     ExtrasOptionenDlg.Cancel
  end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest7
  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

  try
    formatRuby
    Kontext "RubyDialog"
    DialogClose.Click
    Sleep 2
  catch
     warnlog "Something wrong in menu format/Ruby when Asian Language support is on"
  endcatch

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")

     try
       formatRuby
       Kontext "RubyDialog"
       DialogClose.Click
       Sleep 2
       QAErrorlog "#i40913# Something wrong in menu format/Ruby when Asian Language support is off"
     catch
     endcatch
  end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest8

  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

  try
    FormatChangeCaseHalfWidth
  catch
    warnlog "Something wrong in submenu format/change case -> Half width when Asian Language support is on"
  endcatch

  try
    FormatChangeCaseFullWidth
  catch
    warnlog "Something wrong in submenu format/change case -> Full width when Asian Language support is on"
  endcatch

  try
    FormatChangeCaseHiragana
  catch
    warnlog "Something wrong in submenu format/change case -> Hiragana when Asian Language support is on"
  endcatch

  try
    FormatChangeCaseKatagana
  catch
    warnlog "Something wrong in submenu format/change case -> Katagana when Asian Language support is on"
  endcatch

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")

     try
       FormatChangeCaseHalfWidth
       QAErrorlog "#i43434# Something wrong in submenu format/change case -> Half width when Asian Language support is off"
     catch
     endcatch

     try
       FormatChangeCaseFullWidth
       QAErrorlog "#i43434# Something wrong in submenu format/change case -> Full width when Asian Language support is off"
     catch
     endcatch

     try
       FormatChangeCaseHiragana
       QAErrorlog "#i43434# Something wrong in submenu format/change case -> Hiragana when Asian Language support is off"
     catch
     endcatch

     try
       FormatChangeCaseKatagana
       QAErrorlog "#i43434# Something wrong in submenu format/change case -> Katagana when Asian Language support is off"
     catch
     endcatch
  end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest9

    qaerrorlog( "#i105756# - Test case outdated, needs adjustments" )
    goto endsub
    
  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

  FormatPageWriter
  Kontext
   Active.SetPage TabSeite
   Kontext "TabSeite"

   try
     Textfluss.select 1
   catch
     warnlog "Something wrong in list box text direction when Asian Language support is on"
   endcatch
  TabSeite.Cancel

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")

     FormatPageWriter
     Kontext
      Active.SetPage TabSeite
      Kontext "TabSeite"

      try
       Textfluss.select 1
       warnlog "Something wrong in list box text direction when Asian Language support is off"
      catch
      endcatch
     TabSeite.Cancel
  end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest10

  call  hNewDocument

  Call CheckAsianLanguageSupport("On")
   sleep(1)

   Call hToolbarSelect("DRAWING", true)
   try
     VerticalText.Click
   catch
     warnlog "Something wrong in vertical text box when Asian Language support is on"
   endcatch
   try
     VerticalCallout.Click
   catch
     warnlog "Something wrong in vertical text callout when Asian Language support is on"
   endcatch

   if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
       Call CheckAsianLanguageSupport("Off")
       Kontext "Drawbar"
       try
           VerticalText.Click
           warnlog "Something wrong in vertical text box when Asian Language support is off"
       catch
       endcatch
       try
           VerticalCallout.Click
           warnlog "Something wrong in vertical text callout when Asian Language support is off"
       catch
       endcatch
   end if

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tWriterUIChangesTest11

    qaerrorlog( "#i105756# - Test case outdated, needs adjustments" )
    goto endsub

 Dim iHopeNumber1 , iHopeNumber2 as Integer
 dim i as integer
 dim iCount as integer

  iHopeNumber1 = 38
  iHopeNumber2 = 19


  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

    FormatNumberingBullets
     Kontext
     Active.SetPage TabOptionenNumerierung
     Kontext "TabOptionenNumerierung"
     
     iCount = Numerierung.GetItemCount
     if iCount = iHopeNumber1 then
        Numerierung.Select 15
        TabOptionenNumerierung.OK

        printLog Chr(13) + "Pls. go to the printer to find if you can get a paper"
        printLog Chr(13) + "which is printed like '-'.Otherwise , something wrong!"

        FilePrint
        Kontext "Active"
        if Active.Exists then
          if Active.GetRT = 304 then
            Active.Ok
            QAErrorLog "No Default-Printer!"
            Kontext "DruckenDlg"
            Sleep 1
            DruckenDlg.Cancel
          end if
        else
          Kontext "DruckenDlg"
          Sleep 1
          DruckenDlg.OK
          Sleep 5
        end if
     else
        for i =1 to iCount
            printlog "("+i+"/"+iCount+"): "+Numerierung.getItemText(i)
        next i
        'Referenze:
        TabOptionenNumerierung.Cancel
        qaerrorlog "the number of list box numbering is wrong when Asian Language support is on; expected: " + iHopeNumber1 + "; is: " + iCount
     end if
     
     'print warning if no printer is available
     kontext
     if active.exists(5) then
         qaErrorlog "print warning: '" + active.getText + "'"
         active.ok
     endif

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")

       FormatNumberingBullets
        Kontext
        Active.SetPage TabOptionenNumerierung
        Kontext "TabOptionenNumerierung"

         iCount = Numerierung.GetItemCount
        if iCount <> iHopeNumber2 then
        for i =1 to iCount
            printlog "("+i+"/"+iCount+"): "+Numerierung.getItemText(i)
        next i
        'Referenze:
        qaerrorlog "the number of list box numbering is wrong when Asian Language support is off; expected: " + iHopeNumber2 + "; is: " + iCount
        end if
       TabOptionenNumerierung.Cancel
  end if

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tProlongedVowelsTest1

  call  hNewDocument

  Call CheckAsianLanguageSupport("On")

  ToolsOptions
  Call hToolsOptions("LANGUAGESETTINGS","SEARCHINGINJAPANESE")

  if ProlongedSoundmarks.IsChecked <> TRUE  then
     warnlog "The default setting of 'Prolonged vowels (ka-/kaa)' isn't ACTIVATED."
  end if

  Kontext "ExtrasOptionenDlg"
  ExtrasOptionenDlg.Cancel

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tCalcUIChangesTest_10


  call  hNewDocument

  Call CheckAsianLanguageSupport("On")
   sleep(1)
   Call hToolbarSelect("DRAWING", true)
   try
     VerticalText.Click
   catch
     warnlog "Something wrong in vertical text box when Asian Language support is on"
   endcatch
   try
     VerticalCallout.Click
   catch
     warnlog "Something wrong in vertical text callout when Asian Language support is on"
   endcatch

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")
      Kontext "Drawbar"
      try
        VerticalText.Click
        warnlog "Something wrong in vertical text box when Asian Language support is off"
      catch
      endcatch
      try
        VerticalCallout.Click
        warnlog "Something wrong in vertical text callout when Asian Language support is off"
      catch
      endcatch
   end if

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tImpressUIChangesTest_10

  call  hNewDocument

  Call CheckAsianLanguageSupport("On")
   sleep(1)
   Kontext "Drawbar"
   sleep(1)
   try
     VerticalText.Click
   catch
     warnlog "Something wrong in vertical text box when Asian Language support is on"
   endcatch

   if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
      Call CheckAsianLanguageSupport("Off")
      sleep(1)
      Kontext "Drawbar"
      try
        VerticalText.Click
        warnlog "Something wrong in vertical text box when Asian Language support is off"
      catch
      endcatch
   end if

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tDrawUIChangesTest_10

  call  hNewDocument

  Call CheckAsianLanguageSupport("On")
   Kontext "Drawbar"
   sleep(1)
   try
     VerticalText.Click
   catch
     warnlog "Something wrong in vertical text box when Asian Language support is on"
   endcatch

   if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
      Call CheckAsianLanguageSupport("Off")
      Kontext "Drawbar"
      try
        VerticalText.Click
        warnlog "Something wrong in vertical text box when Asian Language support is off"
      catch
      endcatch
   end if

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------
