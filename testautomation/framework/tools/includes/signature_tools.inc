'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Tools to ease working with digital signatures
'*
'\******************************************************************************

function hSelectXMLSecTab( cTab as string ) as boolean


    '///<h3>Switch between tabpages on the Ceritficates/XML-Security Tabpage</h3>

    '///<u>Input value(s):</u><br>
    '///<ol>
    '///+<li>Identifier for the requested tabpage (string). Valid options are:</li>
    '///<ul>
    '///+<li>&quot;GENERAL&quot; for the general (first page)</li>
    '///+<li>&quot;DETAILS&quot; for the details (second page)</li>
    '///+<li>&quot;PATH&quot; for the certification path (third page)</li>
    '///</ul>
    '///</ol>


    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>


    '///<u>Description:</u>
    '///<ul>

    dim brc as boolean
        brc = false
    const CFN = "hSelectXMLSecTab::"

    '///+<li>Switch page</li>
    kontext 

    select case ( ucase( cTab ) )
    case "GENERAL"     : active.setpage TabXMLSecGeneral
                         if ( TabXMLSecGeneral.exists() ) then
                             brc = true
                             printlog( CFN & "Switched to General page" )
                         endif

    case "DETAILS"     : active.setpage TabXMLSecDetails
                         if ( TabXMLSecDetails.exists() ) then
                             brc = true
                             printlog( CFN & "Switched to Details page" )
                         endif

    case "PATH"        : active.setpage TabXMLSecCertPath
                         if ( TabXMLSecCertPath.exists() ) then
                             brc = true
                             printlog( CFN & "Switched to Certification Path page" )
                         endif
    end select

    '///+<li>Set returnvalue</li>
    hSelectXMLSecTab() = brc
    '///</ul>

end function
