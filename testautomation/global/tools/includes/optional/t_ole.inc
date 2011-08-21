'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'*  short description : Tools for OLE objects
'*
'\******************************************************************************

function hGetOfficeVersion() as string

    ' The "Insert OLE object" dialog lists the OLE Objects with application name
    ' and version number. THe version numbers are tracked in the officeinfo.txt
    ' file which must be adjusted each time we change to a new major version.

    const CFN = "global::tools::includes::optional::t_ole.inc::hGetOfficeVersion(): "
    const DATAFILE = "global/input/officeinfo.txt"
    const MAX_LINES_IN_DATAFILE = 20

    dim cPath as string
    dim aItemList( MAX_LINES_IN_DATAFILE ) as string
    
    ' Path to info file
    cPath = convertpath( gTesttoolPath & DATAFILE )
    if ( GVERBOSE ) then printlog( CFN & "Reading: " & cPath )
    
    ' Read the file, store the list of known office versions
    hGetDatafileSection( cPath, aItemList(), "", "", "" )
    
    ' Search for the version number matching the product name, store value in global variable
    gOfficeVersion = hGetValueForKeyAsString( aItemList(), gProductName )
    
    ' Return content of the global variable as well
    hGetOfficeVersion() = gOfficeVersion
    
end function

'*******************************************************************************

function hGetOleObjectName( cApplication as string ) as string

    ' for cApplication you may pass gApplication.

    dim Application_API_Name as string
    dim oUnoOfficeConnection as object
    dim oUnoConfigurationAccess as object
    dim aPropertyValue(1) as new com.sun.star.beans.PropertyValue
    dim xViewRoot as object
    dim cConfigString as string
    dim cString as string

    const CFN = "global::tools::includes::optional::t_ole.inc::hGetOleObjectName(): "

    ' The application names for the API are case sensitive so we cannot use gApplication
    select case ( ucase( cApplication ) )
    case "WRITER" : Application_API_Name = "Writer"
    case "CALC"   : Application_API_Name = "Calc"
    case "IMPRESS": Application_API_Name = "Impress"
    case "DRAW"   : Application_API_Name = "Draw"
    case "MATH"   : Application_API_Name = "Math"
    case "CHART"  : Application_API_Name = "Chart"
    case else
        warnlog( CFN & "Invalid object type passed to function: " & cApplication )
        hGetOleObjectName() = ""
        exit function
    end select
    
    if ( GVERBOSE ) then printlog( CFN & "Retrieving OLE name for: " & cApplication )

    ' ...Embedding is physical path, ObjectNames the top node
    aPropertyValue( 0 ).Name  = "nodepath"
    aPropertyValue( 0 ).Value = "/org.openoffice.Office.Embedding/ObjectNames/" & Application_API_Name
    
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
    dim sAllOle as string
    dim j as integer

    const CFN = "global::tools::includes::optional::t_ole.inc::GetOleDefaultNames(): "
    if ( GVERBOSE ) then printlog( CFN & "Retrieving OLE names" )

    hGetOfficeVersion()
    gOLEWriter  = hGetOleObjectName( "Writer"  )
    gOLECalc    = hGetOleObjectName( "Calc"    )
    gOLEChart   = hGetOleObjectName( "Chart"   )
    gOLEImpress = hGetOleObjectName( "Impress" )
    gOLEDraw    = hGetOleObjectName( "Draw"    )
    gOLEMath    = hGetOleObjectName( "Math"    )    

    ' There is no programatical way to retrieve the word for 'further objects'
    ' Only way to get it language independent is to retrieve all other words and take the last other entry that lasts
    ' Only available on windows platform
    if  ( gPlatgroup = "w95" )  then
        sAllOle = gOLEWriter & gOLECalc & gOLEChart & gOLEImpress & gOLEDraw & gOLEMath
        Call hNewDocument
        InsertObjectOLEObject
        Kontext "OLEObjektEinfuegen"
        NeuErstellen.Check
        WaitSlot()
        for j=1 to ObjektTyp.GetItemCount
            ObjektTyp.Select j
            if 0 = instr (sAllOle, ObjektTyp.GetSelText ) then
            gOleOthers = ObjektTyp.GetSelText
            endif
        next j
        OLEObjektEinfuegen.cancel
        hCloseDocument
    endif
end function

