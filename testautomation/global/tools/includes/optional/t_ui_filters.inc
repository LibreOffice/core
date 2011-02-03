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
'*  short description : Get the UI names for default filters
'*
'\******************************************************************************

global gWriterFilter    as String
global gCalcFilter      as String
global gImpressFilter   as String
global gMasterDocFilter as String
global gMathFilter      as String
global gDrawFilter      as String
global gHTMLFilter      as String

'*******************************************************************************

sub GetDefaultFilterNames()

    const CFN = "global::tools::includes::optional::t_ui_filters.inc::GetDefaultFilterNames():"

    dim sMatchingFile as string
    dim sFilterArray( 100 ) as string
    
    const APPLICATION_COUNT = 7  
    dim cUIFilters( APPLICATION_COUNT ) as string
    dim cAPIFilters( APPLICATION_COUNT ) as string
    
    sMatchingFile = gTesttoolPath & "global\input\filters\"
    sMatchingFile = sMatchingFile & "build_to_filter.txt"
    sMatchingFile = convertpath( sMatchingFile )

    hGetDataFileSection( sMatchingFile, sFilterArray(), "Current", "", "" )

    cAPIFilters( 1 ) = hGetValueForKeyAsString( sFilterArray(), "WRITER"    )
    cAPIFilters( 2 ) = hGetValueForKeyAsString( sFilterArray(), "CALC"      )
    cAPIFilters( 3 ) = hGetValueForKeyAsString( sFilterArray(), "IMPRESS"   )
    cAPIFilters( 4 ) = hGetValueForKeyAsString( sFilterArray(), "MASTERDOCUMENT" )
    cAPIFilters( 5 ) = hGetValueForKeyAsString( sFilterArray(), "MATH"      )
    cAPIFilters( 6 ) = hGetValueForKeyAsString( sFilterArray(), "DRAW"      )
    cAPIFilters( 7 ) = hGetValueForKeyAsString( sFilterArray(), "HTML"      )
    
    hGetFilterGroup( cAPIFilters(), cUIFilters() )

    gWriterFilter     = cUIFilters( 1 ) 
    gCalcFilter       = cUIFilters( 2 )
    gImpressFilter    = cUIFilters( 3 )
    gMasterDocFilter  = cUIFilters( 4 )
    gMathFilter       = cUIFilters( 5 )
    gDrawFilter       = cUIFilters( 6 )
    gHTMLFilter       = cUIFilters( 7 )
    
end sub

'*******************************************************************************

function hGetFilterGroup( api_filters() as string, ui_filters() as string ) 

    ' This is a function designed to deliver a massive speed improvement
    ' compared to multiple calls to hGetUIFiltername() which establish a fresh 
    ' UNO connection on each call. This function establishes only one connection
    ' and works with a list of API filter names which are matched to UI filter
    ' names. This function does not wrap the UNO calls in a try...catch block
    ' which means that if the function fails, it fails hard. Extra hard, that is.
    ' There is no errorhandling. This function is intended for internal use only.
    ' No returnvalue is defined at this time.

    const CFN = "global::tools::includes::optional::t_ui_filters.inc::hGetFilterGroup():"

    dim oUno as object
    dim oService as object
    dim oFilter as object
    
    dim iCurrentFilter as integer
    
    dim iFilterCount as integer : iFilterCount = ubound( api_filters() )
        
    dim iAPIfilterList as integer

    oUno = hGetUNOService( true )
    oService = oUno.createInstance("com.sun.star.document.FilterFactory")
    
    for iCurrentFilter = 1 to iFilterCount
    
        oFilter = oService.getByName( api_filters( iCurrentFilter ) )

        for iAPIFilterList = 0 to ubound( oFilter )
        
            if ( oFilter( iAPIFilterList ).Name = "UIName" ) then
                ui_filters( iCurrentFilter ) = oFilter( iAPIFilterList ).Value()
                if ( GVERBOSE ) then
                    'printlog( CFN & "DEBUG: Index (iCurrentFilter): " & iCurrentFilter )
                    'printlog( CFN & "DEBUG: API Filter: " & api_filters( iCurrentFilter ) )
                    'printlog( CFN & "DEBUG: UI Filter.: " &  ui_filters( iCurrentFilter ) )
                endif
            endif
            
        next iAPIFilterList
        
    next iCurrentFilter

end function

'*******************************************************************************

