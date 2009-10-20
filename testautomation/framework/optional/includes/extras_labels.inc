'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: extras_labels.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'*  owner : joerg.skottke@sun.co
'*
'*  short description : test names of labels
'*
'\******************************************************************************

testcase tLabelTypes()

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for non-en_US languages" )
        goto endsub
    endif

    
    dim iBrandCount as integer
    dim iCurrentBrand as integer
    dim cBrandName as string
    
    dim iTypeCount as integer
    dim iCurrentType as integer
    dim cTypeName as string
    
    dim cBrandType as string
    
    dim al_UI_Labels( 1600 ) as string
        al_UI_Labels( 0 ) = "0"
        
    dim irc as integer
    
    dim sFile as string
        sFile = "Labels_" & gISOLang & ".txt"

    dim sFileIn as string
        sFileIn = gTesttoolPath & "framework\optional\input\extras_formats\" & sFile
        sFileIn = convertpath( sFileIn )

    dim sFileOut as string
        sFileOut = hGetWorkPath() & sFile
       
    
    hCreateLabels()

    kontext
    Active.SetPage TabEtiketten
    
    kontext "TabEtiketten"

    iBrandCount = Marke.getItemCount()

    for iCurrentBrand = 1 to iBrandCount
    
        Marke.select( iCurrentBrand )
        
        cBrandName = Marke.getSelText()
        
        iTypeCount = Typ.getItemCount()
        
        for iCurrentType = 1 to iTypeCount
        
            Typ.select( iCurrentType )
        
            cTypeName = Typ.getSelText()
            
            cBrandType = cBrandName & ":" & cTypeName
            ListAppend( al_UI_Labels() , cBrandType )
            
        next iCurrentType
    
    next iCurrentBrand

    TabEtiketten.Cancel()

    irc = hManageComparisionList( sFileIn, sFileOut, al_UI_Labels() )
    if ( irc <> 0 ) then
        warnlog( "The list of labels (Brand/Types) has changed, please review." )
    else
        printlog( "The lists of labels are unchanged. Good." )
    endif
    
endcase
