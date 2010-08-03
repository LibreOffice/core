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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : global-level-1-test -> insert all OLE-Objects out of OLE-dialog into all doc-types
'*
'\******************************************************************************

testcase tCheckTheOLEObjectDialog

   qaerrorlog( "#i80670# The OLE retrieval routine is broken beyond repair, redesign required" )
   goto endsub

   Dim lListe (10) as String : Dim lListSoll (10) as String
   Dim i, j as Integer
   Dim iCount, sCount as Integer
   
   call ClosePresentationfloat()
   GetOLEDefaultNames()

   for i=1 to 5
   
      select case i
         case 1 : gApplication = "WRITER"
         case 2 : gApplication = "CALC"
         case 3 : gApplication = "IMPRESS"
         case 4 : gApplication = "DRAW"
         case 5 : gApplication = "MASTERDOCUMENT"
      end select
      
      lListe ( 0 ) = 0 : lListSoll ( 0 ) = 0
      printlog "check OLE-Object-Dialog for " + gApplication
      
      Call hNewDocument
      
      InsertObjectOLEObject

      Kontext "OLEObjektEinfuegen"
      if ( not OLEObjektEinfuegen.exists() ) then
        warnlog( "cannot access the OLE object dialog" )
        goto endsub
      endif
      
      NeuErstellen.Check
      WaitSlot()
      
      for j=1 to ObjektTyp.GetItemCount
         ObjektTyp.Select j
         ListAppend ( lListe(), ObjektTyp.GetSelText )
      next j
      
      ListSort ( lListe() )
      iCount = ListCount ( lListe() )
      
      select case gApplication
         case "WRITER"    : ListAppend ( lListSoll (), gOLECalc )
                            ListAppend ( lListSoll (), gOLEImpress )
                            ListAppend ( lListSoll (), gOLEDraw )
                            ListAppend ( lListSoll (), gOLEChart )
                            ListAppend ( lListSoll (), gOLEMath )
         case "CALC"      : ListAppend ( lListSoll (), gOLEWriter )
                            ListAppend ( lListSoll (), gOLEImpress )
                            ListAppend ( lListSoll (), gOLEDraw )
                            ListAppend ( lListSoll (), gOLEChart )
                            ListAppend ( lListSoll (), gOLEMath )
         case "DRAW"      : ListAppend ( lListSoll (), gOLECalc )
                            ListAppend ( lListSoll (), gOLEImpress )
                            ListAppend ( lListSoll (), gOLEWriter )
                            ListAppend ( lListSoll (), gOLEChart )
                            ListAppend ( lListSoll (), gOLEMath )
         case "IMPRESS"   : ListAppend ( lListSoll (), gOLECalc )
                            ListAppend ( lListSoll (), gOLEDraw )
                            ListAppend ( lListSoll (), gOLEWriter ) 
                            ListAppend ( lListSoll (), gOLEChart )
                            ListAppend ( lListSoll (), gOLEMath )
         case "MASTERDOCUMENT" : ListAppend ( lListSoll (), gOLECalc ) 
                            ListAppend ( lListSoll (), gOLEImpress )
                            ListAppend ( lListSoll (), gOLEDraw ) 
                            ListAppend ( lListSoll (), gOLEChart )
                            ListAppend ( lListSoll (), gOLEMath )
      end select
      
      if ( gPlatgroup = "w95" ) then 
         ListAppend ( lListSoll (), gOLEOthers )
         endif
         
      ListSort ( lListSoll () )
      sCount = ListCount ( lListSoll () )
      
      if iCount <> sCount then
         Warnlog "Expected: " + sCount + "   Found: " + iCount
      else
         for j=1 to sCount
            if lListe (j) <> lListSoll (j) then 
               Warnlog "Expected: '" + lListSoll (j) + "'   Found: '" + lListe (j) +"'"
               endif
         next j
      end if
      
      OLEObjektEinfuegen.Cancel
      hCloseDocument()
   next i
endcase

