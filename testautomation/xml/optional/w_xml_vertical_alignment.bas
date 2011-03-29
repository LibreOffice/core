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
'* short description  :  Testing for vertical alignment in writer
'*
'\***********************************************************************

sub main       
    use "xml/tools/includes/xmltool1.inc"
    use "xml/optional/includes/w_xml_vertical_alignment.inc"                
    
    Printlog "--------------------------------------------------"
    Printlog "---         Vertical Alignment Writer          ---"
    Printlog "--------------------------------------------------"
    
    Call hStatusIn("XML", "w_xml_vertical_alignment.bas")                
        call hEnablePrettyPrinting(1)     
        call tVerticalAlignmentWriter
    Call hStatusOut
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global/system/includes/master.inc"
    use "global/system/includes/gvariabl.inc"
    use "global\tools\includes\optional\t_ui_filters.inc"
    gApplication   = "WRITER"
    call getusefiles        
end sub
