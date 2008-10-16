'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: f_filedlg_dialogtest.bas,v $
'*
'* $Revision: 1.3 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 08:07:02 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and\or modify
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
'* <http:\\www.openoffice.org\license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
' **
' ** owner : joerg.skottke@sun.com
' **
' ** short description : check the internal file dialog (Dialog)
' **
'\******************************************************************************

sub main

    use "framework\optional\includes\filedlg_triggers.inc"
    use "framework\optional\includes\filedlg_rename.inc"
    use "framework\optional\includes\filedlg_document_properties.inc"
    use "framework\optional\includes\filedlg_filternames.inc"

    call hStatusIn ( "framework", "f_filedlg_dialogtest.bas" )
    call tDialogTriggers()         
    call tFiledlgDocumentProperties()
    call tVerifyFilterNames()
    call tFileRename()
    call hStatusOut

end sub

' ****************************************

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
   
    use "global\tools\includes\optional\t_stringtools.inc"
    use "global\tools\includes\optional\t_listfuncs.inc"    
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_accels.inc"

    use "framework\tools\includes\tabpages.inc"
   
    gApplication = "WRITER"
    Call GetUseFiles

end sub
