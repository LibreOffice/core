'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: getnames.bas,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:06 $
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
'* owner : helge.delfs@sun.com
'*
'* short description : Creates lists of language dependent filters and OLE objects
'*
'************************************************************************
'*
' #1 GetOLEObjectNames 'Gets the language dependent OLE object names (UI)
' #0 LoadIncludeFiles  
'*
'\***********************************************************************
sub main

   if ( gPlatgroup = "unx" ) then
      warnlog( "Please use a Win32 version to get ALL OLE strings" )
      warnlog( "The <Further Objects> might not be retrieved correctly")
   endif
   
   call GetOLEObjectNames ' create the OLE-object-list
   
end sub

'-------------------------------------------------------------------------

testcase GetOLEObjectNames

   '///Create a list of default OLE objects for the current Office-release

   dim sOLENames (20) as string
       sOLENames( 0 ) = "0"
   dim sPath as string
   dim sFile as string

   sPath = convertpath( gOfficePath & "user\work" )
   sFile = convertpath( sPath & "\ole_" + iSprache + ".txt" )   
   
   dim sOLEItem as string
       
   printlog( "sPath = " & sPath )
   printlog( "sFile = " & sFile )

   '///<ul><li>Get the list for Writer and Calc application</li>      
   ' open a new document
   gApplication = "WRITER"
   call hNewDocument()
   
   ' open dialog "Insert->Object->OLE Object"
   InsertObjectOLEObject
   
   ' read the entries. Note that the order of the entries is significant,
   ' the short names (sc, sw ...) are english only while the OLE names 
   ' can be of any language
   Kontext "OLEObjektEinfuegen"
   sOLEItem = "CALC=" + Objekttyp.GetItemText(1)     ' calc
   call ListAppend ( sOLENames (), sOLEItem )

   sOLEItem = "CHART=" + Objekttyp.GetItemText(2)     ' chart
   call ListAppend ( sOLENames (), sOLEItem )

   sOLEItem = "DRAW=" + Objekttyp.GetItemText(3)     ' draw
   call ListAppend ( sOLENames (), sOLEItem )

   sOLEItem = "IMPRESS=" + Objekttyp.GetItemText(4)     ' impress
   call ListAppend ( sOLENames (), sOLEItem )

   sOLEItem = "MATH=" + Objekttyp.GetItemText(5)     ' math
   call ListAppend ( sOLENames (), sOLEItem )

   OLEObjektEinfuegen.Cancel()
   call hCloseDocument()

   ' open calc document
   gApplication = "CALC"
   call hNewDocument()
   
   ' open dialog "Insert->Object->OLE Object"
   InsertObjectOLEObject
   
   ' read the entries
   Kontext "OLEObjektEinfuegen"
   sOLEItem = "WRITER=" + Objekttyp.GetItemText(5)     ' writer
   call ListAppend ( sOLENames (), sOLEItem )
   
   sOLEItem = "OTHER=" + Objekttyp.GetItemText(6)     ' other
   call ListAppend ( sOLENames (), sOLEItem )
   
   OLEObjektEinfuegen.Cancel()
   call hCloseDocument()
   
   ' check if the output directory exists
   if ( app.dir( sPath ) = "" ) then
   
      printlog( "Directory does not exist, trying to create it." )
      mkdir( sPath ) 
      
   endif
   
   '///<li>write the file to ...global/input/olenames/<app-name></li></ul>
   if ( app.dir( sPath ) <> "" ) then
   
      printlog( "Write list to file" )
      call ListWrite ( sOLENames(), sFile, "utf8" )
      
   else
   
      warnlog( "Failed to write filterlist, the targetdirectory does not exist" )
      
   endif   

   ' Warn that the file has been written.
   warnlog ( "*** NOTE: Manual action required ***" )
   printlog( "Please check " + sFile + " ( UTF8 encoded )!" )
   printlog( "The file containing the newly created OLE-name-lists has to be " )
   printlog( "checked into cvs manually. Its location is: " )
   printlog( gTesttoolPath & "\global\input\olenames\" & gProductName & "\ole_{language-code}.txt" )

endcase

'-------------------------------------------------------------------------

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   gApplication = "WRITER"
   call GetUseFiles
end sub

'-------------------------------------------------------------------------

