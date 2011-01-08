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
'* owner :  helge.delfs@oracle.com
'*
'* short description : Functions mostly used for fields
'*
'*******************************************************************
'*
' #0 fSetLocale
' #0 fSetWestern
' #0 fSetAsian
' #0 fGetType
' #0 fGetFieldContent
' #0 fGetDate
' #0 fGetDate1
'*
'\***********************************************************************

function fSetLocale(sLocaleSetting as String)  as String

    'This function is set Locale in Tools/Options/Language Settings / Languages
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    fSetLocale = Gebietsschema.GetSelText
    Gebietsschema.Select  sLocaleSetting
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

end function

'----------------------------------------------------

function fSetWestern(sWestern as String)  as String

    'This function is set western language in Tools/Options/Language Settings / Languages
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    fSetWestern = Westlich.GetSelText
    if sWestern = "Null" then
        Westlich.Select  1
        goto GoON
    end if
    Westlich.Select  sWestern
    GoON:
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

end function

'----------------------------------------------------

function fSetAsian(sAsian as String)  as String

    'This function is set CJK language in Tools/Options/Language Settings / Languages
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    fSetAsian = Asiatisch.GetSelText
    if sAsian = "Null" then
        if Asiatisch.IsEnabled then Asiatisch.Select  1
    else
        if Asiatisch.IsEnabled then Asiatisch.Select sAsian
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

end function

'----------------------------------------------------

function fGetType(sType as String)  as String

    'This function will RETURN TYPE
    select case sType
    case "Author"
        select case iSprache
        case 1:   	fGetType = "Author"
        case 7:		fGetType = "Автор"
        case 31:  	fGetType = "Gebruiker"
        case 33:  	fGetType = "Utilisateur"
        case 34:  	fGetType = "Autor"
        case 36:  	fGetType = "Szerzó"
        case 39:  	fGetType = "Autore"
        case 46:  	fGetType = "Användare"
        case 49:  	fGetType = "Benutzer"
        case 55:  	fGetType = "Autor"
        case 81:  	fGetType = "ユーザー"
        case 82:  	fGetType = "작성�?"
        case 86:  	fGetType = "使用者"
        case 88:  	fGetType = "使用者"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case "Chapter"
        select case iSprache
        case 1:      	fGetType = "Chapter"
        case 7:		fGetType = "Глава"
        case 31:     	fGetType = "Hoofdstuk"
        case 33:     	fGetType = "Chapitre"
        case 34:     	fGetType = "Capítulo"
        case 36:     	fGetType = "Fejezet"
        case 39:     	fGetType = "Capitolo"
        case 46:     	fGetType = "Kapitel"
        case 49:     	fGetType = "Kapitel"
        case 55:     	fGetType = "Capítulo"
        case 81:     	fGetType = "章"
        case 82:     	fGetType = "장"
        case 86:     	fGetType = "章"
        case 88:     	fGetType = "章"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case "Date"
        select case iSprache
        case 1:      	fGetType = "Date"
        case 7:		fGetTYpe = "Дата"
        case 31:     	fGetType = "Datum"
        case 33:     	fGetType = "Date"
        case 34:     	fGetType = "Fecha"
        case 36:     	fGetType = "Dátum"
        case 39:     	fGetType = "Data"
        case 46:     	fGetType = "Datum"
        case 49:     	fGetType = "Datum"
        case 55:     	fGetType = "Data"
        case 81:     	fGetType = "日付"
        case 82:     	fGetType = "날짜"
        case 86:     	fGetType = "日期"
        case 88:     	fGetType = "日期"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case "File name"
        select case iSprache
        case 1:      	fGetType = "File name"
        case 7:		fGetType = "Имя файла"
        case 31:     	fGetType = "Bestandsnaam"
        case 33:     	fGetType = "Nom de fichier"
        case 34:     	fGetType = "Nombre del archivo"
        case 36:     	fGetType = "Fájlnév"
        case 39:     	fGetType = "Nome file"
        case 46:     	fGetType = "Filnamn"
        case 49:     	fGetType = "Dateiname"
        case 55:     	fGetType = "Nome do arquivo"
        case 81:     	fGetType = "ファイル�??"
        case 82:     	fGetType = "파�?� �?�름"
        case 86:     	fGetType = "文件�??"
        case 88:     	fGetType = "檔案�??"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case "Page"
        select case iSprache
        case 1:      	fGetType = "Page"
        case 7:		fGetType = "Страница"
        case 31:     	fGetType = "Pagina"
        case 33:     	fGetType = "Page"
        case 34:     	fGetType = "Página"
        case 36:     	fGetType = "Oldal"
        case 39:     	fGetType = "Pagina"
        case 46:     	fGetType = "Sida"
        case 49:     	fGetType = "Seite"
        case 55:     	fGetType = "Página"
        case 81:     	fGetType = "ページ"
        case 82:     	fGetType = "페�?�지"
        case 86:     	fGetType = "页"
        case 88:     	fGetType = "�?"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case "Sender"
        select case iSprache
        case 1:      	fGetType = "Sender"
        case 7:		fGetType = "Отправитель"
        case 31:     	fGetType = "Afzender"
        case 33:     	fGetType = "Expéditeur"
        case 34:     	fGetType = "Remitente"
        case 36:     	fGetType = "Feladó"
        case 39:     	fGetType = "Mittente"
        case 46:     	fGetType = "Avsändare"
        case 49:     	fGetType = "Absender"
        case 55:     	fGetType = "Remetente"
        case 81:     	fGetType = "差出人"
        case 82:     	fGetType = "보낸 사람"
        case 86:     	fGetType = "�?�件人"
        case 88:     	fGetType = "寄件者"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case "Statistics"
        select case iSprache
        case 1:      fGetType = "Statistics"
        case 7:		fGetType = "Статистика"
        case 31:     fGetType = "Statistiek"
        case 33:     fGetType = "Statistiques"
        case 34:     fGetType = "Estadística"
        case 36:     fGetType = "Statisztika"
        case 39:     fGetType = "Statistiche"
        case 46:     fGetType = "Statistik"
        case 49:     fGetType = "Statistik"
        case 55:     fGetType = "Estatísticas"
        case 81:     fGetType = "統計"
        case 82:     fGetType = "통계"
        case 86:     fGetType = "统计"
        case 88:     fGetType = "統計"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case "Templates"
        select case iSprache
        case 1:      	fGetType = "Templates"
        case 7:      	fGetType = "Шаблоны"
        case 31:     	fGetType = "Sjablonen"
        case 33:     	fGetType = "Modèle de document"
        case 34:     	fGetType = "Plantillas"
        case 36:     	fGetType = "Sablonok"
        case 39:     	fGetType = "Modello"
        case 46:     	fGetType = "Dokumentmall"
        case 49:     	fGetType = "Dokumentvorlage"
        case 55:     	fGetType = "Modelos"
        case 81:     	fGetType = "ドキュメントテンプレート"
        case 82:     	fGetType = "서�? 파�?�"
        case 86:     	fGetType = "文档�?样"
        case 88:     	fGetType = "文件樣�?"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case "Time"
        select case iSprache
        case 1:      	fGetType = "Time"
        case 7:		fGetTYpe = "Время"
        case 31:     	fGetType = "Tijd"
        case 33:     	fGetType = "Heure"
        case 34:     	fGetType = "Hora"
        case 36:     	fGetTYpe = "Idó"
        case 39:     	fGetType = "Orario"
        case 46:     	fGetType = "Klockslag"
        case 49:     	fGetType = "Uhrzeit"
        case 55:     	fGetType = "Hora"
        case 81:     	fGetType = "時刻"
        case 82:     	fGetType = "시간"
        case 86:     	fGetType = "时间"
        case 88:     	fGetType = "時間"
        case else :
            QAErrorLog "Now, the test does not support for the language " +iSprache
            fGetType = ""
        end select
    case else :        ' Fallback
        QAErrorLog "Now, the test does not support for the type -- " &sType
        fGetType = ""
    end select

end function

'----------------------------------------------------

function fGetFieldContent( ) as String

    Call fFormatCharacter("TabHyperlinkZeichen")
    fGetFieldContent = Hinweis.GetText
    TabHyperlinkZeichen.Cancel

end function

'----------------------------------------------------

function fSetToBigSize( )

    Call fFormatCharacter("TabFont")
    try
        SizeWest.Select SizeWest.GetItemCount
    catch
        Size.Select Size.GetItemCount
    endcatch
    TabFont.OK

end function


'----------------------------------------------------

function fGetDirLanguage( ) as String

    'This function will RETURN language name for
    'install directory ,
    'eg . $SO_path/share/template/?? <- What you need to know
    Select case iSprache
    case 01   : fGetDirLanguage = "en-US"    '(English USA)
    case 31   : fGetDirLanguage = "nl"    ' Dutch
    case 33   : fGetDirLanguage = "fr-FR"    '(French)
    case 34   : fGetDirLanguage = "es-ES"    '(Spanish)
    case 39   : fGetDirLanguage = "it-IT"    '(Italian)
    case 46   : fGetDirLanguage = "sv-SE"    '(Swedish)
    case 49   : fGetDirLanguage = "de-DE"    '(German)
    case 55   : fGetDirLanguage = "pt-BR"    '(Portuguese)
    case 81   : fGetDirLanguage = "ja-JP"    '(Japanese)
    case 82   : fGetDirLanguage = "ko-KR"    '(Korean)
    case 86   : fGetDirLanguage = "zh-CN"    '(Simplified Chinese)
    case 88   : fGetDirLanguage = "zh-TW"    '(Traditional Chinese)
    case else : QAErrorLog "The test does not support the language " + iSprache
        fGetDirLanguage = "en-US"
    end select

end function

'----------------------------------------------------

function fGetCategory( ) as String

    'This function will RETURN Category when inserting from template
    Select case iSprache
    case 01   : fGetCategory = "Education"
    case 31   : fGetCategory = "Opleiding"
    case 33   : fGetCategory = "Éducation"
    case 34   : fGetCategory = "Educación"
    case 39   : fGetCategory = "Formazione"
    case 46   : fGetCategory = "Utbildning"
    case 49   : fGetCategory = "Bildung"
    case 55   : fGetCategory = "Formação"
    case 81   : fGetCategory = "教養"
    case 82   : fGetCategory = "�?사 학위 논문"
    case 86   : fGetCategory = "教育"
    case 88   : fGetCategory = "教育"
    case else : QAErrorLog "The test does not support the language " + iSprache
    end select

end function

'----------------------------------------------------

function fGetStyle( ) as String

    'This function will RETURN Style when inserting from template
    Select case iSprache
    case 01   : fGetStyle = "Master's Thesis"
    case 33   : fGetStyle = "Mémoire"
    case 34   : fGetStyle = "Trabajo de licenciatura"
    case 39   : fGetStyle = "Tesi di laurea"
    case 46   : fGetStyle = "Examensarbete"
    case 49   : fGetStyle = "Diplomarbeit"
    case 55   : fGetStyle = "Dissertação de Mestrado"
    case 81   : fGetStyle = "�?�業論文"
    case 82   : fGetStyle = "�?육"
    case 86   : fGetStyle = "硕士论文"
    case 88   : fGetStyle = "碩士論文"
    case else : QAErrorLog "The test does not support the language " + iSprache
    end select

end function

'----------------------------------------------------

function fGetDate(sDate as String) as String

    Dim sMonth , sDay , sYear as String

    'This function will RETURN Style when inserting from template

    select case iSystemSprache
    case 01   :
        sMonth = Left (sDate , 2)
        sDay   = Mid  (sDate , 4 , 2)
        sYear  = Right(sDate , 2)
    case 49   :
        sMonth = Mid  (sDate , 4 , 2)
        sDay   = Left (sDate , 2)
        sYear  = Right(sDate , 2)
    case 86   :
        sMonth = Mid  (sDate , 6 , 2)
        sDay   = Right(sDate , 2)
        sYear  = Mid  (sDate , 3 , 2)
    case else:

    end select
    fGetDate = sMonth & "/" & sDay & "/" & sYear

end function

'----------------------------------------------------

function fGetDate1(sDate as String) as String

    Dim sMonth , sDay , sYear as String

    'This function will RETURN Style when inserting from template

    select case iSystemSprache
    case 01   :
        sMonth = Left (sDate , 2)
        sDay   = Mid  (sDate , 4 , 2)
        sYear  = Right(sDate , 4)
    case 49   :
        sMonth = Mid  (sDate , 4 , 2)
        sDay   = Left (sDate , 2)
        sYear  = Right(sDate , 4)
    case 86   :
        sMonth = Mid  (sDate , 6 , 2)
        sDay   = Right(sDate , 2)
        sYear  = Left (sDate , 4)
    case else:

    end select
    fGetDate1 = sMonth & "/" & sDay & "/" & sYear

end function

'-------------------------------------------------------------------------------------------

function fConvertBacklashToSlash (sTest) as String
    '/// convert all \ to / , return the string with /
    '///+ eg, \tmp\test\optional\includes\ ==> /tmp/test/optional/includes/

    Dim i    as Integer
    Dim s1$ : Dim s2$ : Dim s3$

    s1$ = sTest

    'convert all \ to /
    do
        i = InStr( s1$, "\" )
        if i = 0 then exit do
        s2$ = Left( s1$ , i-1 )
        s1$ = Mid( s1$ , i+1 )
        s3$ = s3$ + s2$ + "/"
    loop until s1$=""
    fConvertBacklashToSlash = s3$ + s1$

end function

