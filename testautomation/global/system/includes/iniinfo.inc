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
'* short description : get information for startup a test out of important ini-files
'*
'\***********************************************************************

sub GetIniInformation
    '/// Set all important variables.
    '/// Some of them are get only out of the </i>testtool.ini</i>.
    '/// <b>gPathSigne</b>: Pathsign for the OfficePLatform Operating System
    '/// <b>gPort</b>: Portnummer for communication between testtool and StarOffice
    '/// <b>gSystemPath</b>: Systempath from where StarOffice executes.
    '/// <b>gOfficePath</b>: Path where StarOffice is installed.
    '/// <b>gTesttoolPath</b>: Root directory from the <i>TestTool environment</i>.
    '/// <b>oTesttoolPath</b>: Root directory from the <i>TestTool environment</i> (<u>Windows styleed only!</u>)
    '/// <b>gClient</b>: <b>1</b> when you run a Server/Client-Test (Sun&trade; ONE Webtop)
    '/// <b>gClientLaden</b>: <b>0</b> if Load/Save on Client-side, otherwise on Server-side (<u>UNIX only!</u>)
    '/// <b>gNetzInst</b>: <b>1</b> if you test a network- / workstation installaion.
    dim sSoffice(3) as string ' array, because they might all get evaluated sometime
    dim bOOo(3) as string
    dim iSoffice(3) as integer
    dim sTemp as string

    gPathSigne = hGetPathSigne(gPLatform)

    gTTProfileName = hfGetTTProfileName()
    gSystemPath = gtSystemPath
    oTesttoolPath = GetIniValue ( gTesttoolIni, gTTProfileName, "BaseDir" )
    if (right(oTesttoolPath,1) <> gPathSigne) then
        oTesttoolPath = oTesttoolPath + gPathSigne
    end if
    gTesttoolPath = oTesttoolPath
    if gSamePC = FALSE then
        gOfficePath = GetIniValue ( gTesttoolIni, "Office IniPath", "Current" ) + gPathSigne
        gRemotePath = GetIniValue ( gTesttoolIni, "RemoteBaseDir", "Current" ) + gPathSigne
    else
        '/// The part of getting the location of the executable OOo is put into seperated functions.
        '/// The first OOo executeable is being used.
        '///+ <ol><li>Entry in TestTool control file (.testtoolrc on UNIX; testtool.ini on Win32)</li>
        '///+ <li>or<ul><li><b>Win32</b>: Entry in registry</li><li><b>UNIX</b>: If link <pre>/usr/bin/soffice</pre> or <pre>$HOME/soffice</pre> exists</li></ul></li></ol>
        sSoffice(0) = getSofficeTesttool() : bOOO(0) = gOOO
        ' DEBUG: If you need more detailed information about the installation environment
        '        enable the next line.
        ' printlog "RESULT from TestTool: '" + sSoffice(0) + "' - OOo? " + bOOO(0)
        if (sSoffice(0) = "") then
            sSoffice(0) = getSofficeNative()   : bOOO(0) = gOOO
            if (sSoffice(0) <> "") then
                gOfficePath = sSoffice(0)
                gVersionsnummer = FindBuildID()
                call hSetBuildVersionInformation(TRUE)
                iSoffice(0) = gBuild
            end if
            ' DEBUG: If you need more detailed information about the installation environment
            '        enable the next line.
            ' printlog "RESULT from Native  : '" + sSoffice(0) + "' - OOo? " + bOOO(0) + " - BuildID: " + iSoffice(0)
            if sSoffice(0) = "" then
                warnlog "No OOo version found on this computer. - Please look into documentation, how to select an OOo version to test."
            end if
        else
            printlog    "----------------------------------------------------------------------------------------------------"
            printlog    "**  Using [OOoProgramDir] value   : " & sSoffice(0)
            printlog    "----------------------------------------------------------------------------------------------------"
        end if
        gOfficePath = sSoffice(0)
        gOOO = bOOO(0)
    end if
    if ((gSamePC = TRUE) AND (gOfficePath = "")) then
        MsgBox ("The test ends, because no OOo version was found on this computer. - Please look into documentation, how to select an OOo version to test.", 1, "No OOo program installed")
        end
    end if

    gtHidLstPath =  GetIniValue ( gTesttoolIni, gTTProfileName, "HIDDir" ) + hGetPathSigne(gtPlatform)
    gPort = GetIniValue ( gTesttoolIni, "Communication", "TTPort" )
    '--------
    gPCName = GetIniValue ( gTesttoolIni, "Others", "PCname" )
    if (gPCName = "NOT_SET!" OR gPCName = "") then
        gPCName = environ ( "HOSTNAME" )
        if (gPCName = "") then
            gPCName = environ ( "COMPUTERNAME" )
            if (gPCName = "") then
                gPCName = environ ( "hostname" )
                ' If testtool.ini/rc is unmodified, the default value is NOT_SET!
                if gPCName = "" then
                    qaErrorLog "Please insert the computername in section [Others] on the line 'PCname=' in the TestTool confguration file at: '" + gTesttoolIni + "'"
                end if
            end if
        end if
    end if
    '--------

    call GetTheInstallationType            ' gNetzInst and gNetzOfficePath will be set if StarOffice is a Network-Installation
    gOfficeBasisPath = getOfficeBasisPath()
    gOOoStartupTimeOut = fgetOOoStartupTimeOut()
    gOOoShutdownTimeOut = fgetOOoShutdownTimeOut()
    call sCheckValgrindStatus()
    ' set a global path for custom scripts outside of current environment
    gPrivateEnvironmentLocation = getIniValue(gTesttoolIni, "PrivateEnvironment", "Current")
    if (gPrivateEnvironmentLocation <> "" AND gPrivateEnvironmentLocation <> ".") then
        ' check if the gPrivateEnvironmentLocation path exists
        if fileExists(gPrivateEnvironmentLocation) then
            ' check if the path ends with a path sign and add the path sign if not
            if (Right(gPrivateEnvironmentLocation,1) <> gPathSigne) then
                gPrivateEnvironmentLocation = gPrivateEnvironmentLocation & gPathSigne
            endif
        else
            ' check if the gTesttoolPath plus the gPrivateEnvironmentLocation path exists
            ' f.e. this is for a relative path
            if fileExists(gTesttoolPath & gPrivateEnvironmentLocation) then
                gPrivateEnvironmentLocation = gTesttoolPath & gPrivateEnvironmentLocation
                ' check if the path ends with a path sign and add the path sign if not
                if (Right(gPrivateEnvironmentLocation,1) <> gPathSigne) then
                    gPrivateEnvironmentLocation = gPrivateEnvironmentLocation & gPathSigne
                endif
            else
                qaErrorLog "[PrivateEnvironment] defined in: '" & gTesttoolIni & "', but the path '" & gPrivateEnvironmentLocation & "'does not exist!"
                gPrivateEnvironmentLocation = ""
            endif
        end if
    else
       gPrivateEnvironmentLocation = ""
   end if
end sub

'-------------------------------------------------------------------------

sub GetOfficeInformation
    '/// Generates the path to the StarOffice executable.
    '/// <u>Output</u>: <b>sAppExe</b> variable is the StarOffice executable or the client executable (StarOffice server, StarPortal&trade;, Sun&trade; ONE Webtop, ...)
    Dim sProgramNeu as String
    Dim iClient as Integer
    Dim sPlatformProgramPath as string
    Dim sPlatformBinExt as string

    if ( lcase( gPlatform ) = "osx" ) then
        sPlatformProgramPath = "MacOS"
        sPlatformBinExt = ""
    else
        sPlatformProgramPath = "program"
        if gPlatGroup <> "unx" then
            sPlatformBinExt = ".exe"
        else
            sPlatformBinExt = ""
        end if
    end if

    'if it's a client on the same PC
    if gClient = TRUE AND gClientUser <> "" then
        if gtPlatform <> "w95" AND gtPlatform <> "wnt" AND gPlatform <> "w2k" AND gtPlatform <> "os2" AND gtPlatform <> "os4" then
            sAppExe = gsClient + "/bin/sclient"
        else
            if right( lcase ( sAppExe ), 3 ) <> "exe" then sAppExe = gsClient + "\sclient.exe"
        end if
        sAppParameter = " -remote -connect=portal,service=soffice,host=" + gHost + ",port=8208,user=" + gClientUser + ",password=" + gClientUserPWD + ",type=compressed_secure"
    else
        if gNetzInst = TRUE then                              ' wenn es sich um eine Netzwerkinstallation handelt
            select case UCase(gApplication)
            case "WRITER"        : sAppExe = gNetzOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                sFactory = "-writer"
            case "CALC"          : sAppExe = gNetzOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                sFactory = "-calc"
            case "IMPRESS"       : sAppExe = gNetzOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                sFactory = "private:factory/simpress"
            case "DRAW"          : sAppExe = gNetzOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                sFactory = "-draw"
            case "MASTERDOCUMENT": sAppExe = gNetzOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                sFactory = "-global"
            case "MATH"          : sAppExe = gNetzOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                sFactory = "-math"
            case "HTML"          : sAppExe = gNetzOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                sFactory = "-web"
            case else            : sAppExe = gNetzOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
            end select
            'If it's a normal (FAT) office
        else
            'If it's a RVP-Office
            if gClient = TRUE then
                if gPlatgroup <> "unx" then
                    sAppExe = gOfficePath + "client\sclient.exe"
                else
                    sAppExe = gOfficePath + "client/sclient"
                end if
            else
                select case ( gApplication )
                case "WRITER"        : sAppExe = gOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                    sFactory = "-writer"
                case "CALC"          : sAppExe = gOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                    sFactory = "-calc"
                case "IMPRESS"       : sAppExe = gOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                    sFactory = "private:factory/simpress"
                case "DRAW"          : sAppExe = gOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                    sFactory = "-draw"
                case "MASTERDOCUMENT": sAppExe = gOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                    sFactory = "-global"
                case "MATH"          : sAppExe = gOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                    sFactory = "-math"
                case "HTML"          : sAppExe = gOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                    sFactory = "-web"
                case else            : sAppExe = gOfficePath & sPlatformProgramPath & gPathSigne & "soffice" & sPlatformBinExt
                end select
            end if
        end if
    end if
end sub

'-------------------------------------------------------------------------

function hGetPathSigne(sPlatform as string) as string
    '/// Seperators for the different platforms.
    '/// <u>Input</u>: string of class gPlatform
    '/// <u>Output</u>: string which can be used as seperator in directories

    select case sPlatform
    case "w95" : hGetPathSigne = "\"
    case "w98" : hGetPathSigne = "\"
    case "wnt" : hGetPathSigne = "\"
    case "wme" : hGetPathSigne = "\"
    case "wse" : hGetPathSigne = "\"
    case "w2k" : hGetPathSigne = "\"
    case "wxp" : hGetPathSigne = "\"
    case "ecs" : hGetPathSigne = "\"
    case "os4" : hGetPathSigne = "\"
    case "os5" : hGetPathSigne = "\"
    case "osx" : hGetPathSigne = "/"
    case "ppc" : hGetPathSigne = "/"
    case "sol" : hGetPathSigne = "/"
    case "lin" : hGetPathSigne = "/"
    case "x86" : hGetPathSigne = "/"
    case "fbsd": hGetPathSigne = "/"
    case "nbsd": hGetPathSigne = "/"
    case "lin64": hGetPathSigne= "/"
    case "linsparc": hGetPathSigne = "/"
    case else  : hGetPathSigne = ""
        qaErrorLog " No pathseperator is defined for this platform : " + sPlatform
    end select
end function

'-------------------------------------------------------------------------

function ConvertPath( sPath as string ) as string

    if ( getPathSeparator = "/" ) then
    	ConvertPath() = hStringReplaceChar( sPath, "\", "/" )
    else
        ConvertPath() = hStringReplaceChar( sPath, "/", "\" )
    endif
    
end function

'-------------------------------------------------------------------------------

function hStringReplaceChar( byval myString as string, old_char as string, new_char as string ) as string

    dim iChar as integer
    for iChar = 1 to len( myString )
        if ( mid( myString, iChar, 1 ) = old_char ) then mid( myString, iChar, 1, new_char )
    next iChar
    hStringReplaceChar() = myString

end function

'-------------------------------------------------------------------------------

sub GetLanguageInformation
    '/// Extracts the language information from <i>Setup.xcu</i>.
    '/// <u>note</u>: It uses the <b>fgetL10Nvalue</b>.
    Dim sLanOutIni as string

    ' function 'fgetL10Nvalue' is also in this library
    sLanOutIni = fgetL10Nvalue()
    ' BugID 98315 -> looking in networkpath for the language until bug will be fixed.

    if sLanOutIni = "" then
        warnlog "=> default is now 01 = en_US"
    end if

    'language Code ISO 639 (and ISO 3166 for en language) is now gLangCode
    'ISO 639 http://www.oasis-open.org/cover/iso639a.html
    'ISO 3166: http://xml.coverpages.org/country3166.html

    gISOLang = sLanOutIni

     select case lcase ( sLanOutIni )
         case "en_us", "en-us", "en" : iSprache = 01        ' English (USA)
         case "pt_pt", "pt-pt", "pt" : iSprache = 03        ' Portuguese
         case "ru_ru", "ru-ru", "ru" : iSprache = 07        ' Russian
         case "el_gr", "el-gr", "el" : iSprache = 30        ' Greek
         case "nl_nl", "nl-nl", "nl" : iSprache = 31        ' Dutch
         case "fr_fr", "fr-fr", "fr" : iSprache = 33        ' French
         case "es_es", "es-es", "es" : iSprache = 34        ' Spanish
         case "fi_fi", "fi-fi", "fi" : iSprache = 35        ' Finnish
         case "hu_hu", "hu-hu", "hu" : iSprache = 36        ' Hungaria
         case "ca_ad", "ca-ad", "ca" : iSprache = 37        ' Catalan
         case "it_it", "it-it", "it" : iSprache = 39        ' Italian
         case "cs_cz", "cs-cz", "cs" : iSprache = 42        ' Czech
         case "sk_sk", "sk-sk", "sk" : iSprache = 43        ' Slowak
         case "en_gb", "en-gb"       : iSprache = 44        ' English (GB)
         case "da_dk", "da-dk", "da" : iSprache = 45        ' Danish
         case "sv_se", "sv-se", "sv" : iSprache = 46        ' Swedish
         case "no_no", "no-no", "no" : iSprache = 47        ' Norwegian
         case "pl_pl", "pl-pl", "pl" : iSprache = 48        ' Polish
         case "de_de", "de-de", "de" : iSprache = 49        ' German
         case "sl_si", "sl-si", "sl" : iSprache = 50        ' Slovenian
		 case "sr_rs", "sr-rs", "sr" : iSprache = 51        ' Serbian (Cyrillic)
         case "sh_rs", "sh-rs", "sh" : iSprache = 52        ' Serbian (Latin)		 
         case "pt_br", "pt-br", "br" : iSprache = 55        ' Portuguese (Brazil)
         case "th_th", "th-th", "th" : iSprache = 66        ' Thai
         case "ja_jp", "ja-jp", "ja" : iSprache = 81        ' Japanese
         case "ko_kr", "ko-kr", "ko" : iSprache = 82        ' Korean
         case "vi_vi", "vi-vi", "vi" : iSprache = 84        ' Vietnamese
         case "zh_cn", "zh-cn", "zh" : iSprache = 86        ' Chinese (simplified)
         case "zh_tw", "zh-tw"       : iSprache = 88        ' Chinese (traditional)
         case "tr_tr", "tr-tr", "tr" : iSprache = 90        ' Turkish
         case "hi_in", "hi-in", "hi" : iSprache = 91        ' Hindi
         case "ar_ar", "ar-ar", "ar" : iSprache = 96        ' Arabic
         case "he_he", "he-he", "he" : iSprache = 97        ' Hebrew
         case "zz_zz", "zz-zz", "zz" : iSprache = 01        ' unknown
                                       warnlog "iniinfo.inc::GetLanguageInformation()"
                                       warnlog "- sLanOutIni = " & sLanOutIni & " (unknown language)"
                                       warnlog "- iSprache has been set 01!"
         case ""                     : iSprache = 01        ' empty
                                       warnlog "iniinfo.inc::GetLanguageInformation()"
                                       warnlog "- empty sLanOutIni = " & sLanOutIni
                                       warnlog "- iSprache has been set 01!"
         case else                   : iSprache = 01
                                       warnlog "iniinfo.inc::GetLanguageInformation()"
                                       warnlog "- sLanOutIni = " & sLanOutIni & " (fallback mode; unknown language)"
                                       warnlog "- iSprache has been set 01!"
     end select

    if iSprache = 81 OR iSprache = 82 OR iSprache = 86 OR iSprache = 88 then
        bAsianLan = TRUE
    else
        bAsianLan = FALSE
    end if
    if ((iSprache = 66) OR (iSprache = 91) OR (iSprache = 96) OR (iSprache = 97)) then
        bCTLLan = TRUE
    else
        bCTLLan = FALSE
    end if
    gLanguage = GetLanguageText ( iSprache )
end sub

'-------------------------------------------------------------------------

function ConvertLanguage ( Sprache as String ) as Integer
    '/// Sets international language code for the whole language name.

   select case lCase ( Sprache )
      case "deutsch", "german"                              : ConvertLanguage = 49
      case "englisch", "english"                            : ConvertLanguage = 01
      case "portugisisch", "portugiesisch", "portuguese"    : ConvertLanguage = 03
      case "russisch", "russian"                            : ConvertLanguage = 07
      case "czech", "tschechisch"                           : ConvertLanguage = 29
      case "griechisch", "greek"                            : ConvertLanguage = 30
      case "niederlaendisch", "hollaendisch", "netherlands" : ConvertLanguage = 31
      case "franzoesisch", "french"                         : ConvertLanguage = 33
      case "spanisch", "spanish"                            : ConvertLanguage = 34
      case "finisch", "finnish"                             : ConvertLanguage = 35
      case "hungaria", "ungarisch"                          : ConvertLanguage = 36
      case "italienisch", "italian"                         : ConvertLanguage = 39
	  case "slowak", "slovakisch"                           : ConvertLanguage = 43	  
      case "daenisch", "danish"                             : ConvertLanguage = 45
      case "schwedisch", "swedish"                          : ConvertLanguage = 46
      case "norwegian", "norwegisch"                        : ConvertLanguage = 47
      case "polnisch", "polish"                             : ConvertLanguage = 48
      case "slovenian", "slovenisch"                        : ConvertLanguage = 50
	  case "serbisch kyrillisch", "serbian cyrillic"        : ConvertLanguage = 51
	  case "serbisch latein", "serbian latin"               : ConvertLanguage = 52	  
      case "japanisch", "japanese"                          : ConvertLanguage = 81
      case "koreanisch", "korean"                           : ConvertLanguage = 82
      case "vietnamesisch", "vietnamese"                    : ConvertLanguage = 84
      case "chinesisch", "chinese"                          : ConvertLanguage = 86
      case "chinesisch trad.", "chinese trad."              : ConvertLanguage = 88
      case "tuerkisch", "turkish"                           : ConvertLanguage = 90
      case "arabisch", "arab"                               : ConvertLanguage = 96
      case "catalan", "katalanisch"                         : ConvertLanguage = 99
   end select
end function

'-------------------------------------------------------------------------

function GetLanguageText ( iLan ) as String
'/// Sets the language name for the international language-code.
   select case iLan
      case 01   : GetLanguageText = "English (USA)"
      case 03   : GetLanguageText = "Portuguese"
      case 07   : GetLanguageText = "Russian"
      case 30   : GetLanguageText = "Greek"
      case 31   : GetLanguageText = "Dutch"
      case 33   : GetLanguageText = "French"
      case 34   : GetLanguageText = "Spanish"
      case 35   : GetLanguageText = "Finnish"
      case 36   : GetLanguageText = "Hungarian"
      case 37   : GetLanguageText = "Catalan"
      case 39   : GetLanguageText = "Italian"
      case 42   : GetLanguageText = "Czech"
      case 43   : GetLanguageText = "Slowak"
      case 44   : GetLanguageText = "English (GB)"
      case 45   : GetLanguageText = "Danish"
      case 46   : GetLanguageText = "Swedish"
      case 47   : GetLanguageText = "Norwegian"
      case 48   : GetLanguageText = "Polnish"
      case 49   : GetLanguageText = "German"
      case 50   : GetlanguageText = "Slovenian"
      case 51   : GetlanguageText = "Serbian (Cyrillic)"
      case 52   : GetlanguageText = "Serbian (Latin)"	  
      case 55   : GetLanguageText = "Portuguese (Brazil)"
      case 66   : GetLanguageText = "Thai"
      case 81   : GetLanguageText = "Japanese"
      case 82   : GetLanguageText = "Korean"
      case 84   : GetLanguageText = "Vietnamese"
      case 86   : GetLanguageText = "Chinese (simplified)"
      case 88   : GetLanguageText = "Chinese (traditional)"
      case 90   : GetLanguageText = "Turkish"
      case 91   : GetLanguageText = "Hindi"
      case 96   : GetLanguageText = "Arabic"
      case 97   : GetLanguageText = "Hebrew"
      case 99   : GetLanguageText = "Unknown"
      case else : GetLanguageText = "detection failed"
   end select
end function

'-------------------------------------------------------------------------

function ConvertLanguage2 ( sSprache as String, optional bExact ) as Integer
'/// Sets the international language code for the international language shortcut.
'/// <i>optional <b>bExact</i></b>: If language not available, 'shut up' and return '0'
   select case lCase ( sSprache )
         case "en_us", "en-us", "en" : ConvertLanguage2 = 01        ' English (USA)
         case "pt_pt", "pt-pt", "pt" : ConvertLanguage2 = 03        ' Portuguese
         case "ru_ru", "ru-ru", "ru" : ConvertLanguage2 = 07        ' Russian
         case "el_gr", "el-gr", "el" : ConvertLanguage2 = 30        ' Greek
         case "nl_nl", "nl-nl", "nl" : ConvertLanguage2 = 31        ' Dutch
         case "fr_fr", "fr-fr", "fr" : ConvertLanguage2 = 33        ' French
         case "es_es", "es-es", "es" : ConvertLanguage2 = 34        ' Spanish
         case "fi_fi", "fi-fi", "fi" : ConvertLanguage2 = 35        ' Finnish
         case "hu_hu", "hu-hu", "hu" : ConvertLanguage2 = 36        ' Hungaria
         case "ca_ad", "ca-ad", "ca" : ConvertLanguage2 = 37        ' Catalan
         case "it_it", "it-it", "it" : ConvertLanguage2 = 39        ' Italian
         case "cs_cz", "cs-cz", "cs" : ConvertLanguage2 = 42        ' Czech
         case "sk_sk", "sk-sk", "sk" : ConvertLanguage2 = 43        ' Slowak
         case "en_gb", "en-gb"       : ConvertLanguage2 = 44        ' English (GB)
         case "da_dk", "da-dk", "da" : ConvertLanguage2 = 45        ' Danish
         case "sv_se", "sv-se", "sv" : ConvertLanguage2 = 46        ' Swedish
         case "no_no", "no-no", "no" : ConvertLanguage2 = 47        ' Norwegian
         case "pl_pl", "pl-pl", "pl" : ConvertLanguage2 = 48        ' Polish
         case "de_de", "de-de", "de" : ConvertLanguage2 = 49        ' German
         case "sl_si", "sl-si", "sl" : ConvertLanguage2 = 50        ' Slovenian
         case "sr_rs", "sr_rs", "sr" : ConvertLanguage2 = 51        ' Serbian (Cyrillic)
         case "sh_rs", "sh_rs", "sh" : ConvertLanguage2 = 52        ' Serbian (Latin)		 
         case "pt_br", "pt-br", "br" : ConvertLanguage2 = 55        ' Portuguese (Brazil)
         case "th_th", "th-th", "th" : ConvertLanguage2 = 66        ' Thai
         case "ja_jp", "ja-jp", "ja" : ConvertLanguage2 = 81        ' Japanese
         case "ko_kr", "ko-kr", "ko" : ConvertLanguage2 = 82        ' Korean
         case "vi_vi", "vi_vi", "vi" : ConvertLanguage2 = 84        ' Vietnamese
         case "zh_cn", "zh-cn", "zh" : ConvertLanguage2 = 86        ' Chinese (simplified)
         case "zh_tw", "zh-tw"       : ConvertLanguage2 = 88        ' Chinese (traditional)
         case "tr_tr", "tr-tr", "tr" : ConvertLanguage2 = 90        ' Turkish
         case "hi_in", "hi-in", "hi" : ConvertLanguage2 = 91        ' Hindi
         case "ar_ar", "ar-ar", "ar" : ConvertLanguage2 = 96        ' Arabic
         case "he_he", "he-he", "he" : ConvertLanguage2 = 97        ' Hebrew
         case "zz_zz", "zz-zz", "zz" : ConvertLanguage2 = 01        ' unknown
         case else                   : if isMissing (bExact) then
                                         printlog "iniinfo.inc::ConvertLanguage2(): Unknown systemlanguage '" & sSprache & "'!"
                                         printlog "Default will be set to 01!"
                                         ConvertLanguage2 = 01
                                        else
                                         printlog "iniinfo.inc::ConvertLanguage2(): Unknown systemlanguage '" & sSprache & "'!"
                                         if bExact = TRUE then
                                           printlog "Default will be set to 00!"
                                           ConvertLanguage2 = 00
                                         end if
                                       end if
   end select
end function

'-------------------------------------------------------------------------

function ConvertCodeToLanguage ( sSprache as String ) as Integer
    '/// Converts the old language code to international language code

   select case lCase ( sSprache )
         case "1033" : ConvertCodeToLanguage = 01        ' English (USA)
         case "2070" : ConvertCodeToLanguage = 03        ' Portuguese
         case "1049" : ConvertCodeToLanguage = 07        ' Russian
         case "1032" : ConvertCodeToLanguage = 30        ' Greek
         case "1043" : ConvertCodeToLanguage = 31        ' Dutch
         case "1036" : ConvertCodeToLanguage = 33        ' French
         case "3082" : ConvertCodeToLanguage = 34        ' Spanish
         case "1034" : ConvertCodeToLanguage = 34        ' Spanish
         case "1035" : ConvertCodeToLanguage = 35        ' Finnish
         case "1038" : ConvertCodeToLanguage = 36        ' Hungaria
         case "2099" : ConvertCodeToLanguage = 37        ' Catalan
         case "1040" : ConvertCodeToLanguage = 39        ' Italian
         case "1029" : ConvertCodeToLanguage = 42        ' Czech
         case "1051" : ConvertCodeToLanguage = 43        ' Slowak
         case "2057" : ConvertCodeToLanguage = 44        ' English (GB)
         case "1030" : ConvertCodeToLanguage = 45        ' Danish
         case "1053" : ConvertCodeToLanguage = 46        ' Swedish
         case "2047" : ConvertCodeToLanguage = 47        ' Norwegian
         case "1045" : ConvertCodeToLanguage = 48        ' Polish
         case "1031" : ConvertCodeToLanguage = 49        ' German
         case "1060" : ConvertCodeToLanguage = 50        ' Slovenian
         case "    " : ConvertCodeToLanguage = 55        ' Portuguese (Brazil)
         case "3098" : ConvertCodeToLanguage = 51        ' Serbian (Cyriliic)
         case "2074" : ConvertCodeToLanguage = 52        ' Serbian (Latin)		 
         case "    " : ConvertCodeToLanguage = 66        ' Thai
         case "1041" : ConvertCodeToLanguage = 81        ' Japanese
         case "1042" : ConvertCodeToLanguage = 82        ' Korean
         case "1066" : ConvertCodeToLanguage = 84        ' Vietnamese
         case "2052" : ConvertCodeToLanguage = 86        ' Chinese (simplified)
         case "1028" : ConvertCodeToLanguage = 88        ' Chinese (traditional)
         case "1055" : ConvertCodeToLanguage = 90        ' Turkish
'         case "    " : ConvertCodeToLanguage = 91        ' Hindi
         case "1025" : ConvertCodeToLanguage = 96        ' Arabic
'         case "    " : ConvertCodeToLanguage = 97        ' Hebrew
         case else   : printlog "iniinfo.inc::ConvertCodeToLanguage(): Num. Code could not be detect or is unknown and will be set to 01 (English (USA))!"
                       ConvertCodeToLanguage = 01
   end select
end function

'-------------------------------------------------------------------------

sub GetTheInstallationType
    '/// If StarOffice is installed as a network installation the global variable 'gNetzInst' and 'gNetzOfficePath' will be set here.
    Dim sCommonXML as String, sDummy as String, sDestPath as String, sSourcePath as String, sMode as String
    Dim FileNum%
    dim sTemp as string
    dim sURL as string
    dim aTemp
    dim aTemp2
    dim aTemp3
    dim i,x as integer
    dim sPlatformProgramPath as string

    if ( lcase( gPlatform ) = "osx" ) then
        sPlatformProgramPath = "MacOS"
    else
        sPlatformProgramPath = "program"
    endif

    if gSamePC = TRUE then
        gNetzInst = TRUE
        gNetzOfficePath = gOfficePath
        if (gSamePC = TRUE) then
            if gPlatgroup = "unx" then
                sTemp = GetIniValue (gOfficePath & sPlatformProgramPath & "/bootstraprc", "Bootstrap", "UserInstallation")
            else
                sTemp = GetIniValue (gOfficePath & sPlatformProgramPath & "\bootstrap.ini", "Bootstrap", "UserInstallation")
            end if
        end if
        sURL = convertFromURL(sTemp) ' to get the % encoded chars decoded! and to remove "file://" if somebody provides a URL
        ' Formerly I had to fake the string with a starting "file://" to use the function; nowadays it works without it
        if inStr(sURL,"$SYSUSERCONFIG") > 0 then
            gOfficePath = gtSystemPath + right(sURL, len(sURL) - (inStr(sURL,"$SYSUSERCONFIG")+len("$SYSUSERCONFIG"))) + gPathSigne
        else
            if inStr(sURL,"$ORIGIN") > 0 then
                gOfficePath = gOfficePath & sPlatformProgramPath & gPathSigne + right(sURL, len(sURL) - (inStr(sURL,"$ORIGIN")+len("$ORIGIN"))) + gPathSigne
            else
                if inStr(sTemp,"file://") > 0 then
                    ' file:// had to get removed on calling convertFromURL() sTemp -> sURL
                    gOfficePath = sURL + gPathSigne
                else
                    if inStr(sURL,"$BRAND_BASE_DIR") > 0 then
                        gOfficePath = gOfficePath & right(sURL, len(sURL) - (inStr(sURL,"$BRAND_BASE_DIR")+len("$BRAND_BASE_DIR")))
                    else
                        qaErrorLog "ininfo.inc::GetTheInstallationType:: Unexpected entry in bootstrap file.(Entry UserInstallation not starting with: $SYSUSERCONFIG or $ORIGIN or file://; Using entry as absolute path!)"
                        printlog "ininfo.inc::GetTheInstallationType:: original UserInstallation entry from 'bootstrap' file: '" + sTemp + "'"
                        printlog "ininfo.inc::GetTheInstallationType:: using now converted from URL for 'gOfficePath': '" + sURL + "'"
                        gOfficePath = sURL + gPathSigne
                    end if
                end if
            end if
        end if
        ' make unique platform path seperators
        gOfficePath = convertPath(gOfficePath)
        ' to support some developer bootstraprc/ini's:
        gOfficePath = fRelativeToAbsolutePath(gOfficePath)
        gOfficePath = fRemoveDoubleCharacter(gOfficePath + gPathSigne, gPathSigne)
    else
        gNetzInst = GetIniValue ( gTesttoolIni, "Network", "Net" )
        gNetzOfficePath = GetIniValue ( gTesttoolIni, "Network", "Current" )
        if Right ( gNetzOfficePath,1 ) <> gPathSigne then gNetzOfficePath = gNetzOfficePath + gPathSigne
    end if
end sub

'-------------------------------------------------------------------------

function ReplaceCharacter(stringToChange$, charToReplace$, replaceWith$) As String
    '/// Replaces a specified character in a string with another character that you specify
    '///+ INPUT: &lt;string&gt; , &lt;which characterto replace&gt; , &lt;which caracter the old one should be replaced&gt;
    Dim ln, n As Long
    Dim NextLetter As String
    Dim FinalString As String
    Dim txt, char, rep As String

    txt = stringToChange$ 'store all arguments in
    char = charToReplace$ 'new variables
    rep = replaceWith$
    ln = Len(txt)

    for n = 1 To ln Step 1
        NextLetter = Mid(txt, n, 1)
        if NextLetter = char Then
            NextLetter = rep
        end if
        FinalString = FinalString & NextLetter
    next n
    ReplaceCharacter = FinalString
    '///+</ul>
End Function

'-------------------------------------------------------------------------

function FileURLToNormalFile ( sFileURL as String ) as String
    '/// Converts a file URL to a system specific URL.
    FileURLToNormalFile = convertFromURL( sFileURL )
end function

'-------------------------------------------------------------------------

function fgetL10Nvalue() as string
    dim args(0) as new com.sun.star.beans.NamedValue
    args(0).Name = "nodepath"
    args(0).Value = "/org.openoffice.Setup/L10N"
    fgetL10Nvalue = hGetUNOService().createInstance("com.sun.star.configuration.DefaultProvider").createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", args()).getByName("ooLocale")
end function

'-------------------------------------------------------------------------

function hPlatformToGroup(sPLatform as string) as string
    '/// <u>Input</u>: string of class gPlatform
    '/// <u>Output</u>: string of class gPlatGroup
    select case sPLatform
    case "w95":hPlatformToGroup = "w95"
    case "w98":hPlatformToGroup = "w95"
    case "wse":hPlatformToGroup = "w95"
    case "wme":hPlatformToGroup = "w95"
    case "wnt":hPlatformToGroup = "w95"
    case "w2k":hPlatformToGroup = "w95"
    case "wxp":hPlatformToGroup = "w95"
    case "mac":hPlatformToGroup = "mac"
    case "ppc":hPlatformToGroup = "unx"
    case "sol":hPlatformToGroup = "unx"
    case "x86":hPlatformToGroup = "unx"
    case "lin":hPlatformToGroup = "unx"
    case "fbsd":hPlatformToGroup= "unx"
    case "nbsd":hPlatformToGroup= "unx"
    case "lin64":hPlatformToGroup= "unx"
    case "linsparc":hPlatformToGroup = "unx"
    case "ecs":hPlatformToGroup = "ecs"
    end select
end function

'-------------------------------------------------------------------------

sub sGetCrashreporterValues as boolean
    '/// This routine reads the values of the [Crashreporter] section
    '///+ in the TestTool control file (testtool.ini / .testtoolrc).
    '///It returns FALSE if one of the required
    '///+ information is missing in testtool.ini.

    sGetCrashreporterValues = FALSE
    '/// <i>gUseProxy</i> should be TRUE if you need a proxy or FALSE
    '///+ if you have a direct connection to the (inter)net.
    gUseProxy = GetIniValue ( gTesttoolIni, "Crashreporter" , "UseProxy" )
    'If there's no value (=no entries at all...) exit the routine.
    'sGetCrashreporterValues = FALSE so the routine in master.inc
    'will stop testing.
    if gUseProxy = "" then
        exit sub
    else
        'Making a boolean value from it.
        gUseProxy = cBool(gUseProxy)
        '/// <i>gConnectiontype</i> has the value DIRECT or MANUALPROXY in dependency
        '///+ of <i>gUseProxy</i>. If <i>gUseProxy</i> = TRUE then the value of
        '///+ <i>gConnectiontype</i> is MANUALPROXY otherwiese DIRECT.
        if gUseProxy = FALSE then
            gConnectionType = "DIRECT"
            sGetCrashreporterValues = TRUE
        else
            gConnectionType = "MANUALPROXY"
        end if

        if gUseProxy then
            '/// <i>gProxyServer</i> should be the proxy server which is needed to send
            '///+ the crashreporter data via the net.
            gProxyServer = GetIniValue ( gTesttoolIni, "Crashreporter" , "ProxyServer" )
            'Verifying that a proxy server has been entered if
            'gUseProxy has been set to TRUE.
            if gUseProxy = TRUE AND gProxyServer = "" then
                warnlog "qatesttool::global::system::inc::iniinfo.inc::sGetCrashReporterValue: " & "In the [Crashreporter]-section of TestTool control file 'UseProxy=true' but 'ProxyServer=' (empty). Please correct it!"
                sGetCrashreporterValues = FALSE
            else
                sGetCrashreporterValues = TRUE
            end if

            '/// <i>gProxyPort</i> should be the proxy server port which is needed to send
            '///+ the crashreporter data via the net.
            gProxyPort = GetIniValue ( gTesttoolIni, "Crashreporter" , "ProxyPort" )
            'Verifying that a proxy server port has been entered if
            'gUseProxy has been set to TRUE.
            if gUseProxy = TRUE AND gProxyPort = "" then
                sGetCrashreporterValues = FALSE
                warnlog "qatesttool::global::system::inc::iniinfo.inc::sGetCrashReporterValue: " & "In the [Crashreporter]-section of TestTool control file 'UseProxy=true' but 'ProxyPort=' (empty). Please correct it!"
            end if
        else
            ' to workaround issue i94779 set server and port to empty in case of direkt access
            gProxyServer = ""
            gProxyPort = ""
        endif

        '/// <i>gAllowContact</i> should be TRUE or FALSE
        gAllowContact = GetIniValue ( gTesttoolIni, "Crashreporter" , "AllowContact" )
        gAllowContact = cBool(gAllowContact)

        '/// <i>gReturnAddress</i> should be the E-Mail address which can be used
        '///+ to contact the user who has run the test scripts for additional information .
        gReturnAddress = GetIniValue ( gTesttoolIni, "Crashreporter" , "ReturnAddress" )
        'Verifying that a mail address has been entered if
        'gAllowContact has been set to TRUE.
        if gAllowContact = TRUE AND gReturnAddress = "" then
            warnlog "qatesttool::global::system::inc::iniinfo.inc::sGetCrashReporterValue: " & "In the [Crashreporter]-section of TestTool control file 'AllowContact=true' but 'ReturnAddress=' (empty). Please correct it!"
            sGetCrashreporterValues = FALSE
        end if
    end if
end sub

'-------------------------------------------------------------------------

function getSofficeNative() as string
    '/// Entry in Win32-registry or either link exist: /usr/bin/soffice - $HOME/soffice (aka Native Installer (OOo > SRC680m49)) ///'
    dim sVendor(50) as string
    dim sOffice(50) as string
    dim sVersion(50) as string
    dim iVendorEntries as integer
    dim iVersionEntries as integer
    dim iOfficeEntries as integer
    dim iOOoVersionBorder as integer
    dim i,x,y,z,j as integer
    dim iVendor(2) as integer
    dim iVersion(2) as integer
    dim iOffice(2) as integer
    dim sOfficePath(2) as string
    dim sTemp, sTemp2 as string
    dim sSVersionIni(2) as string
    dim iSversion as integer
    dim sCandidates(2) as string
    dim sKey as string
    dim sPath(2) as string
    dim iBuildId(2) as string
    dim slVersion() as string
    dim ilVersion as integer
    dim sLastVersion as string
    dim iPosA, iPosB as integer
    dim bError as boolean

    bError = false

    '/// separated handling between Linux/Unix and Win32 platforms ///'
    if ("unx" = gPlatGroup) then
        '/// on Linux/Unix systems we need to check/resolv 2 possible links which point to the directory with the 'soffice' executable ///'
        try
            '/// created by an installation from the root user: //usr//bin//libreoffice ///'
            sCandidates(1) = getLinkDestination("/usr/bin/libreoffice") ' command fails, if link doesn't exist
            if (left(sCandidates(1),2)="..") then
                ' If the link is relative, make it absolute
                sCandidates(1) = "/usr/bin/" + sCandidates(1)
                sCandidates(1) = fRelativeToAbsolutePath(sCandidates(1))
                sCandidates(1) = fRemoveDoubleCharacter(sCandidates(1), gPathSigne)
            end if
            ' DEBUG: If you need more detailed information about the installation environment
            '        enable the next line.
            ' printlog "getSofficeNative() - : /usr/bin/libreoffice: '" + sCandidates(1) + "' destination exists?: " +fileExists(sCandidates(1))
        catch
            ' DEBUG: If you need more detailed information about the installation environment
            '        enable the next line.
            ' printlog "getSofficeNative() - : /usr/bin/libreoffice: NOT AVAILABLE"
            sCandidates(1) = ""
        endcatch
        try
            '/// HAS TO GET created by installation from NON root user: $HOME/libreoffice ///'
            sTemp = Environ("HOME")
            sCandidates(2) = getLinkDestination(sTemp+"/libreoffice")
            if (left(sCandidates(2),1) <> "/") then
                ' relative path in $HOME directory!
                sCandidates(2) = sTemp + "/" + sCandidates(2)
            end if
            ' DEBUG: If you need more detailed information about the installation environment
            '        enable the next line.
            ' printlog "getSofficeNative() - : $HOME/libreoffice   : '" + sCandidates(2) + "' destination exists?: " +fileExists(sCandidates(2))
        catch
            ' DEBUG: If you need more detailed information about the installation environment
            '        enable the next line.
            ' printlog "getSofficeNative() - : $HOME/libreoffice   : NOT AVAILABLE"
            sCandidates(2) = ""
        endcatch
        for i = 1 to 2
            if fileExists(sCandidates(i)) then
                '/// cut off program//soffice from link destination ///'
                y = instr(sCandidates(i), "program/soffice")
                if ( lcase( gPlatform ) = "osx" AND y < 1 ) then
                    ' Accept also MacOS in link to soffice binary
                    y = instr(sCandidates(i), "MacOS/soffice")
                end if
                if (y > 0) then
                    sCandidates(i) = left(sCandidates(i), y-1)
                else
                    qaErrorLog "master.inc::getSofficeNative():: expected 'program/soffice' at end of link destination: '" + sCandidates(i) + "'"
                    sCandidates(i) = ""
                end if
            end if
        next i
        if (("" = sCandidates(1)) or ("" = sCandidates(2))) then
            ' if one candidate doesn't exist
            '/// If only one candidate exist, take him. ///'
            gOfficePath = sCandidates(1) + sCandidates(2)
        else
            ' both candidates exist
            '/// If both candidates exits... ///'
            for i = 1 to 2
                '/// - Check if link destination exists; No: drop candidate ///'
                if fileExists(sCandidates(i)) then
                    iBuildId(i) = i
                else
                    ' link destination doesn't exist: drop candidate
                    sCandidates(i) = ""
                    iBuildId(i) = 0
                end if
            next i
            ' DEBUG: If you need more detailed information about the installation environment
            '        enable the next line.
            ' printlog "getSofficeNative() - : BildId (1): " + iBuildId(1) + " (2): " + iBuildId(2)
            '/// - Take the candidate which exists and comes from $HOME ///'
            if (iBuildId(1) > iBuildId(2)) then
                gOfficePath = sCandidates(1)
            else
                '/// - If BuildIds are equal, the candidate from $HOME wins ///'
                gOfficePath = sCandidates(2)
            end if
        end if
        if (not fileExists(gOfficePath)) then
            '/// - check if resulting candidate exist; No: error ///'
            bError = true
        end if
    else
        '/// On win32 systems we need to check the registry for two paths with the key 'Path': ///'
        sKey = "Path"
        '                    "--K--\Software\--Vendor--\--Product--\--Version--"
        '/// If User installs, the path 'HKEY_CURRENT_USER\\Software\\' got set.///'
        sPath(1) = "HKEY_CURRENT_USER\Software\"
        '/// If Administrator installs, the path 'HKEY_LOCAL_MACHINE\\Software\\' got set.///'
        sPath(2) = "HKEY_LOCAL_MACHINE\Software\"

        '/// An entry in the registry consists of 2 parts: 1st the Office, 2nd the version-number ///'
        '/// There may be several entries, but only the newest version is used.  ///'
        '/// There may exist both Paths HKCU and HKLM///'
        '/// Which version is used? ///'
        '/// From both Paths the newest version is chosen:  from the lowest sOffice() the lowest sVersion() ///'
        '/// If in both paths the entries are equal, the HKCU wins. ///'
        iVendorEntries = 1
        sVendor(iVendorEntries) = "Sun Microsystems": inc (iVendorEntries)
        sVendor(iVendorEntries) = "OpenOffice.org"  : inc (iVendorEntries)
       sVendor(iVendorEntries) = "Oracle"  : inc (iVendorEntries)
        
        iOfficeEntries = 1
        sOffice(iOfficeEntries) = "Oracle Open Office"    : inc (iOfficeEntries)
        sOffice(iOfficeEntries) = "StarOffice"    : inc (iOfficeEntries)
        sOffice(iOfficeEntries) = "StarSuite"     : inc (iOfficeEntries)
        sOffice(iOfficeEntries) = "OpenOffice.org": inc (iOfficeEntries)

        iVersionEntries = 1
        sVersion(iVersionEntries) = "9"      : inc (iVersionEntries)
        sVersion(iVersionEntries) = "8"      : inc (iVersionEntries)
        sVersion(iVersionEntries) = "3"      : inc (iVersionEntries)
        iOOoVersionBorder = iVersionEntries
        sVersion(iVersionEntries) = "2.0.0"  : inc (iVersionEntries)
        sVersion(iVersionEntries) = "680"    : inc (iVersionEntries)
        sVersion(iVersionEntries) = "2.2"    : inc (iVersionEntries)

        ' put new versions ABOVE THIS LINE! ^^^^^^^^^^^
        sVersion(iVersionEntries) ="NotFound": inc (iVersionEntries) ' HAS TO Stay at LAST

        ' I have to walk over both sPath(z),
        '   over all sOffice(y)-1
        '       over all sVersion(x)-2
        ' the first one found in each sPath(z) wins.
        for z = 1 to 2
            sOfficePath(z) = ""
            for j =1 to iVendorEntries-1
                for y =1 to iOfficeEntries-1
                    for x =1 to iVersionEntries-2
                        'collect all entries != "" (empty string)
                        if sOfficePath(z) = "" then ' if no version found right now, set it
                            sTemp = sPath(z) + sVendor(j) + "\" + sOffice(y) + "\" + sVersion(x)
                            sOfficePath(z) = getRegistryValue(sTemp, sKey)
                            iOffice(z) = z*1000 + j*100 + y*10 + x
                            if (sOfficePath(z) <> "") AND fileExists(sOfficePath(z)) then
                                ' DEBUG: If you need more detailed information about the installation environment
                                '        enable the next line.
                                ' printlog "getSofficeNative() - : A " + z + ":" + j + ":" + y + ":" + x + " " + sTemp +  " +++++++++++ " + sOfficePath(z)
                            else
                                ' printlog "getSofficeNative() - : C " + z + ":" + j + ":" + y + ":" + x + " " + sTemp +  " +++++++++++ " + sOfficePath(z)
                                sOfficePath(z) = ""
                                iOffice(z) = 0
                            end if
                        else
                            ' This is just for completeness. B has no effect, A counts.
                            sTemp = sPath(z) + sVendor(j) + "\" + sOffice(y) + "\" + sVersion(x)
                            if sTemp <> "" then
                                sTemp2 = getRegistryValue(sTemp, sKey)
                                if (sTemp2 <> "") then
                                    ' DEBUG: If you need more detailed information about the installation environment
                                    '        enable the next line.
                                    ' printlog "getSofficeNative() - : B " + z + ":" + j + ":" + y + ":" + x + " " + sTemp +  " +++++++++++ "+sTemp2
                                end if
                            end if
                        end if
                    next x
                next y
            next j
        next z

        ' remove path, if destination doesn't exist
        if (NOT fileExists(sOfficePath(1))) then
            sOfficePath(1) = ""
        end if
        if (NOT fileExists(sOfficePath(2))) then
            sOfficePath(2) = ""
        end if
        ' compute version to be used
        if (sOfficePath(1) = "") or (sOfficePath(2) = "") then
            gOfficePath = sOfficePath(1) + sOfficePath(2)
        else
            if (iOffice(1) > iOffice(2)) then
                gOfficePath = sOfficePath(2)
            else
                gOfficePath = sOfficePath(1)
            end if
        end if

        if (gOfficePath = "" or not fileExists(gOfficePath)) then
            ' Error of no office is found
            bError = true
        else
            ' cut of the executable name
            i = instr(gOfficePath, "program\soffice.exe")
            if i > 0 then
                gOfficePath = left(gOfficePath, i-1)
            else
                qaErrorLog "master.inc::getSofficeNative():: expected 'program/soffice.exe' at end of link destination: '" + gOfficePath + "'"
            end if
        end if
    end if
    ' now we have gOfficePath set.

    if (not bError) then
        ' needed input for function: fGetProductName()
        '/// Look, if we got an OpenOffice.org version ///'
        gNetzOfficePath = gOfficePath
        '/// Setting the global <i>gProductName</i> variable here.
        gProductName = fGetProductName
        ' A special build of OpenOffice.org is OOo-dev; #i56804#
        if (gProductName = "OOo-dev") then
            gProductName = "OpenOffice.org"
            PrintLog "This is a 'OOo-dev' product build."
        endif
        if (instr(gProductName, "OpenOffice.org") > 0) then
            gOOO = TRUE
        else
            gOOO = FALSE
        end if
        getSofficeNative = gOfficePath  ' set return value
    else
        '/// If error occurred, set returnvalue to "" - empty srting ///'
        getSofficeNative = ""           ' set return value
    end if
end function

'-------------------------------------------------------------------------

function getSofficeTesttool() as string
    dim sTemp as string

    sTemp = ""
    '/// <ol><li>Look into the current profile section of the TestTool control file </li>
    '/// deprecated, just kept here for compatibility
    sTemp = GetIniValue ( gTesttoolIni, gTTProfileName, "OOoProgramDir" )
    if (sTemp = "") then
        '///+ <li>Look into the global section of the TestTool control file OOoProgramDir section</li></ol>
        sTemp = GetIniValue ( gTesttoolIni, "OOoProgramDir", "Current" )
    end if
    ' Throw error, if the default '.' from first startup is found
    if fileExists(sTemp) AND NOT (sTemp = "." )then
        ' make sure path ends with path seperator
        if (right(sTemp,1) <> gPathSigne) then
            sTemp = sTemp + gPathSigne
        end if
        ' be tolerant to the chosen path
        ' we just need the base directory, cut of possible "program" at end
        if ( lcase( gPlatform ) = "osx" ) then
            ' to get all possibile directories:
            ' path could be: ...app/Contents/MacOS
            if (right(sTemp,6) = "MacOS"+gPathSigne) then
                sTemp = left(sTemp,len(sTemp)-6)
            end if
            if (right(sTemp,5) = ".app"+gPathSigne) then
                sTemp = sTemp + "Contents/"
            end if
        else
            if (right(sTemp,8) = "program"+gPathSigne) then
                sTemp = left(sTemp,len(sTemp)-8)
            end if
        end if
        getSofficeTesttool = sTemp
        gOfficePath = sTemp
        gNetzOfficePath = gOfficePath
        '/// Setting the global <i>gProductName</i> variable here because GetSofficeNative()
        '///+ won't fill this global variable if [OOoProgramDir] is being used.
        gProductName = fGetProductName
        ' A special build of OpenOffice.org is OOo-dev; #i56804#
        if (gProductName = "OOo-dev") then
            gProductName = "OpenOffice.org"
            PrintLog "This is a 'OOo-dev' product build."
        endif
        if (instr(gProductName, "OpenOffice.org") > 0) then
            gOOO = TRUE
        else
            gOOO = FALSE
        end if
        getSofficeTesttool = sTemp
    else
        '/// No valid path will return an empty string.
        getSofficeTesttool = ""
    end if
end function

'-------------------------------------------------------------------------

function sGetScreenshotValue as boolean
    '/// Read the entry [Screenshot] in the TestTool control file (testtool.ini / .testtoolrc).
    '///It returns FALSE if no screenshots are taken (default)
    '///+ TRUE if the entry in the file exists and is TRUE or 1 ///'
    dim sScreenshotValue as string
    dim bScreenshotValue as boolean

    sScreenshotValue = GetIniValue ( gTesttoolIni, "Screenshot" , "Current" )
    if sScreenshotValue <> "" then
        bScreenshotValue = cBool(sScreenshotValue)
        if bScreenshotValue then
            printlog "**  There will be screenshots taken from every dialog"
        end if
    else
        bScreenshotValue = FALSE
    end if
    sGetScreenshotValue = bScreenshotValue
end function

'-------------------------------------------------------------------------

function fgetOOoStartupTimeOut as integer
    '/// Read the entry [OOoStartupTimeOut] in the TestTool control file (testtool.ini / .testtoolrc).
    '///It returns 0 if the entry doesn't exist (default)
    '///+ >0 if the entry in the file exists and is >0 ///'
    dim sOOoStartupTimeOut as string
    dim iOOoStartupTimeOut as integer

    sOOoStartupTimeOut = getIniValue ( gTesttoolIni, "OOoStartupTimeOut" , "Current" )
    if sOOoStartupTimeOut <> "" then
        iOOoStartupTimeOut = cInt(sOOoStartupTimeOut)
        if iOOoStartupTimeOut > 0 then
            printlog "**  OOo will be given extra time for startup: '" + iOOoStartupTimeOut + "' seconds"
        end if
    else
        iOOoStartupTimeOut = 0
    end if
    fgetOOoStartupTimeOut = iOOoStartupTimeOut
end function

'-------------------------------------------------------------------------

function fgetOOoShutdownTimeOut as integer
    '/// Read the entry [OOoShutdownTimeOut] in the TestTool control file (testtool.ini / .testtoolrc).
    '///It returns 0 if the entry doesn't exist (default)
    '///+ >0 if the entry in the file exists and is >0 ///'
    dim sOOoShutdownTimeOut as string
    dim iOOoShutdownTimeOut as integer

    sOOoShutdownTimeOut = GetIniValue ( gTesttoolIni, "OOoShutdownTimeOut" , "Current" )
    if sOOoShutdownTimeOut <> "" then
        iOOoShutdownTimeOut = cInt(sOOoShutdownTimeOut)
        if iOOoShutdownTimeOut >0 then
            printlog "**  OOo will be given extra time for shutdown: '" + iOOoShutdownTimeOut + "' seconds"
        end if
    else
        iOOoShutdownTimeOut = FALSE
    end if
    fgetOOoShutdownTimeOut = iOOoShutdownTimeOut
end function

'-------------------------------------------------------------------------

function getOfficeBasisPath as string

    const CFN = "global::system::iniinfo.inc:getOfficeBasisPath: "
    'get the basis directory from the information of basis-link
    dim ltemp(5) as string
    dim sFile as string
    dim sTemp as string
    dim sPath as string

    sTemp = gNetzOfficePath
    sFile = gNetzOfficePath & "basis-link"

    if FileExists( sFile ) then
        if ( lcase( gPlatform ) = "osx" ) then
            'MacOS X
            sTemp = sFile
        else
            if ( gPlatGroup = "unx" ) then
                'Unixes
                sPath = getLinkDestination( sFile )
                sTemp = fRelativeToAbsolutePath( gNetzOfficePath & sPath )
                if ( NOT FileExists( sTemp ) ) then
                    sTemp = fRelativeToAbsolutePath( sPath )
                endif

            else
                'all other platforms are Windows/DOS style
                ListRead( lTemp, sFile )
                if lTemp(1) <> "" then
                    stemp = lTemp(1)
                    sTemp = fRelativeToAbsolutePath( gNetzOfficePath & sTemp )
                endif
            endif
        endif
    else
        warnlog( CFN & "Basis-link could not be found" )
    endif

    if ( NOT FileExists( sTemp ) ) then
        warnlog( CFN & "Retrieved invalid path" )
    endif

    getOfficeBasisPath = sTemp & gPathSigne
end function
