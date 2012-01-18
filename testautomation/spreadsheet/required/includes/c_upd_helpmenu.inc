'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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


