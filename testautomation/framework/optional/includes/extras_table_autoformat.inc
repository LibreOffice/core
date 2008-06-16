'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: extras_table_autoformat.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Test the autoformat-feature for tables
'*
'\******************************************************************************

testcase tAutoformatTable( iApp as integer )

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for non-en_US languages" )
        goto endsub
    endif

    '///<h1>Autoformats for tables in Writer and Calc</h1>
    '///<ul>
    
    dim al_UI_formats( 1000 ) as string
        al_UI_formats( 0 ) = "0"
    
    dim iFormatCount as integer
    dim iCurrentFormat as integer
    dim cCurrentFormat as string
    
    dim irc as integer
    
    dim sFileOut as string
    dim sFile as string
    dim sFileIn as string
        sFileIn = gTesttoolPath & "framework\optional\input\extras_formats\"
    
    dim oControl as object
            
    ' Build the filenames         
    select case iApp
    case 1 : sFile = "Tables_writer_" & gISOLang & ".txt"
    case 2 : sFile = "Tables_calc_" & gISOLang & ".txt"
    end select
    
    sFileIn = convertpath( sFileIn & sFile )
    sFileOut = hGetWorkPath() & sFile
    
    '///+<li>Open a new document - Writer or Calc</li>
    hCreateDocument()
    
    '///+<li>Depending on the doctype create a table to work on</li>
    select case iApp
    case 1 : kontext "DocumentWriter"
             hTabelleEinfuegen()
    case 2 : kontext "DocumentCalc"
             DocumentCalc.TypeKeys( "<Shift Right>", 5 )
             DocumentCalc.TypeKeys( "<Shift Down>", 3 )
    end select
    
    '///+<li>Open the table autoformat dialog</li>
    FormatAutoformat
    
    select case iApp
    case 1 :    kontext "AutoformatTabelle"
                if ( AutoformatTabelle.exists( 2 ) ) then
                    oControl = FormatBox
                else
                    warnlog( "Unable to access Table Autoformat dialog (Writer)" )
                    goto endsub
                endif
                
    case 2 :    kontext "AutoformatCalc"
                if ( AutoformatCalc.exists( 2 ) ) then
                    oControl = FormatListe
                else
                    warnlog( "Unable to access Table Autoformat dialog (Calc)" )
                    goto endsub
                endif
    end select
    
    '///+<li>Get the number of items from the list</li>
    iFormatCount = oControl.getItemCount()
    
    '///+<li>Compile a list of the autoformat names</li>
    '///<ul>
    for iCurrentFormat = 1 to iFormatCount
    
        '///+<li>Select the (next) format name from the list</li>
        oControl.select( iCurrentFormat )
        
        '///+<li>Retrieve the name of the current item and store it into a list 
        cCurrentFormat = oControl.getSelText()
        ListAppend( al_UI_formats() , cCurrentFormat )
        
    next iCurrentFormat
    '///</ul>
        
    '///+<li>Close the autoformat dialog</li>
    select case iApp
    case 1 : AutoformatTabelle.ok()
    case 2 : AutoformatCalc.ok()
    end select
    
    '///+<li>Close the document</li>
    hDestroyDocument()
    
    '///+<li>Compare the list against the reference file</li>
    irc = hManageComparisionList( sFileIn, sFileOut, al_UI_formats() )
    if ( irc <> 0 ) then
        warnlog( "The list of table autoformats has changed, please review." )
    else
        printlog( "The list of table autoformats is unchanged. Good." )
    endif
    '///</ul>
    
endcase
        
    


