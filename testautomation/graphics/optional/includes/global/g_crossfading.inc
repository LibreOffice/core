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

    Call hNewDocument
    Call sSelectEmptyLayout					                '/// New Impress document ///'
    gMouseClick 50,50
   sleep 1
    Call hRechteckErstellen ( 5, 5, 20, 30 )			'/// Create rectangle ///'
    gMouseClick 90,90
    Call hRechteckErstellen ( 30, 30, 70, 70 )			'/// create rectangle ///'
   sleep 1
    EditSelectAll							            '/// Select all ///'
   sleep 1
    EditCrossFading                                     '/// Choose Edit / CrossFading ///'
    kontext "Ueberblenden"
    X = Schritte.GetText									' /// Get counter-value and save it in X ///'
    printlog "Steps before we start the test: " + X
    printlog "Schritte.GetText= " + Schritte.GetText
    
    Schritte.ToMin
    if Schritte.GetText <> "1" then qaerrorlog "Could not change the step-number to 1."

    for I = 2 to 10										' /// How many steps do you want? ///'
        Schritte.More 1
            if Schritte.GetText <> I then qaerrorlog "Could not change the step-number to" + I
                Ueberblenden.OK
                sleep 1
    try
    FormatUngroupDraw							'/// open context menu and ungroup objects ///'
    catch
    qaerrorlog "Could not ungroup objects"
    endcatch
    sleep 1
    hTypeKeys "<TAB>"								'/// Should make the first object active ///'
    
    '/// Get position and dimensions of elements ///'
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
         FirstX=PositionX.GetText								'/// Tet Dimensions of first object ///'
         TabPositionAndSize.OK
   else
      qaerrorlog "Couldn't switch tab page :-( "
   endif


anothercounter = I+2										'/// How many objects we have on the screen ///'

   for Y = 1 to anothercounter							'/// How many times we should step to (hopefully) get back to the first one ///'
      if Y > anothercounter then warnlog "Too many steps, something is wrong."
      try
         hTypeKeys "<TAB>"								'/// Step to the next object ///'
      catch
         errolog "Could not step to the next object"
      endcatch
'   Printlog "- Get position and dimensions of elements"
      ContextPositionAndSize								    '/// Open Position and Size for this object ///'
      sleep 1
      Kontext
      Active.SetPage TabPositionAndSize
         Kontext "TabPositionAndSize"
         CheckX=PositionX.GetText								'/// Get Position X for this object ///'
         sleep 1
      TabPositionAndSize.OK
   Kontext "DocumentDraw"
      CounterForX=CounterForX+1 											'/// Up one on the object-counter ///'
      if CheckX=FirstX then 									'/// Check if Position is the same as the first one ///'
         if anothercounter = CounterForX then printlog "The first box is at: " + CheckX + " And we're now at " + FirstX + " = Correct steps between the objects, everything's fine."
         printlog "We should have " + Y	+ " objects to go through."  		'/// Just here for debugging. - FHA
'	  	 printlog "anothercounter= " + anothercounter   '/// Just here for debugging. - FHA
         printlog "Number of objects: " + anothercounter
         printlog ""
      endif
   next Y														'/// End or go on with the checking-loop ///'

      EditSelectAll							            		'/// Select all ///'
      EditDelete                                          		'/// Delete all ///'
   sleep 1
      Call hRechteckErstellen ( 5, 5, 20, 30 )			  		'/// Create rectangle ///'
    gMouseClick 90,90
      Call hRechteckErstellen ( 30, 30, 70, 70 )		  		'/// Create rectangle ///'
    sleep 1
      EditSelectAll							              		'/// Select all ///'
    sleep 1
      EditCrossFading                                     		'/// Choose Edit / CrossFading ///'
      sleep 2
        kontext "Ueberblenden"
    next I

    '/// And now we're gonna check if we can separate the object into it's parts ///'
    '/// And check if the numbers of steps / objects is correct ///'

    Schritte.ToMin                                            '/// Select minimum value for number of steps ///'
    for Z = 1 to X -1
    Schritte.More                                             '/// Raise the value for number of steps with one ///'
    next Z
    printlog "Value has been restored to: " + Schritte.GetText
    printlog "X is now: " + X
    Attributierung.UnCheck
    Ueberblenden.OK
    sleep 1
    Kontext "DocumentDraw"
    EditSelectAll							              '/// Select all ///'
    EditDelete                                          '/// Delete all ///'
    sleep 1
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  '/// Create rectangle ///'
    gMouseClick 90,90
    Call hRechteckErstellen ( 30, 30, 70, 70 )		  '/// Create second rectangle ///'
    EditSelectAll							              '/// Select all ///'
    sleep 1
    EditCrossFading                                     '/// Choose Edit / CrossFading ///'
    Kontext "Ueberblenden"
    if Attributierung.IsChecked = TRUE then qaerrorlog "Attributierung /    was NOT unchecked."
    GleicheOrientierung.UnCheck							  '/// Uncheck "same orienation" ///'   -????
    Ueberblenden.OK									  '/// Close CrossFading-window with "OK" ///'
    sleep 1
    Kontext "DocumentDraw"
    EditSelectAll							              '/// Select all ///'
    EditDelete                                          '/// Delete all ///'
    sleep 1
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  '/// Create rectangle ///'
    gMouseClick 90,90
    Call hRechteckErstellen ( 30, 30, 70, 70 )		  '/// create rectangle ///'
    EditSelectAll							              '/// Select all ///'
    sleep 1
    EditCrossFading                                     '/// Choose Edit / CrossFading ///'
    Kontext "Ueberblenden"
    if GleicheOrientierung.IsChecked = TRUE then qaerrorlog "GleicheOrientierung /    was NOT unchecked."
    Attributierung.Check							          '/// Check "Attributing" ///'  - ???
    Ueberblenden.OK									  '/// Close CrossFading-window with "OK" ///'
    sleep 1
    Kontext "DocumentDraw"
    EditSelectAll							              '/// Select all ///'
    sleep 1
    EditDelete                                          '/// Delete all ///'
    sleep 1
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  '/// Create rectangle ///'
    gMouseClick 90,90
    Call hRechteckErstellen ( 30, 30, 70, 70 )		  '/// Create second rectangle ///'
    EditSelectAll							              '/// Select all ///'
    sleep 1
    EditCrossFading                                     '/// Choose Edit / CrossFading ///'
    Kontext "Ueberblenden"
    if Attributierung.IsChecked = FALSE then qaerrorlog "Attributierung /    was NOT checked again."
    GleicheOrientierung.UnCheck							  '/// Check "same orientation" ///'   -????
    Ueberblenden.OK
    sleep 1
    Kontext "DocumentDraw"
    EditSelectAll										  '/// Select all ///'
    EditDelete										  '/// Delete all ///'
    sleep 1
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  '/// Create rectangle ///'
    gMouseClick 90,90
    Call hRechteckErstellen ( 30, 30, 70, 70 )		  '/// create rectangle ///'
    EditSelectAll				   						  '/// Select all ///'
    sleep 1
    EditCrossFading                                     '/// Choose Edit / CrossFading ///'
    Kontext "Ueberblenden"
    if GleicheOrientierung.IsChecked = TRUE then qaerrorlog "GleicheOrientierung /    was NOT checked again."
    GleicheOrientierung.Check							  '/// Check "same orientation" ///'   -????
    Ueberblenden.OK									  '/// Close CrossFading-window with "OK" ///'
    sleep 1
    Kontext "DocumentDraw"
    EditSelectAll			  							  '/// Select all ///'
    EditDelete										  '/// Delete all ///'
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  '/// Create rectangle ///'
    gMouseClick 90,90
    Call hRechteckErstellen ( 30, 30, 70, 70 )		  '/// create rectangle ///'
    EditSelectAll										  '/// Select all ///'
    sleep 1
    EditCrossFading                                     '/// Choose Edit / CrossFading ///'
    Kontext "Ueberblenden"
    if GleicheOrientierung.IsChecked = FALSE then qaerrorlog "GleicheOrientierung /    was NOT checked again."
    GleicheOrientierung.Check							  '/// Check "same orientation" ///'   -????
    Ueberblenden.OK									  '/// Close CrossFading-window with "OK" ///'
    sleep 1
    
    Kontext "DocumentDraw"                                 '/// EditCrossFading should not be enabled if more than two objects is selected. ///'
    EditSelectAll										  '/// Select all ///'
    EditDelete                                          '/// Delete all ///'
    
    sleep 1
    Call hRechteckErstellen ( 5, 5, 20, 30 )			  '/// Create rectangle ///'
    gMouseClick 90,90
    Call hRechteckErstellen ( 30, 30, 70, 70 )		  '/// Create second rectangle ///'
    gMouseClick 90,90
    Call hRechteckErstellen ( 20, 20, 50, 50 )		  '/// Create third rectangle ///'
    sleep 1
    EditSelectAll										  '/// Select all ///'
    sleep 1
    try
        EditCrossFading                                     '/// Choose Edit / CrossFading ///'
        sleep 2
        If active = "Ueberblenden" then
        qaerrorlog "CrossFading should NOT be selectable since more than two objects is selected."
        kontext "Ueberblenden"
        Ueberblenden.CANCEL								  '/// Close CrossFading-window with "CANCEL" ///'
        endif
        sleep 2
        Kontext "DocumentDraw"
    catch
        printlog "CrossFading could not be selected when more than two objects selected: Correct."
    endcatch

    try
       EditSelectAll										  '/// Select all ///'
       EditDelete                                          '/// Delete all ///'
    catch
       warnlog "Could not select and delete all objects."
    endcatch

    printlog "End of test"

    Call hCloseDocument						              '/// Close document ///'
 
endcase 'tCrossfading
