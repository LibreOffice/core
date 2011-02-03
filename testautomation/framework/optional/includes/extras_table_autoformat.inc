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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Test the autoformat-feature for tables
'*
'\******************************************************************************

testcase tAutoformatTable( iApp as integer )

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for non-en_US languages" )
        goto endsub
    endif

    
    dim al_UI_formats( 1000 ) as string : al_UI_formats( 0 ) = "0"
    
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
    
    hCreateDocument()
    
    select case iApp
    case 1 : kontext "DocumentWriter"
             hTabelleEinfuegen()
    case 2 : kontext "DocumentCalc"
             DocumentCalc.TypeKeys( "<Shift Right>", 5 )
             DocumentCalc.TypeKeys( "<Shift Down>", 3 )
    end select
    
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
    
    iFormatCount = oControl.getItemCount()
    
    for iCurrentFormat = 1 to iFormatCount
    
        oControl.select( iCurrentFormat )
        
        cCurrentFormat = oControl.getSelText()
        ListAppend( al_UI_formats() , cCurrentFormat )
        
    next iCurrentFormat
        
    select case iApp
    case 1 : AutoformatTabelle.ok()
    case 2 : AutoformatCalc.ok()
    end select
    
    hDestroyDocument()
    
    irc = hManageComparisionList( sFileIn, sFileOut, al_UI_formats() )
    hListResultEvaluation( irc , 0 )
    
endcase
        
    


