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
'* short description : Sort functionality test
'*
'************************************************************************
'*
' #1 tSort_1
' #1 tSort_2
'*
'\***********************************************************************

testcase tSort_1

	Dim contentInTable() as String
	Dim i as Integer
	Dim j as Integer
	Dim sKeyType as String
	Dim sLanguage as String
	Dim TestFile as string

    if iSprache <> 01 then
        QaErrorLog "Aborting test since this test currently only work with English."
        goto endsub
    end if

	PrintLog "- Tools / Sorting in table - Ascending mode "
	for j = 1 to 3
		Select Case j
			Case 1:
				PrintLog "- 1.test for Alphanumeric"
				contentInTable = Array( _
				"FIRST NAME", "LAST NAME", "PHONE HOME", "PHONE WORK"     , _
				"Alan"      , "Brown"    , "212121"    , "(407) 555-5454" , _
				"Julie"     , "Clark"    , "321123"    , "(040) 555-007"  , _
				"Patricia"  , "Fisher"   , "121212"    , "(202) 555-4455" , _
				"Peter"     , "Arnold"   , "432104"    , "(212) 555-9876 Ext. 543" , _
				)
				TestFile = gTesttoolPath & "writer\optional\input\sorting\phonelist.odt"
				sKeyType   = fGetKeyType(01)
				sLanguage  = fGetCountryName(01)
			Case 2:
				PrintLog "- 2.test for Phonebook"
				contentInTable = Array( _
				"FIRST NAME", "LAST NAME", "PHONE HOME", "PHONE WORK"     , _
				"Alan"      , "Brown"    , "212121"    , "(407) 555-5454" , _
				"Julie"     , "Clark"    , "321123"    , "(040) 555-007"  , _
				"Patricia"  , "Fisher"   , "121212"    , "(202) 555-4455" , _
				"Peter"     , "Arnold"   , "432104"    , "(212) 555-9876 Ext. 543" , _
				)
				TestFile = gTesttoolPath + "writer\optional\input\sorting\phonelist.odt"
				sKeyType   = fGetKeyType(49)
				sLanguage  = fGetCountryName(49)
			Case 3:
				PrintLog "- 3.test for pinyin"
				contentInTable = Array( _
				"姓名"     , "电话(家)"    ,  "电话(单位)"   ,  _
				"钱二"     , "212121"    , "(407) 555-5454"           , _
				"孙三"     , "432104"    , "(212) 555-9876 Ext. 543"  , _
				"张四"     , "321123"    , "(040) 555-007"            , _
				"赵一"     , "121212"    , "(202) 555-4455"           , _
				)
				TestFile = gTesttoolPath + "writer\optional\input\sorting\phonelist_sc.odt"
				sKeyType   = fGetKeyType(86)
				sLanguage  = fGetCountryName(86)				
		end select

				
		printlog " open a test file"
		hFileOpenLocally(  TestFile )
        

		printlog " select all table"
		Call wTypeKeys "<Mod1 a>", 2

		printlog " Tools/Sort , choose  Ascending mode"
		printlog " choose English in language and Alphanumeric in keytype"
		ToolsSort
		Kontext "Sortieren"
		Sprache.Select sLanguage
		Schluessel1.Check
		Spalte1.SetText "1"
		Schluesseltyp1.Select sKeyType
		Aufsteigend1.Check
		Schluessel2.UnCheck
		Schluessel3.UnCheck
		Sortieren.OK
		Sleep 1

		Call wTypeKeys "<Mod1 a>" ,2 

		For i = 0 to UBound(contentInTable)
			Call wTypeKeys "<Mod1 a>"
			wait 100
			EditCopy
			if GetClipboardText <> contentInTable(i) then
				Warnlog "The content in table is Wrong! We hope to get " + contentInTable(i) + " But get " + GetClipboardText
			end if
			Call wTypeKeys "<Escape><TAB>"
		next i

		Call hCloseDocument
	next j
endcase

'-------------------------------------------------------------------------

testcase tSort_2

	Dim contentInTable() as String
	Dim i as Integer
	Dim j as Integer
	Dim sKeyType as String
	Dim sLanguage as String
	Dim TestFile as string

    if iSprache <> 01 then
        QaErrorLog "Aborting test since this test currently only work with English."
        goto endsub
    end if

	PrintLog "- Tools / Sorting in table - Descending mode "
	for j = 1 to 3
		Select Case j
			Case 1:
				PrintLog "- 1.test for Alphanumeric"
				contentInTable = Array( _
				"FIRST NAME", "LAST NAME", "PHONE HOME", "PHONE WORK"     , _
				"Peter"     , "Arnold"   , "432104"    , "(212) 555-9876 Ext. 543" , _
				"Patricia"  , "Fisher"   , "121212"    , "(202) 555-4455" , _
				"Julie"     , "Clark"    , "321123"    , "(040) 555-007"  , _
				"Alan"      , "Brown"    , "212121"    , "(407) 555-5454" , _
				)
				TestFile = gTesttoolPath & "writer\optional\input\sorting\phonelist.odt"
				sKeyType   = fGetKeyType(01)
				sLanguage  = fGetCountryName(01)
			Case 2:
				PrintLog "- 2.test for Phonebook"
				contentInTable = Array( _
				"FIRST NAME", "LAST NAME", "PHONE HOME", "PHONE WORK"     , _
				"Peter"     , "Arnold"   , "432104"    , "(212) 555-9876 Ext. 543" , _
				"Patricia"  , "Fisher"   , "121212"    , "(202) 555-4455" , _
				"Julie"     , "Clark"    , "321123"    , "(040) 555-007"  , _
				"Alan"      , "Brown"    , "212121"    , "(407) 555-5454" , _
				)
				TestFile = gTesttoolPath + "writer\optional\input\sorting\phonelist.odt"
				sKeyType   = fGetKeyType(49)
				sLanguage  = fGetCountryName(49)
			Case 3:
				PrintLog "- 3.test for pinyin"
				contentInTable = Array( _
				"姓名"     , "电话(家)"    ,  "电话(单位)"   ,  _
				"赵一"     , "121212"    , "(202) 555-4455"           , _
				"张四"     , "321123"    , "(040) 555-007"            , _
				"孙三"     , "432104"    , "(212) 555-9876 Ext. 543"  , _
				"钱二"     , "212121"    , "(407) 555-5454"           , _
				)
				TestFile = gTesttoolPath + "writer\optional\input\sorting\phonelist_sc.odt"
				sKeyType   = fGetKeyType(86)
				sLanguage  = fGetCountryName(86)
		end select

		printlog " open a test file"
		hFileOpenLocally(  TestFile )
        

		printlog " select all table"
		Call wTypeKeys "<Mod1 a>", 2

		printlog " Tools/Sort , choose  Ascending mode."
		printlog " choose English in language and Alphanumeric in keytype."
		ToolsSort
		Kontext "Sortieren"
		Sprache.Select sLanguage
		Schluessel1.Check
		Spalte1.SetText "1"
		Schluesseltyp1.Select sKeyType
		Absteigend1.Check
		Schluessel2.UnCheck
		Schluessel3.UnCheck
		Sortieren.OK
		Sleep 1

		Call wTypeKeys "<Mod1 a>" ,2 

		For i = 0 to UBound(contentInTable)
			Call wTypeKeys "<Mod1 a>"
			wait 100
			EditCopy
			if GetClipboardText <> contentInTable(i) then
				Warnlog "The content in table is Wrong! We hope to get " + contentInTable(i) + " But get " + GetClipboardText
			end if
			Call wTypeKeys "<Escape><TAB>"
		next i

		Call hCloseDocument
	next j
endcase

'--------------------------------------------------------------

function fGetCountryName( CountryID as Integer ) as String

  select case iSprache
      case 01   :        ' English (USA)
            select case CountryID
                  case 1:      fGetCountryName = "English (USA)"
                  case 49:     fGetCountryName = "German (Germany)"
                  case 86:     fGetCountryName = "Chinese (simplified)"
                  case else :
                               QAErrorLog "Now, the test does not support for the language " +iSprache
                               fGetCountryName = ""
            end select

        case 31:
            select case CountryID
                case 1:      fGetCountryName = "Engels (VS)"
                case 49:     fGetCountryName = "Duits (Duitsland)"
                case 86:     fGetCountryName = "Chinees (vereenvoudigd)"
                case else :
                QAErrorLog "Now, the test does not support for the language " +iSprache
                fGetCountryName = ""
           end select

      case 49   :        ' German
           select case CountryID
                  case 1:      fGetCountryName = "Englisch (USA)"
                  case 49:     fGetCountryName = "Deutsch (Deutschland)"
                  case 86:     fGetCountryName = "Chinesisch (einfach)"
                  case else :
                               QAErrorLog "Now, the test does not support for the language " +iSprache
                               fGetCountryName = ""
           end select
      case 81   :        ' Japanese
           select case CountryID
                  case 1:      fGetCountryName = "英語(米国)"
                  case 49:     fGetCountryName = "ドイツ語(ドイツ)"
                  case 86:     fGetCountryName = "中国語(簡体字)"
                  case else :
                               QAErrorLog "Now, the test does not support for the language " +iSprache
                               fGetCountryName = ""
           end select

      case 86   :        ' Chinese (simplified)
          select case CountryID
                 case 1:      fGetCountryName = "英语(美国)"
                 case 49:     fGetCountryName = "德语(德国)"
                 case 86:     fGetCountryName = "中文(简体字)"
                 case else :
                              QAErrorLog "Now, the test does not support for the language " +iSprache
                              fGetCountryName = ""
          end select

      case 88   :        ' Chinese (traditional)
           select case CountryID
                  case 1:      fGetCountryName = "英語(美國)"
                  case 49:     fGetCountryName = "德語(德國)"
                  case 86:     fGetCountryName = "中文(簡體字)"
                  case else :
                               QAErrorLog "Now, the test does not support for the language " +iSprache
                               fGetCountryName = ""
           end select
      case else :        ' Fallback
                  QAErrorLog "Now, the test does not support for the language " +iSprache
                  fGetCountryName = ""
  end select
end function

'-------------------------------------------------------

function fGetKeyType( KeyType as Integer ) as String

    select case iSprache
        case 01, 31, 49   :
            select case KeyType
                case 1:      fGetKeyType   = "Alphanumeric"
                case 49:     fGetKeyType   = "Phone book"
                case 86:     fGetKeyType   = "Pinyin"
                case else :
                QAErrorLog "Now, the test does not support for the language " +iSprache
                fGetKeyType = ""
            end select

        case 81   :' Japanese
            select case KeyType
                case 1:      fGetKeyType   = "英数字"
                case 49:     fGetKeyType   = "電話帳"
                case 86:     fGetKeyType   = "ピンイン(中国語のローマ字表記法)"
                case else :
                QAErrorLog "Now, the test does not support for the language " +iSprache
                fGetKeyType = ""
            end select

        case 86   :' Chinese (simplified)
            select case KeyType
                case 1:      fGetKeyType   = "字母数字式"
                case 49:     fGetKeyType   = "电话簿"
                case 86:     fGetKeyType   = "拼音"
                case else :
                QAErrorLog "Now, the test does not support for the language " +iSprache
                fGetKeyType = ""
            end select

        case 88   :' Chinese (traditional)
            select case KeyType
                case 1:      fGetKeyType   = "字母數字式"
                case 49:     fGetKeyType   = "電話簿"
                case 86:     fGetKeyType   = "拼音"
                case else :
                QAErrorLog "Now, the test does not support for the language " +iSprache
                fGetKeyType = ""
            end select

        case else :' Fallback
            QAErrorLog "The test does not support the language " + iSprache
            fGetKeyType = ""
    end select

end function
