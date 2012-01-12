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
'* short description : Export test for the Impress XML 6.0/7/OOo 1.x format
'*
'\***********************************************************************************

sub main    
    printlog "---------------------------------------------------------------------"
    printlog "-----      X M L  -  I M P R E S S -  U P D A T E T E S T       -----"
    printlog "---------------------------------------------------------------------"

    use "xml\optional\includes\sxi7_01.inc"
    use "xml\tools\includes\xmltool1.inc"
    
    call hStatusIn ("XML" , "impress_xml_7_export.bas")
        call CreateWorkXMLExportDir ( "user\work\xml\impress\level1\" )
        call hEnablePrettyPrinting(1)
        call sxi7_01       
    call hStatusOut
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    
    use "global\tools\includes\optional\t_xml2.inc"    
    gApplication   = "IMPRESS"
    Call GetUseFiles   
end sub

