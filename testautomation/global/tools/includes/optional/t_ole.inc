'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

*****

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

