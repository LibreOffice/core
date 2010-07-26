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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Export test for the Math XML 6.0/7/OOo 1.x format
'*
'\***********************************************************************************
 Global Isliste(250) as string
 Global OutputPath as string 
sub main       
    printlog "---------------------------------------------------------------------"
    printlog "-----        X M L  -  M A T H  -  U P D A T E T E S T          -----"
    printlog "---------------------------------------------------------------------"

    use "xml\optional\includes\sxm7_01.inc"
    use "xml\tools\includes\xmltool1.inc"
        
    'Disabling PrettyPriting if it has been set.       
    call hEnablePrettyPrinting(1)  
    call CreateWorkXMLExportDir ( "user\work\xml\math\level1\" )
       
    call hStatusIn ("XML","math_xml_7_export.bas")
        printlog "------------------- sxm_01.inc ---------------------"
        call tunary_binary
        call trelations
        call tset_operations
        call tfunctions
        call toperators
        call tattributes
        call tothers
        call tbrackets
        call tformats
        call tall_commands
        call tall_commands_star_math_3
        call tall_selections
        call tannuities
        call tcauchy
        call tchemicalelement
        call tdefinitionstraightline
        call teffectiveannualinterestrate
        call thamiltonoperator
        call tinvestmentreturns
        call tlawoferrorpropagation
        call tlimes      
        call tlinearregression
        call tmaxwell
        call tscalarproduct
        call tsinus
        call tsquareroot
        call tstandarddeviation
        call tvectorproduct
    call hStatusOut
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    
    use "global\tools\includes\optional\t_xml2.inc"    
    gApplication   = "MATH"
    Call GetUseFiles   
end sub

