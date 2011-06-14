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
'*                                                                **
'* owner : helge.delfs@oracle.com                                    **
'*                                                                **
'* short description : Misc tools for writer tests                **
'*                                                                **
'*******************************************************************
'*                                                                **
' #1 CountItemsInNavigatorGroup(Group as integer) as Integer
' #1 wCheckOutline
' #1 wSetZoom
' #1 wGetSpellcheckDocumentName
' #1 wToolsSpellcheckCheckWriter
' #1 wEnableAutoSpellcheck
' #1 wEnableHyphenation
'*                                                                **
'\******************************************************************

function CountItemsInNavigatorGroup(Group as integer) as Integer
    Dim j as integer,SelectInNavigator as integer
    'First, select the Navigator depending on the application
    Kontext "Navigator"
    If Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"

    'In Navigator, close all groups and set focus to first group
    Sleep 2
    if Auswahlliste.GetItemCount<12 then Inhaltsansicht.Click
    Wait 500
    'Close all groups in Navigator
    if Auswahlliste.GetItemCount >12 then
        For j= 1 to 12
            Auswahlliste.Select j
            Auswahlliste.TypeKeys "-"
        next j
    end if

    Wait 500
    'Select group given by the function
    Auswahlliste.Select Group
    ' Open group
    Auswahlliste.TypeKeys "+"
    Inhaltsansicht.Click
    CountItemsInNavigatorGroup = Auswahlliste.GetItemCount - 1
    Inhaltsansicht.Click

end function

'*******************************************************************

function wCheckOutline(WhatToFind as string) as boolean
    '/// This function checks the Outline in level 1 test.
    '/// Purpose is to have a little tolerance, because when resolution changes, the
    '/// line breaks are different
    wCheckOutline = False
    Kontext "DocumentImpressOutlineView"
    DocumentImpressOutlineView.TypeKeys "<Home><Down>" ,1
    DocumentImpressOutlineView.TypeKeys "<Home><Shift End>"
    EditCopy
    if GetClipboardText <> WhatToFind then
        DocumentImpressOutlineView.TypeKeys "<Home><Up>" ,2
        DocumentImpressOutlineView.TypeKeys "<Home><Shift End>"
        EditCopy
        if GetClipboardText = WhatToFind then
            wCheckOutline = True
        end if
    else
        wCheckOutline = True
    end if
end function

'*******************************************************************

function wSetZoom (  )
    '/// This function sets the zoom size to 100% in a text document

    ViewZoom
    Kontext "Massstab"
    Vergroesserung100.Check
    Massstab.OK
    Sleep 1


end function

'*******************************************************************

function wGetSpellcheckDocumentName(xCategory as integer) as string
    Dim sTemp as string, sReturn as string
    '/// This function is to return the document to load in spellcheck test
    select case xCategory
    case 1:
        select case iSprache
        case 81, 82, 86, 88
            sReturn = "None"
        case 1
            sTemp = Convertpath(gTesttoolpath + "writer\optional\input\spellcheck\st1_2_01.odt")
        case 3
            sTemp = Convertpath(gTesttoolpath + "writer\optional\input\spellcheck\st1_2_03.odt")
        case 7
            sTemp = Convertpath(gTesttoolpath + "writer\optional\input\spellcheck\st1_2_07.odt")
        case else
            sTemp = Convertpath(gTesttoolpath + "writer\optional\input\spellcheck\st1_2_" & iSprache & ".odt")
        end select

    case 2:
        select case iSprache
        case 81, 82, 86, 88
            sReturn = "None"
        case 1
            sTemp = Convertpath(gTesttoolpath + "writer\optional\input\linguistik\001.odt")
        case 3
            sTemp = Convertpath(gTesttoolpath + "writer\optional\input\linguistik\003.odt")
        case 7
            sTemp = Convertpath(gTesttoolpath + "writer\optional\input\linguistik\007.odt")
        case else
            sTemp = Convertpath(gTesttoolpath + "writer\optional\input\linguistik\0" & iSprache & ".odt")
        end select
    end select

    if sReturn <> "None" then
        if FileExists(sTemp) then
            sReturn = sTemp
        else
            sReturn = "Empty"
        end if
    end if

    wGetSpellcheckDocumentName = sReturn

end function

'*******************************************************************

sub wToolsSpellcheckCheckWriter
    '/// This function opens the spellcheck-dialog in all applications
    Select Case Ucase(gApplication)
    Case "WRITER"		: ToolsSpellcheckWriter
    Case "MASTERDOCUMENT"	: ToolsSpellcheckWriter
    Case "HTML" : ToolsSpellcheckWriter
    case "DRAW"         : ToolsSpellcheck
    case "IMPRESS"     	: ToolsSpellcheck
    end select
end sub

'*******************************************************************

sub wEnableAutoSpellcheck(sEnable as boolean)
    '/// This function checks option 'AutoCheck' in Options
    '/// requires a bool to indicate state
    '/// from 680m61 on it is checked by default
    ExtrasOptionen
    Kontext "ExtrasOptionenDLG"
    Call hToolsOptions("LanguageSettings","WritingAids")
    Optionen.Select 1
    Select Case sEnable
    Case true
        Optionen.Check
    Case false
        Optionen.UnCheck
    end select
    Kontext "ExtrasOptionenDLG"
    ExtrasOptionenDLG.Ok

end sub

'*******************************************************************

sub wEnableHyphenation(sEnable as boolean)
    '/// This function enables/disables hyphenation
    '/// through Format / Paragraph / Textflow
    Call hNewDocument
    FormatParagraph
    Kontext
    Active.SetPage TabTextfluss
    kontext "TabTextfluss"
    Select Case sEnable
    case true
        Automatisch.Check
    case false
        Automatisch.UnCheck
    end select
    TabTextfluss.OK
    Call hCloseDocument

end sub
