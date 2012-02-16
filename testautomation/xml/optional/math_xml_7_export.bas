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

