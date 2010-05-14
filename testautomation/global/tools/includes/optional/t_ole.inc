'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_delete_modules.inc,v $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Tools for OLE objects
'*
'\******************************************************************************

private const VERBOSE = FALSE

'*******************************************************************************

function hGetOfficeVersion() as string

    dim cPath as string
    dim aItemList( 20 ) as string
    
    const CFN = "global::tools::includes::optional::t_ole.inc::hGetOfficeVersion(): "

    ' Path to info file
    cPath = convertpath( gTesttoolPath & "global/input/officeinfo.txt" )
    if ( VERBOSE ) then printlog( CFN & "Reading: " & cPath )
    
    ' Read the file
    hGetDatafileSection( cPath, aItemList(), "", "", "" )
    
    ' Set global variable
    gOfficeVersion = hGetValueForKeyAsString( aItemList(), gProductName )
    
    ' Set returnvalue
    hGetOfficeVersion() = gOfficeVersion
    
end function

'*******************************************************************************

function hGetOleObjectName( ObjectType as string ) as string

    dim oUnoOfficeConnection as object
    dim oUnoConfigurationAccess as object
    dim aPropertyValue(1) as new com.sun.star.beans.PropertyValue
    dim xViewRoot as object
    dim cConfigString as string
    dim cString as string

    const CFN = "global::tools::includes::optional::t_ole.inc::hGetOleObjectName(): "
    
    ' Test function parameters. They are <> gApplication as the API is case sensitive
    select case ObjectType
    case "Writer"
    case "Calc"
    case "Impress"
    case "Draw"
    case "Math"
    case "Chart"
    case else
        warnlog( CFN & "Invalid object type passed to function: " & ObjectType )
        warnlog( CFN & "This function is case sensitive." )
        warnlog( CFN & "Supported are: Writer, Calc, Impress, Draw, Math, Chart" )
        hGetOleObjectName() = ""
        exit function
    end select
    
    if ( VERBOSE ) then printlog( CFN & "Retrieving OLE name for: " & ObjectType )

    ' ...Embedding is physical path, ObjectNames the top node
    aPropertyValue( 0 ).Name  = "nodepath"
    aPropertyValue( 0 ).Value = "/org.openoffice.Office.Embedding/ObjectNames/" & ObjectType
    
    ' Connect to remote UNO
    oUnoOfficeConnection = hGetUnoService( TRUE )
    
    if ( isNull( oUnoOfficeConnection )) then
        warnlog( CFN & "Failed to establish UNO connection, hGetUnoService failed" )
        hGetOleObjectName() = ""
    else
        ' Get a configuration provider
        oUnoConfigurationAccess = oUnoOfficeConnection.createInstance( "com.sun.star.configuration.ConfigurationProvider" )
        
        ' Get access
        xViewRoot = oUnoConfigurationAccess.createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess", aPropertyValue() )
        cConfigString = xViewRoot.getByName( "ObjectUIName" )
        
        ' The string contains placeholders %PRODUCTNAME and %PRODUCTVERSION which have to be replaced
        if ( gOfficeVersion = "" ) then
            warnlog( CFN & "gOOoBaseVersion is empty, run hGetOfficeVersion() first" )
            cString = right( cConfigString, len( cConfigString ) - 29 )
        else
            cString = gProductName & " " & gOfficeVersion & " " & right( cConfigString, len( cConfigString ) - 29 )
        endif
        
    endif
    
    hGetOleObjectName() = cString

end function

'*******************************************************************************

function GetOleDefaultNames()

    const CFN = "global::tools::includes::optional::t_ole.inc::GetOleDefaultNames(): "
    if ( VERBOSE ) then printlog( CFN & "Retrieving OLE names" )

    hGetOfficeVersion()
    gOLEWriter  = hGetOleObjectName( "Writer"  )
    gOLECalc    = hGetOleObjectName( "Calc"    )
    gOLEChart   = hGetOleObjectName( "Chart"   )
    gOLEImpress = hGetOleObjectName( "Impress" )
    gOLEDraw    = hGetOleObjectName( "Draw"    )
    gOLEMath    = hGetOleObjectName( "Math"    )    

end function

'*******************************************************************************
