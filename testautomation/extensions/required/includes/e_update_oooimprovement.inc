'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: e_update.inc,v $
'*
'* $Revision: 1.1.2.3 $
'*
'* last change: $Author: jsk $ $Date: 2008/10/08 09:05:51 $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Extension Update Test
'*
'\******************************************************************************

testcase tUpdtOOoImprovement

    printlog( "Update test for the tools/options dialog of the OOo Improvement extension" )
    if ( not gExtensionOOoImprovementIsInstalled ) then
	qaerrorlog( "The extension is not installed." )  
        goto endsub
    endif

    dim cLogFile as string
        cLogFile = gOfficePath & "user\temp\feedback\Current.csv"
        cLogFile = convertpath( cLogFile )

    if ( hDeleteFile( cLogFile ) ) then
        printlog( "Test environment is clean, no logfile present (anymore)" )
    endif

    ToolsOptions
    hToolsOptions( "StarOffice", "Improvement" )
    printlog( "Improvement Program tabpage" )

    call DialogTest( TabOOoImprovement )

    ParticipateNo.check()
    ParticipateYes.check()
  
    if ( ShowData.isEnabled() ) then 

        ShowData.click()

        Kontext "TextImport"
        if ( TextImport.exists( 1 ) ) then
            printlog( "Text import dialog" )
                
            call DialogTest( TextImport )
            TextImport.cancel()
        else
            warnlog( "#i97340# Show Data button enabled but no logged data found" )
        endif

        Kontext "Active"
        if ( Active.exists( 1 ) ) then
            printlog( "Messagebox: " & Active.getText() )
            Active.ok()
        else
            warnlog( "No warning for missing file: " & cLogFile )
        endif

        Kontext "Active"
        if ( Active.exists( 1 ) ) then
            warnlog( "#97342# Second message for missing " & cLogFile )
            Active.ok()
        endif
    else
        printlog( "Show data is disabled" )
    endif

    Kontext "OptionenDlg"
    OptionenDlg.cancel()

endcase

