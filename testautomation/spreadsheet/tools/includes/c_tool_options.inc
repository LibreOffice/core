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
'* owner : oliver.craemer@oracle.com
'*
'* short description : Tools for Tools::Options dialog
'*
'************************************************************************
'*
' #1 subSetConversionMicrosoftOLE
'*
'\***********************************************************************

sub subSetConversionMicrosoftOLE ( OPTIONAL sConversionType as STRING )
'///<u><b>Routine to adjust the behaviour of Tools::Options::Load/Save::Microsoft Office</b></u>///
'///<b>Possible Keys for 'sConversionType' are:</b>///
'///<ul>
'///<li>RESET (or omitted): Uncheck all boxes (default in a freshly installed OOo))</li>///
'///<li>ALL: Check all boxes</li>/// 
'///<li>LOAD: Check all boxes for loading MS OLE objects (left side), uncheck all boxes for saving</li>///
'///<li>SAVE: Check all boxes for saving MS OLE objects (right side), uncheck all boxes for loading</li>///
'///<li>For other values the subroutine will exit</li>///
'///</ul>
'///<b>Action:</b>///
'///<ul>
    '///<li>Convert omitted 'sConversionType' to RESET</li>///
    if IsMissing ( sConversionType ) then
       sConversionType = "RESET"
    end if
    dim iPosInAuswahl as INTEGER
    dim iExtraToggle as INTEGER
    
    '///<li>Determine value of 'iExtraToggle' for known keys</li>///
    select case sConversionType
           case "RESET" : iExtraToggle = 1
           case "ALL"   : iExtraToggle = 2
           case "LOAD"  : iExtraToggle = 3
           case "SAVE"  : iExtraToggle = 4
           case else 
                warnlog "Unknown keyword for paramter -> Exit the subroutine"
                qaErrorLog "Valid keywords are RESET, ALL, LOAD and SAVE!"
                exit sub
    end select
    
    '///<li>Invoke Tools::Options</li>///       
    ToolsOptions
    '///<li>Go to tabpage Load/Save::Microsoft Office</li>///
    call hToolsOptions ( "LOADSAVE" , "MICROSOFTOFFICE" )
    '///<li>Go to top entry in listbox</li>///
    Auswahl.TypeKeys( "<PageUp>" )
    
    '///<li>Toggle (in a tricky way) to a defined state of checkboxes</li>///
    for iPosInAuswahl = 1 to 4
         while NOT Auswahl.IsChecked
             Auswahl.TypeKeys ( "<Space>" )
         wend
         while Auswahl.IsChecked
             Auswahl.TypeKeys ( "<Space>" )
         wend
         '///<li>Toggle to desired state of boxes</li>///
         Auswahl.TypeKeys ( "<Space>" , iExtraToggle )
         '///<li>Go one entry down in list box</li>///
         Auswahl.TypeKeys ( "<Down>" )
         '///<li>Loop for all lines</li>///
    next iPosInAuswahl
    
    '///<li>Quit dialog</li>///
    Kontext "OptionenDLG"
    OptionenDLG.Ok
    '///</ul>
end sub
