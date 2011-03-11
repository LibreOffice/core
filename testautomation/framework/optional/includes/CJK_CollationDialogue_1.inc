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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : CJK Collation Dialogue Design Draft
'*
'\***********************************************************************

testcase tLanguageTest_1

    Dim sLanguageIDEnglishUSA ,sLanguageIDSChinese, sLanguageIDTChinese , sLanguageIDJapanese , sLanguageIDKoren as String
    Dim sKeyTypeEnglishUSA    ,sKeyTypeSChinese   , sKeyTypeTChinese    , sKeyTypeJapanese    , sKeyTypeKoren    as String
    Dim i as Integer

    '/// Check if the default setting for &quot;key type&quot; is right for CJK when alphanumeric is chosen in English

    select case iSprache
    case 01   :' English (USA)
        sKeyTypeEnglishUSA = "Alphanumeric"
        sKeyTypeSChinese   = "Character set"
        sKeyTypeTChinese   = "Numeric"
        sKeyTypeJapanese   = "Character set"
        sKeyTypeKoren      = "Character set"
    case 49   :' German
        sKeyTypeEnglishUSA = "Alphanumerisch"
        sKeyTypeSChinese   = "Numerisch"
        sKeyTypeTChinese   = "Numerisch"
        sKeyTypeJapanese   = "Numerisch"
        sKeyTypeKoren      = "Numerisch"
    case 50   :' Slovenian
        sKeyTypeEnglishUSA = "Alfanumerično"
        sKeyTypeSChinese   = "Nabor znakov"
        sKeyTypeTChinese   = "Numerično"
        sKeyTypeJapanese   = "Nabor znakov"
        sKeyTypeKoren      = "Nabor znakov"
    case 55   :' Brazilian
        sKeyTypeEnglishUSA = "Alfanumérico"
        sKeyTypeSChinese   = "Conjunto de caracteres"
        sKeyTypeTChinese   = "numérico"
        sKeyTypeJapanese   = "Conjunto de caracteres"
        sKeyTypeKoren      = "Conjunto de caracteres"
    case 81   :' Japanese
        sKeyTypeEnglishUSA = "英数字"
        sKeyTypeSChinese   = "ピンイン(中国語のローマ字表記法)"
        sKeyTypeTChinese   = "ピンイン(中国語のローマ字表記法)"
        sKeyTypeJapanese   = "数字"
        sKeyTypeKoren      = "数字"
        '      case 82   : ' Korean
        '                   sKeyTypeEnglishUSA = 1 'alphanumeric
        '                   sKeyTypeSChinese   = 1 'Character set
        '                   sKeyTypeTChinese   = 3 'Pin yin
        '                   sKeyTypeJapanese   = 2 'Character set
        '                   sKeyTypeKoren      = 3 'Character set
    case 86   :' Chinese (simplified)
        sKeyTypeEnglishUSA = "字母数字式"
        sKeyTypeSChinese   = "字符集"
        sKeyTypeTChinese   = "拼音"
        sKeyTypeJapanese   = "字符集"
        sKeyTypeKoren      = "字典"
    case 88   :' Chinese (traditional)
        sKeyTypeEnglishUSA = "字母數字式"
        sKeyTypeSChinese   = "字型"
        sKeyTypeTChinese   = "拼音"
        sKeyTypeJapanese   = "字型"
        sKeyTypeKoren      = "字典"
    case else :' Fallback
        PrintLog "The test does not support the language " + iSprache
        Goto endsub
    end select

    sLanguageIDEnglishUSA = fGetCountryName(01)
    sLanguageIDSChinese   = fGetCountryName(86)
    sLanguageIDTChinese   = fGetCountryName(88)
    sLanguageIDJapanese   = fGetCountryName(81)
    sLanguageIDKoren      = fGetCountryName(82)

    call hNewDocument

    DocumentWriter.TypeKeys "test"
    DocumentWriter.TypeKeys "<Shift Home>"
    Sleep 1

    '/// Open Tools/Sort
    ToolsSort
    Kontext "Sortieren"

    '/// Choose language: English and key type: alphanumeric
    try
        Sprache.Select (sLanguageIDEnglishUSA)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        try
            Sprache.Select ("English (USA)")
        catch
            qaErrorLog "Can't select even tha fallback: 'English (USA)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeEnglishUSA
    catch
        qaErrorLog "Can't select: '" + sKeyTypeEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        for i = 1 to Schluesseltyp1.getItemCount
            printlog "" + i + ": '" + Schluesseltyp1.getItemText(i) + "'"
        next i
    endcatch

    '/// Check to simplified Chinese
    try
        Sprache.Select (sLanguageIDSChinese)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDSChinese + "'"
        ' try fallback to 'Chinese (simplyfied)'
        try
            Sprache.Select ("Chinese (simplyfied)")
        catch
            qaErrorLog "Can't select even tha fallback: 'Chinese (simplyfied)'"
        endcatch
    endcatch
    Sleep 2

    try
        Sprache.Select (sLanguageIDEnglishUSA)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        try
            Sprache.Select ("English (USA)")
        catch
            qaErrorLog "Can't select even tha fallback: 'English (USA)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeEnglishUSA
    catch
        qaErrorLog "Can't select: '" + sKeyTypeEnglishUSA + "'"
    endcatch

    '/// Check to traditional Chinese
    try
        Sprache.Select (sLanguageIDTChinese)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDTChinese + "'"
        ' try fallback to 'Chinese (traditional)'
        try
            Sprache.Select ("Chinese (traditional)")
        catch
            qaErrorLog "Can't select even tha fallback: 'Chinese (traditional)'"
            for i = 1 to Sprache.getItemCount
                printlog "("+i+"/"+ Sprache.getItemCount + "): '"+Sprache.getItemText(i)+"'"
            next i
        endcatch
    endcatch
    Sleep 2

    try
        Sprache.Select (sLanguageIDEnglishUSA)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        try
            Sprache.Select ("English (USA)")
        catch
            qaErrorLog "Can't select even tha fallback: 'English (USA)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeEnglishUSA
    catch
        qaErrorLog "Can't select: '" + sKeyTypeEnglishUSA + "'"
    endcatch

    '/// Check to Japanese
    try
        Sprache.Select (sLanguageIDJapanese)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDJapanese + "'"
        ' try fallback to 'Japanese'
        try
            Sprache.Select ("Japanese")
        catch
            qaErrorLog "Can't select even tha fallback: 'Japanese'"
        endcatch
    endcatch
    Sleep 2
    if Schluesseltyp1.GetSelText <> sKeyTypeJapanese then
        warnlog "The default key type for Japanese should be: '" + sKeyTypeJapanese + "' but is: '" + Schluesseltyp1.GetSelText + "'"
    end if

    try
        Sprache.Select (sLanguageIDEnglishUSA)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        try
            Sprache.Select ("English (USA)")
        catch
            qaErrorLog "Can't select even tha fallback: 'English (USA)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeEnglishUSA
    catch
        qaErrorLog "Can't select: '" + sKeyTypeEnglishUSA + "'"
    endcatch

    '/// Check to Korean
    try
        Sprache.Select (sLanguageIDKoren)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDKoren + "'"
        ' try fallback to 'Korean (RoK)'
        try
            Sprache.Select ("Korean (RoK)")
        catch
            qaErrorLog "Can't select even tha fallback: 'Korean (RoK)'"
            for i = 1 to Sprache.getItemCount
                printlog "("+i+"/"+ Sprache.getItemCount + "): '"+Sprache.getItemText(i)+"'"
            next i
        endcatch
    endcatch
    Sleep 2
    if Schluesseltyp1.GetSelText <> sKeyTypeKoren then
        warnlog "The default key type for Korean should be " +  sKeyTypeKoren + " but get " + Schluesseltyp1.GetSelText
    end if

    Sortieren.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tLanguageTest_2

    Dim sLanguageIDEnglishUSA ,sLanguageIDSChinese, sLanguageIDTChinese , sLanguageIDJapanese , sLanguageIDKoren as String
    Dim sKeyTypeEnglishUSA    ,sKeyTypeSChinese   , sKeyTypeTChinese    , sKeyTypeJapanese    , sKeyTypeKoren    as String
    Dim i                                                                                                        as Integer

    '/// Check if the default setting for &quot;key type&quot; is right for CJK when numeric is chosen in English

    select case iSprache
    case 01   :' English (USA)
        sKeyTypeEnglishUSA = "Numeric"
        sKeyTypeSChinese   = "Numeric"
        sKeyTypeTChinese   = "Numeric"
        sKeyTypeJapanese   = "Numeric"
        sKeyTypeKoren      = "Numeric"
    case 49   :' German
        sKeyTypeEnglishUSA = "Numerisch"
        sKeyTypeSChinese   = "Numerisch"
        sKeyTypeTChinese   = "Numerisch"
        sKeyTypeJapanese   = "Numerisch"
        sKeyTypeKoren      = "Numerisch"
    case 50   :' Slovenian
        sKeyTypeEnglishUSA = "Alfanumerično"
        sKeyTypeSChinese   = "Nabor znakov"
        sKeyTypeTChinese   = "Numerično"
        sKeyTypeJapanese   = "Nabor znakov"
        sKeyTypeKoren      = "Nabor znakov"
    case 55   :' Brazilian
        sKeyTypeEnglishUSA = "numérico"
        sKeyTypeSChinese   = "numérico"
        sKeyTypeTChinese   = "numérico"
        sKeyTypeJapanese   = "numérico"
        sKeyTypeKoren      = "numérico"
    case 81   :' Japanese
        sKeyTypeEnglishUSA = "数字"
        sKeyTypeSChinese   = "数字"
        sKeyTypeTChinese   = "数字"
        sKeyTypeJapanese   = "数字"
        sKeyTypeKoren      = "数字"
        '      case 82    ' Korean
        '                   sKeyTypeEnglishUSA = 2 'numeric
        '                   sKeyTypeSChinese   = 4 'numeric
        '                   sKeyTypeTChinese   = 4 'numeric
        '                   sKeyTypeJapanese   = 4 'numeric
        '                   sKeyTypeKoren      = 4 'numeric
    case 86   :' Chinese (simplified)
        sKeyTypeEnglishUSA = "数字"
        sKeyTypeSChinese   = "数字"
        sKeyTypeTChinese   = "数字"
        sKeyTypeJapanese   = "数字"
        sKeyTypeKoren      = "数字"
    case 88   :' Chinese (traditional)
        sKeyTypeEnglishUSA = "數字"
        sKeyTypeSChinese   = "數字"
        sKeyTypeTChinese   = "數字"
        sKeyTypeJapanese   = "數字"
        sKeyTypeKoren      = "數字"
    case else :' Fallback
        printLog "The test does not support the language " + iSprache
        Goto endsub
    end select

    sLanguageIDEnglishUSA = fGetCountryName(01)
    sLanguageIDSChinese   = fGetCountryName(86)
    sLanguageIDTChinese   = fGetCountryName(88)
    sLanguageIDJapanese   = fGetCountryName(81)
    sLanguageIDKoren      = fGetCountryName(82)

    call hNewDocument

    DocumentWriter.TypeKeys "123"
    DocumentWriter.TypeKeys "<Shift Home>"
    Sleep 1

    '/// Open Tools/Sort
    ToolsSort
    Kontext "Sortieren"

    '/// Choose language: English and key type: Numeric
    try
        Sprache.Select (sLanguageIDEnglishUSA)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        try
            Sprache.Select ("English (USA)")
        catch
            qaErrorLog "Can't select even tha fallback: 'English (USA)'"
            for i = 1 to Sprache.getItemCount
                printlog "" + i + ": '" + Sprache.getItemText(i) + "'"
            next i
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeEnglishUSA
    catch
        qaErrorLog "Can't select: '" + sKeyTypeEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        for i = 1 to Schluesseltyp1.getItemCount
            printlog "" + i + ": '" + Schluesseltyp1.getItemText(i) + "'"
        next i
    endcatch

    '/// Check to simplified Chinese
    try
        Sprache.Select (sLanguageIDSChinese)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDSChinese + "'"
        ' try fallback to 'Chinese (simplyfied)'
        try
            Sprache.Select ("Chinese (simplyfied)")
        catch
            qaErrorLog "Can't select even tha fallback: 'Chinese (simplyfied)'"
        endcatch
    endcatch
    Sleep 2
    if Schluesseltyp1.GetSelText <> sKeyTypeSChinese then
        warnlog "The default key type for Simplified Chinese should be: '" +  sKeyTypeSChinese + "' but is '" + Schluesseltyp1.GetSelText + "'"
    end if

    try
        Sprache.Select (sLanguageIDEnglishUSA)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        try
            Sprache.Select ("English (USA)")
        catch
            qaErrorLog "Can't select even tha fallback: 'English (USA)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeEnglishUSA
    catch
        qaErrorLog "Can't select: '" + sKeyTypeEnglishUSA + "'"
    endcatch

    '/// Check to traditional Chinese
    try
        Sprache.Select (sLanguageIDTChinese)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDTChinese + "'"
        ' try fallback to 'Chinese (traditional)'
        try
            Sprache.Select ("Chinese (traditional)")
        catch
            qaErrorLog "Can't select even tha fallback: 'Chinese (traditional)'"
        endcatch
    endcatch
    Sleep 2
    if Schluesseltyp1.GetSelText <> sKeyTypeTChinese then
        warnlog "The default key type for traditional Chinese should be " +  sKeyTypeTChinese + " but get " +  Schluesseltyp1.GetSelText
    end if

    try
        Sprache.Select (sLanguageIDEnglishUSA)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        try
            Sprache.Select ("English (USA)")
        catch
            qaErrorLog "Can't select even tha fallback: 'English (USA)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeEnglishUSA
    catch
        qaErrorLog "Can't select: '" + sKeyTypeEnglishUSA + "'"
    endcatch

    '/// Check to Japanese
    try
        Sprache.Select (sLanguageIDJapanese)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDJapanese + "'"
        ' try fallback to 'Japanese'
        try
            Sprache.Select ("Japanese")
        catch
            qaErrorLog "Can't select even tha fallback: 'Japanese'"
        endcatch
    endcatch
    Sleep 2
    if Schluesseltyp1.GetSelText <> sKeyTypeJapanese then
        warnlog "The default key type for Japanese should be: '" + sKeyTypeJapanese + "' but is: '" + Schluesseltyp1.GetSelText + "'"
    end if

    try
        Sprache.Select (sLanguageIDEnglishUSA)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDEnglishUSA + "'"
        ' try fallback to 'English (USA)'
        try
            Sprache.Select ("English (USA)")
        catch
            qaErrorLog "Can't select even tha fallback: 'English (USA)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeEnglishUSA
    catch
        qaErrorLog "Can't select: '" + sKeyTypeEnglishUSA + "'"
    endcatch

    '/// Check to Korean
    try
        Sprache.Select (sLanguageIDKoren)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDKoren + "'"
        ' try fallback to 'Korean (RoK)'
        try
            Sprache.Select ("Korean (RoK)")
        catch
            qaErrorLog "Can't select even tha fallback: 'Korean (RoK)'"
        endcatch
    endcatch
    Sleep 2
    if Schluesseltyp1.GetSelText <> sKeyTypeKoren then
        warnlog "The default key type for Korean should be " +  sKeyTypeKoren + " but get " + Schluesseltyp1.GetSelText
    end if

    Sortieren.Cancel

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tStrokeSort

    Dim testFile              as String
    Dim sLanguageIDSChinese   as String
    Dim sKeyTypeSChinese      as String
    Dim aCorrectSequence()    as String

    Dim i , j                 as Integer

    testFile         = gTesttoolPath & "framework\optional\input\CJK\sortTest.sxw"
    testFile         = convertpath( testFile )
    aCorrectSequence = Array( "1" , "2" , "3" , "4" ,"5" )

    j = UBound(aCorrectSequence)

    select case iSprache
    case 01   :  ' English (USA)
        sKeyTypeSChinese      =  "Stroke"
    case 49   :  ' German
        sKeyTypeSChinese      =  "Stroke"
    case 50   :  ' Slovenian
        sKeyTypeSChinese      =  "Poteza"
    case 55   :  ' Brazilian
        sKeyTypeSChinese      =  "Traço"
    case 81   :  ' Japanese
        sKeyTypeSChinese      =  "総画数"
    case 86   :  ' Chinese (simplified)
        sKeyTypeSChinese      =  "笔画"
    case 88   :  ' Chinese (simplified)
        sKeyTypeSChinese      =  "筆畫"
    case else :  ' Fallback
        printLog "Now, the test does not support for the language " +iSprache
        Goto endsub
    end select

    sLanguageIDSChinese =  fGetCountryName(86)

    '/// Check Stroke sort for Simplified Chinese

    Call hNewDocument
    '/// Open a test file and check <i>No Grid</i> checkbox
    hFileOpenLocally( testfile )
    hUseAsyncSlot( "EditSelectAll" )

    '/// Open <i>Tools/Sort</i>
    ToolsSort
    Kontext "Sortieren"

    '/// Choose language: Simplified Chinese and key type <i>Stroke</i>
    try
        Sprache.Select (sLanguageIDSChinese)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDSChinese + "'"
        ' try fallback to 'Chinese (simplyfied)'
        try
            Sprache.Select ("Chinese (simplyfied)")
        catch
            qaErrorLog "Can't select even tha fallback: 'Chinese (simplyfied)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeSChinese
    catch
        qaErrorLog "Can't select: '" + sKeyTypeSChinese + "'"
        for i = 1 to Schluesseltyp1.getItemCount
            printlog "" + i + ": '" + Schluesseltyp1.getItemText(i) + "'"
        next i
    endcatch

    Sortieren.OK

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<MOD1 Home>"

    For i = 0 to j
        DocumentWriter.TypeKeys "<End>"
        DocumentWriter.TypeKeys "<Shift Left>"

        EditCopy
        if GetClipboardText <> aCorrectSequence(i) then Warnlog "The Stroke sort is wrong , should be " +  aCorrectSequence(i) + " ,but get " +GetClipboardText

        DocumentWriter.TypeKeys "<Down>"
    next i

    Call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-------------------------------------------------------------------------

testcase tPinyinSort

    Dim testFile              as String
    Dim sLanguageIDSChinese   as String
    Dim sKeyTypeSChinese      as String
    Dim aCorrectSequence()    as String

    Dim i , j                 as Integer

    testFile         = gTesttoolPath & "framework\optional\input\CJK\sortTest.sxw"
    testFile         = convertpath( testFile )

    aCorrectSequence = Array( "2" , "3" , "4" , "1" , "5"  )

    j = UBound(aCorrectSequence)

    select case iSprache
    case 01   :  ' English (USA)
        sKeyTypeSChinese      =  "Pinyin"
    case 49   :  ' German
        sKeyTypeSChinese      =  "Pinyin"
    case 50   :  ' Slovenian
        sKeyTypeSChinese      =  "Pinjin"
    case 55   :  ' Brazilian
        sKeyTypeSChinese      =  "Pinyin"
    case 81   :  ' Japanese
        sKeyTypeSChinese      =  "ピンイン(中国語のローマ字表記法)"
    case 86   :  ' Chinese (simplified)
        sKeyTypeSChinese      =  "拼音"
    case 88   :  ' Chinese (simplified)
        sKeyTypeSChinese      =  "拼音"
    case else :  ' Fallback
        printLog "Now, the test does not support for the language " +iSprache
        Goto endsub
    end select

    sLanguageIDSChinese =  fGetCountryName(86)

    '/// Check <i>Pinyin</i> sort for Simplified Chinese

    Call hNewDocument
    '/// Open a test file and check <i>No Grid</i> checkbox
    hFileOpenLocally( testfile )
    hUseAsyncSlot( "EditSelectAll" )

    '/// Open <i>Tools/Sort</i>
    ToolsSort
    Kontext "Sortieren"

    '/// Choose language: Simplified Chinese and key type <i>Pin yin</i>
    try
        Sprache.Select (sLanguageIDSChinese)
    catch
        qaErrorLog "Can't select: '" + sLanguageIDSChinese + "'"
        ' try fallback to 'Chinese (simplyfied)'
        try
            Sprache.Select ("Chinese (simplyfied)")
        catch
            qaErrorLog "Can't select even tha fallback: 'Chinese (simplyfied)'"
        endcatch
    endcatch
    try
        Schluesseltyp1.Select sKeyTypeSChinese
    catch
        qaErrorLog "Can't select: '" + sKeyTypeSChinese + "'"
        for i = 1 to Schluesseltyp1.getItemCount
            printlog "" + i + ": '" + Schluesseltyp1.getItemText(i) + "'"
        next i
    endcatch

    Sortieren.OK

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<MOD1 Home>"

    For i = 0 to j
        DocumentWriter.TypeKeys "<End>"
        DocumentWriter.TypeKeys "<Shift Left>"

        EditCopy
        if GetClipboardText <> aCorrectSequence(i) then
            Warnlog "The Pin yin sort is wrong, should be: '" +  aCorrectSequence(i) + "' ,but is: '" +GetClipboardText + "'"
        endif

        DocumentWriter.TypeKeys "<Down>"
    next i

    Call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-------------------------------------------------------------------------

testcase tSortOptionTest_1

    Dim  i  , j  as Integer
    Dim asList() as String

    select case iSprache
    case 81   :' Japanese
        if iSystemSprache <> 81 then goto NOTest
        asList = Array( "Sun,Mon,Tue,Wed,Thu,Fri,Sat"     , "Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday"    , "Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec"    , "January,February,March,April,May,June,July,August,September,October,November,December"    , "日,月,火,水,木,金,土"    , "日曜日,月曜日,火曜日,水曜日,木曜日,金曜日,土曜日"    , )
    case 82   :' Korean
        if iSystemSprache <> 82 then goto NOTest
        asList = Array( "일,월,화,수,목,금,토" ,"일요일,월요일,화요일,수요일,목요일,금요일,토요일"  ,"1,2,3,4,5,6,7,8,9,10,11,12"    , "日,月,火,水,木,金,土"    , "日曜日,月曜日,火曜日,水曜日,木曜日,金曜日,土曜日"    , )
    case 86   :' Chinese (simplified)
        if iSystemSprache <> 86 then goto NOTest
        asList = Array( "日,一,二,三,四,五,六"     , "星期日,星期一,星期二,星期三,星期四,星期五,星期六"    , "一,二,三,四,五,六,七,八,九,十,十一,十二"    , "一月,二月,三月,四月,五月,六月,七月,八月,九月,十月,十一月,十二月"    , )
    case 88   :' Chinese (traditional)
        if iSystemSprache <> 88 then goto NOTest
        asList = Array( "日,一,二,三,四,五,六"     , "星期日,星期一,星期二,星期三,星期四,星期五,星期六"    , "一,二,三,四,五,六,七,八,九,十,十一,十二"    , "一月,二月,三月,四月,五月,六月,七月,八月,九月,十月,十一月,十二月"    , )
    case else :' Fallback
        NOTest:
        printLog "The test does not support for the language " +iSprache + " System language " + iSystemSprache
        Goto endsub
    end select

    '/// Check if the entries in custom sort order is right in CJK version

    call hNewDocument

    '/// Open <i>Data/Sort</i> -&gt; tabpage <i>option</i>

    DataSort

    Kontext
    active.SetPage TabSortierenOptionen
    Kontext "TabSortierenOptionen"

    BenutzerdefinierteSortierreihenfolge.Check

    For i = 1 to SortierreihenfolgeListe.GetItemCount
        if  SortierreihenfolgeListe.GetItemText(i) <> asList(i-1) then
            warnlog "Should get " +asList(i) + " but get " +SortierreihenfolgeListe.GetItemText(i)
        end if
    next i

    TabSortierenOptionen.Cancel

    Call hCloseDocument

endcase

'----------------------------------------------------------------
