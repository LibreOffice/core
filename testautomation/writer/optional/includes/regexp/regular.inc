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
'* short description : Search for regular expressions
'*
'\***********************************************************************

sub regular
    call tAny_Character
    call tWord_At_Paragraph_Beginning
    call tWord_At_Paragraph_End
    call tNone_Or_Several_Between
    call tAtLeast_One_Between
    call tText_At_WordBeginning
    call tText_At_WordEnd
    call tDolar
    call tTabulator
    call tOr_Search
    call tEmpty_Paragraph
    call tLineBreak
    call tSingle_Selected
    call tOne_Area_Selected
    call tTwo_Areas_Selected
    call tHexadecimal_Code
    call tHexadecimal_Code_NEW
    call tAttach_Behind_WordToFind
    Call tWord_At_Beginning_With_Empty_Field
    Call tWord_At_Beginning_With_Frame
    Call tWord_At_End_With_Empty_Field
    Call tWord_At_End_With_Frame
'---------New------------
   call tChartype_digit
   call tChartype_space
   call tChartype_print
   call tChartype_alnum
   call tChartype_alpha
   call tChartype_cntrl

   call tChartype_lower
   call tChartype_upper
end sub

'---------------------------------------------------------
' Wort mit einem beliebigen Zeichen im Wort Search : geht mit '.'
'
testcase tAny_Character
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///
   RegularExpressions.Check '/// hmmm, one time set, evry time work : just here gets regexp checked ?!///
   '/// Start Search & Compare ///
   Such_Text = "M.ller"             'beliebiges Zeichen im Wort
   '///+ Enter RegExp in Field Search for : 'M.ller'  ///
   Vergleichstext = "MöllerMüller"
   '/// Compare string found with: 'M�llerM�ller' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
'///' Wort am Absatzanfang Search : geht mit '^' ///
'
testcase tWord_At_Paragraph_Beginning
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text = "^Anfang"               'Wort am Absatzanfang
   '///+ Enter RegExp in Field Search for : '^Anfang'  ///
   Vergleichstext = "Anfang"
   '/// Compare string found with: 'Anfang' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' Wort am Absatzende Search : geht mit 'xxx$' - Dollarzeichen
'
testcase tWord_At_Paragraph_End
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text = "Ende$"              'Wort am Absatzende
   '///+ Enter RegExp in Field Search for : 'Ende$'  ///
   Vergleichstext = "Ende"
   '/// Compare string found with: 'Ende' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' Wort oder Satzteile Search. Zwischen den Worten mit dem Suchbegriff darf kein oder mehrere Buchstaben stehen : geht mit '.*'
'
testcase tNone_Or_Several_Between
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "Peter.*Haus"               'Kein oder mehrere Zeichen dazwischen
   '///+ Enter RegExp in Field Search for : 'Peter.*Haus'  ///
   Vergleichstext = "Peter ist im HausPeter liebt dieses HausPeters HausPeterhaus"
   '/// Compare string found with: 'Peter ist im HausPeter liebt dieses HausPeters HausPeterhaus' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' Wort oder Satzteile Search. Zwischen den Worten mit dem Suchbegriff mu├ö├Â┬úÔö£├é├ö├Â┬úÔö£├®├ö├Â┬╝Ôö£Ôòæ├ö├Â┬úÔö£Ôòæ├ö├Â┬úÔö£├æ mindestens ein Buchstabe stehen : geht mit '.+'
'
testcase tAtLeast_One_Between
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "Peter.+Haus"
   '///+ Enter RegExp in Field Search for : 'Peter.+Haus'  ///
   Vergleichstext = "Peter ist im HausPeter liebt dieses HausPeters Haus"
   '/// Compare string found with: 'Peter ist im HausPeter liebt dieses HausPeters Haus' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' Worte mit Vergleichtext am Wortanfang Search : geht mit '\<'
'
testcase tText_At_WordBeginning
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "\<aaa"
   '///+ Enter RegExp in Field Search for : '\<aaa'  ///
   Vergleichstext = "aaa"
   '/// Compare string found with: 'aaa' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' Worte mit Vergleichtext am Wortende Search : geht mit '\>'
'
testcase tText_At_WordEnd
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "eee\>"               'Text am Wortende
   '///+ Enter RegExp in Field Search for : 'eee\>'  ///
   Vergleichstext = "eee"
   '/// Compare string found with: 'eee' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' Normale mit regulren Ausdrcke Search, hier das Dollarzeichen geht mit \$

testcase tDolar
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "\$"                  'Sonderzeichen ($)
   '///+ Enter RegExp in Field Search for : '\$'  ///
   Vergleichstext = "$$"
   '/// Compare string found with: '$$' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht nach Tabulatoren : geht mit \t
'
testcase tTabulator
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "\t"                  'Tabulatoren
   '///+ Enter RegExp in Field Search for : '\t'  ///
   Vergleichstext = Chr$(9)+Chr$(9)+Chr$(9)
   '/// Compare string found with: '            ' ///
   call Search_Und_Vergleichen
' findet nun t
endcase

'---------------------------------------------------------
' entweder das eine Wort oder das andere Wort Search : geht mit 'aaa|bbb'
'
testcase tOr_Search
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "Erstes|Zweites"            'ODER-Suche
   '///+ Enter RegExp in Field Search for : 'Erstes|Zweites'  ///
   Vergleichstext = "ZweitesErstesZweites"
   '/// Compare string found with: 'ZweitesErstesZweites' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht leere Abstze : geht mit '^$'
'
testcase tEmpty_Paragraph
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "^$"
   '///+ Enter RegExp in Field Search for : '^$'  ///
   if gPlatgroup = "unx" then
        Vergleichstext = Chr$(10)
   else
        Vergleichstext = Chr$(13) + Chr$(10)
   end if
   '/// Compare string found with: 'An empty paragraph' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht nach einem weichen Zeilenumbruch ( linefeed, newline ): geht mit '\n'
'
testcase tLineBreak
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "\n"                  'Zeilenumbruch
   '///+ Enter RegExp in Field Search for : '\n'  ///
   Vergleichstext = Chr$(10)
   '/// Compare string found with: a line break ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht alle ausgewlten Zeichen  : geht mit '[xyz]'
'
testcase tSingle_Selected
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[xyz]"               'Einzelne ausgewhlte Zeichen
   '///+ Enter RegExp in Field Search for : '[xyz]'  ///
   Vergleichstext = "xZzzZzxxzzZZzZzXxYyZz"
   '/// Compare string found with: 'xZzzZzxxzzZZzZzXxYyZz' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht alle Zeichen im Bereich zwischen zwei Buchstaben : geht mit '[x-z]'
'
testcase tOne_Area_Selected
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[w-z]"               'Bereich ausgewhlter Zeichen
   '///+ Enter RegExp in Field Search for : '[w-z]'  ///
   Vergleichstext = "xZWWzWzZzwWWxWwxWwzzZwWWwZwWzZzWwXxYyZz"
   '/// Compare string found with: 'xZWWzWzZzwWWxWwxWwzzZwWWwZwWzZzWwXxYyZz' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht alle Zeichen aus zwei angegebenen Buchstabenbereichen : geht mit '[a-cg-h]'
'
testcase tTwo_Areas_Selected
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text = "[B-Dx-z]"              'Zwei Bereiche ausgewhlter Zeichen
   '///+ Enter RegExp in Field Search for : '[B-Dx-z]'  ///
   Vergleichstext = "xdcbbZcddcddbzddcbddbzddcbddZczcdbdcDccxddcxddddzcbdbzZDddZbzZcbdzcBbCcDdXxYyZzcddDd"
   '/// Compare string found with: 'xdcbbZcddcddbzddcbddbzddcbddZczcdbdcDccxddcxddddzcbdbzZDddZbzZcbdzcBbCcDdXxYyZzcddDd' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht alle Zeichen mit dem Hexadezimalcode .. : geht mit '\x..'
'
testcase tHexadecimal_Code_NEW
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "\x3E"             'Zeichen mit Hexadezimal-Code finden
   '///+ Enter RegExp in Field Search for : '\x3E'  ///
   Vergleichstext = ">"
   '/// Compare string found with: '>' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht alle Zeichen mit dem Hexadezimalcode .. : geht mit '\x..'
'
testcase tHexadecimal_Code
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "\x003E"             'Zeichen mit Hexadezimal-Code finden
   '///+ Enter RegExp in Field Search for : '\x003E'  ///
   Vergleichstext = ">"
   '/// Compare string found with: '>' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht Wort am Anfang eines Absatzes mit leerem Feldbefehl davor : geht mit '^'
'
testcase tWord_At_Beginning_With_Empty_Field
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "^Halloo"
   '///+ Enter RegExp in Field Search for : '^Halloo'  ///
   Vergleichstext = "Halloo"
   '/// Compare string found with: 'Halloo' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht Wort am Anfang eines Absatzes mit zeichengebundenen Rahmen davor : geht mit '^'
'
testcase tWord_At_Beginning_With_Frame
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "^Rahmen"
   '///+ Enter RegExp in Field Search for : '^Rahmen'  ///
   Vergleichstext = "Rahmen"
   '/// Compare string found with: 'Rahmen' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht Wort am Ende eines Absatzes mit leerem Feldbefehl davor : geht mit '^'
'
testcase tWord_At_End_With_Empty_Field
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "hhallo$"
   '///+ Enter RegExp in Field Search for : 'hhallo$'  ///
   Vergleichstext = "hhallo"
   '/// Compare string found with: 'hhallo' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
' sucht Wort am Ende eines Absatzes mit zeichengebundenen Rahmen davor : geht mit '^'
'
testcase tWord_At_End_With_Frame
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "Rahmenend$"
   '///+ Enter RegExp in Field Search for : 'Rahmenend$'  ///
   Vergleichstext = "Rahmenend"
   '/// Compare string found with: 'Rahmenend' ///
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
testcase tAttach_Behind_WordToFind
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// look for 'Sonderzeichen' replace with '& sind toll' ///'
   '/// Start Search & Compare ///
   Such_Text = "Sonderzeichen"               'Anh├ö├Â┬úÔö£├é├ö├Â┬úÔö£├®├ö├Â┬╝Ôö£ÔòæÔö£├ÂÔö£├éÔö¼├║Ôö£├ÂÔö£ÔòùÔö£├Ñngen an gesuchtes Wort
   '///+ Enter RegExp in Field Search for : 'Sonderzeichen'  ///
   SearchFor.SetText Such_Text
   ReplaceWith.SetText "& sind toll"
   ReplaceAll.Click
   sleep 1
   Kontext "Active"
   if Active.Exists then
    if Active.GetRT = 304 then
     Active.OK                     'MSG-Box (... x ersetzt) schlie�en
    else
     Warnlog "Messagebox, how often replaced, not up !"
    end if
   end if
   Kontext "FindAndReplace"
   FindAndReplace.Close
   kontext "DocumentWriter"
   DocumentWriter.Typekeys "<PAGEUP>"
   DocumentWriter.Typekeys "<MOD1 HOME>"
   EditSearchAndReplace
   kontext "FindAndReplace"            'Search&Replace-Fenster ├ö├Â┬úÔö£├é├ö├Â┬úÔö£├®├ö├Â┬╝Ôö£ÔòæÔö£├ÂÔö£├éÔö¼├║├ö├Â┬úÔö¼┬½ffnen
   if RegularExpressions.IsVisible = False then More.Click
   RegularExpressions.Uncheck
   Such_Text      = "Sonderzeichen sind toll"
   Vergleichstext = "Sonderzeichen sind tollSonderzeichen sind toll"
   call Search_Und_Vergleichen
endcase

'--------------NEW ONES from TBO; to SO 6.0---------------------------------

testcase tChartype_digit
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[:digit:]*"
   '///+ Enter RegExp in Field Search for : '[:digit:]*'  ///
   Vergleichstext = "²³¹¼½¾"
   '/// Compare string found with: '' ///
   call Search_Und_Vergleichen
endcase

testcase tChartype_space
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[:space:]*"
   '///+ Enter RegExp in Field Search for : '[:space:]*'  ///
   Vergleichstext = "                                                                                                                     "
   '/// Compare string found with: 'All spaces in document' ///
   call Search_Und_Vergleichen
endcase

testcase tChartype_print
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[:print:]*"
   '///+ Enter RegExp in Field Search for : '[:print:]*'  ///
   Vergleichstext = "HuhuhuHuhuhuTest-Text für "+chr$(34)+"Reguläre Ausdrücke"+chr$(34)+"Ein beliebiges Zeichen in einem WortFinde Möller und Müller, nicht Mller und Mueller !Wort am Anfang des AbsatzesAnfang Quaeramus tum aliarum rerum, quae vitam instruunt, diligens sine admiratione cuiusquam, usura fortunae muneribus, non servitura.Wort am Ende des AbsatzesQuaeramus tum aliarum rerum, quae vitam instruunt, diligens sine admiratione cuiusquam, usura fortunae muneribus, non servitura EndeKein oder mehrere Zeichen zwischen den WörternPeter ist im Haus. - Aha !Peter liebt dieses Haus. - Warum?Es ist Peters Haus. Ach so.Das Peterhaus. - Natürlich!Suchtext am Anfang des Wortesaaahier kommt das wortaaaSuchtext am Ende des Worteseeehier kommt das worteeeSonderzeichen$TabulatorenHiersindTabsgesetzt !AlternativgruppenErst kam ein Zweites WortDann kam ein Erstes WortUnd wieder ein Zweites WortLeerer AbsatzZeilenvorschubSonderzeichen@`Àà!Aa¡Áá"+chr$(34)+"Bb¢Ââ#Cc£Ãã$Dd¤Ää%Ee¥Åå&FfÆæ'Gg§Çç(Hh¨Èè)Ii©ÉéJjªÊê+KkËë,Ll¬ÌìMmÍí.®Îî/Oo¯ÏïPp°ÐðQq±ÑñRr²ÒòSs³ÓóTt´ÔôUuÕõVv¶ÖöWw·×÷Xx¸ØøYy¹Ùù:ZzºÚú;[{Ûû<\|¼Üü=]}½Ýý>^~¾Þþ?_¿ßÿ-RahmenHalloo kikiriki kirikeircidkvvdn hhalloDer Rahmen ist hier hinten Rahmenend" 'Test-Text für "+chr$(34)+"Reguläre Ausdrücke"+chr$(34)
   '/// Compare string found with: '' ///
   call Search_Und_Vergleichen
endcase

testcase tChartype_alnum
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[:alnum:]*"
   '///+ Enter RegExp in Field Search for : '[:alnum:]*'  ///
   Vergleichstext = "HuhuhuHuhuhuTestTextfürReguläreAusdrückeEinbeliebigesZeichenineinemWortFindeMöllerundMüllernichtMllerundMuellerWortamAnfangdesAbsatzesAnfangQuaeramustumaliarumrerumquaevitaminstruuntdiligenssineadmirationecuiusquamusurafortunaemuneribusnonservituraWortamEndedesAbsatzesQuaeramustumaliarumrerumquaevitaminstruuntdiligenssineadmirationecuiusquamusurafortunaemuneribusnonservituraEndeKeinodermehrereZeichenzwischendenWörternPeteristimHausAhaPeterliebtdiesesHausWarumEsistPetersHausAchsoDasPeterhausNatürlichSuchtextamAnfangdesWortesaaahierkommtdaswortaaaSuchtextamEndedesWorteseeehierkommtdasworteeeSonderzeichenTabulatorenHiersindTabsgesetztAlternativgruppenErstkameinZweitesWortDannkameinErstesWortUndwiedereinZweitesWortLeererAbsatzZeilenvorschubSonderzeichenÀàAaÁáBbÂâCcÃãDdÄäEeÅåFfÆæGgÇçHhÈèIiÉéJjªÊêKkËëLlÌìMmÍíÎîOoÏïPpÐðQqÑñRr²ÒòSs³ÓóTtÔôUuÕõVvÖöWwXxØøYy¹ÙùZzºÚúÛû¼Üü½Ýý¾ÞþßÿRahmenHallookikirikikirikeircidkvvdnhhalloDerRahmenisthierhintenRahmenend" 'Test
   '/// Compare string found with: '' ///
   call Search_Und_Vergleichen
endcase

testcase tChartype_alpha
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[:alpha:]*"
   '///+ Enter RegExp in Field Search for : '[:alpha:]*'  ///
   Vergleichstext = "HuhuhuHuhuhuTestTextfürReguläreAusdrückeEinbeliebigesZeichenineinemWortFindeMöllerundMüllernichtMllerundMuellerWortamAnfangdesAbsatzesAnfangQuaeramustumaliarumrerumquaevitaminstruuntdiligenssineadmirationecuiusquamusurafortunaemuneribusnonservituraWortamEndedesAbsatzesQuaeramustumaliarumrerumquaevitaminstruuntdiligenssineadmirationecuiusquamusurafortunaemuneribusnonservituraEndeKeinodermehrereZeichenzwischendenWörternPeteristimHausAhaPeterliebtdiesesHausWarumEsistPetersHausAchsoDasPeterhausNatürlichSuchtextamAnfangdesWortesaaahierkommtdaswortaaaSuchtextamEndedesWorteseeehierkommtdasworteeeSonderzeichenTabulatorenHiersindTabsgesetztAlternativgruppenErstkameinZweitesWortDannkameinErstesWortUndwiedereinZweitesWortLeererAbsatzZeilenvorschubSonderzeichenÀàAaÁáBbÂâCcÃãDdÄäEeÅåFfÆæGgÇçHhÈèIiÉéJjªÊêKkËëLlÌìMmÍíÎîOoÏïPpÐðQqÑñRrÒòSsÓóTtÔôUuÕõVvÖöWwXxØøYyÙùZzºÚúÛûÜüÝýÞþßÿRahmenHallookikirikikirikeircidkvvdnhhalloDerRahmenisthierhintenRahmenend" 'Test"
   '/// Compare string found with: '' ///
   call Search_Und_Vergleichen
endcase

testcase tChartype_cntrl
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[:cntrl:]*"
   '///+ Enter RegExp in Field Search for : '[:cntrl:]*'  ///
   Vergleichstext = Chr(9) + Chr(9) + Chr(9)
   '/// Compare string found with: '' ///
   call Search_Und_Vergleichen
endcase

testcase tChartype_lower
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[:lower:]*"
   '///+ Enter RegExp in Field Search for : '[:lower:]*'  ///
   Vergleichstext = "uhuhuuhuhuestextfüreguläreusdrückeinbeliebigeseichenineinemortindeöllerundüllernichtllerunduellerortamnfangdesbsatzesnfanguaeramustumaliarumrerumquaevitaminstruuntdiligenssineadmirationecuiusquamusurafortunaemuneribusnonservituraortamndedesbsatzesuaeramustumaliarumrerumquaevitaminstruuntdiligenssineadmirationecuiusquamusurafortunaemuneribusnonserviturandeeinodermehrereeichenzwischendenörterneteristimaushaeterliebtdiesesausarumsistetersauschsoaseterhausatürlichuchtextamnfangdesortesaaahierkommtdaswortaaauchtextamndedesorteseeehierkommtdasworteeeonderzeichenabulatoreniersindabsgesetztlternativgruppenrstkameinweitesortannkameinrstesortndwiedereinweitesorteererbsatzeilenvorschubonderzeichenàaábâcãdäeåfægçhèiéjªêkëlìmíîoïpðqñròsótôuõvöwxøyùzºúûüýþßÿahmenallookikirikikirikeircidkvvdnhhalloerahmenisthierhintenahmenend" 'est"
   '/// Compare string found with: '' ///
   MatchCase.Check
   call Search_Und_Vergleichen
endcase

testcase tChartype_upper
   call Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///

   '/// Start Search & Compare ///
   Such_Text      = "[:upper:]*"
   '///+ Enter RegExp in Field Search for : '[:upper:]*'  ///
   Vergleichstext = "HHTTRAEZWFMMMMWAAAQWEAQEKZWPHAPHWEPHADPNSAWSEWSTHTAEZWDEWUZWLAZSÀAÁBÂCÃDÄEÅFÆGÇHÈIÉJÊKËLÌMÍÎOÏPÐQÑRÒSÓTÔUÕVÖWXØYÙZÚÛÜÝÞRHDRR"
   '/// Compare string found with: '' ///
   MatchCase.Check
   call Search_Und_Vergleichen
endcase

'---------------------------------------------------------
'---------------------------------------------------------
'---------------   Unterprogramme   ----------------------
'---------------------------------------------------------
sub Ausgangszustand_herstellen
   '/// <B>Set to default values</B> ///
   '///+ Open file "..\writer\optional\input\search\regu_aus.sxw" ///
   '///+ Close 'Find&Replace' dialog if open ///
   '///+ Point cursor to beginning of document ///
   '///+ Open 'Find&Replace' dialog ///
   '///+ Clear Editfield 'Search for' ///
   '///+ Uncheck 'Whole words only' ///
   '///+ Uncheck 'Backwards' ///
   '///+ Uncheck 'Match Case' ///
   '///+ Uncheck 'Current selection only' ///
   '///+ Uncheck 'Search for Styles' ///
   '///+ Uncheck 'Similarity Search' ///
   '///+ Check 'Regular Expressions' ///
   hFileOpenLocally(  gtesttoolpath + "writer\optional\input\search\regu_aus.sxw")
   
   kontext
   if FindAndReplace.Exists then FindAndReplace.Close
   kontext "DocumentWriter"
   DocumentWriter.Typekeys "<PAGEUP>", 2          'an den Anfang des Textes gehen
   DocumentWriter.Typekeys "<MOD1 HOME>"
   EditSearchAndReplace
   kontext "FindAndReplace"            'Search&Replace-Fenster ├ö├Â┬úÔö£├é├ö├Â┬úÔö£├®├ö├Â┬╝Ôö£ÔòæÔö£├ÂÔö£├éÔö¼├║├ö├Â┬úÔö¼┬½ffnen
   try
    SearchFor.Settext ""
   catch
   endcatch
   if SimilaritySearch.IsVisible = False then More.Click
   if SimilaritySearch.IsEnabled then SimilaritySearch.Uncheck
   if SearchForStyles.IsEnabled then SearchForStyles.Uncheck
   if RegularExpressions.IsEnabled then RegularExpressions.Uncheck
   if NoFormat.IsEnabled then NoFormat.Click
   if CurrentSelectionOnly.IsEnabled then CurrentSelectionOnly.Uncheck
   WholeWordsOnly.Uncheck                'Im Suchdialog alles unchecken,
   Backwards.Uncheck                    'damit ein definierter Anfangs-
   MatchCase.Uncheck                    'zustand erreicht wird.
   RegularExpressions.Check                  'Regul├ö├Â┬úÔö£├é├ö├Â┬úÔö£├®├ö├Â┬╝Ôö£ÔòæÔö£├ÂÔö£├éÔö¼├║Ôö£├ÂÔö£ÔòùÔö£├Ñrer Ausdruck mu├ö├Â┬úÔö£├é├ö├Â┬úÔö£├®├ö├Â┬╝Ôö£Ôòæ├ö├Â┬úÔö£Ôòæ├ö├Â┬úÔö£├æ gecheckt sein
end sub

'-------------------------------------------------------
sub Search_Und_Vergleichen
  dim dummy as string            'Sucht den Ausdruck und vergleicht
                                    'Such_Text mit dem Text, der nach
   SearchFor.SetText Such_Text     'nach der Suche markiert ist und
   if Such_Text = "$" then
      SearchNow.Click
   else
      SearchAll.Click
   end if
   Kontext "Active"
   if Active.Exists then
    if Active.GetRT = 304 then ' MESSBOX
     Warnlog "active: " + Active.Gettext
     Active.Ok
    end if
   end if
   kontext "FindAndReplace"
   if FindAndReplace.Exists then
     FindAndReplace.Close
    else
     Warnlog "Dialog 'Find & Replace' not open anymore!"
   end if
   Kontext "DocumentWriter"
    try
     EditCopy
    catch
     Warnlog "Edit /Copy disabled!"
     DocumentWriter.Typekeys "<MOD1 HOME>"
     hCloseDocument
     exit sub
    endcatch

    Sleep 1
    if Vergleichstext = chr(13)+chr(10) OR Vergleichstext = chr(10) OR Vergleichstext = chr(13) then
       dummy = GetClipboard
    else
       dummy = GetClipboardText
    end if
    if Vergleichstext <> dummy then
       Warnlog "'",dummy,"' has been found, and not '",Vergleichstext,"'"
    else
       printlog "'Find' found correct word!"
    end if
    DocumentWriter.Typekeys "<MOD1 HOME>"

   hCloseDocument
end sub
