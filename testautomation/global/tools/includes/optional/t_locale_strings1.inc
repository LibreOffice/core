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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : defining locale dependant strings 
'*
'\***********************************************************************

function fLocaleString (sReturn as string) as string

'///The function fLocaleString should be used to concentrate all locale dependant strings
'///+ in one file in order to minimize the workload for adapting a new language
'///Here is a way how to get these strings out of OOo.
'/// - "LocaleTableHeading"       : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleTableContents"      : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleNumber"             : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleText"               : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleCurrency"           : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleDate"               : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleTime"               : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocalePercent"            : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleScientific"         : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleFraction"           : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleBoolean"            : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleTable"              : run testscript qa\qatesttool\writer\tools\w_get_locale_strings.bas
'/// - "LocaleCurrencyValue"      : enter 10 in a cell and format to currency
'/// - "LocaleDateValue"          : a valid ( for this locale) date format
'/// - "LocaleDateOutput"         : enter a valid date in a cell
'/// - "LocaleTimeInput"          : a valid timecode (e.g. 15:42)
'/// - "LocaleTimeValue"          : enter 15:42 in a cell
'/// - "LocalePercentValue"	      : enter 14% in a cell
'/// - "LocaleScientificValue"	  : enter 1e+008 in a cell
'/// - "LocaleBooleanValue"       : enter 1 in a cell and format to boolean value
'/// - "LocaleLocaleSettings"     : TOOLS-OPTIONS-LANGUAGE SETTINGS-Language
'/// - "LocaleScriptType"         : possible script types are "Western", "CJK" and "CTL"

    Dim sLocaleTableHeading as string
    Dim sLocaleTableContents as string
    Dim sLocaleNumber as string
    Dim sLocaleText as string
    Dim sLocaleCurrency as string
    Dim sLocaleDate as string
    Dim sLocaleTime as string
    Dim sLocalePercent as string
    Dim sLocaleScientific as string
    Dim sLocaleFraction as string
    Dim sLocaleBoolean as string
    Dim sLocaleTable as string
    Dim sLocaleCurrencyValue as string
    Dim sLocaleDateValue as string
    Dim sLocaleDateOutput as string
    Dim sLocaleTimeInput as string
    Dim sLocaleTimeValue as string
    Dim slocalePercentValue as string
    Dim sLocaleScientificValue as string
    Dim sLocaleBooleanValue as string
    Dim sLocaleLocaleSettings as string
    Dim sLocaleScriptType as string
    Dim sLocaleNumFormLanEng as string
    Dim sLocaleNumFormLanLith as string
    Dim sLocaleNumFormLanDutch as string
    
    select case iSprache
          case 01   :  sLocaleTableHeading      = "Table Heading"      ' English (USA)
                       sLocaleTableContents     = "Table Contents"
                       sLocaleNumber            = "Number"
                       sLocaleText              = "Text"
                       sLocaleCurrency          = "Currency"
                       sLocaleDate              = "Date"
                       sLocaleTime              = "Time"
                       sLocalePercent           = "Percent"
                       sLocaleScientific        = "Scientific"
                       sLocaleFraction          = "Fraction"
                       sLocaleBoolean           = "Boolean Value"
                       sLocaleTable             = "Table"
                       sLocaleCurrencyValue     = "$10.00"
                       sLocaleDateValue         = "03/31/06"
                       sLocaleDateOutput	    = "03/31/06"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "03:42:00 PM"
                       slocalePercentValue      = "14.00%"
                       sLocaleScientificValue   = "1.00E+008"
                       sLocaleBooleanValue      = "TRUE"
                       sLocaleLocaleSettings    = "English (USA)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "English (USA)"
                       sLocaleNumFormLanLith    = "Lithuanian"
                       sLocaleNumFormLanDutch   = "Dutch (Netherlands)"
                       
          case 03   :  sLocaleTableHeading      = "Cabeçalho da tabela"      ' Portuguese
                       sLocaleTableContents     = "Conteúdo da tabela"
                       sLocaleNumber            = "Número"
                       sLocaleText              = "Texto"
                       sLocaleCurrency          = "Moeda"
                       sLocaleDate              = "Data"
                       sLocaleTime              = "Hora"
                       sLocalePercent           = "Percentagem"
                       sLocaleScientific        = "Científico"
                       sLocaleFraction          = "Fracção"
                       sLocaleBoolean           = "Valor logico"
                       sLocaleTable             = "Tabela"
                       sLocaleCurrencyValue     = "10,00 €"
                       sLocaleDateValue         = "31-03-06"
                       sLocaleDateOutput	    = "31-03-06"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "VERDADEIRO"
                       sLocaleLocaleSettings    = "Português (Portugal)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Inglês (EUA)"
                       sLocaleNumFormLanLith    = "Lituano"
                       sLocaleNumFormLanDutch   = "Flamengo (Holanda)"
                       
          case 07   :  sLocaleTableHeading      = "Заголовок таблицы"      ' Russian
                       sLocaleTableContents     = "Содержимое таблицы"
                       sLocaleNumber            = "Число"
                       sLocaleText              = "Текст"
                       sLocaleCurrency          = "Деньги"
                       sLocaleDate              = "Дата"
                       sLocaleTime              = "TВремя"
                       sLocalePercent           = "Процент"
                       sLocaleScientific        = "Наука"
                       sLocaleFraction          = "Дробь"
                       sLocaleBoolean           = "Логические значения"
                       sLocaleTable             = "Таблица"
                       sLocaleCurrencyValue     = "10,00руб."
                       sLocaleDateValue         = "31.03.06"
                       sLocaleDateOutput	    = "31.03.06"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "ИСТИНА"
                       sLocaleLocaleSettings    = "Русский"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Английский (США)"
                       sLocaleNumFormLanLith    = "Литовский"
                       sLocaleNumFormLanDutch   = "Голландский (Нидерланды)"
                       
          case 31   :  sLocaleTableHeading      = "Tabelkop"      ' Dutch
                       sLocaleTableContents     = "Inhoud tabel"
                       sLocaleNumber            = "Getal"
                       sLocaleText              = "Tekst"
                       sLocaleCurrency          = "Valuta"
                       sLocaleDate              = "Datum"
                       sLocaleTime              = "Tijd"
                       sLocalePercent           = "Procent"
                       sLocaleScientific        = "Wetenschappelijk"
                       sLocaleFraction          = "Breuk"
                       sLocaleBoolean           = "Logische waarde"
                       sLocaleTable             = "Tabel"
                       sLocaleCurrencyValue     = "€ 10,00"
                       sLocaleDateValue         = "31-03-06"
                       sLocaleDateOutput	    = "31-03-06"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "WAAR"
                       sLocaleLocaleSettings    = "Nederlands (NL)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Engels (VS)"
                       sLocaleNumFormLanLith    = "Litouws"
                       sLocaleNumFormLanDutch   = "Nederlands (NL)"
                       
          case 33   :  sLocaleTableHeading      = "Titre de tableau"      ' French
                       sLocaleTableContents     = "Contenu de tableau"
                       sLocaleNumber            = "Nombre"
                       sLocaleText              = "Texte"
                       sLocaleCurrency          = "Monétaire"
                       sLocaleDate              = "Date"
                       sLocaleTime              = "Heure"
                       sLocalePercent           = "Pourcentage"
                       sLocaleScientific        = "Scientifique"
                       sLocaleFraction          = "Fraction"
                       sLocaleBoolean           = "Valeur logique"
                       sLocaleTable             = "Tableau"
                       sLocaleCurrencyValue     = "10,00 €"
                       sLocaleDateValue         = "31/03/06"
                       sLocaleDateOutput	    = "31/03/06"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "VRAI"
                       sLocaleLocaleSettings    = "Français (France)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Anglais (U.S.A.)"
                       sLocaleNumFormLanLith    = "Lituanien"
                       sLocaleNumFormLanDutch   = "Néerlandais (Pays-Bas)"
                       
          case 34   :  sLocaleTableHeading      = "Encabezado de la tabla"      ' Spanish
                       sLocaleTableContents     = "Contenido de la tabla"
                       sLocaleNumber            = "Número"
                       sLocaleText              = "Texto"
                       sLocaleCurrency          = "Moneda"
                       sLocaleDate              = "Fecha"
                       sLocaleTime              = "Hora"
                       sLocalePercent           = "Porcentaje"
                       sLocaleScientific        = "Ciencia"
                       sLocaleFraction          = "Fracción"
                       sLocaleBoolean           = "Valor lógico"
                       sLocaleTable             = "Tabla"
                       sLocaleCurrencyValue     = "10,00 €"
                       sLocaleDateValue         = "31/03/06"
                       sLocaleDateOutput	    = "31/03/06"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "VERDADERO"
                       sLocaleLocaleSettings    = "Español (España)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Inglés (Estados Unidos)"
                       sLocaleNumFormLanLith    = "Lituano"
                       sLocaleNumFormLanDutch   = "Holandés (Países Bajos)"
                       
          case 36   :  sLocaleTableHeading      = "Táblázatfejléc"      ' Hungarian
                       sLocaleTableContents     = "Táblázattartalom"
                       sLocaleNumber            = "Szám"
                       sLocaleText              = "Szöveg"
                       sLocaleCurrency          = "Pénznem"
                       sLocaleDate              = "Dátum"
                       sLocaleTime              = "Idő"
                       sLocalePercent           = "Százalék"
                       sLocaleScientific        = "Tudományos"
                       sLocaleFraction          = "Tört"
                       sLocaleBoolean           = "Logikai érték"
                       sLocaleTable             = "Táblázat"
                       sLocaleCurrencyValue     = "10,00 Ft"
                       sLocaleDateValue         = "06-03-31"
                       sLocaleDateOutput	    = "2006-03-31"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "IGAZ"
                       sLocaleLocaleSettings    = "Magyar"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Angol (USA)"
                       sLocaleNumFormLanLith    = "Litván"
                       sLocaleNumFormLanDutch   = "Holland (Hollandia)"
          
          case 39   :  sLocaleTableHeading      = "Intestazione tabella"      ' Italian
                       sLocaleTableContents     = "Contenuto tabella"
                       sLocaleNumber            = "Numero"
                       sLocaleText              = "Testo"
                       sLocaleCurrency          = "Valuta"
                       sLocaleDate              = "Data"
                       sLocaleTime              = "Orario"
                       sLocalePercent           = "Percentuale"
                       sLocaleScientific        = "Scientifico"
                       sLocaleFraction          = "Frazione"
                       sLocaleBoolean           = "Valore booleano"
                       sLocaleTable             = "Tabella"
                       sLocaleCurrencyValue     = "€ 10,00"
                       sLocaleDateValue         = "31/03/06"
                       sLocaleDateOutput	    = "31/03/06"
                       sLocaleTimeInput         = "15.42"
                       sLocaleTimeValue         = "15.42.00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "VERO"
                       sLocaleLocaleSettings    = "Italiano (Italia)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Inglese (USA)"
                       sLocaleNumFormLanLith    = "Lituano"
                       sLocaleNumFormLanDutch   = "Olandese (Paesi Bassi)"
                       
          case 46   :  sLocaleTableHeading      = "Tabellöverskrift"      ' Swedish
                       sLocaleTableContents     = "Tabellinnehåll"
                       sLocaleNumber            = "Tal"
                       sLocaleText              = "Text"
                       sLocaleCurrency          = "Valuta"
                       sLocaleDate              = "Datum"
                       sLocaleTime              = "Tid"
                       sLocalePercent           = "Procent"
                       sLocaleScientific        = "Vetenskap"
                       sLocaleFraction          = "Bråk"
                       sLocaleBoolean           = "Sannolikhet"
                       sLocaleTable             = "Tabell"
                       sLocaleCurrencyValue     = "10,00 kr"
                       sLocaleDateValue         = "06-03-31"
                       sLocaleDateOutput	    = "06-03-31"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "SANT"
                       sLocaleLocaleSettings    = "Svenska (Sverige)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Engelska (USA)"
                       sLocaleNumFormLanLith    = "Litauiska"
                       sLocaleNumFormLanDutch   = "Holländska (Nederländerna)"
    
          case 48   :  sLocaleTableHeading      = "Nagłówek tabeli"      ' Polish
                       sLocaleTableContents     = "Zawartość tabeli"
                       sLocaleNumber            = "Liczba"
                       sLocaleText              = "Tekst"
                       sLocaleCurrency          = "Waluta"
                       sLocaleDate              = "Data"
                       sLocaleTime              = "Godzina"
                       sLocalePercent           = "Procent"
                       sLocaleScientific        = "Naukowy"
                       sLocaleFraction          = "Ułamek"
                       sLocaleBoolean           = "Wartość logiczna"
                       sLocaleTable             = "Tabela"
                       sLocaleCurrencyValue     = "10,00 zł"
                       sLocaleDateValue         = "2006-03-31"
                       sLocaleDateOutput	    = "2006-03-31"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "PRAWDA"
                       sLocaleLocaleSettings    = "Polski"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Angielski (USA)"
                       sLocaleNumFormLanLith    = "Litewski"
                       sLocaleNumFormLanDutch   = "Holenderski (Holandia)"
                       
          case 49   :  sLocaleTableHeading      = "Tabellen Überschrift"      ' German
                       sLocaleTableContents     = "Tabellen Inhalt"
                       sLocaleNumber            = "Zahl"
                       sLocaleText              = "Text"
                       sLocaleCurrency          = "Währung"
                       sLocaleDate              = "Datum"
                       sLocaleTime              = "Zeit"
                       sLocalePercent           = "Prozent"
                       sLocaleScientific        = "Wissenschaft"
                       sLocaleFraction          = "Bruch"
                       sLocaleBoolean           = "Wahrheitswert"
                       sLocaleTable             = "Tabelle"
                       sLocaleCurrencyValue     = "10,00 €"
                       sLocaleDateValue         = "31.03.06"
                       sLocaleDateOutput	    = "31.03.06"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "WAHR"
                       sLocaleLocaleSettings    = "Deutsch (Deutschland)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Englisch (USA)"
                       sLocaleNumFormLanLith    = "Litauisch"
                       sLocaleNumFormLanDutch   = "Niederländisch (Niederlande)"

        case 50     : sLocaleTableHeading      = "Glava tabele" ' Slovenian
                      sLocaleTableContents     = "Vsebina tabele"
                      sLocaleNumber            = "Število"
                      sLocaleText              = "Besedilo"
                      sLocaleCurrency          = "Valuta"
                      sLocaleDate              = "Datum"
                      sLocaleTime              = "Čas"
                      sLocalePercent           = "Odstotek"
                      sLocaleScientific        = "Znanstveno"
                      sLocaleFraction          = "Ulomek"
                      sLocaleBoolean           = "Dvojiška vrednost"
                      sLocaleTable             = "Tabela"
                      sLocaleCurrencyValue     = "10,00 €"
                      sLocaleDateValue         = "31.03.06"
                      sLocaleDateOutput	    = "31.03.06"
                      sLocaleTimeInput         = "15:42"
                      sLocaleTimeValue         = "15:42:00"
                      slocalePercentValue	    = "14,00%"
                      sLocaleScientificValue   = "1,00E+008"
                      sLocaleBooleanValue      = "TRUE"
                      sLocaleLocaleSettings    = "angleški (ZDA)"
                      sLocaleScriptType        = "Zahodni"
                      sLocaleNumFormLanEng     = "angleški (ZDA)"
                      sLocaleNumFormLanLith    = "litovski"
                      sLocaleNumFormLanDutch   = "nizozemski (Nizozemska)"

          case 55   :  sLocaleTableHeading      = "Título da tabela"      ' Brazil (Portuguese)
                       sLocaleTableContents     = "Conteudo da table"
                       sLocaleNumber            = "Número"
                       sLocaleText              = "Texto"
                       sLocaleCurrency          = "Moeda"
                       sLocaleDate              = "Data"
                       sLocaleTime              = "Hora"
                       sLocalePercent           = "Porcentagem"
                       sLocaleScientific        = "Científico"
                       sLocaleFraction          = "Fração"
                       sLocaleBoolean           = "Valor booleano"
                       sLocaleTable             = "Tabela"
                       sLocaleCurrencyValue     = "R$ 10,00"
                       sLocaleDateValue         = "31/03/06"
                       sLocaleDateOutput	    = "31/03/06"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14,00%"
                       sLocaleScientificValue   = "1,00E+008"
                       sLocaleBooleanValue      = "VERDADEIRO"
                       sLocaleLocaleSettings    = "Português (Brasil)"
                       sLocaleScriptType        = "Western"
                       sLocaleNumFormLanEng     = "Inglês (EUA)"
                       sLocaleNumFormLanLith    = "Lituano"
                       sLocaleNumFormLanDutch   = "Flamengo (Holanda)"
          
          case 81   :  sLocaleTableHeading      = "表の見出し"                 ' Japanese
                       sLocaleTableContents     = "表の内容"
                       sLocaleNumber            = "数値"
                       sLocaleText              = "テキスト"
                       sLocaleCurrency          = "通貨"
                       sLocaleDate              = "日付"
                       sLocaleTime              = "時刻"
                       sLocalePercent           = "パーセンテージ"
                       sLocaleScientific        = "科学"
                       sLocaleFraction          = "分数"
                       sLocaleBoolean           = "論理値"
                       sLocaleTable             = "表"
                       sLocaleCurrencyValue     = "￥10"
                       sLocaleDateValue         = "3月31日"
                       sLocaleDateOutput	    = "3月31日"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42"
                       slocalePercentValue	    = "14%"
                       sLocaleScientificValue   = "1.00E+08"
                       sLocaleBooleanValue      = "TRUE"
                       sLocaleLocaleSettings    = "日本語"
                       sLocaleScriptType        = "CJK"
                       sLocaleNumFormLanEng     = "英語 (米国)"
                       sLocaleNumFormLanLith    = "リトアニア語"
                       sLocaleNumFormLanDutch   = "オランダ語 (オランダ)"
          
          case 82   :  sLocaleTableHeading      = "표제목"      ' Korean
                       sLocaleTableContents     = "표 내용"
                       sLocaleNumber            = "수"
                       sLocaleText              = "텍스트"
                       sLocaleCurrency          = "통화"
                       sLocaleDate              = "날짜"
                       sLocaleTime              = "시간"
                       sLocalePercent           = "퍼센트"
                       sLocaleScientific        = "과학"
                       sLocaleFraction          = "분수"
                       sLocaleBoolean           = "논리 값"
                       sLocaleTable             = "표"
                       sLocaleCurrencyValue     = "￦10"
                       sLocaleDateValue         = "2006/3/31"
                       sLocaleDateOutput	    = "2006년 3월 31"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15시 42분 0초"
                       slocalePercentValue	    = "14%"
                       sLocaleScientificValue   = "1.00E+08"
                       sLocaleBooleanValue      = "참"
                       sLocaleLocaleSettings    = "한국어(대한민국)"
                       sLocaleScriptType        = "CJK"
                       sLocaleNumFormLanEng     = "영어(미국)"
                       sLocaleNumFormLanLith    = "리투아니아어"
                       sLocaleNumFormLanDutch   = "네덜란드어(네덜란드)"
          
          case 86   :  sLocaleTableHeading      = "表格标题"                  ' Chinese (simplified)
                       sLocaleTableContents     = "表格内容"
                       sLocaleNumber            = "数字"
                       sLocaleText              = "文字"
                       sLocaleCurrency          = "货币"
                       sLocaleDate              = "日期"
                       sLocaleTime              = "时间"
                       sLocalePercent           = "百分比"
                       sLocaleScientific        = "科学"
                       sLocaleFraction          = "分数"
                       sLocaleBoolean           = "逻辑值"
                       sLocaleTable             = "表格"
                       sLocaleCurrencyValue     = "￥10.00"
                       sLocaleDateValue         = "2006/3/31"
                       sLocaleDateOutput	    = "06年3月31日"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14.00%"
                       sLocaleScientificValue   = "1.00E+008"
                       sLocaleBooleanValue      = "真"
                       sLocaleLocaleSettings    = "中文（简体）"
                       sLocaleScriptType        = "CJK"
                       sLocaleNumFormLanEng     = "英语（美国）"
                       sLocaleNumFormLanLith    = "立陶宛语"
                       sLocaleNumFormLanDutch   = "荷兰语（荷兰）"
          
          case 88   :  sLocaleTableHeading      = "表格標題"      ' Chinese (traditional)
                       sLocaleTableContents     = "表格內容"
                       sLocaleNumber            = "数字"
                       sLocaleText              = "文字"
                       sLocaleCurrency          = "货币"
                       sLocaleDate              = "日期"
                       sLocaleTime              = "时间"
                       sLocalePercent           = "百分比"
                       sLocaleScientific        = "科学"
                       sLocaleFraction          = "分数"
                       sLocaleBoolean           = "逻辑值"
                       sLocaleTable             = "表格"
                       sLocaleCurrencyValue     = "NT$10.00"
                       sLocaleDateValue         = "2006/3/31"
                       sLocaleDateOutput	    = "06/3/31"
                       sLocaleTimeInput         = "15:42"
                       sLocaleTimeValue         = "15:42:00"
                       slocalePercentValue	    = "14.00%"
                       sLocaleScientificValue   = "1.00E+008"
                       sLocaleBooleanValue      = "真"
                       sLocaleLocaleSettings    = "中文 (繁體)"
                       sLocaleScriptType        = "CJK"
                       sLocaleNumFormLanEng     = "英文 (美國)"
                       sLocaleNumFormLanLith    = "立陶宛文"
                       sLocaleNumFormLanDutch   = "荷蘭文 (比利時)"
          
        case else :                 ' Fallback
             QAErrorLog "The test does not support language " +iSprache
             fLocaleString = "Abortion"
             exit function
    end select
      
    select case sReturn
        case "LocaleTableHeading"       : fLocaleString = slocaleTableHeading
        case "LocaleTableContents"      : fLocaleString = slocaleTableContents
        case "LocaleNumber"             : fLocaleString = sLocaleNumber
        case "LocaleText"               : fLocaleString = sLocaleText
        case "LocaleCurrency"           : fLocaleString = sLocaleCurrency
        case "LocaleDate"               : fLocaleString = sLocaleDate
        case "LocaleTime"               : fLocaleString = sLocaleTime
        case "LocalePercent"            : fLocaleString = sLocalePercent
        case "LocaleScientific"         : fLocaleString = sLocaleScientific
        case "LocaleFraction"           : fLocaleString = sLocaleFraction
        case "LocaleBoolean"            : fLocaleString = sLocaleBoolean
        case "LocaleTable"              : fLocaleString = sLocaleTable
        case "LocaleCurrencyValue"      : fLocaleString = sLocaleCurrencyValue
        case "LocaleDateValue"          : fLocaleString = sLocaleDateValue
        case "LocaleDateOutput"         : fLocaleString = sLocaleDateOutput
        case "LocaleTimeInput"          : fLocaleString = sLocaleTimeInput
        case "LocaleTimeValue"          : fLocaleString = sLocaleTimeValue
        case "LocalePercentValue"		: fLocaleString = slocalePercentValue
        case "LocaleScientificValue"	: fLocaleString = sLocaleScientificValue
        case "LocaleBooleanValue"       : fLocaleString = sLocaleBooleanValue
        case "LocaleLocaleSettings"     : fLocaleString = sLocaleLocaleSettings
        case "LocaleScriptType"         : fLocaleString = sLocaleScriptType
        case "LocaleNumFormLanEng"      : fLocaleString = sLocaleNumFormLanEng
        case "LocaleNumFormLanLith"     : fLocaleString = sLocaleNumFormLanLith
        case "LocaleNumFormLanDutch"    : fLocaleString = sLocaleNumFormLanDutch
        
        case else :                 ' Fallback
            QAErrorLog "String not available"
            fLocaleString = "Abortion"
    end select
  
end function
