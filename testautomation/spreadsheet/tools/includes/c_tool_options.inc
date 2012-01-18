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
