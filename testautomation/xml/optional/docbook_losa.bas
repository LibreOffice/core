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
'* owner : helge.delfs@oracle.com
'*
'* short description : DocBook XML Import-/Export Test
'*
'\***********************************************************************
Global sSourcePath as string
Global sDestinationPath as string
sub main
        printlog "---------------------------------------------------------------------"
        printlog "-----      D o c B o o k   -   I M -   /   E X P O R T          -----"
        printlog "---------------------------------------------------------------------"

        use "xml\optional\includes\docbook_001.inc"
        use "xml\tools\includes\xmltool1.inc"                   
  
        call hStatusIn ("XML","docbook_losa.bas")
            printlog "xml::tools::xmltool1.inc::CreateWorkXMLExportDir: Creating 'user\work\xml\docbook-testdocs-1.1\tests'-directory." 
            call CreateWorkXMLExportDir ( "user\work\xml\docbook" )   
            sSourcePath = ConvertPath (gTestToolPath & "xml\optional\input\docbook" )
            sDestinationPath = gOfficePath & ConvertPath("user\work\xml\docbook") 
            call tDocBookXMLtestdocs  
            call tToolsXMLFilterSettings
        call hStatusOut   
end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_xml2.inc"
    use "global\tools\includes\optional\t_xml_filter1.inc"    
    gApplication   = "WRITER"   
    Call GetUseFiles   
end sub

