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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Verify that the applications appear in tools/options
'*
'\******************************************************************************

testcase tUpdtOptionItems

    printlog( "Verify that all applications are listed in tools/options" )
        
    const DOCTYPES = 7
    dim iDocType as integer
    dim cDocType as string
    dim iAppItems as integer
    dim iNodes as integer
    
    dim iAppCount( DOCTYPES ) as integer
        iAppCount( 1 ) = 8 ' writer
        iAppCount( 2 ) = 7 ' calc
        iAppCount( 3 ) = 7 ' impress
        iAppCount( 4 ) = 7 ' draw
        iAppCount( 5 ) = 7 ' math
        iAppCount( 6 ) = 8 ' masterdoc
        iAppCount( 7 ) = 8 ' html
        
    for iDocType = 1 to DOCTYPES
    
        printlog( "" )
        printlog( hNumericDocType( iDocType ) )
        hCreateDocument()
        
        ToolsOptions
        
        kontext "ExtrasOptionenDlg"
        if ( ExtrasOptionenDlg.exists( 2 ) ) then

            iAppItems = Optionsliste.getItemCount()
            Optionsliste.typeKeys( "<HOME>" )
            for iNodes = 1 to iAppItems
                Optionsliste.typeKeys( "-" )
                Optionsliste.typeKeys( "<DOWN>" )
            next iNodes

            iAppItems = Optionsliste.getItemCount()
            if ( iAppItems <> iAppCount( iDocType ) ) then
                warnlog( "#i68068# The number of top nodes in Tools/Options is incorrect." )
                printlog( "Found...: " & iAppItems )
                printlog( "Expected: " & iAppCount( iDocType ) )
            else
                printlog( "The number of top nodes is correct." )
            endif

            hCloseDialog( ExtrasOptionenDlg, "Cancel" )
        else
            warnlog( "Dialog <Tools/Options> did not open " )
        endif

        hDestroyDocument()
        
    next iDocType

endcase

