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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'**************************************************************************************
' #1 tCrossfading
'\*************************************************************************************

testcase tCrossfading
    dim X as integer
    dim Z as integer
    dim Y as integer
    dim I as integer
    dim CheckX as integer
    dim FirstX as integer
    dim CounterForX as integer
    dim anothercounter as integer

    printlog "New Impress document"
    Call hNewDocument
    Call sSelectEmptyLayout					                
    gMouseClick 50,50
    sleep 1
    printlog "Create rectangle"
    Call hRechteckErstellen ( 5, 5, 20, 30 )			
    gMouseClick 90,90
    printlog "create rectangle"
    Call hRechteckErstellen ( 30, 30, 70, 70 )			
    sleep 1
    printlog "Select all"
    EditSelectAll							            
    sleep 1
    printlog "Choose Edit / CrossFading"
    EditCrossFading                                     
    kontext "Ueberblenden"
    printlog "Get counter-value and save it in X"
    X = Schritte.GetText									
    printlog "Steps before we start the test: " + X
    printlog "Schritte.GetText= " + Schritte.GetText
    
    Schritte.ToMin
    if Schritte.GetText <> "1" then qaerrorlog "Could not change the step-number to 1."
    printlog "How many steps do you want?"
    for I = 2 to 10										
        Schritte.More 1
            if Schritte.GetText <> I then qaerrorlog "Could not change the step-number to" + I
                Ueberblenden.OK
                sleep 1
    try
    printlog "open context menu and ungroup objects"
    FormatUngroupDraw							
    catch
    qaerrorlog "Could not ungroup objects"
    endcatch
    sleep 1
    printlog "Should make the first object active"
    hTypeKeys "<TAB>"								
    
    printlog "Get position and dimensions of elements"
      try
         ContextPositionAndSize
      catch
         qaerrorlog "Couldnt call 'ContextPositionAndSize' - No object selected?"
      endcatch
   kontext
   active.SetPage TabPositionAndSize
   sleep 1
   kontext "TabPositionAndSize"
   if TabPositionAndSize.exists (5) then
         CounterForX=0
         CheckX=0
         printlog "Tet Dimensions of first object"
         FirstX=PositionX.GetText								
         TabPositionAndSize.OK
   else
      qaerrorlog "Couldn't switch tab page :-( "
   endif

    printlog "How many objects we have on the screen"
    anothercounter = I+2

    printlog "How many times we should step to (hopefully) get back to the first one"
   for Y = 1 to anothercounter							
      if Y > anothercounter then warnlog "Too many steps, something is wrong."
      try
        printlog "Step to the next object"
         hTypeKeys "<TAB>"								
      catch
         errolog "Could not step to the next object"
      endcatch
'   Printlog "- Get position and dimensions of elements"
    printlog "Open Position and Size for this object"
      ContextPositionAndSize								    
      sleep 1
      Kontext
      Active.SetPage TabPositionAndSize
         Kontext "TabPositionAndSize"
         printlog "Get Position X for this object"
         CheckX=PositionX.GetText								
         sleep 1
      TabPositionAndSize.OK
   Kontext "DocumentDraw"
   printlog "Up one on the object-counter"
      CounterForX=CounterForX+1
      printlog "Check if Position is the same as the first one"
      if CheckX=FirstX then 									
         if anothercounter = CounterForX then printlog "The first box is at: " + CheckX + " And we're now at " + FirstX + " = Correct steps between the objects, everything's fine."
         printlog "We should have " + Y	+ " objects to go through."  		
         printlog "Number of objects: " + anothercounter
         printlog ""
      endif
      printlog "End or go on with the checking-loop"
   next Y														
   printlog "Select all"
      EditSelectAll
      printlog "Delete all"
      EditDelete                                          		
   sleep 1
   printlog "Create rectangle"
Call hRechteckErstellen ( 5, 5, 20, 30 )	
    gMouseClick 90,90
    printlog "Create rectangle"
    Call hRechteckErstellen ( 30, 30, 70, 70 )
    sleep 1
    printlog "Select all"
    EditSelectAll
    sleep 1
    printlog "Choose Edit / CrossFading"
    EditCrossFading
    sleep 2
    kontext "Ueberblenden"
    next I

    printlog "And now we're gonna check if we can separate the object into it's parts"
    printlog "And check if the numbers of steps / objects is correct"

    printlog "Select minimum value for number of steps"
    Schritte.ToMin                                            
    for Z = 1 to X -1
    printlog "Raise the value for number of steps with one"
    Schritte.More                                             
    next Z
    printlog "Value has been restored to: " + Schritte.GetText
    printlog "X is now: " + X
    Attributierung.UnCheck
    Ueberblenden.OK
    sleep 1
    Kontext "DocumentDraw"
    printlog "Select all"
    EditSelectAll
    printlog "Delete all"
    EditDelete                                          
    sleep 1
    printlog "Create rectangle"
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  
    gMouseClick 90,90
    printlog "Create second rectangle"
    Call hRechteckErstellen ( 30, 30, 70, 70 )
    printlog "Select all"
    EditSelectAll							              
    sleep 1
    printlog "Choose Edit / CrossFading"
    EditCrossFading                                     
    Kontext "Ueberblenden"
    if Attributierung.IsChecked = TRUE then qaerrorlog "Attributierung /    was NOT unchecked."
    printlog "Uncheck same orienation"
    GleicheOrientierung.UnCheck
    printlog "Close CrossFading-window with OK"
    Ueberblenden.OK									  
    sleep 1
    Kontext "DocumentDraw"
    printlog "Select all"
    EditSelectAll
    printlog "Delete all"
    EditDelete                                          
    sleep 1
    printlog "Create rectangle"
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  
    gMouseClick 90,90
    printlog "create rectangle"
    Call hRechteckErstellen ( 30, 30, 70, 70 )
    printlog "Select all"
    EditSelectAll							              
    sleep 1
    printlog "Choose Edit / CrossFading"
    EditCrossFading                                     
    Kontext "Ueberblenden"
    if GleicheOrientierung.IsChecked = TRUE then qaerrorlog "GleicheOrientierung /    was NOT unchecked."
    printlog "Check Attributing"
    Attributierung.Check
    printlog "Close CrossFading-window with OK"
    Ueberblenden.OK									  
    sleep 1
    Kontext "DocumentDraw"
    printlog "Select all"
    EditSelectAll							              
    sleep 1
    printlog "Delete all"
    EditDelete                                          
    sleep 1
    printlog "Create rectangle"
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  
    gMouseClick 90,90
    printlog "Create second rectangle"
    Call hRechteckErstellen ( 30, 30, 70, 70 )
    printlog "Select all"
    EditSelectAll							              
    sleep 1
    printlog "Choose Edit / CrossFading"
    EditCrossFading                                     
    Kontext "Ueberblenden"
    if Attributierung.IsChecked = FALSE then qaerrorlog "Attributierung /    was NOT checked again."
    printlog "Check same orientation"
    GleicheOrientierung.UnCheck							  
    Ueberblenden.OK
    sleep 1
    Kontext "DocumentDraw"
    printlog "Select all"
    printlog "Delete all"
    EditSelectAll									
    EditDelete										  
    sleep 1
    printlog "Create rectangle"
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  
    gMouseClick 90,90
    printlog "create rectangle"
    Call hRechteckErstellen ( 30, 30, 70, 70 )
    printlog "Select all"
    EditSelectAll				   						  
    sleep 1
    printlog "Choose Edit / CrossFading"
    EditCrossFading                                     
    Kontext "Ueberblenden"
    if GleicheOrientierung.IsChecked = TRUE then qaerrorlog "GleicheOrientierung /    was NOT checked again."    
    printlog "Check same orientation"
    GleicheOrientierung.Check
    printlog "Close CrossFading-window with OK"
    Ueberblenden.OK									  
    sleep 1
    Kontext "DocumentDraw"
    printlog "Select all"
    EditSelectAll
    printlog "Delete all"
    EditDelete
    printlog "Create rectangle"
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  
    gMouseClick 90,90
    printlog "create rectangle"
    Call hRechteckErstellen ( 30, 30, 70, 70 )
    printlog "Select all"
    EditSelectAll										  
    sleep 1
    printlog "Choose Edit / CrossFading"
    EditCrossFading                                     
    Kontext "Ueberblenden"
    if GleicheOrientierung.IsChecked = FALSE then qaerrorlog "GleicheOrientierung /    was NOT checked again."
    printlog "Check same orientation"
    GleicheOrientierung.Check
    printlog "Close CrossFading-window with OK"
    Ueberblenden.OK									  
    sleep 1

    printlog "EditCrossFading should not be enabled if more than two objects is selected."
    Kontext "DocumentDraw"
    printlog "Select all"
    EditSelectAll
    printlog "Delete all"
    EditDelete                                          
    
    sleep 1
    printlog "Create rectangle"
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  
    gMouseClick 90,90
    printlog "Create second rectangle"
    Call hRechteckErstellen ( 30, 30, 70, 70 )		  
    gMouseClick 90,90
    printlog "Create third rectangle"
    Call hRechteckErstellen ( 20, 20, 50, 50 )		  
    sleep 1
    printlog "Select all"
    EditSelectAll										  
    sleep 1
    try
        printlog "Choose Edit / CrossFading"
        EditCrossFading                                     
        sleep 2
        If active = "Ueberblenden" then
        qaerrorlog "CrossFading should NOT be selectable since more than two objects is selected."
        kontext "Ueberblenden"
        printlog "Close CrossFading-window with CANCEL"
        Ueberblenden.CANCEL								  
        endif
        sleep 2
        Kontext "DocumentDraw"
    catch
        printlog "CrossFading could not be selected when more than two objects selected: Correct."
    endcatch

    try
        EditSelectAll
        EditSelectAll
        printlog "Delete all"
        EditDelete
    catch
       warnlog "Could not select and delete all objects."
    endcatch

    printlog "End of test"

    printlog "Close document"
    Call hCloseDocument						              
 
endcase 'tCrossfading