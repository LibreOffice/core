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
'* owner : wolfram.garten@oracle.com
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

