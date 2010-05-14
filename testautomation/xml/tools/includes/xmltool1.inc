'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: xmltool1.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-07-11 10:15:59 $
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
'* owner : wolfram.garten@sun.com
'*
'* short description : XML Tools
'*
'***************************************************************************************
'*
' #1 CreateWorkXMLExportDir
' #1 hEnablePrettyPrinting
'*
'\***********************************************************************************
sub CreateWorkXMLExportDir ( DirName as string )
'/*** <i>CreateWorkXMLExportDir</i> creates in the StarOffice/work-
'/*** +directory a directory. If the same name exists the content (files only)
'/*** +will be deleted.
 Dim OutputPath as string
 Dim Isliste(200) as string
 Dim i, BornToKill
 printlog "Checking work-dir & creating '" & gOfficePath & ConvertPath(DirName) & "'."
  if hDirectoryExists ( gOfficePath & ConvertPath(DirName) ) = TRUE then
    OutputPath = gOfficePath & ConvertPath(DirName)
    IsListe(0) = 0
    GetFileList ( OutputPath ,  "*", IsListe() )
    BornToKill = KillFileList ( IsListe() )
     for i = 1 to BornToKill
      warnlog IsListe(i) & " could not be deleted!"
     next i
   else
    app.mkDir( gOfficePath & ConvertPath(DirName))
  end if
end sub
'-------------------------------------------------------------------------
sub hEnablePrettyPrinting (OPTIONAL A)
'/// <i>hEnablePrettyPrinting</i> enables the pretty printing (=better
'/// +formated view) in XML file export. It takes >10 percent more time
'/// +to save files!!! OPTIONAL parameter is used to DISABLE it.
 ToolsOptions
 Kontext "ExtrasOptionenDLG"
  call hToolsOptions("loadsave","general")
 Kontext "TabSpeichern"
   if IsMissing(A) then
    if NoPrettyPrinting.IsChecked = TRUE then
     NoPrettyPrinting.Uncheck
     sleep(1)
     printlog "- XML pretty printing enabled ( xmltool1.inc )"
    end if
   else
    if NoPrettyPrinting.IsChecked = FALSE then
     NoPrettyPrinting.Check
     sleep(1)
     printlog "- XML pretty printing disabled ( xmltool1.inc )"
    end if
  end if
 Kontext "ExtrasOptionenDLG"
  ExtrasOptionenDLG.OK
end sub
'-------------------------------------------------------------------------

