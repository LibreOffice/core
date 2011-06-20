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
'* short description : OASIS OpenDocument Fileformat Test (Presentation and Drawing)
'*
'\***********************************************************************************

sub main   
    printlog "-------------------------------------------------------------------"
    printlog "- OASIS OpenDocument file format Test (Presentation and Drawing)  -"
    printlog "-------------------------------------------------------------------"
       
    use "xml\required\includes\oasis_graphics_01.inc"
       
    call hStatusIn ("XML" , "oasis_graphics_update.bas")
        call talien_attributes
        call t_all_header_footer
        call t_no_header_footer
        call t_no_header_footer_master_elements
        call t_some_header_footer              
    call hStatusOut
end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\tools\includes\optional\t_xml2.inc"
   Call GetUseFiles   
end sub

