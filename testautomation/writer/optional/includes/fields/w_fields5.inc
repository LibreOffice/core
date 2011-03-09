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
'* owner : helge.delfs@oracle.com
'*
'* short description : Fields - Variables
'*
'\***********************************************************************

sub w_fields5

    Call tVariablesSetValueToText
    Call tVariablesSetValueToNumber
    Call tVariablesSetFormats
    Call tVariablesShowVariable1
    Call tVariablesShowVariable2
    Call tVariablesInsertFormula
    Call tVariablesNumberRange	
    Call tVariablesInputfield
    Call tVariablesUserfield
    Call tVariablesDDEFieldManual
    Call tVariablesDDEFieldAutomatic

end sub

'-----------------------------------------------------------------

testcase tVariablesSetValueToText
    if bAsianLan = true then
        QaErrorLog "No testing with Asian language. Aborting."
        exit sub
    end if

  Dim sName()   as String
  Dim sValue    as String
  Dim sResult() as String
  Dim i         as Integer
  Dim j         as Integer

  sValue  = "This is a test!"

  sName   = Array( _
                   "myTest1" , _
                   "myTest2" , _
                   "myTest3" , _
                   "myTest4" , _
                   "myTest5" , _
                   "myTest6" , _
                  )
  sResult = Array( _
                   "This is a test!" , _
                   "1"               , _
                   "1"               , _
                   "1" & gSeperator & "00"            , _
                   "1"               , _
                   "1" & gSeperator & "00"            , _
                  )
  j = UBound(sResult)

  printlog "Set value to text"
 '/// <b> Set value to text </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "set Variable" ,
  '/// Set Name and Value(text) , then choose Text , General ,
  '/// + -1234 , -1234.12 , -1,234 , -1,234,12 separately
   Call fInsertFieldsOther("TabVariablen")
     for i = 0 to j
         Feldtyp.Select 1
         Sleep 1
         NameText.SetText sName(i)
         Wert.SetText sValue
         Zahlenformat.Select i+1
         Einfuegen.Click
         sleep 1
         Call wTypeKeys "<Return>"
         Kontext "TabVariablen"
     next i
     Kontext "TabVariablen"
   TabVariablen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"

   for i = 0 to j
       Call wTypeKeys "<Shift Right>"
       if fGetFieldContent() <> sResult(i) then
           Warnlog i & "Should get " & sResult(i) & " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Home>"
   next i

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesSetValueToNumber
    if bAsianLan = true then
        QaErrorLog "No testing with Asian language. Aborting."
        exit sub
    end if

  Dim sName()   as String
  Dim sValue    as String
  Dim sResult() as String
  Dim i         as Integer
  Dim j         as Integer

  sValue  = "1234,567"

  sName   = Array( _
                   "myTest1" , _
                   "myTest2" , _
                   "myTest3" , _
                   "myTest4" , _
                   "myTest5" , _
                   "myTest6" , _
                  )
  sResult = Array( _
                   "1234,567" , _
                   "1234,57"  , _
                   "1235"     , _
                   "1234,57"  , _
                   "1.235"    , _
                   "1.234,57" , _
                  )
  j = UBound(sResult)

  printlog "Set value to number"
 '/// <b> Set value to number </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "set Variable" ,
  '/// Set Name and Value(number) , then choose Text , General ,
  '/// + -1234 , -1234.12 , -1,234 , -1,234,12 separately
   Call fInsertFieldsOther("TabVariablen")
     for i = 0 to j
         Feldtyp.Select 1
         Sleep 1
         NameText.SetText sName(i)
         Wert.SetText sValue
         Zahlenformat.Select i+1
         Einfuegen.Click
         sleep 1
         Call wTypeKeys "<Return>"
         Kontext "TabVariablen"
     next i
     Kontext "TabVariablen"
   TabVariablen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"

   for i = 0 to j
       Call wTypeKeys "<Shift Right>"
       if fGetFieldContent() <> sResult(i) then
           Warnlog i & "Should get " & sResult(i) & " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Home>"
   next i

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesSetFormats

  Dim sName     as String
  Dim sValue    as String
  Dim iFormat() as Integer
  Dim sResult() as String
  Dim i         as Integer
  Dim j         as Integer
  Dim mUnit     as String

  sName   = "myTest"
  sValue  = "1234.567"
  
  iFormat = Array( 8,         10          , 52         , 55       , 56)
  sResult = Array( _
                   "123456.70%"  , _
                   "$1,234.57"   , _
                   "1.23E+003"   , _
                   "1234 55/97"  , _
                   "TRUE"        , _
                  )
  j = UBound(sResult)

  printlog "Set format as 'Additional formats...'"
 '/// <b> Set format as 'Additional formats...' </b>

  Call hNewDocument
  CALL GetLanguageText (01)
  mUnit = fSetMeasurementToCM()                 ' Set measurement unit to cm
  
  '/// Insert/Fields/Other/Variables / Select "set Variable" ,
  '/// Set Name and Value(number) , then choose format as 'Additional formats...'
  '/// In the number format dialog , select 'All' in Category
  '/// + Select some different formats ...
   Call fInsertFieldsOther("TabVariablen")
     Feldtyp.Select 1
     Sleep 1
     NameText.SetText sName
     Wert.SetText sValue

     for i = 0 to j
         Zahlenformat.Select Zahlenformat.GetItemCount
         Sleep 3
         Kontext "Zahlenformat"
           Sprache.Select fGetLanguageName(01)
           Sleep 2
           Kategorie.Select 1
           Sleep 1
           KategorieFormat.Select iFormat(i)
           Sleep 1
         Zahlenformat.OK

         Kontext "TabVariablen"
           Einfuegen.Click
           Call wTypeKeys "<Shift Left>"
           sleep 1
           if fGetFieldContent() <> sResult(i) then
	           Warnlog i & ": Should get " & sResult(i) & " but get " & fGetFieldContent()
           end if
         Kontext "TabVariablen"
     next i
   TabVariablen.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesShowVariable1

  Dim sName()   as String
  Dim sResult() as String
  Dim i         as Integer
  Dim j         as Integer

  sName   = Array( _
                   "myTest1" , _
                   "myTest2" , _
                   "myTest3" , _
                  )
  sResult = Array( _
                   "1"     , _
                   "10"    , _
                   "100"   , _
                  )
  j = UBound(sResult)

  printlog "Show Variable - 1"
 '/// <b> Show Variable - 2</b>

  Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "set Variable" ,
  '/// Set Name and Value , then choose General , add 3 variables
  '/// + then select Show Variable , insert 3 variables
   Call fInsertFieldsOther("TabVariablen")
     for i = 0 to j
         Feldtyp.Select 1
         Sleep 1
         NameText.SetText sName(i)
         Wert.SetText sResult(i)
         Zahlenformat.Select 2
         Einfuegen.Click
         sleep 1
     next i
     for i = 0 to j
         Call wTypeKeys "<Return>"
         Kontext "TabVariablen"
         Feldtyp.Select 2
         Sleep 1
         Auswahl.Select i+1
         Einfuegen.Click
         sleep 1
     next i
   TabVariablen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<MOD1 Home>"

   for i = 0 to j
       Call wTypeKeys "<Down><Home>"
       Call wTypeKeys "<Shift Right>"
       if fGetFieldContent() <> sResult(i) then
           Warnlog "Should get " & sResult(i) & " but get " & fGetFieldContent()
       end if
   next i

  '/// Delete the 1st line
   Call wTypeKeys "<MOD1 Home><Shift End><Delete>"

   Call wTypeKeys "<MOD1 Home>"

   for i = 0 to j
       Call wTypeKeys "<Down><Home>"
       Call wTypeKeys "<Shift Right>"
       if fGetFieldContent() <> "0" then
           Warnlog i & "Should get 0 but get " & fGetFieldContent()
       end if
   next i

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesShowVariable2

  Dim sName     as String
  Dim sValue    as String
  Dim sResult() as String
  Dim i         as Integer
  Dim j         as Integer

  sResult = Array( _
                   "100"   , _
                   "100"   , _
                   "200"   , _
                   "200"   , _
                     )
  j = UBound(sResult)

  sName = "myTest"

  printlog "Show Variable - 2"
 '/// <b> Show Variable - 2 </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "set Variable" ,
  '/// Set Name and Value , then choose General , insert a variable
   Call fInsertFieldsOther("TabVariablen")
     sValue = "100"
     Feldtyp.Select 1
     Sleep 1
     NameText.SetText sName
     Wert.SetText sValue
     Zahlenformat.Select 2
     Einfuegen.Click

    '/// Then select Show Variable , insert this variable
     Call wTypeKeys "<Return>"
     Kontext "TabVariablen"
     Feldtyp.Select 2
     Sleep 1
     Auswahl.Select 1
     Einfuegen.Click

    '/// Insert a same name variable again , but set to another number
     Call wTypeKeys "<Return>"
     sValue = "200"
     Kontext "TabVariablen"
     Feldtyp.Select 1
     Sleep 1
     NameText.SetText sName
     Wert.SetText sValue
     Zahlenformat.Select 2
     Einfuegen.Click

    '/// Then select Show Variable , insert this variable again
     Call wTypeKeys "<Return>"
     Kontext "TabVariablen"
     Feldtyp.Select 2
     Sleep 1
     Auswahl.Select 1
     Einfuegen.Click
   TabVariablen.Close

  '/// Check if the insertion is correct
   Call wTypeKeys "<F9><MOD1 Home>"

   for i = 0 to j
       Call wTypeKeys "<Shift Right>"
       if fGetFieldContent() <> sResult(i) then
           Warnlog i & "Should get " & sResult(i) & " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Home>"
   next i

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesInsertFormula

  Dim sName()   as String
  Dim sValue()  as String
  Dim sResult   as String
  Dim i         as Integer
  Dim j         as Integer

  sName   = Array( _
                   "myTest1" , _
                   "myTest2" , _
                  )
  sValue  = Array( _
                   "10"    , _
                   "100"   , _
                  )
  sResult = "110"
  j = UBound(sName)

  printlog "Insert formula"
 '/// <b> Insert formula </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "set Variable" , add 2 variables - A and B
   Call fInsertFieldsOther("TabVariablen")
     for i = 0 to j
         Feldtyp.Select 1
         Sleep 1
         NameText.SetText sName(i)
         Wert.SetText sValue(i)
         Zahlenformat.Select 2
         Einfuegen.Click
         sleep 1
     next i

    '/// Then select 'Insert formula' , A+B
     Call wTypeKeys "<Return>"
     Kontext "TabVariablen"
       Feldtyp.Select 4
       Wert.SetText (sName(0) & "+" & sName(1))
       Einfuegen.Click
   TabVariablen.Close

   Call wTypeKeys "<Shift Left>"
   if fGetFieldContent() <> sResult then
       Warnlog "Should get " & sResult & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesNumberRange

  Dim sResult() as String
  Dim i         as Integer
  Dim x         as Integer

  printlog "Number range"
 '/// <b> Number range </b>

  Call hNewDocument

   Call wTypeKeys "<Return>"

  '/// Insert/Fields/Other/Variables / Select "Number range" , insert 4 numbers for
  '/// + Table , text , drawing and illustration
   Call fInsertFieldsOther("TabVariablen")
     for i = 1 to 4
         Feldtyp.Select 6
         Sleep 1
         Auswahl.Select i
         Einfuegen.Click
         Call wTypeKeys "<Return>"
         Kontext "TabVariablen"
         sleep 1
     next i

    '/// Then insert Table , text , drawing and illustration separately
    '/// + Check if the result is correct
     Call wTypeKeys "<MOD1 Home>"
     Kontext "TabVariablen"

     for i = 1 to 4
         Feldtyp.Select 6
         Sleep 1
         Auswahl.Select i
         Einfuegen.Click
         Sleep 2
         Call wTypeKeys "<Home>"
         sResult = Array("1" , "1" , "1" , "1" , "1")
         sResult(i) = "2"
         for x = 0 to 4
             Call wTypeKeys "<Shift Right>"
             if fGetFieldContent() <> sResult(x) then
                 Warnlog "Should get " & sResult(x) & " but get " & fGetFieldContent()
             end if
             Call wTypeKeys "<Down><Home>"
         next x
         Call wTypeKeys "<MOD1 Home><Shift End><Delete>"
         sleep 1
         Kontext "TabVariablen"
     next i
   TabVariablen.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesInputfield

  Dim sName         as String
  Dim sValue        as String
  Dim sReference    as String
  Dim sFieldContent as String

  sName = "myTest" : sValue = "100"
  sReference    = "NewTest"
  sFieldContent = "10000"

  printlog "Input field"
 '/// <b> Input field </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "set Variable" , add 1 variable - A
   Call fInsertFieldsOther("TabVariablen")
     Feldtyp.Select 1
     Sleep 1
     NameText.SetText sName
     Wert.SetText sValue
     Zahlenformat.Select 2
     Einfuegen.Click
     Call wTypeKeys "<Return>"
     Sleep 1

  '/// Select "Input field" , set reference , press insert ,
  '/// + a dialog should pop up , insert field content
  '/// + Table , text , drawing and illustration
     Kontext "TabVariablen"
     Feldtyp.Select 5
     Wert.SetText sReference
     Einfuegen.Click
     Kontext "Eingabefeld"
       if Titel.GetText <> sReference then
           Warnlog "Reference should be " & sReference & " but get " & Titel.GetText
       end if
       EingabeText.SetText sFieldContent
     Eingabefeld.OK

     Kontext "TabVariablen"
   TabVariablen.Close

  '/// Check if the insertion is correct
     Call wTypeKeys "<MOD1 Home>"
     Call wTypeKeys "<Shift Right>"
     if fGetFieldContent() <> sValue then
         Warnlog "Should get " & sValue & " but get " & fGetFieldContent()
     end if

     Call wTypeKeys "<Down><Home>"
     Call wTypeKeys "<Shift Right>"
     if fGetFieldContent() <> sFieldContent then
         Warnlog "Should get " & sFieldContent & " but get " & fGetFieldContent()
     end if

  '/// Delete variable A in the document , Check if the insertion is still correct
     Call wTypeKeys "<MOD1 Home>"
     Call wTypeKeys "<Shift Right><Delete>"

     Call wTypeKeys "<Down><Home>"
     Call wTypeKeys "<Shift Right>"
     if fGetFieldContent() <> sFieldContent then
         Warnlog "2-Should get " & sFieldContent & " but get " & fGetFieldContent()
     end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesUserfield

  Dim sName()    as String
  Dim sValue()   as String
  Dim sResult1() as String
  Dim sResult2() as String
  Dim i          as Integer
  Dim j          as Integer

  sName    = Array("U1" , "UserField" , "U2")
  sValue   = Array("UserField - 10" , "100" , "UserField + 10")
  sResult1 = Array("-10" , "100" , "110")
  sResult2 = Array("90"  , "100" , "110")
  j = UBound(sResult1)

  printlog "User field"
 '/// <b> User field </b>

  Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "User Field"
   Call fInsertFieldsOther("TabVariablen")
   for i = 0 to j
       Feldtyp.Select 9
       Sleep 1
       NameText.SetText sName(i)
       Wert.SetText sValue(i)
       Zahlenformat.Select 3
       Einfuegen.Click
       Sleep 2
       Call wTypeKeys "<Shift Left>"
       if fGetFieldContent() <> sResult1(i) then
           Warnlog "Should get " & sResult1(i) & " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<End><Return>"
       sleep 1
       Kontext "TabVariablen"
   next i
   TabVariablen.Close

  '/// Check if the result is OK.
   Call wTypeKeys "<F9>"
   Call wTypeKeys "<MOD1 Home>"
   Sleep 1
   for i = 0 to j
       Call wTypeKeys "<Shift Right>"
       if fGetFieldContent() <> sResult2(i) then
           Warnlog "Should get " & sResult2(i) & " but get " & fGetFieldContent()
       end if
       Call wTypeKeys "<Down><Left>"
   next i

   Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tVariablesDDEFieldManual
    qaerrorlog "#i112079#testcase must be reworked"
    goto endsub

    Dim sDDEFile        as String
    Dim sContent        as String
    Dim sName           as String
    Dim sDDEStatement   as String
    Dim sSectionName    as String

    sDDEFile = Convertpath ( gOfficePath + "\user\work\dde_test.odt")
    sContent = "DDE"
    sName    = "myTest"
    sSectionName  = "Section1"
    sDDEStatement = "soffice " & sDDEFile & " " & sSectionName

    printlog "DDE field - manual"
    '/// <b> DDE field - manual </b>

    if FileExists ( sDDEFile ) then app.kill ( sDDEFile )

    Call hNewDocument

    '/// Input something , insert a section , then save it and close the file
    Call wTypeKeys sContent
    Call wTypeKeys "<Shift Home>"
    Call fInsertSection("TabBereiche")
    Bereichsliste.Settext sSectionName
    TabBereiche.OK
    Sleep 1
    Call hFileSaveAsKill(sDDEFile)
    Call hCloseDocument

    '/// Opan a new document
    Call hNewDocument
    '/// Insert/Fields/Other/Variables / Select "DDE Field"
    '/// Set DDE name and DDE statement , select "DDE manual"
    Call fInsertFieldsOther("TabVariablen")
    Feldtyp.Select 3
    Sleep 1
    NameText.SetText sName
    Wert.SetText sDDEStatement
    Formatliste.Select 1
    Einfuegen.Click
    Sleep 2
    TabVariablen.Close
    Sleep 2

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    if fGetFieldContent() <> sContent then
        Warnlog "Should get " & sContent & " but get " & fGetFieldContent()
    end if

    '/// Open the test file which you just closed , add something ,
    '/// Save the file
    hFileOpenLocally( sDDEFile)
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "XYZ"
    FileSave
    Call hCloseDocument

    '/// Check if the content is changed -- Should NOT
    Sleep 2
    Call wTypeKeys "<F9>"
    Call wTypeKeys "<MOD1 Home><Shift End>"
    if fGetFieldContent() <> sContent then
        Warnlog "Should get " & sContent & " but get " & fGetFieldContent()
    end if

    '/// Edit/Fields , press update button
    EditFields
    Kontext "VerknuepfungenBearbeiten"
    Aktualisieren.Click
    VerknuepfungenBearbeiten.Close

    '/// Check if the content is changed -- Should NOT
    Sleep 1
    Call wTypeKeys "<F9>"
    Call wTypeKeys "<MOD1 Home><Shift End>"
    if fGetFieldContent() <> ("XYZ" & sContent) then
        Warnlog "Should get XYZ" & sContent & " but get " & fGetFieldContent()
    end if

    Call hCloseDocument

    if FileExists( sDDEFile ) then app.kill ( sDDEFile )

endcase

'-----------------------------------------------------------------

testcase tVariablesDDEFieldAutomatic
    qaerrorlog "#i112079#testcase must be reworked"
    goto endsub

    qaerrorlog "#i102792# - tVariablesDDEFieldAutomatic outcommented. Fields not updated."
    goto endsub

  Dim sDDEFile      as String
  Dim sContent      as String
  Dim sName         as String
  Dim sDDEStatement as String
  Dim sSectionName  as String

  sDDEFile = Convertpath ( gNetzOfficePath + "dde_test.oxt")
  sContent = "DDE"
  sName    = "myTest"
  sSectionName  = "Section1"
  sDDEStatement = "soffice " & sDDEFile & " " & sSectionName

  printlog "DDE field - automatic"
 '/// <b> DDE field - automatic </b>

  if FileExists ( sDDEFile ) then app.kill ( sDDEFile )

  Call hNewDocument

  '/// Input something , insert a section , then save it and close the file
   Call wTypeKeys sContent
   Call wTypeKeys "<Shift Home>"
   Call fInsertSection("TabBereiche")
     Bereichsliste.Settext sSectionName
   TabBereiche.OK
   Sleep 1
   Call hFileSaveAsKill(sDDEFile)
  Call hCloseDocument

  '/// Opan a new document
   Call hNewDocument

  '/// Insert/Fields/Other/Variables / Select "DDE Field"
  '/// Set DDE name and DDE statement , select "DDE automatic"
   Call fInsertFieldsOther("TabVariablen")
     Feldtyp.Select 3
     Sleep 1
     NameText.SetText sName
     Wert.SetText     sDDEStatement
     Formatliste.Select 2
     Einfuegen.Click
     Sleep 2
   TabVariablen.Close
   Sleep 2

   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   if fGetFieldContent() <> sContent then
       Warnlog "Should get " & sContent & " but get " & fGetFieldContent()
   end if

  '/// Open the test file which you just closed , add something ,
  '/// Save the file
   hFileOpenLocally( sDDEFile)
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "XYZ"
    FileSave
   Call hCloseDocument

  '/// Check if the content is changed -- Should
   Sleep 2
   Call wTypeKeys "<F9>"
   Call wTypeKeys "<MOD1 Home><Shift End>"
   if fGetFieldContent() <> ("XYZ" & sContent) then
       Warnlog "Should get XYZ" & sContent & " but get " & fGetFieldContent()
   end if

  Call hCloseDocument

  if FileExists ( sDDEFile ) then app.kill ( sDDEFile )

endcase

'-----------------------------------------------------------------
