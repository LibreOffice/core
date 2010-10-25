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
'* short description : Tools for writer/Optional - 2
'*
'\***********************************************************************

function fStartupNavigator(navigatorItem as String , subItemNum as Integer)
    '/// Start up Navigator , and set focus to the correct position
    Dim j as Integer

    Kontext "Navigator"
    if NOT Navigator.Exists then
        ViewNavigator
    end if
    Sleep 2
    Kontext "NavigatorWriter"
    if Auswahlliste.GetItemCount < 11 then
        Inhaltsansicht.Click
        Sleep 1
    end if
    Auswahlliste.TypeKeys "<Home>"         '<- Set focus to first entry
    if Auswahlliste.GetItemCount > 11 then '<- If nodes are open
        For j= 1 to 12                     '<- close all nodes
            Auswahlliste.Select j
            Sleep 1
            Auswahlliste.TypeKeys "-"
            Sleep 1
        next j
    end if
    Auswahlliste.TypeKeys "<Home>"   ' point focus to first entry
    sleep 3

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
    sleep 2
    GoON:
    Auswahlliste.TypeKeys "<return>"
    sleep 2
    Inhaltsansicht.Click
    sleep 2
    Auswahlliste.TypeKeys "<Up>",subItemNum
    sleep 2
end function

'-------------------------------------------------------------------------

function fCloseNavigator
    Kontext "Navigator"
    hCloseDialog( Navigator, "close" )
end function

'-------------------------------------------------------------------------

function StrToDouble ( sWert$ ) as Double
    '/// Transfer String to Double , including
    '/// 2.00cm --> 2.00
    '/// 2,00cm --> 2.00
    '/// 20%    --> 20

    dim sDummy$
    dim i, i1, i2 as integer
    dim a as integer
    dim b as integer
    dim c as double
    dim n as integer

    '/// Input : {'a[. ,]b[mm cm " pi pt]' with a, b as integer} as String
    '///+ Output: a[. , ]b as double

    ' get rid of measure unit, the only single character is '"' all others are two chars

    if (isNumeric (sWert$) = FALSE) then
        if (   (StrComp (right (sWert$, 1), chr$(34) ) = 0)  OR  (StrComp (right (sWert$, 1), "%") = 0 )) then
            sDummy$ = Left ( sWert$, Len(sWert$)-1 )
        else
            sDummy$ = Left ( sWert$, Len(sWert$)-Len(gMeasurementUnit) )
        end if
    else
        sDummy$ = sWert$
    end if
    ' get position of fraction mark
    i1 = instr (sDummy$, ",")  ' wrong output
    i2 = instr (sDummy$, ".")
    if i1 > i2 then i = i1 else i = i2
    ' vorkommateil
    try
        a = val (left (sDummy$,i-1))
    catch
        a = val ( sDummy$ )
        c = 0
        GoTo OK
    endcatch

    n = (len (sDummy$)-i)
    b = val (right (sDummy$, n) )
    c = b * 10 ^ -n
    OK:
    StrToDouble = a + c
end function

'-------------------------------------------------------------------------

function fCalculateTolerance (sNumber1 , sNumber2) as Single
    '/// Change 2 parameters to number , then Compare them and return the result.
    Dim ss as Single
    Dim dNumber1        , dNumber2         as Single
    Dim iCommaLocation1 , iCommaLocation2  as Integer

    'the code below 4 lines is set "2,00cm" to "2.00"cm if needed
    iCommaLocation1 = InStr(sNumber1 , ",")
    if iCommaLocation1 > 0 then
        sNumber1 = Left(sNumber1 , iCommaLocation1-1) + "." + Right(sNumber1,Len(sNumber1) - iCommaLocation1)
    end if

    iCommaLocation2 = InStr(sNumber2 , ",")
    if iCommaLocation2 > 0 then
        sNumber2 = Left(sNumber2 , iCommaLocation2-1) + "." + Right(sNumber2,Len(sNumber2) - iCommaLocation2)
    end if

    dNumber1 = Val( Left(sNumber1, Len(sNumber1)-Len(gMeasurementUnit) ))
    dNumber2 = Val( Left(sNumber2, Len(sNumber2)-Len(gMeasurementUnit) ))

    fCalculateTolerance = Abs( dNumber1 - dNumber2 )
end function

'-------------------------------------------------------------------------

function fFindWord (sTest) as String
    '/// Set the focus to the word which you want
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sTest
    Sleep 1
    SearchAll.Click
    Sleep 1
    FindAndReplace.Close
end function

'-------------------------------------------------------------------------

function fGetLanguageName( CountryID as Integer ) as String
    '/// This routine will return the Language name based on CountryID
    select case iSprache
    case 01   :        ' English (USA)
        select case CountryID
        case 1:  fGetLanguageName = "English (USA)"
        case 31:  fGetLanguageName = "Dutch (Netherlands)"
        case 33: fGetLanguageName = "French (France)"
        case 34: fGetLanguageName = "Spanish (Spain)"
        case 39: fGetLanguageName = "Italian (Italy)"
        case 46: fGetLanguageName = "Swedish (Sweden)"
        case 49: fGetLanguageName = "German (Germany)"
        case 55: fGetLanguageName = "Portuguese (Brazil)"
        case 81: fGetLanguageName = "Japanese"
        case 82: fGetLanguageName = "Korean (RoK)"
        case 86: fGetLanguageName = "Chinese (simplified)"
        case 88: fGetLanguageName = "Chinese (traditional)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 31   :        ' Dutch
        select case CountryID
        case 1:  fGetLanguageName = "Engels (VS)"
        case 31:  fGetLanguageName = "Netherlands (NL)"
        case 33: fGetLanguageName = "Frans (Frankrijk)"
        case 34: fGetLanguageName = "Spaans (Spanje)"
        case 39: fGetLanguageName = "Italiaans (Itali�)"
        case 46: fGetLanguageName = "Zweeds (Zweden)"
        case 49: fGetLanguageName = "Duits (Duitsland)"
        case 55: fGetLanguageName = "Portugees (Brazili�)"
        case 81: fGetLanguageName = "Japans"
        case 82: fGetLanguageName = "Korean (RoK)"
        case 86: fGetLanguageName = "Chinese (vereenvoudigd)"
        case 88: fGetLanguageName = "Chinese (traditioneel)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 33   :        ' France
        select case CountryID
        case 1:  fGetLanguageName = "Anglais (U.S.A.)"
        case 31: fGetLanguageName = "N�erlandais (Pays-Bas)"
        case 33: fGetLanguageName = "Français (France)"
        case 34: fGetLanguageName = "Espagnol (Espagne)"
        case 39: fGetLanguageName = "Italien (Italie)"
        case 46: fGetLanguageName = "Suédois (Suède)"
        case 49: fGetLanguageName = "Allemand (Allemagne)"
        case 55: fGetLanguageName = "Portugais (Brésil)"
        case 81: fGetLanguageName = "Japonais"
        case 82: fGetLanguageName = "Coréen"
        case 86: fGetLanguageName = "Chinois (simple)"
        case 88: fGetLanguageName = "Chinois (traditionnel)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 34   :        ' Spanish
        select case CountryID
        case 1:  fGetLanguageName = "Inglés (EE.UU.)"
        case 31: fGetLanguageName = "Holand�s (Pa�ses Bajos)"
        case 33: fGetLanguageName = "Francés (Francia)"
        case 34: fGetLanguageName = "Español (España)"
        case 39: fGetLanguageName = "Italiano (Italia)"
        case 46: fGetLanguageName = "Sueco (Suecia)"
        case 49: fGetLanguageName = "Alemán (Alemania)"
        case 55: fGetLanguageName = "Portugués (Brasil)"
        case 81: fGetLanguageName = "Japonés"
        case 82: fGetLanguageName = "Coreano"
        case 86: fGetLanguageName = "Chino (simple)"
        case 88: fGetLanguageName = "Chino (tradicional)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 39   :        ' Italy
        select case CountryID
        case 1:  fGetLanguageName = "Inglese (US)"
        case 31: fGetLanguageName = "Olandese (Paesi Bassi)"
        case 33: fGetLanguageName = "Francese (Francia)"
        case 34: fGetLanguageName = "Spagnolo (Spagna)"
        case 39: fGetLanguageName = "Italiano (Italia)"
        case 46: fGetLanguageName = "Svedese (Svezia)"
        case 49: fGetLanguageName = "Tedesco (Germania)"
        case 55: fGetLanguageName = "Portoghese (Brasile)"
        case 81: fGetLanguageName = "Giapponese"
        case 82: fGetLanguageName = "Coreano"
        case 86: fGetLanguageName = "Cinese (semplificato)"
        case 88: fGetLanguageName = "Cinese (tradizionale)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 46   :        ' Swedish
        select case CountryID
        case 1:  fGetLanguageName = "engelska"
        case 31: fGetLanguageName = "Holl�ndska (Nederl�nderna)"
        case 33: fGetLanguageName = "franska (Frankrike)"
        case 34: fGetLanguageName = "spanska (Spanien)"
        case 39: fGetLanguageName = "italienska (Italien)"
        case 46: fGetLanguageName = "svenska (Sverige)"
        case 49: fGetLanguageName = "tyska (Tyskland)"
        case 55: fGetLanguageName = "portugisiska (Brasilien)"
        case 81: fGetLanguageName = "japanska"
        case 82: fGetLanguageName = "koreanska"
        case 86: fGetLanguageName = "kinesiska (enkel)"
        case 88: fGetLanguageName = "kinesiska (trad.)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 49   :        ' German
        select case CountryID
        case 1:  fGetLanguageName = "Englisch (USA)"
        case 31: fGetLanguageName = "Niederl�ndisch (Niederlande)"
        case 33: fGetLanguageName = "Französisch (Frankreich)"
        case 34: fGetLanguageName = "Spanisch (Spanien)"
        case 39: fGetLanguageName = "Italienisch (Italien)"
        case 46: fGetLanguageName = "Schwedisch (Schweden)"
        case 49: fGetLanguageName = "Deutsch (Deutschland)"
        case 55: fGetLanguageName = "Portugiesisch (Brasilien)"
        case 81: fGetLanguageName = "Japanisch"
        case 82: fGetLanguageName = "Koreanisch"
        case 86: fGetLanguageName = "Chinesisch (einfach)"
        case 88: fGetLanguageName = "Chinesisch (Trad.)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 55   :        ' Portuguese
        select case CountryID
        case 1:  fGetLanguageName = "Inglês (EUA)"
        case 31: fGetLanguageName = "Holand�s (Holanda)"
        case 33: fGetLanguageName = "Francês (França)"
        case 34: fGetLanguageName = "Espanhol (Espanha)"
        case 39: fGetLanguageName = "Italiano (Itália)"
        case 46: fGetLanguageName = "Sueco (Suécia)"
        case 49: fGetLanguageName = "Alemão (Alemanha)"
        case 55: fGetLanguageName = "Português (Brasil)"
        case 81: fGetLanguageName = "Japonês"
        case 82: fGetLanguageName = "Coreano"
        case 86: fGetLanguageName = "Chinês (simplificado)"
        case 88: fGetLanguageName = "Chinês (Tradicional)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 81   :        ' Japanese
        select case CountryID
        case 1:  fGetLanguageName = "英語(米国)"
        case 33: fGetLanguageName = "フランス語(フランス)"
        case 34: fGetLanguageName = "スペイン語(スペイン)"
        case 39: fGetLanguageName = "イタリア語(イタリア)"
        case 46: fGetLanguageName = "スウェーデン語(スウェーデン)"
        case 49: fGetLanguageName = "ドイツ語(ドイツ)"
        case 55: fGetLanguageName = "�?ルトガル語(ブラジル)"
        case 81: fGetLanguageName = "日本語"
        case 82: fGetLanguageName = "韓国語"
        case 86: fGetLanguageName = "中国語(簡体字)"
        case 88: fGetLanguageName = "中国語(�?体字)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 82   :        ' Korean
        select case CountryID
        case 1:  fGetLanguageName = "�?어(미국)"
        case 33: fGetLanguageName = "스페�?�어(스페�?�)"
        case 34: fGetLanguageName = "프랑스어(프랑스)"
        case 39: fGetLanguageName = "�?�탈리아어 (�?�태리)"
        case 46: fGetLanguageName = "스웨�?�어(스웨�?�)"
        case 49: fGetLanguageName = "�?��?�어(�?��?�)"
        case 55: fGetLanguageName = "�?�르투갈어(브�?�질)"
        case 81: fGetLanguageName = "�?�본어"
        case 82: fGetLanguageName = "한국어"
        case 86: fGetLanguageName = "중국어(간체)"
        case 88: fGetLanguageName = "중국어(번체)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 86   :        ' Chinese (simplified)
        select case CountryID
        case 1:  fGetLanguageName = "英语(美国)"
        case 33: fGetLanguageName = "法语(法国)"
        case 34: fGetLanguageName = "西�?�牙语(西�?�牙)"
        case 39: fGetLanguageName = "�?大利语(�?大利)"
        case 46: fGetLanguageName = "瑞典语(瑞典)"
        case 49: fGetLanguageName = "德语(德国)"
        case 55: fGetLanguageName = "葡�?�牙语(巴西)"
        case 81: fGetLanguageName = "日语"
        case 82: fGetLanguageName = "�?鲜语"
        case 86: fGetLanguageName = "中文(简体字)"
        case 88: fGetLanguageName = "中文(�?体字)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case 88   :        ' Chinese (traditional)
        select case CountryID
        case 1:  fGetLanguageName = "英語(美國)"
        case 33: fGetLanguageName = "法語(法國)"
        case 34: fGetLanguageName = "西�?�牙語(西�?�牙)"
        case 39: fGetLanguageName = "�?大利語(�?大利)"
        case 46: fGetLanguageName = "瑞典語(瑞典)"
        case 49: fGetLanguageName = "德語(德國)"
        case 55: fGetLanguageName = "葡�?�牙語(巴西)"
        case 81: fGetLanguageName = "日語"
        case 82: fGetLanguageName = "韓語"
        case 86: fGetLanguageName = "中文(簡體字)"
        case 88: fGetLanguageName = "中文(�?體)"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetLanguageName = ""
        end select

    case else :        ' Fallback
        QAErrorLog "Now, the test does not support for the language " +iSprache
        fGetLanguageName = ""
    end select
end function
