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
'* short description : Conversion of Microsoft OLE objects
'*
'\************************************************************************

sub main        
    Printlog "--------------------------------------------------"
    Printlog "---    MS OLE -> OpenDocument OLE conversion   ---"
    Printlog "--------------------------------------------------"

    use "xml/optional/includes/f_xml_save_ms_ole.inc"
    use "xml/tools/includes/xmltool1.inc"
   
    Call hStatusIn("XML", "f_xml_save_ms_ole.bas")   
        call hEnablePrettyPrinting()
        call sConfigurationManagement()  
        Call tXML_OLE_Conversion ( "excelOLE.doc" )
        Call tXML_OLE_Conversion ( "excelOLE.ppt" )
        Call tXML_OLE_Conversion ( "mathOLE.doc" )
        Call tXML_OLE_Conversion ( "mathOLE.ppt" )
        Call tXML_OLE_Conversion ( "mathOLE.xls" )
        Call tXML_OLE_Conversion ( "pptOLE.doc" )
        Call tXML_OLE_Conversion ( "pptOLE.xls" )
        Call tXML_OLE_Conversion ( "wordOLE.ppt" )
        Call tXML_OLE_Conversion ( "wordOLE.xls" )
        'Reset Tools::Options::Load/Save::MS Office to default values
        call subSetConversionMicrosoftOLE ( "RESET" ) 
    Call hStatusOut                                 
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_xml1.inc"    
    gApplication   = "CALC"    
    Call GetUseFiles
end sub
'
'-------------------------------------------------------------------------------
'
sub sConfigurationManagement
    'Ensure that paths are stored relatively
    printlog "Tools / Options"
    ToolsOptions
    printlog "Load/Save / General"
    hToolsOptions ( "LoadSave", "General" )
    Kontext "TabSpeichern"
    printlog "Check URL in file system"
    URLimDateisystem.Check
    printlog "OK"
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
end sub
