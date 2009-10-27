'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_filternames.inc,v $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Verify that all filters are listed in the filter listbox
'*
'\******************************************************************************

testcase tVerifyFilterNames

    
    if ( gIsoLang <> "en-US" ) then
        qaerrorlog( "No testing for non-US languages" )
        goto endsub
    endif
    
    const MAX_FILTERCOUNT = 300
    dim asFilterNames( MAX_FILTERCOUNT ) as string
    dim iFilterCount as integer
    dim iCurrentItem as integer
    
    dim cFile as string
    dim iErr as integer


    ' For Windows and Solaris Sparc there exists additional commercial filters
    ' that are available for StarOffice only.
    cFile = gProductName & "_Filternames_"
    if ( not gOOo ) then
        select case gtSysName
            case "Windows"       : cFile = cFile & "add_" & gIsoLang & ".txt"
            case "Solaris SPARC" : cFile = cFile & "add_" & gIsoLang & ".txt"
            case "WinXP"         : cFile = cFile & "add_" & gIsoLang & ".txt"
            case "Mac OS X"      : cFile = cFile & "add_" & gIsoLang & ".txt"
            case "Linux"         : cFile = cFile & gIsoLang & ".txt"
            case "Solaris x86"   : cFile = cFile & gIsoLang & ".txt"
        end select
    else
        cFile = cFile & gIsoLang & ".txt"
    endif
    
    dim cFileIn as string
        cFileIn  = gTesttoolPath & "framework\optional\input\filternames\" & cFile
        cFileIn = convertpath( cFileIn )
        
    dim cFileOut as string
        cFileOut = hGetWorkPath() & cFile
    
    
    hCreateDocument()
    
    FileOpen
    Kontext "OeffnenDlg"

    if ( OeffnenDlg.exists( 1 ) ) then    
    
        iFilterCount = DateiTyp.getItemCount()
        if ( iFilterCount > MAX_FILTERCOUNT ) then
            warnlog( "Too many filters in filterlist for this test to handle" )
            goto endsub
        endif
        
        printlog( "Reading in " & iFilterCount & " filternames" )

        asFilterNames( 0 ) = iFilterCount
        
        for iCurrentItem = 1 to iFilterCount         
        
            kontext "OeffnenDlg"
            try
                asFilterNames( iCurrentItem ) = DateiTyp.getItemText( iCurrentItem )
            catch
                printlog( "Test broke at pos: " & iCurrentItem )
            endcatch
            
        next iCurrentItem
        
        OeffnenDlg.cancel()
        
        iErr = hManageComparisionList( cFileIn, cFileOut, asFilterNames() )    

    else
        warnlog( "File Open dialog not present." )
    endif
    
    hDestroyDocument()
    
    if ( iErr <> 0 ) then
        warnlog( "Filterlist differs from reference, please review" )
    endif
    
     
endcase

