'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_fields6.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:27:26 $
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
'* owner : helge.delfs@sun.com
'*
'* short description : Fields - Functions
'*
'\***********************************************************************

sub w_fields6

    Call tFunctions_1        'Conditional text
    Call tFunctions_2        'Hidden text
    Call tFunctions_3        'Hidden paragraph
    Call tFunctions_4        'Input field
    Call tFunctions_5        'Placeholder - text
    Call tFunctions_6        'Placeholder - table
    Call tFunctions_7        'Placeholder - frame
    Call tFunctions_8        'Placeholder - graphics
    Call tFunctions_9        'Placeholder - object
    Call tFunctions_10       'Input list - Remove
    Call tFunctions_11       'Input list - Move up
    Call tFunctions_12       'Input list - Move down
    Call tFunctions_13       'Combine Character

end sub

'-----------------------------------------------------------------

testcase tFunctions_1

  Dim sName      as String
  Dim sValue()   as String
  Dim sCondition as String
  Dim sElse      as String
  Dim sThen      as String
  Dim sResult()  as String
  Dim iType      as Integer
  Dim i          as Integer
  Dim j          as Integer

  sName      = "X"
  SCondition = "X==" + Chr(34) + "Sun" + Chr(34)
  SThen      = "Yes"
  sElse      = "No"

  sValue  = Array( _
                   "Sun"    , _
                   "NOTSun" , _
                  )
  sResult = Array( _
                   "Sun"    , _
                   "Yes"    , _
                   "NOTSun" , _
                   "No"     , _
                  )

  printlog "Conditional text"
 '/// <b> Conditional text </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "set Variable" ,
  '/// Set 1 Name and set its Value to "Sun" , then goto tabpage function
  '/// + choose "Conditional text" , set condition like
  '/// if X="Sun" then "YES" else "NO"
  '/// Set another value to "NoSun"
   j = UBound(sValue)
   for i = 0 to j
      Call fInsertFieldsOther("TabVariablen")
        Feldtyp.Select 1
        Sleep 1
        NameText.SetText sName
        Wert.SetText sValue(i)
        Zahlenformat.Select 1
        Einfuegen.Click
      TabVariablen.Close
      Call wTypeKeys "<Return>"
      Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select 1
        Sleep 1
        Bedingung.SetText sCondition
        Dann.SetText      sThen
        Sonst.SetText     sElse
        Einfuegen.Click
      TabFunktionen.Close
      Call wTypeKeys "<Return>"
   next i

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"
   j = UBound(sResult)
   for i = 0 to j
       Call wTypeKeys "<Shift Right>"
       if fGetFieldContent() <> sResult(i) then
           Warnlog "Should get " & sResult(i) & " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Home>"
   next i

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_2

  Dim sName       as String
  Dim sValue()    as String
  Dim sCondition  as String
  Dim sHiddenText as String
  Dim sResult()   as String
  Dim bHiddenText as Boolean
  Dim i           as Integer
  Dim j           as Integer

  sName       = "X"
  SCondition  = "X==" + Chr(34) + "ABC" + Chr(34)
  sHiddenText = "HiddenText"

  sValue  = Array( _
                   "ABC"    , _
                   "NOTABC" , _
                  )
  sResult = Array( _
                   ""           , _
                   "HiddenText" , _
                  )

  printlog "Hidden text"
 '/// <b> Hidden text </b>

  Call hNewDocument

  '/// Tools/Options / StarOffice writer / Formatting Aids
  '/// Uncheck 'Fields : Hidden text"
   ToolsOptions
   Call hToolsOptions("WRITER","FORMATTINGAIDS")
     bHiddenText = FeldVersteckterText.IsChecked
     if bHiddenText = TRUE then
         FeldVersteckterText.UnCheck
     end if
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  '/// Insert/Fields/Other/Variables / Select "set Variable" ,
  '/// Set Name and set its Value to "ABC" , then goto tabpage function
  '/// + choose "hidden text" , set condition like
  '/// if X="ABC" as condition , hidden text is "HiddenText"
  '/// Set another value to "NoTABC"
  '/// Check if the insertion is correct
   j = UBound(sValue)
   for i = 0 to j
      Call wTypeKeys "<MOD1 Home><Shift End>"
      Call fInsertFieldsOther("TabVariablen")
        Feldtyp.Select 1
        Sleep 1
        NameText.SetText sName
        Wert.SetText sValue(i)
        Zahlenformat.Select 1
        Einfuegen.Click
      TabVariablen.Close
      Call wTypeKeys "<Return>"
      Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select 7   'Hidden text
        Sleep 1
        Bedingung.SetText sCondition
        Hinweis.SetText   sHiddenText
        Einfuegen.Click
      TabFunktionen.Close

      Call wTypeKeys "<Shift Left>"
      if fGetFieldContent() <> sResult(i) then
          Warnlog "Should get " & sResult(i) & " but get " & fGetFieldContent()
      end if
   next i

  'Recover to default
   ToolsOptions
   Call hToolsOptions("WRITER","FORMATTINGAIDS")
     if bHiddenText = TRUE then FeldVersteckterText.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_3

  Dim sName       as String
  Dim sValue()    as String
  Dim sCondition  as String
  Dim sHiddenText as String
  Dim sResult()   as String
  Dim bHiddenText as Boolean
  Dim i           as Integer
  Dim j           as Integer

  sName       = "X"
  SCondition  = "X==" + Chr(34) + "ABC" + Chr(34)
  sHiddenText = "HiddenText"

  sValue  = Array( _
                   "ABC"    , _
                   "NOTABC" , _
                  )
  sResult = Array( _
                   "ABC"        , _
                   "HiddenText" , _
                  )

  printlog "Hidden paragraph"
 '/// <b> Hidden paragraph </b>

  Call hNewDocument

  '/// Tools/Options / StarOffice writer / Formatting Aids
  '/// Uncheck 'Fields : Hidden paragraph"
   ToolsOptions
   Call hToolsOptions("WRITER","FORMATTINGAIDS")
     bHiddenText = VersteckteAbsatze.IsChecked
     if bHiddenText = TRUE then
         VersteckteAbsatze.UnCheck
     end if
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  '/// Input a paragraph first
   Call wTypeKeys "<Return>"
   Call wTypeKeys sHiddenText
   Call wTypeKeys "<Up>"

  '/// Insert/Fields/Other/Variables / Select "set Variable" ,
  '/// Set Name and set its Value to "ABC" , then goto tabpage function
  '/// + choose "hidden paragraph" , set condition like
  '/// if X="ABC" as condition
  '/// Check if the paragraph disappear or not
   j = UBound(sValue)
   for i = 0 to j
      Call wTypeKeys "<MOD1 Home><Shift End>"
      Call fInsertFieldsOther("TabVariablen")
        Feldtyp.Select 1
        Sleep 1
        NameText.SetText sName
        Wert.SetText sValue(i)
        Zahlenformat.Select 1
        Einfuegen.Click
      TabVariablen.Close
      Call wTypeKeys "<Down><Home>"
      Call fInsertFieldsOther("TabFunktionen")
        Feldtyp.Select 8   'Hidden paragraph
        Sleep 1
        Bedingung.SetText sCondition
        Einfuegen.Click
      TabFunktionen.Close

      Call wTypeKeys "<MOD1 Home><Down><Shift End>"
      if fGetFieldContent() <> sResult(i) then
          Warnlog "Should get " & sResult(i) & " but get " & fGetFieldContent()
      end if
   next i

  'Recover to default
   ToolsOptions
   Call hToolsOptions("WRITER","FORMATTINGAIDS")
     if bHiddenText = TRUE then VersteckteAbsatze.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_4

  Dim sReference as String
  Dim sFieldContent as String

  sReference    = "NewTest"
  sFieldContent = "10000"

  printlog "Input field"
 '/// <b> Input field </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Functions / Select "Input field" ,
  '/// + set reference , press insert button , input field content
   Call fInsertFieldsOther("TabFunktionen")
     Feldtyp.Select 3   'Input field
     Sleep 1
     Hinweis.SetText sReference
     Einfuegen.Click
     Kontext "Eingabefeld"
       if Titel.GetText <> sReference then
           Warnlog "Reference should be " & sReference & " but get " & Titel.GetText
       end if
       EingabeText.SetText sFieldContent
     Eingabefeld.OK

   Kontext "TabFunktionen"
   TabFunktionen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sFieldContent then
       Warnlog "Should get " & sFieldContent & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_5

  Dim sPlaceHolder as String

  sPlaceHolder = "Text"

  printlog "Placeholder - Text"
 '/// <b> Placeholder - Text </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Functions / Select "Placeholder" ,
  '/// + select Text
   Call fInsertFieldsOther("TabFunktionen")
     Feldtyp.Select 5   'Placeholder
     Sleep 1
     Formatliste.Select 1
     Platzhalter.SetText  sPlaceHolder
     Einfuegen.Click
   TabFunktionen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> ("<" & sPlaceHolder & ">" )then
       Warnlog "Should get <" & sPlaceHolder & "> but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_6

    printlog "Placeholder - Table"
    '/// <b> Placeholder - Table </b>

    Call hNewDocument

    Call fFormatPageWriter("TabSeite")
    Breite.SetText "15"
    Hoehe.SetText "15"
    Links.SetText "0"
    Rechts.SetText "0"
    Oben.SetText "0"
    Unten.SetText "0"
    TabSeite.OK
    Sleep 2
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then ' is it a messagebox ?
            Active.Yes
        else
            Warnlog "Messagebox is not up "
        end if
    else
        Warnlog "Something wrong !"
    end if

    '/// Insert/Fields/Other/Functions / Select "Placeholder" ,
    '/// + select Table
    Call fInsertFieldsOther("TabFunktionen")
    Feldtyp.Select 5   'Placeholder
    Sleep 1
    Formatliste.Select 2
    Platzhalter.SetText "TableTableTableTableTable"
    Einfuegen.Click
    TabFunktionen.Close

    '/// Check if the insertion is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    if fGetFieldContent() <> ("<TableTableTableTableTable>" )then
        Warnlog "Should get <TableTableTableTableTable> but get " & fGetFieldContent()
    end if

    Call fSetToBigSize()
    Call wTypeKeys "<MOD1 Home>"
    Sleep 1

	Call gMouseMove(0,0,30,25)
    Call gMouseClick(30,25)
    Sleep 3

    Kontext "TabelleEinfuegenWriter"
    if TabelleEinfuegenWriter.Exists then
        TabelleEinfuegenWriter.Cancel
    else
        Warnlog "Insert table dialog isn't up !"
    end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_7

    printlog "Placeholder - Frame"
    '/// <b> Placeholder - Frame </b>

    Call hNewDocument

    fFormatPageWriter("TabSeite")
    Breite.SetText "15"
    Hoehe.SetText "15"
    Links.SetText "0"
    Rechts.SetText "0"
    Oben.SetText "0"
    Unten.SetText "0"
    TabSeite.OK
    Kontext "Active"
    if Active.Exists then
       if Active.GetRT = 304 then ' is it a messagebox ?
           Active.Yes
       else
           Warnlog "Messagebox is not up "
       end if
    else
       Warnlog "Something wrong !"
    end if

    '/// Insert/Fields/Other/Functions / Select "Placeholder" ,
    '/// + select Frame
    Call fInsertFieldsOther("TabFunktionen")
    Feldtyp.Select 5   'Placeholder
    Sleep 1
    Formatliste.Select 3
    Platzhalter.SetText "FrameFrameFrameFrameFrameFrame"
    Einfuegen.Click
    TabFunktionen.Close

    '/// Check if the insertion is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    if fGetFieldContent() <> ("<FrameFrameFrameFrameFrameFrame>" )then
        Warnlog "Should get <FrameFrameFrameFrameFrameFrame> but get " & fGetFieldContent()
    end if

    Call fSetToBigSize()
    Call wTypeKeys "<MOD1 Home>"
    Sleep 1
	Call gMouseMove(0,0,30,25)
    Call gMouseClick(30,25)
    Sleep 3

    try
        Kontext
        Active.SetPage TabType
        Kontext "TabType"
        AutoHoehe.UnCheck
        TabType.Cancel
    catch
        Warnlog "Format/Frame dialog isn't up !"
    endcatch

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_8

    printlog "Placeholder - Graphics"
    '/// <b> Placeholder - Graphics </b>

    Call hNewDocument

    fFormatPageWriter("TabSeite")
    Breite.SetText "15"
    Hoehe.SetText "15"
    Links.SetText "0"
    Rechts.SetText "0"
    Oben.SetText "0"
    Unten.SetText "0"

    TabSeite.OK
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then ' is it a messagebox ?
            Active.Yes
        else
            Warnlog "Messagebox is not up "
        end if
    else
        Warnlog "Something wrong !"
    end if

    '/// Insert/Fields/Other/Functions / Select "Placeholder" ,
    '/// + select Graphics
    Call fInsertFieldsOther("TabFunktionen")
    Feldtyp.Select 5   'Placeholder
    Sleep 1
    Formatliste.Select 4
    Platzhalter.SetText "GraphicsGraphicsGraphicsGraphicsGraphics"
    Einfuegen.Click
    TabFunktionen.Close

    '/// Check if the insertion is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    if fGetFieldContent() <> ("<GraphicsGraphicsGraphicsGraphicsGraphics>" )then
        Warnlog "Should get <GraphicsGraphicsGraphicsGraphicsGraphics> but get " & fGetFieldContent()
    end if

    Call fSetToBigSize()
    Call wTypeKeys "<MOD1 Home>"
    Sleep 1
	Call gMouseMove(0,0,30,25)
    Call gMouseClick(30,25)
    Sleep 3

    try
        Kontext "GrafikEinfuegenDlg"
        Oeffnen.Click
        GrafikEinfuegenDlg.Cancel
    catch
            Warnlog "Insert picture dialog isn't up !"
    endcatch

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_9

    printlog "Placeholder - Object"
    '/// <b> Placeholder - Object </b>

    Call hNewDocument

    fFormatPageWriter("TabSeite")
    Breite.SetText "15"
    Hoehe.SetText "15"
    Links.SetText "0"
    Rechts.SetText "0"
    Oben.SetText "0"
    Unten.SetText "0"
    TabSeite.OK
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then ' is it a messagebox ?
            Active.Yes
        else
            Warnlog "Messagebox is not up "
        end if
    else
        Warnlog "Something wrong !"
    end if

    '/// Insert/Fields/Other/Functions / Select "Placeholder" ,
    '/// + select object
    Call fInsertFieldsOther("TabFunktionen")
    Feldtyp.Select 5   'Placeholder
    Sleep 1
    Formatliste.Select 5
    Platzhalter.SetText "ObjectObjectObjectObjectObjectObject"
    Einfuegen.Click
    TabFunktionen.Close

    '/// Check if the insertion is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>"
    if fGetFieldContent() <> ("<ObjectObjectObjectObjectObjectObject>" )then
        Warnlog "Should get <ObjectObjectObjectObjectObjectObject> but get " & fGetFieldContent()
    end if

    Call fSetToBigSize()
    Call wTypeKeys "<MOD1 Home>"
    Sleep 1
	Call gMouseMove(0,0,30,25)
    Call gMouseClick(30,25)
    Sleep 3

    try
        Kontext "OLEObjektEinfuegen"
        NeuErstellen.Check
        OLEObjektEinfuegen.Cancel
    catch
        Warnlog "Insert OLE Object dialog isn't up !"
    endcatch

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_10

  Dim sItem1 as String
  Dim sItem2 as String

  sItem1 = "First" : sItem2 = "Second"

  printlog "Input list - Remove"
 '/// <b> Input list Remove</b>

  Call hNewDocument

  '/// Insert/Fields/Other/Functions / Select "Input list" ,
  '/// + insert 2 itmes , remove 1st one
   Call fInsertFieldsOther("TabFunktionen")
     Feldtyp.Select 2   'Input list
     Sleep 1
     NewListEntry.SetText  sItem1
     AddListEntry.Click
     NewListEntry.SetText  sItem2
     AddListEntry.Click
     Sleep 1
     Einfuegen.Click
     Call wTypeKeys "<Return>"
     Kontext "TabFunktionen"
     EntrysList.Select 1
     RemoveEntry.Click
     Einfuegen.Click
   TabFunktionen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sItem1 then
       Warnlog "Should get " & sItem1 & " but get " & fGetFieldContent()
   end if
   Call wTypeKeys "<Down><Home>"
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sItem2 then
       Warnlog "Should get " & sItem2 & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_11

  Dim sItem1 as String
  Dim sItem2 as String

  sItem1 = "First" : sItem2 = "Second"

  printlog "Input list - Move up"
 '/// <b> Input list Move up</b>

  Call hNewDocument

  '/// Insert/Fields/Other/Functions / Select "Input list" ,
  '/// + insert 2 itmes , press move up
   Call fInsertFieldsOther("TabFunktionen")
     Feldtyp.Select 2   'Input list
     Sleep 1
     NewListEntry.SetText  sItem1
     AddListEntry.Click
     NewListEntry.SetText  sItem2
     AddListEntry.Click
     Sleep 1
     Einfuegen.Click
     Call wTypeKeys "<Return>"
     Kontext "TabFunktionen"
     MoveUpEntry.Click
     Einfuegen.Click
   TabFunktionen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sItem1 then
       Warnlog "Should get " & sItem1 & " but get " & fGetFieldContent()
   end if
   Call wTypeKeys "<Down><Home>"
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sItem2 then
       Warnlog "Should get " & sItem2 & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_12

  Dim sItem1 as String
  Dim sItem2 as String

  sItem1 = "First" : sItem2 = "Second"

  printlog "Input list - Move down"
 '/// <b> Input list Move down</b>

  Call hNewDocument

  '/// Insert/Fields/Other/Functions / Select "Input list" ,
  '/// + insert 2 itmes , press move down
   Call fInsertFieldsOther("TabFunktionen")
     Feldtyp.Select 2   'Input list
     Sleep 1
     NewListEntry.SetText  sItem1
     AddListEntry.Click
     NewListEntry.SetText  sItem2
     AddListEntry.Click
     Sleep 1
     Einfuegen.Click
     Call wTypeKeys "<Return>"
     Kontext "TabFunktionen"
     EntrysList.Select 1
     MoveDownEntry.Click
     Einfuegen.Click
   TabFunktionen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sItem1 then
       Warnlog "Should get " & sItem1 & " but get " & fGetFieldContent()
   end if
   Call wTypeKeys "<Down><Home>"
   Call wTypeKeys "<Shift Right>"
   if fGetFieldContent() <> sItem2 then
       Warnlog "Should get " & sItem2 & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tFunctions_13
Warnlog "#i102752# - tFunctions_13 outcommented due to bug."
goto endsub

    Dim sCombineCharacter as String
    Dim sTestFile as String

    sCombineCharacter = "ABC"
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\fields\CombineTest.sxw")

    printlog "Combine Character"
    '/// <b> Combine Character </b>

    Call hNewDocument

    '/// Open test file pageTest.sxw
    Call hFileOpen(sTestFile)
    Call sMakeReadOnlyDocumentEditable
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Right>"

    '/// Insert/Fields/Other/Functions / Select "Combine characters"
    Call fInsertFieldsOther("TabFunktionen")
    Feldtyp.Select 6   'Combine characters
    Sleep 1
    Namefeld.SetText sCombineCharacter
    Einfuegen.Click
    TabFunktionen.Close

    '/// Check if the insertion is correct
    Call wTypeKeys "<Shift Left>"
    if fGetFieldContent() <> sCombineCharacter then
        Warnlog "Should get " & sCombineCharacter & " but get " & fGetFieldContent()
    end if

    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if StrToDouble(Horizontalby.Gettext) < 6.5 OR StrToDouble(Horizontalby.Gettext) > 7.5 then
        Warnlog "Something wrong in combine character"
    end if
    TabPositionAndSizeWriter.Cancel

    Call hCloseDocument

endcase

'-----------------------------------------------------------------
