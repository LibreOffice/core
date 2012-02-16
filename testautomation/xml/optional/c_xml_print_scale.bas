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
'* short description  :  Lookup for correct attributes for calc print scaling
'*
'\***********************************************************************

sub main    
    Printlog "--------------------------------------------------"
    Printlog "---          Print scaling attributes          ---"
    Printlog "--------------------------------------------------"

    use "xml/optional/includes/c_xml_print_scale.inc"
    use "xml/tools/includes/xmltool1.inc"
    
    Call hStatusIn("XML", "c_xml_print_scale.bas")
        call hEnablePrettyPrinting(1)     
        call subPrintScaling
    Call hStatusOut
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global/system/includes/master.inc"
    use "global/system/includes/gvariabl.inc"    
    gApplication   = "CALC"
    call GetUseFiles        
end sub
