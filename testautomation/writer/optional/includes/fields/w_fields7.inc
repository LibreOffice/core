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
'* short description : Fields - L10N test
'*
'\***********************************************************************

sub w_fields7

    Call tL10NtestAboveBelow
    Call tL10NtestDate
    Call tL10NtestTime

end sub

'-----------------------------------------------------------------

testcase tL10NtestAboveBelow

    if iSprache <> 1 then
        QaErrorLog "Currently no any other language than English. Aborting."
        exit sub
    end if
    
    Dim sTestFile as String
    Dim sAbove() as String
    Dim sBelow() as String
    Dim iCountry() as Integer
    Dim i as Integer
    Dim j as Integer
    
    Dim sUILanguage as integer, sLocale as integer
    Dim sCurrency as integer, sWestern as integer, sAsian as integer, sCTL as integer
    Dim sAsiancb as boolean, sCTLcb as boolean
    Dim sAsiancbChecked as boolean, sCTLcbChecked as boolean
    
    if iSprache <> 01 then
        QAErrorlog "This is a English-only testcase"
    end if
    
    Call hNewDocument
    ' Get current settings on language tabpage and remember
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    sUILanguage = UILanguage.GetSelIndex
    sLocale = Gebietsschema.GetSelIndex
    sCurrency = StandardWaehrung.GetSelIndex
    sWestern = Westlich.GetSelIndex
    if Asiatisch.IsEnabled then sAsian = Asiatisch.GetSelIndex
    if LanguageComplexScript.IsEnabled then sCTL = LanguageComplexScript.GetSelIndex
    sAsiancb = Aktivieren.IsEnabled
    sCTLcb = ComplexScriptEnabled.IsEnabled
    sAsiancbChecked = Aktivieren.IsChecked
    sCTLcbChecked = ComplexScriptEnabled.IsChecked
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel
    Call hCloseDocument
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\fields\AboveBelow.sxw")
    
    sBelow  = Array( _
               "below"    , _
               "infra"    , _
               "abajo"    , _
               "sotto"    , _
               "ovan"     , _
               "unten"    , _
               "abaixo"   , _
               "後述"    , _
               "아래"    , _
               "下面"    , _
               "下面"    , _
              )
    sAbove  = Array( _
               "above"    , _
               "supra"    , _
               "arriba"   , _
               "sopra"    , _
               "nedan"    , _
               "oben"     , _
               "acima"    , _
               "前述"    , _
               "위"       , _
               "上面"    , _
               "上面"    , _
              )
    iCountry = Array ( 1 , 33 , 34 , 39 , 46 , 49 , _
                 55, 81 , 82 , 86 , 88 )
    
    printlog "L10N test - above/below"
    '/// <b> L10N test - above/below </b>
    
    Call hNewDocument
    
    '/// Open test file AboveBelow.sxw
    hFileOpenLocally( sTestFile )
    
    
    '/// Check if reference above/below is changed after language is changed
    
    j = UBound(iCountry)
    for i = 0 to j
        Call wTypeKeys "<MOD1 Home>"
        fSetLocale(fGetLanguageName(iCountry(i)))
        if iCountry(i) < 80 then
            fSetWestern(fGetLanguageName(iCountry(i)))
            fSetAsian("Null")
        else
            fSetWestern("Null")
            fSetAsian(fGetLanguageName(iCountry(i)))
        end if
    
        Call wTypeKeys "<F9>"
        Call wTypeKeys "<Shift Right>"
        if fGetFieldContent() <> sBelow(i) then
            Warnlog "Should get " & sBelow(i) & " but get " & fGetFieldContent()
        end if
        Call wTypeKeys "<MOD1 End>"
        Call wTypeKeys "<Shift Left>"
        if fGetFieldContent() <> sAbove(i) then
            Warnlog i & " - Should get " & sAbove(i) & " but get " & fGetFieldContent()
        end if
    next i
    
    ' set back language tabpage values
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    UILanguage.Select sUILanguage
    Gebietsschema.Select sLocale
    StandardWaehrung.Select sCurrency
    Westlich.Select sWestern
    if sAsiancb = true then
        if sAsiancbChecked = true then Aktivieren.IsChecked
    end if
    
    if sCTLcb = true then
        if sCTLcbChecked = true then ComplexScriptEnabled.IsChecked
    end if

    if Asiatisch.IsEnabled and sAsian > 0 then Asiatisch.Select sAsian
    if LanguageComplexScript.IsEnabled and sCTL > 0 then LanguageComplexScript.Select sCTL

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tL10NtestDate
    if iSprache <> 1 then
        QaErrorLog "Currently no any other language than English. Aborting."
        exit sub
    end if
    
    Dim sTestFile    as String
    Dim sDate()      as String
    Dim iCountry()   as Integer
    Dim i , j        as Integer

    Dim sUILanguage as integer, sLocale as integer
    Dim sCurrency as integer, sWestern as integer, sAsian as integer, sCTL as integer
    Dim sAsiancb as boolean, sCTLcb as boolean
    Dim sAsiancbChecked as boolean, sCTLcbChecked as boolean
    
    if iSprache <> 01 then
        QAErrorlog "This is a English-only testcase"
        goto endsub
    end if
    
    Call hNewDocument
    ' Get current settings on language tabpage and remember
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    sUILanguage = UILanguage.GetSelIndex
    sLocale = Gebietsschema.GetSelIndex
    sCurrency = StandardWaehrung.GetSelIndex
    sWestern = Westlich.GetSelIndex
    if Asiatisch.IsEnabled then sAsian = Asiatisch.GetSelIndex
    if LanguageComplexScript.IsEnabled then sCTL = LanguageComplexScript.GetSelIndex
    sAsiancb = Aktivieren.IsEnabled
    sCTLcb = ComplexScriptEnabled.IsEnabled
    sAsiancbChecked = Aktivieren.IsChecked
    sCTLcbChecked = ComplexScriptEnabled.IsChecked
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel
    Call hCloseDocument
  
	sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\fields\Date.sxw")
    sDate  = Array( _
           "Tuesday, August 3, 2004"    , _
           "mardi 3 août 2004"    , _
           "martes, 3 de agosto de 2004"    , _
           "martedì 3 agosto 2004"    , _
           "tisdag 3 augusti 2004"    , _
           "Dienstag, 3. August 2004"    , _
           "Terça-feira, 3 de Agosto de 2004"    , _
           "2004年8月3日（火曜日）"    , _
           "04-8-3 화요일"    , _
           "2004年8月3日 星期二"    , _
           "中華民國93年8月3日 星期二"    , _
          )
    iCountry = Array ( 1 , 33 , 34 , 39 , 46 , 49 , _
             55, 81 , 82 , 86 , 88 )

    printlog "L10N test - Date"
    '/// <b> L10N test - Date </b>
    
    Call hNewDocument
    
    '/// Open test file Date.sxw
    hFileOpenLocally( sTestFile )
    
    '/// Check if Date is changed after language is changed
    j = UBound(iCountry)
    for i = 0 to j
        Call wTypeKeys "<MOD1 Home>"
        fSetLocale(fGetLanguageName(iCountry(i)))
        if iCountry(i) < 80 then
            fSetWestern(fGetLanguageName(iCountry(i)))
            fSetAsian("Null")
        else
            fSetWestern("Null")
            fSetAsian(fGetLanguageName(iCountry(i)))
        end if
    
        Call wTypeKeys "<F9>"
        Call wTypeKeys "<Shift Right>"
        if fGetFieldContent() <> sDate(i) then
            Warnlog i & " - Should get " & sDate(i) & " but get " & fGetFieldContent()
        end if
    next i
    
    ' set back language tabpage values
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    UILanguage.Select sUILanguage
    Gebietsschema.Select sLocale
    StandardWaehrung.Select sCurrency
    Westlich.Select sWestern
    if sAsiancb = true then
        if sAsiancbChecked = true then Aktivieren.IsChecked
    end if
    
    if sCTLcb = true then    
        if sCTLcbChecked = true then ComplexScriptEnabled.IsChecked
    end if

    if Asiatisch.IsEnabled and sAsian > 0 then Asiatisch.Select sAsian
    if LanguageComplexScript.IsEnabled and sCTL > 0 then LanguageComplexScript.Select sCTL

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tL10NtestTime
    if iSprache <> 1 then
        QaErrorLog "Currently no any other language than English. Aborting."
        goto endsub
    end if

    Dim sTestFile as String
    Dim sTime() as String
    Dim iCountry() as Integer
    Dim i as Integer
    Dim j as Integer

    Dim sUILanguage as integer, sLocale as integer
    Dim sCurrency as integer, sWestern as integer, sAsian as integer, sCTL as integer
    Dim sAsiancb as boolean, sCTLcb as boolean
    Dim sAsiancbChecked as boolean, sCTLcbChecked as boolean
    
    if iSprache <> 01 then
        QAErrorlog "This is a English-only testcase"
        goto endsub
    end if
    
    Call hNewDocument
    ' Get current settings on language tabpage and remember
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    sUILanguage = UILanguage.GetSelIndex
    sLocale = Gebietsschema.GetSelIndex
    sCurrency = StandardWaehrung.GetSelIndex
    sWestern = Westlich.GetSelIndex
    if Asiatisch.IsEnabled then sAsian = Asiatisch.GetSelIndex
    if LanguageComplexScript.IsEnabled then sCTL = LanguageComplexScript.GetSelIndex
    sAsiancb = Aktivieren.IsEnabled
    sCTLcb = ComplexScriptEnabled.IsEnabled
    sAsiancbChecked = Aktivieren.IsChecked
    sCTLcbChecked = ComplexScriptEnabled.IsChecked
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel
    Call hCloseDocument
  

	sTestFile = gTesttoolpath + "writer\optional\input\fields\Time.sxw"
    sTime  = Array( _
               "04:46:08 PM"     , _
               "04:46:08 PM"     , _
               "04:46:08 PM"     , _
               "04.46.08 p."     , _
               "04:46:08 em"     , _
               "04:46:08 nachm." , _
               "04:46:08 PM"     , _
               "午後 4:46:08"  , _
               "오후 4시 46분 8초"    , _
               "下午 04:46:08" , _
               "下午 04:46:08" , _
              )
    iCountry = Array ( 1 , 33 , 34 , 39 , 46 , 49 , _
                 55, 81 , 82 , 86 , 88 )
    
    printlog "L10N test - time"
    '/// <b> L10N test - time </b>
    
    Call hNewDocument
    
    '/// Open test file Time.sxw
    hFileOpenLocally( sTestFile )
    
    '/// Check if time is changed after language is changed
    j = UBound(iCountry)
    for i = 0 to j
        Call wTypeKeys "<MOD1 Home>"
        fSetLocale(fGetLanguageName(iCountry(i)))
        if iCountry(i) < 80 then
            fSetWestern(fGetLanguageName(iCountry(i)))
            fSetAsian("Null")
        else
            fSetWestern("Null")
            fSetAsian(fGetLanguageName(iCountry(i)))
        end if
    
        Call wTypeKeys "<F9>"
        Call wTypeKeys "<Shift Right>"
        if fGetFieldContent() <> sTime(i) then
            Warnlog i & " - Should get " & sTime(i) & " but get " & fGetFieldContent()
        end if
    next i
    
    ' set back language tabpage values
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    UILanguage.Select sUILanguage
    Gebietsschema.Select sLocale
    StandardWaehrung.Select sCurrency
    Westlich.Select sWestern
    if sAsiancb = true then
        if sAsiancbChecked = true then Aktivieren.IsChecked
    end if
    
    if sCTLcb = true then    
        if sCTLcbChecked = true then ComplexScriptEnabled.IsChecked
    end if

    if Asiatisch.IsEnabled and sAsian > 0 then Asiatisch.Select sAsian
    if LanguageComplexScript.IsEnabled and sCTL > 0 then LanguageComplexScript.Select sCTL

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    Call hCloseDocument

endcase

'-----------------------------------------------------------------
