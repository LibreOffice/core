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
'* owner : oliver.craemer@Sun.COM
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
