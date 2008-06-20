'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: f_basic_documents.bas,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 06:59:25 $
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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : BASIC organizers and documents
'*
'\******************************************************************************

sub main

    use "framework\optional\includes\basic_documents.inc"


    call hStatusIn( "framework" , "f_basic_documents.bas" )
    call tBasicDocuments
    call hStatusOut()

end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"

    use "global\tools\includes\optional\basicide_tools.inc"
    use "global\tools\includes\optional\basicorganizer_tools.inc"
    use "global\tools\includes\optional\filetools.inc"
    use "global\tools\includes\optional\docfuncs.inc"
    use "global\tools\includes\optional\treelist_tools.inc"
    use "global\tools\includes\optional\accels.inc"
    use "global\tools\includes\optional\security_tools.inc"
    use "global\tools\includes\optional\listfuncs.inc"
    
    use "framework\tools\includes\init_tools.inc"


    gApplication = "WRITER"
    call GetUseFiles()

end sub

'-------------------------------------------------------------------------
