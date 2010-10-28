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
'* owner : hercule.li@oracle.com
'*
'* short description : Tools for CJK feature test in Writer
'*
'\***********************************************************************

function CheckAsianLanguageSupport( sSupport as String )  as String

    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")

    if ( aktivieren.exists() ) then
        if ( aktivieren.isEnabled() ) then
            if ( lcase( sSupport ) = "on" ) then
                printlog( "Enabling Asian Language Support" )
                Aktivieren.Check
            else
                printlog( "Disabling Asian Language Support" )
                Aktivieren.uncheck()
            endif
        else
            warnlog( "Activate Asian Language Support checkbox is disabled" )
        endif
    else
        warnlog( "Activate Asian Language Support checkbox is missing" )
    endif
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK()
    
end function

'******************************************************************************'

function fStartupNavigator(navigatorItem as String , subItemNum as Integer)
    Dim j as Integer
    
    Kontext "Navigator"
    if NOT Navigator.Exists then
        ' If you don' t ask for the Navigator and it's open it would be clodes and test fails !
        ViewNavigator
    end if
    Kontext "NavigatorWriter"
    if Auswahlliste.GetItemCount < 11 then
        Inhaltsansicht.Click
    end if
    if Auswahlliste.GetItemCount > 11 then
        For j= 1 to 1
            Auswahlliste.Select j
            Auswahlliste.TypeKeys "-"
        next j
    end if
    Kontext "NavigatorWriter"
    Auswahlliste.TypeKeys "<Up>", 11
    sleep 1
    Select case navigatorItem
    case "Headings"        : goto GoON
    case "Tables"          : Auswahlliste.TypeKeys "<down>"
    case "TextFrame"       : Auswahlliste.TypeKeys "<down>" , 2
    case "Graphics"        : Auswahlliste.TypeKeys "<down>" , 3
    case "OLEObject"       : Auswahlliste.TypeKeys "<down>" , 4
    case "Bookmarks"       : Auswahlliste.TypeKeys "<down>" , 5
    case "Section"         : Auswahlliste.TypeKeys "<down>" , 6
    case "Hyperlinks"      : Auswahlliste.TypeKeys "<down>" , 7
    case "References"      : Auswahlliste.TypeKeys "<down>" , 8
    case "Indexes"         : Auswahlliste.TypeKeys "<down>" , 9
    case "Notes"           : Auswahlliste.TypeKeys "<down>" , 10
    case "DrawObjects"     : Auswahlliste.TypeKeys "<down>" , 11
    end Select
    sleep 1
    GoON:
    Auswahlliste.TypeKeys "<return>"
    sleep 1
    Toolbox.Click Inhaltsansicht
    sleep 1
    Auswahlliste.TypeKeys "<Up>",subItemNum
    sleep 1
end function

'******************************************************************************'

function fGetCountryName( CountryID as Integer ) as String
    
    select case iSprache
    case 01   :        ' English (USA)
        select case CountryID
        case 1:  fGetCountryName = "English (USA)"
        case 33: fGetCountryName = "French (France)"
        case 34: fGetCountryName = "Spanish (Spain)"
        case 39: fGetCountryName = "Italian (Italy)"
        case 46: fGetCountryName = "Swedish (Sweden)"
        case 49: fGetCountryName = "German (Germany)"
        case 55: fGetCountryName = "Portuguese (Brazil)"
        case 81: fGetCountryName = "Japanese"
        case 82: fGetCountryName = "Korean (RoK)"
        case 86: fGetCountryName = "Chinese (simplified)"
        case 88: fGetCountryName = "Chinese (traditional)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 33   :        ' France
        select case CountryID
        case 1:  fGetCountryName = "Anglais (U.S.A.)"
        case 33: fGetCountryName = "Français (France)"
        case 34: fGetCountryName = "Espagnol (Espagne)"
        case 39: fGetCountryName = "Italien (Italie)"
        case 46: fGetCountryName = "Suédois (Suède)"
        case 49: fGetCountryName = "Allemand (Allemagne)"
        case 55: fGetCountryName = "Portugais (Brésil)"
        case 81: fGetCountryName = "Japonais"
        case 82: fGetCountryName = "Coréen"
        case 86: fGetCountryName = "Chinois (simple)"
        case 88: fGetCountryName = "Chinois (traditionnel)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 34   :        ' Spanish
        select case CountryID
        case 1:  fGetCountryName = "Inglés (EE.UU.)"
        case 33: fGetCountryName = "Francés (Francia)"
        case 34: fGetCountryName = "Español (España)"
        case 39: fGetCountryName = "Italiano (Italia)"
        case 46: fGetCountryName = "Sueco (Suecia)"
        case 49: fGetCountryName = "Alemán (Alemania)"
        case 55: fGetCountryName = "Portugués (Brasil)"
        case 81: fGetCountryName = "Japonés"
        case 82: fGetCountryName = "Coreano"
        case 86: fGetCountryName = "Chino (simple)"
        case 88: fGetCountryName = "Chino (tradicional)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 39   :        ' Italy
        select case CountryID
        case 1:  fGetCountryName = "Inglese (US)"
        case 33: fGetCountryName = "Francese (Francia)"
        case 34: fGetCountryName = "Spagnolo (Spagna)"
        case 39: fGetCountryName = "Italiano (Italia)"
        case 46: fGetCountryName = "Svedese (Svezia)"
        case 49: fGetCountryName = "Tedesco (Germania)"
        case 55: fGetCountryName = "Portoghese (Brasile)"
        case 81: fGetCountryName = "Giapponese"
        case 82: fGetCountryName = "Coreano"
        case 86: fGetCountryName = "Cinese (semplificato)"
        case 88: fGetCountryName = "Cinese (tradizionale)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 46   :        ' Swedish
        select case CountryID
        case 1:  fGetCountryName = "engelska"
        case 33: fGetCountryName = "franska (Frankrike)"
        case 34: fGetCountryName = "spanska (Spanien)"
        case 39: fGetCountryName = "italienska (Italien)"
        case 46: fGetCountryName = "svenska (Sverige)"
        case 49: fGetCountryName = "tyska (Tyskland)"
        case 55: fGetCountryName = "portugisiska (Brasilien)"
        case 81: fGetCountryName = "Japanska"
        case 82: fGetCountryName = "koreanska"
        case 86: fGetCountryName = "kinesiska (enkel)"
        case 88: fGetCountryName = "kinesiska (trad.)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 49   :        ' German
        select case CountryID
        case 1:  fGetCountryName = "Englisch (USA)"
        case 33: fGetCountryName = "Französisch (Frankreich)"
        case 34: fGetCountryName = "Spanisch (Spanien)"
        case 39: fGetCountryName = "Italienisch (Italien)"
        case 46: fGetCountryName = "Schwedisch (Schweden)"
        case 49: fGetCountryName = "Deutsch (Deutschland)"
        case 55: fGetCountryName = "Portugiesisch (Brasilien)"
        case 81: fGetCountryName = "Japanisch"
        case 82: fGetCountryName = "Koreanisch (RoK)"
        case 86: fGetCountryName = "Chinesisch (einfach)"
        case 88: fGetCountryName = "Chinesisch (Trad.)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 50   :        ' Slovenian
       select case CountryID
       case 1:  fGetCountryName = "angleški (ZDA)"
       case 33: fGetCountryName = "francoski (Francija)"
       case 34: fGetCountryName = "španski (Španija)"
       case 39: fGetCountryName = "italijanski (Italija)"
       case 46: fGetCountryName = "švedski (Švedska)"
       case 49: fGetCountryName = "nemški (Nemčija)"
       case 55: fGetCountryName = "portugalski (Brazilija)"
       case 81: fGetCountryName = "japonski"
       case 82: fGetCountryName = "korejski (RK)"
       case 86: fGetCountryName = "kitajski (poenostavljeni)"
       case 88: fGetCountryName = "kitajski (tradicionalni)"
       case else :
           QAErrorLog "Now, the test does not support for the language " +iSprache
           fGetCountryName = ""
       end select
    case 55   :        ' Portuguese
        select case CountryID
        case 1:  fGetCountryName = "Inglês (EUA)"
        case 33: fGetCountryName = "Francês (França)"
        case 34: fGetCountryName = "Espanhol (Espanha)"
        case 39: fGetCountryName = "Italiano (Itália)"
        case 46: fGetCountryName = "Sueco (Suécia)"
        case 49: fGetCountryName = "Alemão (Alemanha)"
        case 55: fGetCountryName = "Português (Brasil)"
        case 81: fGetCountryName = "Japonês"
        case 82: fGetCountryName = "Coreano (RoK)"
        case 86: fGetCountryName = "Chinês (simplificado)"
        case 88: fGetCountryName = "Chinês (tradicional)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 81   :        ' Japanese
        select case CountryID
        case 1:  fGetCountryName = "英語(米国)"
        case 33: fGetCountryName = "フランス語(フランス)"
        case 34: fGetCountryName = "スペイン語(スペイン)"
        case 39: fGetCountryName = "イタリア語(イタリア)"
        case 46: fGetCountryName = "スウェーデン語(スウェーデン)"
        case 49: fGetCountryName = "ドイツ語(ドイツ)"
        case 55: fGetCountryName = "ポルトガル語(ブラジル)"
        case 81: fGetCountryName = "日本語"
        case 82: fGetCountryName = "韓国語"
        case 86: fGetCountryName = "中国語(簡体字)"
        case 88: fGetCountryName = "中国語(繁体字)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 82   :        ' Korean
        select case CountryID
        case 1:  fGetCountryName = "영어(미국)"
        case 33: fGetCountryName = "스페인어(스페인)"
        case 34: fGetCountryName = "프랑스어(프랑스)"
        case 39: fGetCountryName = "이탈리아어 (이태리)"
        case 46: fGetCountryName = "스웨덴어(스웨덴)"
        case 49: fGetCountryName = "독일어(독일)"
        case 55: fGetCountryName = "포르투갈어(브라질)"
        case 81: fGetCountryName = "일본어"
        case 82: fGetCountryName = "한국어"
        case 86: fGetCountryName = "중국어(간체)"
        case 88: fGetCountryName = "중국어(번체)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case 86   :        ' Chinese (simplified)
        select case CountryID
        case 1:  fGetCountryName = "英语(美国)"
        case 33: fGetCountryName = "法语(法国)"
        case 34: fGetCountryName = "西班牙语(西班牙)"
        case 39: fGetCountryName = "意大利语(意大利)"
        case 46: fGetCountryName = "瑞典语(瑞典)"
        case 49: fGetCountryName = "德语(德国)"
        case 55: fGetCountryName = "葡萄牙语(巴西)"
        case 81: fGetCountryName = "日语"
        case 82: fGetCountryName = "朝鲜语"
        case 86: fGetCountryName = "中文(简体字)"
        case 88: fGetCountryName = "中文(繁体字)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
        
    case 88   :        ' Chinese (traditional)
        select case CountryID
        case 1:  fGetCountryName = "英語(美國)"
        case 33: fGetCountryName = "法語(法國)"
        case 34: fGetCountryName = "西班牙語(西班牙)"
        case 39: fGetCountryName = "意大利語(意大利)"
        case 46: fGetCountryName = "瑞典語(瑞典)"
        case 49: fGetCountryName = "德語(德國)"
        case 55: fGetCountryName = "葡萄牙語(巴西)"
        case 81: fGetCountryName = "日文"
        case 82: fGetCountryName = "韓語"
        case 86: fGetCountryName = "中文(簡體字)"
        case 88: fGetCountryName = "中文(繁體)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetCountryName = ""
        end select
    case else :        ' Fallback
        QAErrorLog "Now, the test does not support for the language " +iSprache
        fGetCountryName = ""
    end select
    
end function



