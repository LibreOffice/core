'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'* short description : Writer Table test
'*
'\***********************************************************************

global mUnit as string
global cDecSep as string 'Decimal Seperator init in 'sub w_204_'
global cSep as string 'Seperator for numbers formated as text init in 'sub w_204_'
global gLocaleDefault as string
global gDocumentLanguage as string

sub main

    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool5.inc"
    use "writer\tools\includes\w_tool6.inc"

    use "writer\optional\includes\table\w_204_.inc"
    use "writer\optional\includes\table\w_204a_.inc"
    use "writer\optional\includes\table\w_204b_.inc"

    printlog Chr(13) & "Loading of Include - Files takes: " & Wielange ( StartZeit )

    Printlog ""
    Printlog "----------------------------------------------"
    Printlog "|                           Writer Table Test                           |"
    Printlog "----------------------------------------------"
    Printlog ""

    'Checking for supported language
    if fLocaleString("LocaleText") = "Abortion" then 
        warnlog "This test does not support language " & iSprache
        exit sub
    end if
    'First some settings to verify a stable run
    Call TableConfiguration

    Call hStatusIn("writer","w_table.bas","Writer Table test")
    Call w_204_
    Call w_204a_
    Call w_204b_
    Call hStatusOut

    'Restoring default settings after testrun
    Call RestoreSettings

    Printlog "Duration: "& WieLange ( StartZeit )
    Printlog "Date: " &  Date & "    Time: " & Time
end sub

'---------------------------------------------------------------------------------------------------------------------

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_locale_strings1.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub

sub TableConfiguration
    Call wOptionsUndo("Tabelle")
    Call hNewDocument
    if iSprache = iSystemSprache then
            printlog "Systemlocale matches UI-language"
        else qaerrorlog "Systemlocale does not match UI-language, locale settings have to be adjusted"
            ToolsOptions
            Call hToolsOptions ("LanguageSettings", "Languages")
            gLocaleDefault = Gebietsschema.GetSelText
            Gebietsschema.Select(fLocaleString("LocaleLocaleSettings"))
            Kontext "ExtrasOptionenDlg"
            ExtrasOptionenDlg.OK
    end if
    'Verifiing if documentlanguage matches UI language
    ToolsOptions
    Call hToolsOptions ("LanguageSettings", "Languages")
    if fLocaleString("LocaleScriptType") = "Western" then 
        gDocumentLanguage = Westlich.GetSelText
        if gDocumentLanguage <> iSprache then Westlich.Select(fLocaleString("LocaleLocaleSettings"))
    end if
    if fLocaleString("LocaleScriptType") = "CJK" then 
        gDocumentLanguage = Asiatisch.GetSelText
        if gDocumentLanguage <> iSprache then Asiatisch.Select(fLocaleString("LocaleLocaleSettings"))
    end if
    if fLocaleString("LocaleScriptType") = "CTL" then 
        gDocumentLanguage = LanguageComplexScript.GetSelText
        if gDocumentLanguage <> iSprache then LanguageComplexScript.Select(fLocaleString("LocaleLocaleSettings"))
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    mUnit = fSetMeasurementToCM()
    ToolsOptions
    Call hToolsOptions ("Writer","GENERAL")
    cDecSep = GetDecimalSeperator(Tabulatorenabstand.GetText)
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    ' Seperator to calculate with is not the same as used in UI
    ' so we can't use GetDecimalSeparator here
    Call wTypeKeys ("1/2")
    Call wTypeKeys ("<Shift Home>")
    ToolsCalculate
    if Instr(GetClipBoardtext, ",") > 0 then cSep = ","
    if Instr(GetClipBoardtext, ".") > 0 then cSep = "."
    Call hCloseDocument
end sub

'---------------------------------------------------------------------------------------------------------------------

sub RestoreSettings
    If iSprache <> iSystemSprache then
        Call hNewDocument
        ToolsOptions
        Call hToolsOptions ("LanguageSettings", "Languages")
        Gebietsschema.Select(gLocaleDefault)
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
    end if
    ToolsOptions
    Call hToolsOptions ("LanguageSettings", "Languages")
    if fLocaleString("LocaleScriptType") = "Western" then Westlich.Select(gDocumentLanguage)
    if fLocaleString("LocaleScriptType") = "CJK" then Asiatisch.Select(gDocumentLanguage)
    if fLocaleString("LocaleScriptType") = "CTL" then LanguageComplexScript.Select(gDocumentLanguage)
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    If iSprache <> iSystemSprache then
        Call hCloseDocument
    end if
end sub
