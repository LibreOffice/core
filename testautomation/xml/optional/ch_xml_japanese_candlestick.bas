'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: ch_xml_japanese_candlestick.bas,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-07-11 07:26:56 $
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
'/************************************************************************
'*
'* owner : oliver.creamer@Sun.COM
'*
'* short description  :  Detailed test of the chart types
'*
'\***********************************************************************
sub main       
    use "xml/optional/includes/ch_xml_japanese_candlestick.inc"
    use "xml/tools/includes/xmltool1.inc"
    
    Printlog "--------------------------------------------------"
    Printlog "---         Japanese Candlestick Chart         ---"
    Printlog "--------------------------------------------------"
    
    Call hStatusIn("XML", "ch_xml_japanese_candlestick.bas")                
        call hEnablePrettyPrinting(1)     
        call tJapaneseCandlestickChart
    Call hStatusOut
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global/system/includes/master.inc"
    use "global/system/includes/gvariabl.inc"
    gApplication   = "CALC"
    call getusefiles        
end sub
