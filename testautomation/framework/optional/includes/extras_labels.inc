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
'*  short description : test names of labels
'*
'\******************************************************************************

testcase tLabelTypes()

    printlog( "Verify presence of labels and brands" )

    ' If you want to use this test for other languages, remove the following
    ' if() block and create the reference files by running this test once.
    ' Follow the instructions emitted by hManagaComparisionList() from the log.
    
    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for non-en_US languages" )
        goto endsub
    endif

    const NO_DELTA = 0
    const MAX_LABEL_COUNT = 1600
    const RELATIVE_PATH = "framework\optional\input\extras_formats\"
    
    dim iBrandCount as integer
    dim iCurrentBrand as integer
    dim cBrandName as string
    dim iCurrentType as integer    
    dim cBrandType as string
    
    dim al_UI_Labels( MAX_LABEL_COUNT ) as string : al_UI_Labels( 0 ) = "0"
        
    dim irc as integer
    
    dim sFile as string    : sFile    = "Labels_" & gISOLang & ".txt"
    dim sFileIn as string  : sFileIn  = convertpath( gTesttoolPath & RELATIVE_PATH & sFile )
    dim sFileOut as string : sFileOut = hGetWorkPath() & sFile
       
    printlog( "Open labels dialog, retrieve all label names" )
    hCreateLabels()

    kontext
    Active.SetPage TabEtiketten
    
    kontext "TabEtiketten"

    iBrandCount = Marke.getItemCount()

    for iCurrentBrand = 1 to iBrandCount

        Marke.select( iCurrentBrand )
        cBrandName = Marke.getSelText()

        for iCurrentType = 1 to Typ.getItemCount()

            Typ.select( iCurrentType )
            cBrandType = cBrandName & ":" & Typ.getSelText()
            ListAppend( al_UI_Labels() , cBrandType )

        next iCurrentType

    next iCurrentBrand

    printlog( "Close labels dialog" )
    TabEtiketten.Cancel()

    printlog( "Compare to reference (or create new reference, if none found)" )
    irc = hManageComparisionList( sFileIn, sFileOut, al_UI_Labels() )
    hListResultEvaluation( irc , NO_DELTA )
    
endcase
