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
'* short description :  tools/functions for the Section-testing
'*
'************************************************************************
'*
' #1 fInsertSection
' #1 fInsertBookmark
' #1 fInsertFootnote
' #1 fInsertEndnote
' #1 fInsertTable
' #1 fCloseNavigator
' #1 fCheckDirectCursor
' #1 fMultiSelection
'*
'\***********************************************************************

'******************************************************
'* insert a section with name SectionName            **
'******************************************************
function fInsertSection(SectionName as string)
   InsertSection
   wait 500
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"
   Bereichsliste.Settext SectionName
   TabBereiche.OK
   wait 500
end function

'******************************************************
'* insert a bookmark with name                       **
'******************************************************
function fInsertBookmark(BookmarkName as string)

    InsertBookmark
    Kontext
    Kontext "TextmarkeEinfuegen"
    Textmarken.SetText BookmarkName
    TextmarkeEinfuegen.OK

end function


'******************************************************
'*    insert a footnote                              **
'******************************************************
function fInsertFootnote(Numbering as string , ConTentInCharater as String)

   InsertFootnote
   Kontext "FussnoteEinfuegen"

   ' check footnote
   Fussnote.Check

   Select Case Numbering
    Case "automatic" : Automatisch.Check
    Case "character" : Zeichen.Check
                       ZeichenText.SetText ConTentInCharater
    Case else : Warnlog "Maybe new options! :-)"
                Automatisch.Check
   end select

   FussnoteEinfuegen.OK

end function


'******************************************************
'* insert a endnote                                  **
'******************************************************
function fInsertEndnote(Numbering as string , ConTentInCharater as String)

   InsertFootnote
   Kontext "FussnoteEinfuegen"

   ' check endnote
   Endnote.Check

   Select Case Numbering
    Case "automatic" : Automatisch.Check
    Case "character" : Zeichen.Check
                       ZeichenText.SetText ConTentInCharater
    Case else : Warnlog "Maybe new options! :-)"
                Automatisch.Check
   end select

   FussnoteEinfuegen.OK

end function


'******************************************************
'* insert a table with name TableName                **
'* Column is table's column's number                 **
'* Row    is table's row   's number                 **
'******************************************************
function fInsertTable(TableName as string, ColumnNumber as string, RowNumber as string)

   InsertTableWriter
   Sleep 1
   Kontext "TabelleEinfuegenWriter"
   TabellenName.SetText TableName

   Spalten.SetText ColumnNumber
   Zeilen.SetText  RowNumber

   TabelleEinfuegenWriter.OK

end function

'*******************************************************
'* Close Navigator                                    **
'*******************************************************
function fCloseNavigator

   Kontext
   Kontext "Navigator"
   try
    Navigator.Close
   catch
    Warnlog "Could not close Navigator!"
   endcatch

end function

'*******************************************************
'*  Open options and check 'Direct-Cursor' in options **
'*******************************************************
function fCheckDirectCursor

    ToolsOptions
    Call hToolsOptions("WRITER","FormattingAids")
    Zonen_Cursor.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

end function


'*********************************************************
'*  This function is evaluated multisection             **
'*  selectString is the selected string in the text     **
'*  There must be more than 2 selectStrings in the text **
'*********************************************************
function fMultiSelection(selectedString as String)

   Kontext
   EditSearchAndReplace
   Kontext "FindAndReplace"

   SearchFor.SetText selectedString
   SearchAll.Click
   Sleep 1

   FindAndReplace.Close

end function
