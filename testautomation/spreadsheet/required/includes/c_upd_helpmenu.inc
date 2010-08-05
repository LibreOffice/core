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
'* short description : Resource Test - Help Menu
'*
'************************************************************************
'*
' #1 tHelpOfficeHelp 
' #1 tHelpWhatsThis
' #1 tHelpAbout
'*
'\***********************************************************************

sub c_upd_helpmenu

    Printlog Chr(13) + "--------- Help Menu (c_upd_helpmenu.inc) ---------"
    
    call tHelpOfficeHelp
    call tHelpWhatsThis
    call tHelpAbout
    

end sub

'-----------------------------------------------------------

testcase tHelpOfficeHelp
'///<u><b>Help – Star-/OpenOffice Help</b></u>

    '/// Opening new spreadsheet document for getting defined starting environment
    printlog " Opening new spreadsheet document for getting defined starting environment"
    Call hNewDocument
    '/// Open the help application by 'Help – Star-/OpenOffice Help'
    printlog " Open the help application by 'Help – Star-/OpenOffice Help'"
    HelpContents
    Kontext "StarOfficeHelp"
    '/// Close help application
    printlog " Close help application"
    StarOfficeHelp.TypeKeys "<Mod1 F4>"
    If StarOfficeHelp.Exists then
        warnlog "StarOffice Help could not be closed?!"
    end if
    '/// Close starting document
    printlog " Close starting document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tHelpWhatsThis
'///<u><b>Help – What's This?</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Invoke the 'What's this help' by 'Help – What's This?'
    printlog " Invoke the 'What's this help' by 'Help – What's This?'"
    HelpWhatsThis
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tHelpAbout
'///<u><b>Help – About</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open the 'About' window by 'Help – About Star-/OpenOffice'
    printlog " Open the 'About' window by 'Help – About Star-/OpenOffice'"
    HelpAboutStarOffice
    Kontext "UeberStarWriter"
    DialogTest ( UeberStarWriter )
    '/// Show build-ID and members by STRG-SDT
    printlog " Show build-ID and members by STRG-SDT"
    UeberStarWriter.TypeKeys "<Mod1 S><Mod1 D><Mod1 T>"
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    UeberStarWriter.OK
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------


