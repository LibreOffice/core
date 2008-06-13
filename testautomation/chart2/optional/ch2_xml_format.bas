'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: ch2_xml_format.bas,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 14:26:55 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/***********************************************************************
'*
'* owner : oliver.craemer@sun.com
'*
'* short description : General XML file format check
'*
'\***********************************************************************


sub main         
    Printlog "--------------------------------------------------"
    Printlog "---    Chart2 General XML file format check    ---"
    Printlog "--------------------------------------------------"
    
    use "chart2/loadsave/inc/ch2_xml_format.inc"
    use "chart2/tools/ch_tools_common.inc"
    
    Call hStatusIn("Chart2", "ch2_xml_format.bas")                    
        call tCompareXMLFormatCreate    
        call tCompareXMLFormatLoad
    Call hStatusOut
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global/system/inc/master.inc"
    use "global/system/inc/gvariabl.inc"
    use "global/tools/inc/t_doc2.inc"
    call getusefiles            
end sub
