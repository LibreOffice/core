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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Functiontest for matrix/array constants in calc
'*
'\***********************************************************************


sub main       
    use "spreadsheet/optional/includes/arrayconstants/c_arrayconstants.inc"
    use "spreadsheet/tools/includes/c_cell_tools.inc"
    use "spreadsheet/tools/includes/c_select_tools.inc"
    use "spreadsheet/tools/includes/c_l10n_tools.inc"
    
    Printlog "--------------------------------------------------"
    Printlog "---  Functiontest for arrayconstants in calc   ---"
    Printlog "--------------------------------------------------"
    
    Call hStatusIn("spreadsheet", "c_arrayconstants.bas")                
    
    Call tArrayconstants01
    Call tArrayconstants02  
        
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
